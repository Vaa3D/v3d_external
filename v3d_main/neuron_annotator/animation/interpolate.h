#ifndef NEURON_ANNOTATOR_ANIMATE_INTERPOLATE_H_
#define NEURON_ANNOTATOR_ANIMATE_INTERPOLATE_H_

#include "../geometry/Vector3D.h"
#include "../geometry/Rotation3D.h"
#include "../geometry/CameraModel.h"
#include "AnimationFrame.h"

// Build up interpolation of compound types from interpolation of simpler types

double catmullRomInterpolate(
    double p0, double p1, double p2, double p3, 
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
    
AnimationFrame catmullRomInterpolate(
    AnimationFrame p0, AnimationFrame p1, AnimationFrame p2, AnimationFrame p3, 
    double t);

#endif // NEURON_ANNOTATOR_ANIMATE_INTERPOLATE_H_
