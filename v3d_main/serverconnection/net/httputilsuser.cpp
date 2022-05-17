#include "httputilsuser.h"

#include <QJsonDocument>
#include <QJsonObject>

HttpUtilsUser::HttpUtilsUser(QWidget *parent)
    : HttpUtils(parent)
{
    manager = new QNetworkAccessManager(this);
}

HttpUtilsUser::~HttpUtilsUser()
{

}

void HttpUtilsUser::loginWithHttp(QJsonObject &userInfo)
{
    QJsonObject body;
    body.insert("user", userInfo);
    asyncPostRequest(URL_LOGIN, body);
}

void HttpUtilsUser::asyncPostRequest(QString url, QJsonObject &body)
{
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;
    dataArray = document.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
    request.setUrl(QUrl(url));

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(loginReplyFinished(QNetworkReply*)));
    manager->post(request, dataArray);
}

void HttpUtilsUser::loginReplyFinished(QNetworkReply *reply)
{
    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
//        qDebug() << "statusCode: "<< statusCode;
        emit loginSuccess();
    }
    else {
        emit loginFailed();
    }
    reply->deleteLater();
}
