#include "VolumeColors.h"


//////////////////////////
// VolumeColors methods //
//////////////////////////

/* explicit */
VolumeColors::VolumeColors(
                const NaVolumeData& volumeDataParam,
                const DataColorModel& dataColorModelParam,
                const NeuronSelectionModel& neuronSelectionModelParam)
    : volumeData(volumeDataParam)
    , dataColorModel(dataColorModelParam)
    , neuronSelectionModel(neuronSelectionModelParam)
    , volumeColorsImage(NULL)
    , emptyImage(new My4DImage())
    , volumeColorsProxy(emptyImage)
{
    connect(&volumeData, SIGNAL(dataChanged()),
            this, SLOT(update()));
    connect(&dataColorModel, SIGNAL(dataChanged()),
            this, SLOT(update()));
    connect(&neuronSelectionModel, SIGNAL(dataChanged()),
            this, SLOT(update()));
}

/* virtual */
VolumeColors::~VolumeColors() {}

/* virtual */
void VolumeColors::update()
{
    QTime stopwatch;
    stopwatch.start();
    {
        DataColorModel::Reader colorReader(dataColorModel);
        if (dataColorModel.readerIsStale(colorReader)) return;
        if (colorReader.getNumberOfDataChannels() < 1) return;
        NeuronSelectionModel::Reader selectionReader(neuronSelectionModel);
        if (! selectionReader.hasReadLock()) return;
        NaVolumeData::Reader volumeReader(volumeData);
        if (! volumeReader.hasReadLock()) return; // Don't worry; we'll get another data push later.
        const Image4DProxy<My4DImage>& volumeProxy = volumeReader.getOriginalImageProxy();
        const Image4DProxy<My4DImage>& referenceProxy = volumeReader.getReferenceImageProxy();
        const Image4DProxy<My4DImage>& neuronProxy = volumeReader.getNeuronMaskProxy();
        Writer writer(*this);
        writer.clearData();
        // Allocating a new volume is slow, so only reallocate when necessary.
        if (   (volumeColorsImage == NULL)
            || (volumeColorsProxy.sx != volumeProxy.sx)
            || (volumeColorsProxy.sy != volumeProxy.sy)
            || (volumeColorsProxy.sz != volumeProxy.sz) )
        {
            volumeColorsImage = new My4DImage();
            volumeColorsImage->loadImage(
                    volumeProxy.sx, volumeProxy.sy, volumeProxy.sz,
                    4, // RGBA 4 channel
                    V3D_UINT8);
            volumeColorsProxy = Image4DProxy<My4DImage>(volumeColorsImage);
            qDebug() << "Allocating VolumeColors took" << stopwatch.elapsed()/1000.0 << "seconds";
            stopwatch.restart();
        }
        std::vector<double> channelData(volumeProxy.sc + 1, 0.0);
        channelData[volumeProxy.sc] = 0.0; // clear reference channel for r,g,b part of color
        for (int z = 0; z < volumeProxy.sz; ++z)
            for (int y = 0; y < volumeProxy.sy; ++y)
                for (int x = 0; x < volumeProxy.sx; ++x)
                {
                    for (int c = 0; c < volumeProxy.sc; ++c)
                        channelData[c] = volumeProxy.value_at(x, y, z, c);
                    // red, green, blue components derive from non-reference data channels
                    QRgb color = colorReader.blend(channelData);
                    volumeColorsProxy.put_at(x, y, z, 0, qRed(color));
                    volumeColorsProxy.put_at(x, y, z, 1, qGreen(color));
                    volumeColorsProxy.put_at(x, y, z, 2, qBlue(color));

                    // alpha channel contains reference/nc82
                    // Reference channel should be normalized (0-255) but not-yet-colorized reference value
                    double reference_value =
                        colorReader.getReferenceScaledIntensity(referenceProxy.value_at(x, y, z, 0)) * 255.0;
                    volumeColorsProxy.put_at(x, y, z, 3, reference_value);
                }
        qDebug() << "Populating VolumeColors took" << stopwatch.elapsed()/1000.0 << "seconds";
        stopwatch.restart();
        volumeColorsImage->updateminmaxvalues();
        volumeColorsProxy.set_minmax(volumeColorsImage->p_vmin, volumeColorsImage->p_vmax);
        qDebug() << "Minmaxing VolumeColors took" << stopwatch.elapsed()/1000.0 << "seconds";
    } // release locks
    emit dataChanged();
}


//////////////////////////////////
// VolumeColors::Writer methods //
//////////////////////////////////

void VolumeColors::Writer::clearData()
{
    if (volumeColors.volumeColorsImage != NULL) {
        delete volumeColors.volumeColorsImage;
        volumeColors.volumeColorsImage = NULL;
    }
    if (volumeColors.emptyImage != NULL) {
        delete volumeColors.emptyImage;
        volumeColors.emptyImage = NULL;
    }
}


