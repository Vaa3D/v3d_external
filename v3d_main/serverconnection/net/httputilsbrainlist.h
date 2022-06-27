#ifndef HTTPUTILSBRAINLIST_H
#define HTTPUTILSBRAINLIST_H

#include "net/httputils.h"

class HttpUtilsBrainList: public HttpUtils
{
    Q_OBJECT
public:
    HttpUtilsBrainList(QWidget *parent = nullptr);
    ~HttpUtilsBrainList();

    void getBrainList(QJsonObject &userInfo);
    virtual void asyncPostRequest(QString url, QJsonObject &body);

signals:
    void sendPotentialLocation(QString imageID, QString RES);
    void getbrainlistdone();
public slots:
    void brainListReplyFinished(QNetworkReply* reply);

private:
    QString URL_GET_BRAIN_LIST = SERVER_IP + "/dynamic/image/getimagelist";
};

#endif // HTTPUTILSBRAINLIST_H
