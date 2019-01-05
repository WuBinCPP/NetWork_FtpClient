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
         * �г�FTP��������Ŀ¼dir������,���������������������ء�
         * Ϊ�ҵ���ÿ��Ŀ¼���listInfo()�źš�
         * ��������һ��Ωһ�ı�ʶ�����ñ�ʶ����commandstart()��commandFinished()���ݡ�
         * ����������ʱ��commandStarted()�źŻᷢ�����������ʱ��commandFinished()�źű�������
         */
		ftpStatus = tr("����FTP������ %1...")
			.arg(serverName);
        emit sigConncted(true);
	}
	else {
        ftp->abort();           //��ֹ��ǰ���ɾ�����мƻ������
        ftp->deleteLater();     //���˶������Ϊɾ����
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
			ftpStatus =tr("�޷�����FTP�������� "
				"%1. �������������˿ڡ��û����������Ƿ���ȷ ")
				.arg(serverName);
            emit sigConncted(false);
			return;
		}
		ftpStatus =tr("������FTP��������%1�� ").arg(serverName);
        emit sigConncted(true);
		return;
	}

	if (ftp->currentCommand() == QFtp::Get) {
		QApplication::restoreOverrideCursor();
		if (error) {
			ftpStatus =tr("ȡ�������ļ�%1.")
				.arg(file->fileName());
			file->close();
			file->remove();
            emit sigGot(true);
		} else {
			ftpStatus = tr("�����ļ� %1 ����ǰĿ¼")
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
	if(dir.isEmpty())//���ظ�Ŀ¼
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
	else//������Ŀ¼
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
			tr("%1�ļ��Ѵ��ڡ�")
			.arg(fileName));
		return;
	}

	file = new QFile(fileName);
	if (!file->open(QIODevice::WriteOnly)) {
		QMessageBox::information(0, tr("FTP"),
			tr("�޷������ļ� %1: %2.")
			.arg(fileName).arg(file->errorString()));
		delete file;
		return;
	}

	ftp->get(fileName, file);

	progressDialog->setLabelText(tr("�����ļ�%1...").arg(fileName));
	progressDialog->show();
    emit sigDownloading();

}

void FtpClient::updateDataTransferProgress(qint64 readBytes,qint64 totalBytes)
{
	progressDialog->setMaximum(totalBytes);
	progressDialog->setValue(readBytes);

}
