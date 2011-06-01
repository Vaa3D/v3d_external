#ifndef NAVIEWER_H
#define NAVIEWER_H

#include "../v3d/v3d_core.h"
#include "geometry/CameraModel.h"
#include <cmath>
#include "AnnotationSession.h"

// Base class for 3DViewer, ZStack viewer, and MIP viewer.
// Do not attempt to derive this class from QObject; such folly will create a troublesome diamond pattern in the 3D viewer.
class NaViewer
{
public:
    NaViewer() : bMouseIsDragging(false), defaultScale(1.0), bPaintCrosshair(true) {}
    virtual ~NaViewer() {}
    // TODO replace loadMy4DImage() method with observer relationship to a lighter interface
    virtual bool loadMy4DImage(const My4DImage* my4DImage, const My4DImage* neuronMaskImage = NULL) = 0;
    // Methods to support optional linking of zoom, focus, and rotation between viewers
    virtual void synchronizeWithCameraModel(CameraModel* externalCamera);
    virtual void decoupleCameraModel(CameraModel* externalCamera);
    void setAnnotationSession(AnnotationSession* annotationSession);
    bool mouseIsDragging() const {return bMouseIsDragging;}
    virtual void showCrosshair(bool b) {bPaintCrosshair = b;}

public slots:
   virtual void annotationModelUpdate(QString updateType) = 0;

public:
   // Each viewer has an internal CameraModel that can optionally be synchronized with an external CameraModel
   CameraModel cameraModel;

protected:
    // Helper method for consistent zooming with mouse wheel
    // Zoom using mouse wheel
    virtual void wheelZoom(double delta);

    bool bMouseIsDragging;
    int oldDragX;
    int oldDragY;
    float defaultScale; // view-pixels per image-voxel to exactly fill window
    AnnotationSession* annotationSession;
    bool bPaintCrosshair;
};

#endif // NAVIEWER_H
