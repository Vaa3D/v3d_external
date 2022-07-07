#include "CheckWidget.h"
#include <QPushButton>

CheckWidget::CheckWidget(QWidget *parent)
{
    mparent=(MainWindow *)parent;
    cglwidget.clear();
    for(int i=0;i<10;i++){
        cglwidget.append(new CheckGlWidget(this->mparent,i+1));
    }
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
    drawlayout();
//    connect(checkmap,SIGNAL(clicked()),this,SLOT(downloadImage()));
//    connect(getlocation,SIGNAL(clicked()),this,SLOT(getPotentialLoaction()));
    connect(getbrainlist,SIGNAL(clicked()),this,SLOT(openimage()));
    connect(checkmap,SIGNAL(clicked()),this,SLOT(getPotentialLoaction()));
    connect(httpGetLocation,SIGNAL(getpotentiallocationdone()),this,SLOT(downloadImage()));
    //this->resize(1920,950);
    this->setWindowTitle("CheckWidget");
}

CheckWidget::~CheckWidget()
{

}

void CheckWidget::getPotentialLoaction()
{
    QJsonObject userInfo;
    userInfo.insert("name", InfoCache::getInstance().getAccount());
    userInfo.insert("passwd", InfoCache::getInstance().getToken());
    httpGetLocation->getPotentialLoaction(userInfo);
}

void CheckWidget::getBrainList()
{
    QJsonObject userInfo;
    userInfo.insert("name", InfoCache::getInstance().getAccount());
    userInfo.insert("passwd", InfoCache::getInstance().getToken());
    httpGetBrainList->getBrainList(userInfo);

}

void CheckWidget::downloadImage()
{
    QString res = resMap[this->brainId];
    qDebug() << "input download para, [brainId]:" << this->brainId << "[res]:" << res << "[x, y, z]" << (int)xyzForLoc.x <<"," << (int)xyzForLoc.y<< "," << (int)xyzForLoc.z;
    httpUtilsDownload->downLoadImage(this->brainId, res, (int)(xyzForLoc.x / pow(2, resIndex-1)), (int)(xyzForLoc.y / pow(2, resIndex-1)), (int)(xyzForLoc.z / pow(2, resIndex-1)), DEFAULT_IMAGE_SIZE);
}

void CheckWidget::getSwc()
{

}

void CheckWidget::openimage()
{
    for(int i=0;i<cglwidget.size();i++){
        cglwidget[i]->openimage();
    }
    update();
}

void CheckWidget::setLocXYZ(int id, QString image, int x, int y, int z)
{
    // after test, xyz has been sent out successfully!
    XYZ loc((float)x, (float)y, (float)z);
    xyzForLoc = loc;
//    uertLastPotentialSomaInfo = new PotentialSomaInfo(id, image, &xyzForLoc);
    this->userid = id;
    this->brainId = image;
//    emit
}

void CheckWidget::setPotentialLocation(QString imageID, QString RES)
{
    // id -> brainId, RES -> RES
//    uertLastPotentialSomaInfo = new PotentialSomaInfo(11, imageID, &xyzForLoc);
//    qDebug() << RES;
    resMap.insert(imageID, RES);
//    userLastCoordinateConvert.initLocation(uertLastPotentialSomaInfo->getLoaction());
}

void CheckWidget::drawlayout()
{
    cklayout=new QHBoxLayout;

    checkwidgetgp=new QGroupBox;
    checkwidgetgp->setTitle("CheckWidget");
    QVBoxLayout *cwgplayout=new QVBoxLayout;
    QHBoxLayout *cwgptop=new QHBoxLayout;
    QHBoxLayout *cwgpbottom=new QHBoxLayout;
    if(cglwidget.size()>0){
        for(int i=0;i<cglwidget.size();i++){
            if(i<5){
                cwgptop->addWidget(cglwidget[i]);
            }else{
                cwgpbottom->addWidget(cglwidget[i]);
            }
        }
    }
    cwgplayout->addLayout(cwgptop);
    cwgplayout->addLayout(cwgpbottom);
    checkwidgetgp->setLayout(cwgplayout);


    controlwidgetgp=new QGroupBox;
    controlwidgetgp->setTitle("Controls");

    checkmap=new QPushButton;
    checkmap->setText("CheckMap");

    getlocation=new QPushButton;
    getlocation->setText("getLocation");

    getbrainlist=new QPushButton;
    getbrainlist->setText("getBrainList");

    QVBoxLayout *controllayout=new QVBoxLayout;
    controllayout->addWidget(checkmap);
    controllayout->addWidget(getlocation);
    controllayout->addWidget(getbrainlist);

    controlwidgetgp->setLayout(controllayout);

    cklayout->addWidget(checkwidgetgp,1);
    cklayout->addWidget(controlwidgetgp,0,Qt::AlignRight);


    this->setLayout(cklayout);
}
