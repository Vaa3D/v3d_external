#include "CameraModel.h"

CameraModel::CameraModel()
    : m_scale(1.0)
    , m_focusPosition(0, 0, 0)
{}

void CameraModel::setFocus(const Vector3D& v)
{
    if (v == m_focusPosition) return; // no change
    if (! (v == v)) return; // NaN
    m_focusPosition = v;
    emit focusChanged(m_focusPosition);
}

void CameraModel::setScale(qreal s)
{
    if (s == m_scale) return; // no change
    if (! (s == s)) return; // NaN
    m_scale = s;
    emit scaleChanged(m_scale);
}

void CameraModel::setRotation(const Rotation3D& R)
{
    if (R == m_rotation) return; // no change
    if (! ( R == R )) return; // avoid NaN update
    m_rotation = R;
    emit rotationChanged(m_rotation);
}


