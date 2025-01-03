#ifndef CHECKMANAGER_H
#define CHECKMANAGER_H

#include "CheckWidget.h"

struct ArborInfo;

class HttpGetLocation;
class HttpUtilsBrainList;
class HttpUtilsDownLoad;
class CoordinateConvert;
class CoordinateConvert;

class CheckManager:public QObject{
    Q_OBJECT
private:

    CheckManager(){
        mainwidget=nullptr;
        path = QApplication::applicationDirPath() + "/checkcache/";
        cache_files.clear();
        cache_files=getcachefiles();
        file_count=cache_files.size();
        imgs.clear();
        imgs_managed.clear();
        init_imgs();

        httpGetLocation = new HttpGetLocation(this);
        httpGetBrainList = new HttpUtilsBrainList(this);
//        httpUtilsDownload = new HttpUtilsDownLoad(this);
        coordinateConvert = new CoordinateConvert();
        lastDownloadCoordinateConvert = new CoordinateConvert();
        coordinateConvert->setResIndex(DEFAULT_RES_INDEX);
        coordinateConvert->setImgSize(DEFAULT_IMAGE_SIZE);
        lastDownloadCoordinateConvert->setResIndex(DEFAULT_RES_INDEX);
        lastDownloadCoordinateConvert->setImgSize(DEFAULT_IMAGE_SIZE);

        connect(httpGetLocation, SIGNAL(sendXYZ(int, QString, int, int, int)), this, SLOT(setLocXYZ(int, QString, int, int, int)));
        connect(httpGetBrainList, SIGNAL(sendPotentialLocation(QString, QString)), this, SLOT(setPotentialLocation(QString, QString)));

//        connect(mainwidget,SIGNAL(clicked()),this,SLOT(getPotentialLoaction()));
        connect(httpGetLocation,SIGNAL(getpotentiallocationdone()),this,SLOT(downloadImage()));
//        this->getBrainList();
    }


    //members
    QString path;
    int file_count;
    QStringList cache_files;
    QVector<ArborInfo> imgs;
    QSet<QString> imgs_managed;


    //functions
    QStringList getcachefiles();
    void init_imgs();

    void addimgs();
    void deleteimgs();
    void update();

protected:
    static CheckManager* uniqueInstance;
    HttpGetLocation *httpGetLocation = nullptr;
    HttpUtilsBrainList *httpGetBrainList = nullptr;
    HttpUtilsDownLoad *httpUtilsDownload = nullptr;
    CoordinateConvert *coordinateConvert;
    CoordinateConvert *lastDownloadCoordinateConvert;
    QHash<QString, QString> resMap;
    int userid;
    QString brainId;
    XYZ xyzForLoc;
    int DEFAULT_IMAGE_SIZE = 128;
    int DEFAULT_RES_INDEX = 2;
    int resIndex = 2;
    CheckWidget *mainwidget;
public:
    friend class CheckWidget;

    static CheckManager* instance(){
        if(!uniqueInstance)
            uniqueInstance=new CheckManager();
        return uniqueInstance;
    }

    static void uninstance(){
        if(uniqueInstance){
            delete uniqueInstance;
            uniqueInstance=nullptr;
        }
    }

    ~CheckManager();

    //interface
    bool isExist(QString img_name);
    QStringList validimg(int n);

public slots:

    void setLocXYZ(int id, QString image, int x, int y, int z);
    void setPotentialLocation(QString imageID, QString RES);
    void downloadImage();
    void getPotentialLoaction();
    void getBrainList();
    void setmainwidget(CheckWidget* mw);
};

#endif // CHECKMANAGER_H
