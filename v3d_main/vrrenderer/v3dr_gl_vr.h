//last touch by Hanchuan Peng, 20170615.

#ifndef __V3DR_GL_VR_H__
#define __V3DR_GL_VR_H__
#include <QObject>
#include <SDL.h>
#include "../eegdevice/EEGdevice.h"
#include "../basic_c_fun/v3d_interface.h"


#include <openvr.h>
#include "lodepng.h"

#include "Matrices.h"//todo-yimin: this header is removable
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "V3dR_Communicator.h"
//#include <gltext.hpp>//include freetype and gltest library

#include "mainwindow.h"
#include <QObject>
#include <QTimer>
#include <QDebug>  // 包含 qDebug 以便调试输出

enum ModelControlR
{
	m_drawMode = 0,
	m_deleteMode,
    m_retypeMode,
	m_markMode,
    m_delmarkMode,
	m_splitMode,
	m_insertnodeMode,
	m_reducenodeMode,
	m_clipplaneMode,
	m_ConnectMode, 
    _MovetoCreator,
    m_ssvep
	//m_slabplaneMode
};
enum ModeControlSettings
{
	
	_donothing = 0,
	_TeraShift,
	_TeraZoom,
	_Contrast,
	_UndoRedo,
	_ColorChange,
	_Surface,
	_VirtualFinger,
	_Freeze,
	_LineWidth,
	_AutoRotate,
	_ResetImage,
	_RGBImage,
    _m_ssvep,
	_MovetoMarker,
	_StretchImage
};
enum ModeTouchPadR
{
	tr_contrast = 0,
    tr_clipplane = 1,
    tr_ssvephz
};
enum RGBImageChannel
{
	channel_rgb = 1,
	channel_r,
	channel_g,
	channel_b,

};
enum SecondeMenu
{
	_nothing = 0,
	_colorPad = 1,
//	_cutplane = 2
};
enum FlashType
{
	noflash = 1,
	line,
};
// int global_padm_modeGrip_L = _donothing;//liqi
// int global_padm_modeGrip_R = m_drawMode;
typedef QList<NeuronTree> NTL;

class Shader;
class Sphere;
class Cylinder;
class My4DImage;
class MainWindow;
class CGLRenderModel
{
public:
	CGLRenderModel( const std::string & sRenderModelName );
	~CGLRenderModel();

	bool BInit( const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture );
	void Cleanup();
	void Draw();
	const std::string & GetName() const { return m_sModelName; }

private:
	GLuint m_glVertBuffer;
	GLuint m_glIndexBuffer;
	GLuint m_glVertArray;
	GLuint m_glTexture;
	GLsizei m_unVertexCount;
	std::string m_sModelName;
};
template<class T>
class MinMaxOctree
{
public:
	MinMaxOctree(int width, int height, int depth,int step);
	~MinMaxOctree();
	void build(T *volumeData, int volumeWidth, int volumeHeight, int volumeDepth);
	int getWidth() { return width; }
	int getHeight() { return height; }
	int getDepth() { return depth; }
	T* GetData() { return data; }
private:
	T *data;
	int width;
	int height;
	int depth;
	int step;
};

