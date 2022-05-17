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

    // connect signals and slots
    connect(httpGetLocation, SIGNAL(sendXYZ(int, int, int)), this, SLOT(setLocXYZ(int, int, int)));
    connect(httpGetBrainList, SIGNAL(sendPotentialLocation(QString, QString)), this, SLOT(setPotentialLocation(QString, QString)));
    // after http reply, httputilsxxx emit signal with needed infomation
    // in slot function, set the class member userLastCoordinateConvert and *uertLastPotentialSomaInfo



}

CSMainWindow::~CSMainWindow()
{
    delete ui;
    delete httpGetLocation;
    delete httpGetBrainList;
    delete uertLastPotentialSomaInfo;
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
//    QString brainId = httpUtilSoma->getPotentialSomaInfo()->getBrainId();
//    XYZ* loc = httpUtilSoma->getCoordinateConvert()->getCenterLocation();
//    QString res = httpUtilSoma->resMap[brainId];
//    if(res == nullptr) {
//        qDebug()<<"Fail to download image, something wrong with res list!";
//        isDownloading = false;
//        return;
//    }
//    ui->checkmapwidget->downloadImage(brainId, res, (int)loc->x, (int)loc->y, (int)loc->z, DEFAULT_IMG_SIZE);;
    ui->checkmapwidget->downloadImage("192334", "RES(30801x18821x11515)", 14239, 28711, 10163, 128);
}



void CSMainWindow::on_checkmapBtn_clicked()
{
    // 首先通过httpUtilsSoma的getPotentialLocation设置好内部potentialSomaInfo类
    // 以此获得brainId
    // getPotentialLoaction();
    // 再获得分辨率信息
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
void CSMainWindow::setLocXYZ(int x, int y, int z)
{
    // after test, xyz has been sent out successfully!
    XYZ loc((float)x, (float)y, (float)z);
//    userLastCoordinateConvert.initLocation(&loc); // cause crash!
    xyzForLoc = loc;

}

void CSMainWindow::setPotentialLocation(QString imageID, QString RES)
{
    uertLastPotentialSomaInfo = new PotentialSomaInfo(11, "brainid", &xyzForLoc);
    qDebug()<<"imageID:" << imageID;
    qDebug()<<"RES: " << RES;
}


void CSMainWindow::on_getBrainListBtn_clicked()
{
    getBrainList();
}

