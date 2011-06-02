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
 *  renderer_tex2.h
 *
 *  implementation in renderer_tex2.cpp, renderer_obj2.cpp, renderer_labelfield.cpp, renderer_hit2.cpp
 *
 *
 *  Created by Ruan Zongcai on 8/6/08.
 *  last update: 090117 by Hanchuan Peng. add memory neuron swc passing interface
 *  last update: 090204 by Hanchuan Peng add the findNearestNeuronNode_WinXY()
 *  last update: 090219: add "name" and "comment" fields to all the basic object structs
 *  last edit by Hanchuan Peng, 090306, separate some basic obj types and also neuron manipulation functions
 *  last edit: Hancuan peng and Ruan Zongcai, 090705, add a flag to enable/disable the clipBox which is used in the function drawObj()
 */

#ifndef V3D_RENDERER_TEX2_H
#define V3D_RENDERER_TEX2_H

#include "renderer.h"
#include "marchingcubes.h"


enum v3dr_DataClass { dcDataNone=0,
				dcVolume=1,
				dcSurface=2
				};
enum v3dr_VolSlice { vsSliceNone=0,
				vsXslice=1,
				vsYslice=2,
				vsZslice=3,
				vsFslice=4,
				};
enum v3dr_SurfaceType { stSurfaceNone=0,
				stImageMarker=1,
				stLabelSurface=2,
				stNeuronStructure=3,
				stPointCloud=4,
				stPointSet=5,
				};

//////////////////////////////////////////////////////////////////////////////////////////


class Renderer_tex2 : public Renderer
{
	friend class V3dr_surfaceDialog; //for accessing all surface data structure
	friend class V3dr_colormapDialog;

public:
	Renderer_tex2(void* widget);
	virtual ~Renderer_tex2();
	virtual const int class_version() {return 1;}

public:
// link to Data
	virtual void setupData(void* data);
	virtual void cleanData();                      // makeCurrent
	virtual const bool has_image()   {return (size4>0);}
	virtual const BoundingBox getDataBox() {return dataBox;}

// link to OpenGL window
	//virtual void setupView(int width, int height);// use default
	virtual void initialize(int version=1);
	virtual void reinitializeVol(int version=1);					//MUST makeCurrent for concurrent contexts
	virtual void paint();
	virtual int hitPen(int x, int y); //return 0 means not processed
	virtual int movePen(int x, int y, bool b_move); //return 0 means not processed
	virtual void _appendMarkerPos(int x, int y);
	virtual void blendTrack();       // called by paint()
        virtual void setRenderTextureLast(bool renderTextureLast);

// link to Rendering function
protected:
	virtual int processHit(int namelen, int names[], int x, int y, bool b_menu, char* pTip=0);	// called by selectObj. add the x and y parameters by Hanchuan Peng,090204

	virtual void loadObj();  	// called by initialize()  	// makeCurrent
	virtual void cleanObj(); 	// called by ~Renderer_tex2	// makeCurrent
	virtual void drawObj();  	// called by paint()

	virtual void loadVol();  	// called by initialize()  	// makeCurrent
	virtual void cleanVol();	// called by ~Renderer_tex2	// makeCurrent
	virtual void drawVol(); 	// called by paint()
        virtual void prepareVol();
        virtual void renderVol();

	virtual void subloadTex(V3DLONG timepoint, bool bfisrt=false);	// called by loadVol, drawVol
	virtual void equAlphaBlending();			// for blending equation
	virtual void equMaxIntensityProjection();	// for blending equation
	virtual void equCrossSection();				// for blending equation
	virtual void shaderTexBegin(bool stream) 	{};		// for texture2D/3D shader
	virtual void shaderTexEnd()	 				{};		// for texture2D/3D shader

	virtual void updateVolCutRange();
	virtual void updateBoundingBox();
	virtual void updateThicknessBox();

// Rendering execution function
protected:
	virtual void setMarkerSpace();
	virtual void drawMarker();  // called by paint(), not by drawObj()

	virtual void setSurfaceStretchSpace();
	virtual	void setObjLighting();
	virtual void disObjLighting();
	virtual void beginHighlight();
	virtual void endHighlight();

