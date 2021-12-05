/*
 * CameraTransformGL.cpp
 *
 *  Created on: Nov 6, 2012
 *      Author: Christopher M. Bruns
 */

#include "CameraTransformGL.h"

#include <GL/gl.h>  //2020-2-10 RZC


CameraTransformGL::CameraTransformGL(const Na3DWidget& widget3d)
{
    // adapted from v3dr_glwidget::paintGL()
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    Rotation3D rot = widget3d.getCameraRotationInGround();

    // absolute translation
    {
        Vector3D f = widget3d.getCameraFocusInMicrometers();
        Vector3D df = widget3d.getDefaultFocusInMicrometers();
        f -= df;
        f *= widget3d.glUnitsPerImageVoxel();
        f = rot * f;
        glTranslated(-f.x(), -f.y(), -f.z());
        // cerr << "focus1: " << -f.x() << ", " << -f.y() << ", " << -f.z() << endl;
    }

    // last absolute rotation pose
    GLdouble mRot[16] = {1, 0, 0, 0,
                         0, 1, 0, 0,
                         0, 0, 1, 0,
                         0, 0, 0, 1};
    rot.setGLMatrix(mRot);
    glMultMatrixd(mRot);

    // Y axis down
    const int flip_X= +1, flip_Y= -1, flip_Z= -1;
    glScaled(flip_X,flip_Y,flip_Z); // make y-axis downward conformed with image coordinate

    // Scale back from "volume" coordinates to orthogonal micrometers
    double scale = widget3d.glUnitsPerImageVoxel();
    glScaled(scale, scale, scale);
}

CameraTransformGL::~CameraTransformGL()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
}