class TransferControlPoint
{
public:
	TransferControlPoint(float r,float g,float b,int isovalue)
	{
		Color.x = r;
		Color.y = g;
		Color.z = b;
		Color.w = 1.0f;
		Isovalue = isovalue;
	}
	TransferControlPoint(float alpha,int isovalue)
	{
		Color.x = 0.0f;
		Color.y = 0.0f;
		Color.z = 0.0f;
		Color.w = alpha;
		Isovalue = isovalue;
	}
	glm::vec4 Color;
	int Isovalue;


};
//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class CMainApplication:public QObject
{
    Q_OBJECT
public:
    CMainApplication(int argc = 0, char *argv[] = 0,XYZ glomarkerPOS=0);
	virtual ~CMainApplication();

	bool BInit();
	bool BInitGL();
	bool BInitCompositor();

        void UpdateNTList(QVector<XYZ> coords, int type);//add the receieved message/NT to sketchedNTList
        QStringList NT2QString(NeuronTree); // prepare the message to be sent from currentNT.
        XYZ ConvertLocaltoGlobalCoords(float x,float y,float z,XYZ targetRes);
	XYZ ConvertGlobaltoLocalCoords(float x,float y,float z);
	//bool FlashStuff(FlashType type,XYZ coords);
	void ClearCurrentNT();//clear the currently drawn stroke, and all the flags
	bool HandleOneIteration();//used in collaboration mode 
	QString getHMDPOSstr();//get current HMD position, and prepare the message to be sent to server
	void SetupCurrentUserInformation(string name, int typeNumber);
	//void SetupAgentModels(vector<Agent> &curAgents);//generate spheres models to illustrate the locations of other users and get Collaboration creator Pos
	void RefineSketchCurve(int direction, NeuronTree &oldNT, NeuronTree &newNT);//use Virtual Finger to improve curve
	QString FindNearestSegment(glm::vec3 dPOS);
        QString FindNearestSegmentForDel(glm::vec3 dPOS);
        QString FindNearestMarker(glm::vec3 dPOS);
	XYZ GetSegtobedelete_Node(QString name);
	bool DeleteSegment(QString segName);

	void SetDeleteSegmentColor(QString segName);
        void SetDeleteMarkerColor(QString markerName);

        bool DeleteSegment(QVector<XYZ> coords,float dist);
        int findseg(QVector<XYZ> coords,float dist);
        bool retypeSegment(QVector<XYZ> coords,float dist,int type);
	NeuronSWC FindNearestNode(NeuronTree NT,glm::vec3 dPOS);
	void MergeNeuronTrees(NeuronTree &ntree, const QList<NeuronTree> * NTlist);//merge NTlist to single neurontree
	bool isAnyNodeOutBBox(NeuronSWC S_temp);
	void UpdateDragNodeinNTList(int ntnum,int swcnum,float nodex,float nodey,float nodez);

	void SetupRenderModels();

	void Shutdown();

	void RunMainLoop();
	bool HandleInput();//handle controller and keyboard input
	void ProcessVREvent( const vr::VREvent_t & event );
	void RenderFrame();
	
	bool SetupTexturemaps();//load controller textures and setup properties
	void AddVertex( float fl0, float fl1, float fl2, float fl3, float fl4, std::vector<float> &vertdata );
	void SetupControllerTexture();//update texture coordinates according to controller's new location
	void SetupControllerRay();
	void SetupImageAxes();
	void AddrayVertex(float fl0, float fl1, float fl2, float fl3, float fl4,float fl5, std::vector<float> &vertdata);
	void SetupMorphologyLine(int drawMode);
	void SetupMorphologyLine(NeuronTree neuron_Tree,GLuint& LineModeVAO, GLuint& LineModeVBO, GLuint& LineModeIndex,unsigned int& Vertcount,int drawMode);
	void SetupMorphologySurface(NeuronTree neurontree,vector<Sphere*>& spheres,vector<Cylinder*>& cylinders,vector<glm::vec3>& spheresPos);
	void SetupSingleMorphologyLine(int ntIndex, int procvessMode = 0);
	void SetupAllMorphologyLine();

    void SetupMarkerandSurface(double x,double y,double z,int type);
	void SetupMarkerandSurface(double x,double y,double z,int colorR,int colorG,int colorB);

        bool RemoveMarkerandSurface(double x,double y,double z/*,int type=3,bool asg=0*/);
        bool RemoveMarkerandSurface2(double x,double y,double z,int type=3,bool asg=0);
	void RenderControllerAxes();//draw XYZ axes on the base point of the controllers 

	bool SetupStereoRenderTargets();
	void SetupCompanionWindow();
	void SetupCameras();
	void SetupCamerasForMorphology();

	void MenuFunctionChoose(glm::vec2 UV);
	void ColorMenuChoose(glm::vec2 UV);
	//undo redo
	void UndoLastSketchedNT();
	void RedoLastSketchedNT();
	void ClearUndoRedoVectors();

	void SetupGlobalMatrix();//matrix for glabal transformation
	void RenderStereoTargets();
	void RenderCompanionWindow();
	void RenderScene( vr::Hmd_Eye nEye );

	Matrix4 GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye );
	Matrix4 GetHMDMatrixPoseEye( vr::Hmd_Eye nEye );
	Matrix4 GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye );
	void UpdateHMDMatrixPose();

	Matrix4 ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t &matPose );

	GLuint CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader );
	bool CreateAllShaders();

	void SetupRenderModelForTrackedDevice( vr::TrackedDeviceIndex_t unTrackedDeviceIndex );
	CGLRenderModel *FindOrLoadRenderModel( const char *pchRenderModelName );

	float GetGlobalScale();

        void TriggerHapticPluse()
        {
             m_pHMD->TriggerHapticPulse(m_iControllerIDLeft,0,30000);
        }

     EEGdevice eegDevice; // Instance of EEGdevice
     bool showImage=1;
