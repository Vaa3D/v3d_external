/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/*
 *  renderer.h
 *
 *  Created by Ruan Zongcai on 8/6/08.
 *  Copyright 2008-2009 Hanchuan Peng
 *  Last update: 090204 update processHit().
 *  Last change: 2010-Dec-09. by Hanchuan Peng. add template functions at the end
 */

#ifndef V3D_RENDERER_H
#define V3D_RENDERER_H

#include "v3dr_common.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Renderer
// to create:
//		renderer = new Renderer(widget);
//		renderer->setupData(data);
//		renderer->initialize();
// optional: renderer->getLimitedDataSize(data_size);
// to delete:
//		delete renderer;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Renderer
{
public:
	enum RenderMode {rmCrossSection=0, rmAlphaBlending, rmMaxIntensityProjection };
	enum SelectMode {smObject=0, smMultipleObject,
					smMarkerCreate1, smMarkerCreate2, smMarkerCreate3,
					smMarkerRefineT, smMarkerRefineC,
					smCurveCreate1, smCurveCreate2, smCurveCreate3, smCurveCreate_pointclick,
					smCurveCreateM,
					};
protected:
	RenderMode renderMode;
	SelectMode selectMode;
	void* widget;

public:
	Renderer(void* widget); //100827 add widget
	virtual ~Renderer();
	virtual const int class_version() {return 0;}

	bool hasError() {return b_error;}
	void makeCurrent(); //ensure right-GL-context when resize-view, animate, mouse-drop, delete-object, select-object etc. across multiple views, 081105
	void drawString(float x, float y, float z, char* text, int shadow=0);
	bool beStill();

public:
// link to Data (volume & surface)
	virtual void setupData(void* data) {};
	virtual void cleanData()           {};
	virtual void getLimitedDataSize(int size[5]) {for (int i=0;i<5;i++) size[i]=bufSize[i];};
	virtual bool beLimitedDataSize() {return b_limitedsize;};
	virtual void getBoundingBox(BoundingBox& bb) {bb = boundingBox;};

// link to OpenGL window
	virtual void setupView(int width, int height);		//link to QGLWidget::resizeGL
	virtual void initialize(int version=0);				//link to QGLWidget::initializeGL
	virtual void reinitializeVol(int version=0) {};							//MUST makeCurrent for concurrent contexts
	virtual void paint();								//link to QGLWidget::paintGL
	virtual int hitPoint(int x, int y)					//called by mouse press event
		{
			if (selectMode==smObject)	return selectObj(x,y, true); //pop-up menu
			else						return hitPen(x,y);			 //marker definition
		}
	virtual int selectObj(int x, int y, bool b_menu, char* pTip=0);			//MUST makeCurrent for concurrent contexts, 090715
				// do glSelectBuffer                    //called by hitPoint, tool tip event
	virtual int hitPen(int x, int y)					//called by hitPoint
				{return 0;} //add return 0 on 2010-05-10 to fix the "must return a value" bug. by PHC.
	virtual int movePen(int x, int y, bool b_move)		//called by mouse move event
				{return 0;} //add return 0 on 2010-05-10 to fix the "must return a value" bug. by PHC.

        virtual int hitMenu(int x, int y){}  //overwrite pop-up menu by mouse right click of neuron annotator

// link to Rendering function
protected:
	virtual void setProjection(); 			// called by setupView & selectObj
	virtual int processHit(int namelen, int names[], int x, int y, bool b_menu, char* pTip=0)	// called by selectObj. add x,y by PHC 090204
				{return 0;} // 0 for no any hit. Here just is a virtual interface to process hit.

	virtual void drawObj()  {}; 			// called in paint()
	virtual void cleanObj() {};												// MUST makeCurrent for concurrent contexts, 081025
	virtual void loadObj()	{boundingBox=surfBoundingBox=UNIT_BoundingBox;}	// MUST makeCurrent for concurrent contexts, 081025

	virtual void setObjectSpace();
	virtual void setSurfClipSpace();
	virtual void setBoundingBoxSpace(BoundingBox BB);
	virtual void drawBoundingBoxAndAxes(BoundingBox BB, float BlineWidth=1, float AlineWidth=3);

	virtual void drawScaleBar(float AlineWidth=3);

	virtual void updateVolCutRange();
	virtual void updateSurfClipRange();

// public View control interface
public:
        virtual float getViewDistance() const {return viewDistance;}
        virtual float getViewAngle() const {return viewAngle;}
        virtual float getZoomRatio() const {return zoomRatio;}
	virtual void setZoom(float ratio);

	virtual int setVolumeTimePoint(int t) {volTimeOffset = t-volTimePoint; volTimePoint = t; return volTimePoint;}
	virtual void setRenderMode(RenderMode rm) {renderMode = rm;}
	virtual void setXCut0(int i) {xCut0 = i; updateVolCutRange();}
	virtual void setXCut1(int i) {xCut1 = i; updateVolCutRange();}
	virtual void setYCut0(int i) {yCut0 = i; updateVolCutRange();}
	virtual void setYCut1(int i) {yCut1 = i; updateVolCutRange();}
	virtual void setZCut0(int i) {zCut0 = i; updateVolCutRange();}
	virtual void setZCut1(int i) {zCut1 = i; updateVolCutRange();}
	virtual void setXClip0(float f)	{xClip0 = f; updateSurfClipRange();}
	virtual void setXClip1(float f) {xClip1 = f; updateSurfClipRange();}
	virtual void setYClip0(float f) {yClip0 = f; updateSurfClipRange();}
	virtual void setYClip1(float f) {yClip1 = f; updateSurfClipRange();}
	virtual void setZClip0(float f) {zClip0 = f; updateSurfClipRange();}
	virtual void setZClip1(float f) {zClip1 = f; updateSurfClipRange();}

	virtual void enableClipBoundingBox(BoundingBox& bb, bool bDynamicClip, double border=0.001);
	virtual void disableClipBoundingBox();
	//090705: the folowing three variable/functions are used to set the clip box state for displaying surface objects in drawObj()
	bool b_useClipBoxforSubjectObjs;
	void setClipBoxState_forSubjectObjs(bool mystate) {b_useClipBoxforSubjectObjs = mystate;}
	bool getClipBoxState_forSubjectObjs() {return b_useClipBoxforSubjectObjs;}

	virtual void setViewClip(float f);
	virtual void enableViewClipPlane();
	virtual void disableViewClipPlane();

	virtual void endSelectMode()      {};
	virtual void updateLandmark()     {};
	virtual void updateTracedNeuron() {};

	virtual void loadObjectListFromFile()              {};
	virtual void loadObjectFromFile(const char* url=0) {};
	virtual void createSurfCurrent(int ch=0)            {};
	virtual void saveSurfFile()                         {};
	virtual void editSurfaceObjAnnotation(int dataClass, int surfaceType, int index)    {};
	virtual void editSurfaceObjBasicGeometry(int dataClass, int surfaceType, int index) {};

	virtual void togglePolygonMode() {polygonMode = (polygonMode +1) %3;} // FILL,LINE,POINT
	virtual void toggleLineType()       {};

	virtual void setThickness(double)   {};
	virtual void toggleTexFilter()      {};
	virtual void toggleTex2D3D()        {};
	virtual void toggleTexCompression() {};
	virtual void toggleShader()         {};
	virtual void toggleTexStream()      {};
	virtual void toggleObjShader()      {};

	virtual void updateVolShadingOption() {};
	virtual void updateObjShadingOption() {};


public:
	int sShowTrack, curChannel;

	bool bShowBoundingBox, bShowBoundingBox2, bShowAxes, bOrthoView;
	bool bShowCSline, bShowFSline, bFSlice, bXSlice, bYSlice, bZSlice;
	float CSbeta, alpha_threshold;
	RGBA32f color_background, color_background2, color_line, color_proxy;

	int sShowMarkers, sShowSurfObjects, markerSize;
	bool b_showMarkerLabel, b_showMarkerName, b_showCellName, b_surfStretch;

	int lineType, lineWidth, nodeSize, rootSize;
	int polygonMode, tryObjShader;
	int tryTexNPT, tryTex3D, tryTexCompress, tryVolShader, tryTexStream;
	const char* try_vol_state();

// internal state
protected:
	int volTimePoint, volTimeOffset;
	BoundingBox boundingBox, surfBoundingBox;
	double thickness; //changed from int to double, PHC, 090215
	V3DLONG xCut0,xCut1, yCut0,yCut1, zCut0,zCut1;            // for volume
	float xClip0,xClip1, yClip0,yClip1, zClip0,zClip1;    // for surface
	float viewClip;

	V3DLONG screenW, screenH;
	float viewDistance, viewNear, viewFar, viewAngle, zoomRatio;

	bool b_error;
	bool b_selecting;

	bool b_limitedsize;
	float *depth_buffer;
	RGBA8 *total_rgbaBuf, *rgbaBuf;
	float sampleScale[5];
	V3DLONG bufSize[5]; //(x,y,z,c,t) 090731: add time dim

private:
	void init_members()
	{
	    //public ----------------------------------------
		b_useClipBoxforSubjectObjs = true;

		sShowTrack = 0;
		curChannel = 0;

		bShowBoundingBox = true;
		bShowBoundingBox2 = false;
	    bShowAxes = true;
	    bOrthoView = false;

	    bShowCSline = true;
	    bShowFSline = true;
	    bXSlice = bYSlice = bZSlice = true;
	    bFSlice = true; //100729 RZC: it's not slow, default turn on.
	    CSbeta = alpha_threshold = 0;
	    thickness = 1;

	    color_background = XYZW(.1f, .1f, .25f, 1); // background color for volume
	    color_background2 = XYZW(.8f, .85f, .9f, 1); // background color for geometric object only
		color_line = XYZW(.5f,.5f,.7f, 1);
		color_proxy = XYZW(1, 1, 1, 1);

		sShowMarkers = 2;
		sShowSurfObjects = 2;
		markerSize = 10;
		b_showMarkerLabel = true;
		b_showMarkerName = false; // by Lei Qu, 110425
		b_surfStretch = true;
		b_showCellName    = false;

		lineType = 1; lineWidth = 1; nodeSize = 0, rootSize = 5;
		polygonMode = 0;
		tryObjShader = 0;

		tryTexNPT = 0;
		tryTex3D = 0;
		tryTexCompress = 1;
		tryTexStream = 1;
		tryVolShader = 1;

		volTimePoint = volTimeOffset = 0;

		//protected -------------------------------------

		xCut0 = yCut0 = zCut0 = -1000000; // no cut
	    xCut1 = yCut1 = zCut1 = 1000000;  // no cut
	    xClip0 = yClip0 = zClip0 = -1000000; // no clip
	    xClip1 = yClip1 = zClip1 = 1000000;  // no clip
	    viewClip = 1000000;  // no clip
	    renderMode = rmMaxIntensityProjection;
	    selectMode = smObject;

		//// perspective view frustum
		screenW = screenH = 0;
		viewAngle = 31;
		zoomRatio = 1;
	    viewNear = 1;
	    viewFar = 10;
        viewDistance = 5;

	    b_error = b_selecting = false;

	    b_limitedsize = false;
	    depth_buffer = 0;
	    total_rgbaBuf = rgbaBuf = 0;
		for (int i=0; i<5; i++)
		{
		    sampleScale[i]=1; bufSize[i]=0;
		}
	}

};

