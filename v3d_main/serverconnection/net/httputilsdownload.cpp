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
//    connect(NetWorkUtil::instance(), &NetWorkUtil::finished, this, &HttpUtilsDownLoad::downloadReplyFinished);
}

HttpUtilsDownLoad::~HttpUtilsDownLoad()
{

}

void HttpUtilsDownLoad::downLoadImage(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size)
{
    QJsonObject pa1;
    pa1.insert("x", offsetX/2 - size/2);
    pa1.insert("y", offsetY/2 - size/2);
    pa1.insert("z", offsetZ/2 - size/2);

    QJsonObject pa2;
    pa2.insert("x", offsetX/2 + size/2);
    pa2.insert("y", offsetY/2 + size/2);
    pa2.insert("z", offsetZ/2 + size/2);

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
    body.insert("bb", bBox);
    body.insert("user", userInfo);

    // post request
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;

    dataArray = document.toJson(QJsonDocument::Compact);
    qDebug() << "download image json: " << dataArray;

    // test result, try single instance NetWorkUtil to post http request XD
    QNetworkReply* downloadImage = NetWorkUtil::instance()->postRequst(URL_DOWNLOAD_IMAGE, body);
}



void HttpUtilsDownLoad::asyncPostRequest(QString url, QJsonObject &body)
{
    Q_UNUSED(url);
    Q_UNUSED(body);
}

/**
 * @brief slot for address reply data, thats image binary data
 * @param reply
 */
void HttpUtilsDownLoad::downloadReplyFinished(QNetworkReply *reply)
{
    qDebug() << "do download image22222";
    QByteArray data = reply->readAll();
    qDebug() << data;
    // save file setting
    QString storePath = QCoreApplication::applicationDirPath();
    QDir imageDir(storePath);
    imageDir.mkdir("Image");
    storePath = storePath + "/Image";
//    QString fileName = brainId + "_" + res + "_" + QString::number(offsetX) + "_" + QString::number(offsetY) + "_" + QString::number(offsetZ) + ".v3dpbd";
    QString fileName = "1.v3dpbd";
    qDebug() << "fileName:" << fileName << "whole file:" << storePath + "/" + fileName;
    QFile file(storePath + "/" + fileName);
    file.open(QFile::WriteOnly);
    file.write(data);
    file.close();

    qDebug() << "finish downloading!";
    reply->deleteLater();
    reply = nullptr;
}


































