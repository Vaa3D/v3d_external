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


/**
 * @file nstroke_tracing.cpp
 * @brief n-right-strokes curve drawing (refine)
 * This curve drawing method performs as follows:
 *  1. The user first draws the primary curve using right-mouse moving. This drawing
 *     is based on the method used in solveCurveLineInter() and line intersection.
 *     The intersection between the line of last two locs and the viewing line.
 *
1) smCurveLineInter:
(1) get the nearest line (pa,pb) between line (t-2,t-1) and (loc0,loc1), where
 t-2, t-1 are last two loc of the traced curve, loc0,loc1 are (near,far) loc on
 the viewing line;
(2) get the mass center between (pa,pb) as the candidate loc by using function
  getCenterOfLineProfile().
2) smCurveDirectionInter:
(1) get the nearest line (pa,pb) between line (t-2,t-1) and (loc0,loc1). Set pb
  as one of candidate loc values;
(2) get the mass center between (loc0,loc1) as another candidate loc by using function
  getCenterOfLineProfile();
(3) compare signal (mean+std) difference of two candidate locs and last two traced locs.
 the candidate loc with less difference is the loc we want.

 * @author: Jianlong Zhou
 * @date: Jan 24, 2012
 * Last change by PHC, 20120330. fix a few bugs.
 *
 */

#include "renderer_gl1.h"
#include "v3dr_glwidget.h"
#include "barFigureDialog.h"
#include "v3d_application.h"
#ifndef test_main_cpp

#include "../v3d/v3d_compile_constraints.h"
#include "../v3d/landmark_property_dialog.h"
#include "../v3d/surfaceobj_annotation_dialog.h"
#include "../v3d/surfaceobj_geometry_dialog.h"
#include "../neuron_editing/neuron_sim_scores.h"
#include "../neuron_tracing/neuron_tracing.h"
#include "../basic_c_fun/v3d_curvetracepara.h"
#include "../v3d/dialog_curve_trace_para.h"
#include <time.h>

#include <sstream>
#include <map>
#include <algorithm>
#include <ctime>
//#include <QMessageBox>

//#include <boost/chrono.hpp>
//#include <boost/thread/thread.hpp>

#ifndef Q_OS_MAC
    #include <omp.h>
#endif

#include <boost/algorithm/string.hpp>
#endif //test_main_cpp

#include "../neuron_tracing/fastmarching_linker.h"

#include "line_box_intersection_check.h"

#include "../terafly/src/control/CPlugin.h"
//#include "../terafly/src/control/CViewer.h"

// ------- Conditional inclusion for fragment tracing, MK, Mar, 2019 -------
//#include "../v3d/CustomDefine.h"
//#ifdef _FRAGTRACE_
//#include "NeuronStructExplorer.h"
//#include "NeuronStructUtilities.h"
//#endif
// -------------------------------------------------------------------------
#ifdef _WIN32
    #include <windows.h>

    void my_sleep(unsigned milliseconds)
    {
        Sleep(milliseconds);
    }
#else
    #include <unistd.h>

    void my_sleep(unsigned milliseconds)
    {
        usleep(milliseconds * 1000); // takes microseconds
    }
#endif

#define EPS 0.01
#define PI 3.14159265
#define MAX_DOUBLE 1.79769e+308
#define INF 1.0e300

#ifndef MIN
#define MIN(a, b)  ( ((a)<(b))? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a, b)  ( ((a)>(b))? (a) : (b) )
#endif

#define PROCESS_OUTSWC_TO_CURVE(outswc, sub_orig, i) \
{ \
     if(!outswc.empty()) \
     { \
          V3DLONG nn = outswc.size(); \
          if (nn>=1) \
          { \
            if(i == 1) \
            { \
              XYZ loci; \
              loci.x=outswc.at(0)->x + sub_orig.x; \
              loci.y=outswc.at(0)->y + sub_orig.y; \
              loci.z=outswc.at(0)->z + sub_orig.z; \
              loc_vec.push_back(loci); \
            } \
            for(V3DLONG j=0; j<nn; j++ ) \
            { \
               XYZ locj; \
               assert(outswc[j]); \
               locj.x=outswc.at(j)->x + sub_orig.x; \
               locj.y=outswc.at(j)->y + sub_orig.y; \
               locj.z=outswc.at(j)->z + sub_orig.z; \
            {    ;}\
               if (loc_vec.size()<=0 || loc_vec.back().x != locj.x || loc_vec.back().y != locj.y || loc_vec.back().z != locj.z)  \
               {   loc_vec.push_back(locj); ;}\
            } \
          }\
     } \
}

template <class Tpre, class Tpost>
     void converting_to_8bit(void *pre1d, Tpost *pPost, V3DLONG imsz);

struct nodeInfo
{
	double sqr1, sqr2, dot, radAngle, distToMain, turnCost;
	vector<V_NeuronSWC_unit>::iterator nodeAddress;
	bool front, back;
	long segID, frontSegID, backSegID, nodeNum, x, y, z;
};

struct cutNode
{
	V_NeuronSWC_unit node;
	double distance;
	vector<V_NeuronSWC_unit>::iterator nodeAddress;
	long segID;
	long nodeinSegId;
};

void Renderer_gl1::solveCurveDirectionInter(vector <XYZ> & loc_vec_input, vector <XYZ> &loc_vec, int index)
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

	////////////////////////////////////////////////////////////////////////
	int chno = checkCurChannel();
	if (chno<0 || chno>dim4-1)   chno = 0; //default first channel
	////////////////////////////////////////////////////////////////////////
	qDebug()<<"\n solveCurveDirectionInter: 3d curve in channel # "<<((chno<0)? chno :chno+1);

	loc_vec.clear();

     // add intersection point into listMarker
     listMarker.clear();
     int totalm=0;//total intersection point

	int N = loc_vec_input.size();
	if (b_use_seriespointclick)
	{
		loc_vec = loc_vec_input;
	}
	else //then use the moving mouse location, otherwise using the preset loc_vec_input (which is set by the 3d-curve-by-point-click function)
	{
		N = list_listCurvePos.at(index).size(); // change from 0 to index for different curves, ZJL
		for (int i=0; i<N; i++)
		{
			const MarkerPos & pos = list_listCurvePos.at(index).at(i); // change from 0 to index for different curves, ZJL
			////////////////////////////////////////////////////////////////////////
               double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
			clipplane[3] = viewClip;
			ViewPlaneToModel(pos.MV, clipplane);
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
                    int NL = loc_vec.size();
				int last_j = NL-1;
				int last_j2 = NL-2;
				XYZ lastpos, lastpos2;
                    if (last_j>=0 && b_use_last_approximate) //091114 PHC make it more stable by conditioned on previous location
				{
					lastpos = loc_vec.at(last_j);
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

				// determine loc based on intersection of vector (lastpos2, lastpos) and (loc0,loc1)
				if (last_j2>=0)
				{
					XYZ pa, pb;
					double mua, mub;
					lastpos2 = loc_vec.at(last_j2);
					bool res=lineLineIntersect(lastpos2, lastpos, loc0, loc1, &pa, &pb, &mua, &mub);
                         if(!res)
                         {
                              //no closest pos
                              qDebug()<<"No intersection checking resutls!";
                              loc=getCenterOfLineProfile(loc0, loc1, clipplane, chno);
                         }else
                         {
                              // to see whether this is the right loc by comparing mean+sdev
                              XYZ loci = pb;

                              // check if pb is inside line seg of(loc0,loc1)
                              // confine pb inside (loc0,loc1), if outside, then use locc directly
                              if( !withinLineSegCheck( loc0, loc1, loci ) )
                              {
                                   // pb is outside (loc0,loc), so search in (loc0,loc1)
                                   loc=getCenterOfLineProfile(loc0, loc1, clipplane, chno);
                                   //addMarker(loc); // for comparison purpose, delete it later
                              }else
                              {
                                   // search loc within a smaller ranger around loci
                                   XYZ v_1_0 = loc1-loc0;
                                   float length = dist_L2(loc0, loc1);
                                   float ranget = length/5.0;
                                   XYZ D = v_1_0; normalize(D);
                                   loc0 = loci - D*(ranget);
                                   loc1 = loci + D*(ranget);
                                   loc = getCenterOfLineProfile(loc0, loc1, clipplane, chno);
                              }
                         }


				} else // for first two locs
				{
					loc = getCenterOfLineProfile(loc0, loc1, clipplane, chno);
				}
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
                    // at(0) to at(index) ZJL 110901
                double best_dist;
				V3DLONG n_id_start = findNearestNeuronNode_WinXY(list_listCurvePos.at(index).at(0).x, list_listCurvePos.at(index).at(0).y, p_tree, best_dist);
				V3DLONG n_id_end = findNearestNeuronNode_WinXY(list_listCurvePos.at(index).at(N-1).x, list_listCurvePos.at(index).at(N-1).y, p_tree, best_dist);
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

	if (b_use_seriespointclick==false)
		smooth_curve(loc_vec, 5);
#endif
     if (b_addthiscurve)
     {
		 cout << "addcurve pos mode 7" << endl;
          addCurveSWC(loc_vec, chno, 5); //LMG 26/10/2018 solveCurveDirectionInter mode 5
          // used to convert loc_vec to NeuronTree and save SWC in testing
          vecToNeuronTree(testNeuronTree, loc_vec);
     }
     else //100821
     {
          b_addthiscurve = true; //in this case, always reset to default to draw curve to add to a swc instead of just  zoom
          endSelectMode();
     }
}


/**
 * @brief This function is used for getting loc using center of mass method.
 *  ZJL 2012-01-26
*/
XYZ Renderer_gl1::getLocUsingMassCenter(bool firstloc, XYZ lastpos, XYZ P1, XYZ P2,
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

     // get signal statistics
     double signal_last, signal_cur, diff_min, diff_cur;
     LocationSimple pt, pt_cur;
     XYZ loc_min; //loc with min diff

     if(!firstloc)
     {
          getRgnPropertyAt(lastpos, pt);
          signal_last=pt.ave + pt.sdev;

          getRgnPropertyAt(loc, pt_cur);
          signal_cur=pt_cur.ave + pt_cur.sdev;

          diff_min = abs( signal_last-signal_cur );
          loc_min = loc;
     }

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
                    if(!firstloc)
                         return loc_min;
                    else
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
						if(!firstloc)
                                   return loc_min;
                              else
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

               // get statistical infor on loc and compare with previous one
               if(!firstloc)
               {
                    getRgnPropertyAt(loc, pt_cur);
                    signal_cur=pt_cur.ave+pt_cur.sdev;

                    diff_cur = abs( signal_cur-signal_last );
                    if(diff_cur<diff_min)
                    {
                         loc_min=loc;
                         diff_min=diff_cur;
                    }
               }

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
     if(!firstloc)
          return loc_min;
     else
          return loc;

}

/*
 @brief Calculate the line segment PaPb that is the shortest route between
 two lines P1P2 and P3P4. Calculate also the values of mua and mub where
 Pa = P1 + mua (P2 - P1)
 Pb = P3 + mub (P4 - P3)
 Return FALSE if no solution exists.
http://paulbourke.net/geometry/lineline3d/
 */
bool Renderer_gl1::lineLineIntersect( XYZ p1,XYZ p2,XYZ p3,XYZ p4,XYZ *pa,XYZ *pb,
					  double *mua, double *mub)
{
	XYZ p13,p43,p21;
	double d1343,d4321,d1321,d4343,d2121;
	double numer,denom;

	p13.x = p1.x - p3.x;
	p13.y = p1.y - p3.y;
	p13.z = p1.z - p3.z;
	p43.x = p4.x - p3.x;
	p43.y = p4.y - p3.y;
	p43.z = p4.z - p3.z;
	if (abs(p43.x) < EPS && abs(p43.y) < EPS && abs(p43.z) < EPS)
		return false;
	p21.x = p2.x - p1.x;
	p21.y = p2.y - p1.y;
	p21.z = p2.z - p1.z;
	if (abs(p21.x) < EPS && abs(p21.y) < EPS && abs(p21.z) < EPS)
		return false;

	d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
	d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
	d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
	d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
	d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

	denom = d2121 * d4343 - d4321 * d4321;
	if (abs(denom) < EPS)
		return false;
	numer = d1343 * d4321 - d1321 * d4343;

	*mua = numer / denom;
	*mub = (d1343 + d4321 * (*mua)) / d4343;

	pa->x = p1.x + *mua * p21.x;
	pa->y = p1.y + *mua * p21.y;
	pa->z = p1.z + *mua * p21.z;
	pb->x = p3.x + *mub * p43.x;
	pb->y = p3.y + *mub * p43.y;
	pb->z = p3.z + *mub * p43.z;

	return true;
}

/*
 *@brief Chech whether point pa is on the line (p1,p2) and within (p1,p2)
*/
bool Renderer_gl1::withinLineSegCheck( XYZ p1,XYZ p2,XYZ pa)
{
     XYZ p12 = p2-p1;
     XYZ p1a = pa-p1;
     bool colinear = norm(cross(p12, p1a))<EPS ;
     double dotpro = dot(p12, p1a);
     double dot12 = dot(p12, p12); // square length between 12
     bool within = (dotpro>0 && dotpro<dot12);

     return (colinear && within);
}




// get bounding volume from two stroke points
// chno is channel
void Renderer_gl1::getSubVolFrom2MarkerPos(vector<MarkerPos> & pos, int chno, double* &pSubdata, XYZ &sub_orig, XYZ &max_loc, V3DLONG &sub_szx,
     V3DLONG &sub_szy, V3DLONG &sub_szz)
{
     XYZ minloc, maxloc;

     // find min-max of x y z in loc_veci
     float minx, miny, minz, maxx, maxy, maxz;

     if(selectMode == smCurveFrom1Marker_fm)
     {
          minx = maxx = curveStartMarker.x - 1;
          miny = maxy = curveStartMarker.y - 1;
          minz = maxz = curveStartMarker.z - 1;
     }
     else
     {
          minx = miny = minz = INF;
          maxx = maxy = maxz = 0;
     }

     // Directly using stroke pos for minloc, maxloc
     for(int i=0; i<pos.size(); i++ )
     {
          double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
          clipplane[3] = viewClip;
          ViewPlaneToModel(pos.at(i).MV, clipplane);

          XYZ loc0_t, loc1_t;
          _MarkerPos_to_NearFarPoint(pos.at(i), loc0_t, loc1_t);

          // get intersection point of (loc0,loc1) with data volume
		  IntersectResult<2> intersect = intersectPointsWithData(loc0_t, loc1_t);

          // XYZ loc0_t, loc1_t;
          // _MarkerPos_to_NearFarPoint(pos.at(i), loc0_t, loc1_t);

          // XYZ loc0, loc1;
          // // // check line_box intersection points
          // // CheckLineBox( dataViewProcBox.V0(), dataViewProcBox.V1(), loc0_t, loc1_t, loc0);
          // // CheckLineBox( dataViewProcBox.V0(), dataViewProcBox.V1(), loc1_t, loc0_t, loc1);
          // // use another intersection version
          // getIntersectPt(loc0_t, loc1_t,  dataViewProcBox, loc0);
          // getIntersectPt(loc1_t, loc0_t,  dataViewProcBox, loc1);

          // if(i==0)
          // {
          //      minx=maxx=loc0.x; miny=maxy=loc0.y; minz=maxz=loc0.z;
          // }
          // else
          if (intersect.success[0] && intersect.success[1]) 
		  {
			  XYZ loc0 = intersect.hit_locs[0], loc1 = intersect.hit_locs[1];

              if(minx>loc0.x) minx=loc0.x;
              if(miny>loc0.y) miny=loc0.y;
              if(minz>loc0.z) minz=loc0.z;
			  
			  if(maxx<loc0.x) maxx=loc0.x;
              if(maxy<loc0.y) maxy=loc0.y;
              if(maxz<loc0.z) maxz=loc0.z;

              if(minx>loc1.x) minx=loc1.x;
              if(miny>loc1.y) miny=loc1.y;
              if(minz>loc1.z) minz=loc1.z;

              if(maxx<loc1.x) maxx=loc1.x;
              if(maxy<loc1.y) maxy=loc1.y;
              if(maxz<loc1.z) maxz=loc1.z;
          }
     }

     int boundary = 3; // need to test for this boundary value. It seems that 0 is OK

     minloc.x = minx - boundary;
     minloc.y = miny - boundary;
     minloc.z = minz - boundary;

     maxloc.x = maxx + boundary;
     maxloc.y = maxy + boundary;
     maxloc.z = maxz + boundary;

     if (!dataViewProcBox.isInner(minloc, 0.5))
          dataViewProcBox.clamp(minloc);
     if (!dataViewProcBox.isInner(maxloc, 0.5))
          dataViewProcBox.clamp(maxloc);

     // get data buffer
     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if (w)
          curImg = v3dr_getImage4d(_idep);

     // The data is from minloc to maxloc
     sub_szx = maxloc.x-minloc.x +1;
     sub_szy = maxloc.y-minloc.y +1;
     sub_szz = maxloc.z-minloc.z +1;

     sub_orig = minloc;
     max_loc = maxloc;

     //if(pSubdata) {delete [] pSubdata; pSubdata=0;}

     pSubdata = new double [sub_szx*sub_szy*sub_szz];
     for(V3DLONG k=0; k<sub_szz; k++)
          for(V3DLONG j=0; j<sub_szy; j++)
               for(V3DLONG i=0; i<sub_szx; i++)
               {
                    V3DLONG ind = k*sub_szy*sub_szx + j*sub_szx + i;
                    pSubdata[ind]=curImg->at(minloc.x+i, minloc.y+j, minloc.z+k, chno);
               }

}


// get bounding volume from two stroke points
// chno is channel
void Renderer_gl1::getSubVolFrom3Points(XYZ & loc0_last, XYZ & loc0, XYZ & loc1, int chno, double* &pSubdata,
    XYZ &sub_orig, V3DLONG &sub_szx, V3DLONG &sub_szy, V3DLONG &sub_szz)
{
    XYZ minloc, maxloc;

    // get intersection point of (loc0,loc1) with data volume
	IntersectResult<2> intersect = intersectPointsWithData(loc0, loc1);
	XYZ hit_loc0 = intersect.hit_locs[0], hit_loc1 = intersect.hit_locs[1];

	// find min-max of x y z in loc_veci
    float minx, miny, minz, maxx, maxy, maxz;

    minx=maxx=hit_loc0.x; miny=maxy=hit_loc0.y; minz=maxz=hit_loc0.z;

    if(minx>hit_loc1.x) minx=hit_loc1.x;
    if(miny>hit_loc1.y) miny=hit_loc1.y;
    if(minz>hit_loc1.z) minz=hit_loc1.z;

    if(maxx<hit_loc1.x) maxx=hit_loc1.x;
    if(maxy<hit_loc1.y) maxy=hit_loc1.y;
    if(maxz<hit_loc1.z) maxz=hit_loc1.z;

    if(minx>loc0_last.x) minx=loc0_last.x;
    if(miny>loc0_last.y) miny=loc0_last.y;
    if(minz>loc0_last.z) minz=loc0_last.z;

    if(maxx<loc0_last.x) maxx=loc0_last.x;
    if(maxy<loc0_last.y) maxy=loc0_last.y;
    if(maxz<loc0_last.z) maxz=loc0_last.z;
	
    int boundary = 5;

    minloc.x = minx - boundary;
    minloc.y = miny - boundary;
    minloc.z = minz - boundary;

    maxloc.x = maxx + boundary;
    maxloc.y = maxy + boundary;
    maxloc.z = maxz + boundary;

    if (!dataViewProcBox.isInner(minloc, 0.1))
        dataViewProcBox.clamp(minloc);
    if (!dataViewProcBox.isInner(maxloc, 0.1))
        dataViewProcBox.clamp(maxloc);

    // get data buffer
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    My4DImage* curImg = 0;
    if (w)
        curImg = v3dr_getImage4d(_idep);

    // The data is from minloc to maxloc
    sub_szx=abs(maxloc.x-minloc.x)+1;
    sub_szy=abs(maxloc.y-minloc.y)+1;
    sub_szz=abs(maxloc.z-minloc.z)+1;

    sub_orig = minloc;

    //if(pSubdata) {delete [] pSubdata; pSubdata=0;}

    pSubdata = new double [sub_szx*sub_szy*sub_szz];
    for(V3DLONG k=0; k<sub_szz; k++)
        for(V3DLONG j=0; j<sub_szy; j++)
            for(V3DLONG i=0; i<sub_szx; i++)
            {
                V3DLONG ind = k*sub_szy*sub_szx + j*sub_szx + i;
                pSubdata[ind]=curImg->at(minloc.x+i, minloc.y+j, minloc.z+k, chno);
            }
}



// get bounding volume from a stroke
void Renderer_gl1::getSubVolFromStroke(double* &pSubdata, int c, XYZ &sub_orig, V3DLONG &sub_szx,
     V3DLONG &sub_szy, V3DLONG &sub_szz)
{
     XYZ minloc, maxloc;
     boundingboxFromStroke(minloc, maxloc);

     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if (w)
          curImg = v3dr_getImage4d(_idep);

     // The data is from minloc to maxloc
     sub_szx=abs(maxloc.x-minloc.x)+1;
     sub_szy=abs(maxloc.y-minloc.y)+1;
     sub_szz=abs(maxloc.z-minloc.z)+1;

     sub_orig = minloc;

     //if(pSubdata) {delete [] pSubdata; pSubdata=0;}

     pSubdata = new double [sub_szx*sub_szy*sub_szz];
     for(V3DLONG k=0; k<sub_szz; k++)
          for(V3DLONG j=0; j<sub_szy; j++)
               for(V3DLONG i=0; i<sub_szx; i++)
               {
                    V3DLONG ind = k*sub_szy*sub_szx + j*sub_szx + i;
                    pSubdata[ind]=curImg->at(minloc.x+i, minloc.y+j, minloc.z+k, c);
               }

}



// get the control points of the primary curve
void Renderer_gl1::getSubVolInfo(XYZ lastloc, XYZ loc0, XYZ loc1, XYZ &sub_orig, double* &pSubdata,
                                V3DLONG &sub_szx, V3DLONG &sub_szy, V3DLONG &sub_szz)
{
     //
     int boundary = 10;
     XYZ minloc, maxloc;
     minloc.x = MIN(lastloc.x, MIN(loc0.x, loc1.x)) - boundary;
     minloc.y = MIN(lastloc.y, MIN(loc0.y, loc1.y)) - boundary;
     minloc.z = MIN(lastloc.z, MIN(loc0.z, loc1.z)) - boundary;

     maxloc.x = MAX(lastloc.x, MAX(loc0.x, loc1.x)) + boundary;
     maxloc.y = MAX(lastloc.y, MAX(loc0.y, loc1.y)) + boundary;
     maxloc.z = MAX(lastloc.z, MAX(loc0.z, loc1.z)) + boundary;

     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if (w)
          curImg = v3dr_getImage4d(_idep);


     V3DLONG szx = curImg->getXDim();
     V3DLONG szy = curImg->getYDim();
     V3DLONG szz = curImg->getZDim();
     minloc.x = qBound((V3DLONG)0, (V3DLONG)minloc.x, (V3DLONG)(szx-1));
     minloc.y = qBound((V3DLONG)0, (V3DLONG)minloc.y, (V3DLONG)(szy-1));
     minloc.z = qBound((V3DLONG)0, (V3DLONG)minloc.z, (V3DLONG)(szz-1));

     maxloc.x = qBound((V3DLONG)0, (V3DLONG)maxloc.x, (V3DLONG)(szx-1));
     maxloc.y = qBound((V3DLONG)0, (V3DLONG)maxloc.y, (V3DLONG)(szy-1));
     maxloc.z = qBound((V3DLONG)0, (V3DLONG)maxloc.z, (V3DLONG)(szz-1));

     // The data is from minloc to maxloc
     sub_szx=maxloc.x-minloc.x+1;
     sub_szy=maxloc.y-minloc.y+1;
     sub_szz=maxloc.z-minloc.z+1;

     sub_orig = minloc;

     //if(pSubdata) {delete [] pSubdata; pSubdata=0;}
     pSubdata = new double [sub_szx*sub_szy*sub_szz];
     for(V3DLONG k=0; k<sub_szz; k++)
          for(V3DLONG j=0; j<sub_szy; j++)
               for(V3DLONG i=0; i<sub_szx; i++)
               {
                    V3DLONG ind = k*sub_szy*sub_szx + j*sub_szx + i;
                    pSubdata[ind]=curImg->at(i,j,k);
               }
}


void Renderer_gl1::solveCurveFromMarkersFastMarching()
{
	qDebug("  Renderer_gl1::solveCurveMarkersFastMarching");

	vector <XYZ> loc_vec_input;
     loc_vec_input.clear();

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;
     if (w) curImg = v3dr_getImage4d(_idep); //by PHC, 090119

      int chno = checkCurChannel();
          if (chno<0 || chno>dim4-1)   chno = 0; //default first channel

     if (selectMode == smCurveCreate_pointclick_fm || selectMode == smCurveCreate_MarkerCreate1_fm)
     {
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
     }
     else if(selectMode == smCurveMarkerPool_fm)
     {
          int nn=listCurveMarkerPool.size();
          XYZ cur_xyz;
          int i;
          for (i=0;i<nn;i++)
          {
               cur_xyz.x = listCurveMarkerPool.at(i).x-1; //marker location is 1 based
               cur_xyz.y = listCurveMarkerPool.at(i).y-1;
               cur_xyz.z = listCurveMarkerPool.at(i).z-1;
               loc_vec_input.push_back(cur_xyz);
          }
     }

     // loc_vec is used to store final locs on the curve
     vector <XYZ> loc_vec;
     loc_vec.clear();

     // vec used for second fastmarching
     vector<XYZ> middle_vec;
     middle_vec.clear();

     // set pregress dialog
     PROGRESS_DIALOG( "Curve creating", widget);
     PROGRESS_PERCENT(10);

     if (loc_vec_input.size()>0)
	{
          middle_vec.push_back(loc_vec_input.front());//first marker
          loc_vec.push_back(loc_vec_input.front());

          V3DLONG szx = curImg->getXDim();
          V3DLONG szy = curImg->getYDim();
          V3DLONG szz = curImg->getZDim();

          // get img data pointer
          unsigned char* pImg = 0;
          if (curImg && data4dp && chno>=0 &&  chno <dim4)
          {
               switch (curImg->getDatatype())
               {
                    case V3D_UINT8:
                         pImg = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1);
                         break;
                    case V3D_UINT16:
                         pImg = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(short int);
                         break;
                    case V3D_FLOAT32:
                         pImg = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(float);
                         break;
                    default:
                         v3d_msg("Unsupported data type found. You should never see this.", 0);
                         return ;
               }
          }else
          {
               v3d_msg("No data available. You should never see this.", 0);
               return ;
          }

		qDebug("now get curve using fastmarching method");
          // Using fast_marching method to get loc
          for(int ii=0; ii<loc_vec_input.size()-1; ii++)
          {
               XYZ loc0=loc_vec_input.at(ii);
               XYZ loc1=loc_vec_input.at(ii+1);

               vector<MyMarker*> outswc;

               vector<MyMarker> sub_markers;
               vector<MyMarker> tar_markers;
               MyMarker mloc0(loc0.x, loc0.y, loc0.z);
               MyMarker mloc1(loc1.x, loc1.y, loc1.z);
               sub_markers.push_back(mloc0);
               tar_markers.push_back(mloc1);

               // call fastmarching
               if(selectMode == smCurveCreate_MarkerCreate1_fm)
               {
                   if(!fastmarching_linker_timer(sub_markers, tar_markers, pImg, outswc, szx, szy, szz))
                   {
                       return;
                   }
               }
               else
                   fastmarching_linker(sub_markers, tar_markers, pImg, outswc, szx, szy, szz);
               XYZ sub_orig = XYZ(0,0,0);
               PROCESS_OUTSWC_TO_CURVE(outswc, sub_orig, ii+1);

               // if(!outswc.empty())
               // {
               //      // the 1st loc in outswc is the last pos got in fm
               //      for(int j=0; j<=outswc.size()-2; j++ )
               //      {
               //           XYZ loc;
               //           loc.x=outswc.at(j)->x;
               //           loc.y=outswc.at(j)->y;
               //           loc.z=outswc.at(j)->z;

               //           loc_vec.push_back(loc);
               //           if(j==outswc.size()/2) // save middle point
               //           {
               //                middle_vec.push_back(loc);
               //           }
               //      }

               //     //always remember to free the potential-memory-problematic fastmarching_linker return value
               //     clean_fm_marker_vector(outswc);

               // }else
               // {
               //      loc_vec.push_back(loc1);
               // }

          }

          loc_vec.push_back(loc_vec_input.back());

          PROGRESS_PERCENT(60);

          int N = loc_vec.size();
          if(N<1) return; // all points are outside the volume. ZJL 110913

          // // append the last XYZ of loc_vec to middle_vec
          // middle_vec.push_back(loc_vec_input.back());// last marker

          // //===============================================================================>>>>>>>>>>>>

          // loc_vec.clear();

          // loc_vec.push_back(middle_vec.at(0)); //append the first loc
          // // Do the second fastmarching for smoothing the curve
          // for(int jj=0;jj<middle_vec.size()-1; jj++)
          // {
          //      // create MyMarker list
          //      vector<MyMarker> sub_markers;
          //      vector<MyMarker> tar_markers;
          //      vector<MyMarker*> outswc;
          //      XYZ loc;

          //      // sub_markers
          //      XYZ loc0=middle_vec.at(jj);
          //      MyMarker mloc0 = MyMarker(loc0.x, loc0.y, loc0.z);
          //      sub_markers.push_back(mloc0);

          //      // tar_markers
          //      XYZ loc1=middle_vec.at(jj+1);
          //      MyMarker mloc1 = MyMarker(loc1.x, loc1.y, loc1.z);
          //      tar_markers.push_back(mloc1);

          //      // call fastmarching
          //      fastmarching_linker(sub_markers, tar_markers, pImg, outswc, szx, szy, szz);

          //      if(!outswc.empty())
          //      {
          //           // the 1st loc in outswc is the last pos got in fm
          //           int nn = outswc.size();
          //           for(int j=nn-2; j>=0; j-- )
          //           {
          //                XYZ locj;
          //                locj.x=outswc.at(j)->x;
          //                locj.y=outswc.at(j)->y;
          //                locj.z=outswc.at(j)->z;

          //                loc_vec.push_back(locj);
          //           }
          //           //always remember to free the potential-memory-problematic fastmarching_linker return value
          //           clean_fm_marker_vector(outswc);
          //      }
          //      else
          //      {
          //           loc_vec.push_back(loc1);
          //      }

          // } // end for the second fastmarching
          // loc_vec.push_back(middle_vec.back());
          //===============================================================================<<<<<<<<<<<<<
          PROGRESS_PERCENT(80);

          if(loc_vec.size()<1) return; // all points are outside the volume. ZJL 110913

          // check if there is any existing neuron node is very close to the starting and ending points, if yes, then merge

          MainWindow* V3Dmainwindow = 0;
          V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);

          N = loc_vec.size();

          if (V3Dmainwindow && V3Dmainwindow->global_setting.b_3dcurve_autoconnecttips)
          {
               if (listNeuronTree.size()>0 && curEditingNeuron>0 && curEditingNeuron<=listNeuronTree.size())
               {
                    NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(curEditingNeuron-1)));
                    if (p_tree)
                    {
                         V3DLONG n_id_start = findNearestNeuronNode_Loc(loc_vec.at(0), p_tree);
                         V3DLONG n_id_end = findNearestNeuronNode_Loc(loc_vec.at(N-1), p_tree);
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
                         if (b_start_merged || b_end_merged)
                         {
                             if (cur_node.seg_id>=0 && cur_node.seg_id< curImg->tracedNeuron.seg.size())
                                 if(curImg->tracedNeuron.seg[cur_node.seg_id].row.size() >0)
                                     currentTraceType = curImg->tracedNeuron.seg[cur_node.seg_id].row[0].type;
                         }
                    }
               }
          }

          smooth_curve(loc_vec, 5);

          // adaptive curve simpling
          vector <XYZ> loc_vec_resampled;
          int stepsize = 6; // sampling stepsize
          loc_vec_resampled.clear();
          adaptiveCurveResampling(loc_vec, loc_vec_resampled, stepsize);

          if (b_addthiscurve)
          {
			  cout << "addcurve pos mode 3" << endl;
               addCurveSWC(loc_vec_resampled, chno, 3); //LMG 26/10/2018 solveCurveFromMarkersFastMarching (GD tracing Ctrl+G) mode 3
               // used to convert loc_vec to NeuronTree and save SWC in testing
               vecToNeuronTree(testNeuronTree, loc_vec_resampled);
          }
          else //100821
          {
               b_addthiscurve = true; //in this case, always reset to default to draw curve to add to a swc instead of just  zoom
               endSelectMode();
          }
     }
}
/**
 * @brief The curve is resampled based on its local curvature:
 *  L1(t-1,t), L2(t,t+1). If the angle between L1 and L2 is less than thresh_theta1. sample in stepsize.
 *  If the angle is between thresh_theta1 and thresh_theta2, sample in smaller stepsize.
 *  Otherwise, using original resolution.
 *  Jianong Zhou 20120204
 * this function seems will produce redundant resampling. revised by PHC 20120330
*/
void Renderer_gl1::adaptiveCurveResampling(vector <XYZ> &loc_vec, vector <XYZ> &loc_vec_resampled, int stepsize)
{
     int N = loc_vec.size();
	if (N<=0 || stepsize<=0) return;

     loc_vec_resampled.clear();
     loc_vec_resampled.push_back(loc_vec.at(0)); //should have at least one entry now

     // used to control whether cur-to-nex locs are added
     bool b_prestep_added = false;

     for(int i=stepsize; i<N; i=i+stepsize)
     {
          int ind_nex = ( (i+stepsize)>=N )? (N-1):(i+stepsize);
          XYZ & loc_cur = loc_vec.at(i);
          XYZ & loc_pre = loc_vec.at(i-stepsize);
          XYZ & loc_nex = loc_vec.at(ind_nex);

          XYZ v1 = loc_cur-loc_pre;
          XYZ v2 = loc_nex-loc_cur;

          // check the angle betwen v1 and v2
          float cos_theta;
          normalize(v1); normalize(v2);
          float theta = acos( dot(v1,v2) ) * 180.0/PI; // radius to degree

          // threshold theta
          float thresh_theta1 = 30.0;
          float thresh_theta2 = 60.0;

          if(theta<thresh_theta1)
          {
			  XYZ & loc_end = loc_vec_resampled.back();
			  XYZ & loc_this = loc_vec.at(i);

			  if (loc_end.x!=loc_this.x || loc_end.y!=loc_this.y || loc_end.z!=loc_this.z)
				  loc_vec_resampled.push_back( loc_this );

               b_prestep_added = false;
          }
          else if( (theta>=thresh_theta1) && (theta<=thresh_theta2) )
          {
               float new_step = stepsize/2;
               int ind_start;
               if(b_prestep_added)
                    ind_start = i;
               else
                    ind_start = i-stepsize;

               for( int j=ind_start+new_step; j<=ind_nex; j=j+new_step)
               {
				   XYZ & loc_end = loc_vec_resampled.back();
				   XYZ & loc_this = loc_vec.at(j);

				   if (loc_end.x!=loc_this.x || loc_end.y!=loc_this.y || loc_end.z!=loc_this.z)
					   loc_vec_resampled.push_back( loc_this );
               }
               b_prestep_added = true;

          }
          else if( theta>thresh_theta2 )
          {
               int ind_start;
               if(b_prestep_added)
                    ind_start = i;
               else
                    ind_start = i-stepsize;

               for(int j=ind_start+1; j<=ind_nex; j++)
               {
				   XYZ & loc_end = loc_vec_resampled.back();
				   XYZ & loc_this = loc_vec.at(j);

				   if (loc_end.x!=loc_this.x || loc_end.y!=loc_this.y || loc_end.z!=loc_this.z)
					   loc_vec_resampled.push_back( loc_this );
               }

               b_prestep_added = true;
          }
     }
     loc_vec_resampled.push_back(loc_vec.back());
}

void Renderer_gl1::adaptiveCurveResamplingRamerDouglasPeucker(vector <XYZ> &loc_vec, vector <XYZ> &loc_vec_resampled, float epsilon)
{
     int N = loc_vec.size();
     if (N<=0) return;

     recursiveRamerDouglasPeucker(loc_vec, loc_vec_resampled, 0, N-1, epsilon);
}

