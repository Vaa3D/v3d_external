#include "Rotation3D.h"
#include <cmath>
#include <limits>

////////////////////////
// Quaternion methods //
////////////////////////

Quaternion::Quaternion(const Rotation3D& rotation) {
    setQuaternionFromRotation(rotation);
}

Quaternion& Quaternion::setQuaternionFromAngleAxis( const qreal& a, const UnitVector3D& v )
{
    /// The cost of this method is approximately 80 flops (one sin and one cos).
    qreal ca2 = std::cos(a/2), sa2 = std::sin(a/2);

    // Multiplying an entire quaternion by -1 produces the same Rotation matrix
    // (each element of the Rotation element involves the product of two quaternion elements).
    // The canonical form is to make the first element of the quaternion positive.
    if( ca2 < 0 ) { ca2 = -ca2; sa2 = -sa2; }
    Quaternion& q = *this;
    q[0] = ca2;
    Vector3D v2 = sa2*v;
    q[1] = v2[0];
    q[2] = v2[1];
    q[3] = v2[2];
    return *this;
}

Quaternion& Quaternion::setQuaternionFromRotation(const Rotation3D& R) 
{
    // Stores the return values [cos(theta/2), lambda1*sin(theta/2), lambda2*sin(theta/2), lambda3*sin(theta/2)]
    Quaternion& q = *this;

    // Check if the trace is larger than any diagonal
    const double tr = R.trace();
    if( tr >= R[0][0]  &&  tr >= R[1][1]  &&  tr >= R[2][2] ) {
        q[0] = 1 + tr;
        q[1] = R[2][1] - R[1][2];
        q[2] = R[0][2] - R[2][0];
        q[3] = R[1][0] - R[0][1];

    // Check if R[0][0] is largest along the diagonal
    } else if( R[0][0] >= R[1][1]  &&  R[0][0] >= R[2][2]  ) {
        q[0] = R[2][1] - R[1][2];
        q[1] = 1 - (tr - 2*R[0][0]);
        q[2] = R[0][1]+R[1][0];
        q[3] = R[0][2]+R[2][0];

    // Check if R[1][1] is largest along the diagonal
    } else if( R[1][1] >= R[2][2] ) {
        q[0] = R[0][2] - R[2][0];
        q[1] = R[0][1] + R[1][0];
        q[2] = 1 - (tr - 2*R[1][1]);
        q[3] = R[1][2] + R[2][1];

    // R[2][2] is largest along the diagonal
    } else {
        q[0] = R[1][0] - R[0][1];
        q[1] = R[0][2] + R[2][0];
        q[2] = R[1][2] + R[2][1];
        q[3] = 1 - (tr - 2*R[2][2]);
    }
    // Scale to unit length
    qreal scale = 0.0;
    for (int i = 0; i < 4; ++i)
        scale += q[i] * q[i];
    scale = std::sqrt(scale);
    if( q[0] < 0 )  scale = -scale;   // canonicalize
    for (int i = 0; i < 4; ++i)
        q[i] *= 1.0/scale;

    return *this;
};

Quaternion Quaternion::slerp(const Quaternion& qB, qreal alpha, qreal spin) const {
        /**
        Spherical linear interpolation of quaternions.
        From page 448 of "Visualizing Quaternions" by Andrew J. Hanson
         */
        const Quaternion& qA = *this;
        qreal cos_t = 0;
        for (int i = 0; i < 4; ++i)
            cos_t += qA[i] * qB[i];
        // If qB is on opposite hemisphere from qA, use -qB instead
        bool bFlip = false;
        if (cos_t < 0.0) {
            cos_t = -cos_t;
            bFlip = true;
        }
        // If qB is the same as qA (within precision)
        // just linear interpolate between qA and qB.
        // Can't do spins, since we don't know what direction to spin.
        qreal beta;
        if ((1.0 - cos_t) < 1e-7) {
            beta = 1.0 - alpha;
        }
        else { // normal case
            qreal theta = acos(cos_t);
            qreal phi = theta + spin * 3.14159;
            qreal sin_t = sin(theta);
            beta = sin(theta - alpha * phi) / sin_t;
            alpha = sin(alpha * phi) / sin_t;
        }
        if (bFlip)
            alpha = -alpha;
        // interpolate
        Quaternion result;
        qreal scale = 0;
        for (int i = 0; i < 4; ++i) {
            result[i] = beta*qA[i] + alpha*qB[i];
            scale += result[i] * result[i];
        }
        // scale to unit length (1.0)
        scale = 1.0 / std::sqrt(scale);
        for (int i = 0; i < 4; ++i) {
            result[i] *= scale;
        }
        return result;
}

