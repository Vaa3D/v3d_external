#include "CheckWidget.h"
#include <QPushButton>

CheckWidget::CheckWidget(QWidget *parent)
{


    std::string WritePath = QApplication::applicationDirPath().toStdString() + "/checkcache/";
    cachepath=QString::fromStdString(WritePath);

    QDir ckdir(cachepath);
    if(ckdir.exists()){
        ckdir.removeRecursively();
        ckdir.mkdir(cachepath);
    }else{
        ckdir.mkdir(cachepath);
    }

    QDir cktdir(cachepath+"trash/");
    if(!cktdir.exists())
        cktdir.mkdir(cachepath+"trash/");

    updatedone=false;
    mparent=(MainWindow *)parent;
    cglwidget.clear();
    for(int i=0;i<10;i++){
        cglwidget.append(new CheckGlWidget(this->mparent,i+1));
    }
    // for downloading, its need to get [id],[barinId],[location],[RES]
    // that to say firstly do get potential location info and get brain list with http
    httpGetLocation = new HttpGetLocation(this);
    httpGetBrainList = new HttpUtilsBrainList(this);
//    httpUtilsDownload = new HttpUtilsDownLoad(this);
//    httpUtilsQualityInspection = new HttpUtilsQualityInspection(this);
    getArbor=new HttpUtilsQualityInspection(this);
    isopenimg=new QTimer;
    isopen=false;
//    coordinateConvert = new CoordinateConvert();
//    lastDownloadCoordinateConvert = new CoordinateConvert();
//    coordinateConvert->setResIndex(DEFAULT_RES_INDEX);
//    coordinateConvert->setImgSize(DEFAULT_IMAGE_SIZE);
//    lastDownloadCoordinateConvert->setResIndex(DEFAULT_RES_INDEX);
//    lastDownloadCoordinateConvert->setImgSize(DEFAULT_IMAGE_SIZE);

    // connect signals and slots
    connect(httpGetLocation, SIGNAL(sendXYZ(int, QString, int, int, int)), this, SLOT(setLocXYZ(int, QString, int, int, int)));
    connect(httpGetBrainList, SIGNAL(sendPotentialLocation(QString, QString)), this, SLOT(setPotentialLocation(QString, QString)));
    connect(getArbor,SIGNAL(sendarborinfo(int,QString,QString,QString,double,double,double)),this,SLOT(setArborInfo(int,QString,QString,QString,double,double,double)));
    // after http reply, httputilsxxx emit signal with needed infomation
    // in slot function, set the class member userLastCoordinateConvert and *uertLastPotentialSomaInfo
    drawlayout();
//    connect(checkmap,SIGNAL(clicked()),this,SLOT(downloadImage()));
//    connect(getlocation,SIGNAL(clicked()),this,SLOT(getPotentialLoaction()));
    connect(Getarbor,SIGNAL(clicked()),this,SLOT(getarbor()));
    connect(updatebtn,SIGNAL(clicked()),this,SLOT(update()));
    connect(getArbor,SIGNAL(readytodownload()),this,SLOT(downloadImage()));
    connect(isopenimg,SIGNAL(timeout()),this,SLOT(timetoopen()));

//    connect(updatebtn,SIGNAL(clicked()),this,SLOT(openimage()));
//    connect(checkmap,SIGNAL(clicked()),this,SLOT(getPotentialLoaction()));
//    connect(this,SIGNAL(getPotential(QJsonObject)),CheckManager::instance()->httpGetLocation,SLOT(getPotentialLoaction(QJsonObject)));
//    connect(httpGetLocation,SIGNAL(getpotentiallocationdone()),this,SLOT(downloadImage()));
    //this->resize(1920,950);
    this->setWindowTitle("CheckWidget");
//    CheckManager::instance()->setmainwidget(this);
    InfoCache::getInstance().setPimgs(&imgs);
    isopenimg->start(200);
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
//    emit getPotential(userInfo);
//    emit getPotential();
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
    isopen=false;
//    qDebug()<<imgs.size();
    for(int i=0;i<imgs.size();i++){
//        QThread *nthread=new QThread;
        HttpUtilsDownLoad *temp=new HttpUtilsDownLoad(this);
//        temp->moveToThread(nthread);
//        nthread->start();
        QString res = resMap[imgs[i].image];
        qDebug() << "input download para, [brainId]:" << imgs[i].image << "[res]:" << res << "[x, y, z]" << imgs[i].x <<"," << imgs[i].y<< "," << imgs[0].z;
        temp->downLoadImage(imgs[i].image,res, (int)(imgs[i].x / pow(2, resIndex-1)), (int)(imgs[i].y / pow(2, resIndex-1)), (int)(imgs[i].z / pow(2, resIndex-1)), DEFAULT_IMAGE_SIZE);
        imgs[i].ab_path=temp->getfinalpath();
        ImgStatus nis;
        if(!InfoCache::getInstance().WidgetStatus.contains(imgs[i].ab_path))
            InfoCache::getInstance().WidgetStatus.insert(imgs[i].ab_path,nis);
        QString t_res = "/"+imgs[i].image+"/"+imgs[i].somaId;
        temp->getSWCWithHttp(imgs[i].name,t_res,imgs[i].x,imgs[i].y,imgs[i].z,DEFAULT_IMAGE_SIZE);
    }
}

