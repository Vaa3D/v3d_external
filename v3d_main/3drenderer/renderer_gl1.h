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

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets, Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model, Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/*
 *  renderer_gl1.h
 *
 *  implementation in renderer_tex.cpp	(volume rendering)
 * 			&	renderer_obj.cpp 		(surface object rendering)
 *  		& 	renderer_labelfield.cpp (extracting iso-surface from label-field)
 *  		&	renderer_hit.cpp 		(object selection and the functions of object's context menu)
 *
 *
 *  Created by Ruan Zongcai on 8/6/08.
 *  last update: 090117 by Hanchuan Peng. add memory neuron swc passing interface
 *  last update: 090204 by Hanchuan Peng add the findNearestNeuronNode_WinXY()
 *  last update: 090219: add "name" and "comment" fields to all the basic object structs
 *  last edit by Hanchuan Peng, 090306, separate some basic obj types and also neuron manipulation functions
 *  last edit: Hancuan peng and Ruan Zongcai, 090705, add a flag to enable/disable the clipBox which is used in the function drawObj()
 *  last change: 110813 Ruan Zongcai, change Renderer_tex2 to Renderer_gl1, also changed related file names
 * last change: 20120717: add b_grabhighrez for large data file viz. by PHC
 */

#ifndef V3D_RENDERER_TEX2_H
#define V3D_RENDERER_TEX2_H

#include "renderer.h"
#include "marchingcubes.h"
#include <time.h>
#include <map>
#include <set>
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
                        stNeuronSegment=3,
                        stNeuronStructure=4,
                        stPointCloud=5,
                        stPointSet=6,
                        };

//////////////////////////////////////////////////////////////////////////////////////////

//110813 RZC, change Renderer_tex2 to Renderer_gl1
class Renderer_gl1 : public Renderer
{
    friend class V3dr_surfaceDialog; //for accessing all surface data structure
    friend class V3dr_colormapDialog;

public:
    Renderer_gl1(void* widget);
    virtual ~Renderer_gl1();
    virtual const int class_version() {return 1;}

public:
    XYZ ImageCurRes;
    XYZ ImageStartPoint;
    XYZ ImageMaxRes;

    //Coordinate transform
    XYZ ConvertGlobaltoLocalBlockCroods(double x,double y,double z);
    XYZ ConvertLocalBlocktoGlobalCroods(double x,double y,double z);
    XYZ ConvertMaxRes2CurrResCoords(double x,double y,double z);
    XYZ ConvertCurrRes2MaxResCoords(double x,double y,double z);

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

    virtual int hitWheel(int x, int y); //by PHC, 130424.

    virtual void blendTrack();       // called by paint()
    virtual void setRenderTextureLast(bool renderTextureLast);

    // link to Rendering function
    //protected:
    virtual int processHit(int namelen, int names[], int x, int y, bool b_menu, char* pTip=0);	// called by selectObj. the x and y parameters by Hanchuan Peng,090204

    virtual void loadObj();  	// called by initialize()  	// makeCurrent
    virtual void cleanObj(); 	// called by ~Renderer_gl1	// makeCurrent
    virtual void drawObj();  	// called by paint()

    virtual void loadObj_meshChange(int new_mesh); // maybe naming neuronTube_meshChange is better

    virtual void loadVol();  	// called by initialize()  	// makeCurrent
    virtual void cleanVol();	// called by ~Renderer_gl1	// makeCurrent
    virtual void drawVol(); 	// called by paint()
    virtual void prepareVol();
    virtual void renderVol();

    virtual void subloadTex(V3DLONG timepoint, bool bfisrt=false);	// called by loadVol, drawVol
    virtual void equAlphaBlendingProjection();			// for blending equation
    virtual void equMaxIntensityProjection();	// for blending equation
    virtual void equMinIntensityProjection();	// for blending equation
    virtual void equCrossSection();				// for blending equation
    virtual void shaderTexBegin(bool stream) 	{};		// for texture2D/3D shader
    virtual void shaderTexEnd()	 				{};		// for texture2D/3D shader

    virtual void updateVolCutRange();
    virtual void updateBoundingBox();
    virtual void updateThicknessBox();

