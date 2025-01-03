#include "httputilsdownload.h"

#include <QCoreApplication>
#include <QApplication>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include "serverconnection/infocache.h"
#include "serverconnection/net/networkutils.h"

HttpUtilsDownLoad::HttpUtilsDownLoad(QObject *parent)
{
    manager = new QNetworkAccessManager();
    managerimg= new QNetworkAccessManager();
    managerswc= new QNetworkAccessManager();
    isswcdone=false;
    isimgdone=false;
    m_timer=new QTimer;
    m_etimer=new QElapsedTimer;
    cntimg=0;cntswc=0;
    WritePath = QApplication::applicationDirPath().toStdString() + "/checkcache/";
    finalpath="TBD";
    connect(NetWorkUtil::instance(), &NetWorkUtil::finished, this, &HttpUtilsDownLoad::downloadReplyFinished);
    connect(managerimg, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadReplyFinished(QNetworkReply*)));
    connect(managerswc, SIGNAL(finished(QNetworkReply*)), this, SLOT(SWCReply(QNetworkReply*)));
    connect(m_timer,SIGNAL(timeout()),this,SLOT(istimeout()));
    m_timer->start(10000);
    m_etimer->start();
}

HttpUtilsDownLoad::~HttpUtilsDownLoad()
{

}

void HttpUtilsDownLoad::downLoadImage(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size)
{
    bordercontrol(brainId,res,offsetX,offsetY,offsetZ,size);
    if(InfoCache::getInstance().WidgetStatus[finalpath].isimgready){
        qDebug()<<"The img file already exists";
        isimgdone=true;
        return;
    }
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
    //qDebug() << "download image json: " << dataArray;

    // test result, try single instance NetWorkUtil to post http request XD
//    QNetworkReply* downloadImage = NetWorkUtil::instance()->postRequst(URL_DOWNLOAD_IMAGE, body);
    asyncPostRequest(URL_DOWNLOAD_IMAGE, body);
}

void HttpUtilsDownLoad::getSWCWithHttp(QString arborname,QString res, float offsetX, float offsetY, float offsetZ, int size)
{
//    bordercontrol(brainId,res,offsetX,offsetY,offsetZ,size);
//    qDebug()<<finalpath;
    if(InfoCache::getInstance().WidgetStatus[finalpath].isswcready){

        qDebug()<<"The swc file already exists";
        isswcdone=true;
        return;
    }
    marborname=arborname;
    somaID=res;
    moffsetXs=offsetX;
    moffsetYs=offsetY;
    moffsetZs=offsetZ;
    msizeswc=size;

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
    bBox.insert("obj", arborname);

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
    //qDebug() << "get SWC json: " << dataArray;

    asyncPostRequest(URL_GET_SWC, body);
}



void HttpUtilsDownLoad::asyncPostRequest(QString url, QJsonObject &body)
{
    QJsonDocument document;
    document.setObject(body);
    QByteArray dataArray;
    dataArray = document.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
    request.setUrl(url);

    if(url==URL_DOWNLOAD_IMAGE){
        QNetworkReply* reply = managerimg->post(request, dataArray);
        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    }else if(url==URL_GET_SWC){
        QNetworkReply* reply = managerswc->post(request, dataArray);
        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    }

}

void HttpUtilsDownLoad::bordercontrol(QString brainId, QString res, int offsetX, int offsetY, int offsetZ, int size)
{
    mbrainId=brainId;
    mres=res;

    moffsetX=offsetX;

    moffsetY=offsetY;

    moffsetZ=offsetZ;

    start_x=moffsetX-size/2;
    start_y=moffsetY-size/2;
    start_z=moffsetZ-size/2;
    msizeimg=size;

    if(finalpath=="TBD"){
        finalpath=QString::fromStdString(WritePath)+ mbrainId+"_"+QString::number(moffsetX)+"_"+ QString::number(moffsetY)+"_"+ QString::number(moffsetZ);
    }
}

void HttpUtilsDownLoad::Download(QString brainId, QString arborname,QString res, int offsetX, int offsetY, int offsetZ, int size)
{
    this->downLoadImage(brainId,res,offsetX,offsetY,offsetZ,size);
    this->getSWCWithHttp(arborname,res,offsetX,offsetY,offsetZ,size);
}