//////////////////////////////////////////////////////////////////////////////////////
// data buffer convert
//090731: memory offset change to V3DLONG for 64bit

unsigned char* createSimulatedData(V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,	V3DLONG dim4, V3DLONG dim5);

void getLimitedSampleScaleBufSize(V3DLONG dim1, V3DLONG dim2, V3DLONG dim3, V3DLONG dim4, V3DLONG dim5,
		float sampleScale[5], V3DLONG bufSize[5]);

void data4dp_to_rgba3d(unsigned char* data4dp, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3, V3DLONG dim4, V3DLONG dim5,
		V3DLONG start1, V3DLONG start2, V3DLONG start3, V3DLONG start4,
		V3DLONG size1, V3DLONG size2, V3DLONG size3, V3DLONG size4,
		RGBA8* rgbaBuf, V3DLONG bufSize[5]);
void rgba3d_r2gray(RGBA8* rgbaBuf, V3DLONG bufSize[5]);

template <class T> float sampling3dAllTypes(T* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,
		V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG dx, V3DLONG dy, V3DLONG dz);
RGBA32f sampling3dRGBA8(RGBA8* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,
		V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG dx, V3DLONG dy, V3DLONG dz);

template <class T> float sampling3dAllTypesat(T* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3, 
									 float x, float y, float z);
