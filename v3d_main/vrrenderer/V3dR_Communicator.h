#ifndef V3DR_COMMUNICATOR_H
#define V3DR_COMMUNICATOR_H

#include <QObject>
#include <QtGui>
//#include <QtCore/QCoreApplication>
#include <QTcpSocket>
#include <QMessageBox>
//#include"../3drenderer/v3dr_common.h"
//#include <QRegExpValidator>
//#ifdef _WIN32
//    #include <windows.h>
//#endif
#include "../neuron_editing/v_neuronswc.h"
#include "../basic_c_fun/v3d_interface.h"
#include "fileserver.h"
#include <deque>
class V3dR_Communicator : public QObject
{
    Q_OBJECT
    struct DataType{
        bool isFile=false;//false msg,true file
        qint64 datasize=0;
        qint64 filesize=0;

    };


public:
    explicit V3dR_Communicator(QObject *partent=nullptr);
    ~V3dR_Communicator()=default;
//    void onReadySend(QString send_MSG,bool flag=1);//use sendMSG;
    /**
     * @brief sendMsg
     * @param msg
     * 发送消息
     */
    void sendMsg(QString msg);
    /**
     * @brief UpdateSendPoolNTList
     * @param seg
     * 发送加线segment到服务器
     */
    void UpdateAddSegMsg(V_NeuronSWC seg, vector<V_NeuronSWC> connectedSegs, QString clienttype, bool isBegin);
    void UpdateAddSegMsg(QString TVaddSegMSG);
    void UpdateAddManySegsMsg(vector<V_NeuronSWC> segs, QString clienttype);
    /**
     * @brief UpdateDeleteMsg
     * @param seg
     * 发送减线segment到服务器
     */
    void UpdateDelSegMsg(V_NeuronSWC seg,QString clienttype,vector<V_NeuronSWC> connectedSegs, bool isBegin);
    void UpdateDelManySegsMsg(vector<V_NeuronSWC> segs,QString clienttype);
    void UpdateDelSegMsg(QString TVdelSegMSG);//this node is second node of seg,because this is esay to delete correct seg
    /**
     * @brief UpdateSendPoolNode
     * @param x
     * @param y
     * @param z
     * @param type
     * 发送加点的
     */
    void UpdateAddMarkerMsg(float x,float y,float z,RGBA8 color,QString clienttype);
    void UpdateAddMarkerMsg(QString TVaddMarkerMSG);
    /**
     * @brief UpdateSendDelMarkerInfo
     * @param x
     * @param y
     * @param z
     * 发送减点
     */
    void UpdateDelMarkersMsg(vector<CellAPO> markers,QString clienttype);
    void UpdateDelMarkerMsg(QString TVdelMarkerMSG);
    /**
     * @brief UpdateSendDelMarkerInfo
     * @param x
     * @param y
     * @param z
     * @param color
     * 发送改marker颜色
     */
    void UpdateRetypeMarkerMsg(float x, float y, float z, RGBA8 color, QString clienttype);
    /**
     * @brief Updateretype
     * @param seg
     * @param type
     * 发送改seg颜色
     */
    void UpdateRetypeSegMsg(V_NeuronSWC seg,int type,QString clienttype);
    void UpdateRetypeManySegsMsg(vector<V_NeuronSWC> segs,int type,QString clienttype);
    void UpdateRetypeSegMsg(QString TVretypeSegMSG);

    void UpdateSplitSegMsg(V_NeuronSWC seg,V3DLONG nodeinseg_id,QString clienttype);
    void UpdateSplitSegMsg(QString deleteMsg,QString addMsg1,QString addMsg2);

    void UpdateUndoDeque();
    void UpdateRedoDeque();
    /**
     * @brief V_NeuronSWCToSendMSG
     * @param seg 全局坐标
     * @return 将局部坐标的seg按格式转换为要发送的string
     */

    void UpdateConnectSegMsg(XYZ p1, XYZ p2, V_NeuronSWC seg1, V_NeuronSWC seg2, QString clienttype);

