#ifndef HTTPUTILSUSER_H
#define HTTPUTILSUSER_H

#include "net/httputils.h"

class HttpUtilsUser: public HttpUtils
{
    Q_OBJECT
public:
    HttpUtilsUser(QWidget *parent = nullptr);

    void loginWithHttp(QJsonObject userInfo);

public slots:
   virtual void replyFinished(QNetworkReply* reply);

signals:
    void loginSuccess();
    void loginFailed();

private:
    const QString URL_LOGIN = SERVER_IP + "/dynamic/user/login";
};

#endif // HTTPUTILSUSER_H