public:

	MainWindow *mainwindow;
	My4DImage *img4d;
    QTimer *m_timer;
    QTimer *timer_eegGet;
	static My4DImage *img4d_replace;
	bool replacetexture;
	QList<NeuronTree> *loadedNTList; // neuron trees brought to the VR view from the 3D view.	
	NTL editableLoadedNTL;
	NTL nonEditableLoadedNTL;
	bool READY_TO_SEND;
	bool isOnline;
	static ModelControlR  m_modeGrip_R;
	static ModeControlSettings m_modeGrip_L;

    QString delSegName;
    NeuronTree segtobedeleted;
    QString delMarkerName;
	QString markerPosTobeDeleted;

    QVector<NeuronTree> segaftersplit;

	QString dragnodePOS;
	bool _call_assemble_plugin;
	bool _startdragnode;
	int postVRFunctionCallMode;
	XYZ teraflyPOS;
	XYZ CmainVRVolumeStartPoint;
    XYZ CmainVRVolumeEndPoint;
	int CmainResIndex;
	XYZ CollaborationCreatorPos;
    XYZ CollaborationCreatorGLOPos;
	XYZ CollaborationMaxResolution;
	XYZ CollaborationCurrentRes;
	XYZ CollaborationTargetMarkerRes;
	XYZ collaborationTargetdelcurveRes;
	XYZ SegNode_tobedeleted;//second node of seg , same to terafly collaboration delete seg

private: 
	std::string current_agent_color;
	std::string current_agent_name;
	bool m_bDebugOpenGL;
	bool m_bVerbose;
	bool m_bPerf;
	bool m_bVblank;
	bool m_bGlFinishHack;
	bool m_bShowMorphologyLine;
	bool m_bShowMorphologySurface;
	bool m_bControllerModelON;
	bool m_bShowMorphologyMarker;
	QString line_tobedeleted;
    QString marker_tobedeleted;
    int color_origin_seg;
    int type_origin_marker;
    RGB8 color_origin_marker;
    const int colorForTobeDelete = 19;
	int  sketchNum; // a unique ID for neuron strokes, useful in deleting neurons
    int markerNum;
	NeuronTree loadedNT_merged; // merged result of loadedNTList
	
	QList<NeuronTree> sketchedNTList; //neuron trees drawn in the VR view.	
	public:
	NeuronTree currentNT;// currently drawn stroke of neuron
	private:
	NeuronTree tempNT;//used somewhere, can be change to a local variable
	BoundingBox swcBB;
	QList<ImageMarker> drawnMarkerList;
	vector<int> markerVisibility; //control the visibility of individual markers. temporarily used for VR experiment.
	vector<qint64> elapsedTimes;
	QElapsedTimer timer1;
	int curveDrawingTestStatus;

	vr::IVRSystem *m_pHMD;
	vr::HmdQuad_t *rect;
	vr::IVRRenderModels *m_pRenderModels;
	vr::IVRChaperone *m_pChaperone;

	vr::HmdVector3_t HmdQuadImageOffset;
	std::string m_strDriver;
	std::string m_strDisplay;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ]; //note: contain everything: validity, matrix, ...
	Matrix4 m_rmat4DevicePose[ vr::k_unMaxTrackedDeviceCount ]; //note: store device transform matrices, copied from m_rTrackedDevicePose
	bool m_rbShowTrackedDevice[ vr::k_unMaxTrackedDeviceCount ];

	//gltext::Font * font_VR;//font for render text
    static float fSSVEPHz;
    static float fSSVEPHz_input;
	//undo redo
	bool bIsUndoEnable;
	bool bIsRedoEnable;
    bool isSSVEP=false;
	vector<NTL> vUndoList;
	vector<NTL> vRedoList;
