
/**
 * @file nstroke.cpp
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
 * drag nearest neuron segment:
 *  users can drag any part of the curve by right clicking and moving it. The drag window
 *  size can be modified in real time by "CTRL+W" (increased by 2 each time) and "SHIFT+W"
 *  (decreased by 2 each time).
 *
 * @author: ZJL
 * @date: Oct. 3 2011
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

//ZMS 20151106: For neuron game
void Renderer_gl1::solveCurveExtendGlobal(){
    qDebug("Renderer_gl1::solveCurveRefineLast");

    int NC = list_listCurvePos.size();
    int N = list_listCurvePos.at(0).size();

    if (NC<1 || N <3)  return; //data is not enough

     vector <XYZ> secondary_vec; // ith curve center
     vector <XYZ> loc_vec_input;

     secondary_vec.clear();
     loc_vec_input.clear();

     // get the ith curve center
     if(refineMode==smCurveRefine_fm)
     {
          solveCurveMarkerLists_fm(loc_vec_input, secondary_vec, 0);
     }else
     {
          solveCurveCenterV2(loc_vec_input, secondary_vec, 0);
     }

     int NI=secondary_vec.size();

     // Because we use 5-neighbour of a point to process the refinement, we need to
     // make sure that NI is large enough to perform actions on loc_veci in this function.
     if(NI<6) return;

     // get the control points of the primary curve
     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if (w)
          curImg = v3dr_getImage4d(_idep);

     int last_seg_id;
     // for editing the nearest curve
     if(selectMode == smCurveEditRefine)
     {
          if(edit_seg_id<0) return;
          last_seg_id = edit_seg_id;
     }else
          last_seg_id = curImg->tracedNeuron.seg.size()-1; // last seg

     // two end points of ith curve
     XYZ secondary_a = secondary_vec.at(0);
     XYZ secondary_b = secondary_vec.at(NI-1);

     int minIndex = -1;
     double lowestDist = 100000000;
     XYZ primraryConnector, secondaryConnector;
     for(int i = 0; i < curImg->tracedNeuron.seg.size(); i++){
         V_NeuronSWC& possible_primary_seg = curImg->tracedNeuron.seg[i];

         // loc_vec0 is the primary (first) curve control point vector
         vector <XYZ> primrary_vec;
         primrary_vec.clear();

         int N0 = possible_primary_seg.row.size();
         for (int k=0; k<N0; k++)
         {
              XYZ cpt;
              cpt.x = possible_primary_seg.row.at(k).data[2];
              cpt.y = possible_primary_seg.row.at(k).data[3];
              cpt.z = possible_primary_seg.row.at(k).data[4];
              primrary_vec.push_back(cpt);
         }

         XYZ possible_primrary_a = primrary_vec.at(0);
         XYZ possible_primrary_b = primrary_vec.at(N0-1);

         float d;

         d = dist_L2(possible_primrary_a, secondary_a);
         if(d < lowestDist){
             lowestDist = d;
             primraryConnector = possible_primrary_a;
             secondaryConnector = secondary_a;
             minIndex = i;
         }

         d = dist_L2(possible_primrary_a, secondary_b);
         if(d < lowestDist){
             lowestDist = d;
             primraryConnector = possible_primrary_a;
             secondaryConnector = secondary_b;
             minIndex = i;
         }

         d = dist_L2(possible_primrary_b, secondary_a);
         if(d < lowestDist){
             lowestDist = d;
             primraryConnector = possible_primrary_b;
             secondaryConnector = secondary_a;
             minIndex = i;
         }

         d = dist_L2(possible_primrary_b, secondary_b);
         if(d < lowestDist){
             lowestDist = d;
             primraryConnector = possible_primrary_b;
             secondaryConnector = secondary_b;
             minIndex = i;
         }
     }

     if(minIndex < 0){return;} //Empty set of previous segments

     V_NeuronSWC& primary_seg = curImg->tracedNeuron.seg[minIndex];

     // loc_vec0 is the primary (first) curve control point vector
     vector <XYZ> primrary_vec;
     primrary_vec.clear();

     int N0 = primary_seg.row.size();
     for (int k=0; k<N0; k++)
     {
          XYZ cpt;
          cpt.x = primary_seg.row.at(k).data[2];
          cpt.y = primary_seg.row.at(k).data[3];
          cpt.z = primary_seg.row.at(k).data[4];
          primrary_vec.push_back(cpt);
     }

     XYZ primrary_a = primrary_vec.at(0);
     XYZ primrary_b = primrary_vec.at(N0-1);

     //Hack! Not sure if this is completely safe.
    last_seg_id = minIndex;

     if(true) //ZS:11052015 always join in this gamified mode for intuition.
     {
          // update primary_seg.row
          V_NeuronSWC_unit nu;
          V3DLONG last_n =curImg->tracedNeuron.maxnoden();

          qDebug("Extend curve!");

          // there are four cases of joins
          // 1. insert loc_veci after the last point of loc_vec0
          if ( (primraryConnector==primrary_b)&&(secondaryConnector==secondary_a) )
          {
               double type = primary_seg.row.at(N0-1).type;
               double nnp = primary_seg.row.at(N0-1).n;

               for(int j=0; j<NI; j++)
               {
                    nu.x = secondary_vec.at(j).x;
                    nu.y = secondary_vec.at(j).y;
                    nu.z = secondary_vec.at(j).z;
                    nu.n = last_n+1+j;
                    nu.type = type;
                    nu.r = 1;
                    nu.parent = (j==0)? (nnp) : (nu.n-1);
                    nu.seg_id = last_seg_id;
                    // nu.nodeinseg_id = N0+j;
                    // if(j==NI-1) nu.nchild = 0;
                    // else nu.nchild = 1;

                    primary_seg.append(nu);
               }

               // scanning the whole tracedNeuron to order index n
               reorderNeuronIndexNumber(last_seg_id, NI, false);
          }
          // 2. insert inversed loc_veci after the last point of loc_vec0
          else if ( (primraryConnector==primrary_b)&&(secondaryConnector==secondary_b) )
          {
               double type = primary_seg.row.at(N0-1).type;
               double nnp = primary_seg.row.at(N0-1).n;
               for(int j=0; j<NI; j++)
               {
                    nu.x = secondary_vec.at(NI-1-j).x;
                    nu.y = secondary_vec.at(NI-1-j).y;
                    nu.z = secondary_vec.at(NI-1-j).z;
                    nu.n = last_n+1+j;
                    nu.type = type;
                    nu.r = 1;
                    nu.parent = (j==0)? (nnp) : (nu.n-1);
                    nu.seg_id = last_seg_id;
                    // nu.nodeinseg_id = N0+j;
                    // nu.nchild = (j==NI-1)?  0 : 1;

                    primary_seg.append(nu);
               }

               // scanning the whole tracedNeuron to order index n
               reorderNeuronIndexNumber(last_seg_id, NI, false);
          }
          // 3. insert inversed loc_veci before the first point of loc_vec0
          else if ( (primraryConnector==primrary_a)&&(secondaryConnector==secondary_a) )
          {
               double type = primary_seg.row.at(0).type;

               for(int j=0; j<NI; j++)
               {
                    nu.x = secondary_vec.at(NI-1-j).x;
                    nu.y = secondary_vec.at(NI-1-j).y;
                    nu.z = secondary_vec.at(NI-1-j).z;
                    nu.n = last_n + 1+j;
                    nu.type = type;
                    nu.r = 1;
                    nu.parent = (j==0)? -1 : (nu.n-1);
                    nu.seg_id = last_seg_id;
                    // nu.nodeinseg_id = j;
                    // nu.nchild = 1;

                    primary_seg.row.insert(primary_seg.row.begin()+j, nu);
               }
               // update original primary_seg's info, e.g. parent
               primary_seg.row.at(NI).parent = last_n+NI; // the first node in the primary curvew

               // scanning the whole tracedNeuron to order index n
               reorderNeuronIndexNumber(last_seg_id, NI, true);
          }
          // 4. insert loc_veci before the first point of loc_vec0
          else if ( (primraryConnector==primrary_a)&&(secondaryConnector==secondary_b) )
          {
               double type = primary_seg.row.at(0).type;

               for(int j=0; j<NI; j++)
               {
                    nu.x = secondary_vec.at(j).x;
                    nu.y = secondary_vec.at(j).y;
                    nu.z = secondary_vec.at(j).z;
                    nu.n = last_n+1+j;
                    nu.type = type;
                    nu.r = 1;
                    nu.parent = (j==0)? -1 : (nu.n-1);
                    nu.seg_id = last_seg_id;
                    // nu.nodeinseg_id = j;
                    // nu.nchild = 1;

                    primary_seg.row.insert(primary_seg.row.begin()+j, nu);
               }
               // update original primary_seg's info, e.g. parent
               primary_seg.row.at(NI).parent = last_n+NI; // the first node in the primary curve

               // scanning the whole tracedNeuron to order index n
               reorderNeuronIndexNumber(last_seg_id, NI, true);
          }
          // for curve connection: if end points of last_seg_id are close
          // enough to a seg, connect them to their closest points.
          V3DLONG nums=curImg->tracedNeuron.seg.size();

          MainWindow* V3Dmainwindow = 0;
          V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);
          bool bConnectCurve = (V3Dmainwindow && V3Dmainwindow->global_setting.b_3dcurve_autoconnecttips);
          if(bConnectCurve && nums>1)
          {
               V3DLONG cursegid = last_seg_id;
               connectCurve(cursegid);
               bConnectCurve = false;
          }

          // decide radius of each point
          if (V3dApplication::getMainWindow()->global_setting.b_3dcurve_autowidth)
          {
               int chno = checkCurChannel();
               if (chno<0 || chno>dim4-1)   chno = 0; //default first channel
               CurveTracePara trace_para;
               {
                    trace_para.channo = (chno<0)?0:chno;
                    if (trace_para.channo>=curImg->getCDim())
                         trace_para.channo=curImg->getCDim()-1;
                    trace_para.landmark_id_start = -1;
                    trace_para.landmark_id_end = -1;
                    trace_para.sp_num_end_nodes = 2;
                    trace_para.nloops = 100; //100130 change from 200 to 100
                    trace_para.b_deformcurve = true;
                    trace_para.sp_smoothing_win_sz = 2;
               }

               if (chno >=0)
               {
                    curImg->proj_trace_compute_radius_of_last_traced_neuron(trace_para,
                         last_seg_id, last_seg_id, curImg->trace_z_thickness);
               }
          }// end update radius
     }// end of extend curve

     curImg->proj_trace_history_append();
     // update the neuron display
     curImg->update_3drenderer_neuron_view(w, this);
}

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
     if(refineMode==smCurveRefine_fm)
     {
          solveCurveMarkerLists_fm(loc_vec_input, loc_veci, 0);
     }else
     {
          solveCurveCenterV2(loc_vec_input, loc_veci, 0);
     }

     int NI=loc_veci.size();

     // Because we use 5-neighbour of a point to process the refinement, we need to
     // make sure that NI is large enough to perform actions on loc_veci in this function.
     if(NI<6) return;

     // get the control points of the primary curve
     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if (w)
          curImg = v3dr_getImage4d(_idep);

     int last_seg_id;
     // for editing the nearest curve
     if(selectMode == smCurveEditRefine)
     {
          if(edit_seg_id<0) return;
          last_seg_id = edit_seg_id;
     }else
          last_seg_id = curImg->tracedNeuron.seg.size()-1; // last seg

     V_NeuronSWC& primary_seg = curImg->tracedNeuron.seg[last_seg_id];

     // loc_vec0 is the primary (first) curve control point vector
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
     // they are inner 5-step points (assuming curve points > 3)
     XYZ in_pt0, in_pti;
     // if(end_pos0 == pos0a)
     //      in_pt0 = loc_vec0.at(4);
     // else
     //      in_pt0 = loc_vec0.at(N0-5);

     // if(end_posi == posia)
     //      in_pti = loc_veci.at(4);
     // else
     //      in_pti = loc_veci.at(NI-5);

     // We decide whether to extend or refine curve based on distance and angle of
     // part curve. in_pt0 is an inside point on loc_vec0 used to decide loc_vec0 direction.
     // in_pti is an inside point on loc_veci used to decide loc_veci direction.
     float vdot;
     XYZ v0, vi;

     //if(distf<40)
     {
          // get in_pt0 by computer dist incursively for loc_vec0
          if(end_pos0 == pos0a)
          {
               int i=1;
               float dis=0.0;
               while(dis<3*distf && i<N0)
               {
                    dis = dis+dist_L2(end_pos0, loc_vec0.at(i));
                    i++;
               }
               if(i>4 && i<N0) in_pt0=loc_vec0.at(i);
               else     in_pt0=loc_vec0.at(4);
          }else
          {
               int i=N0-2;
               float dis=0.0;
               while(dis<3*distf && i>0)
               {
                    dis = dis+dist_L2(end_pos0, loc_vec0.at(i));
                    i--;
               }
               if(i<N0-5 && i>0) in_pt0=loc_vec0.at(i);
               else    in_pt0=loc_vec0.at(N0-5);
          }
          // get in_pti by computer dist incursively for loc_veci
          if(end_posi == posia)
          {
               int i=1;
               float dis=0.0;
               while(dis<3*distf && i<NI)
               {
                    dis = dis+dist_L2(end_posi, loc_veci.at(i));
                    i++;
               }
               if(i>4 && i<NI) in_pti=loc_veci.at(i);
               else     in_pti=loc_veci.at(4);
          }else
          {
               int i=NI-2;
               float dis=0.0;
               while(dis<3*distf && i>0)
               {
                    dis = dis+dist_L2(end_posi, loc_veci.at(i));
                    i--;
               }
               if(i<NI-5 && i>0) in_pti=loc_veci.at(i);
               else    in_pti=loc_veci.at(NI-5);
          }

          v0 = in_pt0-end_pos0;
          vi = in_pti-end_posi;
          vdot = dot(v0,vi);
     }

     qDebug("vdot is: %f, distf is: %f",vdot, distf);
     // if vdot<0, the angle should be 90~270.
     // A. If the second curve is far from one end, discard it.
     // if(vdot<0 && distf>40)
     // {
     //      return;
     // }
     // B. join two curves
     if((vdot < 0)&&(distf <= 30))
     {
          // update primary_seg.row
          V_NeuronSWC_unit nu;
          V3DLONG last_n =curImg->tracedNeuron.maxnoden();

          qDebug("Extend curve!");

          // there are four cases of joins
          // 1. insert loc_veci after the last point of loc_vec0
          if ( (end_pos0==pos0b)&&(end_posi==posia) )
          {
               double type = primary_seg.row.at(N0-1).type;
               double nnp = primary_seg.row.at(N0-1).n;

               for(int j=0; j<NI; j++)
               {
                    nu.x = loc_veci.at(j).x;
                    nu.y = loc_veci.at(j).y;
                    nu.z = loc_veci.at(j).z;
                    nu.n = last_n+1+j;
                    nu.type = type;
                    nu.r = 1;
                    nu.parent = (j==0)? (nnp) : (nu.n-1);
                    nu.seg_id = last_seg_id;
                    // nu.nodeinseg_id = N0+j;
                    // if(j==NI-1) nu.nchild = 0;
                    // else nu.nchild = 1;

                    primary_seg.append(nu);
               }

               // scanning the whole tracedNeuron to order index n
               reorderNeuronIndexNumber(last_seg_id, NI, false);
          }
          // 2. insert inversed loc_veci after the last point of loc_vec0
          else if ( (end_pos0==pos0b)&&(end_posi==posib) )
          {
               double type = primary_seg.row.at(N0-1).type;
               double nnp = primary_seg.row.at(N0-1).n;
               for(int j=0; j<NI; j++)
               {
                    nu.x = loc_veci.at(NI-1-j).x;
                    nu.y = loc_veci.at(NI-1-j).y;
                    nu.z = loc_veci.at(NI-1-j).z;
                    nu.n = last_n+1+j;
                    nu.type = type;
                    nu.r = 1;
                    nu.parent = (j==0)? (nnp) : (nu.n-1);
                    nu.seg_id = last_seg_id;
                    // nu.nodeinseg_id = N0+j;
                    // nu.nchild = (j==NI-1)?  0 : 1;

                    primary_seg.append(nu);
               }

               // scanning the whole tracedNeuron to order index n
               reorderNeuronIndexNumber(last_seg_id, NI, false);
          }
          // 3. insert inversed loc_veci before the first point of loc_vec0
          else if ( (end_pos0==pos0a)&&(end_posi==posia) )
          {
               double type = primary_seg.row.at(0).type;

               for(int j=0; j<NI; j++)
               {
                    nu.x = loc_veci.at(NI-1-j).x;
                    nu.y = loc_veci.at(NI-1-j).y;
                    nu.z = loc_veci.at(NI-1-j).z;
                    nu.n = last_n + 1+j;
                    nu.type = type;
                    nu.r = 1;
                    nu.parent = (j==0)? -1 : (nu.n-1);
                    nu.seg_id = last_seg_id;
                    // nu.nodeinseg_id = j;
                    // nu.nchild = 1;

                    primary_seg.row.insert(primary_seg.row.begin()+j, nu);
               }
               // update original primary_seg's info, e.g. parent
               primary_seg.row.at(NI).parent = last_n+NI; // the first node in the primary curvew

               // scanning the whole tracedNeuron to order index n
               reorderNeuronIndexNumber(last_seg_id, NI, true);
          }
          // 4. insert loc_veci before the first point of loc_vec0
          else if ( (end_pos0==pos0a)&&(end_posi==posib) )
          {
               double type = primary_seg.row.at(0).type;

               for(int j=0; j<NI; j++)
               {
                    nu.x = loc_veci.at(j).x;
                    nu.y = loc_veci.at(j).y;
                    nu.z = loc_veci.at(j).z;
                    nu.n = last_n+1+j;
                    nu.type = type;
                    nu.r = 1;
                    nu.parent = (j==0)? -1 : (nu.n-1);
                    nu.seg_id = last_seg_id;
                    // nu.nodeinseg_id = j;
                    // nu.nchild = 1;

                    primary_seg.row.insert(primary_seg.row.begin()+j, nu);
               }
               // update original primary_seg's info, e.g. parent
               primary_seg.row.at(NI).parent = last_n+NI; // the first node in the primary curve

               // scanning the whole tracedNeuron to order index n
               reorderNeuronIndexNumber(last_seg_id, NI, true);
          }
          // for curve connection: if end points of last_seg_id are close
          // enough to a seg, connect them to their closest points.
          V3DLONG nums=curImg->tracedNeuron.seg.size();

          MainWindow* V3Dmainwindow = 0;
          V3Dmainwindow = v3dr_getV3Dmainwindow(_idep);
          bool bConnectCurve = (V3Dmainwindow && V3Dmainwindow->global_setting.b_3dcurve_autoconnecttips);
          if(bConnectCurve && nums>1)
          {
               V3DLONG cursegid = last_seg_id;
               connectCurve(cursegid);
               bConnectCurve = false;
          }

          // decide radius of each point
          if (V3dApplication::getMainWindow()->global_setting.b_3dcurve_autowidth)
          {
               int chno = checkCurChannel();
               if (chno<0 || chno>dim4-1)   chno = 0; //default first channel
               CurveTracePara trace_para;
               {
                    trace_para.channo = (chno<0)?0:chno;
                    if (trace_para.channo>=curImg->getCDim())
                         trace_para.channo=curImg->getCDim()-1;
                    trace_para.landmark_id_start = -1;
                    trace_para.landmark_id_end = -1;
                    trace_para.sp_num_end_nodes = 2;
                    trace_para.nloops = 100; //100130 change from 200 to 100
                    trace_para.b_deformcurve = true;
                    trace_para.sp_smoothing_win_sz = 2;
               }

               if (chno >=0)
               {
                    curImg->proj_trace_compute_radius_of_last_traced_neuron(trace_para,
                         last_seg_id, last_seg_id, curImg->trace_z_thickness);
               }
          }// end update radius
     }
     // C. begin to refine the primary (first) curve using the second curve
     //else
     if(vdot>=0)
     {
          qDebug("Refine curve!");
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

          // save the original position of refined points plus 2*wsize points.
          // used for connection checking of other segs to refined points.
          vector <XYZ> old_refined_loc;
          old_refined_loc.clear();
          int wsize =2;
          // make a larger window for smoothing
          int kaa, kbb;
          kaa = ((ka-wsize)<0)? 0:(ka-wsize); // considering wsize
          kbb = ((kb+wsize)>=N0)? (N0-1):(kb+wsize);

          for(int i=kaa; i<=kbb; i++)
          {
               XYZ pt;
               pt.x=loc_vec0.at(i).x;
               pt.y=loc_vec0.at(i).y;
               pt.z=loc_vec0.at(i).z;
               old_refined_loc.push_back(pt);
          } // end saving old refined points

          // Now do curve refinement between ka and kb on curve0
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
               // if (closest_dist > 20.0)
               // {
               //      // process next point of loc_vec0,k++
               //      continue;
               // }else
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
                    LocationSimple pt0, pt;
                    getRgnPropertyAt(pos0, pt0);
                    getRgnPropertyAt(closest_pos, pt);
                    mean0=pt0.ave; mean = pt.ave;
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
          // make a larger window (ka,kb+wsize) for smoothing
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

          for (int k=kaa; k<=kbb; k++)
          {
               primary_seg.row.at(k).data[2] = loc_vec0.at(k).x;
               primary_seg.row.at(k).data[3] = loc_vec0.at(k).y;
               primary_seg.row.at(k).data[4] = loc_vec0.at(k).z;
          }

          // Update endpoints of linked segments
          V3DLONG nsegs = curImg->tracedNeuron.nsegs();
          for(V3DLONG j=0; j<nsegs; j++)
          {
               if(j!=last_seg_id)
               {
                    V_NeuronSWC& segj = curImg->tracedNeuron.seg.at(j);
                    // get two end points of segj
                    XYZ pt1, pt2;
                    int nn = segj.row.size();
                    pt1.x=segj.row.at(0).x;
                    pt1.y=segj.row.at(0).y;
                    pt1.z=segj.row.at(0).z;

                    pt2.x=segj.row.at(nn-1).x;
                    pt2.y=segj.row.at(nn-1).y;
                    pt2.z=segj.row.at(nn-1).z;
                    // check distance between pt1/pt2 and old refined points
                    // if dist== 0, then update to connect
                    for(int i=kaa; i<=kbb; i++)
                    {
                         XYZ pt;
                         int ii = i-kaa;
                         pt=old_refined_loc.at(ii);
                         double dist;
                         dist = dist_L2(pt1, pt);
                         if(dist==0.0) //for pt1
                         {
                              // update loc of segj's end points to loc of refined pt
                              segj.row.at(0).data[2]= primary_seg.row.at(i).data[2];
                              segj.row.at(0).data[3]= primary_seg.row.at(i).data[3];
                              segj.row.at(0).data[4]= primary_seg.row.at(i).data[4];
                         }
                         dist = dist_L2(pt2, pt);
                         if(dist==0.0) //for pt2
                         {
                              // update loc of segj's end points to loc of refined pt
                              segj.row.at(nn-1).data[2]= primary_seg.row.at(i).data[2];
                              segj.row.at(nn-1).data[3]= primary_seg.row.at(i).data[3];
                              segj.row.at(nn-1).data[4]= primary_seg.row.at(i).data[4];
                         }
                    }

               }
          } // end update linking for refined points

          // update radius of each point
          if (V3dApplication::getMainWindow()->global_setting.b_3dcurve_autowidth)
          {
               int chno = checkCurChannel();
               if (chno<0 || chno>dim4-1)   chno = 0; //default first channel
               CurveTracePara trace_para;
               {
                    trace_para.channo = (chno<0)?0:chno;
                    if (trace_para.channo>=curImg->getCDim())
                         trace_para.channo=curImg->getCDim()-1;
                    trace_para.landmark_id_start = -1;
                    trace_para.landmark_id_end = -1;
                    trace_para.sp_num_end_nodes = 2;
                    trace_para.nloops = 100;
                    trace_para.b_deformcurve = true;
                    trace_para.sp_smoothing_win_sz = 2;
               }

               if (chno >=0)
               {
                    curImg->proj_trace_compute_radius_of_last_traced_neuron(trace_para,
                         last_seg_id, last_seg_id, curImg->trace_z_thickness);
               }
          }//end updating radius

     }// end of refine curve

     curImg->proj_trace_history_append();
     // update the neuron display
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
void Renderer_gl1::getRgnPropertyAt(XYZ &pos, LocationSimple &pt)
{
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
	if (curImg->compute_rgn_stat(pt, cc)!=true)
	{
          // average intensity value
          //mean = pt.ave;
          //sdev = pt.sdev;
          // mass value
          // double mass = pt.mass;
          v3d_msg("Failed to get region properties.\n");
          return ;
	}

     return ;
}

/**
 * @brief This function is based on solveCurveCenter(). The differences in this version are:
 *  1. add curve "int index" to get curve center for different curves. "index" is the curve index
 *  2. return the created loc_vec vector for each index curve
 *  ZJL 110829
*/
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
     if (b_addthiscurve && selectMode==smCurveRefineInit)
     {
          addCurveSWC(loc_vec, chno, 6);//LMG 26/10/2018 solveCurveCenterV2 mode 6
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
 * @brief scanning the whole tracedNeuron to order index n
 * variables from main function
 * @param curSeg_id Current seg_id
 * @param NI  Number of inserted points
 * @param  newInLower Whether the inserted points are at the lower number
 *  side of original curve
*/
void Renderer_gl1::reorderNeuronIndexNumber(V3DLONG curSeg_id, V3DLONG NI, bool newInLower)
{
     // get the control points of the primary curve
     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if (w)
          curImg = v3dr_getImage4d(_idep);

     V3DLONG nseg=curImg->tracedNeuron.nsegs();

     V_NeuronSWC& curSeg = curImg->tracedNeuron.seg[curSeg_id];
     // curSeg's info
     V3DLONG cnsu = curSeg.nrows(); // this already includes NI
     V3DLONG cn0, cnn; // min max n of curSeg before inserting NI points
     if(newInLower==true)
     {
          cn0 = curSeg.row.at(NI).n;
          cnn = curSeg.row.at( cnsu-1 ).n;
     }else
     {
          cn0 = curSeg.row.at(0).n;
          cnn = curSeg.row.at( cnsu-NI-1 ).n;
     }

     // before the curSeg
     // Update segs whose parents' n are after the curSeg n
     for(V3DLONG i=0; i<curSeg_id; i++)
     {
          V_NeuronSWC& segi = curImg->tracedNeuron.seg.at(i);
          V3DLONG num=segi.nrows();
          for(V3DLONG ii=0;ii<num; ii++)
          {
               V3DLONG p_old=segi.row.at(ii).parent;
               // we assume that pp is not inside the inserted points
               if(p_old>=0)
               {
                    if( (p_old >= cn0) && (p_old <= cnn)) // pp is on curSeg
                    {
                         if(newInLower==true)
                              segi.row.at(ii).parent = p_old+NI;
                         else
                              segi.row.at(ii).parent = p_old;
                    }
                    else if(p_old>cnn) // pp is after curSeg
                    {
                         segi.row.at(ii).parent = p_old+NI;
                    }
               }
          }
     }
     // the curSeg
     // n: last_n~last_n+NI, curSeg.at(0).n ~ curSeg.at(num).n
     // last_n does not include NI pts
     // change curSeg's n to (n0~nn+NI)
     for(V3DLONG j=0; j<cnsu; j++)
     {
          curSeg.row.at(j).n = cn0+j;
          curSeg.row.at(j).parent = (curSeg.row.at(j).parent == -1)? -1:(cn0+j-1);
     }

     // after the curSeg
     for(V3DLONG i=curSeg_id+1; i<nseg; i++)
     {
          V_NeuronSWC& segi = curImg->tracedNeuron.seg[i];
          V3DLONG num=segi.nrows();
          for(V3DLONG ii=0;ii<num; ii++)
          {
               V3DLONG pn = segi.row.at(ii).n; // old n
               V3DLONG p_old = segi.row.at(ii).parent; // old p
               segi.row.at(ii).n = pn+NI;
               if(p_old==-1)
                    segi.row.at(ii).parent = -1;
               else if(p_old<cn0) // p_old is before curSeg
                    segi.row.at(ii).parent = p_old;
               else if( (p_old >= cn0)&&(p_old <= cnn)) // p_old is on curSeg
               {
                    if(newInLower==true)
                         segi.row.at(ii).parent = p_old+NI;
                    else
                         segi.row.at(ii).parent = p_old;
               }
               else if(p_old>cnn) // p_old is after curSeg
               {
                    segi.row.at(ii).parent = p_old+NI;
               }
          }
     }
}


/**
 * @brief keyboard short-cuts for a number of drawing and editing functions
 * Alt+B: serial Bboxes curve drawing
 * Alt+T: multiple segments reTyping
 * Alt+D: multiple segments Deleting
 * Alt+S: multiple segments Spliting
 * Alt+C: multiple segments Connection
 * Alt+P: 3D polYline defining

*/
void Renderer_gl1::callStrokeCurveDrawingBBoxes()
{
    if(editinput == 3)
        deleteMultiNeuronsByStrokeCommit();

    selectMode = smCurveTiltedBB_fm_sbbox;
    b_addthiscurve = true;
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (w)
    {
        editinput = 1;
        oldCursor = QCursor(Qt::ArrowCursor);
        w->setCursor(QCursor(Qt::PointingHandCursor));
    }

}

void Renderer_gl1::callStrokeCurveDrawingGlobal()
{
    if(editinput == 3)
        deleteMultiNeuronsByStrokeCommit();

    selectMode = smCurveTiltedBB_fm;
    b_addthiscurve = true;
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (w)
    {
        editinput = 5;
        oldCursor = QCursor(Qt::ArrowCursor);
        w->setCursor(QCursor(Qt::PointingHandCursor));
    }

}

void Renderer_gl1::callStrokeRetypeMultiNeurons()
{
    if(editinput == 3)
        deleteMultiNeuronsByStrokeCommit();

    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (w && listNeuronTree.size()>0)
    {
        w->setEditMode();
        if(listNeuronTree.at(0).editable==true || listNeuronTree.at(listNeuronTree.size()-1).editable==true)
        {
            editinput = 2;
            selectMode = smRetypeMultiNeurons;
            b_addthiscurve = false;
            oldCursor = QCursor(Qt::ArrowCursor);
            w->setCursor(QCursor(Qt::PointingHandCursor));
        }
    }
}

void Renderer_gl1::callStrokeDeleteMultiNeurons()
{
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (w && listNeuronTree.size()>0)
    {
        w->setEditMode();
        if(listNeuronTree.at(0).editable==true || listNeuronTree.at(listNeuronTree.size()-1).editable==true)
        {
            editinput = 3;
            selectMode = smDeleteMultiNeurons;
            b_addthiscurve = false;
            oldCursor = QCursor(Qt::ArrowCursor);
            w->setCursor(QCursor(Qt::PointingHandCursor));
        }
    }
}

void Renderer_gl1::callStrokeSplitMultiNeurons()
{
    if(editinput == 3)
        deleteMultiNeuronsByStrokeCommit();

    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (w && listNeuronTree.size()>0)
    {
        w->setEditMode();
        if(listNeuronTree.at(0).editable==true || listNeuronTree.at(listNeuronTree.size()-1).editable==true)
        {
            editinput = 4;
            selectMode = smBreakMultiNeurons;
            b_addthiscurve = false;
            oldCursor = QCursor(Qt::ArrowCursor);
            w->setCursor(QCursor(Qt::PointingHandCursor));
        }
    }
}


void Renderer_gl1::callStrokeConnectMultiNeurons()
{
    if(editinput == 3)
        deleteMultiNeuronsByStrokeCommit();

    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (w && listNeuronTree.size()>0)
    {
        w->setEditMode();
        if(listNeuronTree.at(0).editable==true || listNeuronTree.at(listNeuronTree.size()-1).editable==true)
        {
            editinput = 6;
            selectMode = smSimpleConnect;
            b_addthiscurve = false;
            oldCursor = QCursor(Qt::ArrowCursor);
            w->setCursor(QCursor(Qt::PointingHandCursor));
        }
    }
}

void Renderer_gl1::callShowSubtree()
{
	if (editinput == 3) deleteMultiNeuronsByStrokeCommit();

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	if (w && listNeuronTree.size()>0)
	{
		w->setEditMode();
		if (listNeuronTree.at(0).editable == true || listNeuronTree.at(listNeuronTree.size() - 1).editable == true)
		{
			editinput = 10;
			selectMode = smShowSubtree;
			b_addthiscurve = false;
			oldCursor = QCursor(Qt::ArrowCursor);
			w->setCursor(QCursor(Qt::PointingHandCursor));
		}
	}
}

void Renderer_gl1::callShowConnectedSegs()
{
	if (editinput == 3) deleteMultiNeuronsByStrokeCommit();

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	if (w && listNeuronTree.size()>0)
	{
		w->setEditMode();
		if (listNeuronTree.at(0).editable == true || listNeuronTree.at(listNeuronTree.size() - 1).editable == true)
		{
			My4DImage* curImg = 0; if (w) curImg = v3dr_getImage4d(_idep);
			this->seg2GridMapping(curImg);

			editinput = 11;
			selectMode = smShowSubtree;
			b_addthiscurve = false;
			oldCursor = QCursor(Qt::ArrowCursor);
			w->setCursor(QCursor(Qt::PointingHandCursor));
		}
	}
}

double distance_wp(double x1, double y1, double z1, double x2, double y2, double z2){
	return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2) + (z1 - z2)*(z1 - z2));
}

