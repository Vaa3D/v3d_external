#ifndef CHECKWIDGET_H
#define CHECKWIDGET_H
#include <QLayout>
#include "xformwidget.h"
#include "mainwindow.h"
#include "3drenderer/v3dr_glwidget.h"
#include "3drenderer/v3dr_mainwindow.h"
#include "basic_4dimage.h"
#include "net/httputilsgetlocation.h"
#include "net/httputilsbrainlist.h"
#include "net/httputilsdownload.h"
#include "net/httputilsqualityinspection.h"
#include "serverconnection/model/coordinateconvert.h"
#include "serverconnection/model/potentialsomainfo.h"
#include "serverconnection/infocache.h"
#include "CheckGlWidget.h"
#include "CheckManager.h"
#include <QJsonArray>
#include <QJsonObject>


class CheckManager;
class CheckGlWidget;



class CheckWidget:public QWidget{

    Q_OBJECT

public:
    explicit CheckWidget(QWidget *parent);
    ~CheckWidget();

public:
    friend class CheckManager;
    HttpUtilsQualityInspection *getArbor;
    HttpGetLocation *httpGetLocation = nullptr;
    HttpUtilsBrainList *httpGetBrainList = nullptr;
    QVector<ArborInfo> imgs;
    HttpUtilsDownLoad *httpUtilsDownload = nullptr;
//    CoordinateConvert *coordinateConvert;
//    CoordinateConvert *lastDownloadCoordinateConvert;
//    PotentialSomaInfo *uertLastPotentialSomaInfo = nullptr;
    QHash<QString, QString> resMap; // <brainId, MAXRES>
    int arborid;
    QString brainId;
    XYZ xyzForLoc;


public slots:

    void setLocXYZ(int id, QString image, int x, int y, int z);
    void setPotentialLocation(QString imageID, QString RES);
    void setArborInfo(int arborid,QString name,QString somaId,QString image,double x,double y,double z);

    void csztest();
    void downloadImage();
    void getPotentialLoaction();
    void getBrainList();
    void getarbor();
    void openimage();

    void timetoopen();

signals:
    void Getbrainlist();
    void Getlocation();
    void getPotential();


private:
    void drawlayout();
    void clearcache();

    QGroupBox *checkwidgetgp;
    QGroupBox *controlwidgetgp;
    QPushButton *checkmap;
    QPushButton *Getarbor;
    QPushButton *updatebtn;
    QHBoxLayout *cklayout;
    QTimer *isopenimg;
    bool isopen;
    QList<CheckGlWidget *> cglwidget;
    MainWindow *mparent;
    QString cachepath;
    int DEFAULT_IMAGE_SIZE = 128;
    int DEFAULT_RES_INDEX = 2;
    int resIndex = 2;
};

#endif // CHECKWIDGET_H
