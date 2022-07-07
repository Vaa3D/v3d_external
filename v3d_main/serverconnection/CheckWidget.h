#ifndef CHECKWIDGET_H
#define CHECKWIDGET_H
#include <QLayout>
#include "xformwidget.h"
#include "mainwindow.h"
#include "3drenderer/v3dr_glwidget.h"
#include "3drenderer/v3dr_mainwindow.h"
#include "basic_4dimage.h"
#include "CheckGlWidget.h"
#include "net/httputilsgetlocation.h"
#include "net/httputilsbrainlist.h"
#include "net/httputilsdownload.h"
#include "net/httputilsqualityinspection.h"
#include "serverconnection/model/coordinateconvert.h"
#include "serverconnection/model/potentialsomainfo.h"
#include "serverconnection/infocache.h"
#include <QJsonArray>
#include <QJsonObject>



class CheckGlWidget;
class CheckWidget:public QWidget{

    Q_OBJECT

public:
    explicit CheckWidget(QWidget *parent);
    ~CheckWidget();

public:
    HttpGetLocation *httpGetLocation = nullptr;
    HttpUtilsBrainList *httpGetBrainList = nullptr;
    HttpUtilsDownLoad *httpUtilsDownload = nullptr;
    CoordinateConvert *coordinateConvert;
    CoordinateConvert *lastDownloadCoordinateConvert;
//    PotentialSomaInfo *uertLastPotentialSomaInfo = nullptr;
    QHash<QString, QString> resMap; // <brainId, MAXRES>
    int userid;
    QString brainId;
    XYZ xyzForLoc;


public slots:

    void setLocXYZ(int id, QString image, int x, int y, int z);
    void setPotentialLocation(QString imageID, QString RES);
    void downloadImage();
    void getPotentialLoaction();
    void getBrainList();
    void getSwc();
    void openimage();



signals:
    void Getbrainlist();
    void Getlocation();

private:
    void drawlayout();


    QGroupBox *checkwidgetgp;
    QGroupBox *controlwidgetgp;
    QPushButton *checkmap;
    QPushButton *getlocation;
    QPushButton *getbrainlist;
    QHBoxLayout *cklayout;

    QList<CheckGlWidget *> cglwidget;
    MainWindow *mparent;

    int DEFAULT_IMAGE_SIZE = 128;
    int DEFAULT_RES_INDEX = 2;
    int resIndex = 2;
};

#endif // CHECKWIDGET_H
