//-----------------------------------------------------------------------------
// File:     Rotation.h
// Class:    Rotation and InverseRotation 
// Parent:   Mat33
// Purpose:  3x3 rotation class relating two right-handed orthogonal bases
//-----------------------------------------------------------------------------
#ifndef SIMTK_ROTATION_H 
#define SIMTK_ROTATION_H 

/* -------------------------------------------------------------------------- *
 *                      SimTK Core: SimTK Simmatrix(tm)                       *
 * -------------------------------------------------------------------------- *
 * This is part of the SimTK Core biosimulation toolkit originating from      *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2005-9 Stanford University and the Authors.         *
 * Authors: Paul Mitiguy and Michael Sherman                                  *
 * Contributors:                                                              *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included in *
 * all copies or substantial portions of the Software.                        *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS, CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,    *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 * -------------------------------------------------------------------------- */

//------------------------------------------------------------------------------

#include "SimTKcommon/SmallMatrix.h"
#include "SimTKcommon/internal/CoordinateAxis.h"
#include "SimTKcommon/internal/UnitVec.h"
#include "SimTKcommon/internal/Quaternion.h"

//------------------------------------------------------------------------------
#include <iosfwd>  // Forward declaration of iostream
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace SimTK {


enum BodyOrSpaceType { BodyRotationSequence=0, SpaceRotationSequence=1 };

//------------------------------------------------------------------------------
// Forward declarations
template <class P> class Rotation_;
template <class P> class InverseRotation_;

typedef Rotation_<Real>             Rotation;
typedef Rotation_<float>           fRotation;
typedef Rotation_<double>          dRotation;

typedef InverseRotation_<Real>      InverseRotation;
typedef InverseRotation_<float>    fInverseRotation;
typedef InverseRotation_<double>   dInverseRotation;

//------------------------------------------------------------------------------
/**
 * The Rotation class is a Mat33 that guarantees that the matrix is a legitimate 
 * 3x3 array associated with the relative orientation of two right-handed, 
 * orthogonal, unit vector bases. The Rotation class takes advantage of
 * known properties of orthogonal matrices. For example, multiplication by a 
 * rotation matrix preserves a vector's length so unit vectors are still unit 
 * vectors afterwards and don't need to be re-normalized.
 * 
 * A rotation is an orthogonal matrix whose columns and rows are directions 
 * (that is, unit vectors) that are mutually orthogonal. Furthermore, if the 
 * columns (or rows) are labeled x,y,z it always holds that z = x X y (rather 
 * than -(x X y)) ensuring that this is a right-handed rotation matrix and not 
 * a reflection. This is equivalent to saying that the determinant of a rotation
 * matrix is 1, not -1.
 *
 * Suppose there is a vector v_F expressed in terms of the right-handed, 
 * orthogonal unit vectors Fx, Fy, Fz and one would like to express v instead
 * as v_G, in terms of a right-handed, orthogonal unit vectors Gx, Gy, Gz. To 
 * calculate it, we form a rotation matrix R_GF whose columns are the F unit 
 * vectors re-expressed in G:
 * <pre>
 *             G F   (      |      |      )
 *      R_GF =  R  = ( Fx_G | Fy_G | Fz_G )
 *                   (      |      |      )
 * where
 *      Fx_G = ~( ~Fx*Gx, ~Fx*Gy, ~Fx*Gz ), etc.
 * </pre>
 * (~Fx*Gx means dot(Fx,Gx)). Note that we use "monogram" notation R_GF in 
 * code to represent the more typographically demanding superscripted notation 
 * for rotation matrices. Now we can re-express the vector v from frame F to 
 * frame G via
 * <pre>
 *      v_G = R_GF * v_F. 
 * </pre>
 * Because a rotation is orthogonal, its transpose is its inverse. Hence
 * R_FG = ~R_GF (where ~ is the SimTK "transpose" operator). This transpose 
 * matrix can be used to expressed v_G in terms of Fx, Fy, Fz as
 * <pre>
 *      v_F = R_FG * v_G  or  v_F = ~R_GF * v_G
 * </pre>
 * In either direction, correct behavior can be obtained by using the 
 * recommended notation and then matching up the frame labels (after
 * interpreting the "~" operator as reversing the labels).
 */
//------------------------------------------------------------------------------
template <class P> // templatized by precision
class Rotation_ : public Mat<3,3,P> {
    typedef P               RealP;
    typedef Mat<2,2,P>      Mat22P;
    typedef Mat<3,2,P>      Mat32P;
    typedef Mat<3,3,P>      Mat33P;
    typedef Vec<2,P>        Vec2P;
    typedef Vec<3,P>        Vec3P;
    typedef Vec<4,P>        Vec4P;
    typedef UnitVec<P,1>    UnitVec3P; // stride is 1 here, length is always 3
    typedef SymMat<3,P>     SymMat33P;
    typedef Quaternion_<P>  QuaternionP;
public:
    // Default constructor and constructor-like methods
    Rotation_() : Mat33P(1) {}    
    Rotation_&  setRotationToIdentityMatrix()  { Mat33P::operator=(RealP(1));  return *this; }
    Rotation_&  setRotationToNaN()             { Mat33P::setToNaN();    return *this; } 

    // Default copy constructor and assignment operator
    Rotation_( const Rotation_& R ) : Mat33P(R)  {}
    Rotation_&  operator=( const Rotation_& R )  { Mat33P::operator=( R.asMat33() );  return *this; }

    /// Constructor for right-handed rotation by an angle (in radians) about a coordinate axis.
    //@{
    Rotation_( RealP angle, const CoordinateAxis& axis )             { setRotationFromAngleAboutAxis( angle, axis ); }
    Rotation_( RealP angle, const CoordinateAxis::XCoordinateAxis )  { setRotationFromAngleAboutX( std::cos(angle), std::sin(angle) ); }
    Rotation_( RealP angle, const CoordinateAxis::YCoordinateAxis )  { setRotationFromAngleAboutY( std::cos(angle), std::sin(angle) ); }
    Rotation_( RealP angle, const CoordinateAxis::ZCoordinateAxis )  { setRotationFromAngleAboutZ( std::cos(angle), std::sin(angle) ); }
    //@}
    /// Set this Rotation_ object to a right-handed rotation by an angle (in radians) about a coordinate axis.
    //@{
    Rotation_&  setRotationFromAngleAboutAxis( RealP angle, const CoordinateAxis& axis )  { return axis.isXAxis() ? setRotationFromAngleAboutX(angle) : (axis.isYAxis() ? setRotationFromAngleAboutY(angle) : setRotationFromAngleAboutZ(angle) ); }
    Rotation_&  setRotationFromAngleAboutX( RealP angle )  { return setRotationFromAngleAboutX( std::cos(angle), std::sin(angle) ); }
    Rotation_&  setRotationFromAngleAboutY( RealP angle )  { return setRotationFromAngleAboutY( std::cos(angle), std::sin(angle) ); }
    Rotation_&  setRotationFromAngleAboutZ( RealP angle )  { return setRotationFromAngleAboutZ( std::cos(angle), std::sin(angle) ); }
    Rotation_&  setRotationFromAngleAboutX( RealP cosAngle, RealP sinAngle )  { Mat33P& R = *this;  R[0][0] = 1;   R[0][1] = R[0][2] = R[1][0] = R[2][0] = 0;   R[1][1] = R[2][2] = cosAngle;  R[1][2] = -(R[2][1] = sinAngle);  return *this; }
    Rotation_&  setRotationFromAngleAboutY( RealP cosAngle, RealP sinAngle )  { Mat33P& R = *this;  R[1][1] = 1;   R[0][1] = R[1][0] = R[1][2] = R[2][1] = 0;   R[0][0] = R[2][2] = cosAngle;  R[2][0] = -(R[0][2] = sinAngle);  return *this; }
    Rotation_&  setRotationFromAngleAboutZ( RealP cosAngle, RealP sinAngle )  { Mat33P& R = *this;  R[2][2] = 1;   R[0][2] = R[1][2] = R[2][0] = R[2][1] = 0;   R[0][0] = R[1][1] = cosAngle;  R[0][1] = -(R[1][0] = sinAngle);  return *this; }
    //@}

    /// Constructor for right-handed rotation by an angle (in radians) about an arbitrary vector.
    //@{
    Rotation_( RealP angle, const UnitVec3P& unitVector ) { setRotationFromAngleAboutUnitVector(angle,unitVector); }
    Rotation_( RealP angle, const Vec3P& nonUnitVector )  { setRotationFromAngleAboutNonUnitVector(angle,nonUnitVector); }
    //@}
    /// Set this Rotation_ object to a right-handed rotation of an angle (in radians) about an arbitrary vector.
    //@{
    Rotation_&  setRotationFromAngleAboutNonUnitVector( RealP angle, const Vec3P& nonUnitVector )  { return setRotationFromAngleAboutUnitVector( angle, UnitVec3P(nonUnitVector) ); }
    SimTK_SimTKCOMMON_EXPORT Rotation_&  setRotationFromAngleAboutUnitVector( RealP angle, const UnitVec3P& unitVector );
    //@}

    /// Constructor for two-angle, two-axes, Body-fixed or Space-fixed rotation sequences (angles are in radians)
    Rotation_( BodyOrSpaceType bodyOrSpace, RealP angle1, const CoordinateAxis& axis1, RealP angle2, const CoordinateAxis& axis2 )                                            { setRotationFromTwoAnglesTwoAxes(    bodyOrSpace,angle1,axis1,angle2,axis2); }
    /// Constructor for three-angle Body-fixed or Space-fixed rotation sequences (angles are in radians)
    Rotation_( BodyOrSpaceType bodyOrSpace, RealP angle1, const CoordinateAxis& axis1, RealP angle2, const CoordinateAxis& axis2, RealP angle3, const CoordinateAxis& axis3 )  { setRotationFromThreeAnglesThreeAxes(bodyOrSpace,angle1,axis1,angle2,axis2,angle3,axis3); }
    /// Set this Rotation_ object to a two-angle, two-axes, Body-fixed or Space-fixed rotation sequences (angles are in radians)
    SimTK_SimTKCOMMON_EXPORT Rotation_&  setRotationFromTwoAnglesTwoAxes(     BodyOrSpaceType bodyOrSpace, RealP angle1, const CoordinateAxis& axis1, RealP angle2, const CoordinateAxis& axis2 ); 
    /// Set this Rotation_ object to a three-angle Body-fixed or Space-fixed rotation sequences (angles are in radians)
    SimTK_SimTKCOMMON_EXPORT Rotation_&  setRotationFromThreeAnglesThreeAxes( BodyOrSpaceType bodyOrSpace, RealP angle1, const CoordinateAxis& axis1, RealP angle2, const CoordinateAxis& axis2, RealP angle3, const CoordinateAxis& axis3 );

    /// Set this Rotation_ to represent a rotation characterized by subsequent rotations of:
    /// +v[0] about the body frame's X axis,      followed by a rotation of 
    /// +v[1] about the body frame's NEW Y axis.  See Kane, Spacecraft Dynamics, pg. 423, body-three: 1-2-3.
    void setRotationToBodyFixedXY( const Vec2P& v)   { setRotationFromTwoAnglesTwoAxes(     BodyRotationSequence, v[0], XAxis, v[1], YAxis ); }
    /// Set this Rotation_ to represent a rotation characterized by subsequent rotations of:
    /// +v[0] about the body frame's X axis,      followed by a rotation of 
    /// +v[1] about the body frame's NEW Y axis,  followed by a rotation of 
    /// +v[2] about the body frame's NEW Z axis.  See Kane, Spacecraft Dynamics, pg. 423, body-three: 1-2-3.
    void setRotationToBodyFixedXYZ( const Vec3P& v)  { setRotationFromThreeAnglesThreeAxes( BodyRotationSequence, v[0], XAxis, v[1], YAxis, v[2], ZAxis ); }

    /// Constructor for creating a rotation matrix from a quaternion.
    explicit Rotation_( const QuaternionP& q )  { setRotationFromQuaternion(q); }
    /// Method for creating a rotation matrix from a quaternion.
    SimTK_SimTKCOMMON_EXPORT Rotation_&  setRotationFromQuaternion( const QuaternionP& q );

    /// Construct a Rotation_ directly from a Mat33P (we trust that m is a valid Rotation_!)
    Rotation_( const Mat33P& m, bool ) : Mat33P(m) {}

    /// Constructs an (hopefully nearby) orthogonal rotation matrix from a generic Mat33P.
    explicit Rotation_( const Mat33P& m )  { setRotationFromApproximateMat33(m); }
    /// Set this Rotation_ object to an (hopefully nearby) orthogonal rotation matrix from a generic Mat33P.
    SimTK_SimTKCOMMON_EXPORT Rotation_&  setRotationFromApproximateMat33( const Mat33P& m );

    /// Calculate R_AB by knowing one of B's unit vector expressed in A.
    /// Note: The other vectors are perpendicular (but somewhat arbitrarily so).
    //@{
    Rotation_( const UnitVec3P& uvec, const CoordinateAxis axis )  { setRotationFromOneAxis(uvec,axis); }
    SimTK_SimTKCOMMON_EXPORT Rotation_&  setRotationFromOneAxis( const UnitVec3P& uvec, const CoordinateAxis axis );
    //@}

    /// Calculate R_AB by knowing one of B's unit vectors u1 (could be Bx, By, or Bz) 
    /// expressed in A and a vector v (also expressed in A) that is approximately in 
    /// the desired direction for a second one of B's unit vectors, u2 (!= u1). 
    /// If v is not perpendicular to u1, no worries - we'll find a direction for u2 
    /// that is perpendicular to u1 and comes closest to v. The third vector u3
    /// is +/- u1 X u2, as appropriate for a right-handed rotation matrix.
    //@{
    Rotation_( const UnitVec3P& uveci, const CoordinateAxis& axisi, const Vec3P& vecjApprox, const CoordinateAxis& axisjApprox )  { setRotationFromTwoAxes(uveci,axisi,vecjApprox,axisjApprox); }
    SimTK_SimTKCOMMON_EXPORT Rotation_&  setRotationFromTwoAxes( const UnitVec3P& uveci, const CoordinateAxis& axisi, const Vec3P& vecjApprox, const CoordinateAxis& axisjApprox );
    //@}

    // Converts rotation matrix to one or two or three orientation angles.
    // Note:  The result is most meaningful if the Rotation_ matrix is one that can be produced by such a sequence.
    // Use1:  someRotation.convertOneAxisRotationToOneAngle( XAxis );
    // Use2:  someRotation.convertTwoAxesRotationToTwoAngles(     SpaceRotationSequence, YAxis, ZAxis );
    // Use3:  someRotation.convertThreeAxesRotationToThreeAngles( SpaceRotationSequence, ZAxis, YAxis, XAxis );
    // Use4:  someRotation.convertRotationToAngleAxis();   Return: [angleInRadians, unitVectorX, unitVectorY, unitVectorZ].

    /// Converts rotation matrix to a single orientation angle.
    /// Note:  The result is most meaningful if the Rotation_ matrix is one that can be produced by such a sequence.
    SimTK_SimTKCOMMON_EXPORT RealP  convertOneAxisRotationToOneAngle( const CoordinateAxis& axis1 ) const;
    /// Converts rotation matrix to two orientation angles.
    /// Note:  The result is most meaningful if the Rotation_ matrix is one that can be produced by such a sequence.
    SimTK_SimTKCOMMON_EXPORT Vec2P  convertTwoAxesRotationToTwoAngles(     BodyOrSpaceType bodyOrSpace, const CoordinateAxis& axis1, const CoordinateAxis& axis2 ) const;
    /// Converts rotation matrix to three orientation angles.
    /// Note:  The result is most meaningful if the Rotation_ matrix is one that can be produced by such a sequence.
    SimTK_SimTKCOMMON_EXPORT Vec3P  convertThreeAxesRotationToThreeAngles( BodyOrSpaceType bodyOrSpace, const CoordinateAxis& axis1, const CoordinateAxis& axis2, const CoordinateAxis& axis3 ) const;
    /// Converts rotation matrix to a quaternion.
    SimTK_SimTKCOMMON_EXPORT QuaternionP convertRotationToQuaternion() const;
    /// Converts rotation matrix to angle-axis form.
    Vec4P  convertRotationToAngleAxis() const  { return convertRotationToQuaternion().convertQuaternionToAngleAxis(); }

    /// A convenient special case of convertTwoAxesRotationToTwoAngles().
    Vec2P  convertRotationToBodyFixedXY() const   { return convertTwoAxesRotationToTwoAngles( BodyRotationSequence, XAxis, YAxis ); }
    /// A convenient special case of convertThreeAxesRotationToThreeAngles().
    Vec3P  convertRotationToBodyFixedXYZ() const  { return convertThreeAxesRotationToThreeAngles( BodyRotationSequence, XAxis, YAxis, ZAxis ); }

    /// Perform an efficient transform of a symmetric matrix that must be re-expressed with
    /// a multiply from both left and right, such as an inertia matrix. Details: assuming
    /// this Rotation is R_AB, and given a symmetric dyadic matrix S_BB expressed in B,
    /// we can reexpress it in A using S_AA=R_AB*S_BB*R_BA. The matrix should be one that
    /// is formed as products of vectors expressed in A, such as inertia, gyration or
    /// covariance matrices. This can be done efficiently exploiting properties of R
    /// (orthogonal) and S (symmetric). Total cost is 57 flops.
    SimTK_SimTKCOMMON_EXPORT SymMat33P reexpressSymMat33(const SymMat33P& S_BB) const;

    /// Return true if "this" Rotation is nearly identical to "R" within a specified pointing angle error
    //@{
    SimTK_SimTKCOMMON_EXPORT bool  isSameRotationToWithinAngle( const Rotation_& R, RealP okPointingAngleErrorRads ) const;
    bool isSameRotationToWithinAngleOfMachinePrecision( const Rotation_& R) const       
    {   return isSameRotationToWithinAngle( R, NTraits<P>::getSignificant() ); }
    //@}
    RealP  getMaxAbsDifferenceInRotationElements( const Rotation_& R ) const {            
        const Mat33P& A = asMat33();  const Mat33P& B = R.asMat33();  RealP maxDiff = 0;  
        for( int i=0;  i<=2; i++ ) for( int j=0; j<=2; j++ ) 
        {   RealP absDiff = std::fabs(A[i][j] - B[i][j]);  
            if( absDiff > maxDiff ) maxDiff = absDiff; }  
        return maxDiff; 
    } 

    bool  areAllRotationElementsSameToEpsilon( const Rotation_& R, RealP epsilon ) const 
    {   return getMaxAbsDifferenceInRotationElements(R) <= epsilon ; }
    bool  areAllRotationElementsSameToMachinePrecision( const Rotation_& R ) const       
    {   return areAllRotationElementsSameToEpsilon( R, NTraits<P>::getSignificant() ); } 

    /// Like copy constructor but for inverse rotation.  This allows implicit conversion from InverseRotation_ to Rotation_.
    inline Rotation_( const InverseRotation_<P>& );
    /// Like copy assignment but for inverse rotation.
    inline Rotation_& operator=( const InverseRotation_<P>& );

    /// Convert from Rotation_ to InverseRotation_ (no cost). Overrides base class invert().
    const InverseRotation_<P>&  invert() const  { return *reinterpret_cast<const InverseRotation_<P>*>(this); }
    /// Convert from Rotation_ to writable InverseRotation_ (no cost).
    InverseRotation_<P>&        updInvert()     { return *reinterpret_cast<InverseRotation_<P>*>(this); }

    /// Transpose, and transpose operators. For an orthogonal matrix like this one, transpose
    /// is the same thing as inversion. These override the base class transpose methods.
    //@{
    const InverseRotation_<P>&  transpose() const  { return invert(); }
    const InverseRotation_<P>&  operator~() const  { return invert(); }
    InverseRotation_<P>&        updTranspose()     { return updInvert(); }
    InverseRotation_<P>&        operator~()        { return updInvert(); }
    //@}

    /// In-place composition of Rotation matrices.
    //@{
    inline Rotation_&  operator*=( const Rotation_<P>& R );
    inline Rotation_&  operator/=( const Rotation_<P>& R );
    inline Rotation_&  operator*=( const InverseRotation_<P>& );
    inline Rotation_&  operator/=( const InverseRotation_<P>& );
    //@}

    /// Conversion from Rotation to its base class Mat33.
    /// Note: asMat33 is more efficient than toMat33() (no copy), but you have to know the internal layout.
    //@{
    const Mat33P&  asMat33() const  { return *static_cast<const Mat33P*>(this); }
    Mat33P         toMat33() const  { return asMat33(); }
    //@}

    /// The column and row unit vector types do not necessarily have unit spacing.
    typedef  UnitVec<P,Mat33P::RowSpacing>  ColType;
    typedef  UnitRow<P,Mat33P::ColSpacing>  RowType;
    const RowType&  row( int i ) const         { return reinterpret_cast<const RowType&>(asMat33()[i]); }
    const ColType&  col( int j ) const         { return reinterpret_cast<const ColType&>(asMat33()(j)); }
    const ColType&  x() const                  { return col(0); }
    const ColType&  y() const                  { return col(1); }
    const ColType&  z() const                  { return col(2); }
    const RowType&  operator[]( int i ) const  { return row(i); }
    const ColType&  operator()( int j ) const  { return col(j); }

    /// Set the Rotation_ matrix directly - but you had better know what you are doing!
    //@{
    Rotation_&  setRotationFromMat33TrustMe( const Mat33P& m )  
    {   Mat33P& R = *this; R=m;  return *this; }   
    Rotation_&  setRotationColFromUnitVecTrustMe( int colj, const UnitVec3P& uvecj )  
    {   Mat33P& R = *this; R(colj)=uvecj.asVec3(); return *this; }   
    Rotation_&  setRotationFromUnitVecsTrustMe( const UnitVec3P& colA, const UnitVec3P& colB, const UnitVec3P& colC )  
    {   Mat33P& R = *this; R(0)=colA.asVec3(); R(1)=colB.asVec3(); R(2)=colC.asVec3(); return *this; }  
    //@}


//--------------------------------- PAUL CONTINUE FROM HERE ----------------------------------
public:
//--------------------------------------------------------------------------------------------
    /// Given Euler angles forming a body-fixed 3-2-1 sequence, and the relative
    /// angular velocity vector of B in the parent frame, *BUT EXPRESSED IN
    /// THE BODY FRAME*, return the Euler angle
    /// derivatives. You are dead if q[1] gets near 90 degrees!
    /// See Kane's Spacecraft Dynamics, page 428, body-three: 3-2-1.
    static Vec3P convertAngVelToBodyFixed321Dot(const Vec3P& q, const Vec3P& w_PB_B) {
        const RealP s1 = std::sin(q[1]), c1 = std::cos(q[1]);
        const RealP s2 = std::sin(q[2]), c2 = std::cos(q[2]);
        const RealP ooc1 = RealP(1)/c1;
        const RealP s2oc1 = s2*ooc1, c2oc1 = c2*ooc1;

        const Mat33P E( 0,    s2oc1  ,  c2oc1  ,
                        0,      c2   ,   -s2   ,
                        1,  s1*s2oc1 , s1*c2oc1 );
        return E * w_PB_B;
    }

    /// Inverse of the above routine. Returned angular velocity is B in P,
    /// expressed in *B*: w_PB_B.
    static Vec3P convertBodyFixed321DotToAngVel(const Vec3P& q, const Vec3P& qd) {
        const RealP s1 = std::sin(q[1]), c1 = std::cos(q[1]);
        const RealP s2 = std::sin(q[2]), c2 = std::cos(q[2]);

        const Mat33P Einv(  -s1  ,  0  ,  1 ,
                           c1*s2 ,  c2 ,  0 ,
                           c1*c2 , -s2 ,  0 );
        return Einv*qd;
    }

    // TODO: sherm: is this right? Warning: everything is measured in the
    // *PARENT* frame, but has to be expressed in the *BODY* frame.
    static Vec3P convertAngVelDotToBodyFixed321DotDot
        (const Vec3P& q, const Vec3P& w_PB_B, const Vec3P& wdot)
    {
        const RealP s1 = std::sin(q[1]), c1 = std::cos(q[1]);
        const RealP s2 = std::sin(q[2]), c2 = std::cos(q[2]);
        const RealP ooc1  = 1/c1;
        const RealP s2oc1 = s2*ooc1, c2oc1 = c2*ooc1;

        const Mat33P E( 0 ,   s2oc1  ,  c2oc1  ,
                       0 ,     c2   ,   -s2   ,
                       1 , s1*s2oc1 , s1*c2oc1 );
        const Vec3P qdot = E * w_PB_B;

        const RealP t =  qdot[1]*qdot[2]*s1*ooc1;
        const RealP a =  t*c2oc1; // d/dt s2oc1
        const RealP b = -t*s2oc1; // d/dt c2oc1

        const Mat33P Edot( 0 ,       a           ,         b         ,
                          0 ,   -qdot[2]*s2     ,    -qdot[2]*c2    ,
                          0 , s1*a + qdot[1]*s2 , s1*b + qdot[1]*c2 );

        return E*wdot + Edot*w_PB_B;
    }
    
    /// Given Euler angles q forming a body-fixed X-Y-Z sequence return the block
    /// of the N matrix such that qdot=N(q)*w where w is the angular velocity of
    /// B in P EXPRESSED IN *B*!!! This matrix will be singular if Y (q[1]) gets
    /// near 90 degrees!
    /// @note This version is very expensive because it has to calculate sines
    ///       and cosines. If you already have those, use the alternate form
    ///       of this method.
    /// @see Kane's Spacecraft Dynamics, page 427, body-three: 1-2-3.
    static Mat33P calcNForBodyXYZInBodyFrame(const Vec3P& q) {
        // Note: q[0] is not referenced so we won't waste time calculating
        // its cosine and sine here.
        return calcNForBodyXYZInBodyFrame(Vec3P(0,std::cos(q[1]),std::cos(q[2])),
                                          Vec3P(0,std::sin(q[1]),std::sin(q[2])));
    }

    /// This fast version of calcNForBodyXYZInBodyFrame() assumes you have already
    /// calculated the cosine and sine of the three q's. Note that we only
    /// look at the cosines and sines of q[1] and q[2]; q[0] does not matter
    /// so you don't have to fill in the 0'th element of cq and sq.
    /// Cost is one divide plus 6 flops.
    static Mat33P calcNForBodyXYZInBodyFrame(const Vec3P& cq, const Vec3P& sq) {
        const RealP s1 = sq[1], c1 = cq[1];
        const RealP s2 = sq[2], c2 = cq[2];
        const RealP ooc1  = 1/c1;
        const RealP s2oc1 = s2*ooc1, c2oc1 = c2*ooc1;

        return Mat33P(    c2oc1  , -s2oc1  , 0,
                            s2   ,    c2   , 0,
                       -s1*c2oc1 , s1*s2oc1, 1 );
    }

    /// Given Euler angles forming a body-fixed X-Y-Z sequence q, and their time
    /// derivatives qdot, return the block of the NDot matrix such that 
    /// qdotdot=N(q)*wdot + NDot(q,u)*w where w is the angular velocity of
    /// B in P EXPRESSED IN *B*!!! This matrix will be singular if Y (q[1]) gets
    /// near 90 degrees! See calcNForBodyXYZInBodyFrame() for the matrix we're
    /// differentiating here.
    /// @note This version is very expensive because it has to calculate sines
    ///       and cosines. If you already have those, use the alternate form
    ///       of this method.
    static Mat33P calcNDotForBodyXYZInBodyFrame(const Vec3P& q, const Vec3P& qdot) {
        // Note: q[0] is not referenced so we won't waste time calculating
        // its cosine and sine here.
        return calcNDotForBodyXYZInBodyFrame(Vec3P(0,std::cos(q[1]),std::cos(q[2])),
                                             Vec3P(0,std::sin(q[1]),std::sin(q[2])),
                                             qdot);
    }

    /// This fast version of calcNDotForBodyXYZInBodyFrame() assumes you have already
    /// calculated the cosine and sine of the three q's. Note that we only
    /// look at the cosines and sines of q[1] and q[2]; q[0] does not matter
    /// so you don't have to fill in the 0'th element of cq and sq.
    /// Cost is one divide plus 19 flops.
    static Mat33P calcNDotForBodyXYZInBodyFrame(const Vec3P& cq, const Vec3P& sq, 
                                                const Vec3P& qdot) 
    {
        const RealP s1 = sq[1], c1 = cq[1];
        const RealP s2 = sq[2], c2 = cq[2];
        const RealP ooc1  = 1/c1;
        const RealP s2oc1 = s2*ooc1, c2oc1 = c2*ooc1;

        const RealP t =  qdot[1]*qdot[2]*s1*ooc1;
        const RealP a =  t*c2oc1; // d/dt s2oc1
        const RealP b = -t*s2oc1; // d/dt c2oc1

        return Mat33P(       b           ,        -a         , 0,
                          qdot[2]*c2     ,    -qdot[2]*s2    , 0,
                      -s1*b - qdot[1]*c2 , s1*a + qdot[1]*s2 , 0 );
    }

    /// Inverse of the above routine. Return the inverse NInv of the N block
    /// computed above, such that w=NInv(q)*qdot where w is the angular velocity of
    /// B in P EXPRESSED IN *B*!!! This matrix is never singular.
    /// @note This version is very expensive because it has to calculate sines
    ///       and cosines. If you already have those, use the alternate form
    ///       of this method.
    static Mat33P calcNInvForBodyXYZInBodyFrame(const Vec3P& q) {
        // Note: q[0] is not referenced so we won't waste time calculating
        // its cosine and sine here.
        return calcNInvForBodyXYZInBodyFrame(Vec3P(0,std::cos(q[1]),std::cos(q[2])),
                                             Vec3P(0,std::sin(q[1]),std::sin(q[2])));
    }

    /// This fast version of calcNInvForBodyXYZInBodyFrame() assumes you have already
    /// calculated the cosine and sine of the three q's. Note that we only
    /// look at the cosines and sines of q[1] and q[2]; q[0] does not matter
    /// so you don't have to fill in the 0'th element of cq and sq.
    /// Cost is 3 flops.
    static Mat33P calcNInvForBodyXYZInBodyFrame(const Vec3P& cq, const Vec3P& sq) 
    {
        const RealP s1 = sq[1], c1 = cq[1];
        const RealP s2 = sq[2], c2 = cq[2];

        return Mat33P( c1*c2 ,  s2 , 0 ,
                      -c1*s2 ,  c2 , 0 ,
                        s1   ,  0  , 1 );
    }

    /// Given Euler angles forming a body-fixed 1-2-3 sequence, and the relative
    /// angular velocity vector of B in the parent frame,  *BUT EXPRESSED IN
    /// THE BODY FRAME*, return the Euler angle
    /// derivatives. You are dead if q[1] gets near 90 degrees!
    /// @note This version is very expensive because it has to calculate sines
    ///       and cosines. If you already have those, use the alternate form
    ///       of this method.
    /// @see Kane's Spacecraft Dynamics, page 427, body-three: 1-2-3.
    static Vec3P convertAngVelInBodyFrameToBodyXYZDot(const Vec3P& q, const Vec3P& w_PB_B)
    {   return convertAngVelInBodyFrameToBodyXYZDot(Vec3P(0,std::cos(q[1]),std::cos(q[2])),
                                                    Vec3P(0,std::sin(q[1]),std::sin(q[2])),
                                                    w_PB_B); }

    /// This fast version of convertAngVelInBodyFrameToBodyXYZDot() assumes you have 
    /// already calculated the cosine and sine of the three q's. Note that we only
    /// look at the cosines and sines of q[1] and q[2]; q[0] does not matter
    /// so you don't have to fill in the 0'th element of cq and sq.
    /// Cost is one divide plus 21 flops.
    static Vec3P convertAngVelInBodyFrameToBodyXYZDot
       (const Vec3P& cq, const Vec3P& sq, const Vec3P& w_PB_B) 
    {   return calcNForBodyXYZInBodyFrame(cq,sq)*w_PB_B; }

    /// Inverse of the above routine. Returned angular velocity is B in P,
    /// expressed in *B*: w_PB_B.
    /// @note This version is very expensive because it has to calculate sines
    ///       and cosines. If you already have those, use the alternate form
    ///       of this method.
    static Vec3P convertBodyXYZDotToAngVelInBodyFrame(const Vec3P& q, const Vec3P& qdot)
    {   return convertBodyXYZDotToAngVelInBodyFrame(Vec3P(0,std::cos(q[1]),std::cos(q[2])),
                                                    Vec3P(0,std::sin(q[1]),std::sin(q[2])),
                                                    qdot); }

    /// This fast version of convertBodyXYZDotToAngVelInBodyFrame() assumes you have 
    /// already calculated the cosine and sine of the three q's. Note that we only
    /// look at the cosines and sines of q[1] and q[2]; q[0] does not matter
    /// so you don't have to fill in the 0'th element of cq and sq.
    /// Cost is 18 flops.
    static Vec3P convertBodyXYZDotToAngVelInBodyFrame
       (const Vec3P& cq, const Vec3P& sq, const Vec3P& qdot) 
    {   return calcNInvForBodyXYZInBodyFrame(cq,sq)*qdot; }

    /// TODO: sherm: is this right? Warning: everything is measured in the
    /// *PARENT* frame, but has to be expressed in the *BODY* frame.
    /// @note This version is very expensive because it has to calculate sines
    ///       and cosines. If you already have those, use the alternate form
    ///       of this method.
    static Vec3P convertAngVelDotInBodyFrameToBodyXYZDotDot
        (const Vec3P& q, const Vec3P& w_PB_B, const Vec3P& wdot_PB_B)
    {
        // Note: q[0] is not referenced so we won't waste time calculating
        // its cosine and sine here.
        return convertAngVelDotInBodyFrameToBodyXYZDotDot
                   (Vec3P(0,std::cos(q[1]),std::cos(q[2])),
                    Vec3P(0,std::sin(q[1]),std::sin(q[2])),
                    w_PB_B, wdot_PB_B);
    }

    /// This faster version of convertAngVelDotInBodyFrameToBodyXYZDotDot() assumes 
    /// you have already calculated the cosine and sine of the three q's. Note 
    /// that we only look at the cosines and sines of q[1] and q[2]; q[0] does not 
    /// matter so you don't have to fill in the 0'th element of cq and sq.
    /// Cost is two divides plus 73 flops.
    static Vec3P convertAngVelDotInBodyFrameToBodyXYZDotDot
       (const Vec3P& cq, const Vec3P& sq, const Vec3P& w_PB_B, const Vec3P& wdot_PB_B)
    {
        const Mat33P N      = calcNForBodyXYZInBodyFrame(cq,sq);
        const Vec3P  qdot   = N * w_PB_B; // 15 flops
        const Mat33P NDot   = calcNDotForBodyXYZInBodyFrame(cq,sq,qdot);

        return N*wdot_PB_B + NDot*w_PB_B; // 33 flops
    }

    /// Given a possibly unnormalized quaternion q, calculate the 4x3 matrix
    /// N which maps angular velocity w to quaternion derivatives qdot. We
    /// expect the angular velocity in the parent frame, i.e. w==w_PB_P.
    /// We don't normalize, so N=|q|N' where N' is the normalized version.
    /// Cost is 7 flops.
    static Mat<4,3,P> calcUnnormalizedNForQuaternion(const Vec4P& q) {
        const Vec4P e = q/2;
        const RealP ne1 = -e[1], ne2 = -e[2], ne3 = -e[3];
        return Mat<4,3,P>( ne1,  ne2,  ne3,
                           e[0], e[3], ne2,
                           ne3,  e[0], e[1],
                           e[2], ne1,  e[0]);
    }

    /// Given the time derivative qdot of a possibly unnormalized quaternion
    /// q, calculate the 4x3 matrix NDot which is the time derivative of the
    /// matrix N as described in calcUnnormalizedNForQuaternion(). Note that
    /// NDot = d/dt N = d/dt (|q|N') = |q|(d/dt N'), where N' is the normalized
    /// matrix, since the length of the quaternion should be a constant.
    /// Cost is 7 flops.
    static Mat<4,3,P> calcUnnormalizedNDotForQuaternion(const Vec4P& qdot) {
        const Vec4P ed = qdot/2;
        const RealP ned1 = -ed[1], ned2 = -ed[2], ned3 = -ed[3];
        return Mat<4,3,P>( ned1,  ned2,  ned3,
                           ed[0], ed[3], ned2,
                           ned3,  ed[0], ed[1],
                           ed[2], ned1,  ed[0]);
    }

    /// Given a (possibly unnormalized) quaternion q, calculate the 3x4 matrix
    /// NInv (= N^-1) which maps quaternion derivatives qdot to angular velocity
    /// w, where the angular velocity is in the parent frame, i.e. w==w_PB_P.
    /// Note: when the quaternion is not normalized, this is not precisely the
    /// (pseudo)inverse of N. inv(N)=inv(N')/|q| but we're returning
    /// |q|*inv(N')=|q|^2*inv(N). That is, NInv*N =|q|^2*I, which is I
    /// if the original q was normalized. (Note: N*NInv != I, not even close.)
    /// Cost is 7 flops.
    static Mat<3,4,P> calcUnnormalizedNInvForQuaternion(const Vec4P& q) {
        const Vec4P e = 2*q;
        const RealP ne1 = -e[1], ne2 = -e[2], ne3 = -e[3];
        return Mat<3,4,P>(ne1, e[0], ne3,  e[2],
                          ne2, e[3], e[0], ne1,
                          ne3, ne2,  e[1], e[0]);
    }


    /// Given a possibly unnormalized quaternion (0th element is the scalar) and the
    /// relative angular velocity vector of B in its parent, expressed 
    /// in the *PARENT*, return the quaternion derivatives. This is never singular.
    /// Cost is 27 flops.
    static Vec4P convertAngVelToQuaternionDot(const Vec4P& q, const Vec3P& w_PB_P) {
        return calcUnnormalizedNForQuaternion(q)*w_PB_P;
    }

    /// Inverse of the above routine. Returned AngVel is expressed in
    /// the *PARENT* frame: w_PB_P.
    /// Cost is 28 flops.
    static Vec3P convertQuaternionDotToAngVel(const Vec4P& q, const Vec4P& qdot) {
        return calcUnnormalizedNInvForQuaternion(q)*qdot;
    }

    /// Given a quaternion q representing R_PB, angular velocity of B in P, and
    /// the time derivative of the angular velocity, return the second time 
    /// derivative qdotdot of the quaternion. Everything is measured and 
    /// expressed in the parent.
    /// Cost is 78 flops.
    static Vec4P convertAngVelDotToQuaternionDotDot
       (const Vec4P& q, const Vec3P& w_PB_P, const Vec3P& wdot_PB_P)
    {
        const Mat<4,3,P> N    = calcUnnormalizedNForQuaternion(q);
        const Vec4P      qdot = N*w_PB_P;   // 20 flops
        const Mat<4,3,P> NDot = calcUnnormalizedNDotForQuaternion(qdot);

        return  N*wdot_PB_P + NDot*w_PB_P;  // 44 flops
    }


private:
    // This is only for the most trustworthy of callers, that is, methods of 
    // the Rotation_ class.  There are a lot of ways for this NOT to be a 
    // legitimate rotation matrix -- be careful!!
    // Note that these are supplied in rows.
    Rotation_( const RealP& xx, const RealP& xy, const RealP& xz,
               const RealP& yx, const RealP& yy, const RealP& yz,
               const RealP& zx, const RealP& zy, const RealP& zz )
    :   Mat33P( xx,xy,xz, yx,yy,yz, zx,zy,zz ) {}

    // These next methods are highly-efficient power-user methods. Read the 
    // code to understand them.
    SimTK_SimTKCOMMON_EXPORT Rotation_&  setTwoAngleTwoAxesBodyFixedForwardCyclicalRotation(     RealP cosAngle1, RealP sinAngle1, const CoordinateAxis& axis1, RealP cosAngle2, RealP sinAngle2, const CoordinateAxis& axis2 );
    SimTK_SimTKCOMMON_EXPORT Rotation_&  setThreeAngleTwoAxesBodyFixedForwardCyclicalRotation(   RealP cosAngle1, RealP sinAngle1, const CoordinateAxis& axis1, RealP cosAngle2, RealP sinAngle2, const CoordinateAxis& axis2, RealP cosAngle3, RealP sinAngle3 );
    SimTK_SimTKCOMMON_EXPORT Rotation_&  setThreeAngleThreeAxesBodyFixedForwardCyclicalRotation( RealP cosAngle1, RealP sinAngle1, const CoordinateAxis& axis1, RealP cosAngle2, RealP sinAngle2, const CoordinateAxis& axis2, RealP cosAngle3, RealP sinAngle3, const CoordinateAxis& axis3 );

    // These next methods are highly-efficient power-user methods to convert 
    // Rotation matrices to orientation angles.  Read the code to understand them.
    SimTK_SimTKCOMMON_EXPORT Vec2P  convertTwoAxesBodyFixedRotationToTwoAngles(     const CoordinateAxis& axis1, const CoordinateAxis& axis2 ) const;
    SimTK_SimTKCOMMON_EXPORT Vec3P  convertTwoAxesBodyFixedRotationToThreeAngles(   const CoordinateAxis& axis1, const CoordinateAxis& axis2 ) const;
    SimTK_SimTKCOMMON_EXPORT Vec3P  convertThreeAxesBodyFixedRotationToThreeAngles( const CoordinateAxis& axis1, const CoordinateAxis& axis2, const CoordinateAxis& axis3 ) const;

//------------------------------------------------------------------------------
// These are obsolete names from a previous release, listed here so that 
// users will get a decipherable compilation error. (sherm 091101)
//------------------------------------------------------------------------------
private:
    // REPLACED BY: calcNForBodyXYZInBodyFrame()
    static Mat33P calcQBlockForBodyXYZInBodyFrame(const Vec3P& a)
    {   return calcNForBodyXYZInBodyFrame(a); }
    // REPLACED BY: calcNInvForBodyXYZInBodyFrame()
    static Mat33P calcQInvBlockForBodyXYZInBodyFrame(const Vec3P& a)
    {   return calcNInvForBodyXYZInBodyFrame(a); }
    // REPLACED BY: calcUnnormalizedNForQuaternion()
    static Mat<4,3,P> calcUnnormalizedQBlockForQuaternion(const Vec4P& q)
    {   return calcUnnormalizedNForQuaternion(q); }
    // REPLACED BY: calcUnnormalizedNInvForQuaternion()
    static Mat<3,4,P> calcUnnormalizedQInvBlockForQuaternion(const Vec4P& q)
    {   return calcUnnormalizedNInvForQuaternion(q); }
    // REPLACED BY: convertAngVelInBodyFrameToBodyXYZDot
    static Vec3P convertAngVelToBodyFixed123Dot(const Vec3P& q, const Vec3P& w_PB_B) 
    {   return convertAngVelInBodyFrameToBodyXYZDot(q,w_PB_B); }
    // REPLACED BY: convertBodyXYZDotToAngVelInBodyFrame
    static Vec3P convertBodyFixed123DotToAngVel(const Vec3P& q, const Vec3P& qdot) 
    {   return convertBodyXYZDotToAngVelInBodyFrame(q,qdot); }
    // REPLACED BY: convertAngVelDotInBodyFrameToBodyXYZDotDot
    static Vec3P convertAngVelDotToBodyFixed123DotDot
        (const Vec3P& q, const Vec3P& w_PB_B, const Vec3P& wdot_PB_B)
    {   return convertAngVelDotInBodyFrameToBodyXYZDotDot(q,w_PB_B,wdot_PB_B); }

//------------------------------------------------------------------------------
// The following code is obsolete - it is here temporarily for backward 
// compatibility (Mitiguy 9/5/2007)
//------------------------------------------------------------------------------
private:
    // These static methods are like constructors with friendlier names.
    static Rotation_ zero() { return Rotation_(); }
    static Rotation_ NaN()  { Rotation_ r;  r.setRotationToNaN();  return r; }

    /// By zero we mean "zero rotation", i.e., an identity matrix.
    Rotation_&  setToZero()            { return setRotationToIdentityMatrix(); }
    Rotation_&  setToIdentityMatrix()  { return setRotationToIdentityMatrix(); }
    Rotation_&  setToNaN()             { return setRotationToNaN(); }
    static Rotation_  trustMe( const Mat33P& m )  { return Rotation_(m,true); }

    // One-angle rotations.
    static Rotation_ aboutX( const RealP& angleInRad ) { return Rotation_( angleInRad, XAxis ); }
    static Rotation_ aboutY( const RealP& angleInRad ) { return Rotation_( angleInRad, YAxis ); }
    static Rotation_ aboutZ( const RealP& angleInRad ) { return Rotation_( angleInRad, ZAxis ); }
    static Rotation_ aboutAxis( const RealP& angleInRad, const UnitVec3P& axis ) { return Rotation_(angleInRad,axis); }
    static Rotation_ aboutAxis( const RealP& angleInRad, const Vec3P& axis )     { return Rotation_(angleInRad,axis); }
    void  setToRotationAboutZ( const RealP& q ) { setRotationFromAngleAboutZ( q ); }

    // Two-angle space-fixed rotations.
    static Rotation_ aboutXThenOldY(const RealP& xInRad, const RealP& yInRad) { return Rotation_( SpaceRotationSequence, xInRad, XAxis, yInRad, YAxis ); }
    static Rotation_ aboutYThenOldX(const RealP& yInRad, const RealP& xInRad) { return Rotation_( SpaceRotationSequence, yInRad, YAxis, xInRad, XAxis ); }
    static Rotation_ aboutXThenOldZ(const RealP& xInRad, const RealP& zInRad) { return Rotation_( SpaceRotationSequence, xInRad, XAxis, zInRad, ZAxis ); }
    static Rotation_ aboutZThenOldX(const RealP& zInRad, const RealP& xInRad) { return Rotation_( SpaceRotationSequence, zInRad, ZAxis, xInRad, XAxis ); }
    static Rotation_ aboutYThenOldZ(const RealP& yInRad, const RealP& zInRad) { return Rotation_( SpaceRotationSequence, yInRad, YAxis, zInRad, ZAxis ); }
    static Rotation_ aboutZThenOldY(const RealP& zInRad, const RealP& yInRad) { return Rotation_( SpaceRotationSequence, zInRad, ZAxis, yInRad, YAxis ); }

    // Two-angle body fixed rotations (reversed space-fixed ones).
    static Rotation_ aboutXThenNewY(const RealP& xInRad, const RealP& yInRad) { return Rotation_( BodyRotationSequence, xInRad, XAxis, yInRad, YAxis ); }
    static Rotation_ aboutYThenNewX(const RealP& yInRad, const RealP& xInRad) { return aboutXThenOldY(xInRad, yInRad); }
    static Rotation_ aboutXThenNewZ(const RealP& xInRad, const RealP& zInRad) { return aboutZThenOldX(zInRad, xInRad); }
    static Rotation_ aboutZThenNewX(const RealP& zInRad, const RealP& xInRad) { return aboutXThenOldZ(xInRad, zInRad); }
    static Rotation_ aboutYThenNewZ(const RealP& yInRad, const RealP& zInRad) { return aboutZThenOldY(zInRad, yInRad); }
    static Rotation_ aboutZThenNewY(const RealP& zInRad, const RealP& yInRad) { return aboutYThenOldZ(yInRad, zInRad); }

    /// Create a Rotation_ matrix by specifying only its z axis. 
    /// This will work for any stride UnitVec because there is always an implicit conversion available to the packed form used as the argument.
    explicit Rotation_( const UnitVec3P& uvecZ )  { setRotationFromOneAxis(uvecZ,ZAxis); }

    /// Create a Rotation_ matrix by specifying its x axis, and a "y like" axis. 
    //  We will take x seriously after normalizing, but use the y only to create z = normalize(x X y), 
    //  then y = z X x. Bad things happen if x and y are aligned but we may not catch it.
    Rotation_( const Vec3P& x, const Vec3P& yish )  { setRotationFromTwoAxes( UnitVec3P(x), XAxis, yish, YAxis ); }

    /// Set this Rotation_ to represent the same rotation as the passed-in quaternion.
    void setToQuaternion( const QuaternionP& q )  { setRotationFromQuaternion(q); }

    /// Set this Rotation_ to represent a rotation of +q0 about the body frame's Z axis, 
    /// followed by a rotation of +q1 about the body frame's NEW Y axis, 
    /// followed by a rotation of +q3 about the body frame's NEW X axis.
    /// See Kane, Spacecraft Dynamics, pg. 423, body-three: 3-2-1.
    //  Similarly for BodyFixed123.
    void setToBodyFixed321( const Vec3P& v)  { setRotationFromThreeAnglesThreeAxes( BodyRotationSequence, v[0], ZAxis, v[1], YAxis, v[2], XAxis ); }
    void setToBodyFixed123( const Vec3P& v)  { setRotationToBodyFixedXYZ(v); }

    /// Convert this Rotation_ matrix to an equivalent (angle,axis) representation: 
    /// The returned Vec4P is [angleInRadians, unitVectorX, unitVectorY, unitVectorZ].
    Vec4P convertToAngleAxis() const  { return convertRotationToAngleAxis(); }

    /// Convert this Rotation_ matrix to equivalent quaternion representation.
    QuaternionP convertToQuaternion() const  { return convertRotationToQuaternion(); }

    /// Set this Rotation_ to represent a rotation of +q0 about the base frame's X axis, 
    /// followed by a rotation of +q1 about the base frame's (unchanged) Y axis.
    void setToSpaceFixed12( const Vec2P& q ) { setRotationFromTwoAnglesTwoAxes( SpaceRotationSequence, q[0], XAxis, q[1], YAxis ); }

    /// Convert this Rotation_ matrix to the equivalent 1-2-3 body fixed Euler angle sequence.
    /// Similarly, convert Rotation_ matrix to the equivalent 1-2 body  fixed Euler angle sequence. 
    /// Similarly, convert Rotation_ matrix to the equivalent 1-2 space fixed Euler angle sequence. 
    Vec3P  convertToBodyFixed123() const  { return convertRotationToBodyFixedXYZ(); }
    Vec2P  convertToBodyFixed12() const   { return convertRotationToBodyFixedXY(); }
    Vec2P  convertToSpaceFixed12() const  { return convertTwoAxesRotationToTwoAngles( SpaceRotationSequence, XAxis, YAxis ); }
};


///-----------------------------------------------------------------------------
///  This InverseRotation class is the inverse of a Rotation 
///  See the Rotation class for information.
///-----------------------------------------------------------------------------
template <class P>
class InverseRotation_ : public Mat<3,3,P>::TransposeType {
    typedef P               RealP;
    typedef Rotation_<P>    RotationP;
    typedef Mat<3,3,P>      Mat33P; // not the base type!
    typedef SymMat<3,P>     SymMat33P;
    typedef Mat<2,2,P>      Mat22P;
    typedef Mat<3,2,P>      Mat32P;
    typedef Vec<2,P>        Vec2P;
    typedef Vec<3,P>        Vec3P;
    typedef Vec<4,P>        Vec4P;
    typedef Quaternion_<P>  QuaternionP;
public:
    /// This is the type of the underlying 3x3 matrix; note that it will have
    /// unusual row and column spacing since we're viewing it as transposed.
    typedef typename Mat<3,3,P>::TransposeType  BaseMat;

    /// Note that the unit vectors representing the rows and columns of this
    /// matrix do not necessarily have unit stride.
    //@{
    /// This is the type of a column of this InverseRotation.
    typedef  UnitVec<P,BaseMat::RowSpacing>  ColType;
    /// This is the type of a row of this InverseRotation.
    typedef  UnitRow<P,BaseMat::ColSpacing>  RowType;
    //@}

    /// You should not ever construct one of these as they should only occur as expression 
    /// intermediates resulting from use of the "~" operator on a Rotation.
    /// But if you must, the default will produce an identity rotation.
    InverseRotation_() : BaseMat(1) {}

    /// This is an explicit implementation of the default copy constructor.
    InverseRotation_( const InverseRotation_& R ) : BaseMat(R) {}
    /// This is an explicit implementation of the default copy assignment operator.
    InverseRotation_&  operator=( const InverseRotation_& R )  
    {   BaseMat::operator=(R.asMat33());  return *this; }

    /// Assuming this InverseRotation_ is R_AB, and given a symmetric dyadic matrix S_BB expressed 
    /// in B, we can reexpress it in A using S_AA=R_AB*S_BB*R_BA. The matrix should be one
    /// that is formed as products of vectors expressed in A, such as inertia, gyration or
    /// covariance matrices. This can be done efficiently exploiting properties of R and S.
    /// Cost is 57 flops.
    /// @see Rotation::reexpressSymMat33()
    SimTK_SimTKCOMMON_EXPORT SymMat33P reexpressSymMat33(const SymMat33P& S_BB) const;

    /// We can invert an InverseRotation just by recasting it to a Rotation at zero cost.
    //@{
    const RotationP&  invert() const {return *reinterpret_cast<const RotationP*>(this);}
    RotationP&        updInvert() {return *reinterpret_cast<RotationP*>(this);}
    //@}

    /// Transpose, and transpose operators (override BaseMat versions of transpose).
    /// For an orthogonal matrix like this one transpose is the same as inverse.
    //@{
    const RotationP&  transpose() const  { return invert(); }
    const RotationP&  operator~() const  { return invert(); }
    RotationP&        updTranspose()     { return updInvert(); }
    RotationP&        operator~()        { return updInvert(); }
    //@}

    /// Access individual rows and columns of this InverseRotation; no cost or
    /// copying since suitably-cast references to the actual data are returned.
    /// There are no writable versions of these methods since changing a single
    /// row or column would violate the contract that these are always legitimate
    /// rotation matrices.
    //@{
    const RowType&  row( int i ) const         { return reinterpret_cast<const RowType&>(asMat33()[i]); }
    const ColType&  col( int j ) const         { return reinterpret_cast<const ColType&>(asMat33()(j)); }
    const ColType&  x() const                  { return col(0); }
    const ColType&  y() const                  { return col(1); }
    const ColType&  z() const                  { return col(2); }
    const RowType&  operator[]( int i ) const  { return row(i); }
    const ColType&  operator()( int j ) const  { return col(j); }
    //@}

    /// Conversion from InverseRotation_ to BaseMat.
    /// Note: asMat33 is more efficient than toMat33() (no copy), but you have to know the internal layout.
    //@{
    const BaseMat&  asMat33() const  { return *static_cast<const BaseMat*>(this); }
    BaseMat         toMat33() const  { return asMat33(); }
    //@}
};

/// Write a Rotation matrix to an output stream by writing out its underlying Mat33.
template <class P> SimTK_SimTKCOMMON_EXPORT std::ostream& 
operator<<(std::ostream&, const Rotation_<P>&);
/// Write an InverseRotation matrix to an output stream by writing out its underlying Mat33.
template <class P> SimTK_SimTKCOMMON_EXPORT std::ostream& 
operator<<(std::ostream&, const InverseRotation_<P>&);

/// Rotating a unit vector leaves it unit length, saving us from having to perform
/// an expensive normalization. So we override the multiply operators here changing
/// the return type to UnitVec or UnitRow.
//@{
template <class P, int S> inline UnitVec<P,1>  
operator*(const Rotation_<P>& R, const UnitVec<P,S>& v)        {return UnitVec<P,1>(R.asMat33()* v.asVec3(),  true);}
template <class P, int S> inline UnitRow<P,1>  
operator*(const UnitRow<P,S>& r, const Rotation_<P>& R)        {return UnitRow<P,1>(r.asRow3() * R.asMat33(), true);}
template <class P, int S> inline UnitVec<P,1>  
operator*(const InverseRotation_<P>& R, const UnitVec<P,S>& v) {return UnitVec<P,1>(R.asMat33()* v.asVec3(),  true);}
template <class P, int S> inline UnitRow<P,1>  
operator*(const UnitRow<P,S>& r, const InverseRotation_<P>& R) {return UnitRow<P,1>(r.asRow3() * R.asMat33(), true);}
//@}

// Couldn't implement these Rotation_ methods until InverseRotation_ was defined.
template <class P> inline
Rotation_<P>::Rotation_(const InverseRotation_<P>& R) : Mat<3,3,P>( R.asMat33() ) {}
template <class P> inline Rotation_<P>&  
Rotation_<P>::operator=(const InverseRotation_<P>& R)  {static_cast<Mat<3,3,P>&>(*this)  = R.asMat33();    return *this;}
template <class P> inline Rotation_<P>&  
Rotation_<P>::operator*=(const Rotation_<P>& R)        {static_cast<Mat<3,3,P>&>(*this) *= R.asMat33();    return *this;}
template <class P> inline Rotation_<P>&  
Rotation_<P>::operator/=(const Rotation_<P>& R)        {static_cast<Mat<3,3,P>&>(*this) *= (~R).asMat33(); return *this;}
template <class P> inline Rotation_<P>&  
Rotation_<P>::operator*=(const InverseRotation_<P>& R) {static_cast<Mat<3,3,P>&>(*this) *= R.asMat33();    return *this;}
template <class P> inline Rotation_<P>&  
Rotation_<P>::operator/=(const InverseRotation_<P>& R) {static_cast<Mat<3,3,P>&>(*this) *= (~R).asMat33(); return *this;}

/// Composition of Rotation matrices via operator*.
//@{
template <class P> inline Rotation_<P>
operator*(const Rotation_<P>&        R1, const Rotation_<P>&        R2)  {return Rotation_<P>(R1) *= R2;}
template <class P> inline Rotation_<P>
operator*(const Rotation_<P>&        R1, const InverseRotation_<P>& R2)  {return Rotation_<P>(R1) *= R2;}
template <class P> inline Rotation_<P>
operator*(const InverseRotation_<P>& R1, const Rotation_<P>&        R2)  {return Rotation_<P>(R1) *= R2;}
template <class P> inline Rotation_<P>
operator*(const InverseRotation_<P>& R1, const InverseRotation_<P>& R2)  {return Rotation_<P>(R1) *= R2;}
//@}

/// Composition of a Rotation matrix and the inverse of another Rotation via operator/, that is
/// R1/R2 == R1*(~R2).
//@{
template <class P> inline Rotation_<P>
operator/( const Rotation_<P>&        R1, const Rotation_<P>&        R2 )  {return Rotation_<P>(R1) /= R2;}
template <class P> inline Rotation_<P>
operator/( const Rotation_<P>&        R1, const InverseRotation&     R2 )  {return Rotation_<P>(R1) /= R2;}
template <class P> inline Rotation_<P>
operator/( const InverseRotation_<P>& R1, const Rotation_<P>&        R2 )  {return Rotation_<P>(R1) /= R2;}
template <class P> inline Rotation_<P>
operator/( const InverseRotation_<P>& R1, const InverseRotation_<P>& R2 )  {return Rotation_<P>(R1) /= R2;}
//@}


//------------------------------------------------------------------------------
}  // End of namespace SimTK

//--------------------------------------------------------------------------
#endif // SIMTK_ROTATION_H_
//--------------------------------------------------------------------------


