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
Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,鈥� Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )
Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model, Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )
3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.
*************/
/*
 * renderer_hit.cpp
 * Last update: 20090220, Hanchuan Peng. add editing surface object func
 * Last update: 20090310, Hanchuan Peng. add neuron average distance score
 * Last update: 20090421, Zongcai Ruan. move code of object hit processing form renderer_tex2.cpp
 * Last update: 20090618: Hanchuan Peng. add actMarkerZoomin3D for new local zoom-in 3D viewer
 * Last update: 20100211: Hanchuan Peng. add neuron seg merge closeby menuitem
 * Last update: 20100404: Hanchuan Peng. add the new merge closeby method and disable the old merge one branch menu
 * Last update: 20100820: Hanchuan Peng. add a 3d curve and zoom function
 * Last update: 20101008: Hanchuan Peng. add support to v3d_imaging
 * Last update: 20101105: Hanchuan Peng. add more surgical operations
 *
 *  Copyright Hanchuan Peng. All rights reserved.
 *
 */

#include "GLee2glew.h" ////2020-2-10

#include <QAction>
#include "renderer_gl1.h"
#include "v3dr_glwidget.h"
#include "barFigureDialog.h"
#include <fstream>
#include <sstream>
#include <QElapsedTimer>
#ifndef test_main_cpp
#include "../v3d/v3d_compile_constraints.h"
#include "../v3d/landmark_property_dialog.h"
#include "../v3d/surfaceobj_annotation_dialog.h"
#include "../v3d/surfaceobj_geometry_dialog.h"
#include "../neuron_editing/neuron_sim_scores.h"
#include "../neuron_tracing/neuron_tracing.h"
#include "../imaging/v3d_imaging.h"
#include "../basic_c_fun/v3d_curvetracepara.h"
#include "../neuron_toolbox/vaa3d_neurontoolbox.h"
#include "../terafly/src/control/CImport.h"
#include "v3d_application.h"

#endif //test_main_cpp

bool Renderer_gl1::rightClickMenuDisabled = false; //added by TDP 201602 - way of using buttons/keystrokes rather than right click menu to select render modes
double total_etime; //added by PHC, 20120412, as a convenient way to know the total elipsed time for a lengthy operation
//#define _IMAGING_MENU_
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Select Object / Define marker
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LIST_SELECTED(list, i, s) \
{ \
    if (i>=0 && i<list.size()) \
        list[i].selected = s; \
}
#define LIST_ON(list, i, s) \
{ \
    if (i>=0 && i<list.size()) \
        list[i].on = s; \
}
#define LIST_COLOR(list, i, w) \
{ \
    if (i>=0 && i<list.size()) \
    { \
        QColor qc = QColorFromRGBA8(list[i].color); \
        if (v3dr_getColorDialog( &qc, w))  list[i].color = RGBA8FromQColor( qc ); \
    } \
}
#define MIN_BB(minbb, loc) \
{ \
    if(minbb.x > loc.x) minbb.x = loc.x; \
    if(minbb.y > loc.y) minbb.y = loc.y; \
    if(minbb.z > loc.z) minbb.z = loc.z; \
}
#define MAX_BB(maxbb, loc) \
{ \
    if(maxbb.x < loc.x) maxbb.x = loc.x; \
    if(maxbb.y < loc.y) maxbb.y = loc.y; \
    if(maxbb.z < loc.z) maxbb.z = loc.z; \
}
//#define LIST_COLOR_A(list, i, w) \
//{ \
//	if (i>=0 && i<list.size()) \
//	{ \
//		QColor qc = QColor::fromRgba( QColorDialog::getRgba(QColorFromRGBA8(list[i].color).rgba(), 0, w) ); \
//		list[i].color = RGBA8FromQColor( qc ); \
//	} \
//}
int Renderer_gl1::processHit(int namelen, int names[], int cx, int cy, bool b_menu, char* pTip) // called by selectObj() after getting object's names
{

    //qDebug("  Renderer_gl1::processHit  pTip=%p", pTip);
#define __object_name_info__ // dummy, just for easy locating
    // object name string
    QString qsName;
    QString qsInfo;
#define IS_VOLUME() 	(namelen>=3 && names[0]==dcVolume)
#define IS_SURFACE() 	(namelen>=3 && names[0]==dcSurface)
#define NEURON_CONDITION  (listNeuronTree.size()>=1 && w && curImg) //only allow one neuron, and assume it is the one being reconstructed from image
    //qDebug("	namelen=%d, names[0]=%d", namelen, names[0]);
    lastSliceType = vsSliceNone; //for define maker on cross-section slice
    if (IS_VOLUME()) // volume
    {
            QString bound_info = QString("(%1 %2 %3 -- %4 %5 %6)").arg(start1+1).arg(start2+1).arg(start3+1).arg(start1+size1).arg(start2+size2).arg(start3+size3);
            QString unit_info = (data_unitbytes==1)?"uint8":(
                                (data_unitbytes==2)?"uint16":(
                                (data_unitbytes==4)?"float32":(
                                    QString("%1bytes").arg(data_unitbytes)
                                )));
            QString data_title = "";	//if (w) data_title = QFileInfo(w->getDataTitle()).fileName();
            qsName = QString("volume %1 %2 ... ").arg(bound_info).arg(unit_info) + (data_title);
            //qsName += QString("%1").arg(names[2]);
            lastSliceType = names[2]; //100731
            //qDebug()<<"sliceType:"<<currentSliceType;
    }
    if (IS_SURFACE()) // surface object
    {
        switch (names[1])
        {
        case stImageMarker: {//marker
            (qsName = QString("marker #%1 ... ").arg(names[2]) + listMarker.at(names[2]-1).name);
            //qDebug() << qsName;
            /* ======== Select and unselect marker with mouse left button, MK, Sep, 2019 ======= */
            switch (b_menu)
            {
            case true: // With menu popping up (right click)
                if (!this->FragTraceMarkerDetector3Dviewer) LIST_SELECTED(listMarker, names[2] - 1, true);
                break;
            case false: // Without menu popping up (left click)
                switch (listMarker.at(names[2] - 1).selected)
                {
                case true:
                    LIST_SELECTED(listMarker, names[2] - 1, false);
                    break;
                case false:
                    LIST_SELECTED(listMarker, names[2] - 1, true);
                    break;
                }
            }
            /* ================================================================================= */

            qsInfo = info_Marker(names[2]-1);
        }break;
        case stLabelSurface: {//label surface
            (qsName = QString("label surface #%1 ... ").arg(names[2]) + listLabelSurf.at(names[2]-1).name);
            LIST_SELECTED(listLabelSurf, names[2]-1, true);
            int vertex_i=0;
            Triangle * T = findNearestSurfTriangle_WinXY(cx, cy, vertex_i, (Triangle*)list_listTriangle.at(names[2]-1));
            qsInfo = info_SurfVertex(vertex_i, T, listLabelSurf.at(names[2]-1).label);
        }break;
        case stNeuronStructure: {//swc
            (qsName = QString("neuron/line #%1 ... ").arg(names[2]) + listNeuronTree.at(names[2]-1).name);
            LIST_SELECTED(listNeuronTree, names[2]-1, true);
            if (listNeuronTree.at(names[2]-1).editable) qsName += " (editing)";
            NeuronTree *p_tree = (NeuronTree *)&(listNeuronTree.at(names[2]-1));
            //this->teraflyTreePtr = p_tree;
            double best_dist;
            //V3DLONG index = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);
            qsInfo = info_NeuronNode(findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist), p_tree);
            //cout << p_tree->listNeuron.at(index).x << " " << p_tree->listNeuron.at(index).y << " " << p_tree->listNeuron.at(index).z << endl;
        }break;
        case stPointCloud: {//apo
            (qsName = QString("point cloud #%1 ... ").arg(names[2]) + listCell.at(names[2]-1).name);
            if (!this->FragTraceMarkerDetector3Dviewer) LIST_SELECTED(listCell, names[2]-1, true);
        }break;
        }
    }
    //qDebug() <<"\t Hit " << (qsName);
    if (pTip) //090427 RZC: object tooltip name
    {
        LIST_SELECTED(listLabelSurf, names[2]-1, false);
        LIST_SELECTED(listNeuronTree, names[2]-1, false);
        LIST_SELECTED(listCell, names[2]-1, false);
        LIST_SELECTED(listMarker, names[2]-1, false);
        sprintf(pTip, "%s", Q_CSTR(qsName.left(100-1)+qsInfo.left(200-1)));
        return 0; // just for tooltip text, no farther processing
    }
    if (! b_menu) //100731 RZC: for X-section pinpoint
    {
        return 0;
    }
#define __right_click_popup_menu__ // dummy, just for easy locating
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
#ifndef test_main_cpp
    My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
    XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);
    //qDebug("	_idep = %p, _idep->image4d = %p", _idep, ((iDrawExternalParameter*)_idep)->image4d);
    //qDebug("	My4DImage* = %p, XFormWidget* = %p", curImg, curXWidget);
#else
    void* curImg=0;
    void* curXWidget = 0;
#endif //test_main_cpp
    // right click popup menu
    QList<QAction*> listAct;
    QAction *act=0, *actShowFullPath=0,
            *app2Convenient = 0,
            *app2Terafly=0,
            *app2MultiTerafly = 0,
            *app2TeraflyWithPara=0,
            *app2MultiTeraflyWithPara=0,
            *actProperty=0, *actVolColormap=0, *actObjectManager=0, *actOff=0, *actColor=0, *actEditNameComment=0,
            *actSaveSurfaceObj=0,
            *actLockSceneEditObjGeometry=0, *actAddtoMarkerPool=0, *actClearMarkerPool=0,//ZJL
            *actMarkerCreate1=0, *actMarkerCreate1Stroke=0, *actMarkerCreate2=0, *actMarkerCreate3=0, *actMarkerRefineT=0, *actMarkerRefineC=0, *actMarkerRefineLocal=0, *actMarkerAutoSeed=0,
            *actMarkerZoomin3D=0, *actMarkerZoomin3D_terafly=0,
            *actMarkerMoveToMiddleZ=0,
            *actMarkerDelete=0, *actMarkerClearAll=0, *actMarkerCreateNearestNeuronNode=0,
            *actMarkerTraceFromStartPos=0, *actMarkerTraceFromStartPos_selPara=0, *actMarkerLineProfileFromStartPos=0, *actMarkerLineProfileFromStartPos_drawline=0, *actMarkerLabelAsStartPos=0,
            *actMarkerTraceFromOnePos=0, *actMarkerTraceFromOnePosToOtherMarkers=0,
            *actCurveCreate1=0, *actCurveCreate2=0, *actCurveCreate3=0, *actCurveCreate_pointclick=0,
            *actCurveCreate_zoom=0, *actMarkerCreate_zoom=0,
               *actCurveRefine=0, *actCurveEditRefine=0, *actCurveRubberDrag=0,  *actCurveDirectionInter=0,
          *actCurveCreate_pointclick_fm=0, *actCurveMarkerLists_fm=0, *actCurveRefine_fm=0,*actCurveEditRefine_fm=0,
          *actCurveMarkerPool_fm=0, *actCurveCreateMarkerGD=0, *actCurveFrom1Marker_fm=0, *actCurveTiltedBB_fm=0, *actCurveTiltedBB_fm_sbbox=0,
          *actCurveCreateTest=0,// ZJL 110905
            *actClearedAllGeneratedObjects=0, //PHC 120522
            *actCurveCreate_zoom_imaging=0, *actMarkerCreate_zoom_imaging=0,
            *actMarkerAblateOne_imaging=0, *actMarkerAblateAll_imaging=0,
            *actMarkerAblateOne_imaging_nocalib=0, *actMarkerAblateAll_imaging_nocalib=0, //without calib and low speed
            *actMarkerAblateOne_imaging_feedback=0, *actMarkerAblateAll_imaging_feedback=0, //with feedback
            *actMarkerOne_imaging=0, *actMarkerAll_imaging=0,
            *act_imaging_pinpoint_n_shoot=0, *act_imaging_cut_3d_curve=0,
            *actCurveCreate_zoom_grabhighrezdata=0, *actMarkerCreate_zoom_grabhighrezdata=0, //for TeraManager or similar high-rez data acqusition. by PHC, 20120717
            *actVaa3DNeuron2App2=0, //for Vaa3D_Neuron2 APP tracing. by PHC, 20121201
            *actGDCurveline=0, //for GD based curveline detection. by PHC, 20170529
            //need to add more surgical operations here later, such as curve_ablating (without displaying the curve first), etc. by PHC, 20101105
            *actNeuronToEditable=0, *actDecomposeNeuron=0, *actNeuronFinishEditing=0,
            *actChangeNeuronSegType=0, *actChangeNeuronSegRadius=0, *actReverseNeuronSeg=0,
            *actDispRecNeuronSegInfo=0, *actDeleteNeuronSeg=0, *actBreakNeuronSegNearestNeuronNode=0, *actBreakNeuronSeg_markclick=0,
            *actJoinNeuronSegs_nearby_markclick=0, *actJoinNeuronSegs_nearby_pathclick=0, *actJoinNeuronSegs_all=0,
            *actNeuronSegDeform=0, *actNeuronSegProfile=0,
                *actChangeMultiNeuronSegType=0, //Zhi Zhou
                *actBreakMultiNeuronSeg=0, //Zhi Zhou
            *actNeuronOneSegMergeToCloseby=0, *actNeuronAllSegMergeToCloseby=0,
               *actDeleteMultiNeuronSeg=0, // ZJL, 20120806
            *actDispNeuronNodeInfo=0,	*actAveDistTwoNeurons=0, *actDispNeuronMorphoInfo=0,
            *actDoNeuronToolBoxPlugin=0,
            *actDispSurfVertexInfo=0,
            *actComputeSurfArea=0, *actComputeSurfVolume=0,
            *actZoomin_currentviewport=0, //PHC, 130701
            *actRetrace=0,
            *actNeuronConnect = 0, *actPointCloudConnect = 0, *actMarkerConnect = 0, *actNeuronCut = 0, // MK, 2017 April
            *simpleConnect = 0, *simpleConnect_loopSafe = 0, // MK, 2018, April
            *actMarkerAltRotationCenter=0 //RZC, 20180703
            ;
     // used to control whether menu item is added in VOLUME popup menu ZJL
     //bool bHasSegID = false;
    if (qsName.size()>0)
    {
        listAct.append(actProperty = new QAction(qsName, w));
        //##############################################################################
        // volume data
        //##############################################################################
        if (IS_VOLUME())
        {
            if (tryVolShader)
            {
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actVolColormap = new QAction("volume colormap", w));
            }

            if (curImg)
            {

#ifdef _ALLOW_ADVANCE_PROCESSING_MENU_
            listAct.append(act = new QAction("", w)); act->setSeparator(true);
            listAct.append(actMarkerCreate1 = new QAction("1-right-click to define a marker (Esc to finish)", w));
            actMarkerCreate1->setIcon(QIcon(":/icons/click1.png"));
            actMarkerCreate1->setVisible(true);
            actMarkerCreate1->setIconVisibleInMenu(true);
            listAct.append(actMarkerCreate1Stroke = new QAction("1-right-stroke to define a marker (starting locus will be the output marker, Esc to finish)", w));

            actMarkerCreate1Stroke->setIcon(QIcon(":/icons/click1.png"));
            actMarkerCreate1Stroke->setVisible(true);
            actMarkerCreate1Stroke->setIconVisibleInMenu(true);
            listAct.append(act = new QAction("", w)); act->setSeparator(true);
            listAct.append(actMarkerCreate2 = new QAction("2-right-clicks to define a marker", w));
            actMarkerCreate2->setIcon(QIcon(":/icons/click2.png"));
            actMarkerCreate2->setVisible(true);
            actMarkerCreate2->setIconVisibleInMenu(true);
            if (0)
            {
            listAct.append(actMarkerCreate3 = new QAction("3-right-clicks to define a marker", w));
            actMarkerCreate3->setIcon(QIcon(":/icons/click3.png"));
            actMarkerCreate3->setVisible(true);
            actMarkerCreate3->setIconVisibleInMenu(true);
            }
            //listAct.append(act = new QAction("", w)); act->setSeparator(true);
#ifdef _ALLOW_AUTOMARKER_MENU_
            listAct.append(actMarkerAutoSeed = new QAction("AutoMarker", w));
#endif
#ifdef _ALLOW_3D_CURVE_
            if (0) //disable two menu items, PHC 20120813
            {
                listAct.append(actCurveDirectionInter = new QAction("1-right-stroke to define a 3D curve by direction intersection", w));
                actCurveDirectionInter->setIcon(QIcon(":/icons/stroke1.png"));
                actCurveDirectionInter->setVisible(true);
                actCurveDirectionInter->setIconVisibleInMenu(true);
                listAct.append(actCurveMarkerLists_fm = new QAction("1-right-stroke to define a 3D curve (adjacent-pair fast-marching)", w));
                actCurveMarkerLists_fm->setIcon(QIcon(":/icons/stroke1.png"));
                actCurveMarkerLists_fm->setVisible(true);
                actCurveMarkerLists_fm->setIconVisibleInMenu(true);
            }

            if (this->dataViewProcBox.x0 < this->dataViewProcBox.x1 &&
                this->dataViewProcBox.y0 < this->dataViewProcBox.y1 &&
                this->dataViewProcBox.z0 < this->dataViewProcBox.z1)
                //disable the fast-marching based menu when one plane is used. by PHC 20140419. because the computation is still dependent on the entire volume
            {
                listAct.append(act = new QAction("", w)); act->setSeparator(true);

                listAct.append(actCurveTiltedBB_fm = new QAction("1-right-stroke to define a 3D curve (Ver 2a. adjacent-pair fast-marching - global optimal)", w));
                actCurveTiltedBB_fm->setIcon(QIcon(":/icons/stroke1.png"));
                actCurveTiltedBB_fm->setVisible(true);
                actCurveTiltedBB_fm->setIconVisibleInMenu(true);

                listAct.append(actCurveTiltedBB_fm_sbbox = new QAction("1-right-stroke to define a 3D curve (Ver 2b. adjacent-pair fast-marching - serial BBoxes)", w));
                actCurveTiltedBB_fm_sbbox->setIcon(QIcon(":/icons/stroke1.png"));
                actCurveTiltedBB_fm_sbbox->setVisible(true);
                actCurveTiltedBB_fm_sbbox->setIconVisibleInMenu(true);
            }

            listAct.append(act = new QAction("", w)); act->setSeparator(true);
            listAct.append(actCurveCreate1 = new QAction("1-right-stroke to define a 3D curve (Ver 1.0)", w));
            actCurveCreate1->setIcon(QIcon(":/icons/stroke1.png"));
            actCurveCreate1->setVisible(true);
            actCurveCreate1->setIconVisibleInMenu(true);

            if (0) //disable two not-often used functions
            {
                listAct.append(actCurveCreate2 = new QAction("2-right-strokes to define a 3D curve", w));
                actCurveCreate2->setIcon(QIcon(":/icons/stroke2.png"));
                actCurveCreate2->setVisible(true);
                actCurveCreate2->setIconVisibleInMenu(true);
                listAct.append(actCurveCreate3 = new QAction("3-right-strokes to define a 3D curve", w));
                actCurveCreate3->setIcon(QIcon(":/icons/stroke3.png"));
                actCurveCreate3->setVisible(true);
                actCurveCreate3->setIconVisibleInMenu(true);
            }

            listAct.append(act = new QAction("", w)); act->setSeparator(true);
            listAct.append(actCurveCreate_pointclick_fm = new QAction("Series of right-clicks to define a 3D curve (Esc to finish)", w));
            actCurveCreate_pointclick_fm->setIcon(QIcon(":/icons/strokeN.png"));
            actCurveCreate_pointclick_fm->setVisible(true);
            actCurveCreate_pointclick_fm->setIconVisibleInMenu(true);
            listAct.append(actCurveCreate_pointclick = new QAction("Series of right-clicks to define a 3D polyline (Esc to finish)", w));
            actCurveCreate_pointclick->setIcon(QIcon(":/icons/strokeN.png"));
            actCurveCreate_pointclick->setVisible(true);
            actCurveCreate_pointclick->setIconVisibleInMenu(true);

            if(listCurveMarkerPool.size()>=2)
            {
                listAct.append(actCurveMarkerPool_fm = new QAction("Using marker pool to define a 3D curve by FM", w));
                actCurveMarkerPool_fm->setIcon(QIcon(":/icons/strokeN.png"));
                actCurveMarkerPool_fm->setVisible(true);
                actCurveMarkerPool_fm->setIconVisibleInMenu(true);
                listAct.append(actCurveCreateMarkerGD = new QAction("Using marker pool to define a 3D curve by GD", w));
                actCurveCreateMarkerGD->setIcon(QIcon(":/icons/strokeN.png"));
                actCurveCreateMarkerGD->setVisible(true);
                actCurveCreateMarkerGD->setIconVisibleInMenu(true);
                // clear listCurveMarkerPool
                listAct.append(actClearMarkerPool = new QAction("Clear curve marker pool", w));
                actClearMarkerPool->setVisible(true);
            }

            if (0) //disabled by PHC, 2012-08-13.
            {
                listAct.append(actCurveCreateTest = new QAction("Test multiple curve creation methods", w));
                actCurveCreateTest->setIcon(QIcon(":/icons/strokeN.png"));
                actCurveCreateTest->setVisible(true);
                actCurveCreateTest->setIconVisibleInMenu(true);
                // For curve refinement, ZJL 110831
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actCurveRefine = new QAction("n-right-strokes to define a 3D curve by mean shift (refine)", w));
                actCurveRefine->setIcon(QIcon(":/icons/strokeN.png"));
                actCurveRefine->setVisible(true);
                actCurveRefine->setIconVisibleInMenu(true);
                listAct.append(actCurveRefine_fm = new QAction("n-right-strokes to define a 3D curve by adjacent-pair fast marching (refine)", w));
                actCurveRefine_fm->setIcon(QIcon(":/icons/strokeN.png"));
                actCurveRefine_fm->setVisible(true);
                actCurveRefine_fm->setIconVisibleInMenu(true);
            }

            //if (!(((iDrawExternalParameter*)_idep)->b_local)) //only enable the menu for global 3d viewer. as it seems there is a bug in the local 3d viewer. by PHC, 100821
            {
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actCurveCreate_zoom = new QAction("Zoom-in view: 1-right-stroke ROI", w));
                listAct.append(actMarkerCreate_zoom = new QAction("Zoom-in view: 1-right-click ROI", w));
                listAct.append(actZoomin_currentviewport = new QAction("Zoom-in view: current viewport content ROI", w));
                { //conditionally add tera manager
                    QDir pluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
                    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
                        pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
                    if (pluginsDir.dirName() == "MacOS") {
                        pluginsDir.cdUp();
                        pluginsDir.cdUp();
                        pluginsDir.cdUp();
                    }
#endif

                    QDir pluginsDir1 = pluginsDir;
                    // @FIXED by Alessandro on 2015-09-30.
                    // Re-enabled after migration of TeraFly from plugin to Vaa3D.
                    {
                        listAct.append(act = new QAction("", w)); act->setSeparator(true);
                        listAct.append(actCurveCreate_zoom_grabhighrezdata = new QAction("Zoom-in HighRezImage: 1-right-stroke ROI", w));
                        listAct.append(actMarkerCreate_zoom_grabhighrezdata = new QAction("Zoom-in HighRezImage: 1-right-click ROI", w));
                    }

                    pluginsDir1 = pluginsDir;
                    if (pluginsDir1.cd("plugins/neuron_tracing/Vaa3D_Neuron2")==true)
                    {
                        listAct.append(act = new QAction("", w)); act->setSeparator(true);
                        listAct.append(actVaa3DNeuron2App2 = new QAction("Vaa3D-Neuron2 auto-tracing", w));
                    }

                    pluginsDir1 = pluginsDir;
                    if (pluginsDir1.cd("plugins/line_detector")==true) //by PHC 20170529
                    {
                        listAct.append(act = new QAction("", w)); act->setSeparator(true);
                        listAct.append(actGDCurveline = new QAction("GD-curveline detection", w));
                    }

                    pluginsDir1 = pluginsDir;
                    if (pluginsDir1.cd("plugins/Retrace")==true)
                    {
                        listAct.append(act = new QAction("", w)); act->setSeparator(true);
                        listAct.append(actRetrace = new QAction("Retreace", w));
                        listAct.append(app2Convenient = new QAction("app2Convenient", w));

                        listAct.append(app2Terafly = new QAction("app2Terafly", w));
                        listAct.append(app2MultiTerafly = new QAction("app2MultiTerafly", w));
                        listAct.append(app2TeraflyWithPara = new QAction("app2TeraflyWithPara", w));
                        listAct.append(app2MultiTeraflyWithPara = new QAction("app2MultiTeraflyWithPara", w));

                    }


                }

//150616. Add neuron menu when there is only one neuron
                if (listNeuronTree.size()==1 && w && curImg)
                    //by PHC. only enable the following pop-up menu when there is the image data, only one neuron (presumably the one being reconstructed
                    //this may be revised later so that the raw image can contain multiple neurons. But as of now (090206), I only allow one neuron being reconstructed at a time
                {
                    listAct.append(actMarkerCreateNearestNeuronNode = new QAction("create marker from the nearest neuron-node", w));
                    if (listNeuronTree.at(0).editable==false)
                    {
                        listAct.append(act = new QAction("", w)); act->setSeparator(true);
                        listAct.append(actNeuronToEditable = new QAction("edit this neuron (*only 1 editable neuron at one time*)", w));
                    }
                    if (listNeuronTree.at(0).editable==true) //this requires V_NeuronSWC
                    {
                        listAct.append(act = new QAction("", w)); act->setSeparator(true);
                        // ZJL 110913:
                        // Edit the curve by refining or extending as in "n-right-strokes to define a curve (refine)"
                        listAct.append(actCurveEditRefine = new QAction("extend/refine nearest neuron-segment by mean shift", w));
                        actCurveEditRefine->setIcon(QIcon(":/icons/strokeN.png"));
                        actCurveEditRefine->setVisible(true);
                        actCurveEditRefine->setIconVisibleInMenu(true);
                        //listAct.append(act = new QAction("", w));
                        listAct.append(actCurveEditRefine_fm = new QAction("extend/refine nearest neuron-segment by adjacent-pair fast marching", w));
                        actCurveEditRefine_fm->setIcon(QIcon(":/icons/strokeN.png"));
                        actCurveEditRefine_fm->setVisible(true);
                        actCurveEditRefine_fm->setIconVisibleInMenu(true);
                        //listAct.append(act = new QAction("", w));
                        // Drag a curve to refine it by using rubber-band line like method
                        listAct.append(actCurveRubberDrag = new QAction("drag nearest neuron-segment", w));
                        actCurveRubberDrag->setIcon(QIcon(":/icons/click3.png"));
                        actCurveRubberDrag->setVisible(true);
                        actCurveRubberDrag->setIconVisibleInMenu(true);
                        listAct.append(act = new QAction("", w));
                        act->setSeparator(true);
                        // End of ZJL
                        listAct.append(actDispRecNeuronSegInfo = new QAction("display nearest neuron-segment info", w));
                        listAct.append(actChangeNeuronSegType = new QAction("change nearest neuron-segment type", w));
                        //By Zhi Zhou
                        listAct.append(actChangeMultiNeuronSegType = new QAction("change multiple neuron-segments type by a stroke", w));
                        listAct.append(actChangeNeuronSegRadius = new QAction("change nearest neuron-segment radius", w));
                        listAct.append(actReverseNeuronSeg = new QAction("reverse nearest neuron-segment link order", w));
                        listAct.append(actDeleteNeuronSeg = new QAction("delete the nearest neuron-segment", w));

                        // 2015-05-06. @ADDED by Alessandro. Just enabled an already existing function developed by ZJL, 20120806
                        listAct.append(actDeleteMultiNeuronSeg = new QAction("delete multiple neuron-segments by a stroke", w));

                        // MK, 2017 April, 2018 April
                        listAct.append(simpleConnect = new QAction("SWC simple connecting (only 2 segments at a time)", w));
                        listAct.append(actNeuronConnect = new QAction("connect segments with one stroke (auto smoothing)", w));
                        listAct.append(simpleConnect_loopSafe = new QAction("SWC simple connecting with loop detection (only 2 segments at a time)", w));

                        // MK, 2017 June
                        listAct.append(actNeuronCut = new QAction("cut neurons with one stroke", w));

                        //listAct.append(actNeuronOneSegMergeToCloseby = new QAction("merge a terminal-segment to nearby segments", w));
                        //listAct.append(actNeuronAllSegMergeToCloseby = new QAction("merge nearby segments", w)); //disable as of 20140630 for further dev. PHC
                        if (curImg->tracedNeuron.isJointed()==false)
                        {
                            listAct.append(actBreakNeuronSegNearestNeuronNode = new QAction("break the segment using nearest neuron-node", w));
                            listAct.append(actBreakMultiNeuronSeg = new QAction("break multiple segments by a stroke", w));

                            //listAct.append(actJoinNeuronSegs_nearby_pathclick = new QAction("join nearby (direct-connected) neuron segments", w));
                            listAct.append(actJoinNeuronSegs_all = new QAction("join *all* neuron-segments (and remove all duplicated nodes)", w));
                        }
                        if (curImg->tracedNeuron.isJointed())
                        {
                            listAct.append(actDecomposeNeuron = new QAction("decompose to *simple* neuron-segments", w));
                        }
                        listAct.append(act = new QAction("", w)); act->setSeparator(true);
                        listAct.append(actNeuronFinishEditing = new QAction("finish editing this neuron", w));
                        if (curImg->valid())
                        {
                            listAct.append(act = new QAction("", w)); act->setSeparator(true);
                            listAct.append(actNeuronSegDeform = new QAction("deform the neuron-segment", w));
                            listAct.append(actNeuronSegProfile = new QAction("neuron-segment intensity profile", w));
                        }
                    }
                }

                //101008
                //#ifdef _WIN32 && _MSC_VER
#ifdef _IMAGING_MENU_
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actCurveCreate_zoom_imaging = new QAction("Zoom-in imaging: 1-right-stroke ROI", w));
                listAct.append(actMarkerCreate_zoom_imaging = new QAction("Zoom-in imaging: 1-right-click ROI", w));
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(act_imaging_pinpoint_n_shoot = new QAction("Pinpoint-N-shoot", w));
                listAct.append(act_imaging_cut_3d_curve = new QAction("Cut-3D-curve", w));
#endif
                //#endif
                if (NEURON_CONDITION)
                {
                    listAct.append(act = new QAction("", w)); act->setSeparator(true);
                    listAct.append(actClearedAllGeneratedObjects = new QAction("Clear all generated surface objects (e.g. traced neuron)", w));
                }
            }