void Renderer_gl1::recursiveRamerDouglasPeucker(vector <XYZ> &loc_vec, vector <XYZ> &loc_vec_resampled, int start_i, int end_i, float epsilon)
{
     // Recursive Ramer–Douglas–Peucker algorithm
     loc_vec_resampled.clear();
     XYZ & loc_start = loc_vec.at(start_i);
     XYZ & loc_final = loc_vec.at(end_i);

     if (start_i >= end_i)
     {
          loc_vec_resampled.push_back(loc_start);
          return;
     }
     else if (end_i - start_i == 1)
     {
          loc_vec_resampled.push_back(loc_start);
          loc_vec_resampled.push_back(loc_final);
          return;
     }

     float dx = loc_final.x-loc_start.x;
     float dy = loc_final.y-loc_start.y;
     float dz = loc_final.z-loc_start.z;
     // To be used in distance from point to line calculation below
     float dd = std::sqrt(dx*dx + dy*dy + dz*dz);
     // Find point with max distance between it and v1
     float max_dist_squared = -1.0f;
     int max_ind = -1;
     for (int j=start_i+1; j<end_i; j++)
     {
          XYZ & loc_this = loc_vec.at(j);
          // Compute distance from point at j to line between loc_start and loc_final
          Vector3D v1 = Vector3D(loc_this.x-loc_start.x, loc_this.y-loc_start.y, loc_this.z-loc_start.z);
          Vector3D v2 = Vector3D(loc_this.x-loc_final.x, loc_this.y-loc_final.y, loc_this.z-loc_final.z);
          Vector3D v3 = v1.cross(v2);
          float this_dist_sqaured = v3.normSquared();
          if (this_dist_sqaured > max_dist_squared)
          {
               max_dist_squared = this_dist_sqaured;
               max_ind = j;
          }
     }
     // Calculate actual max distance and compare to epsilon
     bool within_epsilon = false;
     if (dd > 0) // avoid divide by zero
     {
          float max_dist = std::sqrt(max_dist_squared) / dd;
          within_epsilon = (max_dist <= epsilon);
     }
     if (within_epsilon)
     {
          // If within epsilon, we can safely skip the points in between. Just return first and last points.
          loc_vec_resampled.push_back(loc_start);
          loc_vec_resampled.push_back(loc_final);
     }
     else
     {
          // If outside of epsilon, sample recursively between the two segments: start_i -> max_ind and max_indx -> end_i
          vector <XYZ> loc_vec_resampled1, loc_vec_resampled2;
          recursiveRamerDouglasPeucker(loc_vec, loc_vec_resampled1, start_i, max_ind, epsilon);
          recursiveRamerDouglasPeucker(loc_vec, loc_vec_resampled2, max_ind, end_i, epsilon);
          // Quickly join the two vectors, removing last element of vec1 (which would be duplicated)
          loc_vec_resampled1.pop_back();
          loc_vec_resampled.reserve(loc_vec_resampled1.size() + loc_vec_resampled2.size());
          loc_vec_resampled.insert(loc_vec_resampled.end(), loc_vec_resampled1.begin(), loc_vec_resampled1.end());
          loc_vec_resampled.insert(loc_vec_resampled.end(), loc_vec_resampled2.begin(), loc_vec_resampled2.end());
     }
     
}

/**
 * @brief This function is based on findNearestNeuronNode_WinXY(int cx, int cy,
 *  NeuronTree* ptree).
 */
V3DLONG Renderer_gl1::findNearestNeuronNode_Loc(XYZ &loc, NeuronTree *ptree)
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

		double cur_dist = (loc.x-ix)*(loc.x-ix)+(loc.y-iy)*(loc.y-iy)+(loc.z-iz)*(loc.z-iz);
		if (i==0) {	best_dist = cur_dist; best_ind=0; }
		else {	if (cur_dist<best_dist) {best_dist=cur_dist; best_ind = i;}}
	}

	return best_ind; //by PHC, 090209. return the index in the SWC file
}

V3DLONG Renderer_gl1::findNearestMarker_Loc(XYZ &loc, QList<LocationSimple> &listlandmarks,int mode)
{
    if(listlandmarks.empty()) return -1;

    GLdouble ix , iy , iz;
    V3DLONG best_ind = -1;double best_dist = 100000;
    bool b_valid = false;
    for(V3DLONG i = 0; i < listlandmarks.size(); ++i)
    {
        b_valid = false;
        if(mode == 1)//for special marker
        {
            if(listlandmarks[i].category == 77)
                b_valid = true;
        }
        else
        {
            b_valid = true;
        }

        if(b_valid)
        {
            ix = listlandmarks[i].x - 1;
            iy = listlandmarks[i].y - 1;
            iz = listlandmarks[i].z - 1;
            double cur_dist = (loc.x-ix)*(loc.x-ix)+(loc.y-iy)*(loc.y-iy)+(loc.z-iz)*(loc.z-iz);
            if(i == 0)
            {
                best_dist = cur_dist;
                best_ind = 0;
            }
            else
            {
                if(cur_dist < best_dist)
                {
                    best_dist = cur_dist;
                    best_ind = i;
                }
            }
        }
    }

    return best_ind; //by XZ, 20190720, return the index of the listlandmarks
}


double Renderer_gl1::solveCurveMarkerLists_fm(vector <XYZ> & loc_vec_input,  //used for marker-pool type curve-generating
                                            vector <XYZ> & loc_vec,  //the ouput 3D curve
                                            int index //the idnex to which stroke this function is computing on, assuming there may be multiple strokes
                                            )
{
    QTime t;
    t.start();

	bool b_use_seriespointclick = (loc_vec_input.size()>0) ? true : false;
    if (b_use_seriespointclick==false && list_listCurvePos.size()<1)  return -1;
    if (index < 0 || index>=list_listCurvePos.size())
    {
        v3d_msg("The index variable in solveCurveMarkerLists_fm() is incorrect. Check your program.\n");
      return -1;//by PHC
    }
    else
    {
        if (list_listCurvePos.at(index).size()<=0)
        {
            v3d_msg("You enter an empty curve for solveCurveMarkerLists_fm(). Check your code.\n");
            return -1;
        }
    }

	bool b_use_last_approximate=true;

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;
     if (w)
     {
         curImg = v3dr_getImage4d(_idep);
         if (!curImg)
         {
             v3d_msg("The original tri-view data has been removed. Thus this function is disabled.");
             return -1;
         }

         if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
         {
             deleteMultiNeuronsByStroke();
			 cout << "call delete muti seg pos 1" << endl;

             curImg->tracedNeuron.deleteMultiSeg();

         }
     }

#ifndef test_main_cpp
	MainWindow* V3Dmainwindow = 0;
	V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);
	if (V3Dmainwindow)
		b_use_last_approximate = V3Dmainwindow->global_setting.b_3dcurve_inertia;
#endif

	int chno = checkCurChannel();
	if (chno<0 || chno>dim4-1)   chno = 0; //default first channel
	qDebug()<<"\n solveCurveMarkerLists_fm: 3d curve in channel # "<<((chno<0)? chno :chno+1);

	loc_vec.clear();

     V3DLONG szx = curImg->getXDim();
     V3DLONG szy = curImg->getYDim();
     V3DLONG szz = curImg->getZDim();

     // set pregress dialog
     PROGRESS_DIALOG( "Curve creating", widget);
     PROGRESS_PERCENT(10);

     // get img data pointer for fastmarching
     unsigned char* pImg = 0;
     int datatype = curImg->getDatatype();

     if (curImg && data4dp && chno>=0 &&  chno <dim4)
     {
          unsigned short *pIntImg = 0;
          float *pFloatImg = 0;
          switch (datatype)
          {
               case V3D_UINT8:
                    pImg = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1);
                    break;
               case V3D_UINT16:
                    pIntImg = (unsigned short *) (data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(unsigned short));
                    pImg = new unsigned char [dim3*dim2*dim1];
                    converting_to_8bit<unsigned short, unsigned char>((unsigned short*)pIntImg, pImg, dim3*dim2*dim1);
                    break;
               case V3D_FLOAT32:
                    pFloatImg = (float*) (data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(float));
                    pImg = new unsigned char [dim3*dim2*dim1];
                    converting_to_8bit<float, unsigned char>((float*)pFloatImg, pImg, dim3*dim2*dim1);
                    break;
               default:
                    v3d_msg("Unsupported data type found. You should never see this.", 0);
                    return t.elapsed();
          }
     }else
     {
          v3d_msg("No data available. You should never see this.", 0);
          return t.elapsed();
     }

     // get subvolume from stroke for FM
     XYZ sub_orig;
     double* pSubdata;
     V3DLONG sub_szx, sub_szy, sub_szz;
     bool b_useStrokeBB  = true;           // use the stroke decided BB
     bool b_use2PointsBB = !b_useStrokeBB; // use the two-point decided BB
     bool b_useTiltedBB  = false;             // use tilted BB
     bool b_useSerialBBox=false; //added by PHC 20120405
     bool b_useTiltedBB_or_Global=false;//added by jsd 20180505. global and bbox mode for the adjustment of intensity threshold

     if(selectMode == smCurveUseStrokeBB_fm)
     {
          b_useStrokeBB = true;
          b_use2PointsBB = !b_useStrokeBB;
          b_useTiltedBB =  !b_useStrokeBB;
     }
     if(selectMode==smCurveTiltedBB_fm||selectMode==smCurveTiltedBB_fm_sbbox)
     {
         b_useTiltedBB_or_Global=true;
     }

     if(selectMode == smCurveTiltedBB_fm || selectMode == smCurveTiltedBB_fm_sbbox 
        || selectMode==smMarkerCreate1Curve //by PHC 20121012
        || selectMode == smCurveEditExtendOneNode || selectMode == smCurveEditExtendTwoNode
        || selectMode == smCurveCreate_MarkerCreate1_fm || selectMode == smCurveCreate_MarkerCreate1) //by ZMS 20151203
     {
          b_useTiltedBB = true;
          b_useStrokeBB = false;
          b_use2PointsBB = false;

         b_useSerialBBox = (selectMode == smCurveTiltedBB_fm_sbbox || selectMode==smMarkerCreate1Curve || selectMode == smCurveEditExtendOneNode
                            || selectMode == smCurveEditExtendTwoNode || selectMode == smCurveCreate_MarkerCreate1_fm || selectMode == smCurveCreate_MarkerCreate1)? //PHC 20121012
            true : false;
     }

     vector<MyMarker> nearpos_vec, farpos_vec; // for near/far locs testing
     nearpos_vec.clear();
     farpos_vec.clear();

     if(b_useStrokeBB)
          getSubVolFromStroke(pSubdata, chno, sub_orig, sub_szx, sub_szy, sub_szz);

	int N = loc_vec_input.size();
	if (b_use_seriespointclick)
	{
		loc_vec = loc_vec_input;
	}
	else //then use the moving mouse location, otherwise using the preset loc_vec_input (which is set by the 3d-curve-by-point-click function)
	{
		N = list_listCurvePos.at(index).size(); // change from 0 to index for different curves, ZJL
		int firstPointIndex = 0;

        // resample curve strokes
        vector<int> inds; // reserved stroke index
        resampleCurveStrokes(0, chno, inds);

        if(b_useTiltedBB)
        {
			for (firstPointIndex = 0; firstPointIndex < N; firstPointIndex++)
			{
				NearFarPoints pts = markerPosToNearFarLocs(index, firstPointIndex);

				if (pts.valid)
				{
					// marker pos was inside the image volume, start here
					nearpos_vec.push_back(MyMarker(pts.near_pt.x, pts.near_pt.y, 
						pts.near_pt.z));
					farpos_vec.push_back(MyMarker(pts.far_pt.x, pts.far_pt.y,
						pts.far_pt.z));

					break;
				}
				else
				{
					// marker pos wasn't inside the image value, skip it
					continue;
				}
			}
        }

        int last_i; // used for computing 2points_bb
        for (int i=firstPointIndex + 1; i<N; i++) // 0 must be in
        {
			// check whether i is in inds
			bool b_inds=false;

			if(inds.empty())
			{
				b_inds=true;
			}
			else
			{
				for(int ii=1; ii<inds.size(); ii++)
				{
					if(i == inds.at(ii))
					{
						b_inds=true;
						break;
					}
				}
			}

			// only process resampled strokes
			if(i==1 || i==(N-1) || b_inds) // make sure to include the last N-1 pos
			{
				const MarkerPos & pos = list_listCurvePos.at(index).at(i); // change from 0 to index for different curves, ZJL
				double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
				clipplane[3] = viewClip;
				ViewPlaneToModel(pos.MV, clipplane);

				// this is intersection points with view volume
				XYZ loc0_t, loc1_t;
				_MarkerPos_to_NearFarPoint(pos, loc0_t, loc1_t);

				//get intersection points with data volume
				IntersectResult<2> intersect = intersectPointsWithData(loc0_t, loc1_t);
				
				if (!intersect.success[0] || !intersect.success[1])
				{
					// marker position is outside the image volume; skip this point
					continue;
				}

				XYZ &loc0 = intersect.hit_locs[0], &loc1 = intersect.hit_locs[1];

				// near/far pos locs for b_useTitltedBB
				if(b_useTiltedBB) // still not finished
				{
					nearpos_vec.push_back(MyMarker(loc0.x, loc0.y, loc0.z));
					farpos_vec.push_back(MyMarker(loc1.x, loc1.y, loc1.z));
				}
				else// beginning of non b_useTitltedBB
				{
					float length01 = dist_L2(loc0, loc1);
					// preparing the two-markerpos decided boundingbox

					int last_j = loc_vec.size()-1;

					vector<MarkerPos> pos_vec;
					double* pSubdata2;
					XYZ sub_orig2;
					V3DLONG sub_szx2, sub_szy2, sub_szz2;
					BoundingBox bb_2Points;
					XYZ loci0; //meanshift point for i==0
					if(i==1) //only do this for the first point
					{
						//the logic of this section need change. no need to use meanshift before the testing of the potential intersection point with existing curves

						const MarkerPos & pos_0 = list_listCurvePos.at(index).at(0); //get the first point, note that already check list_listCurvePos.at(index) has at least one node
						XYZ nearest_loc;
						if( pickSeedpointFromExistingCurves(pos_0, nearest_loc) ) // if there is a nearest curve, use the nearest loc as the start point
						{
							loci0 = nearest_loc;
							v3d_msg("Use the existing curve point as starting location.\n",0);
						}
						else //use mean-shift
						{
							NearFarPoints pts = markerPosToNearFarLocs(index, 0);

							assert(pts.valid);

							loci0 = getCenterOfLineProfile(pts.near_pt, pts.far_pt, clipplane, chno);
						}

						last_i=0; // for the first time run
					}

					if(b_use2PointsBB)
					{
						// using 2 points on stroke to get BB
						pos_vec.clear();
						MarkerPos pos_last = list_listCurvePos.at(index).at(last_i);
						pos_vec.push_back(pos_last);
						pos_vec.push_back(pos);

						XYZ max_loc2;
						getSubVolFrom2MarkerPos(pos_vec, chno, pSubdata2, sub_orig2, max_loc2, sub_szx2, sub_szy2, sub_szz2);
						bb_2Points = BoundingBox(sub_orig2, max_loc2);

						// update last_i for the next loop
						last_i = i;
					}

					XYZ lastpos;
					vector<MyMarker> sub_markers; sub_markers.clear();
					vector<MyMarker> tar_markers; tar_markers.clear();
					vector<MyMarker*> outswc;     outswc.clear();

					if (i==1)//
					{
						XYZ loci = loci0;

						if(b_useStrokeBB)
							loci = loci-sub_orig;
						else if(b_use2PointsBB)
						{
							if(selectMode == smCurveFrom1Marker_fm)
							{
								loci.x = curveStartMarker.x-1; loci.y = curveStartMarker.y-1; loci.z = curveStartMarker.z-1;
								loci = loci-sub_orig2;
							}
							else
							{
								loci = loci-sub_orig2;
							}
						}
						sub_markers.push_back(MyMarker(loci.x, loci.y, loci.z));

						// get the loc with a random middle loc
						//getMidRandomLoc(pos, chno, loc);
						//middle_vec.push_back(loc);
					}
					else
					{
						lastpos = loc_vec.at(last_j);
						// sub_markers is the lastpos
						if(b_useStrokeBB) // use stroke bounding box
							lastpos = lastpos-sub_orig;
						else if(b_use2PointsBB)
							lastpos = lastpos-sub_orig2;

						sub_markers.push_back(MyMarker(lastpos.x, lastpos.y, lastpos.z));
					} // end of preparing sub_markers

					// preparing tar_markers
					float length = dist_L2(loc0, loc1);
					if (length<1.0)
					{
						XYZ loci=(loc0+loc1)/2.0;
						if(b_useStrokeBB) // use stroke bounding box
							loci = loci-sub_orig;
						else if(b_use2PointsBB)
							loci = loci-sub_orig2;

						tar_markers.push_back(MyMarker(loci.x, loci.y, loci.z));
					}
					else
					{
						XYZ v_1_0 = loc1-loc0;
						XYZ D = v_1_0; normalize(D);
						for(int ii=0; ii<(int)(length+0.5); ii++)
						{
							XYZ loci = loc0 + D*ii; // incease 1 each step

							if(b_useStrokeBB)
							{
								loci = loci-sub_orig; // use stroke bounding box
								tar_markers.push_back(MyMarker(loci.x, loci.y, loci.z));
							}
							else if(b_use2PointsBB)
							{
								if(bb_2Points.isInner(loci, 0))
								{
									loci = loci-sub_orig2;
									tar_markers.push_back( MyMarker(loci.x, loci.y, loci.z));
								}
							}
							else
							{
								tar_markers.push_back(MyMarker(loci.x, loci.y, loci.z));
							}

						}

					} // end of tar_markers


					// call fastmarching
					// using time spent on each step to decide whether the tracing in this step is acceptable.
					// if time is over time_thresh, then break and use center method
					// I found that the result is not so good when using this time limit
					XYZ loc;
					if (b_useStrokeBB)  // using stroke to creating a bounding box and do FM
					{
						fastmarching_linker(sub_markers, tar_markers, pSubdata, outswc, sub_szx, sub_szy, sub_szz);
						PROCESS_OUTSWC_TO_CURVE(outswc, sub_orig, i);
					}
					else if (b_use2PointsBB)  // using stroke to creating a bounding box and do FM
					{
						fastmarching_linker(sub_markers, tar_markers, pSubdata2, outswc, sub_szx2, sub_szy2, sub_szz2);
						PROCESS_OUTSWC_TO_CURVE(outswc, sub_orig2, i);
					}
					else  // This version uses full image as the bounding box
					{
						fastmarching_linker(sub_markers, tar_markers, pImg, outswc, szx, szy, szz);
						XYZ sub_orig = XYZ(0,0,0);
						PROCESS_OUTSWC_TO_CURVE(outswc, sub_orig, i);
					}

					//always remember to free the potential-memory-problematic fastmarching_linker return value
					CLEAN_FM_MARKER_VECTOR(outswc);

					if(pSubdata2) {delete []pSubdata2; pSubdata2=0;}
				} // end of non b_useTitltedBB
			} // end of if(i==1 || i==(N-1) || b_inds)
        } // end of for (int i=1; i<N; i++)
        // clean pSubdata of subvolume boundingbox
        if (b_useStrokeBB){ if(pSubdata) {delete [] pSubdata; pSubdata=0;} }

        // using titled BB for curve
        if(b_useTiltedBB)
        {
             vector<MyMarker *> outswc;
             bool b_res;

             //ZMS 20160208 fake a vector to insert to the beginning and the end of the fastmarcher for extend/connect
             QList<NeuronTree>::iterator j;
             NeuronSWC cur_node;

             if(selectMode == smCurveEditExtendOneNode || selectMode == smCurveEditExtendTwoNode){
                 for ( j = listNeuronTree.begin(); j != listNeuronTree.end(); ++j){
                     QList <NeuronSWC> p_listneuron = j->listNeuron;

                     for (int i=0; i<p_listneuron.size(); i++)
                     {
                         if(i == highlightedNode){
                             cur_node = p_listneuron.at(i);
                             XYZ cur_node_xyz = XYZ(cur_node.x, cur_node.y, cur_node.z);
                             nearpos_vec.insert(nearpos_vec.begin(), MyMarker(cur_node.x, cur_node.y, cur_node.z + 1));
                             farpos_vec.insert(farpos_vec.begin(), MyMarker(cur_node.x, cur_node.y, cur_node.z - 1));
                         }
                     }
                 }
             }

             if(selectMode == smCurveEditExtendTwoNode){
                 for ( j = listNeuronTree.begin(); j != listNeuronTree.end(); ++j){
                     QList <NeuronSWC> p_listneuron = j->listNeuron;

                     for (int i=0; i<p_listneuron.size(); i++)
                     {
                         if(i == highlightedEndNode){
                             cur_node = p_listneuron.at(i);
                             XYZ cur_node_xyz = XYZ(cur_node.x, cur_node.y, cur_node.z);
                             nearpos_vec.push_back(MyMarker(cur_node.x, cur_node.y, cur_node.z + 1));
                             farpos_vec.push_back(MyMarker(cur_node.x, cur_node.y, cur_node.z - 1));
                         }
                     }
                 }
             }

             // all pImg are unsigned char now
             int modetest=0;//by shengdianjiang.20180505.add a shortcut of intensity threshold adjustment
             if(QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)&&b_useTiltedBB_or_Global)
             {
                 modetest=1;
             }
             if(modetest==0)
                 b_res = (b_useSerialBBox) ?
                  fastmarching_drawing_serialbboxes(nearpos_vec, farpos_vec, (unsigned char*)pImg, outswc, szx, szy, szz, 1, 5)
                  : fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)pImg, outswc, szx, szy, szz, 1, 5);
             else if(modetest==1)
             {
                 b_res=(b_useSerialBBox)?
                             fastmarching_drawing_serialbboxes(nearpos_vec, farpos_vec, (unsigned char*)pImg, outswc, szx, szy, szz, 1, 5,true)
                             : fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)pImg, outswc, szx, szy, szz, 1, 5,true);
             }

             // delete pImg created for two datatypes
             if(datatype == V3D_UINT16 || datatype == V3D_FLOAT32)
             {
                  if(pImg) {delete [] pImg; pImg=0;}
             }

             // switch (datatype)
             // {
             //      case V3D_UINT8:
             //           b_res = (b_useSerialBBox) ?
             //                fastmarching_drawing_serialbboxes(nearpos_vec, farpos_vec, (unsigned char*)pImg, outswc, szx, szy, szz, 1, 5)
             //                : fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)pImg, outswc, szx, szy, szz, 1, 5);
             //           break;
             //      case V3D_UINT16:
             //           b_res = (b_useSerialBBox) ?
             //                fastmarching_drawing_serialbboxes(nearpos_vec, farpos_vec, (short int*)pImg, outswc, szx, szy, szz, 1, 5)
             //                : fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (short int*)pImg, outswc, szx, szy, szz, 1, 5);
             //           break;
             //      case V3D_FLOAT32:
             //           b_res = (b_useSerialBBox) ?
             //                fastmarching_drawing_serialbboxes(nearpos_vec, farpos_vec, (float*)pImg, outswc, szx, szy, szz, 1, 5)
             //                : fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (float*)pImg, outswc, szx, szy, szz, 1, 5);
             //           break;
             //      default:
             //           v3d_msg("Unsupported data type found. You should never see this.", 0);
             //           return t.elapsed();
             // }

            //  b_res = (b_useSerialBBox) ?
            //       fastmarching_drawing_serialbboxes(nearpos_vec, farpos_vec, pImg, outswc, szx, szy, szz, 1, 5) //replace the above method, 20120405, PHC
            //     //fastmarching_drawing5(nearpos_vec, farpos_vec, pImg, outswc, szx, szy, szz, 1) //replace the above method, 20120405, PHC
            // : fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, pImg, outswc, szx, szy, szz, 1, 5); // 20120405, PHC

            if (!b_res)
            {
                  v3d_msg("Error in creating the curve", 0);
             }

            //if fail, should not add curve? 120405, PHC?
             XYZ sub_orig = XYZ(0,0,0);
             PROCESS_OUTSWC_TO_CURVE(outswc, sub_orig, 1);
             //always remember to free the potential-memory-problematic fastmarching_linker return value
             CLEAN_FM_MARKER_VECTOR(outswc);
        }// end of b_useTitltedBB
        //clean pSubdata of subvolume boundingbox
        if(b_useStrokeBB) {if(pSubdata) {delete [] pSubdata; pSubdata=0;}}
    }

    // Save near/far locs for testing:
if (0)
{
     v3d_msg("Write near/far marker to files.");
    FILE *nfile=fopen("near.marker", "wt");
     for(int ii=0; ii<nearpos_vec.size(); ii++)
          fprintf(nfile, "%f,%f,%f,5,1,,\n", nearpos_vec.at(ii).x+1, nearpos_vec.at(ii).y+1, nearpos_vec.at(ii).z+1);
     fclose(nfile);
     FILE *ffile=fopen("far.marker", "wt");
     for(int ii=0; ii<farpos_vec.size(); ii++)
          fprintf(ffile, "%f,%f,%f,5,1,,\n", farpos_vec.at(ii).x+1, farpos_vec.at(ii).y+1, farpos_vec.at(ii).z+1);
     fclose(ffile);
}

     PROGRESS_PERCENT(90);

     N = loc_vec.size();
     if(N<1) return t.elapsed(); // all points are outside the volume. ZJL 110913

#ifndef test_main_cpp
	// check if there is any existing neuron node is very close to the starting and ending points, if yes, then merge
	if (V3Dmainwindow && V3Dmainwindow->global_setting.b_3dcurve_autoconnecttips && b_use_seriespointclick==false &&
          (selectMode == smCurveMarkerLists_fm || selectMode == smCurveRefine_fm || selectMode == smCurveFrom1Marker_fm ||
               selectMode == smCurveUseStrokeBB_fm || selectMode == smCurveTiltedBB_fm || selectMode==smCurveTiltedBB_fm_sbbox
           || selectMode == smCurveEditExtendOneNode || selectMode == smCurveEditExtendTwoNode) ) //ZMS 20151203
	{
		if (listNeuronTree.size()>0 && curEditingNeuron>0 && curEditingNeuron<=listNeuronTree.size())
		{
			NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(curEditingNeuron-1)));

            bool b_startnodeisspecialmarker=false;

            float ratio = getZoomRatio();
            int resindex = tf::PluginInterface::getRes();

            double th_times = (7-resindex); //adaptive th_times, by XZ, 20190721

            My4DImage* image4d = v3dr_getImage4d(_idep);
            if(image4d)
            {
                QList <LocationSimple> & listLoc = image4d->listLandmarks;
                int mode=1;
                V3DLONG marker_id_start = findNearestMarker_Loc(loc_vec.at(0),listLoc,mode);
                V3DLONG marker_id_end = findNearestMarker_Loc(loc_vec.at(N-1),listLoc,mode);
                qDebug("detect nearest special marker [%ld] for the curve-start and marker [%ld] for curve-end",marker_id_start,marker_id_end);

                double th_merge_m = 7;

                bool b_start_merged = false, b_end_merged = false;

                if(marker_id_start>=0)
                {
                    XYZ cur_node_xyz = XYZ(listLoc.at(marker_id_start).x-1, listLoc.at(marker_id_start).y-1, listLoc.at(marker_id_start).z-1);
                    qDebug()<<cur_node_xyz.x<<" "<<cur_node_xyz.y<<" "<<cur_node_xyz.z;
                    qDebug()<<"th_merge_m: "<<th_merge_m<<endl;
                    qDebug()<<"dist: "<<dist_L2(cur_node_xyz,loc_vec.at(0))<<endl;
                    if (dist_L2(cur_node_xyz, loc_vec.at(0))<th_merge_m)
                    {
                        loc_vec.at(0) = cur_node_xyz;
                        b_startnodeisspecialmarker = true;
                        b_start_merged = true;
                        qDebug()<<"force set the first point of this curve to the above marker node as they are close.";
                    }
                }

                if(marker_id_end>=0)
                {
                    XYZ cur_node_xyz = XYZ(listLoc.at(marker_id_end).x-1, listLoc.at(marker_id_end).y-1, listLoc.at(marker_id_end).z-1);
                    qDebug()<<cur_node_xyz.x<<" "<<cur_node_xyz.y<<" "<<cur_node_xyz.z;
                    qDebug()<<"th_merge_m: "<<th_merge_m<<endl;
                    qDebug()<<"dist: "<<dist_L2(cur_node_xyz,loc_vec.at(N-1))<<endl;
                    if (dist_L2(cur_node_xyz, loc_vec.at(N-1))<th_merge_m)
                    {
                        loc_vec.at(N-1) = cur_node_xyz;
                        b_startnodeisspecialmarker = true;
                        b_end_merged = true;
                        qDebug()<<"force set the last point of this curve to the above marker node as they are close.";
                    }
                }

                if(b_start_merged==false && b_end_merged==true)
                {
                    vector <XYZ> loc_vec_tmp = loc_vec;
                    for (int i=0;i<N;i++)
                        loc_vec.at(i) = loc_vec_tmp.at(N-1-i);
                }

                if(b_startnodeisspecialmarker)
                {
                    QList <LocationSimple> ::iterator it=listLoc.begin();
                    V3DLONG marker_index = (b_start_merged)?marker_id_start:marker_id_end;
                    listLoc.erase(it+marker_index);
                    XYZ loc(loc_vec.at(N-1).x,loc_vec.at(N-1).y,loc_vec.at(N-1).z);
                    addSpecialMarker(loc);
                }

            }//add code to deal with the special marker problem, by XZ, 20190720


            if (p_tree&&b_startnodeisspecialmarker==false)
			{
                    // at(0) to at(index) ZJL 110901
                    V3DLONG n_id_start = findNearestNeuronNode_Loc(loc_vec.at(0), p_tree);
                    V3DLONG n_id_end = findNearestNeuronNode_Loc(loc_vec.at(N-1), p_tree);
                    qDebug("detect nearest neuron node [%ld] for curve-start and node [%ld] for curve-end for the [%d] neuron", n_id_start, n_id_end, curEditingNeuron);


//                    qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
//                    for(V3DLONG i=0;i<N;++i)
//                    {
//                        qDebug()<<"loc_vec "<<i<<" : "<<loc_vec[i].x<<" "<<loc_vec[i].y<<" "<<loc_vec[i].z<<endl;
//                    }
//                    qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;


                double th_merge = 5;//*th_times;

                bool b_start_merged=false, b_end_merged=false;
                NeuronSWC cur_node;
                NeuronSWC selected_node;

				if (n_id_start>=0)
				{
                    if(selectMode == smCurveEditExtendOneNode || selectMode == smCurveEditExtendTwoNode){
                        QList<NeuronTree>::iterator j;
                        for (j = listNeuronTree.begin(); j != listNeuronTree.end(); ++j){
                            QList <NeuronSWC> p_listneuron = j->listNeuron;//curr_renderer->listNeuronTree.at(0).listNeuron;//&(ptree->listNeuron);

                            for (int i=0; i<p_listneuron.size(); i++)
                            {
                                if(i == highlightedNode){
                                    cur_node = p_listneuron.at(i);
                                    XYZ cur_node_xyz = XYZ(cur_node.x, cur_node.y, cur_node.z);
                                    loc_vec.at(0) = cur_node_xyz;
                                    b_start_merged = true;
                                    qDebug()<<"force set the first point of this curve to the above neuron node due to extend mode.";
                                }
                            }
                        }
                    }else{
                        cur_node = p_tree->listNeuron.at(n_id_start);
                        qDebug()<<cur_node.x<<" "<<cur_node.y<<" "<<cur_node.z;
                        XYZ cur_node_xyz = XYZ(cur_node.x, cur_node.y, cur_node.z);
                        if (dist_L2(cur_node_xyz, loc_vec.at(0))<th_merge) //Last two conditions ZMS 20151203
                        {
                            loc_vec.at(0) = cur_node_xyz;
                            b_start_merged = true;
                            selected_node = cur_node;
                            qDebug()<<"force set the first point of this curve to the above neuron node as they are close.";
                        }
                    }
				}

				if (n_id_end>=0)
				{
                    if(selectMode == smCurveEditExtendTwoNode){
                        QList<NeuronTree>::iterator j;
                        for (j = listNeuronTree.begin(); j != listNeuronTree.end(); ++j){
                            QList <NeuronSWC> p_listneuron = j->listNeuron;//curr_renderer->listNeuronTree.at(0).listNeuron;//&(ptree->listNeuron);

                            for (int i=0; i<p_listneuron.size(); i++)
                            {
                                if(i == highlightedEndNode){
                                    cur_node = p_listneuron.at(i);
                                    // If connecting to an undefined node, give that segment this type if defined
                                    if (cur_node.type == 0)
                                    {
                                        int type_to_check = (highlightedNodeType >= 0) ? highlightedNodeType : currentTraceType;
                                        if (type_to_check > 0) // type is defined, retype the undefined segment
                                        {
                                            change_type_in_seg_of_V_NeuronSWC_list(curImg->tracedNeuron, cur_node.seg_id, type_to_check);
                                            // TODO: recurse, any addional segments extending from this seg_id that have unknown type should be typed too
                                        }
                                    }
                                    XYZ cur_node_xyz = XYZ(cur_node.x, cur_node.y, cur_node.z);
                                    loc_vec.at(N-1) = cur_node_xyz;
                                    b_start_merged = true;
                                    highlightedEndNode = -1; //Prevent this node from being highlighted
                                    qDebug()<<"force set the first point of this curve to the above neuron node due to extend mode.";
                                }
                            }
                        }
                    }else{
                        cur_node = p_tree->listNeuron.at(n_id_end);
                        qDebug()<<cur_node.x<<" "<<cur_node.y<<" "<<cur_node.z;
                        XYZ cur_node_xyz = XYZ(cur_node.x, cur_node.y, cur_node.z);
                        if (dist_L2(cur_node_xyz, loc_vec.at(N-1))<th_merge) //Last condition ZMS20151203
                        {
                            loc_vec.at(N-1) = cur_node_xyz;
                            b_end_merged = true;
                            selected_node = cur_node;
                            qDebug()<<"force set the last point of this curve to the above neuron node as they are close.";

                        }
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

                if (b_start_merged || b_end_merged)
                {
                    if (selected_node.seg_id>=0 && selected_node.seg_id< curImg->tracedNeuron.seg.size())
                        if(curImg->tracedNeuron.seg[selected_node.seg_id].row.size() >0)
                            currentTraceType = curImg->tracedNeuron.seg[selected_node.seg_id].row[0].type;
                }
			}
		}
	}

	if (b_use_seriespointclick==false)
		smooth_curve(loc_vec, 5);
#endif

     // adaptive curve simpling
     vector <XYZ> loc_vec_resampled;
#ifdef DEBUG_RESAMPLING
     vector <XYZ> loc_vec_resampled_old;
     int stepsize = 7; // sampling stepsize 5
     adaptiveCurveResampling(loc_vec, loc_vec_resampled_old, stepsize); //this function should be the source of the redundant intermediate points
#endif
     // TODO: investigate best value for epsilon, perhaps based on current voxel size. For now, use 0.2
     adaptiveCurveResamplingRamerDouglasPeucker(loc_vec, loc_vec_resampled, 0.2f);
     //loc_vec_resampled = loc_vec;

	//the intensity-based resampled method could lead to totally wrong path (especially for binary image).
	//Need to use a better and more evenly spaced method. by PHC, 20120330.

     if(selectMode == smCurveMarkerLists_fm || selectMode == smCurveRefine_fm || selectMode == smCurveFrom1Marker_fm ||
          selectMode == smCurveTiltedBB_fm || selectMode == smCurveTiltedBB_fm_sbbox || selectMode == smCurveUseStrokeBB_fm
             || selectMode == smCurveEditExtendOneNode || selectMode == smCurveEditExtendTwoNode) //by ZMS 20151203
     {
          if (b_addthiscurve)
          {
			  cout << "addcurve pos mode 2" << endl;
               addCurveSWC(loc_vec_resampled, chno, 2); //LMG 26/10/2018 solveCurveMarkerLists_fm (BBox/Draw Global tracing alt+B/alt+G) mode 2 <- BBox will be converted to 1 in addCurveSwc
               // used to convert loc_vec to NeuronTree and save SWC in testing
               vecToNeuronTree(testNeuronTree, loc_vec_resampled);
#ifdef DEBUG_RESAMPLING
               addCurveSWC(loc_vec_resampled_old, chno);
               vecToNeuronTree(testNeuronTree, loc_vec_resampled_old);
               addCurveSWC(loc_vec, chno);
               vecToNeuronTree(testNeuronTree, loc_vec);
               qDebug() << "Unsampled: " << loc_vec.size() << " points";
               qDebug() << "Old sampling: " << loc_vec_resampled_old.size() << " points";
               qDebug() << "New sampling: " << loc_vec_resampled.size() << " points";
#endif
          }
          else
          {
               b_addthiscurve = true; //in this case, always reset to default to draw curve to add to a swc instead of just  zoom
               endSelectMode();
          }
     }

    return t.elapsed();
}