void Renderer_gl1::callShowBreakPoints()
{

	V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
	My4DImage* curImg = 0;       if (w) curImg = v3dr_getImage4d(_idep);
	XFormWidget* curXWidget = 0; if (w) curXWidget = v3dr_getXWidget(_idep);

	if (curImg->listLandmarks.size() > 0){
		curImg->listLandmarks.clear();
		updateLandmark();
		return;
	}
	size_t inputSegID = 0;

	this->segEnd2SegIDmapping(curImg);

	set<size_t> alltreeSegs;

	alltreeSegs.clear();
	this->subtreeSegs.clear();

	cout << "wp_debug: " << __LINE__ << __FUNCTION__ << ": " << curImg->tracedNeuron.seg.size() << endl;

	if (curImg->tracedNeuron.seg[inputSegID].to_be_deleted) return;

	size_t curSegNum = alltreeSegs.size();
	double radius = 10.0;

	while (1){
		this->subtreeSegs.clear();
		this->subtreeSegs.insert(inputSegID);
		cout << inputSegID << endl;

		cout << "wp_debug: " << __LINE__ << __FUNCTION__ << "curSegNum_start: " << curSegNum << endl;
		this->rc_findConnectedSegs_continue(curImg, inputSegID);
		curSegNum = alltreeSegs.size();
		cout << "wp_debug: " << __LINE__ << __FUNCTION__ << "curSegNum_end: " << curSegNum << endl;

		for (set<size_t>::iterator it1 = alltreeSegs.begin(); it1 != alltreeSegs.end(); it1++){
			for (set<size_t>::iterator it2 = this->subtreeSegs.begin(); it2 != this->subtreeSegs.end(); it2++){
				float xLabelTail = curImg->tracedNeuron.seg[*it1].row.begin()->x;
				float yLabelTail = curImg->tracedNeuron.seg[*it1].row.begin()->y;
				float zLabelTail = curImg->tracedNeuron.seg[*it1].row.begin()->z;
				float xLabelHead = (curImg->tracedNeuron.seg[*it1].row.end() - 1)->x;
				float yLabelHead = (curImg->tracedNeuron.seg[*it1].row.end() - 1)->y;
				float zLabelHead = (curImg->tracedNeuron.seg[*it1].row.end() - 1)->z;


				float xLabelTail2 = curImg->tracedNeuron.seg[*it2].row.begin()->x;
				float yLabelTail2 = curImg->tracedNeuron.seg[*it2].row.begin()->y;
				float zLabelTail2 = curImg->tracedNeuron.seg[*it2].row.begin()->z;
				float xLabelHead2 = (curImg->tracedNeuron.seg[*it2].row.end() - 1)->x;
				float yLabelHead2 = (curImg->tracedNeuron.seg[*it2].row.end() - 1)->y;
				float zLabelHead2 = (curImg->tracedNeuron.seg[*it2].row.end() - 1)->z;
				if (distance_wp(xLabelTail, yLabelTail, zLabelTail, xLabelTail2, yLabelTail2, zLabelTail2) < radius){
					XYZ loc1(xLabelTail, yLabelTail, zLabelTail);
					addMarker(loc1);
					XYZ loc2(xLabelTail2, yLabelTail2, zLabelTail2);
					addMarker(loc2);
				}

				if (distance_wp(xLabelTail, yLabelTail, zLabelTail, xLabelHead2, yLabelHead2, zLabelHead2) < radius){
					XYZ loc1(xLabelTail, yLabelTail, zLabelTail);
					addMarker(loc1);
					XYZ loc2(xLabelHead2, yLabelHead2, zLabelHead2);
					addMarker(loc2);
				}

				if (distance_wp(xLabelHead, yLabelHead, zLabelHead, xLabelTail2, yLabelTail2, zLabelTail2) < radius){
					XYZ loc1(xLabelHead, yLabelHead, zLabelHead);
					addMarker(loc1);
					XYZ loc2(xLabelTail2, yLabelTail2, zLabelTail2);
					addMarker(loc2);
				}

				if (distance_wp(xLabelHead, yLabelHead, zLabelHead, xLabelHead2, yLabelHead2, zLabelHead2) < radius){
					XYZ loc1(xLabelHead, yLabelHead, zLabelHead);
					addMarker(loc1);
					XYZ loc2(xLabelHead2, yLabelHead2, zLabelHead2);
					addMarker(loc2);
				}


			}
		}

		for (set<size_t>::iterator it = this->subtreeSegs.begin(); it != this->subtreeSegs.end(); it++){
			alltreeSegs.insert(*it);

		}
		while (alltreeSegs.find(inputSegID) != alltreeSegs.end()){
			inputSegID++;
		}

		if (inputSegID >= curImg->tracedNeuron.seg.size()){
			return;
		}

	}

}

