#include "csmainwindow.h"
#include "ui_csmainwindow.h"

#include <QJsonArray>
#include <QJsonObject>
#include "serverconnection/infocache.h"


CSMainWindow::CSMainWindow(QWidget *parent)
    :QMainWindow(parent),
    ui(new Ui::CSMainWindow)
{
    ui->setupUi(this);

    // for downloading, its need to get [id],[barinId],[location],[RES]
    // that to say firstly do get potential location info and get brain list with http
    httpGetLocation = new HttpGetLocation(this);
    httpGetBrainList = new HttpUtilsBrainList(this);
    httpUtilsDownload = new HttpUtilsDownLoad(this);
//    httpUtilsQualityInspection = new HttpUtilsQualityInspection(this);

    coordinateConvert = new CoordinateConvert();
    lastDownloadCoordinateConvert = new CoordinateConvert();
    coordinateConvert->setResIndex(DEFAULT_RES_INDEX);
    coordinateConvert->setImgSize(DEFAULT_IMAGE_SIZE);
    lastDownloadCoordinateConvert->setResIndex(DEFAULT_RES_INDEX);
    lastDownloadCoordinateConvert->setImgSize(DEFAULT_IMAGE_SIZE);

    // connect signals and slots
    connect(httpGetLocation, SIGNAL(sendXYZ(int, QString, int, int, int)), this, SLOT(setLocXYZ(int, QString, int, int, int)));
    connect(httpGetBrainList, SIGNAL(sendPotentialLocation(QString, QString)), this, SLOT(setPotentialLocation(QString, QString)));
    // after http reply, httputilsxxx emit signal with needed infomation
    // in slot function, set the class member userLastCoordinateConvert and *uertLastPotentialSomaInfo



}

CSMainWindow::~CSMainWindow()
{
    delete ui;
    delete httpGetLocation;
    delete httpGetBrainList;
//    delete uertLastPotentialSomaInfo;
    delete httpUtilsDownload;
//    delete httpUtilsQualityInspection;
}

void CSMainWindow::getPotentialLoaction()
{
    QJsonObject userInfo;
    userInfo.insert("name", InfoCache::getInstance().getAccount());
    userInfo.insert("passwd", InfoCache::getInstance().getToken());
    httpGetLocation->getPotentialLoaction(userInfo);
}

void CSMainWindow::getBrainList()
{
    QJsonObject userInfo;
    userInfo.insert("name", InfoCache::getInstance().getAccount());
    userInfo.insert("passwd", InfoCache::getInstance().getToken());
    httpGetBrainList->getBrainList(userInfo);
}

void CSMainWindow::downloadImage()
{

    QString res = resMap[this->brainId];
    qDebug() << "input download para, [brainId]:" << this->brainId << "[res]:" << res << "[x, y, z]" << (int)xyzForLoc.x <<"," << (int)xyzForLoc.y<< "," << (int)xyzForLoc.z;
    httpUtilsDownload->downLoadImage(this->brainId, res, (int)(xyzForLoc.x / pow(2, resIndex-1)), (int)(xyzForLoc.y / pow(2, resIndex-1)), (int)(xyzForLoc.z / pow(2, resIndex-1)), DEFAULT_IMAGE_SIZE);

}

void CSMainWindow::getSwc()
{
//    QString arborName = curPotentialArborMarkerInfo.getArborName();
//    XYZ *loc = curPotentialArborMarkerInfo.getLocation();
//    QString res = "/" + curPotentialArborMarkerInfo.getBrainId() + "/" + curPotentialArborMarkerInfo.getSomaId();

//    httpUtilsQualityInspection->getSWCWithHttp(res, (float)loc->x, (float)loc->y, (float)loc->z, DEFAULT_IMAGE_SIZE * (int)pow(2, lastDownloadCoordinateConvert->getResIndex()-1), arborName);

}



void CSMainWindow::on_checkmapBtn_clicked()
{
    // get potential location
//    getPotentialLoaction();
    // get brainlist
//    getBrainList();
    // do download
    downloadImage();
}


void CSMainWindow::on_getLocationBtn_clicked()
{
    getPotentialLoaction();
}

/**
 * @brief SLOT connect to SIGNAL(sendXYZ))
 * @param x
 * @param y
 * @param z
 */
void CSMainWindow::setLocXYZ(int id, QString image, int x, int y, int z)
{
    // after test, xyz has been sent out successfully!
    XYZ loc((float)x, (float)y, (float)z);
    xyzForLoc = loc;
//    uertLastPotentialSomaInfo = new PotentialSomaInfo(id, image, &xyzForLoc);
    this->userid = id;
    this->brainId = image;
}

/**
 * @brief according brainid and max res to set resMap
 * @param imageID
 * @param RES
 */
void CSMainWindow::setPotentialLocation(QString imageID, QString RES)
{
    // id -> brainId, RES -> RES
//    uertLastPotentialSomaInfo = new PotentialSomaInfo(11, imageID, &xyzForLoc);
//    qDebug() << RES;
    resMap.insert(imageID, RES);
//    userLastCoordinateConvert.initLocation(uertLastPotentialSomaInfo->getLoaction());
}


void CSMainWindow::on_getBrainListBtn_clicked()
{
    getBrainList();
}

