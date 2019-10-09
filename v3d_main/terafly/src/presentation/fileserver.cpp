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
    connect(socket,SIGNAL(disconnected()),this,SLOT(deleteLater()));
    connect(socket,SIGNAL(received(QString)),this,SIGNAL(filereceived(QString)));
}


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
            qDebug()<<filename<<endl<<endl;
            QByteArray block;
            in>>block;
            QFile file("C://annotationdata/"+filename);
            file.open(QIODevice::WriteOnly);
            file.write(block);
            file.close();
            m_bytesreceived=0;

            QRegExp apoRex("(.*).ano.apo");
            if(apoRex.indexIn(filename)!=-1)
                QMessageBox::information(0, tr("information"),tr("download successfully."));
            this->write(QString("received "+filename+"\n").toUtf8());
            qDebug()<<QString("received "+filename)<<endl<<endl;

        }
    }else {
            if(this->bytesAvailable()+m_bytesreceived>=totalsize)
            {
                QString filename;
                in>>filename;
                QByteArray block;
                in>>block;
                qDebug()<<filename<<endl<<endl;
                QFile file("I://annotationdata/"+filename);
                file.open(QIODevice::WriteOnly);
                file.write(block);
                file.close();
                m_bytesreceived=0;
                QRegExp apoRex("(.*).ano.apo");
                if(apoRex.indexIn(filename)!=-1)
                {
                    emit received(apoRex.cap(1)+".ano");
                    QMessageBox::information(0, tr("information"),tr("download successfully."));
                }
                this->write(QString("received "+filename+"\n").toUtf8());
                qDebug()<<QString("received "+filename)<<endl<<endl;

            }
        }
}


