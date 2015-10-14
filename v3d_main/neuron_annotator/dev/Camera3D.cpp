/*
 * Camera3D.cpp
 *
 *  Created on: Nov 15, 2012
 *      Author: Christopher M. Bruns
 */

#include "Camera3D.h"
#include <iostream>

using namespace std;

CameraSetterGL::CameraSetterGL(const Camera3D& camera)
{
    glViewport(0, 0,
            camera.getViewportWidthPixels(),
            camera.getViewportHeightPixels());

    // Save current transformation
    glPushAttrib(GL_TRANSFORM_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    /*
    gluPerspective(
            camera.getFieldOfViewYDegrees(),
            camera.getAspectRatio(),
            camera.getZNearMicrometers(),
            camera.getZFarMicrometers());
            */
    gluPerspective(
            camera.getFieldOfViewYDegrees(),
            camera.getAspectRatio(),
            camera.getZNearMicrometers(),
            camera.getZFarMicrometers());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Vector3D f = -camera.getFocusInGroundMicrometers();
    // glTranslated(f.x(), f.y(), f.z());
    Vector3D focus = camera.getFocusInGroundMicrometers();
    Vector3D camPos = camera.getRotation() * Vector3D(0, 0, -camera.getFocusDistanceMicrometers());
    Vector3D up;
    if (camera.getKeepYUp())
        up = Vector3D(0, 1, 0); // TODO - does not work
    else
        up = camera.getRotation() * Vector3D(0, 1, 0);
    gluLookAt(camPos.x(), camPos.y(), camPos.z(),
            focus.x(), focus.y(), focus.z(), // focus
            up.x(), up.y(), up.z()); // up vector
}

CameraSetterGL::~CameraSetterGL()
{
    // Restore previous transformation
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

////////////

Camera3D::Camera3D()
    // initialize to non-ridiculous values
    : focusDistanceMicrometers(1000)
    , viewportWidthPixels(1920)
    , viewportHeightPixels(1080)
    , frontClipRelative(0.2)
    , rearClipRelative(10.0)
    , focusDistancePixels(4000)
    , keepYUp(false)
{}

Camera3D::~Camera3D()
{}
