#include "httputilsuser.h"

#include <QJsonObject>

HttpUtilsUser::HttpUtilsUser(QWidget *parent)
    : HttpUtils(parent)
{

}

void HttpUtilsUser::loginWithHttp(QJsonObject userInfo)
{
    QJsonObject body;
    body.insert("user", userInfo);
    asyncPostRequest(URL_LOGIN, body);
}

void HttpUtilsUser::replyFinished(QNetworkReply *reply)
{
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
//        qDebug() << "statusCode: "<< statusCode;

        emit loginSuccess();
    }
    else {
        emit loginFailed();
    }
}