void Renderer_gl1::rc_findConnectedSegs_continue(My4DImage* curImg, size_t inputSegID){
	//this->subtreeSegs.insert(inputSegID);
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
					cout << "wp_debug: " << __LINE__ << __FUNCTION__ << ": " << middleIt->second << endl;
					this->rc_findConnectedSegs_continue(curImg, middleIt->second);
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
				cout << "wp_debug: " << __LINE__ << __FUNCTION__ << ": " << *regionSegIt << endl;
				this->rc_findConnectedSegs_continue(curImg, *regionSegIt);

			}
		}
	}
	/* ------- END of [Find segments that are connected to the head or tail of input segment] ------- */


}

void Renderer_gl1::seg2GridMapping(My4DImage* curImg)
{
	// This method profiles the geometrical information of every segment in Cartesian grids with selected grid length.
	// Each grid is mapped to segments that run through it. set<segments run through the grid> = Renderer_gl1::wholeGrid2segIDmap(grid).
	// -- MK, June, 2018

	this->gridLength = 50;
	for (vector<V_NeuronSWC>::iterator segIt = curImg->tracedNeuron.seg.begin(); segIt != curImg->tracedNeuron.seg.end(); ++segIt)
	{
		for (vector<V_NeuronSWC_unit>::iterator nodeIt = segIt->row.begin(); nodeIt != segIt->row.end(); ++nodeIt)
		{
			int xLabel = nodeIt->x / this->gridLength;
			int yLabel = nodeIt->y / this->gridLength;
			int zLabel = nodeIt->z / this->gridLength;
			QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
			string gridKey = gridKeyQ.toStdString();
			this->wholeGrid2segIDmap[gridKey].insert(size_t(segIt - curImg->tracedNeuron.seg.begin()));
		}
	}

	//cout << this->wholeGrid2segIDmap.size() << endl;
}

