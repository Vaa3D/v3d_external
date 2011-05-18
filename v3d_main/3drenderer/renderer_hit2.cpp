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
 * renderer_hit2.cpp
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

#include "renderer_tex2.h"
#include "v3dr_glwidget.h"
#include "barFigureDialog.h"


#ifndef test_main_cpp

#include "../v3d/v3d_compile_constraints.h"

#include "../v3d/landmark_property_dialog.h"
#include "../v3d/surfaceobj_annotation_dialog.h"
#include "../v3d/surfaceobj_geometry_dialog.h"

#include "../neuron_editing/neuron_sim_scores.h"
#include "../neuron_tracing/neuron_tracing.h"

#include "../imaging/v3d_imaging.h"

#endif

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
//#define LIST_COLOR_A(list, i, w) \
//{ \
//	if (i>=0 && i<list.size()) \
//	{ \
//		QColor qc = QColor::fromRgba( QColorDialog::getRgba(QColorFromRGBA8(list[i].color).rgba(), 0, w) ); \
//		list[i].color = RGBA8FromQColor( qc ); \
//	} \
//}


int Renderer_tex2::processHit(int namelen, int names[], int cx, int cy, bool b_menu, char* pTip) // called by hitObj after getting object's names
{
	//qDebug("  Renderer_tex2::processHit  pTip=%p", pTip);

#define __object_name_info__ // dummy, just for easy locating
	// object name string
	QString qsName;
	QString qsInfo;

#define IS_VOLUME() 	(namelen>=3 && names[0]==dcVolume)
#define IS_SURFACE() 	(namelen>=3 && names[0]==dcSurface)
	//qDebug("	namelen=%d, names[0]=%d", namelen, names[0]);

	lastSliceType = vsSliceNone;
	if (IS_VOLUME()) // volume
	{
			QString bound_info = QString("(%1 %2 %3 -- %4 %5 %6)").arg(start1+1).arg(start2+1).arg(start3+1).arg(start1+size1).arg(start2+size2).arg(start3+size3);
			QString data_title = "";	//if (w) data_title = QFileInfo(w->getDataTitle()).fileName();
			(qsName = QString("volume %1 ... ").arg(bound_info) +(data_title));
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
			LIST_SELECTED(listMarker, names[2]-1, true);

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
			qsInfo = info_NeuronNode(findNearestNeuronNode_WinXY(cx, cy, p_tree), p_tree);
		}break;

		case stPointCloud: {//apo
			(qsName = QString("point cloud #%1 ... ").arg(names[2]) + listCell.at(names[2]-1).name);
			LIST_SELECTED(listCell, names[2]-1, true);
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
#endif

	// right click popup menu
	QList<QAction*> listAct;
	QAction *act=0,
			*actProperty=0, *actVolColormap=0, *actObjectManager=0, *actOff=0, *actColor=0, *actEditNameComment=0,
			*actSaveSurfaceObj=0,
			*actLockSceneEditObjGeometry=0,

			*actMarkerCreate1=0, *actMarkerCreate2=0, *actMarkerCreate3=0, *actMarkerRefineT=0, *actMarkerRefineC=0, *actMarkerRefineLocal=0, *actMarkerAutoSeed=0,
			*actMarkerZoomin3D=0, *actMarkerMoveToMiddleZ=0, 
			*actMarkerDelete=0, *actMarkerClearAll=0, *actMarkerCreateNearestNeuronNode=0,
			*actMarkerTraceFromStartPos=0, *actMarkerTraceFromStartPos_selPara=0, *actMarkerLineProfileFromStartPos=0, *actMarkerLineProfileFromStartPos_drawline=0, *actMarkerLabelAsStartPos=0,
			*actMarkerTraceFromOnePos=0, *actMarkerTraceFromOnePosToOtherMarkers=0,

			*actCurveCreate1=0, *actCurveCreate2=0, *actCurveCreate3=0, *actCurveCreate_pointclick=0,
			*actCurveCreate_zoom=0, *actMarkerCreate_zoom=0,
	
			*actCurveCreate_zoom_imaging=0, *actMarkerCreate_zoom_imaging=0,
	        *actMarkerAblateOne_imaging=0, *actMarkerAblateAll_imaging=0, 
			//need to add more surgical operations here later, such as curve_ablating (without displaying the curve first), etc. by PHC, 20101105

			*actNeuronToEditable=0, *actDecomposeNeuron=0, *actNeuronFinishEditing=0,
			*actChangeNeuronSegType=0, *actChangeNeuronSegRadius=0, *actReverseNeuronSeg=0,
			*actDispRecNeuronSegInfo=0, *actDeleteNeuronSeg=0, *actBreakNeuronSegNearestNeuronNode=0, *actBreakNeuronSeg_markclick=0,
			*actJoinNeuronSegs_nearby_markclick=0, *actJoinNeuronSegs_nearby_pathclick=0, *actJoinNeuronSegs_all=0,
			*actNeuronSegDeform=0, *actNeuronSegProfile=0,
			*actNeuronOneSegMergeToCloseby=0, *actNeuronAllSegMergeToCloseby=0,

			*actDispNeuronNodeInfo=0,	*actAveDistTwoNeurons=0, *actDispNeuronMorphoInfo=0,

			*actDispSurfVertexInfo=0,
			*actComputeSurfArea=0, *actComputeSurfVolume=0;

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
#ifdef _ALLOW_ADVANCE_PROCESSING_MENU_
			listAct.append(act = new QAction("", w)); act->setSeparator(true);

			listAct.append(actMarkerCreate1 = new QAction("1-right-click to define a marker (Esc to finish)", w));
			
			actMarkerCreate1->setIcon(QIcon(":/icons/click1.svg"));
			actMarkerCreate1->setVisible(true);
			actMarkerCreate1->setIconVisibleInMenu(true);
			
			listAct.append(actMarkerCreate2 = new QAction("2-right-clicks to define a marker", w));
			
			actMarkerCreate2->setIcon(QIcon(":/icons/click2.svg"));
			actMarkerCreate2->setVisible(true);
			actMarkerCreate2->setIconVisibleInMenu(true);
			
			listAct.append(actMarkerCreate3 = new QAction("3-right-clicks to define a marker", w));
			
			actMarkerCreate3->setIcon(QIcon(":/icons/click3.svg"));
			actMarkerCreate3->setVisible(true);
			actMarkerCreate3->setIconVisibleInMenu(true);
			
			//listAct.append(act = new QAction("", w)); act->setSeparator(true);
#ifdef _ALLOW_AUTOMARKER_MENU_
			listAct.append(actMarkerAutoSeed = new QAction("AutoMarker", w));
#endif

#ifdef _ALLOW_3D_CURVE_
			listAct.append(act = new QAction("", w)); act->setSeparator(true);
			listAct.append(actCurveCreate1 = new QAction("1-right-stroke to define a 3D curve", w));
			
			actCurveCreate1->setIcon(QIcon(":/icons/stroke1.svg"));
			actCurveCreate1->setVisible(true);
			actCurveCreate1->setIconVisibleInMenu(true);
			
			listAct.append(actCurveCreate2 = new QAction("2-right-strokes to define a 3D curve", w));
			
			actCurveCreate2->setIcon(QIcon(":/icons/stroke2.svg"));
			actCurveCreate2->setVisible(true);
			actCurveCreate2->setIconVisibleInMenu(true);
			
			listAct.append(actCurveCreate3 = new QAction("3-right-strokes to define a 3D curve", w));
			
			actCurveCreate3->setIcon(QIcon(":/icons/stroke3.svg"));
			actCurveCreate3->setVisible(true);
			actCurveCreate3->setIconVisibleInMenu(true);
			
			listAct.append(actCurveCreate_pointclick = new QAction("Series of right-clicks to define a 3D polyline (Esc to finish)", w));
			
			actCurveCreate_pointclick->setIcon(QIcon(":/icons/strokeN.svg"));
			actCurveCreate_pointclick->setVisible(true);
			actCurveCreate_pointclick->setIconVisibleInMenu(true);

			//if (!(((iDrawExternalParameter*)_idep)->b_local)) //only enable the menu for global 3d viewer. as it seems there is a bug in the local 3d viewer. by PHC, 100821
			{
				listAct.append(act = new QAction("", w)); act->setSeparator(true);
				listAct.append(actCurveCreate_zoom = new QAction("Zoom-in view: 1-right-stroke ROI", w));
				listAct.append(actMarkerCreate_zoom = new QAction("Zoom-in view: 1-right-click ROI", w));
//101008
//#ifdef _WIN32 && _MSC_VER
#ifdef _IMAGING_MENU_
				listAct.append(act = new QAction("", w)); act->setSeparator(true);
				listAct.append(actCurveCreate_zoom_imaging = new QAction("Zoom-in imaging: 1-right-stroke ROI", w));
				listAct.append(actMarkerCreate_zoom_imaging = new QAction("Zoom-in imaging: 1-right-click ROI", w));
#endif
//#endif
			}
#endif
#endif
		}

		//##############################################################################
		// surface data
		//##############################################################################
		if (IS_SURFACE())
		{
			listAct.append(act = new QAction("", w)); act->setSeparator(true);
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
				listAct.append(actSaveSurfaceObj = new QAction("save all point-cloud objects to file", w));
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
#ifdef _ALLOW_ADVANCE_PROCESSING_MENU_
#ifdef _ALLOW_LOCAL_ZOOMIN_3D_VIEWER_
				listAct.append(act = new QAction("", w)); act->setSeparator(true);
				listAct.append(actMarkerZoomin3D = new QAction("open the local zoom-in 3d viewer", w));
#endif
				listAct.append(act = new QAction("", w)); act->setSeparator(true);
				//listAct.append(actMarkerRefineLocal = new QAction("refine marker to local center", w));
				listAct.append(actMarkerRefineC = new QAction("re-define marker on intense position by 1 right-click", w));
				listAct.append(actMarkerRefineT = new QAction("translate marker position by 1 right-click", w));
				listAct.append(actMarkerDelete = new QAction("delete this marker", w));
				listAct.append(actMarkerClearAll = new QAction("clear All markers", w));
				listAct.append(actMarkerMoveToMiddleZ = new QAction("change all markers' Z locations to mid-Z-slice", w));

#ifdef _IMAGING_MENU_
				listAct.append(act = new QAction("", w)); act->setSeparator(true);
				listAct.append(actMarkerAblateOne_imaging = new QAction("ablate this marker", w));
				listAct.append(actMarkerAblateAll_imaging = new QAction("ablate All markers", w));
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
					
					actMarkerTraceFromOnePosToOtherMarkers->setIcon(QIcon(":/icons/trace.svg"));
					actMarkerTraceFromOnePosToOtherMarkers->setVisible(true);
					actMarkerTraceFromOnePosToOtherMarkers->setIconVisibleInMenu(true);
				}
#endif

#ifdef _ALLOW_NEURONTREE_ONE2ENTIREIMG_MENU_
				listAct.append(actMarkerTraceFromOnePos = new QAction("trace from 1 start pos through entire image", w));
				
				actMarkerTraceFromOnePos->setIcon(QIcon(":/icons/trace.svg"));
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
					
					actMarkerLabelAsStartPos->setIcon(QIcon(":/icons/start.svg"));
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
#endif
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
						listAct.append(actDispRecNeuronSegInfo = new QAction("display nearest neuron-segment info", w));
						listAct.append(actChangeNeuronSegType = new QAction("change nearest neuron-segment type", w));
						listAct.append(actChangeNeuronSegRadius = new QAction("change nearest neuron-segment radius", w));
						listAct.append(actReverseNeuronSeg = new QAction("reverse nearest neuron-segment link order", w));
						listAct.append(actDeleteNeuronSeg = new QAction("delete the nearest neuron-segment", w));
						//listAct.append(actNeuronOneSegMergeToCloseby = new QAction("merge a terminal-segment to nearby segments", w));
						listAct.append(actNeuronAllSegMergeToCloseby = new QAction("merge nearby segments", w));
						if (curImg->tracedNeuron.isJointed()==false)
						{
							listAct.append(actBreakNeuronSegNearestNeuronNode = new QAction("break the segment using nearest neuron-node", w));
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

		if (w) w->updateGL(); //for highlight object
		//###############################################################
		// do menu
		//###############################################################
		QMenu menu;
		foreach (QAction* a, listAct) {  menu.addAction(a); }
		//menu.setWindowOpacity(POPMENU_OPACITY); // no effect on MAC? on Windows cause blink
		act = menu.exec(QCursor::pos());

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
			break;
		}
	}
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
				saveLandmarks_to_file(""); //use "" an empty string to force open a file dialog
				break;
		}
	}

