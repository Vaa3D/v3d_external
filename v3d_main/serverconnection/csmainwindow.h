#ifndef CSMAINWINDOW_H
#define CSMAINWINDOW_H

#include <QMainWindow>
#include "basic_4dimage.h"
#include "net/httputilsgetlocation.h"
#include "net/httputilsbrainlist.h"
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
    CoordinateConvert userLastCoordinateConvert;
    PotentialSomaInfo *uertLastPotentialSomaInfo = nullptr;

public slots:
    void on_checkmapBtn_clicked();
    void on_getLocationBtn_clicked();

    void setLocXYZ(int x, int y, int z);
    void setPotentialLocation(QString imageID, QString RES);

private slots:
    void on_getBrainListBtn_clicked();

private:
    Ui::CSMainWindow *ui;
    int DEFAULT_IMG_SIZE = 128;

    XYZ xyzForLoc;

};

#endif // CSMAINWINDOW_H