QByteArray HttpUtilsDownLoad::coordinateconvert(QString response)
{
    QByteArray result;
    QStringList responselines=response.split('\n');
    for(int i=0;i<responselines.size();i++){
        QString templines=responselines[i];
        //qDebug()<<templines;
        if(templines.size()<=1)
            continue;
        if(templines[0]=='#'){
            result.append((templines+'\n').toLatin1());
            continue;
        }else{
            QStringList templine=templines.split(' ');
            int type=4;
            float x=templine[2].toFloat()/2-start_x;
            float y=templine[3].toFloat()/2-start_y;
            float z=templine[4].toFloat()/2-start_z;
            QString tempresult;
            for(int j=0;j<templine.size();j++){
                if(j!=1&&j!=2&&j!=3&&j!=4&&j!=11)
                    tempresult.append(templine[j]+' ');
                else if(j==1){
                    tempresult.append(QString::number(type)+' ');
                }
                else if(j==2){
                    tempresult.append(QString::number(x)+' ');
                }
                else if(j==3){
                    tempresult.append(QString::number(y)+' ');
                }
                else if(j==4){
                    tempresult.append(QString::number(z)+' ');
                }
                else if(j==templine.size()-1){
                    tempresult.append(templine[j]);
                }
            }
            result.append((tempresult+'\n').toLatin1());

        }
    }
    //qDebug()<<result;
    return result;
}

QString HttpUtilsDownLoad::getfinalpath() const
{
    return finalpath;
}

/**
 * @brief slot for address reply data, thats image binary data
 * @param reply
 */

void HttpUtilsDownLoad::downloadReplyFinished(QNetworkReply *reply)
{
//    qDebug()<<QThread::currentThreadId();
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray response = reply->readAll();
    if(status == 200) {
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
        isimgdone=true;
        InfoCache::getInstance().WidgetStatus[finalpath].isimgready=true;
        qDebug() << "img done download!";
    }
    else {
        qDebug()<< "ERROR: download failed!";
        qDebug()<< "try again!";
        if(cntimg==5)
            return;
        downLoadImage(mbrainId,mres,moffsetX,moffsetY,moffsetZ,msizeimg);
        cntimg++;

    }
    reply->deleteLater();
    reply = nullptr;
}

void HttpUtilsDownLoad::SWCReply(QNetworkReply *reply)
{
//    qDebug()<<QThread::currentThreadId();
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(status == 200) {
        QString response = reply->readAll();
        QByteArray result=coordinateconvert(response);
        QString filePath = QString::fromStdString(WritePath)+ mbrainId+"_"+QString::number(moffsetX)+"_"+ QString::number(moffsetY)+"_"+ QString::number(moffsetZ)+".eswc";
        filePath.replace(QString("/"),QString("\\"));
        QFile file(filePath);
        if(!file.open(QFile::ReadWrite)){
            qDebug()<<filePath<<"open failed!";
            return;
        }
        file.write(result);
        file.close();
        isswcdone=true;
        //qDebug()<<finalpath;
        InfoCache::getInstance().WidgetStatus[finalpath].isswcready=true;
        qDebug() << "swc done download!";
    }else {
        qDebug()<< "ERROR: download failed!";
        qDebug()<< "try again!";
        if(cntswc==5)
            return;
        getSWCWithHttp(marborname,somaID,moffsetXs,moffsetYs,moffsetZs,msizeswc);
        cntswc++;

    }
    reply->deleteLater();
    reply = nullptr;
}

void HttpUtilsDownLoad::deletethis()
{
    if(!isimgdone){
        qDebug()<< mbrainId+"_"+QString::number(moffsetX)+"_"+ QString::number(moffsetY)+"_"+ QString::number(moffsetZ)<<"img download failed.";
        if(isswcdone){
            std::string WritePath = QApplication::applicationDirPath().toStdString() + "/checkcache/";
            QFile temp(QString::fromStdString(WritePath)+ mbrainId+"_"+QString::number(moffsetX)+"_"+ QString::number(moffsetY)+"_"+ QString::number(moffsetZ)+".eswc");
            temp.remove();
        }
    }
    if(!isswcdone){
        qDebug()<< mbrainId+"_"+QString::number(moffsetX)+"_"+ QString::number(moffsetY)+"_"+ QString::number(moffsetZ)<<"swc download failed.";
        if(isimgdone){
            std::string WritePath = QApplication::applicationDirPath().toStdString() + "/checkcache/";
            QFile temp(QString::fromStdString(WritePath)+ mbrainId+"_"+QString::number(moffsetX)+"_"+ QString::number(moffsetY)+"_"+ QString::number(moffsetZ)+".v3dpbd");
            temp.remove();
        }
    }
    delete managerimg;
    delete managerswc;
    delete this;
}

void HttpUtilsDownLoad::istimeout()
{
//    if(m_etimer->elapsed()>10000){
//        deletethis();
//    }
    if(isswcdone&&isimgdone)
        deletethis();
}


































