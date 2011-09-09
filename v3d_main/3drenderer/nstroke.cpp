
#include "renderer_gl1.h"
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

#endif //test_main_cpp

// n-right-strokes curve drawing (refine), ZJL 20110826
// This curve drawing method performs as follows:
// 1. The user first draws the primary curve using right-mouse moving. This drawing
// is based on the method used in solveCurveCenter()(modified version).
// 2. The user then draws (by right-mouse moving) the modifying-curve. This modifying curve
// is also got based on the method used in solveCurveCenter(). This curve represents
// the position that part of the primary curve should be.
// 3. The curve refinement is then performed to get the refined curve.
// 4. This refine process can be performed n times on the primary curve.
// 5. Press "Esc" to exit the curve refinement operation.
void Renderer_gl1::solveCurveRefineLast()
{
	qDebug("Renderer_gl1::solveCurveRefineLast");

	int NC = list_listCurvePos.size();
	int N = list_listCurvePos.at(0).size();

	if (NC<1 || N <3)  return; //data is not enough

     vector <XYZ> loc_veci; // ith curve center
     vector <XYZ> loc_vec_input;

     loc_veci.clear();
     // get the ith curve center
     solveCurveCenterV2(loc_vec_input, loc_veci, 0);

     // loc_vec0 is the previous curve control point vector
     // it is a global value
     int N0 = loc_vec0.size();
     int NI=loc_veci.size();

     // Find two points ka and kb on curve0 that are closest to the two end
     // points of ith curve. i.e. the ith curve is mapped onto curve0 between ka and kb.
     int ka, kb; // index of control points on curve0
     float closest_dista, closest_distb;

     // two end points of ith curve
     XYZ posia = loc_veci.at(0);
     XYZ posib = loc_veci.at(NI-1);
     for(int kk=0; kk<N0; kk++)
     {
          XYZ poskk = loc_vec0.at(kk);
          // dista, distb
          float dista, distb;
          dista = dist_L2(poskk, posia);
          distb = dist_L2(poskk, posib);
          if(kk==0 || dista<closest_dista)
          {
               closest_dista=dista;
               ka = kk;
          }
          if(kk==0 || distb<closest_distb)
          {
               closest_distb=distb;
               kb = kk;
          }
     }

     // to ensure ka<=kb
     if(ka>kb)
     {
          int temp = ka;
          ka = kb;
          kb = temp;
     }

     // Now do curve refinement between ka and kb on curve0
     //for (int k=0; k<N0; k++)
     for (int k=ka; k<=kb; k++)
     {
          XYZ pos0 = loc_vec0.at(k);

          // get the closest control point on ith curve to kth point on curve0:
          float closest_dist;
          XYZ closest_pos;
          int jj; // closest pos index
          int j1; // pos index before the closest pos
          int j2; // pos index after the closet pos

          for (int j=0; j<NI; j++)
          {
               XYZ pos = loc_veci.at(j);
               // distance computation
               float dist = dist_L2(pos0, pos);
               if (j==0 || dist < closest_dist)
               {
                    closest_pos = pos;
                    closest_dist = dist;
                    jj = j;
                    j1 = ((jj-1)<=0)? 0 : (jj-1);
                    j2 = ((jj+1)>=NI)? (NI-1) : (jj+1);
               }
          }

          // qDebug("Closest dist at %d is %d", k, closest_dist);
          // if the pos on ith curve is too far (e.g. >10), ignore it and process next point
          if (closest_dist > 20.0)
          {
               // process next point of loc_vec0,k++
               continue;
          }else
          {
               // further processing to get correct closest_pos
               if ((jj==0)||(jj==NI-1))
               {
                    closest_pos = loc_veci.at(jj);
               } else
               {
                    // compare perpendicular distance between kth point on curve0 and
                    // neighboring points of jj on ith curve
                    // dist from pos0 to j1-jj, and perpendicular point
                    double dist1;
                    XYZ Pb1;
                    // dist from pos0 to jj-j2, and perpendicular point
                    double dist2;
                    XYZ Pb2;
                    getPerpendPointDist(pos0, loc_veci.at(j1), loc_veci.at(jj), Pb1, dist1);
                    getPerpendPointDist(pos0, loc_veci.at(jj), loc_veci.at(j2), Pb2, dist2);

                    if (dist1<=dist2)
                         // closest_pos from pos0 to jj-j1
                         closest_pos = Pb1;
                    else
                         // closest_pos from pos0 to j2-jj
                         closest_pos = Pb2;
               }

               // compare average intensity value of pos0 and closest_pos with windows size 5
               double mean0, mean;
               mean0 = getRgnPropertyAt(pos0);
               mean = getRgnPropertyAt(closest_pos);
               if (mean0 < mean)
               {
                    // replace loc_vec0.at(k) with closest_pos
                    loc_vec0.at(k) = closest_pos;
               }
          }
     }


     // only smooth the curve between ka and kb of loc_vec0
     vector <XYZ> loc_vecsub;
     loc_vecsub.clear();
     // copy data between ka-kb in loc_vec0 to loc_vecsub
     // make a larger window to smoothing
     int kaa, kbb;
     kaa = ((ka-2)<0)? 0:(ka-2);
     kbb = ((kb+2)>=N0)? (N0-1):(kb+2);
     for(int i=kaa; i<=kbb; i++)
     {
          int ii=i-kaa;
          loc_vecsub.push_back(loc_vec0.at(i));
     }
#ifndef test_main_cpp
	smooth_curve(loc_vecsub, 5);
#endif

     // copy data back from loc_vecsub to loc_vec0
     for(int i=kaa; i<=kbb; i++)
     {
          int ii=i-kaa;
          loc_vec0.at(i)=loc_vecsub.at(ii);
     }

     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if (w)
          curImg = v3dr_getImage4d(_idep);

     int last_seg_id = curImg->tracedNeuron.seg.size()-1;
     V_NeuronSWC& last_segs = curImg->tracedNeuron.seg[last_seg_id];

     for (int k=0; k<last_segs.row.size(); k++)
     {
          last_segs.row.at(k).data[2] = loc_vec0.at(k).x;
          last_segs.row.at(k).data[3] = loc_vec0.at(k).y;
          last_segs.row.at(k).data[4] = loc_vec0.at(k).z;
     }

     curImg->update_3drenderer_neuron_view(w, this);
}


