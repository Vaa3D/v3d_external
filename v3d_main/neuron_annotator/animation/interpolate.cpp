#include "interpolate.h"

double catmullRomInterpolate(
    double p0, double p1, double p2, double p3, 
    double t) 
{
    double t2 = t*t;
    double t3 = t2*t;
    return 0.5 * ( (2.0*p1)
                   + (-p0 + p2) * t
                   + (2.0*p0 - 5.0*p1 + 4.0*p2 - p3) * t2
                   + (-p0 + 3.0*(p1 - p2) + p3) * t3 );
}

Vector3D catmullRomInterpolate(
    const Vector3D& p0, const Vector3D& p1, const Vector3D& p2, const Vector3D& p3, 
    double t)
{
    return Vector3D(
        catmullRomInterpolate(p0.x(), p1.x(), p2.x(), p3.x(), t),
        catmullRomInterpolate(p0.y(), p1.y(), p2.y(), p3.y(), t),
        catmullRomInterpolate(p0.z(), p1.z(), p2.z(), p3.z(), t));
}

Quaternion catmullRomInterpolate(
    const Quaternion& q00, const Quaternion& q01, const Quaternion& q02, const Quaternion& q03, 
    double t)
{
    // From method CatmullQuat(...) on page 449 of Visualizing Quaternions
    Quaternion q10 = q00.slerp(q01, t+1.0);
    Quaternion q11 = q01.slerp(q02, t+0.0);
    Quaternion q12 = q02.slerp(q03, t-1.0);
    Quaternion q20 = q10.slerp(q11, (t+1.0)/2.0);
    Quaternion q21 = q11.slerp(q12, t/2.0);
    return q20.slerp(q21, t);
}

Rotation3D catmullRomInterpolate(
    const Rotation3D& p0, const Rotation3D& p1, const Rotation3D& p2, const Rotation3D& p3, 
    double t)
{
    Quaternion q0, q1, q2, q3;
    q0.setQuaternionFromRotation(p0);
    q1.setQuaternionFromRotation(p1);
    q2.setQuaternionFromRotation(p2);
    q3.setQuaternionFromRotation(p3);
    Quaternion q = catmullRomInterpolate(q0, q1, q2, q3, t);
    Rotation3D result;
    result.setRotationFromQuaternion(q);
    return result;
}

AnimationFrame catmullRomInterpolate(
    const AnimationFrame& p0, const AnimationFrame& p1, const AnimationFrame& p2, const AnimationFrame& p3, 
    double t)
{
    AnimationFrame result;

    result.cameraFocus = catmullRomInterpolate(
        p0.cameraFocus, p1.cameraFocus, p2.cameraFocus, p3.cameraFocus,
        t);
    result.cameraRotation = catmullRomInterpolate(
        p0.cameraRotation, p1.cameraRotation, p2.cameraRotation, p3.cameraRotation,
        t);
    result.cameraZoom = catmullRomInterpolate(
        p0.cameraZoom, p1.cameraZoom, p2.cameraZoom, p3.cameraZoom,
        t);

    result.channelZeroVisibility = catmullRomInterpolate(
        p0.channelZeroVisibility, p1.channelZeroVisibility, p2.channelZeroVisibility, p3.channelZeroVisibility,
        t);
    result.channelOneVisibility = catmullRomInterpolate(
        p0.channelOneVisibility, p1.channelOneVisibility, p2.channelOneVisibility, p3.channelOneVisibility,
        t);
    result.channelTwoVisibility = catmullRomInterpolate(
        p0.channelTwoVisibility, p1.channelTwoVisibility, p2.channelTwoVisibility, p3.channelTwoVisibility,
        t);
    result.channelThreeVisibility = catmullRomInterpolate(
        p0.channelThreeVisibility, p1.channelThreeVisibility, p2.channelThreeVisibility, p3.channelThreeVisibility,
        t);

    return result;
}