#ifndef test_main_cpp
	else if (act == actLockSceneEditObjGeometry)
	{
		finishEditingNeuronTree(); //090928
		editSurfaceObjBasicGeometry(names[0], names[1], names[2]);

	}
#endif

#define __create_curve__ // dummy, just for easy locating

	// real operation in selectObj() waiting next click
	else if (act == actCurveCreate1)
	{
		selectMode = smCurveCreate1;
		b_addthiscurve = true;
		if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
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
		if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::CrossCursor)); }
	}
	else if (act == actCurveCreate_zoom)
	{
		selectMode = smCurveCreate1;
		b_addthiscurve = false;
		if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
	}

#define __create_marker__ // dummy, just for easy locating

	// real operation in selectObj() waiting next click
	else if (act == actMarkerCreate1)
	{
		selectMode = smMarkerCreate1;
		b_addthismarker = true;
		if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::CrossCursor)); }
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

#define __v3d_imaging_func__ // dummy, just for easy locating
	else if (act == actCurveCreate_zoom_imaging)
	{
		selectMode = smCurveCreate1;
		b_addthiscurve = false;
		b_imaging = true;
		if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
	}
	else if (act == actMarkerCreate_zoom_imaging)
	{
		selectMode = smMarkerCreate1;
		b_addthismarker = false;
		b_imaging = true;
		if (w) { oldCursor = w->cursor(); w->setCursor(QCursor(Qt::PointingHandCursor)); }
	}
	else if (act == actMarkerAblateOne_imaging || act == actMarkerAblateAll_imaging)
	{
		if (w && curImg && curXWidget)
		{
			v3d_imaging_paras myimagingp;
			myimagingp.OPS = "POINT_ABLATING";
			myimagingp.imgp = (Image4DSimple *)curImg; //the image data for a plugin to call
			
			bool doit = (curImg->listLandmarks.size()>0) ? true : false;
			
			if (doit && act == actMarkerAblateAll_imaging)
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
					v3d_msg("Fail to set up the curHiddenSelectedXWidget for the V3D mainwindow. Do nothing.");
				}

			}
		}
	}
	
