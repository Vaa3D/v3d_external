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
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadReplyFinished(QNetworkReply*)));
}

HttpUtilsDownLoad::~HttpUtilsDownLoad()
{

}

void HttpUtilsDownLoad::downLoadImage(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size)
{
    bordercontrol(brainId,res,offsetX,offsetY,offsetZ,size);
    QJsonObject pa1;
    pa1.insert("x", moffsetX - size/2);
    pa1.insert("y", moffsetY - size/2);
    pa1.insert("z", moffsetZ - size/2);

    QJsonObject pa2;
    pa2.insert("x", moffsetX + size/2);
    pa2.insert("y", moffsetY + size/2);
    pa2.insert("z", moffsetZ + size/2);

    QJsonObject bBox;
    bBox.insert("pa1", pa1);
    bBox.insert("pa2", pa2);
    bBox.insert("res", mres);
    bBox.insert("obj", mbrainId);

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


    QNetworkReply* reply = manager->post(request, dataArray);
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);

}

void HttpUtilsDownLoad::bordercontrol(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size)
{
    mbrainId=brainId;
    mres=res;
    QString border=res.mid(4,res.size()-5);
    QStringList xyzborder=border.split("x");
    int xborder=xyzborder[0].toInt();
    int yborder=xyzborder[1].toInt();
    int zborder=xyzborder[2].toInt();

    if(offsetX<=size/2)
        moffsetX=size/2+1;
    else if(offsetX>=xborder-size/2)
        moffsetX=xborder-size/2-1;
    else
        moffsetX=offsetX;

    if(offsetY<=size/2)
        moffsetY=size/2+1;
    else if(offsetY>=yborder-size/2)
        moffsetY=yborder-size/2-1;
    else
        moffsetY=offsetY;

    if(offsetZ<=size/2)
        moffsetZ=size/2+1;
    else if(offsetZ>=zborder-size/2)
        moffsetZ=zborder-size/2-1;
    else
        moffsetZ=offsetZ;
}

/**
 * @brief slot for address reply data, thats image binary data
 * @param reply
 */
#include <QApplication>
void HttpUtilsDownLoad::downloadReplyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray response = reply->readAll();
    if(status == 200) {
        std::string WritePath = QApplication::applicationDirPath().toStdString() + "/checkcache/";
        QString filePath = QString::fromStdString(WritePath)+ mbrainId+"_"+QString::number(moffsetX)+"_"+ QString::number(moffsetY)+"_"+ QString::number(moffsetZ)+".v3dpbd";
        filePath.replace(QString("/"),QString("\\"));
        //qDebug()<<response.size();
        QFile file(filePath);
        //qDebug()<<file.isOpen();
        if(!file.open(QFile::ReadWrite)){
            qDebug()<<filePath<<"open failed!";
            return;
        }
        file.write(response);
        file.close();
        qDebug() << "done download!";
    }
    else {
        qDebug()<< "ERROR: download failed!";

    }
    reply->deleteLater();
    reply = nullptr;
}


































