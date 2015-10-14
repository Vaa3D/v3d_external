#ifndef NEURON_ANNOTATOR_ANIMATE_KEY_FRAME_H_
#define NEURON_ANNOTATOR_ANIMATE_KEY_FRAME_H_

#include "AnimationFrame.h"

class KeyFrame : public AnimationFrame {
public:
    KeyFrame(double durationSeconds) 
        : secondsToNextFrame(durationSeconds)
    {}

    double secondsToNextFrame;
};

#endif // NEURON_ANNOTATOR_ANIMATE_KEY_FRAME_H_