/**
 * Resample curve strokes:
 * 1. compute max value on each stroke viewing direction, and get a vector of these max values
 * 2. sort this vector, and return stroke indexes that their max values are above 10%
*/
void  Renderer_gl1::resampleCurveStrokes(int index, int chno, vector<int> &ids)
{
     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if (w)
          curImg = v3dr_getImage4d(_idep);

	int N = list_listCurvePos.at(index).size();
     vector<double> maxval;
     maxval.clear();

     for (int i=0; i<N; i++)
     {
          const MarkerPos & pos = list_listCurvePos.at(index).at(i); // change from 0 to index for different curves, ZJL
          ////////////////////////////////////////////////////////////////////////
          //100730 RZC, in View space, keep for dot(clip, pos)>=0
          double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
          clipplane[3] = viewClip;
          ViewPlaneToModel(pos.MV, clipplane);
          //qDebug()<<"   clipplane:"<<clipplane[0]<<clipplane[1]<<clipplane[2]<<clipplane[3];
          ////////////////////////////////////////////////////////////////////////

          XYZ loc0, loc1;
          _MarkerPos_to_NearFarPoint(pos, loc0, loc1);

          // get max value on each (loc0,loc)
          XYZ D = loc1-loc0; normalize(D);

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
				return;
		}

          // directly use max value on the line
          // double length = norm(loc1-loc0);
          // float maxvali=0.0;

          // double step;
          // int nstep = int(length + 0.5);
          // if(length<0.5)
          //      step = 1.0;
          // else
          //      step = length/nstep;

          // for (int i=0; i<=nstep; i++)
          // {
          //      XYZ P;
          //      if(length<0.5)
          //           P = (loc0+loc1)*0.5;
          //      else
          //           P= loc0 + D*step*(i);
          //      float value;
          //      switch (curImg->getDatatype())
          //      {
          //           case V3D_UINT8:
          //                value = sampling3dAllTypesatBounding( vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
          //                break;
          //           case V3D_UINT16:
          //                value = sampling3dAllTypesatBounding( (short int *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
          //                break;
          //           case V3D_FLOAT32:
          //                value = sampling3dAllTypesatBounding( (float *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
          //                break;
          //           default:
          //                v3d_msg("Unsupported data type found. You should never see this.", 0);
          //                return;
          //      }

          //      if(value > maxvali)
          //           maxvali = value;
          // }


          // use value on center of mass for comparing
          XYZ P  = getCenterOfLineProfile(loc0, loc1, clipplane, chno);
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
                    return;
          }
          float maxvali=value;

          // for two approaches
          maxval.push_back(maxvali);
     }

     // using map to sort maxval
     map<double, int> max_score;
     for(int i=0; i<maxval.size(); i++)
     {
          max_score[maxval.at(i)] = i;
     }

     map<double, int>::reverse_iterator it;

     int count=0;
     for(it=max_score.rbegin(); it!=max_score.rend(); it++)
     {
          count++;
         if(count >= max_score.size()/1) //tentatively make it no downsampling. by PHC 20120405.
//          if(count >= max_score.size()/10.0)
               break;
          ids.push_back(it->second);
     }

}



// void  Renderer_gl1::resampleCurveStrokes2(int index, int chno, vector<int> &ids)
// {
// #define DIST
//      V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
//      My4DImage* curImg = 0;
//      if (w)
//           curImg = v3dr_getImage4d(_idep);

// 	int N = list_listCurvePos.at(index).size();
//      vector<double> maxval;
//      maxval.clear();


//      // push_back the first index
//      ids.push_back(0);

//      for (int i=0; i<N; i++)
//      {
//           const MarkerPos & pos = list_listCurvePos.at(index).at(i);


//           double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
//           clipplane[3] = viewClip;
//           ViewPlaneToModel(pos.MV, clipplane);

//           XYZ loc0, loc1;
//           _MarkerPos_to_NearFarPoint(pos, loc0, loc1);

//           // get max value on each (loc0,loc)
//           XYZ D = loc1-loc0; normalize(D);

// 		unsigned char* vp = 0;
// 		switch (curImg->getDatatype())
// 		{
// 			case V3D_UINT8:
// 				vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1);
// 				break;
// 			case V3D_UINT16:
// 				vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(short int);
// 				break;
// 			case V3D_FLOAT32:
// 				vp = data4dp + (chno + volTimePoint*dim4)*(dim3*dim2*dim1)*sizeof(float);
// 				break;
// 			default:
// 				v3d_msg("Unsupported data type found. You should never see this.", 0);
// 				return;
// 		}

//           // directly use max value on the line
//           // double length = norm(loc1-loc0);
//           // float maxvali=0.0;

//           // double step;
//           // int nstep = int(length + 0.5);
//           // if(length<0.5)
//           //      step = 1.0;
//           // else
//           //      step = length/nstep;

//           // for (int i=0; i<=nstep; i++)
//           // {
//           //      XYZ P;
//           //      if(length<0.5)
//           //           P = (loc0+loc1)*0.5;
//           //      else
//           //           P= loc0 + D*step*(i);
//           //      float value;
//           //      switch (curImg->getDatatype())
//           //      {
//           //           case V3D_UINT8:
//           //                value = sampling3dAllTypesatBounding( vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
//           //                break;
//           //           case V3D_UINT16:
//           //                value = sampling3dAllTypesatBounding( (short int *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
//           //                break;
//           //           case V3D_FLOAT32:
//           //                value = sampling3dAllTypesatBounding( (float *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
//           //                break;
//           //           default:
//           //                v3d_msg("Unsupported data type found. You should never see this.", 0);
//           //                return;
//           //      }

//           //      if(value > maxvali)
//           //           maxvali = value;
//           // }


//           // use value on center of mass for comparing
//           XYZ P  = getCenterOfLineProfile(loc0, loc1, clipplane, chno);
//           float value;
//           switch (curImg->getDatatype())
//           {
//                case V3D_UINT8:
//                     value = sampling3dAllTypesatBounding( vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
//                     break;
//                case V3D_UINT16:
//                     value = sampling3dAllTypesatBounding( (short int *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
//                     break;
//                case V3D_FLOAT32:
//                     value = sampling3dAllTypesatBounding( (float *)vp, dim1, dim2, dim3,  P.x, P.y, P.z, dataViewProcBox.box, clipplane);
//                     break;
//                default:
//                     v3d_msg("Unsupported data type found. You should never see this.", 0);
//                     return;
//           }
//           float maxvali=value;

//           // for two approaches
//           maxval.push_back(maxvali);
//      }

//      // using map to sort maxval
//      map<double, int> max_score;
//      for(int i=0; i<maxval.size(); i++)
//      {
//           max_score[maxval.at(i)] = i;
//      }

//      map<double, int>::reverse_iterator it;

//      int count=0;
//      for(it=max_score.rbegin(); it!=max_score.rend(); it++)
//      {
//           count++;
//           if(count >= max_score.size()/10.0)
//                break;
//           ids.push_back(it->second);
//      }

// }




void Renderer_gl1::solveCurveFromMarkersGD(bool b_customized_bb)
{
	qDebug("  Renderer_gl1::solveCurveMarkersGD");

	vector <XYZ> loc_vec_input;
     loc_vec_input.clear();

     // set pregress dialog
     PROGRESS_DIALOG( "Curve creating", widget);
     PROGRESS_PERCENT(10);

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;
     if (w) curImg = v3dr_getImage4d(_idep); //by PHC, 090119

     int chno = checkCurChannel();
     if (chno<0 || chno>dim4-1)   chno = 0; //default first channel

     int nn=listCurveMarkerPool.size();
     XYZ cur_xyz;
     int i;
     for (i=0;i<nn;i++)
     {
          cur_xyz.x = listCurveMarkerPool.at(i).x-1; //marker location is 1 based
          cur_xyz.y = listCurveMarkerPool.at(i).y-1;
          cur_xyz.z = listCurveMarkerPool.at(i).z-1;
          loc_vec_input.push_back(cur_xyz);
     }

     // Prepare paras for GD, using default paras
     CurveTracePara trace_para;
	{
		trace_para.sp_num_end_nodes = 1;
		trace_para.b_deformcurve = false;
	}

     ParaShortestPath sp_para;
	sp_para.edge_select       = trace_para.sp_graph_connect; // 090621 RZC: add sp_graph_connect selection
	sp_para.background_select = trace_para.sp_graph_background; // 090829 RZC: add sp_graph_background selection
	sp_para.node_step      = trace_para.sp_graph_resolution_step; // 090610 RZC: relax the odd constraint.
	sp_para.outsample_step = trace_para.sp_downsample_step;
	sp_para.smooth_winsize = trace_para.sp_smoothing_win_sz;

	vector< vector<V_NeuronSWC_unit> > mmUnit;

     V3DLONG szx = curImg->getXDim();
     V3DLONG szy = curImg->getYDim();
     V3DLONG szz = curImg->getZDim();
     curImg->trace_z_thickness = this->thicknessZ;

     // get bounding box
     XYZ minloc, maxloc;
	if (b_customized_bb == false)
	{
          curImg->trace_bounding_box = this->dataViewProcBox; //dataBox;
          minloc.x = curImg->trace_bounding_box.x0;
          minloc.y = curImg->trace_bounding_box.y0;
          minloc.z = curImg->trace_bounding_box.z0;

          maxloc.x = curImg->trace_bounding_box.x1;
          maxloc.y = curImg->trace_bounding_box.y1;
          maxloc.z = curImg->trace_bounding_box.z1;
	}
     else
     {
          bool res = boundingboxFromStroke(minloc, maxloc);
          if(!res)
          {
               curImg->trace_bounding_box = this->dataViewProcBox; //dataBox;
               minloc.x = curImg->trace_bounding_box.x0;
               minloc.y = curImg->trace_bounding_box.y0;
               minloc.z = curImg->trace_bounding_box.z0;

               maxloc.x = curImg->trace_bounding_box.x1;
               maxloc.y = curImg->trace_bounding_box.y1;
               maxloc.z = curImg->trace_bounding_box.z1;
          }
     }

     PROGRESS_PERCENT(40);

     // loc_vec is used to store final locs on the curve
     vector <XYZ> loc_vec;
     loc_vec.clear();

     if (loc_vec_input.size()>0)
	{
		qDebug("now get curve using GD find_shortest_path_graphimg >>> ");

          loc_vec.push_back(loc_vec_input.at(0)); // the first loc is always here
          for(int ii=0; ii<loc_vec_input.size()-1; ii++)
          {
               XYZ loc0=loc_vec_input.at(ii);
               XYZ loc1=loc_vec_input.at(ii+1);

               // call GD tracing
               const char* s_error = find_shortest_path_graphimg(curImg->data4d_uint8[chno], szx, szy, szz,
                    curImg->trace_z_thickness,
                    minloc.x, minloc.y, minloc.z,
                    maxloc.x, maxloc.y, maxloc.z,
                    loc0.x, loc0.y, loc0.z,
                    1,
                    &(loc1.x), &(loc1.y), &(loc1.z),
                    mmUnit,
                    sp_para);

               if (s_error)
               {
                    throw (const char*)s_error;
                    return;
               }

               // append traced result to loc_vec
               if(!mmUnit.empty())
               {
                    // only have one seg and get the first seg
                    vector<V_NeuronSWC_unit> seg0 = mmUnit.at(0);
                    int num = seg0.size();
                    for(int j=num-2; j>=0; j-- )
                    {
                         XYZ loc;
                         loc.x=seg0.at(j).x;
                         loc.y=seg0.at(j).y;
                         loc.z=seg0.at(j).z;

                         loc_vec.push_back(loc);
                    }
               }else
               {
                    loc_vec.push_back(loc1);
               }
          }

          PROGRESS_PERCENT(80);

          loc_vec.push_back(loc_vec_input.back());

          if(loc_vec.size()<1) return; // all points are outside the volume. ZJL 110913

          // check if there is any existing neuron node is very close to the starting and ending points, if yes, then merge

          MainWindow* V3Dmainwindow = 0;
          V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);

          int N = loc_vec.size();

          if (V3Dmainwindow && V3Dmainwindow->global_setting.b_3dcurve_autoconnecttips)
          {
               if (listNeuronTree.size()>0 && curEditingNeuron>0 && curEditingNeuron<=listNeuronTree.size())
               {
                    NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(curEditingNeuron-1)));
                    if (p_tree)
                    {
                         V3DLONG n_id_start = findNearestNeuronNode_Loc(loc_vec.at(0), p_tree);
                         V3DLONG n_id_end = findNearestNeuronNode_Loc(loc_vec.at(N-1), p_tree);
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
                         if (b_start_merged || b_end_merged)
                         {
                             if (cur_node.seg_id>=0 && cur_node.seg_id< curImg->tracedNeuron.seg.size())
                                 if(curImg->tracedNeuron.seg[cur_node.seg_id].row.size() >0)
                                     currentTraceType = curImg->tracedNeuron.seg[cur_node.seg_id].row[0].type;
                         }
                    }
               }
          }

          PROGRESS_PERCENT(90);

          smooth_curve(loc_vec, 5);

          // adaptive curve simpling
          vector <XYZ> loc_vec_resampled;
          int stepsize = 6; // sampling stepsize
          loc_vec_resampled.clear();
          adaptiveCurveResampling(loc_vec, loc_vec_resampled, stepsize);

          if (b_addthiscurve)
          {
			  cout << "addcurve pos mode 8" << endl;
               addCurveSWC(loc_vec_resampled, chno, 4); //LMG 26/10/2018 solveCurveFromMarkersGD mode 4
               // used to convert loc_vec to NeuronTree and save SWC in testing
               vecToNeuronTree(testNeuronTree, loc_vec_resampled);
          }
          else //100821
          {
               b_addthiscurve = true; //in this case, always reset to default to draw curve to add to a swc instead of just  zoom
               endSelectMode();
          }
     }
     PROGRESS_PERCENT(100);
}



// get the bounding box from a stroke
bool Renderer_gl1::boundingboxFromStroke(XYZ& minloc, XYZ& maxloc)
{

     int NC = list_listCurvePos.size();
	int N = list_listCurvePos.at(0).size();

	if (NC<1 || N <3)  return false; //data is not enough and use the whole image as the bounding box

     // find min-max of x y z in loc_veci
     float minx, miny, minz, maxx, maxy, maxz;
     minx = miny = minz = INF;
     maxx = maxy = maxz = 0;
     // Directly using stroke pos for minloc, maxloc
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

          if(minx>loc0.x) minx=loc0.x;
          if(miny>loc0.y) miny=loc0.y;
          if(minz>loc0.z) minz=loc0.z;

          if(maxx<loc0.x) maxx=loc0.x;
          if(maxy<loc0.y) maxy=loc0.y;
          if(maxz<loc0.z) maxz=loc0.z;

          if(minx>loc1.x) minx=loc1.x;
          if(miny>loc1.y) miny=loc1.y;
          if(minz>loc1.z) minz=loc1.z;

          if(maxx<loc1.x) maxx=loc1.x;
          if(maxy<loc1.y) maxy=loc1.y;
          if(maxz<loc1.z) maxz=loc1.z;
     }

     //
     int boundary = 5;

     minloc.x = minx - boundary; cout << minloc.x << endl;
     minloc.y = miny - boundary;
     minloc.z = minz - boundary;

     maxloc.x = maxx + boundary; cout << maxloc.x << endl;
     maxloc.y = maxy + boundary;
     maxloc.z = maxz + boundary;

     dataViewProcBox.clamp(minloc);
     dataViewProcBox.clamp(maxloc);

     return true;
}


void  Renderer_gl1::vecToNeuronTree(NeuronTree &SS, vector<XYZ> loc_list)
{

	QList <NeuronSWC> listNeuron;
	QHash <int, int>  hashNeuron;
	listNeuron.clear();
	hashNeuron.clear();

    int count = 0;

     qDebug("-------------------------------------------------------");
     for (int k=0;k<loc_list.size();k++)
     {
          count++;
          NeuronSWC S;

          S.n 	= 1+k;
          S.type 	= 3;
          S.x 	= loc_list.at(k).x;
          S.y 	= loc_list.at(k).y;
          S.z 	= loc_list.at(k).z;
          S.r 	= 1;
          S.pn 	= (k==0)? -1 : k;

          //qDebug("%s  ///  %d %d (%g %g %g) %g %d", buf, S.n, S.type, S.x, S.y, S.z, S.r, S.pn);
          {
               listNeuron.append(S);
               hashNeuron.insert(S.n, listNeuron.size()-1);
          }
     }

     SS.n = -1;
     RGBA8 cc;
     cc.r=0; cc.g=20;cc.b=200;cc.a=0;
     SS.color = cc; //random_rgba8(255);//RGBA8(255, 0,0,0);
     SS.on = true;

     SS.listNeuron = listNeuron;
     SS.hashNeuron = hashNeuron;

}

/*
 * Create random positions around pos. Then get loc using mean shift method on each pos.
 * Sort the depth of all locs and get the loc at the middle of the sorted locs as the return value.
*/
void  Renderer_gl1::getMidRandomLoc(MarkerPos pos, int chno, XYZ &mid_loc)
{
     int range=6;
     srand(clock()); // initialize random seek

     vector <XYZ> rand_loc_vec;
     rand_loc_vec.clear();

     double clipplane[4] = { 0.0,  0.0, -1.0,  0 };
     clipplane[3] = viewClip;
     ViewPlaneToModel(pos.MV, clipplane);
     XYZ loc0, loc1;
     _MarkerPos_to_NearFarPoint(pos, loc0, loc1);

     XYZ loc = getCenterOfLineProfile(loc0, loc1, clipplane, chno);
     float dist = dist_L2(loc0, loc);
     rand_loc_vec.push_back(loc);

     // using map to sort dist
     map<float, int> dist_score;
     dist_score[dist] = 0;

     for(int i=0; i<2*range; i++)
     {
          // generate pos and then get new loc
          int rand_x = rand()%(range+1); // generate value from 0~range
          int rand_y = rand()%(range+1); // generate value from 0~range
          // map rand from 0~range to -range/2~range/2
          rand_x = -range/2+rand_x;
          rand_y = -range/2+rand_y;
          pos.x = pos.x + rand_x;
          pos.y = pos.y + rand_y;

          ViewPlaneToModel(pos.MV, clipplane);

          XYZ loc00, loc11;
          _MarkerPos_to_NearFarPoint(pos, loc00, loc11);

          XYZ loc_temp = getCenterOfLineProfile(loc00, loc11, clipplane, chno);
          rand_loc_vec.push_back(loc_temp);
          float dist_temp = dist_L2(loc00, loc_temp);

          dist_score[dist_temp] = rand_loc_vec.size()-1;
     }
     // sort rand_loc_vec elements in depth order and get the middle loc
     map<float, int>::iterator it;

     int count=0;
     int mid_index;
     for(it=dist_score.begin(); it!=dist_score.end(); it++)
     {
          count++;
          if(count == dist_score.size()/2)
          {
               mid_index = it->second;
               break;
          }
     }
     // get middle loc
     mid_loc = rand_loc_vec.at(mid_index);

}

/**
 * compute distance between two neuron lines
 */
double Renderer_gl1::distance_between_2lines(NeuronTree &line1, NeuronTree &line2)
{
     V3DLONG size1=line1.listNeuron.size();
     V3DLONG size2=line2.listNeuron.size();

     if(size1==0 || size2==0) return INF;
     double sum_dist1 = 0.0;
     for(V3DLONG i = 0; i < size1; i++)
     {
          NeuronSWC ns1 = line1.listNeuron.at(i);
          XYZ loc1(ns1.x, ns1.y, ns1.z);
          double min_dist = MAX_DOUBLE;
          for(V3DLONG j = 0; j < size2; j++)
          {
               NeuronSWC ns2 = line2.listNeuron.at(j);
               XYZ loc2(ns2.x, ns2.y, ns2.z);
               double dst = dist_L2(loc1, loc1);
               min_dist = MIN(dst, min_dist);
          }
          sum_dist1 += min_dist;
     }
     double sum_dist2 = 0.0;
     for(V3DLONG j = 0; j < size2; j++)
     {
          NeuronSWC ns2 = line2.listNeuron.at(j);
          XYZ loc2(ns2.x, ns2.y, ns2.z);
          double min_dist = MAX_DOUBLE;
          for(V3DLONG i = 0; i < size1; i++)
          {
               NeuronSWC ns1 = line1.listNeuron.at(i);
               XYZ loc1(ns1.x, ns1.y, ns1.z);
               double dst = dist_L2(loc1, loc2);
               min_dist = MIN(dst, min_dist);
          }
          sum_dist2 += min_dist;
     }
     return (sum_dist1/size1 + sum_dist2/size2)/2.0;
}

// // use percentage
// double distance_between_lines1(vector<MyMarker*> & line1, vector<MyMarker*> & line2, double thresh = 2.0)
// {
//     if(line1.empty() || line2.empty()) return INF;
//     double sum1 = 0.0;
//     for(int i = 0; i < line1.size(); i++)
//     {
//         MyMarker * marker1 = line1[i];
//         double min_dist = MAX_DOUBLE;
//         for(int j = 0; j < line2.size(); j++)
//         {
//             MyMarker * marker2 = line2[j];
//             double dst = dist(*marker1, *marker2);
//             min_dist = MIN(dst, min_dist);
//         }
//         sum1 += (min_dist >= thresh) ? 1.0 : 0.0;
//     }
//     double sum2 = 0.0;
//     for(int j = 0; j < line2.size(); j++)
//     {
//         MyMarker * marker2 = line2[j];
//         double min_dist = MAX_DOUBLE;
//         for(int i = 0; i < line1.size(); i++)
//         {
//             MyMarker * marker1 = line1[i];
//             double dst = dist(*marker1, *marker2);
//             min_dist = MIN(dst, min_dist);
//         }
//         sum2 += (min_dist >= thresh) ? 1.0 : 0.0;
//     }
//     sum1 /= line1.size();
//     sum2 /= line2.size();
//     return MAX(sum1, sum2);
//     //return (sum1/line1.size() + sum2/line2.size())/2.0;
// }

void Renderer_gl1::swcBoundingBox(NeuronTree &line, XYZ &minloc, XYZ &maxloc)
{
     int N = line.listNeuron.size();

	if ( N <= 0 )  return; //data is not enough and use the whole image as the bounding box

     // find min-max of x y z in loc_veci
     double minx, miny, minz, maxx, maxy, maxz;

     // Directly using stroke pos for minloc, maxloc
     for (int i=0; i<N; i++)
     {
          NeuronSWC ns = line.listNeuron.at(i);

          if(i==0)
          {
               minx=maxx=ns.x; miny=maxy=ns.y; minz=maxz=ns.z;
          }

          if(minx>ns.x) minx=ns.x;
          if(miny>ns.y) miny=ns.y;
          if(minz>ns.z) minz=ns.z;

          if(maxx<ns.x) maxx=ns.x;
          if(maxy<ns.y) maxy=ns.y;
          if(maxz<ns.z) maxz=ns.z;
     }

     minloc.x = minx;
     minloc.y = miny;
     minloc.z = minz;

     maxloc.x = maxx;
     maxloc.y = maxy;
     maxloc.z = maxz;
}



void Renderer_gl1::MIP_XY_YZ_XZ(unsigned char * &pXY, unsigned char* &pYZ, unsigned char* &pXZ, XYZ &minloc, XYZ &maxloc)
{
     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if (w)
          curImg = v3dr_getImage4d(_idep);

     // The data is from minloc to maxloc
     V3DLONG sub_szx=abs(maxloc.x-minloc.x)+1;
     V3DLONG sub_szy=abs(maxloc.y-minloc.y)+1;
     V3DLONG sub_szz=abs(maxloc.z-minloc.z)+1;

     if(pXY) {delete []pXY; pXY=0;}
     if(pYZ) {delete []pYZ; pYZ=0;}
     if(pXZ) {delete []pXZ; pXZ=0;}

     V3DLONG offset_xy = sub_szx*sub_szy;
     V3DLONG offset_yz = sub_szy*sub_szz;
     V3DLONG offset_xz = sub_szx*sub_szz;

     pXY = new unsigned char [3 * offset_xy]; //RGB color image
     pYZ = new unsigned char [3 * offset_yz]; //RGB color image
     pXZ = new unsigned char [3 * offset_xz]; //RGB color image

     memset(pXY, 0, 3*offset_xy);
     memset(pYZ, 0, 3*offset_yz);
     memset(pXZ, 0, 3*offset_xz);

     int chno = checkCurChannel();
	if (chno<0 || chno>dim4-1)   chno = 0; //default first channel

     // get MIP_YZ
     for(V3DLONG k=0; k<sub_szy; k++)
     {
          for(V3DLONG j=0; j<sub_szz; j++)
          {
               V3DLONG ind_yz = k*sub_szz + j;
               for(V3DLONG i=0; i<sub_szx; i++)
               {
                    unsigned char value = curImg->at(minloc.x+i, minloc.y+k, minloc.z+j, chno);
                    if(pYZ[ind_yz] < value)
                    {
                         pYZ[ind_yz] = value;
                         pYZ[ind_yz + offset_yz] = value;
                         pYZ[ind_yz + 2*offset_yz] = value;
                    }
               }
          }
     }

     // get MIP_XY
     for(V3DLONG j=0; j<sub_szy; j++)
     {
          for(V3DLONG i=0; i<sub_szx; i++)
          {
               V3DLONG ind_xy = j*sub_szx + i;
               for(V3DLONG k=0; k<sub_szz; k++)
               {
                    unsigned char value = curImg->at(minloc.x+i, minloc.y+j, minloc.z+k, chno);
                    if(pXY[ind_xy] < value)
                    {
                         pXY[ind_xy] = value;
                         pXY[ind_xy + offset_xy] = value;
                         pXY[ind_xy + 2*offset_xy] = value;
                    }
               }
          }
     }

     // get MIP_XZ
     for(V3DLONG k=0; k<sub_szz; k++)
     {
          for(V3DLONG j=0; j<sub_szx; j++)
          {
               V3DLONG ind_xz = k*sub_szx + j;
               for(V3DLONG i=0; i<sub_szy; i++)
               {
                    unsigned char value = curImg->at(minloc.x+j, minloc.y+i, minloc.z+k, chno);
                    if(pXZ[ind_xz] < value)
                    {
                         pXZ[ind_xz] = value;
                         pXZ[ind_xz + offset_xz] = value;
                         pXZ[ind_xz + 2*offset_xz] = value;
                    }
               }
          }
     }

}

void Renderer_gl1::projectSWC_XY_YZ_XZ(unsigned char * &pXY, unsigned char * &pYZ, unsigned char * &pXZ, XYZ &minloc, XYZ &maxloc, NeuronTree &line, unsigned char color[3])
{
     V3DLONG sub_szx=abs(maxloc.x-minloc.x)+1;
     V3DLONG sub_szy=abs(maxloc.y-minloc.y)+1;
     V3DLONG sub_szz=abs(maxloc.z-minloc.z)+1;

     V3DLONG offset_xy = sub_szx*sub_szy;
     V3DLONG offset_yz = sub_szy*sub_szz;
     V3DLONG offset_xz = sub_szx*sub_szz;

     int N=line.listNeuron.size();

     if(N<=0) return;

     int c_size = 0; // cross size
     // Project on XY
     for(int k = 0; k < N; k++)
     {
          NeuronSWC ns = line.listNeuron.at(k);
          for(int j=0; j<sub_szy; j++)
          {
               for(int i=0; i<sub_szx; i++)
               {
                    V3DLONG ind_xy=j*sub_szx + i;
                    int xx = (int) ns.x - minloc.x;
                    int yy = (int) ns.y - minloc.y;

                    V3DLONG ind_swc = yy*sub_szx + xx;

                    if(ind_xy == ind_swc)
                    {
                         // draw a cross. rr is size of cross
                         for(int rr =-c_size; rr<=c_size; rr++)
                         {
                              int ii = i+rr;
                              if(ii < 0) ii=0;
                              if(ii >= sub_szx) ii = sub_szx-1;
                              V3DLONG ind_x = j*sub_szx + ii;

                              pXY[ind_x] = color[0];               // one color value
                              pXY[ind_x + offset_xy] = color[1];
                              pXY[ind_x + 2*offset_xy] = color[2];

                              int jj = j+rr;
                              if(jj < 0) jj = 0;
                              if(jj >= sub_szy) jj = sub_szy-1;
                              V3DLONG ind_y = jj*sub_szx + i;

                              pXY[ind_y] = color[0];               // one color value
                              pXY[ind_y + offset_xy] = color[1];
                              pXY[ind_y + 2*offset_xy] = color[2];
                         }
                    }
               }
          }

     }

     // Project on YZ
     for(int k = 0; k < N; k++)
     {
          NeuronSWC ns = line.listNeuron.at(k);
          for(int j=0; j<sub_szy; j++)
          {
               for(int i=0; i<sub_szz; i++)
               {
                    V3DLONG ind_yz=j*sub_szz + i;
                    int zz = (int) ns.z - minloc.z;
                    int yy = (int) ns.y - minloc.y;

                    V3DLONG ind_swc = yy*sub_szz + zz;

                    if(ind_yz == ind_swc)
                    {
                         // draw a cross. rr is size of cross
                         for(int rr =-c_size; rr<=c_size; rr++)
                         {
                              int ii = i+rr;
                              if(ii < 0) ii=0;
                              if(ii >= sub_szz) ii = sub_szz-1;
                              V3DLONG ind_z = j*sub_szz + ii;

                              pYZ[ind_z] = color[0];              // one coglor value
                              pYZ[ind_z + offset_yz] = color[1];
                              pYZ[ind_z + 2*offset_yz] = color[2];

                              int jj = j+rr;
                              if(jj < 0) jj = 0;
                              if(jj >= sub_szy) jj = sub_szy-1;
                              V3DLONG ind_y = jj*sub_szz + i;

                              pYZ[ind_y] = color[0];               // one color value
                              pYZ[ind_y + offset_yz] = color[1];
                              pYZ[ind_y + 2*offset_yz] = color[2];
                         }
                    }
               }
          }

     }


     // Project on XZ
     for(int k = 0; k < N; k++)
     {
          NeuronSWC ns = line.listNeuron.at(k);
          for(int j=0; j<sub_szz; j++)
          {
               for(int i=0; i<sub_szx; i++)
               {
                    V3DLONG ind_xz=j*sub_szx + i;
                    int zz = (int) ns.z - minloc.z;
                    int xx = (int) ns.x - minloc.x;

                    V3DLONG ind_swc = zz*sub_szx + xx;

                    if(ind_xz == ind_swc)
                    {
                         // draw a cross. rr is size of cross
                         for(int rr =-c_size; rr<=c_size; rr++)
                         {
                              int ii = i+rr;
                              if(ii < 0) ii=0;
                              if(ii >= sub_szx) ii = sub_szx-1;
                              V3DLONG ind_x = j*sub_szx + ii;

                              pXZ[ind_x] = color[0];              // one coglor value
                              pXZ[ind_x + offset_xz] = color[1];
                              pXZ[ind_x + 2*offset_xz] = color[2];

                              int jj = j+rr;
                              if(jj < 0) jj = 0;
                              if(jj >= sub_szz) jj = sub_szz-1;
                              V3DLONG ind_z = jj*sub_szx + i;

                              pXZ[ind_z] = color[0];               // one color value
                              pXZ[ind_z + offset_xz] = color[1];
                              pXZ[ind_z + 2*offset_xz] = color[2];
                         }
                    }
               }
          }

     }

}


bool Renderer_gl1::pickSeedpointFromExistingCurves(const MarkerPos &pos, XYZ &nearest_loc)
{
     MainWindow* V3Dmainwindow = 0;
	 V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);

     if (V3Dmainwindow && V3Dmainwindow->global_setting.b_3dcurve_autoconnecttips)
     {
          if (listNeuronTree.size()>0 && curEditingNeuron>0 && curEditingNeuron<=listNeuronTree.size())
          {
               NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(curEditingNeuron-1)));
               if (p_tree)
               {
                   double best_dist;
                   V3DLONG n_id_start = findNearestNeuronNode_WinXY(pos.x, pos.y, p_tree, best_dist);
                   if (best_dist<=5 && n_id_start>=0) //this means it is valid!
                   {
                       nearest_loc = p_tree->listNeuron.at(n_id_start);
                       return true;
                   }
                   else
                   {
                       return false;
                   }
               }
          }
     }
     return false;
}


void Renderer_gl1::createLastTestID(QString &curFilePath, QString &curSuffix, int &test_id)
{
     //
	QStringList myList;
	myList.clear();

	// get the image files namelist in the directory
	// QFileInfo fileInfo(individualFileName);
	// QString curFilePath = fileInfo.path();
	// QString curSuffix = fileInfo.suffix();

	QDir dir(curFilePath);
	if (!dir.exists())
	{
		qWarning("Cannot find the directory and now mkdir.");
		bool success_mkdir = dir.mkdir(curFilePath);

		qDebug()<<"mkdir "<<curFilePath<<success_mkdir;
	}

	QStringList imgfilters;
	imgfilters.append("*." + curSuffix);
	foreach (QString file, dir.entryList(imgfilters, QDir::Files, QDir::Name))
	{
		myList += QFileInfo(dir, file).absoluteFilePath();
	}

     test_id = myList.size() + 1;
}

// @ADDED by Alessandro on 2015-05-23. Called when "Esc" key is pressed and tracedNeuron must be updated.
void Renderer_gl1::deleteMultiNeuronsByStrokeCommit()
{
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

    My4DImage* curImg = 0;       if (w) {editinput = 3;curImg = v3dr_getImage4d(_idep);}
    if (w->TeraflyCommunicator
    &&w->TeraflyCommunicator->socket->state()==QAbstractSocket::ConnectedState)
	{
        vector<V_NeuronSWC> vector_VSWC;
        curImg->ExtractDeletingNode(vector_VSWC);

		w->SetupCollaborateInfo();
        for(auto seg:vector_VSWC)
            w->TeraflyCommunicator->UpdateDelSegMsg(seg,"TeraFly");//ask QiLi
//        w->getRenderer()->endSelectMode();
//        CViewer::getCurrent()->loadAnnotations(false);
    }
    curImg->tracedNeuron.deleteMultiSeg();

    //curImg->proj_trace_history_append();          // no need to update the history
    curImg->update_3drenderer_neuron_view(w, this);
//    NeuronTree nt= terafly::PluginInterface::getSWC();
//    terafly::PluginInterface::setSWC(nt,false);// remove status delete segment
}

bool Renderer_gl1::deleteMultiNeuronsByStrokeCommit(vector <XYZ> local_list,float mindist)//not use by huanglei
{
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

    My4DImage* curImg = 0;       if (w) {/*editinput = 3;*/curImg = v3dr_getImage4d(_idep);}

    auto blocksegs=curImg->tracedNeuron.seg;
    int index=-1;
    for(int i=0;i<blocksegs.size();i++)
    {
        if(local_list.size()!=blocksegs[i].row.size()) continue;
        auto seg=blocksegs[i].row;
        float sum=0;
        for(int j=0;j<local_list.size();j++)
        {
            sum+=sqrt(pow(local_list[j].x-seg[j].x,2)+pow(local_list[j].y-seg[j].y,2)
                      +pow(local_list[j].z-seg[j].z,2));
        }
        if(sum/local_list.size()<mindist)
        {
            mindist=sum/local_list.size();
            index=i;
        }
        reverse(local_list.begin(),local_list.end());
        sum=0;
        for(int j=0;j<local_list.size();j++)
        {
            sum+=sqrt(pow(local_list[j].x-seg[j].x,2)+pow(local_list[j].y-seg[j].y,2)
                      +pow(local_list[j].z-seg[j].z,2));
        }
        if(sum/local_list.size()<mindist)
        {
            mindist=sum/local_list.size();
            index=i;
        }
    }
    if(index>=0)
    {
        curImg->tracedNeuron.seg.erase(curImg->tracedNeuron.seg.begin()+index);
        return true;
    }else
    {
        return false;
    }
}

// @ADDED by Alessandro on 2015-09-30. Select multiple markers by one-mouse stroke.
void Renderer_gl1::selectMultiMarkersByStroke()
{
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

    My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
    XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

    // contour 2 polygon
    QPolygon poly;
    for (V3DLONG i=0; i<list_listCurvePos.at(0).size(); i++)
        poly.append(QPoint(list_listCurvePos.at(0).at(i).x, list_listCurvePos.at(0).at(i).y));

    // contour mode := Qt::Key_Shift pressed := delete markers intersecting the line, otherwise delete all markers within the contour
    bool contour_mode = !QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);

    const float tolerance_squared = 10; // tolerance distance (squared for faster dist computation) from the backprojected marker to the curve point

    // back-project the 3D points and label hitted markers
    for(QList<ImageMarker>::iterator i=listMarker.begin(); i!=listMarker.end(); i++)
    {
        GLdouble px, py, pz, ix, iy, iz;
        ix = i->x;
        iy = i->y;
        iz = i->z;
        if(gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
        {
            py = viewport[3]-py; //the Y axis is reversed
            QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));
            {
                if(contour_mode)
                {
                    if(poly.boundingRect().contains(p) && pointInPolygon(p.x(), p.y(), poly))
                        i->selected = true;
                }
                else
                {
                    for (V3DLONG k=0; k<list_listCurvePos.at(0).size(); k++)
                    {
                        QPointF p2(list_listCurvePos.at(0).at(k).x, list_listCurvePos.at(0).at(k).y);
                        if( (p.x()-p2.x())*(p.x()-p2.x()) + (p.y()-p2.y())*(p.y()-p2.y()) <= tolerance_squared  )
                        {
                           i->selected = true;
                           break;   // found intersection with marker: no more need to continue on this inner loop
                        }
                    }
                }
            }
        }
    }

    // inform TeraFly (SAFE)
    // this does nothing except when TeraFly is active
    tf::TeraFly::doaction("marker multiselect");
}

