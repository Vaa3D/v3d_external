#ifndef NEURON_ANNOTATOR_ANIMATE_ANIMATION_FRAME_H_
#define NEURON_ANNOTATOR_ANIMATE_ANIMATION_FRAME_H_

#include <cmath>
#include <limits>
#include "../geometry/Vector3D.h"
#include "../geometry/Rotation3D.h"
#include "../geometry/CameraModel.h"
#include "../data_model/DataColorModel.h"
#include "../../basic_c_fun/basic_surf_objs.h"

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
    AnimationFrame();

    AnimationFrame catmullRomInterpolateFrame(
        const AnimationFrame& p0, const AnimationFrame& p2, const AnimationFrame& p3, 
        double t) const;

    void retrieveCameraSettings(CameraModel& camera) const;
    void retrieveLandmarkVisibility(QList<ImageMarker>& landmarks) const;

    void storeCameraSettings(const CameraModel& camera);
    void storeChannelColorModel(const DataColorModel::Reader& reader);
    void storeLandmarkVisibility(const QList<ImageMarker>& landmarks);

private:
    std::vector<double> landmarkVisibility;
};

#endif
