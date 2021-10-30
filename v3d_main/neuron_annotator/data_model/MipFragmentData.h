#ifndef MIPFRAGMENTDATA_H
#define MIPFRAGMENTDATA_H

#include "NaLockableData.h"
#include "NaVolumeData.h"

// MipFragmentData stores 16-bit maximum intensity projections of neuron fragments.
// The My4DImage data structures store the different fragments along the z-dimension.
// Background (non-fragment) data are in fragment index (z-slice) zero (0)
// Reference (nc82) intensities are in fragment index (z-slice) number-of-fragments-plus-one.
// Reference data are in the *intensity* image, not the *data* image, since there is just one reference channel.
// TODO - deprecate MipData class in favor of MipFragmentData
class MipFragmentData : public NaLockableData
{
    Q_OBJECT
public:
    explicit MipFragmentData(const NaVolumeData& volumeDataParam);
    virtual ~MipFragmentData();

public slots:
    void updateFromVolumeData();

private:
    const NaVolumeData& volumeData;
    My4DImage* fragmentData; // 16-bit intensities by channel, z-coordinate is actually fragment/neuron index
    My4DImage* fragmentZValues; // value is z-coordinate where max intensity was found.  Zero value means no overlap with this fragment.
    My4DImage* fragmentIntensities; // sum of all data channels.  saves arithmetic during recombinations.  Reference/nc82 channel is here.
    std::vector<int> fragmentMinimumIntensities; // min across all channels, for HDR use
    std::vector<int> fragmentMaximumIntensities; // max across all channels, for HDR use

public:
    class Reader; friend class Reader;
    class Reader : public BaseReadLocker
    {
    public:
        Reader(const MipFragmentData& mipParam)
            : BaseReadLocker(mipParam)
            , m_mipFragmentData(mipParam)
        {}

        int getNumberOfDataChannels() const {
            if (! m_mipFragmentData.fragmentData) return 0;
            return m_mipFragmentData.fragmentData->getCDim() + 1; // +1 for reference
        }
        const Image4DProxy<My4DImage> getMipProxy() const {
            return Image4DProxy<My4DImage>(const_cast<My4DImage*>(m_mipFragmentData.fragmentData));
        }
        const Image4DProxy<My4DImage> getZBufferProxy() const {
            return Image4DProxy<My4DImage>(const_cast<My4DImage*>(m_mipFragmentData.fragmentZValues));
        }
        const Image4DProxy<My4DImage> getIntensityProxy() const {
            return Image4DProxy<My4DImage>(const_cast<My4DImage*>(m_mipFragmentData.fragmentIntensities));
        }

        ImagePixelType getOriginalDatatype() const {return m_mipFragmentData.fragmentData->getDatatype();}

    private:
        const MipFragmentData& m_mipFragmentData;
    };


    class Writer; friend class Writer;
    class Writer : public BaseWriteLocker
    {
    public:
        Writer(MipFragmentData& mipParam)
            : BaseWriteLocker(mipParam)
            , m_mipFragmentData(mipParam)
        {}

        void clearImageData();

    private:
        MipFragmentData& m_mipFragmentData;
    };
};

#endif // MIPFRAGMENTDATA_H
