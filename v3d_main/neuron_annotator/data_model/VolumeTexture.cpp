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
{}

bool VolumeTexture::initializeGL() const
{
    bool result = false;
    {
        // Writer(*this); // acquire lock, waits for Readers to release their locks
        result = d.constData()->initializeGL();
    } // release lock
    return result;
}

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
    // TODO - what if dataFlowModel is NULL?
    d->setNeuronSelectionModel(dataFlowModel->getNeuronSelectionModel());
    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(dataChanged()),
            this, SLOT(updateNeuronVisibilityTexture()), Qt::UniqueConnection);
}

/* slot */
bool VolumeTexture::updateVolume()
{
    // qDebug() << "VolumeTexture::updateVolume()" << __FILE__ << __LINE__;
    bool bSucceeded = true; // avoid signalling before unlocking
    if (NULL == volumeData) return false;
    size_t numSlices = 0;
    {
        NaVolumeData::Reader volumeReader(*volumeData); // acquire lock
        if(! volumeReader.hasReadLock())
            return false;
        d->initializeSizes(volumeReader);
        numSlices = d->usedTextureSize.z();
    } // release lock before emit
    if (numSlices < 1) return false;

    emit progressMessageChanged("Sampling volume for 3D viewer");
    float progress = 1.0; // out of 100
    emit progressValueChanged(int(progress));
    int deltaZ = 10; // Report every 10 slices
    qDebug() << "Populating volume data for 3D viewer";
    {
        NaVolumeData::Reader volumeReader(*volumeData);
        if(volumeReader.hasReadLock()) {
            Writer textureWriter(*this); // acquire lock
            d->initializeSizes(volumeReader);
            if (! d->subsampleColorField(volumeReader))
                bSucceeded = false;
            if (! d->subsampleReferenceField(volumeReader))
                bSucceeded = false;
            if (! d->subsampleLabelField(volumeReader))
                bSucceeded = false;
        }
        else
            bSucceeded = false;
    }
    /*
    for (int z = 0; z < numSlices; z += deltaZ)
    {
        if (!bSucceeded) break;
        {
            NaVolumeData::Reader volumeReader(*volumeData);
            if(! volumeReader.hasReadLock()) {
                bSucceeded = false;
                break;
            }
            Writer textureWriter(*this); // acquire lock
            if (! d->populateVolume(volumeReader, z, z + deltaZ))
                bSucceeded = false;
        } // release lock
        if (! bSucceeded) break;
        progress = 3.0 + 90.0 * z / numSlices; // 3 - 93 percent in this loop
        emit progressValueChanged((int) progress);
    }
    */
    if (bSucceeded) {
        emit progressComplete();
        emit volumeTexturesChanged();
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
        emit neuronVisibilityTextureChanged();
}


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

bool VolumeTexture::Reader::uploadVolumeTexturesToVideoCardGL() const
{
    return d.constData()->uploadVolumeTexturesToVideoCardGL();
}

bool VolumeTexture::Reader::uploadNeuronVisibilityTextureToVideoCardGL() const
{
    return d.constData()->uploadNeuronVisibilityTextureToVideoCardGL();
}

bool VolumeTexture::Reader::uploadColorMapTextureToVideoCardGL() const
{
    return d.constData()->uploadColorMapTextureToVideoCardGL();
}

const void* VolumeTexture::Reader::Xtex_list() const {
    return d.constData()->getTexIDPtr(jfrc::PrivateVolumeTexture::Stack::X);
}

const void* VolumeTexture::Reader::Ytex_list() const {
    return d.constData()->getTexIDPtr(jfrc::PrivateVolumeTexture::Stack::Y);
}

const void* VolumeTexture::Reader::Ztex_list() const {
    return d.constData()->getTexIDPtr(jfrc::PrivateVolumeTexture::Stack::Z);
}

bool VolumeTexture::Reader::use3DSignalTexture() const {
    return d.constData()->use3DSignalTexture();
}

unsigned int VolumeTexture::Reader::signal3DTextureId() const {
    return d.constData()->signal3DTextureId();
}


///////////////////////////////////
// VolumeTexture::Writer methods //
///////////////////////////////////

VolumeTexture::Writer::Writer(VolumeTexture& volumeTexture)
    : BaseWriter(volumeTexture)
{}


} // namespace jfrc

