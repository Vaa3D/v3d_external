#include "MipFragmentData.h"

/* explicit */
MipFragmentData::MipFragmentData(const NaVolumeData& volumeDataParam)
    : volumeData(volumeDataParam)
    , fragmentData(NULL)
    , fragmentZValues(NULL)
    , fragmentIntensities(NULL)
{
    connect(&volumeData, SIGNAL(dataChanged()),
            this, SLOT(updateFromVolumeData()));
}

/* virtual */
MipFragmentData::~MipFragmentData()
{
    Writer mipWriter(*this); // wait for readers to release lock
    mipWriter.clearImageData();
}

/* slot */
void MipFragmentData::updateFromVolumeData()
{
	// qDebug() << "MipFragmentData::updateFromVolumeData()" << __FILE__ << __LINE__;
    QTime stopwatch;
    stopwatch.start();

    { // block containing read/write locks
        // acquire read lock on volume data
        NaVolumeData::Reader volumeReader(volumeData);
        if (! volumeReader.hasReadLock()) return; // Don't worry; we'll get another data push later.

        int layerCount = volumeReader.getNumberOfNeurons() + 1; // +1 for background
        if (volumeReader.hasReferenceImage())
            layerCount += 1;
        const int refIndex = volumeReader.getNumberOfNeurons() + 1;

        // acquire write lock on Mip data
        Writer mipWriter(*this);
        // perhaps it took some time to acquire the write lock, so check the volume again
        if (! volumeReader.refreshLock()) return;

        const Image4DProxy<My4DImage>& originalProxy = volumeReader.getOriginalImageProxy();
        const Image4DProxy<My4DImage>& referenceProxy = volumeReader.getReferenceImageProxy();
        const Image4DProxy<My4DImage>& maskProxy = volumeReader.getNeuronMaskProxy();

        // Sanity check
        if (originalProxy.sx < 1) return;
        if (originalProxy.sy < 1) return;
        if (originalProxy.sz < 1) return;
        if (originalProxy.sc < 1) return;
        if (originalProxy.su < 1) return;

        // Allocate mip images
        mipWriter.clearImageData();

		// clear max/min cache - AFTER clearImageData();
        fragmentMaximumIntensities.assign(layerCount, 0);
        fragmentMinimumIntensities.assign(layerCount, 0);
		// qDebug() << "fragmentMaximumIntensities.size() =" << fragmentMaximumIntensities.size();
		// qDebug() << "fragmentMinimumIntensities.size() =" << fragmentMinimumIntensities.size();

        // allocate channel data
        fragmentData = new My4DImage();
        fragmentData->loadImage(
                originalProxy.sx,
                originalProxy.sy,
                volumeReader.getNumberOfNeurons() + 1, // frags + bkgd; (reference/nc82 channel is stored in fragmentIntensities)
                originalProxy.sc,
                volumeReader.getOriginalDatatype() );
        // set to zero
        memset(fragmentData->getRawData(), 0, fragmentData->getTotalBytes());

        // allocate Z buffer
        fragmentZValues = new My4DImage();
        fragmentZValues->loadImage(
                originalProxy.sx,
                originalProxy.sy,
                layerCount, // +1 reference/nc82 channel
                1, // only one channel, containing z values
                V3D_UINT16 );
        // clear each byte to xFF, should result in -1?
        memset(fragmentZValues->getRawData(), 255, fragmentZValues->getTotalBytes());

        // allocate intensity cache
        fragmentIntensities = new My4DImage();
        fragmentIntensities->loadImage(
                originalProxy.sx,
                originalProxy.sy,
                layerCount, // +1 reference/nc82 channel is stored in slice nFrags+1
                1, // only one channel, containing z intensities
                V3D_UINT16 );
        // initialize to zero
        memset(fragmentIntensities->getRawData(), 0, fragmentIntensities->getTotalBytes());
        if (! volumeReader.refreshLock()) return;

        Image4DProxy<My4DImage> mipProxy(fragmentData);
        Image4DProxy<My4DImage> zProxy(fragmentZValues);
        Image4DProxy<My4DImage> intensityProxy(fragmentIntensities);

        // Populate mip images
        int imageX = originalProxy.sx;
        int imageY = originalProxy.sy;
        int imageZ = originalProxy.sz;
        int imageC = originalProxy.sc;
        // Reference/nc82 channel appears after all of the neuron/fragments
		// qDebug() << "refIndex =" << refIndex;
		// qDebug() << "fragmentMaximumIntensities.size() =" << fragmentMaximumIntensities.size();
		// qDebug() << "fragmentMinimumIntensities.size() =" << fragmentMinimumIntensities.size();
        for (int z = 0; z < imageZ; z++)
        {
            for (int y = 0; y < imageY; y++)
            {
                for (int x = 0; x < imageX; x++)
                {
                    if (volumeReader.hasReferenceImage()) {
                        // Reference/nc82
                        float referenceIntensity = referenceProxy.value_at(x,y,z,0);
                        float previousReferenceIntensity = intensityProxy.value_at(x, y, refIndex, 0);
                        if (referenceIntensity > previousReferenceIntensity) {
                            intensityProxy.put_at(x, y, refIndex, 0, referenceIntensity);
                            zProxy.put_at(x, y, refIndex, 0, z);
                        }
                        // update max/min cache
                        if (referenceIntensity > fragmentMaximumIntensities[refIndex])
                            fragmentMaximumIntensities[refIndex] = referenceIntensity;
                        if (referenceIntensity < fragmentMinimumIntensities[refIndex])
                            fragmentMinimumIntensities[refIndex] = referenceIntensity;
                    }

                    // Neurons and Background
                    int maskIndex = 0;
                    if (volumeReader.hasNeuronMask())
                        maskIndex = maskProxy.value_at(x,y,z,0);
                    float previousIntensity = intensityProxy.value_at(x, y, maskIndex, 0);
                    float intensity = 0;
                    for (int c = 0; c < imageC; c++) {
                        float channel_intensity = originalProxy.value_at(x, y, z, c);
                        // accumulate total intensity
                        intensity += channel_intensity;
                        // remember max/min channel intensity
                        if (channel_intensity > fragmentMaximumIntensities[maskIndex])
                            fragmentMaximumIntensities[maskIndex] = channel_intensity;
                        if (channel_intensity < fragmentMinimumIntensities[maskIndex])
                            fragmentMinimumIntensities[maskIndex] = channel_intensity;
                    }
                    if (intensity > previousIntensity) {
                        intensityProxy.put_at(x, y, maskIndex, 0, intensity);
                        zProxy.put_at(x, y, maskIndex, 0, z);
                        for (int c = 0; c < imageC; c++)
                            mipProxy.put_at(x, y, maskIndex, c, originalProxy.value_at(x, y, z, c));
                    }
                }
                if (! volumeReader.refreshLock()) return; // Try to reacquire lock every 25 ms
            }
        }
    } // release locks before emit

    // nerd report
    size_t data_size = 0;
    data_size += fragmentData->getTotalBytes();
    data_size += fragmentZValues->getTotalBytes();
    data_size += fragmentIntensities->getTotalBytes();
    // qDebug() << "Projecting 16-bit fragment MIP images took " << stopwatch.elapsed() / 1000.0 << " seconds";
    // qDebug() << "Projecting 16-bit fragment MIP images absorbed "
    //         << (double)data_size / double(1e6) << " MB of RAM"; // kibibytes boo hoo whatever...

    emit dataChanged(); // declare victory!
}


/////////////////////////////////////
// MipFragmentData::Writer methods //
/////////////////////////////////////

void MipFragmentData::Writer::clearImageData()
{
    m_mipFragmentData.fragmentMinimumIntensities.clear();
    m_mipFragmentData.fragmentMaximumIntensities.clear();

    if (m_mipFragmentData.fragmentData != NULL) {
        delete m_mipFragmentData.fragmentData;
        m_mipFragmentData.fragmentData = NULL;
    }
    if (m_mipFragmentData.fragmentZValues != NULL) {
        delete m_mipFragmentData.fragmentZValues;
        m_mipFragmentData.fragmentZValues = NULL;
    }
    if (m_mipFragmentData.fragmentIntensities != NULL) {
        delete m_mipFragmentData.fragmentIntensities;
        m_mipFragmentData.fragmentIntensities = NULL;
    }
}