#ifndef test_main_cpp

	else if (act == actMarkerAutoSeed)
	{
		if (w && curImg)
		{
			bool ok1=true;
			V3DLONG chno=1;
			if (curImg->getCDim()>1)
				chno = QInputDialog::getInteger(0, QString("select a channel"), QString("select a channel of image you'd apply AutoMarker to:"), 1, 1, int(curImg->getCDim()), 1, &ok1);

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

#define __actions_of_marker__ // dummy, just for easy locating

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
		NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
		if (p_tree)
		{
			V3DLONG n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);
			qDebug("detect nearest neuron node [%d] for the [%d] neuron", n_id, names[2]-1);

			NeuronSWC cur_node;
			if (n_id>=0)
			{
				cur_node = p_tree->listNeuron.at(n_id);
				qDebug()<<cur_node.x<<" "<<cur_node.y<<" "<<cur_node.z;

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
#endif //test_main_cpp



#define __actions_of_neuron__ // dummy, just for easy locating
#ifndef test_main_cpp

#define NEURON_CONDITION  (listNeuronTree.size()>=1 && w && curImg) //only allow one neuron, and assume it is the one being reconstructed from image

	else if (act==actNeuronToEditable || act==actDecomposeNeuron)
	{
		if (NEURON_CONDITION)
		{
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			curImg->tracedNeuron = copyToEditableNeuron(p_tree);
			curImg->proj_trace_history_append();
			curImg->update_3drenderer_neuron_view(w, this);
		}
	}
	else if (act==actNeuronFinishEditing)
	{
		if (NEURON_CONDITION)
		{
			finishEditingNeuronTree();
		}
	}

	else if (act==actDispRecNeuronSegInfo)
	{
		if (NEURON_CONDITION)
		{
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			V3DLONG n_id;
			if (p_tree)	{n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);}
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
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			V3DLONG n_id;
			if (p_tree)	{n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);}
			if (n_id>=0)
			{
				curImg->proj_trace_changeNeuronSegType(n_id, p_tree);
				curImg->update_3drenderer_neuron_view(w, this);
			}
		}
	}
	else if (act==actChangeNeuronSegRadius)
	{
		if (NEURON_CONDITION)
		{
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			V3DLONG n_id;
			if (p_tree)	{n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);}
			if (n_id>=0)
			{
				curImg->proj_trace_changeNeuronSegRadius(n_id, p_tree);
				curImg->update_3drenderer_neuron_view(w, this);
			}
		}
	}
	else if (act==actReverseNeuronSeg)
	{
		if (NEURON_CONDITION)
		{
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			V3DLONG n_id;
			if (p_tree)	{n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);}
			if (n_id>=0)
			{
				curImg->proj_trace_reverseNeuronSeg(n_id, p_tree);
				curImg->update_3drenderer_neuron_view(w, this);
			}
		}
	}
	else if (act==actDeleteNeuronSeg)
	{
		if (NEURON_CONDITION)
		{
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			V3DLONG n_id;
			if (p_tree)	{n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);}
			if (n_id>=0)
			{
				curImg->proj_trace_deleteNeuronSeg(n_id, p_tree);
				curImg->update_3drenderer_neuron_view(w, this);
			}
		}
	}
	else if (act==actBreakNeuronSegNearestNeuronNode)
	{
		if (NEURON_CONDITION)
		{
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			if (p_tree)
			{
				V3DLONG n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);
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
		{
			ImageMarker *c_pos = (ImageMarker *)(&listMarker.at(names[2]-1));
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(0)));
			V3DLONG n_id=-1;
			for (int ci=0;ci<p_tree->listNeuron.size();ci++)
			{
				if (p_tree->listNeuron.at(ci).x==c_pos->x &&
				    p_tree->listNeuron.at(ci).y==c_pos->y &&
					p_tree->listNeuron.at(ci).z==c_pos->z)
					n_id==ci;
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
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			V3DLONG n_id;
			if (p_tree)	{n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);}
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
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			V3DLONG n_id;
			if (p_tree)	{n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);}
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
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			V3DLONG n_id;
			if (p_tree)	{n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);}
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
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			V3DLONG n_id;
			if (p_tree)	{n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);}
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
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			V3DLONG n_id;
			if (p_tree)	{n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);}
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
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			V3DLONG n_id;
			if (p_tree)	{n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);}
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
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
			curImg->proj_trace_mergeAllClosebyNeuronNodes(p_tree);
			curImg->update_3drenderer_neuron_view(w, this);
		}
	}
	else if (act==actDispNeuronNodeInfo)
	{
		NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(names[2]-1)));
		V3DLONG n_id = findNearestNeuronNode_WinXY(cx, cy, p_tree);
		QString tmpstr, tmpstr1;
		tmpstr.setNum(n_id); tmpstr.prepend("The neuron node has row index ");
		tmpstr.append("\n");
		tmpstr.append(info_NeuronNode(n_id, p_tree));

