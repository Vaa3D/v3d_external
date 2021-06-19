#ifndef ROTATION_CMB_H
#define ROTATION_CMB_H

#include "Vector3D.h"
// I had to remove the gl.h include to get linux to compile - Mac seems to be OK without it also - Sean
// Hmm. Doesn't compile on my Mac..., perhaps including QGLWidget instead of gl.h will succeed on linux. - Christopher
// My mac needs one of these to define the symbol "GLdouble"

#include <cmath>
#include <iostream>

class Rotation3D;

class Quaternion
{
public:
    // Default constructor yields the identity quaternion
    Quaternion() : m_w(1.0), m_x(0), m_y(0), m_z(0) {}
    Quaternion(const qreal& angleInRadians, const UnitVector3D& axis) {
        setQuaternionFromAngleAxis(angleInRadians, axis);
    }
    Quaternion(const Rotation3D& rotation);

    Quaternion& setQuaternionFromAngleAxis( const qreal& angleInRadians, const UnitVector3D& axis );
    // read-only access to preserve special properties
    const qreal& operator[](int index) const {return m_data[index];}
    Quaternion& setQuaternionFromRotation(const Rotation3D& R);
    Quaternion slerp(const Quaternion& rhs, qreal alpha, qreal spin = 0.0) const;

protected:
    qreal& operator[](int index) {return m_data[index];}

    union {
        qreal m_data[4];
        struct {qreal m_w, m_x, m_y, m_z;};
        // struct {qreal m_s; Vector3D m_v;}; // scalar, vector // oops, can't put Vector3D in a union
    };
};

// rotation matrix
class Rotation3D
{
public:
    class Row : public UnitVector3D {
        friend class Rotation3D;
    };

    Rotation3D();
   // Rotation3D(const GLdouble mRot[16]);
    Rotation3D(const Quaternion& quat) {
        setRotationFromQuaternion(quat);
    }

    // read-only version of index operator is public
    const Row& operator[](int r) const {return data[r];}
    // compose rotation matrices
    Rotation3D operator*(const Rotation3D& rhs) const;
    // rotate vector by rotation
    Vector3D operator*(const Vector3D& rhs) const;
    // inverse/transpose of rotation
    Rotation3D operator~() const;
    Rotation3D transpose() const {return ~(*this);}
    bool operator==(const Rotation3D& rhs) const;
    bool operator!=(const Rotation3D& rhs) const;
    // Interconvert body-centered XYZ Euler angles and rotation matrix.
    // These mathematics are lifted from the SimTK tool kit, and were
    // previously unavailable in V3D.
    // Returns body centered X->Y->Z Euler angles in radians.
    Vector3D convertBodyFixedXYZRotationToThreeAngles() const;
    // Sets rotation matrix from Euler angles in radians.
    Rotation3D& setRotationFromBodyFixedXYZAngles(qreal rotX, qreal rotY, qreal rotZ);
    Rotation3D& setRotationFromAngleAboutUnitVector( qreal angleInRadians, const UnitVector3D& unitVector );
    Rotation3D& setRotationFromQuaternion( const Quaternion& q );
   // void setGLMatrix(GLdouble mRot[16]) const;
    qreal trace() const;

protected:
    static const qreal& getEps(); // machine precision
    static qreal Eps; // machine precision
    Rotation3D& setElements(qreal r00, qreal r01, qreal r02,
                     qreal r10, qreal r11, qreal r12,
                     qreal r20, qreal r21, qreal r22);
    // Writable access is protected, to prevent user from spoiling special properties
    Row& operator[](int r) {return data[r];}

    Row data[3];
};

std::ostream& operator<<(std::ostream& os, const Rotation3D& rot);

// transformation for one position in a nutation sequence
Rotation3D nutation(qreal angleInRadians, qreal coneAngleInRadians);
// change in transformation between two positions in a nutation sequence
Rotation3D deltaNutation(qreal angleInRadians, qreal angleChangeInRadians, qreal coneAngleInRadians = 0.02);

#endif // ROTATION_CMB_H

