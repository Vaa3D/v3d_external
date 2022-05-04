#ifndef HTTPUTILSUSER_H
#define HTTPUTILSUSER_H

#include "serverconnection/net/httputils.h"

class HttpUtilsUser: public HttpUtils
{
    Q_OBJECT
public:
    HttpUtilsUser();

    void loginWithHttp(QJsonObject userInfo);

public slots:
//    virtual void replyFinished(QNetworkReply* reply);

private:
    const QString URL_LOGIN = SERVER_IP + "/dynamic/user/login";
};

#endif // HTTPUTILSUSER_H
