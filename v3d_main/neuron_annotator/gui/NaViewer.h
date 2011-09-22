#ifndef NAVIEWER_H
#define NAVIEWER_H

#include "../../v3d/v3d_core.h"
#include "../geometry/CameraModel.h"
#include <cmath>
#include "../DataFlowModel.h"
#include "MouseClickManager.h"

// Base class for 3DViewer, ZStack viewer, and MIP viewer.
// Do not attempt to derive this class from QObject; such folly will create a troublesome diamond pattern in the 3D viewer.
class NaViewer
{
public:
    NaViewer();
    virtual ~NaViewer() {}
    // Methods to support optional linking of zoom, focus, and rotation between viewers
    virtual void synchronizeWithCameraModel(CameraModel* externalCamera);
    virtual void decoupleCameraModel(CameraModel* externalCamera);
    virtual void setDataFlowModel(const DataFlowModel&);
    bool mouseIsDragging() const {return bMouseIsDragging;}
    virtual void showCrosshair(bool b) {bPaintCrosshair = b;}

// public slots: // NaViewer is not a QObject, so does not have slots.
    virtual void toggleNeuronDisplay(NeuronSelectionModel::NeuronIndex index, bool checked) = 0;
    virtual void updateFullVolume() = 0;

public:
    // Each viewer has an internal CameraModel that can optionally be synchronized with an external CameraModel
    CameraModel cameraModel;

protected:
    // Helper method for consistent zooming with mouse wheel
    // Zoom using mouse wheel
    virtual void wheelZoom(double delta);

    float defaultScale; // view-pixels per image-voxel to exactly fill window
    bool bMouseIsDragging;
    int oldDragX;
    int oldDragY;
    const DataFlowModel* dataFlowModel;
    bool bPaintCrosshair;
    // Help distinguish between single clicks and double clicks
    MouseClickManager mouseClickManager;
};

#endif // NAVIEWER_H
