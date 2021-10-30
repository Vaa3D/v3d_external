/*
 * WireCube.cpp
 *
 *  Created on: Nov 6, 2012
 *      Author: Christopher M. Bruns
 */

#include "WireCube.h"
#include "../3drenderer/GLee_r.h"

WireCube::WireCube()
{
    // TODO Auto-generated constructor stub

}

WireCube::~WireCube()
{
    // TODO Auto-generated destructor stub
}

/* virtual */
void WireCube::paintGL()
{
    float s = 50.0;
    glBegin(GL_QUADS);
    glVertex3f(-s, -s, -s);
    glVertex3f( s, -s, -s);
    glVertex3f( s,  s, -s);
    glVertex3f(-s,  s, -s);
    glEnd();
}
