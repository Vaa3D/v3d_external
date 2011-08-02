#ifndef NAVOLUMEDATA_H
#define NAVOLUMEDATA_H

#include "NaLockableData.h"
#include "../v3d/v3d_core.h"

// NaVolumeData holds a collection of My4DImage volumes, plus a QReadWriteLock to manage
// access to the data.  Read-only clients are expected to call refreshLock() on their
// NaVolumeData::Reader objects every 25 ms or so, to keep the application responsive.
class NaVolumeData : public NaLockableData
{
public:
    NaVolumeData();
    virtual ~NaVolumeData();

    // TODO - eventually deprecate direct accessors in favor of stack-allocated lock objects
    // non-const accessors should only be used for *modifying* volume data
    // Don't forget to get a write lock while writing!
    My4DImage* getOriginalImageStackAsMy4DImage() { return originalImageStack; }
    My4DImage* getReferenceStack() { return referenceStack; }
    My4DImage* getNeuronMaskAsMy4DImage() { return neuronMaskStack; }
    // const versions for read-only clients
    // Don't forget to get a read lock while reading!
    // TODO - enforce this by embedding My4DImage* access in read locker object.
    const My4DImage* getOriginalImageStackAsMy4DImage() const { return originalImageStack; }
    const My4DImage* getReferenceStack() const { return referenceStack; }
    const My4DImage* getNeuronMaskAsMy4DImage() const { return neuronMaskStack; }

public slots:
    void loadAllVolumeData();

private:
    QString originalImageStackFilePath;
    QString maskLabelFilePath;
    QString referenceStackFilePath;
    My4DImage* originalImageStack;
    My4DImage* neuronMaskStack;
    My4DImage* referenceStack;

public:

    class Reader; friend class Reader;
    class Reader : public BaseReadLocker
    {
    public:
        Reader(const NaVolumeData& data)
            : BaseReadLocker(data.getLock())
            , m_data(&data)
        {}

        const Image4DProxy<My4DImage> getNeuronMaskProxy() const;
        const Image4DProxy<My4DImage> getOriginalImageProxy() const;
        const Image4DProxy<My4DImage> getReferenceImageProxy() const;

        V3DLONG getXDim() const;
        V3DLONG getYDim() const;
        V3DLONG getZDim() const;
        V3DLONG getCDim() const;

    private:
        const NaVolumeData * m_data;
    };

    // NaVolumeData::Writer is a blocking stack-allocated write lock manager
    class Writer; friend class Writer;
    class Writer : public QWriteLocker
    {
    public:
        Writer(NaVolumeData& data)
            : QWriteLocker(data.getLock())
            , m_data(&data)
        {}

        Image4DProxy<My4DImage> getNeuronMaskProxy();
        Image4DProxy<My4DImage> getOriginalImageProxy();
        Image4DProxy<My4DImage> getReferenceImageProxy();

        V3DLONG getXDim() const;
        V3DLONG getYDim() const;
        V3DLONG getZDim() const;
        V3DLONG getCDim() const;

        void setOriginalImageStackFilePath(QString path) {m_data->originalImageStackFilePath = path;}
        void setMaskLabelFilePath(QString path) {m_data->maskLabelFilePath = path;}
        void setReferenceStackFilePath(QString path) {m_data->referenceStackFilePath = path;}

        bool loadOriginalImageStack();
        bool loadNeuronMaskStack();
        bool loadReferenceStack();

        void clearImageData();

    private:
        NaVolumeData * m_data;
    };
};

#endif // NAVOLUMEDATA_H