// @ADDED by Alessandro on 2015-05-07.
void Renderer_gl1::deleteMultiNeuronsByStroke()
{
	cout << "delete pos find success" << endl;
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
	XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

//    qDebug()<<"deleteMultiNeuronsByStroke-1";
	//v3d_msg(QString("getNumShiftHolding() = ") + QString(w->getNumShiftHolding() ? "YES" : "no"));
	const float tolerance_squared = 100; // tolerance distance squared (for faster dist computation) from the backprojected neuron to the curve point

	// 没跑进去
    if(deleteKey==1)
    {
        qDebug()<<"type i to delete isolated node(s)";
        const V3DLONG nsegs = curImg->tracedNeuron.seg.size();
        for (V3DLONG s=0; s<nsegs; s++)
        {
            if(curImg->tracedNeuron.seg[s].row.size()==1)
            {
                curImg->tracedNeuron.seg[s].to_be_deleted = true;
            }
        }

        curImg->update_3drenderer_neuron_view(w, this);
        curImg->proj_trace_history_append();

        return;
    }
	//没跑进去
    if(deleteKey==2)
    {
        qDebug()<<"type t to delete type is not 2 or 3";
        const V3DLONG nsegs = curImg->tracedNeuron.seg.size();
        for (V3DLONG s=0; s<nsegs; s++)
        {
            double type = curImg->tracedNeuron.seg[s].row[0].type;
            if(type!=2 && type!=3)
            {
                curImg->tracedNeuron.seg[s].to_be_deleted = true;
            }
        }

        curImg->update_3drenderer_neuron_view(w, this);
        curImg->proj_trace_history_append();

        return;
    }

    // contour mode := Qt::Key_Shift pressed := delete all segments within the contour, otherwise delete segments intersecting the line
    int contour_mode = 0;
    if(QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier))
        contour_mode = 1; //press "shift"
    else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
    {
        if(editinput==3)
            contour_mode = 2; //press "ctrl"
    }

//    qDebug()<<"deleteMultiNeuronsByStroke-2";
	// contour 2 polygon
	QPolygon poly;
	for (V3DLONG i=0; i<list_listCurvePos.at(0).size(); i++)
		poly.append(QPoint(list_listCurvePos.at(0).at(i).x, list_listCurvePos.at(0).at(i).y));
	// 把划线的点全都放到多边形中去
//    qDebug()<<"deleteMultiNeuronsByStroke-3";
	const V3DLONG nsegs = curImg->tracedNeuron.seg.size();


	for (V3DLONG s=0; s<nsegs; s++)
	{
		if (s >= curImg->tracedNeuron.seg.size())
		{
			qDebug() << "WARNING! curImg->tracedNeuron.size() was changed during call to Renderer_gl1::deleteMultiNeuronsByStroke()";
			break;
		}
		//如果这条线已经要被删除了 那就不做处理
		if (curImg->tracedNeuron.seg[s].to_be_deleted)
			continue;

		//拿到traced_neuron 的第i条 seg
		V_NeuronSWC this_seg = curImg->tracedNeuron.seg.at(s);
		const V3DLONG nrows = this_seg.row.size();


		bool allUnitsOutsideZCut = false;
		for (V3DLONG i=0;i<nrows;i++)
		{

			GLdouble px, py, pz, ix, iy, iz;
			V_NeuronSWC_unit this_unit = this_seg.row.at(i);
			ix = this_unit.x;
			iy = this_unit.y;
			iz = this_unit.z;
			allUnitsOutsideZCut = ! ((((float) iz) >=  this->swcBB.z0)&&( ((float) iz) <=  this->swcBB.z1));
			//只要有一个点在swcBB外面 这个Bool值就是true
			if (curImg->tracedNeuron.seg[s].to_be_deleted)
                break;    //保证 ？ L3438???


			if(gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
			{
				py = viewport[3]-py; //the Y axis is reversed
				QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));
				
                if(contour_mode == 1)
				{
					cout << "contour_mode == 1" << endl;
                    if(poly.boundingRect().contains(p) && pointInPolygon(p.x(), p.y(), poly)&& !allUnitsOutsideZCut)
						curImg->tracedNeuron.seg[s].to_be_deleted = true;
					//如果这个点在 投影得到的多边形中 并且没有超出Z范围 那就删除这个线段
                }else if (contour_mode == 2)
                {
					cout << "contour_mode == 2" << endl;
                    if(!poly.boundingRect().contains(p) || !pointInPolygon(p.x(), p.y(), poly)&& !allUnitsOutsideZCut)
                        curImg->tracedNeuron.seg[s].to_be_deleted = true;
                }
				else
				{
//					cout << "contour_mode ==  else " << endl;
					for (V3DLONG k=0; k<list_listCurvePos.at(0).size(); k++)
					{
						//对画的这条线的每个点取xy 来构成一个点 如果距离小于阈值 那就把这个片段标记为to be deleted
						QPointF p2(list_listCurvePos.at(0).at(k).x, list_listCurvePos.at(0).at(k).y);
                        if( (p.x()-p2.x())*(p.x()-p2.x()) + (p.y()-p2.y())*(p.y()-p2.y()) <= tolerance_squared  && !allUnitsOutsideZCut)
						{
							if (s >= curImg->tracedNeuron.seg.size())
							{
//								qDebug() << "WARNING! curImg->tracedNeuron.size() was changed during call to Renderer_gl1::deleteMultiNeuronsByStroke()";
								break;
							}
							curImg->tracedNeuron.seg[s].to_be_deleted = true;
							break;   // found intersection with neuron segment: no more need to continue on this inner loop
						}
					}
				}
            }
		}
		if (this->cuttingZ)		{
			curImg->tracedNeuron.seg[s].to_be_deleted = curImg->tracedNeuron.seg[s].to_be_deleted && !allUnitsOutsideZCut;
		}
	}
	//以上就是把距离判断做了一遍 看看是不是要删除某个seg 以及判断有没有Zcut

//    qDebug()<<"deleteMultiNeuronsByStroke-4";
    vector <XYZ> specialmarkerloc;
    vector <V3DLONG> specialmarkerslocindex;
    QList <LocationSimple> &listloc = curImg->listLandmarks;
    for(V3DLONG i=0; i<listloc.size(); ++i)
    {
        if(listloc[i].category==77)
        {
            XYZ tmp(listloc[i].x,listloc[i].y,listloc[i].z);
            specialmarkerloc.push_back(tmp);
            specialmarkerslocindex.push_back(i);
        }
    }
	//把 category的Landmarker放到Special里面去？
//    qDebug()<<"deleteMultiNeuronsByStroke-5";
    V3DLONG specialmarkersegindex = -1;
    V3DLONG specialmarkerlocindex = -1;
    for(V3DLONG i=0; i<nsegs; ++i)
    {
        V_NeuronSWC this_seg = curImg->tracedNeuron.seg.at(i);
        const V3DLONG nrows = this_seg.row.size();
        if(curImg->tracedNeuron.seg[i].to_be_deleted==true)
        {
			//取当前seg的最后一个点 如果这个位置和之前的special marker的位置相同  记录下当前的seg的索引
            XYZ segloclast(this_seg.row.at(nrows-1).x+1,this_seg.row.at(nrows-1).y+1,this_seg.row.at(nrows-1).z+1);
            for(V3DLONG j=0; j<specialmarkerloc.size(); ++j)
            {
                if(segloclast==specialmarkerloc.at(j))
                {
                    specialmarkerlocindex = specialmarkerslocindex.at(j);
                    specialmarkersegindex = i;
                    break;
                }

            }
        }
        if(specialmarkersegindex!=-1)
            break;
    }

	//如果当前记录的special marker 和 special seg都不为空
    if(specialmarkerlocindex!=-1 && specialmarkersegindex!=-1)
    {
        QList <LocationSimple> ::iterator it = listloc.begin();
        listloc.erase(it+specialmarkerlocindex);
		//把当前的那个specialmarker删掉？
        bool islastseg = false;
        while(!islastseg)
        {
            bool changed = false;
            for(V3DLONG i=0; i<nsegs; ++i)
            {
                if(curImg->tracedNeuron.seg[i].to_be_deleted==true && i!=specialmarkersegindex)
                {
					//对于要被删除的线段 并且它不是special seg
					//如果这条线是special seg的前一条线
					//把i设置成这条线 changed为true
					//从seg index开始 一直往前删除
					//直到没的删除为止
                    XYZ parent(curImg->tracedNeuron.seg[i].row.back().x,curImg->tracedNeuron.seg[i].row.back().y,curImg->tracedNeuron.seg[i].row.back().z);
                    XYZ child(curImg->tracedNeuron.seg[specialmarkersegindex].row.front().x,curImg->tracedNeuron.seg[specialmarkersegindex].row.front().y,curImg->tracedNeuron.seg[specialmarkersegindex].row.front().z);
                    if(parent==child)
                    {
                        changed = true;
                        specialmarkersegindex = i;
                    }
                }
            }
            if(!changed)
            {
                islastseg = true;
            }
            changed = false;
        }
    }
    if(specialmarkerlocindex!=-1 && specialmarkersegindex!=-1)
    {
        XYZ markerloc(curImg->tracedNeuron.seg[specialmarkersegindex].row.front().x,curImg->tracedNeuron.seg[specialmarkersegindex].row.front().y,curImg->tracedNeuron.seg[specialmarkersegindex].row.front().z);
        addSpecialMarker(markerloc);
    } // by XZ, 20190726
//    qDebug()<<"deleteMultiNeuronsByStroke-11";
    curImg->update_3drenderer_neuron_view(w, this);
    curImg->proj_trace_history_append();
}

// --------- Simple connecting tool (no geometrical analysis, only 2 segments at a time), MK, April, 2018 ---------
void Renderer_gl1::simpleConnect()
{
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
	XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

	//if (this->isTera)
	//{
		// In terafly mode, the number of segments carried by curImg->->tracedNeuron.seg only includes those within the annotation space.
		// See CAnnotations::findCurves for more information.
		//cout << "number of editable segments:" << curImg->tracedNeuron.seg.size() << endl;

		//simpleConnectExecutor(w, curImg, curXWidget);
	//}
	//else if (!this->isTera)
	//{
		float tolerance = 20; // tolerance distance from the backprojected neuron to the curve point

		for (V3DLONG j = 0; j < listNeuronTree.size(); j++)
		{
			this->treeOnTheFly = V_NeuronSWC_list__2__NeuronTree(curImg->tracedNeuron);
			this->treeOnTheFly.editable = true;
			//cout << this->treeOnTheFly.listNeuron.size() << endl;
			
			/*if (!this->isLoadFromFile)
			{
				curImg->tracedNeuron = NeuronTree__2__V_NeuronSWC_list(&(this->treeOnTheFly));
				cout << "Annotation loaded from file? " << this->isLoadFromFile << endl;
			}*/

			NeuronTree* p_tree = (NeuronTree*)(&(listNeuronTree.at(j))); //curEditingNeuron-1
			if (p_tree && p_tree->editable)    // @FIXED by Alessandro on 2015-05-23. Removing segments from non-editable neurons causes crash.
			{
				QList<NeuronSWC>* p_listneuron = &(p_tree->listNeuron);
				if (!p_listneuron) return;
				// for (int testi=0; testi<list_listCurvePos.at(0).size(); testi++) qDebug() << list_listCurvePos.at(0).at(testi).x << " " << list_listCurvePos.at(0).at(testi).y;

				vector<segInfoUnit> segInfo;
				long segCheck = 0;
				long cummNodeNum = 0;

				/* ============== Get all segments information included in the movePen trajectory, and then decide where to connect ============== */
				/* ======== Only take in the nodes within the rectangle that contains the stroke ======== */
				long minX = list_listCurvePos.at(0).at(0).x, maxX = list_listCurvePos.at(0).at(0).x;
				long minY = list_listCurvePos.at(0).at(0).y, maxY = list_listCurvePos.at(0).at(0).y;
				for (size_t i = 0; i < list_listCurvePos.at(0).size(); ++i)
				{
					if (list_listCurvePos.at(0).at(i).x <= minX) minX = list_listCurvePos.at(0).at(i).x;
					if (list_listCurvePos.at(0).at(i).x >= maxX) maxX = list_listCurvePos.at(0).at(i).x;
					if (list_listCurvePos.at(0).at(i).y <= minY) minY = list_listCurvePos.at(0).at(i).y;
					if (list_listCurvePos.at(0).at(i).y >= maxY) maxY = list_listCurvePos.at(0).at(i).y;
				}
				minX = minX - 5; maxX = maxX + 5;
				minY = minY - 5; maxY = maxY + 5;
				//cout << minX << " " << maxX << " " << minY << " " << maxY << endl;
				QList<NeuronSWC> nodeOnStroke;
				for (size_t i = 0; i < p_listneuron->size(); ++i)
				{
					GLdouble px, py, pz, ix, iy, iz;
					ix = p_listneuron->at(i).x;
					iy = p_listneuron->at(i).y;
					iz = p_listneuron->at(i).z;
					if (gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
					{
						py = viewport[3] - py; //the Y axis is reversed
						QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));
						if ((p.x() >= minX && p.x() <= maxX) && (p.y() >= minY && p.y() <= maxY))
						{
							nodeOnStroke.push_back(p_listneuron->at(i));
							//cout << p.x() << " " << p.y() << endl;
						}
					}
				}
				/* ==== END of [Only take in the nodes within the rectangle that contains the stroke] ==== */

				/* ========= Acquire the 1st 2 and only the 1st 2 segments touched by stroke ========= */
				for (V3DLONG i = 0; i < list_listCurvePos.at(0).size(); i++)
				{
					for (V3DLONG j = 0; j < nodeOnStroke.size(); j++)
					{
						GLdouble px, py, pz, ix, iy, iz;
						ix = nodeOnStroke.at(j).x;
						iy = nodeOnStroke.at(j).y;
						iz = nodeOnStroke.at(j).z;
						if (gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
						{
							py = viewport[3] - py; //the Y axis is reversed
							QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));

							QPointF p2(list_listCurvePos.at(0).at(i).x, list_listCurvePos.at(0).at(i).y);
							if (std::sqrt((p.x() - p2.x())*(p.x() - p2.x()) + (p.y() - p2.y())*(p.y() - p2.y())) <= tolerance)
							{
								if (curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.begin()->data[6] != 2) // Sort the node numbers of involved segments
								{
									int nodeNo = 1;
									for (vector<V_NeuronSWC_unit>::iterator it = curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.begin();
										it != curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.end(); it++)
									{
										it->data[0] = nodeNo;
										it->data[6] = nodeNo + 1;
										++nodeNo;
									}
									(curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.end() - 1)->data[6] = -1;
								}

								for (vector<V_NeuronSWC_unit>::iterator it = curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.begin();
									it != curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.end(); it++)
								{
									if (nodeOnStroke.at(j).x == it->data[2] && nodeOnStroke.at(j).y == it->data[3] && nodeOnStroke.at(j).z == it->data[4])
									{
										//---------------------- Get seg IDs
										//qDebug() << nodeOnStroke->at(j).seg_id << " " << nodeOnStroke->at(j).parent << " " << p.x() << " " << p.y();
										segInfoUnit curSeg;
										curSeg.head_tail = it->data[6];
										curSeg.segID = nodeOnStroke.at(j).seg_id;
										curSeg.nodeCount = curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.size();
										curSeg.refine = false;
										curSeg.branchID = curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].branchingProfile.ID;
										curSeg.paBranchID = curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].branchingProfile.paID;
										curSeg.hierarchy = curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].branchingProfile.hierarchy;
										vector<segInfoUnit>::iterator chkIt = segInfo.end();
										if (segInfo.begin() == segInfo.end())
										{
											segInfo.push_back(curSeg);
											segCheck = it->data[6];
										}
										else
										{
											bool repeat = false;
											while (chkIt >= segInfo.begin())
											{
												if (chkIt->segID == curSeg.segID)
												{
													repeat = true;
													break;
												}
												else --chkIt;
											}
											if (repeat == false)
											{
												segInfo.push_back(curSeg);
												segCheck = it->data[6];
											}
										}
									}
								}
								break;
							}
						}
					}
					if (segInfo.size() == 2) break; // simple connection only allows 2 segments involved
				}
				cout << endl;
				for (vector<segInfoUnit>::iterator segInfoIt = segInfo.begin(); segInfoIt != segInfo.end(); ++segInfoIt)
					cout << "seg ID:" << segInfoIt->segID << " head tail:" << segInfoIt->head_tail << endl; //<< " || branching ID:" << segInfoIt->branchID << " parent branch ID:" << segInfoIt->paBranchID << " hierarchy:" << segInfoIt->hierarchy << endl;
				
				if (segInfo.size() < 2) return;
				/* ========= END of [Acquire the 1st 2 and only the 1st 2 segments touched by stroke] ========= */
				
				simpleConnectExecutor(curImg, segInfo);
				this->hierarchyRelabel = false; // Editing neuron structure rarely starts with a sorted swc. Thus this switch is disabled for now.
												//  -- MK, July, 2018
				if (this->hierarchyRelabel)
				{
					if (curImg->tracedNeuron.seg[segInfo[0].segID].to_be_deleted) this->hierarchyReprofile(curImg, segInfo[1].segID, segInfo[0].segID);
					else if (curImg->tracedNeuron.seg[segInfo[1].segID].to_be_deleted) this->hierarchyReprofile(curImg, segInfo[0].segID, segInfo[1].segID);
				}
				else
				{
					// ----------------- For debug purpose -----------------
					//for (vector<V_NeuronSWC_unit>::iterator debugIt = curImg->tracedNeuron.seg[segInfo[1].segID].row.begin(); debugIt != curImg->tracedNeuron.seg[segInfo[1].segID].row.end(); ++debugIt)
					//	cout << "ID:" << debugIt->n << " parent:" << debugIt->parent << endl;
					// -----------------------------------------------------
					if (curImg->tracedNeuron.seg[segInfo[0].segID].to_be_deleted)
					{
						vector<V_NeuronSWC> connectedSegDecomposed = decompose_V_NeuronSWC(curImg->tracedNeuron.seg[segInfo[1].segID]);
						for (vector<V_NeuronSWC>::iterator addedIt = connectedSegDecomposed.begin(); addedIt != connectedSegDecomposed.end(); ++addedIt)
							curImg->tracedNeuron.seg.push_back(*addedIt);

						curImg->tracedNeuron.seg[segInfo[1].segID].to_be_deleted = true;
						curImg->tracedNeuron.seg[segInfo[1].segID].on = false;
					}
					else if (curImg->tracedNeuron.seg[segInfo[1].segID].to_be_deleted)
					{
						vector<V_NeuronSWC> connectedSegDecomposed = decompose_V_NeuronSWC(curImg->tracedNeuron.seg[segInfo[0].segID]);
						for (vector<V_NeuronSWC>::iterator addedIt = connectedSegDecomposed.begin(); addedIt != connectedSegDecomposed.end(); ++addedIt)
							curImg->tracedNeuron.seg.push_back(*addedIt);

						curImg->tracedNeuron.seg[segInfo[0].segID].to_be_deleted = true;
						curImg->tracedNeuron.seg[segInfo[0].segID].on = false;
					}
				}
				
				curImg->update_3drenderer_neuron_view(w, this);
				curImg->proj_trace_history_append();

				size_t to_be_deletedCount = 0;
				cout << "segment to be deleted:";
				for (vector<V_NeuronSWC>::iterator it = curImg->tracedNeuron.seg.begin(); it != curImg->tracedNeuron.seg.end(); ++it)
				{
					if (it->to_be_deleted)
					{
						++to_be_deletedCount;
						cout << size_t(it - curImg->tracedNeuron.seg.begin()) << " ";
						//cout << this->branchSegIDmap[it->branchingProfile.ID] << ", " << it->branchingProfile.ID << endl;
					}
				}
				cout << endl;
			}
		}
	//}

	return;
}

void Renderer_gl1::simpleConnectExecutor(My4DImage* curImg, vector<segInfoUnit>& segInfo)
{
	// This method is the "executor" of Renderer_gl1::simpleConnect(), MK, May, 2018

	//////////////////////////////////////////// HEAD TAIL CONNECTION ////////////////////////////////////////////
	if ((segInfo.at(0).head_tail == -1 || segInfo.at(0).head_tail == 2) && (segInfo.at(1).head_tail == -1 || segInfo.at(1).head_tail == 2))
	{
		segInfoUnit mainSeg, branchSeg;
		if (segInfo.at(0).nodeCount >= segInfo.at(1).nodeCount)
		{
			mainSeg = segInfo.at(0);
			branchSeg = segInfo.at(1);
			cout << "primary seg length:" << mainSeg.nodeCount << "   primary seg orient:" << mainSeg.head_tail << endl;
			cout << "secondary seg length:" << branchSeg.nodeCount << "   secondary seg orient:" << branchSeg.head_tail << endl;
		}
		else
		{
			mainSeg = segInfo.at(1);
			branchSeg = segInfo.at(0);
			cout << "primary seg length:" << mainSeg.nodeCount << "   primary seg orient:" << mainSeg.head_tail << endl;
			cout << "secondary seg length:" << branchSeg.nodeCount << "   secondary seg orient:" << branchSeg.head_tail << endl;
		}

		double assignedType;
		assignedType = curImg->tracedNeuron.seg[segInfo.at(0).segID].row[0].type;
		curImg->tracedNeuron.seg[mainSeg.segID].row[0].seg_id = mainSeg.segID;
		if (mainSeg.head_tail == -1)
		{
			if (branchSeg.head_tail == -1) // head to head
			{
				for (vector<V_NeuronSWC_unit>::iterator itNextSeg = curImg->tracedNeuron.seg[branchSeg.segID].row.end() - 1;
					itNextSeg >= curImg->tracedNeuron.seg[branchSeg.segID].row.begin(); --itNextSeg)
				{
					itNextSeg->seg_id = branchSeg.segID; 
					curImg->tracedNeuron.seg[mainSeg.segID].row.push_back(*itNextSeg);
				}
			}
			else if (branchSeg.head_tail == 2) // head to tail
			{
				for (vector<V_NeuronSWC_unit>::iterator itNextSeg = curImg->tracedNeuron.seg[branchSeg.segID].row.begin();
					itNextSeg != curImg->tracedNeuron.seg[branchSeg.segID].row.end(); ++itNextSeg)
				{
					itNextSeg->seg_id = branchSeg.segID;
					curImg->tracedNeuron.seg[mainSeg.segID].row.push_back(*itNextSeg);
				}
			}
			curImg->tracedNeuron.seg[branchSeg.segID].to_be_deleted = true;
			curImg->tracedNeuron.seg[branchSeg.segID].on = false;

			// sorting the new segment here, and reassign the root node to the new tail
			size_t nextSegNo = 1;
			for (vector<V_NeuronSWC_unit>::iterator itSort = curImg->tracedNeuron.seg[mainSeg.segID].row.begin();
				itSort != curImg->tracedNeuron.seg[mainSeg.segID].row.end(); ++itSort)
			{
				itSort->data[0] = nextSegNo;
				itSort->data[6] = itSort->data[0] + 1;
				++nextSegNo;
			}
			(curImg->tracedNeuron.seg[mainSeg.segID].row.end() - 1)->data[6] = -1;
		}
		else if (mainSeg.head_tail == 2)
		{
			std::reverse(curImg->tracedNeuron.seg[mainSeg.segID].row.begin(), curImg->tracedNeuron.seg[mainSeg.segID].row.end());
			if (branchSeg.head_tail == -1) // tail to head
			{
				for (vector<V_NeuronSWC_unit>::iterator itNextSeg = curImg->tracedNeuron.seg[branchSeg.segID].row.end() - 1;
					itNextSeg >= curImg->tracedNeuron.seg[branchSeg.segID].row.begin(); itNextSeg--)
				{
					itNextSeg->seg_id = branchSeg.segID;
					curImg->tracedNeuron.seg[mainSeg.segID].row.push_back(*itNextSeg);
				}
			}
			else if (branchSeg.head_tail == 2) // tail to tail
			{
				for (vector<V_NeuronSWC_unit>::iterator itNextSeg = curImg->tracedNeuron.seg[branchSeg.segID].row.begin();
					itNextSeg != curImg->tracedNeuron.seg[branchSeg.segID].row.end(); itNextSeg++)
				{
					itNextSeg->seg_id = branchSeg.segID;
					curImg->tracedNeuron.seg[mainSeg.segID].row.push_back(*itNextSeg);
				}
			}
			curImg->tracedNeuron.seg[branchSeg.segID].to_be_deleted = true;
			curImg->tracedNeuron.seg[branchSeg.segID].on = false;

			// sorting the new segment here, and reassign the root node to the new tail
			std::reverse(curImg->tracedNeuron.seg[mainSeg.segID].row.begin(), curImg->tracedNeuron.seg[mainSeg.segID].row.end());
			size_t nextSegNo = 1;
			for (vector<V_NeuronSWC_unit>::iterator itSort = curImg->tracedNeuron.seg[mainSeg.segID].row.begin();
				itSort != curImg->tracedNeuron.seg[mainSeg.segID].row.end(); itSort++)
			{
				itSort->data[0] = nextSegNo;
				itSort->data[6] = itSort->data[0] + 1;
				++nextSegNo;
			}
			(curImg->tracedNeuron.seg[mainSeg.segID].row.end() - 1)->data[6] = -1;
		}

		// correcting types, based on the main segment type
		for (vector<V_NeuronSWC_unit>::iterator reID = curImg->tracedNeuron.seg[mainSeg.segID].row.begin();
			reID != curImg->tracedNeuron.seg[mainSeg.segID].row.end(); ++reID)
		{
			reID->seg_id = mainSeg.segID;
			reID->type = assignedType;
		}
	}
	//////////////////////////////////////////// END of [HEAD TAIL CONNECTION] ////////////////////////////////////////////

	//////////////////////////////////////////// BRANCHING CONNECTION ////////////////////////////////////////////
	if ((segInfo.at(0).head_tail != -1 && segInfo.at(0).head_tail != 2) ^ (segInfo.at(1).head_tail != -1 && segInfo.at(1).head_tail != 2))
	{
		segInfoUnit mainSeg, branchSeg;
		if (segInfo.at(0).head_tail == -1 || segInfo.at(0).head_tail == 2)
		{
			mainSeg = segInfo.at(1);
			branchSeg = segInfo.at(0);
			cout << "primary seg length:" << mainSeg.nodeCount << "   primary seg orient:" << mainSeg.head_tail << endl;
			cout << "secondary seg length:" << branchSeg.nodeCount << "   secondary seg orient:" << branchSeg.head_tail << endl;
		}
		else
		{
			mainSeg = segInfo.at(0);
			branchSeg = segInfo.at(1);
			cout << "primary seg length:" << mainSeg.nodeCount << "   primary seg orient:" << mainSeg.head_tail << endl;
			cout << "secondary seg length:" << branchSeg.nodeCount << "   secondary seg orient:" << branchSeg.head_tail << endl;
		}

		double assignedType;
		assignedType = curImg->tracedNeuron.seg[segInfo.at(0).segID].row[0].type;
		curImg->tracedNeuron.seg[mainSeg.segID].row[0].seg_id = mainSeg.segID;
		if (branchSeg.head_tail == 2) // branch to tail
		{
			std::reverse(curImg->tracedNeuron.seg[branchSeg.segID].row.begin(), curImg->tracedNeuron.seg[branchSeg.segID].row.end());
			size_t branchSegLength = curImg->tracedNeuron.seg[branchSeg.segID].row.size();
			size_t mainSegLength = curImg->tracedNeuron.seg[mainSeg.segID].row.size();
			curImg->tracedNeuron.seg[mainSeg.segID].row.insert(curImg->tracedNeuron.seg[mainSeg.segID].row.end(), curImg->tracedNeuron.seg[branchSeg.segID].row.begin(), curImg->tracedNeuron.seg[branchSeg.segID].row.end());
			size_t branchN = mainSegLength + 1;
			for (vector<V_NeuronSWC_unit>::iterator itNextSeg = curImg->tracedNeuron.seg[mainSeg.segID].row.end() - 1;
				itNextSeg != curImg->tracedNeuron.seg[mainSeg.segID].row.begin() + ptrdiff_t(mainSegLength - 1); --itNextSeg)
			{
				itNextSeg->n = branchN;
				itNextSeg->seg_id = mainSeg.segID;
				itNextSeg->parent = branchN - 1;
				++branchN;
			}
			(curImg->tracedNeuron.seg[mainSeg.segID].row.end() - 1)->parent = (curImg->tracedNeuron.seg[mainSeg.segID].row.begin() + ptrdiff_t(mainSeg.head_tail - 2))->n;
			curImg->tracedNeuron.seg[branchSeg.segID].to_be_deleted = true;
			curImg->tracedNeuron.seg[branchSeg.segID].on = false;
		}
		else if (branchSeg.head_tail == -1) // branch to head
		{
			size_t branchSegLength = curImg->tracedNeuron.seg[branchSeg.segID].row.size();
			size_t mainSegLength = curImg->tracedNeuron.seg[mainSeg.segID].row.size();
			curImg->tracedNeuron.seg[mainSeg.segID].row.insert(curImg->tracedNeuron.seg[mainSeg.segID].row.end(), curImg->tracedNeuron.seg[branchSeg.segID].row.begin(), curImg->tracedNeuron.seg[branchSeg.segID].row.end());
			size_t branchN = mainSegLength + 1;
			for (vector<V_NeuronSWC_unit>::iterator itNextSeg = curImg->tracedNeuron.seg[mainSeg.segID].row.end() - 1;
				itNextSeg != curImg->tracedNeuron.seg[mainSeg.segID].row.begin() + ptrdiff_t(mainSegLength - 1); --itNextSeg)
			{
				itNextSeg->n = branchN;
				itNextSeg->seg_id = mainSeg.segID;
				itNextSeg->parent = branchN - 1;
				++branchN;
			}
			(curImg->tracedNeuron.seg[mainSeg.segID].row.end() - 1)->parent = (curImg->tracedNeuron.seg[mainSeg.segID].row.begin() + ptrdiff_t(mainSeg.head_tail - 2))->n;
			curImg->tracedNeuron.seg[branchSeg.segID].to_be_deleted = true;
			curImg->tracedNeuron.seg[branchSeg.segID].on = false;
		}

		// correcting types, based on the main segment type
		for (vector<V_NeuronSWC_unit>::iterator reID = curImg->tracedNeuron.seg[mainSeg.segID].row.begin();
			reID != curImg->tracedNeuron.seg[mainSeg.segID].row.end(); ++reID)
		{
			reID->seg_id = mainSeg.segID;
			reID->type = assignedType;
		}
	}	
	//////////////////////////////////////////// END of [BRANCHING CONNECTION] ////////////////////////////////////////////

	return;
}

