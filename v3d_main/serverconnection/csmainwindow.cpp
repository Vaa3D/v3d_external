#include "csmainwindow.h"
#include "ui_csmainwindow.h"

#include <QJsonArray>
#include <QJsonObject>



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
//    ui->checkmapwidget->downloadImage(brainId, res, (int)loc->x, (int)loc->y, (int)loc->z, DEFAULT_IMG_SIZE);
//    ui->checkmapwidget->downloadImage("201589", "RES(15400x9474x5683)", 7587, 15888, 10274, 128);
//    QString brainId = uertLastPotentialSomaInfo->getBrainId();
//    XYZ* loc = userLastCoordinateConvert.getCenterLocation();
//    QString res = resMap.value("brainId");
//    ui->checkmapwidget->downloadImage(brainId, res, (int)loc->x, (int)loc->y, (int)loc->z, DEFAULT_IMG_SIZE);
    QString res = resMap[this->brainId];
    qDebug() << "input download para, [brainId]:" << this->brainId << "[res]:" << res << "[x, y, z]" << (int)xyzForLoc.x <<"," << (int)xyzForLoc.y<< "," << (int)xyzForLoc.z;
    ui->checkmapwidget->downloadImage(this->brainId, res, (int)xyzForLoc.x, (int)xyzForLoc.y, (int)xyzForLoc.z, DEFAULT_IMG_SIZE);
//    httpUtilsDownload->downLoadImage(brainId, res, (int)xyzForLoc.x, (int)xyzForLoc.y, (int)xyzForLoc.z, DEFAULT_IMG_SIZE);
}



void CSMainWindow::on_checkmapBtn_clicked()
{
    // get potential location
    // getBrainList();
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

