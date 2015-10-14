#ifndef NEURON_ANNOTATOR_ANIMATE_INTERPOLATE_H_
#define NEURON_ANNOTATOR_ANIMATE_INTERPOLATE_H_

#include "../geometry/Vector3D.h"
#include "../geometry/Rotation3D.h"
#include "../geometry/CameraModel.h"

// Build up interpolation of compound types from interpolation of simpler types

double catmullRomInterpolate(
    double p0, double p1, double p2, double p3, 
    double t);

std::vector<double> catmullRomInterpolate(
    std::vector<double> p0, std::vector<double> p1, std::vector<double> p2, std::vector<double> p3, 
    double t);

Vector3D catmullRomInterpolate(
    const Vector3D& p0, const Vector3D& p1, const Vector3D& p2, const Vector3D& p3, 
    double t);

Quaternion catmullRomInterpolate(
    const Quaternion& p0, const Quaternion& p1, const Quaternion& p2, const Quaternion& p3, 
    double t);

Rotation3D catmullRomInterpolate(
    const Rotation3D& p0, const Rotation3D& p1, const Rotation3D& p2, const Rotation3D& p3, 
    double t);
    
#endif // NEURON_ANNOTATOR_ANIMATE_INTERPOLATE_H_