void Renderer_gl1::connectSameTypeSegs(map<int, vector<int> >& inputSegMap, My4DImage*& curImgPtr)
{
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;       
	if (w) curImg = v3dr_getImage4d(_idep);

	float fMOSTxyResSquare = 1;// this->fragTraceParams.at("xyResRatio") * this->fragTraceParams.at("xyResRatio");
	float fMOSTzResSquare = 1;// this->fragTraceParams.at("zResRatio") * this->fragTraceParams.at("zResRatio");

	cout << " -- post elongation threshold: " << this->fragTraceParams.at("labeledDistThreshold") << endl;
	cout << " ---- xy resolution ratio: " << this->fragTraceParams.at("xyResRatio") << endl;
	cout << " ---- z resolution ratio: " << this->fragTraceParams.at("zResRatio") << endl;

	int oldSegCount, newSegCount;
	while (1)
	{ 
		oldSegCount = 0; newSegCount = 0;
		for (map<int, vector<int> >::iterator segCountIt = inputSegMap.begin(); segCountIt != inputSegMap.end(); ++segCountIt) 
			oldSegCount = oldSegCount + segCountIt->second.size();

		map<float, vector<segInfoUnit> > dist2segsMap;
		vector<segInfoUnit> segPair(2);
		segInfoUnit seg1, seg2;
		segPair[0] = seg1;
		segPair[1] = seg2;
		for (map<int, vector<int> >::iterator segTypeIt = inputSegMap.begin(); segTypeIt != inputSegMap.end(); ++segTypeIt)
		{
			cout << segTypeIt->first << ": ";
			for (vector<int>::iterator labeledSegIt = segTypeIt->second.begin(); labeledSegIt != segTypeIt->second.end(); ++labeledSegIt)
				cout << *labeledSegIt << "(" << curImgPtr->tracedNeuron.seg.at(*labeledSegIt).row.size() << ") ";
			cout << endl;

			if (segTypeIt->second.size() == 1) continue;

			for (vector<int>::iterator clusterSegIt1 = segTypeIt->second.begin(); clusterSegIt1 != segTypeIt->second.end() - 1; ++clusterSegIt1)
			{
				float minDist = 10000, dist;
				
				float seg1HeadX, seg1HeadY, seg1HeadZ, seg1TailX, seg1TailY, seg1TailZ, seg2HeadX, seg2HeadY, seg2HeadZ, seg2TailX, seg2TailY, seg2TailZ;
				for (vector<int>::iterator clusterSegIt2 = clusterSegIt1 + 1; clusterSegIt2 != segTypeIt->second.end(); ++clusterSegIt2)
				{
					segPair[0].segID = *clusterSegIt1;
					segPair[1].segID = *clusterSegIt2;

					seg1HeadX = (curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.end() - 1)->x;
					seg2HeadX = (curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.end() - 1)->x;
					seg1HeadY = (curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.end() - 1)->y;
					seg2HeadY = (curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.end() - 1)->y;
					seg1HeadZ = (curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.end() - 1)->z;
					seg2HeadZ = (curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.end() - 1)->z;
					dist = sqrt((seg1HeadX - seg2HeadX) * (seg1HeadX - seg2HeadX) + 
						(seg1HeadY - seg2HeadY) * (seg1HeadY - seg2HeadY) + (seg1HeadZ - seg2HeadZ) * (seg1HeadZ - seg2HeadZ));
					segPair[0].head_tail = -1;
					segPair[0].nodeCount = curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.size();
					segPair[1].head_tail = -1;
					segPair[1].nodeCount = curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.size();
					dist2segsMap.insert(pair<float, vector<segInfoUnit> >(dist, segPair));

					seg1HeadX = (curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.end() - 1)->x;
					seg2TailX = curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.begin()->x;
					seg1HeadY = (curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.end() - 1)->y;
					seg2TailY = curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.begin()->y;
					seg1HeadZ = (curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.end() - 1)->z;
					seg2TailZ = curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.begin()->z;
					dist = sqrt((seg1HeadX - seg2TailX) * (seg1HeadX - seg2TailX) + 
						(seg1HeadY - seg2TailY) * (seg1HeadY - seg2TailY) + (seg1HeadZ - seg2TailZ) * (seg1HeadZ - seg2TailZ));
					segPair[0].head_tail = -1;
					segPair[0].nodeCount = curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.size();
					segPair[1].head_tail = 2;
					segPair[1].nodeCount = curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.size();
					dist2segsMap.insert(pair<float, vector<segInfoUnit> >(dist, segPair));

					seg1TailX = curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.begin()->x;
					seg2HeadX = (curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.end() - 1)->x;
					seg1TailY = curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.begin()->y;
					seg2HeadY = (curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.end() - 1)->y;
					seg1TailZ = curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.begin()->z;
					seg2HeadZ = (curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.end() - 1)->z;
					dist = sqrt((seg1TailX - seg2HeadX) * (seg1TailX - seg2HeadX) + 
						(seg1TailY - seg2HeadY) * (seg1TailY - seg2HeadY) + (seg1TailZ - seg2HeadZ) * (seg1TailZ - seg2HeadZ));
					segPair[0].head_tail = 2;
					segPair[0].nodeCount = curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.size();
					segPair[1].head_tail = -1;
					segPair[1].nodeCount = curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.size();
					dist2segsMap.insert(pair<float, vector<segInfoUnit> >(dist, segPair));

					seg1TailX = curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.begin()->x;
					seg2TailX = curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.begin()->x;
					seg1TailY = curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.begin()->y;
					seg2TailY = curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.begin()->y;
					seg1TailZ = curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.begin()->z;
					seg2TailZ = curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.begin()->z;
					dist = sqrt((seg1TailX - seg2TailX) * (seg1TailX - seg2TailX) + 
						(seg1TailY - seg2TailY) * (seg1TailY - seg2TailY) + (seg1TailZ - seg2TailZ) * (seg1TailZ - seg2TailZ));
					segPair[0].head_tail = 2;
					segPair[0].nodeCount = curImgPtr->tracedNeuron.seg.at(*clusterSegIt1).row.size();
					segPair[1].head_tail = 2;
					segPair[1].nodeCount = curImgPtr->tracedNeuron.seg.at(*clusterSegIt2).row.size();
					dist2segsMap.insert(pair<float, vector<segInfoUnit> >(dist, segPair));
				}
			}

			//for (map<float, vector<segInfoUnit> >::iterator pairIt = dist2segsMap.begin(); pairIt != dist2segsMap.end(); ++pairIt) cout << pairIt->first << " ";
			//cout << endl;
			int seg1ID = dist2segsMap.begin()->second.begin()->segID;
			int seg2ID = (dist2segsMap.begin()->second.begin() + 1)->segID;
			//cout << " -- post elongation distance measured: " << dist2segsMap.begin()->first << " " << seg1ID << "_" << dist2segsMap.begin()->second.begin()->head_tail << " " << seg2ID << "_" << (dist2segsMap.begin()->second.begin() + 1)->head_tail << endl;
			//cout << "  -- seg1 head: " << (curImgPtr->tracedNeuron.seg.at(seg1ID).row.end() - 1)->x << " " << (curImgPtr->tracedNeuron.seg.at(seg1ID).row.end() - 1)->y << " " << (curImgPtr->tracedNeuron.seg.at(seg1ID).row.end() - 1)->z << endl;
			//cout << "  -- seg1 tail: " << curImgPtr->tracedNeuron.seg.at(seg1ID).row.begin()->x << " " << curImgPtr->tracedNeuron.seg.at(seg1ID).row.begin()->y << " " << curImgPtr->tracedNeuron.seg.at(seg1ID).row.begin()->z << endl;
			//cout << "  -- seg2 head: " << (curImgPtr->tracedNeuron.seg.at(seg2ID).row.end() - 1)->x << " " << (curImgPtr->tracedNeuron.seg.at(seg2ID).row.end() - 1)->y << " " << (curImgPtr->tracedNeuron.seg.at(seg2ID).row.end() - 1)->z << endl;
			//cout << "  -- seg2 tail: " << curImgPtr->tracedNeuron.seg.at(seg2ID).row.begin()->x << " " << curImgPtr->tracedNeuron.seg.at(seg2ID).row.begin()->y << " " << curImgPtr->tracedNeuron.seg.at(seg2ID).row.begin()->z << endl;
			if (dist2segsMap.begin()->first < this->fragTraceParams.at("labeledDistThreshold"))
			{
				vector<float> seg1Vec(3);
				vector<float> seg2Vec(3);
				if (dist2segsMap.begin()->second.begin()->head_tail == -1)
				{
					seg1Vec[0] = (curImgPtr->tracedNeuron.seg.at(seg1ID).row.end() - 1)->x - curImgPtr->tracedNeuron.seg.at(seg1ID).row.begin()->x;
					seg1Vec[1] = (curImgPtr->tracedNeuron.seg.at(seg1ID).row.end() - 1)->y - curImgPtr->tracedNeuron.seg.at(seg1ID).row.begin()->y;
					seg1Vec[2] = (curImgPtr->tracedNeuron.seg.at(seg1ID).row.end() - 1)->z - curImgPtr->tracedNeuron.seg.at(seg1ID).row.begin()->z;
				}
				else if (dist2segsMap.begin()->second.begin()->head_tail == 2)
				{
					seg1Vec[0] = curImgPtr->tracedNeuron.seg.at(seg1ID).row.begin()->x - (curImgPtr->tracedNeuron.seg.at(seg1ID).row.end() - 1)->x;
					seg1Vec[1] = curImgPtr->tracedNeuron.seg.at(seg1ID).row.begin()->y - (curImgPtr->tracedNeuron.seg.at(seg1ID).row.end() - 1)->y;
					seg1Vec[2] = curImgPtr->tracedNeuron.seg.at(seg1ID).row.begin()->z - (curImgPtr->tracedNeuron.seg.at(seg1ID).row.end() - 1)->z;
				}
				if ((dist2segsMap.begin()->second.begin() + 1)->head_tail == -1)
				{
					seg2Vec[0] = curImgPtr->tracedNeuron.seg.at(seg2ID).row.begin()->x - (curImgPtr->tracedNeuron.seg.at(seg2ID).row.end() - 1)->x;
					seg2Vec[1] = curImgPtr->tracedNeuron.seg.at(seg2ID).row.begin()->y - (curImgPtr->tracedNeuron.seg.at(seg2ID).row.end() - 1)->y;
					seg2Vec[2] = curImgPtr->tracedNeuron.seg.at(seg2ID).row.begin()->z - (curImgPtr->tracedNeuron.seg.at(seg2ID).row.end() - 1)->z;
				}
				else if ((dist2segsMap.begin()->second.begin() + 1)->head_tail == 2)
				{
					seg2Vec[0] = (curImgPtr->tracedNeuron.seg.at(seg2ID).row.end() - 1)->x - curImgPtr->tracedNeuron.seg.at(seg2ID).row.begin()->x;
					seg2Vec[1] = (curImgPtr->tracedNeuron.seg.at(seg2ID).row.end() - 1)->y - curImgPtr->tracedNeuron.seg.at(seg2ID).row.begin()->y;
					seg2Vec[2] = (curImgPtr->tracedNeuron.seg.at(seg2ID).row.end() - 1)->z - curImgPtr->tracedNeuron.seg.at(seg2ID).row.begin()->z;
				}

				float innerProduct = seg1Vec[0] * seg2Vec[0] + seg1Vec[1] * seg2Vec[1] + seg1Vec[2] * seg2Vec[2];
				//cout << innerProduct << endl;
				if (innerProduct >= 0) this->simpleConnectExecutor(curImgPtr, dist2segsMap.begin()->second);
				else
				{
					segTypeIt->second.erase(find(segTypeIt->second.begin(), segTypeIt->second.end(), seg1ID));
					segTypeIt->second.erase(find(segTypeIt->second.begin(), segTypeIt->second.end(), seg2ID));
				}
			}
			//cout << endl;

			if (curImgPtr->tracedNeuron.seg.at(seg1ID).to_be_deleted) segTypeIt->second.erase(find(segTypeIt->second.begin(), segTypeIt->second.end(), seg1ID));
			else if (curImgPtr->tracedNeuron.seg.at(seg2ID).to_be_deleted) segTypeIt->second.erase(find(segTypeIt->second.begin(), segTypeIt->second.end(), seg2ID));
			dist2segsMap.clear();
		}

		for (map<int, vector<int> >::iterator segCountIt = inputSegMap.begin(); segCountIt != inputSegMap.end(); ++segCountIt)
			newSegCount = newSegCount + segCountIt->second.size();

		if (oldSegCount == newSegCount) break;
	}
}
// --------- END of [Simple connecting tool (no geometrical analysis, only 2 segments at a time), MK, April, 2018] ---------

// --------- Remove duplicates and save sorted swc, Peng Xie, June, 2019 --------
//void VNeuron_to_Neuron(V_NeuronSWC_unit* vS, NeuronSWC* S){
//    S->n = vS->n;
//    S->type = vS->type;
//    S->x = vS->x;
//    S->y= vS->y;
//    S->z = vS->z;
//    S->r = vS->r;
//    S->pn = vS->parent;
//    S->seg_id = vS->seg_id;
//    S->level = vS->level;
//    S->creatmode = vS->creatmode;  // Creation Mode LMG 8/10/2018
//    S->timestamp = vS->timestamp;  // Timestamp LMG 27/9/2018
//    S->tfresindex = vS->tfresindex; // TeraFly resolution index LMG 13/12/2018
//}

struct SegmentSWC{
    int name;
    int parent;
    int size;

    SegmentSWC () {name=1; parent=-1; size=0;}
    SegmentSWC (int s_name, int s_parent, int s_size) {name=s_name; parent=s_parent; size=s_size;}
};

//int whether_reverse_child_segment(vector<V_NeuronSWC_unit> parent_seg, vector<V_NeuronSWC_unit> child_seg){
//    double phead_x = parent_seg.end().x;
//    double phead_y = parent_seg.end().y;
//    double phead_z = parent_seg.end().z;
//    double chead_x = child_seg.end().x;
//    double chead_y = child_seg.end().y;
//    double chead_z = child_seg.end().z;
//    double ptail_x = parent_seg.begin().x;
//    double ptail_y = parent_seg.begin().y;
//    double ptail_z = parent_seg.begin().z;
//    double ctail_x = child_seg.begin().x;
//    double ctail_y = child_seg.begin().y;
//    double ctail_z = child_seg.begin().z;
//    if (((phead_x==chead_x) && (phead_y==chead_y) && (phead_z==chead_z)) ||
//            ((ptail_x==chead_x) && (ptail_y==chead_y) && (ptail_z==chead_z)))
//    {
//        return true;
//    }
//    return false;
//}

void Renderer_gl1::sort_tracedNeuron(My4DImage* curImg, size_t rootID)
{
    // Assert given rootID is valid
    NeuronTree* p_tree = (NeuronTree*)(&(listNeuronTree.at(0)));
    if(!p_tree)return;
    QList<NeuronSWC> p_listneuron = p_tree->listNeuron;
    if(p_tree->hashNeuron.find(rootID)==p_tree->hashNeuron.end())return;
    v3d_msg(QString("p_tree->hashNeuron.value(rootID): %1").arg(p_tree->hashNeuron.value(rootID)));

    // ----------- Starting from the specified root, traverse the connected components ---------
    NeuronTree sorted_nt;
    QList<NeuronSWC> listNeuron;
    QHash<int, int> hashNeuron;
    int test_type = 0; // Testing Only
    int cur_treesize;

    // ----------- Sort tree structure by segments -------------

    // If root is an internal node, then we need to split the segment
    // To be implemented
    // ------------ Begin DFS ------------
    QStack<SegmentSWC> pstack;
    set<size_t> visited;
    size_t cur_segID = p_listneuron.at(p_tree->hashNeuron.value(rootID)).seg_id;
    vector<V_NeuronSWC_unit> curSeg = curImg->tracedNeuron.seg[cur_segID].row;
    visited.insert(cur_segID);
    pstack.push(SegmentSWC(cur_segID, -1, curSeg.size()));
    cur_treesize = listNeuron.size();
    for (vector<V_NeuronSWC_unit>::iterator unitIt = curSeg.begin(); unitIt != curSeg.end(); ++unitIt)
    {
        NeuronSWC S;
        S.n = unitIt->n + cur_treesize;
//        S.type = unitIt->type;
        S.type = test_type;
        S.x = unitIt->x;
        S.y= unitIt->y;
        S.z = unitIt->z;
        S.r = unitIt->r;
        S.pn = unitIt==curSeg.begin()? (-1):((unitIt-1)->n + cur_treesize);
        S.seg_id = unitIt->seg_id;
//        S.level = unitIt->level;
        S.level = test_type;
        S.creatmode = unitIt->creatmode;  // Creation Mode LMG 8/10/2018
        S.timestamp = unitIt->timestamp;  // Timestamp LMG 27/9/2018
        S.tfresindex = unitIt->tfresindex; // TeraFly resolution index LMG 13/12/2018
        listNeuron.append(S);
        hashNeuron.insert(S.n, listNeuron.size()-1);
    }
    test_type++;
//    v3d_msg(QString("Inserted segment: %1").arg(cur_segID));

    bool new_push;
    int parent_segID, child_segID;
    while(!pstack.isEmpty()){
        new_push = false;
        /* --------- Find segments that are connected to the head or tail of input segment --------- */
        cur_segID = pstack.top().name;
//        v3d_msg(QString("Finding children of: %1").arg(cur_segID));
        set<size_t> curSegEndRegionSegs;
        curSegEndRegionSegs.clear();
        curSegEndRegionSegs = this->segEndRegionCheck(curImg, cur_segID);
        parent_segID = pstack.top().parent;
        set<size_t> sibling_segs;
        sibling_segs.clear();
        if(parent_segID!=(-1)) sibling_segs = this->segEndRegionCheck(curImg, parent_segID);

        if (!curSegEndRegionSegs.empty())
        {
            for (set<size_t>::iterator child_segIDIt = curSegEndRegionSegs.begin(); child_segIDIt != curSegEndRegionSegs.end(); ++child_segIDIt)
            {
                child_segID = *child_segIDIt;
                if (child_segID == cur_segID) continue;
                else if (visited.find(child_segID) != visited.end())
                {
                    // If already visited
                    continue;
                }
                else if (sibling_segs.find(child_segID) != sibling_segs.end()){
                    // If is the child of the same parent
                    continue;
                }
                else
                {
                    vector<V_NeuronSWC_unit> child_seg = curImg->tracedNeuron.seg[child_segID].row;
                    visited.insert(child_segID);
                    pstack.push(SegmentSWC(child_segID, cur_segID, child_seg.size()));
                    new_push = true;
                    // Whether to reverse the child segment

                    cur_treesize = listNeuron.size();
                    for (vector<V_NeuronSWC_unit>::iterator unitIt = child_seg.begin(); unitIt != child_seg.end(); ++unitIt)
                    {
                        NeuronSWC S;
                        S.n = unitIt->n + cur_treesize;
                //        S.type = unitIt->type;
                        S.type = test_type;
                        S.x = unitIt->x;
                        S.y= unitIt->y;
                        S.z = unitIt->z;
                        S.r = unitIt->r;
                        S.pn = unitIt==child_seg.begin()? (-1):((unitIt-1)->n + cur_treesize);
                        S.seg_id = unitIt->seg_id;
//                        S.level = unitIt->level;
                        S.level = test_type;
                        S.creatmode = unitIt->creatmode;  // Creation Mode LMG 8/10/2018
                        S.timestamp = unitIt->timestamp;  // Timestamp LMG 27/9/2018
                        S.tfresindex = unitIt->tfresindex; // TeraFly resolution index LMG 13/12/2018
                        listNeuron.append(S);
                        hashNeuron.insert(S.n, listNeuron.size()-1);
                    }
                    test_type++;
                    qDebug()<<QString("Inserted segment: %1").arg(child_segID);
                    break;
                }
            }
        }
        if(!new_push){
            pstack.pop();
        }
    }

    // ----------- END: Sort tree structure by segments -------------

    sorted_nt.n = 1; //only one neuron if read from a file
    sorted_nt.listNeuron = listNeuron;
    sorted_nt.hashNeuron = hashNeuron;
    sorted_nt.color = XYZW(0,0,0,0); /// alpha==0 means using default neuron color, 081115
    writeSWC_file(QString("C:/Users/pengx/Desktop/test.sorted.swc"), sorted_nt);

    return;
}

// --------- Highlight the selected segment with its downstream subtree, MK, June, 2018 ---------
void Renderer_gl1::showConnectedSegs()
{
	// This method highlighs all connected segments with regard to the stroked segment when [alt + N] is pressed.
	// -- MK, June, 2018

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
	XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);
	
	w->subtreeHighlightModeMonitor(); // Switch on subtree highlighting mode monitor in v3dr_glwidget.

	//for (vector<V_NeuronSWC_unit>::iterator tempIt = curImg->tracedNeuron.seg.at(517).row.begin(); tempIt != curImg->tracedNeuron.seg.at(517).row.end(); ++tempIt)
	//	tempIt->type = 0; // --> this is for debug purpose
	//curImg->update_3drenderer_neuron_view(w, this); 

	float tolerance = 20; // tolerance distance from the backprojected neuron to the curve point

//    v3d_msg(QString("listNeuronTree.size(): %1").arg(listNeuronTree.size()));
    for (V3DLONG j = 0; j < listNeuronTree.size(); j++)
	{
		NeuronTree* p_tree = (NeuronTree*)(&(listNeuronTree.at(j))); //curEditingNeuron-1
		if (p_tree && p_tree->editable)    // @FIXED by Alessandro on 2015-05-23. Removing segments from non-editable neurons causes crash.
		{
			QList<NeuronSWC>* p_listneuron = &(p_tree->listNeuron);
			if (!p_listneuron) return;
			// for (int testi=0; testi<list_listCurvePos.at(0).size(); testi++) qDebug() << list_listCurvePos.at(0).at(testi).x << " " << list_listCurvePos.at(0).at(testi).y;

			/***************** Get segment information included in the movePen trajectory *****************/
			/* ------- Only take in the nodes within the rectangle that contains the stroke ------- */
			long minX = list_listCurvePos.at(0).at(0).x, maxX = list_listCurvePos.at(0).at(0).x;
			long minY = list_listCurvePos.at(0).at(0).y, maxY = list_listCurvePos.at(0).at(0).y;
			for (size_t i = 0; i < list_listCurvePos.at(0).size(); ++i)
			{
				if (list_listCurvePos.at(0).at(i).x <= minX) minX = list_listCurvePos.at(0).at(i).x;
				if (list_listCurvePos.at(0).at(i).x >= maxX) maxX = list_listCurvePos.at(0).at(i).x;
				if (list_listCurvePos.at(0).at(i).y <= minY) minY = list_listCurvePos.at(0).at(i).y;
				if (list_listCurvePos.at(0).at(i).y >= maxY) maxY = list_listCurvePos.at(0).at(i).y;
			}
			minX = minX - 5; maxX = maxX + 5;
			minY = minY - 5; maxY = maxY + 5;
			//cout << minX << " " << maxX << " " << minY << " " << maxY << endl;
			QList<NeuronSWC> nodeOnStroke;
			for (size_t i = 0; i < p_listneuron->size(); ++i)
			{
				GLdouble px, py, pz, ix, iy, iz;
				ix = p_listneuron->at(i).x;
				iy = p_listneuron->at(i).y;
				iz = p_listneuron->at(i).z;
				if (gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
				{
					py = viewport[3] - py; //the Y axis is reversed
					QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));
					if ((p.x() >= minX && p.x() <= maxX) && (p.y() >= minY && p.y() <= maxY))
					{
						nodeOnStroke.push_back(p_listneuron->at(i));
						//cout << p.x() << " " << p.y() << endl;
					}
				}
			}
			if (nodeOnStroke.size() == 0) return;
			/* ------- END of [Only take in the nodes within the rectangle that contains the stroke] ------- */
//            size_t rootID = nodeOnStroke.at(0).n;
//            v3d_msg(QString("Root ID: %1").arg(rootID));
//            this->sort_tracedNeuron(curImg, rootID);
//            v3d_msg("Sorting finished");
//            return;


			/* ------- Acquire the starting segment ------- */
			vector<size_t> involvedSegs;
			float nearestDist = 1000;
			NeuronSWC nearestNode;
			for (V3DLONG i = 0; i < list_listCurvePos.at(0).size(); i++)
			{
				for (V3DLONG j = 0; j < nodeOnStroke.size(); j++)
				{
					GLdouble px, py, pz, ix, iy, iz;
					ix = nodeOnStroke.at(j).x;
					iy = nodeOnStroke.at(j).y;
					iz = nodeOnStroke.at(j).z;
					if (gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
					{
						py = viewport[3] - py; //the Y axis is reversed
						QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));

						QPointF p2(list_listCurvePos.at(0).at(i).x, list_listCurvePos.at(0).at(i).y);
						float dist = std::sqrt((p.x() - p2.x())*(p.x() - p2.x()) + (p.y() - p2.y())*(p.y() - p2.y()));
						if (std::sqrt((p.x() - p2.x())*(p.x() - p2.x()) + (p.y() - p2.y())*(p.y() - p2.y())) <= tolerance) // p: global coordinates; p2: view port coordinates
						{
							if (dist < nearestDist)
							{
								nearestDist = dist;
								nearestNode = nodeOnStroke.at(j);
							}
							involvedSegs.push_back(nodeOnStroke.at(j).seg_id);
						}
					}
				}
			}
			map<size_t, int> segIDCountMap;
			for (vector<size_t>::iterator segIt = involvedSegs.begin(); segIt != involvedSegs.end(); ++segIt)
				++segIDCountMap.insert(pair<size_t, int>(*segIt, 0)).first->second;
			int segCount = segIDCountMap.begin()->second;
			size_t startingSegID = segIDCountMap.begin()->first;
			for (map<size_t, int>::iterator mapIt = segIDCountMap.begin(); mapIt != segIDCountMap.end(); ++mapIt)
			{
				if (mapIt->second > segCount)
				{
					segCount = mapIt->second;
					startingSegID = mapIt->first; // <-- This is the starting point of the subtree to be highlighted.
				}
			}
			/* ------- END of [Acquire the starting segment] ------- */
			/***************** END of [Get segment information included in the movePen trajectory] *****************/

			/* --------------------- Start finding connected segments --------------------- */
			this->segEnd2SegIDmapping(curImg);
			
			this->subtreeSegs.clear();
			this->subtreeSegs.insert(startingSegID);
			this->rc_findConnectedSegs(curImg, startingSegID);
            int color_code = 0;
			for (set<size_t>::iterator segIt = this->subtreeSegs.begin(); segIt != this->subtreeSegs.end(); ++segIt)
			{
				this->originalSegMap.insert(pair<size_t, vector<V_NeuronSWC_unit> >(*segIt, curImg->tracedNeuron.seg[*segIt].row));
                cout << *segIt << " "<<curImg->tracedNeuron.seg[*segIt].row.size()<<endl;
				for (vector<V_NeuronSWC_unit>::iterator unitIt = curImg->tracedNeuron.seg[*segIt].row.begin(); unitIt != curImg->tracedNeuron.seg[*segIt].row.end(); ++unitIt)
                    unitIt->type = color_code;
//                color_code++;
				this->highlightedSegMap.insert(pair<size_t, vector<V_NeuronSWC_unit> >(*segIt, curImg->tracedNeuron.seg[*segIt].row));
//                boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
//                my_sleep(500);
            }
			//cout << endl;
			/* ----------------- END of [Start finding connected segments] ----------------- */

            curImg->update_3drenderer_neuron_view(w, this);
			//curImg->proj_trace_history_append(); // -> Highlighting is for temporary checking purpose, should not be appended to the history.
		}
	}

	this->pressedShowSubTree = true;
}

void Renderer_gl1::rc_findConnectedSegs(My4DImage* curImg, size_t inputSegID)
{
	// This method recursively finds segments that are connected with given input segment ID.
	// -- MK, June, 2018

	//cout << endl << "INPUT SEGID:" << inputSegID << endl;

	if (curImg->tracedNeuron.seg[inputSegID].to_be_deleted) return;

	size_t curSegNum = this->subtreeSegs.size();

	// -- obtaining inputSegID head gridKey and tail gridKey
	double xLabelTail = curImg->tracedNeuron.seg[inputSegID].row.begin()->x;
	double yLabelTail = curImg->tracedNeuron.seg[inputSegID].row.begin()->y;
	double zLabelTail = curImg->tracedNeuron.seg[inputSegID].row.begin()->z;
	double xLabelHead = (curImg->tracedNeuron.seg[inputSegID].row.end() - 1)->x;
	double yLabelHead = (curImg->tracedNeuron.seg[inputSegID].row.end() - 1)->y;
	double zLabelHead = (curImg->tracedNeuron.seg[inputSegID].row.end() - 1)->z;
	QString key1Q = QString::number(xLabelTail) + "_" + QString::number(yLabelTail) + "_" + QString::number(zLabelTail);
	string key1 = key1Q.toStdString();
	QString key2Q = QString::number(xLabelHead) + "_" + QString::number(yLabelHead) + "_" + QString::number(zLabelHead);
	string key2 = key2Q.toStdString();

	/* --------- Find segments that are connected in the middle of input segment --------- */
	if (curImg->tracedNeuron.seg[inputSegID].row.size() > 2)
	{
		for (vector<V_NeuronSWC_unit>::iterator unitIt = curImg->tracedNeuron.seg[inputSegID].row.begin() + 1; unitIt != curImg->tracedNeuron.seg[inputSegID].row.end() - 1; ++unitIt)
		{
			double middleX = unitIt->x;
			double middleY = unitIt->y;
			double middleZ = unitIt->z;
			QString middleNodeKeyQ = QString::number(middleX) + "_" + QString::number(middleY) + "_" + QString::number(middleZ);
			string middleNodeKey = middleNodeKeyQ.toStdString();

			pair<multimap<string, size_t>::iterator, multimap<string, size_t>::iterator> middleRange = this->segEnd2segIDmap.equal_range(middleNodeKey);
			for (multimap<string, size_t>::iterator middleIt = middleRange.first; middleIt != middleRange.second; ++middleIt)
			{
				if (middleIt->second == inputSegID) continue;
				else if (this->subtreeSegs.find(middleIt->second) != this->subtreeSegs.end())
				{
					//cout << "  --> already picked, move to the next." << endl;
					continue;
				}
				else if (middleIt->first == middleNodeKey)
				{
					//cout << "  Found a segment in the middle of the route, adding it to the recursive searching process:" << middleNodeKey << " " << middleIt->second << endl;
					if (curImg->tracedNeuron.seg[middleIt->second].to_be_deleted) continue;
					this->subtreeSegs.insert(middleIt->second);
					this->rc_findConnectedSegs(curImg, middleIt->second);
				}
			}
		}
	}
	/* ------- END of [Find segments that are connected in the middle of input segment] ------- */

	/* --------- Find segments that are connected to the head or tail of input segment --------- */
	set<size_t> curSegEndRegionSegs;
	curSegEndRegionSegs.clear();
	curSegEndRegionSegs = this->segEndRegionCheck(curImg, inputSegID);
	//cout << curSegEndRegionSegs.size() << endl;
	if (!curSegEndRegionSegs.empty())
	{
		for (set<size_t>::iterator regionSegIt = curSegEndRegionSegs.begin(); regionSegIt != curSegEndRegionSegs.end(); ++regionSegIt)
		{	
			//cout << "  testing segs at the end region:" << *regionSegIt << endl;
			if (*regionSegIt == inputSegID) continue;
			else if (this->subtreeSegs.find(*regionSegIt) != this->subtreeSegs.end())
			{
				//cout << "  --> already picked, move to the next." << endl;
				continue;
			}
			else
			{
				//cout << "    ==> segs at the end region added:" << *regionSegIt << endl;
				if (curImg->tracedNeuron.seg[*regionSegIt].to_be_deleted) continue;
				
				this->subtreeSegs.insert(*regionSegIt);
				this->rc_findConnectedSegs(curImg, *regionSegIt);
			}
		}
	}
	/* ------- END of [Find segments that are connected to the head or tail of input segment] ------- */

	if (this->subtreeSegs.size() == curSegNum) return;
}

set<size_t> Renderer_gl1::segEndRegionCheck(My4DImage* curImg, size_t inputSegID)
{
	// This method picks up any segments that run through the head or tail of input segment using grid-seg approach.
	// -- MK, June 2018

	set<size_t> otherConnectedSegs;
	otherConnectedSegs.clear();

	int xHead = (curImg->tracedNeuron.seg[inputSegID].row.end() - 1)->x / this->gridLength;
	int yHead = (curImg->tracedNeuron.seg[inputSegID].row.end() - 1)->y / this->gridLength;
	int zHead = (curImg->tracedNeuron.seg[inputSegID].row.end() - 1)->z / this->gridLength;
	int xTail = curImg->tracedNeuron.seg[inputSegID].row.begin()->x / this->gridLength;
	int yTail = curImg->tracedNeuron.seg[inputSegID].row.begin()->y / this->gridLength;
	int zTail = curImg->tracedNeuron.seg[inputSegID].row.begin()->z / this->gridLength;
	QString gridKeyHeadQ = QString::number(xHead) + "_" + QString::number(yHead) + "_" + QString::number(zHead);
	string gridKeyHead = gridKeyHeadQ.toStdString();
	QString gridKeyTailQ = QString::number(xTail) + "_" + QString::number(yTail) + "_" + QString::number(zTail);
	string gridKeyTail = gridKeyTailQ.toStdString();

	set<size_t> headRegionSegs = this->wholeGrid2segIDmap[gridKeyHead];
	set<size_t> tailRegionSegs = this->wholeGrid2segIDmap[gridKeyTail];

	//cout << " Head region segs:";
	for (set<size_t>::iterator headIt = headRegionSegs.begin(); headIt != headRegionSegs.end(); ++headIt)
	{
		if (*headIt == inputSegID || curImg->tracedNeuron.seg[*headIt].to_be_deleted) continue;
		//cout << *headIt << " ";
		for (vector<V_NeuronSWC_unit>::iterator nodeIt = curImg->tracedNeuron.seg[*headIt].row.begin(); nodeIt != curImg->tracedNeuron.seg[*headIt].row.end(); ++nodeIt)
		{
			if (nodeIt->x == (curImg->tracedNeuron.seg[inputSegID].row.end() - 1)->x && nodeIt->y == (curImg->tracedNeuron.seg[inputSegID].row.end() - 1)->y && nodeIt->z == (curImg->tracedNeuron.seg[inputSegID].row.end() - 1)->z)
				otherConnectedSegs.insert(*headIt);
		}
	}
	//cout << endl << " Tail region segs:";
	for (set<size_t>::iterator tailIt = tailRegionSegs.begin(); tailIt != tailRegionSegs.end(); ++tailIt)
	{
		if (*tailIt == inputSegID || curImg->tracedNeuron.seg[*tailIt].to_be_deleted) continue;
		//cout << *tailIt << " ";
		for (vector<V_NeuronSWC_unit>::iterator nodeIt = curImg->tracedNeuron.seg[*tailIt].row.begin(); nodeIt != curImg->tracedNeuron.seg[*tailIt].row.end(); ++nodeIt)
		{
			if (nodeIt->x == curImg->tracedNeuron.seg[inputSegID].row.begin()->x && nodeIt->y == curImg->tracedNeuron.seg[inputSegID].row.begin()->y && nodeIt->z == curImg->tracedNeuron.seg[inputSegID].row.begin()->z)
				otherConnectedSegs.insert(*tailIt);
		}
	}
	//cout << endl;

	return otherConnectedSegs;
}

void Renderer_gl1::loopDetection()
{
#ifdef Q_OS_WIN32
	char* numProcsC;
	numProcsC = getenv("NUMBER_OF_PROCESSORS");
	string numProcsString(numProcsC);
	int numProcs = stoi(numProcsString);
#endif

	this->connectEdit = loopEdit;

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
	XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

	this->seg2SegsMap.clear();
	this->segTail2segIDmap.clear();
	for (set<size_t>::iterator it = subtreeSegs.begin(); it != subtreeSegs.end(); ++it)
	{
		set<size_t> connectedSegs;
		connectedSegs.clear();
		if (curImg->tracedNeuron.seg[*it].row.size() <= 1)
		{
			curImg->tracedNeuron.seg[*it].to_be_deleted = true;
			continue;
		}
		else if (curImg->tracedNeuron.seg[*it].to_be_deleted) continue;

		for (vector<V_NeuronSWC_unit>::iterator nodeIt = curImg->tracedNeuron.seg[*it].row.begin(); nodeIt != curImg->tracedNeuron.seg[*it].row.end(); ++nodeIt)
		{
			int xLabel = nodeIt->x / this->gridLength;
			int yLabel = nodeIt->y / this->gridLength;
			int zLabel = nodeIt->z / this->gridLength;
			QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
			string gridKey = gridKeyQ.toStdString();

			set<size_t> scannedSegs = this->wholeGrid2segIDmap[gridKey];
			if (!scannedSegs.empty())
			{
				for (set<size_t>::iterator scannedIt = scannedSegs.begin(); scannedIt != scannedSegs.end(); ++scannedIt)
				{
					int connectedSegsSize = connectedSegs.size();
					if (*scannedIt == *it || curImg->tracedNeuron.seg[*scannedIt].to_be_deleted) continue;
					if (curImg->tracedNeuron.seg[*scannedIt].row.size() == 1) continue;

					if (curImg->tracedNeuron.seg[*scannedIt].row.begin()->x == nodeIt->x && curImg->tracedNeuron.seg[*scannedIt].row.begin()->y == nodeIt->y && curImg->tracedNeuron.seg[*scannedIt].row.begin()->z == nodeIt->z)
					{
						connectedSegs.insert(*scannedIt);
						set<size_t> reversed;
						reversed.insert(*it);
						if (!this->seg2SegsMap.insert(pair<size_t, set<size_t> >(*scannedIt, reversed)).second) this->seg2SegsMap[*scannedIt].insert(*it);
						if (!this->segTail2segIDmap.insert(pair<size_t, set<size_t> >(*scannedIt, reversed)).second) this->segTail2segIDmap[*scannedIt].insert(*it);
					}
					else if ((curImg->tracedNeuron.seg[*scannedIt].row.end() - 1)->x == nodeIt->x && (curImg->tracedNeuron.seg[*scannedIt].row.end() - 1)->y == nodeIt->y && (curImg->tracedNeuron.seg[*scannedIt].row.end() - 1)->z == nodeIt->z)
					{
						connectedSegs.insert(*scannedIt);
						set<size_t> reversed;
						reversed.insert(*it);
						if (!this->seg2SegsMap.insert(pair<size_t, set<size_t> >(*scannedIt, reversed)).second) this->seg2SegsMap[*scannedIt].insert(*it);
					}
				}
			}
		}
		if (!this->seg2SegsMap.insert(pair<size_t, set<size_t> >(*it, connectedSegs)).second)
		{
			for (set<size_t>::iterator otherSegIt = connectedSegs.begin(); otherSegIt != connectedSegs.end(); ++otherSegIt)
				this->seg2SegsMap[*it].insert(*otherSegIt);
		}
	}

	w->progressBarPtr = new QProgressBar;
	w->progressBarPtr->show();
	w->progressBarPtr->move(w->x() + w->width() - w->progressBarPtr->width() / 2, w->y() + w->height() - w->progressBarPtr->height() / 2);
	w->progressBarPtr->setTextVisible(true);
	w->progressBarPtr->setAlignment(Qt::AlignCenter);
	w->progressBarPtr->setFixedWidth(350);
	double segSize = curImg->tracedNeuron.seg.size();
	
	cout << endl << "Starting loop detection.. " << endl;
	clock_t begin = clock();
	this->detectedLoopsSet.clear();
	this->finalizedLoopsSet.clear();
	this->nonLoopErrors.clear();
//#ifdef Q_OS_WIN32
//#pragma omp parallel num_threads(numProcs)
//	{
//#endif
		for (map<size_t, set<size_t> >::iterator it = this->seg2SegsMap.begin(); it != this->seg2SegsMap.end(); ++it)
		{
			double progressBarValue = (double(it->first) / segSize) * 100;
			int progressBarValueInt = floor(progressBarValue);

			if (it->second.empty()) continue;
			else if (it->second.size() <= 2) continue;

			vector<size_t> loops2ThisSeg;
			loops2ThisSeg.clear();

			cout << "Starting segment: " << it->first << " ==> ";
			for (set<size_t>::iterator seg2SegsIt = this->seg2SegsMap[it->first].begin(); seg2SegsIt != this->seg2SegsMap[it->first].end(); ++seg2SegsIt)
				cout << *seg2SegsIt << " ";
			cout << endl;

			int loopCount = this->finalizedLoopsSet.size();

			this->rc_loopPathCheck(it->first, loops2ThisSeg, curImg);

			w->progressBarPtr->setValue(progressBarValueInt);
			w->progressBarPtr->setFormat("detecting loops.. " + QString::number(progressBarValueInt) + "%");

			if (this->finalizedLoopsSet.size() - loopCount == 0) cout << " -- no loops detected with this starting seg." << endl;
			else cout << this->finalizedLoopsSet.size() - loopCount << " loops detected with seg " << it->first << endl << endl;
		}
//#ifdef Q_OS_WIN32
//	}
//#endif
	w->progressBarPtr->setValue(100);
	w->progressBarPtr->setFormat(QString::number(100) + "%");
	w->progressBarPtr->close();

	clock_t end = clock();
	float elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	cout << "TIME ELAPSED: " << elapsed_secs << " SECS" << endl << endl;

	if (this->finalizedLoopsSet.empty()) return;
	else
	{
		set<size_t> loopSegsChecked;
		int loopCount = 0;
		for (set<set<size_t> >::iterator loopIt = this->finalizedLoopsSet.begin(); loopIt != this->finalizedLoopsSet.end(); ++loopIt)
		{
			set<size_t> thisLoop = *loopIt;
			int jointCount = 0;
			vector<size_t> thisSet;
			for (set<size_t>::iterator setIt = loopIt->begin(); setIt != loopIt->end(); ++setIt) thisSet.push_back(*setIt);

			V_NeuronSWC_unit jointNode;
			for (vector<V_NeuronSWC_unit>::iterator seg1It = curImg->tracedNeuron.seg.at(thisSet.at(0)).row.begin(); seg1It != curImg->tracedNeuron.seg.at(thisSet.at(1)).row.end(); ++seg1It)
			{
				for (vector<V_NeuronSWC_unit>::iterator seg2It = curImg->tracedNeuron.seg.at(thisSet.at(1)).row.begin(); seg2It != curImg->tracedNeuron.seg.at(thisSet.at(1)).row.end(); ++seg2It)
				{
					if (seg1It->x == seg2It->x && seg1It->y == seg2It->y && seg1It->z == seg2It->z)
					{
						jointNode = *seg1It;
						jointCount = 2;
						goto FOUND_JOINT_NODE;
					}
				}
			}

			++loopCount;		
			for (set<size_t>::iterator it = thisLoop.begin(); it != thisLoop.end(); ++it)
			{
				cout << *it << " ";
				for (vector<V_NeuronSWC_unit>::iterator unitIt = curImg->tracedNeuron.seg[*it].row.begin(); unitIt != curImg->tracedNeuron.seg[*it].row.end(); ++unitIt)
					unitIt->type = 6; //changed to be yellow by ZZ 04022019
				loopSegsChecked.insert(*it);
			}
			cout << endl << endl;
			continue;

		FOUND_JOINT_NODE:
			for (vector<size_t>::iterator segIt = thisSet.begin() + 2; segIt != thisSet.end(); ++segIt)
			{
				for (vector<V_NeuronSWC_unit>::iterator nodeIt = curImg->tracedNeuron.seg.at(*segIt).row.begin(); nodeIt != curImg->tracedNeuron.seg.at(*segIt).row.end(); ++nodeIt)
				{
					if (nodeIt->x == jointNode.x && nodeIt->y == jointNode.y && nodeIt->z == jointNode.z) ++jointCount;
				}
			}

			if (jointCount == loopIt->size()) this->nonLoopErrors.insert(*loopIt);
			else
			{
				++loopCount;
				for (set<size_t>::iterator it = thisLoop.begin(); it != thisLoop.end(); ++it)
				{
					cout << *it << " ";
					for (vector<V_NeuronSWC_unit>::iterator unitIt = curImg->tracedNeuron.seg[*it].row.begin(); unitIt != curImg->tracedNeuron.seg[*it].row.end(); ++unitIt)
						unitIt->type = 6; //changed to be yellow by ZZ 04022019
					loopSegsChecked.insert(*it);
				}
				cout << endl << endl;
				continue;
			}
		}
		cout << "LOOPS NUMBER (set): " << loopCount << endl << endl;

		if (!this->nonLoopErrors.empty())
		{
			for (set<set<size_t> >::iterator loopIt = this->nonLoopErrors.begin(); loopIt != this->nonLoopErrors.end(); ++loopIt)
			{
				set<size_t> thisLoop = *loopIt;
				for (set<size_t>::iterator it = thisLoop.begin(); it != thisLoop.end(); ++it)
				{
					cout << *it << " ";
					if (loopSegsChecked.find(*it) == loopSegsChecked.end())
					{
						for (vector<V_NeuronSWC_unit>::iterator unitIt = curImg->tracedNeuron.seg[*it].row.begin(); unitIt != curImg->tracedNeuron.seg[*it].row.end(); ++unitIt)
							unitIt->type = 20;
					}
				}
				cout << endl << endl;
			}
		}
		cout << "non LOOPS ERROR NUMBER (set): " << this->nonLoopErrors.size() << endl << endl;
	}

	curImg->update_3drenderer_neuron_view(w, this);
}

