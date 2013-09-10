#ifndef NEURON_ANNOTATOR_ANIMATE_ANIMATION_FRAME_H_
#define NEURON_ANNOTATOR_ANIMATE_ANIMATION_FRAME_H_

#include <cmath>
#include <limits>
#include "../data_model/DataColorModel.h"

class AnimationFrame {
public:
    double nan;
    Vector3D cameraFocus;
    Quaternion cameraRotation;
    double cameraZoom;
    // Channel visibility
    double channelZeroVisibility;
    double channelOneVisibility;
    double channelTwoVisibility;
    double channelThreeVisibility;

    /**
     * Values set to "NaN" imply that such values should not be used in the animation.
     */
    AnimationFrame() 
        : nan(std::numeric_limits<double>::quiet_NaN())
        , cameraFocus(nan, nan, nan)
        // TODO rotation to nan
        , cameraZoom(nan)
        , channelZeroVisibility(nan)
        , channelOneVisibility(nan)
        , channelTwoVisibility(nan)
        , channelThreeVisibility(nan)
    {}

    void storeCameraSettings(const CameraModel& camera) {
        cameraFocus = camera.focus();
        cameraRotation.setQuaternionFromRotation(camera.rotation());
        cameraZoom = camera.scale();
    }

    void storeChannelColorModel(const DataColorModel::Reader& reader) {
        // Initialize to NaN
        double* cv[] = {&channelZeroVisibility, &channelOneVisibility, &channelTwoVisibility, &channelThreeVisibility};
        for (int i = 0; i < 4; ++i)
            *cv[i] = nan;
        int sc = reader.getNumberOfDataChannels();
        for (int i = 0; i < sc; ++i) {
            if (i >= 4) break; // we only know how to store 4 channels
            if (reader.getChannelVisibility(i))
                *cv[i] = 1.0;
            else
                *cv[i] = 0.0;
        }
    }

    void retrieveCameraSettings(CameraModel& camera) const {
        if (! my_is_nan(cameraFocus[0]))
            camera.setFocus(cameraFocus);
        if (! my_is_nan(cameraRotation[0]))
            camera.setRotation(Rotation3D(cameraRotation));
        if (! my_is_nan(cameraZoom))
            camera.setScale(cameraZoom);
    }

private:
    bool my_is_nan(double x) const {
        return x != x;
    }

};

#endif