public:
	static int m_curMarkerColorType;

private: // SDL bookkeeping
	SDL_Window *m_pCompanionWindow;
	uint32_t m_nCompanionWindowWidth;
	uint32_t m_nCompanionWindowHeight;

	SDL_GLContext m_pContext;

private: // OpenGL bookkeeping
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	static bool m_bFrozen; //freeze the view
	static bool m_bVirtualFingerON;

	//control main functions in right controller
	int  m_modeControlTouchPad_R;
	int m_modeControlGrip_R;
	//control other functions in left controller
	static int m_modeControlGrip_L;

	static ModeTouchPadR m_modeTouchPad_R;
	static SecondeMenu m_secondMenu;
	static RGBImageChannel m_rgbChannel;
	/*FlashType m_flashtype;
	XYZ FlashCoords;
	long m_FlashCount;
	int m_Flashcolor;
	int m_Flashoricolor;*/
	bool singlechannel;
	bool m_contrastMode;
	bool m_rotateMode;
	bool m_zoomMode;
	bool m_autoRotateON;
	bool m_TouchFirst;
	bool m_pickUpState;
	/////store the pos every first time touch on the touchpad
	float m_fTouchOldX;
	float m_fTouchOldY;

	int pick_point_index_A;
	int pick_point_index_B;
	NeuronSWC * pick_node;

	float detX;
	float detY;
	
	glm::vec3 loadedNTCenter;
	glm::vec3 autoRotationCenter;
	long int vertexcount, swccount;
	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	char m_rDevClassChar[ vr::k_unMaxTrackedDeviceCount ];   // for each device, a character representing its class


	
	float m_fNearClip;

	float m_fFarClip;

	GLuint m_iTexture;
	GLuint m_ControllerTexVAO;
	GLuint m_ControllerTexVBO;
	GLuint m_unCtrTexProgramID;
	GLint m_nCtrTexMatrixLocation;
	unsigned int m_uiControllerTexIndexSize;
	
	//volume rendering
	//MinMaxOctree* minmaxOctree_step8;
	//MinMaxOctree* minmaxOctree_step16;
	//MinMaxOctree* minmaxOctree_step32;
	//right controller shootingray VAO/VBO
	GLuint m_iControllerRayVAO;
	GLuint m_iControllerRayVBO;
	// controller index , get them in HandleInput()
	int	m_iControllerIDLeft;
	int	m_iControllerIDRight;

	//unsigned int m_uiVertcount;

	//VAO/VBO for surface and line of loaded neuron
	vector<Sphere*> loaded_spheres;
	vector<Cylinder*> loaded_cylinders;
	vector<glm::vec3> loaded_spheresPos;
	vector<glm::vec3> loaded_spheresColor;

	vector<Sphere*> Agents_spheres;
	vector<glm::vec3> Agents_spheresPos;
	vector<glm::vec3> Agents_spheresColor;

	vector<Sphere*> Markers_spheres;
	vector<glm::vec3> Markers_spheresPos;
	vector<glm::vec3> Markers_spheresColor;

	Sphere* ctrSphere; // indicate the origin for curve drawing
	glm::vec3 ctrSpherePos;
	glm::vec3 ctrSphereColor;
	glm::vec3 u_clipnormal;
	glm::vec3 u_clippoint;

	GLuint m_unMorphologyLineModeVAO;
	GLuint m_glMorphologyLineModeVertBuffer;
	GLuint m_glMorphologyLineModeIndexBuffer;
	unsigned int m_uiMorphologyLineModeVertcount;

	//VAO/VBO for surface and line of loaded neuron
	vector<Sphere*> sketch_spheres; //2017/11/13, wym: kind of obselete, drawn curves are not shown in surface mode 
	vector<Cylinder*> sketch_cylinders;
	vector<glm::vec3> sketch_spheresPos;

	GLuint m_unSketchMorphologyLineModeVAO;//for local sketch swc
	GLuint m_glSketchMorphologyLineModeVertBuffer;
	GLuint m_glSketchMorphologyLineModeIndexBuffer;
	unsigned int m_uiSketchMorphologyLineModeVertcount;



	GLuint m_unCompanionWindowVAO; //two 2D boxes
	GLuint m_glCompanionWindowIDVertBuffer;
	GLuint m_glCompanionWindowIDIndexBuffer;
	unsigned int m_uiCompanionWindowIndexSize;

	GLuint m_glControllerVertBuffer;
	GLuint m_unControllerVAO;//note: axes for controller
	GLuint m_unImageAxesVAO;
	GLuint m_unImageAxesVBO;
	unsigned int m_uiControllerVertcount;
	unsigned int m_uiControllerRayVertcount;//note: used to draw controller ray
	unsigned int m_uiImageAxesVertcount;
	Matrix4 m_mat4HMDPose;//note: m_rmat4DevicePose[hmd].invert()
	Matrix4 m_mat4eyePosLeft;
	Matrix4 m_mat4eyePosRight;

	Matrix4 m_mat4ProjectionCenter;
	Matrix4 m_mat4ProjectionLeft;
	Matrix4 m_mat4ProjectionRight;

	//for morphology rendering
	glm::mat4 m_HMDTrans;
	glm::mat4 m_EyeTransLeft;//head to eye
	glm::mat4 m_EyeTransRight; 
	glm::vec3 m_EyePosLeft;
	glm::vec3 m_EyePosRight;
	glm::mat4 m_ProjTransLeft;
	glm::mat4 m_ProjTransRight;

	float m_globalScale; // m_globalScale is consistent with m_globalMatrix, and is required somewhere
	static glm::mat4 m_globalMatrix;
	glm::mat4 m_oldGlobalMatrix;
	glm::mat4 m_ctrlChangeMatrix;
	glm::mat4 m_oldCtrlMatrix;
	   

	//matrices to store frozen state
	Matrix4 m_frozen_mat4HMDPose;
	glm::mat4 m_frozen_HMDTrans;
	glm::mat4 m_frozen_globalMatrix;


	struct VertexDataScene//question: why define this? only used for sizeof()
	{
		Vector3 position;
		Vector2 texCoord;
	};

	struct VertexDataWindow//question: companion window just uses the projected data points from HMD?
	{
		Vector2 position;
		Vector2 texCoord;

		VertexDataWindow( const Vector2 & pos, const Vector2 tex ) :  position(pos), texCoord(tex) {	}
	};

	Shader* morphologyShader;
	GLuint m_unCompanionWindowProgramID;
	GLuint m_unControllerTransformProgramID;
	GLuint m_unRenderModelProgramID;
	GLuint m_unControllerRayProgramID;
	GLuint m_unImageAxesProgramID;
	GLint m_nControllerMatrixLocation;
	GLint m_nRenderModelMatrixLocation;
	GLint m_nControllerRayMatrixLocation;
	GLint m_nImageAxesMatrixLocation;
	struct FramebufferDesc
	{
		GLuint m_nDepthBufferId;
		GLuint m_nRenderTextureId;
		GLuint m_nRenderFramebufferId;
		GLuint m_nResolveTextureId;
		GLuint m_nResolveFramebufferId;
	};
	FramebufferDesc leftEyeDesc;
	FramebufferDesc rightEyeDesc;

	bool CreateFrameBuffer( int nWidth, int nHeight, FramebufferDesc &framebufferDesc );
	
	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;

	std::vector< CGLRenderModel * > m_vecRenderModels; //note: a duplicated access to below. used in shutdown destroy, check existence routines;
	CGLRenderModel *m_rTrackedDeviceToRenderModel[ vr::k_unMaxTrackedDeviceCount ]; //note: maintain all the render models for VR devices; used in drawing


	//sketchNTL all NTs' VAO&VBO
	vector<GLuint> iSketchNTLMorphologyVAO;
	vector<GLuint> iSketchNTLMorphologyVertBuffer;
	vector<GLuint> iSketchNTLMorphologyIndexBuffer; 
	vector<unsigned int> iSketchNTLMorphologyVertcount;
