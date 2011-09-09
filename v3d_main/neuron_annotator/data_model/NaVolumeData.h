#ifndef NAVOLUMEDATA_H
#define NAVOLUMEDATA_H

#include "NaLockableData.h"
#include "../v3d/v3d_core.h"


// NaVolumeDataLoadableStack used to be an inner class of NaVolumeData,
// but inner classes cannot be QObjects.
class NaVolumeDataLoadableStack : public QObject
{
    Q_OBJECT

public:
    NaVolumeDataLoadableStack(My4DImage* stackp, QString filename, int stackIndex = -1);
    virtual bool load();

signals:
    void progressValueChanged(int progressValue, int stackIndex);
    void failed();
    void finished();

private:
    void setRelativeProgress(float relativeProgress);

    My4DImage* stackp;
    QString filename;
    int stackIndex; // to help keep track of combined progress

    int progressValue;
    int progressMin;
    int progressMax;
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

public slots:
    void loadVolumeDataFromFiles(); // Assumes file name paths have already been set
    void setProgressValue(int progressValue);
    void setStackLoadProgress(int progressValue, int stackIndex);

private:
    QString originalImageStackFilePath;
    QString maskLabelFilePath;
    QString referenceStackFilePath;
    My4DImage* originalImageStack;
    My4DImage* neuronMaskStack;
    My4DImage* referenceStack;
    My4DImage* emptyImage;
    Image4DProxy<My4DImage> originalImageProxy;
    Image4DProxy<My4DImage> neuronMaskProxy;
    Image4DProxy<My4DImage> referenceImageProxy;
    std::vector<int> stackLoadProgressValues;
    int currentProgress;

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
        int getNumberOfNeurons() const {return m_data->neuronMaskStack->getChannalMaxIntensity(0);}
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

        void setOriginalImageStackFilePath(QString path) {m_data->originalImageStackFilePath = path;}
        void setMaskLabelFilePath(QString path) {m_data->maskLabelFilePath = path;}
        void setReferenceStackFilePath(QString path) {m_data->referenceStackFilePath = path;}

        bool loadStacks();
        bool normalizeReferenceStack(My4DImage* initialReferenceStack);

        void clearImageData();
        void clearLandmarks();
        void setLandmarks(const QList<LocationSimple>);

    private:
        NaVolumeData * m_data;
    };


};

#endif // NAVOLUMEDATA_H