RGBA32f sampling3dRGBA8at(RGBA8* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,
		float x, float y, float z);
template <class T> float sampling3dAllTypesatBounding(T* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,
		float x, float y, float z,
		const float box[6], //bounding box
		const double clip[4]=0 //clip plane, keep for dot(clip, pos)>=0
		);

//100721: small tag for pinpointing in getCenterOfLineProfile
//        if sample in bound
//              add a small tag 1e-6 to return
//        else out of bound
//              return 0



RGB8 getRGB3dUINT8(unsigned char* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3, V3DLONG dim4,
		V3DLONG x, V3DLONG y, V3DLONG z);
void setRGB3dUINT8(unsigned char* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3, V3DLONG dim4,
		V3DLONG x, V3DLONG y, V3DLONG z, RGB8 tmp);


void data4dp_to_rgba3d(Image4DProxy<Image4DSimple>& img4dp, V3DLONG dim5,
		V3DLONG start1, V3DLONG start2, V3DLONG start3, V3DLONG start4,
		V3DLONG size1, V3DLONG size2, V3DLONG size3, V3DLONG size4,
		RGBA8* rgbaBuf, V3DLONG bufSize[5]);
float sampling3dUINT8(Image4DProxy<Image4DSimple>& img4dp,
		V3DLONG c,
		V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG dx, V3DLONG dy, V3DLONG dz);