void Renderer_gl1::rc_loopPathCheck(size_t inputSegID, vector<size_t> curPathWalk, My4DImage* curImg)
{
	if (this->seg2SegsMap[inputSegID].size() < 2) return;

	//cout << "  input seg num: " << inputSegID << " ";
	curPathWalk.push_back(inputSegID);
	/*for (vector<size_t>::iterator curPathIt = curPathWalk.begin(); curPathIt != curPathWalk.end(); ++curPathIt)
		cout << *curPathIt << " ";
	cout << endl << endl;*/

	for (set<size_t>::iterator it = this->seg2SegsMap[inputSegID].begin(); it != this->seg2SegsMap[inputSegID].end(); ++it)
	{
		if (this->segTail2segIDmap.find(*it) == this->segTail2segIDmap.end()) continue;

		// ------- duplicated root detection ------- //
		if (curPathWalk.size() >= 2 && *it == *(curPathWalk.end() - 2))
		{
			V_NeuronSWC_unit headUnit = *(curImg->tracedNeuron.seg[*it].row.end() - 1);
			V_NeuronSWC_unit tailUnit = *curImg->tracedNeuron.seg[*it].row.begin();

			bool headCheck = false, tailCheck = false;
			for (vector<V_NeuronSWC_unit>::iterator it2 = curImg->tracedNeuron.seg[*(curPathWalk.end() - 1)].row.begin(); it2 != curImg->tracedNeuron.seg[*(curPathWalk.end() - 1)].row.end(); ++it2)
			{
				if (it2->x == headUnit.x && it2->y == headUnit.y && it2->z == headUnit.z) headCheck = true;
				if (it2->x == tailUnit.x && it2->y == tailUnit.y && it2->z == tailUnit.z) tailCheck = true;
			}

			if (headCheck == true && tailCheck == true)
			{
				set<size_t> detectedLoopPathSet;
				detectedLoopPathSet.clear();
				for (vector<size_t>::iterator loopIt = find(curPathWalk.begin(), curPathWalk.end(), *it); loopIt != curPathWalk.end(); ++loopIt)
					detectedLoopPathSet.insert(*loopIt);

				if (this->detectedLoopsSet.insert(detectedLoopPathSet).second)
				{
					this->nonLoopErrors.insert(detectedLoopPathSet);
					continue;
				}
				else return;
			}
			else continue;
		}
		// --------------------------------------- //

		if (find(curPathWalk.begin(), curPathWalk.end(), *it) == curPathWalk.end())
		{
			this->rc_loopPathCheck(*it, curPathWalk, curImg);
		}
		else
		{
			// a loop is found
			set<size_t> detectedLoopPathSet;
			detectedLoopPathSet.clear();
			for (vector<size_t>::iterator loopIt = find(curPathWalk.begin(), curPathWalk.end(), *it); loopIt != curPathWalk.end(); ++loopIt)
				detectedLoopPathSet.insert(*loopIt);

			if (this->detectedLoopsSet.insert(detectedLoopPathSet).second)
			{
				// pusedoloop by fork intersection check
				cout << "pusedoloop check.." << endl;

				if (*(curPathWalk.end() - 3) == *it) // exclude 3-way intersection
				{
					if (this->seg2SegsMap[*(curPathWalk.end() - 2)].find(*it) != this->seg2SegsMap[*(curPathWalk.end() - 2)].end())
					{
						vector<V_NeuronSWC_unit> forkCheck;
						forkCheck.push_back(*(curImg->tracedNeuron.seg[*(curPathWalk.end() - 1)].row.end() - 1));
						forkCheck.push_back(*curImg->tracedNeuron.seg[*(curPathWalk.end() - 1)].row.begin());
						forkCheck.push_back(*(curImg->tracedNeuron.seg[*(curPathWalk.end() - 2)].row.end() - 1));
						forkCheck.push_back(*curImg->tracedNeuron.seg[*(curPathWalk.end() - 2)].row.begin());

						int headConnectedCount = 0;
						for (vector<V_NeuronSWC_unit>::iterator checkIt = forkCheck.begin(); checkIt != forkCheck.end(); ++checkIt)
						{
							if (checkIt->x == (curImg->tracedNeuron.seg[*it].row.end() - 1)->x && checkIt->y == (curImg->tracedNeuron.seg[*it].row.end() - 1)->y && checkIt->z == (curImg->tracedNeuron.seg[*it].row.end() - 1)->z)
								++headConnectedCount;
						}

						int tailConnectedCount = 0;
						for (vector<V_NeuronSWC_unit>::iterator checkIt = forkCheck.begin(); checkIt != forkCheck.end(); ++checkIt)
						{
							if (checkIt->x == curImg->tracedNeuron.seg[*it].row.begin()->x && checkIt->y == curImg->tracedNeuron.seg[*it].row.begin()->y && checkIt->z == curImg->tracedNeuron.seg[*it].row.begin()->z)
								++tailConnectedCount;
						}

						if (!(headConnectedCount == 1 && tailConnectedCount == 1)) // check if it's a 3 segment loop 
						{
							cout << "  -> 3 seg intersection detected, exluded from loop candidates. (" << *it << ") ";
							for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
								cout << *thisLoopIt << " ";
							cout << endl;
							continue;
						}
						else
						{
							this->finalizedLoopsSet.insert(detectedLoopPathSet);
							cout << "  Loop from 3 way detected ----> (" << *it << ") ";
							for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
								cout << *thisLoopIt << " ";
							cout << endl << endl;
							return;
						}
					}
				}
				else if (curPathWalk.size() == 4) // exclude 4-way intersection  
				{
					if ((*curPathWalk.end() - 4) == *it)
					{
						if (this->seg2SegsMap[*(curPathWalk.end() - 2)].find(*it) != this->seg2SegsMap[*(curPathWalk.end() - 2)].end() &&
							this->seg2SegsMap[*(curPathWalk.end() - 3)].find(*it) != this->seg2SegsMap[*(curPathWalk.end() - 3)].end())
						{
							if (this->seg2SegsMap[*(curPathWalk.end() - 2)].find(*(curPathWalk.end() - 1)) != this->seg2SegsMap[*(curPathWalk.end() - 2)].end() &&
								this->seg2SegsMap[*(curPathWalk.end() - 3)].find(*(curPathWalk.end() - 2)) != this->seg2SegsMap[*(curPathWalk.end() - 3)].end() &&
								this->seg2SegsMap[*(curPathWalk.end() - 4)].find(*(curPathWalk.end() - 3)) != this->seg2SegsMap[*(curPathWalk.end() - 4)].end())
							{
								this->nonLoopErrors.insert(detectedLoopPathSet);
								cout << "  -> 4 way intersection detected ----> (" << *it << ") ";
								for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
									cout << *thisLoopIt << " ";
								cout << endl << endl;
								//cout << "  -> 4 seg intersection detected, exluded from loop candidates. (" << *it << ")" << endl;
								//continue;
							}
						}
					}
				}
				else
				{
					this->finalizedLoopsSet.insert(detectedLoopPathSet);
					cout << "  Topological loop identified ----> (" << *it << ") ";
					for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
						cout << *thisLoopIt << " ";
					cout << endl << endl;

					while (1)
					{
						for (set<set<size_t> >::iterator setCheckIt1 = this->finalizedLoopsSet.begin(); setCheckIt1 != this->finalizedLoopsSet.end(); ++setCheckIt1)
						{
							for (set<set<size_t> >::iterator setCheckIt2 = this->finalizedLoopsSet.begin(); setCheckIt2 != this->finalizedLoopsSet.end(); ++setCheckIt2)
							{
								if (setCheckIt1 == setCheckIt2) continue;
								else
								{
									int segNum = 0;
									for (set<size_t>::iterator segCheck1 = setCheckIt1->begin(); segCheck1 != setCheckIt1->end(); ++segCheck1)
										if (setCheckIt2->find(*segCheck1) != setCheckIt2->end()) ++segNum;

									if (segNum == setCheckIt1->size())
									{
										this->finalizedLoopsSet.erase(setCheckIt1);
										goto SET_ERASED;
									}
								}
							}
						}
						break;

					SET_ERASED:
						continue;
					}
				}
			}
			else return;
		}
	}

	curPathWalk.pop_back();
	//cout << endl;
}

void Renderer_gl1::escPressed_subtree()
{
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

	if (this->pressedShowSubTree == true)
	{
		My4DImage* curImg = 0;
		if (w) curImg = v3dr_getImage4d(_idep);
		//cout << "restoring" << endl;

		if (this->originalSegMap.empty()) return;

		for (map<size_t, vector<V_NeuronSWC_unit> >::iterator it = this->originalSegMap.begin(); it != this->originalSegMap.end(); ++it)
			curImg->tracedNeuron.seg[it->first].row = it->second;

		curImg->update_3drenderer_neuron_view(w, this);

		this->pressedShowSubTree = false;
		this->connectEdit = connectEdit_none;

		this->originalSegMap.clear();
		this->highlightedSegMap.clear();
	}
}
// ----------------- END of [Highlight the selected segment with its downstream subtree, MK, June, 2018] -----------------

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ subtree highlighting functions [DEPRICATED] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
void Renderer_gl1::showSubtree()
{
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
	XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);
	w->subtreeHighlightModeMonitor();

	float tolerance = 20; // tolerance distance from the backprojected neuron to the curve point

	for (V3DLONG j = 0; j < listNeuronTree.size(); j++)
	{
		NeuronTree* p_tree = (NeuronTree*)(&(listNeuronTree.at(j))); //curEditingNeuron-1
		if (p_tree && p_tree->editable)    // @FIXED by Alessandro on 2015-05-23. Removing segments from non-editable neurons causes crash.
		{
			QList<NeuronSWC>* p_listneuron = &(p_tree->listNeuron);
			if (!p_listneuron) return;
			// for (int testi=0; testi<list_listCurvePos.at(0).size(); testi++) qDebug() << list_listCurvePos.at(0).at(testi).x << " " << list_listCurvePos.at(0).at(testi).y;

			/***************** Get segment information included in the movePen trajectory *****************/
			/* ------- Only take in the nodes within the rectangle that contains the stroke ------- */
			long minX = list_listCurvePos.at(0).at(0).x, maxX = list_listCurvePos.at(0).at(0).x;
			long minY = list_listCurvePos.at(0).at(0).y, maxY = list_listCurvePos.at(0).at(0).y;
			for (size_t i = 0; i < list_listCurvePos.at(0).size(); ++i)
			{
				if (list_listCurvePos.at(0).at(i).x <= minX) minX = list_listCurvePos.at(0).at(i).x;
				if (list_listCurvePos.at(0).at(i).x >= maxX) maxX = list_listCurvePos.at(0).at(i).x;
				if (list_listCurvePos.at(0).at(i).y <= minY) minY = list_listCurvePos.at(0).at(i).y;
				if (list_listCurvePos.at(0).at(i).y >= maxY) maxY = list_listCurvePos.at(0).at(i).y;
			}
			minX = minX - 5; maxX = maxX + 5;
			minY = minY - 5; maxY = maxY + 5;
			//cout << minX << " " << maxX << " " << minY << " " << maxY << endl;
			QList<NeuronSWC> nodeOnStroke;
			for (size_t i = 0; i < p_listneuron->size(); ++i)
			{
				GLdouble px, py, pz, ix, iy, iz;
				ix = p_listneuron->at(i).x;
				iy = p_listneuron->at(i).y;
				iz = p_listneuron->at(i).z;
				if (gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
				{
					py = viewport[3] - py; //the Y axis is reversed
					QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));
					if ((p.x() >= minX && p.x() <= maxX) && (p.y() >= minY && p.y() <= maxY))
					{
						nodeOnStroke.push_back(p_listneuron->at(i));
						//cout << p.x() << " " << p.y() << endl;
					}
				}
			}
			if (nodeOnStroke.size() == 0) return;
			/* ------- END of [Only take in the nodes within the rectangle that contains the stroke] ------- */

			/* ------- Acquire the starting segment ------- */
			vector<size_t> involvedSegs;
			float nearestDist = 1000;
			NeuronSWC nearestNode;
			for (V3DLONG i = 0; i < list_listCurvePos.at(0).size(); i++)
			{
				for (V3DLONG j = 0; j < nodeOnStroke.size(); j++)
				{
					GLdouble px, py, pz, ix, iy, iz;
					ix = nodeOnStroke.at(j).x;
					iy = nodeOnStroke.at(j).y;
					iz = nodeOnStroke.at(j).z;
					if (gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
					{
						py = viewport[3] - py; //the Y axis is reversed
						QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));

						QPointF p2(list_listCurvePos.at(0).at(i).x, list_listCurvePos.at(0).at(i).y);
						float dist = std::sqrt((p.x() - p2.x())*(p.x() - p2.x()) + (p.y() - p2.y())*(p.y() - p2.y()));
						if (std::sqrt((p.x() - p2.x())*(p.x() - p2.x()) + (p.y() - p2.y())*(p.y() - p2.y())) <= tolerance) // p: global coordinates; p2: view port coordinates
						{
							if (dist < nearestDist)
							{
								nearestDist = dist;
								nearestNode = nodeOnStroke.at(j);
							}
							involvedSegs.push_back(nodeOnStroke.at(j).seg_id);
						}
					}
				}
			}
			map<size_t, int> segIDCountMap;
			for (vector<size_t>::iterator segIt = involvedSegs.begin(); segIt != involvedSegs.end(); ++segIt)
				++segIDCountMap.insert(pair<size_t, int>(*segIt, 0)).first->second;
			int segCount = segIDCountMap.begin()->second;
			size_t startingSegID = segIDCountMap.begin()->first;
			for (map<size_t, int>::iterator mapIt = segIDCountMap.begin(); mapIt != segIDCountMap.end(); ++mapIt)
			{
				if (mapIt->second > segCount)
				{
					segCount = mapIt->second;
					startingSegID = mapIt->first; // <-- This is the starting point of the subtree to be highlighted.
				}
			}
			/* ------- END of [Acquire the starting segment] ------- */
			/***************** END of [Get segment information included in the movePen trajectory] *****************/

			/******************** Produce gridID - segID map ********************/
			this->subtreeSegs.clear();
			double rangeLength = 1;
			double sqrdRange = rangeLength * rangeLength * 3;
			int gridLength = 1;
			this->grid2segIDmap.clear();
			for (vector<V_NeuronSWC>::iterator it = curImg->tracedNeuron.seg.begin(); it != curImg->tracedNeuron.seg.end(); ++it)
			{
				int xLabelTail = it->row.begin()->x / gridLength;
				int yLabelTail = it->row.begin()->y / gridLength;
				int zLabelTail = it->row.begin()->z / gridLength;
				int xLabelHead = (it->row.end() - 1)->x / gridLength;
				int yLabelHead = (it->row.end() - 1)->y / gridLength;
				int zLabelHead = (it->row.end() - 1)->z / gridLength;
				QString key1Q = QString::number(xLabelTail) + "_" + QString::number(yLabelTail) + "_" + QString::number(zLabelTail);
				string key1 = key1Q.toStdString();
				QString key2Q = QString::number(xLabelHead) + "_" + QString::number(yLabelHead) + "_" + QString::number(zLabelHead);
				string key2 = key2Q.toStdString();

				this->grid2segIDmap.insert(pair<string, size_t>(key1, size_t(it - curImg->tracedNeuron.seg.begin())));
				this->grid2segIDmap.insert(pair<string, size_t>(key2, size_t(it - curImg->tracedNeuron.seg.begin())));
			}
			/***************** END of [Produce gridID - segID map] *****************/

			V_NeuronSWC originalStartingSeg = curImg->tracedNeuron.seg[startingSegID];
			vector<V_NeuronSWC_unit> cutStartingSeg;
			cutStartingSeg.clear();
			int xStartTail = curImg->tracedNeuron.seg[startingSegID].row.begin()->x / gridLength;
			int yStartTail = curImg->tracedNeuron.seg[startingSegID].row.begin()->y / gridLength;
			int zStartTail = curImg->tracedNeuron.seg[startingSegID].row.begin()->z / gridLength;
			QString startKeyTailQ = QString::number(xStartTail) + "_" + QString::number(yStartTail) + "_" + QString::number(zStartTail);
			string startKeyTail = startKeyTailQ.toStdString();
			for (vector<V_NeuronSWC_unit>::iterator cutIt = curImg->tracedNeuron.seg[startingSegID].row.begin(); cutIt != curImg->tracedNeuron.seg[startingSegID].row.end(); ++cutIt)
			{
				cutStartingSeg.push_back(*cutIt);
				if (cutIt->x == nearestNode.x && cutIt->y == nearestNode.y && cutIt->z == nearestNode.z)
				{
					(cutStartingSeg.end() - 1)->parent = -1;
					break;
				}
			}
			curImg->tracedNeuron.seg[startingSegID].row = cutStartingSeg;
			this->rc_findDownstreamSegs(curImg, startingSegID, startKeyTail, gridLength);
			//this->rc_downstreamSeg(curImg, startingSegID);
			//this->rc_downstream_segID(curImg, startingSegID);

			curImg->tracedNeuron.seg[startingSegID] = originalStartingSeg;
			this->originalSegMap.insert(pair<size_t, vector<V_NeuronSWC_unit> >(startingSegID, curImg->tracedNeuron.seg[startingSegID].row));
			for (vector<V_NeuronSWC_unit>::iterator firstSegIt = curImg->tracedNeuron.seg[startingSegID].row.begin(); firstSegIt != curImg->tracedNeuron.seg[startingSegID].row.end(); ++firstSegIt)
			{
				if (firstSegIt->x == nearestNode.x && firstSegIt->y == nearestNode.y && firstSegIt->z == nearestNode.z)
				{
					firstSegIt->type = 0;
					break;
				}
				firstSegIt->type = 0;
			}
			this->highlightedSegMap.insert(pair<size_t, vector<V_NeuronSWC_unit> >(startingSegID, curImg->tracedNeuron.seg[startingSegID].row));

			for (set<size_t>::iterator segIt = this->subtreeSegs.begin(); segIt != this->subtreeSegs.end(); ++segIt)
			{
				if (*segIt == startingSegID) continue;

				this->originalSegMap.insert(pair<size_t, vector<V_NeuronSWC_unit> >(*segIt, curImg->tracedNeuron.seg[*segIt].row));
				//cout << *segIt << " ";
				for (vector<V_NeuronSWC_unit>::iterator unitIt = curImg->tracedNeuron.seg[*segIt].row.begin(); unitIt != curImg->tracedNeuron.seg[*segIt].row.end(); ++unitIt)
					unitIt->type = 0;
				this->highlightedSegMap.insert(pair<size_t, vector<V_NeuronSWC_unit> >(*segIt, curImg->tracedNeuron.seg[*segIt].row));
			}
			//cout << endl;

			curImg->update_3drenderer_neuron_view(w, this);
			//curImg->proj_trace_history_append(); // -> Highlighting is for temporary checking purpose, should not be appended to the history.
		}
	}

	this->pressedShowSubTree = true;
}

void Renderer_gl1::hierarchyReprofile(My4DImage* curImg, long mainSegID, long branchSegID)
{
	cout << " ---> primary seg hierarchy: " << curImg->tracedNeuron.seg[mainSegID].branchingProfile.hierarchy << endl;

	vector<V_NeuronSWC> connectedSegDecomposed = decompose_V_NeuronSWC(curImg->tracedNeuron.seg[mainSegID]);

	if (connectedSegDecomposed.size() > 1)
	{
		cout << "new segment breaks into " << connectedSegDecomposed.size() << " subsegments." << endl << endl;

		size_t primaryPaSegID = this->branchSegIDmap[curImg->tracedNeuron.seg[mainSegID].branchingProfile.paID];
		V_NeuronSWC_unit oldPrimaryHead = *(curImg->tracedNeuron.seg[mainSegID].row.end() - 1);
		V_NeuronSWC_unit oldPrimaryTail = *(curImg->tracedNeuron.seg[mainSegID].row.begin());
		//cout << "Old primary tail coords:" << oldPrimaryTail.x << " " << oldPrimaryTail.y << " " << oldPrimaryTail.z << endl;
		//cout << "Old primary head coords:" << oldPrimaryHead.x << " " << oldPrimaryHead.y << " " << oldPrimaryHead.z << endl << endl;

		bool allAdded = false;
		if (connectedSegDecomposed.at(0).row.begin()->x == (connectedSegDecomposed.at(1).row.end() - 1)->x && connectedSegDecomposed.at(0).row.begin()->x == (connectedSegDecomposed.at(2).row.end() - 1)->x)
		{
			curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(0));
			for (vector<V_NeuronSWC_unit>::iterator it = curImg->tracedNeuron.seg[mainSegID].row.begin(); it != curImg->tracedNeuron.seg[mainSegID].row.end(); ++it)
			{
				if (it->x == (connectedSegDecomposed.at(1).row.begin() + 1)->x)
				{
					curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(1));
					curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(2));
					allAdded = true;
					break;
				}
			}
			if (!allAdded)
			{
				curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(2));
				curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(1));
			}
		}
		else if (connectedSegDecomposed.at(1).row.begin()->x == (connectedSegDecomposed.at(0).row.end() - 1)->x && connectedSegDecomposed.at(1).row.begin()->x == (connectedSegDecomposed.at(2).row.end() - 1)->x)
		{
			curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(1));
			for (vector<V_NeuronSWC_unit>::iterator it = curImg->tracedNeuron.seg[mainSegID].row.begin(); it != curImg->tracedNeuron.seg[mainSegID].row.end(); ++it)
			{
				if (it->x == (connectedSegDecomposed.at(0).row.begin() + 1)->x)
				{
					curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(0));
					curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(2));
					allAdded = true;
					break;
				}
			}
			if (!allAdded)
			{
				curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(2));
				curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(0));
			}
		}
		else if (connectedSegDecomposed.at(2).row.begin()->x == (connectedSegDecomposed.at(0).row.end() - 1)->x && connectedSegDecomposed.at(2).row.begin()->x == (connectedSegDecomposed.at(1).row.end() - 1)->x)
		{
			curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(2));
			for (vector<V_NeuronSWC_unit>::iterator it = curImg->tracedNeuron.seg[mainSegID].row.begin(); it != curImg->tracedNeuron.seg[mainSegID].row.end(); ++it)
			{
				if (it->x == (connectedSegDecomposed.at(1).row.begin() + 1)->x)
				{
					curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(1));
					curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(0));
					allAdded = true;
					break;
				}
			}
			if (!allAdded)
			{
				curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(0));
				curImg->tracedNeuron.seg.push_back(connectedSegDecomposed.at(1));
			}
		}

		vector<V_NeuronSWC>::iterator newSegIt = curImg->tracedNeuron.seg.end() - 3;
		for (ptrdiff_t i = 0; i < 3; ++i)
		{
			++newSegIt;
			newSegIt->branchingProfile.ID = curImg->tracedNeuron.seg.size();
			int segNums = curImg->tracedNeuron.seg.size() - 1;

			if (i == 0)
			{
				newSegIt->branchingProfile.paID = curImg->tracedNeuron.seg[primaryPaSegID].branchingProfile.ID;
				newSegIt->branchingProfile.hierarchy = curImg->tracedNeuron.seg[primaryPaSegID].branchingProfile.hierarchy + 1;
				newSegIt->branchingProfile.childIDs.clear();
				this->branchSegIDmap[newSegIt->branchingProfile.ID] = segNums;
				this->branchSegIDmap.erase(curImg->tracedNeuron.seg[mainSegID].branchingProfile.ID);
				for (vector<int>::iterator childIt = curImg->tracedNeuron.seg[primaryPaSegID].branchingProfile.childIDs.begin(); childIt != curImg->tracedNeuron.seg[primaryPaSegID].branchingProfile.childIDs.end(); ++childIt)
					if (*childIt == curImg->tracedNeuron.seg[mainSegID].branchingProfile.ID) *childIt = newSegIt->branchingProfile.ID;
			}
			else if (i == 1)
			{
				newSegIt->branchingProfile.paID = (newSegIt - 1)->branchingProfile.ID;
				newSegIt->branchingProfile.hierarchy = (newSegIt - 1)->branchingProfile.hierarchy + 1;
				this->branchSegIDmap[newSegIt->branchingProfile.ID] = segNums;
				//cout << "segID:" << segNums << " branchID:" << newSegIt->branchingProfile.ID << endl;
				newSegIt->branchingProfile.childIDs.clear();
				for (vector<int>::iterator childIt = curImg->tracedNeuron.seg[mainSegID].branchingProfile.childIDs.begin(); childIt != curImg->tracedNeuron.seg[mainSegID].branchingProfile.childIDs.end(); ++childIt)
				{
					//cout << "child branchID: " << *childIt << endl;
					int childBranchID = *childIt;
					newSegIt->branchingProfile.childIDs.push_back(childBranchID);
					curImg->tracedNeuron.seg[this->branchSegIDmap[*childIt]].branchingProfile.paID = newSegIt->branchingProfile.ID;
				}
				(newSegIt - 1)->branchingProfile.childIDs.push_back(newSegIt->branchingProfile.ID);
				cout << "=======> recursive downstream relabeling" << endl;
				this->rc_downstreamRelabel(curImg, segNums);
			}
			else if (i == 2)
			{
				newSegIt->branchingProfile.paID = (newSegIt - 2)->branchingProfile.ID;
				newSegIt->branchingProfile.hierarchy = (newSegIt - 2)->branchingProfile.hierarchy + 1;
				(newSegIt - 2)->branchingProfile.childIDs.push_back(newSegIt->branchingProfile.ID);
				this->branchSegIDmap[newSegIt->branchingProfile.ID] = segNums;

				int oldPaSegID = this->branchSegIDmap[curImg->tracedNeuron.seg[branchSegID].branchingProfile.paID];
				for (vector<int>::iterator oldPaChildIt = curImg->tracedNeuron.seg[oldPaSegID].branchingProfile.childIDs.begin(); oldPaChildIt != curImg->tracedNeuron.seg[oldPaSegID].branchingProfile.childIDs.end(); ++oldPaChildIt)
				{
					if (*oldPaChildIt == curImg->tracedNeuron.seg[branchSegID].branchingProfile.ID)
					{
						cout << this->branchSegIDmap[*oldPaChildIt] << " " << this->branchSegIDmap[newSegIt->branchingProfile.ID] << endl;
						*oldPaChildIt = newSegIt->branchingProfile.ID;
					}
				}

				V_NeuronSWC* newPaSegPtr = &(curImg->tracedNeuron.seg[segNums]);
				int curSegID = this->branchSegIDmap[curImg->tracedNeuron.seg[branchSegID].branchingProfile.paID];
				V_NeuronSWC* curSegPtr = &(curImg->tracedNeuron.seg[curSegID]);
				cout << "=======> upstream relabeling" << endl;
				upstreamRelabel(curImg, curSegPtr, newPaSegPtr);
				this->branchSegIDmap.erase(curImg->tracedNeuron.seg[branchSegID].branchingProfile.ID);
			}
		}

		curImg->tracedNeuron.seg[mainSegID].to_be_deleted = true;
	}
	else if (connectedSegDecomposed.size() <= 1)
	{
		size_t paSegID = this->branchSegIDmap[curImg->tracedNeuron.seg[branchSegID].branchingProfile.paID];
		cout << endl << "-- parent segment ID of the secondary segment: " << paSegID;
		cout << "    parent branch ID: " << curImg->tracedNeuron.seg[branchSegID].branchingProfile.paID << endl;
		curImg->tracedNeuron.seg[paSegID].branchingProfile.hierarchy = curImg->tracedNeuron.seg[mainSegID].branchingProfile.hierarchy + 1;

		vector<int> curStemChildBranchIDs = curImg->tracedNeuron.seg[paSegID].branchingProfile.childIDs;

		cout << "old immediate children segment ID-branchID-hierarchy: ";
		for (vector<int>::iterator chkIt = curStemChildBranchIDs.begin(); chkIt != curStemChildBranchIDs.end(); ++chkIt)
			cout << this->branchSegIDmap[*chkIt] << "-" << *chkIt << "-" << curImg->tracedNeuron.seg[this->branchSegIDmap[*chkIt]].branchingProfile.hierarchy << ", ";

		curImg->tracedNeuron.seg[mainSegID].branchingProfile.childIDs.clear();
		for (vector<int>::iterator it = curStemChildBranchIDs.begin(); it != curStemChildBranchIDs.end(); ++it)
		{
			if (this->branchSegIDmap[*it] == branchSegID) this->branchSegIDmap.erase(*it);
			else
			{
				curImg->tracedNeuron.seg[this->branchSegIDmap[*it]].branchingProfile.hierarchy = curImg->tracedNeuron.seg[paSegID].branchingProfile.hierarchy;
				this->rc_downstreamRelabel(curImg, this->branchSegIDmap[*it]);
				curImg->tracedNeuron.seg[mainSegID].branchingProfile.childIDs.push_back(*it);
				curImg->tracedNeuron.seg[this->branchSegIDmap[*it]].branchingProfile.paID = curImg->tracedNeuron.seg[mainSegID].branchingProfile.ID;
			}
		}

		cout << endl << "new immediate children segment ID-branchID-hierarchy: ";
		for (vector<int>::iterator chkIt = curStemChildBranchIDs.begin(); chkIt != curStemChildBranchIDs.end(); ++chkIt)
			cout << this->branchSegIDmap[*chkIt] << "-" << *chkIt << "-" << curImg->tracedNeuron.seg[this->branchSegIDmap[*chkIt]].branchingProfile.hierarchy << ", ";
		cout << endl;

		V_NeuronSWC* upstreamStartSegPtr = &(curImg->tracedNeuron.seg[paSegID]);
		V_NeuronSWC* newPaSegPtr = &(curImg->tracedNeuron.seg[mainSegID]);
		this->upstreamRelabel(curImg, upstreamStartSegPtr, newPaSegPtr);
	}
}

void Renderer_gl1::upstreamRelabel(My4DImage* curImg, V_NeuronSWC* curSegPtr, V_NeuronSWC* newPaSegPtr)
{
	cout << endl << "curent segID: " << this->branchSegIDmap[curSegPtr->branchingProfile.ID] << ", new parent segID: " << this->branchSegIDmap[newPaSegPtr->branchingProfile.ID] << endl;
	cout << "start upstream relabeling..." << endl;
	while (1)
	{
		int curBranchID = curSegPtr->branchingProfile.ID;
		int curPaBranchID = curSegPtr->branchingProfile.paID;
		size_t curSegID = this->branchSegIDmap[curBranchID];
		size_t curPaSegID = this->branchSegIDmap[curPaBranchID];
		curSegPtr->branchingProfile.hierarchy = newPaSegPtr->branchingProfile.hierarchy + 1;

		vector<int> curStemChildBranchIDs = curImg->tracedNeuron.seg[curPaSegID].branchingProfile.childIDs;
		curSegPtr->branchingProfile.childIDs.clear();
		for (vector<int>::iterator it = curStemChildBranchIDs.begin(); it != curStemChildBranchIDs.end(); ++it)
		{
			if (this->branchSegIDmap[*it] == curSegID)
			{
				curSegPtr->branchingProfile.paID = newPaSegPtr->branchingProfile.ID;
				newPaSegPtr->branchingProfile.childIDs.push_back(*it);

				cout << "  stem segID:" << this->branchSegIDmap[newPaSegPtr->branchingProfile.ID] << endl;
				for (vector<int>::iterator chkIt = newPaSegPtr->branchingProfile.childIDs.begin(); chkIt != newPaSegPtr->branchingProfile.childIDs.end(); ++chkIt)
					cout << "    child segID:" << this->branchSegIDmap[*chkIt] << " adjusted hierarchy:" << curImg->tracedNeuron.seg[this->branchSegIDmap[*chkIt]].branchingProfile.hierarchy << endl;
				cout << endl;
			}
			else
			{
				curImg->tracedNeuron.seg[this->branchSegIDmap[*it]].branchingProfile.hierarchy = curSegPtr->branchingProfile.hierarchy + 1;
				this->rc_downstreamRelabel(curImg, this->branchSegIDmap[*it]);
				curSegPtr->branchingProfile.childIDs.push_back(*it);
				curImg->tracedNeuron.seg[this->branchSegIDmap[*it]].branchingProfile.paID = curBranchID;
			}
		}

		newPaSegPtr = curSegPtr;
		curSegPtr = &(curImg->tracedNeuron.seg[curPaSegID]);
		if (curSegPtr->branchingProfile.paID == -1)
		{
			curSegPtr->branchingProfile.paID = newPaSegPtr->branchingProfile.ID;
			newPaSegPtr->branchingProfile.childIDs.push_back(curSegPtr->branchingProfile.ID);
			curSegPtr->branchingProfile.hierarchy = newPaSegPtr->branchingProfile.hierarchy + 1;
			break;
		}
	}
}

void Renderer_gl1::rc_downstreamRelabel(My4DImage* curImg, size_t curPaSegID)
{
	int childSegCount;

	vector<int> nextLevelBranchIDs = curImg->tracedNeuron.seg[curPaSegID].branchingProfile.childIDs;
	childSegCount = nextLevelBranchIDs.size();
	cout << endl << "current seg number: " << curPaSegID << "  child seg number: " << childSegCount << " ";
	if (childSegCount == 0)
	{
		cout << " ---> terminal leaf reached, return and move to the next sibling branch." << endl;
		return;
	}
	for (vector<int>::iterator childIt = nextLevelBranchIDs.begin(); childIt != nextLevelBranchIDs.end(); ++childIt)
		cout << this->branchSegIDmap[*childIt] << "-" << *childIt << "-" << curImg->tracedNeuron.seg[this->branchSegIDmap[*childIt]].branchingProfile.hierarchy << ", ";
	cout << endl;

	for (vector<int>::iterator it = nextLevelBranchIDs.begin(); it != nextLevelBranchIDs.end(); ++it)
	{
		cout << "  current child segID: " << this->branchSegIDmap[*it] << ", branchID: " << *it << ", original hierarchy: " << curImg->tracedNeuron.seg[this->branchSegIDmap[*it]].branchingProfile.hierarchy << endl;
		curImg->tracedNeuron.seg[this->branchSegIDmap[*it]].branchingProfile.hierarchy = curImg->tracedNeuron.seg[curPaSegID].branchingProfile.hierarchy + 1;
		cout << "  current child segID: " << this->branchSegIDmap[*it] << ", branchID: " << *it << ", adjusted hierarchy: " << curImg->tracedNeuron.seg[this->branchSegIDmap[*it]].branchingProfile.hierarchy << endl;
		this->rc_downstreamRelabel(curImg, this->branchSegIDmap[*it]);
	}

	return;
}