    // Rendering execution function
    //protected:
    virtual void setMarkerSpace();
    virtual void drawMarker();  // called by paint(), not by drawObj()
    virtual void MarkerSpaceToNormalizeSpace(XYZ & p);

    virtual void setSurfaceStretchSpace();
    virtual void setObjLighting();
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
    virtual void _streamTex(int stack_i, int slice_i, int step, int slice0, int slice1) {}; //091003
    virtual void _streamTex_end()													 	{}; //091003
    virtual bool _streamTex_ready()									{return false;} //091015

    virtual void setupFrontSliceBuffer();
    virtual void drawUnitFrontSlice(int line=0);

    virtual void drawBackFillVolCube();
    virtual void drawCrossLine(float lineWidth=1);

    // public View control interface
    //public:
    virtual void setRenderMode(RenderMode);
    virtual void setThickness(double t);
    virtual void toggleTexFilter();
    virtual void toggleTex2D3D();
    virtual void toggleTexCompression();

    virtual void togglePolygonMode() {polygonMode = (polygonMode +1) %4;} // FILL,LINE,POINT, transparent
    virtual void toggleLineType();

#ifndef test_main_cpp //140211
    virtual void toggleNStrokeCurveDrawing(); // For n-right-strokes curve shortcut ZJL 110920
    virtual void setDragWinSize(int csize); // set rubber drag win size, ZJL 110921
    virtual void _updateDragPoints(int x, int y); // For rubber editing. ZJL 110920

#endif

    //virtual void updateVolShadingOption();

    virtual void createSurfCurrent(int ch=0);                 // updateBoundingBox
    virtual void loadObjectFilename(const QString& filename); // updateBoundingBox // makeCurrent
    virtual void loadObjectListFromFile();             // call loadObjectFilename
    virtual void loadObjectFromFile(const char* url);  // call loadObjectFilename
    virtual void saveSurfFile();

    void loadV3DSFile(const QString& filename);

    virtual void endSelectMode();
    virtual void updateLandmark();
    virtual void updateTracedNeuron();

    virtual int zoomview_wheel_event();//by PHC, 20130424
    virtual int zoomview_currentviewport();//by PHC, 20130701

    //added a number of shortcuts for whole mouse brain data tracing, by ZZ, 20212018
    virtual void callStrokeCurveDrawingBBoxes(); // call serial BBoxes curve drawing
    virtual void callStrokeRetypeMultiNeurons();//  call multiple segments retyping
    virtual void callStrokeDeleteMultiNeurons();//  call multiple segments deleting
    virtual void callStrokeSplitMultiNeurons();//  call multiple segments spliting
    virtual void callStrokeConnectMultiNeurons();//  call multiple segments connection
    virtual void callShowSubtree(); // highlight the selected segment and its downstream subtree. MK, June, 2018
    virtual void callShowBreakPoints();
    virtual void rc_findConnectedSegs_continue(My4DImage* curImg, size_t inputSegID);
    virtual void callShowConnectedSegs();
    virtual void callStrokeCurveDrawingGlobal(); // call Global optimal curve drawing
    virtual void callDefine3DPolyline(); // call 3D polyline defining
    virtual void callCreateMarkerNearestNode(int x, int y); // call creating marker
    virtual void callCreateSpecialMarkerNearestNode(int x,int y); //add special marker, by XZ, 20190720
    virtual void callGDTracing();

    virtual void toggleEditMode();
    virtual void setEditMode();


    // process Object hit ///////////////////////////////////////////////////////////////////////////////////////
public:

#ifndef test_main_cpp

    virtual void editSurfaceObjAnnotation(int dataClass, int surfaceType, int index);
    virtual void editSurfaceObjBasicGeometry(int dataClass, int surfaceType, int index);

    void updateNeuronTree(V_NeuronSWC & neuron_seg); //the interface to get a real-time generated swc tree
    V_NeuronSWC_list copyToEditableNeuron(NeuronTree * ptree);
    void finishEditingNeuronTree();

#endif

    static bool rightClickMenuDisabled;
    QString info_Marker(int marker_i);
    QString info_NeuronNode(int node_i, NeuronTree * ptree);
    QString info_SurfVertex(int vertex_i, Triangle * triangle, int label);

