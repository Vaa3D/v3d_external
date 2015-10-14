/*
 * LegacyVolumeActor.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Christopher M. Bruns
 */

#include "LegacyVolumeActor.h"

LegacyVolumeActor::LegacyVolumeActor(RendererPtr renderer)
    : renderer(renderer)
{}

LegacyVolumeActor::~LegacyVolumeActor()
{}

/* virtual */
void LegacyVolumeActor::initGL()
{
    // TODO
}

/* virtual */
void LegacyVolumeActor::paintGL()
{
    double s1 = 1.0 / renderer->glUnitsPerImageVoxel();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScaled(s1, s1, s1);
    renderer->paint();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
