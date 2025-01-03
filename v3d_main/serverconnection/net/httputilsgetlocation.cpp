#include "httputilsgetlocation.h"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>

HttpGetLocation::HttpGetLocation(QObject *parent)
{
    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(locationReplyFinished(QNetworkReply*)));
}

HttpGetLocation::~HttpGetLocation()
{

}

/**
 * @brief Construct get potential location post request Json body
 * @param userInfo
 */
void HttpGetLocation::getPotentialLoaction(QJsonObject &userInfo)
{
    QJsonObject body;
    body.insert("user", userInfo);
    asyncPostRequest(URL_GET_POTENTIAL_LOCATION, body);
}

/**
 * @brief HttpGetLocation::asyncPostRequest
 * @param url
 * @param body
 */
void HttpGetLocation::asyncPostRequest(QString url, QJsonObject &body)
{
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;
    dataArray = document.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
    request.setUrl(QUrl(url));


    QNetworkReply *reply = manager->post(request, dataArray);
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
}

/**
 * @brief slots
 */
void HttpGetLocation::locationReplyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(status == 200) {
        QByteArray data = reply->readAll().trimmed();
        qDebug() << data;
        // pharse Json, need "loc" to download
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        if(error.error == QJsonParseError::NoError) {
            QVariantMap map = doc.toVariant().toMap();
            // "id"
            int id = map["id"].toInt();
            // "image"
            QString image = map["image"].toString();
            // "loc"
            QVariantMap mapLoc = map["loc"].toMap();
            int x = mapLoc["x"].toInt();
            int y = mapLoc["y"].toInt();
            int z = mapLoc["z"].toInt();
            // let x,y,z out to CSMainWindow to construct CoordinateConvert
            emit sendXYZ(id, image, x, y, z);
            emit getpotentiallocationdone();

        }
    }else{
        qDebug()<<"Getlocation failed! Exit status "<<status;
    }
    reply->deleteLater();
}