    QList <NeuronTree> * getHandleNeuronTrees() {return &listNeuronTree;}
    V3DLONG findNearestNeuronNode_WinXY(int cx, int cy, NeuronTree * ptree, double & best_dist);	//find the nearest node in a neuron in XY project of the display window.//return the index of the respective neuron node

    LandmarkList * getHandleLandmark(); //20141016, by Hanbo Chen
    void setHandleLandmark(LandmarkList & landmark_list);

    QList <CellAPO> *getHandleAPOCellList() {return &listCell;}

    Triangle * findNearestSurfTriangle_WinXY(int cx, int cy, int & vertex_i, Triangle * plist);

    double computeSurfaceArea(int dataClass, int surfaceType, int index);
    double computeSurfaceVolume(int dataClass, int surfaceType, int index);
    void showLineProfile(int marker1, int marker2);
    QVector<int> getLineProfile(XYZ p1, XYZ p2, int chno=0);
    void setDeleteKey(int key);

#ifndef test_main_cpp

    XYZ selectPosition(int x, int y); 	// neuron selector
    int hitMenu(int x, int y, bool b_glwidget); // neuron annotator right click popup menu

    QList <LabelSurf> getListLabelSurf();
    void setListLabelSurf(QList <LabelSurf> listLabelSurfinput);
#endif


    // define Marker/Curve  ///////////////////////////////////////////////
    //protected:
    GLint viewport[4];
    GLdouble projectionMatrix[16];
    GLdouble markerViewMatrix[16];
    struct MarkerPos {
        double x, y;		// input point coordinates
        int view[4];        // view-port
        double P[16];		// 4x4 projection matrix
        double MV[16];		// 4x4 model-view matrix
        bool drawn;			// has this marker already been drawn to the screen?
    };
    QList <MarkerPos> listMarkerPos; //081221, screen projection position
    QList< QList <MarkerPos> > list_listCurvePos; //screen projection position list for curve
    MarkerPos wheelPos; //130424, by PHC. a one time record of the last wheel/cursor location

    QList <LocationSimple> listCurveMarkerPool; // used for curve drawing from marker pool

    // computation
    void _MarkerPos_to_NearFarPoint(const MarkerPos & pos, XYZ &loc0, XYZ &loc1);
    double distanceOfMarkerPos(const MarkerPos & pos0, const MarkerPos & pos);
    XYZ getLocationOfListMarkerPos();
    XYZ getTranslateOfMarkerPos(const MarkerPos &pos, const ImageMarker &S);
    XYZ getPointOnPlane(XYZ P1, XYZ P2, double plane[4]);
    XYZ getPointOnSections(XYZ P1, XYZ P2, double F_plane[4]=0);
    XYZ getCenterOfLineProfile(XYZ p1, XYZ p2,
                               double clipplane[4]=0,	//clipplane==0 means no clip plane
                               int chno=0,    			//must be a valid channel number
                               float *value=0			//if value!=0, output value at center
                               );
    XYZ getCenterOfLocal(XYZ loc);

    int getVolumeXsectPosOfMarkerLine(XYZ & locA, XYZ & locB, const MarkerPos& pos, int defaultChanno=-1);//by PHC 20130425
    int getVolumeXsectPosOfMarkerLine(XYZ P1, XYZ P2,
                                      double clipplane[4],	//clipplane==0 means no clip plane
                                      int chno,    			//must be a valid channel number
                                      XYZ & posCloseToP1, XYZ & posCloseToP2 //output
                                      ); //by PHC 20130425

    int checkCurChannel();
    bool isInBound(const XYZ & loc, float factor=0.001, bool b_message=true);

    // marker
    QCursor oldCursor;
    int lastSliceType; //for cross-section
    int currentMarkerName;
    XYZ getCenterOfMarkerPos(const MarkerPos& pos, int defaultChanno=-1);
    double solveMarkerCenter();
    double solveMarkerCenterMaxIntensity();
    void solveMarkerViews();
    void refineMarkerTranslate();
    void refineMarkerCenter();
    void refineMarkerLocal(int marker_id);

