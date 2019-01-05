#include "ftpclient.h"
#include <QtNetwork>
#include <QtGui>

FtpClient::FtpClient(QObject *parent)
	: QObject(parent),ftp(NULL),file(NULL)
{
	progressDialog = new QProgressDialog((QWidget *)parent);

    connect(progressDialog, SIGNAL(canceled()),\
            this, SLOT(cancelDownload()));

}

FtpClient::~FtpClient()
{

}

void FtpClient::ftpConnect()
{
	if (ftp ==NULL) {
        //QApplication::setOverrideCursor(Qt::WaitCursor);
		ftp = new QFtp(this);

        connect(ftp, SIGNAL(commandFinished(int, bool)),\
                this, SLOT(ftpCommandFinished(int, bool)));

        connect(ftp, SIGNAL(listInfo(const QUrlInfo &)),\
                this, SLOT(addToList(const QUrlInfo &)));

        connect(ftp, SIGNAL(dataTransferProgress(qint64, qint64)),\
                this, SLOT(updateDataTransferProgress(qint64, qint64)));

		ftp->connectToHost(serverName,serverPort.toUShort());
		ftp->login(userName,passWord);
		ftp->list();
        /* ---------------------------QFtp::list()----------------------------
         * 列出FTP服务器上目录dir的内容,函数不会阻塞并立即返回。
         * 为找到的每个目录项发出listInfo()信号。
         * 函数返回一个惟一的标识符，该标识符由commandstart()和commandFinished()传递。
         * 当命令启动时，commandStarted()信号会发出。当它完成时，commandFinished()信号被发出。
         */
		ftpStatus = tr("连接FTP服务器 %1...")
			.arg(serverName);
        emit sigConncted(true);
	}
	else {
        ftp->abort();           //终止当前命令并删除所有计划的命令。
        ftp->deleteLater();     //将此对象调度为删除。
		ftp = NULL;
        emit sigConncted(false);
	}

}

void FtpClient::cancelDownload()
{
		ftp->abort();
}

void FtpClient::ftpCommandFinished(int /*commandId*/, bool error)
{

	if (ftp->currentCommand() == QFtp::Login) {
		if (error) {
			QApplication::restoreOverrideCursor();
			ftpStatus =tr("无法连接FTP服务器： "
				"%1. 请检查主机名、端口、用户名和密码是否正确 ")
				.arg(serverName);
            emit sigConncted(false);
			return;
		}
		ftpStatus =tr("已连接FTP服务器：%1。 ").arg(serverName);
        emit sigConncted(true);
		return;
	}

	if (ftp->currentCommand() == QFtp::Get) {
		QApplication::restoreOverrideCursor();
		if (error) {
			ftpStatus =tr("取消下载文件%1.")
				.arg(file->fileName());
			file->close();
			file->remove();
            emit sigGot(true);
		} else {
			ftpStatus = tr("下载文件 %1 到当前目录")
				.arg(file->fileName());
			file->close();
            emit sigGot(false);
		}
		delete file;
	} else if (ftp->currentCommand() == QFtp::List) {
		QApplication::restoreOverrideCursor();
        if (DirectoryList.isEmpty()) {
            emit sigList(true);
		}
	}
}

void FtpClient::addToList(const QUrlInfo &urlInfo)
{

    //DirectoryList[urlInfo.name()] = urlInfo.isDir();
    DirectoryList.insert(urlInfo.name(),urlInfo.isDir());
    emit sigChangeList(urlInfo);
}


void FtpClient::changeDir(const QString& dir)
{
	if(dir.isEmpty())//返回父目录
	{
		QApplication::setOverrideCursor(Qt::WaitCursor);
        DirectoryList.clear();
		currentPath = currentPath.left(currentPath.lastIndexOf('/'));
		if (currentPath.isEmpty()) {
			ftp->cd("/");
            emit sigIsTopDir(true);
		} else {
			ftp->cd(currentPath);
		}
		ftp->list();
	}
	else//进入子目录
	{
        if (DirectoryList.value(dir)) {
            DirectoryList.clear();
			currentPath += "/" + dir;
			ftp->cd(dir);
			ftp->list();
            emit sigIsTopDir(false);
			QApplication::setOverrideCursor(Qt::WaitCursor);
		}
	}
}

void FtpClient::getFile(const QString& fileName)
{
	if (QFile::exists(fileName)) {
		QMessageBox::information(0, tr("FTP"),
			tr("%1文件已存在。")
			.arg(fileName));
		return;
	}

	file = new QFile(fileName);
	if (!file->open(QIODevice::WriteOnly)) {
		QMessageBox::information(0, tr("FTP"),
			tr("无法保存文件 %1: %2.")
			.arg(fileName).arg(file->errorString()));
		delete file;
		return;
	}

	ftp->get(fileName, file);

	progressDialog->setLabelText(tr("下载文件%1...").arg(fileName));
	progressDialog->show();
    emit sigDownloading();

}

void FtpClient::updateDataTransferProgress(qint64 readBytes,qint64 totalBytes)
{
	progressDialog->setMaximum(totalBytes);
	progressDialog->setValue(readBytes);

}
