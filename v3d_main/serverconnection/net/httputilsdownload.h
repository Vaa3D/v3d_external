#ifndef HTTPUTILSDOWNLOAD_H
#define HTTPUTILSDOWNLOAD_H

#include "net/httputils.h"

class HttpUtilsDownLoad: public HttpUtils
{
    Q_OBJECT
public:
    HttpUtilsDownLoad(QWidget *parent = nullptr);
    ~HttpUtilsDownLoad();

    void downLoadImage(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size);
    virtual void asyncPostRequest(QString url, QJsonObject &body);

public:
    ////// member //////
    QString brainId;
    QString res;
    int offsetX, offsetY, offsetZ;


signals:
    //pass

public slots:
    void downloadReplyFinished(QNetworkReply* reply);

private:
    QString URL_DOWNLOAD_IMAGE = SERVER_IP + "/dynamic/image/cropimage";
};

#endif // HTTPUTILSDOWNLOAD_H