float sampling3dUINT8_2(Image4DProxy<Image4DSimple>& img4dp,
                V3DLONG c,
                V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG dx, V3DLONG dy, V3DLONG dz, V3DLONG dxyz);

//float sampling3dUINT8at(Image4DProxy<Image4DSimple>& data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,
//		float x, float y, float z);
//float sampling3dUINT8atBounding(Image4DProxy<Image4DSimple>& data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,
//		float x, float y, float z,
//		const float box[6], //bounding box
//		const double clip[4]=0 //clip plane, keep for dot(clip, pos)>=0
//		);


//////////////////////////////////////////////////////////////////////////////////////
// GL operator

// render operator
int numPassFloatDraw(int sShow);
void setFloatDrawOp(int pass, int sShow); // sShow: 1--normal occlusion, 2--object float over other object, else nothing
// for (int i=0; i<numPassFloatDraw(sShow); i++) {
//		setFloatDrawOp(i, sShow);
//		......
// } setFloatDrawOp(-1, sShow); //for end

void blendBrighten(float fbright); // fast, 8-bit precision
void accumBrighten(float fbright); // slow, 16-bit precision


// 1 texture pixel = (components * data_type) bytes, compress_format=GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
int getMaxTexSize1D(int components=4, int data_type=1, int compress_format=0);
int getMaxTexSize2D(int components=4, int data_type=1, int compress_format=0);
int getMaxTexSize3D(int components=4, int data_type=1, int compress_format=0);
int getMaxTexSize2X(int components=4, int data_type=1, int compress_format=0); //test X when fixed Y
int getMaxTexSize2Y(int components=4, int data_type=1, int compress_format=0); //when fixed X text Y
int getMaxTexSize3Z(int components=4, int data_type=1, int compress_format=0); //when fixed X*Y test Z
int getMaxTexSize3X(int components=4, int data_type=1, int compress_format=0); //test X*Y when fixed Z


