#ifndef FILESERVER_H
#define FILESERVER_H

#include <QtNetwork>
class FileServer:public QTcpServer
{
    Q_OBJECT
public:
    explicit FileServer(QObject *parent=0);
public slots:
    void Socketdisconnect(QString);
private:
    void incomingConnection(int socketDesc);
signals:
    void receivedfile(QString filename);
private:
    quint64 clientNum;
};

class FileSocket_receive:public QTcpSocket
{
    Q_OBJECT
public:
    explicit FileSocket_receive(int socketDesc,QObject *parent=0);
public slots:
//    void socketstart();
    void readFile();
signals:
    void receivefile(QString filename);
private:
    int socketId;
    quint64 totalsize;
    quint64 filenamesize;
    quint64 m_bytesreceived;

};

#endif // FILESERVER_H