	virtual void setUnitVolumeSpace();
	virtual void drawUnitVolume();
	virtual void setupStackTexture(bool bfirst);
	virtual int  _getBufFillSize(int w);
	virtual int  _getTexFillSize(int w);
	virtual void setTexParam3D();
	virtual void setTexParam2D();
	virtual void drawStackZ(float direction, int section, bool t3d, bool stream);
	virtual void drawStackY(float direction, int section, bool t3d, bool stream);
	virtual void drawStackX(float direction, int section, bool t3d, bool stream);
	virtual void _drawStack( double ts, double th, double tw,
			double s0, double s1, double h0, double h1, double w0, double w1,
			double ds, int slice0, int slice1, int thickness,
			GLuint tex3D, GLuint texs[], int stack_i,
			float direction, int section, bool t3d, bool stream);

	virtual bool supported_TexStream()								{return false;} //091003
	virtual void setupTexStreamBuffer()							{tex_stream_buffer = false;}; //091003
	virtual void cleanTexStreamBuffer()							{tex_stream_buffer = false;}; //091003
	virtual bool _streamingTex()									{return false;} //091015
	virtual void _streamTex(int stack_i, int slice_i, int step, int slice0, int slice1) {}; //091003
	virtual void _streamTex_end()													 	{}; //091003

	virtual void setupFrontSliceBuffer();
	virtual void drawUnitFrontSlice(int line=0);

	virtual void drawBackFillVolCube();
	virtual void drawCrossLine(float lineWidth=1);

// public View control interface
public:
	virtual void setRenderMode(RenderMode);
	virtual void setThickness(double t);
	virtual void toggleTexFilter();
	virtual void toggleTex2D3D();
	virtual void toggleTexCompression();

	virtual void togglePolygonMode() {polygonMode = (polygonMode +1) %4;} // FILL,LINE,POINT, transparent
	virtual void toggleLineType();

	//virtual void updateVolShadingOption();

	virtual void createSurfCurrent(int ch=0);                 // updateBoundingBox
	virtual void loadObjectFilename(const QString& filename); // updateBoundingBox // makeCurrent
	virtual void loadObjectListFromFile();             // call loadObjectFilename
	virtual void loadObjectFromFile(const char* url);  // call loadObjectFilename
	virtual void saveSurfFile();

	virtual void endSelectMode();
	virtual void updateLandmark();
	virtual void updateTracedNeuron();

// process Object hit ///////////////////////////////////////////////////////////////////////////////////////
public:

#ifndef test_main_cpp

	virtual void editSurfaceObjAnnotation(int dataClass, int surfaceType, int index);
	virtual void editSurfaceObjBasicGeometry(int dataClass, int surfaceType, int index);

	void updateNeuronTree(V_NeuronSWC & neuron_seg); //the interface to get a real-time generated swc tree
	V_NeuronSWC_list copyToEditableNeuron(NeuronTree * ptree);
	void finishEditingNeuronTree();

#endif

	QString info_Marker(int marker_i);
	QString info_NeuronNode(int node_i, NeuronTree * ptree);
	QString info_SurfVertex(int vertex_i, Triangle * triangle, int label);

	QList <NeuronTree> * getHandleNeuronTrees() {return &listNeuronTree;}
	V3DLONG findNearestNeuronNode_WinXY(int cx, int cy, NeuronTree * ptree);	//find the nearest node in a neuron in XY project of the display window.//return the index of the respective neuron node

	QList <CellAPO> *getHandleAPOCellList() {return &listCell;}

	Triangle * findNearestSurfTriangle_WinXY(int cx, int cy, int & vertex_i, Triangle * plist);

	double computeSurfaceArea(int dataClass, int surfaceType, int index);
	double computeSurfaceVolume(int dataClass, int surfaceType, int index);
	void showLineProfile(int marker1, int marker2);
	QVector<int> getLineProfile(XYZ p1, XYZ p2, int chno=0);

        XYZ selectPosition(int x, int y); 	// neuron selector
        int hitMenu(int x, int y); // neuron annotator right click popup menu

// define Marker/Curve  ///////////////////////////////////////////////
protected:
	GLint viewport[4];
	GLdouble projectionMatrix[16];
	GLdouble markerViewMatrix[16];
	struct MarkerPos {
		double x, y;		// input point coordinates
		int view[4];        // view-port
		double P[16];		// 4x4 projection matrix
		double MV[16];		// 4x4 model-view matrix
	};
	
