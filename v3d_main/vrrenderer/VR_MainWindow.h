#ifndef VR_MainWindow_H
#define VR_MainWindow_H

#include <QWidget>
#include <QtGui>
//#include <QtCore/QCoreApplication>
#include <QTcpSocket>
//#include <QRegExpValidator>
//#ifdef _WIN32
//    #include <windows.h>
//#endif

#include "../basic_c_fun/v3d_interface.h"
//#include "VRthread.h"

class CMainApplication;
struct VRoutInfo
{
	std::vector<XYZ> deletedcurvespos;
};
class CMainApplication;
class My4DImage;
class MainWindow;
class VR_MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit VR_MainWindow();
    ~VR_MainWindow();
	void onReadySend(QString &send_MSG);
	bool SendLoginRequest(bool resume = false);
	int StartVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain,bool isLinkSuccess,QString ImageVolumeInfo,int &CreatorRes,XYZ* zoomPOS = 0,XYZ *CreatorPos = 0,XYZ  MaxResolution = 0);
	XYZ VRVolumeStartPoint;
	XYZ VRVolumeEndPoint;
	XYZ VRVolumeCurrentRes;
	XYZ VRvolumeMaxRes;
	int ResIndex;
	VRoutInfo VROutinfo;
public slots:
	void RunVRMainloop(XYZ* zoomPOS = 0);
	void SendHMDPosition();
private slots:

    void onReadyRead();
    void onConnected();
    void onDisconnected();
public:
	CMainApplication *pMainApplication;
signals:
	void VRSocketDisconnect();
private:
	
    QTcpSocket* socket;
	QString userName;
	QString vr_Port;
	bool CURRENT_DATA_IS_SENT;
public:
	void GetResindexandStartPointfromVRInfo(QString VRinfo,XYZ CollaborationMaxResolution);
	QString ConvertsendCoords(QString coords);
	XYZ ConvertreceiveCoords(float x,float y,float z);
};

// bool startStandaloneVRScene(QList<NeuronTree> *ntlist, My4DImage *img4d, MainWindow *pmain);
int startStandaloneVRScene(QList<NeuronTree> *ntlist, My4DImage *img4d, MainWindow *pmain, XYZ* zoomPOS = 0);


#endif // VR_MainWindow_H