/***********************************
***    volume image rendering    ***
***********************************/
public:
	void SetupCubeForImage4D();
	GLuint initTFF1DTex();
	GLuint initFace2DTex(GLuint texWidth, GLuint texHeight);
	GLuint initVol3DTex();
	GLuint initVolOctree3DTex(int step,GLuint octreestep);
	void initFrameBufferForVolumeRendering(GLuint texObj, GLuint texWidth, GLuint texHeight);
	void SetupVolumeRendering();
	bool CreateVolumeRenderingShaders();
	void RenderImage4D(Shader* shader, vr::Hmd_Eye nEye, GLenum cullFace);
	void SetUinformsForRayCasting();

public:
	bool m_bHasImage4D;
private:
	
	void * RGBImageTexData;
	GLuint m_clipPatchVAO;
	GLuint m_VolumeImageVAO;
	Shader* backfaceShader;//back face, first pass
	Shader* raycastingShader;//ray casting front face, second pass
	Shader* clipPatchShader;//ray casting front face, second pass

	GLuint g_winWidth; //todo: may be removable. wym
	GLuint g_winHeight;
	GLuint g_frameBufferBackface; //render backface to frameBufferBackface
	GLuint g_tffTexObj;	// transfer function
	GLuint g_bfTexObj;
	GLuint g_texWidth;
	GLuint g_texHeight;
	GLuint g_volTexObj;
	GLuint g_volTexObj_octree_8;
	GLuint g_volTexObj_octree_16;
	GLuint g_volTexObj_octree_32;	
	GLuint g_volTexObj_octree_64;
	GLuint g_volTexObj_octree_128;			
	static float fBrightness;
	static float fContrast;
	float fSlabwidth;//used to control slabplane width

	double countsPerSecond;
	__int64 CounterStart;

	int frameCount;
	int fps;

	__int64 frameTimeOld;
	double frameTime;

	void StartTimer();
	double GetTime();
	double GetFrameTime();

	static float iLineWid;
	static float iscaleZ;
	public:
	static bool showshootingPad;

	glm::vec3  shootingraystartPos;
	glm::vec3  shootingrayDir;
	glm::vec3 shootingraycutPos;
	glm::vec2 calculateshootingPadUV();
	
	bool showshootingray;
	QString collaboration_creator_name;
	template<typename T>
	void HelpFunc_createOctreetexture(int step);
	void bindTexturePara();

public:
        float get_mglobalScal()const;

public:
        NeuronTree UndoNT;
        bool undo=false;
        bool redo=false;



        QList<double> getSumData() const;
        QList<double> getCurSingleData() const;
public slots:
        void ImageDisplay(bool show);
        void onTimerTimeout();
        void timerTimeout();
private:
        unsigned int framecnt=0;
        const int numofframe=7;
//signals:
//        void undo();
};

//Help Function


#endif

