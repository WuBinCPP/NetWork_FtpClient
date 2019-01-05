#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <QObject>
#include <QHash>

class QFile;
class QFtp;
class QUrlInfo;
class QProgressDialog;

class FtpClient : public QObject
{
	Q_OBJECT

public:

    FtpClient(QObject *parent);
    ~FtpClient();

    void setserverName(const QString& name){serverName = name;}
    void setserverPort(const QString& port){serverPort = port;}
    void setuserName(const QString& name){userName = name;}
    void setpassWord(const QString& pwd){passWord = pwd;}

    bool isDir(QString fileName){return DirectoryList.value(fileName);}
    QString getStatus(){return ftpStatus;}
	void ftpConnect();
	void changeDir(const QString&);
	void getFile(const QString&);
signals:
    void sigConncted(bool isconnect);
    void sigGot(bool error);
    void sigList(bool error);
    void sigIsTopDir(bool istop);
    void sigChangeList(const QUrlInfo &);
    void sigDownloading();
public slots:
	void ftpCommandFinished(int commandId, bool error);
	void updateDataTransferProgress(qint64 readBytes,
		qint64 totalBytes);
	void addToList(const QUrlInfo& );
	void cancelDownload();
private:

    QHash<QString, bool> DirectoryList;
	QString currentPath;
	QString downloadFileName;
	QString serverName;
	QString serverPort;
	QString userName;
	QString passWord;
	QString ftpStatus;
	QFtp *ftp;
	QFile *file;
	QProgressDialog *progressDialog;


};

#endif // FTPCLIENT_H
