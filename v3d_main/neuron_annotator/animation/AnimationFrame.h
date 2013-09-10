#ifndef NEURON_ANNOTATOR_ANIMATE_ANIMATION_FRAME_H_
#define NEURON_ANNOTATOR_ANIMATE_ANIMATION_FRAME_H_

#include <cmath>
#include <limits>

class AnimationFrame {
public:
    double nan;
    Vector3D cameraFocus;
    Quaternion cameraRotation;
    double cameraZoom;

    /**
     * Values set to "NaN" imply that such values should not be used in the animation.
     */
    AnimationFrame() 
        : nan(std::numeric_limits<double>::quiet_NaN())
        , cameraFocus(nan, nan, nan)
        // TODO rotation to nan
        , cameraZoom(nan)
    {}

    void storeCameraSettings(const CameraModel& camera) {
        cameraFocus = camera.focus();
        cameraRotation.setQuaternionFromRotation(camera.rotation());
        cameraZoom = camera.scale();
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