#endif
#endif
            }
        }
        //##############################################################################
        // surface data
        //##############################################################################
        if (IS_SURFACE())
        {
            if (this->FragTraceMarkerDetector3Dviewer)
            {
                if (names[1] == stImageMarker)
                {
                    this->surType = stImageMarker;
                    switch (listMarker.at(names[2] - 1).selected)
                    {
                    case true:
                        LIST_SELECTED(listMarker, names[2] - 1, false);
                        break;
                    case false:
                        LIST_SELECTED(listMarker, names[2] - 1, true);
                        break;
                    }

                    V3DPluginArgList pluginInputList, pluginOutputList;
                    V3DPluginArgItem dummyInput, inputParam, dummyOutput;
                    vector<char*> pluginInputArgList;
                    vector<char*> pluginOutputArgList;
                    dummyInput.type = "dummy";
                    dummyInput.p = (void*)(&pluginInputArgList);
                    inputParam.type = "dummy";
                    inputParam.p = (void*)(&pluginInputArgList);
                    pluginInputList.push_back(dummyInput);
                    pluginInputList.push_back(inputParam);
                    dummyOutput.type = "dummy";
                    dummyOutput.p = (void*)(&pluginOutputArgList);
                    XFormWidget* curXWidget = v3dr_getXWidget(_idep);
                    curXWidget->getMainControlWindow()->pluginLoader->callPluginFunc("Fragmented_Auto-trace", "3DViewer_marker_click", pluginInputList, pluginOutputList);

                    return 0;
                }
                else if (names[1] == stPointCloud)
                {
                    this->surType = stPointCloud;
                    switch (listCell.at(names[2] - 1).selected)
                    {
                    case true:
                        LIST_SELECTED(listCell, names[2] - 1, false);
                        break;
                    case false:
                        LIST_SELECTED(listCell, names[2] - 1, true);
                        break;
                    }

                    V3DPluginArgList pluginInputList, pluginOutputList;
                    V3DPluginArgItem dummyInput, inputParam, dummyOutput;
                    vector<char*> pluginInputArgList;
                    vector<char*> pluginOutputArgList;
                    dummyInput.type = "dummy";
                    dummyInput.p = (void*)(&pluginInputArgList);
                    inputParam.type = "dummy";
                    inputParam.p = (void*)(&pluginInputArgList);
                    pluginInputList.push_back(dummyInput);
                    pluginInputList.push_back(inputParam);
                    dummyOutput.type = "dummy";
                    dummyOutput.p = (void*)(&pluginOutputArgList);
                    XFormWidget* curXWidget = v3dr_getXWidget(_idep);
                    curXWidget->getMainControlWindow()->pluginLoader->callPluginFunc("Fragmented_Auto-trace", "3DViewer_marker_click", pluginInputList, pluginOutputList);

                    return 0;
                }
            }

            listAct.append(act = new QAction("", w)); act->setSeparator(true);
            listAct.append(actShowFullPath = new QAction("Show full path", w));
            listAct.append(actObjectManager = new QAction("object manager", w));
            listAct.append(actOff   = new QAction("off", w));
            listAct.append(actColor = new QAction("color", w));
            //listAct.append(actEditNameComment = new QAction("name/comment", w));
            // --------------------------------------------------------------------------------------------------
            listAct.append(act = new QAction("", w)); act->setSeparator(true);
            if (names[1]==stNeuronStructure)
                listAct.append(actSaveSurfaceObj = new QAction("save the selected structure to file", w));
            else if (names[1]==stImageMarker)
                listAct.append(actSaveSurfaceObj = new QAction("save all markers to file", w));
            else if (names[1]==stLabelSurface)
                listAct.append(actSaveSurfaceObj = new QAction("save all label-field surface objects to file", w));
            else if (names[1]==stPointCloud)
            {
                listAct.append(actPointCloudConnect = new QAction("create neuron segments by connecting points cloud", w));
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actSaveSurfaceObj = new QAction("save all point-cloud objects to file", w));
            }
#ifdef _ALLOW_ADVANCE_PROCESSING_MENU_
            if (names[1]==stNeuronStructure || names[1]==stPointCloud)
            {
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actLockSceneEditObjGeometry = new QAction("lock scene and adjust this object", w));
            }
#endif
            //##############################################################################
            // maker
            //##############################################################################
            if (names[1]==stImageMarker)
            {
                currentMarkerName = names[2];
                    listAct.append(act = new QAction("", w)); act->setSeparator(true); // ZJL
                    listAct.append(actAddtoMarkerPool = new QAction("add this marker to the curve marker pool", w));
                    listAct.append(actClearMarkerPool = new QAction("clear the curve marker pool", w));
#ifdef _ALLOW_ADVANCE_PROCESSING_MENU_
#ifdef _ALLOW_LOCAL_ZOOMIN_3D_VIEWER_
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actMarkerZoomin3D = new QAction("open the local zoom-in 3d viewer", w));
#endif


                QDir pluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
                if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
                    pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
                if (pluginsDir.dirName() == "MacOS") {
                    pluginsDir.cdUp();
                    pluginsDir.cdUp();
                    pluginsDir.cdUp();
                }
#endif

                listAct.append(actMarkerZoomin3D_terafly = new QAction("Zoom-in to this select marker location", w));
                listAct.append(actMarkerAltRotationCenter = new QAction("alternate rotation center at this marker location (holding ALT key)", w));
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                //listAct.append(actMarkerRefineLocal = new QAction("refine marker to local center", w));
                listAct.append(actMarkerRefineC = new QAction("re-define marker on intense position by 1 right-click", w));
                listAct.append(actMarkerRefineT = new QAction("translate marker position by 1 right-click", w));
                listAct.append(actMarkerConnect = new QAction("create segments by connecting markers", w)); // MK, 2017 April
                listAct.append(actMarkerDelete = new QAction("delete this marker", w));
                listAct.append(actMarkerClearAll = new QAction("clear All markers", w));
                listAct.append(actMarkerMoveToMiddleZ = new QAction("change all markers' Z locations to mid-Z-slice", w));
#ifdef _IMAGING_MENU_
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actMarkerAblateOne_imaging = new QAction("ablate this marker", w));
                listAct.append(actMarkerAblateAll_imaging = new QAction("ablate All markers", w));
                listAct.append(actMarkerAblateOne_imaging_nocalib = new QAction("ablate this marker (no calibration & low speed)", w));
                listAct.append(actMarkerAblateAll_imaging_nocalib = new QAction("ablate All markers (no calibration & low speed)", w));
                listAct.append(actMarkerAblateOne_imaging_feedback = new QAction("ablate this marker (with feedback)", w));
                listAct.append(actMarkerAblateAll_imaging_feedback = new QAction("ablate All markers (with feedback)", w));
                listAct.append(actMarkerOne_imaging = new QAction("Imaging on this marker", w));
                //listAct.append(actMarkerAll_imaging = new QAction("Imaging on All markers", w));
#endif
                // marker to tracing -----------------------------------------------------------
#ifdef _ALLOW_NEURONSEG_MENU_
#ifdef _ALLOW_NEURONTREE_ONE2OTHERS_MENU_
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
#else ifdef _ALLOW_NEURONTREE_ONE2ENTIREIMG_MENU_
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
#endif
#ifdef _ALLOW_NEURONTREE_ONE2OTHERS_MENU_
                if (listMarker.size()>=2)
                {
                    listAct.append(actMarkerTraceFromOnePosToOtherMarkers = new QAction("trace from 1 start pos to all other markers", w));
                    actMarkerTraceFromOnePosToOtherMarkers->setIcon(QIcon(":/icons/trace.png"));
                    actMarkerTraceFromOnePosToOtherMarkers->setVisible(true);
                    actMarkerTraceFromOnePosToOtherMarkers->setIconVisibleInMenu(true);
                }
#endif
#ifdef _ALLOW_NEURONTREE_ONE2ENTIREIMG_MENU_
                listAct.append(actMarkerTraceFromOnePos = new QAction("trace from 1 start pos through entire image", w));
                actMarkerTraceFromOnePos->setIcon(QIcon(":/icons/trace.png"));
                actMarkerTraceFromOnePos->setVisible(true);
                actMarkerTraceFromOnePos->setIconVisibleInMenu(true);
#endif
#endif //_ALLOW_NEURONSEG_MENU_
#endif //_ALLOW_ADVANCE_PROCESSING_MENU_
                if (listNeuronTree.size()==1 && w && curImg)
                {
                    //listAct.append(act = new QAction("", w)); act->setSeparator(true);
                    //listAct.append(actJoinNeuronSegs_nearby_markclick = new QAction("join nearby (direct-connected) neuron segments of this marker", w));
                    //listAct.append(actBreakNeuronSeg_markclick = new QAction("break neuron segment connecting this marker", w));
                }
#ifndef test_main_cpp
                if (curImg) //try to update the marker hit info if possible. by PHC, 090119
                {
                    //only display the trace menu if the original image data is valid, and the last_hit_mark is valid (not -1) and the current_hit location is different
#ifdef _ALLOW_ADVANCE_PROCESSING_MENU_
#ifdef _ALLOW_NEURONSEG_MENU_
                    listAct.append(act = new QAction("", w)); act->setSeparator(true);
                    listAct.append(actMarkerLabelAsStartPos = new QAction("label as starting pos for tracing/measuring", w)); //by PHC, 090119
                    actMarkerLabelAsStartPos->setIcon(QIcon(":/icons/start.png"));
                    actMarkerLabelAsStartPos->setVisible(true);
                    actMarkerLabelAsStartPos->setIconVisibleInMenu(true);
                    if (curImg->last_hit_landmark >= 0 && curImg->last_hit_landmark!=currentMarkerName-1)
                    {
                        listAct.append(actMarkerTraceFromStartPos = new QAction("trace from the starting pos (and use the *1st* data channel)", w));
                        listAct.append(actMarkerTraceFromStartPos_selPara = new QAction("trace from the starting pos but select para first", w));
                        listAct.append(actMarkerLineProfileFromStartPos_drawline = new QAction("line profile from the starting pos (drawing a line)", w));
                        listAct.append(actMarkerLineProfileFromStartPos = new QAction("line profile from the starting pos (w/o drawing line)", w));
                    }
#else
                    listAct.append(act = new QAction("", w)); act->setSeparator(true);
                    listAct.append(actMarkerLabelAsStartPos = new QAction("label as starting pos for measuring", w)); //by PHC, 090119
                    if (curImg->last_hit_landmark >= 0 && curImg->last_hit_landmark!=currentMarkerName-1)
                    {
                        listAct.append(actMarkerLineProfileFromStartPos_drawline = new QAction("line profile from the starting pos (drawing a line)", w));
                        listAct.append(actMarkerLineProfileFromStartPos = new QAction("line profile from the starting pos (w/o drawing line)", w));
                    }
#endif
#endif
                }
                    listAct.append(actCurveFrom1Marker_fm = new QAction("Start from this marker to define a 3D curve by adjacent-pair fast-marching", w));
                    actCurveFrom1Marker_fm->setIcon(QIcon(":/icons/stroke1.png"));
                    actCurveFrom1Marker_fm->setVisible(true);
                    actCurveFrom1Marker_fm->setIconVisibleInMenu(true);
#endif //test_main_cpp
            }
            //##############################################################################
            // neuron
            //##############################################################################
            else if (names[1]==stNeuronStructure)
            {
#ifdef _ALLOW_ADVANCE_PROCESSING_MENU_
#ifdef _ALLOW_NEURONSEG_MENU_
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actDispNeuronMorphoInfo = new QAction("display morphology info for the whole neuron", w));
                listAct.append(actDispNeuronNodeInfo = new QAction("display nearest neuron-node info", w));
                //check the existence of a neuron_toolbox plugin, if yes, then create a menu. If no, do nothing. by PHC, 20120406
                {
            QDir pluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
            if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
                pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
            if (pluginsDir.dirName() == "MacOS") {
                pluginsDir.cdUp();
                pluginsDir.cdUp();
                pluginsDir.cdUp();
            }
#endif
            if (pluginsDir.cd("plugins/neuron_toolbox")==true)
            {
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actDoNeuronToolBoxPlugin = new QAction("NeuronToolbox", w));
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
            }
        }
        if (listNeuronTree.size()>=1 && w && curImg)
            //by PHC. only enable the following pop-up menu when there is the image data, only one neuron (presumably the one being reconstructed
            //this may be revised later so that the raw image can contain multiple neurons. But as of now (090206), I only allow one neuron being reconstructed at a time
        {
            listAct.append(actMarkerCreateNearestNeuronNode = new QAction("create marker from the nearest neuron-node", w));
            if (listNeuronTree.at(names[2]-1).editable==false)
            {
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actNeuronToEditable = new QAction("edit this neuron (*only 1 editable neuron at one time*)", w));
            }
            if (listNeuronTree.at(names[2]-1).editable==true) //this requires V_NeuronSWC
            {
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                // ZJL 110913:
                // Edit the curve by refining or extending as in "n-right-strokes to define a curve (refine)"
                listAct.append(actCurveEditRefine = new QAction("extend/refine nearest neuron-segment by mean shift", w));
                actCurveEditRefine->setIcon(QIcon(":/icons/strokeN.png"));
                actCurveEditRefine->setVisible(true);
                actCurveEditRefine->setIconVisibleInMenu(true);
                //listAct.append(act = new QAction("", w));
                listAct.append(actCurveEditRefine_fm = new QAction("extend/refine nearest neuron-segment by adjacent-pair fast marching", w));
                actCurveEditRefine_fm->setIcon(QIcon(":/icons/strokeN.png"));
                actCurveEditRefine_fm->setVisible(true);
                actCurveEditRefine_fm->setIconVisibleInMenu(true);
                //listAct.append(act = new QAction("", w));
                // Drag a curve to refine it by using rubber-band line like method
                listAct.append(actCurveRubberDrag = new QAction("drag nearest neuron-segment", w));
                actCurveRubberDrag->setIcon(QIcon(":/icons/click3.png"));
                actCurveRubberDrag->setVisible(true);
                actCurveRubberDrag->setIconVisibleInMenu(true);
                listAct.append(act = new QAction("", w));
                act->setSeparator(true);
                // End of ZJL
                listAct.append(actDispRecNeuronSegInfo = new QAction("display nearest neuron-segment info", w));
                listAct.append(actChangeNeuronSegType = new QAction("change nearest neuron-segment type", w));
                //By Zhi Zhou
                listAct.append(actChangeMultiNeuronSegType = new QAction("change multiple neuron-segments type by a stroke", w));
                listAct.append(actChangeNeuronSegRadius = new QAction("change nearest neuron-segment radius", w));
                listAct.append(actReverseNeuronSeg = new QAction("reverse nearest neuron-segment link order", w));
                listAct.append(actDeleteNeuronSeg = new QAction("delete the nearest neuron-segment", w));

                // 2015-05-06. @ADDED by Alessandro. Just enabled an already existing function developed by ZJL, 20120806
                listAct.append(actDeleteMultiNeuronSeg = new QAction("delete multiple neuron-segments by a stroke", w));

                // MK, 2017 April, 2018 April
                listAct.append(simpleConnect = new QAction("SWC simple connecting (only 2 segments at a time)", w));
                listAct.append(actNeuronConnect = new QAction("connect segments with one stroke (auto smoothing)", w));
                listAct.append(simpleConnect_loopSafe = new QAction("SWC simple connecting with loop detection (only 2 segments at a time)", w));

                // MK, 2017 June
                listAct.append(actNeuronCut = new QAction("cut neurons with one stroke", w));

                //listAct.append(actNeuronOneSegMergeToCloseby = new QAction("merge a terminal-segment to nearby segments", w));
                //listAct.append(actNeuronAllSegMergeToCloseby = new QAction("merge nearby segments", w)); //disable as of 20140630 for further dev. PHC
                if (curImg->tracedNeuron.isJointed()==false)
                {
                    listAct.append(actBreakNeuronSegNearestNeuronNode = new QAction("break the segment using nearest neuron-node", w));
                    listAct.append(actBreakMultiNeuronSeg = new QAction("break multiple segments by a stroke", w));

                    //listAct.append(actJoinNeuronSegs_nearby_pathclick = new QAction("join nearby (direct-connected) neuron segments", w));
                    listAct.append(actJoinNeuronSegs_all = new QAction("join *all* neuron-segments (and remove all duplicated nodes)", w));
                }
                if (curImg->tracedNeuron.isJointed())
                {
                    listAct.append(actDecomposeNeuron = new QAction("decompose to *simple* neuron-segments", w));
                }
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actNeuronFinishEditing = new QAction("finish editing this neuron", w));
                if (curImg->valid())
                {
                    listAct.append(act = new QAction("", w)); act->setSeparator(true);
                    listAct.append(actNeuronSegDeform = new QAction("deform the neuron-segment", w));
                    listAct.append(actNeuronSegProfile = new QAction("neuron-segment intensity profile", w));
                }
            }
        }
#else
        listAct.append(act = new QAction("", w)); act->setSeparator(true);
        if (listNeuronTree.size()==1 && w && curImg)
            //by PHC. only enable the following pop-up menu when there is the image data, only one neuron (presumably the one being reconstructed
            //this may be revised later so that the raw image can contain multiple neurons. But as of now (090206), I only allow one neuron being reconstructed at a time
        {
            listAct.append(actDeleteNeuronSeg = new QAction("delete the line segment", w));
        }
#endif
        if (listNeuronTree.size()>=2)
        {
            listAct.append(act = new QAction("", w)); act->setSeparator(true);
            listAct.append(actAveDistTwoNeurons = new QAction("distances of two neurons", w));
        }
#endif
            }
            //##############################################################################
            // label surface
            //##############################################################################
            else if (names[1]==stLabelSurface)
            {
                listAct.append(act = new QAction("", w)); act->setSeparator(true);
                listAct.append(actDispSurfVertexInfo = new QAction("display nearest surface vertex info", w));
                listAct.append(actComputeSurfArea = new QAction("surface area", w));
                //listAct.append(actComputeSurfVolume = new QAction("compute surface volume", w));
            }
        }
#if defined(USE_Qt5)
        if (w) w->update(); //for highlight object
#else
        if (w) w->update(); //for highlight object
#endif
        //###############################################################
        // do menu
        //###############################################################
        QMenu menu;
        if (!Renderer_gl1::rightClickMenuDisabled)
        {
            foreach (QAction* a, listAct) {  menu.addAction(a); }
            //menu.setWindowOpacity(POPMENU_OPACITY); // no effect on MAC? on Windows cause blink
            act = menu.exec(QCursor::pos());
        }
    }
    // have selected a menu action, then make highlight off
    LIST_SELECTED(listMarker, names[2]-1, false);
    LIST_SELECTED(listLabelSurf, names[2]-1, false);
    LIST_SELECTED(listNeuronTree, names[2]-1, false);
    LIST_SELECTED(listCell, names[2]-1, false);
    int update = 0; // Whether need update display after return
#define __processing_menu_actions__ // dummy, just for easy locating
    // processing menu actions /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (act==0) 	return 0; // 081215: fix pop dialog when no choice of menu
    else if (act == actProperty || act == actEditNameComment)
    {
        //if (w)  w->annotationDialog(names[0], names[1], names[2]);
        editSurfaceObjAnnotation(names[0], names[1], names[2]); // directly call
    }
    else if (act == actShowFullPath)
    {
        //注释了
        v3d_msg(QString("The full path=[").append(w->data_title).append("]"));
    }
    else if (act == actVolColormap)
    {
        if (w)  w->volumeColormapDialog();
    }
    else if (act == actObjectManager)
    {
        int curTab = names[1]-1;
        if (w)  w->surfaceSelectDialog(curTab);
    }
    else if (act == actOff)
    {
        update = 1;
        switch (names[1])
        {
            case stLabelSurface:
                LIST_ON(listLabelSurf, names[2]-1, false);
                break;
            case stNeuronStructure:
                LIST_ON(listNeuronTree, names[2]-1, false);
                break;
            case stPointCloud:
                LIST_ON(listCell, names[2]-1, false);
                break;
            case stImageMarker:
                LIST_ON(listMarker, names[2]-1, false);
                break;
        }
    }
    else if (act == actColor)
    {
        qDebug()<<"20211021===========1";
        update = 1;
        switch (names[1])
        {
            case stLabelSurface:
                LIST_COLOR(listLabelSurf, names[2]-1, w);
                break;
            case stNeuronStructure:
                LIST_COLOR(listNeuronTree, names[2]-1, w);
                break;
            case stPointCloud:
                LIST_COLOR(listCell, names[2]-1, w);
                break;
            case stImageMarker:
                LIST_COLOR(listMarker, names[2]-1, w);
         //ALSO UPDATE THE TRIVIEW's marker color //by PHC 20140626
#ifndef test_main_cpp
        {
            My4DImage* image4d = v3dr_getImage4d(_idep);
            qDebug()<<"20211021===========2";
            if (image4d)
            {
                qDebug()<<"20211021===========3";
                if (names[2]-1>=0 && names[2]-1<listMarker.size())
                {
                    qDebug()<<"20211021===========4";
                    RGBA8 cc = listMarker.at(names[2]-1).color;
                    LocationSimple *s = (LocationSimple *)(&(image4d->listLandmarks.at(names[2]-1)));
                    s->color = cc;
                }
            }
        }
#endif

                break;
        }
        curImg->proj_trace_history_append();
    }
#ifndef test_main_cpp    //140211
    else if (act == actSaveSurfaceObj)
    {
        switch (names[1])
        {
            case stLabelSurface:
                saveSurfFile();
                break;
            case stNeuronStructure:
                saveNeuronTree(names[2]-1, ""); //use "" an empty string to force open a file dialog
                break;
            case stPointCloud:
                saveCellAPO(""); //use "" an empty string to force open a file dialog
                break;
            case stImageMarker:
            {
                QString curFile = "";
                if (curImg)
                {
                    curFile = QFileDialog::getSaveFileName(0,
                                                               "Select a text (CSV format with .marker extension) file to save the coordinates of landmark points... ",
                       (curImg->getXWidget())? curImg->getXWidget()->getOpenFileNameLabel()+".marker" : ".marker",
                                                                "");
                    if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
                    return update;
                }
                saveLandmarks_to_file(curFile); //use "" an empty string to force open a file dialog
                break;
            }
        }
    }
#endif

#ifndef test_main_cpp    //140211

    else if (act == actAddtoMarkerPool) //ZJL
    {
        if (w && curImg)
        {
            int tmpind = names[2]-1;
            if (tmpind>=0)
            {
                if (tmpind<curImg->last_hit_landmark) //in this case shift the last hit forward
                    curImg->last_hit_landmark--;
                else if (tmpind==curImg->last_hit_landmark) //in this case remove the last hit
                    curImg->last_hit_landmark = -1;
                // otherwise do nothing, - the last hit pos will not change
                LocationSimple mk = curImg->listLandmarks.at(tmpind); //get the specified landmark
                // add to listCurveMarkerPool
                listCurveMarkerPool.push_back(mk);
            }
        }
    }
#endif

    else if (act == actClearMarkerPool) //ZJL
    {
        listCurveMarkerPool.clear();
    }

#ifndef test_main_cpp    //140211

    else if (act==actMarkerZoomin3D_terafly) // by PHC, 20130417
    {
        if (w && curImg)
        {
            int tmpind = names[2]-1;
            if (tmpind>=0)
            {
                LocationSimple mk = curImg->listLandmarks.at(tmpind); //get the specified landmark

                v3d_msg("Invoke terafly local-zoomin based on an existing marker.", 0);
                vector <XYZ> loc_vec;
                XYZ loc; loc.x = mk.x; loc.y = mk.y; loc.z = mk.z;
                loc_vec.push_back(loc);

                b_grabhighrez = true;
                produceZoomViewOf3DRoi(loc_vec,
                                       0  //one means from non-wheel event
                                       );
            }
        }
    }
#endif

#ifndef test_main_cpp
    else if (act == actLockSceneEditObjGeometry)
    {
        finishEditingNeuronTree(); //090928
        editSurfaceObjBasicGeometry(names[0], names[1], names[2]);
    }
#endif //test_main_cpp
#define __create_curve__ // dummy, just for easy locating
    // real operation in selectObj() waiting next click
    else if (act == actCurveCreate1)
    {
        selectMode = smCurveCreate1;
        b_addthiscurve = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        total_etime = 0; //reset the timer
    }
    else if (act == actCurveCreate2)
    {
        selectMode = smCurveCreate2;
        b_addthiscurve = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    else if (act == actCurveCreate3)
    {
        selectMode = smCurveCreate3;
        b_addthiscurve = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    else if (act == actCurveCreate_pointclick)
    {
        selectMode = smCurveCreate_pointclick;
        b_addthiscurve = true;
        cntCur3DCurveMarkers=0; //reset
        //if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        if (w) { editinput = 7; oldCursor = w->cursor(); w->setCursor(QCursor(Qt::CrossCursor)); }
        total_etime = 0; //reset the timer
    }

#ifndef test_main_cpp //140211
    else if (act == actCurveCreateMarkerGD)
    {
        selectMode = smCurveCreateMarkerGD;
        b_addthiscurve = true;
        if (QMessageBox::question(0, "", "Do you want to use a 1-right stroke to define a bounding box? Default bounding box is the whole image.", QMessageBox::Yes, QMessageBox::No)
                == QMessageBox::No)
        {
            if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::WaitCursor)); }
            bool b_boundingbox = false;
            solveCurveFromMarkersGD(b_boundingbox);
            // clear listCurveMarkerPool
            listCurveMarkerPool.clear();
            endSelectMode();
        }
        else
        {
            // use one stroke to define a curve and then get a custmized bounding box
            if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        }
    }
#endif
    else if (act == actCurveCreate_pointclick_fm)
    {
        selectMode = smCurveCreate_pointclick_fm;
        b_addthiscurve = true;
        cntCur3DCurveMarkers=0; //reset
        //if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::CrossCursor)); }
    }
#ifndef test_main_cpp    //140211
    else if (act == actCurveMarkerPool_fm) // using fastmarching to trace curve through a marker pool
    {//ZJL
        selectMode = smCurveMarkerPool_fm;
        b_addthiscurve = true;
        //if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::CrossCursor)); }
        // solve Curve
        solveCurveFromMarkersFastMarching();
        // delete markers used for curve drawing
        // for(int i=0; i<curImg->listLandmarks.size(); i++)
        // {
        //      for(int j=0; j<listCurveMarkerPool.size(); j++)
        //      {
        //           if(listCurveMarkerPool.at(j).name == curImg->listLandmarks.at(i).name)
        //           {
        //                curImg->listLandmarks.removeAt(i);
        //                listMarker.removeAt(i);
        //           }
        //      }
        // }
        // clear listCurveMarkerPool
        listCurveMarkerPool.clear();
        endSelectMode();
    }
