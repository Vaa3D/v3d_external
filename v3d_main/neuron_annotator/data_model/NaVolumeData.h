#ifndef NAVOLUMEDATA_H
#define NAVOLUMEDATA_H

#include "StagedFileLoader.h"
#include "NaLockableData.h"
#include "../../v3d/v3d_core.h"
#include "../utility/ImageLoader.h"

namespace jfrc{
class VolumeTexture;
} // namespace jfrc

// NaVolumeDataLoadableStack used to be an inner class of NaVolumeData,
// but inner classes cannot be QObjects.
class NaVolumeDataLoadableStack : public QObject
{
    Q_OBJECT

public:
    NaVolumeDataLoadableStack(My4DImage* stackp, QUrl fileUrl, int stackIndex = -1);
    virtual bool load();
    QUrl determineFullFileUrl() const;
    const QUrl& getFileUrl() const {return fileUrl;}
    bool isCanceled() const {return bIsCanceled;}

signals:
    void progressValueChanged(int progressValue, int stackIndex);
    void progressMessageChanged(QString);
    void failed();
    void finished();
    void canceled();

public slots:
    void cancel() {
        if (bIsCanceled) return;
        bIsCanceled = true;
        imageLoader.cancel();
        emit canceled();
    }

protected:
    void setRelativeProgress(float relativeProgress);

    My4DImage* stackp;
    QUrl fileUrl;
    int stackIndex; // to help keep track of combined progress

    int progressValue;
    int progressMin;
    int progressMax;
    ImageLoader imageLoader;

    volatile bool bIsCanceled;
};


// NaVolumeData holds a collection of My4DImage volumes, plus a QReadWriteLock to manage
// access to the data.  Read-only clients are expected to call refreshLock() on their
// NaVolumeData::Reader objects every 25 ms or so, to keep the application responsive.
// Use the NaVolumeData::Reader and NaVolumeData::Writer classes to automatically acquire
// temporary locks when accessing the image data.
class NaVolumeData : public NaLockableData
{
    Q_OBJECT

public:
    explicit NaVolumeData();
    virtual ~NaVolumeData();
    void setTextureInput(const jfrc::VolumeTexture* texture);
    bool doFlipY_image, doFlipY_mask;
    bool bDoUpdateSignalTexture; // because texture could be populated upstream by VolumeTexture
    void setAuxillaryImagery( QUrl li_path, QUrl vli_path, QString chanSpec )
    {
        visuallyLosslessImage = vli_path;
        losslessImage = li_path;
        channel_spec = chanSpec;
    }
    void splitH5JStack();

signals:
    void referenceLoaded();
    void channelLoaded(int channel_index);
    void channelsLoaded(int channel_count);
    void neuronMaskLoaded();
    void benchmarkTimerResetRequested();
    void benchmarkTimerPrintRequested(QString);

public slots:
    bool loadChannels(QUrl url); // includes loading general volumes
    // staged loading
    bool queueSeparationFolder(QUrl folder); // using new staged loader
    void loadStagedVolumes();

protected slots:
    void loadVolumeDataFromFiles(); // Assumes file name paths have already been set
    void loadSecondaryVolumeDataFromFiles(); // When VolumeTexture has already been populated
    void setProgressValue(int progressValue);
    void setProgressMessage(QString message) {emit progressMessageChanged(message);}
    void setStackLoadProgress(int progressValue, int stackIndex);

