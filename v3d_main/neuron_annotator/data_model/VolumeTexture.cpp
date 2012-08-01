#include "PrivateVolumeTexture.h"
#include "VolumeTexture.h"
#include "PrivateDataColorModel.h" // avoid compile error on msvc?
#include "PrivateNeuronFragmentData.h" // avoid compile error on msvc?
#include "NaSharedDataModel.cpp"
#include "../DataFlowModel.h"
#include "Fast3DTexture.h"

template class NaSharedDataModel<jfrc::PrivateVolumeTexture>;

namespace jfrc {

///////////////////////////
// VolumeTexture methods //
///////////////////////////

VolumeTexture::VolumeTexture()
    : dataFlowModel(NULL)
{}

/* virtual */
VolumeTexture::~VolumeTexture()
{
    Writer(*this); // wait for clients to return before destruction
}

void VolumeTexture::setDataFlowModel(const DataFlowModel* dataFlowModelParam)
{
    dataFlowModel = dataFlowModelParam;

    if (NULL == dataFlowModel)
        return;

    const NaVolumeData* volumeData = &dataFlowModel->getVolumeData();

    // qDebug() << "Connecting NaVolumeData::dataChanged() to VolumeTexture::updateVolume()";
    // I cannot understand why this signal gets disconnected between loads sometimes, but it does.
    // ...so try reestablishing it every time.  Thank you Qt::UniqueConnection.
    connect(volumeData, SIGNAL(dataChanged()),
            this, SLOT(updateVolume()), Qt::UniqueConnection);
    Writer(*this);
    d->setNeuronSelectionModel(dataFlowModel->getNeuronSelectionModel());
    d->setDataColorModel(dataFlowModel->getFast3DColorModel());
    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(dataChanged()),
            this, SLOT(updateNeuronVisibilityTexture()), Qt::UniqueConnection);
    connect(&dataFlowModel->getFast3DColorModel(), SIGNAL(dataChanged()),
            this, SLOT(updateColorMapTexture()), Qt::UniqueConnection);

#ifdef USE_FFMPEG
    const Fast3DTexture* fast3DTexture = &dataFlowModel->getFast3DTexture();
    fast3DTexture = &dataFlowModel->getFast3DTexture();
    connect(this, SIGNAL(signalTextureChanged()),
            fast3DTexture, SLOT(loadNextVolume())); // Now that previous volume was safely copied
    connect(fast3DTexture, SIGNAL(volumeUploadRequested(int,int,int,void*)),
            this, SLOT(loadFast3DTexture()));
#endif

}


bool VolumeTexture::loadLabelPbdFile()
{
    if (labelPbdFileName.isEmpty())
        return false;
    if (! QFileInfo(labelPbdFileName).exists())
        return false;
    {
        Writer textureWriter(*this);
        d->loadLabelPbdFile(labelPbdFileName);
    }
    emit labelTextureChanged();
    return true;
}


/* slot */
bool VolumeTexture::updateVolume()
{
    // qDebug() << "VolumeTexture::updateVolume()" << __FILE__ << __LINE__;
    if (NULL == dataFlowModel) return false;

    bool bSucceeded = true; // avoid signalling before unlocking
    bool bSignalChanged = false;
    bool bLabelChanged = false;
    bool bMetadataChanged = false;

    const NaVolumeData* volumeData = &dataFlowModel->getVolumeData();

    emit progressMessageChanged("Sampling volume for 3D viewer");
    // emit benchmarkTimerPrintRequested("Starting to sample 3D volume");
    float progress = 1.0; // out of 100
    emit progressValueChanged(int(progress));
    // qDebug() << "Populating volume data for 3D viewer" << __FILE__ << __LINE__;
    {
        NaVolumeData::Reader volumeReader(*volumeData);
        if(volumeReader.hasReadLock()) {
            Writer textureWriter(*this); // acquire lock
            d->initializeSizes(volumeReader);
            if (volumeReader.doUpdateSignalTexture()) {
                if (d->subsampleColorField(volumeReader))
                    bSignalChanged = true;
                else
                    bSucceeded = false;
                if (! d->subsampleReferenceField(volumeReader))
                    bSucceeded = false;

                // Reset Slow 3D color
                SampledVolumeMetadata md = d.constData()->getMetadata();
                const Image4DProxy<My4DImage>& volProxy = volumeReader.getOriginalImageProxy();
                for (int c = 0; c < volProxy.sc; ++c) {
                    md.channelGamma[c] = 1.0;
                    md.channelHdrMinima[c] = volProxy.vmin[c];
                    md.channelHdrMaxima[c] = volProxy.vmax[c];
                    // qDebug() << "volume hdr max =" << volProxy.vmax[c] << c << __FILE__ << __LINE__;
                }
                if (volumeReader.hasReferenceImage()) {
                    const Image4DProxy<My4DImage>& refProxy = volumeReader.getReferenceImageProxy();
                    int c = 3;
                    md.channelGamma[c] = 1.0;
                    md.channelHdrMinima[c] = refProxy.vmin[0];
                    md.channelHdrMaxima[c] = refProxy.vmax[0];
                    // qDebug() << "volume hdr max =" << refProxy.vmax[0] << c << __FILE__ << __LINE__;
                }
                // TODO
                d->setMetadata(md);
                bMetadataChanged = true;

                if (d->subsampleLabelField(volumeReader))
                    bLabelChanged = true;
                // It is not a failure to have no label field
            }
        }
        else
            bSucceeded = false;
    }
    emit progressValueChanged(80);
    if (bSucceeded) {
        // emit benchmarkTimerPrintRequested("Finished sampling 3D volume");
        emit progressComplete();
        if (bSignalChanged)
            emit signalTextureChanged();
        if (bLabelChanged)
            emit labelTextureChanged();
        if (bMetadataChanged)
            emit signalMetadataChanged();
    }
    else {
        emit progressAborted("Volume update failed");
    }
    return bSucceeded;
}

