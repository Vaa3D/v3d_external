#include "fileserver.h"
#include <QMessageBox>

FileSocket_receive::FileSocket_receive(QString ip,QString port,QObject *parent)
    :ip(ip),port(port),QTcpSocket(parent)
{
    this->connectToHost(ip,port.toInt());
    if(!this->waitForConnected())
        QMessageBox::critical(0,"error","Load .ano failed ,please check connection and restart");
    connect(this,SIGNAL(readyRead()),this,SLOT(readFile()));
    connect(this,SIGNAL(disconnected()),this,SLOT(deleteLater()));
    filenamesize=0;m_bytesreceived=0;totalsize=0;
}
void FileSocket_receive::readFile()
{
	QDataStream in(this);
	in.setVersion(QDataStream::Qt_4_7);
    if(isDown==false)
    {
        if (this->m_bytesreceived == 0)
        {
            if (this->bytesAvailable() >= sizeof(quint64) * 2)
            {
                in >> totalsize >> filenamesize;
                qDebug() << totalsize << "\t" << filenamesize;
                m_bytesreceived += sizeof(quint64) * 2;
            }
            if (this->bytesAvailable() + m_bytesreceived >= totalsize)
            {
                QDir rootDir("./");
                if (!rootDir.cd("clouddata"))
                {
                    rootDir.mkdir("clouddata");
                    rootDir.cd("clouddata");
                }

                QString filename;
                in >> filename;
                qDebug() << filename;
                QByteArray block;
                in >> block;
                QFile file("./clouddata/" + filename);

                file.open(QIODevice::WriteOnly);
                file.write(block);
                file.close();
                m_bytesreceived = 0;
                this->write(QString("received " + filename + "\n").toUtf8());
                QRegExp apoRex("(.*).apo");
                if (apoRex.indexIn(filename) != -1)
                {

                    emit receivefile(apoRex.cap(1));
                    qDebug()<<"==============================================";
                    disconnectFromHost();
                }
            }
        }
        else {
            if (this->bytesAvailable() + m_bytesreceived >= totalsize)
            {
                QDir rootDir("./");
                if (!rootDir.cd("clouddata"))
                {
                    rootDir.mkdir("clouddata");
                    rootDir.cd("clouddata");
                }
                QString filename;
                in >> filename;
                QByteArray block;
                in >> block;
                QFile file("./clouddata/" + filename);
                file.open(QIODevice::WriteOnly);
                file.write(block);
                file.close();
                m_bytesreceived = 0;
                this->write(QString("received " + filename + "\n").toUtf8());
                QRegExp apoRex("(.*).apo");
                if (apoRex.indexIn(filename) != -1)
                {
                    emit receivefile(apoRex.cap(1));
                    disconnectFromHost();
                }
            }
        }
    }

    else
    {
        if (this->m_bytesreceived == 0)
        {
            if (this->bytesAvailable() >= sizeof(quint64) * 2)
            {
                in >> totalsize >> filenamesize;
                qDebug() << totalsize << "\t" << filenamesize;
                m_bytesreceived += sizeof(quint64) * 2;
            }
            if (this->bytesAvailable() + m_bytesreceived >= totalsize)
            {
                QDir rootDir("./");
                if (!rootDir.cd("download"))
                {
                    rootDir.mkdir("download");
                    rootDir.cd("download");
                }

                QString filename;
                in >> filename;
                qDebug() << filename;
                QByteArray block;
                in >> block;
                QFile file("./download/" + filename);

                file.open(QIODevice::WriteOnly);
                file.write(block);
                file.close();
                m_bytesreceived = 0;
                this->write(QString("received " + filename + "\n").toUtf8());
                QRegExp apoRex("(.*).apo");
                if (apoRex.indexIn(filename) != -1)
                {
                    emit receivefile(apoRex.cap(1));
                    disconnectFromHost();
                }
            }
        }
        else {
            if (this->bytesAvailable() + m_bytesreceived >= totalsize)
            {
                QDir rootDir("./");
                if (!rootDir.cd("download"))
                {
                    rootDir.mkdir("download");
                    rootDir.cd("download");
                }
                QString filename;
                in >> filename;
                qDebug() << filename;
                QByteArray block;
                in >> block;
                QFile file("./download/" + filename);
                file.open(QIODevice::WriteOnly);
                file.write(block);
                file.close();
                m_bytesreceived = 0;
                this->write(QString("received " + filename + "\n").toUtf8());
                QRegExp apoRex("(.*).apo");
                if (apoRex.indexIn(filename) != -1)
                {
                    emit receivefile(apoRex.cap(1));
                    disconnectFromHost();
                }
            }
        }
    }


}


