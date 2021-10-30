/*
 * TrackballInteractorCamera.cpp
 *
 *  Created on: Nov 15, 2012
 *      Author: Christopher M. Bruns
 */

#include "TrackballInteractorCamera.h"
#include <QDebug>

TrackballInteractorCamera::TrackballInteractorCamera(Camera3D& camera)
    : camera(camera)
    , previousPositionIsValid(false)
{
}

TrackballInteractorCamera::~TrackballInteractorCamera()
{
}

void TrackballInteractorCamera::mouseMoveEvent(QMouseEvent* event)
{
    if (previousPositionIsValid) {
        QPoint diff = event->pos() - previousMousePosition;
        // qDebug() << diff;
        bool doTranslate = false;
        if (event->modifiers() & Qt::ShiftModifier)
            doTranslate = true;
        if (event->buttons() & Qt::MidButton)
            doTranslate = true;
        if (doTranslate)
            camera.translatePixel(diff.x(), diff.y(), 0.0);
        else { // rotate
            double winSize = 0.5 * (
                    camera.getViewportWidthPixels()
                    + camera.getViewportWidthPixels());
            double angle = std::sqrt(diff.x()*diff.x() + diff.y()*diff.y()); // proportional to drag length
            angle = angle / winSize; // scale to size of viewport
            angle = angle * 2.0 * 3.14159; // scale to one full rotation over viewport
            if (angle != 0.0) {
                UnitVector3D axis(diff.y(), -diff.x(), 0.0);
                Rotation3D r;
                r.setRotationFromAngleAboutUnitVector(angle, axis);
                camera.setRotation(camera.getRotation() * r);
            }
        }
    }
    previousMousePosition = event->pos();
    previousPositionIsValid = true;
}

void TrackballInteractorCamera::mouseDoubleClickEvent(QMouseEvent* event)
{
    Vector3D clickPos(event->pos().x(), event->pos().y(), 0.0);
    Vector3D center(0.5 * camera.getViewportWidthPixels(),
            0.5 * camera.getViewportHeightPixels(), 0.0);
    Vector3D diff = center - clickPos;
    camera.translatePixel(diff.x(), diff.y(), diff.z());
}

void TrackballInteractorCamera::mousePressEvent(QMouseEvent* event)
{
    previousMousePosition = event->pos();
    previousPositionIsValid = true;
}

void TrackballInteractorCamera::mouseReleaseEvent(QMouseEvent* event)
{
    previousPositionIsValid = false;
}

void TrackballInteractorCamera::wheelEvent(QWheelEvent* event)
{
    int delta = event->delta();
    double numDegrees = delta / 8.0;
    double numTicks = numDegrees / 15.0;
    if (numTicks == 0.0)
        return;
    double factor = std::pow(1.10, numTicks);
    if (factor == 1.0)
        return;
    camera.setFocusDistanceMicrometers(factor * camera.getFocusDistanceMicrometers());
}