void Renderer_gl1::segEnd2SegIDmapping(My4DImage* curImg)
{
	// This method creates segment end -> segment ID map. Used in finding any segment end that is attached in the middle of input segment. (See Renderer_gl1::rc_findConnectedSegs)
	// -- MK, June, 2018

	this->segEnd2segIDmap.clear();
	//this->head2segIDmap.clear();
	//this->tail2SegIDmap.clear();
	for (vector<V_NeuronSWC>::iterator it = curImg->tracedNeuron.seg.begin(); it != curImg->tracedNeuron.seg.end(); ++it)
	{
		double xLabelTail = it->row.begin()->x;
		double yLabelTail = it->row.begin()->y;
		double zLabelTail = it->row.begin()->z;
		double xLabelHead = (it->row.end() - 1)->x;
		double yLabelHead = (it->row.end() - 1)->y;
		double zLabelHead = (it->row.end() - 1)->z;
		QString key1Q = QString::number(xLabelTail) + "_" + QString::number(yLabelTail) + "_" + QString::number(zLabelTail);
		string key1 = key1Q.toStdString();
		QString key2Q = QString::number(xLabelHead) + "_" + QString::number(yLabelHead) + "_" + QString::number(zLabelHead);
		string key2 = key2Q.toStdString();

		this->segEnd2segIDmap.insert(pair<string, size_t>(key1, size_t(it - curImg->tracedNeuron.seg.begin())));
		this->segEnd2segIDmap.insert(pair<string, size_t>(key2, size_t(it - curImg->tracedNeuron.seg.begin())));
		//this->head2segIDmap.insert(pair<string, size_t>(key1, size_t(it - curImg->tracedNeuron.seg.begin())));
		//this->tail2SegIDmap.insert(pair<string, size_t>(key2, size_t(it - curImg->tracedNeuron.seg.begin())));
	}
}