void Renderer_gl1::rc_findDownstreamSegs(My4DImage* curImg, size_t inputSegID, string gridKey, int gridLength)
{
	// This method finds all sebsequent segments following a given starting segment. 
	// The approach is geometrical by employing an endpoint-grid map.

	//cout << endl << "Current gridKey:" << gridKey << " Input segID:" << inputSegID;

	if (curImg->tracedNeuron.seg[inputSegID].to_be_deleted) return;

	size_t curSegNum = this->subtreeSegs.size();

	pair<multimap<string, size_t>::iterator, multimap<string, size_t>::iterator> range = grid2segIDmap.equal_range(gridKey);
	if (range.first == range.second) return;

	for (multimap<string, size_t>::iterator rangeIt = range.first; rangeIt != range.second; ++rangeIt)
	{
		size_t curSegID = rangeIt->second;
		//cout << "  Finding subtree --> current segID:" << curSegID << endl;
		this->subtreeSegs.insert(curSegID);

		if (this->subtreeSegs.size() == curSegNum)
		{
			//cout << "    Seg already picked up, move to the next." << endl;
			continue;
		}
		else
		{
			curSegNum = this->subtreeSegs.size();

			for (vector<V_NeuronSWC_unit>::iterator unitIt = curImg->tracedNeuron.seg[curSegID].row.begin() + 1; unitIt != curImg->tracedNeuron.seg[curSegID].row.end() - 1; ++unitIt)
			{
				int middleX = unitIt->x / gridLength;
				int middleY = unitIt->y / gridLength;
				int middleZ = unitIt->z / gridLength;
				QString middleGridKeyQ = QString::number(middleX) + "_" + QString::number(middleY) + "_" + QString::number(middleZ);
				string middleGridKey = middleGridKeyQ.toStdString();

				pair<multimap<string, size_t>::iterator, multimap<string, size_t>::iterator> middleRange = grid2segIDmap.equal_range(middleGridKey);
				for (multimap<string, size_t>::iterator middleIt = middleRange.first; middleIt != middleRange.second; ++middleIt)
				{
					if (middleIt->second == curSegID) continue;

					if (middleIt->first == middleGridKey)
					{
						//cout << "  Found a segment in the middle of the route, adding it to the recursive searching process:" << middleGridKey << " " << middleIt->second << endl;
						curSegNum = this->subtreeSegs.size();
						this->rc_findDownstreamSegs(curImg, middleIt->second, middleGridKey, gridLength);
					}
				}
			}

			int xCurHead = (curImg->tracedNeuron.seg[rangeIt->second].row.end() - 1)->x / gridLength;
			int yCurHead = (curImg->tracedNeuron.seg[rangeIt->second].row.end() - 1)->y / gridLength;
			int zCurHead = (curImg->tracedNeuron.seg[rangeIt->second].row.end() - 1)->z / gridLength;
			QString curKeyHeadQ = QString::number(xCurHead) + "_" + QString::number(yCurHead) + "_" + QString::number(zCurHead);
			string curKeyHead = curKeyHeadQ.toStdString();
			//cout << curKeyHead << " " << curSegID << endl;

			if (curKeyHead == gridKey)
			{
				//cout << "tail to next head" << endl;
				int xCurTail = curImg->tracedNeuron.seg[rangeIt->second].row.begin()->x / gridLength;
				int yCurTail = curImg->tracedNeuron.seg[rangeIt->second].row.begin()->y / gridLength;
				int zCurTail = curImg->tracedNeuron.seg[rangeIt->second].row.begin()->z / gridLength;
				QString curKeyTailQ = QString::number(xCurTail) + "_" + QString::number(yCurTail) + "_" + QString::number(zCurTail);
				string curKeyTail = curKeyTailQ.toStdString();
				this->rc_findDownstreamSegs(curImg, curSegID, curKeyTail, gridLength);
			}
			//else this->rc_findDownstreamSegs(curImg, curKeyHead, gridLength);
		}
	}

	if (this->subtreeSegs.size() == curSegNum) return;
}

void Renderer_gl1::segTreeFastReprofile(My4DImage* curImg)
{
	// This method profiles the topology with endpoint-segID map. Currently prone to be buggy, not in use.

	this->tail2segIDmap.clear();
	this->head2segIDmap.clear();

	for (vector<V_NeuronSWC>::iterator it = curImg->tracedNeuron.seg.begin(); it != curImg->tracedNeuron.seg.end(); ++it)
	{
		QString coordsKeyTailQ = QString::number(it->row.begin()->x);
		coordsKeyTailQ = coordsKeyTailQ + QString::number(it->row.begin()->y) + QString::number(it->row.begin()->z);
		string coordsKeyTail = coordsKeyTailQ.toStdString();
		QString coordsKeyHeadQ = QString::number((it->row.end() - 1)->x);
		coordsKeyHeadQ = coordsKeyHeadQ + QString::number((it->row.end() - 1)->y) + QString::number((it->row.end() - 1)->z);
		string coordsKeyHead = coordsKeyTailQ.toStdString();

		this->tail2segIDmap.insert(pair<string, size_t>(coordsKeyTail, size_t(it - curImg->tracedNeuron.seg.begin())));
		this->head2segIDmap.insert(pair<string, size_t>(coordsKeyHead, size_t(it - curImg->tracedNeuron.seg.begin())));
	}
	//cout << endl << tail2segIDmap.size() << " " << head2segIDmap.size() << endl;

	int connectedCount = 0;
	for (map<string, size_t>::iterator tailIt = this->tail2segIDmap.begin(); tailIt != tail2segIDmap.end(); ++tailIt)
	{
		pair<multimap<string, size_t>::iterator, multimap<string, size_t>::iterator> range = this->head2segIDmap.equal_range(tailIt->first);
		//cout << tailIt->first << "_" << tailIt->second << endl;
		for (multimap<string, size_t>::iterator headRangeIt = range.first; headRangeIt != range.second; ++headRangeIt)
		{
			//cout << headRangeIt->first << "_" << headRangeIt->second << " ";
			if (!headRangeIt->first.compare(tailIt->first))
			{
				++connectedCount;
				curImg->tracedNeuron.seg[headRangeIt->second].branchingProfile.segLoc = headRangeIt->second;
				curImg->tracedNeuron.seg[headRangeIt->second].branchingProfile.segPaLoc = tailIt->second;
				curImg->tracedNeuron.seg[tailIt->second].branchingProfile.segLoc = tailIt->second;
				curImg->tracedNeuron.seg[tailIt->second].branchingProfile.childSegLocs.push_back(headRangeIt->second);
				//cout << tailIt->second << " " << headRangeIt->second << endl;
				//cout << tailIt->first << " " << headRangeIt->first << endl;
			}
		}
		//cout << endl << endl;
	}
	//cout << connectedCount << endl;
}

void Renderer_gl1::rc_downstreamSeg(My4DImage* curImg, size_t segID)
{
	// This method finds out the subsequent segments from a given segment [branch ID] recursively.

	int childSegCount;

	vector<int> nextLevelBranchIDs = curImg->tracedNeuron.seg[segID].branchingProfile.childIDs;
	childSegCount = nextLevelBranchIDs.size();
	//cout << endl << "starting seg number: " << segID << "  child seg number: " << childSegCount << " ";
	if (childSegCount == 0)
	{
		//cout << " ---> terminal leaf reached, return and move to the next sibling branch." << endl;
		return;
	}
	/*for (vector<int>::iterator childIt = nextLevelBranchIDs.begin(); childIt != nextLevelBranchIDs.end(); ++childIt)
	cout << this->branchSegIDmap[*childIt] << "-" << *childIt << ", ";
	cout << endl;*/

	for (vector<int>::iterator it = nextLevelBranchIDs.begin(); it != nextLevelBranchIDs.end(); ++it)
	{
		//cout << "  current child segID: " << this->branchSegIDmap[*it] << ", branchID: " << *it << endl;
		this->subtreeSegs.insert(this->branchSegIDmap[*it]);
		this->rc_downstreamSeg(curImg, this->branchSegIDmap[*it]);
	}

	return;
}

void Renderer_gl1::rc_downstream_segID(My4DImage* curImg, size_t segID)
{
	// This method finds out the subsequent segments from a given [segment ID] recursively.

	int childSegCount;

	vector<int> nextLevelSegLocs = curImg->tracedNeuron.seg[segID].branchingProfile.childSegLocs;
	childSegCount = nextLevelSegLocs.size();
	//cout << endl << "starting seg number: " << segID << "  child seg number: " << childSegCount << " ";
	if (childSegCount == 0)
	{
		//cout << " ---> terminal leaf reached, return and move to the next sibling branch." << endl;
		return;
	}
	/*for (vector<int>::iterator childIt = nextLevelBranchIDs.begin(); childIt != nextLevelBranchIDs.end(); ++childIt)
	cout << this->branchSegIDmap[*childIt] << "-" << *childIt << ", ";
	cout << endl;*/

	for (vector<int>::iterator it = nextLevelSegLocs.begin(); it != nextLevelSegLocs.end(); ++it)
	{
		//cout << "  current child segID: " << *it << endl;
		this->subtreeSegs.insert(size_t(*it));
		this->rc_downstream_segID(curImg, size_t(*it));
	}

	return;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ subtree highlighting functions [DEPRICATED] ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

// --------------------- segment/points could/marker connecting tool, by MK 2017 April --------------------------
void Renderer_gl1::connectNeuronsByStroke()
{
	connectEdit = segmentEdit;

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
	XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

    //v3d_msg(QString("getNumShiftHolding() = ") + QString(w->getNumShiftHolding() ? "YES" : "no"));
    float tolerance = 10; // tolerance distance from the backprojected neuron to the curve point

    // back-project the node curve points and mark segments to be deleted
    for(V3DLONG j=0; j<listNeuronTree.size(); j++)
    {
        NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(j))); //curEditingNeuron-1
        if (p_tree && p_tree->editable)    // @FIXED by Alessandro on 2015-05-23. Removing segments from non-editable neurons causes crash.
        {
            QList <NeuronSWC> *p_listneuron = &(p_tree->listNeuron);
            if (!p_listneuron) continue;
			//for (int testi=0; testi<list_listCurvePos.at(0).size(); testi++) qDebug() << list_listCurvePos.at(0).at(testi).x << " " << list_listCurvePos.at(0).at(testi).y;
			
			vector<segInfoUnit> segInfo;
			long segCheck = 0;
			long cummNodeNum = 0;
			
			/* ============== Get all segments information included in the movePen trajectory, merge, and refine. ============== */
			  /* ======== Only take in the nodes within the rectangle that contains the stroke ======== */
			long minX = list_listCurvePos.at(0).at(0).x, maxX = list_listCurvePos.at(0).at(0).x;
			long minY = list_listCurvePos.at(0).at(0).y, maxY = list_listCurvePos.at(0).at(0).y;
			for (size_t i=0; i<list_listCurvePos.at(0).size(); ++i)
			{
				if (list_listCurvePos.at(0).at(i).x <= minX) minX = list_listCurvePos.at(0).at(i).x;
				if (list_listCurvePos.at(0).at(i).x >= maxX) maxX = list_listCurvePos.at(0).at(i).x;
				if (list_listCurvePos.at(0).at(i).y <= minY) minY = list_listCurvePos.at(0).at(i).y;
				if (list_listCurvePos.at(0).at(i).y >= maxY) maxY = list_listCurvePos.at(0).at(i).y;
			}
			minX = minX - 5; maxX = maxX + 5;
			minY = minY - 5; maxY = maxY + 5;
			//cout << minX << " " << maxX << " " << minY << " " << maxY << endl;
			QList<NeuronSWC> nodeOnStroke;
			for (size_t i=0; i<p_listneuron->size(); ++i)
			{
				GLdouble px, py, pz, ix, iy, iz;
				ix = p_listneuron->at(i).x;
				iy = p_listneuron->at(i).y;
				iz = p_listneuron->at(i).z;
				if (gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
				{
					py = viewport[3]-py; //the Y axis is reversed
					QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));
					if ((p.x()>=minX && p.x()<=maxX) && (p.y()>=minY && p.y()<=maxY))
					{
						nodeOnStroke.push_back(p_listneuron->at(i));
						//cout << p.x() << " " << p.y() << endl;
					}
				}
			}
			  /* ==== END of [Only take in the nodes within the rectangle that contains the stroke] ==== */

			for (V3DLONG i=0; i<list_listCurvePos.at(0).size(); i++)
			{
				for (V3DLONG j=0; j<nodeOnStroke.size(); j++)
				{
					GLdouble px, py, pz, ix, iy, iz;
					ix = nodeOnStroke.at(j).x;
					iy = nodeOnStroke.at(j).y;
					iz = nodeOnStroke.at(j).z;
					if(gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
					{
						if (curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.begin()->data[6] != 2) // Sort the node numbers of involved segments
						{
							int nodeNo = 1;
							for (vector<V_NeuronSWC_unit>::iterator it = curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.begin();
								it != curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.end(); it++)
							{
								it->data[0] = nodeNo;
								it->data[6] = nodeNo + 1;
								++nodeNo;
							}
							(curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.end() - 1)->data[6] = -1;
						}

						py = viewport[3]-py; //the Y axis is reversed
						QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));

                        QPointF p2(list_listCurvePos.at(0).at(i).x, list_listCurvePos.at(0).at(i).y);
						if( std::sqrt((p.x()-p2.x())*(p.x()-p2.x()) + (p.y()-p2.y())*(p.y()-p2.y())) <= tolerance  )
                        {
							for (vector<V_NeuronSWC_unit>::iterator it=curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.begin();
								it!=curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.end(); it++)
							{
								if (nodeOnStroke.at(j).x==it->data[2] && nodeOnStroke.at(j).y==it->data[3] && nodeOnStroke.at(j).z==it->data[4]) 
								{
									if (it->data[6]==2 || it->data[6]==-1) // only allows heads or tails
									{
										//---------------------- Get seg IDs
										//qDebug() << nodeOnStroke->at(j).seg_id << " " << nodeOnStroke->at(j).parent << " " << p.x() << " " << p.y();
										segInfoUnit curSeg;
										curSeg.head_tail = it->data[6]; 
										curSeg.segID = nodeOnStroke.at(j).seg_id;
										curSeg.nodeCount = curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.size();
										curSeg.refine = false;
										vector<segInfoUnit>::iterator chkIt = segInfo.end();
										if (segInfo.begin() == segInfo.end())
										{
											segInfo.push_back(curSeg);
											segCheck = it->data[6];
										}
										else 
										{
											bool repeat = false;
											while (chkIt >= segInfo.begin())
											{
												if (chkIt->segID == curSeg.segID) 
												{
													repeat = true;
													break;
												}
												else --chkIt;
											}
											if (repeat == false)
											{
												segInfo.push_back(curSeg);
												segCheck = it->data[6];
											}
										}
									}
								}
							}
							break;
						}
					}
				}
			}
			cout << " ==== segments involved:" << endl;
			for (vector<segInfoUnit>::iterator test=segInfo.begin(); test!=segInfo.end(); ++test) cout << test->segID << ":" << test->nodeCount << " " << test->head_tail << ", ";
			cout << endl << endl;
			if (segInfo.size() <= 1) return;

			if (segInfo.size() == 2)
			{
				if (segInfo[0].nodeCount == 1 && segInfo[1].head_tail == -1)
				{
					segInfo[0].nodeCount = segInfo[0].nodeCount + segInfo[1].nodeCount;
					std::reverse(curImg->tracedNeuron.seg[segInfo[1].segID].row.begin(), curImg->tracedNeuron.seg[segInfo[1].segID].row.end());
					for (vector<V_NeuronSWC_unit>::iterator aa=curImg->tracedNeuron.seg[segInfo[1].segID].row.begin();
						aa!=curImg->tracedNeuron.seg[segInfo[1].segID].row.end(); ++aa) 
					{
						curImg->tracedNeuron.seg[segInfo[0].segID].row.push_back(*aa);
						aa->seg_id = segInfo[0].segID;
					}

					size_t nextSeg = 1;
					for (vector<V_NeuronSWC_unit>::iterator sort2segs=curImg->tracedNeuron.seg[segInfo[0].segID].row.begin();
						sort2segs!=curImg->tracedNeuron.seg[segInfo[0].segID].row.end(); ++sort2segs)
					{
						sort2segs->data[0] = nextSeg;
						sort2segs->data[6] = sort2segs->data[0] + 1;
						++nextSeg;
					}
					(curImg->tracedNeuron.seg[segInfo[0].segID].row.end()-1)->data[6] = -1;
					curImg->tracedNeuron.seg[segInfo[1].segID].to_be_deleted = true;
					segInfo.erase(segInfo.end()-1);
					cout << segInfo.size() << " " << segInfo[0].segID << " " << segInfo[0].nodeCount << endl;
					curImg->update_3drenderer_neuron_view(w, this);
					curImg->proj_trace_history_append();
					return;
				}
				else if (segInfo[1].nodeCount == 1)
				{
					if (segInfo[0].head_tail == 2)
					{
						++segInfo[0].nodeCount;
						std::reverse(curImg->tracedNeuron.seg[segInfo[0].segID].row.begin(), curImg->tracedNeuron.seg[segInfo[0].segID].row.end());
						curImg->tracedNeuron.seg[segInfo[1].segID].row[0].seg_id = segInfo[0].segID;
						curImg->tracedNeuron.seg[segInfo[0].segID].row.push_back(curImg->tracedNeuron.seg[segInfo[1].segID].row[0]);

						size_t nextSeg = 1;
						for (vector<V_NeuronSWC_unit>::iterator sort2segs=curImg->tracedNeuron.seg[segInfo[0].segID].row.begin();
							sort2segs!=curImg->tracedNeuron.seg[segInfo[0].segID].row.end(); ++sort2segs)
						{
							sort2segs->data[0] = nextSeg;
							sort2segs->data[6] = sort2segs->data[0] + 1;
							++nextSeg;
						}
						(curImg->tracedNeuron.seg[segInfo[0].segID].row.end()-1)->data[6] = -1;
						curImg->tracedNeuron.seg[segInfo[1].segID].to_be_deleted = true;
						segInfo.erase(segInfo.end()-1);
						cout << segInfo.size() << " " << segInfo[0].segID << " " << segInfo[0].nodeCount << endl;
						curImg->update_3drenderer_neuron_view(w, this);
						curImg->proj_trace_history_append();
						return;
					} 
					else if (segInfo[0].head_tail == -1)
					{
						++segInfo[0].nodeCount;
						curImg->tracedNeuron.seg[segInfo[1].segID].row[0].seg_id = segInfo[0].segID;
						curImg->tracedNeuron.seg[segInfo[0].segID].row.push_back(curImg->tracedNeuron.seg[segInfo[1].segID].row[0]);

						size_t nextSeg = 1;
						for (vector<V_NeuronSWC_unit>::iterator sort2segs=curImg->tracedNeuron.seg[segInfo[0].segID].row.begin();
							sort2segs!=curImg->tracedNeuron.seg[segInfo[0].segID].row.end(); ++sort2segs)
						{
							sort2segs->data[0] = nextSeg;
							sort2segs->data[6] = sort2segs->data[0] + 1;
							++nextSeg;
						}
						(curImg->tracedNeuron.seg[segInfo[0].segID].row.end()-1)->data[6] = -1;
						curImg->tracedNeuron.seg[segInfo[1].segID].to_be_deleted = true;
						segInfo.erase(segInfo.end()-1);
						cout << segInfo.size() << " " << segInfo[0].segID << " " << segInfo[0].nodeCount << endl;
						curImg->update_3drenderer_neuron_view(w, this);
						curImg->proj_trace_history_append();
						return;
					}
				}
			}
			
			for (vector<segInfoUnit>::iterator segIt=segInfo.begin(); segIt!=segInfo.end(); ++segIt)
			{
				if (segIt->nodeCount > 1) continue;
				else if (segIt->nodeCount == 1) 
				{
					if ((segIt+1) <= (segInfo.end()-1) && (segIt+1)->nodeCount > 1)
					{
						++segIt;
						continue;
					}
					else 
					{
						while ((segIt+1) <= (segInfo.end()-1) && (segIt+1)->nodeCount == 1)
						{
							curImg->tracedNeuron.seg[segIt->segID].row.push_back(curImg->tracedNeuron.seg[(segIt+1)->segID].row[0]);
							(curImg->tracedNeuron.seg[segIt->segID].row.end()-1)->seg_id = segIt->segID;
							++(segIt->nodeCount);
							curImg->tracedNeuron.seg[(segIt+1)->segID].to_be_deleted = true;
							segInfo.erase(segIt+1);
						}
						segIt->refine = true;
					}
				}
			}

			for (vector<segInfoUnit>::iterator refineIt=segInfo.begin(); refineIt!=segInfo.end(); ++refineIt)
			{
				if (refineIt->refine == true && refineIt->nodeCount >= 2) 
				{
					segmentStraighten(curImg->tracedNeuron.seg[refineIt->segID].row, curImg, refineIt);
					refineIt->head_tail = 2;
				}
			}
			if (segInfo.begin()->refine == true) segInfo.begin()->head_tail = -1;

			cout << endl << endl << " ===================== after merging dots, " << segInfo.size() << " segments left" << endl;
			for (vector<segInfoUnit>::iterator test=segInfo.begin(); test!=segInfo.end(); ++test) cout << test->segID << ":" << test->nodeCount << " " << test->refine << " " << test->head_tail << ", ";
			cout << endl;
			/* ============== END of [Get all segments information included in the movePen trajectory, merge, and refine.] ============== */

			/* ============================== Connet segments ============================== */
			vector<segInfoUnit>::iterator it=segInfo.begin();
			if (segInfo.size() > 0)
			{
				curImg->tracedNeuron.seg[segInfo[0].segID].row[0].seg_id = segInfo[0].segID;
				if (segInfo[0].head_tail == -1)
				{
					while (it != (segInfo.end()-1))
					{
						if ((it+1)->head_tail == -1) 
						{
							for (vector<V_NeuronSWC_unit>::iterator itNextSeg=curImg->tracedNeuron.seg[(it+1)->segID].row.end()-1;
								itNextSeg>=curImg->tracedNeuron.seg[(it+1)->segID].row.begin(); --itNextSeg)
							{
								itNextSeg->seg_id = (it+1)->segID;
								curImg->tracedNeuron.seg[segInfo[0].segID].row.push_back(*itNextSeg);
							}
						}
						else if ((it+1)->head_tail == 2)
						{
							for (vector<V_NeuronSWC_unit>::iterator itNextSeg=curImg->tracedNeuron.seg[(it+1)->segID].row.begin();
							itNextSeg!=curImg->tracedNeuron.seg[(it+1)->segID].row.end(); ++itNextSeg)
							{
								itNextSeg->seg_id = (it+1)->segID;
								curImg->tracedNeuron.seg[segInfo[0].segID].row.push_back(*itNextSeg);
							}
						}
						curImg->tracedNeuron.seg[(it+1)->segID].to_be_deleted = true;
						segInfoShow.push_back((it+1)->segID);
						++it;
					} 

					size_t nextSegNo = 1;
					for (vector<V_NeuronSWC_unit>::iterator itSort=curImg->tracedNeuron.seg[segInfo[0].segID].row.begin();
						itSort!=curImg->tracedNeuron.seg[segInfo[0].segID].row.end(); ++itSort)
					{
						itSort->data[0] = nextSegNo;
						itSort->data[6] = itSort->data[0] + 1;
						++nextSegNo;
					}
					(curImg->tracedNeuron.seg[segInfo[0].segID].row.end()-1)->data[6] = -1;
				}
				else if (segInfo[0].head_tail == 2)
				{
					std::reverse(curImg->tracedNeuron.seg[segInfo[0].segID].row.begin(), curImg->tracedNeuron.seg[segInfo[0].segID].row.end());
					while (it != (segInfo.end()-1))
					{
						if ((it+1)->head_tail == -1) 
						{
							for (vector<V_NeuronSWC_unit>::iterator itNextSeg=curImg->tracedNeuron.seg[(it+1)->segID].row.end()-1;
							itNextSeg>=curImg->tracedNeuron.seg[(it+1)->segID].row.begin(); itNextSeg--)
							{
								itNextSeg->seg_id = (it+1)->segID;
								curImg->tracedNeuron.seg[segInfo[0].segID].row.push_back(*itNextSeg);
							}
						}
						else if ((it+1)->head_tail == 2)
						{
							for (vector<V_NeuronSWC_unit>::iterator itNextSeg=curImg->tracedNeuron.seg[(it+1)->segID].row.begin();
							itNextSeg!=curImg->tracedNeuron.seg[(it+1)->segID].row.end(); itNextSeg++)
							{
								itNextSeg->seg_id = (it+1)->segID;
								curImg->tracedNeuron.seg[segInfo[0].segID].row.push_back(*itNextSeg);
							}
						}
						curImg->tracedNeuron.seg[(it+1)->segID].to_be_deleted = true;
						segInfoShow.push_back((it+1)->segID);
						++it;
					}

					std::reverse(curImg->tracedNeuron.seg[segInfo[0].segID].row.begin(), curImg->tracedNeuron.seg[segInfo[0].segID].row.end());
					size_t nextSegNo = 1;
					for (vector<V_NeuronSWC_unit>::iterator itSort=curImg->tracedNeuron.seg[segInfo[0].segID].row.begin();
						itSort!=curImg->tracedNeuron.seg[segInfo[0].segID].row.end(); itSort++)
					{
						itSort->data[0] = nextSegNo;
						itSort->data[6] = itSort->data[0] + 1;
						++nextSegNo;
					}
					(curImg->tracedNeuron.seg[segInfo[0].segID].row.end()-1)->data[6] = -1;
				}

				for (vector<V_NeuronSWC_unit>::iterator reID=curImg->tracedNeuron.seg[segInfo[0].segID].row.begin(); 
					reID!=curImg->tracedNeuron.seg[segInfo[0].segID].row.end(); ++reID) 
				{
					reID->seg_id = segInfo[0].segID;
                    reID->type = 3;
				}
			}
			/* ============================== END of [Connet segments] ============================== */

            curImg->update_3drenderer_neuron_view(w, this);
            curImg->proj_trace_history_append();

			segInfoShow.push_back(segInfo.size());
			segInfoShow.push_back(curImg->tracedNeuron.seg.size());
			segInfoShow.push_back(segInfoShow[1] - segInfoShow[0]);
        }
    }
	return;
}

void Renderer_gl1::connectPointCloudByStroke()
{
	connectEdit = pointCloudEdit;

	//qDebug() << listCell.size();
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
	XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

    //v3d_msg(QString("getNumShiftHolding() = ") + QString(w->getNumShiftHolding() ? "YES" : "no"));
    float tolerance = 7; // tolerance distance from the backprojected neuron to the curve point
	
	/* ======== Only take in the nodes within the rectangle that contains the stroke ======== */
	long minX = list_listCurvePos.at(0).at(0).x, maxX = list_listCurvePos.at(0).at(0).x;
	long minY = list_listCurvePos.at(0).at(0).y, maxY = list_listCurvePos.at(0).at(0).y;
	for (size_t i=0; i<list_listCurvePos.at(0).size(); ++i)
	{
		if (list_listCurvePos.at(0).at(i).x <= minX) minX = list_listCurvePos.at(0).at(i).x;
		if (list_listCurvePos.at(0).at(i).x >= maxX) maxX = list_listCurvePos.at(0).at(i).x;
		if (list_listCurvePos.at(0).at(i).y <= minY) minY = list_listCurvePos.at(0).at(i).y;
		if (list_listCurvePos.at(0).at(i).y >= maxY) maxY = list_listCurvePos.at(0).at(i).y;
	}
	minX = minX - 5; maxX = maxX + 5;
	minY = minY - 5; maxY = maxY + 5;
	//cout << minX << " " << maxX << " " << minY << " " << maxY << endl;
	QList<CellAPO> nodeOnStroke;
	for (size_t i=0; i<listCell.size(); ++i)
	{
		GLdouble px, py, pz, ix, iy, iz;
		ix = listCell[i].x;
		iy = listCell[i].y;
		iz = listCell[i].z;
		if (gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
		{
			py = viewport[3]-py; //the Y axis is reversed
			QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));
			if ((p.x()>=minX && p.x()<=maxX) && (p.y()>=minY && p.y()<=maxY))
			{
				nodeOnStroke.push_back(listCell[i]);
				//cout << p.x() << " " << p.y() << endl;
			}
		}
	}
	/* ==== END of [Only take in the nodes within the rectangle that contains the stroke] ==== */

	V_NeuronSWC_unit node;
	V_NeuronSWC_unit nodeTemp;
	V_NeuronSWC newSeg;
	for (V3DLONG i=0; i<list_listCurvePos.at(0).size(); i++)
	{
		for (V3DLONG j=0; j<nodeOnStroke.size(); j++)
		{
			GLdouble px, py, pz, ix, iy, iz;
			ix = nodeOnStroke[j].x;
			iy = nodeOnStroke[j].y;
			iz = nodeOnStroke[j].z;
			if(gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
			{
				py = viewport[3]-py; //the Y axis is reversed
				QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));

                QPointF p2(list_listCurvePos.at(0).at(i).x, list_listCurvePos.at(0).at(i).y);
				if( std::sqrt((p.x()-p2.x())*(p.x()-p2.x()) + (p.y()-p2.y())*(p.y()-p2.y())) <= tolerance  )
                {
					//cout << ix << " " << iy << " " << iz << endl;
					nodeTemp.set(ix, iy, iz, 1, 2);
					bool repeat = false;
					for (vector<V_NeuronSWC_unit>::iterator pointIt=newSeg.row.begin(); pointIt!=newSeg.row.end(); ++pointIt)
					{
						if (nodeTemp.x==pointIt->x && nodeTemp.y==pointIt->y && nodeTemp.z==pointIt->z)
						{
							repeat = true;
							break;
						}
					}
					if (repeat == false)
					{
						node = nodeTemp;
						newSeg.row.push_back(node);
					}
				}
			}
		}
	}
	if (newSeg.row.empty()) return;
	size_t nodeLabel = 1, segNum = curImg->tracedNeuron.seg.size() + 1;
	std::reverse(newSeg.row.begin(), newSeg.row.end());
	for (vector<V_NeuronSWC_unit>::iterator itSort=newSeg.row.begin(); itSort!=newSeg.row.end(); itSort++)
	{
		itSort->seg_id = segNum;
        itSort->type = 3;
		itSort->data[0] = nodeLabel;
		itSort->data[6] = nodeLabel + 1;
		++nodeLabel;
	}
	(newSeg.row.end()-1)->data[6] = -1;
	cout << " -- number of points included: " << newSeg.row.size() << endl; 

	vector<segInfoUnit> createdSegs;
	segInfoUnit singleSeg;
	singleSeg.nodeCount = newSeg.row.size();
	singleSeg.refine = true;
	singleSeg.segID = curImg->tracedNeuron.seg.size() + 1;
	createdSegs.push_back(singleSeg);
	vector<segInfoUnit>::iterator refinIt = createdSegs.begin();
	segmentStraighten(newSeg.row, curImg, refinIt);
	curImg->tracedNeuron.seg.push_back(newSeg);

	size_t totalSeg = curImg->tracedNeuron.seg.size();
	segInfoShow.push_back(totalSeg);

	curImg->update_3drenderer_neuron_view(w, this);
    curImg->proj_trace_history_append();
	
	return;
}

void Renderer_gl1::connectMarkerByStroke()
{
	connectEdit = markerEdit;

	//qDebug() << listCell.size();
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
	XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

    //v3d_msg(QString("getNumShiftHolding() = ") + QString(w->getNumShiftHolding() ? "YES" : "no"));
    float tolerance = 15; // tolerance distance from the backprojected neuron to the curve point
	
	/* ======== Only take in the nodes within the rectangle that contains the stroke ======== */
	long minX = list_listCurvePos.at(0).at(0).x, maxX = list_listCurvePos.at(0).at(0).x;
	long minY = list_listCurvePos.at(0).at(0).y, maxY = list_listCurvePos.at(0).at(0).y;
	for (size_t i=0; i<list_listCurvePos.at(0).size(); ++i)
	{
		if (list_listCurvePos.at(0).at(i).x <= minX) minX = list_listCurvePos.at(0).at(i).x;
		if (list_listCurvePos.at(0).at(i).x >= maxX) maxX = list_listCurvePos.at(0).at(i).x;
		if (list_listCurvePos.at(0).at(i).y <= minY) minY = list_listCurvePos.at(0).at(i).y;
		if (list_listCurvePos.at(0).at(i).y >= maxY) maxY = list_listCurvePos.at(0).at(i).y;
	}
	minX = minX - 5; maxX = maxX + 5;
	minY = minY - 5; maxY = maxY + 5;
	//cout << minX << " " << maxX << " " << minY << " " << maxY << endl;
	QList<ImageMarker> nodeOnStroke;
	for (size_t i=0; i<listMarker.size(); ++i)
	{
		GLdouble px, py, pz, ix, iy, iz;
		ix = listMarker[i].x;
		iy = listMarker[i].y;
		iz = listMarker[i].z;
		if (gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
		{
			py = viewport[3]-py; //the Y axis is reversed
			QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));
			if ((p.x()>=minX && p.x()<=maxX) && (p.y()>=minY && p.y()<=maxY))
			{
				nodeOnStroke.push_back(listMarker[i]);
				//cout << p.x() << " " << p.y() << endl;
			}
		}
	}
	/* ==== END of [Only take in the nodes within the rectangle that contains the stroke] ==== */

	V_NeuronSWC_unit node;
	V_NeuronSWC_unit nodeTemp;
	V_NeuronSWC newSeg;
	for (V3DLONG i=0; i<list_listCurvePos.at(0).size(); i++)
	{
		for (V3DLONG j=0; j<nodeOnStroke.size(); j++)
		{
			GLdouble px, py, pz, ix, iy, iz;
			ix = nodeOnStroke[j].x - 1;
			iy = nodeOnStroke[j].y - 1;
			iz = nodeOnStroke[j].z - 1;
			if(gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
			{
				py = viewport[3]-py; //the Y axis is reversed
				QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));

                QPointF p2(list_listCurvePos.at(0).at(i).x, list_listCurvePos.at(0).at(i).y);
				if( std::sqrt((p.x()-p2.x())*(p.x()-p2.x()) + (p.y()-p2.y())*(p.y()-p2.y())) <= tolerance  )
                {
					//cout << ix << " " << iy << " " << iz << endl;
					nodeTemp.set(ix, iy, iz, 1, 2);
					bool repeat = false;
					for (vector<V_NeuronSWC_unit>::iterator pointIt=newSeg.row.begin(); pointIt!=newSeg.row.end(); ++pointIt)
					{
						if (nodeTemp.x==pointIt->x && nodeTemp.y==pointIt->y && nodeTemp.z==pointIt->z)
						{
							repeat = true;
							break;
						}
					}
					if (repeat == false)
					{
						node = nodeTemp;
						newSeg.row.push_back(node);
					}
				}
			}
		}
	}
	if (newSeg.row.empty()) return;
	size_t nodeLabel = 1, segNum = curImg->tracedNeuron.seg.size() + 1;
	std::reverse(newSeg.row.begin(), newSeg.row.end());
	for (vector<V_NeuronSWC_unit>::iterator itSort=newSeg.row.begin(); itSort!=newSeg.row.end(); itSort++)
	{
		itSort->seg_id = segNum;
        itSort->type = 3;
		itSort->data[0] = nodeLabel;
		itSort->data[6] = nodeLabel + 1;
		++nodeLabel;
	}
	(newSeg.row.end()-1)->data[6] = -1;
	cout << " -- number of markers included: " << newSeg.row.size() << endl; 

	vector<segInfoUnit> createdSegs;
	segInfoUnit singleSeg;
	singleSeg.nodeCount = newSeg.row.size();
	singleSeg.refine = true;
	singleSeg.segID = curImg->tracedNeuron.seg.size() + 1;
	createdSegs.push_back(singleSeg);
	vector<segInfoUnit>::iterator refinIt = createdSegs.begin();
	segmentStraighten(newSeg.row, curImg, refinIt);
	curImg->tracedNeuron.seg.push_back(newSeg);

	size_t totalSeg = curImg->tracedNeuron.seg.size();
	segInfoShow.push_back(totalSeg);

	curImg->update_3drenderer_neuron_view(w, this);
    curImg->proj_trace_history_append();
	
	return;
}