    QStringList V_NeuronSWCToSendMSG(V_NeuronSWC seg);
    //Coordinate transform
    XYZ ConvertGlobaltoLocalBlockCroods(double x,double y,double z);
    XYZ ConvertLocalBlocktoGlobalCroods(double x,double y,double z);
    XYZ ConvertMaxRes2CurrResCoords(double x,double y,double z);
    XYZ ConvertCurrRes2MaxResCoords(double x,double y,double z);
public slots:
    /**
     * @brief TFProcess
     * @param msg
     * 消息处理函数
     */
    void TFProcess(QString msg);
    /**
     * @brief processWarnMsg
     * @param msg
     * 警告处理函数
     */
    void processWarnMsg(QString msg);
    /**
     * @brief processAnalyzeMsg
     * @param msg
     * 分析处理函数
     */
    void processAnalyzeMsg(QString msg);
    /**
     * @brief processSendMsg
     * @param msg
     * 分析处理函数
     */
    void processSendMsg(QString msg);
    /**
     * @brief onReadyRead
     * 读取输入，并执行相关处理
     */
    void onReadyRead();
    /**
     * @brief onConnected
     * 发送用户登陆消息
     */
    void onConnected();
    /**
     * @brief autoReconnect
     * 自动重连
     */
//    void autoReconnect();
//    /**
//     * @brief initConnect
//     * 初始连接
//     */
//    void initConnect();
    void autoExit();
    void resetWarnMulBifurcationFlag();
    void resetWarnLoopFlag();
//    /**
//     * @brief onDisconnected
//     * 服务器断开
//     */
//    void onDisconnected();

//    void read_autotrace(QString,XYZ*);
//    void undo();
signals:
    void load(QString);
    //msg process
    void msgtoprocess(QString);//转发消息给消息处理函数（TFProcess/TVProcess）
    void msgtowarn(QString);//转发消息给警告处理函数（processWarnMsg）
    void msgtoanalyze(QString);//转发消息给分析处理函数(processAnalyzeMsg)
    void msgtosend(QString);//转发消息给发送处理函数(processSendMsg)

    void addSeg(QString,int);//加线信号 （type x y z;type x y z;...）
    void addManySegs(QString);//加很多线信号
    void delSeg(QString,int);//减线信号 （type x y z;type x y z;...）
    void splitSeg(QString);//break seg信号
    void addMarker(QString,QString);//加marker信号 (type x y z)
    void addManyMarkers(QString,QString);//增加很多marker信号 (type x y z;type x y z;...)
    void delMarker(QString);//减marker信号 (type x y z)
    void retypeMarker(QString);//改marker颜色信号(r,g,b,x,y,z)
    void retypeSeg(QString,int,int);//改线的颜色信号（type x y z;type x y z;...）
    void connectSeg(QString);
    void updateuserview(QString);
    void setDefineSomaActionState(bool);
    //msg process end

    void exit();
public:
    void emitAddSeg(QString segInfo, int isBegin) {emit addSeg(segInfo, isBegin);}
    void emitAddManySegs(QString segsInfo) {emit addManySegs(segsInfo);}
    void emitAddManyMarkers(QString markerInfos, QString comment) {emit addManyMarkers(markerInfos, comment);}
    void emitDelSeg(QString segInfo, int isMany) {emit delSeg(segInfo,isMany);}
    void emitAddMarker(QString markerInfo, QString comment) {emit addMarker(markerInfo, comment);}
    void emitDelMarker(QString markerInfo) {emit delMarker(markerInfo);}
    void emitRetypeSeg(QString segInfo,int type, int isMany) {emit retypeSeg(segInfo,type,isMany);}
    void emitConnectSeg(QString segInfo){emit connectSeg(segInfo);}
    void setAddressIP(QString addressIp);
    void setPort(uint port);
    void resetdatatype();
private:
    /**
     * @brief processReaded
     * @param list
     * 处理接受的消息或文件
     * 1. 文件要求加载，开始协作
     * 2. 消息：解析操作，同步操作
     */
    void preprocessmsgs(const QStringList list);

    QString XYZ2String(XYZ node,int type=-1)
    {
        if(type!=-1)
            return QString("%1 %2 %3 %4").arg(type).arg(node.x).arg(node.y).arg(node.z);
        else
            return QString("%1 %2 %3").arg(node.x).arg(node.y).arg(node.z);
    }

public:
//	float VR_globalScale;//used to
    static QString userId;//
    static QString userName;
    static QString password;
    QString m_strAddressIP;
    uint m_iPort;
    static QTcpSocket* socket;//
    //连接状态
    bool b_isConnectedState;
    bool b_isWarnMulBifurcationHandled;
    bool b_isWarnLoopHandled;
//    //重连定时器
//    QTimer *m_timerConnect;
//    //初始化连接定时器
//    QTimer *timer_iniconn;
    QTimer *timer_exit;

    int initConnectCnt;
    int reconnectCnt;

    double cur_createmode;
    int cur_chno;

    XYZ ImageMaxRes;//
    XYZ ImageCurRes;
    XYZ ImageStartPoint;

    XYZ CreatorMarkerPos;
    int CreatorMarkerRes;


//    XYZ AutoTraceNode;
//    int flag_x,flag_y,flag_z;

//    QStringList undoStack;
//    QStringList undo_delcure;
    std::deque<QString> undoDeque;
    std::deque<QString> redoDeque;
private:
    DataType datatype;
    const int dequeszie=15;


//    int receiveCNT=0;
};




#endif // V3DR_COMMUNICATOR_H
