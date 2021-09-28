#ifndef MESSAGESERVER_H
#define MESSAGESERVER_H

#include <QtNetwork>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QThread>
#include <QReadWriteLock>
#include "basic_surf_objs.h"

class MessageSocket:public QTcpSocket
{
    Q_OBJECT
public:
    explicit MessageSocket(QString ip,QString port,QString username,QObject *parent=0);
protected:
    void SendToServer(const QString &msg);//已定义
private:
    quint64 nextblocksize;
    QString username;
public slots:
    void MessageSocketSlot_Read();//已绑定
signals:

};
#endif // MESSAGESERVER_H