////////////////////////
// Rotation3D methods //
////////////////////////

qreal Rotation3D::Eps = Rotation3D::getEps();

Rotation3D::Rotation3D()
{
    Rotation3D& t = *this;
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            t[r][c] = 0.0;
    t[0][0] = t[1][1] = t[2][2] = 1.0;
}

Rotation3D::Rotation3D(const GLdouble mRot[16]) {
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            // TODO - is the GLdouble matrix transposed with respect to normal rotation?
            (*this)[r][c] = mRot[4*c + r]; // transpose
}

void Rotation3D::setGLMatrix(GLdouble mRot[16]) const {
    for(int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            mRot[4*j + i] = (*this)[i][j]; // transpose
}

// compose rotation matrices
Rotation3D Rotation3D::operator*(const Rotation3D& rhs) const
{
    const Rotation3D& lhs = *this;
    Rotation3D answer;
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j) {
            answer[i][j] = 0.0;
            for(int k = 0; k < 3; ++k)
                answer[i][j] += lhs[i][k] * rhs[k][j];
        }
    return answer;
}

// rotate vector by rotation
Vector3D Rotation3D::operator*(const Vector3D& rhs) const
{
    const Rotation3D& lhs = *this;
    Vector3D answer;
    for(int i = 0; i < 3; ++i) {
        answer[i] = 0.0;
        for(int j = 0; j < 3; ++j)
            answer[i] += lhs[i][j] * rhs[j];
    }
    return answer;
}

// inverse/transpose of rotation
Rotation3D Rotation3D::operator~() const
{
    Rotation3D answer;
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
            answer[i][j] = (*this)[j][i];
    return answer;
}

bool Rotation3D::operator==(const Rotation3D& rhs) const {
    const Rotation3D& lhs = *this;
    return !(lhs != rhs);
}

bool Rotation3D::operator!=(const Rotation3D& rhs) const {
    const Rotation3D& lhs = *this;
    if (lhs[0] != rhs[0]) return true;
    if (lhs[1] != rhs[1]) return true;
    if (lhs[2] != rhs[2]) return true;
    return false;
}

/* static */
const qreal& Rotation3D::getEps() {
    static const qreal c = std::numeric_limits<qreal>::epsilon();
    return c;
}

static qreal square(qreal x) {return x*x;}

Vector3D Rotation3D::convertBodyFixedXYZRotationToThreeAngles() const
{
    const int i = 0;
    const int j = 1;
    const int k = 2;

    // Shortcut to the elements of the rotation matrix
    const Rotation3D& R = *this;

    // Calculate theta2 using lots of information in the rotation matrix
    qreal Rsum   =  std::sqrt((square(R[i][i]) + square(R[i][j]) + square(R[j][k]) + square(R[k][k])) / 2.0);
    qreal theta2 =  std::atan2( R[i][k], Rsum ); // Rsum = abs(cos(theta2)) is inherently positive
    qreal theta1, theta3;

    // There is a "singularity" when cos(theta2) == 0
    if( Rsum > 4*Eps ) {
        theta1 =  std::atan2( -R[j][k], R[k][k] );
        theta3 =  std::atan2( -R[i][j], R[i][i] );
    }
    else if( R[i][k] > 0 ) {
        const qreal spos = R[j][i] + R[k][j];  // 2*sin(theta1 + theta3)
        const qreal cpos = R[j][j] + -R[k][i];  // 2*cos(theta1 + theta3)
        const qreal theta1PlusMinusTheta3 = std::atan2( spos, cpos );
        theta1 = theta1PlusMinusTheta3;  // Arbitrary split
        theta3 = 0;                      // Arbitrary split
    }
    else {
        const qreal sneg = (R[k][j] - R[j][i]);  // 2*sin(theta1 + minusPlus*theta3)
        const qreal cneg = R[j][j] + R[k][i];              // 2*cos(theta1 + minusPlus*theta3)
        const qreal theta1MinusPlusTheta3 = std::atan2( sneg, cneg );
        theta1 = theta1MinusPlusTheta3;  // Arbitrary split
        theta3 = 0;                      // Arbitrary split
    }

    // Return values have the following ranges:
    // -pi   <=  theta1  <=  +pi
    // -pi/2 <=  theta2  <=  +pi/2   (Rsum is inherently positive)
    // -pi   <=  theta3  <=  +pi
    return Vector3D( theta1, theta2, theta3 );
}