    bool loadReference(QUrl fileUrl);
    bool loadOneChannel(QUrl fileUrl, int channel_index = 0);
    bool loadNeuronMask(QUrl fileUrl);
    bool loadVolumeFromTexture();
    // support loading of magically appearing directory of files
    bool loadBestVolumeFromDirectory(QUrl);

private:
    QUrl originalImageStackFileUrl;
    QUrl maskLabelFileUrl;
    QUrl referenceStackFileUrl;
    QUrl visuallyLosslessImage, losslessImage;
    QString channel_spec;
    My4DImage* originalImageStack;
    My4DImage* neuronMaskStack;
    My4DImage* referenceStack;
    My4DImage* emptyImage;
    Image4DProxy<My4DImage> originalImageProxy;
    Image4DProxy<My4DImage> neuronMaskProxy;
    Image4DProxy<My4DImage> referenceImageProxy;
    std::vector<int> stackLoadProgressValues;
    const jfrc::VolumeTexture* volumeTexture;
    int currentProgress;
    // Staged image loader
    ProgressiveLoader progressiveLoader;

public:
    typedef NaVolumeDataLoadableStack LoadableStack;

    class Reader; friend class Reader;
    class Reader : public BaseReadLocker
    {
    public:
        Reader(const NaVolumeData& data)
            : BaseReadLocker(data)
            , m_data(&data)
        {}

        const Image4DProxy<My4DImage>& getNeuronMaskProxy() const;
        const Image4DProxy<My4DImage>& getOriginalImageProxy() const;
        const Image4DProxy<My4DImage>& getReferenceImageProxy() const;
        ImagePixelType getOriginalDatatype() const {return m_data->originalImageStack->getDatatype();}
        bool hasReferenceImage() const {return
                (m_data->referenceStack != NULL)
                && (m_data->referenceStack->getTotalBytes() > 0);}
        bool hasNeuronMask() const {return m_data->neuronMaskStack != NULL &&
                                            m_data->neuronMaskStack->getXDim() &&
                                            m_data->neuronMaskStack->getYDim() &&
                                            m_data->neuronMaskStack->getZDim(); }
        bool doUpdateSignalTexture() const;
        int getNumberOfNeurons() const {
            if (NULL == m_data->neuronMaskStack) return 0;
            int numNeurons = m_data->neuronMaskStack->getChannalMaxIntensity(0);
            // qDebug() << "Number of neurons =" << numNeurons;
            return numNeurons;
        }
        const QList<LocationSimple>& getLandmarks() const {return m_data->originalImageStack->listLandmarks;}

    private:
        const NaVolumeData * m_data;
    };

    // NaVolumeData::Writer is a blocking stack-allocated write lock manager
    class Writer; friend class Writer;
    class Writer : public BaseWriteLocker
    {
    public:
        Writer(NaVolumeData& data)
            : BaseWriteLocker(data)
            , m_data(&data)
        {}

        Image4DProxy<My4DImage>& getNeuronMaskProxy();
        Image4DProxy<My4DImage>& getOriginalImageProxy();
        Image4DProxy<My4DImage>& getReferenceImageProxy();
        ImagePixelType getOriginalDatatype() const {return m_data->originalImageStack->getDatatype();}
        int getNumberOfNeurons() const {return m_data->neuronMaskStack->getChannalMaxIntensity(0);}

        void setOriginalImageStackFileUrl(QUrl path) {m_data->originalImageStackFileUrl = path;}
        void setMaskLabelFileUrl(QUrl path) {m_data->maskLabelFileUrl = path;}
        void setReferenceStackFileUrl(QUrl path) {m_data->referenceStackFileUrl = path;}

        bool loadStacks();
        bool normalizeReferenceStack(My4DImage* initialReferenceStack);

        void clearImageData();
        void clearLandmarks();
        void setLandmarks(const QList<LocationSimple>);

        bool loadSingleImageMovieVolume(QUrl fileUrl);
        bool setSingleImageVolume(My4DImage* img);

        bool loadReference(QUrl fileUrl);
        bool loadOneChannel(QUrl fileUrl, int channel_index = 0);
        int  loadChannels(QUrl url); // includes loading general volumes
        bool loadNeuronMask(QUrl fileUrl);
        bool loadVolumeFromTexture(const jfrc::VolumeTexture* texture);

    private:
        NaVolumeData * m_data;
    };


};

#endif // NAVOLUMEDATA_H
