#ifndef HTTPUTILSUSER_H
#define HTTPUTILSUSER_H

#include "net/httputils.h"

class HttpUtilsUser: public HttpUtils
{
    Q_OBJECT
public:
    HttpUtilsUser(QObject *parent = nullptr);
    ~HttpUtilsUser();

    void loginWithHttp(QJsonObject &userInfo);
    virtual void asyncPostRequest(QString url, QJsonObject &body);

public slots:
   void loginReplyFinished(QNetworkReply* reply);

signals:
    void loginSuccess();
    void loginFailed();

private:
    const QString URL_LOGIN = SERVER_IP + "/dynamic/user/login";

};

#endif // HTTPUTILSUSER_H
