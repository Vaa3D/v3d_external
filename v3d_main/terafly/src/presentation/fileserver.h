#ifndef FILESERVER_H
#define FILESERVER_H

#include <QtNetwork>
class FileSocket:public QTcpSocket
{
    Q_OBJECT
public:
    explicit FileSocket(QObject *parent=0);

public slots:
    void readFile();
signals:
    void received(QString/*,QString*/);
//    void filesaved();
private:
    quint64 totalsize;
    quint64 filenamesize;
    quint64 m_bytesreceived;

};

class FileServer:public QTcpServer
{
    Q_OBJECT
public:
    explicit FileServer(QObject *parent=0);

public slots:

//    void ondisconnect();
//    void received(QString,QString);
signals:
    void filereceived(QString/*,QString*/);

private:
    int clientNum;
    void incomingConnection(int socketDesc);

};

#endif // FILESERVER_H
