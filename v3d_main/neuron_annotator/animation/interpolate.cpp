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

std::vector<double> catmullRomInterpolate(
    std::vector<double> p0, std::vector<double> p1, std::vector<double> p2, std::vector<double> p3, 
    double t) 
{
    std::vector<double> result;
    int sx = p1.size();
    if (p2.size() > sx)
        sx = p2.size();
    for (int i = 0; i < sx; ++i) {
        double d0, d1, d2, d3;
        if (p0.size() > i)
            d0 = p0[i];
        else
            d0 = 0.0;
        if (p1.size() > i)
            d1 = p1[i];
        else
            d1 = 0.0;
        if (p2.size() > i)
            d2 = p2[i];
        else
            d2 = 0.0;
        if (p3.size() > i)
            d3 = p3[i];
        else
            d3 = 0.0;
        result.push_back(catmullRomInterpolate(d0, d1, d2, d3, t));
    }
    return result;
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

