#include "PrivateVolumeTexture.h"
#include "VolumeTexture.h"
#include "PrivateDataColorModel.h" // avoid compile error on msvc?
#include "PrivateNeuronFragmentData.h" // avoid compile error on msvc?
#include "NaSharedDataModel.cpp"
#include "../DataFlowModel.h"

template class NaSharedDataModel<jfrc::PrivateVolumeTexture>;

namespace jfrc {


///////////////////////////
// VolumeTexture methods //
///////////////////////////

VolumeTexture::VolumeTexture()
    : volumeData(NULL)
#ifdef USE_FFMPEG
    , fast3DTexture(NULL)
#endif USE_FFMPEG
{}

void VolumeTexture::setDataFlowModel(const DataFlowModel* dataFlowModel)
{
    if (NULL == dataFlowModel) {
        volumeData = NULL;
        return;
    }

    volumeData = &dataFlowModel->getVolumeData();

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
    fast3DTexture = &dataFlowModel->getFast3DTexture();
    connect(this, SIGNAL(signalTextureChanged()),
            fast3DTexture, SLOT(loadNextVolume())); // Now that previous volume was safely copied
    connect(fast3DTexture, SIGNAL(volumeUploadRequested(int,int,int,void*)),
            this, SLOT(loadFast3DTexture()));
#endif

}

/* slot */
bool VolumeTexture::updateVolume()
{
    // qDebug() << "VolumeTexture::updateVolume()" << __FILE__ << __LINE__;
    bool bSucceeded = true; // avoid signalling before unlocking
    bool bSignalChanged = false;
    bool bLabelChanged = false;
    if (NULL == volumeData) return false;

    emit progressMessageChanged("Sampling volume for 3D viewer");
    float progress = 1.0; // out of 100
    emit progressValueChanged(int(progress));
    qDebug() << "Populating volume data for 3D viewer";
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
            }
            if (d->subsampleLabelField(volumeReader))
                bLabelChanged = true;
            // It is not a failure to have no label field
        }
        else
            bSucceeded = false;
    }
    emit progressValueChanged(80);
    if (bSucceeded) {
        emit progressComplete();
        if (bSignalChanged)
            emit signalTextureChanged();
        if (bLabelChanged)
            emit labelTextureChanged();
    }
    else {
        emit progressAborted("Volume update failed");
    }
    return bSucceeded;
}

/* slot */
void VolumeTexture::updateNeuronVisibilityTexture()
{
    qDebug() << "VolumeTexture::updateNeuronVisibilityTexture()" << __FILE__ << __LINE__;
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
}

#ifdef USE_FFMPEG
bool VolumeTexture::loadFast3DTexture()
{
    if (NULL == fast3DTexture)
        return false;
    {
        Fast3DTexture::Reader textureReader(*fast3DTexture);
        if (! textureReader.hasReadLock())
            return false;
        size_t sx = textureReader.width();
        size_t sy = textureReader.height();
        size_t sz = textureReader.depth();
        const uint8_t* data = textureReader.data();
        Writer(*this);
        if (! d->loadFast3DTexture(sx, sy, sz, data))
            return false;
        Dimension size(sx, sy, sz);
        // TODO - get original and used from file somehow
        d->originalImageSize = d->paddedTextureSize = d->usedTextureSize = size;
        d->subsampleScale = 1.0;
    } // release locks before emit
    emit signalTextureChanged();
    return true;
}
#endif

///////////////////////////////////
// VolumeTexture::Reader methods //
///////////////////////////////////

VolumeTexture::Reader::Reader(const VolumeTexture& volumeTexture)
    : BaseReader(volumeTexture)
{}

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


///////////////////////////////////
// VolumeTexture::Writer methods //
///////////////////////////////////

VolumeTexture::Writer::Writer(VolumeTexture& volumeTexture)
    : BaseWriter(volumeTexture)
{}


} // namespace jfrc

