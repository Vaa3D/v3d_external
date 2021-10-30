/*
 * Camera3D.h
 *
 *  Created on: Nov 15, 2012
 *      Author: Christopher M. Bruns
 */

#ifndef CAMERA3D_H_
#define CAMERA3D_H_

#include "../geometry/Vector3D.h"
#include "../geometry/Rotation3D.h"
#include <QObject>

class Camera3D;

/**
 * Stack allocated object that temporarily sets OpenGL camera position
 */
class CameraSetterGL
{
public:
    CameraSetterGL(const Camera3D& camera);
    virtual ~CameraSetterGL();
};

/**
 * Stores position and orientation of virtual camera and raster viewport
 */
class Camera3D : public QObject
{
    Q_OBJECT

public:
    Camera3D();
    virtual ~Camera3D();

    double getAspectRatio() const
    {
        double result = 1.0;
        if (viewportHeightPixels > 0)
            result = viewportWidthPixels / (double) viewportHeightPixels;
        return result;
    }

    double getFieldOfViewYDegrees() const {
        double pixelHalfHeight = viewportHeightPixels * 0.5;
        double pixelDistance = focusDistancePixels;
        return std::abs(2.0 * atan2(pixelHalfHeight, pixelDistance) * 180.0 / 3.14159);
    }

    double getFocusDistanceMicrometers() const {
        return focusDistanceMicrometers;
    }

    const Vector3D& getFocusInGroundMicrometers() const {
        return focusInGroundMicrometers;
    }

    bool getKeepYUp() const {
        return keepYUp;
    }

    const Rotation3D& getRotation() const {
        return rotation;
    }

    double getViewportHeightPixels() const
    {
        return viewportHeightPixels;
    }

    double getViewportWidthPixels() const
    {
        return viewportWidthPixels;
    }

    double getZFarMicrometers() const
    {
        return std::abs(rearClipRelative * focusDistanceMicrometers);
    }

    double getZNearMicrometers() const
    {
        return std::abs(frontClipRelative * focusDistanceMicrometers);
    }

    void setFocusDistanceMicrometers(double distance)
    {
        if (distance == focusDistanceMicrometers)
            return;
        focusDistanceMicrometers = distance;
        emit focusDistanceChanged(distance);
    }

    void setRotation(const Rotation3D& rotation)
    {
        this->rotation = rotation;
        emit rotationChanged(rotation);
    }

    void setViewportPixels(int width, int height)
    {
        // Adjust camera on resize to keep a similar view
        if (  (height > 0)
                && (viewportHeightPixels > 0)
                && (height != viewportHeightPixels))
        {
            double ratio = height / (double) viewportHeightPixels;
            // micrometersPerPixel *= ratio;
            focusDistanceMicrometers /= ratio; // dolly in to enlarge view
        }
        viewportWidthPixels = width;
        viewportHeightPixels = height;
    }

    void translatePixel(double x, double y, double z)
    {
        Vector3D diff(x, y, z);
        if (diff == Vector3D(0,0,0))
            return;
        // convert from pixels to micrometers
        diff *= focusDistanceMicrometers / focusDistancePixels;
        diff = getRotation() * diff;
        focusInGroundMicrometers += diff;
        emit focusChanged(focusInGroundMicrometers);
    }

signals:
    void focusChanged(Vector3D);
    void rotationChanged(Rotation3D);
    void focusDistanceChanged(double);

protected:
    Vector3D focusInGroundMicrometers; // pan
    Rotation3D rotation; // about focus; TODO - camera to ground or vice versa?
    double focusDistanceMicrometers; // dolly
    // double micrometersPerPixel; // scale
    double focusDistancePixels;
    int viewportWidthPixels;
    int viewportHeightPixels;
    double frontClipRelative;
    double rearClipRelative;
    bool keepYUp;
};

#endif /* CAMERA3D_H_ */
