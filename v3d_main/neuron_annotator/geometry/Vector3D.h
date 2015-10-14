#ifndef VECTOR3D_CMB_H
#define VECTOR3D_CMB_H

#include <QtCore>
#include <cmath>
#include <iostream>

class Vector3D;
class UnitVector3D;

// BaseVector3D is the common base class of Vector3D and UnitVector3D
// Many methods are defined in this header so they can be inlined by the compiler.
// I would prefer to avoid virtual methods in these classes to keep the memory layout simple.
class BaseVector3D
{
public:
    BaseVector3D(qreal px, qreal py, qreal pz) : m_x(px), m_y(py), m_z(pz) {}
    BaseVector3D(const BaseVector3D& v) : m_x(v.m_x), m_y(v.m_y), m_z(v.m_z) {}

    qreal x() const {return m_x;}
    qreal y() const {return m_y;}
    qreal z() const {return m_z;}
    qreal operator[](int i) const {return data[i];}
    bool operator==(const BaseVector3D& rhs) const {
        return ((m_x == rhs.m_x)
             && (m_y == rhs.m_y)
             && (m_z == rhs.m_z));
    }
    bool operator!=(const BaseVector3D& rhs) const {
        return ((m_x != rhs.m_x)
             || (m_y != rhs.m_y)
             || (m_z != rhs.m_z));
    }
    qreal dot(const BaseVector3D& rhs) const {
        return m_x * rhs.m_x + m_y * rhs.m_y + m_z * rhs.m_z;
    }
    qreal normSquared() const {
        return this->dot(*this);
    }
    qreal norm() const {
        return std::sqrt(normSquared());
    }

protected:
    BaseVector3D& operator/=(qreal s) {
        m_x /= s;
        m_y /= s;
        m_z /= s;
        return *this;
    }

    BaseVector3D& operator*=(qreal s) {
        m_x *= s;
        m_y *= s;
        m_z *= s;
        return *this;
    }

    BaseVector3D& negate() {
        (*this) *= -1;
        return *this;
    }

    // Writeable access is OK for Vector3D, but not for UnitVector3D
    // so these methods are protected in the base class BaseVector3D
    qreal& operator[](int i) {return data[i];}
    qreal& x() {return m_x;}
    qreal& y() {return m_y;}
    qreal& z() {return m_z;}

public: //added by PHC, 20130424 to make it possible to build for Vaa3D
    union {
        qreal data[3];
        struct {qreal m_x, m_y, m_z;};
    };
};

class Vector3D : public BaseVector3D
{
public:
    Vector3D() : BaseVector3D(0,0,0) {}
    Vector3D(qreal px, qreal py, qreal pz) : BaseVector3D(px,py,pz) {}

    // Expose writeable accessors, unlike UnitVector3D
    using BaseVector3D::x;
    using BaseVector3D::y;
    using BaseVector3D::z;
    using BaseVector3D::operator[];

    // Methods that return Vector3D reference should not be defined in BaseVector3D
    Vector3D& operator+=(const BaseVector3D& rhs) {
        x() += rhs.x();
        y() += rhs.y();
        z() += rhs.z();
        return *this;
    }
    Vector3D& operator-=(const BaseVector3D& rhs) {
        x() -= rhs.x();
        y() -= rhs.y();
        z() -= rhs.z();
        return *this;
    }
    Vector3D& operator*=(qreal s) {
        x() *= s;
        y() *= s;
        z() *= s;
        return *this;
    }
    Vector3D& operator/=(qreal s) {
        x() /= s;
        y() /= s;
        z() /= s;
        return *this;
    }

    Vector3D operator-() const {
        Vector3D result = *this;
        result.negate();
        return result;
    }

    Vector3D cross(const BaseVector3D& rhs) const {
        return Vector3D(
                y()*rhs.z() - z()*rhs.y(),
                z()*rhs.x() - x()*rhs.z(),
                x()*rhs.y() - y()*rhs.x());
    }
};

// Length-modifying operators must return a Vector3D, even if a UnitVector3D was an argument
// scalar-times-vector cannot be a member operator
inline Vector3D operator*(qreal s, const BaseVector3D& rhs) {
    return Vector3D(s*rhs.x(), s*rhs.y(), s*rhs.z());
}
inline Vector3D operator*(const BaseVector3D& lhs, qreal s) {
    return Vector3D(lhs.x()*s, lhs.y()*s, lhs.z()*s);
}
inline Vector3D operator/(const BaseVector3D& lhs, qreal s) {
    return Vector3D(lhs.x()/s, lhs.y()/s, lhs.z()/s);
}
inline Vector3D operator+(const Vector3D& lhs, const Vector3D& rhs) {
    return Vector3D(lhs.x() + rhs.x(),
                    lhs.y() + rhs.y(),
                    lhs.z() + rhs.z());
}
inline Vector3D operator-(const Vector3D& lhs, const Vector3D& rhs) {
    return Vector3D(lhs.x() - rhs.x(),
                    lhs.y() - rhs.y(),
                    lhs.z() - rhs.z());
}

// UnitVector3D has a length of 1.0
// Writable accessors are protected to preserve this property
class UnitVector3D : public BaseVector3D
{
public:
    UnitVector3D() : BaseVector3D(1, 0, 0) {}
    UnitVector3D(const UnitVector3D& v) : BaseVector3D(v) {}
    // Expensive constructors scale to unit value
    UnitVector3D(const Vector3D& v) : BaseVector3D(v / v.norm()) {}
    UnitVector3D(qreal px, qreal py, qreal pz) : BaseVector3D(px, py, pz)
    {
        (*this) /= BaseVector3D::norm(); // don't use UnitVector3::norm()!
    }
    UnitVector3D operator-() const {
        UnitVector3D result = *this;
        result.negate();
        return result;
    }


    qreal norm() const {return 1.0;}
    qreal normSquared() const {return 1.0;}
};

inline std::ostream& operator<<(std::ostream& os, const BaseVector3D& v) {
    os << "[" << v[0] << ", " << v[1] << ", " << v[2] << "]";
    return os;
}

#endif /* VECTOR3D_CMB_H */

