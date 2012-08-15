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
{
    invalidate();
}

/* virtual */
VolumeTexture::~VolumeTexture()
{
    invalidate();
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
    // During fast load, update NaVolumeData without later updating VolumeTexture
    connect(this, SIGNAL(volumeLoadSequenceCompleted()),
            volumeData, SLOT(loadSecondaryVolumeDataFromFiles()));
    Writer(*this);
    d->setNeuronSelectionModel(dataFlowModel->getNeuronSelectionModel());
    d->setDataColorModel(dataFlowModel->getFast3DColorModel());
    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(dataChanged()),
            this, SLOT(updateNeuronVisibilityTexture()), Qt::UniqueConnection);
    connect(&dataFlowModel->getFast3DColorModel(), SIGNAL(dataChanged()),
            this, SLOT(updateColorMapTexture()), Qt::UniqueConnection);
    // Try to load another volume, either mp4 or raw
    connect(this, SIGNAL(loadNextVolumeRequested()),
            this, SLOT(loadNextVolume()));

#ifdef USE_FFMPEG
    const Fast3DTexture* fast3DTexture = &dataFlowModel->getFast3DTexture();
    connect(this, SIGNAL(signalTextureChanged()),
            fast3DTexture, SLOT(loadNextVolume())); // Now that previous volume was safely copied
    connect(fast3DTexture, SIGNAL(volumeUploadRequested(int,int,int,void*)),
            this, SLOT(loadFast3DTexture()));
    // Delegate loading of mpeg files to Fast3DTexture
    // Add a mp4 volume to the queue
    connect(this, SIGNAL(mpegQueueRequested(QString,int)),
            fast3DTexture, SLOT(queueVolume(QString,int)));
    // Start loading the mp4 queue
    connect(this, SIGNAL(mpegLoadSequenceRequested()),
            fast3DTexture, SLOT(loadNextVolume()));
    // Report back when mp4 loading is done.
    connect(fast3DTexture, SIGNAL(volumeLoadSequenceCompleted()),
            this, SLOT(loadNextVolume()));
#endif

}


bool VolumeTexture::loadLabelPbdFile()
{
    if (labelPbdFileName.isEmpty())
        return false;
    if (! QFileInfo(labelPbdFileName).exists())
        return false;
    if (! representsActualData())
        return false;
    {
        Writer textureWriter(*this);
        if (! d->loadLabelPbdFile(labelPbdFileName))
            return false;
    }
    emit labelTextureChanged();
    return true;
}

bool VolumeTexture::loadSignalRawFile(QString fileName)
{
    if (fileName.isEmpty())
        return false;
    if (! QFileInfo(fileName).exists())
        return false;
    {
        Writer textureWriter(*this);
        if (! d->loadSignalRawFile(fileName))
            return false;
    }
    emit signalTextureChanged();
    return true;
}

bool VolumeTexture::loadReferenceRawFile(QString fileName)
{
    if (fileName.isEmpty())
        return false;
    if (! QFileInfo(fileName).exists())
        return false;
    {
        Writer textureWriter(*this);
        if (! d->loadReferenceRawFile(fileName))
            return false;
    }
    emit signalTextureChanged();
    return true;
}

