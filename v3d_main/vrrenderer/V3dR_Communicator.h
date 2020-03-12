#ifndef V3DR_COMMUNICATOR_H
#define V3DR_COMMUNICATOR_H

#include <QWidget>
#include <QtGui>
//#include <QtCore/QCoreApplication>
#include <QTcpSocket>
//#include"../3drenderer/v3dr_common.h"
//#include <QRegExpValidator>
//#ifdef _WIN32
//    #include <windows.h>
//#endif
#include "../neuron_editing/v_neuronswc.h"
#include "../basic_c_fun/v3d_interface.h"
#include "fileserver.h"


struct Agent {
	QString name;
	bool isItSelf;
	int colorType;
	float position[16];

};
class FileSocket_send:public QTcpSocket
{
    Q_OBJECT
public:
    explicit FileSocket_send(QString ip,QString port,QString anofile_path,QObject *parent=0);

    void sendFile(QString filepath,QString filename);
public slots:
    void readMSG();
private:
    QString anopath;
    QString anoname;
};

//class CMainApplication;
class My4DImage;
class MainWindow;
class ManageSocket:public QTcpSocket
{
	Q_OBJECT
public:
	explicit ManageSocket(QObject *parent=0);
	QString ip;
	QString manageport;
	QString name;
//    QString loadfile_name;

public slots:
    void onReadyRead();
    void send1(QListWidgetItem*);
    void send2(QListWidgetItem*);
    void messageMade();
//    void receivedfile(QString anofile);
    void receivefile(QString anofile);
//    void TFProcess(QString msg);
protected:
signals:
    void makeMessageSocket(QString ip,QString port,QString username);
    void loadANO(QString);
//    void receivefile(QString anofile);
private:
    QString messageport;
    QString loadfilename;
    bool MSGsocket;
    bool FileRec;
};

class V3dR_Communicator : public QWidget
{
    Q_OBJECT

public:
    explicit V3dR_Communicator(bool *client_flag = 0, V_NeuronSWC_list* ntlist=0);
    ~V3dR_Communicator();
    void onReadySend(QString send_MSG,bool flag=1);
//	bool SendLoginRequest();
	//void StartVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain,bool isLinkSuccess);
	//void Update3DViewNTList(QString &msg, int type);
	void UpdateSendPoolNTList(V_NeuronSWC seg);
	void UpdateDeleteMsg(vector<XYZ> deleteLocNode);//this node is second node of seg,because this is esay to delete correct seg
    void UpdateSendPoolNode(float,float,float,int type=3);
    void UpdateSendDelMarkerInfo(float x,float y,float z);
    void Updateretype(V_NeuronSWC_unit row_unit,int type);
	void Collaborationsendmessage();
	void Collaborationaskmessage();
	//trans func
    QString V_NeuronSWCToSendMSG(V_NeuronSWC seg,bool f=1);
    QString V_NeuronSWCToSendMSG(V_NeuronSWC seg,XYZ* para);
	QString V_DeleteNodeToSendMSG(vector<XYZ> loc_list);
	void MsgToV_NeuronSWC(QString msg);


public:
	float VR_globalScale;//used to 
    QString userName;
	std::vector<Agent> Agents;
//	ManageSocket * managesocket;
	 QTcpSocket* socket;
	 double cur_createmode;
	 int cur_chno;
	 XYZ ImageMaxRes;
	 XYZ ImageCurRes;
	 XYZ ImageStartPoint;

	 XYZ CreatorMarkerPos;
	 int CreatorMarkerRes;
    QTimer *asktimer;
public slots:
    bool SendLoginRequest(QString ip,QString port,QString username);
	//void RunVRMainloop();
	//void SendHMDPosition();
	void CollaborationMainloop();
    void TFProcess(QString msg,bool flag_init=0);
    void read_autotrace(QString,XYZ*);
    void timerStart(QString,int);
    void undo();
//    void setautomarker(XYZ);
private slots:	
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void askserver();

signals:
    void messageMade();
	void CollaAddcurveSWC(vector<XYZ>, int chno, double createmode);
    void CollAddMarker(XYZ);

    void addSeg(QString,int);
    void delSeg(QString);
    void addMarker(QString,int);
    void delMarker(QString);
    void msgtoprocess(QString);
    void retypeSeg(QString);
private:

	QString vr_Port;
	bool CURRENT_DATA_IS_SENT;
	bool * clienton;
//	V_NeuronSWC_list * NTList_SendPool;
	vector<vector<XYZ>> loc_ReceivePool;
	int NTNumReceieved;
	int NTNumcurrentUser;
public:
    quint16 nextblocksize;
private:
	XYZ ConvertGlobaltoLocalCroods(double x,double y,double z);
	XYZ ConvertLocaltoGlobalCroods(double x,double y,double z);
    XYZ ConvertLocaltoGlobalCroods(double x,double y,double z,XYZ* para);
public:
    XYZ AutoTraceNode;
//    XYZ MoveToMarkerPos;
    int flag_x,flag_y,flag_z;

    QStringList undoStack;
    QStringList undo_delcure;
    void pushVSWCundoStack(vector<V_NeuronSWC> vector_VSWC);
    void pushUndoStack(QString,QString);
};




#endif // V3DR_COMMUNICATOR_H
