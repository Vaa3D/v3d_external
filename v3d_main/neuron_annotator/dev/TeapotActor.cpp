/*
 * TeapotActor.cpp
 *
 *  Created on: Nov 15, 2012
 *      Author: Christopher M. Bruns
 */

#include "TeapotActor.h"
#include "../../3drenderer/GLee_r.h"
#include "GLUT/glut.h"
#include <iostream>

using namespace std;

TeapotActor::TeapotActor()
    : sizeInMicrometers(100.0)
{}

TeapotActor::~TeapotActor()
{}

/* virtual */
void TeapotActor::initGL()
{}

/* virtual */
void TeapotActor::paintGL()
{
    // due to a bug in glutSolidTeapot, triangle vertices are in CW order
    glPushAttrib(GL_POLYGON_BIT); // remember current GL_FRONT_FACE indictor
    glFrontFace(GL_CW);
    glColor3f(0.40, 0.27, 0.00);
    glutSolidTeapot(sizeInMicrometers);
    glPopAttrib(); // restore GL_FRONT_FACE
}
