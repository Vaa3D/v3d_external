#include "tangent_plane_gui.h"
#include "basic_memory.cpp"
#include "neuron_tracing.h"
#include "basic_surf_objs.cpp"
#include <string>
#include <iostream>
#include <sstream>
using namespace std;
void TangentPlaneWidget::update()
{
	v3dhandleList win_list = callback->getImageWindowList();
	bool is_tangent_win_exist = false;
	bool is_curwin_exist = false;
	for(int i = 0; i < win_list.size(); i++) 
	{
		if(tangent_win == win_list[i]){is_tangent_win_exist = true;}
		if(curwin == win_list[i]){is_curwin_exist = true;}
	}
	if(!is_tangent_win_exist) tangent_win = callback->newImageWindow();
	if(!is_curwin_exist){this->close(); return;}

	landmarks = callback->getLandmark(curwin);
	forward_scroller->setMaximum(landmarks.size());
	backward_scroller->setMaximum(landmarks.size()-1);
	forward_label = new QLabel(tr("forward (2 ~ %1)").arg(landmarks.size()));
	backward_label = new QLabel(tr("backward (1 ~ %1)").arg(landmarks.size()-1));

	radius_factor =  factor_scroller->value();
	threshold =  thresh_scroller->value();
	plane_thick =  thick_scroller->value();
	forward_id =  forward_scroller->value()-1;
	backward_id =  backward_scroller->value()-1;
	if(sender() == forward_scroller) direction = 0;
	if(sender() == backward_scroller) direction = 1;

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
	MyMarker marker1, marker2;
	marker1.x = loc1.x;
	marker1.y = loc1.y;
	marker1.z = loc1.z;
	marker1.radius = loc1.radius;

	marker2.x = loc2.x;
	marker2.y = loc2.y;
	marker2.z = loc2.z;
	marker2.radius = loc2.radius;

	estimated_label->setText(tr("original radius = %1").arg(marker2.radius));
	if(threshold > 0){
		double original_radius = marker2.radius;
		marker2.radius = markerRadius(inimg1d, in_sz, marker2, (double)threshold);
		estimated_label->setText(tr("original radius = %1 , estimated radius = %2").arg(original_radius).arg(marker2.radius));
	}
	if(!get_tangent_plane(outimg1d, out_sz, inimg1d, in_sz, marker1, marker2, radius_factor, plane_thick))
	{
		QMessageBox::information(0,"","unable to get tangent_plane");
		return;
	}

	Image4DSimple * newImg4d = new Image4DSimple();//callback->getImage(tangent_win);
	newImg4d->setData(outimg1d, out_sz[0], out_sz[1], out_sz[2], out_sz[3], V3D_UINT8);
	callback->setImage(tangent_win, newImg4d);
	int marker_index = (direction == 0) ? forward_id : backward_id;
	callback->setImageName(tangent_win,tr("factor = %1 thresh = %2 thick = %3 marker = %4").arg(radius_factor).arg(threshold).arg(plane_thick).arg(marker_index+1));
	callback->updateImageWindow(tangent_win);
	if(view3d_checker->isChecked())
	{
		callback->open3DWindow(tangent_win);
		callback->pushImageIn3DWindow(tangent_win);
	}
	/*
	   QMessageBox::information(0,"",tr("radius factor = %1\nimage threshold = %2\nplane thickness = %3\nforward id = %4\nbackward id = %5")
	   .arg(radius_factor)
	   .arg(threshold)
	   .arg(plane_thick)
	   .arg(forward_id)
	   .arg(backward_id));
	   */
}

void TrackingWithoutBranchWidget::update()
{
	v3dhandleList win_list = callback->getImageWindowList();
	if(win_list.empty()) return;
	curwin = callback->currentImageWindow();

	landmarks = callback->getLandmark(curwin);
	if(landmarks.size() < 2) return;

	marker1_label = new QLabel(tr("marker1 (1 ~ %1)").arg(landmarks.size()));
	marker2_label = new QLabel(tr("marker2 (1 ~ %1)").arg(landmarks.size()));
	marker1_id =  marker1_combo->currentIndex();
	marker2_id =  marker2_combo->currentIndex();
	marker1_combo->clear(); marker2_combo->clear();
	for(int i = 0; i < landmarks.size(); i++) marker1_combo->addItem(tr("%1").arg(i+1));
	for(int i = 0; i < landmarks.size(); i++) marker2_combo->addItem(tr("%1").arg(i+1));
	marker1_combo->setCurrentIndex(marker1_id); marker2_combo->setCurrentIndex(marker2_id);

	radius_factor =  factor_scroller->value();
	threshold =  thresh_scroller->value();
	plane_thick =  thick_scroller->value();

	centroid_method_id = centroid_method_combo->currentIndex();
	direction = direction_combo->currentIndex(); 

	this->setWindowTitle(tr("Tracking without branch : factor = %1 thresh = %2 thick = %3 marker = (%4,%5) centroid_method = %6 direction = %7").arg(radius_factor).arg(threshold).arg(plane_thick).arg(marker1_id+1).arg(marker2_id+1).arg(centroid_method_id).arg(direction));

	if(marker1_id == marker2_id) return;

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
	marker1->radius = loc1.radius;
	marker1->parent = root_marker;

	marker2->x = loc2.x;
	marker2->y = loc2.y;
	marker2->z = loc2.z;
	marker2->radius = loc2.radius;
	marker2->parent = root_marker;

	root_marker->x = (marker1->x + marker2->x)/2.0; root_marker->y = (marker1->y + marker2->y)/2.0;
	root_marker->z = (marker1->z + marker2->z)/2.0; root_marker->radius = (marker1->radius + marker2->radius)/2.0;

	vector<MyMarker*> allmarkers;
	allmarkers.push_back(root_marker); allmarkers.push_back(marker1); allmarkers.push_back(marker2);

	if(direction == 0 && !neuron_tracing_method_no_branch(allmarkers, inimg1d, in_sz, marker1, marker2, radius_factor, plane_thick, threshold))
	{
		QMessageBox::information(0,"","unable to do tracking without branch");
		return;
	}
	else if(direction == 1 && !neuron_tracing_method_no_branch(allmarkers, inimg1d, in_sz, marker2, marker1, radius_factor, plane_thick, threshold))
	{
		QMessageBox::information(0,"","unable to do tracking without branch");
		return;
	}

	//ostringstream oss; 
	//if(direction == 0) oss<<"forward_tracing_marker"<<(marker1_id+1)<<"_marker"<<(marker2_id+1)<<".swc"; 
	//else oss<<"backward_tracing_marker"<<(marker1_id+1)<<"_marker"<<(marker2_id+1)<<".swc"; 
	//string out_marker_file = oss.str();
	string out_marker_file = "out_swc.swc";

	if(!saveSWC_file(out_marker_file, allmarkers))return;
	NeuronTree nt = readSWC_file(out_marker_file.c_str());

	callback->setSWC(curwin, nt);
	callback->updateImageWindow(curwin);
	if(view3d_checker->isChecked())
	{
		callback->open3DWindow(curwin);
		callback->pushObjectIn3DWindow(curwin);
	}
}
