#ifndef CSMAINWINDOW_H
#define CSMAINWINDOW_H

#include <QMainWindow>
#include "basic_4dimage.h"
#include "net/httputilsgetlocation.h"
#include "net/httputilsbrainlist.h"
#include "net/httputilsdownload.h"
#include "net/httputilsqualityinspection.h"
#include "serverconnection/model/coordinateconvert.h"
#include "serverconnection/model/potentialsomainfo.h"


namespace Ui {
class CSMainWindow;
}

class CSMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CSMainWindow(QWidget *parent = nullptr);
    ~CSMainWindow();

//    void handleDownloadSWCResult();

    void getPotentialLoaction();
    void getBrainList();
    void downloadImage();
    void getSwc();

//// public member //////
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
    void on_checkmapBtn_clicked();
    void on_getLocationBtn_clicked();

    void setLocXYZ(int id, QString image, int x, int y, int z);
    void setPotentialLocation(QString imageID, QString RES);

    void on_getBrainListBtn_clicked();

private:
    Ui::CSMainWindow *ui;
    int DEFAULT_IMAGE_SIZE = 128;
    int DEFAULT_RES_INDEX = 2;
    int resIndex = 2;

};

#endif // CSMAINWINDOW_H
