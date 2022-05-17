#ifndef HTTPGETLOCATION_H
#define HTTPGETLOCATION_H

#include "net/httputils.h"

class HttpGetLocation: public HttpUtils
{
    Q_OBJECT
public:
////// public functions ///////
    HttpGetLocation(QWidget *parent = nullptr);
    ~HttpGetLocation();

    void getPotentialLoaction(QJsonObject &userInfo);
    virtual void asyncPostRequest(QString url, QJsonObject &body);

signals:
    void sendXYZ(int x, int y, int z);

public slots:
    void locationReplyFinished(QNetworkReply* reply);

private:
    QString URL_GET_POTENTIAL_LOCATION = SERVER_IP + "/dynamic/soma/getpotentiallocation";
};

#endif // HTTPGETLOCATION_H
