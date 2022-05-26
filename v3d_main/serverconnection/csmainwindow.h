#ifndef CSMAINWINDOW_H
#define CSMAINWINDOW_H

#include <QMainWindow>
#include "basic_4dimage.h"
#include "net/httputilsgetlocation.h"
#include "net/httputilsbrainlist.h"
#include "net/httputilsdownload.h"
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

    void getPotentialLoaction();
    void getBrainList();
    void downloadImage();

//// public member //////
public:
    HttpGetLocation *httpGetLocation = nullptr;
    HttpUtilsBrainList *httpGetBrainList = nullptr;
    HttpUtilsDownLoad *httpUtilsDownload = nullptr;
//    CoordinateConvert userLastCoordinateConvert;
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
    int DEFAULT_IMG_SIZE = 128;



};

#endif // CSMAINWINDOW_H