    void addMarker(XYZ &loc,bool fromserver=0);
    void addMarkerUnique(XYZ &loc);
    void addSpecialMarker(XYZ &loc); //add special marker, by XZ, 20190720
    void updateMarkerLocation(int marker_id, XYZ &loc); //PHC, 090120

    // curve
    int cntCur3DCurveMarkers; //091226. marker cnt when define a curve using marker clicking
    bool b_addthiscurve; //for 1-stroke curve based zoom-in, PHC 100821
    bool b_addthismarker; //for 1-click based zoom-in, PHC 100821
    bool b_grabhighrez; //for v3d_largedataviz, PHC 120717
    bool b_imaging; //for v3d_imaging, PHC 101008
    bool b_ablation; //for 3D imaging, PHC, 120506
    bool b_lineAblation; // for line cutting ablation, ZJL, 20120801
    void solveCurveCenter(vector <XYZ> & loc_vec_input);
    void solveCurveViews();
    void solveCurveFromMarkers();

    // beginning ZMS 2016125
    QHash<V3DLONG, V3DLONG> segmentLengthDict;
    QHash<V3DLONG, V3DLONG> segmentParentDict;
    QHash<V3DLONG, V3DLONG> segmentLevelDict;
    QList<V3DLONG> loopSegs; // a list of segments involved in a loop
    QList<V3DLONG> debugSegs; // a list of segments in debug mode

    QList<V3DLONG> childSegs; // a list of child segments (for a TBD node)

    QHash<V3DLONG, DoublyLinkedNeuronsList*> dict_dlnh; //  A list of segments, hases seg_id  to doubly-linked segments
    //static const GLubyte neuron_type_color[275 ][3] ;
    //static const
    //int neuron_type_color_num;// = sizeof(neuron_type_color)/(sizeof(GLubyte)*3);

    void updateMarkerList(QList <ImageMarker> markers, int i); // sync markers with object_manager

    void initColorMaps();
    bool colorByAncestry;
    bool colorByTypeOnlyMode; //This is only checked if colorByAncestry is enabled
    bool setColorAncestryInfo();

    void addToListOfLoopingSegs(V3DLONG firstParent, V3DLONG secondParent, V3DLONG violationSeg);
    void setColorByAncestry(NeuronSWC s, time_t seconds); // colorByAncestry mode
    // end ZMS

    void addToListOfChildSegs(V3DLONG segID); // add this segment and all of its children to the list of child segments

    bool cuttingZ;
    void setBBZcutFlag(bool cuttingZ);
    void updateNeuronBoundingBoxWithZCut(float zMin, float zMax);
    void setBBZ(float zMinIn, float zMaxIn);
    float zMin, zMax;

    bool cuttingXYZ;
    void setBBcutFlag(bool cuttingXYZ);
    void updateNeuronBoundingBoxWithXYZCut(float xMin, float xMax,float yMin, float yMax, float zMin, float zMax);
    void setBBXYZ(float xMinIn, float xMaxIn,float yMinIn, float yMaxIn,float zMinIn, float zMaxIn);
    float xMin, xMax, yMin, yMax;


    void setNeuronColor(NeuronSWC s, time_t seconds);  // method to set different color modes.

    void setUserColor(int userId);
    int userColorid;
    // this will call setColorByAncestry if needed.
    void setNeuronReviewColors(NeuronSWC s); // review mode
    void setHighlightColors(NeuronSWC s); // highlight only the children of a selected node
    void setBasicNeuronColors(NeuronSWC s);
    void setConfidenceLevelColors(NeuronSWC s); //confidence level mode by ZZ 06192018
    bool childHighlightMode;
    int deleteKey; // 1: key_i; 2: key_t



// beginning of ZJL
#ifndef test_main_cpp //140211
    void solveCurveFromMarkersFastMarching(); //using fast marching method
#endif
    void getPerpendPointDist(XYZ &P, XYZ &P0, XYZ &P1, XYZ &Pb, double &dist);
    void getRgnPropertyAt(XYZ &pos, LocationSimple &pt);