/* slot */
void VolumeTexture::updateNeuronVisibilityTexture()
{
    // qDebug() << "VolumeTexture::updateNeuronVisibilityTexture()" << __FILE__ << __LINE__;
    bool bSucceeded = true;
    {
        Writer(*this);
        bSucceeded = d->updateNeuronVisibilityTexture();
    }
    if (bSucceeded)
        emit visibilityTextureChanged();
}

bool VolumeTexture::updateColorMapTexture()
{
    // qDebug() << "updateColorMapTexture()" << __FILE__ << __LINE__;
    bool bSucceeded = true;
    {
        Writer(*this);
        bSucceeded = d->updateColorMapTexture();
    }
    if (bSucceeded) {
        // qDebug() << "emitting colorMapTextureChanged()" << __FILE__ << __LINE__;
        emit colorMapTextureChanged();
    }
    return bSucceeded;
}

#ifdef USE_FFMPEG
bool VolumeTexture::loadFast3DTexture()
{
    if (NULL == dataFlowModel)
        return false;
    const Fast3DTexture* fast3DTexture = &dataFlowModel->getFast3DTexture();
    QElapsedTimer timer;
    timer.start();
    // emit benchmarkTimerPrintRequested("Started VolumeTexture::loadFast3DTexture()");
    // qDebug() << "VolumeTexture::loadFast3DTexture()" << __FILE__ << __LINE__;
    if (NULL == fast3DTexture)
        return false;
    bool bMetadataChanged = false;
    {
        Fast3DTexture::Reader textureReader(*fast3DTexture);
        if (! textureReader.hasReadLock())
            return false;
        size_t sx = textureReader.width();
        size_t sy = textureReader.height();
        size_t sz = textureReader.depth();
        const uint8_t* data = textureReader.data();
        Writer(*this);
        const SampledVolumeMetadata& md = textureReader.metadata();
        if (md != d.constData()->getMetadata()) {
            bMetadataChanged = true;
            d->setMetadata(md);
        }
        d->originalImageSize = md.originalImageSize;
        d->paddedTextureSize = md.paddedImageSize;
        d->usedTextureSize = md.usedImageSize;
        d->subsampleScale = md.usedImageSize.x()/md.originalImageSize.x();
        if (! d->loadFast3DTexture(sx, sy, sz, data))
            return false;
    } // release locks before emit
    emit signalTextureChanged();
    if (bMetadataChanged)
        emit signalMetadataChanged();
    // emit benchmarkTimerPrintRequested("Finished VolumeTexture::loadFast3DTexture()");
    // qDebug() << "VolumeTexture::loadFast3DTexture() took" << timer.elapsed() << "milliseconds";
    return true;
}
#endif

///////////////////////////////////
// VolumeTexture::Reader methods //
///////////////////////////////////

VolumeTexture::Reader::Reader(const VolumeTexture& volumeTexture)
    : BaseReader(volumeTexture)
{}

/* virtual */
VolumeTexture::Reader::~Reader()
{
}

const jfrc::Dimension& VolumeTexture::Reader::originalImageSize() const
{
    return d.constData()->originalImageSize;
}

const jfrc::Dimension& VolumeTexture::Reader::usedTextureSize() const
{
    return d.constData()->usedTextureSize;
}

const jfrc::Dimension& VolumeTexture::Reader::paddedTextureSize() const
{
    return d.constData()->paddedTextureSize;
}

bool VolumeTexture::Reader::use3DSignalTexture() const {
    return d.constData()->use3DSignalTexture();
}

const uint32_t* VolumeTexture::Reader::signalData3D() const {
    return d.constData()->signalData3D();
}

const uint16_t* VolumeTexture::Reader::labelData3D() const {
    return d.constData()->labelData3D();
}

const uint32_t* VolumeTexture::Reader::visibilityData2D() const {
    return d.constData()->visibilityData2D();
}

const uint32_t* VolumeTexture::Reader::colorMapData2D() const {
    return d.constData()->colorMapData2D();
}

const SampledVolumeMetadata& VolumeTexture::Reader::metadata() const {
    return d.constData()->getMetadata();
}

///////////////////////////////////
// VolumeTexture::Writer methods //
///////////////////////////////////

VolumeTexture::Writer::Writer(VolumeTexture& volumeTexture)
    : BaseWriter(volumeTexture)
{}


} // namespace jfrc

