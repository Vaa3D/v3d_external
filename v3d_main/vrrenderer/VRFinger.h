#ifndef __VR_FINGER_H__
#define __VR_FINGER_H__
#include "./v3dr_gl_vr.h"
#include "../neuron_annotator/geometry/Vector3D.h"

struct MyMarker;

struct Point
{
    double x,y,z,r;
    V3DLONG type;
    Point* p;
    V3DLONG childNum;
};
void VectorResampling(vector <XYZ> &loc_vec, vector <XYZ> &loc_vec_resampled, float epsilon);
void VectorResamplinger(vector <XYZ> &loc_vec, vector <XYZ> &loc_vec_resampled, int start_i, int end_i, float epsilon);
void VectorToNeuronTree(NeuronTree &SS, vector<XYZ> loc_list, int nttype=3, double creatmode=10); //LMG 26/10/2017 VR creation mode 10
bool smooth_sketch_curve(std::vector<MyMarker *> & mCoord, int winsize);

#endif