#endif
    else if (act == actCurveMarkerLists_fm) // 20120124 ZJL
    {
        selectMode = smCurveMarkerLists_fm;
        b_addthiscurve = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        total_etime = 0; //reset the timer
    }
    else if (act == actCurveTiltedBB_fm) // 20120124 ZJL
    {
        selectMode = smCurveTiltedBB_fm;
        b_addthiscurve = true;
        if (w) { editinput = 5; oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        total_etime = 0; //reset the timer
    }
    else if (act == actCurveTiltedBB_fm_sbbox) // 20120124 ZJL
    {
        selectMode = smCurveTiltedBB_fm_sbbox;
        b_addthiscurve = true;
        if (w) { editinput = 1; oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        total_etime = 0; //reset the timer
    }
    else if (act == actMarkerCreate1Stroke) // 20121011 PHC
    {
        selectMode = smMarkerCreate1Curve;
        b_addthiscurve = false;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        total_etime = 0; //reset the timer
    }
#ifndef test_main_cpp    //140211
    else if (act == actCurveFrom1Marker_fm) // 20120328 ZJL
    {
        selectMode = smCurveFrom1Marker_fm;
        b_addthiscurve = true;
        if (w && curImg)
        {
            int tmpind = names[2]-1;
            if (tmpind>=0)
            {
                if (tmpind<curImg->last_hit_landmark) //in this case shift the last hit forward
                    curImg->last_hit_landmark--;
                else if (tmpind==curImg->last_hit_landmark) //in this case remove the last hit
                    curImg->last_hit_landmark = -1;
                LocationSimple mk = curImg->listLandmarks.at(tmpind); //get the specified landmark
                curveStartMarker.x= mk.x;  curveStartMarker.y = mk.y; curveStartMarker.z= mk.z;
            }
        }
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
#endif
    else if (act == actCurveCreateTest) // 20120124 ZJL
    {
        selectMode = smCurveCreateTest;
        b_addthiscurve = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    else if (act == actCurveCreate_zoom)
    {
        selectMode = smCurveCreate1;
        b_addthiscurve = false;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    else if (act == actCurveRefine) // 110831 ZJL
    {
        selectMode = smCurveRefineInit;
        b_addthiscurve = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    else if (act == actCurveRefine_fm) // 120223 ZJL
    {
        selectMode = smCurveRefine_fm;
        b_addthiscurve = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    else if (act == actCurveDirectionInter) // 20120124 ZJL
    {
        selectMode = smCurveDirectionInter;
        b_addthiscurve = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
#define __create_marker__ // dummy, just for easy locating
    // real operation in selectObj() waiting next click
    else if (act == actMarkerCreate1)
    {
        selectMode = smMarkerCreate1;
        b_addthismarker = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::CrossCursor)); }
        total_etime = 0; //reset the timer
    }
    else if (act == actMarkerCreate2)
    {
        selectMode = smMarkerCreate2;
        b_addthismarker = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::CrossCursor)); }
    }
    else if (act == actMarkerCreate3)
    {
        selectMode = smMarkerCreate3;
        b_addthismarker = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::CrossCursor)); }
    }
    else if (act == actMarkerRefineT)
    {
        selectMode = smMarkerRefineT;
        b_addthismarker = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::CrossCursor)); }
    }
    else if (act == actMarkerRefineC)
    {
        selectMode = smMarkerRefineC;
        b_addthismarker = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::CrossCursor)); }
    }
    else if (act == actMarkerRefineLocal)
    {
        refineMarkerLocal(names[2]-1);
        b_addthismarker = true;
    }
    else if (act == actMarkerCreate_zoom)
    {
        selectMode = smMarkerCreate1;
        b_addthismarker = false;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
#ifndef test_main_cpp
#define __v3d_googleearth_databrowsing__ // dummy, just for easy locating
    else if (act == actMarkerCreate_zoom_grabhighrezdata) //for grabbing large data from existing files . by PHC 20120712
    {
        selectMode = smMarkerCreate1;
        b_addthismarker = false;
        b_imaging = false;
        b_grabhighrez = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    else if (act == actCurveCreate_zoom_grabhighrezdata)
    {
        selectMode = smCurveCreate1;
        b_addthiscurve = false;
        b_imaging = false;
        b_grabhighrez = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    else if (act == actZoomin_currentviewport)
    {
        zoomview_currentviewport();
    }

#define __v3d_imaging_func__ // dummy, just for easy locating
    else if (act == actMarkerCreate_zoom_imaging)
    {
        selectMode = smMarkerCreate1;
        b_addthismarker = false;
        b_imaging = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    else if (act == actCurveCreate_zoom_imaging)
    {
        selectMode = smCurveCreate1;
        b_addthiscurve = false;
        b_imaging = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    //added by Hanchuan Peng, 120506.
    else if (act == act_imaging_pinpoint_n_shoot)
    {
        selectMode = smMarkerCreate1;
        b_addthismarker = true;
        b_imaging = false;
        b_ablation = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    else if (act == act_imaging_cut_3d_curve)
    {
        selectMode = smCurveCreate1;
        b_addthiscurve = true;
        b_imaging = false;
        b_ablation = true;
        b_lineAblation = true;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    else if (act == actMarkerAblateOne_imaging || act == actMarkerAblateAll_imaging ||
                act == actMarkerAblateOne_imaging_nocalib || act == actMarkerAblateAll_imaging_nocalib ||
                act == actMarkerAblateOne_imaging_feedback || act == actMarkerAblateAll_imaging_feedback)
    {
        if (w && curImg && curXWidget)
        {
            v3d_imaging_paras myimagingp;
            if (act == actMarkerAblateOne_imaging || act == actMarkerAblateAll_imaging)
                myimagingp.OPS = "Marker Ablation from 3D Viewer";
            else if(act == actMarkerAblateOne_imaging_nocalib || act == actMarkerAblateAll_imaging_nocalib)
                myimagingp.OPS = "Marker Ablation from 3D Viewer (no Calibration)";
            else if(act == actMarkerAblateOne_imaging_feedback || act == actMarkerAblateAll_imaging_feedback)
                myimagingp.OPS = "Marker Ablation from 3D Viewer (with feedback)";
            myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call
            bool doit = (curImg->listLandmarks.size()>0) ? true : false;
            if (doit && (act == actMarkerAblateAll_imaging || act == actMarkerAblateAll_imaging_nocalib || act == actMarkerAblateAll_imaging_feedback) )
                myimagingp.list_landmarks = curImg->listLandmarks;
            else // act == actMarkerAblateOne_imaging
            {
                int tmpind = names[2]-1;
                if (tmpind>=0)
                    myimagingp.list_landmarks << curImg->listLandmarks.at(tmpind);
                else
                    doit = false;
            }
            //do imaging
            if (doit)
            {
                //set the hiddenSelectWidget for the V3D mainwindow
                if (curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
                {
                    v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
                }
                else
                {
                    v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
                }
            }
        }
    }
    else if (act == actMarkerOne_imaging || act == actMarkerAll_imaging) // marker imaging
    {
        if (w && curImg && curXWidget)
        {
            v3d_imaging_paras myimagingp;
            myimagingp.OPS = "Acquisition: Markers from 3D Viewer";
            myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call
            bool doit = (curImg->listLandmarks.size()>0) ? true : false;
            if (doit && act == actMarkerAll_imaging)
                myimagingp.list_landmarks = curImg->listLandmarks;
            else // act == actMarkerOne_imaging
            {
                int tmpind = names[2]-1;
                if (tmpind>=0)
                    myimagingp.list_landmarks << curImg->listLandmarks.at(tmpind);
                else
                    doit = false;
            }
            //do imaging
            if (doit)
            {
                //set the hiddenSelectWidget for the V3D mainwindow
                if (curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
                {
                    v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
                }
                else
                {
                    v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
                }
            }
        }
    }
    else if (act == actMarkerAutoSeed)
    {
        if (w && curImg)
        {
            bool ok1=true;
            V3DLONG chno=1;
            if (curImg->getCDim()>1)
#if defined(USE_Qt5)
                chno = QInputDialog::getInt(0, QString("select a channel"), QString("select a channel of image you'd apply AutoMarker to:"), 1, 1, int(curImg->getCDim()), 1, &ok1);
#else
                chno = QInputDialog::getInt(0, QString("select a channel"), QString("select a channel of image you'd apply AutoMarker to:"), 1, 1, int(curImg->getCDim()), 1, &ok1);
#endif
            if (ok1)
            {
                //QList <LocationSimple> rlist = curImg->autoMarkerFromImg(chno-1, this->dataBox, this->thicknessZ); //change to the following 091113, PHC
                //				qDebug()<<"viewproc box:"<<this->dataViewProcBox.x0<<" "<<this->dataViewProcBox.x1<<" "<<
                //					this->dataViewProcBox.y0<<" "<<this->dataViewProcBox.y1<<" " <<
                //					this->dataViewProcBox.z0<<" "<<this->dataViewProcBox.z1<<" ";
                QList <LocationSimple> rlist = curImg->autoMarkerFromImg(chno-1, this->dataViewProcBox, this->thicknessZ);
                curImg->listLandmarks <<(rlist);
                updateLandmark(); //update the landmark list in 3D viewer.
            }
        }
    }

#define __vaa3d_neuron2_auto_tracing__ // dummy, just for easy locating
    else if (act == actVaa3DNeuron2App2)
    {
        if (w && curImg)
        {
            v3d_msg("Enter the neuron tracing module APP2.", 0);
            v3d_imaging_paras myimagingp;
            myimagingp.OPS = "Vaa3D-Neuron2-APP2";
            myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call

            //set the hiddenSelectWidget for the V3D mainwindow
            if (curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
            {
                v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
            }
            else
            {
                v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
            }
        }
    }

    else if (act == actRetrace)
       {
   //        QTableWidget *tableWidget = new QTableWidget(1,1);
   //        QPushButton *button = new QPushButton("ok");
   //        tableWidget->setCellWidget(1,1,button);
   //        combobox = new QComboBox();
   //        combobox->resize(200,200);
   //        combobox->setWindowTitle("Retrace");
   //        QStringList strList;
   //        strList<<"Retrace"<<"app2Convenient"<<"app2Terafly"<<"app2MultiTerafly"<<"app2TeraflyWithPara"<<"app2MultiTeraflyWithPara";
   //        combobox->addItems(strList);



   //        tableWidget->setCellWidget(0,0,combobox);
   //        connect(button,SIGNAL(clicked(bool)),this,SLOT(pressBtn()));

   //        QString name = combobox->currentText();
   //        qDebug()<<name;



           if (w && curImg)
           {
               v3d_msg("Enter the neuron tracing module APP2.", 0);
               v3d_imaging_paras myimagingp;
               myimagingp.OPS = "Retrace";
               myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call

               //set the hiddenSelectWidget for the V3D mainwindow
               if (curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
               {
                   v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
               }
               else
               {
                   v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
               }
           }
       }



       else if (act == app2Convenient)
       {
           if (w && curImg)
           {
               v3d_msg("app2Convenient", 0);
               v3d_imaging_paras myimagingp;
               myimagingp.OPS = "app2Convenient";
               myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call

               //set the hiddenSelectWidget for the V3D mainwindow
               if (curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
               {
                   v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
               }
               else
               {
                   v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
               }
           }
       }

       else if (act == app2MultiTerafly)
       {
           if (w && curImg)
           {
               v3d_msg("app2MultiTerafly", 0);
               v3d_imaging_paras myimagingp;
               myimagingp.OPS = "app2MultiTerafly";
               myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call

               //set the hiddenSelectWidget for the V3D mainwindow
               if (curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
               {
                   v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
               }
               else
               {
                   v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
               }
           }
       }

       else if (act == app2MultiTeraflyWithPara)
       {
           if (w && curImg)
           {
               v3d_msg("app2MultiTeraflyWithPara", 0);
               v3d_imaging_paras myimagingp;
               myimagingp.OPS = "app2MultiTeraflyWithPara";
               myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call

               //set the hiddenSelectWidget for the V3D mainwindow
               if (curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
               {
                   v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
               }
               else
               {
                   v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
               }
           }
       }

       else if (act == app2Terafly)
       {
           if (w && curImg)
           {
               v3d_msg("app2Terafly", 0);
               v3d_imaging_paras myimagingp;
               myimagingp.OPS = "app2Terafly";
               myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call

               //set the hiddenSelectWidget for the V3D mainwindow
               if (curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
               {
                   v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
               }
               else
               {
                   v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
               }
           }
       }
       else if (act == app2TeraflyWithPara)
       {
           if (w && curImg)
           {
               v3d_msg("app2TeraflyWithPara", 0);
               v3d_imaging_paras myimagingp;
               myimagingp.OPS = "app2TeraflyWithPara";
               myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call

               //set the hiddenSelectWidget for the V3D mainwindow
               if (curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
               {
                   v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
               }
               else
               {
                   v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
               }
           }
       }

#define __vaa3d_gd_curveline_tracing__ // dummy, just for easy locating //by PHC 20170529
    else if (act == actGDCurveline)
    {
        if (w && curImg)
        {
            v3d_msg("Now try to use the GD curveline detection feature.", 0);
            v3d_imaging_paras myimagingp;
            myimagingp.OPS = "GD Curveline";
            myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call

            //set the hiddenSelectWidget for the V3D mainwindow
            if (curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
            {
                v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
            }
            else
            {
                v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
            }
        }
    }




#define __actions_of_marker__ // dummy, just for easy locating
    else if (act == actMarkerAltRotationCenter)
    {
        if (w && curImg)
        {
            int tmpind = names[2]-1;
            if (tmpind>=0 && tmpind<listMarker.size())
            {
                const ImageMarker & m = listMarker.at(tmpind);
                XYZ p = XYZ(m);

                p.x = p.x*thicknessX -start1;
                p.y = p.y*thicknessY -start2;
                p.z = p.z*thicknessZ -start3;

                BoundingBox & BB = boundingBox;
                float DX = BB.Dx();
                float DY = BB.Dy();
                float DZ = BB.Dz();
                float maxD = BB.Dmax();
                double s[3];
                s[0] = 1/maxD *2;
                s[1] = 1/maxD *2;
                s[2] = 1/maxD *2;
                double t[3];
                t[0] = -BB.x0 -DX /2;
                t[1] = -BB.y0 -DY /2;
                t[2] = -BB.z0 -DZ /2;

                p.x = s[0]*(p.x +t[0]);
                p.y = s[1]*(p.y +t[1]);
                p.z = s[2]*(p.z +t[2]);

                qDebug("normalized alt rotation center: (%f %f %f)", p.x, p.y, p.z);

                w->setAltCenter(p.x, p.y, p.z);
            }
        }
    }
    else if (act == actMarkerDelete)
    {
        if (w && curImg)
        {
            bool ok = true; //QMessageBox::warning(0, QObject::tr("3D View"), QObject::tr("Are you sure to delete this marker?"),
            //                     QMessageBox::Yes | QMessageBox::Cancel,   QMessageBox::Yes)	== QMessageBox::Yes;
            int tmpind = names[2]-1;
            if (tmpind>=0 && ok)
            {
                if (tmpind<curImg->last_hit_landmark) //in this case shift the last hit forward
                    curImg->last_hit_landmark--;
                else if (tmpind==curImg->last_hit_landmark) //in this case remove the last hit
                    curImg->last_hit_landmark = -1;
                // otherwise do nothing, - the last hit pos will not change
                curImg->listLandmarks.removeAt(tmpind); //remove the specified landmark
                //updateLandmark(); //update the landmark list in 3D viewer. Commented as this is too expensive, use the following cheap way
                listMarker.removeAt(tmpind);
            }
        }
    }
    else if (act == actMarkerClearAll)
    {
        if (w && curImg)
        {
            bool ok = QMessageBox::warning(0, QObject::tr("3D View"), QObject::tr("Are you sure to clear ALL markers?"),
                    QMessageBox::Yes | QMessageBox::Cancel,   QMessageBox::Cancel)	== QMessageBox::Yes;
            if (ok)
            {
                curImg->last_hit_landmark = -1;
                curImg->listLandmarks.clear();
                listMarker.clear();
            }
        }
    }
    else if (act == actMarkerMoveToMiddleZ)
    {
        if (w && curImg)
        {
            V3DLONG newz = curImg->getZDim()/2;
            for (V3DLONG i=0; i < curImg->listLandmarks.size(); i++)
            {
                ImageMarker & im = listMarker[i];
                LocationSimple & ls = curImg->listLandmarks[i];
                im.z = ls.z = newz;
            }
        }
    }
    else if (act == actMarkerLabelAsStartPos)
    {
        if (w && curImg) curImg->last_hit_landmark = names[2]-1;
    }
    else if (act == actMarkerTraceFromOnePos)
    {
        if (w && curImg)
        {
            curImg->cur_hit_landmark = names[2]-1;
            qDebug()<<"viewproc box:"<<this->dataViewProcBox.x0<<" "<<this->dataViewProcBox.x1<<" "<<
                this->dataViewProcBox.y0<<" "<<this->dataViewProcBox.y1<<" " <<
                this->dataViewProcBox.z0<<" "<<this->dataViewProcBox.z1<<" ";
            curImg->trace_bounding_box = this->dataViewProcBox; //dataBox
            curImg->trace_z_thickness = this->thicknessZ;
            if (curImg->proj_trace_deformablepath_one_point(curImg->cur_hit_landmark)==true)
            {
                curImg->update_3drenderer_neuron_view(w, this);
            }
        }
    }
    else if (act == actMarkerTraceFromOnePosToOtherMarkers)
    {
        if (w && curImg)
        {
            curImg->cur_hit_landmark = names[2]-1;
            //			qDebug()<<"viewproc box:"<<this->dataViewProcBox.x0<<" "<<this->dataViewProcBox.x1<<" "<<
            //			this->dataViewProcBox.y0<<" "<<this->dataViewProcBox.y1<<" " <<
            //			this->dataViewProcBox.z0<<" "<<this->dataViewProcBox.z1<<" ";
            curImg->trace_bounding_box = this->dataViewProcBox; //dataBox
            curImg->trace_z_thickness = this->thicknessZ;
            if (curImg->proj_trace_deformablepath_one_point_to_allotherpoints(curImg->cur_hit_landmark)==true)
            {
                curImg->update_3drenderer_neuron_view(w, this);
            }
        }
    }
    else if (act == actMarkerTraceFromStartPos || act == actMarkerTraceFromStartPos_selPara || act==actMarkerLineProfileFromStartPos || act==actMarkerLineProfileFromStartPos_drawline)
    {
        if (w && curImg)
        {
            curImg->cur_hit_landmark = names[2]-1;
            curImg->trace_bounding_box = this->dataViewProcBox; //dataBox;
            curImg->trace_z_thickness = this->thicknessZ;
            if (curImg->last_hit_landmark != curImg->cur_hit_landmark)
            {
                if (act==actMarkerTraceFromStartPos)
                {
                    bool b_select_para = false; bool b_fitradius=true; int method_code = 0; //0 for shortest path and 1 for deformable model
                    if (curImg->proj_trace_deformablepath_two_points(curImg->last_hit_landmark, curImg->cur_hit_landmark, b_select_para, method_code)==true)
                    {
                        curImg->update_3drenderer_neuron_view(w, this);
                    }
                }
                else  if (act == actMarkerTraceFromStartPos_selPara) //in this case select parameters and then trace
                {
                    bool b_select_para = true; bool b_fitradius=true; int method_code=0;
                    if (curImg->proj_trace_deformablepath_two_points(curImg->last_hit_landmark, curImg->cur_hit_landmark, b_select_para, method_code)==true)
                    {
                        curImg->update_3drenderer_neuron_view(w, this);
                    }
                }
                else if (act==actMarkerLineProfileFromStartPos_drawline || act == actMarkerLineProfileFromStartPos)
                {
                    if (act==actMarkerLineProfileFromStartPos_drawline)
                    {
                        int npoints = 1;
                        bool b_select_para = false; bool b_fitradius=false; int method_code=1; //1 for deformable model and 0 for shortest path
                        if (curImg->proj_trace_deformablepath_two_points(curImg->last_hit_landmark, curImg->cur_hit_landmark, npoints, b_select_para, b_fitradius, method_code)==true)
                        {
                            curImg->update_3drenderer_neuron_view(w, this);
                        }
                    }
                    showLineProfile(curImg->last_hit_landmark, curImg->cur_hit_landmark);
                }
                //then reset the starting position location
                curImg->last_hit_landmark = -1;
            }
            else
            {
                qDebug("You input the same starting and ending landmarks for tracing, - do nothing. ");
            }
        }
    }
    else if (act==actMarkerZoomin3D) //by PHC, 090618.
    {
        if (w && curImg && curXWidget)
        {
            curImg->cur_hit_landmark = names[2]-1;
            curXWidget->doImage3DLocalMarkerView();
            //QTimer::singleShot( 1000, curXWidget, SLOT(doImage3DLocalView()) ); //090710 090714 RZC
        }
    }
    else if (act==actMarkerCreateNearestNeuronNode)
    {
        NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                    (NeuronTree *)(&(listNeuronTree.at(0))) :
                    (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

        if (p_tree)
        {
            double best_dist;
            V3DLONG n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);
            if (listNeuronTree.size()>1)
                qDebug("detect nearest neuron node [%d] for the [%d] neuron", n_id, names[2]-1);
            NeuronSWC cur_node;
            if (n_id>=0)
            {
                cur_node = p_tree->listNeuron.at(n_id);
                qDebug()<<"cur_node.x="<<cur_node.x<<" "<<"cur_node.y="<<cur_node.y<<" "<<"cur_node.z="<<cur_node.z;
                int ii;
                ImageMarker *p_marker=0; bool b_exist_marker=false;
                for (ii=0;ii<listMarker.size();ii++)
                {
                    p_marker = (ImageMarker *)(&(listMarker.at(ii)));
                    qDebug()<<ii<<" "<<p_marker->x<<" "<<p_marker->y<<" "<<p_marker->z;
                    if (cur_node.x==p_marker->x && cur_node.y==p_marker->y && cur_node.z==p_marker->z)
                    {
                        b_exist_marker=true;
                        break;
                    }
                }
                if (b_exist_marker) {qDebug("you select an existing marker [%d], - do nothing.", ii+1);}
                else
                {
                    XYZ loc(cur_node.x, cur_node.y, cur_node.z);
                    addMarker(loc);
                }
            }
        }
    }
#define __actions_of_neuron__ // dummy, just for easy locating
    else if (act==actNeuronToEditable || act==actDecomposeNeuron)
    {
        if (NEURON_CONDITION)
        {
            if (listNeuronTree.size()>1 && 0)
            {
                v3d_msg("The neuron editing mode allows ONLY ONE neuron at a time, and there is NO existing traced neuron fromthe current image.");
            }
            else
            {
                listNeuronTree_old = listNeuronTree;

                NeuronTree *p_tree = 0;

                if (listNeuronTree.size()==1)
                {
                    p_tree = (NeuronTree *)(&(listNeuronTree.at(0)));
                    curEditingNeuron = 1;
                    realCurEditingNeuron_inNeuronTree = curEditingNeuron-1; //keep an index of the real neuron being edited. Note that curEditingNeuron can be changed later during editing
                }
                else
                {
                    p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
                    curEditingNeuron = names[2];
                    realCurEditingNeuron_inNeuronTree = curEditingNeuron-1; //keep an index of the real neuron being edited. Note that curEditingNeuron can be changed later during editing
                    qDebug() << "names[2]=" << names[2] << " p_tree=" << p_tree;
                }

                curImg->tracedNeuron_old = curImg->tracedNeuron; //150523, by PHC
                //v3d_msg(QString("before editing current traceNeuron.nseg=%1 traceNeuron_old.nseg=%2").arg(curImg->tracedNeuron.nsegs()).arg(curImg->tracedNeuron_old.nsegs()));

                if (listNeuronTree[realCurEditingNeuron_inNeuronTree].name!="vaa3d_traced_neuron" ||
                        listNeuronTree[realCurEditingNeuron_inNeuronTree].file!="vaa3d_traced_neuron")
                {
                    b_editDroppedNeuron = true;
                }

                curImg->tracedNeuron = copyToEditableNeuron(p_tree);
                curImg->tracedNeuron.name = "vaa3d_traced_neuron";
                curImg->tracedNeuron.file = "vaa3d_traced_neuron";
                listNeuronTree.clear();
                qDebug("	listNeuronTree.size() = %d!!!!!", listNeuronTree.size());

                //v3d_msg(QString("after copy current traceNeuron.nseg=%1").arg(curImg->tracedNeuron.nsegs()));
                curImg->proj_trace_history_append();
                curImg->update_3drenderer_neuron_view(w, this);
            }
        }
    }
    else if (act==actNeuronFinishEditing)
    {
        if (NEURON_CONDITION)
        {
            //150523. There seems to be some remaining bugs that need to be fixed later when multiple neurons are presented.
            if (w && curImg && curXWidget)
            {
                if (listNeuronTree.size()==1 && listNeuronTree[0].name=="vaa3d_traced_neuron" && listNeuronTree[0].file=="vaa3d_traced_neuron")
                {
                    //do nothing
                }

                if (b_editDroppedNeuron || listNeuronTree_old.size()>=2)
                {
                    if (realCurEditingNeuron_inNeuronTree>=0 && realCurEditingNeuron_inNeuronTree<listNeuronTree_old.size())
                    {
                        NeuronTree newtree = V_NeuronSWC_list__2__NeuronTree(curImg->tracedNeuron);
                        listNeuronTree = listNeuronTree_old;
                        listNeuronTree.replace(realCurEditingNeuron_inNeuronTree, newtree);

                        NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(realCurEditingNeuron_inNeuronTree)));
                        p_tree->name = listNeuronTree_old.at(realCurEditingNeuron_inNeuronTree).name;
                        p_tree->file = listNeuronTree_old.at(realCurEditingNeuron_inNeuronTree).file;
                    }

                    curImg->tracedNeuron = curImg->tracedNeuron_old; //150523, by PHC
                    if (curImg->tracedNeuron.nsegs()>0) //need to add to the original list in this case
                    {
                        curImg->update_3drenderer_neuron_view(w, this);
                    }

                    b_editDroppedNeuron = false;
                }
            }

            for (int i=0; i<curImg->tracedNeuron.seg.size(); i++)
            {curImg->tracedNeuron.seg[i].on = true;}
            curImg->update_3drenderer_neuron_view(w, this);

            finishEditingNeuronTree();
        }
    }
    else if (act==actDispRecNeuronSegInfo)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            V3DLONG n_id;
            double best_dist;
            if (p_tree)	{n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);}
            if (n_id>=0)
            {
                double d = curImg->proj_trace_measureLengthNeuronSeg(n_id, p_tree);
                QString tmpstr="The length of current segment is ", ts2;
                ts2.setNum(d);
                tmpstr += ts2 + "<br>";
                qDebug() << "length of current segment is "<<d;
                QMessageBox::information(0, "Morphology measure", tmpstr);
            }
        }
    }
    else if (act==actChangeNeuronSegType)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            V3DLONG n_id;
            if (p_tree)	{double best_dist; n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);}
            if (n_id>=0)
            {
                curImg->proj_trace_changeNeuronSegType(n_id, p_tree);
                curImg->update_3drenderer_neuron_view(w, this);
            }
        }
    }
    else if (act==actChangeMultiNeuronSegType) // Zhi Zhou
   {
       if (NEURON_CONDITION)
       {
              selectMode = smRetypeMultiNeurons;
              b_addthiscurve = false;
              if (w) { editinput = 2; oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
       }
   }
    else if (act==actBreakMultiNeuronSeg) // Zhi Zhou
   {
       if (NEURON_CONDITION)
       {
              selectMode = smBreakMultiNeurons;
              b_addthiscurve = false;
              if (w) { editinput = 4; oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
       }
   }
    else if (act==actChangeNeuronSegRadius)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            V3DLONG n_id;
            if (p_tree)	{double best_dist; n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);}
            if (n_id>=0)
            {
                curImg->proj_trace_changeNeuronSegRadius(n_id, p_tree);
                curImg->update_3drenderer_neuron_view(w, this);
            }
        }
    }
    // ZJL 110913
    else if (act==actCurveEditRefine || act==actCurveEditRefine_fm)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            V3DLONG n_id;
            if (p_tree)	{double best_dist; n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);}
            if (n_id>=0)
            {
                // using the pipeline of "n-right-strokes to define a curve (refine)"
                if (act==actCurveEditRefine)
                    selectMode = smCurveEditRefine;
                else
                    selectMode = smCurveEditRefine_fm;
                b_addthiscurve = true;
                if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
                // get seg_id and then using "n-right-strokes" refine pipeline
                edit_seg_id = p_tree->listNeuron.at(n_id).seg_id;
            }
        }
    }
    // ZJL 110921
    else if (act==actCurveRubberDrag)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            V3DLONG n_id;
            if (p_tree)	{double best_dist; n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);}
            if (n_id>=0)
            {
                selectMode = smCurveRubberDrag;
                if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
                // get seg_id and then using "n-right-strokes" refine pipeline
                edit_seg_id = p_tree->listNeuron.at(n_id).seg_id;
            }
        }
    }
     else if (act==actDeleteMultiNeuronSeg) // ZJL, 20120806
    {
        if (NEURON_CONDITION)
        {
            selectMode = smDeleteMultiNeurons;
               b_addthiscurve = false;
               if (w) { editinput = 3; oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        }
    }
    else if (act==actDeleteNeuronSeg)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            V3DLONG n_id;
            if (p_tree)	{double best_dist; n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);}
            if (n_id>=0)
            {
                curImg->proj_trace_deleteNeuronSeg(n_id, p_tree);
                curImg->update_3drenderer_neuron_view(w, this);
            }
        }
    }
    /*** Segment functionalities. MK, 2017 April ***/
    else if (act == actNeuronConnect) //MK
    {
        if (NEURON_CONDITION)
        {
            selectMode = smConnectNeurons;
            b_addthiscurve = false;
            if (w) { editinput = 6; oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        }
    }
    else if (act == actPointCloudConnect)
    {
        selectMode = smConnectPointCloud;
        b_addthiscurve = false;
        if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
    }
    else if (act == actMarkerConnect)
    {
        if (NEURON_CONDITION)
        {
            selectMode = smConnectMarker;
            b_addthismarker = false;
            if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        }
    }
    else if (act == simpleConnect)
    {
        if (NEURON_CONDITION)
        {
            selectMode = smSimpleConnect;
            b_addthiscurve = false;
            if (w) { editinput = 6; oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        }
    }
    else if (act == simpleConnect_loopSafe)
    {
        if (NEURON_CONDITION)
        {
            selectMode = smSimpleConnectLoopSafe;
            b_addthiscurve = false;
            if (w) { editinput = 9; oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        }
    }
    /*************************************************/

    /*** Neuron cutting functionalities. MK, 2017 June ***/
    else if (act == actNeuronCut)
    {
        if (NEURON_CONDITION)
        {
            selectMode = smCutNeurons;
            b_addthiscurve = false;
            if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
        }
    }
    /*****************************************************/

    else if (act==actBreakNeuronSegNearestNeuronNode)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            if (p_tree)
            {
                double best_dist;
                V3DLONG n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);
                qDebug("detect nearest neuron node [%d] for the [%d] neuron", n_id, names[2]-1);
                NeuronSWC cur_node;
                if (n_id>=0)
                {
                    cur_node = p_tree->listNeuron.at(n_id);
                    qDebug()<<cur_node.x<<" "<<cur_node.y<<" "<<cur_node.z;
                    //					//first add an additional marker
                    //					int ii;
                    //					ImageMarker *p_marker=0; bool b_exist_marker=false;
                    //					for (ii=0;ii<listMarker.size();ii++)
                    //					{
                    //						p_marker = (ImageMarker *)(&(listMarker.at(ii)));
                    //						qDebug()<<ii<<" "<<p_marker->x<<" "<<p_marker->y<<" "<<p_marker->z;
                    //						if (cur_node.x==p_marker->x && cur_node.y==p_marker->y && cur_node.z==p_marker->z)
                    //						{
                    //							b_exist_marker=true;
                    //							break;
                    //						}
                    //					}
                    //					if (b_exist_marker) {qDebug("you select an existing marker [%d], - do nothing.", ii+1);}
                    //					else
                    //					{
                    //						XYZ loc(cur_node.x, cur_node.y, cur_node.z);
                    //						addMarker(loc);
                    //					}
                    //now break the seg
                    curImg->proj_trace_breakNeuronSeg(n_id, p_tree);
                    curImg->update_3drenderer_neuron_view(w, this);
                }
            }
        }
    }
    else if (act==actBreakNeuronSeg_markclick)
    {
        if (NEURON_CONDITION)
        {  //is the following code correct. Noted BY PHC, 150621.
            ImageMarker *c_pos = (ImageMarker *)(&listMarker.at(names[2]-1));
            NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(0)));
            V3DLONG n_id=-1;
            for (int ci=0;ci<p_tree->listNeuron.size();ci++)
            {
                if (p_tree->listNeuron.at(ci).x==c_pos->x &&
                        p_tree->listNeuron.at(ci).y==c_pos->y &&
                        p_tree->listNeuron.at(ci).z==c_pos->z
                   )
                    n_id=ci; //n_id==ci; //110814 RZC fixed by Eclipse indigo, but this act never be executed, so no effect.
            }
            if (n_id>=0)
            {
                curImg->proj_trace_breakNeuronSeg(n_id, p_tree);
                curImg->update_3drenderer_neuron_view(w, this);
            }
        }
    }
    else if (act==actJoinNeuronSegs_nearby_pathclick)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            V3DLONG n_id;
            if (p_tree)	{double best_dist; n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);}
            if (n_id>=0)
            {
                curImg->proj_trace_joinNearbyNeuronSegs_pathclick(n_id, p_tree);
                curImg->update_3drenderer_neuron_view(w, this);
            }
        }
    }
    else if (act==actJoinNeuronSegs_nearby_markclick)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            V3DLONG n_id;
            if (p_tree)	{double best_dist; n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);}
            if (n_id>=0)
            {
                curImg->proj_trace_joinNearbyNeuronSegs_markclick(n_id, p_tree);
                curImg->update_3drenderer_neuron_view(w, this);
            }
        }
    }
    else if (act==actJoinNeuronSegs_all)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            V3DLONG n_id;
            if (p_tree)	{double best_dist; n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);}
            if (n_id>=0)
            {
                curImg->proj_trace_joinAllNeuronSegs(n_id, p_tree);
                curImg->update_3drenderer_neuron_view(w, this);
                //V_NeuronSWC merged_neuron = curImg->tracedNeuron.merge(); updateNeuronTree(merged_neuron);
            }
        }
    }
    else if (act==actNeuronSegDeform)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            V3DLONG n_id;
            if (p_tree)	{double best_dist; n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);}
            if (n_id>=0)
            {
                curImg->proj_trace_deformNeuronSeg(n_id, p_tree);
                curImg->update_3drenderer_neuron_view(w, this);
            }
        }
    }
    else if (act==actNeuronSegProfile)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            V3DLONG n_id;
            if (p_tree)	{double best_dist; n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);}
            if (n_id>=0)
            {
                curImg->proj_trace_profileNeuronSeg(n_id, p_tree, true);
                curImg->update_3drenderer_neuron_view(w, this);
            }
        }
    }
    else if (act==actNeuronOneSegMergeToCloseby)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            V3DLONG n_id;
            if (p_tree)	{double best_dist; n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);}
            if (n_id>=0)
            {
                curImg->proj_trace_mergeOneClosebyNeuronSeg(n_id, p_tree);
                curImg->update_3drenderer_neuron_view(w, this);
            }
        }
    }
    else if (act==actNeuronAllSegMergeToCloseby)
    {
        if (NEURON_CONDITION)
        {
            NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                        (NeuronTree *)(&(listNeuronTree.at(0))) :
                        (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

            curImg->proj_trace_mergeAllClosebyNeuronNodes(p_tree);
            curImg->update_3drenderer_neuron_view(w, this);
        }
    }
    else if (act==actDispNeuronNodeInfo)
    {
        NeuronTree *p_tree = (listNeuronTree.size()==1) ?
                    (NeuronTree *)(&(listNeuronTree.at(0))) :
                    (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));

        double best_dist;
        V3DLONG n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree, best_dist);
        QString tmpstr, tmpstr1;
        tmpstr.setNum(n_id); tmpstr.prepend("The neuron node has row index ");
        tmpstr.append("\n");
        tmpstr.append(info_NeuronNode(n_id, p_tree));
        QMessageBox::information(0, "neuron node info", tmpstr);
    }
    else if (act==actAveDistTwoNeurons)
    {
        if (listNeuronTree.size()<2)
            v3d_msg("only one neuron, nothing is computed");
        else
        {
            QString tmpstr, ts2;
            float ave_sd=0, ave_ssd=0, ave_ssd_percent=0;
            for (int ci=0;ci<listNeuronTree.size();ci++)
            {
                if (ci!=(names[2]-1))
                {
                    ts2.setNum(names[2]); ts2.prepend("dists between "); tmpstr += ts2;
                    ts2.setNum(ci+1); ts2.prepend(" and "); tmpstr += ts2;
                    NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(&(listNeuronTree.at(names[2]-1)), &(listNeuronTree.at(ci)),1);
                    ts2.setNum(tmp_score.dist_allnodes); ts2.prepend(" are <br> entire-structure-average = "); tmpstr += ts2 + "<br>";
                    ts2.setNum(tmp_score.dist_apartnodes); ts2.prepend(" different-structure-average = "); tmpstr += ts2 + "<br>";
                    ts2.setNum(tmp_score.percent_apartnodes); ts2.prepend(" percent of different-structure = "); tmpstr += ts2 + "<br>";
                    ts2.setNum(tmp_score.dist_max); ts2.prepend(" max distance of neurons' nodes = "); tmpstr += ts2 + "<br><br>";
                    qDebug() << "score between "<<names[2]
                             << " and "<<ci+1<< "="
                             << tmp_score.dist_allnodes << " "
                             << tmp_score.dist_apartnodes << " "
                             << tmp_score.percent_apartnodes << " "
                             << tmp_score.dist_max;
                    ave_sd += tmp_score.dist_allnodes;
                    ave_ssd += tmp_score.dist_apartnodes;
                    ave_ssd_percent += tmp_score.percent_apartnodes;
                }
            }

            if (listNeuronTree.size()>=2)
            {
                ave_sd /= float(listNeuronTree.size()-1);
                ave_ssd /= float(listNeuronTree.size()-1);
                ave_ssd_percent /= float(listNeuronTree.size()-1);
                ts2.setNum(names[2]); ts2.prepend("Average scores between "); tmpstr += ts2;
                tmpstr += " and the remaining neurons are <br>";
                ts2.setNum(ave_sd); ts2.prepend(" entire-structure-average = "); tmpstr += ts2 + "<br>";
                ts2.setNum(ave_ssd); ts2.prepend(" different-structure-average = "); tmpstr += ts2 + "<br>";
                ts2.setNum(ave_ssd_percent); ts2.prepend(" percent of different-structure = "); tmpstr += ts2 + "<br><br>";
                qDebug() << "score between "<<names[2]<< " and remaining neurons =" << ave_sd << " " << ave_ssd << " " << ave_ssd_percent;
            }
            QMessageBox::information(0, "neuron distance scores", tmpstr);
        }
    }
    else if (act==actDispNeuronMorphoInfo)
    {
        QString tmpstr = "Neuron ", ts2;
        ts2.setNum(names[2]);

        if (listNeuronTree.size()==1)
            tmpstr += ts2 + "<br>" + get_neuron_morpho_features_str(&(listNeuronTree.at(0)));
        else
            tmpstr += ts2 + "<br>" + get_neuron_morpho_features_str(&(listNeuronTree.at(names[2]-1)));

        QMessageBox::information(0, "neuron info", tmpstr);
        qDebug() << tmpstr;
    }
    else if (act==actDoNeuronToolBoxPlugin) //still testing. 20120413, PHC
    {
        vaa3d_neurontoolbox_paras* np = new vaa3d_neurontoolbox_paras;
        np->OPS = "Neuron Toolbox";
        np->nt = (listNeuronTree.size()==1) ? listNeuronTree.at(0) : listNeuronTree.at(names[2]-1);
        double best_dist;
        np->n_id = findNearestNeuronNode_WinXY(cx, cy, &np->nt, best_dist);
        np->win = (V3dR_MainWindow *)w->getMainWindow();
        printf("the main window pointer = [%p]\n", ((iDrawExternalParameter*)_idep)->V3Dmainwindow);
        doNeuronToolBoxPlugin(((iDrawExternalParameter*)_idep)->V3Dmainwindow, *np);
    }
    else if (act==actClearedAllGeneratedObjects)
    {
        if (NEURON_CONDITION)
        {
            curImg->tracedNeuron.seg.clear();
            curImg->update_3drenderer_neuron_view();
        }
    }