	QCursor oldCursor;
	int lastSliceType;
	int currentMarkerName;
	QList <MarkerPos> listMarkerPos; //081221, screen projection position
	void _MarkerPos_to_NearFarPoint(const MarkerPos & pos, XYZ &loc0, XYZ &loc1);
	XYZ getLocationOfListMarkerPos();
	bool isInBound(const XYZ & loc, float factor=0.001, bool b_message=true);

	void solveMarkerViews();
	void solveMarkerCenter();
	void refineMarkerTranslate();
	void refineMarkerCenter();
	void refineMarkerLocal(int marker_id);
	XYZ getCenterOfMarkerPos(const MarkerPos& pos);
	XYZ getPointOnPlane(XYZ P1, XYZ P2, double plane[4]);
	XYZ getPointOnSections(XYZ P1, XYZ P2, double f_plane[4]);
	XYZ getCenterOfLineProfile(XYZ p1, XYZ p2,
			double clipplane[4]=0, //clipplane==0 to use the default viewClip plane
			int chno=-1);    //chno==-1 to re-check the current channel # for processing
	XYZ getCenterOfLocal(XYZ loc);
	void addMarker(XYZ &loc);
	void updateMarkerLocation(int marker_id, XYZ &loc); //PHC, 090120

	int cntCur3DCurveMarkers; //091226. marker cnt when define a curve using marker clicking
	bool b_addthiscurve; //for 1-stroke curve based zoom-in, PHC 100821
	bool b_addthismarker; //for 1-click based zoom-in, PHC 100821
	bool b_imaging; //for v3d_imaging, PHC 101008
	QList< QList <MarkerPos> > list_listCurvePos; //screen projection position
	void solveCurveCenter(vector <XYZ> & loc_vec_input);
	void solveCurveViews();
	void solveCurveFromMarkers();
	double getDistanceOfMarkerPos(const MarkerPos & pos0, const MarkerPos & pos);

	// in renderer_obj2.cpp
	void addCurveSWC(vector<XYZ> &loc_list, int chno=0); //if no chno is specified, then assume to be the first channel

	//for local view
	void produceZoomViewOf3DRoi(vector <XYZ> & loc_vec);


// Volume Parameters /////////////////////////////////////////////
protected:
	void* _idep;
	bool isSimulatedData;
	int data_unitbytes;
	unsigned char* data4dp;
	unsigned char**** data4d_uint8;
	// data4d_uint8[dim4][dim3][dim2][dim1]
	V3DLONG dim1, dim2, dim3, dim4, dim5;
	V3DLONG start1, start2, start3, start4, start5;
	V3DLONG size1, size2, size3, size4, size5;
	BoundingBox dataBox;
	BoundingBox dataViewProcBox; //current clip box that data are visible (and thus are processable). 091113 PHC

	bool texture_unit0_3D, tex_stream_buffer, drawing_fslice;
	GLenum texture_format, image_format, image_type;
	GLuint tex3D, texFslice;
	GLuint *Ztex_list, *Ytex_list, *Xtex_list;
	RGBA8 *Zslice_data, *Yslice_data, *Xslice_data, *Fslice_data;
	RGBA8 *rgbaBuf_Yzx, *rgbaBuf_Xzy;
	float thicknessX, thicknessY, thicknessZ;
	float sampleScaleX, sampleScaleY, sampleScaleZ;
	int imageX, imageY, imageZ, imageT;
	int safeX, safeY, safeZ;
	int realX, realY, realZ, realF;
	int fillX, fillY, fillZ, fillF;
	GLdouble volumeViewMatrix[16]; // for choosing stack direction
	float VOL_X1, VOL_X0, VOL_Y1, VOL_Y0, VOL_Z1, VOL_Z0;
	int VOLUME_FILTER;
	RGBA32f SLICE_COLOR; // proxy geometry color+alpha
        bool b_renderTextureLast;

private:
	void init_members()
	{
		lastSliceType = vsSliceNone;
		currentMarkerName = -1;
		curEditingNeuron = -1;

		_idep=0;
		isSimulatedData=false;
		data_unitbytes=0;
		data4dp = 0;
		data4d_uint8 = 0;
		dim1=dim2=dim3=dim4=dim5=0;
		start1=start2=start3=start4=start5=0;
		size1=size2=size3=size4=size5=0;

		texture_unit0_3D = tex_stream_buffer = drawing_fslice = false;
		texture_format = image_format = image_type = -1;
		tex3D = texFslice= 0;
		Ztex_list = Ytex_list = Xtex_list = 0;
		Zslice_data = Yslice_data = Xslice_data = Fslice_data = 0;
		rgbaBuf_Yzx = rgbaBuf_Xzy = 0;
		thicknessX = thicknessY = 1; thicknessZ = 1;
		sampleScaleX = sampleScaleY = sampleScaleZ = 1;
		imageX = imageY = imageZ = imageT = 0;
		safeX = safeY = safeZ = 0;
		fillX = fillY = fillZ = fillF = 0;
		realX = realY = realZ = realF = 0;
		VOL_X1 = VOL_Y1 = VOL_Z1 = 1;
		VOL_X0 = VOL_Y0 = VOL_Z0 = 0;
		VOLUME_FILTER = 1;
		SLICE_COLOR = XYZW(1,1,1,1);

		b_imaging = false; //101008
                b_renderTextureLast = false;
	}


// Surface Object //////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	// landmark
	QList <ImageMarker> listMarker;
	// 0-pxUnknown, 1-pxLocaNotUseful, 2-pxLocaUseful, 3-pxLocaUnsure, 4-pxTemp
	#define NTYPE_MARKER  5
	GLuint glistMarker[NTYPE_MARKER];
	RGBA8 marker_color[NTYPE_MARKER];

