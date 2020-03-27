#include "messageserverandmessagesocket.h"
#include <QDataStream>
#include <QByteArray>
MessageSocket::MessageSocket(QString ip,QString port,QString username,QObject *parent)
    :QTcpSocket (parent),username(username)
{
    nextblocksize=0;
    connect(this,SIGNAL(disconnected()),this,SLOT(deleteLater()));
    connect(this,SIGNAL(readyRead()),this,SLOT(MessageSocketSlot_Read()));
    this->connectToHost(ip,port.toInt());
    if(!this->waitForConnected())
    {
        QMessageBox::information(0, tr("Error"),tr("can not connect messageserver."));
    }

}

void MessageSocket::MessageSocketSlot_Read()
{
//    读取数据 begin
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_4_7);

    if (nextblocksize == 0) {
        if (bytesAvailable() < sizeof(quint64))
            return;
        in >> nextblocksize;
    }

    if (bytesAvailable() < nextblocksize)
        return;

    QString msg;
    in>>msg;
    nextblocksize=0;
//end 数据存放在msg中

//    数据处理
}

//发消息
void MessageSocket::SendToServer(const QString &msg)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);

    out<<quint64(0)<<msg;
    out.device()->seek(0);
    out<<quint64(sizeof (block)-sizeof (quint64))<<msg;

    this->write(block);
}


