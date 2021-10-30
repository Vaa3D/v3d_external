/*
 * Movie.h
 *
 *  Created on: Sep 13, 2012
 *      Author: Christopher M. Bruns
 */

#ifndef MOVIE_H_
#define MOVIE_H_

#include <vector>
#include "../geometry/Vector3D.h"
#include "../geometry/Rotation3D.h"

namespace jfrc
{

class Movie
{
public:
    class Frame {
    public:
        int keyFrameIndex;
        Vector3D focus;
        Rotation3D rotation;
        float scale;
        float secondsToNextFrame;
    };


public:
    Movie(float framesPerSecond = 24.0);
    virtual ~Movie();
    void play();

protected:
    float framesPerSecond;
    std::vector<Frame> keyFrames;
    int currentKeyFrameIndex;
};

}

#endif /* MOVIE_H_ */