    void solveCurveCenterV2(vector <XYZ> & loc_vec_input, vector <XYZ> &loc_vec, int index);
    void solveCurveRefineLast();
    void solveCurveExtendGlobal(); //extends the closest seg. By ZMS for neuron game 20151106
    void reorderNeuronIndexNumber(V3DLONG curSeg_id, V3DLONG NI, bool newInLower);
    void blendRubberNeuron();
    void solveCurveRubberDrag();

#ifndef test_main_cpp //140211
    void blendDraggedNeuron();
#endif

    void adaptiveCurveResampling(vector <XYZ> &loc_vec, vector <XYZ> &loc_vec_resampled, int stepsize);
    void adaptiveCurveResamplingRamerDouglasPeucker(vector <XYZ> &loc_vec, vector <XYZ> &loc_vec_resampled, float epsilon);
    void recursiveRamerDouglasPeucker(vector <XYZ> &loc_vec, vector <XYZ> &loc_vec_resampled, int start_i, int end_i, float epsilon);
    //void resampleCurveStrokes(QList <MarkerPos> &listCurvePos, int chno, vector<int> &ids);
    void resampleCurveStrokes2(QList <MarkerPos> &listCurvePos, int chno, vector<int> &ids);

    void resampleCurveStrokes(int index, int chno, vector<int> &ids);


    bool boundingboxFromStroke(XYZ& minloc, XYZ& maxloc);
    //Using boundingboxFromStroke() get boundingbox and then get subvol
    void getSubVolFromStroke(double* &pSubdata, int chno, XYZ &sub_orig, V3DLONG &sub_szx, V3DLONG &sub_szy, V3DLONG &sub_szz);
    void getSubVolFrom2MarkerPos(vector<MarkerPos> & pos, int chno, double* &pSubdata, XYZ &sub_orig, XYZ &max_loc, V3DLONG &sub_szx,
                                 V3DLONG &sub_szy, V3DLONG &sub_szz);
    void getSubVolFrom3Points(XYZ & loc0_last, XYZ & loc0, XYZ & loc1, int chno, double* &pSubdata,
                              XYZ &sub_orig, V3DLONG &sub_szx, V3DLONG &sub_szy, V3DLONG &sub_szz);

    bool pickSeedpointFromExistingCurves(const MarkerPos &pos, XYZ &nearest_loc); //change to this name, by PHC, 20120330

#ifndef test_main_cpp //140211
    void vecToNeuronTree(NeuronTree &SS, vector<XYZ> loc_list);
#endif
    void getMidRandomLoc(MarkerPos pos, int chno, XYZ &mid_loc);
    double distance_between_2lines(NeuronTree &line1, NeuronTree &line2);

    V3DLONG findNearestNeuronNode_Loc(XYZ &loc, NeuronTree *ptree);

    V3DLONG findNearestMarker_Loc(XYZ &loc, QList <LocationSimple> &listlandmarks,int mode); //add the function to deal with find nearestmarker, by XZ, 20190720

    bool lineLineIntersect( XYZ p1,XYZ p2,XYZ p3,XYZ p4,XYZ *pa,XYZ *pb,
                           double *mua, double *mub);
    void getSubVolInfo(XYZ lastloc, XYZ loc0, XYZ loc1, XYZ &sub_orig, double* &pSubdata,
                       V3DLONG &sub_szx, V3DLONG &sub_szy, V3DLONG &sub_szz);

    void solveCurveDirectionInter(vector <XYZ> & loc_vec_input, vector <XYZ> &loc_vec, int index);
    double solveCurveMarkerLists_fm(vector <XYZ> & loc_vec_input, vector <XYZ> &loc_vec, int index);

    // @ADDED by Alessandro on 2015-05-07. Multiple neuron segments delete by one-mouse stroke.
    void deleteMultiNeuronsByStroke();

    // ------ Segment/points could/marker connecting/cutting tool, by MK 2017 April ------------
    struct segInfoUnit
    {
        segInfoUnit() { hierarchy = 0; }
        long segID;
        long head_tail;
        long nodeCount;
        bool refine;

        int branchID, paBranchID;
        int hierarchy;
    };

    NeuronTree treeOnTheFly;
    bool isLoadFromFile;
    bool hierarchyRelabel;

