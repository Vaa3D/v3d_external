/*
 * Movie.cpp
 *
 *  Created on: Sep 13, 2012
 *      Author: Christopher M. Bruns
 */

#include "Movie.h"
#include <QElapsedTimer>

namespace jfrc
{

Movie::Movie(float framesPerSecond)
     : framesPerSecond(framesPerSecond)
{
    // TODO Auto-generated constructor stub

}

Movie::~Movie()
{
    // TODO Auto-generated destructor stub
}

void Movie::play() {
    QElapsedTimer timer;
    timer.start();
    float keyFrameElapsed = 0.0;
    std::vector<Frame>::const_iterator fI;
    float keyFrameStartSeconds = 0.0;
    float keyFrameEndSeconds = 0.0;
    for (fI = keyFrames.begin(); fI != keyFrames.end(); ++fI)
    {
        const Frame& keyFrame = *fI;
        keyFrameStartSeconds = keyFrameEndSeconds;
        keyFrameEndSeconds = keyFrameStartSeconds + keyFrame.secondsToNextFrame;
        if (timer.elapsed()/1000.0 > keyFrameEndSeconds)
            continue; // Skip this frame. We are falling behind.
        // TODO
    }
}

}