void Renderer_gl1::segmentStraighten(vector<V_NeuronSWC_unit>& inputSeg, My4DImage*& curImgPtr, vector<segInfoUnit>::iterator& segInfoPtr)
{
	// When using the connecting tool other than 'simple connecting', the connected segments will be passed here to be smoothed.
	// This smoothing algorithm is an iterative process based on the geometrical profile of the whole segment.
	// ------- MK, Jun, 2017

	if (inputSeg.size() <= 3) return;

#define PI 3.1415926
	//long segInfoNodeCount = 0;
	
	double dot, sq1, sq2, dist, turnCost, radAngle, turnCostMean = 0;
	double mainSqr = ((inputSeg.end()-1)->x-inputSeg.begin()->x)*((inputSeg.end()-1)->x-inputSeg.begin()->x) + 
					 ((inputSeg.end()-1)->y-inputSeg.begin()->y)*((inputSeg.end()-1)->y-inputSeg.begin()->y) +
					 ((inputSeg.end()-1)->z-inputSeg.begin()->z)*((inputSeg.end()-1)->z-inputSeg.begin()->z);
	double segDist = sqrt(mainSqr);
	double nodeHeadDot, nodeHeadSqr, nodeHeadRadAngle, nodeToMainDist, nodeToMainDistMean = 0;
	//cout << "segment displacement: " << segDist << endl << endl;

	cout << "    -- nodes contained in the segment to be straightned: " << segInfoPtr->nodeCount << endl;
	for (vector<V_NeuronSWC_unit>::iterator check=inputSeg.begin(); check!=inputSeg.end(); ++check) cout << "[" << check->x << " " << check->y << " " << check->z << "] ";
	cout << endl;
	vector<nodeInfo> pickedNode;
	for (vector<V_NeuronSWC_unit>::iterator it=inputSeg.begin()+1; it!=inputSeg.end()-1; ++it)
	{	
		dot = ((it-1)->x-it->x)*((it+1)->x-it->x) + ((it-1)->y-it->y)*((it+1)->y-it->y) + ((it-1)->z-it->z)*((it+1)->z-it->z);
		sq1 = ((it-1)->x-it->x)*((it-1)->x-it->x) + ((it-1)->y-it->y)*((it-1)->y-it->y) + ((it-1)->z-it->z)*((it-1)->z-it->z);
		sq2 = ((it+1)->x-it->x)*((it+1)->x-it->x) + ((it+1)->y-it->y)*((it+1)->y-it->y) + ((it+1)->z-it->z)*((it+1)->z-it->z);
		if (isnan( acos(dot/sqrt(sq1*sq2)) )) return;
		radAngle = acos(dot/sqrt(sq1*sq2));
		nodeHeadDot = (it->x-inputSeg.begin()->x)*((inputSeg.end()-1)->x-inputSeg.begin()->x) + 
					  (it->y-inputSeg.begin()->y)*((inputSeg.end()-1)->y-inputSeg.begin()->y) + 
					  (it->z-inputSeg.begin()->z)*((inputSeg.end()-1)->z-inputSeg.begin()->z);
		nodeHeadSqr = (it->x-inputSeg.begin()->x)*(it->x-inputSeg.begin()->x) + (it->y-inputSeg.begin()->y)*(it->y-inputSeg.begin()->y) + (it->z-inputSeg.begin()->z)*(it->z-inputSeg.begin()->z);
		nodeHeadRadAngle = PI - acos(nodeHeadDot/sqrt(mainSqr*nodeHeadSqr));
		nodeToMainDist = sqrt(nodeHeadSqr) * sin(nodeHeadRadAngle);
		nodeToMainDistMean = nodeToMainDistMean + nodeToMainDist;
		cout << "       d(node-main):" << nodeToMainDist << " radian/pi:" << (radAngle/PI) << " turning cost:" << (sqrt(sq1)+sqrt(sq2)) / (radAngle/PI) << " " << it->x << " " << it->y << " " << it->z << endl;
		
		if ((radAngle/PI) < 0.6) // Detecting sharp turns and distance outliers => a) obviously errorneous depth situation
		{
			nodeInfo sharp;
			sharp.x = it->x; sharp.y = it->y; sharp.z = it->z;
			cout << "this node is picked" << endl;
			
			sharp.segID = it->seg_id;
			sharp.distToMain = nodeToMainDist;
			sharp.sqr1 = sq1; sharp.sqr2 = sq2; sharp.dot = dot;
			sharp.radAngle = radAngle;
			sharp.nodeAddress = it;
			sharp.turnCost = (sqrt(sq1)+sqrt(sq2)) / (radAngle/PI);
			
			pickedNode.push_back(sharp);
			turnCostMean = turnCostMean + sharp.turnCost;
		}
	}
	if (pickedNode.empty()) return;

	nodeToMainDistMean = nodeToMainDistMean / (inputSeg.size()-2);
	turnCostMean = turnCostMean / pickedNode.size();
	cout << endl << endl << "  ==== start deleting nodes... " << endl;
	
	int delete_count = 0;
	ptrdiff_t order = 0;
	vector<V_NeuronSWC> preservedSegs;
	for (vector<nodeInfo>::iterator it=pickedNode.begin(); it!=pickedNode.end(); ++it)
	{
		cout << "  Avg(d(node_main)):" << nodeToMainDistMean << " d(node-main):" << it->distToMain << " Avg(turning cost):" << turnCostMean << " turning cost:" << it->turnCost;
		cout << " [" << it->x << " " << it->y << " " << it->z << "] " << endl;
		if (it->distToMain>=nodeToMainDistMean || it->turnCost>=turnCostMean || it->distToMain>=segDist)
		{
			++delete_count;
			if (connectEdit == segmentEdit) curImgPtr->tracedNeuron.seg[it->segID].to_be_deleted = true;
			--(segInfoPtr->nodeCount);

			V_NeuronSWC preservedNode;
			preservedNode.row.push_back(*(it->nodeAddress - order));
			preservedNode.row[0].data[6] = -1;
			preservedSegs.push_back(preservedNode);
			cout << "delete [" << (it->nodeAddress-order)->x << " " << (it->nodeAddress-order)->y << " " << (it->nodeAddress-order)->z << "] " << delete_count << endl;
			inputSeg.erase(it->nodeAddress - order);
			++order;
			cout << "----" << endl;
		}
	}

	cout << endl << "  ==== cheking angles... " << endl;
	
	int deleteCount2;
	do
	{
		deleteCount2 = 0;
		for (vector<V_NeuronSWC_unit>::iterator it=inputSeg.begin()+1; it!=inputSeg.end()-1; ++it)
		{
			double dot2 = (it->x-(it-1)->x)*((it+1)->x-it->x) + (it->y-(it-1)->y)*((it+1)->y-it->y) + (it->z-(it-1)->z)*((it+1)->z-it->z);
			double sq1_2 = ((it-1)->x-it->x)*((it-1)->x-it->x) + ((it-1)->y-it->y)*((it-1)->y-it->y) + ((it-1)->z-it->z)*((it-1)->z-it->z);
			double sq2_2 = ((it+1)->x-it->x)*((it+1)->x-it->x) + ((it+1)->y-it->y)*((it+1)->y-it->y) + ((it+1)->z-it->z)*((it+1)->z-it->z);
			if (isnan( acos(dot2/sqrt(sq1_2*sq2_2)) )) break;
			double radAngle_2 = acos(dot2/sqrt(sq1_2*sq2_2));
			cout << "2nd rad Angle:" << radAngle_2 << " [" << it->x << " " << it->y << " " << it->z << "]" << endl;
			
			if ((radAngle_2/PI)*180 > 75) 
			{
				if (sqrt(sq1_2) > (1/10)*sqrt(sq2_2))
				{
					--(segInfoPtr->nodeCount);
					++deleteCount2;
					cout << "delete " << " [" << it->x << " " << it->y << " " << it->z << "] " << deleteCount2 << endl;
					if ((inputSeg.size()-deleteCount2) <= 2)
					{
						--deleteCount2;
						++(segInfoPtr->nodeCount);
						break;
					}
					if (connectEdit == segmentEdit) curImgPtr->tracedNeuron.seg[it->seg_id].to_be_deleted = true;

					V_NeuronSWC preservedNode;
					preservedNode.row.push_back(*it);
					preservedNode.row[0].data[6] = -1;
					preservedSegs.push_back(preservedNode);
					inputSeg.erase(it);
				}
			}
		}
		cout << "deleted nodes: " << deleteCount2 << "\n=================" << endl;
	} while (deleteCount2 > 0);
	
	size_t label = 1;
	cout << "number of nodes after straightening process: " << inputSeg.size() << " ( segID = " << segInfoPtr->segID << " )" << endl;
	//cout << "seg num: " << curImgPtr->tracedNeuron.seg.size() << endl;
	for (vector<V_NeuronSWC_unit>::iterator it=inputSeg.begin(); it!=inputSeg.end(); ++it)
	{
		it->seg_id = segInfoPtr->segID;
		it->data[0] = label; 
		it->data[6] = label + 1;
		++label;
		cout << "[" << it->seg_id << ": " << it->x << " " << it->y << " " << it->z << "] ";
	}
	cout << endl;
	(inputSeg.end()-1)->data[6] = -1;
	
	if (connectEdit == segmentEdit)
	{
		V_NeuronSWC newSeg;
		newSeg.row = inputSeg;
		curImgPtr->tracedNeuron.seg[segInfoPtr->segID] = newSeg;
	
		size_t singleNodeCount = 1;
		for (vector<V_NeuronSWC>::iterator nodeIt=preservedSegs.begin(); nodeIt!=preservedSegs.end(); ++nodeIt)
		{
			nodeIt->row[0].seg_id = curImgPtr->tracedNeuron.seg.size() + singleNodeCount;
			++singleNodeCount;
			curImgPtr->tracedNeuron.seg.push_back(*nodeIt);
			//cout << "seg num: " << curImgPtr->tracedNeuron.seg.size() << endl;
		}
	}
		
	return;
}
// ---------------- END of [segment/points could/marker connecting tool, by MK 2017 April] ---------------------

// ------------------------ neuron chopping tool, by MK 2017 June ----------------------------
void Renderer_gl1::cutNeuronsByStroke()
{
	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
	XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

    //v3d_msg(QString("getNumShiftHolding() = ") + QString(w->getNumShiftHolding() ? "YES" : "no"));
    float tolerance = 100; // tolerance distance from the backprojected neuron to the curve point
	
    // back-project the node curve points and mark segments to be chopped
    for(V3DLONG j=0; j<listNeuronTree.size(); j++)
    {
        NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(j))); //curEditingNeuron-1
        if (p_tree && p_tree->editable)    // @FIXED by Alessandro on 2015-05-23. Removing segments from non-editable neurons causes crash.
        {
            QList <NeuronSWC> *p_listneuron = &(p_tree->listNeuron);
            if (!p_listneuron) continue;
			//for (int testi=0; testi<list_listCurvePos.at(0).size(); testi++) qDebug() << list_listCurvePos.at(0).at(testi).x << " " << list_listCurvePos.at(0).at(testi).y;
			
			/* ============== Get all segments and nodes information included in the movePen trajectory. ============== */
			  /* ======== Only take in the nodes within the rectangle that contains the stroke ======== */
			long minX = list_listCurvePos.at(0).at(0).x, maxX = list_listCurvePos.at(0).at(0).x;
			long minY = list_listCurvePos.at(0).at(0).y, maxY = list_listCurvePos.at(0).at(0).y;
			for (size_t i=0; i<list_listCurvePos.at(0).size(); ++i)
			{
				if (list_listCurvePos.at(0).at(i).x <= minX) minX = list_listCurvePos.at(0).at(i).x;
				if (list_listCurvePos.at(0).at(i).x >= maxX) maxX = list_listCurvePos.at(0).at(i).x;
				if (list_listCurvePos.at(0).at(i).y <= minY) minY = list_listCurvePos.at(0).at(i).y;
				if (list_listCurvePos.at(0).at(i).y >= maxY) maxY = list_listCurvePos.at(0).at(i).y;
			}
			minX = minX - 5; maxX = maxX + 5;
			minY = minY - 5; maxY = maxY + 5;
			//cout << minX << " " << maxX << " " << minY << " " << maxY << endl;
			QList<NeuronSWC> nodeOnStroke;
			for (size_t i=0; i<p_listneuron->size(); ++i)
			{
				GLdouble px, py, pz, ix, iy, iz;
				ix = p_listneuron->at(i).x;
				iy = p_listneuron->at(i).y;
				iz = p_listneuron->at(i).z;
				if (gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
				{
					py = viewport[3]-py; //the Y axis is reversed
					QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));
					if ((p.x()>=minX && p.x()<=maxX) && (p.y()>=minY && p.y()<=maxY))
					{
						nodeOnStroke.push_back(p_listneuron->at(i));
						//cout << p.x() << " " << p.y() << endl;
					}
				}
			}
			//for (QList<NeuronSWC>::iterator strokeIt=nodeOnStroke.begin(); strokeIt!=nodeOnStroke.end(); ++strokeIt) cout << strokeIt->n << " " << strokeIt->parent << endl;
			  /* ==== END of [Only take in the nodes within the rectangle that contains the stroke] ==== */
			
			/* ==== Group nodes with same segID ========================================================= */
			vector< vector<cutNode> > sameSegList;
			vector<cutNode> dummyList;
			sameSegList.push_back(dummyList);
			cutNode dummyNode;
			dummyNode.segID = 0;
			sameSegList[0].push_back(dummyNode);
			for (V3DLONG i=0; i<list_listCurvePos.at(0).size(); ++i)
			{
				for (V3DLONG j=0; j<nodeOnStroke.size(); ++j)
				{
					GLdouble px, py, pz, ix, iy, iz;
					ix = nodeOnStroke.at(j).x;
					iy = nodeOnStroke.at(j).y;
					iz = nodeOnStroke.at(j).z;
					if(gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
					{
						py = viewport[3]-py; //the Y axis is reversed
						QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));

                        QPointF p2(list_listCurvePos.at(0).at(i).x, list_listCurvePos.at(0).at(i).y);
						double dist = 0;
						dist = std::sqrt( (p.x()-p2.x())*(p.x()-p2.x()) + (p.y()-p2.y())*(p.y()-p2.y()) );
						if( dist <= tolerance )
                        {
							for (vector<V_NeuronSWC_unit>::iterator it=curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.begin();
								it!=curImg->tracedNeuron.seg[nodeOnStroke.at(j).seg_id].row.end(); ++it)
							{
								if (nodeOnStroke.at(j).x==it->data[2] && nodeOnStroke.at(j).y==it->data[3] && nodeOnStroke.at(j).z==it->data[4]) 
								{
									vector< vector<cutNode> >::iterator segCheck;
									for (segCheck=sameSegList.begin(); segCheck!=sameSegList.end(); ++segCheck)
									{
										if (nodeOnStroke.at(j).seg_id == segCheck->at(0).segID)
										{
											cutNode cut;
											cut.node = *it;
											cut.distance = dist;
											cut.nodeAddress = it;
											cut.segID = nodeOnStroke.at(j).seg_id;
											cut.nodeinSegId = nodeOnStroke.at(j).nodeinseg_id;
											segCheck->push_back(cut);
											//cout << cut.segID << " " << cut.node.data[0] << " " << endl; 
											break;
										}
									}
									if (segCheck == sameSegList.end())
									{
										vector<cutNode> newSameSegCluster;
										cutNode cut;
										cut.node = *it;
										cut.distance = dist;
										cut.nodeAddress = it;
										cut.segID = nodeOnStroke.at(j).seg_id;
										cut.nodeinSegId = nodeOnStroke.at(j).nodeinseg_id;
										newSameSegCluster.push_back(cut);
										sameSegList.push_back(newSameSegCluster);
										//cout << cut.segID << " " << cut.node.data[0] << " " << endl;
									}
								}
							}
						}
					}
				}
			}
			/* ==== END of [Group nodes with same segID ] ================================================ */

			/* ==== Cut the original segment short, append the remnant segment ============================ */
			double smallest;
			vector<V_NeuronSWC_unit>::iterator cutAddress;
			vector<cutNode>::iterator cutIt;
			for (vector< vector<cutNode> >::iterator listCheck=sameSegList.begin()+1; listCheck!=sameSegList.end(); ++listCheck)
			{
				smallest = listCheck->at(0).distance;
				for (vector<cutNode>::iterator cutCheck=listCheck->begin(); cutCheck!=listCheck->end(); ++cutCheck)
				{
					if (cutCheck->distance <= smallest) 
					{
						smallest = cutCheck->distance;
						cutAddress = cutCheck->nodeAddress;
						cutIt = cutCheck;
					}
					//cout << cutCheck->segID << " " << cutCheck->node.data[0] << " " << cutCheck->distance << endl;
				}

				V_NeuronSWC newSeg;
				vector<V_NeuronSWC_unit>::const_iterator newSegStart = curImg->tracedNeuron.seg[cutIt->segID].row.begin();
				vector<V_NeuronSWC_unit>::const_iterator newSegEnd = cutAddress + 1;
				vector<V_NeuronSWC_unit> newRow(newSegStart, newSegEnd);
				newSeg.row = newRow;
				(newSeg.row.end()-1)->data[6] = -1;

				//cout << curImg->tracedNeuron.seg[cutIt->segID].row.size() << endl;
				size_t i;
				for (i=0; i<newSeg.row.size(); ++i) 
					curImg->tracedNeuron.seg[cutIt->segID].row.erase(curImg->tracedNeuron.seg[cutIt->segID].row.begin());
				//cout << curImg->tracedNeuron.seg[cutIt->segID].row.size() << endl;
				curImg->tracedNeuron.append(newSeg);
				/* ==== END of [Cut the original segment short, append the remnant segment] ================= */
			}		
		}

		curImg->update_3drenderer_neuron_view(w, this);
        curImg->proj_trace_history_append();
	}

	return;
}
// --------------------- END of [neuron cutting tool, by MK 2017 June] -----------------------
void Renderer_gl1::retypeMultiNeuronsbyshortcut()
{
    qDebug()<<"retypeMultiNeuronsbyshortcut";
	int node_type = 0;
	int node_level = 0;

	bool ok;
	bool contour_mode = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);
	bool node_mode = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
    qDebug()<<contour_mode;
	if (neuronColorMode == 0)
	{
		if (useCurrentTraceTypeForRetyping)
		{
			node_type = currentTraceType;
		}
        else if(contour_mode)
		{
			node_type = QInputDialog::getInteger(0, QObject::tr("Change node type in segment"),
				QObject::tr("SWC type: "
				"\n 0 -- undefined (white)"
				"\n 1 -- soma (black)"
				"\n 2 -- axon (red)"
				"\n 3 -- dendrite (blue)"
				"\n 4 -- apical dendrite (purple)"
				"\n else -- custom \n"),
				currentTraceType, 0, 100, 1, &ok);
            if (!ok) return;
        }else{
            qDebug()<<"heiheihiehi";
            node_type=2;
        }


		currentTraceType = node_type;
        const int neuron_type_color[ ][3] = {///////////////////////////////////////////////////////
                {255, 255, 255},  // white,   0-undefined
                {20,  20,  20 },  // black,   1-soma
                {200, 20,  0  },  // red,     2-axon
                {0,   20,  200},  // blue,    3-dendrite
                {200, 0,   200},  // purple,  4-apical dendrite
                //the following is Hanchuan's extended color. 090331
                {0,   200, 200},  // cyan,    5
                {220, 200, 0  },  // yellow,  6
                {0,   200, 20 },  // green,   7
                {188, 94,  37 },  // coffee,  8
                {180, 200, 120},  // asparagus,	9
                {250, 100, 120},  // salmon,	10
                {120, 200, 200},  // ice,		11
                {100, 120, 200},  // orchid,	12
            //the following is Hanchuan's further extended color. 111003
            {255, 128, 168},  //	13
            {128, 255, 168},  //	14
            {128, 168, 255},  //	15
            {168, 255, 128},  //	16
            {255, 168, 128},  //	17
            {168, 128, 255}, //	18
            {0, 0, 0}, //19 //totally black. PHC, 2012-02-15
                };
        currentMarkerColor.r=neuron_type_color[node_type][0];
        currentMarkerColor.g=neuron_type_color[node_type][1];
        currentMarkerColor.b=neuron_type_color[node_type][2];
		return;
	}
	else
		return;
}
void Renderer_gl1::retypeMultiNeuronsByStroke()
{

    qDebug()<<"use this retypeMultiNeuronByStrokeFunction";

    int node_type = 0;
    int node_level = 0;

    bool ok;
    bool contour_mode = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);
    bool node_mode = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);

    if(neuronColorMode==0)
    {
                node_type = currentTraceType;
        if (useCurrentTraceTypeForRetyping)
        {
            node_type = currentTraceType;
        }
        else
        {

            node_type = QInputDialog::getInteger(0, QObject::tr("Change node type in segment"),
                                                 QObject::tr("SWC type: "
                                                             "\n 0 -- undefined (white)"
                                                             "\n 1 -- soma (black)"
                                                             "\n 2 -- axon (red)"
                                                             "\n 3 -- dendrite (blue)"
                                                             "\n 4 -- apical dendrite (purple)"
                                                             "\n else(<21) -- custom \n"),
                                                 currentTraceType, 0, 100, 1, &ok);
        }

        if(!ok) return;
        currentTraceType = node_type;

        const int neuron_type_color[ ][3] = {///////////////////////////////////////////////////////
                {255, 255, 255},  // white,   0-undefined
                {20,  20,  20 },  // black,   1-soma
                {200, 20,  0  },  // red,     2-axon
                {0,   20,  200},  // blue,    3-dendrite
                {200, 0,   200},  // purple,  4-apical dendrite
                //the following is Hanchuan's extended color. 090331
                {0,   200, 200},  // cyan,    5
                {220, 200, 0  },  // yellow,  6
                {0,   200, 20 },  // green,   7
                {188, 94,  37 },  // coffee,  8
                {180, 200, 120},  // asparagus,	9
                {250, 100, 120},  // salmon,	10
                {120, 200, 200},  // ice,		11
                {100, 120, 200},  // orchid,	12
            //the following is Hanchuan's further extended color. 111003
            {255, 128, 168},  //	13
            {128, 255, 168},  //	14
            {128, 168, 255},  //	15
            {168, 255, 128},  //	16
            {255, 168, 128},  //	17
            {168, 128, 255}, //	18
            {0, 0, 0}, //19 //totally black. PHC, 2012-02-15
            //the following (20-275) is used for matlab heat map. 120209 by WYN
            {0,0,131}, //20
                };
        currentMarkerColor.r=neuron_type_color[node_type][0];
        currentMarkerColor.g=neuron_type_color[node_type][1];
        currentMarkerColor.b=neuron_type_color[node_type][2];

    }else if(neuronColorMode == 5)
    {
        int inputlevel = QInputDialog::getInt(0, QObject::tr("Change node confidence level in segment"),
                                              QObject::tr("Confidence level: "
                                                          "\n 0 -- confidence"
                                                          "\n 1 -- uncertainty"
                                                          "\n 2 -- error"),
                                              0, 0, 2, 1, &ok);
        if(!ok) return;
        switch (inputlevel)
        {
        case 0:  node_level = 20;break;
        case 1:  node_level = 150;break;
        case 2:  node_level = 275;break;
        }

    }else
        return;
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

    My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
    XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

    //v3d_msg(QString("getNumShiftHolding() = ") + QString(w->getNumShiftHolding() ? "YES" : "no"));
    const float tolerance_squared = 100; // tolerance distance from the backprojected neuron to the curve point (squared for faster dist computation)

    // contour 2 polygon
    QPolygon poly;
    for (V3DLONG i=0; i<list_listCurvePos.at(0).size(); i++)
        poly.append(QPoint(list_listCurvePos.at(0).at(i).x, list_listCurvePos.at(0).at(i).y));

    // back-project the node curve points and mark segments to be deleted
    qDebug()<<"==============test flag=====================";

    qDebug()<<"listNeuronTree.size()"<<listNeuronTree.size();
    for(V3DLONG j=0; j<listNeuronTree.size(); j++)
    {
        NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(j))); //curEditingNeuron-1
        if (p_tree
                /*&& p_tree->editable*/)    // @FIXED by Alessandro on 2015-05-23. Removing segments from non-editable neurons causes crash.
        {
            QList <NeuronSWC> *p_listneuron = &(p_tree->listNeuron);
            if (!p_listneuron)
                continue;
            bool allUnitsOutsideZCut = false;
            QList<V3DLONG> idlist;
            for (V3DLONG i=0;i<p_listneuron->size();i++)
            {

                GLdouble px, py, pz, ix, iy, iz;
                ix = p_listneuron->at(i).x;
                iy = p_listneuron->at(i).y;
                iz = p_listneuron->at(i).z;
                allUnitsOutsideZCut = ! ((((float) iz) >=  this->swcBB.z0)&&( ((float) iz) <=  this->swcBB.z1));

                if(gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
                {

                    py = viewport[3]-py; //the Y axis is reversed
                    QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));
                    if(contour_mode)
                    {
                        if(   (poly.boundingRect().contains(p) && pointInPolygon(p.x(), p.y(), poly)) && !allUnitsOutsideZCut)
                        {
                            if(neuronColorMode==0)
                            {
                                change_type_in_seg_of_V_NeuronSWC_list(curImg->tracedNeuron, p_listneuron->at(i).seg_id, node_type);

                                if(idlist.indexOf(p_listneuron->at(i).seg_id)==-1)
                                {
                                    idlist.push_back(p_listneuron->at(i).seg_id);
                                }
                            }
                            else
                                change_level_in_seg_of_V_NeuronSWC_list(curImg->tracedNeuron, p_listneuron->at(i).seg_id, node_level);

                        }
                    }
                    else
                    {
                        bool changedTyp=false;
                        for (V3DLONG k=0; k<list_listCurvePos.at(0).size(); k++)
                        {

                            QPointF p2(list_listCurvePos.at(0).at(k).x, list_listCurvePos.at(0).at(k).y);
                            if(  ( (p.x()-p2.x())*(p.x()-p2.x()) + (p.y()-p2.y())*(p.y()-p2.y()) <= tolerance_squared  )  && !allUnitsOutsideZCut)
                            {
//                                cout<<"hei hei hei k="<<k;
                                if(neuronColorMode==0)
                                {
                                    if(node_mode)
                                    {
//                                        cout<<" node_mode\n";
                                        GLdouble spx, spy, spz;
                                        vector <V_NeuronSWC_unit> & row = (curImg->tracedNeuron.seg[p_listneuron->at(i).seg_id].row);
                                        int best_dist;
                                        int best_id;
                                        for (V3DLONG j=0;j<row.size();j++)
                                        {

                                            gluProject(row[j].x, row[j].y, row[j].z, markerViewMatrix, projectionMatrix, viewport, &spx, &spy, &spz);
                                            spy =  viewport[3]-spy;

                                            double dist = (spx-p2.x())*(spx-p2.x()) + (spy-p2.y())*(spy-p2.y());
                                            if(j==0)
                                            {
                                                best_dist = dist;
                                                best_id = 0;
                                            }
                                            else if(best_dist>dist)
                                            {
                                                best_dist = dist;
                                                best_id = j;
                                            }
                                        }
                                        row[best_id].type = node_type;
                                    }
                                    else
                                    {
//                                        cout<<" !node_mode================\n";
                                        qDebug()<<change_type_in_seg_of_V_NeuronSWC_list(curImg->tracedNeuron, p_listneuron->at(i).seg_id, node_type);

                                        if(idlist.indexOf(p_listneuron->at(i).seg_id)==-1)
                                        {
                                            idlist.push_back(p_listneuron->at(i).seg_id);
                                        }
                                    }
                                }
                                else
                                {
                                    change_level_in_seg_of_V_NeuronSWC_list(curImg->tracedNeuron, p_listneuron->at(i).seg_id, node_level);
                                }
                                break;   // found intersection with neuron segment: no more need to continue on this inner loop
                            }
                        }
                    }
                }
            }

            if(w->TeraflyCommunicator)
            {
                for(int cnt=0;cnt<idlist.size();cnt++)
                    if (!(idlist.at(cnt)<0 || idlist.at(cnt)>= curImg->tracedNeuron.seg.size()))
                    {
                        if(w->TeraflyCommunicator
                        &&w->TeraflyCommunicator->socket->state()==QAbstractSocket::ConnectedState)
                        {
                            w->SetupCollaborateInfo();
                            w->TeraflyCommunicator->UpdateRetypeSegMsg(curImg->tracedNeuron.seg[idlist.at(cnt)],currentTraceType,"TeraFly");
                        }
                    }
            }


            curImg->update_3drenderer_neuron_view(w, this);
            QHash<QString, int>  soma_cnt;
;           curImg->proj_trace_history_append();
            for (V3DLONG i=0;i<p_listneuron->size();i++)
            {
                if(p_listneuron->at(i).type == 1)
                {
                    QString soma_str = QString("(%1,%2,%3)").arg(p_listneuron->at(i).x).arg(p_listneuron->at(i).y).arg(p_listneuron->at(i).z);
                    soma_cnt[soma_str]++;
                }
            }
            if(soma_cnt.size()>1) v3d_msg(QString("%1 nodes have been typed as soma (type = 1). Please double check!").arg(soma_cnt.size()));
        }
    }
}

void Renderer_gl1::breakMultiNeuronsByStrokeCommit()
{
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

    My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
    curImg->tracedNeuron.decompose();
    curImg->update_3drenderer_neuron_view(w, this);
}


void Renderer_gl1::breakMultiNeuronsByStroke()
{
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

    My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
    XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

    const float tolerance_squared = 10*10; // tolerance distance (squared for faster dist computation) from the backprojected neuron to the curve point

    // contour 2 polygon
    QPolygon poly;
    for (V3DLONG i=0; i<list_listCurvePos.at(0).size(); i++)
        poly.append(QPoint(list_listCurvePos.at(0).at(i).x, list_listCurvePos.at(0).at(i).y));
    qDebug()<<"breakMultiNeuronsByStroke";
    // back-project the node curve points and mark segments to be deleted
    for(V3DLONG j=0; j<listNeuronTree.size(); j++)
    {
        NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(j))); //curEditingNeuron-1
        if (p_tree
            && p_tree->editable)    // @FIXED by Alessandro on 2015-05-23. Removing segments from non-editable neurons causes crash.
        {
            QList <NeuronSWC> *p_listneuron = &(p_tree->listNeuron);
            if (!p_listneuron)
                continue;
            bool allUnitsOutsideZCut = false;
            V3DLONG p_listneuron_num = p_listneuron->size();
            for (V3DLONG i=0;i<p_listneuron_num;i++)
            {
                GLdouble px, py, pz, ix, iy, iz;
                ix = p_listneuron->at(i).x;
                iy = p_listneuron->at(i).y;
                iz = p_listneuron->at(i).z;
                allUnitsOutsideZCut = ! ((((float) iz) >=  this->swcBB.z0)&&( ((float) iz) <=  this->swcBB.z1));


                if(gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
                {
                    py = viewport[3]-py; //the Y axis is reversed
                    QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));
                    for (V3DLONG k=0; k<list_listCurvePos.at(0).size(); k++)
                    {
                        QPointF p2(list_listCurvePos.at(0).at(k).x, list_listCurvePos.at(0).at(k).y);
						float dist2d_squared = (p.x()-p2.x())*(p.x()-p2.x()) + (p.y()-p2.y())*(p.y()-p2.y());
                        if(dist2d_squared <= tolerance_squared && !allUnitsOutsideZCut)
                        {
                            V_NeuronSWC seg=curImg->tracedNeuron.seg.at(p_listneuron->at(i).seg_id);

                            if(w->TeraflyCommunicator
                                &&w->TeraflyCommunicator->socket->state()==QAbstractSocket::ConnectedState)
                            {
                                w->SetupCollaborateInfo();
                                w->TeraflyCommunicator->UpdateSplitSegMsg(
                                    seg,p_listneuron->at(i).nodeinseg_id,"TeraFly");
                            }
                            auto &tmp=curImg->tracedNeuron.seg[p_listneuron->at(i).seg_id];
                            for(auto &node:tmp.row){
                                node.type=currentTraceType;
                            }
                            curImg->tracedNeuron.split(p_listneuron->at(i).seg_id,p_listneuron->at(i).nodeinseg_id);
                            qDebug()<<"split success "<<currentTraceType;
							curImg->update_3drenderer_neuron_view(w, this);
                            p_tree = (NeuronTree *)(&(listNeuronTree.at(j)));
                            p_listneuron = &(p_tree->listNeuron);
                            break;   // found intersection with neuron segment: no more need to continue on this inner loop
                        }
                    }

                }
            } // for listneuron,size
            curImg->proj_trace_history_append();
        } // if editable
    } // for listneurontree.size
}

void Renderer_gl1::breakTwoNeuronsByStroke() //(bool forceSingleCut)
{
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

    My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
    XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

    const float tolerance_squared = 100; // tolerance distance (squared for faster dist computation) from the backprojected neuron to the curve point
	float bestCutDist_squared = -1.0f; // for single cut, track closest cut dist squared
	V3DLONG bestCutTreeIndx = -1; // for single cut, track closest neuron tree index
	V3DLONG bestCutNeurIndx = -1; // for single cut, track closest index within neuron tree

    // contour 2 polygon
    QPolygon poly;
    for (V3DLONG i=0; i<list_listCurvePos.at(0).size(); i++)
        poly.append(QPoint(list_listCurvePos.at(0).at(i).x, list_listCurvePos.at(0).at(i).y));

    // back-project the node curve points and mark segments to be deleted
    for(V3DLONG j=0; j<listNeuronTree.size(); j++)
    {
        NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(j))); //curEditingNeuron-1
        if (p_tree
            && p_tree->editable)    // @FIXED by Alessandro on 2015-05-23. Removing segments from non-editable neurons causes crash.
        {
            QList <NeuronSWC> *p_listneuron = &(p_tree->listNeuron);
            if (!p_listneuron)
                continue;
            V3DLONG p_listneuron_num = p_listneuron->size();
            for (V3DLONG i=0;i<p_listneuron_num;i++)
            {
                GLdouble px, py, pz, ix, iy, iz;
                ix = p_listneuron->at(i).x;
                iy = p_listneuron->at(i).y;
                iz = p_listneuron->at(i).z;
                if(gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz))
                {
                    py = viewport[3]-py; //the Y axis is reversed
                    QPoint p(static_cast<int>(round(px)), static_cast<int>(round(py)));
                    for (V3DLONG k=0; k<list_listCurvePos.at(0).size(); k++)
                    {
                        QPointF p2(list_listCurvePos.at(0).at(k).x, list_listCurvePos.at(0).at(k).y);
						float dist2d_squared = (p.x()-p2.x())*(p.x()-p2.x()) + (p.y()-p2.y())*(p.y()-p2.y());
                        if(dist2d_squared <= tolerance_squared)
                       //     && curImg->tracedNeuron.seg[p_listneuron->at(i).seg_id].to_be_broken == false)
                        {
                           // curImg->tracedNeuron.seg[p_listneuron->at(i).seg_id].to_be_broken = true;
                           // curImg->tracedNeuron.seg[p_listneuron->at(i).seg_id].row[p_listneuron->at(i).nodeinseg_id].parent = -1;
//<<<<<<< HEAD
                           // if (forceSingleCut) {
                                if (bestCutDist_squared < 0 || dist2d_squared < bestCutDist_squared)
                                {
                                    bestCutDist_squared = dist2d_squared;
                                    bestCutTreeIndx = j;
                                    bestCutNeurIndx = i;
                                }
//                            }
//                            else
//                            {
//                                curImg->tracedNeuron.split(p_listneuron->at(i).seg_id,p_listneuron->at(i).nodeinseg_id);
//                                curImg->update_3drenderer_neuron_view(w, this);
//
//                                break;   // found intersection with neuron segment: no more need to continue on this inner loop
//                            }
                        }
                    }

                }
            } // for listneuron,size
           // curImg->proj_trace_history_append(); // TDP 20160127: This seems to be redundant, it is adding to the undo/redo stack BEFORE splitting which isn't useful
        } // if editable
    } // for listnerontree.size
    if (//forceSingleCut &&
    		bestCutDist_squared > 0)
    {
        NeuronTree *p_tree_to_split = (NeuronTree *)(&(listNeuronTree.at(bestCutTreeIndx)));
        QList <NeuronSWC> *p_listneuron_to_split = &(p_tree_to_split->listNeuron);
        curImg->tracedNeuron.split(p_listneuron_to_split->at(bestCutNeurIndx).seg_id,p_listneuron_to_split->at(bestCutNeurIndx).nodeinseg_id);
        curImg->update_3drenderer_neuron_view(w, this);
        curImg->proj_trace_history_append();
    }
}


// func of converting kernel
template <class Tpre, class Tpost>
void converting_to_8bit(void *pre1d, Tpost *pPost, V3DLONG imsz)
{
     if (!pre1d ||!pPost || imsz<=0 )
     {
          v3d_msg("Invalid parameters to converting_to_8bit().", 0);
          return;
     }

	Tpre *pPre = (Tpre *)pre1d;

     Tpre max_v=0, min_v = 255;

     for(V3DLONG i=0; i<imsz; i++)
     {
          if(max_v<pPre[i]) max_v = pPre[i];
          if(min_v>pPre[i]) min_v = pPre[i];
     }
     max_v -= min_v;

     if(max_v>0)
     {
          for(V3DLONG i=0; i<imsz; i++)
          {
               pPost[i] = (Tpost) 255*(double)(pPre[i] - min_v)/max_v;
          }
     }
     else
     {
          for(V3DLONG i=0; i<imsz; i++)
          {
               pPost[i] = (Tpost) pPre[i];
          }
     }

}

