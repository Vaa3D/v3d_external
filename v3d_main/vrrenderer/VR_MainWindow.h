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
#include "V3dR_Communicator.h"
#include "../basic_c_fun/v3d_interface.h"
struct VRoutInfo
{
    std::vector<QString> deletedcurvespos;
    std::vector<QString> deletemarkerspos;
    std::vector<QString> retypeMsgs;
};
class CMainApplication;
class My4DImage;
class MainWindow;
class VR_MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit VR_MainWindow(V3dR_Communicator *TeraflyCommunicator);
    ~VR_MainWindow();
	int StartVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain,bool isLinkSuccess,QString ImageVolumeInfo,int &CreatorRes,V3dR_Communicator*TeraflyCommunicator, XYZ* zoomPOS = 0,XYZ *CreatorPos = 0,XYZ  MaxResolution = 0);
    void RunVRMainloop(XYZ* zoomPOS = 0);
    void GetResindexandStartPointfromVRInfo(QString VRinfo,XYZ CollaborationMaxResolution);
    QString ConvertToMaxGlobal(QString coords);
    XYZ ConvertMaxGlobal2LocalBlock(float x,float y,float z);
    XYZ ConvertBlock2GloabelInRES(XYZ local);
    void SendVRconfigInfo();
public slots:
    void TVProcess(QString);
//    /**
//     * @brief onReadySendSeg
//     * 从队列中发出画线命令
//     */
//    void onReadySendSeg();
signals:
	void VRSocketDisconnect();
//	void sendPoolHead();
public:
    CMainApplication *pMainApplication;
    XYZ VRVolumeStartPoint;
    XYZ VRVolumeEndPoint;
    XYZ VRVolumeCurrentRes;
    XYZ VRvolumeMaxRes;
    int ResIndex;
//    VRoutInfo VROutinfo;
private:
	V3dR_Communicator* VR_Communicator;
    QString userName;
    bool CURRENT_DATA_IS_SENT;
    vector<QString> CollaborationSendPool;
//    QTcpSocket* socket;
//	QString vr_Port;

private:



};

// bool startStandaloneVRScene(QList<NeuronTree> *ntlist, My4DImage *img4d, MainWindow *pmain);
int startStandaloneVRScene(QList<NeuronTree> *ntlist, My4DImage *img4d, MainWindow *pmain, XYZ* zoomPOS = 0);

#endif // VR_MainWindow_H
