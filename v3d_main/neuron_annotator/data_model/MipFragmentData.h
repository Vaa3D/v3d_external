#ifndef MIPFRAGMENTDATA_H
#define MIPFRAGMENTDATA_H

#include "NaLockableData.h"
#include "NaVolumeData.h"

// MipFragmentData stores 16-bit maximum intensity projections of neuron fragments.
// The My4DImage data structures store the different fragments along the z-dimension.
// Background (non-fragment) data are in fragment index [XXX WHAT? XXX]
// Reference (nc82) intensities are in fragment index [XXX WHAT? XXX]
// TODO - deprecate MipData class in favor of MipFragmentData
class MipFragmentData : public NaLockableData
{
    Q_OBJECT
public:
    explicit MipFragmentData(const NaVolumeData& volumeDataParam, QObject *parent = 0);
    virtual ~MipFragmentData();

public slots:
    void updateFromVolumeData();

private:
    const NaVolumeData& volumeData;
    My4DImage* fragmentData; // 16-bit intensities by channel
    My4DImage* fragmentZValues; // value is z-coordinate where max intensity was found.  Zero value means no overlap with this fragment.
    My4DImage* fragmentIntensities; // sum of all data channels.  saves arithmetic during recombinations
    My4DImage* fragmentMinMax; // x(X), y(Y), fragmentIndex(Z), min[0];max[1](C); useful for rescaling lookups


public:
    class Reader; friend class Reader;
    class Reader : public BaseReadLocker
    {
    public:
        Reader(const MipFragmentData& mipParam)
            : BaseReadLocker(mipParam.getLock())
            , m_mipFragmentData(mipParam)
        {}

    private:
        const MipFragmentData& m_mipFragmentData;
    };


    class Writer; friend class Writer;
    class Writer : public QWriteLocker
    {
    public:
        Writer(MipFragmentData& mipParam)
            : QWriteLocker(mipParam.getLock())
            , m_mipFragmentData(mipParam)
        {}

    private:
        MipFragmentData& m_mipFragmentData;
    };
};

#endif // MIPFRAGMENTDATA_H
