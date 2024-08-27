#ifndef VR_MainWindow_H
#define VR_MainWindow_H

#include <QWidget>
#include <QTimer>
#include <QtGui>
//#include <QtCore/QCoreApplication>
#include <QTcpSocket>
//#include <QRegExpValidator>
//#ifdef _WIN32
//    #include <windows.h>
//#endif
#include "V3dR_Communicator.h"
#include "../basic_c_fun/v3d_interface.h"
class V3dR_Communicator;
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
    explicit VR_MainWindow(V3dR_Communicator* TeraflyCommunicator);
    ~VR_MainWindow();
	int StartVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain,bool isLinkSuccess,QString ImageVolumeInfo,int &CreatorRes,V3dR_Communicator*TeraflyCommunicator, XYZ* zoomPOS = 0,XYZ *CreatorPos = 0,XYZ  MaxResolution = 0);
    void RunVRMainloop(XYZ* zoomPOS = 0);
    void GetResindexandStartPointfromVRInfo(QString VRinfo,XYZ CollaborationMaxResolution);
    QString ConvertToMaxGlobalForMarker(QString coords);
    QString ConvertToMaxGlobalForSeg(QString coords);
    XYZ ConvertMaxGlobal2LocalBlock(float x,float y,float z);
    XYZ ConvertBlock2GloabelInRES(XYZ local);
    void SendVRconfigInfo();
public slots:
    void TVProcess(QString);
    void processWarnMsg(QString);
    void processAnalyzeMsg(QString line);
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
public slots:
    //    VRoutInfo VROutinfo;
    void performFileTransfer();
    void startFileTransferTask();
    void updateBCIstate(QString receivedString);
private slots:
    void onReplyFinished();
private:
	V3dR_Communicator* VR_Communicator;
    QString userId;
    bool CURRENT_DATA_IS_SENT;
    vector<QString> CollaborationSendPool;
    QStringList previousFiles; // 用于存储上一次检查时的文件列表
    int previousFileCount = 0; // 上一次检查时的文件数量
    QTimer *transferTimer;
    QString lastBCIstate;
    // 定义BCI参数的结构体
    struct BCIParameters {
        QString userId;
        QString BCI_paradigm;
        bool BCI_ssvep_mode;
        float BCI_parameter;

        // 设置函数，用于设置结构体成员变量的值
        void setParams(QString uid, const QString &paradigm, const bool &ssvep_mode, const float &parameter) {
            userId = uid;
            BCI_paradigm = paradigm;
            BCI_ssvep_mode = ssvep_mode;
            BCI_parameter = parameter;
        }
    };
    // 创建一个BCIParameters对象
    BCIParameters params;

//    QTcpSocket* socket;
//	QString vr_Port;

private:



    void sendDataToServer(const QByteArray &jsonData);
    void generateAndSendData();
};

// bool startStandaloneVRScene(QList<NeuronTree> *ntlist, My4DImage *img4d, MainWindow *pmain);
int startStandaloneVRScene(QList<NeuronTree> *ntlist, My4DImage *img4d, MainWindow *pmain, XYZ* zoomPOS = 0);

#endif // VR_MainWindow_H
