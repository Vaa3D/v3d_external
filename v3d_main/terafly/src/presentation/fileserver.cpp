#include "fileserver.h"
#include <QMessageBox>

FileServer::FileServer(QObject *parent):QTcpServer (parent)
{
    clientNum=0;
//    connect(this,SIGNAL(newConnection()),this,SLOT(onNewConnect()));
}

void FileServer::incomingConnection(int socketDesc)
{
    qDebug()<<"file new connection";
    FileSocket *socket=new FileSocket;
    socket->setSocketDescriptor(socketDesc);
    clientNum++;
    connect(socket,SIGNAL(readyRead()),socket,SLOT(readFile()));
//    connect(socket,SIGNAL(disconnected()),socket,SLOT(deleteLater()));
    connect(socket,SIGNAL(disconnected()),this,SLOT(deleteLater()));
//    connect(socket,SIGNAL(disconnected()),this,SLOT(ondisconnect()));
//    connect(socket,SIGNAL(received(QString)),this,SLOT(received(QString)));
}

//void FileServer::received(QString msg,QString ip)
//{
//    emit filereceived(msg,ip);
//}

//void FileServer::ondisconnect()
//{
//    clientNum--;
//    if(clientNum==0)
//        this->deleteLater();
//}

FileSocket::FileSocket(QObject *parent):QTcpSocket (parent)
{
    totalsize=0;
    filenamesize=0;
    m_bytesreceived=0;
}

void FileSocket::readFile()
{
    qDebug()<<"in read file";
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_4_7);
    if(this->m_bytesreceived==0)
    {

        if(this->bytesAvailable()>=sizeof (quint64)*2)
        {
            in>>totalsize>>filenamesize;
            qDebug()<<totalsize <<"\t"<<filenamesize;
            m_bytesreceived+=sizeof (quint64)*2;
        }
        if(this->bytesAvailable()+m_bytesreceived>=totalsize)
        {
            QString filename;
            in>>filename;
            QByteArray block;
            in>>block;
            QFile file("C://annotationdata/"+filename);
            file.open(QIODevice::WriteOnly);
            file.write(block);
            file.close();
            m_bytesreceived=0;
//            emit received(QString("received "+filename+"\n"),this->peerAddress().toString());
            this->write(QString("received "+filename+"\n").toUtf8());
            qDebug()<<QString("received "+filename);
            QRegExp apoRex("(.*).ano.apo");
            if(apoRex.indexIn(filename)!=-1)
                QMessageBox::information(0, tr("information"),tr("download successfully."));
        }
    }else {
            if(this->bytesAvailable()+m_bytesreceived>=totalsize)
            {
                QString filename;
                in>>filename;
                QByteArray block;
                in>>block;
                QFile file("I://annotationdata/"+filename);
                file.open(QIODevice::WriteOnly);
                file.write(block);
                file.close();
                m_bytesreceived=0;
//                emit received(QString("received "+filename+"\n"),this->peerAddress().toString());
                this->write(QString("received "+filename+"\n").toUtf8());
                qDebug()<<QString("received "+filename);
                QRegExp apoRex("(.*).ano.apo");
                if(apoRex.indexIn(filename)!=-1)
                    QMessageBox::information(0, tr("information"),tr("download successfully."));
            }
        }
}


