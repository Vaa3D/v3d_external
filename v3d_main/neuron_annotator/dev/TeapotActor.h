/*
 * TeapotActor.h
 *
 *  Created on: Nov 15, 2012
 *      Author: Christopher M. Bruns
 */

#ifndef TEAPOTACTOR_H_
#define TEAPOTACTOR_H_

#include "../render/ActorGL.h"

class TeapotActor : public ActorGL
{
public:
    TeapotActor();
    virtual ~TeapotActor();
    virtual void initGL();
    virtual void paintGL();

protected:
    double sizeInMicrometers;
};

#endif /* TEAPOTACTOR_H_ */
