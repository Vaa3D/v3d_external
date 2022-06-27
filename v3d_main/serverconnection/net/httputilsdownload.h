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
    void bordercontrol(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size);
signals:
    //pass

public slots:
    void downloadReplyFinished(QNetworkReply* reply);

private:
    QString URL_DOWNLOAD_IMAGE = SERVER_IP + "/dynamic/image/cropimage";
    QString mbrainId;
    QString mres;
    int moffsetX, moffsetY, moffsetZ;
};

#endif // HTTPUTILSDOWNLOAD_H