#endif //tes_main_cpp
#define __actions_of_label_surface__ // dummy, just for easy locating
    else if (act==actDispSurfVertexInfo)
    {
        int vertex_i=0;
        Triangle * T = findNearestSurfTriangle_WinXY(cx, cy, vertex_i, (Triangle*)list_listTriangle.at(names[2]-1));
        if (T!=NULL)
        {
            QString qsInfo = info_SurfVertex(vertex_i, T, listLabelSurf.at(names[2]-1).label);
            QMessageBox::information(0, "Surface Vertex", QString("The nearest surface vertex info:\n %1").arg(qsInfo));
        }
    }
    else if (act==actComputeSurfArea)
    {
        double a = computeSurfaceArea(names[0], names[1], names[2]);
        if (a>0)
            QMessageBox::information(0, "Surface Area", QString("The surface area is: %1").arg(a));
    }
    else if (act==actComputeSurfVolume)
    {
        double a = computeSurfaceVolume(names[0], names[1], names[2]);
        if (a>0)
            QMessageBox::information(0, "Surface Volume", QString("The surface volume is: %1").arg(a));
    }
    return update;
}
#define __interaction__
void Renderer_gl1::endSelectMode()
{
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

    if (selectMode == smShowSubtree) // This part is needed for restoring neuron color yet staying in the highlighting mode.
    {                                // -- MK, June, 2018
        if (this->pressedShowSubTree == true)
        {
            My4DImage* curImg = 0;
            if (w) curImg = v3dr_getImage4d(_idep);
            //cout << "restoring" << endl;

            if (this->originalSegMap.empty()) return;

            for (map<size_t, vector<V_NeuronSWC_unit> >::iterator it = this->originalSegMap.begin(); it != this->originalSegMap.end(); ++it)
                curImg->tracedNeuron.seg[it->first].row = it->second;

            curImg->update_3drenderer_neuron_view(w, this);
            curImg->proj_trace_history_append();

            this->pressedShowSubTree = false;
            this->connectEdit = connectEdit_none;

            this->originalSegMap.clear();
            this->highlightedSegMap.clear();

            return;
        }
    }


    qDebug() << "  Renderer_gl1::endSelectMode" << " total elapsed time = [" << total_etime << "] milliseconds";
    total_etime = 0;
    if (selectMode == smCurveCreate_pointclick || selectMode == smCurveCreate_pointclickAutoZ || selectMode == smCurveCreate_MarkerCreate1)
    {
        if (cntCur3DCurveMarkers >=2)
        {
            qDebug("\t %i markers to solve Curve", cntCur3DCurveMarkers);
            solveCurveFromMarkers(); //////////
        }
    }

//#ifndef test_main_cpp    //140211
    if (selectMode == smCurveCreate_pointclick_fm || selectMode == smCurveCreate_MarkerCreate1_fm)
    {
        if (cntCur3DCurveMarkers >=2)
        {
            qDebug("\t %i markers to solve Curve", cntCur3DCurveMarkers);
            solveCurveFromMarkersFastMarching(); //////////
            if(selectMode == smCurveCreate_MarkerCreate1_fm)
            {
                b_addthiscurve = true;
                cntCur3DCurveMarkers=0; //reset
                return;
            }
        }
    }
//#endif

    // @ADDED by Alessandro on 2015-05-23. Called when the operation is finalized (i.e. "Esc" key is pressed) and
    // neuron segments have to be deleted also from the underlying tracedNeuron structure (and not only from the display)
    if(selectMode == smDeleteMultiNeurons)
        deleteMultiNeuronsByStrokeCommit();

//    if(selectMode == smBreakMultiNeurons)
//        breakMultiNeuronsByStrokeCommit();

    cntCur3DCurveMarkers = 0;
    list_listCurvePos.clear();
    listMarkerPos.clear();
    b_ablation = false; //by Jianlong Zhou, 20120726
    b_lineAblation = false; //by Jianlong Zhou, 20120801
    if (selectMode != smObject)
    {
        selectMode = smObject;
        if (w) { w->setCursor(oldCursor); }
    }
    editinput = 0;
}
void Renderer_gl1::_appendMarkerPos(int x, int y)
{
    MarkerPos pos;
    pos.x = x;
    pos.y = y;
    pos.drawn = false;
    for (int i=0; i<4; i++){
        pos.view[i] = viewport[i];
        qDebug("--------pos.view[%d]--------%d", i,pos.view[i]);
    }
    for (int i=0; i<16; i++)
    {
        pos.P[i]  = projectionMatrix[i];
        pos.MV[i] = markerViewMatrix[i];
        qDebug("--------pos.P[%d]----%e      pos.MV[%d]--%e", i,pos.P[i],i,pos.MV[i]);
    }
    listMarkerPos.append(pos);
    qDebug("\t (%d, %d) listMarkerPos.size = %d", x,y, listMarkerPos.size());
}
int Renderer_gl1::movePen(int x, int y, bool b_move)
{
    //qDebug("  Renderer_gl1::movePen");
    //	//100731 RZC
    //	if (renderMode==rmCrossSection)
    //		selectObj(x,y, false, 0); //no menu, no tip, just for lastSliceType
    // define a curve //091023
//    if (selectMode == smCurveCreate1 || selectMode == smCurveCreate2 || selectMode == smCurveCreate3 || selectMode == smSelectMultiMarkers ||
//    	selectMode == smDeleteMultiNeurons || selectMode == smRetypeMultiNeurons || selectMode == smBreakMultiNeurons || selectMode == smBreakTwoNeurons)

    if (selectMode == smCurveCreate1 || selectMode == smCurveCreate2 || selectMode == smCurveCreate3 || selectMode == smSelectMultiMarkers ||
        selectMode == smDeleteMultiNeurons ||  selectMode == smRetypeMultiNeurons || selectMode == smBreakMultiNeurons || selectMode == smBreakTwoNeurons ||
        selectMode == smConnectNeurons || selectMode == smConnectPointCloud || selectMode == smConnectMarker || selectMode == smCutNeurons || selectMode == smSimpleConnect || selectMode == smSimpleConnectLoopSafe ||
        selectMode == smShowSubtree)
    {
#ifdef _ENABLE_MACX_DRAG_DROP_FIX_
        x = 2 * x;
        y = 2 * y;
#endif
        _appendMarkerPos(x,y);
        if (b_move)
        {
            qDebug("\t track ( %i, %i ) to define Curve", x,y);
            this->sShowTrack = 1;
            return 1; //display 2d track
        }
        // release button
        qDebug("\t track-end ( %i, %i ) to define Curve (%i points)", x,y, listMarkerPos.size());
        if (listMarkerPos.size() >=3) //drop short click
            list_listCurvePos.append(listMarkerPos);
        listMarkerPos.clear();

//        int N = (selectMode == smConnectPointCloud || selectMode == smConnectNeurons || selectMode == smConnectMarker || selectMode == smCutNeurons ||
//        		 selectMode == smCurveCreate1 || selectMode == smDeleteMultiNeurons || selectMode == smSelectMultiMarkers ||
//        		 selectMode == smRetypeMultiNeurons || selectMode == smBreakMultiNeurons || selectMode == smBreakTwoNeurons) //20170731 smBreakTwoNeurons used in mozak
//        		 ? 1 : (selectMode == smCurveCreate2)? 2 : 3;
        int N = (selectMode == smCurveCreate3)? 3 : (selectMode == smCurveCreate2)? 2 : 1; //20170731 RZC: more simple expression for less bugs

        if (list_listCurvePos.size() >= N)
        {
            //qDebug("\t %i tracks to solve Curve", list_listCurvePos.size());
            if (selectMode == smCurveCreate1)
            {
                vector <XYZ> loc_vec_input; //here as an empty input, so use list_listCurvePos internal
                solveCurveCenter(loc_vec_input);
            }
            else if (selectMode == smCurveCreate2 || selectMode == smCurveCreate3)
            {
                solveCurveViews();
            }

            // 2015-05-06. @ADDED by Alessandro. Just enabled an already existing function developed by ZJL, 20120806
            else if (selectMode == smDeleteMultiNeurons)
            {
                deleteMultiNeuronsByStroke();
            }
            else if (selectMode == smRetypeMultiNeurons)
            {
                retypeMultiNeuronsByStroke();
            }
            else if (selectMode == smBreakMultiNeurons)
            {
                breakMultiNeuronsByStroke();
            }
            else if (selectMode == smBreakTwoNeurons)
            {
                //breakMultiNeuronsByStroke(true);
                breakTwoNeuronsByStroke(); //20170731 RZC: make a separate function for mozak to prevent confusion and interference
            }
            // @ADDED by Alessandro on 2015-09-30. Select multiple markers with one-mouse stroke
            else if( selectMode == smSelectMultiMarkers)
            {
                selectMultiMarkersByStroke();
            }
            // MK, 2017 April ---------------------------------------------------------
            else if (selectMode == smConnectNeurons) connectNeuronsByStroke();
            else if (selectMode == smConnectPointCloud) connectPointCloudByStroke();
            else if (selectMode == smConnectMarker) connectMarkerByStroke();
            else if (selectMode == smSimpleConnect) simpleConnect();
            else if (selectMode == smSimpleConnectLoopSafe) simpleConnect();
            else if (selectMode == smShowSubtree)
            {
                if (editinput == 10) showSubtree();
                else if (editinput == 11) showConnectedSegs();
            }
            // MK, 2017 June ----------------------------------------------------------
            else if (selectMode == smCutNeurons) cutNeuronsByStroke();
            // ------------------------------------------------------------------------

            list_listCurvePos.clear();
            if (selectMode == smCurveCreate2 || selectMode == smCurveCreate3) // make 1-track continue selected mode
                endSelectMode();
        }
    }

#ifndef test_main_cpp    //140211
    //curve generation
    else if (selectMode == smCurveRefineInit || selectMode == smCurveRefineLast || selectMode == smCurveEditRefine ||
            selectMode == smCurveEditRefine_fm || selectMode == smCurveDirectionInter || selectMode == smCurveRefine_fm ||
            selectMode == smCurveMarkerLists_fm || selectMode == smCurveFrom1Marker_fm || selectMode == smCurveCreateMarkerGD ||
            selectMode == smCurveTiltedBB_fm || selectMode == smCurveTiltedBB_fm_sbbox || selectMode == smCurveCreateTest ||
             selectMode == smMarkerCreate1Curve || selectMode == smCurveEditExtend || //by PHC 20121011
            selectMode == smCurveEditExtendOneNode || selectMode == smCurveEditExtendTwoNode ||
            selectMode == smCurveCreate_MarkerCreate1_fm || selectMode == smCurveCreate_MarkerCreate1) //by ZMS 20151203
    {
#ifdef _ENABLE_MACX_DRAG_DROP_FIX_
        x = 2 * x;
        y = 2 * y;
#endif
        _appendMarkerPos(x,y);
        if (b_move)
        {
            //qDebug("\t track ( %i, %i ) to refine Curve", x,y);
            this->sShowTrack = 1;
            return 1; //display 2d track
        }
        // else release button
        qDebug("\t track-end ( %i, %i ) to refine Curve (%i points)", x,y, listMarkerPos.size());
        if (listMarkerPos.size() >=3) //drop short click
            list_listCurvePos.append(listMarkerPos);
        listMarkerPos.clear();
        if (list_listCurvePos.size() >= 1)
        {
            //qDebug("\t %i tracks to solve Curve", list_listCurvePos.size());
            if (selectMode == smCurveRefineInit)
            {
                vector <XYZ> loc_vec_input;
                vector <XYZ> loc_vec0;
                loc_vec0.clear();
                solveCurveCenterV2(loc_vec_input, loc_vec0, 0);
                refineMode = smCurveRefine_ms; // mean shift
                selectMode = smCurveRefineLast; // switch to smCurveRefineLast
            }
            else if(selectMode == smCurveRefine_fm)
            {
                // using two marker lists for fast marching to get a curve
                vector <XYZ> loc_vec_input;
                vector <XYZ> loc_vec0;
                loc_vec0.clear();
                solveCurveMarkerLists_fm(loc_vec_input, loc_vec0, 0);
                refineMode = smCurveRefine_fm;
                selectMode = smCurveRefineLast; // switch to smCurveRefineLast for refine mode
            }
            else if (selectMode == smCurveRefineLast)
            {
                solveCurveRefineLast();
            }
            else if (selectMode == smCurveEditRefine ) // edit with mean shift
            {
                refineMode = smCurveRefine_ms;
                solveCurveRefineLast();
            }
            else if (selectMode == smCurveEditRefine_fm ) // edit with fm
            {
                refineMode = smCurveRefine_fm;
                solveCurveRefineLast();
            }
            else if (selectMode == smCurveEditExtend ) // edit with fm
            {
                refineMode = smCurveRefine_fm;
                solveCurveExtendGlobal();
            }
            else if(selectMode == smCurveDirectionInter)
            {
                vector <XYZ> loc_vec_input;
                vector <XYZ> loc_vec0;
                loc_vec0.clear();
                solveCurveDirectionInter(loc_vec_input, loc_vec0, 0);
            }
            else if(selectMode == smCurveMarkerLists_fm || selectMode == smCurveFrom1Marker_fm || selectMode == smCurveTiltedBB_fm || selectMode == smCurveTiltedBB_fm_sbbox ||
                    selectMode == smMarkerCreate1Curve || //by PHC 20121011
                    selectMode == smCurveEditExtendTwoNode || selectMode == smCurveEditExtendOneNode ||
                    selectMode == smCurveCreate_MarkerCreate1_fm || selectMode == smCurveCreate_MarkerCreate1) //by ZMS 20151203
            {
                // using two marker lists for fast marching to get a curve
                vector <XYZ> loc_vec_input;
                vector <XYZ> loc_vec0;
                loc_vec0.clear();
                total_etime += solveCurveMarkerLists_fm(loc_vec_input, loc_vec0, 0);
                if (selectMode == smMarkerCreate1Curve || selectMode == smCurveCreate_MarkerCreate1_fm || selectMode == smCurveCreate_MarkerCreate1) //PHC 20121011
                {
                    XYZ & loc = loc_vec0.at(0);
                                        if (dataViewProcBox.isInner(loc, 0.5)) //keep this for now? PHC 121011. 100725 RZC
                                            dataViewProcBox.clamp(loc); //keep this for now? PHC 121011. 100722 RZC
                    if (1)
                    {
                        addMarker(loc);
                        cntCur3DCurveMarkers++;
                    }
                }
                if(selectMode == smCurveFrom1Marker_fm) //by PHC 20121011
                {
                    endSelectMode(); //disable the option for 1-right-stroke to define a marker function
                }
            }
            else if(selectMode == smCurveCreateMarkerGD)
            {
                solveCurveFromMarkersGD(true); // ture means using customized bounding box
                endSelectMode();
            }
            else if(selectMode == smCurveCreateTest)
            {
                V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
                My4DImage* curImg = 0;
                if (w) curImg = v3dr_getImage4d(_idep);
                QString imgname(curImg->getFileName()); //include path
                QFileInfo pathinfo(imgname);
                QString fname= pathinfo.baseName(); //only the file name without extension
                QString testOutputDir;
                if(listCurveMarkerPool.size() > 2)
                {
                    testOutputDir = pathinfo.absolutePath() + "/GDBased_" + fname +"_testSWC";
                }
                else
                {
                    testOutputDir = pathinfo.absolutePath() + "/FM2Based_" + fname +"_testSWC";
                }
                QDir curdir(testOutputDir);
                if (!curdir.exists())
                    curdir.mkdir(testOutputDir);
                // get number of tests already had from dir's .txt
                int test_id_num;
                QString cursurfix=QString("txt");
                createLastTestID(testOutputDir, cursurfix, test_id_num);
                QString test_id = QString::number(test_id_num);
                QString anofilename = testOutputDir + "/" + test_id + "_" + fname + "_curveTest.ano";
                FILE *fp;
                bTestCurveBegin=true;
                fp=fopen(anofilename.toStdString().c_str(), "wt"); // open a new empty file
                if (!fp)
                {
                    v3d_msg(QString("Fail to open file %1 to write.").arg(anofilename));
                    return 1;
                }
                // write file name
                QString swcimg = "../" + pathinfo.fileName();
                fprintf(fp, "GRAYIMG=%s\n", swcimg.toStdString().c_str());
                // using two marker lists (2PointsBB) for fast marching to get a curve
                vector <XYZ> loc_vec_input;
                vector <XYZ> loc_vec0;
                clock_t t1;
                loc_vec0.clear();
                selectMode = smCurveMarkerLists_fm;
                t1=clock(); // time
                solveCurveMarkerLists_fm(loc_vec_input, loc_vec0, 0);
                clock_t time_ml = (clock()-t1);
                NeuronTree tree_ml = testNeuronTree;
                // Save to a file
                QString filenameml= test_id+ "_MarkerLists2PointsBB_fm"+".swc";
                QString filenameml_ab=testOutputDir+"/"+filenameml;
                writeSWC_file(filenameml_ab, testNeuronTree);
                // save to ano
                fprintf(fp, "SWCFILE=%s\n", filenameml.toStdString().c_str());
                // ===================================================================
                // using two marker lists (OneStrokeBB) for fast marching to get a curve
                // loc_vec_input.clear();
                // loc_vec0.clear();
                // selectMode = smCurveUseStrokeBB_fm; //============================================ to change
                // t1=clock(); // time
                // solveCurveMarkerLists_fm(loc_vec_input, loc_vec0, 0);
                // clock_t time_ml_StrokeBB = (clock()-t1);
                // NeuronTree tree_ml_StrokeBB = testNeuronTree;
                // // Save to a file
                // QString filenameml_StrokeBB= test_id+ "_MarkerListsOneStrokeBB_fm"+".swc";
                // QString filenameml_ab_StrokeBB=testOutputDir+"/"+filenameml_StrokeBB;
                // writeSWC_file(filenameml_ab_StrokeBB, testNeuronTree);
                // // save to ano
                // fprintf(fp, "SWCFILE=%s\n", filenameml_StrokeBB.toStdString().c_str());
                // using two marker lists (OneStrokeBB) for fast marching to get a curve
                loc_vec_input.clear();
                loc_vec0.clear();
                selectMode = smCurveTiltedBB_fm;
                t1=clock(); // time
                solveCurveMarkerLists_fm(loc_vec_input, loc_vec0, 0);
                clock_t time_ml_tiltedBB = (clock()-t1);
                NeuronTree tree_ml_tiltedBB = testNeuronTree;
                // Save to a file
                QString filenameml_tiltedBB= test_id+ "_MarkerListsTiltedBB_fm"+".swc";
                QString filenameml_ab_tiltedBB=testOutputDir+"/"+filenameml_tiltedBB;
                writeSWC_file(filenameml_ab_tiltedBB, testNeuronTree);
                // save to ano
                fprintf(fp, "SWCFILE=%s\n", filenameml_tiltedBB.toStdString().c_str());
                // ===================================================================
                // curve from mean shift
                loc_vec_input.clear();
                selectMode = smCurveCreate1;
                t1=clock(); // time
                solveCurveCenter(loc_vec_input);
                clock_t time_cc = (clock()-t1);
                NeuronTree tree_cc = testNeuronTree;
                QString filenamecc=test_id+ "_MeanShift"+".swc";
                QString filenamecc_ab=testOutputDir + "/" +filenamecc ;
                writeSWC_file(filenamecc_ab, testNeuronTree);
                // save to ano
                fprintf(fp, "SWCFILE=%s\n", filenamecc.toStdString().c_str());
                // curve from direction intersection
                loc_vec_input.clear();
                loc_vec0.clear();
                selectMode = smCurveDirectionInter;
                t1 = clock();
                solveCurveDirectionInter(loc_vec_input, loc_vec0, 0);
                clock_t time_di = clock()-t1;
                NeuronTree tree_di = testNeuronTree;
                QString filenamedi = test_id+ "_DirInter"+".swc";
                QString filenamedi_ab=testOutputDir + "/" +filenamedi;
                writeSWC_file(filenamedi_ab, testNeuronTree);
                // save to ano
                fprintf(fp, "SWCFILE=%s\n", filenamedi.toStdString().c_str());
                NeuronTree tree_mp, tree_gd;
                clock_t time_mp, time_gd;
                if(listCurveMarkerPool.size() > 2)
                {
                    // curve from FM
                    selectMode = smCurveMarkerPool_fm;
                    t1 = clock(); // time
                    solveCurveFromMarkersFastMarching();
                    time_mp = (clock()-t1); // /CLOCKS_PER_SEC;
                    tree_mp = testNeuronTree;
                    QString filenamemp = test_id+ "_MarkerPool_fm"+".swc";
                    QString filenamemp_ab =testOutputDir + "/" + filenamemp;
                    writeSWC_file(filenamemp_ab, testNeuronTree);
                    // save to ano
                    fprintf(fp, "SWCFILE=%s\n", filenamemp.toStdString().c_str());
                    // curve from GD
                    t1 = clock();
                    solveCurveFromMarkersGD(false); //boundingbox is the whole image
                    time_gd = (clock()-t1);
                    tree_gd = testNeuronTree;
                    QString filenamegd = test_id+ "_MarkerPool_GD"+".swc";
                    QString filenamegd_ab = testOutputDir + "/" +filenamegd;
                    writeSWC_file(filenamegd_ab, testNeuronTree);
                    // save to ano
                    fprintf(fp, "SWCFILE=%s\n", filenamegd.toStdString().c_str());
                }
                // distance computation
                // for writing curve distance information
                QString distfilename = testOutputDir + "/" + test_id + "_" + fname + "_Distance.txt";
                QString pre_distfilename = testOutputDir + "/" +  QString::number(test_id_num-1) + "_" + fname + "_Distance.txt";
                // distance threshold for the whole Test
                const static double dist_threshold = 3.0;
                FILE *fpdist=fopen(distfilename.toStdString().c_str(), "wt"); // open a new empty file
                if (!fpdist)
                {
                    v3d_msg(QString("Fail to open file %1 to write.").arg(distfilename));
                    return 1;
                }
                if(listCurveMarkerPool.size() > 2) // the ground truth is the curve from GD
                {
                    // ======================================================
                    int ml_success, ml_tiltedBB_success, ml_strokeBB_success, cc_success, di_success, mp_success;
                    int num_test;
                    if(test_id_num==1)
                    {
                        // initialize nums for the first time
                        ml_success = 0;
                        ml_tiltedBB_success = 0;
                        ml_strokeBB_success = 0;
                        cc_success = 0;
                        di_success = 0;
                        mp_success = 0;
                        num_test = 0;
                    }
                    else
                    {
                        ifstream fpredist(pre_distfilename.toStdString().c_str());
                        if(fpredist.is_open())
                        {
                            string line;
                            getline(fpredist, line);
                            istringstream buffer(line);
                            buffer >> num_test >> ml_success >> ml_tiltedBB_success >>  ml_strokeBB_success >> cc_success >> di_success >> mp_success;
                        }
                        fpredist.close();
                    }
                    // =======================================================
                    num_test ++;
                    // computer distance from GD-curve to other curves
                    double dist_gd_ml = distance_between_2lines(tree_gd, tree_ml);
                    //double dist_gd_ml_StrokeBB = distance_between_2lines(tree_gd, tree_ml_StrokeBB);
                    double dist_gd_ml_tiltedBB = distance_between_2lines(tree_gd, tree_ml_tiltedBB);
                    double dist_gd_cc = distance_between_2lines(tree_gd, tree_cc);
                    double dist_gd_di = distance_between_2lines(tree_gd, tree_di);
                    double dist_gd_mp = distance_between_2lines(tree_gd, tree_mp);
                    if(dist_gd_ml <= dist_threshold) ml_success++;
                    if(dist_gd_ml_tiltedBB <= dist_threshold) ml_tiltedBB_success++;
                    //if(dist_gd_ml_StrokeBB <= dist_threshold) ml_strokeBB_success++;
                    if(dist_gd_cc <= dist_threshold) cc_success++;
                    if(dist_gd_di <= dist_threshold) di_success++;
                    if(dist_gd_mp <= dist_threshold) mp_success++;
                    float ml_succ_rate = (float)ml_success/num_test;
                    float ml_tiltedBB_succ_rate = (float)ml_tiltedBB_success/num_test;
                    //float ml_strokeBB_succ_rate = (float)ml_strokeBB_success/num_test;
                    float cc_succ_rate = (float)cc_success/num_test;
                    float di_succ_rate = (float)di_success/num_test;
                    float mp_succ_rate = (float)mp_success/num_test;
                    //fprintf(fpdist, "%d\n", num_test); // total test num
                    fprintf(fpdist, "%d %d %d %d %d %d %d\n", num_test, ml_success, ml_tiltedBB_success, ml_strokeBB_success, cc_success, di_success, mp_success);// total test num, success num above dist_threshold
                    fprintf(fpdist, "The ground truth curve is the curve from GD method.\n");
                    fprintf(fpdist, "Time consuming for the curve from GD method: %ld CLOCKS, %f s\n\n", time_gd, (double)time_gd/CLOCKS_PER_SEC);
                    fprintf(fpdist, "The distance_threshold  = %.4f\n\n", dist_threshold);
                    fprintf(fpdist, "Distance between curves of GD and Markerlists2PointsBB_fm                       = %.4f\n",   dist_gd_ml);
                    fprintf(fpdist, "Rate of distance between GD and Markerlists2PointsBB_fm below dist_threshold    = %.2f %% \n", ml_succ_rate*100);
                    fprintf(fpdist, "Time consuming for the curve from Markerlists2PointsBB_fm                       = %ld CLOCKS, %f s\n\n", time_ml, (double)time_ml/CLOCKS_PER_SEC);
                    fprintf(fpdist, "Distance between curves of GD and MarkerlistsTiltedBB_fm                        = %.4f\n",   dist_gd_ml_tiltedBB);
                    fprintf(fpdist, "Rate of distance between GD and MarkerlistsTiltedBB_fm below dist_threshold     = %.2f %% \n", ml_tiltedBB_succ_rate*100);
                    fprintf(fpdist, "Time consuming for the curve from MarkerlistsTiltedBB_fm                        = %ld CLOCKS, %f s\n\n", time_ml_tiltedBB, (double)time_ml_tiltedBB/CLOCKS_PER_SEC);
                    // fprintf(fpdist, "Distance between curves of GD and MarkerlistsOneStrokeBB_fm                     = %.4f\n",   dist_gd_ml_StrokeBB);
                    // fprintf(fpdist, "Rate of distance between GD and MarkerlistsOneStrokeBB_fm below dist_threshold  = %.2f %% \n", ml_strokeBB_succ_rate*100);
                    // fprintf(fpdist, "Time consuming for the curve from MarkerlistsOneStrokeBB_fm                     = %ld CLOCKS, %f s\n\n", time_ml_StrokeBB, (double)time_ml_StrokeBB/CLOCKS_PER_SEC);
                    fprintf(fpdist, "Distance between curves of GD and mean_shift                         = %.4f\n",   dist_gd_cc);
                    fprintf(fpdist, "Rate of distance between GD and mean_shift below dist_threshold      = %.2f %% \n", cc_succ_rate*100);
                    fprintf(fpdist, "Time consuming for the curve from mean_shift                         = %ld CLOCKS, %f s\n\n", time_cc, (double)time_cc/CLOCKS_PER_SEC);
                    fprintf(fpdist, "Distance between curves of GD and direction_intersection             = %.4f\n",   dist_gd_di);
                    fprintf(fpdist, "Rate of distance between GD and dir_inter below dist_threshold       = %.2f %% \n", di_succ_rate*100);
                    fprintf(fpdist, "Time consuming for the curve from dir_inter                          = %ld CLOCKS, %f s\n\n", time_di, (double)time_di/CLOCKS_PER_SEC);
                    fprintf(fpdist, "Distance between curves of GD and Marker_pool_fm                     = %.4f\n",   dist_gd_mp);
                    fprintf(fpdist, "Rate of distance between GD and Marker_pool below dist_threshold     = %.2f %% \n", mp_succ_rate*100);
                    fprintf(fpdist, "Time consuming for the curve from Marker_pool                        = %ld CLOCKS, %f s\n\n", time_mp, (double)time_mp/CLOCKS_PER_SEC);
                }
                else // the ground truth is the curve from Markerlists_fm
                {
                    // ======================================================
                    int ml_mlTiltedBB_success, ml_mlStrokeBB_success, ml_cc_success, ml_di_success;
                    int ml_num_test;
                    if(test_id_num==1)
                    {
                        // initialize nums for the first time
                        ml_mlTiltedBB_success = 0;
                        ml_mlStrokeBB_success = 0;
                        ml_cc_success = 0;
                        ml_di_success = 0;
                        ml_num_test = 0;
                    }
                    else
                    {
                        ifstream fpredist(pre_distfilename.toStdString().c_str());
                        if(fpredist.is_open())
                        {
                            string line;
                            getline(fpredist, line);
                            istringstream buffer(line);
                            buffer >> ml_num_test >> ml_mlTiltedBB_success >> ml_mlStrokeBB_success >> ml_cc_success >> ml_di_success;
                        }
                        fpredist.close();
                    }
                    // =========================================================
                    // computer distance from GD-curve to other curves
                    double dist_ml_cc = distance_between_2lines(tree_ml, tree_cc);
                    double dist_ml_di = distance_between_2lines(tree_ml, tree_di);
                    //double dist_ml_mlStrokeBB = distance_between_2lines(tree_ml, tree_ml_StrokeBB);
                    double dist_ml_mlTiltedBB = distance_between_2lines(tree_ml, tree_ml_tiltedBB);
                    ml_num_test ++;
                    if(dist_ml_mlTiltedBB <= dist_threshold) ml_mlTiltedBB_success++;
                    //if(dist_ml_mlStrokeBB <= dist_threshold) ml_mlStrokeBB_success++;
                    if(dist_ml_cc <= dist_threshold) ml_cc_success++;
                    if(dist_ml_di <= dist_threshold) ml_di_success++;
                    float ml_mlTiltedBB_succ_rate = (float)ml_mlTiltedBB_success/ml_num_test;
                    //float ml_mlStrokeBB_succ_rate = (float)ml_mlStrokeBB_success/ml_num_test;
                    float ml_cc_succ_rate = (float)ml_cc_success/ml_num_test;
                    float ml_di_succ_rate = (float)ml_di_success/ml_num_test;
                    // computer distance from GD-curve to other curves
                    //fprintf(fpdist, "%d\n", ml_num_test); // total test num
                    fprintf(fpdist, "%d %d %d %d %d\n", ml_num_test, ml_mlTiltedBB_success, ml_mlStrokeBB_success, ml_cc_success, ml_di_success);// total test num, success num above dist_threshold
                    fprintf(fpdist, "The ground truth curve is the curve from Markerlists2PointsBB_fm (FM2PointsBB_fm).\n");
                    fprintf(fpdist, "The distance_threshold  = %.4f\n", dist_threshold);
                    fprintf(fpdist, "Time consuming for the curve from FM2PointsBB_fm                                   = %ld CLOCKS, %f s\n\n", time_ml, (double)time_ml/CLOCKS_PER_SEC);
                    fprintf(fpdist, "Distance between curves of FM2PointsBB_fm and FMTiltedBB_fm                        = %.4f\n", dist_ml_mlTiltedBB);
                    fprintf(fpdist, "Rate of distance between FM2PointsBB_fm and FMTiltedBB_fm below dist_threshold     = %.2f %% \n", ml_mlTiltedBB_succ_rate*100);
                    fprintf(fpdist, "Time consuming for the curve from FMTiltedBB_fm                                    = %ld CLOCKS, %f s\n\n", time_ml_tiltedBB, (double)time_ml_tiltedBB/CLOCKS_PER_SEC);
                    // fprintf(fpdist, "Distance between curves of FM2PointsBB_fm and FMOneStrokeBB_fm                     = %.4f\n", dist_ml_mlStrokeBB);
                    // fprintf(fpdist, "Rate of distance between FM2PointsBB_fm and FMOneStrokeBB_fm below dist_threshold  = %.2f %% \n", ml_mlStrokeBB_succ_rate*100);
                    // fprintf(fpdist, "Time consuming for the curve from FMOneStrokeBB_fm                                 = %ld CLOCKS, %f s\n\n", time_ml_StrokeBB, (double)time_ml_StrokeBB/CLOCKS_PER_SEC);
                    fprintf(fpdist, "Distance between curves of FM2PointsBB_fm and mean_shift                           = %.4f\n", dist_ml_cc);
                    fprintf(fpdist, "Rate of distance between FM2PointsBB_fm and mean_shift below dist_threshold        = %.2f %%\n", ml_cc_succ_rate*100);
                    fprintf(fpdist, "Time consuming for the curve from mean_shift                                       = %ld CLOCKS, %f s\n\n", time_cc, (double)time_cc/CLOCKS_PER_SEC);
                    fprintf(fpdist, "Distance between curves of FM2PointsBB_fm and direction_intersection               = %.4f\n", dist_ml_di);
                    fprintf(fpdist, "Rate of distance between FM2PointsBB_fm and direc_intersec below dist_threshold    = %.2f %%\n", ml_di_succ_rate*100);
                    fprintf(fpdist, "Time consuming for the curve from direc_intersec                                   = %ld CLOCKS, %f s\n\n", time_di, (double)time_di/CLOCKS_PER_SEC);
                }
                // projection image computation
                // 1. get max SWC boundingbox
                XYZ minbb, maxbb;
                if(listCurveMarkerPool.size() > 2) // the ground truth is the curve from GD
                {
                    swcBoundingBox(tree_gd, minbb, maxbb);
                    XYZ minloc_ml, maxloc_ml;
                    swcBoundingBox(tree_ml, minloc_ml, maxloc_ml);
                    XYZ minloc_cc, maxloc_cc;
                    swcBoundingBox(tree_cc, minloc_cc, maxloc_cc);
                    XYZ minloc_di, maxloc_di;
                    swcBoundingBox(tree_di, minloc_di, maxloc_di);
                    XYZ minloc_mp, maxloc_mp;
                    swcBoundingBox(tree_mp, minloc_mp, maxloc_mp);
                    XYZ minloc_mltiltedBB, maxloc_mltiltedBB;
                    swcBoundingBox(tree_ml_tiltedBB, minloc_mltiltedBB, maxloc_mltiltedBB);
                    // XYZ minloc_mlstrokeBB, maxloc_mlstrokeBB;
                    // swcBoundingBox(tree_ml_StrokeBB, minloc_mlstrokeBB, maxloc_mlstrokeBB);
                    MIN_BB(minbb, minloc_ml);
                    MAX_BB(maxbb, maxloc_ml);
                    MIN_BB(minbb, minloc_cc);
                    MAX_BB(maxbb, maxloc_cc);
                    MIN_BB(minbb, minloc_di);
                    MAX_BB(maxbb, maxloc_di);
                    MIN_BB(minbb, minloc_mp);
                    MAX_BB(maxbb, maxloc_mp);
                    MIN_BB(minbb, minloc_mltiltedBB);
                    MAX_BB(maxbb, maxloc_mltiltedBB);
                    // MIN_BB(minbb, minloc_mlstrokeBB);
                    // MAX_BB(maxbb, maxloc_mlstrokeBB);
                } else
                {
                    swcBoundingBox(tree_ml, minbb, maxbb);
                    XYZ minloc_cc, maxloc_cc;
                    swcBoundingBox(tree_cc, minloc_cc, maxloc_cc);
                    XYZ minloc_di, maxloc_di;
                    swcBoundingBox(tree_di, minloc_di, maxloc_di);
                    XYZ minloc_mltiltedBB, maxloc_mltiltedBB;
                    swcBoundingBox(tree_ml_tiltedBB, minloc_mltiltedBB, maxloc_mltiltedBB);
                    // XYZ minloc_mlstrokeBB, maxloc_mlstrokeBB;
                    // swcBoundingBox(tree_ml_StrokeBB, minloc_mlstrokeBB, maxloc_mlstrokeBB);
                    MIN_BB(minbb, minloc_cc);
                    MAX_BB(maxbb, maxloc_cc);
                    MIN_BB(minbb, minloc_di);
                    MAX_BB(maxbb, maxloc_di);
                    MIN_BB(minbb, minloc_mltiltedBB);
                    MAX_BB(maxbb, maxloc_mltiltedBB);
                    // MIN_BB(minbb, minloc_mlstrokeBB);
                    // MAX_BB(maxbb, maxloc_mlstrokeBB);
                }
                // add boundary to minbb and maxbb
                int boundary = 30;
                minbb.x = minbb.x - boundary;
                minbb.y = minbb.y - boundary;
                minbb.z = minbb.z - boundary;
                maxbb.x = maxbb.x + boundary;
                maxbb.y = maxbb.y + boundary;
                maxbb.z = maxbb.z + boundary;
                dataViewProcBox.clamp(minbb);
                dataViewProcBox.clamp(maxbb);
                // 2. get subvolume in boundingbox and get MAX Intensity Projection on XY, YZ plane
                // use whole size of image
                //==============================================
                //minbb.x =0; minbb.y=0; minbb.z=0;
                //maxbb.x =dim1-1; maxbb.y=dim2-1; maxbb.z=dim3-1;
                //=============================================
                unsigned char * pXY=0;
                unsigned char * pYZ=0;
                unsigned char * pXZ=0;
                MIP_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb);
                // 3. project SWC to MIP_XY, MIP_YZ planes
                unsigned char curve_color[ ][3] = {
                    {200, 20,  0  },  // red,      ml  0
                    {0,   200, 20 },  // green,    cc  1
                    {0,   20,  200},  // blue,     di  2
                    {200, 0,   200},  // purple,   mp  3
                    {220, 200, 0  },  // yellow,   gd  4
                    {0,   200, 200},  // cyan,     mlStrokeBB 5
                    {188, 94,  37 },  // coffee,   mltiltedBB 6
                    {180, 200, 120},  // asparagus,
                    {250, 100, 120},  // salmon,
                    {120, 200, 200},  // ice,
                    {100, 120, 200},  // orchid,
                };
                if(listCurveMarkerPool.size() > 2) // the ground truth is the curve from GD
                {
                    projectSWC_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb, tree_gd, curve_color[4]);
                    projectSWC_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb, tree_ml, curve_color[0]);
                    //projectSWC_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb, tree_ml_StrokeBB, curve_color[5]);
                    projectSWC_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb, tree_ml_tiltedBB, curve_color[6]);
                    projectSWC_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb, tree_cc, curve_color[1]);
                    projectSWC_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb, tree_di, curve_color[2]);
                    projectSWC_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb, tree_mp, curve_color[3]);
                }else
                {
                    projectSWC_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb, tree_ml, curve_color[0]);
                    //projectSWC_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb, tree_ml_StrokeBB, curve_color[5]);
                    projectSWC_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb, tree_ml_tiltedBB, curve_color[6]);
                    projectSWC_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb, tree_cc, curve_color[1]);
                    projectSWC_XY_YZ_XZ(pXY, pYZ, pXZ, minbb, maxbb, tree_di, curve_color[2]);
                }
                // 4. save image
                // 4.1 combine pXY, pYZ
                V3DLONG sz[3];
                sz[0]=maxbb.x-minbb.x+1; sz[1]=maxbb.y-minbb.y+1;
                sz[2]=maxbb.z-minbb.z+1;
                unsigned char* pXY_YZ_XZ = new unsigned char [3 * (sz[1]+sz[2]) * (sz[0]+sz[2]) ];
                V3DLONG offset_xy_yz_xz = (sz[1]+sz[2])*(sz[0]+sz[2]);
                V3DLONG offset_xy = sz[1]*sz[0];
                V3DLONG offset_yz = sz[1]*sz[2];
                V3DLONG offset_xz = sz[0]*sz[2];
                memset(pXY_YZ_XZ, 0 , 3*offset_xy_yz_xz);
                // combine pXZ
                for(V3DLONG z=0; z<sz[2]; z++)
                {
                    for(V3DLONG x=0; x<sz[0]; x++)
                    {
                        V3DLONG ind_xz=z*sz[0] + x;
                        V3DLONG ind = (z)*(sz[0]+sz[2]) + x;
                        pXY_YZ_XZ[ind] = pXZ[ind_xz];
                        pXY_YZ_XZ[ind + offset_xy_yz_xz] = pXZ[ind_xz + offset_xz];
                        pXY_YZ_XZ[ind + 2*offset_xy_yz_xz] = pXZ[ind_xz + 2*offset_xz];
                    }
                }
                // combine pXY_pYZ
                for(V3DLONG y=0; y<sz[1]; y++)
                {
                    for(V3DLONG x=0; x<sz[0]; x++)
                    {
                        V3DLONG ind_xy=y*sz[0] + x;
                        V3DLONG ind = (y+sz[2])*(sz[0]+sz[2]) + x;
                        pXY_YZ_XZ[ind] = pXY[ind_xy];
                        pXY_YZ_XZ[ind + offset_xy_yz_xz] = pXY[ind_xy + offset_xy];
                        pXY_YZ_XZ[ind + 2*offset_xy_yz_xz] = pXY[ind_xy + 2*offset_xy];
                    }
                    for(V3DLONG z=0; z<sz[2]; z++)
                    {
                        V3DLONG ind_yz=y*sz[2] + z;
                        V3DLONG ind = (y+sz[2])*(sz[0]+sz[2]) + (sz[0] + z);
                        pXY_YZ_XZ[ind] = pYZ[ind_yz];
                        pXY_YZ_XZ[ind + offset_xy_yz_xz] = pYZ[ind_yz + offset_yz];
                        pXY_YZ_XZ[ind + 2*offset_xy_yz_xz] = pYZ[ind_yz + 2*offset_yz];
                    }
                }
                // 4.2 Save image
                V3DLONG img_sz[4];
                img_sz[0]=sz[0]+sz[2]; img_sz[1]=sz[1]+sz[2]; img_sz[2]=1; img_sz[3]=3;
                QString imgfilename = testOutputDir + "/" + test_id + "_" + fname + "_CompareImg.tiff";
                saveImage(imgfilename.toStdString().c_str(), pXY_YZ_XZ, img_sz, V3D_UINT8);
                // clear memory
                if(pXY) {delete [] pXY; pXY=0;}
                if(pYZ) {delete [] pYZ; pYZ=0;}
                if(pXZ) {delete [] pXZ; pXZ=0;}
                if(pXY_YZ_XZ) {delete [] pXY_YZ_XZ; pXY_YZ_XZ=0;}
                // clear MarkerPool for the next drawing
                listCurveMarkerPool.clear();
                // continue to use smCurveCreateTest mode
                selectMode = smCurveCreateTest;
                // close file
                if(fpdist) fclose(fpdist);
                if(fp) fclose(fp);
            }
            // refine last
            list_listCurvePos.clear();
            //press Esc to endSelectMode();
        }
    } // end of curve refine ZJL 110905
