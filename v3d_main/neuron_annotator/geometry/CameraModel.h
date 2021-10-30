#ifndef CAMERAMODEL_H
#define CAMERAMODEL_H

#include <QObject>
#include "Rotation3D.h"

class CameraModel : public QObject
{
    Q_OBJECT

public:
    CameraModel();
    qreal scale() const {return m_scale;}
    const Vector3D& focus() const {return m_focusPosition;}
    const Rotation3D& rotation() const {return m_rotation;}

signals:
    void focusChanged(const Vector3D&);
    void rotationChanged(const Rotation3D&);
    void scaleChanged(qreal);
    void viewChanged();

public slots:
    void setFocus(const Vector3D& v);
    void setScale(qreal);
    void setRotation(const Rotation3D&);

protected:
    Vector3D m_focusPosition; // In image volume coordinates

    Rotation3D m_rotation; // only applies to 3D viewer

    // Scale is zoom level, ratio of screen size to viewable object size.
    // Value of 1.0 means viewable objects exactly fill viewport.
    // Larger value means objects appear larger.
    // Smaller value means objects appear smaller.
    // All viewers are assumed to be isotropic with respect to object metric space.
    // (Though volume voxel sizes may be anisotropic)
    qreal m_scale;
};

#endif // CAMERAMODEL_H
