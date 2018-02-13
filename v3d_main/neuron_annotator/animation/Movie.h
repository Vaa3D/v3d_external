#ifndef NEURON_ANNOTATOR_ANIMATION_MOVIE_H_
#define NEURON_ANNOTATOR_ANIMATION_MOVIE_H_

#include "KeyFrame.h"
#include "interpolate.h"

class Movie {
public:
    double secondsPerFrame;

    Movie() 
        : secondsPerFrame(1.0/60.0)
    {
        rewind();
    }

    void appendKeyFrame(KeyFrame frame) {
        keyFrames.push_back(frame);
    }

    void clear() {
        rewind();
        keyFrames.clear();
    }

    AnimationFrame getNextFrame() {
        // Avoid going past final key frame
        int finalIndex = (int)keyFrames.size() - 1;
        if (currentKeyFrameIndex >= finalIndex)
            currentKeyFrameIndex = finalIndex;
        KeyFrame* keyFrame = &keyFrames[currentKeyFrameIndex];

        // Maybe advance to next frame (or more!)
        while (timeElapsedInFrame >= keyFrame->secondsToNextFrame) {
            timeElapsedInFrame -= keyFrame->secondsToNextFrame;
            currentKeyFrameIndex += 1;
            keyFrame = &keyFrames[currentKeyFrameIndex];
        }

        // Interpolate
        double interpolationParameter = 0;
        if (keyFrame->secondsToNextFrame > 0)
            interpolationParameter = timeElapsedInFrame / keyFrame->secondsToNextFrame;
        // indices of four interpolation frames
        // TODO option to loop from final frame to first
        int i1 = currentKeyFrameIndex;
        int i0 = std::max(0, i1 - 1);
        int i2 = std::min(finalIndex, i1 + 1);
        int i3 = std::min(finalIndex, i1 + 2);
        AnimationFrame frame = keyFrames[i1].catmullRomInterpolateFrame(
            keyFrames[i0], keyFrames[i2], keyFrames[i3],
            interpolationParameter);

        // Prepare for next frame
        totalTimeElapsed += secondsPerFrame;
        timeElapsedInFrame += secondsPerFrame;

        // Return interpolated frame
        return frame;
    }

    // Iterator interface for generating movie frames
    bool hasMoreFrames() const {
        if (keyFrames.size() < 1)
            return false;
        if (currentKeyFrameIndex < (keyFrames.size() - 1))
            return true;
        if (timeElapsedInFrame > 0)
            return false;
        return true;
    }

    void rewind() {
        currentKeyFrameIndex = 0;
        timeElapsedInFrame = 0;
        totalTimeElapsed = 0;
    }

private:
    int currentKeyFrameIndex;
    double timeElapsedInFrame;
    double totalTimeElapsed;
    std::vector<KeyFrame> keyFrames;
};

#endif
