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

class RendererNeuronAnnotator : public QObject, public Renderer_gl2
{

Q_OBJECT

public:
    enum Stereo3DMode {
        STEREO_OFF,
        STEREO_LEFT_EYE,
        STEREO_RIGHT_EYE,
        STEREO_QUAD_BUFFERED,
        STEREO_ANAGLYPH_RED_CYAN,
        STEREO_ANAGLYPH_GREEN_MAGENTA,
        STEREO_ROW_INTERLEAVED
    };

public:
    RendererNeuronAnnotator(void* widget);
    virtual ~RendererNeuronAnnotator();
    virtual void paint();
    void paint_mono();
    virtual void setupData(void* data);
    virtual int  _getBufFillSize(int w);
    virtual int  _getTexFillSize(int w);
    virtual void loadVol();
    virtual void cleanVol();
    virtual void loadShader();
    virtual void equAlphaBlendingProjection();
    //
    virtual void drawBackFillVolCube() {}
    // virtual void drawUnitFrontSlice(int line=0) {}
    //
    void setDepthClip(float totalDepthInGlUnits);
    // Renderer_gl1::selectPosition(x,y) is not virtual, so I renamed
    // this reimplementation to screenPositionToVolumePosition(QPoint)
    virtual XYZ screenPositionToVolumePosition(const QPoint& screenPos);
    // useful value for computing zoom level
    float getZoomedPerspectiveViewAngle() const {return viewAngle * zoomRatio;}
    void setInternalZoomRatio(float z) {zoomRatio = z;}
    float glUnitsPerImageVoxel() const;
    bool hasBadMarkerViewMatrix() const;
    void clearLandmarks();
    void setLandmarks(const QList<ImageMarker>& landmarks);
    void updateSettingsFromVolumeTexture(
            const jfrc::VolumeTexture::Reader& textureReader);
    // expose sampleScale[XYZ], thickness[XYZ]
    void setShowClipGuide(bool b) {bShowClipGuide = b;}
    void applyCustomCut(const CameraModel&);
    void setUndoStack(QUndoStack& undoStackParam); // for undo/redo custom clip planes
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

public slots:
    void setAlphaBlending(bool);
    void setStereoMode(int);
    void setShowCornerAxes(bool b);

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

    Stereo3DMode stereo3DMode;
    bool bStereoSwapEyes;
    bool bShowCornerAxes;
    bool bShowClipGuide;

    jfrc::CustomClipPlanes customClipPlanes;
};

#endif // RENDERERNEURONANNOTATOR_H