//FileServer::FileServer(QObject *parent):QTcpServer (parent)
//{
//    qDebug()<<"receive_file.h";
//    clientNum=0;
//}

//void FileServer::incomingConnection(int socketDesc)
//{
//    FileSocket_receive *filesocket=new FileSocket_receive(socketDesc);


////    connect(filesocket,SIGNAL(receivefile(QString)),this,SIGNAL(receivedfile(QString)));
////    connect(filesocket,SIGNAL(disconnected()),this,SLOT(Socketdisconnect()));

////    connect(filesocket,SIGNAL(receivefile(QString)),this,SIGNAL(Socketdisconnect(QString)));
////    connect(filesocket,SIGNAL(disconnected()),this,SLOT(Socketdisconnect()));
//      connect(filesocket,SIGNAL(receivefile(QString)),this,SLOT(Socketdisconnect(QString)));
//      clientNum++;
//}

//void FileServer::Socketdisconnect(QString ANOfile)
//{
//    emit receivedfile(ANOfile);
//    this->deleteLater();
//}

//FileSocket_receive::FileSocket_receive(int socketDesc,QObject *parent)
//    :socketId(socketDesc),QTcpSocket (parent)
//{
//    totalsize=0;
//    filenamesize=0;
//    m_bytesreceived=0;
//    connect(this,SIGNAL(disconnected()),this,SLOT(deleteLater()));
//    this->setSocketDescriptor(socketId);
//    connect(this,SIGNAL(readyRead()),this,SLOT(readFile()));
//}


////void FileSocket_receive::socketstart()
////{
////    this->setSocketDescriptor(socketId);
////    connect(this,SIGNAL(readyRead()),this,SLOT(readFile()));
////}

//void FileSocket_receive::readFile()
//{
//    QDataStream in(this);
//    in.setVersion(QDataStream::Qt_4_7);
//    if(this->m_bytesreceived==0)
//    {
//        if(this->bytesAvailable()>=sizeof (quint64)*2)
//        {
//            in>>totalsize>>filenamesize;
//            qDebug()<<totalsize <<"\t"<<filenamesize;
//            m_bytesreceived+=sizeof (quint64)*2;
//        }
//        if(this->bytesAvailable()+m_bytesreceived>=totalsize)
//        {
//            QDir rootDir("./");
//            if(!rootDir.cd("clouddata"))
//            {
//                rootDir.mkdir("clouddata");
//                rootDir.cd("clouddata");
//            }

//            QString filename;
//            in>>filename;
//            qDebug()<<filename;
//            QByteArray block;
//            in>>block;
//            QFile file("./clouddata/"+filename);

//            file.open(QIODevice::WriteOnly);
//            file.write(block);
//            file.close();
//            m_bytesreceived=0;
//            QRegExp apoRex("(.*).apo");
//            if(apoRex.indexIn(filename)!=-1)
//            {
//                emit receivefile(apoRex.cap(1));

//                disconnectFromHost();
////                this->disconnectFromHost();
//            }
//            this->write(QString("received "+filename+"\n").toUtf8());


//        }
//    }else {
//            if(this->bytesAvailable()+m_bytesreceived>=totalsize)
//            {
//                QDir rootDir("./");
//                if(!rootDir.cd("clouddata"))
//                {
//                    rootDir.mkdir("clouddata");
//                    rootDir.cd("clouddata");
//                }

//                QString filename;
//                in>>filename;
//                qDebug()<<filename;
//                QByteArray block;
//                in>>block;
//                QFile file("./clouddata/"+filename);
//                file.open(QIODevice::WriteOnly);
//                file.write(block);
//                file.close();
//                m_bytesreceived=0;

//                QRegExp apoRex("(.*).apo");
//                if(apoRex.indexIn(filename)!=-1)
//                {
//                    emit receivefile(apoRex.cap(1));
//                    disconnectFromHost();
//                }
//                this->write(QString("received "+filename+"\n").toUtf8());
//            }
//        }
//}
