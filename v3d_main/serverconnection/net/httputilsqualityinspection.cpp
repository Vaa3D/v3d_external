#include "httputilsqualityinspection.h"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>

#include "serverconnection/infocache.h"

HttpUtilsQualityInspection::HttpUtilsQualityInspection(QObject *parent)
{
    updatecnt=0;
    managerarbor = new QNetworkAccessManager();
    connect(managerarbor, SIGNAL(finished(QNetworkReply*)), this, SLOT(arborReply(QNetworkReply*)));
    managerupdate = new QNetworkAccessManager();
    connect(managerupdate, SIGNAL(finished(QNetworkReply*)), this, SLOT(updateReply(QNetworkReply*)));
}

HttpUtilsQualityInspection::~HttpUtilsQualityInspection()
{

}

//void HttpUtilsQualityInspection::getSWCWithHttp(QString res, float offsetX, float offsetY, float offsetZ, int size, QString arborName)
//{
//    bordercontrol(arborName,res,offsetX,offsetY,offsetZ,size);
//    QJsonObject pa1;
//    pa1.insert("x", moffsetX - size/2);
//    pa1.insert("y", moffsetY - size/2);
//    pa1.insert("z", moffsetZ - size/2);

//    QJsonObject pa2;
//    pa2.insert("x", moffsetX + size/2);
//    pa2.insert("y", moffsetY + size/2);
//    pa2.insert("z", moffsetZ + size/2);

//    QJsonObject bBox;
//    bBox.insert("pa1", pa1);
//    bBox.insert("pa2", pa2);
//    bBox.insert("res", mres);
//    bBox.insert("obj", mbrainId);

//    QJsonObject userInfo;
//    userInfo.insert("name", InfoCache::getInstance().getAccount());
//    userInfo.insert("passwd", InfoCache::getInstance().getToken());

//    // requestBody
//    QJsonObject body;
//    body.insert("user", userInfo);
//    body.insert("bb", bBox);

//    // post request
//    QJsonDocument document;
//    document.setObject(body);
//    QByteArray dataArray;

//    dataArray = document.toJson(QJsonDocument::Compact);
//    qDebug() << "get SWC json: " << dataArray;

//    asyncPostRequest(URL_GET_SWC, body);
//}

void HttpUtilsQualityInspection::getarborWithHttp(int Maxid)
{

    QJsonObject userInfo;
    userInfo.insert("name", InfoCache::getInstance().getAccount());
    userInfo.insert("passwd", InfoCache::getInstance().getToken());

    // requestBody
    QJsonObject body;
    body.insert("user", userInfo);
    body.insert("MaxId", Maxid);

    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;

    dataArray = document.toJson(QJsonDocument::Compact);
    qDebug() << "get arbor json: " << dataArray;

    asyncPostRequest(URL_GET_ARBOR, body);
}

void HttpUtilsQualityInspection::updateArborResult(int arborid, int result)
{
    QJsonObject userInfo;
    userInfo.insert("name", InfoCache::getInstance().getAccount());
    userInfo.insert("passwd", InfoCache::getInstance().getToken());

    QJsonObject arborResult;
    arborResult.insert("owner", InfoCache::getInstance().getAccount());
    arborResult.insert("arborid", arborid);
    arborResult.insert("result", result);

    QJsonArray insertList;
    insertList.push_back(arborResult);

    QJsonObject updateArborResultParam;
    updateArborResultParam.insert("insertlist",insertList);

    QJsonObject body;
    body.insert("user", userInfo);
    body.insert("pa", updateArborResultParam);

    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;

    dataArray = document.toJson(QJsonDocument::Compact);
    qDebug() << "update json: " << dataArray;

    asyncPostRequest(URL_UPDATE_ARBOR_RESULT, body);
}

void HttpUtilsQualityInspection::asyncPostRequest(QString url, QJsonObject &body)
{
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;
    dataArray = document.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
    request.setUrl(QUrl(url));

    if(url==URL_GET_ARBOR){
        QNetworkReply *reply = managerarbor->post(request, dataArray);
        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    }else if(url==URL_UPDATE_ARBOR_RESULT){
        QNetworkReply *reply = managerupdate->post(request, dataArray);
        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    }
}

int HttpUtilsQualityInspection::getupdatecnt() const
{
    return updatecnt;
}

void HttpUtilsQualityInspection::resetupdatecnt()
{
    updatecnt=0;
}


void HttpUtilsQualityInspection::arborReply(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<status;
    if(status == 200) {
        QString response = reply->readAll();
        QJsonDocument docu;
        docu=QJsonDocument::fromJson(response.toUtf8());
        QJsonArray arborInfoArray(docu.array());
        for(int i=0;i<arborInfoArray.size();i++){
            QJsonValue t_arborInfo=arborInfoArray[i];
            QJsonObject arborInfo=t_arborInfo.toObject();
            QJsonValue t_loc=arborInfo.value("loc");
            QJsonObject loc=t_loc.toObject();
            QJsonValue t_id=arborInfo.value("id");
            QJsonValue t_name=arborInfo.value("name");
            QJsonValue t_somaid=arborInfo.value("somaId");
            QJsonValue t_image=arborInfo.value("image");
            QJsonValue t_x=loc.value("x");
            QJsonValue t_y=loc.value("y");
            QJsonValue t_z=loc.value("z");
            emit sendarborinfo(t_id.toInt(),t_name.toString(),t_somaid.toString(),t_image.toString(),t_x.toDouble(),t_y.toDouble(),t_z.toDouble());
        }
        emit readytodownload();
    }else {
        qDebug()<< "ERROR: download failed!";

    }
    reply->deleteLater();
    reply = nullptr;
}

void HttpUtilsQualityInspection::updateReply(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(status == 200) {
        QString response = reply->readAll();
        updatecnt++;
    }else {
        qDebug()<< "ERROR: download failed!";

    }
    reply->deleteLater();
    reply = nullptr;
}
