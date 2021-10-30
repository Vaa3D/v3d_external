/*
 * TrackballInteractorCamera.h
 *
 *  Created on: Nov 15, 2012
 *      Author: Christopher M. Bruns
 */

#ifndef TRACKBALLINTERACTORCAMERA_H_
#define TRACKBALLINTERACTORCAMERA_H_

#include "Camera3D.h"
#include <QMouseEvent>

class TrackballInteractorCamera
{
public:
    TrackballInteractorCamera(Camera3D& camera);
    virtual ~TrackballInteractorCamera();

    void mouseMoveEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);

protected:
    Camera3D& camera;
    QPoint previousMousePosition;
    bool previousPositionIsValid;
};

#endif /* TRACKBALLINTERACTORCAMERA_H_ */
