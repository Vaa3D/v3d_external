#ifndef RENDERERNEURONANNOTATOR_H
#define RENDERERNEURONANNOTATOR_H

#include <QObject>
#include "../../3drenderer/renderer_gl2.h"
#include "../geometry/Vector3D.h"
#include "../data_model/NaVolumeData.h"
#include "../data_model/VolumeTexture.h"
#include "../data_model/Dimension.h"
#include "../data_model/CustomClipPlanes.h"
#include "../geometry/CameraModel.h"

/**
 * Prior to Nov 2012, I was schizophrenic about the separation of
 * responsibilities between Na3DWidget, and its member
 * RendererNeuronAnnotator class. Now I have settled on the following
 * policy, based on refactoring the ActorGL concept:
 * Na3DWidget
 *   * camera configuration, including stereo 3d
 * RendererNeuronAnnotator
 *   * volume geometry, including volume centering
 * It might take a while before I have compartmentalized all the details
 * of these responsibilities into their separate classes.
 *
 * TODO - remove from RendererNeuronAnnotator:
 *   color_background
 *   zoom level
 * ADD to RendererNeuronAnnotator:
 *   data flow model
 */
class RendererNeuronAnnotator : public QObject, public Renderer_gl2
{

Q_OBJECT

public:
    RendererNeuronAnnotator(void* widget);
    virtual ~RendererNeuronAnnotator();
    virtual void paint();
    void paint_mono(bool clear=true); // one-eye worth of stereo viewing
    virtual void setupData(void* data);
    virtual int  _getBufFillSize(int w);
    virtual int  _getTexFillSize(int w);
    virtual void loadVol();
    virtual void cleanVol();
    virtual void renderVol();
    virtual void loadShader();
    virtual void cleanShader();
    virtual void equAlphaBlendingProjection();
    //
    virtual void drawBackFillVolCube() {}
    virtual void drawUnitFrontSlice(int line=0) {
        // qDebug() << "drawUnitFrontSlice";
    }
    virtual int getScreenWidth() const {return screenW;}
    virtual int getScreenHeight() const {return screenH;}
    //
    void setDepthClip(float totalDepthInGlUnits);
    void updateDepthClip();
    // Renderer_gl1::selectPosition(x,y) is not virtual, so I renamed
    // this reimplementation to screenPositionToVolumePosition(QPoint)
    virtual XYZ screenPositionToVolumePosition(
            const QPoint& screenPos,
            const NaVolumeData::Reader& volumeReader);
    // useful value for computing zoom level
    float getZoomedPerspectiveViewAngle() const {return viewAngle * zoomRatio;}
    void setInternalZoomRatio(float z) {zoomRatio = z;}
    float glUnitsPerImageVoxel() const;
    bool hasBadMarkerViewMatrix() const;
    void clearLandmarks();
    void setLandmarks(const QList<ImageMarker>& landmarks);

    void updateSettingsFromVolumeTexture(
            const jfrc::VolumeTexture::Reader& textureReader);
    void setOriginalVolumeDimensions(long x, long y, long z);
    jfrc::Dimension getOriginalVolumeDimensions() const;
    jfrc::Dimension getPaddedTextureDimensions() const;
    void setResampledVolumeDimensions(long x, long y, long z);
    void setPaddedVolumeDimensions(long x, long y, long z);
    void setSingleVolumeDimensions(long x, long y, long z);

    // expose sampleScale[XYZ], thickness[XYZ]
    void setShowClipGuide(bool b) {bShowClipGuide = b;}
    void applyCustomCut(const CameraModel&);
    void applyCutPlaneInImageFrame(Vector3D point, Vector3D direction);
    void applyKeepCut(const CameraModel& cameraModel);
    void applyKeepPlaneInImageFrame(Vector3D point, Vector3D direction);
    void setUndoStack(QUndoStack& undoStackParam); // for undo/redo custom clip planes
    void clearClipPlanes();
    using Renderer_gl2::sampleScaleX;
    using Renderer_gl2::sampleScaleY;
    using Renderer_gl2::sampleScaleZ;
    using Renderer_gl2::thicknessX;
    using Renderer_gl2::thicknessY;
    using Renderer_gl2::thicknessZ;

signals:
    void progressValueChanged(int);
    void progressComplete();
    void progressMessageChanged(QString);
    void progressAborted(QString);
    void alphaBlendingChanged(bool);
    void showCornerAxesChanged(bool);
    void slabThicknessChanged(int);

public slots:
    void setAlphaBlending(bool);
    void setShowCornerAxes(bool b);
    bool setSlabThickness(int); // range 2-1000 voxels
    void clipSlab(const CameraModel& cameraModel); // Apply clip plane to current slab
    bool resetSlabThickness();
    void set3dTextureMode(unsigned int textureId);
    void setColorMapTextureId(unsigned int textureId);
    void clearImage(); // avoid stale image pointers

protected:
    virtual void shaderTexBegin(bool stream);
    virtual void shaderTexEnd();
    virtual void setupStackTexture(bool bfirst);
    virtual void _drawStack( double ts, double th, double tw,
                    double s0, double s1, double h0, double h1, double w0, double w1,
                    double ds, int slice0, int slice1, int thickness,
                    GLuint tex3D, GLuint texs[], int stack_i,
                    float direction, int section, bool t3d, bool stream);
    void paintCornerAxes();
    void paintClipGuide();
    // We want all of these OFF for now to keep the texture handling constant across different hardware environments
    virtual bool supported_TexStream() {return false;}
    virtual void setupTexStreamBuffer() {tex_stream_buffer = false;}
    virtual void cleanTexStreamBuffer() {tex_stream_buffer = false;}
    virtual bool _streamingTex() {return false;}
    virtual void _streamTex(int stack_i, int slice_i, int step, int slice0, int slice1) {}
    virtual void _streamTex_end() {}

    bool bShowCornerAxes;
    bool bShowClipGuide;

    jfrc::CustomClipPlanes customClipPlanes;
    jfrc::ClipPlane keepPlane;

    int slabThickness;
    int slabDepth;
};

#endif // RENDERERNEURONANNOTATOR_H