void ViewRotToModel(const double mRot[16], double& rx, double& ry, double& rz);
void ModelRotToView(const double mRot[16], double& rx, double& ry, double& rz);
// 081222: For 64-bit Linux, seems only memcpy works properly !!! SO CRAZY
//#define MAT4x4_TO_MAT16(m4x4, m16)  { for (V3DLONG i=0; i<16; i++)  (m16)[i] = (&((m4x4)[0][0]))[i]; }
//#define MAT16_TO_MAT4x4(m16, m4x4)  { for (V3DLONG i=0; i<16; i++)  (&((m4x4)[0][0]))[i] = (m16)[i]; }
#define MAT4x4_TO_MAT16(m4x4, m16)  memcpy( (m16), &((m4x4)[0][0]), sizeof(m4x4))
#define MAT16_TO_MAT4x4(m16, m4x4)  memcpy( &((m4x4)[0][0]), (m16), sizeof(m4x4))

void ViewToModel(const double modelView[16], double& vx, double& vy, double& vz);
void ViewPlaneToModel(const double modelView[16], double plane[4]);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL related
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool supported_MultiSample()
{
	GLeeInit();	return (GLEE_ARB_multisample>0);
}
inline bool supported_Tex3D()
{
	GLeeInit();	return (GLEE_VERSION_2_0>0 || GLEE_EXT_texture3D>0);
	//return getMaxTexSize3D()>0; // (VERSION_2_0==0 when ssh -X) (EXT_texture3D==0 when NVIDIA)
}
inline bool supported_TexCompression()
{
	GLeeInit();	return (GLEE_ARB_texture_compression>0);
}
inline bool supported_TexNPT()
{
	GLeeInit();	return (GLEE_ARB_texture_non_power_of_two>0);
}
inline bool supported_GL2()
{
	GLeeInit();	return (GLEE_VERSION_2_0>0);
}
inline bool supported_GLSL()
{
	GLeeInit();	return (GLEE_ARB_shading_language_100>0) && (GLEE_VERSION_2_0>0);
}
inline bool supported_PBO()
{
	GLeeInit();	return (GLEE_ARB_pixel_buffer_object>0);
}

/////////////////////////////////////////////////////////////////////////////

#define CHECK_GLErrorId_throw() \
{ \
	GLenum id = glGetError(); \
	switch (id) \
	{ \
	case GL_NO_ERROR: \
		break; \
	case GL_INVALID_ENUM: \
	case GL_INVALID_VALUE: \
	case GL_INVALID_OPERATION: \
		break; \
	case GL_STACK_OVERFLOW: \
	case GL_STACK_UNDERFLOW: \
	case GL_OUT_OF_MEMORY: \
	case GL_TABLE_TOO_LARGE: \
		throw id; break; \
	default: \
		throw id; \
	} \
}
#define CHECK_GLErrorString_throw() \
try { \
	CHECK_GLErrorId_throw(); \
} catch(GLenum id) { \
	const char* str = (const char*)gluErrorString(id); \
	throw str; \
}
#define CHECK_GLError_print() CheckGLError_print(__FILE__, __LINE__)
inline int CheckGLError_print(const char *file, int line)
{
	GLenum glErr;
	while ((glErr = glGetError()) != GL_NO_ERROR)
	{
		const GLubyte* sError = gluErrorString(glErr);
		if (sError)
		std::cerr << "GL Error #" << glErr << "(" << sError << ") " << " in file(" << file << ") at line(" << line << ")\n";
		else
		std::cerr << "GL Error #" << glErr << " (no message available)" << " in file(" << file << ") at line(" << line << ")\n";
	}
	return glErr;
}

//special definition: there are some cases these following definitions are missing, and always define here if not defined before. 090730
// for mac 10.4 __DARWIN__ ?
#ifndef GL_SHADING_LANGUAGE_VERSION_ARB
#define GL_SHADING_LANGUAGE_VERSION_ARB  0x8B8C
#endif
#ifndef GL_MAX_DRAW_BUFFERS_ARB
#define GL_MAX_DRAW_BUFFERS_ARB  0x8824
#endif
#ifndef GL_MAX_COLOR_ATTACHMENTS_EXT
#define GL_MAX_COLOR_ATTACHMENTS_EXT  0x8CDF
#endif

void GLinfoDetect(std::string* pinfo);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int err_printf(char* format, ...);
char* bs_printf(char* format, ...);
//#define printf cerr<<
//#define printf qDebug


