#ifndef VR_MainWindow_H
#define VR_MainWindow_H

#include <QWidget>
#include <QtGui>
//#include <QtCore/QCoreApplication>
#include <QTcpSocket>
//#include "GL/glew.h"
#include <QtOpenGL/QGLWidget>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform2.hpp"
#include "glm/gtc/type_ptr.hpp"
//#include <QRegExpValidator>
//#ifdef _WIN32
//    #include <windows.h>
//#endif
#include "V3dR_Communicator.h"
#include "../basic_c_fun/v3d_interface.h"
class V3dR_Communicator;
using glm::mat4;
using glm::vec3;
#define GL_ERROR() checkForOpenGLError(__FILE__, __LINE__)
struct VRoutInfo
{
	std::vector<XYZ> deletedcurvespos;
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
//	void onReadySend();
	bool SendLoginRequest(bool resume = false);
	int StartVRScene(QList<NeuronTree>* ntlist, My4DImage *i4d, MainWindow *pmain,bool isLinkSuccess,QString ImageVolumeInfo,int &CreatorRes,V3dR_Communicator*TeraflyCommunicator, XYZ* zoomPOS = 0,XYZ *CreatorPos = 0,XYZ  MaxResolution = 0);
	XYZ VRVolumeStartPoint;
	XYZ VRVolumeEndPoint;
	XYZ VRVolumeCurrentRes;
	XYZ VRvolumeMaxRes;
	int ResIndex;
	VRoutInfo VROutinfo;
public slots:
	void RunVRMainloop(XYZ* zoomPOS = 0);
	void SendHMDPosition();
    void TVProcess(QString);
    void onReadySend();
private slots:

//    void onReadyRead();
//    void onConnected();
//    void onDisconnected();
public:
	CMainApplication *pMainApplication;
signals:
	void VRSocketDisconnect();
	void sendPoolHead();
private:
	V3dR_Communicator* VR_Communicator;
//    QTcpSocket* socket;
	QString userName;
	QString vr_Port;
	bool CURRENT_DATA_IS_SENT;
public:
	void GetResindexandStartPointfromVRInfo(QString VRinfo,XYZ CollaborationMaxResolution);
	QString ConvertsendCoords(QString coords);
	XYZ ConvertreceiveCoords(float x,float y,float z);
private:
    int numreceivedmessage;//for debug HL
    int numsendmessage;
	//Prevent receiving/sending conflicts  LiQi
	vector<QString> CollaborationSendPool;
	void SendVRconfigInfo();
};

// bool startStandaloneVRScene(QList<NeuronTree> *ntlist, My4DImage *img4d, MainWindow *pmain);
int startStandaloneVRScene(QList<NeuronTree> *ntlist, My4DImage *img4d, MainWindow *pmain, XYZ* zoomPOS = 0);

class VR_Window : public QGLWidget
{
	Q_OBJECT

public:
	VR_Window(QWidget *parent = 0);
	~VR_Window();

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

	void runcomputeshader_occu();
	void runcomputeshader_dis();
	void initest();
	void linkShader(GLuint shaderPgm, GLuint newVertHandle, GLuint newFragHandle);
	GLuint initShaderObj(const char* srcfile, GLenum shaderType);
	void render(GLenum cullFace);
	void initShader();
	GLuint initOccupancyTex();
	void initVBO();
	void drawBox(GLenum glFaces);
	GLboolean compileCheck(GLuint shader);

	GLint checkShaderLinkStatus(GLuint pgmHandle);
	GLuint createShaderPgm();
	//init test ÊÇ·ñÐèÒª£¿
	//initTFF1DTex?
	GLuint initFace2DTex(GLuint bfTexWidth, GLuint bfTexHeight);
	//initVol3DTex?
	void checkFramebufferStatus();
	void initFrameBuffer(GLuint texObj, GLuint texWidth, GLuint texHeight);
	void rcSetUinforms();
	//DISPLAY?
public:

	GLuint g_vao;
	GLuint g_programHandle;
	GLuint g_programOCC;
	GLuint g_programDIS;
	GLuint g_winWidth = 800;
	GLuint g_winHeight = 800;
	GLint g_angle = 0;
	GLuint g_frameBuffer;
	// transfer function
	GLuint g_tffTexObj;
	GLuint g_bfTexObj;
	GLuint g_texWidth;
	GLuint g_texHeight;
	GLuint g_volTexObj;
	GLuint g_occupancymap;
	GLuint g_distancemap;
	GLuint g_rcVertHandle;
	GLuint g_rcFragHandle;
	GLuint g_bfVertHandle;
	GLuint g_bfFragHandle;
	GLuint g_compute_occ;
	GLuint g_compute_dis;
	GLuint shaderProgram;
	GLuint VAO;
	float g_stepSize = 0.001f;
};

#endif // VR_MainWindow_H