void Renderer_gl1::callDefine3DPolyline()
{
    if(editinput == 3)
        deleteMultiNeuronsByStrokeCommit();

    selectMode = smCurveCreate_MarkerCreate1;
    b_addthiscurve = true;
    cntCur3DCurveMarkers=0; //reset
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (w)
    {
        editinput = 7;
        oldCursor = QCursor(Qt::ArrowCursor);
        w->setCursor(QCursor(Qt::PointingHandCursor));
    }
}

void Renderer_gl1::callCreateMarkerNearestNode(int x, int y)
{
    if(editinput == 3)
        deleteMultiNeuronsByStrokeCommit();

    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (w && listNeuronTree.size()>0)
    {
        w->setEditMode();
        if(listNeuronTree.at(0).editable==true || listNeuronTree.at(listNeuronTree.size()-1).editable==true)
        {
            NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(curEditingNeuron-1)));
            double best_dist;
            V3DLONG n_id = findNearestNeuronNode_WinXY(x, y ,p_tree, best_dist);
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
            My4DImage* curImg = 0;
            if(w) curImg = v3dr_getImage4d(_idep);
            curImg->proj_trace_history_append();
        }
    }
}

void Renderer_gl1::callCreateSpecialMarkerNearestNode(int x, int y)
{
    if(editinput == 3)
        deleteMultiNeuronsByStrokeCommit();

    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (w && listNeuronTree.size()>0)
    {
        w->setEditMode();
        if(listNeuronTree.at(0).editable==true || listNeuronTree.at(listNeuronTree.size()-1).editable==true)
        {
            NeuronTree *p_tree = (NeuronTree *)(&(listNeuronTree.at(curEditingNeuron-1)));
            double best_dist;
            V3DLONG n_id = findNearestNeuronNode_WinXY(x, y ,p_tree, best_dist);
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
                    addSpecialMarker(loc);
                }
            }
            My4DImage* curImg = 0;
            if(w) curImg = v3dr_getImage4d(_idep);
            curImg->proj_trace_history_append();
        }
    }

}//add special marker, by XZ, 20190720

void Renderer_gl1::callGDTracing()
{
    if(editinput == 3)
        deleteMultiNeuronsByStrokeCommit();

    selectMode = smCurveCreate_MarkerCreate1_fm;
    b_addthiscurve = true;
    cntCur3DCurveMarkers=0; //reset
    V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
    if (w)
    {
        editinput = 8;
        oldCursor = QCursor(Qt::ArrowCursor);
        w->setCursor(QCursor(Qt::PointingHandCursor));
    }
}

/**
 * @brief This is used for keyboard short-cut for n-right-strokes curve drawing
 * short-cut: Shift-L
*/
void Renderer_gl1::toggleNStrokeCurveDrawing()
{
     selectMode = smCurveRefineInit;
     b_addthiscurve = true;
     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;

     if (w)
     { oldCursor = QCursor(Qt::ArrowCursor)/*w->cursor()*/; w->setCursor(QCursor(Qt::PointingHandCursor)); }
}
/**
 * @brief Edit curve using rubber-band line
 * Steps:
 * 1. right-clicking to select the curve, get segid
 * 2. right-clicking one point on curve. this is the centre point of rubber for moving
 * 3. moving the point with a window-size to drag the curve and refine it.
*/
void Renderer_gl1::solveCurveRubberDrag()
{
     // edit_seg_id is the current seg
     if(edit_seg_id<0) return;

     updateDraggedNeuronXYZ();

     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if(w) curImg = v3dr_getImage4d(_idep);

     curImg->proj_trace_history_append();
     // update the neuron display
     curImg->update_3drenderer_neuron_view(w, this);
}

void Renderer_gl1::_updateDragPoints(int x, int y)
{
     // init DraggedNeurons
     if(!bInitDragPoints) // process at the first click
     {
          // edit_seg_id is the current seg
          if(edit_seg_id<0) return;

          V3DLONG center_id;
          V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
          My4DImage* curImg = 0;
          if(w) curImg = v3dr_getImage4d(_idep);

          V_NeuronSWC& segi = curImg->tracedNeuron.seg.at(edit_seg_id);
          center_id = findNearestNeuronNode_WinXYV2(x, y);

          if (center_id < 0)
               return; // cannot drag
          else
               draggedCenterIndex = segi.row.at(center_id).n;

          // using the drag point as center point to get "nDragWinSize" points
          int halfw = floor(nDragWinSize/2.0);

          V3DLONG start_id = center_id - halfw; // start point for draging
          V3DLONG end_id = center_id + halfw;   // end point for draging

          int nuNum = segi.row.size();
          if(start_id < 0)
               start_id = 0;
          if(end_id >= nuNum)
               end_id = nuNum-1;

          // init DraggedNeurons
          DraggedNeurons.clear();
          for(V3DLONG j=start_id; j<= end_id; j++)
          {
               V_NeuronSWC_unit v;
               v = segi.row.at(j);
               DraggedNeurons.append(v);
          }

          bInitDragPoints = true;
     }else
     {
          qDebug("Update dragging points (%d, %d)!", x, y);

          // get pos from the last element of listMarkerPos
          int nm = listMarkerPos.size();
          if(nm<1) return;
          const MarkerPos &pos = listMarkerPos.at(nm-1);

          // get XYZ from MarkerPos
          XYZ dragPt = getCenterOfMarkerPos(pos);
          //MarkerPosToXYZCenter(pos, dragPt, lastDragPos);
          //lastDragPos = dragPt;

          if (dataViewProcBox.isInner(dragPt, 0.5))
               dataViewProcBox.clamp(dragPt);

          // get dragged center neuron
          for (int i=0; i<DraggedNeurons.size(); i++)
          {
               V3DLONG ind = DraggedNeurons.at(i).n;
               if(ind == draggedCenterIndex)
               {
                    V_NeuronSWC_unit nuroc=DraggedNeurons.at(i);
                    nuroc.x = dragPt.x;
                    nuroc.y = dragPt.y;
                    nuroc.z = dragPt.z;
                    DraggedNeurons.replace(i, nuroc);
                    break;
               }
          }
     }
}


