#include "tangent_plane_gui.h"
#include "basic_memory.cpp"
#include "neuron_tracing.h"
#include "dist_transform.h"
#include "img_convert.h"
#include "img_composite.h"
#include "basic_surf_objs.cpp"
#include <string>
#include <iostream>
#include <sstream>
using namespace std;
static bool is_update1_locked = false;
static bool is_update2_locked = false;
extern bool scale_double_to_uint8(unsigned char* &outimg1d, double * inimg1d,  V3DLONG sz[3]);
void TangentPlaneWidget::update()
{
	if(is_update1_locked) return;
	else is_update1_locked = true;
	v3dhandleList win_list = callback->getImageWindowList();
	bool is_tangent_win_exist = false;
	bool is_curwin_exist = false;
	for(int i = 0; i < win_list.size(); i++) 
	{
		if(tangent_win == win_list[i]){is_tangent_win_exist = true;}
		if(curwin == win_list[i]){is_curwin_exist = true;}
	}
	if(!is_tangent_win_exist) tangent_win = callback->newImageWindow();
	if(!is_curwin_exist){this->close(); is_update1_locked = false; return;}

	landmarks = callback->getLandmark(curwin);
	forward_scroller->setMaximum(landmarks.size());
	backward_scroller->setMaximum(landmarks.size()-1);
	forward_label = new QLabel(tr("forward (2 ~ %1)").arg(landmarks.size()));
	backward_label = new QLabel(tr("backward (1 ~ %1)").arg(landmarks.size()-1));
	is_df = df_checker->isChecked();

	radius_factor =  factor_scroller->value();
	threshold =  thresh_scroller->value();
	plane_thick =  thick_scroller->value();
	forward_id =  forward_scroller->value()-1;
	backward_id =  backward_scroller->value()-1;
	if(sender() == forward_scroller) direction = 0;
	if(sender() == backward_scroller) direction = 1;

	out_thresh_type = out_thresh_type_combo->currentIndex();

	Image4DSimple * pImg4d = callback->getImage(curwin);
	unsigned char * inimg1d = pImg4d->getRawDataAtChannel(0);
	V3DLONG *in_sz = new V3DLONG[4]; 
	in_sz[0] = pImg4d->getXDim();
	in_sz[1] = pImg4d->getYDim();
	in_sz[2] = pImg4d->getZDim();
	in_sz[3] = 1;

	unsigned char * outimg1d = 0;
	V3DLONG * out_sz = 0;
	LocationSimple loc1 = (direction == 0)?landmarks.at(forward_id -1):landmarks.at(backward_id+1);
	LocationSimple loc2 = (direction == 0)?landmarks.at(forward_id):landmarks.at(backward_id);
	int marker_index = (direction == 0) ? forward_id : backward_id;
	MyMarker marker1, marker2;
	marker1.x = loc1.x;
	marker1.y = loc1.y;
	marker1.z = loc1.z;

	marker2.x = loc2.x;
	marker2.y = loc2.y;
	marker2.z = loc2.z;
	double ini_radius = dist(marker1,marker2);
	marker1.radius = ini_radius;
	marker2.radius = ini_radius;

	estimated_label->setText(tr("original radius = %1").arg(marker2.radius));
	if(threshold > 0){
		double original_radius = marker2.radius;
		marker2.radius = markerRadius(inimg1d, in_sz, marker2, (double)threshold);
		estimated_label->setText(tr("original radius = %1 , estimated radius = %2").arg(original_radius).arg(marker2.radius));
	}
	if(!get_tangent_plane(outimg1d, out_sz, inimg1d, in_sz, marker1, marker2, radius_factor, plane_thick))
	{
		QMessageBox::information(0,"","unable to get tangent_plane");
		is_update1_locked = false; return;
	}

	if(out_thresh_type == 0) out_thresh = out_thresh_spin->value();
	else if(out_thresh_type == 1) average_threshold(out_thresh, outimg1d, out_sz);
	else if(out_thresh_type == 2) otsu_threshold(out_thresh, outimg1d, out_sz);
	out_thresh_spin->setValue(int(out_thresh + 0.5));
	if(!is_df)
	{
		V3DLONG * out_sz2 = new V3DLONG[4];
		out_sz2[0] = 2*out_sz[0] + 1;
		out_sz2[1] = out_sz[1];
		out_sz2[2] = out_sz[2];
		out_sz2[3] = out_sz[3];
		V3DLONG tol_sz2 = out_sz2[0] * out_sz2[1] * out_sz2[2] * out_sz2[3];
		V3DLONG sz10 = out_sz[1] * out_sz[0];
		V3DLONG sz10_2 = out_sz2[1] * out_sz2[0];
		unsigned char* outimg1d_2 = new unsigned char[tol_sz2];
		for(V3DLONG k = 0; k < out_sz[2]; k++)
		{
			for(V3DLONG j = 0; j < out_sz[1]; j++)
			{
				V3DLONG i = 0;
				for(i = 0; i < out_sz[0]; i++) 
				{
					int ind1 = k * sz10 + j * out_sz[0] + i;
					int ind2_1 = k * sz10_2 + j * out_sz2[0] + i;
					int ind2_2 = k * sz10_2 + j * out_sz2[0] + i + out_sz[0] + 1;
					outimg1d_2[ind2_1] = outimg1d[ind1];
					outimg1d_2[ind2_2] = outimg1d[ind1] >= out_thresh ? 255 : 0;
				}
				outimg1d_2[k * sz10_2 + j * out_sz2[0] + out_sz[0]] = 0;
			}
		}
		if(outimg1d){delete [] outimg1d; outimg1d = 0;}
		if(out_sz){delete [] out_sz; out_sz = 0;}
		outimg1d = outimg1d_2; out_sz = out_sz2;
	}
	else
	{
		double * pDist = 0;normal_distance_transform(pDist, outimg1d, out_sz, out_thresh);
		unsigned char * outdist_img1d = 0; scale_double_to_uint8(outdist_img1d, pDist, out_sz);
		unsigned char * outmerge_img1d = 0; V3DLONG * out_merge_sz = 0;
		merge_along_xaxis(outmerge_img1d, out_merge_sz, outimg1d, outdist_img1d, out_sz);
		if(pDist){delete [] pDist; pDist = 0;}
		if(outdist_img1d){delete [] outdist_img1d; outdist_img1d = 0;}
		if(outimg1d){delete [] outimg1d; outimg1d = 0;}
		if(out_sz){delete [] out_sz; out_sz = 0;}
		outimg1d = outmerge_img1d;
		out_sz = out_merge_sz;
	}
	this->setWindowTitle(tr("factor = %1 thresh = %2 thick = %3 marker = %4 out_thresh = %5").arg(radius_factor).arg(threshold).arg(plane_thick).arg(marker_index+1).arg(out_thresh));


	Image4DSimple * newImg4d = new Image4DSimple();//callback->getImage(tangent_win);
	newImg4d->setData(outimg1d, out_sz[0], out_sz[1], out_sz[2], out_sz[3], V3D_UINT8);
	callback->setImage(tangent_win, newImg4d);
	callback->setImageName(tangent_win,tr("factor = %1 thresh = %2 thick = %3 marker = %4 out_thresh = %5").arg(radius_factor).arg(threshold).arg(plane_thick).arg(marker_index+1).arg(out_thresh));
	callback->updateImageWindow(tangent_win);
	if(view3d_checker->isChecked())
	{
		callback->open3DWindow(tangent_win);
		callback->pushImageIn3DWindow(tangent_win);
	}

	is_update1_locked = false;
}

