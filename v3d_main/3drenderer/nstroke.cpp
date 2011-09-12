
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
     loc_vec_input.clear();
     // get the ith curve center
     solveCurveCenterV2(loc_vec_input, loc_veci, 0);

     // get the control points of the primary curve
     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if (w)
          curImg = v3dr_getImage4d(_idep);

     //int seg_id = curImg->tracedNeuron.seg.size()-1; // last seg
     V_NeuronSWC& primary_seg = curImg->tracedNeuron.seg[last_seg_id];
     vector <XYZ> loc_vec0;
     loc_vec0.clear();

     int N0 = primary_seg.row.size();
     for (int k=0; k<N0; k++)
     {
          XYZ cpt;
          cpt.x = primary_seg.row.at(k).data[2];
          cpt.y = primary_seg.row.at(k).data[3];
          cpt.z = primary_seg.row.at(k).data[4];
          loc_vec0.push_back(cpt);
     }

     // loc_vec0 is the previous curve control point vector
     //int N0 = loc_vec0.size();
     int NI=loc_veci.size();

     // two end points of ith curve
     XYZ posia = loc_veci.at(0);
     XYZ posib = loc_veci.at(NI-1);

     // decide whether to join the curve or refine the curve
     // find the two closest end points on two curves
     XYZ end_pos0, end_posi;
     XYZ pos0a = loc_vec0.at(0);
     XYZ pos0b = loc_vec0.at(N0-1);

     float dista, distb, distf;
     float d1, d2;
     XYZ end_posia, end_posib;
     d1 = dist_L2(pos0a, posia);
     d2 = dist_L2(pos0a, posib);
     if(d1<d2)
     {
          end_posia = posia;
          dista = d1;
     }else
     {
          end_posia = posib;
          dista = d2;
     }

     d1 = dist_L2(pos0b, posia);
     d2 = dist_L2(pos0b, posib);
     if(d1<d2)
     {
          end_posib = posia;
          distb = d1;
     }else
     {
          end_posib = posib;
          distb = d2;
     }
     // final two closest two end points and dist
     if(dista<distb)
     {
          end_pos0 = pos0a;
          end_posi = end_posia;
          distf = dista;
     }else
     {
          end_pos0 = pos0b;
          end_posi = end_posib;
          distf = distb;
     }

     // get two vectors for direction comparision
     // they are inner 3-step points (assuming curve points > 3)
     XYZ in_pt0, in_pti;
     if(end_pos0 == pos0a)
          in_pt0 = loc_vec0.at(2);
     else
          in_pt0 = loc_vec0.at(N0-3);

     if(end_posi == posia)
          in_pti = loc_veci.at(2);
     else
          in_pti = loc_veci.at(NI-3);

     XYZ v0 = in_pt0-end_pos0;
     XYZ vi = in_pti-end_posi;
     float vdot = dot(v0,vi);

     // join two curves
     if((vdot < 0)&&(distf < 50))
     {
          // update primary_seg.row
          V_NeuronSWC_unit nu;

          // there are four cases of joins
          // 1. insert loc_veci after the last point of loc_vec0
          if ( (end_pos0==pos0b)&&(end_posi==posia) )
          {
               double type = primary_seg.row.at(N0-1).type;
               double rr = primary_seg.row.at(N0-1).r;
               double nn = primary_seg.row.at(N0-1).n;
               for(int j=0; j<NI; j++)
               {
                    nu.x = loc_veci.at(j).x;
                    nu.y = loc_veci.at(j).y;
                    nu.z = loc_veci.at(j).z;
                    nu.n = nn+1+j;
                    nu.type = type;
                    nu.r = rr;
                    nu.parent =  nn+j;
                    nu.seg_id = last_seg_id;
                    nu.nodeinseg_id = N0+j;
                    if(j==NI-1) nu.nchild = 0;
                    else nu.nchild = 1;

                    primary_seg.row.push_back(nu);
               }
          }
          // 2. insert inversed loc_veci after the last point of loc_vec0
          else if ( (end_pos0==pos0b)&&(end_posi==posib) )
          {
               double type = primary_seg.row.at(N0-1).type;
               double rr = primary_seg.row.at(N0-1).r;
               double nn = primary_seg.row.at(N0-1).n;
               for(int j=0; j<NI; j++)
               {
                    nu.x = loc_veci.at(NI-1-j).x;
                    nu.y = loc_veci.at(NI-1-j).y;
                    nu.z = loc_veci.at(NI-1-j).z;
                    nu.n = nn+1+j;
                    nu.type = type;
                    nu.r = rr;
                    nu.parent = nn+j;
                    nu.seg_id = last_seg_id;
                    nu.nodeinseg_id = N0+j;
                    nu.nchild = (j==NI-1)?  0 : 1;

                    primary_seg.row.push_back(nu);
               }
          }
          // 3. insert inversed loc_veci before the first point of loc_vec0
          else if ( (end_pos0==pos0a)&&(end_posi==posia) )
          {
               double type = primary_seg.row.at(0).type;
               double rr = primary_seg.row.at(0).r;

               for(int j=0; j<NI; j++)
               {
                    nu.x = loc_veci.at(NI-1-j).x;
                    nu.y = loc_veci.at(NI-1-j).y;
                    nu.z = loc_veci.at(NI-1-j).z;
                    nu.n = 1+j;
                    nu.type = type;
                    nu.r = rr;
                    nu.parent = (j==0)? -1 : j;
                    nu.seg_id = last_seg_id;
                    nu.nodeinseg_id = j;
                    nu.nchild = 1;

                    primary_seg.row.insert(primary_seg.row.begin()+j, nu);
               }
               // update original primary_seg's info, e.g. parent, n, nodeinseg_id
               for(int i=0; i<N0; i++)
               {
                    int ind = i+NI;
                    primary_seg.row.at(ind).n = primary_seg.row.at(ind).n + NI;
                    primary_seg.row.at(ind).parent = primary_seg.row.at(ind).parent + NI;
                    primary_seg.row.at(ind).nodeinseg_id = primary_seg.row.at(ind).nodeinseg_id + NI;
               }
          }
          // 4. insert loc_veci before the first point of loc_vec0
          else if ( (end_pos0==pos0a)&&(end_posi==posib) )
          {
               double type = primary_seg.row.at(0).type;
               double rr = primary_seg.row.at(0).r;

               for(int j=0; j<NI; j++)
               {
                    nu.x = loc_veci.at(j).x;
                    nu.y = loc_veci.at(j).y;
                    nu.z = loc_veci.at(j).z;
                    nu.n = 1+j;
                    nu.type = type;
                    nu.r = rr;
                    nu.parent = (j==0)? -1 : j;
                    nu.seg_id = last_seg_id;
                    nu.nodeinseg_id = j;
                    nu.nchild = 1;

                    primary_seg.row.insert(primary_seg.row.begin()+j, nu);
               }
               // update original primary_seg's info, e.g. parent, n, nodeinseg_id
               for(int i=0; i<N0; i++)
               {
                    int ind = i+NI;
                    primary_seg.row.at(ind).n = primary_seg.row.at(ind).n + NI;
                    primary_seg.row.at(ind).parent = primary_seg.row.at(ind).parent + NI;
                    primary_seg.row.at(ind).nodeinseg_id = primary_seg.row.at(ind).nodeinseg_id + NI;
               }
          }

     }
     // begin to refine the primary (first) curve using the second curve
     else
     {
          // Find two points ka and kb on curve0 that are closest to the two end
          // points of ith curve. i.e. the ith curve is mapped onto curve0 between ka and kb.
          int ka, kb; // index of control points on curve0
          float closest_dista, closest_distb;

          for(int kk=0; kk<N0; kk++)
          {
               XYZ poskk = loc_vec0.at(kk);

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

          qDebug("ka and kb: ka=%d, kb=%d",ka, kb);
          qDebug("Size of loc_vec0: %d", N0);
          qDebug("Size of loc_veci: %d", NI);
          XYZ bpos=loc_vec0.at(N0-2);
          qDebug("Last pos in loc_vec0 before refining: (%d, %d, %d)", bpos.x, bpos.y, bpos.z);

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

                         XYZ bpos=loc_vec0.at(N0-2);
                         qDebug("Last pos in loc_vec0 %d th before refining: (%d, %d, %d)", k, bpos.x, bpos.y, bpos.z);

                         getPerpendPointDist(pos0, loc_veci.at(j1), loc_veci.at(jj), Pb1, dist1);
                         getPerpendPointDist(pos0, loc_veci.at(jj), loc_veci.at(j2), Pb2, dist2);

                         XYZ lpos=loc_vec0.at(N0-2);
                         qDebug("Last pos in loc_vec0 after %d th refining: (%d, %d, %d)", k, lpos.x, lpos.y, lpos.z);

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
               XYZ pt = loc_vec0.at(i);
               loc_vecsub.push_back(pt);
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

          qDebug("After refining.....");
          qDebug("kaa and kbb: kaa=%d, kbb=%d",kaa, kbb);
          qDebug("Size of loc_vec0: %d", loc_vec0.size());
          XYZ lpos=loc_vec0.at(N0-2);
          qDebug("Last pos in loc_vec0 after refining: (%d, %d, %d)", lpos.x, lpos.y, lpos.z);


          // temprarily delete for testing refine using pressing key "D"
          // V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
          // My4DImage* curImg = 0;
          // if (w)
          //      curImg = v3dr_getImage4d(_idep);

          // int seg_id = curImg->tracedNeuron.seg.size()-1; // last seg
          // V_NeuronSWC& last_segs = curImg->tracedNeuron.seg[seg_id];

          //for (int k=0; k<last_segs.row.size(); k++)
          for (int k=kaa; k<=kbb; k++)
          {
               primary_seg.row.at(k).data[2] = loc_vec0.at(k).x;
               primary_seg.row.at(k).data[3] = loc_vec0.at(k).y;
               primary_seg.row.at(k).data[4] = loc_vec0.at(k).z;
          }
     }// end of refine curve

     curImg->update_3drenderer_neuron_view(w, this);

     // used for testing curve refine
     //addCurveSWC(loc_veci, -1);
}