    void simpleConnect();
    bool simpleConnectExecutor(My4DImage* curImg, vector<segInfoUnit>& segInfo);
    void showSubtree();
    void showConnectedSegs();
    void sort_tracedNeuron(My4DImage* curImg, size_t rootID);  // Sort swc, Peng Xie, June 2019

    bool fragmentTrace; // Fragment tracing mode switch, MK, Dec 2018
    map<string, float> fragTraceParams;
    void connectSameTypeSegs(map<int, vector<int> >& inputSegMap, My4DImage*& curImgPtr);

    void connectNeuronsByStroke();
    void connectPointCloudByStroke();
    void connectMarkerByStroke();

    void segmentStraighten(vector<V_NeuronSWC_unit>& inputSeg, My4DImage*& curImgPtr, vector<segInfoUnit>::iterator& refineIt);
    void cutNeuronsByStroke();

    // --------- loop safe guard and hilighting [subtree/connected segs] for both 3D view and terafly editing mode, MK 2018 May ---------
    int gridLength;
    map<string, set<size_t> > wholeGrid2segIDmap;
    multimap<string, size_t> segEnd2segIDmap;

    set<size_t> subtreeSegs;
    map<size_t, vector<V_NeuronSWC_unit> > originalSegMap;
    map<size_t, vector<V_NeuronSWC_unit> > highlightedSegMap;

    void segEnd2SegIDmapping(My4DImage* curImg);
    void seg2GridMapping(My4DImage* curImg);
    void rc_findConnectedSegs(My4DImage* curImg, size_t startSegID);
    set<size_t> segEndRegionCheck(My4DImage* curImg, size_t inputSegID);
    bool pressedShowSubTree;
    void escPressed_subtree();

    set<vector<size_t> > detectedLoops;
    set<set<size_t> > detectedLoopsSet;
    set<set<size_t> > finalizedLoopsSet;
    set<set<size_t> > nonLoopErrors;
    map<size_t, set<size_t> > seg2SegsMap;
    map<size_t, set<size_t> > segTail2segIDmap;
    void loopDetection();
    void rc_loopPathCheck(size_t inputSegID, vector<size_t> curPathWalk, My4DImage* curImg);


    bool isTera;
    map<size_t, size_t> branchSegIDmap;
    map<string, size_t> tail2segIDmap;
    multimap<size_t, string> segID2gridMap;

    multimap<string, size_t> grid2segIDmap;
    multimap<string, size_t> head2segIDmap;
    multimap<string, size_t> tail2SegIDmap;

    void hierarchyReprofile(My4DImage* curImg, long mainSegID, long branchSegID);
    void rc_downstreamRelabel(My4DImage* curImg, size_t curStemSegID);
    void upstreamRelabel(My4DImage* curImg, V_NeuronSWC* startingSegPtr, V_NeuronSWC* newPaSegPtr);
    void rc_downstreamSeg(My4DImage* curImg, size_t segID);
    void rc_downstream_segID(My4DImage* curImg, size_t segID);
    void segTreeFastReprofile(My4DImage* curImg);
    void rc_findDownstreamSegs(My4DImage* curImg, size_t inputSegID, string gridKey, int gridLength);
    // -----------------------------------------------------------------------------------------------------------------------------------

    // @ADDED by Alessandro on 2015-05-23. Called when "Esc" key is pressed and tracedNeuron must be updated.
    void deleteMultiNeuronsByStrokeCommit();
    bool deleteMultiNeuronsByStrokeCommit(vector <XYZ> local_list,float mindist);
    // @ADDED by Alessandro on 2015-09-30. Select multiple markers by one-mouse stroke.
    void selectMultiMarkersByStroke();

    void retypeMultiNeuronsByStroke();

    // forceSingleCut @ADDED T Pavlik 20151217, split was splitting more segments than desired so best cut option added
    void breakMultiNeuronsByStrokeCommit();
    void breakMultiNeuronsByStroke(); //20170731 RZC: no args for master implementation //(bool forceSingleCut=false);
    void breakTwoNeuronsByStroke();   //20170731 RZC: make a separate function for mozak to prevent confusion and interference

#ifndef test_main_cpp //140211
    void solveCurveFromMarkersGD(bool b_customized_bb);
#endif