void Renderer_gl1::setDragWinSize(int csize)
{
     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if(w) curImg = v3dr_getImage4d(_idep);

     V_NeuronSWC& segi = curImg->tracedNeuron.seg.at(edit_seg_id);
     int N = segi.row.size();

     nDragWinSize += csize;
     if(nDragWinSize<3) nDragWinSize = 3;
     if(nDragWinSize>=N) nDragWinSize = N-1;

     qDebug("Drag window size is: %d", nDragWinSize);
     // update DraggedNeurons
     // using the drag point as center point to get "nDragWinSize" points
     int halfw = floor(nDragWinSize/2.0);

     // get dragged center index in the segi
     int sindex;
     for (int i=0; i<N; i++)
     {
          V3DLONG ind = segi.row.at(i).n;
          if(ind == draggedCenterIndex)
          {
               sindex =i;
               break;
          }
     }

     V3DLONG start_ind = sindex - halfw; // start point for draging
     V3DLONG end_ind = sindex + halfw;   // end point for draging

     if(start_ind < 0)
          start_ind = 0;
     if(end_ind >= N)
          end_ind = N-1;

     DraggedNeurons.clear();
     for(V3DLONG j=start_ind; j<= end_ind; j++)
     {
          V_NeuronSWC_unit v;
          v=segi.row.at(j);
          DraggedNeurons.append(v);
     }
}

/**
 * @brief This function is based on findNearestNeuronNode_WinXY(int cx, int cy,
 *  NeuronTree* ptree).
 */
V3DLONG Renderer_gl1::findNearestNeuronNode_WinXYV2(int cx, int cy)
{
     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if(w) curImg = v3dr_getImage4d(_idep);
     V_NeuronSWC& segi = curImg->tracedNeuron.seg.at(edit_seg_id);

     GLdouble px, py, pz, ix, iy, iz;

     int N = segi.row.size();
	V3DLONG best_ind=-1; double best_dist=-1;
	for (V3DLONG i=0;i<N;i++)
	{
		ix = segi.row.at(i).x, iy = segi.row.at(i).y, iz = segi.row.at(i).z;
		GLint res = gluProject(ix, iy, iz, markerViewMatrix, projectionMatrix, viewport, &px, &py, &pz);// note: should use the saved modelview,projection and viewport matrix
		py = viewport[3]-py; //the Y axis is reversed
		if (res==GL_FALSE) {qDebug()<<"gluProject() fails for NeuronTree ["<<i<<"] node"; return -1;}

		double cur_dist = (px-cx)*(px-cx)+(py-cy)*(py-cy);
		if (i==0) {	best_dist = cur_dist; best_ind=0; }
		else {	if (cur_dist<best_dist) {best_dist=cur_dist; best_ind = i;}}
	}

     return best_ind;
}


void Renderer_gl1::updateDraggedNeuronXYZ()
{
     XYZ sp, ep, cp;
     int numd = DraggedNeurons.size();
     int wsize = 1; // the half-number of points extended for the dragged points used in smooth_curve

     if(numd<=0) return;
     sp.x = DraggedNeurons.at(0).x;
     sp.y = DraggedNeurons.at(0).y;
     sp.z = DraggedNeurons.at(0).z;

     ep.x = DraggedNeurons.at(numd-1).x;
     ep.y = DraggedNeurons.at(numd-1).y;
     ep.z = DraggedNeurons.at(numd-1).z;

     // get dragged center index in the DraggedNeurons
     int cindex;
     for (int i=0; i<DraggedNeurons.size(); i++)
     {
          V3DLONG ind = DraggedNeurons.at(i).n;
          if(ind == draggedCenterIndex)
          {
               cindex =i;
               break;
          }
     }

     cp.x =DraggedNeurons.at(cindex).x;
     cp.y =DraggedNeurons.at(cindex).y;
     cp.z =DraggedNeurons.at(cindex).z;


     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if(w)  curImg = v3dr_getImage4d(_idep);

     V_NeuronSWC& segi = curImg->tracedNeuron.seg.at(edit_seg_id);

     // get dragged center index in the segi
     int sindex;
     for (int i=0; i<segi.row.size(); i++)
     {
          V3DLONG ind = segi.row.at(i).n;
          if(ind == draggedCenterIndex)
          {
               sindex =i;
               break;
          }
     }

     // save the original position of dragged points plus 2*wsize points.
     // used for connection checking of other segs to dragged points.
     vector <XYZ> old_dragged_loc;
     old_dragged_loc.clear();
     int ia = sindex - cindex; // start index in original seg
     int ib = sindex - cindex + numd -1; // end index in original seg
     int N0 = segi.row.size();
     int iaa, ibb;
     iaa = ((ia-wsize)<0)? 0:(ia-wsize); // considering wsize
     ibb = ((ib+wsize)>=N0)? (N0-1):(ib+wsize);

     for(int i=iaa; i<=ibb; i++)
     {
          XYZ pt;
          pt.x=segi.row.at(i).x;
          pt.y=segi.row.at(i).y;
          pt.z=segi.row.at(i).z;
          old_dragged_loc.push_back(pt);
     } // end saving old dragged points


     // update dragged center pos in SWC.
     segi.row.at(sindex).x = cp.x;
     segi.row.at(sindex).y = cp.y;
     segi.row.at(sindex).z = cp.z;

     // temp pts for Lagrange interpolation
     vector <XYZ> inpt;
     inpt.clear();
     inpt.push_back(sp);
     inpt.push_back(cp);
     inpt.push_back(ep);

     // smooth dragged pt using Lagrange interpolation
     vector <XYZ> outpt;
     outpt.clear();
     smoothLagrange(inpt, outpt, numd-1);

     // copy back the interpolated pts to swc seg.
     // we project pts between sp to ep to interpolated pts.
     // sp, cp, ep are not changed
     for(int i=1; i<numd-1; i++)
     {
          int ind = sindex - cindex + i;
          segi.row.at(ind).x = outpt.at(i).x;
          segi.row.at(ind).y = outpt.at(i).y;
          segi.row.at(ind).z = outpt.at(i).z;
     }

     // // project pt beteen sp and ep onto lines sp-cp, cp-ep
     // for(int i=1; i<numd-1; i++)
     // {
     //      // 1. get bp between sp-cp
     //      if(i<cindex)
     //      {
     //           // node between sp-cp
     //           XYZ bp; // perpendicular point
     //           XYZ ip1;
     //           double dist;
     //           ip1.x = DraggedNeurons.at(i).x;
     //           ip1.y = DraggedNeurons.at(i).y;
     //           ip1.z = DraggedNeurons.at(i).z;

     //           // we only need bp here, dist is no use and discard it
     //           getPerpendPointDist(ip1, sp, cp, bp, dist);
     //           // use bp to update the nuero pos in SWC
     //           int ind = sindex - cindex + i;
     //           segi.row.at(ind).x = bp.x;
     //           segi.row.at(ind).y = bp.y;
     //           segi.row.at(ind).z = bp.z;
     //      }else if (i > cindex)
     //      {
     //           // 2. get bp between cp-sp
     //           XYZ bp; // perpendicular point
     //           XYZ ip2; // node between cp-ep
     //           double dist;
     //           ip2.x = DraggedNeurons.at(i).x;
     //           ip2.y = DraggedNeurons.at(i).y;
     //           ip2.z = DraggedNeurons.at(i).z;

     //           // we only need bp here, dist is no use and discard it
     //           getPerpendPointDist(ip2, cp, ep, bp, dist);

     //           // use bp to update the nuero pos
     //           int ind = sindex - cindex + i;
     //           segi.row.at(ind).x = bp.x;
     //           segi.row.at(ind).y = bp.y;
     //           segi.row.at(ind).z = bp.z;
     //      }
     // }



     // smooth using one section
     // // only smooth the curve between dragged neuron nodes of loc_vec0
//      vector <XYZ> loc_vecsub;
//      loc_vecsub.clear();
//      // copy data between ka-kb in original seg to loc_vecsub
//      // make a larger window for smoothing
//      int ka = sindex - cindex;
//      int kb = sindex - cindex + numd -1;
//      int N0 = segi.row.size();
//      int kaa, kbb;
//      int wsize = 1;
//      kaa = ((ka-wsize)<0)? 0:(ka-wsize);
//      kbb = ((kb+wsize)>=N0)? (N0-1):(kb+wsize);
//      for(int i=kaa; i<=kbb; i++)
//      {
//           int ii=i-kaa;
//           XYZ pt;
//           pt.x=segi.row.at(i).x;
//           pt.y=segi.row.at(i).y;
//           pt.z=segi.row.at(i).z;
//           loc_vecsub.push_back(pt);
//      }
// #ifndef test_main_cpp
//      smooth_curve(loc_vecsub, 5); // do smoothing
// #endif
//      // copy data back from loc_vecsub to segi
//      for(int i=kaa; i<=kbb; i++)
//      {
//           int ii=i-kaa;
//           XYZ pt = loc_vecsub.at(ii);

//           segi.row.at(i).data[2] = pt.x;
//           segi.row.at(i).data[3] = pt.y;
//           segi.row.at(i).data[4] = pt.z;
//      }


     // smooth using two sections
     // only smooth the curve between dragged neuron nodes of loc_vec0
     vector <XYZ> loc_vecsub;
     loc_vecsub.clear();
     // 1) smooth 0~cindex section
     // copy data between ka-kb in original seg to loc_vecsub
     // make a larger window for smoothing
     int ka = sindex - cindex;
     //int kb = sindex;
     // smooth without center point
     int kb = sindex-1;
     if(kb<ka)
          kb = ka;

     int kaa, kbb;
     kaa = ((ka-wsize)<0)? 0:(ka-wsize);
     kbb = kb;
     for(int i=kaa; i<=kbb; i++)
     {
          XYZ pt;
          pt.x=segi.row.at(i).x;
          pt.y=segi.row.at(i).y;
          pt.z=segi.row.at(i).z;
          loc_vecsub.push_back(pt);
     }
#ifndef test_main_cpp
     smooth_curve(loc_vecsub, 5); // do smoothing
#endif
     // copy data back from loc_vecsub to segi
     for(int i=kaa; i<=kbb; i++)
     {
          int ii=i-kaa;
          XYZ pt = loc_vecsub.at(ii);

          segi.row.at(i).data[2] = pt.x;
          segi.row.at(i).data[3] = pt.y;
          segi.row.at(i).data[4] = pt.z;
     }

     // 2) smooth cindex~draggedneuronend section
     // copy data between ka-kb in original seg to loc_vecsub
     // make a larger window for smoothing
     loc_vecsub.clear();
     //ka = sindex;
     kb = sindex - cindex + numd -1;
     // smooth without center point
     ka = sindex+1;
     if(ka>kb)
          ka = kb;

     kaa = ka;
     kbb = ((kb+wsize)>=N0)? (N0-1):(kb+wsize);
     for(int i=kaa; i<=kbb; i++)
     {
          int ii=i-kaa;
          XYZ pt;
          pt.x=segi.row.at(i).x;
          pt.y=segi.row.at(i).y;
          pt.z=segi.row.at(i).z;
          loc_vecsub.push_back(pt);
     }
#ifndef test_main_cpp
     smooth_curve(loc_vecsub, 5); // do smoothing
#endif
     // copy data back from loc_vecsub to segi
     for(int i=kaa; i<=kbb; i++)
     {
          int ii=i-kaa;
          XYZ pt = loc_vecsub.at(ii);

          segi.row.at(i).data[2] = pt.x;
          segi.row.at(i).data[3] = pt.y;
          segi.row.at(i).data[4] = pt.z;
     }


//      // 3) smooth size 3 sindex-centered section
//      // copy data between ka-kb in original seg to loc_vecsub
//      // make a larger window for smoothing
//      loc_vecsub.clear();
//      ka = sindex-1;
//      kb = sindex+1;
//      kaa = ka;
//      kbb = kb;
//      for(int i=kaa; i<=kbb; i++)
//      {
//           int ii=i-kaa;
//           XYZ pt;
//           pt.x=segi.row.at(i).x;
//           pt.y=segi.row.at(i).y;
//           pt.z=segi.row.at(i).z;
//           loc_vecsub.push_back(pt);
//      }
// #ifndef test_main_cpp
//      smooth_curve(loc_vecsub, 5); // do smoothing
// #endif
//      // copy data back from loc_vecsub to segi
//      for(int i=kaa; i<=kbb; i++)
//      {
//           int ii=i-kaa;
//           XYZ pt = loc_vecsub.at(ii);

//           segi.row.at(i).data[2] = pt.x;
//           segi.row.at(i).data[3] = pt.y;
//           segi.row.at(i).data[4] = pt.z;
//      }


     // Update endpoints of linked segments
     V3DLONG nsegs = curImg->tracedNeuron.nsegs();
     for(V3DLONG j=0; j<nsegs; j++)
     {
          if(j!=edit_seg_id)
          {
               V_NeuronSWC& segj = curImg->tracedNeuron.seg.at(j);
               // get two end points of segj
               XYZ pt1, pt2;
               int nn = segj.row.size();
               pt1.x=segj.row.at(0).x;
               pt1.y=segj.row.at(0).y;
               pt1.z=segj.row.at(0).z;

               pt2.x=segj.row.at(nn-1).x;
               pt2.y=segj.row.at(nn-1).y;
               pt2.z=segj.row.at(nn-1).z;
               // check distance between pt1/pt2 and old dragged points
               // if dist== 0, then update to connect
               for(int i=iaa; i<=ibb; i++)
               {
                    XYZ pt;
                    int ii = i-iaa;
                    pt=old_dragged_loc.at(ii);
                    double dist;
                    dist = dist_L2(pt1, pt);
                    if(dist==0.0) //for pt1
                    {
                         // update loc of segj's end points to loc of dragged pt
                         segj.row.at(0).data[2]= segi.row.at(i).x;
                         segj.row.at(0).data[3]= segi.row.at(i).y;
                         segj.row.at(0).data[4]= segi.row.at(i).z;
                    }
                    dist = dist_L2(pt2, pt);
                    if(dist==0.0) //for pt2
                    {
                         // update loc of segj's end points to loc of dragged pt
                         segj.row.at(nn-1).data[2]= segi.row.at(i).x;
                         segj.row.at(nn-1).data[3]= segi.row.at(i).y;
                         segj.row.at(nn-1).data[4]= segi.row.at(i).z;
                    }
               }

          }
     }

     // update radius of each point
     if (V3dApplication::getMainWindow()->global_setting.b_3dcurve_autowidth)
     {
          int chno = checkCurChannel();
          if (chno<0 || chno>dim4-1)   chno = 0; //default first channel
          CurveTracePara trace_para;
          {
               trace_para.channo = (chno<0)?0:chno; if (trace_para.channo>=curImg->getCDim())
                                                         trace_para.channo=curImg->getCDim()-1;
               trace_para.landmark_id_start = -1;
               trace_para.landmark_id_end = -1;
               trace_para.sp_num_end_nodes = 2;
               trace_para.nloops = 100;
               trace_para.b_deformcurve = true;
               trace_para.sp_smoothing_win_sz = 2;
          }

          if (chno >=0) //100115, 100130: for solveCurveViews.
          {
               curImg->proj_trace_compute_radius_of_last_traced_neuron(trace_para, edit_seg_id, edit_seg_id,
                    curImg->trace_z_thickness);
          }
     }//end updating radius
}


