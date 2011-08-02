#ifndef NAVOLUMEDATA_H
#define NAVOLUMEDATA_H

#include "NaLockableData.h"
#include "../v3d/v3d_core.h"

class NaVolumeData : public NaLockableData
{
public:
    NaVolumeData();
    virtual ~NaVolumeData();

    bool loadOriginalImageStack(QString originalImageStackFilePath);
    bool loadNeuronMaskStack(QString maskLabelFilePath);
    bool loadReferenceStack(QString referenceStackFilePath);

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

private:
    My4DImage* originalImageStack;
    My4DImage* neuronMaskStack;
    My4DImage* referenceStack;

public:
    class Reader : public BaseReadLocker
    {
    public:
        Reader(const NaVolumeData& data)
            : BaseReadLocker(data.getLock())
            , originalImageStack(data.getOriginalImageStackAsMy4DImage())
            , neuronMaskStack(data.getNeuronMaskAsMy4DImage())
            , referenceStack(data.getReferenceStack())
        {}

        const Image4DProxy<My4DImage> getNeuronMaskProxy();
        const Image4DProxy<My4DImage> getOriginalImageProxy();
        const Image4DProxy<My4DImage> getReferenceImageProxy();

        V3DLONG getXDim() const {return originalImageStack->getXDim();}
        V3DLONG getYDim() const {return originalImageStack->getYDim();}
        V3DLONG getZDim() const {return originalImageStack->getZDim();}
        V3DLONG getCDim() const {return originalImageStack->getCDim();}

    private:
        const My4DImage* originalImageStack;
        const My4DImage* neuronMaskStack;
        const My4DImage* referenceStack;
    };

    const Reader getTemporaryReadLock() const {return Reader(*this);}
};

#endif // NAVOLUMEDATA_H
