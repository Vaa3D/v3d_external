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
        qint64 filesize=0;
        QString filename;
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
    void UpdateAddSegMsg(V_NeuronSWC seg,QString clienttype);
    void UpdateAddSegMsg(QString TVaddSegMSG);
    /**
     * @brief UpdateDeleteMsg
     * @param seg
     * 发送减线segment到服务器
     */
    void UpdateDelSegMsg(V_NeuronSWC seg,QString clienttype);
    void UpdateDelSegMsg(QString TVdelSegMSG);//this node is second node of seg,because this is esay to delete correct seg
    /**
     * @brief UpdateSendPoolNode
     * @param x
     * @param y
     * @param z
     * @param type
     * 发送加点的
     */
    void UpdateAddMarkerMsg(float x,float y,float z,int type,QString clienttype);
    void UpdateAddMarkerMsg(QString TVaddMarkerMSG);
    /**
     * @brief UpdateSendDelMarkerInfo
     * @param x
     * @param y
     * @param z
     * 发送减点
     */
    void UpdateDelMarkerSeg(float x,float y,float z,QString clienttype);
    void UpdateDelMarkerSeg(QString TVdelMarkerMSG);
    /**
     * @brief Updateretype
     * @param seg
     * @param type
     * 发送改颜色
     */
    void UpdateRetypeSegMsg(V_NeuronSWC seg,int type,QString clienttype);
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
    QStringList V_NeuronSWCToSendMSG(V_NeuronSWC seg);
    //Coordinate transform
    XYZ ConvertGlobaltoLocalBlockCroods(double x,double y,double z);
    XYZ ConvertLocalBlocktoGlobalCroods(double x,double y,double z);
    XYZ ConvertMaxRes2CurrResCoords(double x,double y,double z);
    XYZ ConvertCurrRes2MaxResCoords(double x,double y,double z);
//    XYZ ConvertLocaltoGlobalCroods(double x,double y,double z,XYZ* para);
    //end Coordinate transform

//    void pushVSWCundoStack(vector<V_NeuronSWC> vector_VSWC);
//    void pushUndoStack(QString,QString);
    //    QString V_NeuronSWCToSendMSG(V_NeuronSWC seg,XYZ* para);转换读取的autotrace坐标到全局坐标
    //	void MsgToV_NeuronSWC(QString msg);
public slots:
    /**
     * @brief TFProcess
     * @param msg
     * @param flag_init
     * 消息处理函数
     */
    void TFProcess(QString msg,bool flag_init=0);
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

    void addSeg(QString);//加线信号 （type x y z;type x y z;...）
    void delSeg(QString);//减线信号 （type x y z;type x y z;...）
    void addMarker(QString);//加marker信号 (type x y z)
    void delMarker(QString);//减marker信号 (type x y z)
    void retypeSeg(QString,int);//改线的颜色信号（type x y z;type x y z;...）
    void updateuserview(QString);
    //msg process end
public:
    void emitAddSeg(QString segInfo) {emit addSeg(segInfo);}
    void emitDelSeg(QString segInfo) {emit delSeg(segInfo);}
    void emitAddMarker(QString markerInfo) {emit addMarker(markerInfo);}
    void emitDelMarker(QString markerInfo) {emit delMarker(markerInfo);}
    void emitRetypeSeg(QString segInfo,int type) {emit retypeSeg(segInfo,type);}
private:
    /**
     * @brief resetDataInfo
     * 重置接收的数据结构
     */
    void resetDataType();
    /**
     * @brief processReaded
     * @param list
     * 处理接受的消息或文件
     * 1. 文件要求加载，开始协作
     * 2. 消息：解析操作，同步操作
     */
    void processReaded(QStringList list);

    QString XYZ2String(XYZ node,int type=-1)
    {
        if(type!=-1)
            return QString("%1 %2 %3 %4").arg(type).arg(node.x).arg(node.y).arg(node.z);
        else
            return QString("%1 %2 %3").arg(node.x).arg(node.y).arg(node.z);
    }

public:
//	float VR_globalScale;//used to
    QString userName;//
    QTcpSocket* socket;//
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
    bool isLoad=true;

    const int dequeszie=10;


//    int receiveCNT=0;
};




#endif // V3DR_COMMUNICATOR_H
