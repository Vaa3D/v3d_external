#include "fileserver.h"
#include <QMessageBox>

FileServer::FileServer(QObject *parent):QTcpServer (parent)
{
    qDebug()<<"receive_file.h";
    clientNum=0;
}

void FileServer::incomingConnection(int socketDesc)
{
    FileSocket_receive *filesocket=new FileSocket_receive(socketDesc);
//    connect(filesocket,SIGNAL(receivefile(QString)),filesocket,SIGNAL(dis);
//    connect(filesocket,SIGNAL(receivefile(QString)),this,SIGNAL(receivedfile(QString)));
    connect(filesocket,SIGNAL(disconnected()),this,SLOT(Socketdisconnect()));


    clientNum++;
//    QThread *thread=new QThread;
//    connect(thread,SIGNAL(started()),filesocket,SLOT(socketstart()));
//    connect(filesocket,SIGNAL(disconnected()),thread,SLOT(deleteLater()));
//    filesocket->moveToThread(thread);
//    thread->start();
}



void FileServer::Socketdisconnect()
{
    if(--clientNum==0)
    {
        qDebug()<<"delete";
        this->deleteLater();
    }
}

FileSocket_receive::FileSocket_receive(int socketDesc,QObject *parent)
    :socketId(socketDesc),QTcpSocket (parent)
{
    totalsize=0;
    filenamesize=0;
    m_bytesreceived=0;
    connect(this,SIGNAL(disconnected()),this,SLOT(deleteLater()));
    this->setSocketDescriptor(socketId);
    connect(this,SIGNAL(readyRead()),this,SLOT(readFile()));
}


//void FileSocket_receive::socketstart()
//{
//    this->setSocketDescriptor(socketId);
//    connect(this,SIGNAL(readyRead()),this,SLOT(readFile()));
//}

void FileSocket_receive::readFile()
{
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
            QDir rootDir("./");
            if(!rootDir.cd("clouddata"))
            {
                rootDir.mkdir("clouddata");
                rootDir.cd("clouddata");
            }

            QString filename;
            in>>filename;
            qDebug()<<filename;
            QByteArray block;
            in>>block;
            QFile file("I://annotation/"+filename);

            file.open(QIODevice::WriteOnly);
            file.write(block);
            file.close();
            m_bytesreceived=0;
            this->write(QString("received "+filename+"\n").toUtf8());

            qDebug()<<QString("received "+filename+"\n");
            qDebug()<<"hghjghjg";
            QRegExp apoRex("(.*).apo");
            if(apoRex.indexIn(filename)!=-1)
            {
//                    emit receivefile(filename);
                this->disconnectFromHost();
            }
        }
    }else {
            if(this->bytesAvailable()+m_bytesreceived>=totalsize)
            {
                QDir rootDir("./");
                if(!rootDir.cd("clouddata"))
                {
                    rootDir.mkdir("clouddata");
                    rootDir.cd("clouddata");
                }

                QString filename;
                in>>filename;
                qDebug()<<filename;
                QByteArray block;
                in>>block;
                QFile file("I://annotation/"+filename);
                file.open(QIODevice::WriteOnly);
                file.write(block);
                file.close();
                m_bytesreceived=0;
                this->write(QString("received "+filename+"\n").toUtf8());
                qDebug()<<QString("received "+filename+"\n");
                qDebug()<<"hghjghjg";
                QRegExp apoRex("(.*).apo");
                if(apoRex.indexIn(filename)!=-1)
                {
//                    emit receivefile(filename);
                    this->disconnectFromHost();
                }

            }
        }
}
