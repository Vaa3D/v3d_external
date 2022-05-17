#include "httputilsbrainlist.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

HttpUtilsBrainList::HttpUtilsBrainList(QWidget *parent)
{
    manager = new QNetworkAccessManager();
}

HttpUtilsBrainList::~HttpUtilsBrainList()
{

}

void HttpUtilsBrainList::getBrainList(QJsonObject &userInfo)
{
    QJsonObject queryCondition;
    queryCondition.insert("off", 0);
    queryCondition.insert("limit", 2000);

    QJsonObject body;
    body.insert("user", userInfo);
    body.insert("condition", queryCondition);
    asyncPostRequest(URL_GET_BRAIN_LIST, body);
}

void HttpUtilsBrainList::asyncPostRequest(QString url, QJsonObject &body)
{
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;
    dataArray = document.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
    request.setUrl(QUrl(url));

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(brainListReplyFinished(QNetworkReply*)));
    manager->post(request, dataArray);
}

void HttpUtilsBrainList::brainListReplyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(status == 200) {
        QByteArray data = reply->readAll();
//        qDebug()<<data;
        // pharse data, need BrainId and RES()
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        qDebug() << doc;
        if(error.error == QJsonParseError::NoError) {
            qDebug() << "doc is array?" << doc.isArray() << "or doc is object?:" << doc.isObject();
//            emit sendPotentialLocation(imageID, RES);
        }
    }
    reply->deleteLater();
}