	// cell apo
	QList <CellAPO> listCell;
	GLuint glistCell;
	QMap <QString, QList<int> > map_APOFile_IndexList;
	QMap <int, QString>         map_CellIndex_APOFile;
	BoundingBox apoBB;

	// neuron swc
	QList <NeuronTree>         listNeuronTree;
	GLuint glistTube, glistTubeEnd;
	BoundingBox swcBB;
	int curEditingNeuron;

	// labelfield surf
	QList <LabelSurf> listLabelSurf;
	QList <Triangle*> list_listTriangle;
	QList <GLuint> list_glistLabel;
	BoundingBox labelBB;

	void createMarker_atom();  					// makeCurrent & called in loadObj
	virtual void drawMarkerList();
	void loadLandmarks_from_file(const QString & filename);
	void saveLandmarks_to_file(const QString & filename);

	void createCell_atom();    					// makeCurrent & called in loadObj
	void saveCellAPO(const QString& filename);
	QList <CellAPO> listFromAPO_file(const QString& filename);
	void loadCellAPO(const QString& filename);
	virtual void drawCellList();

	void createNeuron_tube(); 				// makeCurrent & called in loadObj
	void drawDynamicNeuronTube(float rb, float rt, float length);
	void saveNeuronTree(int kk, const QString& filename); //kk is the cur number of the tree to save
	void loadNeuronTree(const QString& filename);
	void updateNeuronBoundingBox();
	virtual void drawNeuronTree(int i);
	virtual void drawNeuronTreeList();

	void loadLabelfieldSurf(const QString& filename, int ch=0);
	void constructLabelfieldSurf(int mesh_method, int mesh_density);
	void compileLabelfieldSurf(int update=0);  					// makeCurrent
	virtual void drawLabelfieldSurf();
	void cleanLabelfieldSurf();
	void loadWavefrontOBJ(const QString& filename);
	void saveWavefrontOBJ(const QString& filename);
	void loadV3DSurface(const QString& filename);
	void saveV3DSurface(const QString& filename);

        // texture rendering
        static RGBA8* _safe3DBuf;

        static void _safeRelease3DBuf()
        {
                if (_safe3DBuf) delete[] _safe3DBuf;  _safe3DBuf = 0;
        }