#endif

    // for rubber drag the curve
#ifndef test_main_cpp //140211

    else if (selectMode == smCurveRubberDrag)
    {
        _appendMarkerPos(x, y);
        _updateDragPoints(x,y);
        if (b_move)
        {
            this->sShowRubberBand = 1;
            return 1; //display 2d track
        }
        // else release button
        listMarkerPos.clear();
        bInitDragPoints = false; // prepare for the next drag
        solveCurveRubberDrag();
        DraggedNeurons.clear();
        // press Esc to endSelectMode();
    }
#endif

    this->sShowRubberBand = 0;
    this->sShowTrack = 0;
    return 0; //no 2d track to display
}

int Renderer_gl1::hitWheel(int x, int y)
{
    //qDebug("  Renderer_gl1::hitWheel \t (%d, %d)", x,y);
    wheelPos.x = x;
    wheelPos.y = y;
    int i;
    for (i=0; i<4; i++)
    {
        wheelPos.view[i] = viewport[i];
    }
  /*  qDebug(" wheel pos (x=%5.3f y=%5.3f) viewport (%d, %d, %d, %d)", wheelPos.x, wheelPos.y,
           wheelPos.view[0], wheelPos.view[1], wheelPos.view[2], wheelPos.view[3]);*/
    for (i=0; i<16; i++)
    {
        wheelPos.P[i]  = projectionMatrix[i];
        wheelPos.MV[i] = markerViewMatrix[i];
    }
    return 1;
}

int Renderer_gl1::hitPen(int x, int y)
{
    //qDebug("  Renderer_gl1::hitPen");

    if (selectMode == smCurveCreate1 || selectMode == smCurveCreate2 || selectMode == smCurveCreate3 || selectMode == smDeleteMultiNeurons || selectMode == smSelectMultiMarkers || selectMode == smRetypeMultiNeurons ||
            // for curve refinement, 110831 ZJL
            selectMode == smCurveRefineInit || selectMode == smCurveRefineLast || selectMode == smCurveEditRefine ||
            selectMode == smCurveEditRefine_fm || selectMode == smCurveDirectionInter || selectMode == smCurveMarkerLists_fm)
    {
        //qDebug("\t track-start ( %i, %i ) to define Curve", x,y);
        _appendMarkerPos(x,y);
        // endSlectMode() in movePen
        return 1;
    }
    // for rubber drag the curve
    else if (selectMode == smCurveRubberDrag)
    {
        _appendMarkerPos(x, y);
        return 1;
    }
    else if (selectMode == smCurveCreate_pointclick || selectMode == smCurveCreate_pointclick_fm || selectMode == smCurveCreate_pointclickAutoZ) //091226
    {
        _appendMarkerPos(x,y);
        int N = 1;
        if (listMarkerPos.size() >= N)
        {
            qDebug("\t click ( %i, %i ) for Markers to Curve", x,y);
            b_addthismarker = true; // by ZJL 20120203 for prohibitting displaying a 3d local view window
            if (selectMode == smCurveCreate_pointclickAutoZ) // TDP 20160204
            {
                solveMarkerCenterMaxIntensity();
            }
            else
            {
                solveMarkerCenter();
            }
            cntCur3DCurveMarkers++;
            listMarkerPos.clear();
            //			if (selectMode != smCurveCreate_pointclick) // make 1-click continue selected mode
            //				endSelectMode();
        }
        return 1;
    }
    // define a marker
    else if (selectMode == smMarkerCreate1 || selectMode == smMarkerCreate2 || selectMode == smMarkerCreate3)
    {
        qDebug("\t click ( %i, %i ) to define Marker", x,y);
        _appendMarkerPos(x,y);
        int N = (selectMode == smMarkerCreate1)? 1 : (selectMode == smMarkerCreate2)? 2 : 3;
        if (listMarkerPos.size() >= N)
        {

            if (selectMode == smMarkerCreate1)
            {
                qDebug("--------------------------------------------solveMarkerCenter");
                total_etime += solveMarkerCenter();//走这里
            }
            else
            {
                qDebug("---------------------------------------------solveMarkerViews");
                solveMarkerViews(); //////////
            }
            listMarkerPos.clear();
            if (selectMode != smMarkerCreate1) // make 1-click continue selected mode
            {
                endSelectMode();
            }
        }
        return 1;
    }
    // refine a marker
    else if (selectMode == smMarkerRefineC || selectMode == smMarkerRefineT)
    {
        qDebug("\t click ( %i, %i ) to refine Marker", x,y);
        _appendMarkerPos(x,y);
        if (selectMode == smMarkerRefineC)
            refineMarkerCenter(); /////////////
        else
            refineMarkerTranslate(); ///////////
        listMarkerPos.clear();
        endSelectMode();
        return 1;
    }
    return 0; // not processed
}
#ifndef test_main_cpp
void Renderer_gl1::editSurfaceObjBasicGeometry(int dc, int st, int i) // i is 1-based
{
    qDebug("  Renderer_gl1::editSurfaceObjBasicGeometry");
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (dc==dcSurface)
    {
        setClipBoxState_forSubjectObjs(false); //now disable the clip box limitation
        if (st==stNeuronStructure)
        {
            int ii=i-1;
            NeuronTree old_nt; old_nt.copy(listNeuronTree[ii]); //first make a backup
            SurfaceObjGeometryDialog tmp_dialog(w, this, dc, st, ii);
            int res = tmp_dialog.exec(); //note that the neuron's data as well as its 3D view will get real-time update when the dialog is running
            if (res!=QDialog::Accepted) //restore the old tree
            {
                listNeuronTree.replace(ii, old_nt);
            }
            else //now update bounding box
            {
                swcBB = NULL_BoundingBox;
                foreach (NeuronTree T, listNeuronTree)
                {
                    BoundingBox myBB = NULL_BoundingBox;
                    foreach(NeuronSWC S, T.listNeuron)
                    {
                        //myBB.expand(XYZ(S));
                        float d = S.r *2;
                        myBB.expand(BoundingBox(XYZ(S)-d, XYZ(S)+d));
                    }
                    swcBB.expand(myBB);
                }
            }
        }
        else if (st==stPointCloud)
        {
            int ii=i-1;
            QList <CellAPO> old_clist = listCell; //first make a backup
            SurfaceObjGeometryDialog tmp_dialog(w, this, dc, st, ii);
            int res = tmp_dialog.exec(); //note that the apo data as well as its 3D view will get real-time update when the dialog is running
            if (res!=QDialog::Accepted) //restore the old list
            {
                listCell = old_clist;
            }
            else //now update the bounding box
            {
                BoundingBox myBB = NULL_BoundingBox;
                foreach(CellAPO S, listCell)
                {
                    //myBB.expand(XYZ(S));
                    float d = 2.0*pow(S.volsize/3.1415926*0.75, 1/3.0);
                    myBB.expand(BoundingBox(XYZ(S)-d/2, XYZ(S)+d/2));
                }
                apoBB.expand(myBB);
            }
        }
        updateBoundingBox(); //update the global bounding  box
        setClipBoxState_forSubjectObjs(true); //now enable the clip box limitation
        if (w) w->update();
    }
}
void Renderer_gl1::editSurfaceObjAnnotation(int dc, int st, int i) // i is 1-based
{
    qDebug("  Renderer_gl1::editSurfaceObjAnnotation");
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* imgData = v3dr_getImage4d(_idep);
    if (dc==dcSurface)
    {
        if (st==stImageMarker && imgData && imgData->getCDim())
        {
            //if (imgData)  //091006 RZC: fixed when only a null image for neuron editing
            {
                int ind_min = i-1;
                LandmarkPropertyDialog 	landmarkView(&(imgData->listLandmarks), ind_min, imgData);
                if (landmarkView.exec()==QDialog::Accepted)
                {
                    landmarkView.fetchData(&(imgData->listLandmarks), ind_min);
                    qDebug("edit landmark [%ld]. data fetched [%s][%s][%d]", ind_min,
                            imgData->listLandmarks.at(ind_min).name.c_str(), imgData->listLandmarks.at(ind_min).comments.c_str(),
                            int(imgData->listLandmarks.at(ind_min).shape));
                    //important: set the shape of the landmark
                    LocationSimple * p_tmp_location = (LocationSimple *) & (imgData->listLandmarks.at(ind_min));
                    switch (p_tmp_location->shape)
                    {
                        case pxUnset:	p_tmp_location->inputProperty = pxUnknown; break;
                        case pxSphere:	p_tmp_location->inputProperty = pxLocaUseful; break;
                        case pxCube:    p_tmp_location->inputProperty = pxLocaNotUseful; break;
                        default: p_tmp_location->inputProperty = pxLocaUnsure; break;
                    }
                    updateLandmark();
                }
            }
        }
        //note that as of now (090220) I do not update the respective content in the object manager for (surface, swc, and apo) if it is OPENED.
        // The reason is that this manager's info may be associated with a different window. The best solution is to
        //rewrite the object manager as a model-view later. Thus the potential incosistency of the "BEING" display info
        //is a known bug, - but can be easily avoided if the object manager if always close when it is not used.
        //Thus to get around this problem, I always close the object manager if it is still open when the right-click property-editing is called.
        else if (st==stImageMarker || st==stLabelSurface || st==stNeuronStructure || st==stPointCloud)
        {
            //first close the object manager if it is open
            if (w) w->surfaceDialogHide();
            //then open the object editing dialog
            QString *realobj_name=0, *realobj_comment=0, realobj_otherinfo;
            int ii=i-1;
            if (st==stLabelSurface) //for label surface
            {
                realobj_name = &(listLabelSurf[ii].name); realobj_comment=&(listLabelSurf[ii].comment);
                realobj_otherinfo = "surface area = "; QString ts2; double a = computeSurfaceArea(dc, st, i); ts2.setNum(a); realobj_otherinfo += ts2 + "<br>";
                //realobj_otherinfo += "surface volume = "; a = computeSurfaceVolume(dc, st, i); ts2.setNum(a); realobj_otherinfo += ts2;
            }
            else if (st==stNeuronStructure) //for swc
            {
                realobj_name = &(listNeuronTree[ii].name); realobj_comment=&(listNeuronTree[ii].comment);
                realobj_otherinfo = get_neuron_morpho_features_str(&(listNeuronTree[ii]));
            }
            else if (st==stPointCloud) //for APO
            {
                realobj_name = &(listCell[ii].name); realobj_comment=&(listCell[ii].comment);
            }
            else if (st==stImageMarker)
            {
                realobj_name = &(listMarker[ii].name); realobj_comment=&(listMarker[ii].comment);
            }
            qDebug()<<QString("SurfaceObjAnnotationDialog( %1, %2, %3 )").arg(i).arg(*realobj_name).arg(*realobj_comment);
            SurfaceObjAnnotationDialog tmp_dialog(i, realobj_name, realobj_comment, &realobj_otherinfo);
            int res = tmp_dialog.exec();
            if (res==QDialog::Accepted)
            {
                tmp_dialog.fetchData(realobj_name, realobj_comment);
            }
        }
    }
}
#endif
void Renderer_gl1::updateTracedNeuron()
{
#ifndef test_main_cpp
    qDebug("  Renderer_gl1::updateTracedNeuron");
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* curImg =  v3dr_getImage4d(_idep); //by PHC, 090119
    if (curImg)
        curImg->update_3drenderer_neuron_view(w, this);
#endif
}
void Renderer_gl1::loadLandmarks_from_file(const QString & filename)
{
#ifndef test_main_cpp
    QList <ImageMarker> tmp_list = readMarker_file(filename);
    if (tmp_list.size()>0) //only replace the current marker list when the list from file is non-empty
    {
        listMarker = tmp_list;
        //should also update the marker list in the main tri-view window if there is one
        V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
        My4DImage* curImg = 0;
        if (w) curImg = v3dr_getImage4d(_idep); //->image4d;
        if (curImg)
        {
            curImg->listLandmarks.clear();
            for (V3DLONG j=0;j<listMarker.size();j++)
            {
                LocationSimple S;
                S.inputProperty = pxLocaUseful;
                // 090617 RZC: marker file is 1-based
                S.x = listMarker[j].x;
                S.y = listMarker[j].y;
                S.z = listMarker[j].z;
                S.color = listMarker[j].color; // 090508 RZC: hold same color
                S.on = true;
                //S.radius = listMarker[j].radius; // 090508 RZC: listMarker[j].radius only for 3D display
                S.shape = (PxLocationMarkerShape)(listMarker[j].shape);
                S.name = qPrintable(listMarker[j].name);
                S.comments = qPrintable(listMarker[j].comment);
                curImg->listLandmarks.append(S);
            }
        }
    }
#endif
}
void Renderer_gl1::saveLandmarks_to_file(const QString & filename)
{
#ifndef test_main_cpp
    writeMarker_file(filename, listMarker);
#endif
}
#define __info_of_object__
QString Renderer_gl1::info_Marker(int marker_i)
{
    QString tmpstr;
    if (marker_i>=0 && marker_i<listMarker.size())
    {
        const ImageMarker & S = listMarker.at(marker_i);
        tmpstr = QString("\n(%1, %2, %3)").arg(S.x).arg(S.y).arg(S.z);
    }
    return tmpstr;
}
QString Renderer_gl1::info_NeuronNode(int n_id, NeuronTree *p_tree)
{
    QString tmpstr, tmpstr1;
    if (p_tree && n_id>=0 && n_id<p_tree->listNeuron.size())
    {
        tmpstr1.setNum(p_tree->listNeuron.at(n_id).n);    tmpstr1.prepend("\n 1) node #	= "); tmpstr.append(tmpstr1);
        tmpstr1.setNum(p_tree->listNeuron.at(n_id).type); tmpstr1.prepend("\n 2) type	= "); tmpstr.append(tmpstr1);
        tmpstr1.setNum(p_tree->listNeuron.at(n_id).x);    tmpstr1.prepend("\n 3) x coord	= "); tmpstr.append(tmpstr1);
        tmpstr1.setNum(p_tree->listNeuron.at(n_id).y);    tmpstr1.prepend("\n 4) y coord	= "); tmpstr.append(tmpstr1);
        tmpstr1.setNum(p_tree->listNeuron.at(n_id).z);    tmpstr1.prepend("\n 5) z coord	= "); tmpstr.append(tmpstr1);
        tmpstr1.setNum(p_tree->listNeuron.at(n_id).r);    tmpstr1.prepend("\n 6) radius	= "); tmpstr.append(tmpstr1);
        tmpstr1.setNum(p_tree->listNeuron.at(n_id).pn);   tmpstr1.prepend("\n 7) parent	= "); tmpstr.append(tmpstr1);
        tmpstr += QString("\n segment (index) = %1 (%2)").arg(p_tree->listNeuron.at(n_id).seg_id).arg(p_tree->listNeuron.at(n_id).nodeinseg_id);

                QList<float> features = p_tree->listNeuron.at(n_id).fea_val;
        if (features.size()>0) {
            tmpstr1=QString("");
            for (int j = 0 ; j< features.size(); j++){
                tmpstr1 += QString::number (features[j]);
                tmpstr1 += " ";
            }
            tmpstr1.prepend("\n features = ");
            tmpstr.append(tmpstr1);
        }
    }
    return tmpstr;
}
QString Renderer_gl1::info_SurfVertex(int n_id, Triangle * face, int label)
{
    QString tmpstr, tmpstr1;
    if (face &&  n_id>=0 && n_id<3)
    {
        tmpstr1 = QString("\nlabel	= %1").arg(label); tmpstr.append(tmpstr1);
        tmpstr1 = QString("\nvertex	= (%1, %2, %3)").arg(face->vertex[n_id][0]).arg(face->vertex[n_id][1]).arg(face->vertex[n_id][2]); tmpstr.append(tmpstr1);
        tmpstr1 = QString("\nnormal	= (%1, %2, %3)").arg(face->normal[n_id][0]).arg(face->normal[n_id][1]).arg(face->normal[n_id][2]); tmpstr.append(tmpstr1);
    }
    return tmpstr;
}
V3DLONG Renderer_gl1::findNearestNeuronNode_WinXY(int cx, int cy, NeuronTree * ptree, double &best_dist) //find the nearest node in a neuron in XY project of the display window
{
    if (!ptree) return -1;
    QList <NeuronSWC> *p_listneuron = &(ptree->listNeuron);
    if (!p_listneuron) return -1;
    //qDebug()<<"win click position:"<<cx<<" "<<cy;
    GLdouble px, py, pz, ix, iy, iz;
    V3DLONG best_ind=-1; best_dist=-1;
    for (V3DLONG i=0;i<p_listneuron->size();i++)
    {
        ix = p_listneuron->at(i).x, iy = p_listneuron->at(i).y, iz = p_listneuron->at(i).z;
        GLint res = gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz);// note: should use the saved modelview,projection and viewport matrix
        py = viewport[3]-py; //the Y axis is reversed
        if (res==GL_FALSE) {qDebug()<<"gluProject() fails for NeuronTree ["<<i<<"] node"; return -1;}
        //qDebug()<<i<<" "<<px<<" "<<py<<" "<<pz<<"\n";
        double cur_dist = (px-cx)*(px-cx)+(py-cy)*(py-cy);

#ifdef _NEURON_ASSEMBLER_
        if (cur_dist < this->radius * this->radius) this->indices.insert(i);
#endif

        if (i==0) {	best_dist = cur_dist; best_ind=0; }
        else
        {
            if (cur_dist<best_dist)
            {
                best_dist=cur_dist;
                best_ind = i;
            }
        }
    }

    // Sensitivity test for mouse click and projected coordinates -- MK, May, 2020
    /*GLint res = gluProject(p_listneuron->at(best_ind).x, p_listneuron->at(best_ind).y, p_listneuron->at(best_ind).z, currMviewMatrix, currPmatrix, currViewport, &px, &py, &pz);
    cout << " --- mouse click coords from rendere_hit: " << cx << " " << cy << endl;
    cout << " --- nearesr node projected coords from renderer_hit: (" << best_ind << ") " << px << " " << py << endl;
    cout << " --- 1st time projected coords: " << bestPx << " " << bestPy << endl;
    cout << endl;*/

    //best_ind = p_listneuron->at(best_ind).n; // this no used, because it changed in V_NeuronSWC
    return best_ind; //by PHC, 090209. return the index in the SWC file
}

