#ifndef VR_MainWindow_H
#define VR_MainWindow_H
//#include "GL/glew.h"
//#include "GL/gl.h"
//#include "GL/glext.h"
//#include "GL/glut.h"
#include <QWidget>
#include <QtGui>
//#include <QtCore/QCoreApplication>
#include <QTcpSocket>
//#include "GL/glew.h"
#include <QtOpenGL/QGLWidget>
//#include<QGLFunctions>
//#include"glfw3.h"

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
#include"tiffio.h"
class V3dR_Communicator;
class Shader;
using glm::mat4;
using glm::vec3;
#define GL_ERROR() checkForOpenGLError(__FILE__, __LINE__)
#define STACK_PIXEL_8(img,x,y,z,c) \
      ((uint8  *) ((img)->array +   \
           ((((z)*(img)->height + (y))*(img)->width + (x))*(img)->kind + (c))))

#define STACK_PIXEL_16(img,x,y,z,c) \
      ((uint16 *) ((img)->array +   \
           ((((z)*(img)->height + (y))*(img)->width + (x))*(img)->kind + (c))))
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


#define GREY   1   // 1-byte grey-level image or stack
#define GREY16 2   // 2-byte grey-level image or stack
#define COLOR  3   // 3-byte RGB image or stack
typedef struct
{
	int      kind;
	int      width;
	int      height;
	unsigned char* array;   // Array of pixel values lexicographically ordered on (y,x,c).
} Image;
typedef struct {
	int kind;
	int width;
	int height;
	int depth;
	unsigned char* array;
}myStack;
typedef struct _myStack
{
	struct _myStack* next;
	int             vsize;
	myStack           stack;
} _myStack;

class VR_Window : public QGLWidget
//class VR_Window : public QGLWidget
{
	Q_OBJECT

public:
	VR_Window(QWidget *parent = 0);
	~VR_Window();
	
protected:
	//test Liqi little demo
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL(); 
private:

	QTimer timer;

	//initialize
	//void linkShader(GLuint shaderPgm, GLuint newVertHandle, GLuint newFragHandle);
	void initShader();
	GLuint initOccupancyTex();
	void initVBO();
	//GLuint createShaderPgm();
	GLuint initTFF1DTex(const char* filename);
	GLuint initFace2DTex(GLuint bfTexWidth, GLuint bfTexHeight);
	GLuint initVol3DTex(const char* filename);
	void checkFramebufferStatus();
	void initFrameBuffer( GLuint texWidth, GLuint texHeight);
	void rcSetUinforms(GLuint g_programid);
	//DISPLAY
	//void linkShader(GLuint shaderPgm, GLuint newVertHandle, GLuint newFragHandle);
	void render(GLenum cullFace);
	void drawBox(GLenum glFaces);
	void render(GLenum cullFace, GLuint g_programid);
	//执行计算着色器
	void runcomputeshader_occu();
	void runcomputeshader_dis();
	//return error
	int checkForOpenGLError(const char* file, int line);
	GLint checkShaderLinkStatus(GLuint pgmHandle);
	GLboolean compileCheck(GLuint shader);
	GLuint CompileGLShader(const char *pchShaderName, const char *p_shader, GLenum shaderType);

	
public:





	//acc部分
	Shader * backfaceShader;
	Shader * raycastingShader;

	GLuint g_vao;
	GLuint g_backprogramHandle;
	GLuint g_rayprogramHandle;
	GLuint g_programOCC;
	GLuint g_programDIS;
	GLuint g_winWidth = 800;
	GLuint g_winHeight = 800;
	GLint g_angle = 0;
	GLuint g_frameBuffer;

	GLuint g_tffTexObj;	// transfer function
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
	static int render_compute;


	//读取tiff
	int width = 0;
	int height = 0;
	int depth = 0;
	static _myStack* Free_Stack_List;
	static int  Stack_Offset, Stack_Inuse;
	TIFF* Open_Tiff(const char* file_name, const char* mode);
	void Free_Stack(myStack* stack);
	int determine_kind(TIFF* tif);
	static void* Guarded_Malloc(int size, const char* routine);
	static void* Guarded_Realloc(void* p, int size, const char* routine);
	static inline myStack* new_stack(int vsize, const char* routine)
	{
		_myStack* object;

		if (Free_Stack_List == NULL)
		{
			object = (_myStack*)Guarded_Malloc(sizeof(_myStack), routine);
			Stack_Offset = ((char*)&(object->stack)) - ((char*)object);
			object->vsize = vsize;
			object->stack.array = (uint8*)Guarded_Malloc(vsize, routine);
			Stack_Inuse += 1;
		}
		else
		{
			object = Free_Stack_List;
			Free_Stack_List = object->next;
			if (object->vsize < vsize)
			{
				object->vsize = vsize;
				object->stack.array = (uint8*)Guarded_Realloc(object->stack.array,
					vsize, routine);
			}
		}
		return (&(object->stack));
	}
	uint32* get_raster(int npixels, const char* routine);
	int error(const char* msg, const char* arg);
	inline int Get_Stack_Pixel(myStack* stack, int x, int y, int z, int c)
	{
		if (stack->kind == GREY16)
			return (*STACK_PIXEL_16(stack, x, y, z, c));
		else
			return (*STACK_PIXEL_8(stack, x, y, z, c));
	}
	void read_directory(TIFF* tif, Image* image, const char* routine);
	void Kill_Stack(myStack* stack);
	Image* Select_Plane(myStack* a_stack, int plane);
	myStack* Read_Stack(const char* file_name);
	bool loadTif2Stack(const char* filename, unsigned char*& img, long long* &sz, int &datatype);
};

#endif // VR_MainWindow_H