//the real implementation of several template functions
//inline
template <class T> float sampling3dAllTypes(T* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,
										 V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG dx, V3DLONG dy, V3DLONG dz)
{
	float avg = 0;
	float d = (dx*dy*dz);
	
	V3DLONG pagesz = dim1*dim2*dim3;
	
	if (d>0 && x>=0 && y>=0 && z>=0 && x+dx<=dim1 && y+dy<=dim2 && z+dz<=dim3)
	{
		//float s = 0;
		V3DLONG xi,yi,zi;
		for (zi=0; zi<dz; zi++)
			for (yi=0; yi<dy; yi++)
				for (xi=0; xi<dx; xi++)
				{
					//float w = MAX( (2-ABS(xi-0.5*dx)*2.0/dx), MAX( (2-ABS(yi-0.5*dy)*2.0/dy), (2-ABS(zi-0.5*dz)*2.0/dz) )); //090712
					//s += w;
					
					V3DLONG idx = (z + zi)*dim2*dim1 + (y + yi)*dim1 + (x	+ xi);
					
					if(idx<pagesz)
						avg += data[idx];// *w;
					else
						continue;
				}
		//d = s;
		avg /= d;
	}
	return avg;
}

//inline
template <class T> float sampling3dAllTypesat(T* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,
										   float x, float y, float z)
{
#define SAMPLE(ix,iy,iz)	sampling3dAllTypes(data,dim1,dim2,dim3,  ix,iy,iz, 1,1,1)
	
	V3DLONG x0,x1, y0,y1, z0,z1;
	x0 = floor(x); 		x1 = ceil(x);
	y0 = floor(y); 		y1 = ceil(y);
	z0 = floor(z); 		z1 = ceil(z);
	float xf, yf, zf;
	xf = x-x0;
	yf = y-y0;
	zf = z-z0;
	float is[2][2][2];
	is[0][0][0] = SAMPLE(x0, y0, z0);
	is[0][0][1] = SAMPLE(x0, y0, z1);
	is[0][1][0] = SAMPLE(x0, y1, z0);
	is[0][1][1] = SAMPLE(x0, y1, z1);
	is[1][0][0] = SAMPLE(x1, y0, z0);
	is[1][0][1] = SAMPLE(x1, y0, z1);
	is[1][1][0] = SAMPLE(x1, y1, z0);
	is[1][1][1] = SAMPLE(x1, y1, z1);
	float sf[2][2][2];
	sf[0][0][0] = (1-xf)*(1-yf)*(1-zf);
	sf[0][0][1] = (1-xf)*(1-yf)*(  zf);
	sf[0][1][0] = (1-xf)*(  yf)*(1-zf);
	sf[0][1][1] = (1-xf)*(  yf)*(  zf);
	sf[1][0][0] = (  xf)*(1-yf)*(1-zf);
	sf[1][0][1] = (  xf)*(1-yf)*(  zf);
	sf[1][1][0] = (  xf)*(  yf)*(1-zf);
	sf[1][1][1] = (  xf)*(  yf)*(  zf);
	
	float* ip = &is[0][0][0];
	float* sp = &sf[0][0][0];
	float sum = 0;
	for (V3DLONG i=0; i<8; i++) // weight sum
	{
		sum = sum + (ip[i]) * sp[i];
	}
	return (sum);
}

template <class T> float sampling3dAllTypesatBounding(T* data, V3DLONG dim1, V3DLONG dim2, V3DLONG dim3,
												   float x, float y, float z,
												   const float box[6], //bounding box
												   const double clip[4] //clip plane, keep for dot(clip, pos)>=0
												   )
{
	const float d = 0.5;
	if (BETWEENEQ(box[0]-d,box[3]+d, x) && BETWEENEQ(box[1]-d,box[4]+d, y) && BETWEENEQ(box[2]-d,box[5]+d, z))
	{
		if (clip && (clip[0]*x + clip[1]*y + clip[2]*z + clip[3] <0)) //clipped
			return 0;
		else
			return sampling3dAllTypesat(data, dim1, dim2, dim3,  x, y, z) + (1e-6); ///////
	}
	else
		return 0;
}




#endif //V3D_RENDERER_H