#ifdef _NEURON_ASSEMBLER_
void Renderer_gl1::localSWCcoord2projectedWindowCoord(const float swcLocalCoord[], double swcWindowCoord[])
{
    GLdouble ix, iy, iz, px, py, pz;
    ix = GLdouble(swcLocalCoord[0]);
    iy = GLdouble(swcLocalCoord[1]);
    iz = GLdouble(swcLocalCoord[2]);

    GLint res = gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz);
    py = viewport[3] - py;

    swcWindowCoord[0] = px;
    swcWindowCoord[1] = py;
    swcWindowCoord[2] = pz;
}

void Renderer_gl1::addMarker_NA(XYZ& loc, RGBA8 color)
{
    XYZ pt(loc.x + 1, loc.y + 1, loc.z + 1); // marker position is 1-based

    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* image4d = v3dr_getImage4d(_idep);
    MainWindow* V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);

    if (image4d)
    {
        QList<LocationSimple>& listLoc = image4d->listLandmarks;
        LocationSimple S;
        V3DLONG markerindex = -1; // the index of no special marker, by XZ, 20190721
        for (V3DLONG i = listLoc.size() - 1; i >= 0; --i)
        {
            if (listLoc.at(i).category != 77)
            {
                markerindex = i;
                break;
            }
        }
        if (markerindex >= 0/*listLoc.size()>0*/)
        {
            S.inputProperty = listLoc.at(markerindex).inputProperty;
            S.comments = listLoc.at(markerindex).comments;
            S.category = listLoc.at(markerindex).category;
            S.color = color;
        }
        else
        {
            S.inputProperty = pxLocaUseful;
            //S.color = random_rgba8(255);
            S.color = color;
        }
        S.x = pt.x;
        S.y = pt.y;
        S.z = pt.z;
        if (V3Dmainwindow) S.radius = V3Dmainwindow->global_setting.default_marker_radius;
        S.on = true;
        listLoc.append(S);
        updateLandmark();
    }
}
#endif

Triangle * Renderer_gl1::findNearestSurfTriangle_WinXY(int cx, int cy, int & vertex_i, Triangle * plist)
{
    if (!plist) return NULL;
    //qDebug()<<"win click position:"<<cx<<" "<<cy;
    GLdouble px, py, pz, ix, iy, iz;
    V3DLONG best_ind=-1, best_vertex=-1; double best_dist=-1;
    Triangle * best_pT=NULL;
    int i=0;
    for (Triangle * pT=plist; pT->next!=NULL; pT=pT->next, i++)
        for (int j=0; j<3; j++) // 3 vertexes in triangle
        {
            ix = pT->vertex[j][0], iy = pT->vertex[j][1], iz = pT->vertex[j][2];
            GLint res = gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz); // note: should use the saved modelview,projection and viewport matrix
            py = viewport[3]-py; //the Y axis is reversed
            if (res==GL_FALSE) {qDebug()<<"gluProject() fails for Triangle ["<<i<<""<<j<<"] vertex"; return NULL;}
            //qDebug()<<i<<" "<<px<<" "<<py<<" "<<pz<<"\n";
            double cur_dist = (px-cx)*(px-cx)+(py-cy)*(py-cy);
            if (i==0 && j==0) {	best_dist = cur_dist; best_ind=0; best_vertex=0; best_pT=pT;}
            else {	if (cur_dist<best_dist) {best_dist=cur_dist; best_ind = i; best_vertex = j; best_pT=pT;}}
        }
    vertex_i = best_vertex;
    return best_pT; //091020 RZC
}
double Renderer_gl1::computeSurfaceArea(int dc, int st, int index) //index is 1-based
{
    qDebug("  Renderer_gl1::computeSurfaceArea");
    if (dc==dcSurface && st==stLabelSurface)
    {
        //LabelSurf &S = listLabelSurf[index-1];
        Triangle* pT = list_listTriangle[index-1];
        double sum = 0;
        for (Triangle* p = pT; p!=NULL; p = p->next)
        {
            XYZ V[3];
            for (int iCorner = 0; iCorner < 3; iCorner++)
            {
                V[iCorner] = XYZ(p->vertex[iCorner][0],p->vertex[iCorner][1],p->vertex[iCorner][2]);
            }
            double area = 0.5*norm( cross(V[1]-V[0], V[2]-V[0]) );
            sum += area;
        }
        return sum;
    }
    return 0;
}
double Renderer_gl1::computeSurfaceVolume(int dc, int st, int index) //index is 1-based
{
    qDebug("  Renderer_gl1::computeSurfaceVolume");
    if (dc==dcSurface && st==stLabelSurface)
    {
        //LabelSurf &S = listLabelSurf[index-1];
        Triangle* pT = list_listTriangle[index-1];
    }
    return 0;
}
void Renderer_gl1::showLineProfile(int marker1, int marker2) // 0-based
{
    qDebug("  Renderer_gl1::showLineProfile");
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    XYZ V1 = XYZ(listMarker[marker1]) - XYZ(1,1,1); // 090505 RZC: marker position is 1-based
    XYZ V2 = XYZ(listMarker[marker2]) - XYZ(1,1,1);
    double length = norm(V1-V2);
    //QVector<int> vec(300);	for (int i=0; i<vec.size(); i++) vec[i] = i; // for debug
    //int nChannel = MIN(3, dim4); // up to 3 channels
    int nChannel = dim4;
    QVector< QVector<int> > vvec;
    QStringList labelsLT;
    for (int i=0; i<nChannel; i++)
    {
        QVector<int> vec = getLineProfile(V1, V2, i);
        vvec.append( vec );
        labelsLT.append( QString("channel %1").arg(i+1) );
    }
    QString labelRB = QString("length=%1").arg(length);
    barFigureDialog *dlg = new barFigureDialog(vvec, labelsLT, labelRB, w, QSize(500, 150), QColor(50,50,50));
    dlg->setWindowTitle(QObject::tr("Line Profile (marker #%1 --> marker #%2)").arg(marker1+1).arg(marker2+1));
    dlg->show();
}
QVector<int> Renderer_gl1::getLineProfile(XYZ P1, XYZ P2, int chno)
{
    QVector<int> prof;
#ifndef test_main_cpp
    //V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* curImg = v3dr_getImage4d(_idep);//->image4d; //by PHC, 090119
    if (curImg && data4dp && chno>=0 &&  chno <dim4)
    {
        XYZ D = P2-P1;
        double length = norm(D);
        int nstep = int(length + 0.5);
        double step = length/nstep;
        normalize(D);
        //		unsigned char* vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1);
        Image4DProxy<Image4DSimple> img4dp( curImg );
        img4dp.set_minmax(curImg->p_vmin, curImg->p_vmax);
        for (int i=0; i<nstep; i++)
        {
            XYZ P = P1 + D*step*(i);
            //			XYZ P = P1 + (P2-P1)*(double(i)/(nstep-1));
            int ix = int(P.x +0.5);
            int iy = int(P.y +0.5);
            int iz = int(P.z +0.5);
            float value = sampling3dUINT8( img4dp, (chno + volTimePoint*dim4), ix, iy, iz, 1,1,1);
            //			float value = sampling3dUINT8( vp, dim1, dim2, dim3, ix, iy, iz, 1,1,1);
            //			float value = sampling3dUINT8at( vp, dim1, dim2, dim3, P.x, P.y, P.z);
            prof << int(value);
        }
    }
#endif
    return prof;
}
//##########################################################################################
// 090617: NOTICE that marker position is 1-based to consist with LocatonSimple
// Now only 3 parts are concerned with adjusting +/-1:
// (1) addMarker (+1)
// (2) updateMakerLocation (+1)
// (3) drawMarkerList (-1)
//##########################################################################################
//
// 090716: marker coordinates Fully are handled in paint() and Original image space!
//
//##########################################################################################
int Renderer_gl1::checkCurChannel()
{
    ////////////////////////////////////////////////////////////////////////
    int chno=-1;
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (w)
    {
        //when chno<0, then need to recheck the current chno
        if (chno<0) chno = w->getNumKeyHolding()-1;	// NumKey state is used firstly
        if (chno<0) chno = curChannel; 				// GUI state is used secondly, 100802
    }
    return chno;
}
#define __creat_curve___
void Renderer_gl1::solveCurveCenter(vector <XYZ> & loc_vec_input)
{
    QElapsedTimer t1a;
    t1a.start();
    v3d_msg("staring the timer for solveCurveCenter()",0);

    //

    bool b_use_seriespointclick = (loc_vec_input.size()>0) ? true : false;
    if (b_use_seriespointclick==false && list_listCurvePos.size()<1)  return;
    bool b_use_last_approximate=true;
#ifndef test_main_cpp
    MainWindow* V3Dmainwindow = 0;
    V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);
    if (V3Dmainwindow)
        b_use_last_approximate = V3Dmainwindow->global_setting.b_3dcurve_inertia;
#endif
    ////////////////////////////////////////////////////////////////////////
    int chno = checkCurChannel();
    if (chno<0 || chno>dim4-1)   chno = 0; //default first channel
    ////////////////////////////////////////////////////////////////////////
    qDebug()<<"\n  3d curve in channel # "<<((chno<0)? chno :chno+1);
    vector <XYZ> loc_vec;
    loc_vec.clear();
    int N = loc_vec_input.size();
    if (b_use_seriespointclick)
    {
        loc_vec = loc_vec_input;//一个个的取出loc_vec_input中的XYZ类型的变量
    }
    else //then use the moving mouse location, otherwise using the preset loc_vec_input (which is set by the 3d-curve-by-point-click function)
    {
        N = list_listCurvePos.at(0).size();
        for (int i=0; i<N; i++)
        {
            const MarkerPos & pos = list_listCurvePos.at(0).at(i);
            ////////////////////////////////////////////////////////////////////////
            //100730 RZC, in View space, keep for dot(clip, pos)>=0
            double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
            clipplane[3] = viewClip;
            ViewPlaneToModel(pos.MV, clipplane);
            //qDebug()<<"   clipplane:"<<clipplane[0]<<clipplane[1]<<clipplane[2]<<clipplane[3];
            ////////////////////////////////////////////////////////////////////////
            XYZ loc0, loc1;
            _MarkerPos_to_NearFarPoint(pos, loc0, loc1);
            XYZ loc;
            float length01 = dist_L2(loc0, loc1);
            if (length01<1.0)
            {
                loc=(loc0+loc1)/2.0;
            }
            else
            {
                int last_j = loc_vec.size()-1;
                if (last_j>=0 && b_use_last_approximate) //091114 PHC make it more stable by conditioned on previous location
                {
                    XYZ lastpos = loc_vec.at(last_j);
                    if (dataViewProcBox.isInner(lastpos, 0.5))
                    {
                        XYZ v_1_0 = loc1-loc0, v_0_last=loc0-lastpos;
                        XYZ nearestloc = loc0-v_1_0*dot(v_0_last, v_1_0)/dot(v_1_0, v_1_0); //since loc0!=loc1, this is safe
                        double ranget = (length01/2.0)>10?10:(length01/2.0); //at most 30 pixels aparts
                        XYZ D = v_1_0; normalize(D);
                        loc0 = nearestloc - D*(ranget);
                        loc1 = nearestloc + D*(ranget);
                    }
                }
                //printf("loc0--loc1: (%g, %g, %g)--(%g, %g, %g)\n", loc0.x,loc0.y,loc0.z, loc1.x,loc1.y,loc1.z);
                loc = getCenterOfLineProfile(loc0, loc1, clipplane, chno);
            }
            if (dataViewProcBox.isInner(loc, 0.5))
            {
                dataViewProcBox.clamp(loc); //100722 RZC
                loc_vec.push_back(loc);
            }
        }
    }
    if(loc_vec.size()<1) return; // all points are outside the volume. ZJL 110913

#ifndef test_main_cpp
    // check if there is any existing neuron node is very close to the starting and ending points, if yes, then merge
    N = loc_vec.size(); //100722 RZC
    if (V3Dmainwindow && V3Dmainwindow->global_setting.b_3dcurve_autoconnecttips && b_use_seriespointclick==false)
    {
        if (listNeuronTree.size()>0 && curEditingNeuron>0 && curEditingNeuron<=listNeuronTree.size())
        {
            NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(curEditingNeuron-1)));
            if (p_tree)
            {
                double best_dist;
                V3DLONG n_id_start = findNearestNeuronNode_WinXY(list_listCurvePos.at(0).at(0).x, list_listCurvePos.at(0).at(0).y, p_tree, best_dist);
                V3DLONG n_id_end = findNearestNeuronNode_WinXY(list_listCurvePos.at(0).at(N-1).x, list_listCurvePos.at(0).at(N-1).y, p_tree, best_dist);
                qDebug("detect nearest neuron node [%ld] for curve-start and node [%ld] for curve-end for the [%d] neuron", n_id_start, n_id_end, curEditingNeuron);
                double th_merge = 5;

                bool b_start_merged=false, b_end_merged=false;
                NeuronSWC cur_node;
                if (n_id_start>=0)
                {
                    cur_node = p_tree->listNeuron.at(n_id_start);
                    qDebug()<<cur_node.x<<" "<<cur_node.y<<" "<<cur_node.z;
                    XYZ cur_node_xyz = XYZ(cur_node.x, cur_node.y, cur_node.z);
                    if (dist_L2(cur_node_xyz, loc_vec.at(0))<th_merge)
                    {
                        loc_vec.at(0) = cur_node_xyz;
                        b_start_merged = true;
                        qDebug()<<"force set the first point of this curve to the above neuron node as they are close.";
                    }
                }
                if (n_id_end>=0)
                {
                    cur_node = p_tree->listNeuron.at(n_id_end);
                    qDebug()<<cur_node.x<<" "<<cur_node.y<<" "<<cur_node.z;
                    XYZ cur_node_xyz = XYZ(cur_node.x, cur_node.y, cur_node.z);
                    if (dist_L2(cur_node_xyz, loc_vec.at(N-1))<th_merge)
                    {
                        loc_vec.at(N-1) = cur_node_xyz;
                        b_end_merged = true;
                        qDebug()<<"force set the last point of this curve to the above neuron node as they are close.";
                    }
                }
                //a special operation is that if the end point is merged, but the start point is not merged,
                //then this segment is reversed direction to reflect the prior knowledge that a neuron normally grow out as branches
                if (b_start_merged==false && b_end_merged==true)
                {
                    vector <XYZ> loc_vec_tmp = loc_vec;
                    for (int i=0;i<N;i++)
                        loc_vec.at(i) = loc_vec_tmp.at(N-1-i);
                }

            }
        }
    }
    //
    if (b_use_seriespointclick==false)
        smooth_curve(loc_vec, 5);

#endif

    int etime1 = t1a.restart();
    v3d_msg(QString("** time to calculate 3d curve costs [%1] ms.").arg(etime1),0);

#ifndef test_main_cpp //140211
    if (b_addthiscurve)
    {
        addCurveSWC(loc_vec, chno, 8); //LMG 26/10/2018 solveCurveCenter mode 8
        // used to convert loc_vec to NeuronTree and save SWC in testing
       // vecToNeuronTree(testNeuronTree, loc_vec);
        //added by PHC, 120506
        if (b_ablation)
            ablate3DLocationSeries(loc_vec);
        if (b_imaging || b_grabhighrez)
            produceZoomViewOf3DRoi(loc_vec,
                                   1  //one means from non-wheel event
                                   );
    }
    else //100821
    {
        b_addthiscurve = true; //in this case, always reset to default to draw curve to add to a swc instead of just  zoom
        endSelectMode();
        //added by PHC, 120506
        if (b_ablation)
            ablate3DLocationSeries(loc_vec);
        produceZoomViewOf3DRoi(loc_vec,
                               1  //one means from non-wheel event
                               );
    }
#endif

    v3d_msg(QString("** time to invoke subsquent operation 3d curve costs [%1] ms, to calculate curve costs [%2] ms.").arg(t1a.restart()).arg(etime1), 0);

}

void Renderer_gl1::setDeleteKey(int key)
{
    deleteKey = key;
}