qreal Rotation3D::trace() const {
    const Rotation3D& R = *this;
    return R[0][0] + R[1][1] + R[2][2];
}

Rotation3D& Rotation3D::setRotationFromBodyFixedXYZAngles(qreal rotX, qreal rotY, qreal rotZ)
{
    // Calculate the sines and cosines (some hardware can do this more
    // efficiently as one Taylor series).
    const qreal cosAngle1 = std::cos( rotX ),  sinAngle1 = std::sin( rotX );
    const qreal cosAngle2 = std::cos( rotY ),  sinAngle2 = std::sin( rotY );
    const qreal cosAngle3 = std::cos( rotZ ),  sinAngle3 = std::sin( rotZ );

    // Repeated calculations (for efficiency)
    qreal s1c3 = sinAngle1 * cosAngle3;
    qreal s3c1 = sinAngle3 * cosAngle1;
    qreal s1s3 = sinAngle1 * sinAngle3;
    qreal c1c3 = cosAngle1 * cosAngle3;

    const int i = 0;
    const int j = 1;
    const int k = 2;

    Rotation3D& R = *this;
    R[i][i] =  cosAngle2 * cosAngle3;
    R[i][j] = -sinAngle3 * cosAngle2;
    R[i][k] =  sinAngle2;
    R[j][i] =  s3c1 + sinAngle2 * s1c3;
    R[j][j] =  c1c3 - sinAngle2 * s1s3;
    R[j][k] = -sinAngle1 * cosAngle2;
    R[k][i] =  s1s3 - sinAngle2 * c1c3;
    R[k][j] =  s1c3 + sinAngle2 * s3c1;
    R[k][k] =  cosAngle1 * cosAngle2;

    return *this;
}

Rotation3D& Rotation3D::setRotationFromAngleAboutUnitVector( qreal angleInRadians, const UnitVector3D& unitVector )
{
    Quaternion q;
    q.setQuaternionFromAngleAxis( angleInRadians, unitVector );
    return setRotationFromQuaternion( q );
}

Rotation3D& Rotation3D::setRotationFromQuaternion( const Quaternion& q )
{
    const qreal q00=q[0]*q[0], q11=q[1]*q[1], q22=q[2]*q[2], q33=q[3]*q[3];
    const qreal q01=q[0]*q[1], q02=q[0]*q[2], q03=q[0]*q[3];
    const qreal q12=q[1]*q[2], q13=q[1]*q[3], q23=q[2]*q[3];

    setElements( q00+q11-q22-q33,    2*(q12-q03)  ,   2*(q13+q02),
                   2*(q12+q03)  ,  q00-q11+q22-q33,   2*(q23-q01),
                   2*(q13-q02)  ,    2*(q23+q01)  , q00-q11-q22+q33);
    return *this;
}

Rotation3D& Rotation3D::setElements(qreal r00, qreal r01, qreal r02,
                 qreal r10, qreal r11, qreal r12,
                 qreal r20, qreal r21, qreal r22)
{
    Rotation3D& R = *this;
    R[0][0] = r00;
    R[0][1] = r01;
    R[0][2] = r02;
    R[1][0] = r10;
    R[1][1] = r11;
    R[1][2] = r12;
    R[2][0] = r20;
    R[2][1] = r21;
    R[2][2] = r22;
    return *this;
}

std::ostream& operator<<(std::ostream& os, const Rotation3D& rot) {
    for (int i = 0; i < 3; ++i) {
        os << "[";
        for (int j = 0; j < 3; ++j) {
            if (j > 0)
                os << ", ";
            os << rot[i][j];
        }
        os << "]" << std::endl;
    }
    return os;
}

// transformation for one position in a nutation sequence
Rotation3D nutation(qreal angleInRadians, qreal coneAngleInRadians)
{
    UnitVector3D axis(std::cos(angleInRadians), std::sin(angleInRadians), 0.0);
    return Rotation3D().setRotationFromAngleAboutUnitVector(coneAngleInRadians, axis);
}

// change in transformation between two positions in a nutation sequence
Rotation3D deltaNutation(qreal angleInRadians, qreal angleChangeInRadians, qreal coneAngleInRadians)
{
    Rotation3D R0 = nutation(-angleInRadians + angleChangeInRadians, coneAngleInRadians);
    Rotation3D R1 = nutation(-angleInRadians, coneAngleInRadians);
    return R1 * ~R0;
}