void TrackingWithoutBranchWidget::update()
{
	if(is_update2_locked) return;
	else is_update2_locked = true;

	v3dhandleList win_list = callback->getImageWindowList();
	if(win_list.empty()) {is_update2_locked = false; return;}
	curwin = callback->currentImageWindow();

	landmarks = callback->getLandmark(curwin);
	if(landmarks.size() < 2) {is_update2_locked = false;return;}

	marker1_label = new QLabel(tr("marker1 (1 ~ %1)").arg(landmarks.size()));
	marker2_label = new QLabel(tr("marker2 (1 ~ %1)").arg(landmarks.size()));
	marker1_id =  marker1_combo->currentIndex();
	marker2_id =  marker2_combo->currentIndex();
	marker1_combo->clear(); marker2_combo->clear();
	for(int i = 0; i < landmarks.size(); i++) marker1_combo->addItem(tr("%1").arg(i+1));
	for(int i = 0; i < landmarks.size(); i++) marker2_combo->addItem(tr("%1").arg(i+1));
	marker1_combo->setCurrentIndex(marker1_id); marker2_combo->setCurrentIndex(marker2_id);
	if(marker1_id >= landmarks.size() || marker2_id >= landmarks.size()){is_update2_locked = false;return;}

	radius_factor =  factor_scroller->value();

	thresh_method = thresh_type_combo->currentIndex();
	threshold =  (thresh_method == 0) ? thresh_scroller->value() : -1.0;
	if(sender() == thresh_type_combo) thresh_scroller->setEnabled(thresh_type_combo->currentIndex() == 0);

	global_tangent_radius =  (tangent_radius_type_combo->currentIndex() == 0) ? tangent_radius_scroller->value() : -1.0;
	if(sender() == tangent_radius_type_combo) tangent_radius_scroller->setEnabled(tangent_radius_type_combo->currentIndex() == 0);
	plane_thick =  thick_scroller->value();

	centroid_method_id = centroid_method_combo->currentIndex();
	direction = direction_combo->currentIndex(); 
	is_display_temp_points = display_temp_points_checker->isChecked();

	this->setWindowTitle(tr("Tracking without branch : factor = %1 thresh = %2 tangent_radius = %3 thick = %4 marker = (%5,%6) centroid_method = %7 direction = %8").arg(radius_factor).arg(threshold).arg(global_tangent_radius).arg(plane_thick).arg(marker1_id+1).arg(marker2_id+1).arg(centroid_method_id).arg(direction));

	if(marker1_id == marker2_id) {is_update2_locked = false; return;}

	Image4DSimple * pImg4d = callback->getImage(curwin);
	unsigned char * inimg1d = pImg4d->getRawDataAtChannel(0);
	V3DLONG *in_sz = new V3DLONG[4]; 
	in_sz[0] = pImg4d->getXDim();
	in_sz[1] = pImg4d->getYDim();
	in_sz[2] = pImg4d->getZDim();
	in_sz[3] = 1;

	unsigned char * outimg1d = 0;
	V3DLONG * out_sz = 0;
	LocationSimple loc1 = landmarks.at(marker1_id);
	LocationSimple loc2 = landmarks.at(marker2_id);
	MyMarker * root_marker = new MyMarker();
	MyMarker * marker1 = new MyMarker();
	MyMarker * marker2 = new MyMarker();
	marker1->x = loc1.x;
	marker1->y = loc1.y;
	marker1->z = loc1.z;
	marker1->parent = root_marker;

	marker2->x = loc2.x;
	marker2->y = loc2.y;
	marker2->z = loc2.z;
	marker2->parent = root_marker;
	double ini_radius = dist(*marker1, *marker2);
	marker1->radius = ini_radius;
	marker2->radius = ini_radius;
	root_marker->radius = ini_radius;

	root_marker->x = (marker1->x + marker2->x)/2.0; root_marker->y = (marker1->y + marker2->y)/2.0;
	root_marker->z = (marker1->z + marker2->z)/2.0; root_marker->radius = (marker1->radius + marker2->radius)/2.0;

	vector<MyMarker*> allmarkers;
	allmarkers.push_back(root_marker); allmarkers.push_back(marker1); allmarkers.push_back(marker2);

	if(direction == 0 && !neuron_tracing_method_no_branch(allmarkers, inimg1d, in_sz, marker1, marker2, radius_factor, plane_thick, threshold,centroid_method_id, is_display_temp_points, global_tangent_radius,thresh_method))
	{
		QMessageBox::information(0,"","unable to do tracking without branch");
		is_update2_locked = false; return;
	}
	else if(direction == 1 && !neuron_tracing_method_no_branch(allmarkers, inimg1d, in_sz, marker2, marker1, radius_factor, plane_thick, threshold,centroid_method_id, is_display_temp_points,global_tangent_radius,thresh_method))
	{
		QMessageBox::information(0,"","unable to do tracking without branch");
		is_update2_locked = false; return;
	}

	//ostringstream oss; 
	//if(direction == 0) oss<<"forward_tracing_marker"<<(marker1_id+1)<<"_marker"<<(marker2_id+1)<<".swc"; 
	//else oss<<"backward_tracing_marker"<<(marker1_id+1)<<"_marker"<<(marker2_id+1)<<".swc"; 
	//string out_marker_file = oss.str();
	string out_marker_file = "out_swc.swc";

	if(!saveSWC_file(out_marker_file, allmarkers)){is_update2_locked = false; return;}
	NeuronTree nt = readSWC_file(out_marker_file.c_str());

	callback->setSWC(curwin, nt);
	callback->updateImageWindow(curwin);
	if(view3d_checker->isChecked())
	{
		callback->open3DWindow(curwin);
		callback->pushObjectIn3DWindow(curwin);
	}
	is_update2_locked = false;
}