void CheckWidget::getarbor()
{
    qDebug()<<isopen;
    if(isopen==true){
        qDebug()<<"imgs are opened!";
        return;
    }
    //QString res = resMap[this->brainId];
    //qDebug() << "input download para, [brainId]:" << this->brainId << "[res]:" << res << "[x, y, z]" << (int)xyzForLoc.x <<"," << (int)xyzForLoc.y<< "," << (int)xyzForLoc.z;
    //getArbor->getSWCWithHttp( res, (int)(xyzForLoc.x / pow(2, resIndex-1)), (int)(xyzForLoc.y / pow(2, resIndex-1)), (int)(xyzForLoc.z / pow(2, resIndex-1)), DEFAULT_IMAGE_SIZE,this->brainId);
    imgs.clear();
    InfoCache::getInstance().resetws();
    getArbor->getarborWithHttp();
    //httpUtilsDownload->downLoadImage(this->brainId, res, (int)(xyzForLoc.x / pow(2, resIndex-1)), (int)(xyzForLoc.y / pow(2, resIndex-1)), (int)(xyzForLoc.z / pow(2, resIndex-1)), DEFAULT_IMAGE_SIZE);
}

void CheckWidget::openimage()
{
    QDir dir(cachepath);
    QStringList nameFilters;
    nameFilters <<"*.v3dpbd";
    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
    for(int i=0;i<imgs.size();i++){
        if(!QFile::rename(cachepath+imgs[i].ab_path,cachepath+"trash/"+imgs[i].ab_path))
            QFile::remove(cachepath+imgs[i].ab_path);
        QString swcrawpath=(cachepath+imgs[i].ab_path).mid(0,(cachepath+imgs[i].ab_path).size()-7);
        QString swcfinalpath=(cachepath+"trash/"+imgs[i].ab_path).mid(0,(cachepath+"trash/"+imgs[i].ab_path).size()-7);
        if(!QFile::rename(swcrawpath+".eswc",swcfinalpath+".eswc"))
            QFile::remove(swcrawpath+".eswc");
        cglwidget[i]->imgname=imgs[i].ab_path;
        cglwidget[i]->openimage(cachepath+"trash/"+imgs[i].ab_path);
    }
    //    update();
}

void CheckWidget::openimage2()
{
    for(int i=0;i<imgs.size();i++){
        QStringList t=imgs[i].ab_path.split('/');
        cglwidget[i]->imgname=t[t.size()-1];
        cglwidget[i]->openimage(imgs[i].ab_path);
    }
}

void CheckWidget::timetoopen()
{
    if(updatedone&&updatecnt==getArbor->getupdatecnt()){
        getArbor->resetupdatecnt();
        updatedone=false;
        getarbor();
    }
//    qDebug()<<InfoCache::getInstance().getready();
    if(InfoCache::getInstance().getready()>=10){
        if(isopen==false){
            openimage2();
            isopen=true;
        }
    }
}

void CheckWidget::setLocXYZ(int id, QString image, int x, int y, int z)
{
    // after test, xyz has been sent out successfully!
    XYZ loc((float)x, (float)y, (float)z);
    xyzForLoc = loc;
//    uertLastPotentialSomaInfo = new PotentialSomaInfo(id, image, &xyzForLoc);
    this->arborid = id;
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

void CheckWidget::setArborInfo(int arborid, QString name, QString somaId, QString image, double x, double y, double z)
{
    ArborInfo temp;
    temp.arborid=arborid;
    temp.name=name;
    temp.somaId=somaId;
    temp.image=image;
    temp.x=x;
    temp.y=y;
    temp.z=z;
    temp.print();
    imgs.push_back(temp);
}

void CheckWidget::update()
{
//    qDebug()<<imgs.size();
    updatecnt=0;
    for(int i=0;i<imgs.size();i++){
        qDebug()<<cglwidget[i]->imgname<<cglwidget[i]->getstatus();
        if(cglwidget[i]->getstatus()!=0){
            getArbor->updateArborResult(imgs[i].arborid,cglwidget[i]->getstatus());
            InfoCache::getInstance().WidgetStatus[imgs[i].ab_path].ischecked=true;
            updatecnt++;
        }
        cglwidget[i]->clear();
    }
    updatedone=true;
    isopen=false;
    qDebug()<<"imgs are updated!"<<isopen;
    //clearcache();
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

//    checkmap=new QPushButton;
//    checkmap->setText("CheckMap");

    Getarbor=new QPushButton;
    Getarbor->setText("Getarbor");

    updatebtn=new QPushButton;
    updatebtn->setText("update");

    QVBoxLayout *controllayout=new QVBoxLayout;
//    controllayout->addWidget(checkmap);
    controllayout->addWidget(Getarbor);
    controllayout->addWidget(updatebtn);

    controlwidgetgp->setLayout(controllayout);

    cklayout->addWidget(checkwidgetgp,1);
    cklayout->addWidget(controlwidgetgp,0,Qt::AlignRight);


    this->setLayout(cklayout);
}

void CheckWidget::clearcache()
{
    QDir dir(cachepath);
    QStringList nameFilters;
    nameFilters <<"*.v3dpbd"<<"*.eswc";
    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
    for(int i=0;i<files.size();i++){
        QFile temp(cachepath+files[i]);
        temp.remove();
        qDebug()<<cachepath+files[i]<<"has been removed.";
    }
}
