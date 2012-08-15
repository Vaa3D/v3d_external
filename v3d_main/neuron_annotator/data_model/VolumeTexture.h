#ifndef VOLUMETEXTURE_H
#define VOLUMETEXTURE_H

#include "NaSharedDataModel.h"
#include "Dimension.h"
#include "SampledVolumeMetadata.h"
#include <QObject>
#include <QFileInfo>
#include <QDir>
#include <vector>
#include <deque>
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
    explicit VolumeTexture(const VolumeTexture& rhs);
    virtual ~VolumeTexture();
    void setDataFlowModel(const DataFlowModel* dataFlowModelParam);

signals:
    // When textures change, they must be uploaded in the main/OpenGL thread
    void visibilityTextureChanged();
    void colorMapTextureChanged();
    void labelTextureChanged();
    void signalTextureChanged();
    void signalMetadataChanged();
    void benchmarkTimerResetRequested();
    void benchmarkTimerPrintRequested(QString);
    void volumeLoadSequenceCompleted();
    void mpegQueueRequested(QString fileName, int channel);
    void mpegLoadSequenceRequested();
    void loadNextVolumeRequested();

public slots:
    bool updateVolume();
    void updateNeuronVisibilityTexture();
    bool updateColorMapTexture();
    bool loadLabelPbdFile();
    void setLabelPbdFileName(QString fileName) {labelPbdFileName = fileName;}
    bool loadSignalRawFile(QString fileName);
    bool loadReferenceRawFile(QString fileName);
    bool queueFastLoadVolumes(QDir separationDirectory);
    void loadVolumeQueue();
    void loadNextVolume();
#ifdef USE_FFMPEG
    bool loadFast3DTexture();
#endif

private:
    // semantic sugar
    typedef NaSharedDataModel<PrivateVolumeTexture> super;

protected:
    bool queueFile(QString fileName);

    const DataFlowModel* dataFlowModel;
    QString labelPbdFileName;

public:


    /// Allows clients (such as Na3DViewer) to upload pixels in main/OpenGL thread.
    /// (because this operation MUST be done in the OpenGL thread)
    class Reader : public BaseReader
    {
    public:
        Reader(const VolumeTexture&);
        virtual ~Reader();
        const jfrc::Dimension& originalImageSize() const;
        const jfrc::Dimension& usedTextureSize() const;
        const jfrc::Dimension& paddedTextureSize() const;
        const SampledVolumeMetadata& metadata() const;
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


    /// One file of a series of volume images to be loaded into a VolumeTexture
    struct QueuedFile
    {
        enum FileFormat {RAW_FORMAT, MPEG_FORMAT};
        enum VolumeType {LABEL_VOLUME, REFERENCE_VOLUME,
                         SIGNAL_RGB_VOLUME,
                         SIGNAL_RED_VOLUME,
                         SIGNAL_GREEN_VOLUME,
                         SIGNAL_BLUE_VOLUME};

        QueuedFile(QString fileName)
            : fileName(fileName)
        {
            QFileInfo fi(fileName);
            QString extension = fi.suffix().toLower();
            QString baseName = fi.baseName();
            if (extension == "mp4")
                format = MPEG_FORMAT;
            else
                format = RAW_FORMAT;
            if (baseName.startsWith("Reference"))
                volumeType = REFERENCE_VOLUME;
            else if (baseName.startsWith("ConsolidatedLabel"))
                volumeType = LABEL_VOLUME;
            else if (baseName.startsWith("ConsolidatedSignal")) {
                if (baseName.contains("Red"))
                    volumeType = SIGNAL_RED_VOLUME;
                else if (baseName.contains("Green"))
                    volumeType = SIGNAL_GREEN_VOLUME;
                else if (baseName.contains("Blue"))
                    volumeType = SIGNAL_BLUE_VOLUME;
                else
                    volumeType = SIGNAL_RGB_VOLUME;
            }
            else volumeType = SIGNAL_RGB_VOLUME;
        }

        QString fileName;
        FileFormat format;
        VolumeType volumeType;
    };

    std::deque<QueuedFile> fileQueue; // for staged loading
};

} // namespace jfrc

#endif // VOLUMETEXTURE_H
