/*
 * ActorGL.h
 *
 *  Created on: Nov 6, 2012
 *      Author: Christopher M. Bruns
 */

#ifndef ACTORGL_H_
#define ACTORGL_H_

class ActorGL
{
public:
    ActorGL();
    virtual ~ActorGL();

    virtual void initGL() = 0;
    virtual void paintGL() = 0;
    virtual void destroyGL() = 0;
};

#endif /* ACTORGL_H_ */
