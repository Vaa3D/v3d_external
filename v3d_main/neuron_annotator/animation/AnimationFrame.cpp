#include "AnimationFrame.h"
#include "interpolate.h"

static bool my_is_nan(double x) {
    return x != x;
}

AnimationFrame::AnimationFrame() 
    : nan(std::numeric_limits<double>::quiet_NaN())
    , cameraFocus(nan, nan, nan)
    // TODO rotation to nan
    , cameraZoom(nan)
    , channelZeroVisibility(nan)
    , channelOneVisibility(nan)
    , channelTwoVisibility(nan)
    , channelThreeVisibility(nan)
{}

AnimationFrame AnimationFrame::catmullRomInterpolateFrame(
    const AnimationFrame& p0, const AnimationFrame& p2, const AnimationFrame& p3, 
    double t) const 
{
    const AnimationFrame& p1 = *this;

    AnimationFrame result;

    // camera
    result.cameraFocus = catmullRomInterpolate(
        p0.cameraFocus, p1.cameraFocus, p2.cameraFocus, p3.cameraFocus,
        t);
    result.cameraRotation = catmullRomInterpolate(
        p0.cameraRotation, p1.cameraRotation, p2.cameraRotation, p3.cameraRotation,
        t);
    result.cameraZoom = catmullRomInterpolate(
        p0.cameraZoom, p1.cameraZoom, p2.cameraZoom, p3.cameraZoom,
        t);

    // channel visibility
    result.channelZeroVisibility = catmullRomInterpolate(
        p0.channelZeroVisibility, p1.channelZeroVisibility, p2.channelZeroVisibility, p3.channelZeroVisibility,
        t);
    result.channelOneVisibility = catmullRomInterpolate(
        p0.channelOneVisibility, p1.channelOneVisibility, p2.channelOneVisibility, p3.channelOneVisibility,
        t);
    result.channelTwoVisibility = catmullRomInterpolate(
        p0.channelTwoVisibility, p1.channelTwoVisibility, p2.channelTwoVisibility, p3.channelTwoVisibility,
        t);
    result.channelThreeVisibility = catmullRomInterpolate(
        p0.channelThreeVisibility, p1.channelThreeVisibility, p2.channelThreeVisibility, p3.channelThreeVisibility,
        t);

    // landmark visibility
    result.landmarkVisibility = catmullRomInterpolate(
        p0.landmarkVisibility, p1.landmarkVisibility, p2.landmarkVisibility, p3.landmarkVisibility,
        t);

    return result;
}

void AnimationFrame::storeCameraSettings(const CameraModel& camera) {
    cameraFocus = camera.focus();
    cameraRotation.setQuaternionFromRotation(camera.rotation());
    cameraZoom = camera.scale();
}

void AnimationFrame::storeChannelColorModel(const DataColorModel::Reader& reader) {
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

void AnimationFrame::storeLandmarkVisibility(const QList<ImageMarker>& landmarksParam) 
{
    landmarkVisibility.clear();
    QList<ImageMarker>::const_iterator i;
    for (i = landmarksParam.begin(); i != landmarksParam.end(); ++i) {
        if (i->on)
            landmarkVisibility.push_back(1.0);
        else
            landmarkVisibility.push_back(0.0);
    }
}

void AnimationFrame::retrieveCameraSettings(CameraModel& camera) const {
    if (! my_is_nan(cameraFocus[0]))
        camera.setFocus(cameraFocus);
    if (! my_is_nan(cameraRotation[0]))
        camera.setRotation(Rotation3D(cameraRotation));
    if (! my_is_nan(cameraZoom))
        camera.setScale(cameraZoom);
}

void AnimationFrame::retrieveLandmarkVisibility(QList<ImageMarker>& landmarksParam) const
{
    int ix = 0;
    QList<ImageMarker>::iterator i;
    for (i = landmarksParam.begin(); i != landmarksParam.end(); ++i) {
        bool bShow = false;
        if (ix < landmarkVisibility.size()) {
            bShow = (landmarkVisibility[ix] > 0.5);
            // qDebug() << "landmark visibility" << ix << landmarkVisibility[ix];
        }
        i->on = bShow;
        ++ix;
    }
}
