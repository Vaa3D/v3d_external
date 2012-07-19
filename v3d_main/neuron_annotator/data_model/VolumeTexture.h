#ifndef VOLUMETEXTURE_H
#define VOLUMETEXTURE_H

#include "NaSharedDataModel.h"
#include "DataColorModel.h"
#include "Dimension.h"
#include <QObject>
#include <vector>
#include <stdint.h>

class DataFlowModel;
class Fast3DTexture;
class NaVolumeData;

namespace jfrc {

class PrivateVolumeTexture; // forward declaration

/// Contains OpenGL texture data for 3D viewer representation of NeuronAnnotator volume
class VolumeTexture : public NaSharedDataModel<PrivateVolumeTexture>
{
    Q_OBJECT

public:
    VolumeTexture();
    virtual ~VolumeTexture();
    void setDataFlowModel(const DataFlowModel* dataFlowModelParam);

signals:
    // When textures change, they must be uploaded in the main/OpenGL thread
    void visibilityTextureChanged();
    void colorMapTextureChanged();
    void labelTextureChanged();
    void signalTextureChanged();
    void benchmarkTimerResetRequested();
    void benchmarkTimerPrintRequested(QString);

public slots:
    bool updateVolume();
    void updateNeuronVisibilityTexture();
    bool updateColorMapTexture();
#ifdef USE_FFMPEG
    bool loadFast3DTexture();
#endif

private:
    // semantic sugar
    typedef NaSharedDataModel<PrivateVolumeTexture> super;

protected:
    const DataFlowModel* dataFlowModel;

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
        bool use3DSignalTexture() const;
        const uint32_t* signalData3D() const;
        const uint16_t* labelData3D() const;
        const uint32_t* visibilityData2D() const;
        const uint32_t* colorMapData2D() const;
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
