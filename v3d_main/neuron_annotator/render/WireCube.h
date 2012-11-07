/*
 * WireCube.h
 *
 *  Created on: Nov 6,  2012
 *      Author: Christopher M. Bruns
 */

#ifndef WIRECUBE_H_
#define WIRECUBE_H_

#include "ActorGL.h"

/**
 * Draws an OpenGL cube in wire frame.
 * Test object during refactoring of rendering infrastructure.
 * Leading to VolumeBrickActor class.
 */
class WireCube : public ActorGL
{
public:
    WireCube();
    virtual ~WireCube();

    virtual void initGL() {}
    virtual void paintGL();
};

#endif /* WIRECUBE_H_ */
