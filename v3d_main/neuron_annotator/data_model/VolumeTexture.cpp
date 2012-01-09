#include "VolumeTexture.h"
#include "PrivateVolumeTexture.h"
#include "NaSharedDataModel.cpp"

template class NaSharedDataModel<jfrc::PrivateVolumeTexture>;

namespace jfrc {


///////////////////////////
// VolumeTexture methods //
///////////////////////////

VolumeTexture::VolumeTexture()
    : volumeData(NULL)
{}

bool VolumeTexture::initializeGL()
{
    {
        Writer(*this); // acquire lock, waits for Readers to release their locks
        d->initializeGL();
    } // release lock
}

void VolumeTexture::setDataFlowModel(const DataFlowModel& dataFlowModel)
{
    if (volumeData != &dataFlowModel.getVolumeData())
    {
        volumeData = &dataFlowModel.getVolumeData();
        connect(volumeData, SIGNAL(dataChanged()),
                this, SLOT(updateVolume()));
        Writer(*this);
        d->setNeuronSelectionModel(dataFlowModel.getNeuronSelectionModel());
        connect(&dataFlowModel.getNeuronSelectionModel(), SIGNAL(visibilityChanged()),
                this, SLOT(updateNeuronVisibilityTexture()));
    }
}

/* slot */
bool VolumeTexture::updateVolume()
{
    bool bSucceeded = true; // avoid signalling before unlocking
    if (NULL == volumeData) return false;
    int numSlices = 0;
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
    for (int z = 0; z < numSlices; z += deltaZ)
    {
        {
            NaVolumeData::Reader volumeReader(*volumeData);
            if(! volumeReader.hasReadLock()) {
                bSucceeded = false;
                break;
            }
            Writer textureWriter(*this); // acquire lock
            bSucceeded = d->populateVolume(volumeReader, z, z + deltaZ);
        } // release lock
        if (! bSucceeded) break;
        progress = 3.0 + 90.0 * z / numSlices; // 3 - 93 percent in this loop
        emit progressValueChanged((int) progress);
    }
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

const GLuint* VolumeTexture::Reader::Xtex_list() const {
    return d.constData()->getTexIDPtr(jfrc::PrivateVolumeTexture::Stack::X);
}

const GLuint* VolumeTexture::Reader::Ytex_list() const {
    return d.constData()->getTexIDPtr(jfrc::PrivateVolumeTexture::Stack::Y);
}

const GLuint* VolumeTexture::Reader::Ztex_list() const {
    return d.constData()->getTexIDPtr(jfrc::PrivateVolumeTexture::Stack::Z);
}


///////////////////////////////////
// VolumeTexture::Writer methods //
///////////////////////////////////

VolumeTexture::Writer::Writer(VolumeTexture& volumeTexture)
    : BaseWriter(volumeTexture)
{}


} // namespace jfrc