/*
 signal pathway:

 Classes:
 Fast3DTexture      VolumeTexture          NaVolumeData

 methods()/<signals>:
                  loadNextVolume()
                    <mpegQueueRequested>
                               |
 queueVolume() <---------------|
                    <mpegLoadSequenceRequested>
                               |
 loadNextVolume() <------------|
   (loads mpeg files for 3D viewer)
   <volumeLoadSequenceCompleted>
                  |
                  |------------|
                               |
                               v
                   loadNextVolume()
                     (loads v3draw files for 3D viewer)
                     <volumeLoadSequenceCompleted>--|
                                                    |
                                                    v
                                    loadSecondaryVolumeDataFromFiles()
                                      (loads v3draw files for all viewers)

*/
void VolumeTexture::loadNextVolume()
{
    // qDebug() << "VolumeTexture::loadNextVolume()" << __FILE__ << __LINE__;
    if (fileQueue.empty()) {
        emit volumeLoadSequenceCompleted();
        return;
    }
    // Create a secondary queue for mpeg4 files
    int mpeg_count = 0;
    while(fileQueue.front().format == QueuedFile::MPEG_FORMAT) {
        QueuedFile f = fileQueue.front();
        fileQueue.pop_front();
        BlockScaler::Channel channel = BlockScaler::CHANNEL_RGB;
        if (f.volumeType == QueuedFile::REFERENCE_VOLUME)
            channel = BlockScaler::CHANNEL_ALPHA;
        if (f.volumeType == QueuedFile::SIGNAL_RED_VOLUME)
            channel = BlockScaler::CHANNEL_RED;
        if (f.volumeType == QueuedFile::SIGNAL_GREEN_VOLUME)
            channel = BlockScaler::CHANNEL_GREEN;
        if (f.volumeType == QueuedFile::SIGNAL_BLUE_VOLUME)
            channel = BlockScaler::CHANNEL_BLUE;
        emit mpegQueueRequested(f.fileName, int(channel));
        ++mpeg_count;
        if (fileQueue.empty())
            break;
    }
    if (mpeg_count > 0) {
        emit mpegLoadSequenceRequested();
        return;
    }
    // Load raw files one at a time.
    QueuedFile f = fileQueue.front();
    fileQueue.pop_front();
    if (QFile(f.fileName).exists()) {
        // qDebug() << "Loading volume" << f.fileName << __FILE__ << __LINE__;
        // TODO - load one raw file
        assert(f.format == QueuedFile::RAW_FORMAT);
        if (f.volumeType == QueuedFile::LABEL_VOLUME) {
            qDebug() << "Loading volume" << f.fileName << __FILE__ << __LINE__;
            setLabelPbdFileName(f.fileName);
            loadLabelPbdFile();
        }
        else if (f.volumeType == QueuedFile::REFERENCE_VOLUME) {
            loadReferenceRawFile(f.fileName);
        }
        else {
            loadSignalRawFile(f.fileName);
        }
    }
    if (fileQueue.empty()) {
        emit volumeLoadSequenceCompleted();
    }
    else {
        emit loadNextVolumeRequested();
    }
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
        setRepresentsActualData();
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

// Prepare to load a sequence of files, trading off fast user feedback
// for data accuracy.  Start by loading small, fast, crude volumes,
// and progressively replace with larger, slower, more accurate volumes.
// In the following sequence:
//    1. ConsolidatedSignal2_25.mp4
//    2. ConsolidatedSignal2_100.mp4 (number might not be 100 here and below)
//    3. Reference2_100.mp4
//    4. ConsolidatedSignal2Red_100.mp4
//    5. ConsolidatedSignal2Green_100.mp4
//    6. ConsolidatedSignal2Blue_100.mp4
//    7. ConsolidatedSignal2_100.v3dpbd
//    8. ConsolidatedLabel2_100.v3dpbd
//    9. Reference2_100.v3dpbd
// This completes the population of the 3D viewer, which shows
// a subsampled, lossless, 8-bit truncated version of the data.
// The other viewers load an additional series of files to show
// a not-subsampled version of the data.

bool VolumeTexture::queueFastLoadVolumes(QDir separationDirectory)
{
    int mpeg_count = 0;
    int pbd_count = 0;
    QDir fl = QDir(separationDirectory.filePath("fastLoad"));
    if (! fl.exists()) return false;

    // Figure out the finest/largest subsampled image that will fit on the video card.
    size_t max_mb = 350; // default to max memory of 350 MB
    // Fetch preset maximum texture memory user preference, if any.
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    QVariant val = settings.value("NaMaxVideoMegabytes");
    // qDebug() << "Loading preferences";
    if (val.isValid()) {
        size_t mb = val.toInt();
        if (mb > 0)
            max_mb = mb;
    }
    // Look for the largest existing file we can load
    // Subsample options are 25, 50, 100, and 200 megavoxels
    int mvoxels = 25;  // Max megavoxels
    // six bytes per megavoxel with current texture implementation
    while ((2 * mvoxels * 6) <= max_mb) { // "2" to scale up to the next candidate size
        mvoxels *= 2; // progress 25, 50, 100, 200
    }
    assert(mvoxels >= 25); // lowest sampling rate is 25
    QString mv = QString("%1").arg(mvoxels);
    while ( (mvoxels >= 25) && (! fl.exists("ConsolidatedSignal2_"+mv+".v3dpbd")) )
    {
        mvoxels /= 2;
        QString mv = QString("%1").arg(mvoxels);
    }

    Writer(*this);

#ifdef USE_FFMPEG
    // 25 megavoxels is the smallest/fastest file we use.
    if (queueFile(fl.filePath("ConsolidatedSignal2_25.mp4")))
        ++mpeg_count;
    if (mvoxels > 25)
        if (queueFile(fl.filePath("ConsolidatedSignal2_"+mv+".mp4")))
            ++mpeg_count;
    QVector<QString> froots;
    froots.push_back("Reference2_"+mv+".mp4");
    froots.push_back("ConsolidatedSignal2Red_"+mv+".mp4");
    froots.push_back("ConsolidatedSignal2Green_"+mv+".mp4");
    froots.push_back("ConsolidatedSignal2Blue_"+mv+".mp4");
    for (int f = 0; f < froots.size(); ++f)
        if (queueFile(fl.filePath(froots[f])))
            ++mpeg_count;
#endif

    // Finally load a lossless (but 8-bit truncated) version of the signal/reference volumes
    if (queueFile(fl.filePath("ConsolidatedSignal2_"+mv+".v3dpbd")))
        ++pbd_count;
    if (queueFile(fl.filePath("ConsolidatedLabel2_"+mv+".v3dpbd")))
        ++pbd_count;
    if (queueFile(fl.filePath("Reference2_"+mv+".v3dpbd")))
        ++pbd_count;
    // Success means that at least one volume file was found.

    return (mpeg_count + pbd_count) > 0;
}

bool VolumeTexture::queueFile(QString fileName)
{
    if (! QFile(fileName).exists())
        return false;
    fileQueue.push_back(QueuedFile(fileName));
}

void VolumeTexture::loadVolumeQueue()
{

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
    setRepresentsActualData();
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