// Get perpendicular point Pb from P to a line determined by two points of P0 and P1
// The computation algorithm is from (A Parametric Line):
// http://softsurfer.com/Archive/algorithm_0102/algorithm_0102.htm
//      P
//    / |  \
//   /  |    \
//  P0--Pb----P1
void Renderer_gl1::getPerpendPointDist(XYZ &P, XYZ &P0, XYZ &P1, XYZ &Pb, double &dist)
{
    XYZ V = P1 - P0;
    XYZ W = P - P0;

    double c1 = dot(W,V);
    double c2 = dot(V,V);

    //qDebug(" getPerpendPointDist: c1 = %d, c2 = %d", c1, c2);

    if(c2==0.0)
    {
         // This means that P0 and P1 are the same point. Usually this is not possible
         // because if this appears, P0 and P1 will be one point
         v3d_msg(" getPerpendPointDist: c2 = %d, cannot processing further!!!!", c2);
         return;
    }

    double b = c1/c2;
    XYZ Pbb = P0 + b * V;

    // dist is d(P, Pbb)
    dist = sqrt( dot(Pbb-P, Pbb-P) );

    // P is on the left of P0
    if ( c1 <= 0 )
    {
         Pb = P0;
         return;
    }
    // P is on the right of P1
    if ( c2 <= c1 )
    {
         Pb = P1;
         return;
    }
    // P is between P0 and P1
    Pb = Pbb;
}

// get mean value on pos with the window size 5
double Renderer_gl1::getRgnPropertyAt(XYZ &pos)
{
     LocationSimple pt;

	pt.x = pos.x;
	pt.y = pos.y;
	pt.z = pos.z;

     My4DImage* curImg = v3dr_getImage4d(_idep);
	V3DLONG cc = (V3DLONG) checkCurChannel();
     if (cc<0) cc=0;
     if (cc>=curImg->getCDim())
          cc=curImg->getCDim()-1;
	pt.radius = 5.0;
	pt.shape = pxCube;

	//now do the computation
	if (curImg->compute_rgn_stat(pt, cc)==true)
	{
          // average intensity value
          double mean = pt.ave;
          // mass value
          // double mass = pt.mass;
          return mean;
	}
     v3d_msg("Failed to get region properties.\n");
     return 0.0;
}


// This function is based on solveCurveCenter(). The differences in this version are:
// 1. add curve "int index" to get curve center for different curves. "index" is the curve index
// 2. return the created loc_vec vector for each index curve
// ZJL 110829
void Renderer_gl1::solveCurveCenterV2(vector <XYZ> & loc_vec_input, vector <XYZ> &loc_vec, int index)
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
	qDebug()<<"\n solveCurveCenterV2: 3d curve in channel # "<<((chno<0)? chno :chno+1);

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
     if (b_addthiscurve && selectMode==smCurveRefineInit)
     {
          addCurveSWC(loc_vec, chno);
     }
}