    bool withinLineSegCheck( XYZ p1,XYZ p2,XYZ pa); // check wether pa is within the line seg (p1,p1)
    XYZ getLocUsingMassCenter(bool firstloc, XYZ lastpos, XYZ p1, XYZ p2,
                              double clipplane[4]=0,	//clipplane==0 means no clip plane
                              int chno=0,    			//must be a valid channel number
                              float *value=0			//if value!=0, output value at center
                              );

    // for curve testing
    void swcBoundingBox(NeuronTree &line, XYZ &minloc, XYZ &maxloc);
    void MIP_XY_YZ_XZ(unsigned char * &pXY, unsigned char * &pYZ, unsigned char * &pXZ, XYZ &minloc, XYZ &maxloc);
    void projectSWC_XY_YZ_XZ(unsigned char * &pXY, unsigned char * &pYZ, unsigned char * &pXZ, XYZ &minloc, XYZ &maxloc, NeuronTree &line, unsigned char color[3]);
    void createLastTestID(QString &curFilePath, QString &curSuffix, int &test_id);

    void updateDraggedNeuronXYZ();
    V3DLONG findNearestNeuronNode_WinXYV2(int cx, int cy);
    void canCurveConnect(XYZ &e, V3DLONG &closest_seg, V3DLONG &closest_node, bool &bConnect);
    void connectCurve(V3DLONG &curSeg);
    void smoothLagrange(vector <XYZ> inPoints, vector <XYZ> & outPoints, int numberOfSegments);
    V3DLONG edit_seg_id;
    int nDragWinSize;
    bool bInitDragPoints;

    // for curve testing
    bool bTestCurveBegin;

    NeuronTree testNeuronTree;

    // END of ZJL

    // intersect helpers for nstroke_tracing
    template <int N>
    struct IntersectResult {
        bool success[N];
        XYZ hit_locs[N];
    };

    struct NearFarPoints {
        bool valid;
        XYZ near_pt;
        XYZ far_pt;
    };

    IntersectResult<1> directedIntersectPoint(const XYZ &loc0, const XYZ &loc1) const
    {
        IntersectResult<1> result;
        result.success[0] = false;

        if (dataViewProcBox.isInner(loc0, 0))
        {
            result.success[0] = true;
            result.hit_locs[0] = loc0;
        }
        else
        {
            XYZ v_1_0 = loc1 - loc0;
            XYZ D = v_1_0; normalize(D);
            float length = dist_L2(loc0, loc1);

            for (int ii = 0; ii < length; ii++)
            {
                XYZ loci = loc0 + D * ii;
                if (dataViewProcBox.isInner(loci, 0))
                {
                    result.success[0] = true;
                    result.hit_locs[0] = loci;
                    break;
                }
            }
        }

        return result;
    }

    IntersectResult<2> intersectPointsWithData(const XYZ &loc0_t, const XYZ &loc1_t) const
    {
        IntersectResult<2> result;

        {
            IntersectResult<1> result0 = directedIntersectPoint(loc0_t, loc1_t);
            result.success[0] = result0.success[0];
            result.hit_locs[0] = result0.hit_locs[0];
        }

        {
            IntersectResult<1> result1 = directedIntersectPoint(loc1_t, loc0_t);
            result.success[1] = result1.success[0];
            result.hit_locs[1] = result1.hit_locs[0];
        }

        return result;
    }

    NearFarPoints markerPosToNearFarLocs(int curveIndex, int pointIndex)
    {
        NearFarPoints result;

        const MarkerPos &pos = list_listCurvePos.at(curveIndex).at(pointIndex);
        double clipplane[4] = { 0.0, 0.0, -1.0, viewClip };

        ViewPlaneToModel(pos.MV, clipplane); // modifies clipplane

        XYZ loc0_t, loc1_t;

        _MarkerPos_to_NearFarPoint(pos, loc0_t, loc1_t);

        IntersectResult<2> intersect = intersectPointsWithData(loc0_t, loc1_t);

        result.valid = intersect.success[0] && intersect.success[1];
        result.near_pt = intersect.hit_locs[0];
        result.far_pt = intersect.hit_locs[1];

        return result;
    }

