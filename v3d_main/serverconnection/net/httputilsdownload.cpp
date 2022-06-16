#include "httputilsdownload.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include "serverconnection/infocache.h"
#include "serverconnection/net/networkutils.h"

HttpUtilsDownLoad::HttpUtilsDownLoad(QWidget *parent)
{
    manager = new QNetworkAccessManager();
    connect(NetWorkUtil::instance(), &NetWorkUtil::finished, this, &HttpUtilsDownLoad::downloadReplyFinished);
}

HttpUtilsDownLoad::~HttpUtilsDownLoad()
{

}

void HttpUtilsDownLoad::downLoadImage(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size)
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
    bBox.insert("obj", brainId);

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
    qDebug() << "download image json: " << dataArray;

    // test result, try single instance NetWorkUtil to post http request XD
//    QNetworkReply* downloadImage = NetWorkUtil::instance()->postRequst(URL_DOWNLOAD_IMAGE, body);
    asyncPostRequest(URL_DOWNLOAD_IMAGE, body);
}



void HttpUtilsDownLoad::asyncPostRequest(QString url, QJsonObject &body)
{
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;
    dataArray = document.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
    request.setUrl(url);

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadReplyFinished(QNetworkReply*)));
    QNetworkReply* reply = manager->post(request, dataArray);
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);

}

/**
 * @brief slot for address reply data, thats image binary data
 * @param reply
 */
void HttpUtilsDownLoad::downloadReplyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray response = reply->readAll();
    qDebug() << response;
    if(status == 200) {
        QByteArray response = reply->readAll();
    //    qDebug() << response;
        // save file
    //    QString storePath = QCoreApplication::applicationDirPath() + "/Image";
    //    // brainId
    //    QString fileName = brainId + "_" + res + "_" + offsetX + "_" + offsetY + "_" + offsetZ + ".v3dpbd";

        QString filePath = "c:\\Users\\SEU\\Desktop\\1.v3dpbd";
        QFile file(filePath);
        file.open(QFile::WriteOnly);
        file.write(response);
        file.close();

        // todo: render this image in CSMainwindow
    //    csglwidget->loadObjectFromFile(filePath);
        qDebug() << "done download!";
    }
    else {
        qDebug()<< "ERROR: download failed!";
    }

    reply->deleteLater();
    reply = nullptr;
}


































