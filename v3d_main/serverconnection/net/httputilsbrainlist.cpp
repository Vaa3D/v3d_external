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
        qDebug()<<data;
        // pharse data, need BrainId and RES()
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);

        if(error.error == QJsonParseError::NoError) {
            // doc is array
            QJsonArray docArray = doc.array();
            for(int i = 0; i < docArray.count(); i++) {
                QJsonObject obj = docArray[i].toObject();
                // locate "detail:RES" and "name"
                // detail-> RES list, Type is String
                QString resList = obj.value("detail").toString();
                QStringList list = resList.split(",");
                // read first QString, thats the max RES
                QString res = list.at(1);
//                res = res.remove("[");
                res = res.remove("\ ");
                res = res.remove("\"");

                QString imageID = obj.value("name").toString();
                emit sendPotentialLocation(imageID, res);
            }

        }
    }
    reply->deleteLater();
}
