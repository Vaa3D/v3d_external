/*
 * CubeTestActorGL.cpp
 *
 *  Created on: Nov 9, 2012
 *      Author: Christopher M. Bruns
 */

#include "CubeTestActorGL.h"
#include "../../3drenderer/GLee_r.h"
#include <iostream>

using namespace std;

CubeTestActorGL::CubeTestActorGL()
{}

CubeTestActorGL::~CubeTestActorGL()
{}

/* virtual */
void CubeTestActorGL::paintGL()
{
    const float sideLength = 10.0;
    const float s = sideLength / 2.0;
    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_LINE_STRIP);
    glVertex3f( s,  s,  s);
    glVertex3f(-s,  s,  s);
    glVertex3f(-s, -s,  s);
    glVertex3f( s, -s,  s);
    glVertex3f( s,  s,  s);
    glVertex3f( s,  s, -s);
    glVertex3f(-s,  s, -s);
    glVertex3f(-s, -s, -s);
    glVertex3f( s, -s, -s);
    glVertex3f( s,  s, -s);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(-s,  s,  s);
    glVertex3f(-s,  s, -s);
    glVertex3f(-s, -s,  s);
    glVertex3f(-s, -s, -s);
    glVertex3f( s, -s,  s);
    glVertex3f( s, -s, -s);
    glEnd();
    // cerr << "Paint cube" << endl;
}

