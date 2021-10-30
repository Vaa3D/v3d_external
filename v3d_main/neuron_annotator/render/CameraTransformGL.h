/*
 * CameraTransformGL.h
 *
 *  Created on: Nov 6, 2012
 *      Author: Christopher M. Bruns
 */

#ifndef CAMERATRANSFORMGL_H_
#define CAMERATRANSFORMGL_H_

#include "../gui/Na3DWidget.h"

// Stack allocated OpenGL transformation for use during rendering
class CameraTransformGL
{
public:
    CameraTransformGL(const Na3DWidget& widget3d);
    virtual ~CameraTransformGL();

protected:

};

#endif /* CAMERATRANSFORMGL_H_ */