// blend rubber-bank line like neuron dynamically when editing an existing neuron
// This function is used for debug. It is not used now.
void Renderer_gl1::blendRubberNeuron()
{
    	if (DraggedNeurons.size()<1)  return;

	int N = DraggedNeurons.size();

     // get dragged center index in the DraggedNeurons
     int cindex;
     for (int i=0; i<N; i++)
     {
          V3DLONG ind = DraggedNeurons.at(i).n;
          if(ind == draggedCenterIndex)
          {
               cindex =i;
               break;
          }
     }

     glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT);
	if (lineType==1) // float line
	{
		glDisable(GL_LIGHTING);
	}
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
     glDisable(GL_DEPTH_TEST);

     // only draw with 3 points
     glLineWidth(5);
     glPushMatrix();

     // light color of the editted seg for different appearences
     RGBA8 cc = listNeuronTree.at(edit_seg_id).color;
     cc.r = cc.r/3; cc.g = cc.g/3; cc.b = cc.b/3;

     glColor3ub( cc.r, cc.g, cc.b );

	glBegin(GL_LINE_STRIP);
     // 0 index
     V_NeuronSWC_unit dnu = DraggedNeurons.at(0);
     glVertex3d( dnu.x, dnu.y, dnu.z );
     // cindex
     dnu = DraggedNeurons.at(cindex);
     glVertex3d( dnu.x, dnu.y, dnu.z );
     // N-1 index
     dnu = DraggedNeurons.at(N-1);
     glVertex3d( dnu.x, dnu.y, dnu.z );
	glEnd();

     glPopMatrix();
	glLineWidth(1);

	glPopAttrib();
	//glFlush();
}


// from renderer_obj.cpp
const GLubyte neuron_type_color[ ][3] = {///////////////////////////////////////////////////////
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
		};//////////////////////////////////////////////////////////////////////////////////
const int neuron_type_color_num = sizeof(neuron_type_color)/(sizeof(GLubyte)*3);


