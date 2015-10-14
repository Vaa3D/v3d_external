#include "ZSliceColors.h"
#include "../DataFlowModel.h"
#include "../gui/Na2DViewer.h"

/* explicit */
ZSliceColors::ZSliceColors(const NaVolumeData& volumeDataParam,
                           const DataColorModel& dataColorModelParam,
                           const NeuronSelectionModel& neuronSelectionModelParam)
    : volumeData(volumeDataParam)
    , dataColorModel(dataColorModelParam)
    , neuronSelectionModel(neuronSelectionModelParam)
    , currentZIndex(-1)
    , image(NULL)
{
    // qDebug() << "ZSliceColors constructor";
    connect(&volumeData, SIGNAL(dataChanged()),
            this, SLOT(update()));
    connect(&dataColorModel, SIGNAL(dataChanged()),
            this, SLOT(update()));
    connect(&neuronSelectionModel, SIGNAL(visibilityChanged()),
            this, SLOT(update()));
    invalidate();
}

/* virtual */
ZSliceColors::~ZSliceColors()
{
    // qDebug() << "ZSliceColors destructor";
    invalidate();
    Writer writer(*this);
    writer.clearData();
}

/* slot: virtual */
void ZSliceColors::update()
{
    // Sometimes the update() signals come too fast.
    SlotMerger updateMerger(statusOfUpdateSlot);
    if (! updateMerger.shouldRun()) return;

    // qDebug() << "ZSliceColors update";
    QTime stopwatch;
    stopwatch.start();
    if (currentZIndex < 0) return;
    { // curly brackets for read/write locks
        DataColorModel::Reader colorReader(dataColorModel);
        if (dataColorModel.readerIsStale(colorReader)) return;
        NeuronSelectionModel::Reader selectionReader(neuronSelectionModel);
        if (! selectionReader.hasReadLock()) return;
        NaVolumeData::Reader volumeReader(volumeData);
        if (! volumeReader.hasReadLock()) return;
        const Image4DProxy<My4DImage>& volumeProxy = volumeReader.getOriginalImageProxy();
        if (currentZIndex >= volumeProxy.sz) return; // inconsistent state
        Writer zWriter(*this);
        if ( (! image) || (image->width() != volumeProxy.sx) || (image->height() != volumeProxy.sy) )
        {
            zWriter.clearData();
            image = new QImage(volumeProxy.sx, volumeProxy.sy, QImage::Format_RGB32);
        }
        std::vector<double> channelData(volumeProxy.sc + 1, 0.0); // +1 for reference
        channelData[volumeProxy.sc] = 0.0; // clear reference channel
        const Image4DProxy<My4DImage>& referenceProxy = volumeReader.getReferenceImageProxy();
        const Image4DProxy<My4DImage>& neuronProxy = volumeReader.getNeuronMaskProxy();

        // Is the reference channel visible?  It depends...
        bool bShowReference = true;
        if (selectionReader.getOverlayStatusList().size() > DataFlowModel::REFERENCE_MIP_INDEX)
             bShowReference = selectionReader.getOverlayStatusList()[DataFlowModel::REFERENCE_MIP_INDEX];
        if (! volumeReader.hasReferenceImage())
            bShowReference = false;

        bool bShowBackground = true;
        if (selectionReader.getOverlayStatusList().size() > DataFlowModel::BACKGROUND_MIP_INDEX)
             bShowBackground = selectionReader.getOverlayStatusList()[DataFlowModel::BACKGROUND_MIP_INDEX];

        for (int y = 0; y < volumeProxy.sy; ++y)
        {
            for (int x = 0; x < volumeProxy.sx; ++x)
            {
                // Investigate visibility of the neuron in this voxel
                bool bShowNeuron = true;
                if (volumeReader.hasNeuronMask())
                {
                    int neuronIndex = neuronProxy.value_at(x, y, currentZIndex, 0);
                    if (neuronIndex == 0) // background channel
                        bShowNeuron = bShowBackground;
                    else if (selectionReader.getMaskStatusList().size() >= neuronIndex)
                        bShowNeuron = selectionReader.getMaskStatusList()[neuronIndex - 1];
                }
                for (int c = 0; c < volumeProxy.sc; ++c)
                {
                    if (bShowNeuron)
                        channelData[c] = volumeProxy.value_at(x, y, currentZIndex, c);
                    else
                        channelData[c] = 0.0;
                }
                // Reference channel
                if (bShowReference)
                    channelData[volumeProxy.sc] = referenceProxy.value_at(x, y, currentZIndex, 0);
                image->setPixel(x, y, colorReader.blend(&channelData[0]));
            }
            if (dataColorModel.readerIsStale(colorReader)) return;
        }
    } // release locks before emit

    // qDebug() << "ZSliceColors update took" << stopwatch.elapsed() / 1000.0 << "seconds"; // takes 31 ms for 512x512 x 3
    // qDebug() << "ZSliceColors occupies" << 4 * image->width() * image->height() / 1000.0 << "MB of RAM";

    setRepresentsActualData();
    emit dataChanged();
}

/* slot */
void ZSliceColors::setZIndex(int z)
{
    // qDebug() << "ZSliceColors::setZIndex" << z;
    if (z == currentZIndex) return; // no change
    {
        Writer writer(*this);
        currentZIndex = z;
    } // release write lock
    update();
}


void ZSliceColors::onCameraFocusChanged(const Vector3D& focus)
{
    // qDebug() << "ZSliceColors::onCameraFocusChanged()" << focus.x() << focus.y() << focus.z();
    int z = int(floor(focus.z() + 0.5));
    if (z < 0) return;
    NaVolumeData::Reader volumeReader(volumeData);
    if (! volumeReader.hasReadLock()) return;
    const Image4DProxy<My4DImage>& volumeProxy = volumeReader.getOriginalImageProxy();
    if (volumeProxy.sz <= 0) return;
    float midZ = (volumeProxy.sz - 1) / 2.0f;
    z = int(midZ + Na2DViewer::flip_Z * (z - midZ));
    setZIndex(z);
}


//////////////////////////////////
// ZSliceColors::Writer methods //
//////////////////////////////////

void ZSliceColors::Writer::clearData()
{
    if (zSliceColors.image) {
        delete zSliceColors.image;
        zSliceColors.image = NULL;
    }
}

