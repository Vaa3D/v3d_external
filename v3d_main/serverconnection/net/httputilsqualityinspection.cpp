#include "httputilsqualityinspection.h"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>

#include "serverconnection/infocache.h"

HttpUtilsQualityInspection::HttpUtilsQualityInspection(QObject *parent)
{
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

//void HttpUtilsQualityInspection::bordercontrol(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size)
//{
//    mbrainId=brainId;
//    mres=res;
//    QString border=res.mid(4,res.size()-5);
//    QStringList xyzborder=border.split("x");
//    int xborder=xyzborder[0].toInt();
//    int yborder=xyzborder[1].toInt();
//    int zborder=xyzborder[2].toInt();

//    if(offsetX<=size/2)
//        moffsetX=size/2+1;
//    else if(offsetX>=xborder-size/2)
//        moffsetX=xborder-size/2-1;
//    else
//        moffsetX=offsetX;

//    if(offsetY<=size/2)
//        moffsetY=size/2+1;
//    else if(offsetY>=yborder-size/2)
//        moffsetY=yborder-size/2-1;
//    else
//        moffsetY=offsetY;

//    if(offsetZ<=size/2)
//        moffsetZ=size/2+1;
//    else if(offsetZ>=zborder-size/2)
//        moffsetZ=zborder-size/2-1;
//    else
//        moffsetZ=offsetZ;
//}

//void HttpUtilsQualityInspection::SWCReply(QNetworkReply *reply)
//{
//    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//    if(status == 200) {
//        QByteArray response = reply->readAll();
//        std::string WritePath = QApplication::applicationDirPath().toStdString() + "/checkcache/";
//        //QString filePath = QString::fromStdString(WritePath)+ mbrainId+"_"+QString::number(moffsetX)+"_"+ QString::number(moffsetY)+"_"+ QString::number(moffsetZ)+".v3dpbd";
//        QString filePath =QString::fromStdString(WritePath)+"1.swc";
//        filePath.replace(QString("/"),QString("\\"));
//        QFile file(filePath);
//        if(!file.open(QFile::ReadWrite)){
//            qDebug()<<filePath<<"open failed!";
//            return;
//        }
//        file.write(response);
//        file.close();
//        qDebug() << "done download!";
//    }else {
//        qDebug()<< "ERROR: download failed!";

//    }
//    reply->deleteLater();
//    reply = nullptr;
//}

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
//    qDebug()<<status;
    if(status == 200) {
        QString response = reply->readAll();
//        qDebug()<<response;
    }else {
        qDebug()<< "ERROR: download failed!";

    }
    reply->deleteLater();
    reply = nullptr;
}