void Renderer_gl1::blendDraggedNeuron()
{
	if (DraggedNeurons.size() <1)  return;

     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if(w) curImg = v3dr_getImage4d(_idep);
     V_NeuronSWC& segi = curImg->tracedNeuron.seg.at(edit_seg_id);

     unsigned char *rgba=segi.color_uc;
	V_NeuronSWC_unit S0, S1;

     int N = DraggedNeurons.size();

     // light color for different appearance
     rgba[0] = rgba[0]/2; rgba[1] = rgba[1]/2; rgba[2] = rgba[2]/2; rgba[3] = rgba[3]/2;

     glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT);
	if (lineType==1) // float line
	{
		glDisable(GL_LIGHTING);
	}
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

     glDisable(GL_DEPTH_TEST);
     glEnable(GL_ALPHA_TEST);

     int cindex;
     for (int i=0; i<N; i++)
     {
          V3DLONG ind = DraggedNeurons.at(i).n;
          if(ind == draggedCenterIndex)
          {
               cindex =i;
               break;
          }
     }

     // draw connections between three nodes
	for (int i=0; i<N; i++)
	{
		// S1 = DraggedNeurons.at(i);
		// bool valid = false;
		// if (S1.parent == -1) // root end, 081105
		// {
		// 	S0 = S1;
		// 	valid = true;
		// }
		// else if (S1.parent >=0) //change to >=0 from >0, PHC 091123
		// {
		// 	// or using hash for finding parent node
		// 	//int j = hashNeuron.value(S1.pn, -1);
          //      int j = i+1;
		// 	if (j>=0 && j <DraggedNeurons.size())
		// 	{
		// 		S0 = DraggedNeurons.at(j);
		// 		valid = true;
		// 	}
		// }
		// if (! valid)  continue;

          if (i<cindex)
          {
               S0=DraggedNeurons.at(0);
               S1=DraggedNeurons.at(cindex);
               i = i+cindex;// only allow to do once in 0~cindex
          }else if (i>cindex)
          {
               S0=DraggedNeurons.at(cindex);
               S1=DraggedNeurons.at(N-1);
               i = i+N-cindex-1;// only allow to do once in cindex~N-1
          }

		//drawNeuronTube + TubeEnd
		glPushMatrix();
		{
               if (rgba[3]==0) //make the skeleton be able to use the default color by adjusting alpha value
			{
				int type = S1.type; 			 // 090925
                    type = (int)S1.seg_id %(neuron_type_color_num -5)+5;

                    // light color for different appearences
                    int idx = (type>=0 && type<neuron_type_color_num)? type : 0;
                    GLubyte cc[4];
                    cc[0]= neuron_type_color[idx][0]/3;
                    cc[1]= neuron_type_color[idx][1]/3;
                    cc[2]= neuron_type_color[idx][2]/3;
                    cc[3]= 80;
                    glColor4ubv( cc );

                    // GLubyte cc0[3];
                    // RGBA8 cc = listNeuronTree.at(edit_seg_id).color;
                    // cc0[0] = cc.r/3; cc0[1] = cc.g/3; cc0[2] = cc.b/3; //cc0[3]=cc.a/3;
                    // glColor3ubv( cc0 );
                    //glColor4ub(rgba[0],rgba[1],rgba[2],150);
               }
			else
				glColor3ub(rgba[0],rgba[1],rgba[2]);


			// (0,0,0)--(0,0,1) ==> S0--S1
               XYZ D0, D1;
               D0.x=S0.x; D0.y=S0.y; D0.z=S0.z;
               D1.x=S1.x; D1.y=S1.y; D1.z=S1.z;
			XYZ D = D0 - D1;
			float length = norm(D);
			float r1 = S1.r;
			float r0 = S0.r;

			float rf = 2;
			r1 *= rf;
			r0 *= rf;

			if (lineType==0)
			{
				GLfloat m[4][4];
				XYZ A, B, C;
				C = //XYZ(0,0,1);
					D; normalize(C);	 if (norm(C)<.9) C = XYZ(0,0,1);
				B = //XYZ(0,1,0);
					cross(C, XYZ(0,0,1)); normalize(B);		 if (norm(B)<.9) B = XYZ(0,1,0);
				A = //XYZ(1,0,0);
					cross(C, B); //normalize(A);
				m[0][0] = A.x;	m[1][0] = B.x;	m[2][0] = C.x;	m[3][0] = S1.x;
				m[0][1] = A.y;	m[1][1] = B.y;	m[2][1] = C.y;	m[3][1] = S1.y;
				m[0][2] = A.z;	m[1][2] = B.z;	m[2][2] = C.z;	m[3][2] = S1.z;
				m[0][3] = 0;	m[1][3] = 0;	m[2][3] = 0;	m[3][3] = 1;
				glMultMatrixf(&m[0][0]);

				if (length >0)
				{
					glPushMatrix();
					drawDynamicNeuronTube(r1, r0, length); // dynamically create tube, slowly
					glPopMatrix();
				}

				glPushMatrix();
				{
					glScaled(r1, r1, r1);
					glCallList(glistTubeEnd);
				}
				glPopMatrix();

			}
			else if (lineType==1)
			{
				if (length >0)  // branch line
				{
					glLineWidth(lineWidth);
					glBegin(GL_LINES);
						glVertex3f(S0.x, S0.y, S0.z);	glVertex3f(S1.x, S1.y, S1.z);
					glEnd();
					if (nodeSize)
					{
						glPointSize(nodeSize);
						glBegin(GL_POINTS);
							glVertex3f(S1.x, S1.y, S1.z);
						glEnd();
					}
				}
				else if (rootSize)// root point
				{
					glPointSize(rootSize);
					glBegin(GL_POINTS);
						glVertex3f(S1.x, S1.y, S1.z);
					glEnd();
				}
				glLineWidth(1);
				glPointSize(1);
			}
		}
		glPopMatrix();

	}//for

     glDisable(GL_ALPHA_TEST);
     glEnable(GL_DEPTH_TEST);

     glPopAttrib();
 }

/**
 * @brief Function: decide whether point e is close enough to connect a curve
 */
void Renderer_gl1::canCurveConnect(XYZ &e, V3DLONG &closest_seg, V3DLONG &closest_node,
     bool &bConnect)
{
     // find the seg closest to e
     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if(w) curImg = v3dr_getImage4d(_idep);

     V3DLONG N = curImg->tracedNeuron.seg.size();
     closest_seg = -1;
     closest_node = -1;
     double closest_dist = -1;
     for(V3DLONG i=0;i<N;i++)
     {
          V_NeuronSWC& segi = curImg->tracedNeuron.seg.at(i);
          V3DLONG nr = segi.row.size();
          for(V3DLONG j=0;j<nr;j++)
          {
               XYZ pt;
               pt.x=segi.row.at(j).x;
               pt.y=segi.row.at(j).y;
               pt.z=segi.row.at(j).z;
               // dist between e and pt
               double dist = dist_L2(e, pt);
               if (i==0 && j==0) {closest_dist = dist; closest_seg=0; closest_node=0;}
               else if (dist<closest_dist)
               {closest_dist = dist; closest_seg=i; closest_node=j;}
          }
     }

     if(closest_dist<5) // threshold for determining whether connect or not
     {
          bConnect = true;
          return;
     }else
     {
          // do not connect, too far from every seg
          closest_seg = -1;
          closest_node = -1;
          bConnect = false;
          return;
     }
}
/**
 * @brief  Function: connect curve at the end points of segid
 */
void Renderer_gl1::connectCurve(V3DLONG &segid)
{
     // for curve connection
     V3dR_GLWidget* w = (V3dR_GLWidget*)widget;
     My4DImage* curImg = 0;
     if(w) curImg = v3dr_getImage4d(_idep);

     V3DLONG N = curImg->tracedNeuron.seg.size();

     // two end points of segid
     XYZ p0, pn;
     V_NeuronSWC& curSeg = curImg->tracedNeuron.seg[segid];
     int nn=curSeg.row.size();
     p0.x=curSeg.row.at(0).x; p0.y=curSeg.row.at(0).y; p0.z=curSeg.row.at(0).z;
     pn.x=curSeg.row.at(nn-1).x; pn.y=curSeg.row.at(nn-1).y; pn.z=curSeg.row.at(nn-1).z;

     V3DLONG closest_seg, closest_node;
     bool bConnect;
     // justify p0
     canCurveConnect(p0, closest_seg, closest_node, bConnect);
     if(bConnect)
     {
          // replace p0 with closest_node on closest_seg
          V_NeuronSWC& conSeg = curImg->tracedNeuron.seg[closest_seg];
          curSeg.row.at(0).x = conSeg.row.at(closest_node).x;
          curSeg.row.at(0).y = conSeg.row.at(closest_node).y;
          curSeg.row.at(0).z = conSeg.row.at(closest_node).z;
     }
     // justify pn
     canCurveConnect(pn, closest_seg, closest_node, bConnect);
     if(bConnect)
     {
          // replace p0 with closest_node on closest_seg
          V_NeuronSWC& conSeg = curImg->tracedNeuron.seg[closest_seg];
          curSeg.row.at(nn-1).x = conSeg.row.at(closest_node).x;
          curSeg.row.at(nn-1).y = conSeg.row.at(closest_node).y;
          curSeg.row.at(nn-1).z = conSeg.row.at(closest_node).z;
     }
}


/**
 * @brief Smooth the sharp dragged part of a curve using Lagrange interpolation.
 * The method is based on:
 * http://stackoverflow.com/questions/4120839/iteratively-smooth-a-curve
 */
void Renderer_gl1::smoothLagrange(vector <XYZ> inPoints, vector <XYZ> & outPoints, int numberOfSegments)
{
     int pointsCount = inPoints.size();

     outPoints.clear();

     // compute multipliers
	 // dynamically allocate a 2D array
	 double** multipliers = new double *[pointsCount];
	 for(int pc=0; pc<pointsCount; pc++)
		 multipliers[pc]=new double[numberOfSegments];

     double pointCountMinusOne = (double)(pointsCount - 1);

     for (int currentStep = 0; currentStep <= numberOfSegments; currentStep++)
     {
          double t = currentStep / (double)numberOfSegments;
          for (int pointIndex1 = 0; pointIndex1 < pointsCount; pointIndex1++)
          {
               double point1Weight = pointIndex1 / pointCountMinusOne;
               double currentMultiplier = 1;
               for (int pointIndex2 = 0; pointIndex2 < pointsCount; pointIndex2++)
               {
                    if (pointIndex2 == pointIndex1)
                         continue;

                    double point2Weight = pointIndex2 / pointCountMinusOne;
                    currentMultiplier *= (t - point2Weight) / (point1Weight - point2Weight);
               }
               multipliers[pointIndex1][currentStep] = currentMultiplier;
          }
     }

     // create points
     for (int currentStep = 0; currentStep <= numberOfSegments; currentStep++)
     {
          double sumX = 0;
          double sumY = 0;
          double sumZ = 0;
          for (int pointIndex = 0; pointIndex < pointsCount; pointIndex++)
          {
               sumX += inPoints.at(pointIndex).x * multipliers[pointIndex][currentStep];
               sumY += inPoints.at(pointIndex).y * multipliers[pointIndex][currentStep];
               sumZ += inPoints.at(pointIndex).z * multipliers[pointIndex][currentStep];
          }

          XYZ newPoint = XYZ(round(sumX), round(sumY), round(sumZ));
          // save this newPoint as a control point in curve
          outPoints.push_back(newPoint);
     }

	 // free memory
	 for(int pc=0; pc<pointsCount; pc++)
		 delete [] multipliers[pc];
	 delete [] multipliers;
}


