#include "NeuronFragmentData.h"
#include "PrivateNeuronFragmentData.h"
#include "NaSharedDataModel.cpp"
#include <cmath>

// instantiate NaSharedDataModel<PrivateNeuronFragmentData>
template class NaSharedDataModel<PrivateNeuronFragmentData>;

NeuronFragmentData::NeuronFragmentData()
    : naVolumeData(NULL)
{}

/* explicit */
NeuronFragmentData::NeuronFragmentData(const NeuronFragmentData& rhs)
    : naVolumeData(rhs.naVolumeData)
{
    if (naVolumeData)
        connect(naVolumeData, SIGNAL(dataChanged()),
                this, SLOT(update()));
}

/* explicit */
NeuronFragmentData::NeuronFragmentData(const NaVolumeData& naVolumeDataParam)
    : naVolumeData(&naVolumeDataParam)
{
    connect(naVolumeData, SIGNAL(dataChanged()),
            this, SLOT(update()));
}

/* virtual */
NeuronFragmentData::~NeuronFragmentData() {}

/* slot */
void NeuronFragmentData::update()
{
    if (! representsActualData()) return;
    // qDebug() << "NeuronFragmentData::update()" << __FILE__ << __LINE__;
    if (!naVolumeData) return;
    { // stack block containing read/write locks
        NaVolumeData::Reader volumeReader(*naVolumeData);
        if (! volumeReader.hasReadLock()) return;
        Writer(*this);
        int sf = volumeReader.getNumberOfNeurons(); // does not include background, at index zero
        d->setNumberOfFragments(sf);
        for (int f = 0; f < sf; ++f) {
            d->setFragmentSize(f, 0);
        }
        const Image4DProxy<My4DImage>& fragmentProxy = volumeReader.getNeuronMaskProxy();
        const Image4DProxy<My4DImage>& dataProxy = volumeReader.getOriginalImageProxy();
        // Temporary 2D representation of color (ignoring saturation) because hue is a circular parameter
        std::vector< std::pair<float, float> > fragment2DColors(sf, std::pair<float, float>(0.0, 0.0) );
        const std::vector<int>& fragmentSizes = d.constData()->getFragmentSizes();
        // Hue wheel has a number of spokes depending on number of data channels.
        // (polar coordinates with angle as hue and radius as intensity)
        std::vector<float> hueChannelX(dataProxy.sc, 0.0);
        std::vector<float> hueChannelY(dataProxy.sc, 0.0);
        for (int c = 0; c < dataProxy.sc; ++c) {
            double angle = 2.0 * 3.14159 * (double)c / (double)dataProxy.sc;
            hueChannelX[c] = std::cos(angle);
            hueChannelY[c] = std::sin(angle);
        }
        // Special case where sc == 2 requires 90 degree separation, not 180 degrees.
        if (dataProxy.sc == 2) {
            hueChannelX[1] = 1.0;
            hueChannelY[1] = 0.0;
        }
        if (! representsActualData()) return;
        for (int z = 0; z < fragmentProxy.sz; ++z) {
            for (int y = 0; y < fragmentProxy.sy; ++y) {
                for (int x = 0; x < fragmentProxy.sx; ++x)
                {
                    int f = fragmentProxy.value_at(x, y, z, 0) - 1; // fragment index, offset by one to exclude background
                    if (f < 0) continue; // skip background
                    std::pair<float, float>& color2d = fragment2DColors[f];
                    // update voxel count
                    d->setFragmentSize(f, fragmentSizes[f] + 1);
                    for (int c = 0; c < dataProxy.sc; ++c) {
                        double val = dataProxy.value_at(x, y, z, c);
                        color2d.first += hueChannelX[c] * val;
                        color2d.second += hueChannelY[c] * val;
                    }
                }
            }
            if (! representsActualData())
                return;
        }
        // Save final hue values
        for (int f = 0; f < sf; ++f)
        {
            const std::pair<float, float>& color2d = fragment2DColors[f];
            double hue = std::atan2(color2d.second, color2d.first) / (2.0 * 3.14159);
            // restore 2-channel hue to range 0-0.5, instead of 0-0.25
            if (dataProxy.sc == 2)
                hue *= 2.0;
            d->setFragmentHue(f, (float) hue);
        }
    } // release locks
    emit dataChanged();
}


////////////////////////////////////////
// NeuronFragmentData::Reader methods //
////////////////////////////////////////

NeuronFragmentData::Reader::Reader(const NeuronFragmentData& fragmentData)
    : NeuronFragmentData::BaseReader(fragmentData)
{}

int NeuronFragmentData::Reader::getNumberOfFragments() const
{
    return d.constData()->getNumberOfFragments();
}

const std::vector<int>& NeuronFragmentData::Reader::getFragmentSizes() const // in voxels
{
    return d.constData()->getFragmentSizes();
}

const std::vector<float>& NeuronFragmentData::Reader::getFragmentHues() const // in range 0.0-1.0
{
    return d.constData()->getFragmentHues();
}

/* virtual */
NeuronFragmentData::Reader::~Reader() {}

///////////////////////////////////////
// NeuronFragmntData::Writer methods //
///////////////////////////////////////

NeuronFragmentData::Writer::~Writer() {}