//		tmpstr1.setNum(p_tree->listNeuron.at(n_id).n); tmpstr1.prepend("\nnode number (1st col in SWC) = "); tmpstr.append(tmpstr1);
//		tmpstr1.setNum(p_tree->listNeuron.at(n_id).type); tmpstr1.prepend("\ntype (2th col) = "); tmpstr.append(tmpstr1);
//		tmpstr1.setNum(p_tree->listNeuron.at(n_id).x); tmpstr1.prepend("\nx coord (3rd col) = "); tmpstr.append(tmpstr1);
//		tmpstr1.setNum(p_tree->listNeuron.at(n_id).y); tmpstr1.prepend("\ny coord (4th col) = "); tmpstr.append(tmpstr1);
//		tmpstr1.setNum(p_tree->listNeuron.at(n_id).z); tmpstr1.prepend("\nz coord (5th col) = "); tmpstr.append(tmpstr1);
//		tmpstr1.setNum(p_tree->listNeuron.at(n_id).r); tmpstr1.prepend("\nradius (6th col) = "); tmpstr.append(tmpstr1);
//		tmpstr1.setNum(p_tree->listNeuron.at(n_id).pn); tmpstr1.prepend("\nparent number (7th col) = "); tmpstr.append(tmpstr1);

		QMessageBox::information(0, "neuron node info", tmpstr);
	}
	else if (act==actAveDistTwoNeurons)
	{
		if (listNeuronTree.size()<2)
			QMessageBox::information(0, "only one neuron", "only one neuron, nothing is computed");

		QString tmpstr, ts2;
		float ave_sd=0, ave_ssd=0, ave_ssd_percent=0;
		for (int ci=0;ci<listNeuronTree.size();ci++)
		{

			if (ci!=(names[2]-1))
			{
				ts2.setNum(names[2]); ts2.prepend("dists between "); tmpstr += ts2;
				ts2.setNum(ci+1); ts2.prepend(" and "); tmpstr += ts2;
				NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(&(listNeuronTree.at(names[2]-1)), &(listNeuronTree.at(ci)));
				ts2.setNum(tmp_score.dist_allnodes); ts2.prepend(" are <br> entire-structure-average = "); tmpstr += ts2 + "<br>";
				ts2.setNum(tmp_score.dist_apartnodes); ts2.prepend(" different-structure-average = "); tmpstr += ts2 + "<br>";
				ts2.setNum(tmp_score.percent_apartnodes); ts2.prepend(" percent of different-structure = "); tmpstr += ts2 + "<br><br>";
				qDebug() << "score between "<<names[2]<< " and "<<ci+1<< "=" << tmp_score.dist_allnodes << " " << tmp_score.dist_apartnodes << " " << tmp_score.percent_apartnodes;

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
	else if (act==actDispNeuronMorphoInfo)
	{
		QString tmpstr = "Neuron ", ts2;
		ts2.setNum(names[2]); tmpstr += ts2 + "<br>" + get_neuron_morpho_features_str(&(listNeuronTree.at(names[2]-1)));
		QMessageBox::information(0, "neuron info", tmpstr);
		qDebug() << tmpstr;
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

void Renderer_tex2::endSelectMode()
{
	qDebug("  Renderer_tex2::endSelectMode");
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

	if (selectMode == smCurveCreate_pointclick)
	{
		if (cntCur3DCurveMarkers >=2)
		{
			qDebug("\t %i markers to solve Curve", cntCur3DCurveMarkers);

			solveCurveFromMarkers(); //////////
		}
	}
	cntCur3DCurveMarkers = 0;

	list_listCurvePos.clear();
	listMarkerPos.clear();

	if (selectMode != smObject)
	{
		selectMode = smObject;
		if (w) { w->setCursor(oldCursor); }
	}
}

void Renderer_tex2::_appendMarkerPos(int x, int y)
{
		MarkerPos pos;
		pos.x = x;
		pos.y = y;
		for (int i=0; i<4; i++)
			pos.view[i] = viewport[i];
		for (int i=0; i<16; i++)
		{
			pos.P[i]  = projectionMatrix[i];
			pos.MV[i] = markerViewMatrix[i];
		}
		listMarkerPos.append(pos);
		//qDebug("\t (%d, %d) listMarkerPos.size = %d", x,y, listMarkerPos.size());
}

int Renderer_tex2::movePen(int x, int y, bool b_move)
{
	//qDebug("  Renderer_tex2::movePen");

//	//100731 RZC
//	if (renderMode==rmCrossSection)
//		selectObj(x,y, false, 0); //no menu, no tip, just for lastSliceType

	// define a curve //091023
	if (selectMode == smCurveCreate1 || selectMode == smCurveCreate2 || selectMode == smCurveCreate3)
	{
		_appendMarkerPos(x,y);

		if (b_move)
		{
			//qDebug("\t track ( %i, %i ) to define Curve", x,y);

			this->sShowTrack = 1;
			return 1; //display 2d track
		}

		// release button
		qDebug("\t track-end ( %i, %i ) to define Curve (%i points)", x,y, listMarkerPos.size());

		if (listMarkerPos.size() >=3) //drop short click
			list_listCurvePos.append(listMarkerPos);
		listMarkerPos.clear();

		int N = (selectMode == smCurveCreate1)? 1 : (selectMode == smCurveCreate2)? 2 : 3;

		if (list_listCurvePos.size() >= N)
		{
			//qDebug("\t %i tracks to solve Curve", list_listCurvePos.size());

			if (selectMode == smCurveCreate1)
			{
				vector <XYZ> loc_vec_input; //here as an empty input, so use list_listCurvePos internal
				solveCurveCenter(loc_vec_input);
			}
			else
				solveCurveViews();

			list_listCurvePos.clear();
			if (selectMode != smCurveCreate1) // make 1-track continue selected mode
				endSelectMode();
		}
	}

	this->sShowTrack = 0;
	return 0; //no 2d track to display
}

int Renderer_tex2::hitPen(int x, int y)
{
	qDebug("  Renderer_tex2::hitPen");

//	//100731 RZC
//	if (renderMode==rmCrossSection)
//		selectObj(x,y, false, 0); //no menu, no tip, just for lastSliceType

	// define a curve //091023
	if (selectMode == smCurveCreate1 || selectMode == smCurveCreate2 || selectMode == smCurveCreate3)
	{
		qDebug("\t track-start ( %i, %i ) to define Curve", x,y);

		_appendMarkerPos(x,y);

		// endSlectMode() in movePen
		return 1;
	}

	else if (selectMode == smCurveCreate_pointclick) //091226
	{
		_appendMarkerPos(x,y);

		int N = 1;

		if (listMarkerPos.size() >= N)
		{
			qDebug("\t click ( %i, %i ) for Markers to Curve", x,y);

			solveMarkerCenter(); //////////
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
			//qDebug("\t %i clicks to solve Marker", listMarkerPos.size());

			if (selectMode == smMarkerCreate1)
				solveMarkerCenter(); //////////
			else
				solveMarkerViews(); //////////

			listMarkerPos.clear();
			if (selectMode != smMarkerCreate1) // make 1-click continue selected mode
				endSelectMode();
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

void Renderer_tex2::editSurfaceObjBasicGeometry(int dc, int st, int i) // i is 1-based
{
	qDebug("  Renderer_tex2::editSurfaceObjBasicGeometry");
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

void Renderer_tex2::editSurfaceObjAnnotation(int dc, int st, int i) // i is 1-based
{
	qDebug("  Renderer_tex2::editSurfaceObjAnnotation");
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

void Renderer_tex2::updateTracedNeuron()
{
#ifndef test_main_cpp
	qDebug("  Renderer_tex2::updateTracedNeuron");
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg =  v3dr_getImage4d(_idep); //by PHC, 090119
	if (curImg)
		curImg->update_3drenderer_neuron_view(w, this);
#endif
}

void Renderer_tex2::loadLandmarks_from_file(const QString & filename)
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

void Renderer_tex2::saveLandmarks_to_file(const QString & filename)
{
#ifndef test_main_cpp
	wirteMarker_file(filename, listMarker);
#endif
}


#define __info_of_object__

QString Renderer_tex2::info_Marker(int marker_i)
{
	QString tmpstr;
	if (marker_i>=0 && marker_i<listMarker.size())
	{
		const ImageMarker & S = listMarker.at(marker_i);
		tmpstr = QString("\n(%1, %2, %3)").arg(S.x).arg(S.y).arg(S.z);
	}
	return tmpstr;
}
QString Renderer_tex2::info_NeuronNode(int n_id, NeuronTree *p_tree)
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
	}
	return tmpstr;
}

QString Renderer_tex2::info_SurfVertex(int n_id, Triangle * face, int label)
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



V3DLONG Renderer_tex2::findNearestNeuronNode_WinXY(int cx, int cy, NeuronTree * ptree) //find the nearest node in a neuron in XY project of the display window
{
	if (!ptree) return -1;
	QList <NeuronSWC> *p_listneuron = &(ptree->listNeuron);
	if (!p_listneuron) return -1;

	//qDebug()<<"win click position:"<<cx<<" "<<cy;

	GLdouble px, py, pz, ix, iy, iz;

	V3DLONG best_ind=-1; double best_dist=-1;
	for (V3DLONG i=0;i<p_listneuron->size();i++)
	{
		ix = p_listneuron->at(i).x, iy = p_listneuron->at(i).y, iz = p_listneuron->at(i).z;
		GLint res = gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz);// note: should use the saved modelview,projection and viewport matrix
		py = viewport[3]-py; //the Y axis is reversed
		if (res==GL_FALSE) {qDebug()<<"gluProject() fails for NeuronTree ["<<i<<"] node"; return -1;}
		//qDebug()<<i<<" "<<px<<" "<<py<<" "<<pz<<"\n";

		double cur_dist = (px-cx)*(px-cx)+(py-cy)*(py-cy);
		if (i==0) {	best_dist = cur_dist; best_ind=0; }
		else {	if (cur_dist<best_dist) {best_dist=cur_dist; best_ind = i;}}
	}

	//best_ind = p_listneuron->at(best_ind).n; // this no used, because it changed in V_NeuronSWC
	return best_ind; //by PHC, 090209. return the index in the SWC file
}

Triangle * Renderer_tex2::findNearestSurfTriangle_WinXY(int cx, int cy, int & vertex_i, Triangle * plist)
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


double Renderer_tex2::computeSurfaceArea(int dc, int st, int index) //index is 1-based
{
	qDebug("  Renderer_tex2::computeSurfaceArea");

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

double Renderer_tex2::computeSurfaceVolume(int dc, int st, int index) //index is 1-based
{
	qDebug("  Renderer_tex2::computeSurfaceVolume");

	if (dc==dcSurface && st==stLabelSurface)
	{
		//LabelSurf &S = listLabelSurf[index-1];
		Triangle* pT = list_listTriangle[index-1];

	}
	return 0;
}



void Renderer_tex2::showLineProfile(int marker1, int marker2) // 0-based
{
	qDebug("  Renderer_tex2::showLineProfile");

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

QVector<int> Renderer_tex2::getLineProfile(XYZ P1, XYZ P2, int chno)
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
// (1) refineMarkerTranlate (-1)
// (2) addMarker, updateMakerLocation (+1)
// (3) drawMarkerList (-1)
// BUT IT IS STILL EASY TO BE CHAOS !!!
//##########################################################################################
//
// 090716: marker coordinates Fully are handled in paint() and Original image space!
//
//##########################################################################################
#define __creat_curve_and_marker___

void Renderer_tex2::solveCurveCenter(vector <XYZ> & loc_vec_input)
{
	bool b_use_seriespointclick = (loc_vec_input.size()>0) ? true : false;
	if (b_use_seriespointclick==false && list_listCurvePos.size()<1)  return;

	bool b_use_last_approximate=true;

#ifndef test_main_cpp
	MainWindow* V3Dmainwindow = 0;
	V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);
	if (V3Dmainwindow)
		b_use_last_approximate = V3Dmainwindow->global_setting.b_3dcurve_inertia;
#endif

	int chno=0;
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	if (w)
	{
		chno = w->getNumKeyHolding()-1;
		if (chno<0) chno = 0; //default channel 1
		qDebug()<<"in w chno for 3d curve"<<chno;
	}


	vector <XYZ> loc_vec;
	loc_vec.clear();

	int N = loc_vec_input.size();
	if (b_use_seriespointclick)
	{
		loc_vec = loc_vec_input;
	}
	else //then use the moving mouse location, otherwise using the preset loc_vec_input (which is set by the 3d-curve-by-point-click function)
	{
		N = list_listCurvePos.at(0).size();
		for (int i=0; i<N; i++)
		{
			const MarkerPos & pos = list_listCurvePos.at(0).at(i);

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

				loc = getCenterOfLineProfile(loc0, loc1, 0, chno);
			}

			if (dataViewProcBox.isInner(loc, 0.5))
			{
				dataViewProcBox.clamp(loc); //100722 RZC
				loc_vec.push_back(loc);
			}
		}
	}

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
				V3DLONG n_id_start = findNearestNeuronNode_WinXY(list_listCurvePos.at(0).at(0).x, list_listCurvePos.at(0).at(0).y, p_tree);
				V3DLONG n_id_end = findNearestNeuronNode_WinXY(list_listCurvePos.at(0).at(N-1).x, list_listCurvePos.at(0).at(N-1).y, p_tree);
				qDebug("detect nearest neuron node [%d] for curve-start and node [%d] for curve-end for the [%d] neuron", n_id_start, n_id_end, curEditingNeuron);

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

	if (b_addthiscurve)
	{
		addCurveSWC(loc_vec, chno);
	}
	else //100821
	{
		b_addthiscurve = true; //in this case, always reset to default to draw curve to add to a swc instead of just  zoom
		endSelectMode();

		produceZoomViewOf3DRoi(loc_vec);
	}

}

void Renderer_tex2::produceZoomViewOf3DRoi(vector <XYZ> & loc_vec)
{
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
#ifndef test_main_cpp
	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
	XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

	//qDebug("	_idep = %p, _idep->image4d = %p", _idep, ((iDrawExternalParameter*)_idep)->image4d);
	//qDebug("	My4DImage* = %p, XFormWidget* = %p", curImg, curXWidget);

#else
	void* curImg=0;
	void* curXWidget = 0;
#endif

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

		V3DLONG margin=5; //the default margin is small
		if (loc_vec.size()==1) margin=21; //for marker then define a bigger margin
		mx -= margin; Mx += margin; if (mx<0) mx=0; if (Mx>curImg->getXDim()-1) Mx = curImg->getXDim()-1;
		my -= margin; My += margin; if (my<0) my=0; if (My>curImg->getYDim()-1) My = curImg->getYDim()-1;
		mz -= margin; Mz += margin; if (mz<0) mz=0; if (Mz>curImg->getZDim()-1) Mz = curImg->getZDim()-1;

//by PHC 101008
		if (b_imaging && curXWidget)
		{
			b_imaging = false; //reset the status

			//set the hiddenSelectWidget for the V3D mainwindow
			if (!curXWidget->getMainControlWindow()->setCurHiddenSelectedWindow(curXWidget))
			{
				v3d_msg("Fail to set up the curHiddenSelectedXWidget for the V3D mainwindow. Do nothing.");
				return;
			}

			//set up parameters
			v3d_imaging_paras myimagingp;
			myimagingp.OPS = "ROI_IMAGING";
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
			v3d_imaging(curXWidget->getMainControlWindow(), myimagingp);
		}


		curXWidget->setLocal3DViewerBBox(mx, Mx, my, My, mz, Mz);
		//QTimer::singleShot( 1000, curXWidget, SLOT(doImage3DLocalView()) );

		curXWidget->doImage3DLocalBBoxView(); //by PHC 101012. move from before if(b_imaging...)

	}
}

void Renderer_tex2::solveCurveViews()
{
	qDebug("  Renderer_tex2::solveCurveViews");

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
				double dist = getDistanceOfMarkerPos(pos0, pos);

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

	addCurveSWC(loc_vec, -1); //turn off post deform
}

void Renderer_tex2::solveCurveFromMarkers()
{
	qDebug("  Renderer_tex2::solveCurveMarkers");

#ifndef test_main_cpp
	vector <XYZ> loc_vec_input;

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep); //by PHC, 090119
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


void Renderer_tex2::solveMarkerCenter()
{
	if (listMarkerPos.size()<1)  return;

	const MarkerPos & pos = listMarkerPos.at(0);

	XYZ loc = getCenterOfMarkerPos(pos);

	if (dataViewProcBox.isInner(loc, 0.5)) //100725 RZC
		dataViewProcBox.clamp(loc); //100722 RZC

	if (b_addthismarker) //100822, PHC
		addMarker(loc);
	else //then zoom-in, 100822, PHC
	{
		b_addthismarker = true; //in this case, always reset to default to add a marker instead of just  zoom
		endSelectMode();

		vector <XYZ> loc_vec;
		loc_vec.push_back(loc);
		produceZoomViewOf3DRoi(loc_vec);
	}
}

void Renderer_tex2::refineMarkerCenter()
{
	if (currentMarkerName<1 || currentMarkerName>listMarker.size())  return;
	if (listMarkerPos.size()<1)  return;

	const MarkerPos & pos = listMarkerPos.at(0);

	XYZ loc = getCenterOfMarkerPos(pos);
	//added by PHC, 090120. update the marker location in both views
	updateMarkerLocation(currentMarkerName-1, loc);
}


bool Renderer_tex2::isInBound(const XYZ & loc, float factor, bool b_message)
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

void Renderer_tex2::solveMarkerViews()
{
	if (listMarkerPos.size()<2)  return;

	XYZ loc = getLocationOfListMarkerPos();

	if (isInBound(loc, 0.01, true))
	{
		addMarker(loc);
	}
}

void Renderer_tex2::refineMarkerTranslate()
{
	if (currentMarkerName<1 || currentMarkerName>listMarker.size())  return;
	if (listMarkerPos.size()<1)  return;

		const MarkerPos & pos = listMarkerPos.at(0);
		ImageMarker & S = listMarker[currentMarkerName-1];
		XYZ pt(S.x-1, S.y-1, S.z-1); // 090505 RZC : marker position is 1-based

		ColumnVector X(4);		X << pt.x << pt.y << pt.z << 1;

		Matrix P(4,4);		P << pos.P;   P = P.t();    // OpenGL is row-inner / C is column-inner
		Matrix M(4,4);		M << pos.MV;  M = M.t();
		Matrix PM = P * M;
		//cout << "P M PM \n" << P << endl << M << endl << PM << endl;

		ColumnVector pX  = PM * X;
		pX = pX / pX(4);
		//cout << " pX \n" << pX.t() << endl;

		double x = (pos.x             - pos.view[0])*2.0/pos.view[2] -1;
		double y = (pos.view[3]-pos.y - pos.view[1])*2.0/pos.view[3] -1; // OpenGL is bottom to top
		double z = pX(3);                              // hold the clip space depth

	ColumnVector pY(4); 	pY << x << y << z << 1;
	ColumnVector Y = PM.i() * pY;
	Y = Y / Y(4);
	cout << "refine from: " << X.t() //<< endl
	     << "         to: " << Y.t() << endl;;

	XYZ loc(Y(1), Y(2), Y(3));

	//added by PHC, 090120. update the marker location in both views
	updateMarkerLocation(currentMarkerName-1, loc);
}

void Renderer_tex2::refineMarkerLocal(int marker_id)
{
	if (marker_id<0 || marker_id>listMarker.size()-1)  return;

	XYZ P = XYZ(listMarker[marker_id])-XYZ(1,1,1);
	XYZ loc = getCenterOfLocal(P);

	//added by PHC, 090120. update the marker location in both views
	updateMarkerLocation(marker_id, loc);
}

void Renderer_tex2::addMarker(XYZ &loc)
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
		S.inputProperty = pxLocaUseful;
		S.x = pt.x;
		S.y = pt.y;
		S.z = pt.z;
		if (V3Dmainwindow)
			S.radius = V3Dmainwindow->global_setting.default_marker_radius;
		S.color = random_rgba8(255);
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
	S.color = random_rgba8(255);
	S.on = true;

	listMarker.append(S);

#endif
}

void Renderer_tex2::updateMarkerLocation(int marker_id, XYZ &loc) //added by PHC, 090120
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


#define __computation__

// in Image space (model space)
void Renderer_tex2::_MarkerPos_to_NearFarPoint(const MarkerPos & pos, XYZ &loc0, XYZ &loc1)
{
	Matrix P(4,4);		P << pos.P;   P = P.t();    // OpenGL is row-inner / C is column-inner
	Matrix M(4,4);		M << pos.MV;  M = M.t();
	Matrix PM = P * M;
	//cout << "P M PM \n" << P << endl << M << endl << PM << endl;

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

double Renderer_tex2::getDistanceOfMarkerPos(const MarkerPos & pos0, const MarkerPos & pos)
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

XYZ Renderer_tex2::getCenterOfMarkerPos(const MarkerPos & pos)
{
	XYZ loc0, loc1;
	_MarkerPos_to_NearFarPoint(pos, loc0, loc1);
	qDebug("loc0--loc1: (%g, %g, %g)--(%g, %g, %g)\n", loc0.x,loc0.y,loc0.z, loc1.x,loc1.y,loc1.z);

//	//100730 RZC
//	// in View space, keep for dot(clip, pos)>=0
//	double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
//	// [0, 1] ==> [+1, -1]*(s)
//	clipplane[3] = viewClip;
//	ViewPlaneToModel(pos.MV, clipplane);

	XYZ loc = getCenterOfLineProfile(loc0, loc1, 0);//clipplane);
	return loc;
}

XYZ Renderer_tex2::getLocationOfListMarkerPos()
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

XYZ Renderer_tex2::getPointOnPlane(XYZ P1, XYZ P2, double plane[4]) //100731
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
XYZ Renderer_tex2::getPointOnSections(XYZ P1, XYZ P2, double f_plane[4]) //100801
{
	double plane[4];
	XYZ P = P2; // from the far location
	XYZ loc;

#define REPLACE_NEAR() \
	loc = getPointOnPlane(P1,P2, plane); \
	if (dist_L2(loc,P2) > dist_L2(P,P2) && dataViewProcBox.isInner(loc, 0.5)) \
		P = loc;

	//qDebug("  P1(%g %g %g)  P2(%g %g %g)", P1.x,P1.y,P1.z, P2.x,P2.y,P2.z);
	if (bXSlice)
	{
		plane[0] = -1; plane[1] = 0; plane[2] = 0; plane[3] = start1+ VOL_X0*(size1-1);
		REPLACE_NEAR();
		//qDebug("  X-(%g %g %g)", loc.x,loc.y,loc.z);
	}
	if (bYSlice)
	{
		plane[0] = 0; plane[1] = -1; plane[2] = 0; plane[3] = start2+ VOL_Y0*(size2-1);
		REPLACE_NEAR();
		//qDebug("  Y-(%g %g %g)", loc.x,loc.y,loc.z);
	}
	if (bZSlice)
	{
		plane[0] = 0; plane[1] = 0; plane[2] = -1; plane[3] = start3+ VOL_Z0*(size3-1);
		REPLACE_NEAR();
		//qDebug("  Z-(%g %g %g)", loc.x,loc.y,loc.z);
	}
	if (bFSlice)
	{
		for (int i=0; i<4; i++) plane[i] = f_plane[i];
		REPLACE_NEAR();
		//qDebug("  F-(%g %g %g)", loc.x,loc.y,loc.z);
	}
	//qDebug("  P(%g %g %g)", P.x,P.y,P.z);
	return P;
}

// in Image space (model space)
XYZ Renderer_tex2::getCenterOfLineProfile(XYZ P1, XYZ P2, double clip[4], int chno)
{
	//100731 RZC
	// in View space, keep for dot(clip, pos)>=0
	double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
	// [0, 1] ==> [+1, -1]*(s)
	clipplane[3] = viewClip;
	ViewPlaneToModel(markerViewMatrix, clipplane);
	//qDebug()<<"   clipplane:"<<clipplane[0]<<clipplane[1]<<clipplane[2]<<clipplane[3];

	if (clip)
	{
		for (int i=0; i<4; i++) clipplane[i]=clip[i];
	}
	//qDebug()<<"   clip:     "<<clipplane[0]<<clipplane[1]<<clipplane[2]<<clipplane[3];

	if (renderMode==rmCrossSection)
	{
		return getPointOnSections(P1,P2, clipplane);
	}

	XYZ loc = (P1+P2)*.5;

#ifndef test_main_cpp

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;

	////////////////////////////////////////////////////////////////////////
	if (w)
	{
		curImg = v3dr_getImage4d(_idep);
		//when chno<0, then need to recheck the current chno
		if (chno<0) chno = w->getNumKeyHolding()-1;
		//if (chno<0) chno = 0; //default channel 1
		if (chno<0) chno = curChannel; //100802 RZC: default channel set by user
	}
	////////////////////////////////////////////////////////////////////////

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

		for (int i=0; i<200; i++) // iteration, (2-f)^200 is big enough
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
	}
#endif

	//100721: Now small tag added in sampling3dUINT8atBounding makes loc in bound box
	return loc;
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

XYZ Renderer_tex2::getCenterOfLocal(XYZ P)
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


