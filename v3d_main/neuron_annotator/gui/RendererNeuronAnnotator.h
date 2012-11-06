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
#include "Stereo3dMode.h"

class RendererNeuronAnnotator : public QObject, public Renderer_gl2
{

Q_OBJECT

public:
    RendererNeuronAnnotator(void* widget);
    virtual ~RendererNeuronAnnotator();
    virtual void paint(); // for stereo viewing
    void paint_mono(bool clear=true);
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

    jfrc::Stereo3DMode getStereoMode() const {return stereo3DMode;}
    bool getStereoSwapEyes() const {return bStereoSwapEyes;}
    bool getScreenRowParity() const {return screenRowParity;}
    bool getScreenColumnParity() const {return screenColumnParity;}

    // expose sampleScale[XYZ], thickness[XYZ]
    void setShowClipGuide(bool b) {bShowClipGuide = b;}
    void applyCustomCut(const CameraModel&);
    void applyCutPlaneInImageFrame(Vector3D point, Vector3D direction);
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
    void setStereoMode(int);
    // For use in stencilled display modes
    bool setScreenRowParity(bool b) {
        // qDebug() << "setScreenRowParity()" << b << __FILE__ << __LINE__;
        if (screenRowParity != b) {
            screenRowParity = b;
            return true; // value changed
        }
        return false;
    }
    bool setScreenColumnParity(bool b) {
        if (screenColumnParity != b) {
            screenColumnParity = b;
            return true;
        }
        return false;
    }
    void setShowCornerAxes(bool b);
    bool setSlabThickness(int); // range 2-1000 voxels
    void clipSlab(const CameraModel& cameraModel); // Apply clip plane to current slab
    bool resetSlabThickness();
    void set3dTextureMode(unsigned int textureId);
    void setColorMapTextureId(unsigned int textureId);

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

    jfrc::Stereo3DMode stereo3DMode;
    bool bStereoSwapEyes;
    bool screenRowParity; // is the topmost pixel on an odd scan line?
    bool screenColumnParity; // is the leftmost pixel on an odd screen column?
    bool bShowCornerAxes;
    bool bShowClipGuide;

    jfrc::CustomClipPlanes customClipPlanes;

    int slabThickness;
    int slabDepth;
};

#endif // RENDERERNEURONANNOTATOR_H