        static RGBA8* _safeReference3DBuf(RGBA8* rgbaBuf, int bufX, int bufY, int bufZ,
                         int &safeX, int &safeY, int &safeZ)
        {
                _safeRelease3DBuf();
        //	V3DLONG limitX = MAX_3DTEX_SIZE;
        //	V3DLONG limitY = MAX_3DTEX_SIZE;
        //	V3DLONG limitZ = MAX_3DTEX_SIZE;
                V3DLONG limitX = LIMIT_VOLX;
                V3DLONG limitY = LIMIT_VOLY;
                V3DLONG limitZ = LIMIT_VOLZ;
                V3DLONG fillX = power_of_two_ceil(bufX);
                V3DLONG fillY = power_of_two_ceil(bufY);
                V3DLONG fillZ = power_of_two_ceil(bufZ);
                if (fillX<=limitX && fillY<=limitY && fillZ<=limitZ)
                {
                        safeX = bufX;
                        safeY = bufY;
                        safeZ = bufZ;
                        return rgbaBuf;
                }

                float sx, sy, sz;
                V3DLONG dx, dy, dz;
                sx = float(bufX)/MIN(limitX, bufX);
                sy = float(bufY)/MIN(limitY, bufY);
                sz = float(bufZ)/MIN(limitZ, bufZ);
                dx = V3DLONG(sx);
                dy = V3DLONG(sy);
                dz = V3DLONG(sz);
                MESSAGE_ASSERT(dx*dy*dz >=1); // down sampling
                limitX = V3DLONG(bufX/sx);
                limitY = V3DLONG(bufY/sy);
                limitZ = V3DLONG(bufZ/sz);
                safeX = limitX;
                safeY = limitY;
                safeZ = limitZ;
                //qDebug("	safe = %dx%dx%d", limitX, limitY, limitZ);

                _safe3DBuf = new RGBA8[safeX*safeY*safeZ];
                //memset(_safe3DBuf, 0, sizeof(RGBA8)*safeX*safeY*safeZ);
                V3DLONG ox, oy, oz;
                V3DLONG ix, iy, iz;
                for (oz = 0; oz < safeZ; oz++)
                        for (oy = 0; oy < safeY; oy++)
                                for (ox = 0; ox < safeX; ox++)
                                {
                                        ix = CLAMP(0,bufX-1, IROUND(ox*sx));
                                        iy = CLAMP(0,bufY-1, IROUND(oy*sy));
                                        iz = CLAMP(0,bufZ-1, IROUND(oz*sz));

                                        RGBA32f rgbaf;
                                        RGBA8 rgba8;
                                        rgbaf = sampling3dRGBA8( rgbaBuf, bufX, bufY, bufZ, ix, iy, iz, dx, dy, dz);
                                        rgba8.r = (unsigned char)rgbaf.r;
                                        rgba8.g = (unsigned char)rgbaf.g;
                                        rgba8.b = (unsigned char)rgbaf.b;
                                        rgba8.a = (unsigned char)rgbaf.a;

                                        _safe3DBuf[ oz*(safeY*safeX) + oy*(safeX) + ox] = rgba8;
                                }
                return _safe3DBuf;
        }
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper operators
#define BUFFER_NPT 0
#if 0
#define COPY_X fillX
#define COPY_Y fillY
#define COPY_Z fillZ
#else
#define COPY_X realX
#define COPY_Y realY
#define COPY_Z realZ
#endif

void _copySliceFromStack(RGBA8* rgbaBuf, int imageX, int imageY, int imageZ,
						RGBA8* slice, int copyW, int stack_i, int slice_i,
						RGBA8* rgbaYzx=0, RGBA8* rgbaXzy=0);
void _copyXzyFromZyx(RGBA8* rgbaXzy, RGBA8* rgbaZyx, int imageX, int imageY, int imageZ);
void _copyYzxFromZyx(RGBA8* rgbaYzx, RGBA8* rgbaZyx, int imageX, int imageY, int imageZ);


#define HIGHLIGHT_ON()   {\
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT); \
	glBlendColorEXT(1, 1, 1, 1); glEnable(GL_DEPTH_TEST); glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); \
	beginHighlight();\
	}
#define HIGHLIGHT_OFF()  {\
	endHighlight();\
	glPopAttrib();\
	}

inline QColor QColorFromRGBA8(RGBA8 c)
{
	return QColor(c.c[0], c.c[1], c.c[2], c.c[3]);
}

inline RGBA8 RGBA8FromQColor(QColor qcolor)
{
	RGBA8 c;
	RGBA32i ic;
	qcolor.getRgb(&ic.c[0], &ic.c[1], &ic.c[2], &ic.c[3]);
	c.r=(unsigned char)ic.r; c.g=(unsigned char)ic.g; c.b=(unsigned char)ic.b; c.a=(unsigned char)ic.a;
	return c;
}

#endif
