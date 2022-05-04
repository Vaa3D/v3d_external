#include "httputils.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QMessageBox>

HttpUtils::HttpUtils()
{
    manager = new QNetworkAccessManager(this);
}

void HttpUtils::asyncPostRequest(QUrl url, QJsonObject body)
{
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;
    dataArray = document.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setUrl(url);

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    manager->post(request, dataArray);

}


void HttpUtils::replyFinished(QNetworkReply *reply)
{
//    if(reply->error()) {
//        qDebug()<<reply->errorString();
//        //QMessageBox::critical(this, "ERROR!", "Login failed");
//        reply->deleteLater();
//        return;
//    }
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    qDebug() << "statusCode: "<< statusCode;
    if(statusCode == 200) {
        emit loginSuccess();
    }

    reply->deleteLater();
}
