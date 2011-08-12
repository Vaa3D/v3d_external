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
        if (! colorReader.hasReadLock()) return;
        NeuronSelectionModel::Reader selectionReader(neuronSelectionModel);
        if (! selectionReader.hasReadLock()) return;
        NaVolumeData::Reader volumeReader(volumeData);
        if (! volumeReader.hasReadLock()) return; // Don't worry; we'll get another data push later.
        const Image4DProxy<My4DImage>& volumeProxy = volumeReader.getOriginalImageProxy();
        const Image4DProxy<My4DImage>& referenceProxy = volumeReader.getReferenceImageProxy();
        const Image4DProxy<My4DImage>& neuronProxy = volumeReader.getNeuronMaskProxy();
        Writer writer(*this);
        writer.clearData();
        volumeColorsImage = new My4DImage();
        volumeColorsImage->loadImage(
                volumeProxy.sx, volumeProxy.sy, volumeProxy.sz,
                4, // RGBA 4 channel
                V3D_UINT8);
        volumeColorsProxy = Image4DProxy<My4DImage>(volumeColorsImage);
        std::vector<double> channelData(volumeProxy.sc + 1, 0.0);
        for (int z = 0; z < volumeProxy.sz; ++z)
            for (int y = 0; y < volumeProxy.sy; ++y)
                for (int x = 0; x < volumeProxy.sx; ++x)
                {
                    // TODO
                }
        volumeColorsImage->updateminmaxvalues();
        volumeColorsProxy.set_minmax(volumeColorsImage->p_vmin, volumeColorsImage->p_vmax);
    } // release locks
    qDebug() << "Updating VolumeColors took" << stopwatch.elapsed()/1000.0 << "seconds";
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


