/*
 * CubeTestActorGL.h
 *
 *  Created on: Nov 9, 2012
 *      Author: Christopher M. Bruns
 */

#ifndef CUBETESTACTORGL_H_
#define CUBETESTACTORGL_H_

#include "ActorGL.h"

class CubeTestActorGL : public ActorGL
{
public:
    CubeTestActorGL();
    virtual ~CubeTestActorGL();

    virtual void initGL() {}
    virtual void paintGL();
};

#endif /* CUBETESTACTORGL_H_ */