// press key "D" to apply curve refinement
// this function has been disabled. It is used for testing purpose.
void Renderer_gl1::applyCurveRefine()
{
     // update first curve
     // V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     // My4DImage* curImg = 0;
     // if (w)
     //      curImg = v3dr_getImage4d(_idep);

     // // last_seg_id is a global value, it is the curve0 to be updated
     // V_NeuronSWC& last_seg = curImg->tracedNeuron.seg[last_seg_id];
     // for (int k=0; k<last_seg.row.size(); k++)
     // {
     //      last_seg.row.at(k).data[2] = loc_vec0.at(k).x;
     //      last_seg.row.at(k).data[3] = loc_vec0.at(k).y;
     //      last_seg.row.at(k).data[4] = loc_vec0.at(k).z;
     // }

     // // get the second curve seg id
     // int seg_id = curImg->tracedNeuron.seg.size()-1;

     // // delete the second curve
     // if(seg_id!=last_seg_id && seg_id>=0)
     // {
     //      bool res = curImg->tracedNeuron.deleteSeg(seg_id);
     //      if (res)  curImg->proj_trace_history_append();
     // }

	// curImg->update_3drenderer_neuron_view(w, this);
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

          // save the this last_seg_id and used in solveCurveRefineLast()
          // for testing refine
          V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
          My4DImage* curImg = 0;
          if (w)
               curImg = v3dr_getImage4d(_idep);

          last_seg_id = curImg->tracedNeuron.seg.size()-1;
     }
}

// used in v3dr_glwidget.cpp for Qt::Key_A event
bool Renderer_gl1::isCurveRefineMode()
{
     if(selectMode == smCurveRefineLast)
          return true;
     else
          return false;
}
