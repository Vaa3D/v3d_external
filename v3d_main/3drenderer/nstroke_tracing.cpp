
/**
 * @file nstroke_tracing.cpp
 * @brief n-right-strokes curve drawing (refine)
 * This curve drawing method performs as follows:
 *  1. The user first draws the primary curve using right-mouse moving. This drawing
 *     is based on the method used in solveCurveCenter()(modified version).
 *  2. The user then draws (by right-mouse moving) the modifying-curve. This modifying curve
 *     is also got based on the method used in solveCurveCenter(). This curve represents
 *      the position that part of the primary curve should be.
 *  3. The curve refinement is then performed to get the refined curve.
 *  4. This refine process can be performed n times on the primary curve.
 *  5. The curve can also be extended on both ends by drawing extensions close to both ends.
 *  6. Press "Esc" to exit the curve refinement operation.
 *
 * extend/refine nearest neuron segment:
 *  When right-clicking on/near an existing curve, a menu is popupped.
 *  A menu item of "extend/refine nearest neuron segment" is added to allow
 *  users edit existing curves using same operations as in "n-right-strokes
 *  curve drawing (refine)".
 *
 * @author: Jianlong Zhou
 * @date: Jan 24, 2012
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
#include "../imaging/v3d_imaging.h"
#include "../basic_c_fun/v3d_curvetracepara.h"

#endif //test_main_cpp
#define SIGNAL_THRESHOLD 3

void Renderer_gl1::solveCurveTracing(vector <XYZ> & loc_vec_input, vector <XYZ> &loc_vec, int index)
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
	qDebug()<<"\n solveCurveTracing: 3d curve in channel # "<<((chno<0)? chno :chno+1);

	loc_vec.clear();

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
                    XYZ lastpos;
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

                    if(last_j<0) //first loc
                         loc = getLocUsingMassCenter(true, lastpos, loc0, loc1, clipplane, chno);
                    else
                         loc = getLocUsingMassCenter(false, lastpos, loc0, loc1, clipplane, chno);

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
				V3DLONG n_id_start = findNearestNeuronNode_WinXY(list_listCurvePos.at(index).at(0).x, list_listCurvePos.at(index).at(0).y, p_tree);
				V3DLONG n_id_end = findNearestNeuronNode_WinXY(list_listCurvePos.at(index).at(N-1).x, list_listCurvePos.at(index).at(N-1).y, p_tree);
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
     if (b_addthiscurve && selectMode==smCurveTracing)
     {
          addCurveSWC(loc_vec, chno);
          // for curve connection
          V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
          My4DImage* curImg = 0;
          if(w) curImg = v3dr_getImage4d(_idep);

          V3DLONG N = curImg->tracedNeuron.seg.size();

          MainWindow* V3Dmainwindow = 0;
          V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);
          bool bConnectCurve = (V3Dmainwindow && V3Dmainwindow->global_setting.b_3dcurve_autoconnecttips);
          if(bConnectCurve && N>1)
          {
               // the added curve is the last curve
               V3DLONG segid = N-1;
               connectCurve(segid);
               bConnectCurve = false;
          }
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
