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

/********************************************************

NOTICE 12/1/2016, MK

Due to the use of Windows Kits 8.1, the variable scr2 has been defined in dlgs.h and cannot be declared here. Changed the varuable name to sqr2 to avoid the error.

********************************************************/


//v3dimg_proc_neuron.cpp
//by Hanchuan Peng
//separated from my4DImage.cpp
//2009-11-14
//2010-01-30

#include "../3drenderer/v3dr_common.h"
#include "v3d_core.h"
#include "dialog_curve_trace_para.h"
#include "mainwindow.h"
#include "v3d_application.h"

#include "../neuron_tracing/neuron_tracing.h"
#include "../3drenderer/barFigureDialog.h"
#include "../terafly/src/control/CPlugin.h"
//#ifdef __ALLOW_VR_FUNCS__
//#include "../mozak/MozakUI.h"
//#endif

//LMG for cross-platform UTC Timestamp 15/10/2018
#if defined(Q_OS_WIN32)
    #define timegm _mkgmtime
#elif defined(Q_OS_WIN64)
    #define timegm _mkgmtime
#endif

//------------------------------------------------------------------------------------------

#define CATCH_TO_QString( type, msg ) \
	catch (std::exception& e) { \
		type = "[std]"; \
		msg = e.what(); \
	} \
	catch (int id) { \
		type = "[int]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (char* str) { \
		type = "[char]"; \
		msg = (const char*)str; \
	} \
	catch (const char* str) { \
		type = "[cchar]"; \
		msg = str; \
	} \
	catch (...) { \
		type = "[unknown]"; \
		msg = QString("unknown exception in file(%1) at line(%2)").arg(__FILE__).arg(__LINE__); \
	}
#define TRACE_ERROR_MSG(etype, emsg, trace_res, funcname) \
	if (etype=="[std]") \
	{ \
		QMessageBox::warning(0, "Tracing failure", QString("Exception in %1 \n\n").arg(funcname) + emsg + \
				"\n\nOut of memory!"); \
		return false; \
	} \
	else if (emsg.size() || trace_res==false) \
	{ \
		QMessageBox::warning(0, "Tracing failure", QString("Exception in %1 \n\n").arg(funcname) + emsg + \
				"\n\nPlease make sure that:\n  (1) the markers are located in valid CONNECTED image region.\n  (2) the chosen image CHANNEL is your wanted."); \
		return false; \
	}

//#define SAVE_HISTORY_OF_TRACE_STEP -2
#define SAVE_HISTORY_OF_TRACE_STEP 0 // must be <=0; 0 only save the final res, -1 also saves the smoothing res, -2 also save the graph step res. -3 for a special use
//#define FORCE_GRAPH_RESOLUTION	//trace_para.sp_graph_resolution_step=2; //force high resolution


bool My4DImage::proj_trace_deformablepath_one_point(V3DLONG startmark_id)
{
	if (startmark_id<0 && startmark_id>=listLandmarks.size())
	{
		v3d_msg("You input the invalid landmarks for tracing, - do nothing. \n");
		return false;
	}

	CurveTracePara trace_para;
	{
		trace_para.landmark_id_start = startmark_id;
		trace_para.landmark_id_end = startmark_id;
		trace_para.sp_num_end_nodes = 0;
		trace_para.b_deformcurve = false;

		CurveTraceParaDialog d(this->getCDim(), this->listLandmarks.size(), trace_para);
		int res=d.exec();
		if (res==QDialog::Accepted)
		{
			d.fetchData(&trace_para);
			startmark_id = trace_para.landmark_id_start;
		}
		else
		{
			qDebug("You cancel the tracing. ");
			return false;
		}
	}

	//because it is possible the user change the starting and ending landmarks in the dialog, thus use the latest ones
	LocationSimple l1 = listLandmarks.at(startmark_id); l1.x-=1;l1.y-=1;l1.z-=1;
	//LocationSimple l2 = l1; // make tracing from 1 point
	vector<LocationSimple> ll2; ll2.clear(); // 090727 RZC: redirect to the all points function

	int trace_res = 0; // 090506 RZC
	QString etype, emsg; // 090508 RZC
	try { ////////////////////////////////////////////////////////////////////////////
		qDebug("try proj_trace_deformablepath_one_point()... ");

		//trace_res = proj_trace_deformablepath_two_points_shortestdist(l1, l2, trace_para); // 090508: add trace_para
		trace_res = proj_trace_deformablepath_all_points_shortestdist(l1, ll2, trace_para); // 090727: redirect to the all points function

	} ////////////////////////////////////////////////////////////////////////////////
	CATCH_TO_QString(etype, emsg);
	TRACE_ERROR_MSG(etype, emsg, trace_res, "proj_trace_deformablepath_one_point()");

	if (trace_res) 	proj_trace_history_append();
	return true;
}

bool My4DImage::proj_trace_deformablepath_one_point_to_allotherpoints(V3DLONG startmark_id)
{
	if (startmark_id<0 && startmark_id>=listLandmarks.size())
	{
		v3d_msg("You input the invalid landmarks for tracing, - do nothing. \n");
		return false;
	}

	CurveTracePara trace_para;
	{
		trace_para.landmark_id_start = startmark_id;
		trace_para.landmark_id_end = startmark_id;
		trace_para.sp_num_end_nodes = 2;
		trace_para.b_deformcurve = false;

		CurveTraceParaDialog d(this->getCDim(), this->listLandmarks.size(), trace_para);
		int res=d.exec();
		if (res==QDialog::Accepted)
		{
			d.fetchData(&trace_para);
			startmark_id = trace_para.landmark_id_start;
		}
		else
		{
			qDebug("You cancel the tracing. ");
			return false;
		}
	}

	//because it is possible the user change the starting and ending landmarks in the dialog, thus use the latest ones
	LocationSimple l1 = listLandmarks.at(startmark_id); l1.x-=1;l1.y-=1;l1.z-=1;
	vector<LocationSimple> ll2; // make tracing to all points
	ll2.clear();
	for (int i=0;i<listLandmarks.size();i++)
	{
		if (i==startmark_id)	continue;
		LocationSimple l2 = listLandmarks.at(i); l2.x-=1;l2.y-=1;l2.z-=1;
		ll2.push_back(l2);
	}

	int last_seg = tracedNeuron.nsegs()-1;//091023 RZC
	int trace_res = 0;
	QString etype, emsg;
	try { ////////////////////////////////////////////////////////////////////////////
		qDebug("try proj_trace_all... ");

		trace_res = proj_trace_deformablepath_all_points_shortestdist(l1, ll2, trace_para);

#if SAVE_HISTORY_OF_TRACE_STEP<=-2
		if (trace_res) 	proj_trace_history_append();
#endif

		if (trace_res)	proj_trace_smooth_downsample_last_traced_neuron(trace_para, last_seg+1, tracedNeuron.nsegs()-1);

#if SAVE_HISTORY_OF_TRACE_STEP<=-1
		if (trace_res) 	proj_trace_history_append();
#endif

		if (trace_res && trace_para.b_estRadii) proj_trace_compute_radius_of_last_traced_neuron(trace_para, last_seg+1, tracedNeuron.nsegs()-1, trace_z_thickness);

		if (trace_res && trace_para.b_postMergeClosebyBranches) proj_trace_mergeAllClosebyNeuronNodes(); //PHC, 100406

	} ////////////////////////////////////////////////////////////////////////////////
	CATCH_TO_QString(etype, emsg);
	TRACE_ERROR_MSG(etype, emsg, trace_res, "proj_trace_deformablepath_one_point_to_allotherpoints()");

	if (trace_res) 	proj_trace_history_append();
	return true;
}

bool My4DImage::proj_trace_deformablepath_two_points(V3DLONG startmark_id, V3DLONG endmark_id, bool b_select_para, int method_code) //overloading function provided for convenience
{
	return proj_trace_deformablepath_two_points(startmark_id, endmark_id, 8, b_select_para, true, method_code);
}

bool My4DImage::proj_trace_deformablepath_two_points(V3DLONG startmark_id, V3DLONG endmark_id, int npoints, bool b_select_para, bool b_fitradius, int method_code)
{
	if (startmark_id<0 || startmark_id>=listLandmarks.size() ||
	    endmark_id<0   || endmark_id>=listLandmarks.size()   ||
	    startmark_id == endmark_id)
	{
		v3d_msg("You input the same starting and ending landmarks or invalid landmarks for tracing, - do nothing. \n");
		return false;
	}

	CurveTracePara trace_para;
	trace_para.landmark_id_start = startmark_id;
	trace_para.landmark_id_end = endmark_id;
	trace_para.n_points = npoints;
	trace_para.b_deformcurve = (method_code==0)?false:true;

	if (b_select_para)
	{
		CurveTraceParaDialog d(this->getCDim(), this->listLandmarks.size(), trace_para);
		int res=d.exec();
		if (res==QDialog::Accepted)
		{
			d.fetchData(&trace_para);
		}
		else
		{
			qDebug("You cancel the tracing. ");
			return false;
		}
	}
//		else
//		{
//			trace_para.b_deformcurve = false; //if not select parameter, then use deformable model
//		}

	//because it is possible the user change the starting and ending landmarks in the dialog, thus use the latest ones
	LocationSimple l1 = listLandmarks.at(trace_para.landmark_id_start); l1.x-=1;l1.y-=1;l1.z-=1;
	LocationSimple l2 = listLandmarks.at(trace_para.landmark_id_end);   l2.x-=1;l2.y-=1;l2.z-=1;

	int last_seg = tracedNeuron.nsegs()-1; //091023 RZC
	int trace_res = 0;
	QString etype, emsg;
	try { ////////////////////////////////////////////////////////////////////////////
		qDebug("try proj_trace_two... ");

		if (trace_para.b_deformcurve)
		{
			if (trace_para.b_adaptiveCtrlPoints)
				trace_res = proj_trace_deformablepath_two_points_basic(l1, l2, trace_para); //if selected para, then use the basic version
			else
				trace_res = proj_trace_deformablepath_two_points_adaptive(l1, l2, trace_para); //if not select para, then use the adaptive version
		}
		else
		{
			trace_res = proj_trace_deformablepath_two_points_shortestdist(l1, l2, trace_para);

#if SAVE_HISTORY_OF_TRACE_STEP<=-2
			if (trace_res) 	proj_trace_history_append();
#endif

			// also can do smoothing inside in find_shortest_path_graphimg
			proj_trace_smooth_downsample_last_traced_neuron(trace_para, last_seg+1, tracedNeuron.nsegs()-1);

		}

#if SAVE_HISTORY_OF_TRACE_STEP<=-1
		if (trace_res) 	proj_trace_history_append();
#endif

		// 090518
		if (trace_res && b_fitradius) //090712: b_fitradius must be used by draw 3D line function
			proj_trace_compute_radius_of_last_traced_neuron(trace_para, last_seg+1, tracedNeuron.nsegs()-1, trace_z_thickness);

	} ////////////////////////////////////////////////////////////////////////////////
	CATCH_TO_QString(etype, emsg);
	TRACE_ERROR_MSG(etype, emsg, trace_res, "proj_trace_deformablepath_two_points()");

	if (trace_res) 	proj_trace_history_append();
	return true;
}

//------------------------------------------------------------------------------
//
#define CHECK_DATA_trace_deformablepath() \
	if (trace_para.channo<0 || trace_para.channo>=getCDim()) \
	{ \
		v3d_msg ("Color channel info is not valid in CHECK_DATA_trace_deformablepath MACRO;\n"); \
		throw ("Color channel info is not valid in CHECK_DATA_trace_deformablepath MACRO();\n"); \
		return false; \
	} \
	if (this->getDatatype()!=V3D_UINT8 || !data4d_uint8 ) \
	{ \
		v3d_msg ("Now supports UINT8 only in CHECK_DATA_trace_deformablepath MACRO.\n"); \
		throw ("Now supports UINT8 only in CHECK_DATA_trace_deformablepath MACRO.\n"); \
		return false; \
	}


#define TRACED_NAME "vaa3d_traced_neuron"
#define TRACED_FILE "vaa3d_traced_neuron"


bool My4DImage::proj_trace_deformablepath_two_points_basic(LocationSimple & p1, LocationSimple & p2, CurveTracePara & trace_para)
{
	CHECK_DATA_trace_deformablepath();

	int i;
	int N=trace_para.n_points;
	unsigned char ***inimg_data3d = data4d_uint8[trace_para.channo];
	vector <Coord3D> mCoord;
	Coord3D curpos;
	double xstep = double(p2.x-p1.x)/N;
	double ystep = double(p2.y-p1.y)/N;
	double zstep = double(p2.z-p1.z)/N;
	for (i=0;i<=N;i++)
	{
		curpos.x = p1.x + xstep*i;
		curpos.y = p1.y + ystep*i;
		curpos.z = p1.z + zstep*i;
		mCoord.push_back(curpos);
	}

	BDB_Minus_ConfigParameter bdb_para;
	bdb_para.nloops   =trace_para.nloops;
	bdb_para.f_image  =1;
	bdb_para.f_smooth =trace_para.internal_force2_weight;
	bdb_para.f_length =trace_para.internal_force_weight;
	bdb_para.TH = 2;
	bdb_para.radius = sqrt(xstep*xstep+ystep*ystep+zstep*zstep)/2.0;

	bool res = point_bdb_minus_3d_localwinmass(inimg_data3d, getXDim(), getYDim(), getZDim(), mCoord, bdb_para);

	LocationSimple curpt;
	if (res==true) //then add the detected positions to the list
	{
		//V3DLONG nexist = tracedNeuron.nnodes();
		V3DLONG nexist = tracedNeuron.maxnoden();

		V_NeuronSWC cur_seg;
		set_simple_path (cur_seg, nexist, mCoord, false); // reverse link

		QString tmpss;  tmpss.setNum(tracedNeuron.nsegs()+1);
		cur_seg.name = qPrintable(tmpss);
		cur_seg.b_linegraph=true; //donot forget to do this
		tracedNeuron.append(cur_seg);
		tracedNeuron.name = TRACED_NAME;
		tracedNeuron.file = TRACED_FILE;
	}

	return res; // 090506 RZC
}

bool My4DImage::proj_trace_deformablepath_two_points_adaptive(LocationSimple & p1, LocationSimple & p2, CurveTracePara & trace_para)
{
	CHECK_DATA_trace_deformablepath();

	bool res = false;
	QList < vector <Coord3D> > pathList;
	double bestscore; V3DLONG besti;
	int k, i, N;
	int N2=ceil((double)trace_para.n_points/2.0+0.5);
	for (k=N2; k<trace_para.n_points+N2; k++)
	{
		N=k;
		unsigned char ***inimg_data3d = data4d_uint8[trace_para.channo];
		vector <Coord3D> mCoord;
		Coord3D curpos;
		double xstep = double(p2.x-p1.x)/N;
		double ystep = double(p2.y-p1.y)/N;
		double zstep = double(p2.z-p1.z)/N;
		for (i=0;i<=N;i++)
		{
			curpos.x = p1.x + xstep*i;
			curpos.y = p1.y + ystep*i;
			curpos.z = p1.z + zstep*i;
			mCoord.push_back(curpos);
			//qDebug("init pos [%d][%d] %5.3f %5.3f %5.3f ", k, i, curpos.x, curpos.y, curpos.z);
		}

		BDB_Minus_ConfigParameter bdb_para;
		bdb_para.nloops   =trace_para.nloops;
		bdb_para.f_image  =1;
		bdb_para.f_smooth =trace_para.internal_force2_weight;
		bdb_para.f_length =trace_para.internal_force_weight;
		bdb_para.TH = 2;
		bdb_para.radius = sqrt(xstep*xstep+ystep*ystep+zstep*zstep)/2.0;

		bool res = point_bdb_minus_3d_localwinmass(inimg_data3d, getXDim(), getYDim(), getZDim(), mCoord, bdb_para);

		pathList.append(mCoord);

		Basic_Path_Statistics bps;
		path_statistics_3d(bps, inimg_data3d, getXDim(), getYDim(), getZDim(), mCoord);

		double cur_path_score = (bps.ave*k);
		printf("path [%d] score = %5.3f\n", k, cur_path_score);
		if (k==N2) {bestscore = cur_path_score; besti=k;}
		else {if (bestscore<cur_path_score) {bestscore = cur_path_score; besti=k;}}
	}

	vector <Coord3D> mCoord = pathList.at(besti-N2);
	//qDebug("path len should be  = %d, actual path len = %d", besti+1, mCoord.size());

	LocationSimple curpt;
	if (res==true) //then add the detected positions to the list
	{
		//V3DLONG nexist = tracedNeuron.nnodes();
		V3DLONG nexist = tracedNeuron.maxnoden();

		V_NeuronSWC cur_seg;
		set_simple_path (cur_seg, nexist, mCoord, false); //reverse link

		QString tmpss;  tmpss.setNum(tracedNeuron.nsegs()+1);
		cur_seg.name = qPrintable(tmpss);
		cur_seg.b_linegraph=true; //donot forget to do this
		tracedNeuron.append(cur_seg);
		tracedNeuron.name = TRACED_NAME;
		tracedNeuron.file = TRACED_FILE;
	}

	return res; // 090506 RZC
}

int My4DImage::proj_trace_deformablepath_two_points_shortestdist(LocationSimple & p1, LocationSimple & p2, CurveTracePara & trace_para) //added on 090208
{
	vector<LocationSimple> ll2; // redirect to all points function
	ll2.clear();
	ll2.push_back(p2);
	return proj_trace_deformablepath_all_points_shortestdist(p1, ll2, trace_para); // 090508 RZC: add trace_para
}

int My4DImage::proj_trace_deformablepath_all_points_shortestdist(LocationSimple & p0, vector<LocationSimple> & pp, CurveTracePara & trace_para) //added on 090208
{
	CHECK_DATA_trace_deformablepath();

	ParaShortestPath sp_para;
	sp_para.edge_select       = trace_para.sp_graph_connect; // 090621 RZC: add sp_graph_connect selection
	sp_para.background_select = trace_para.sp_graph_background; // 090829 RZC: add sp_graph_background selection
	sp_para.node_step      = trace_para.sp_graph_resolution_step; // 090610 RZC: relax the odd constraint.
	sp_para.outsample_step = trace_para.sp_downsample_step;
	sp_para.smooth_winsize = trace_para.sp_smoothing_win_sz;

	vector< vector<V_NeuronSWC_unit> > mmUnit;
	int chano = trace_para.channo;
	int n_end_nodes = pp.size();
	vector<float> px, py, pz;
	px.clear(), py.clear(), pz.clear();
	for (int i=0;i<pp.size();i++) {px.push_back(pp[i].x), py.push_back(pp[i].y), pz.push_back(pp[i].z);}

	qDebug("find_shortest_path_graphimg >>> ");
	if (trace_bounding_box == NULL_BoundingBox)
	{
		trace_bounding_box.x0 = trace_bounding_box.y0 = trace_bounding_box.z0 = 0;
		trace_bounding_box.x1 = getXDim()-1;
		trace_bounding_box.y1 = getYDim()-1;
		trace_bounding_box.z1 = getZDim()-1;
		printf("set z1=%ld\n", V3DLONG(trace_bounding_box.z1));
	}
	printf("z1=%ld\n", V3DLONG(trace_bounding_box.z1));

        const char* s_error = find_shortest_path_graphimg(data4d_uint8[chano], getXDim(), getYDim(), getZDim(),
			trace_z_thickness,
			trace_bounding_box.x0, trace_bounding_box.y0, trace_bounding_box.z0,
			trace_bounding_box.x1, trace_bounding_box.y1, trace_bounding_box.z1,
			p0.x, p0.y, p0.z,
			n_end_nodes,
			&(px[0]), &(py[0]), &(pz[0]),
			mmUnit,
			sp_para);
	qDebug("find_shortest_path_graphimg <<< ");
	if (s_error)
	{
		//v3d_msg ("Fail to run find_shortest_path_graphimg() in proj_trace_deformablepath_two_points_shortestdist()\n");
		throw (const char*)s_error;
		return 0;
	}

	//merge traced path /////////////////////////////////////////////////////////
//	if (n_end_nodes >=2)
//	{
//
//		merge_back_traced_paths(mmUnit); // start --> n end
//
//#if SAVE_HISTORY_OF_TRACE_STEP<=-3
//		{
//			// append to tracedNeuron for saving history
//			for (V3DLONG ii=0;ii<mmUnit.size();ii++)
//			{
//				//V3DLONG nexist = tracedNeuron.nnodes();
//				V3DLONG nexist = tracedNeuron.maxnoden();
//
//				V_NeuronSWC cur_seg;	cur_seg.clear();//////////must clear
//				vector<V_NeuronSWC_unit> & mUnit = mmUnit[ii];
//
//				for (V3DLONG i=0;i<mUnit.size();i++)
//				{
//					if (mUnit[i].nchild<0) continue; //090610 RZC: here for saving the V3DLONG time of deleting nodes of nchild<0
//
//					V_NeuronSWC_unit v;
//					set_simple_path_unit (v, nexist, mUnit, i, true); // n end --> start
//
//					cur_seg.append(v);
//					//qDebug("%d ", cur_seg.nnodes());
//				}
//
//				QString tmpss;  tmpss.setNum(tracedNeuron.nsegs()+1);
//				cur_seg.name = qPrintable(tmpss);
//				cur_seg.b_linegraph=true; //donot forget to do this
//				tracedNeuron.append(cur_seg);
//				tracedNeuron.name = TRACED_NAME;
//				tracedNeuron.file = TRACED_FILE;
//			}
//
//			proj_trace_history_append();
//
//			// restore tracedNeuron.seg
//			for (V3DLONG ii=0;ii<mmUnit.size();ii++)	tracedNeuron.seg.pop_back();
//		}
//#endif
//	}
//
//	//put into tracedNeuron /////////////////////////////////////////////////////////
//	if (n_end_nodes==0) // entire image, direct copy
//	{
//		//V3DLONG nexist = tracedNeuron.nnodes();
//		V3DLONG nexist = tracedNeuron.maxnoden();
//
//		V_NeuronSWC cur_seg;	cur_seg.clear();
//		vector<V_NeuronSWC_unit> & mUnit = mmUnit[0];
//
//		for (V3DLONG i=0;i<mUnit.size();i++)
//		{
//			if (mUnit[i].nchild<0) continue; //090610: here for saving the V3DLONG time of deleting nodes of nchild<0
//
//			V_NeuronSWC_unit node = mUnit[i];
//			node.r = 0.5;
//			node.n += nexist;
//			if (node.parent >=1)  node.parent += nexist;
//			else node.parent = -1;
//			cur_seg.append(node);
//		}
//
//		QString tmpss;  tmpss.setNum(tracedNeuron.nsegs()+1);
//		cur_seg.name = qPrintable(tmpss);
//		cur_seg.b_linegraph=false; //donot forget to do this
//		tracedNeuron.append(cur_seg);
//		tracedNeuron.name = TRACED_NAME;
//		tracedNeuron.file = TRACED_FILE;
//	}
//
//	else
//	{
//		for (V3DLONG ii=0;ii<mmUnit.size();ii++)
//		{
//			//V3DLONG nexist = tracedNeuron.nnodes();
//			V3DLONG nexist = tracedNeuron.maxnoden();
//
//			V_NeuronSWC cur_seg;	cur_seg.clear();
//			vector<V_NeuronSWC_unit> & mUnit = mmUnit[ii];
//
//			for (V3DLONG i=0;i<mUnit.size();i++)
//			{
//				if (mUnit[i].nchild<0) continue; //090610: here for saving the V3DLONG time of deleting nodes of nchild<0
//
//				V_NeuronSWC_unit v;
//				set_simple_path_unit (v, nexist, mUnit, i, (n_end_nodes==1)); // link_order determined by 1/N path
//
//				cur_seg.append(v);
//				//qDebug("%d ", cur_seg.nnodes());
//			}
//
//			QString tmpss;  tmpss.setNum(tracedNeuron.nsegs()+1);
//			cur_seg.name = qPrintable(tmpss);
//			cur_seg.b_linegraph=true; //donot forget to do this
//			tracedNeuron.append(cur_seg);
//			tracedNeuron.name = TRACED_NAME;
//			tracedNeuron.file = TRACED_FILE;
//		}
//	}
//
//	return mmUnit.size();
	return mergeback_mmunits_to_neuron_path(n_end_nodes, mmUnit, tracedNeuron);
}


int mergeback_mmunits_to_neuron_path(int n_end_nodes, vector< vector<V_NeuronSWC_unit> > & mmUnit, V_NeuronSWC_list & tNeuron)
{
	//merge traced path /////////////////////////////////////////////////////////
	if (n_end_nodes >=2)
	{
		merge_back_traced_paths(mmUnit); // start --> n end

#if SAVE_HISTORY_OF_TRACE_STEP<=-3
		{
			// append to tNeuron for saving history
			for (V3DLONG ii=0;ii<mmUnit.size();ii++)
			{
				//V3DLONG nexist = tNeuron.nnodes();
				V3DLONG nexist = tNeuron.maxnoden();

				V_NeuronSWC cur_seg;	cur_seg.clear();//////////must clear
				vector<V_NeuronSWC_unit> & mUnit = mmUnit[ii];

				for (V3DLONG i=0;i<mUnit.size();i++)
				{
					if (mUnit[i].nchild<0) continue; //here for saving the V3DLONG time of deleting nodes of nchild<0

					V_NeuronSWC_unit v;
					set_simple_path_unit (v, nexist, mUnit, i, true); // n end --> start

					cur_seg.append(v);
					//qDebug("%d ", cur_seg.nnodes());
				}

				QString tmpss;  tmpss.setNum(tNeuron.nsegs()+1);
				cur_seg.name = qPrintable(tmpss);
				cur_seg.b_linegraph=true; //donot forget to do this
				tNeuron.append(cur_seg);
				tNeuron.name = TRACED_NAME;
				tNeuron.file = TRACED_FILE;
			}

			proj_trace_history_append(tNeuron);

			// restore tNeuron.seg
			for (V3DLONG ii=0;ii<mmUnit.size();ii++)	tNeuron.seg.pop_back();
		}
#endif
	}
	//put into tNeuron /////////////////////////////////////////////////////////
	if (n_end_nodes<=0) // entire image, direct copy
	{
		//V3DLONG nexist = tNeuron.nnodes();
		V3DLONG nexist = tNeuron.maxnoden();

		V_NeuronSWC cur_seg;	cur_seg.clear();
		vector<V_NeuronSWC_unit> & mUnit = mmUnit[0];

		for (V3DLONG i=0;i<mUnit.size();i++)
		{
			if (mUnit[i].nchild<0) continue; //090610: here for saving the V3DLONG time of deleting nodes of nchild<0

			V_NeuronSWC_unit node = mUnit[i];
			node.r = 0.5;
			node.n += nexist;
			if (node.parent >=1)  node.parent += nexist;
				else node.parent = -1;
					cur_seg.append(node);
		}

		QString tmpss;  tmpss.setNum(tNeuron.nsegs()+1);
		cur_seg.name = qPrintable(tmpss);
		cur_seg.b_linegraph=false; //don't forget to do this
		tNeuron.append(cur_seg);
		tNeuron.name = TRACED_NAME;
		tNeuron.file = TRACED_FILE;
	}

	else
	{
		for (V3DLONG ii=0;ii<mmUnit.size();ii++)
		{
			//V3DLONG nexist = tNeuron.nnodes();
			V3DLONG nexist = tNeuron.maxnoden();

			V_NeuronSWC cur_seg;	cur_seg.clear();
			vector<V_NeuronSWC_unit> & mUnit = mmUnit[ii];

			for (V3DLONG i=0;i<mUnit.size();i++)
			{
				if (mUnit[i].nchild<0) continue; //090610: here for saving the V3DLONG time of deleting nodes of nchild<0

				V_NeuronSWC_unit v;
				set_simple_path_unit (v, nexist, mUnit, i, (n_end_nodes==1)); // link_order determined by 1/N path

				cur_seg.append(v);
				//qDebug("%d ", cur_seg.nnodes());
			}

			QString tmpss;  tmpss.setNum(tNeuron.nsegs()+1);
			cur_seg.name = qPrintable(tmpss);
			cur_seg.b_linegraph=true; //don't forget to do this
			tNeuron.append(cur_seg);
			tNeuron.name = TRACED_NAME;
			tNeuron.file = TRACED_FILE;
		}
	}

	return mmUnit.size();
}

bool My4DImage::proj_trace_smooth_downsample_last_traced_neuron(CurveTracePara & trace_para, int seg_begin, int seg_end)
{
	v3d_msg("proj_trace_smooth_downsample_last_traced_neuron(). \n",0);
	CHECK_DATA_trace_deformablepath();

	V3DLONG nexist = 0; // re-create index number

	// (VneuronSWC_list tracedNeuron).(V_neuronSWC seg[]).(V_nueronSWC_unit row[])
	for(int iseg=0; iseg<tracedNeuron.seg.size(); iseg++)
	{
		if (iseg <seg_begin || iseg >seg_end) continue; //091023

		V_NeuronSWC & cur_seg = (tracedNeuron.seg[iseg]);
		printf("#seg=%d(%d)", iseg, cur_seg.row.size());

		vector<V_NeuronSWC_unit> & mUnit = cur_seg.row; // do in place
		{
			//------------------------------------------------------------
			vector<V_NeuronSWC_unit> mUnit_prior = mUnit; // a copy as prior

			//smooth_curve(mCoord, trace_para.sp_smoothing_win_sz);
			mUnit = downsample_curve(mUnit, trace_para.sp_downsample_step);

			//------------------------------------------------------------
			BDB_Minus_Prior_Parameter bdbp_para;
			bdbp_para.nloops   =trace_para.nloops;
			bdbp_para.f_smooth =trace_para.internal_force2_weight;
			bdbp_para.f_length =trace_para.internal_force_weight;
			bdbp_para.f_prior  = 0.2;
			int chano = trace_para.channo;

			point_bdb_minus_3d_localwinmass_prior(data4d_uint8[chano], getXDim(), getYDim(), getZDim(),
					mUnit, bdbp_para, true, // 090618 RZC: add constraint to fix 2 ends
					mUnit_prior);
			//-------------------------------------------------------------

		}
		printf(">>%d(%d) ", iseg, mUnit.size());

		reset_simple_path_index (nexist, mUnit);
		nexist += mUnit.size();
	}
	printf("\n");

	return true;
}

//refind the best shortest path using a small concatenated image region around the supplied curve
bool My4DImage::proj_trace_shortestpath_rgnaroundcurve(CurveTracePara & trace_para, int seg_begin, int seg_end)
{
//	v3d_msg("proj_trace_shortestpath_rgnaroundcurve(). \n",0);
//	CHECK_DATA_trace_deformablepath();
//
//	V3DLONG nexist = 0; // re-create index number
//
//	// (VneuronSWC_list tracedNeuron).(V_neuronSWC seg[]).(V_nueronSWC_unit row[])
//	for(int iseg=0; iseg<tracedNeuron.seg.size(); iseg++)
//	{
//		if (iseg <seg_begin || iseg >seg_end) continue; //091023
//
//		V_NeuronSWC & cur_seg = (tracedNeuron.seg[iseg]);
//		printf("#seg=%d(%d)", iseg, cur_seg.row.size());
//
//		vector<V_NeuronSWC_unit> & mUnit = cur_seg.row;
//		{
//			//------------------------------------------------------------
//			vector<V_NeuronSWC_unit> mUnit_prior = mUnit; // a copy as prior
//
//			mUnit = downsample_curve(mUnit, trace_para.sp_downsample_step);
//
//			//------------------------------------------------------------
//			int chano = trace_para.channo;
//			shortestpath_around_curve(data4d_uint8[chano], getXDim(), getYDim(), getZDim(),
//												  mUnit, trace_para,
//												  mUnit_prior);
//			//-------------------------------------------------------------
//
//		}
//		printf(">>%d(%d) ", iseg, mUnit.size());
//
//		reset_simple_path_index (nexist, mUnit);
//		nexist += mUnit.size();
//	}
//	printf("\n");
//
	return true;
}


bool My4DImage::proj_trace_compute_radius_of_last_traced_neuron(CurveTracePara & trace_para, int seg_begin, int seg_end, float myzthickness)
{
	v3d_msg("proj_trace_compute_radius_of_last_traced_neuron. \n", 0);
	CHECK_DATA_trace_deformablepath();

	int chano = trace_para.channo;
	int smoothing_win_sz = trace_para.sp_smoothing_win_sz;

	for(int iseg=0; iseg<tracedNeuron.seg.size(); iseg++)
	{
		if (iseg <seg_begin || iseg >seg_end) continue; //091023

		V_NeuronSWC & cur_seg = (tracedNeuron.seg[iseg]);
		printf("#seg=%d(%d) ", iseg, cur_seg.row.size());

		std::vector<V_NeuronSWC_unit> & mUnit = cur_seg.row; // do in place
		{

			fit_radius_and_position(data4d_uint8[chano], getXDim(), getYDim(), getZDim(),
						mUnit,
						false,       // 090619: do not move points because the deformable model has done it
						myzthickness, // 100404: add the zthickness
                                                V3dApplication::getMainWindow()->global_setting.b_3dcurve_width_from_xyonly); //100415

			smooth_radius(mUnit, smoothing_win_sz, false); // 090602, 090620

		}
	}
	printf("\n");

	return true;
}

#define ___trace_add_segment_default_type___
bool My4DImage::proj_trace_add_curve_segment(vector<XYZ> &mCoord, int chno, double default_type/*=3*/, double default_radius/*=1*/, double creatmode/*=0*/, double default_timestamp/*=0*/, double default_tfresindex/*=0*/)
{
    if (mCoord.size()<=0)  return false;

    //V3DLONG nexist = tracedNeuron.nnodes();
    V3DLONG nexist = tracedNeuron.maxnoden();

    V_NeuronSWC cur_seg;
    set_simple_path(cur_seg, nexist, mCoord, false, default_radius, default_type, creatmode, default_timestamp, default_tfresindex); //reverse link

    // Add timestamp LMG 10/10/2018
    // Get current timestamp
    time_t timer2;
    struct tm y2k = {0};
    double seconds;

    y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
    y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1; // seconds since January 1, 2000 in UTC

    time(&timer2);  /* get current time; same as: timer = time(NULL)  */

    seconds = difftime(timer2,timegm(&y2k)); //Timestamp LMG 27/9/2018
    qDebug("Timestamp at proj_trace_add_curve_segment (seconds since January 1, 2000 in UTC): %.0f", seconds);

    for (V3DLONG k=0;k<(V3DLONG)cur_seg.nrows();k++) if(cur_seg.row[k].timestamp == 0) cur_seg.row[k].timestamp = seconds;
    //qDebug("raw/cur_seg Timestamp: %.0f / %.0f", seconds, cur_seg.row[cur_seg.nrows()-1].timestamp);

    //LMG 13-12-2017 get current resolution and save in eswc
   // tf::PluginInterface resinterface;
    //int resindex = resinterface.getRes();
    //int allresnum = resinterface.getallRes();
//    resindex = int(pow(2,double(allresnum-resindex)));
//    if(resindex != 1) qDebug() << "Saving Tera-Fly resolution (downsampled" << resindex << "times) in eswc";
//    else qDebug() << "Saving Tera-Fly resolution (Full Resolution, index 1) in eswc";
//    for (V3DLONG k=0;k<(V3DLONG)cur_seg.nrows();k++) cur_seg.row[k].tfresindex = resindex;

    QString tmpss;  tmpss.setNum(tracedNeuron.nsegs()+1);
    cur_seg.name = qPrintable(tmpss);
    cur_seg.b_linegraph=true; //donot forget to do this
    tracedNeuron.append(cur_seg);
    tracedNeuron.name = TRACED_NAME;
    tracedNeuron.file = TRACED_FILE;

    //091115 add an automatic deform step
    CurveTracePara trace_para;
    {
        trace_para.channo = (chno<0)?0:chno; if (trace_para.channo>=getCDim()) trace_para.channo=getCDim()-1;
        trace_para.landmark_id_start = -1;
        trace_para.landmark_id_end = -1;
        trace_para.sp_num_end_nodes = 2;
        trace_para.nloops = 100; //100130 change from 200 to 100
        trace_para.b_deformcurve = true;
        trace_para.sp_smoothing_win_sz = 2;
    }

    V3DLONG cur_segid = tracedNeuron.nsegs()-1;

    if (chno >=0) //100115, 100130: for solveCurveViews.
    {
        if (V3dApplication::getMainWindow()->global_setting.b_3dcurve_autodeform)
            proj_trace_smooth_downsample_last_traced_neuron(trace_para, cur_segid, cur_segid);

        bool b_use_shortestpath_rgnaroundcurve=true; //100130
        if (b_use_shortestpath_rgnaroundcurve)
            proj_trace_shortestpath_rgnaroundcurve(trace_para, cur_segid, cur_segid);

        if (V3dApplication::getMainWindow()->global_setting.b_3dcurve_autowidth)
            proj_trace_compute_radius_of_last_traced_neuron(trace_para, cur_segid, cur_segid, trace_z_thickness);
    }
    //

    proj_trace_history_append();
    return true;
}

NeuronTree My4DImage::proj_trace_add_curve_segment_append_to_a_neuron(vector<XYZ> &mCoord, int chno, NeuronTree & neuronEdited, double default_type/*=3*/, double creatmode/*=0*/, double default_timestamp/*=0*/, double default_tfresindex/*=0*/)
{
    NeuronTree newNeuronEdited;
    if (mCoord.size()<=0)  return newNeuronEdited;

    V_NeuronSWC cur_seg;
    set_simple_path(cur_seg, 0, mCoord, false, default_type, creatmode, default_timestamp, default_tfresindex); //reverse link

    // Add timestamp LMG 26/10/2018
    // Get current timestamp
    time_t timer2;
    struct tm y2k = {0};
    double seconds;

    y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
    y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1; // seconds since January 1, 2000 in UTC

    time(&timer2);  /* get current time; same as: timer = time(NULL)  */

    seconds = difftime(timer2,timegm(&y2k)); //Timestamp LMG 26/10/2018
    qDebug("Timestamp at proj_trace_add_curve_segment (seconds since January 1, 2000 in UTC): %.0f", seconds);

    for (V3DLONG k=0;k<(V3DLONG)cur_seg.nrows();k++) if(cur_seg.row[k].timestamp == 0) cur_seg.row[k].timestamp = seconds;

//    //LMG 13-12-2017 get current resolution and save in eswc
//    tf::PluginInterface resinterface;
//    int resindex = resinterface.getRes();
//    int allresnum = resinterface.getallRes();
//    resindex = int(pow(2,double(allresnum-resindex)));
//    if(resindex != 1) qDebug() << "Saving Tera-Fly resolution (downsampled" << resindex << "times) in eswc";
//    else qDebug() << "Saving Tera-Fly resolution (Full Resolution, index 1) in eswc";
//    for (V3DLONG k=0;k<(V3DLONG)cur_seg.nrows();k++) cur_seg.row[k].tfresindex = resindex;

    QString tmpss;  tmpss.setNum(tracedNeuron.nsegs()+1);
    cur_seg.name = qPrintable(tmpss);
    cur_seg.b_linegraph=true; //donot forget to do this

    V_NeuronSWC_list tmpTracedNeuron = NeuronTree__2__V_NeuronSWC_list(neuronEdited);

    tmpTracedNeuron.append(cur_seg);
    tmpTracedNeuron.name = qPrintable(neuronEdited.name);
    tmpTracedNeuron.file = qPrintable(neuronEdited.file);

    //091115 add an automatic deform step
    CurveTracePara trace_para;
    {
        trace_para.channo = (chno<0)?0:chno; if (trace_para.channo>=getCDim()) trace_para.channo=getCDim()-1;
        trace_para.landmark_id_start = -1;
        trace_para.landmark_id_end = -1;
        trace_para.sp_num_end_nodes = 2;
        trace_para.nloops = 100; //100130 change from 200 to 100
        trace_para.b_deformcurve = true;
        trace_para.sp_smoothing_win_sz = 2;
    }

    V3DLONG cur_segid = tmpTracedNeuron.nsegs()-1;

    if (chno >=0) //100115, 100130: for solveCurveViews.
    {
        if (V3dApplication::getMainWindow()->global_setting.b_3dcurve_autodeform)
            proj_trace_smooth_downsample_last_traced_neuron(trace_para, cur_segid, cur_segid);

        bool b_use_shortestpath_rgnaroundcurve=true; //100130
        if (b_use_shortestpath_rgnaroundcurve)
            proj_trace_shortestpath_rgnaroundcurve(trace_para, cur_segid, cur_segid);

        if (V3dApplication::getMainWindow()->global_setting.b_3dcurve_autowidth)
            proj_trace_compute_radius_of_last_traced_neuron(trace_para, cur_segid, cur_segid, trace_z_thickness);
    }
    //

    //proj_trace_history_append();

    QStringList tmpstlst;
    writeSWC_file(QString("ttt1.swc"), neuronEdited, &tmpstlst);
    newNeuronEdited = V_NeuronSWC_list__2__NeuronTree(tmpTracedNeuron);
    writeSWC_file("ttt2.swc", neuronEdited, &tmpstlst);

    return newNeuronEdited;
}

#define ___trace_history_append___
void My4DImage::proj_trace_history_append()
{
    proj_trace_history_append(tracedNeuron);

    // @ADDED by Alessandro on 2015-10-01 to integrate undo/redo on both markers and neurons.
    // this is SAFE: it only informs TeraFly (SAFE) that a neuron has been edited.
    tf::TeraFly::doaction("neuron edit");

////#ifdef __ALLOW_VR_FUNCS__
////    //20170803 RZC
////    mozak::MozakUI::onImageTraceHistoryChanged();
////#endif
    
//    emit signal_trace_history_append();      //20170801 RZC: not convenient for other widgets except xform widget
//	SEND_EVENT(qApp, QEvent_HistoryChanged); //20170801 RZC: notify by qApp event filter
}

void My4DImage::proj_trace_history_append(V_NeuronSWC_list & tNeuron)
{

    if (tracedNeuron.seg.size()<=0) return;
	//null seg is also a undo/redo status

	// remove from cur_history+1

    for (int i=cur_history+1; i>=0 && i<tracedNeuron_historylist.size();i++) {
        qDebug()<<tracedNeuron_historylist.size();

        tracedNeuron_historylist.removeAt(i);
    }

	// make size <= MAX_history

	while (tracedNeuron_historylist.size()>=MAX_history) tracedNeuron_historylist.pop_front();

	tracedNeuron_historylist.push_back(tNeuron);
	cur_history = tracedNeuron_historylist.size()-1;


}

void My4DImage::proj_trace_history_undo()
{
	proj_trace_history_undo(tracedNeuron);
}

void My4DImage::proj_trace_history_undo(V_NeuronSWC_list & tNeuron)
{
//	qDebug()<<"***************************************************************";
//	qDebug()<<"UNDO  historylist last ="<<tracedNeuron_historylist.size()-1<<"  cur_history ="<<cur_history;

    cur_history--;
	if (tracedNeuron_historylist.size()<1 ||  //090924 RZC: fixed from <2 to <1
		cur_history < -1)
	{
		cur_history = -1;
        v3d_msg("Has reached the earliest of saved history!");
	}
	else if (cur_history == -1)  //20170803 RZC: make no msgbox for terafly undo
    {
        if (tNeuron.b_traced) tNeuron.seg.clear();
	}
	else if (cur_history>=0 && cur_history<tracedNeuron_historylist.size())
	{
		tNeuron = tracedNeuron_historylist.at(cur_history);
    }

//	qDebug()<<"      historylist last ="<<tracedNeuron_historylist.size()-1<<"  cur_history ="<<cur_history;
//	qDebug()<<"***************************************************************";
}

void My4DImage::proj_trace_history_redo()
{
	proj_trace_history_redo(tracedNeuron);
}

void My4DImage::proj_trace_history_redo(V_NeuronSWC_list & tNeuron)
{
//	qDebug()<<"***************************************************************";
//	qDebug()<<"REDO  historylist last ="<<tracedNeuron_historylist.size()-1<<"  cur_history ="<<cur_history;

	cur_history++;
	if (tracedNeuron_historylist.size()<1 ||   //090924: fixed from <2 to <1
		cur_history > tracedNeuron_historylist.size()-1)
	{
		cur_history = tracedNeuron_historylist.size()-1;
		v3d_msg("Has reach the latest of saved history!");
	}
	else if (cur_history>=0 && cur_history<=tracedNeuron_historylist.size()-1)
	{
		tNeuron = tracedNeuron_historylist.at(cur_history);
	}

//	qDebug()<<"      historylist last ="<<tracedNeuron_historylist.size()-1<<"  cur_history ="<<cur_history;
//	qDebug()<<"***************************************************************";
}

//V3DLONG findTracedNeuronSeg(V_NeuronSWC_list & tracedNeuron, NeuronSWC & node)
//{
//	double x = node.x;
//	double y = node.y;
//	double z = node.z;
//	V3DLONG seg_id = find_seg_num_in_V_NeuronSWC_list(tracedNeuron, x,y,z);     //use (x,y,z) as keyword
//	if (seg_id<0) return -1;
//	return seg_id; //by PHC, 090209. return the index in the SWC file
//}

#define  CHECK_segment_id(seg_id, node_id, p_tree) \
	if (node_id<0 || node_id>=tracedNeuron.nrows()) return false; \
	V3DLONG seg_id = p_tree->listNeuron.at(node_id).seg_id;  \
	if (seg_id<0) { printf("can not find seg_id of node (%d)\n", node_id); return false;}


bool My4DImage::proj_trace_changeNeuronSegType(V3DLONG node_id, NeuronTree *p_tree)
{
	CHECK_segment_id(seg_id, node_id, p_tree);

	bool res;
	bool ok;
	int node_type = p_tree->listNeuron.at(node_id).type;

#if defined(USE_Qt5)
	node_type = QInputDialog::getInt(0, QObject::tr("Change node type in segment"),
							  QObject::tr("SWC type: "
										"\n 0 -- undefined (white)"
										"\n 1 -- soma (black)"
										"\n 2 -- axon (red)"
										"\n 3 -- dendrite (blue)"
										"\n 4 -- apical dendrite (purple)"
										"\n else -- custom \n"),
									  node_type, 0, 100, 1, &ok);
#else
    node_type = QInputDialog::getInt(0, QObject::tr("Change node type in segment"),
							  QObject::tr("SWC type: "
										"\n 0 -- undefined (white)"
										"\n 1 -- soma (black)"
										"\n 2 -- axon (red)"
										"\n 3 -- dendrite (blue)"
										"\n 4 -- apical dendrite (purple)"
										"\n else -- custom \n"),
									  node_type, 0, 100, 1, &ok);
#endif
	if (ok)
	{
		res = change_type_in_seg_of_V_NeuronSWC_list(tracedNeuron, seg_id, node_type);
	}
	else res = false;

	if (res)  proj_trace_history_append();
	return res;
}

bool My4DImage::proj_trace_changeNeuronSegRadius(V3DLONG node_id, NeuronTree *p_tree)
{
	CHECK_segment_id(seg_id, node_id, p_tree);

	QString qtitle = QObject::tr("Change node radius in segment");
	bool res;
	bool ok;
	double node_radius = p_tree->listNeuron.at(node_id).r;
	node_radius = QInputDialog::getDouble(0, qtitle,
							  QObject::tr("SWC radius: "
									    "\n >0 -- set the radius of all nodes of this segment to be this target value)"
										"\n  0 -- auto-estimate radius of this segment based on image content)"
										"\n -1 -- auto-estimate radius of all segments based on image content)\n"),
									  node_radius, -1, 10000, 1, &ok);
	if (ok)
	{
		if (node_radius==0)
		{
			int channo = 1;
			if (this->getCDim()!=1) //only ask channel no if it is not 1
			{

#if defined(USE_Qt5)
				channo = QInputDialog::getInt(0, qtitle,
					  QObject::tr("image data channel: "), 1, 1, this->getCDim(), 1, &ok);
#else
                channo = QInputDialog::getInt(0, qtitle,
					  QObject::tr("image data channel: "), 1, 1, this->getCDim(), 1, &ok);
#endif
				if (! ok)  return false;
			}

#if defined(USE_Qt5)
			int win_sz = QInputDialog::getInt(0, qtitle,
					  QObject::tr("radius smoothing window size: "), 5, 1, 20, 1, &ok);
#else
            int win_sz = QInputDialog::getInt(0, qtitle,
					  QObject::tr("radius smoothing window size: "), 5, 1, 20, 1, &ok);
#endif
			if (! ok)  return false;

			CurveTracePara trace_para;
			trace_para.channo = channo-1;
			trace_para.sp_smoothing_win_sz = win_sz;
			res = proj_trace_compute_radius_of_last_traced_neuron(trace_para, seg_id, seg_id, trace_z_thickness);
		}
		else if (node_radius==-1)
		{
			int channo = 1;
			if (this->getCDim()!=1) //only ask channel no if it is not 1
			{

#if defined(USE_Qt5)
				channo = QInputDialog::getInt(0, qtitle,
												  QObject::tr("image data channel: "), 1, 1, this->getCDim(), 1, &ok);
#else
                channo = QInputDialog::getInt(0, qtitle,
												  QObject::tr("image data channel: "), 1, 1, this->getCDim(), 1, &ok);
#endif
				if (! ok)  return false;
			}

#if defined(USE_Qt5)
			int win_sz = QInputDialog::getInt(0, qtitle,
												  QObject::tr("radius smoothing window size: "), 5, 1, 20, 1, &ok);
#else
            int win_sz = QInputDialog::getInt(0, qtitle,
												  QObject::tr("radius smoothing window size: "), 5, 1, 20, 1, &ok);
#endif
			if (! ok)  return false;

			CurveTracePara trace_para;
			trace_para.channo = channo-1;
			trace_para.sp_smoothing_win_sz = win_sz;
			for (V3DLONG i=0;i<tracedNeuron.seg.size();i++)
			{
				res = proj_trace_compute_radius_of_last_traced_neuron(trace_para, i, i, trace_z_thickness);
				if (!res)
				{
					v3d_msg(QString("Error happened in changing the radius of the %1 segment of the current neuron. Stop.\n").arg(i+1));
					break;
				}
			}
		}
		else if (node_radius>0)
		{
			res = change_radius_in_seg_of_V_NeuronSWC_list(tracedNeuron, seg_id, node_radius);
		}
		else
		{
			v3d_msg("Invalid parameter set for the change radius function. Ignore it.\n");
		}
	}
	else res = false;

	if (res)  proj_trace_history_append();
	return res;
}

bool My4DImage::proj_trace_reverseNeuronSeg(V3DLONG node_id, NeuronTree *p_tree)
{
	CHECK_segment_id(seg_id, node_id, p_tree);

	qDebug("Reverse link order in segment");

	bool res;

	if (tracedNeuron.nsegs()==1)
	{
		res = tracedNeuron.reverse();
	}
	else
	{
		res = tracedNeuron.seg[seg_id].reverse();
	}

	if (res)  proj_trace_history_append();
	return res;
}


bool My4DImage::proj_trace_deleteNeuronSeg(V3DLONG node_id, NeuronTree *p_tree)
{
	CHECK_segment_id(seg_id, node_id, p_tree);

	bool res;

	//res = delete_seg_in_V_NeuronSWC_list(tracedNeuron, seg_id); //commented by PHC, 2010-02-10. use the following simpler interface, just for simplicity
	res = tracedNeuron.deleteSeg(seg_id);

	if (res)  proj_trace_history_append();
	return res;
}

bool My4DImage::proj_trace_breakNeuronSeg(V3DLONG node_id, NeuronTree *p_tree)
{
	CHECK_segment_id(seg_id, node_id, p_tree);
	V3DLONG nodeinseg_id = p_tree->listNeuron.at(node_id).nodeinseg_id;

	bool res;

	res = tracedNeuron.split(seg_id, nodeinseg_id);

	if (res)  proj_trace_history_append();
	return res;
}

bool My4DImage::proj_trace_joinNearbyNeuronSegs_pathclick(V3DLONG node_id, NeuronTree *p_tree)
{
//	V3DLONG n_segs = tracedNeuron.nsegs();
//	if (n_segs<=1) return true; //do nothing is there is only less than two segs
//
//	V3DLONG *seg_id_array = 0;
//	try{ seg_id_array = new V3DLONG [n_segs];} catch (...) {qDebug()<<"fail to allocacte memory in proj_trace_joinAllNeuronSegs()";return false;}
//	for (V3DLONG i=0;i<n_segs;i++) seg_id_array[i]=i;
//
//	V_NeuronSWC m_neuron = join_segs_in_V_NeuronSWC_list(tracedNeuron, seg_id_array, n_segs);
//	tracedNeuron.seg.clear();
//	tracedNeuron.append(m_neuron);
//
//	if (seg_id_array) {delete []seg_id_array; seg_id_array=0;}
	return true;
}

bool My4DImage::proj_trace_joinNearbyNeuronSegs_markclick(V3DLONG node_id, NeuronTree *p_tree)
{
//	V3DLONG n_segs = tracedNeuron.nsegs();
//	if (n_segs<=1) return true; //do nothing is there is only less than two segs
//
//	V3DLONG *seg_id_array = 0;
//	try{ seg_id_array = new V3DLONG [n_segs];} catch (...) {qDebug()<<"fail to allocacte memory in proj_trace_joinAllNeuronSegs()";return false;}
//	for (V3DLONG i=0;i<n_segs;i++) seg_id_array[i]=i;
//
//	V_NeuronSWC m_neuron = join_segs_in_V_NeuronSWC_list(tracedNeuron, seg_id_array, n_segs);
//	tracedNeuron.seg.clear();
//	tracedNeuron.append(m_neuron);
//
//	if (seg_id_array) {delete []seg_id_array; seg_id_array=0;}
	return true;
}

bool My4DImage::proj_trace_joinAllNeuronSegs(V3DLONG node_id, NeuronTree *p_tree) //need to add an arbitrary node root function. 090208.
{
	V3DLONG n_segs = tracedNeuron.nsegs();
	if (n_segs<=1) return true; //do nothing if there is only less than two segs

	V3DLONG *seg_id_array = 0;
	try{ seg_id_array = new V3DLONG [n_segs];}
	catch (...) {v3d_msg("fail to allocate memory in proj_trace_joinAllNeuronSegs()");return false;}
	for (V3DLONG i=0;i<n_segs;i++) seg_id_array[i]=i;

	V_NeuronSWC m_neuron = join_segs_in_V_NeuronSWC_list(tracedNeuron, seg_id_array, n_segs);
	tracedNeuron.seg.clear();
	tracedNeuron.append(m_neuron);

	if (seg_id_array) {delete []seg_id_array; seg_id_array=0;}

	if (true)  proj_trace_history_append();
	return true;
}

double My4DImage::proj_trace_measureLengthNeuronSeg(V3DLONG node_id, NeuronTree *p_tree)
{
	CHECK_segment_id(seg_id, node_id, p_tree);

	return length_seg_in_V_NeuronSWC_list(tracedNeuron, seg_id);
}

bool My4DImage::proj_trace_deformNeuronSeg(V3DLONG node_id, NeuronTree *p_tree, bool b_select_para)
{
	CHECK_segment_id(seg_id, node_id, p_tree);
	V3DLONG nodeinseg_id = p_tree->listNeuron.at(node_id).nodeinseg_id;

	bool res=true;

	CurveTracePara trace_para;
	{
		trace_para.landmark_id_start = -1;
		trace_para.landmark_id_end = -1;
		trace_para.sp_num_end_nodes = 2;
		trace_para.nloops = 200;
		trace_para.b_deformcurve = true;
		trace_para.sp_downsample_step = 1;
		trace_para.sp_smoothing_win_sz = 2;

		if (b_select_para)
		{
			CurveTraceParaDialog d(this->getCDim(), this->listLandmarks.size(), trace_para);
			d.setHideState_spinBox_landmark_start(true); //hide the start position
			d.setHideState_spinBox_landmark_end(true); //hide the end position
			d.setEnabledState_checkBox_b_curve(false); //disable the shortest path curve checkbox
			int res=d.exec();
			if (res==QDialog::Accepted)
			{
				d.fetchData(&trace_para);
			}
			else
			{
				qDebug("You cancel the tracing. ");
				return false;
			}
		}
	}

	qDebug() << "nodeinseg_id=" << seg_id;
	proj_trace_smooth_downsample_last_traced_neuron(trace_para, seg_id, seg_id);

	return res;
}

bool My4DImage::proj_trace_profileNeuronSeg(V3DLONG node_id, NeuronTree *p_tree, bool b_dispfig)
{
	if (getDatatype()!=V3D_UINT8)
	{
		v3d_msg("Invalid data type (only uint8 is supported now) in proj_trace_profileNeuronSeg()");
		return false;
	}

	CHECK_segment_id(seg_id, node_id, p_tree);
	V3DLONG nodeinseg_id = p_tree->listNeuron.at(node_id).nodeinseg_id;

	bool res=true;
	double rr=10;

	if (seg_id<0 && seg_id>=tracedNeuron.seg.size())
		return false;

	//for the puctuation distribution experiments
	bool b_do_all_seg=false; //true;
	int s_beg, s_end, seg_id0=seg_id;
	if (b_do_all_seg) {s_beg=0; s_end=tracedNeuron.seg.size()-1;}
	else {s_beg=s_end=seg_id;}

	for (seg_id=s_beg;seg_id<=s_end;seg_id++)
	{
		V_NeuronSWC & subject_swc = tracedNeuron.seg.at(seg_id);
		map <V3DLONG, V3DLONG> sub_index_map = unique_V_NeuronSWC_nodeindex(subject_swc);
		QVector< QVector<int> > vvec1, vvec2;
		QStringList labelsLT1, labelsLT2;
		int length = subject_swc.nrows();
		for (int k=0; k<getCDim(); k++)
		{
			QVector<int> prof_total;
			QVector<int> prof_ave;
			QVector<double> prof_len;
			V3DLONG i;
			for (i=0;i<subject_swc.nrows();i++)
			{
				int cx = int(subject_swc.row.at(i).data[2]+0.5);
				int cy = int(subject_swc.row.at(i).data[3]+0.5);
				int cz = int(subject_swc.row.at(i).data[4]+0.5);

				rr = subject_swc.row.at(i).data[5];

				if (cx<0 || cx>=this->getXDim() || cy<0 || cy>=this->getYDim() || cz<0 || cz>=this->getZDim())
				{
					v3d_msg(QString("coordinate of node %1 is not within the image rgn. Skip it.").arg(i), 0);
					prof_total << int(-1); //in this case give it an invalid value
					prof_ave << int(-1); //in this case give it an invalid value
				}
				else
				{
					//prof << int(data4d_uint8[k][cz][cy][cx]); //getLocalInfo(cx,cy,cz, channo, rr);
					double cur_len=0; //default set it as 0 (also for the root node case)
					if (subject_swc.row.at(i).data[6]>=0)
					{
						V3DLONG parent_row = sub_index_map[subject_swc.row.at(i).data[6]];
						double dx = subject_swc.row.at(i).data[2] - subject_swc.row.at(parent_row).data[2];
						double dy = subject_swc.row.at(i).data[3] - subject_swc.row.at(parent_row).data[3];
						double dz = subject_swc.row.at(i).data[4] - subject_swc.row.at(parent_row).data[4];
						cur_len = sqrt(dx*dx+dy*dy+dz*dz);
					}

					double v = 0;
					int xs=qBound(0, int(cx-rr), int(this->getXDim()-1)),
						xe=qBound(0, int(cx+rr), int(this->getXDim()-1)),
						ys=qBound(0, int(cy-rr), int(this->getYDim()-1)),
						ye=qBound(0, int(cy+rr), int(this->getYDim()-1)),
						zs=qBound(0, int(cz-rr), int(this->getZDim()-1)),
						ze=qBound(0, int(cz+rr), int(this->getZDim()-1));
					double d=rr*rr, tt;
					V3DLONG n=0;
					for (int kt=zs;kt<=ze;kt++)
					{
						tt = double(cz)-kt;
						double d1 = tt*tt;
						for (int jt=ys;jt<=ye;jt++)
						{
							tt = double(cy)-jt;
							double d2 = d1 + tt*tt;
							if (d2>rr)
								continue;
							for (int it=xs;it<=xe;it++)
							{
								tt = double(cx)-it;
								double d3 = d2+tt*tt;
								if (d3>rr)
									continue;
								v += data4d_uint8[k][cz][cy][cx];
								n++;
							}
						}
					}

					if (n<=0)
					{
						prof_total << int(-1);
						prof_ave << int(-1);
						prof_len << double(-1);
					}
					else
					{
						prof_total << int(v);
						prof_ave << int(v/n);
						prof_len << cur_len;
					}
				}
			}

			vvec1.append(prof_total);
			labelsLT1.append( QString("channel %1 . %2").arg(k+1).arg("sum") );
			FILE *fp=fopen(qPrintable(QString("%1.sum.%2").arg(p_tree->file).arg(seg_id)), "wt");
			for (i=0;i<prof_total.size();i++)
				fprintf(fp, "%d\n",prof_total.at(i));
			fclose(fp);

			vvec2.append(prof_ave);
			labelsLT2.append( QString("channel %1 . %2").arg(k+1).arg("ave") );
			fp=fopen(qPrintable(QString("%1.ave.%2").arg(p_tree->file).arg(seg_id)), "wt");
			for (i=0;i<prof_ave.size();i++)
				fprintf(fp, "%d\n",prof_ave.at(i));
			fclose(fp);

			fp=fopen(qPrintable(QString("%1.length.%2").arg(p_tree->file).arg(seg_id)), "wt");
			for (i=0;i<prof_len.size();i++)
				fprintf(fp, "%7.6f\n",prof_len.at(i));
			fclose(fp);
		}
		QString labelRB = QString("# nodes/length=%1").arg(length);

		if (b_do_all_seg && seg_id!=seg_id0)
			continue; //in this case only display the pinpointed seg in figure

//		if (b_dispfig)
//		{
//			barFigureDialog *dlg1 = new barFigureDialog(vvec1, labelsLT1, labelRB, 0, QSize(500, 150), QColor(50,50,50));
//			dlg1->setWindowTitle(QObject::tr("Profile (neuron #%1 : seg #%2) Total Mass").arg(p_tree->file).arg(seg_id));
//			dlg1->show();

//			barFigureDialog *dlg2 = new barFigureDialog(vvec2, labelsLT2, labelRB, 0, QSize(500, 150), QColor(50,50,50));
//			dlg2->setWindowTitle(QObject::tr("Profile (neuron #%1 : seg #%2) Mean Intensity").arg(p_tree->file).arg(seg_id));
//			dlg2->show();
//		}
	}
	return res;
}


bool My4DImage::proj_trace_mergeOneClosebyNeuronSeg(V3DLONG node_id, NeuronTree *p_tree) //this function should be deleted later as the method is obsolete (and still not perfect), noted by PHC 100404
{
//	if (getDatatype()!=V3D_UINT8)
//	{
//		v3d_msg("Invalid data type (only uint8 is supported now) in proj_trace_mergeOneCloseNeuronSeg()");
//		return false;
//	}

	CHECK_segment_id(seg_id, node_id, p_tree);
	V3DLONG nodeinseg_id = p_tree->listNeuron.at(node_id).nodeinseg_id;

	bool res=true;
	double rr=10;

	int NSegs = tracedNeuron.seg.size();
	if (seg_id<0 && seg_id>=NSegs)
	{
		v3d_msg("seg id is not correct. return w/o doing anything.",0);
		return false;
	}
	if (NSegs==1)
	{
		v3d_msg("Only one segment. Do nothing.",0);
		return false;
	}

	//first find the closeby path
	V_NeuronSWC & subject_swc = tracedNeuron.seg.at(seg_id);
	if (!subject_swc.isLineGraph())
	{
		v3d_msg("The current selected neuron segment is not a simple line graph. You should first select Edit to turn it into simpler pieces.");
		return false;
	}
	map <V3DLONG, V3DLONG> subject_index_map = unique_V_NeuronSWC_nodeindex(subject_swc);
	int slength = subject_swc.nrows();
	if (slength<=0)
		return false; //this should never happen

	int *sMergeT_segInd = new int [slength];
	int *sMergeT_nodeInd = new int [slength];
	double *sMergeT_mindist = new double [slength];
	V3DLONG j;
	for (j=0; j<slength; j++)
	{
		sMergeT_segInd[j] = -1; //-1 means not merge
		sMergeT_nodeInd[j] = -1; //-1 means not merge
		sMergeT_mindist[j] = -1;
	}

	int cur_sid;

	//first check if both ends are overlapping with other segments' tips, if so, then skip this "continuous seg" and quit w/o doing anything
	bool b_headoverlap=false, b_tailoverlap=false;
	double scx = subject_swc.row.at(0).data[2];
	double scy = subject_swc.row.at(0).data[3];
	double scz = subject_swc.row.at(0).data[4];
	for (cur_sid=0;cur_sid<NSegs;cur_sid++)
	{
		if (cur_sid==seg_id)
			continue;

		V_NeuronSWC & target_swc = tracedNeuron.seg.at(cur_sid);
		if (find_node_in_V_NeuronSWC(target_swc, scx, scy, scz) >= 0)
		{
			b_headoverlap = true;
			break;
		}
	}
	scx = subject_swc.row.at(slength-1).data[2];
	scy = subject_swc.row.at(slength-1).data[3];
	scz = subject_swc.row.at(slength-1).data[4];
	for (cur_sid=0;cur_sid<NSegs;cur_sid++)
	{
		if (cur_sid==seg_id)
			continue;

		V_NeuronSWC & target_swc = tracedNeuron.seg.at(cur_sid);
		int tlength = target_swc.nrows();

		if (find_node_in_V_NeuronSWC(target_swc, scx, scy, scz) >= 0)
		{
			b_tailoverlap = true;
			break;
		}
	}
	if (b_tailoverlap && b_headoverlap) //skip the continuous segments
	{
		v3d_msg("Both the head and tail tips of this segment overlap with other segments. You should choose a different segment to merge.");
		if (sMergeT_segInd) {delete []sMergeT_segInd; sMergeT_segInd=0;}
		if (sMergeT_nodeInd) {delete []sMergeT_nodeInd; sMergeT_nodeInd=0;}
		if (sMergeT_mindist) {delete []sMergeT_mindist; sMergeT_mindist=0;}
		return false;
	}


	//the real search the diverging path
	for (cur_sid=0;cur_sid<NSegs;cur_sid++)
	{
		if (cur_sid==seg_id)
			continue;

		V_NeuronSWC & target_swc = tracedNeuron.seg.at(cur_sid);
		map <V3DLONG, V3DLONG> target_index_map = unique_V_NeuronSWC_nodeindex(target_swc);
		int tlength = target_swc.nrows();

		for (V3DLONG j=0; j<slength; j++)
		{
			scx = subject_swc.row.at(j).data[2];
			scy = subject_swc.row.at(j).data[3];
			scz = subject_swc.row.at(j).data[4];
			double sqr2 = subject_swc.row.at(j).data[5];
			sqr2 *= sqr2; //squared radius
			for (V3DLONG i=0;i<tlength;i++)
			{
				double tcr2 = target_swc.row.at(i).data[5]; tcr2 *= tcr2; //squared radius

				double tt = (sqr2>tcr2)?sqr2:tcr2;
				tt=tt/2; //devide by 2 so that the transition will be smoother

				double dtcx = target_swc.row.at(i).data[2] - scx; dtcx *= dtcx;
				if (dtcx>tt)
					continue;

				double dtcy = target_swc.row.at(i).data[3] - scy; dtcy *= dtcy;
				if (dtcy>tt || dtcy+dtcx>tt)
					continue;

				double dtcz = target_swc.row.at(i).data[4] - scz; dtcz *= dtcz;
				if (dtcz>tt || dtcz+dtcy+dtcx>tt)
					continue;

				//keep the best one among all segments and all nodes
				if (sMergeT_segInd[j]<0) //unset yet
				{
					sMergeT_segInd[j] = cur_sid;
					sMergeT_nodeInd[j] = i;
					sMergeT_mindist[j] = dtcz+dtcy+dtcx;
				}
				else
				{
					if (sMergeT_mindist[j]>dtcz+dtcy+dtcx)
					{
						sMergeT_segInd[j] = cur_sid;
						sMergeT_nodeInd[j] = i;
						sMergeT_mindist[j] = dtcz+dtcy+dtcx;
					}
				}
				v3d_msg(QString("merge subject swc node %1 to target seg %2 node %3.").arg(j).arg(cur_sid).arg(i), 0);

				break;
			}
		}
	}

	//detect the parts that need merging
	vector <V_NeuronSWC> splitSegs;

	if (slength==1) //cannot be <1, or it should exit already
	{
		if (sMergeT_segInd[0]>=0) //then just delete this segment, or do nothing
		{
			res = tracedNeuron.deleteSeg(seg_id);
			if (res)  proj_trace_history_append();
		}
	}
	else
	{
		QVector <int> mnodes;
		QVector <int> mnodes_direction;
		for (j=0;j<slength-1;j++)
		{
			if (sMergeT_segInd[j]>=0)
			{
				if (sMergeT_segInd[j+1]<0) {mnodes << (j+1); mnodes_direction << 1;} //direction = 1 means keep behind, remove before
			}
			else
			{
				if (sMergeT_segInd[j+1]>0) {mnodes << (j); mnodes_direction << 2; } //direction = 2 means keep before, remove behind
			}
		}

		for (V3DLONG tk=0;tk<mnodes.size();tk++) //chop to mnodes.size()+1 segments
		{
			if (mnodes_direction.at(tk)==1)
			{
				printf("push enter res_swc2 [%d] direction=1\n", splitSegs.size());
				V_NeuronSWC  res_swc1;
				V3DLONG posend = slength-1; if (tk<mnodes.size()-1) posend = mnodes.at(tk+1);
				for (j=mnodes.at(tk);j<=posend;j++)	res_swc1.append(subject_swc.row.at(j)); //note to include the breaking point
				if (subject_swc.row[0].parent<0)
					res_swc1.row[0].parent = -1;
				res_swc1.b_linegraph=true; //this must be set

				splitSegs.push_back(res_swc1);

				//create a new segmention of two nodes, to connect to the main trunk
				V_NeuronSWC res_swc2;
				V_NeuronSWC_unit u = tracedNeuron.seg.at(sMergeT_segInd[mnodes.at(tk)-1]).row.at(sMergeT_nodeInd[mnodes.at(tk)-1]);
				res_swc2.append(u);
				res_swc2.append(subject_swc.row.at(mnodes.at(tk)));
				res_swc2.row[0].data[0] = 0; res_swc2.row[0].parent = -1;
				res_swc2.row[1].data[0] = 1; res_swc2.row[1].parent = 0;
				res_swc2.b_linegraph=true;
				splitSegs.push_back(res_swc2);
				printf("push res_swc2 [%d] direction=1\n", splitSegs.size());
			}
			else
			{
				printf("push enter res_swc2 [%d] direction=2\n", splitSegs.size());
				V_NeuronSWC  res_swc1;
				V3DLONG posbeg = 0; if (tk>=1) posbeg = mnodes.at(tk-1);
				for (j=posbeg;j<=mnodes.at(tk);j++)	res_swc1.append(subject_swc.row.at(j)); //note to include the breaking point
				if (subject_swc.row[0].parent>0) //then the other end must <0
					res_swc1.row[j-1].parent = -1;
				res_swc1.b_linegraph=true; //this must be set

				splitSegs.push_back(res_swc1);

				//create a new segmention of two nodes, to connect to the main trunk
				V_NeuronSWC res_swc2;
				V_NeuronSWC_unit u = tracedNeuron.seg.at(sMergeT_segInd[mnodes.at(tk)+1]).row.at(sMergeT_nodeInd[mnodes.at(tk)+1]);

				res_swc2.append(u);
				res_swc2.append(subject_swc.row.at(mnodes.at(tk)));
				res_swc2.row[0].data[0] = 0; res_swc2.row[0].parent = -1;
				res_swc2.row[1].data[0] = 1; res_swc2.row[1].parent = 0;
				res_swc2.b_linegraph=true;
				splitSegs.push_back(res_swc2);
				v3d_msg(QString("push res_swc2 [%1] direction=2").arg(splitSegs.size()),0);
			}
		}

		if (mnodes.size()==1)
		{
			tracedNeuron.append(splitSegs);
			res = tracedNeuron.deleteSeg(seg_id);
			if (res)  proj_trace_history_append();
		}
	}

	//free space
	if (sMergeT_segInd) {delete []sMergeT_segInd; sMergeT_segInd=0;}
	if (sMergeT_nodeInd) {delete []sMergeT_nodeInd; sMergeT_nodeInd=0;}
	if (sMergeT_mindist) {delete []sMergeT_mindist; sMergeT_mindist=0;}
	return res;
}


bool My4DImage::proj_trace_mergeAllClosebyNeuronNodes(NeuronTree *p_tree)
{
	return proj_trace_mergeAllClosebyNeuronNodes();
}

bool My4DImage::proj_trace_mergeAllClosebyNeuronNodes()  //bug inside this function? tentatively disable this in the render_hit as of 20140630.
{
	//this function will merge closeby nodes in different segments.
	bool res=true;

	V3DLONG seg_id, cur_sid;
	V3DLONG i,j;

	int NSegs = tracedNeuron.seg.size();
	if (NSegs==1) //do nothing when there is only one segment
	{
		v3d_msg("Only one segment. Do nothing.",0);
		return false;
	}

	//if there are multiple segs, then ensure each one is a line graph
	for (seg_id=0; seg_id<NSegs;seg_id++)
	{
		V_NeuronSWC & subject_swc = tracedNeuron.seg.at(seg_id);
		if (!subject_swc.isLineGraph())
		{
			v3d_msg(QString("The %1 neuron segment is not a simple line graph. You should first select Edit to turn it into simpler pieces.").arg(seg_id));
			return false;
		}
	}

	//first	determine the termini nodes which should be preserved
	vector <V_NeuronSWC_unit> terminiNodePool;
	vector <V_NeuronSWC_unit> realRootNodePool;
	for (seg_id=0; seg_id<NSegs;seg_id++)
	{
		//for the first node of the segment
		V_NeuronSWC & subject_swc = tracedNeuron.seg.at(seg_id);
		int slength = subject_swc.nrows();

		bool b_termini;
		for (cur_sid=0, b_termini=true;cur_sid<NSegs;cur_sid++)
		{
			if (cur_sid==seg_id)
				continue;

			V_NeuronSWC & target_swc = tracedNeuron.seg.at(cur_sid);
			int tlength = target_swc.nrows();

			for (i=0;i<target_swc.row.size();i++)
			{
				if (subject_swc.row.at(0).get_coord() == target_swc.row.at(i).get_coord())
				{
					b_termini=false;
					break;
				}
			}
			if (!b_termini)
				break;
		}

		if (b_termini)
		{
			terminiNodePool.push_back(subject_swc.row.at(0));
			if (subject_swc.row.at(0).parent<0) //then it is a root, and does not overlap with others; this should be real root in the whole swc file
				realRootNodePool.push_back(subject_swc.row.at(0));
		}

		if (slength>1) //for the last node of the segment
		{
			for (cur_sid=0, b_termini=true;cur_sid<NSegs;cur_sid++)
			{
				if (cur_sid==seg_id)
					continue;

				V_NeuronSWC & target_swc = tracedNeuron.seg.at(cur_sid);
				int tlength = target_swc.nrows();

				for (i=0;i<target_swc.row.size();i++)
				{
					if (subject_swc.row.at(slength-1).get_coord() == target_swc.row.at(i).get_coord())
					{
						b_termini=false;
						break;
					}
				}
				if (!b_termini)
					break;
			}

			if (b_termini)
			{
				terminiNodePool.push_back(subject_swc.row.at(slength-1));
				if (subject_swc.row.at(slength-1).parent<0) //then it is a root, and does not overlap with others; this should be real root in the whole swc file
					realRootNodePool.push_back(subject_swc.row.at(slength-1));
			}
		}
	}


	//then find and update the location/radius that correspond to the merged nodes

#define V3DNEURON_MERGE_TO_BIGSPHERE 1

	V_NeuronSWC_list tracedNeuronNew = tracedNeuron;
	for (seg_id=0; seg_id<NSegs;seg_id++)
	{
		V_NeuronSWC & subject_swc = tracedNeuron.seg.at(seg_id);
		V_NeuronSWC & subject_swc_new = tracedNeuronNew.seg.at(seg_id);
		V3DLONG slength = subject_swc.nrows();

		for (j=0; j<slength; j++)
		{
			double scx = subject_swc.row.at(j).x;
			double scy = subject_swc.row.at(j).y;
			double scz = subject_swc.row.at(j).z;
			double sqr2 = subject_swc.row.at(j).r;
			sqr2 *= sqr2; //squared radius

			V3DLONG ind_best_merge_seg=-1, ind_best_merge_node=-1;	double r_best_merge=-1, dist_best; //set as -1 for initialization
			for (cur_sid=seg_id+1;cur_sid<NSegs;cur_sid++)
			{
				if (cur_sid==seg_id) //only find in a different seg
					continue;

				V_NeuronSWC & target_swc = tracedNeuron.seg.at(cur_sid);
				//map <V3DLONG, V3DLONG> target_index_map = unique_V_NeuronSWC_nodeindex(target_swc);
				V3DLONG tlength = target_swc.nrows();

				for (i=0;i<tlength;i++)
				{
					double tcr2 = target_swc.row.at(i).r; tcr2 *= tcr2; //squared radius

					double tt = (sqr2>tcr2)?sqr2:tcr2;
					tt=tt; //4; //devide by 2*2=4 so that the transition will be smoother

					double dtcx = target_swc.row.at(i).x - scx; dtcx *= dtcx;
					if (dtcx>tt)
						continue;

					double dtcy = target_swc.row.at(i).y - scy; dtcy *= dtcy;
					if (dtcy>tt || dtcy+dtcx>tt)
						continue;

					double dtcz = target_swc.row.at(i).z - scz; dtcz *= dtcz;
					if (dtcz>tt || dtcz+dtcy+dtcx>tt)
						continue;

					if (tcr2>sqr2 && tcr2>r_best_merge)
					{
						ind_best_merge_seg = cur_sid;
						ind_best_merge_node = i;
						r_best_merge = tcr2;
						dist_best = dtcz+dtcy+dtcx;
					}
				}
			}

			if (ind_best_merge_seg>=0) // && ind_best_merge_node>=0 && r_best_merge>=0) //only one judgment is enough
			{
				V_NeuronSWC & target_swc = tracedNeuron.seg.at(ind_best_merge_seg);

				if (V3DNEURON_MERGE_TO_BIGSPHERE) //I set it to 0, thus don't use this method
				{
					subject_swc_new.row.at(j).x = target_swc.row.at(ind_best_merge_node).x;
					subject_swc_new.row.at(j).y = target_swc.row.at(ind_best_merge_node).y;
					subject_swc_new.row.at(j).z = target_swc.row.at(ind_best_merge_node).z;
					subject_swc_new.row.at(j).r = target_swc.row.at(ind_best_merge_node).r;
				}
				else //then merge the location proportionally
				{
					V_NeuronSWC & target_swc_new = tracedNeuronNew.seg.at(ind_best_merge_seg);
					//
					double rs = subject_swc.row.at(j).r, rt = target_swc.row.at(ind_best_merge_node).r;
					double newx = (subject_swc.row.at(j).x*rs + target_swc.row.at(ind_best_merge_node).x*rt)/(rs+rt);
					double newy = (subject_swc.row.at(j).y*rs + target_swc.row.at(ind_best_merge_node).y*rt)/(rs+rt);
					double newz = (subject_swc.row.at(j).z*rs + target_swc.row.at(ind_best_merge_node).z*rt)/(rs+rt);
					double newr = (rs + rt + sqrt(dist_best))/2.0;

					//still just update one
					subject_swc_new.row.at(j).x = target_swc_new.row.at(ind_best_merge_node).x = newx;
					subject_swc_new.row.at(j).y = target_swc_new.row.at(ind_best_merge_node).y = newy;
					subject_swc_new.row.at(j).z = target_swc_new.row.at(ind_best_merge_node).z = newz;
					subject_swc_new.row.at(j).r = target_swc_new.row.at(ind_best_merge_node).r = newr;
				}
			}
		}
	}

	//then determine the pool of all nodes
	vector <V_NeuronSWC_unit> finalNodePool;
	for (seg_id=0; seg_id<NSegs;seg_id++)
	{
		V_NeuronSWC & subject_swc = tracedNeuronNew.seg.at(seg_id);
		V3DLONG slength = subject_swc.nrows();
		for (j=0; j<slength; j++)
		{
			bool b_exist=false;
			for (i=0;i<finalNodePool.size();i++)
			{
				if (subject_swc.row.at(j).get_coord() == finalNodePool.at(i).get_coord())
				{
					b_exist=true;
					break;
				}
			}
			if (!b_exist)
				finalNodePool.push_back(subject_swc.row.at(j)); //thus finalNodePool contains unique coordinates. Note the respective radius has been updated to the largest already.
		}
	}

	//now determine the edge graph
	std::vector<Edge> 	edge_array;
	for (seg_id=0; seg_id<NSegs;seg_id++)
	{
		V_NeuronSWC & subject_swc = tracedNeuronNew.seg.at(seg_id);
		map <V3DLONG, V3DLONG> subject_index_map = unique_V_NeuronSWC_nodeindex(subject_swc);
		V3DLONG slength = subject_swc.nrows();

		printf("seg=%ld\n", seg_id);
		for (j=0; j<slength; j++)
		{
			printf("j=%ld key=%ld mapped row=%ld node=%ld x=%5.3f y=%5.3f z=%5.3f parent=%ld\n", j, V3DLONG(subject_swc.row.at(j).n), subject_index_map[V3DLONG(subject_swc.row.at(j).n)], V3DLONG(subject_swc.row.at(j).n), subject_swc.row.at(j).x, subject_swc.row.at(j).y, subject_swc.row.at(j).z, V3DLONG(subject_swc.row.at(j).parent));
		}
		printf("\n\n");

		for (j=0; j<slength; j++)
		{
			V3DLONG c = j;
			V3DLONG p =  V3DLONG(subject_swc.row.at(j).parent);
			if (p<0) continue;
			else p = subject_index_map[p];

			//printf("c0=%ld p0=%ld c=%ld p=%ld\n", c0, p0, c, p);
			if (p<0 || p>=slength) {v3d_msg(QString("detect a strange parent! row(%1) node=%2 parent=%3").arg(j).arg(subject_swc.row.at(j).n).arg(p)); continue;}

			V3DLONG ipos=-1;	for (i=0;i<finalNodePool.size();i++)	{if (subject_swc.row.at(c).get_coord() == finalNodePool.at(i).get_coord())	{ipos=i;break;	}}
			V3DLONG ippos=-1;	for (i=0;i<finalNodePool.size();i++)	{if (subject_swc.row.at(p).get_coord() == finalNodePool.at(i).get_coord())	{ippos=i;break;	}}
			if (ipos>=0 && ippos>=0)
			{
				edge_array.push_back(Edge(ippos, ipos)); //for simplicity here I only keep a one-directional edge
			}
			else
			{
				v3d_msg("Error: this message should never appear. You have a neuron node whose coordinate cannot be detected!");
				return false;
			}
		}
	}


	//find the indexes of the termini nodes and root nodes in the finalNodePool (which contains the complete node info)
	V3DLONG *ind_end_nodes = new V3DLONG [terminiNodePool.size()], ind_startnode;
	if (realRootNodePool.size()<=0)
		ind_startnode=0;
	else
	{
		double cur_dist = distL2square(realRootNodePool.at(0).get_coord(), finalNodePool.at(0).get_coord()); //note that if realRootNodePool has size >=1, then finalNodePool should at least that big; then finalNodePool(0) should be safe
		for (j=0;j<finalNodePool.size();j++)
		{
			double tmp = distL2square(realRootNodePool.at(0).get_coord(), finalNodePool.at(j).get_coord());
			if (tmp<=cur_dist)
			{
				cur_dist = tmp;
				ind_startnode = j;
				if (cur_dist==0)
				{
					break;
				}
			}
		}
	}
	for (i=0;i<terminiNodePool.size();i++)
	{
		double cur_dist = distL2square(terminiNodePool.at(i).get_coord(), finalNodePool.at(0).get_coord()); //note that if realRootNodePool has size >=1, then finalNodePool should at least that big; then finalNodePool(0) should be safe
		for (j=0;j<finalNodePool.size();j++)
		{
			double tmp = distL2square(terminiNodePool.at(i).get_coord(), finalNodePool.at(j).get_coord());
			if (tmp<=cur_dist)
			{
				cur_dist = tmp;
				ind_end_nodes[i] = j;
				if (cur_dist == 0)
				{
					break;
				}
			}
		}
	}

	//now recompute the neuron structure based on merged node locations
	V3DLONG ntotalnodes = finalNodePool.size();
	double *xa = new double [ntotalnodes];
	double *ya = new double [ntotalnodes];
	double *za = new double [ntotalnodes];
	double *va = new double [ntotalnodes];
	for (i=0;i<ntotalnodes;i++)
	{
		xa[i] = finalNodePool.at(i).x;
		ya[i] = finalNodePool.at(i).y;
		za[i] = finalNodePool.at(i).z;
		va[i] = 255;
	}
	float zthickness = 1.0;
	vector< vector<V_NeuronSWC_unit> > mmUnit;
	ParaShortestPath para;
	V_NeuronSWC_list curTraceNeuron;
	curTraceNeuron.name = tracedNeuron.name;
	curTraceNeuron.file = tracedNeuron.file;

        const char *err_msg = find_shortest_path_graphpointset(ntotalnodes,
										   xa, ya, za, va, //the coordinates and values of all nodes
										   zthickness, // z-thickness for weighted edge
										   edge_array,
										   ind_startnode,        // start node's index
										   terminiNodePool.size(),          // n_end_nodes == (0 for shortest path tree) (1 for shortest path) (n-1 for n pair path)
										   ind_end_nodes,      // all end nodes' indexes
										   mmUnit, // change from Coord3D for shortest path tree
										   para);

	int nsegsnew = mergeback_mmunits_to_neuron_path(terminiNodePool.size(), mmUnit, curTraceNeuron);

	for (i=0;i<curTraceNeuron.seg.size();i++) //now use the already estimated radius information
	{
		for (j=0;j<curTraceNeuron.seg.at(i).row.size();j++)
		{
			bool b_find=false;
			for (V3DLONG ipos=0;ipos<finalNodePool.size();ipos++)
			{
				if (curTraceNeuron.seg.at(i).row.at(j).get_coord() == finalNodePool.at(ipos).get_coord())
				{
					curTraceNeuron.seg.at(i).row.at(j).r = finalNodePool.at(ipos).r;
					b_find=true;
					break;
				}
			}
			if (!b_find)
			{
				v3d_msg(QString("The %1 seg %2 node has an unmatched node.\n").arg(i).arg(j), 0);
			}
		}
	}



	//tracedNeuron = curTraceNeuron; //overwrite the neuron tracing result
	tracedNeuron.seg = curTraceNeuron.seg; //overwrite the neuron tracing result w/o producing an additional neuron. why? 100415
	proj_trace_history_append();

	//free space
	if (xa) {delete []xa; xa=0;}
	if (ya) {delete []ya; ya=0;}
	if (za) {delete []za; za=0;}
	if (va) {delete []va; va=0;}
	if (ind_end_nodes) {delete []ind_end_nodes; ind_end_nodes=0;}

	v3d_msg("quite merge all successfully", 0);
	return res;
}