void simple_delay(V3DLONG n) //delay n seconds
{
    QTime dieTime= QTime::currentTime().addSecs(n);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

bool Renderer_gl1::produceZoomViewOf3DRoi(vector <XYZ> & loc_vec, int ops_type)
{
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
#ifndef test_main_cpp
    My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
    XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);
    if (w && curImg && curXWidget && loc_vec.size()>0)
    {
        double mx, Mx, my, My, mz, Mz;
        mx = Mx = loc_vec.at(0).x;
        my = My = loc_vec.at(0).y;
        mz = Mz = loc_vec.at(0).z;
        for (V3DLONG i=1; i<loc_vec.size(); i++)
        {
            XYZ & curpos = loc_vec.at(i);
            if (curpos.x < mx) mx = curpos.x;
            else if (curpos.x > Mx) Mx = curpos.x;
            if (curpos.y < my) my = curpos.y;
            else if (curpos.y > My) My = curpos.y;
            if (curpos.z < mz) mz = curpos.z;
            else if (curpos.z > Mz) Mz = curpos.z;
        }
        qDebug()<< mx << " " << Mx << " " << my << " " << My << " " << mz << " " << Mz << " ";
        V3DLONG marginx,marginy,marginz; //the default margin is small
        marginx=marginy=marginz=5;
        if (loc_vec.size()==1)
        {
            marginx=marginy=marginz=61;
        }
        mx -= marginx; Mx += marginx; //if (mx<0) mx=0; if (Mx>curImg->getXDim()-1) Mx = curImg->getXDim()-1;
        my -= marginy; My += marginy; //if (my<0) my=0; if (My>curImg->getYDim()-1) My = curImg->getYDim()-1;
        mz -= marginz; Mz += marginz; //if (mz<0) mz=0; if (Mz>curImg->getZDim()-1) Mz = curImg->getZDim()-1;
//        by PHC 101008
        if (b_imaging && curXWidget)
        {
            b_imaging = false; //reset the status
            //set the hiddenSelectWidget for the V3D mainwindow
            if (!curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
            {
                v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
                return false;
            }
            //set up parameters
            v3d_imaging_paras myimagingp;
            myimagingp.OPS = "Acquisition: ROI from 3D Viewer";
            myimagingp.ops_type = ops_type;
            myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call
            myimagingp.xs = mx;
            myimagingp.ys = my;
            myimagingp.zs = mz; //starting coordinates (in pixel space)
            myimagingp.xe = Mx;
            myimagingp.ye = My;
            myimagingp.ze = Mz; //ending coordinates (in pixel space)
            myimagingp.xrez = curImg->getRezX() / 2.0;
            myimagingp.yrez = curImg->getRezY() / 2.0;
            myimagingp.zrez = curImg->getRezZ() / 2.0;
            //do imaging
            return v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
        }
        else if (b_grabhighrez && curXWidget) //120717
        {
            b_grabhighrez = false;
            //set the hiddenSelectWidget for the V3D mainwindow

            int tmpcnt=0;
            while (tmpcnt<100 && !curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
            {
                tmpcnt++;
                v3d_msg(QString("produceZoomViewOf3DRoi(): Try again [%1] to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.").arg(tmpcnt), 0);
                simple_delay(1);
            }
            if (!curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
            {
                v3d_msg("produceZoomViewOf3DRoi(): Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
                return false;
            }

            //set up parameters
            v3d_imaging_paras myimagingp;
            myimagingp.OPS = "Fetch Highrez Image Data from File"; //this is to open a new local 3D viewer window
            myimagingp.ops_type = ops_type;
            myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call
            myimagingp.xs = mx;
            myimagingp.ys = my;
            myimagingp.zs = mz; //starting coordinates (in pixel space)
            myimagingp.xe = Mx;
            myimagingp.ye = My;
            myimagingp.ze = Mz; //ending coordinates (in pixel space)
            myimagingp.xrez = curImg->getRezX() / 2.0;
            myimagingp.yrez = curImg->getRezY() / 2.0;
            myimagingp.zrez = curImg->getRezZ() / 2.0;
            //qDebug("move to rexyz %f and %f and %f",myimagingp.xrez,myimagingp.yrez,myimagingp.zrez);
            //do imaging
            //bool v3d_imaging_return_value = v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
            //cout << "v3d_imaging_return_value:" << v3d_imaging_return_value << endl;
            //return v3d_imaging_return_value;
            return v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
        }
        else //b_imaging does not open 3D Viewer here
        {
            curXWidget->setLocal3DViewerBBox(mx, Mx, my, My, mz, Mz);
            //QTimer::singleShot( 1000, curXWidget, SLOT(doImage3DLocalView()) );
            curXWidget->doImage3DLocalBBoxView(); //by PHC 101012. move from before if(b_imaging...)
            return true;
        }
    }
#endif //test_main_cpp

    return false;
}
void Renderer_gl1::ablate3DLocationSeries(vector <XYZ> & loc_vec) //added 120506, PHC
{
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
#ifndef test_main_cpp
    My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
    XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);
    if (w && curImg && curXWidget && loc_vec.size()>0)
    {
        double mx, Mx, my, My, mz, Mz;
        mx = Mx = loc_vec.at(0).x;
        my = My = loc_vec.at(0).y;
        mz = Mz = loc_vec.at(0).z;
        v3d_imaging_paras myimagingp;
        myimagingp.OPS = "Marker Ablation from 3D Viewer (with feedback)";
        myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call
        // resample landmarkers when using cutting curve line ablation
        if(b_lineAblation)
        {
            double rezx, rezy, rezz;
            rezx = curImg->getRezX();
            rezy = curImg->getRezY();
            rezz = curImg->getRezZ();
            cout <<"Image resolution: "<< rezx <<", "<<rezy <<", "<<rezz<<endl;
            // always use the first marker
            XYZ & pos0 = loc_vec.at(0);
            LocationSimple loc0;
            loc0.x = pos0.x;
            loc0.y = pos0.y;
            loc0.z = pos0.z;
            if (loc0.x>=0 && loc0.x<curImg->getXDim() &&
                    loc0.y>=0 && loc0.y<curImg->getYDim() &&
                    loc0.z>=0 && loc0.z<curImg->getZDim())
            {
                myimagingp.list_landmarks.push_back(loc0);
            }
            V3DLONG pre_pos_id = 0;
            for (V3DLONG i=1; i<loc_vec.size(); i++)
            {
                // curent pos
                XYZ & curpos = loc_vec.at(i);
                LocationSimple loc;
                loc.x = curpos.x;
                loc.y = curpos.y;
                loc.z = curpos.z;
                // previous pos
                XYZ & prepos = loc_vec.at(pre_pos_id);
                LocationSimple locpre;
                locpre.x = prepos.x;
                locpre.y = prepos.y;
                locpre.z = prepos.z;
                if (loc.x>=0 && loc.x<curImg->getXDim() &&
                    loc.y>=0 && loc.y<curImg->getYDim() &&
                    loc.z>=0 && loc.z<curImg->getZDim())
                {
                    // compute distance between two locs
                    double dist;
                    dist = sqrt((loc.x*rezx - locpre.x*rezx)*(loc.x*rezx - locpre.x*rezx) +
                        (loc.y*rezy - locpre.y*rezy)*(loc.y*rezy - locpre.y*rezy) + (loc.z*rezz - locpre.z*rezz)*(loc.z*rezz - locpre.z*rezz) );
                    if(dist > 2.0)
                    {
                        myimagingp.list_landmarks.push_back(loc);
                        pre_pos_id = i;
                    }
                }
            }
        }
        else // not curve cutting ablation
        {
            for (V3DLONG i=0; i<loc_vec.size(); i++) //i=1
            {
                XYZ & curpos = loc_vec.at(i);
                LocationSimple loc;
                loc.x = curpos.x;
                loc.y = curpos.y;
                loc.z = curpos.z;
                if (loc.x>=0 && loc.x<curImg->getXDim() &&
                    loc.y>=0 && loc.y<curImg->getYDim() &&
                    loc.z>=0 && loc.z<curImg->getZDim())
                    myimagingp.list_landmarks.push_back(loc);
            }
        }
        if (b_ablation && w && curImg && curXWidget && myimagingp.list_landmarks.size()>0)
        {
            //b_ablation = false; //reset the status. by Hanchuan peng, 120506 // reset b_ablation in endSelectMode() by Jianlong Zhou 20120726
            //set the hiddenSelectWidget for the V3D mainwindow
            if (!curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
            {
                v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
                return;
            }
            //do imaging
            if (curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
            {
                v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
            }
            else
            {
                v3d_msg("Fail to set up the curHiddenSelectedXWidget for the Vaa3D mainwindow. Do nothing.");
            }
        }
    }
#endif //test_main_cpp
}
void Renderer_gl1::solveCurveViews()
{
    qDebug("  Renderer_gl1::solveCurveViews");
    int NC = list_listCurvePos.size();
    int N = list_listCurvePos.at(0).size(); //points of first track
    if (NC<2 || N <3)  return; //data is not enough
    vector <XYZ> loc_vec;
    loc_vec.clear();
    for (int i=0; i<N; i++) //for each point in track 0
    {
        const MarkerPos & pos0 = list_listCurvePos.at(0).at(i);
        // work space for solving each control point of curve
        listMarkerPos.clear();
        listMarkerPos.append(pos0);
        for (int k=1; k<NC; k++) //for other views (except first track)
        {
            int NK = list_listCurvePos.at(k).size();
            MarkerPos closest_pos;
            double closest_dist;
            int jj;
            for (int j=0; j<NK; j++) //find the closest point to pos0
            {
                //qDebug("0(%d) -- %d(%d) ", i, k, j);
                const MarkerPos & pos = list_listCurvePos.at(k).at(j);
                double dist = distanceOfMarkerPos(pos0, pos);
                if (j==0 || dist < closest_dist)
                {
                    closest_pos = pos;
                    closest_dist = dist;
                    jj = j;
                }
            }
            listMarkerPos.append(closest_pos);
            qDebug("closest distance = %g (0(%d) -- %d(%d))", closest_dist, i, k, jj);
        }
        // solve one control point of curve
        XYZ loc = getLocationOfListMarkerPos();
        listMarkerPos.clear(); //
        if (isInBound(loc, 0.5, false))
        {
            loc_vec.push_back(loc);
        }
    }
#ifndef test_main_cpp
    smooth_curve(loc_vec, 5);
#endif
    addCurveSWC(loc_vec, -1, 9); //turn off post deform //LMG 26/10/2018 solveCurveViews mode 9
}
void Renderer_gl1::solveCurveFromMarkers()
{
    qDebug("  Renderer_gl1::solveCurveMarkers");
#ifndef test_main_cpp
    vector <XYZ> loc_vec_input;
    loc_vec_input.clear();
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* curImg = 0;
    if (w) curImg = v3dr_getImage4d(_idep); //by PHC, 090119
    if (curImg)
    {
        curImg->update_3drenderer_neuron_view(w, this);
        QList <LocationSimple> & listLoc = curImg->listLandmarks;
        int spos = listLoc.size()-cntCur3DCurveMarkers;
        XYZ cur_xyz;
        int i;
        for (i=spos;i<listLoc.size();i++)
        {
            cur_xyz.x = listLoc.at(i).x-1; //marker location is 1 based
            cur_xyz.y = listLoc.at(i).y-1;
            cur_xyz.z = listLoc.at(i).z-1;
            loc_vec_input.push_back(cur_xyz);
        }
        for (i=listLoc.size()-1;i>=spos;i--)
        {
            listLoc.removeLast();
        }
        updateLandmark();
    }
    if (loc_vec_input.size()>0)
    {
        qDebug("now pass to solveCurveCenter for point-click-3d-curve");
        solveCurveCenter(loc_vec_input);
    }
#endif
}
#define __creat_marker___
XYZ Renderer_gl1::getCenterOfMarkerPos(const MarkerPos& pos, int defaultChanno)
{
    qDebug("进入了Renderer_gl1::getCenterOfMarkerPos----------------");
    ////////////////////////////////////////////////////////////////////////
    int chno;
    if (defaultChanno>=0 && defaultChanno<dim4)
        chno = defaultChanno; //130424, by PHC
    else
        chno = checkCurChannel();

    double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
    clipplane[3] = viewClip;

    ViewPlaneToModel(pos.MV, clipplane);

    qDebug("ViewPlaneToModel之后 pos的值  (%g, %g)", pos.x,pos.y);
    //qDebug()<<"   clipplane:"<<clipplane[0]<<clipplane[1]<<clipplane[2]<<clipplane[3];
    ////////////////////////////////////////////////////////////////////////
    XYZ loc0, loc1;
    qDebug("_MarkerPos_to_NearFarPoint之前	loc0--loc1: (%g, %g, %g)--(%g, %g, %g)", loc0.x,loc0.y,loc0.z, loc1.x,loc1.y,loc1.z);

    qDebug("_MarkerPos_to_NearFarPoint之前 pos的值  (%g, %g)", pos.x,pos.y);
    _MarkerPos_to_NearFarPoint(pos, loc0, loc1);
    qDebug("_MarkerPos_to_NearFarPoint之后 pos的值  (%g, %g)", pos.x,pos.y);
    qDebug("_MarkerPos_to_NearFarPoint之后	loc0--loc1: (%g, %g, %g)--(%g, %g, %g)", loc0.x,loc0.y,loc0.z, loc1.x,loc1.y,loc1.z);
    XYZ loc;
    if (chno>=0 && chno<dim4)
    {

        loc = getCenterOfLineProfile(loc0, loc1, clipplane, chno);

    }
    else //find max value location in all channels
    {
        //走的是这
        float maxval, curval;
        for (int ichno=0; ichno<dim4; ichno++)
        {
            XYZ curloc = getCenterOfLineProfile(loc0, loc1, clipplane, ichno, &curval);
            if (ichno==0)
            {
                maxval = curval;
                loc = curloc;
                continue;
            }
            else if (curval > maxval)
            {
                maxval = curval;
                loc = curloc;
            }
        }
    }

    qDebug("走出了Renderer_gl1::getCenterOfMarkerPos----------------");
    return loc;
}
double Renderer_gl1::solveMarkerCenter()
{
    QElapsedTimer t;
    t.start();
    if (listMarkerPos.size()<1)  return t.elapsed();
    const MarkerPos & pos = listMarkerPos.at(0);
//    MarkerPos pos = listMarkerPos.at(0);
//    pos.x = 360.00;
//    pos.y = 380.00;
    qDebug("第一步传进去的 pos (%g, %g)", pos.x,pos.y);

    XYZ loc = getCenterOfMarkerPos(pos);



    //这就是最终转化的3d坐标了

    qDebug("getCenterOfMarkerPos之后 loc的值 这算是最后转化的3d坐标了 (%g, %g, %g)", loc.x,loc.y,loc.z);

    vector <XYZ> loc_vec;
    if (dataViewProcBox.isInner(loc, 0.5)) //100725 RZC
        dataViewProcBox.clamp(loc); //100722 RZC
    if (b_addthismarker) //100822, PHC, 120506
    {
        addMarker(loc);
        if (b_ablation)
        {
            loc_vec.push_back(loc);
            ablate3DLocationSeries(loc_vec);
        }
        if (b_imaging || b_grabhighrez)
        {
            loc_vec.push_back(loc);
            produceZoomViewOf3DRoi(loc_vec);
        }
    }
    else //then zoom-in, 100822, PHC
    {
        b_addthismarker = true; //in this case, always reset to default to add a marker instead of just  zoom
        endSelectMode();
        loc_vec.push_back(loc);
        if (b_ablation)
            ablate3DLocationSeries(loc_vec);
        produceZoomViewOf3DRoi(loc_vec,
                               1  //one means from non-wheel event
                               );
    }
    return t.elapsed(); //note that the elapsed time may not be correct for the zoom-in view generation, as it calls endSelectMode(0 in which I reset the total_etime., by PHC, 20120419
}
double Renderer_gl1::solveMarkerCenterMaxIntensity()
{
    int chno = 0;
    double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
    clipplane[3] = viewClip;
    QElapsedTimer t;
    t.start();
    if (listMarkerPos.size()<1)  return t.elapsed();
    const MarkerPos & pos = listMarkerPos.at(0);
    XYZ loc = getCenterOfMarkerPos(pos);
    vector <XYZ> loc_vec;
    if (dataViewProcBox.isInner(loc, 0.5)) //100725 RZC
        dataViewProcBox.clamp(loc); //100722 RZC
    // Find max intensity along z (dataViewProcBox.z0 -> dataViewProcBox.z1)

    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* curImg = v3dr_getImage4d(_idep);
    float max_val = 0.0f;
    XYZ max_loc;
    if (curImg && data4dp)
    {
        unsigned char* vp = 0;
        switch (curImg->getDatatype())
        {
            case V3D_UINT8:
                vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1);
                break;
            case V3D_UINT16:
                vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(short int);
                break;
            case V3D_FLOAT32:
                vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(float);
                break;
            default:
                v3d_msg("Unsupported data type found. You should never see this.", 0);
                return 0;
        }
        const V3DLONG RAD = 3;
        float value;
        for (V3DLONG zz=dataViewProcBox.z0; zz <= dataViewProcBox.z1; zz++)
        {
            for (V3DLONG dy=-RAD; dy<=RAD; dy++)
            {
                for (V3DLONG dx=-RAD; dx<=RAD; dx++)
                {
                    // Only process points within the desired radius
                    if (dx*dx + dy*dy > RAD*RAD)
                        continue;
                    XYZ P = XYZ(loc.x + dx, loc.y + dy, zz);
                    switch (curImg->getDatatype())
                    {
                        case V3D_UINT8:
                            value = sampling3dAllTypesatBounding( vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                            break;
                        case V3D_UINT16:
                            value = sampling3dAllTypesatBounding( (short int *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                            break;
                        case V3D_FLOAT32:
                            value = sampling3dAllTypesatBounding( (float *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                            break;
                        default:
                            v3d_msg("Unsupported data type found. You should never see this.", 0);
                            return 0;
                    }
                    if (value > max_val)
                    {
                        max_val = value;
                        max_loc = P;
                    }
                }
            }
        }
        if (max_val > 0.0f)
        {
            addMarker(max_loc);
        }
    }
    return t.elapsed(); //note that the elapsed time may not be correct for the zoom-in view generation, as it calls endSelectMode(0 in which I reset the total_etime., by PHC, 20120419
}
void Renderer_gl1::solveMarkerViews()
{
    if (listMarkerPos.size()<2)  return;
    XYZ loc = getLocationOfListMarkerPos();
    if (isInBound(loc, 0.01, true))
    {
        addMarker(loc);
    }
}
void Renderer_gl1::refineMarkerTranslate()
{
    if (currentMarkerName<1 || currentMarkerName>listMarker.size())  return;
    if (listMarkerPos.size()<1)  return;
    const MarkerPos & pos = listMarkerPos.at(0);
    const ImageMarker & S = listMarker[currentMarkerName-1];
    XYZ loc = getTranslateOfMarkerPos(pos, S); //what is the problem here with the new XCode compiler?? by PHC20131029
    //added by PHC, 090120. update the marker location in both views
    updateMarkerLocation(currentMarkerName-1, loc);
}

void Renderer_gl1::refineMarkerCenter()
{
    if (currentMarkerName<1 || currentMarkerName>listMarker.size())  return;
    if (listMarkerPos.size()<1)  return;
    const MarkerPos & pos = listMarkerPos.at(0);
    XYZ loc = getCenterOfMarkerPos(pos);
    //added by PHC, 090120. update the marker location in both views
    updateMarkerLocation(currentMarkerName-1, loc);
}
void Renderer_gl1::refineMarkerLocal(int marker_id)
{
    if (marker_id<0 || marker_id>listMarker.size()-1)  return;
    XYZ P = XYZ(listMarker[marker_id])-XYZ(1,1,1);
    XYZ loc = getCenterOfLocal(P);
    //added by PHC, 090120. update the marker location in both views
    updateMarkerLocation(marker_id, loc);
}
void Renderer_gl1::addMarker(XYZ &loc)
{
    XYZ pt(loc.x+1, loc.y+1, loc.z+1); // marker position is 1-based
#ifndef test_main_cpp
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* image4d = v3dr_getImage4d(_idep);
    MainWindow* V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);
    if (image4d)
    {
        QList <LocationSimple> & listLoc = image4d->listLandmarks;
        LocationSimple S;
        V3DLONG markerindex = -1; // the index of no special marker, by XZ, 20190721
        for(V3DLONG i=listLoc.size()-1;i>=0;--i)
        {
            if(listLoc.at(i).category!=77)
            {
                markerindex = i;
                break;
            }
        }
        if (markerindex>=0/*listLoc.size()>0*/)
        {
            S.inputProperty = listLoc.at(markerindex).inputProperty;
            S.comments = listLoc.at(markerindex).comments;
            S.category = listLoc.at(markerindex).category;
            S.color = listLoc.at(markerindex).color;
            currentMarkerColor = listLoc.at(markerindex).color;;
        }
        else
        {
            S.inputProperty = pxLocaUseful;
            //S.color = random_rgba8(255);
            S.color = currentMarkerColor;
        }
        S.x = pt.x;
        S.y = pt.y;
        S.z = pt.z;
        if (V3Dmainwindow)
            S.radius = V3Dmainwindow->global_setting.default_marker_radius;
        S.on = true;
        listLoc.append(S);
        updateLandmark();
    }
#else
    ImageMarker S;
    memset(&S, 0, sizeof(S));
    S.x = pt.x;
    S.y = pt.y;
    S.z = pt.z;
    if (listMarker.size()>0)
    {
        S.color = listMarker.last().color;
    }
    else
    {
        S.color = random_rgba8(255);
    }
    S.on = true;
    listMarker.append(S);
#endif
}

void Renderer_gl1::addMarkerUnique(XYZ &loc)
{

    XYZ pt(loc.x + 1, loc.y + 1, loc.z + 1); // marker position is 1-based
#ifndef test_main_cpp
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* image4d = v3dr_getImage4d(_idep);
    MainWindow* V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);

    if (image4d)
    {
        /*if (image4d->listLandmarks.size() > 0){
            image4d->listLandmarks.clear();
            updateLandmark();
            return;
        }*/

        QList <LocationSimple> & listLoc = image4d->listLandmarks;
        LocationSimple S;
        V3DLONG markerindex = -1; // the index of no special marker, by XZ, 20190721
        for (V3DLONG i = listLoc.size() - 1; i >= 0; --i)
        {
            if (listLoc.at(i).category != 77)
            {
                markerindex = i;
                break;
            }
        }
        if (markerindex >= 0/*listLoc.size()>0*/)
        {
            S.inputProperty = listLoc.at(markerindex).inputProperty;
            S.comments = listLoc.at(markerindex).comments;
            S.category = listLoc.at(markerindex).category;
            S.color = listLoc.at(markerindex).color;
            currentMarkerColor = listLoc.at(markerindex).color;;
        }
        else
        {
            S.inputProperty = pxLocaUseful;
            //S.color = random_rgba8(255);
            S.color = currentMarkerColor;
        }
        S.x = pt.x;
        S.y = pt.y;
        S.z = pt.z;
        if (V3Dmainwindow)
            S.radius = V3Dmainwindow->global_setting.default_marker_radius;
        S.on = true;
        listLoc.append(S);
        updateLandmark();
    }
#else
    ImageMarker S;
    memset(&S, 0, sizeof(S));
    S.x = pt.x;
    S.y = pt.y;
    S.z = pt.z;
    if (listMarker.size()>0)
    {
        S.color = listMarker.last().color;
    }
    else
    {
        S.color = random_rgba8(255);
    }
    S.on = true;
    listMarker.append(S);
#endif
}


void Renderer_gl1::addSpecialMarker(XYZ &loc)
{
    XYZ pt(loc.x+1, loc.y+1, loc.z+1); // marker position is 1-based
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* image4d = v3dr_getImage4d(_idep);
    MainWindow* V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);
    if (image4d)
    {
        QList <LocationSimple> & listLoc = image4d->listLandmarks;
        LocationSimple S;
        if (listLoc.size()>0)
        {
            S.inputProperty = listLoc.last().inputProperty;
            //S.comments = "special marker";
            //S.category = listLoc.last().category;
            S.color = listLoc.last().color;
            currentMarkerColor = listLoc.last().color;;
        }
        else
        {
            S.inputProperty = pxLocaUseful;
            //S.color = random_rgba8(255);
            S.color = currentMarkerColor;
        }
        S.comments = "special marker";
        S.category = 77;
        S.x = pt.x;
        S.y = pt.y;
        S.z = pt.z;
        if (V3Dmainwindow)
            S.radius = V3Dmainwindow->global_setting.default_marker_radius;
        S.on = true;
        listLoc.append(S);
        updateLandmark();
    }
}//add special marker, by XZ, 20190720
void Renderer_gl1::updateMarkerLocation(int marker_id, XYZ &loc) //added by PHC, 090120
{
    XYZ pt(loc.x+1, loc.y+1, loc.z+1); // 090505 RZC : marker position is 1-based
    // 090617 RZC: move from refine*() to make consisted
    ImageMarker & S = listMarker[marker_id];
    S.x = pt.x; //090513 RZC: fixed from S.x = Y(1);
    S.y = pt.y;
    S.z = pt.z;
#ifndef test_main_cpp
    My4DImage* image4d = v3dr_getImage4d(_idep);
    if (image4d)
    {
        QList <LocationSimple> & listLoc = image4d->listLandmarks;
        if (marker_id<0 || marker_id>listLoc.size()-1)
        {
            printf("The marker_id is out of range in replaceMarker(). Check your data and code. !\n");
            return;
        }
        LocationSimple & S = listLoc[marker_id] ;
        S.x = pt.x;
        S.y = pt.y;
        S.z = pt.z;
    }
#endif
}
bool Renderer_gl1::isInBound(const XYZ & loc, float factor, bool b_message)
{
    float dZ = MAX((dim1+dim2+dim3)/3.0*factor, 10);
    if (loc.z >= -dZ  &&  loc.z < dim3+dZ &&
            loc.y >= -dZ  &&  loc.y < dim2+dZ &&
            loc.x >= -dZ  &&  loc.x < dim1+dZ)
    {
        return true;
    }
    else
    {
        printf("*** ERROR: location is out of bound in solveMarker()!\n");
        if (b_message)
            QMessageBox::warning( 0, "warning",
                    QObject::tr("ERROR: location is out of image bound too much!\n\n location: (%1, %2, %3) \n\n").arg(loc.x).arg(loc.y).arg(loc.z) +
                    QObject::tr(" bound: X[0, %1) Y[0, %2) Z[0, %3)\n\n").arg(dim1).arg(dim2).arg(dim3) +
                    QObject::tr("3D View: Please make clicks in different view directions and click the same 3D location.\n\n") );
        return false;
    }
}

#define ___computation_functions___

//
XYZ Renderer_gl1::getTranslateOfMarkerPos(const MarkerPos& pos, const ImageMarker& S)
{
    XYZ pt(S.x-1, S.y-1, S.z-1); // 090505 RZC : marker position is 1-based
    ColumnVector X(4);		X << pt.x << pt.y << pt.z << 1;

    Matrix P(4,4);		P << pos.P;   P = P.t();    // OpenGL is row-inner / C is column-inner
    Matrix M(4,4);		M << pos.MV;  M = M.t();
    Matrix PM = P * M;

    ColumnVector pX  = PM * X;

    pX = pX / pX(4);
    //cout << " pX \n" << pX.t() << endl;
    double x = (pos.x             - pos.view[0])*2.0/pos.view[2] -1;
    double y = (pos.view[3]-pos.y - pos.view[1])*2.0/pos.view[3] -1; // OpenGL is bottom to top
    double z = pX(3);                              // hold the clip space depth
    ColumnVector pY(4); 	pY << x << y << z << 1;
    ColumnVector Y = PM.i() * pY;
    Y = Y / Y(4);
    //std::cout << "refine from: " << X.t()  << "   to: " << Y.t() << endl; //20131029. by PHC avoid OSX 10.9 issue
    XYZ loc(Y(1), Y(2), Y(3));
    return loc;
}

// in Image space (model space)
void Renderer_gl1::_MarkerPos_to_NearFarPoint(const MarkerPos & pos, XYZ &loc0, XYZ &loc1)
{
    Matrix P(4,4);		P << pos.P;   P = P.t();    // OpenGL is row-inner / C is column-inner
    Matrix M(4,4);		M << pos.MV;  M = M.t();
    Matrix PM = P * M;


    double x = (pos.x             - pos.view[0])*2.0/pos.view[2] -1;
    double y = (pos.view[3]-pos.y - pos.view[1])*2.0/pos.view[3] -1; // OpenGL is bottom to top
    //double z = 0,1;                              // the clip space depth from 0 to 1
    ColumnVector pZ0(4); 	pZ0 << x << y << 0 << 1;
    ColumnVector pZ1(4); 	pZ1 << x << y << 1 << 1;
    if (bOrthoView)
    {
        pZ0(3) = -1;  //100913
    }
    ColumnVector Z0 = PM.i() * pZ0;       //cout << "Z0 \n" << Z0 << endl;
    ColumnVector Z1 = PM.i() * pZ1;       //cout << "Z1 \n" << Z1 << endl;
    Z0 = Z0 / Z0(4);
    Z1 = Z1 / Z1(4);
    loc0 = XYZ(Z0(1), Z0(2), Z0(3));
    loc1 = XYZ(Z1(1), Z1(2), Z1(3));
}

double Renderer_gl1::distanceOfMarkerPos(const MarkerPos & pos0, const MarkerPos & pos)
{
    XYZ Y1, Y2;
    _MarkerPos_to_NearFarPoint(pos, Y1, Y2);
    XYZ X1, X2;
    _MarkerPos_to_NearFarPoint(pos0, X1, X2);
    XYZ D  = Y2-Y1; normalize(D);
    XYZ D0 = X2-X1; normalize(D0);
    XYZ H = cross(D0, D); normalize(H);
    double dist = fabs(dot(H, X1-Y1));
    return dist;
    //	// pos's epipolar line in pos0's image space
    //
    //	Matrix P0(4,4);		P0 << pos0.P;   P0 = P0.t();    // OpenGL is row-inner / C is column-inner
    //	Matrix M0(4,4);		M0 << pos0.MV;  M0 = M0.t();
    //	Matrix PM0 = P0 * M0;
    //
    //	ColumnVector X0 = PM0 * Y0;
    //	X0 = X0 / X0(4);
    //	ColumnVector X1 = PM0 * Y1;
    //	X1 = X1 / X1(4);
    //
    //	XYZ A(X0(1), X0(2), X0(3));
    //	XYZ B(X1(1), X1(2), X1(3));
    //	XYZ L = cross(A, B);
    //
    //	// point to line in image
    //
    //	double x0 = (pos0.x              - pos0.view[0])*2.0/pos0.view[2] -1;
    //	double y0 = (pos0.view[3]-pos0.y - pos0.view[1])*2.0/pos0.view[3] -1; // OpenGL is bottom to top
    //	double dist = fabs(x0*L.x + y0*L.y + L.z)/sqrt(L.x*L.x + L.y*L.y);
    //	return dist;
}

XYZ Renderer_gl1::getLocationOfListMarkerPos()
{
    int N = listMarkerPos.size();
    if (N<2)
        qDebug("getLocationOfListMarkerPos ERROR: listMarkerPos.size()<2");
    Matrix A(N*3, 4);
    for (int i=0; i<N; i++)
    {
        const MarkerPos & pos = listMarkerPos.at(i);
        double x = (pos.x             - pos.view[0])*2.0/pos.view[2] -1;
        double y = (pos.view[3]-pos.y - pos.view[1])*2.0/pos.view[3] -1; // OpenGL is bottom to top
        double z = 1;
        Matrix Cx(3,3);
        Cx  << 0 << -z << y
            << z << 0 << -x
            << -y << x << 0;
        Matrix P(4,4);		P << pos.P;   P = P.t();    // OpenGL is row-inner / C is column-inner
        Matrix M(4,4);		M << pos.MV;  M = M.t();
        Matrix PM = P * M;
        Matrix Pr(3, 4);
        Pr.row(1) = PM.row(1);
        Pr.row(2) = PM.row(2);
        Pr.row(3) = PM.row(4);  // drop PM.row(3), index is 1-based
        //cout << "Cx P M Pr \n" << Cx << endl << P << endl << M << endl << Pr << endl;
        Matrix As = Cx * Pr;
        //cout << i << " As \n" << As << endl;
        A.rows(i*3+1, i*3+3) << As;
    }
    DiagonalMatrix D;  Matrix U, V;
    SVD(A, D, U, V);
    //cout << "svd: A D V\n" << A << endl << D << endl << V << endl;
    ColumnVector X = V.column(4) / V(4,4);
    //cout << "location: " << X.t() << endl;
    XYZ loc(X(1), X(2), X(3));
    return loc;
}

XYZ Renderer_gl1::getPointOnPlane(XYZ P1, XYZ P2, double plane[4]) //100731
{
    //         A*N + d
    //  t = -------------
    //        (A - B)*N
    double t = (P1.x*plane[0] + P1.y*plane[1] + P1.z*plane[2] + plane[3])
        /((P1.x-P2.x)*plane[0] + (P1.y-P2.y)*plane[1] + (P1.z-P2.z)*plane[2]);
    XYZ loc = P1 + t*(P2-P1);
    return loc;
}

// in Image space (model space)
XYZ Renderer_gl1::getPointOnSections(XYZ P1, XYZ P2, double F_plane[4]) //100801
{
    double plane[4];
    XYZ P = P2; // from the far location
    XYZ loc;
#define REPLACE_NEAR( plane ) { \
    loc = getPointOnPlane(P1,P2, plane); \
    if (dist_L2(loc,P2) > dist_L2(P,P2) && dataViewProcBox.isInner(loc, 0.5)) \
    P = loc; \
    }
    //qDebug("  P1(%g %g %g)  P2(%g %g %g)", P1.x,P1.y,P1.z, P2.x,P2.y,P2.z);
    if (bXSlice)
    {
        plane[0] = -1; plane[1] = 0; plane[2] = 0; plane[3] = start1+ VOL_X0*(size1-1);
        REPLACE_NEAR( plane );
        //qDebug("  X-(%g %g %g)", loc.x,loc.y,loc.z);
    }
    if (bYSlice)
    {
        plane[0] = 0; plane[1] = -1; plane[2] = 0; plane[3] = start2+ VOL_Y0*(size2-1);
        REPLACE_NEAR( plane );
        //qDebug("  Y-(%g %g %g)", loc.x,loc.y,loc.z);
    }
    if (bZSlice)
    {
        plane[0] = 0; plane[1] = 0; plane[2] = -1; plane[3] = start3+ VOL_Z0*(size3-1);
        REPLACE_NEAR( plane );
        //qDebug("  Z-(%g %g %g)", loc.x,loc.y,loc.z);
    }
    if (bFSlice)
    {
        if (F_plane)
            for (int i=0; i<4; i++) plane[i] = F_plane[i];
        else
        {
            ////////////////////////////////////////////////////////////////////////
            //100730 RZC, in View space, keep for dot(clip, pos)>=0
            double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
            clipplane[3] = viewClip;
            ViewPlaneToModel(markerViewMatrix, clipplane);
            //qDebug()<<"   clipplane:"<<clipplane[0]<<clipplane[1]<<clipplane[2]<<clipplane[3];
            ////////////////////////////////////////////////////////////////////////
            for (int i=0; i<4; i++) plane[i] = clipplane[i];
        }
        REPLACE_NEAR( plane );
        //qDebug("  F-(%g %g %g)", loc.x,loc.y,loc.z);
    }
    //qDebug("  P(%g %g %g)", P.x,P.y,P.z);
    return P;
}

// in Image space (model space)
XYZ Renderer_gl1::getCenterOfLineProfile(XYZ P1, XYZ P2,
        double clipplane[4],	//clipplane==0 means no clip plane
        int chno,    			//must be a valid channel number
        float *p_value			//if p_value!=0, output value at center
        )
{

    if (renderMode==rmCrossSection)
    {
        return getPointOnSections(P1,P2, clipplane); //clip plane also is the F-plane
    }
    XYZ loc = (P1+P2)*0.5;





#ifndef test_main_cpp
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* curImg = v3dr_getImage4d(_idep);
    if (curImg && data4dp && chno>=0 &&  chno <dim4)
    {
        double f = 0.8; // must be LESS 1 to converge, close to 1 is better
        XYZ D = P2-P1; normalize(D);
        unsigned char* vp = 0;
        switch (curImg->getDatatype())
        {
            case V3D_UINT8:
                vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1);
                break;
            case V3D_UINT16:
                vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(short int);
                break;
            case V3D_FLOAT32:
                vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(float);
                break;
            default:
                v3d_msg("Unsupported data type found. You should never see this.", 0);
                return loc;
        }
        for (int i=0; i<200; i++) // iteration, (1/f)^200 is big enough
        {
            double length = norm(P2-P1);
            if (length < 0.5) // pixel
                break; //////////////////////////////////
            int nstep = int(length + 0.5);
            double step = length/nstep;
            XYZ sumloc(0,0,0);
            float sum = 0;
            for (int i=0; i<=nstep; i++)
            {
                XYZ P = P1 + D*step*(i);
                float value;
                switch (curImg->getDatatype())
                {
                    case V3D_UINT8:
                        value = sampling3dAllTypesatBounding( vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                        break;
                    case V3D_UINT16:
                        value = sampling3dAllTypesatBounding( (short int *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                        break;
                    case V3D_FLOAT32:
                        value = sampling3dAllTypesatBounding( (float *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                        break;
                    default:
                        v3d_msg("Unsupported data type found. You should never see this.", 0);
                        return loc;
                }
                sumloc = sumloc + P*(value);
                sum = sum + value;
            }
            if (sum)
                loc = sumloc / sum;
            else
                break; //////////////////////////////////
            P1 = loc - D*(length*f/2);
            P2 = loc + D*(length*f/2);
        }
        if (p_value)
        {
            XYZ P = loc;
            float value;
            switch (curImg->getDatatype())
            {
                case V3D_UINT8:
                    value = sampling3dAllTypesatBounding( vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                    break;
                case V3D_UINT16:
                    value = sampling3dAllTypesatBounding( (short int *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                    break;
                case V3D_FLOAT32:
                    value = sampling3dAllTypesatBounding( (float *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                    break;
                default:
                    v3d_msg("Unsupported data type found. You should never see this.", 0);
                    return loc;
            }
            *p_value = value;
        }
    }//valid data
#endif
    //100721: Now small tag added in sampling3dUINT8atBounding makes loc in bound box
    return loc;
}


int Renderer_gl1::getVolumeXsectPosOfMarkerLine(XYZ & locA, XYZ & locB, const MarkerPos& pos, int defaultChanno)
{
    ////////////////////////////////////////////////////////////////////////
    int chno;
    if (defaultChanno>=0 && defaultChanno<dim4)
        chno = defaultChanno; //130424, by PHC
    else
        chno = checkCurChannel();
    ////////////////////////////////////////////////////////////////////////
    qDebug()<<"\n  3d marker in channel # "<<((chno<0)? chno :chno+1);
    ////////////////////////////////////////////////////////////////////////
    //in View space, keep for dot(clip, pos)>=0
    double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
    clipplane[3] = viewClip;
    ViewPlaneToModel(pos.MV, clipplane);
    //qDebug()<<"   clipplane:"<<clipplane[0]<<clipplane[1]<<clipplane[2]<<clipplane[3];
    ////////////////////////////////////////////////////////////////////////
    XYZ loc0, loc1;
    _MarkerPos_to_NearFarPoint(pos, loc0, loc1);
    // qDebug("	loc0--loc1: (%g, %g, %g)--(%g, %g, %g)", loc0.x,loc0.y,loc0.z, loc1.x,loc1.y,loc1.z);
    XYZ loc;
    if (chno>=0 && chno<dim4)
    {
        getVolumeXsectPosOfMarkerLine(loc0, loc1, clipplane, chno, locA, locB);
    }
    else //use locations in all channels
    {
        float maxval, curval;
        for (int ichno=0; ichno<dim4; ichno++)
        {
            XYZ curlocA, curlocB;
            getVolumeXsectPosOfMarkerLine(loc0, loc1, clipplane, ichno, curlocA, curlocB);

            if (ichno==0)
            {
                locA = curlocA;
                locB = curlocB;
            }
            else
            {
                locA = locA+curlocA;
                locB = locB+curlocB;
            }
        }

        locA = locA*(1.0/dim4);
        locB = locB*(1.0/dim4);
    }

    return 1;
}

// in Image space (model space), by PHC 20130425
int Renderer_gl1::getVolumeXsectPosOfMarkerLine(XYZ P1, XYZ P2,
        double clipplane[4],	//clipplane==0 means no clip plane
        int chno,    			//must be a valid channel number
        XYZ & posCloseToP1, XYZ & posCloseToP2 //output
        )
{
    int VIS_TH=1; //a threshold determining what intensity of a pixel is "visible"

    XYZ loc = (P1+P2)*0.5;
#ifndef test_main_cpp
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* curImg = v3dr_getImage4d(_idep);
    if (curImg && data4dp && chno>=0 &&  chno <dim4)
    {
        XYZ D = P2-P1; normalize(D);
        unsigned char* vp = 0;
        switch (curImg->getDatatype())
        {
            case V3D_UINT8:
                vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1);
                break;
            case V3D_UINT16:
                vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(short int);
                break;
            case V3D_FLOAT32:
                vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(float);
                break;
            default:
                v3d_msg("Unsupported data type found. You should never see this.", 0);
                return 0;
        }

        {
            double length = norm(P2-P1);
            if (length < 0.5) // pixel
            {
                posCloseToP1 = P1;
                posCloseToP2 = P2;
                return 0;
            }

            int nstep = int(length + 0.5);
            double step = length/nstep;
            XYZ sumloc(0,0,0);
            float sum = 0;
            int i;
            for (i=0; i<=nstep; i++)
            {
                XYZ P = P1 + D*step*(i);
                float value;
                switch (curImg->getDatatype())
                {
                    case V3D_UINT8:
                        value = sampling3dAllTypesatBounding( vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                        break;
                    case V3D_UINT16:
                        value = sampling3dAllTypesatBounding( (short int *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                        break;
                    case V3D_FLOAT32:
                        value = sampling3dAllTypesatBounding( (float *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                        break;
                    default:
                        v3d_msg("Unsupported data type found. You should never see this.", 0);
                        return 0;
                }

                if (value>VIS_TH)
                {
                    posCloseToP1 = P;
                    break;
                }
            }
            if (i==nstep)
            {
                posCloseToP1 = (P1+P2)*0.5;
                posCloseToP2 = (P1+P2)*0.5;
                return 0; //in this case, no need to do the following any more
            }

            for (i=0; i<=nstep; i++)
            {
                XYZ P = P2 - D*step*(i);
                float value;
                switch (curImg->getDatatype())
                {
                    case V3D_UINT8:
                        value = sampling3dAllTypesatBounding( vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                        break;
                    case V3D_UINT16:
                        value = sampling3dAllTypesatBounding( (short int *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                        break;
                    case V3D_FLOAT32:
                        value = sampling3dAllTypesatBounding( (float *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
                        break;
                    default:
                        v3d_msg("Unsupported data type found. You should never see this.", 0);
                        return 0;
                }

                if (value>VIS_TH)
                {
                    posCloseToP2 = P;
                    break;
                }
            }

        }

    }//valid data
#endif
    return 1;
}


#define NUM_ITER_POSITION 10
bool fit_center_position(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
        XYZ & P)
{
#ifndef test_main_cpp
    double imgAve = getImageAveValue(img3d, dim0, dim1, dim2);
    double imgStd = getImageStdValue(img3d, dim0, dim1, dim2);
    double imgTH = imgAve + imgStd;
    float my_zthickness=1.0;
    bool my_b_est_in_xyplaneonly = false;
    double AR = (dim0 + dim1 + dim2)/6;
    {
        float x = P.x;
        float y = P.y;
        float z = P.z;
        double r = AR;
        qDebug("fit_center_position P: (%g %g %g) %g",  x,y,z,r);
        for (int j=0; j<NUM_ITER_POSITION; j++)
        {
            fitPosition(img3d, dim0, dim1, dim2,   0,   r*2, x, y, z);
            r = fitRadiusPercent(img3d, dim0, dim1, dim2, imgTH,  AR*2, x, y, z, my_zthickness, my_b_est_in_xyplaneonly);
            qDebug("fit_center_position %d: (%g %g %g) %g", j, x,y,z, r);
        }
        P.x = x;
        P.y = y;
        P.z = z;
    }
#endif
    return true;
}
XYZ Renderer_gl1::getCenterOfLocal(XYZ P)
{
#ifndef test_main_cpp
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* curImg = 0;
    int chno = -1;
    if (w)
    {
        curImg = v3dr_getImage4d(_idep);//->image4d; //by PHC, 090119
        chno = w->getNumKeyHolding()-1;
        if (chno<0) chno = 0; //default channel 1
    }
    if (curImg && data4dp && chno>=0 &&  chno <dim4)
    {
        unsigned char*** img3d = data4d_uint8[chno + volTimePoint*dim4];
        fit_center_position(img3d, dim1, dim2, dim3, P);
    }
#endif
    //	// guarantee that the location is in image
    //	if (loc.x < 0) loc.x = 0;  if (loc.x >= dim1) loc.x = dim1-1;
    //	if (loc.y < 0) loc.y = 0;  if (loc.y >= dim2) loc.y = dim2-1;
    //	if (loc.z < 0) loc.z = 0;  if (loc.z >= dim3) loc.z = dim3-1;
    return P;
}

int Renderer_gl1::zoomview_wheel_event()//by PHC, 20130424
{
    //first check the wheelPos is valid, if not, then reset to the center of the viewport

    //qDebug("wheel pos x=%5.3f,y=%5.3f, view port[%d, %d, %d, %d]",wheelPos.x,wheelPos.y, viewport[0], viewport[1], viewport[2], viewport[3]);
    if (wheelPos.x<viewport[0]+5 || wheelPos.x>=viewport[2]-5 ||
        wheelPos.y<viewport[1]+5 || wheelPos.y>=viewport[3]-5 ) //add a margin 5 to force have a hitWhell event in case wheelPos has not been initialized
    {
        //v3d_msg("reset wheel pos in zoomview_wheel_event()");
        hitWheel((viewport[0]+viewport[2])/2.0, (viewport[1]+viewport[3])/2.0);
    }

    //find the center line XYZ loc, and then 4 corner's XYZ locs, then based on all five locs define the bounding box and return
    QList <MarkerPos> curlist;
    //curlist.append(wheelPos);
    qDebug("wheel pos x=%5.3f,y=%5.3f",wheelPos.x,wheelPos.y);

    int i;
    for (i=0; i<5; i++)
    {
        MarkerPos p;
        p = wheelPos;
        switch (i)
        {
            case 0: p.x = (wheelPos.view[0]+wheelPos.view[2])/2.0; p.y = (wheelPos.view[1]+wheelPos.view[3])/2.0; break;
            case 1: p.x = wheelPos.view[0]+1; p.y = wheelPos.view[1]+1; break;
            case 2: p.x = wheelPos.view[0]+1; p.y = wheelPos.view[3]-1; break;
            case 3: p.x = wheelPos.view[2]-1; p.y = wheelPos.view[1]+1; break;
            case 4: p.x = wheelPos.view[2]-1; p.y = wheelPos.view[3]-1; break;
            default:break;
        }
        curlist.append(p);
        qDebug("i=%d, x=%5.3f,y=%5.3f",i, p.x,p.y);
    }

    QElapsedTimer t1;
    t1.start();

    vector <XYZ> loc_vec;
    for (i=0;i<curlist.size();i++)
    {
        //XYZ loc_m = getCenterOfMarkerPos(curlist.at(i));

        XYZ loc0, loc1;
        getVolumeXsectPosOfMarkerLine(loc0, loc1, curlist.at(i), 0);

        //XYZ loc = (loc0+loc1)*0.5;

        loc_vec.push_back(loc0);
        loc_vec.push_back(loc1);

        qDebug("i=%d loc0(%5.3f %5.3f %5.3f) loc1(%5.3f,%5.3f,%5.3f) \n",
               i, loc0.x, loc0.y, loc0.z, loc1.x, loc1.y, loc1.z
               );

        //qDebug("%5.3f, %5.3f, %5.3f,1,1,,,\n%5.3f,%5.3f,%5.3f,1,1,,, \n", loc0.x, loc0.y, loc0.z, loc1.x, loc1.y, loc1.z);

    }
    v3d_msg(QString("** time to find zoom-in area costs [%1] ms.").arg(t1.elapsed()),0);

    //check if terafly exists
    QDir pluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
        pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#endif

    QDir pluginsDir1 = pluginsDir;
    if (pluginsDir1.cd("plugins/teramanager")==true)
    {
        b_grabhighrez = true;
        if (!produceZoomViewOf3DRoi(loc_vec,
                               2    //2 means zoom-in from wheel event
                               ))
        {  //if fail, which means the data is returned, then use the default Vaa3D module
            b_grabhighrez = false;
            produceZoomViewOf3DRoi(loc_vec,2);
            return 2;
        }
        return 1;
    }
    else //this following session of seems OK for a local 3D viewer,
        //but somehow cause a crashing bug in the global 3D viewer. why??
        //by PHC, 2013-06-11.
    {
        b_grabhighrez = false;
        produceZoomViewOf3DRoi(loc_vec);
        return 2;
    }
    //now invoke the code!!


    return 0;
}


int Renderer_gl1::zoomview_currentviewport()//by PHC, revised from zoomview_wheel_event, 20130701
{
    //qDebug("wheel pos x=%5.3f,y=%5.3f, view port[%d, %d, %d, %d]",wheelPos.x,wheelPos.y, viewport[0], viewport[1], viewport[2], viewport[3]);
//    if (wheelPos.x<viewport[0]+5 || wheelPos.x>=viewport[2]-5 ||
//        wheelPos.y<viewport[1]+5 || wheelPos.y>=viewport[3]-5 ) //add a margin 5 to force have a hitWhell event in case wheelPos has not been initialized
//    {
        //v3d_msg("reset wheel pos in zoomview_wheel_event()");
        hitWheel((viewport[0]+viewport[2])/2.0, (viewport[1]+viewport[3])/2.0);
//    }

    //find the center line XYZ loc, and then 4 corner's XYZ locs, then based on all five locs define the bounding box and return
    QList <MarkerPos> curlist;
    //curlist.append(wheelPos);
    qDebug("wheel pos x=%5.3f,y=%5.3f",wheelPos.x,wheelPos.y);

    int i;
    for (i=0; i<5; i++)
    {
        MarkerPos p;
        p = wheelPos;
        switch (i)
        {
            case 0: p.x = (wheelPos.view[0]+wheelPos.view[2])/2.0; p.y = (wheelPos.view[1]+wheelPos.view[3])/2.0; break;
            case 1: p.x = wheelPos.view[0]+1; p.y = wheelPos.view[1]+1; break;
            case 2: p.x = wheelPos.view[0]+1; p.y = wheelPos.view[3]-1; break;
            case 3: p.x = wheelPos.view[2]-1; p.y = wheelPos.view[1]+1; break;
            case 4: p.x = wheelPos.view[2]-1; p.y = wheelPos.view[3]-1; break;
            default:break;
        }
        curlist.append(p);
        qDebug("i=%d, x=%5.3f,y=%5.3f",i, p.x,p.y);
    }

    vector <XYZ> loc_vec;
    for (i=0;i<curlist.size();i++)
    {
        //XYZ loc_m = getCenterOfMarkerPos(curlist.at(i));

        XYZ loc0, loc1;
        getVolumeXsectPosOfMarkerLine(loc0, loc1, curlist.at(i), 0);

        //XYZ loc = (loc0+loc1)*0.5;

        loc_vec.push_back(loc0);
        loc_vec.push_back(loc1);

        qDebug("i=%d loc0(%5.3f %5.3f %5.3f) loc1(%5.3f,%5.3f,%5.3f) \n",
               i, loc0.x, loc0.y, loc0.z, loc1.x, loc1.y, loc1.z
               );

        //qDebug("%5.3f, %5.3f, %5.3f,1,1,,,\n%5.3f,%5.3f,%5.3f,1,1,,, \n", loc0.x, loc0.y, loc0.z, loc1.x, loc1.y, loc1.z);

    }

    produceZoomViewOf3DRoi(loc_vec);
    return 0;
}

LandmarkList * Renderer_gl1::getHandleLandmark() //by Hanbo Chen, 20141018
{
    My4DImage* image4d = v3dr_getImage4d(_idep);
    if (image4d){
        return &(image4d->listLandmarks);
    }else{
        return 0;
    }
}

void Renderer_gl1::setHandleLandmark(LandmarkList & landmark_list)
{
    My4DImage* image4d = v3dr_getImage4d(_idep);
    if (image4d){
        image4d->listLandmarks = landmark_list;
    }else{
        return;
    }
}

QString XYZtoQString(XYZ pos){
    return QString::number(pos.x) + " " + QString::number(pos.y) + " " + QString::number(pos.z);
}
//
////Need a list to store this <- which does it's own initialization upon insert and delete
////The following classes are data structures used to calculate level information in the neuron tree
//class DoublyLinkedNeuronNode{
//public:
//    DoublyLinkedNeuronNode* upstream;
//    DoublyLinkedNeuronNode* downstream;
//    XYZ position;
//    V3DLONG seg_id;
//    DoublyLinkedNeuronNode(V3DLONG segId, XYZ pos):
//        seg_id(segId),
//        downstream(NULL),
//        upstream(NULL),
//        position(pos)
//        {}
//    bool isHead(){return (upstream == NULL);}
//    bool isTail(){return (downstream == NULL);}
//};
//
//class DoublyLinkedNeuronsList{
//public:
//    int length;
//    DoublyLinkedNeuronNode* head;
//    DoublyLinkedNeuronNode* tail;
//    DoublyLinkedNeuronsList(): head(NULL), tail(NULL), length(0){}
//    void append(V3DLONG segId, XYZ pos){
//        length++;
//        DoublyLinkedNeuronNode* toAdd = new DoublyLinkedNeuronNode(segId, pos);
//        if(head == NULL){
//            head = toAdd;
//            tail = toAdd;
//            toAdd->downstream = NULL;
//            toAdd->upstream = NULL;
//        }else{
//            tail->downstream = toAdd;
//            toAdd->upstream = tail;
//            toAdd->downstream = NULL;
//            tail = toAdd;
//        }
//    }
//    ~DoublyLinkedNeuronsList(){
//        //cout << "calling deleting" << endl;
//        DoublyLinkedNeuronNode* current = head;
//        while( current != NULL ) {
//            //cout << "deleting" << endl;
//            DoublyLinkedNeuronNode* next = current->downstream;
//            delete current;
//            current = next;
//        }
//    }
//};

//This list stores all nodes in the same temporal location. The list should only be visited once, otherwise a loop exists.
class SamePointList{
public:
    SamePointList(): visited(false), firstVisitSegId(-1){}
    void insert(DoublyLinkedNeuronNode* dln){
        list.push_back(dln);
    }
    int length(){
        return list.length();
    }
    bool hasVisited(){
        return visited;
    }
    QList<DoublyLinkedNeuronNode*> markAsVisitedAndGetConnections(V3DLONG id){
        visited = true;
        firstVisitSegId = id;
        return list;
    }
    int getFirstVisitSegId(){
        return firstVisitSegId;
    }
private:
    QList<DoublyLinkedNeuronNode*> list;
    bool visited;
    int firstVisitSegId;
};

class PointCloudHash{
public:
    QHash<QString, SamePointList*> hash;
    PointCloudHash(){}
    void Hash(DoublyLinkedNeuronNode* dln){
        QHash<QString, SamePointList*>::iterator i = hash.find(XYZtoQString(dln->position));
        if(i == hash.end()){
            SamePointList* spl = new SamePointList;
            hash.insert(XYZtoQString(dln->position), spl);
            spl->insert(dln);
        }else{
            i.value()->insert(dln);
        }
    }
    ~PointCloudHash(){
        QHash<QString, SamePointList*>::const_iterator i = hash.constBegin();
        while (i != hash.constEnd()) {
            delete i.value();
            ++i;
        }
    }
};

class FringeNode{
public:
    DoublyLinkedNeuronNode* node;
    bool isGoingUpstream;
    V3DLONG level;
    V3DLONG parent;
    FringeNode(DoublyLinkedNeuronNode*n, bool up, V3DLONG lvl, V3DLONG p): node(n), isGoingUpstream(up), level(lvl), parent(p){}
};




void Renderer_gl1::addToListOfLoopingSegs(V3DLONG firstVisitSegId, V3DLONG secondVisitSegId, V3DLONG violatingSegId){

    QList<V3DLONG> segsInFirstVisitNode;
    QList<V3DLONG> segsInRepeatVisitNode;

    /*
    V3DLONG firstVisitSegId = (pch.hash.value(XYZtoQString(tvs.node->position)))->getFirstVisitSegId();
    V3DLONG secondVisitSegId = tvs.node->seg_id;
    */

    /*
    if(segmentLevelDict[firstVisitSegId] == 1){
        do {
            cout << "Pushed second " << firstVisitSegId << endl;
            segsInFirstVisitNode.push_back(firstVisitSegId);
            firstVisitSegId = segmentParentDict[firstVisitSegId];
        } while (segmentParentDict[firstVisitSegId] != firstVisitSegId);

    }else{
        cout << "error: coincident point not at root " << endl;
    }
    */

    QHash<V3DLONG, bool> loopVisitDict;
    // loopVisitDict is needed to avoid a case where segmentParentDict points from
    // seg A -> seg B -> seg A, etc
    loopVisitDict.clear();
    segsInFirstVisitNode.push_back(violatingSegId); // The violating segment will always be highlighted
    do {
       // qDebug() << "Pushed first " << firstVisitSegId;
        segsInFirstVisitNode.push_back(firstVisitSegId);
        loopVisitDict.insert(firstVisitSegId, true);
        firstVisitSegId = segmentParentDict[firstVisitSegId];
      //  qDebug() << "New firstVisitSegId: " << firstVisitSegId;
    } while (segmentParentDict[firstVisitSegId] != firstVisitSegId &&
             !loopVisitDict.contains(firstVisitSegId));

    QList<V3DLONG>::iterator it;
    loopVisitDict.clear();
    do {
        //cout << "Pushed second " << secondVisitSegId << endl;
        segsInRepeatVisitNode.push_back(secondVisitSegId);
        loopVisitDict.insert(secondVisitSegId, true);
        secondVisitSegId = segmentParentDict[secondVisitSegId];
        it = std::find(segsInFirstVisitNode.begin(), segsInFirstVisitNode.end(), secondVisitSegId);
        //Update this second list until we coincide with the first list
    } while (segmentParentDict[secondVisitSegId] != secondVisitSegId && it == segsInFirstVisitNode.end() &&
             !loopVisitDict.contains(secondVisitSegId));

    segsInRepeatVisitNode.push_back(secondVisitSegId);

    segsInFirstVisitNode.erase(it, segsInFirstVisitNode.end());
    for(it = segsInFirstVisitNode.begin(); it != segsInFirstVisitNode.end(); it++){
        loopSegs.push_back(*it);
    }
    for(it = segsInRepeatVisitNode.begin(); it != segsInRepeatVisitNode.end(); it++){
        loopSegs.push_back(*it);
    }
        /*
    }else{
        cout << "error: loop in second segment not found in first segment " << endl;
    }*/

}



void Renderer_gl1::addToListOfChildSegs(V3DLONG segID){
    childSegs.clear();


    // use the segment parent hash calculated in setColorAncestryInfo...

    if (segmentParentDict.isEmpty())
        setColorAncestryInfo();

    QList<V3DLONG> lookingList;
    QList<V3DLONG> nextList;
    lookingList.clear();
    lookingList.append(segID);


// I'm just going to try searching the segmentParentDict every time... we'll see how slow it is.
    QTime startTime = QTime::currentTime();

    while (!lookingList.isEmpty()){
        nextList.clear();
        for (V3DLONG i = 0; i<lookingList.length() ; i++){
            childSegs.append(lookingList.at(i));


            QHash<V3DLONG, V3DLONG>::const_iterator j = segmentParentDict.constBegin();
            while (j != segmentParentDict.constEnd()) {
                if (j.value()==lookingList.at(i)) nextList.append(j.key());
                ++j;
            }

        }
        lookingList=nextList;

    }
    QTime endTime = QTime::currentTime();

    qDebug()<<"elapsed time to find descendents :"<< startTime.msecsTo(endTime); // this is fast enough.
}


//Assigns each segment neuron list a level (corresponding to the segment'slevel in the neuron tree), given that a soma is present
//The coloring effects of this function will only be availible when the ColorByLevel bool is toggled to true
//Returns false if a loop esists, and sets color of all segments to purple. Otherwise, sets the segmentParentsList and segmentLevelList correctly

//Level 0 Is reserved for the soma
//-1 Indicats a segment removed from the main tree
//-2 Indicates the prescence of a loop
bool Renderer_gl1::setColorAncestryInfo(){

    segmentParentDict.clear();
    segmentLevelDict.clear();
    segmentLengthDict.clear();
    loopSegs.clear();

    QList<FringeNode> f; //A list of nodes to traverse next, in a depth-first search fashion
    QList<NeuronSWC> somas; //List of soma nodes
    PointCloudHash pch; //Hashes the location of the nodes to a list containing all nodes occupying the same point
    //QHash<V3DLONG, DoublyLinkedNeuronsList*> dict_dlnh; //A list of segments, hases seg_id  to doubly-linked segments
    dict_dlnh.clear();



    QList<NeuronTree>::iterator it;
    if(listNeuronTree.length() > 0){
        //There seems to sometimes be two trees storing the exact same contents?
        it = listNeuronTree.end();
        --it;

        QList <NeuronSWC> p_listneuron = it->listNeuron;

        for (int i=0; i<p_listneuron.size(); i++)
        {
            NeuronSWC node = p_listneuron.at(i);

            if(node.type == 1){ //This is a soma
                somas.push_back(node);
            }

            QHash<V3DLONG, DoublyLinkedNeuronsList*>::iterator j = dict_dlnh.find(node.seg_id);
            if(j == dict_dlnh.end()){
                //cout << "created new segment list " << endl;
                DoublyLinkedNeuronsList* dln = new DoublyLinkedNeuronsList();
                dict_dlnh.insert(node.seg_id, dln);
                dln->append(node.seg_id, XYZ(node));
                pch.Hash(dln->tail);
                segmentLengthDict.insert(node.seg_id, 1);
            }else{
                //cout << "added to old segment list " << endl;
                j.value()->append(node.seg_id, XYZ(node));
                pch.Hash(j.value()->tail);
                segmentLengthDict.insert(node.seg_id, j.value()->length);
            }

            segmentParentDict.insert(node.seg_id, -1);
            segmentLevelDict.insert(node.seg_id, -1);
        }
    }
        //Initialize all values to -1, this is also the expected return value of the case without soma
    //}

    //If fringe not empty, then we insert all the points connected to the soma into
    //the fringe and do depth-first traversal around the entire neuron tree, in addition marking the soma as visited
    if(somas.length() != 0){

        segmentParentDict.insert(somas.at(0).seg_id, somas.at(0).seg_id); //If a segment's parent is itself, then its the soma
        segmentLevelDict.insert(somas.at(0).seg_id, 0); //Soma has level 0

        QList<NeuronSWC>::iterator soma_iter;

        for(soma_iter = somas.begin(); soma_iter != somas.end(); soma_iter++){
            QList<DoublyLinkedNeuronNode*> l = ((pch.hash.value(XYZtoQString(XYZ(*soma_iter))))->markAsVisitedAndGetConnections( soma_iter->seg_id ));
            //Note that this particular spacial location has already been visited

            QList<DoublyLinkedNeuronNode*>::iterator it;
            for (it = (l.begin()); it != (l.end()); ++it){
                //cout << "Exist element in samePointList x:" << (*it)->position.x << endl;

                if((*it)->seg_id != (*soma_iter).seg_id){
                    //cout << "Exist non-root element in samePointList" << endl;
                    bool isHead = (*it)->upstream == NULL;
                    bool isTail = (*it)->downstream == NULL;

                    //Propigate from all segments list either upstream or downstream from that location
                    if(!isHead)
                        f.push_back( FringeNode((*it)->upstream, true, 1, (*soma_iter).seg_id) );
                    if(!isTail)
                        f.push_back( FringeNode((*it)->downstream, false, 1, (*soma_iter).seg_id) );

                    segmentParentDict.insert((*it)->seg_id, (*soma_iter).seg_id);
                    segmentLevelDict.insert((*it)->seg_id, 1);
                }
            }
        }
    }

    while(!f.isEmpty()){

        //QHash<QString, SamePointList*>::iterator s;

        //qDebug()<<"size: "<<f.size();

        //cout << "Processing fringe" << endl;
        FringeNode tvs = f.takeFirst();

        if(&tvs == NULL)
        {
            qDebug()<<"found a null fringenode assigned";
            continue;
        }

        if(tvs.node == NULL)
        {
            qDebug()<<" ... ... found a null node assigned";
            continue;
        }

        //cout << "Taking node from seg: " << tvs.node->seg_id << " with x: " << tvs.node->position.x << " y: "
        //     << tvs.node->position.y << " z: " << tvs.node->position.z << endl;
        QList<DoublyLinkedNeuronNode*> l;

        //qDebug()<<XYZtoQString(tvs.node->position)<<pch.hash.size();

        if(!(pch.hash.value(XYZtoQString(tvs.node->position))->hasVisited())){
            l = ((pch.hash.value(XYZtoQString(tvs.node->position)))->markAsVisitedAndGetConnections(tvs.node->seg_id));
        }else{
            //Each spacial location should only be visited once, otherwise there is a loop
            //This means a loop exists, so we choose not to expand

            continue; //abandon attempts to continue this loop
        }

        //
        // Here is the truth table that decides the branching policy of each scenario
        // etb = is this the end of the current segment?
        // eob = is this the end of the other segment? (only applicatble when there's exactly 1 other segment)
        // otr = number of other segments that uses this point
        //
        // etb|eob| otr | policy
        //  T |N/A|  0 | do nothing
        //  T | T |  1 | connect this branch to other branch (treat it as the same branch and give same parent #)
        //  T | F |  1 | consider this point a branching point
        //  T |N/A| 1+ | consider this point a branching point
        //  F |N/A|  0 | continue along this branch
        //  F |TvF| 0+ | Branch out for all other branches and continue along this branch
        //

        QList<DoublyLinkedNeuronNode*>::iterator it;
        bool loopExists = false;
        bool etb = (tvs.node->downstream == NULL || tvs.node->upstream == NULL);
        V3DLONG otr = l.length() - 1;
        if(etb){
            for (it = (l.begin()); it != (l.end()); ++it){
                if((*it)->seg_id != tvs.node->seg_id){
                    bool isHead = (*it)->isHead();
                    bool isTail = (*it)->isTail();
                    bool eob = isHead || isTail;
                    if(segmentParentDict[(*it)->seg_id] == -1){
                        if(otr == 1 && eob){
                                //connect this branch to other branch (treat it as the same branch and give same parent #)
                                if(!isHead){
                                    f.push_back( FringeNode((*it)->upstream, true, tvs.level, tvs.node->seg_id) );
                                    //cout << "route 1" << endl;
                                }
                                else if(!isTail){
                                    f.push_back( FringeNode((*it)->downstream, false, tvs.level, tvs.node->seg_id) );
                                    //cout << "route 2" << endl;
                                }
                                //cout << "Added pair p: " << tvs.parent << " c: " << (*it)->seg_id;
                                segmentParentDict.insert((*it)->seg_id, tvs.node->seg_id);
                                segmentLevelDict.insert((*it)->seg_id, tvs.level);
                        }else{
                            //consider this point a branching point
                            if(!isHead){
                                f.push_back( FringeNode((*it)->upstream, true, tvs.level + 1, tvs.node->seg_id) );
                                //cout << "route 3" << endl;
                            }
                            if(!isTail){
                                f.push_back( FringeNode((*it)->downstream, false, tvs.level + 1, tvs.node->seg_id) );
                                //cout << "route 4" << endl;
                            }
                            //cout << "Added pair p: " << tvs.node->seg_id << " c: " << (*it)->seg_id << endl;
                            segmentParentDict.insert((*it)->seg_id, tvs.node->seg_id);
                            segmentLevelDict.insert((*it)->seg_id, tvs.level + 1);
                       }
                    }else{
                          //Someone else has visited this segment already! Do not expand.
                          addToListOfLoopingSegs(segmentParentDict[(*it)->seg_id], tvs.node->seg_id, (*it)->seg_id);
                    }
                }
            }
        }else{
            for (it = (l.begin()); it != (l.end()); ++it){
                bool isHead = (*it)->isHead();
                bool isTail = (*it)->isTail();
                    if((*it)->seg_id == tvs.node->seg_id){
                        //continue along this seg
                        if(tvs.isGoingUpstream){
                            f.push_back( FringeNode((*it)->upstream, true, tvs.level, tvs.parent) );
                            //cout << "route 5" << endl;
                        }
                        else{
                            f.push_back( FringeNode((*it)->downstream, false, tvs.level, tvs.parent) );
                            //cout << "route 6" << endl;
                        }
                    }else{
                        if(segmentParentDict[(*it)->seg_id] == -1){
                        //consider this point a branching point
                            if(!isHead){
                                f.push_back( FringeNode((*it)->upstream, true, tvs.level + 1, tvs.node->seg_id) );
                                //cout << "route 7" << endl;
                            }
                            if(!isTail){
                                f.push_back( FringeNode((*it)->downstream, false, tvs.level + 1, tvs.node->seg_id) );
                                //cout << "route 8" << endl;
                            }
                            //cout << "Added pair p: " << tvs.node->seg_id << " c: " << (*it)->seg_id << endl;
                            segmentParentDict.insert((*it)->seg_id, tvs.node->seg_id);
                            segmentLevelDict.insert((*it)->seg_id, tvs.level + 1);
                        }else{
                              //Someone else has visited this segment already! Do not expand.
                              addToListOfLoopingSegs(segmentParentDict[(*it)->seg_id], tvs.node->seg_id, (*it)->seg_id);
                        }
                    }
            }
        }


    }

    QList<V3DLONG>::iterator iter;
    for (iter = (loopSegs.begin()); iter != (loopSegs.end()); ++iter){
        //cout << "digging from loopsegs and get " << *iter << endl;
        segmentParentDict.insert(*iter, -2);
        segmentLevelDict.insert(*iter, -2);
    }


    //For debugging

    /*
    QHash<V3DLONG, V3DLONG>::iterator i;
    for (i = segmentParentDict.begin(); i != segmentParentDict.end(); ++i){
        cout << "parent key" << i.key() << endl;
        cout << "parent value" << i.value() << endl;
    }

    QHash<V3DLONG, V3DLONG>::iterator i;
    for (i = segmentLevelDict.begin(); i != segmentLevelDict.end(); ++i){
        cout << "level key " << i.key() << endl;
        cout << "level value " << i.value() << endl;
    }
        QHash<V3DLONG, DoublyLinkedNeuronsList*>::iterator dit;
    for (dit = (dict_dlnh.begin()); dit != (dict_dlnh.end()); ++dit){
        delete (dit.value());
    }
    */
    //We're done.
    return true;
}

void Renderer_gl1::updateMarkerList(QList <ImageMarker> markers, int i)
{
    listMarker[i] = markers[i];

    My4DImage* image4d = v3dr_getImage4d(_idep);
    if (image4d)
    {
        LocationSimple *s = (LocationSimple *)(&(image4d->listLandmarks[i]));
        s->color = listMarker[i].color;
    }
}
