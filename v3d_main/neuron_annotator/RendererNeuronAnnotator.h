#ifndef RENDERERNEURONANNOTATOR_H
#define RENDERERNEURONANNOTATOR_H

#include "../3drenderer/Renderer_gl2.h"
#include "geometry/Vector3D.h"

class RendererNeuronAnnotator : public Renderer_gl2
{

public:
    RendererNeuronAnnotator(void* widget);
    virtual ~RendererNeuronAnnotator();
    virtual void loadVol();
    bool populateNeuronMask(const My4DImage* my4Dmask);
    void setBackgroundBaseTexture(QList<int> maskIndexList, QProgressDialog & dialog);
    void setAllBaseTexture(QProgressDialog & dialog);
    void setBlankBaseTexture(QList<int> maskIndexList, QProgressDialog & dialog);
    void updateCurrentTextureMask(int maskIndex, int state, QProgressDialog & dialog);
    bool initializeTextureMasks();
    void setMasklessSetupStackTexture(bool state) { masklessSetupStackTexture=state; }
    // useful value for computing zoom level
    float getZoomedPerspectiveViewAngle() const {return viewAngle * zoomRatio;}
    void setInternalZoomRatio(float z) {zoomRatio = z;}
    float glUnitsPerImageVoxel() const {
        return 2.0 / boundingBox.Dmax();
    }

protected:
    virtual void setupStackTexture(bool bfirst);
    void load3DTextureSet(RGBA8* tex3DBuf, QProgressDialog & dialog);
    RGBA8* extendTextureFromMaskList(RGBA8* sourceTexture, const QList<int> & maskIndexList);
    void cleanExtendedTextures();
    void updateProgressDialog(QProgressDialog & dialog, int level);
    bool populateBaseTextures();

    // We want all of these OFF for now to keep the texture handling constant across different hardware environments
    virtual bool supported_TexStream() {return false;}
    virtual void setupTexStreamBuffer() {tex_stream_buffer = false;}
    virtual void cleanTexStreamBuffer() {tex_stream_buffer = false;}
    virtual bool _streamingTex() {return false;}
    virtual void _streamTex(int stack_i, int slice_i, int step, int slice0, int slice1) {}
    virtual void _streamTex_end() {}

private:
    unsigned char* neuronMask; // sized to texture buffer dimensions realX,Y,Z
    RGBA8* texture3DAll;
    RGBA8* texture3DBackground;
    RGBA8* texture3DBlank;
    RGBA8* texture3DCurrent;
    bool textureSetAlreadyLoaded;
    bool masklessSetupStackTexture;
};

#endif // RENDERERNEURONANNOTATOR_H
