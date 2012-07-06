#ifndef VOLUMETEXTURE_H
#define VOLUMETEXTURE_H

#include "NaSharedDataModel.h"
#include "DataColorModel.h"
#include "NaVolumeData.h"
#include "Dimension.h"
#include <QObject>
#include <vector>

class DataFlowModel;

namespace jfrc {

class PrivateVolumeTexture; // forward declaration

/// Contains OpenGL texture data for 3D viewer representation of NeuronAnnotator volume
class VolumeTexture : public NaSharedDataModel<PrivateVolumeTexture>
{
    Q_OBJECT

public:
    VolumeTexture();
    bool initializeGL() const;
    void setDataFlowModel(const DataFlowModel* dataFlowModelParam);
    const uint8_t* signalData3D() const;

signals:
    // When textures change, they must be uploaded in the main/OpenGL thread
    void neuronVisibilityTextureChanged();
    void colorMapTextureChanged();
    void labelFieldChanged();
    void signalTextureChanged();

public slots:
    bool updateVolume();
    void updateNeuronVisibilityTexture();
    bool loadFast3DTexture();

private:
    // semantic sugar
    typedef NaSharedDataModel<PrivateVolumeTexture> super;

protected:
    const NaVolumeData* volumeData;
    const Fast3DTexture* fast3DTexture;

public:
    /// Allows clients (such as Na3DViewer) to upload pixels in main/OpenGL thread.
    /// (because this operation MUST be done in the OpenGL thread)
    class Reader : public BaseReader
    {
    public:
        Reader(const VolumeTexture&);
        const jfrc::Dimension& originalImageSize() const;
        const jfrc::Dimension& usedTextureSize() const;
        const jfrc::Dimension& paddedTextureSize() const;
        // All of the following OpenGL-using methods must be call from the main/OpenGL thread only.
        bool uploadNeuronVisibilityTextureToVideoCardGL() const;
        bool uploadColorMapTextureToVideoCardGL() const;
        bool use3DSignalTexture() const;
        const uint32_t* signalData3D() const;
        const uint16_t* labelData3D() const;
    };

protected:
    class Writer : public BaseWriter
    {
    public:
        Writer(VolumeTexture&);
    };
};

} // namespace jfrc

#endif // VOLUMETEXTURE_H