    // in renderer_obj.cpp
    void addCurveSWC(vector<XYZ> &loc_list, int chno=0, double creatmode=0,bool fromserver = false); //if no chno is specified, then assume to be the first channel //LMG 26/10/2018 if no creatmode specified set to 0
    void addCurveSWC(vector<XYZ> &loc_list, int chno, double creatmode,int type);
    //for local view
    bool produceZoomViewOf3DRoi(vector <XYZ> & loc_vec, int ops_type=0);
    void ablate3DLocationSeries(vector <XYZ> & loc_vec); //added by PHC, 120506


    // Volume Parameters /////////////////////////////////////////////
    //protected:
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
    double currentTraceType;
    bool useCurrentTraceTypeForRetyping;

    RGBA8 currentMarkerColor;//added by ZZ 05142018

    float zThick;

private:
    void init_members()
    {
        lastSliceType = vsSliceNone;
        currentMarkerName = -1;
        curEditingNeuron = -1;
        realCurEditingNeuron_inNeuronTree = -1;

        highlightedNode = -1; //Added by ZMS 20151203 highlight initial node we are going to extend.
        highlightedEndNode = -1; //Added by ZMS 20151203 highlight final node we are going to extend.
        selectedStartNode = -1;
        highlightedEndNodeChanged = false;
        rotateAxisBeginNode = XYZ(0, 0, 1);
        rotateAxisEndNode = XYZ(0, 0, 0);

        childHighlightMode = false;
        showingGrid = false;
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

        b_grabhighrez = false; //120717, PHC
        b_imaging = false; //101008
        b_ablation = false; //120506
        b_lineAblation = false;
        b_renderTextureLast = false;
        edit_seg_id = -1; // ZJL 110913
        draggedCenterIndex = -1; // ZJL 110921
        nDragWinSize = 3; // better be odd, ZJL 110921
        bInitDragPoints = false;
        // for curve testing
        bTestCurveBegin=false;

        b_editDroppedNeuron = false; //20150527, PHC

        highlightedNodeType = -1; //20170804 RZC
        currentTraceType= 3;
        useCurrentTraceTypeForRetyping = false;
        cuttingZ = false;
        cuttingXYZ = false;
        zMin =-1.0;
        zMax = 1.0;
        initColorMaps();
        gridSpacing = 10.0;
        currentMarkerColor.r=0;
        currentMarkerColor.g=20;
        currentMarkerColor.b=200;
        deleteKey = 0;
    }


    // Surface Object //////////////////////////////////////////////////////////////////////////////////////////////////
public:
    //protected:
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
    QList <NeuronTree>         listNeuronTree_old;
    bool b_editDroppedNeuron;

    GLuint glistTube, glistTubeEnd;
    BoundingBox swcBB;
    int curEditingNeuron;
    int realCurEditingNeuron_inNeuronTree;

    // dragged neuron
    // the neuron is copied from original and pos is changed
    QList <V_NeuronSWC_unit> DraggedNeurons; // ZJL 110921
    V3DLONG draggedCenterIndex; // ZJL 110921

    //overlay grid
    QList <ImageMarker> gridList;
    QList<long> gridIndexList;
    float gridSpacing;
    bool showingGrid;


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
    virtual void drawGrid();

    void setLocalGrid(QList<ImageMarker> inputGridList,QList<long> inputGridNumber, float gridside);
    QList<ImageMarker> getLocalGrid();
    void retypeMultiNeuronsbyshortcut();

    int highlightedNode; //Added by ZMS 20151203 highlight initial node we are going to extend.
    int selectedStartNode; // TDP 20160203 for selecting start node for joining two nodes
    int highlightedNodeType; //Added by ZMS 20151203 highlight initial node type we are going to extend.
    V3DLONG highlightedEndNode; //Added by ZMS 20151203 highlight final node we are going to extend.
    bool highlightedEndNodeChanged;
    XYZ rotateAxisBeginNode; //Added by ZMS 20160209 for wriggle feature. The first node of the last-drawn segment.
    XYZ rotateAxisEndNode; //Added by ZMS 20160209 for wriggle feature. The final node of the last-drawn segment.


    V3DLONG highlightedStartNode; // this is for highlighting all children of selected node

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


#endif


