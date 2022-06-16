#include "httputilsqualityinspection.h"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>

#include "serverconnection/infocache.h"

HttpUtilsQualityInspection::HttpUtilsQualityInspection(QWidget *parent)
{
    manager = new QNetworkAccessManager();
}

HttpUtilsQualityInspection::~HttpUtilsQualityInspection()
{

}

void HttpUtilsQualityInspection::getSWCWithHttp(QString res, float offsetX, float offsetY, float offsetZ, int size, QString arborName)
{
    QJsonObject pa1;
    pa1.insert("x", offsetX - size/2);
    pa1.insert("y", offsetY - size/2);
    pa1.insert("z", offsetZ - size/2);

    QJsonObject pa2;
    pa2.insert("x", offsetX + size/2);
    pa2.insert("y", offsetY + size/2);
    pa2.insert("z", offsetZ + size/2);

    QJsonObject bBox;
    bBox.insert("pa1", pa1);
    bBox.insert("pa2", pa2);
    bBox.insert("res", res);
    bBox.insert("obj", arborName);

    QJsonObject userInfo;
    userInfo.insert("name", InfoCache::getInstance().getAccount());
    userInfo.insert("passwd", InfoCache::getInstance().getToken());

    // requestBody
    QJsonObject body;
    body.insert("user", userInfo);
    body.insert("bb", bBox);

    // post request
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;

    dataArray = document.toJson(QJsonDocument::Compact);
    qDebug() << "get SWC json: " << dataArray;

    asyncPostRequest(URL_GET_SWC, body);
}

void HttpUtilsQualityInspection::asyncPostRequest(QString url, QJsonObject &body)
{
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;
    dataArray = document.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
    request.setUrl(QUrl(url));

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(locationReplyFinished(QNetworkReply*)));
    QNetworkReply *reply = manager->post(request, dataArray);
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
}

void HttpUtilsQualityInspection::SWCReply(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(status == 200) {

    }
}
