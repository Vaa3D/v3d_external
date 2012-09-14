#include "PrivateVolumeTexture.h"
#include "VolumeTexture.h"
#include "PrivateDataColorModel.h" // avoid compile error on msvc?
#include "PrivateNeuronFragmentData.h" // avoid compile error on msvc?
#include "NaSharedDataModel.cpp"
#include "../DataFlowModel.h"
#include "Fast3DTexture.h"
#include "../utility/FooDebug.h"

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
    // connect(this, SIGNAL(loadNextVolumeRequested()),
    //         this, SLOT(loadNextVolume()));
    connect(this, SIGNAL(loadNextVolumeRequested()),
            this, SLOT(loadStagedVolumes()));

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
            this, SLOT(loadStagedVolumes()));
    // connect(fast3DTexture, SIGNAL(volumeLoadSequenceCompleted()),
    //         this, SLOT(loadNextVolume()));
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
// TODO obsolete this method in favor of StagedLoader
void VolumeTexture::loadNextVolume()
{
    // qDebug() << "VolumeTexture::loadNextVolume()" << __FILE__ << __LINE__;
    if (fileQueue.empty()) {
        emit volumeLoadSequenceCompleted();
        return;
    }
#ifdef USE_FFMPEG
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
#endif
    // Load raw files one at a time.
    QueuedFile f = fileQueue.front();
    fileQueue.pop_front();
    if (QFile(f.fileName).exists()) {
        // qDebug() << "Loading volume" << f.fileName << __FILE__ << __LINE__;
        // TODO - load one raw file
        assert(f.format == QueuedFile::RAW_FORMAT);
        if (f.volumeType == QueuedFile::LABEL_VOLUME) {
            // qDebug() << "Loading volume" << f.fileName << __FILE__ << __LINE__;
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

// Loads a non-mpeg4 volume
void VolumeTexture::loadOneVolume(ProgressiveCompanion* item, QList<QDir> foldersToSearch)
{
    if (item->isFileItem()) {
        ProgressiveFileCompanion* fileItem = dynamic_cast<
                ProgressiveFileCompanion*>(item);
        QString fileName = fileItem->getFileName(foldersToSearch);
        fooDebug() << "Loading" << fileName << __FILE__ << __LINE__;
        SignalChannel channel = fileItem->second;
        if (channel == CHANNEL_LABEL) {
            setLabelPbdFileName(fileName);
            loadLabelPbdFile();
        }
        else if (channel == CHANNEL_ALPHA) {
            loadReferenceRawFile(fileName);
        }
        else {
            loadSignalRawFile(fileName);
        }
    }
    else {
        assert(false);
        // TODO - non-file volumes
    }
}

// New version of loadNextVolume, for use with StagedLoader object
// Sept 14, 2012 CMB
void VolumeTexture::loadStagedVolumes()
{
    // So we can return to an unloaded item
    static ProgressiveCompanion* pendingCompanion = NULL;
    static bool mpegVolumesAreQueued = false;

    // Might have left one file unloaded last time
    if (pendingCompanion != NULL)
    {
        loadOneVolume(pendingCompanion, progressiveLoader.getFoldersToSearch());
        pendingCompanion = NULL;
    }
    // Loop over all volumes to load
    for (ProgressiveCompanion* item = progressiveLoader.next();
            item != NULL; item = progressiveLoader.next())
    {
        // Mpeg-4 files get loaded to a special queue
        if (item->isMpeg4Volume()) {
            mpegVolumesAreQueued = true;
            ProgressiveFileCompanion* fileItem =
                    dynamic_cast<ProgressiveFileCompanion*>(item);
            QString fileName = fileItem->getFileName(progressiveLoader.getFoldersToSearch());
            SignalChannel channel = fileItem->second;
            emit mpegQueueRequested(fileName, channel);
        }
        // Flush mpeg-4 queue upon observing any non mpeg-4 file
        else if (mpegVolumesAreQueued)
        {
            pendingCompanion = item;
            mpegVolumesAreQueued = false;
            emit mpegLoadSequenceRequested();
            return; // don't worry, we'll be back for the rest
        }
        // Load a non-mpeg 4 volume
        else {
            loadOneVolume(item, progressiveLoader.getFoldersToSearch());
        }
    }
    //
    if (mpegVolumesAreQueued)
    {
        mpegVolumesAreQueued = false;
        emit mpegLoadSequenceRequested();
        return; // don't worry, we'll be back for the rest
    }
    else {
        emit volumeLoadSequenceCompleted();
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
// TODO - obsolete these methods
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

    pbd_count += chooseFinalVolumes(separationDirectory, mvoxels);

    return (mpeg_count + pbd_count) > 0;
}

// Select the highest quality volume files that will fit on the graphics card
int VolumeTexture::chooseFinalVolumes(QDir separationDirectory, int maxMegaVoxels)
{
    int numFilesAdded = 0;

    if (chooseFinalVolume(separationDirectory, maxMegaVoxels, "ConsolidatedLabel2"))
        ++numFilesAdded;
    if (chooseFinalVolume(separationDirectory, maxMegaVoxels, "ConsolidatedSignal2"))
        ++numFilesAdded;
    if (chooseFinalVolume(separationDirectory, maxMegaVoxels, "Reference2"))
        ++numFilesAdded;

    return numFilesAdded;
}

bool VolumeTexture::chooseFinalVolume(QDir separationDirectory, int maxMegaVoxels, QString fileRoot)
{
    QString mv = QString("%1").arg(maxMegaVoxels);
    if (! separationDirectory.exists("fastLoad"))
        return false;
    QDir fl = QDir(separationDirectory.filePath("fastLoad"));
    while (maxMegaVoxels >= 1) {
        QString fileName = fl.filePath(fileRoot+"_"+mv+".v3dpbd");
        if (QFileInfo(fileName).exists())
            return queueFile(fileName);
        maxMegaVoxels = maxMegaVoxels / 2;
    }
    return false;
}

bool VolumeTexture::queueFile(QString fileName)
{
    if (! QFile(fileName).exists())
        return false;
    std::deque<QueuedFile>::const_iterator f;
    // Is this file already in the list?
    for (f = fileQueue.begin(); f != fileQueue.end(); ++f)
        if (f->fileName == fileName)
            return false;
    fileQueue.push_back(QueuedFile(fileName));
}

bool VolumeTexture::hasFastVolumesQueued() const
{
    if (progressiveLoader.size() == 0)
        return false;
    // Check for peristent setting
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    QVariant val = settings.value("NaBUseFastLoad3D");
    if (val.isValid() && (!val.toBool()))
        return false; // There are files, but we won't use them
    // The way we do it now, ANY file item in the queue means fast load is available.
    if (progressiveLoader.hasFileItem())
        return true;
    return false;
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

bool VolumeTexture::queueSeparationFolder(QDir folder) // using new staged loader
{
    progressiveLoader.queueSeparationFolder(folder);

    /// Loading sequence: ///

    // 1) The smallest, fastest mpeg4 signal file we can find
    progressiveLoader.addLoneFile("ConsolidatedSignal2_25.mp4");

    // 2) The largest mpeg4 signal file we can fit in texture memory
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
    // Available sizes are 25*2^n megavoxels
    // six bytes per megavoxel with current texture implementation
    // Look for the largest existing file we can load
    // Subsample options are 25, 50, 100, and 200 megavoxels
    int mvoxels = 25;  // Max megavoxels
    while ((2 * mvoxels * 6) <= max_mb) { // "2" to scale up to the next candidate size
        mvoxels *= 2; // progress 25, 50, 100, 200
    }
    assert(mvoxels >= 25); // lowest sampling rate is 25
    // We are not yet checking whether files exist; That happens at load time.
    // Start with largest, most desirable file name
    int mvoxels0 = mvoxels;
    ProgressiveLoadItem* fullSizeItem = new ProgressiveLoadItem();
    // File candidates are ordered by decreasing size, starting with the
    // largest that could fit in video memory.
    while (mvoxels0 > 25)
    {
        // Load both signal and reference for these volumes
        ProgressiveLoadCandidate* candidate = new ProgressiveLoadCandidate();
        QString mv = QString("%1").arg(mvoxels0);
        // First the full color signal with poor color separation
        *candidate << new ProgressiveFileCompanion("ConsolidatedSignal2_"+mv+".mp4");
        // Second the fourth reference channel to round out the channels
        *candidate << new ProgressiveFileCompanion("Reference2_"+mv+".mp4", CHANNEL_ALPHA); // companion file
        // Next the individual color channels, to sharpen the colors
        *candidate << new ProgressiveFileCompanion("ConsolidatedSignal2Red_"+mv+".mp4", CHANNEL_RED); // companion file
        *candidate << new ProgressiveFileCompanion("ConsolidatedSignal2Green_"+mv+".mp4", CHANNEL_GREEN); // companion file
        *candidate << new ProgressiveFileCompanion("ConsolidatedSignal2Blue_"+mv+".mp4", CHANNEL_BLUE); // companion file
        mvoxels0 /= 2;
        *fullSizeItem << candidate;
    }
    progressiveLoader << fullSizeItem;

    // TODO - where does copying from NaVolumeData to VolumeTexture fit
    // into all this?
    //  - don't load from NaVolumeData if we got lossless data.
    //  - but do load from NaVolumeData if we lack lossless data.
    //  so it's not a part of a sequence, but rather an inferior
    //  candidate at the lossless step.

    // 4) The largest lossless signal file that will fit in texture memory
    ProgressiveLoadItem* losslessItem = new ProgressiveLoadItem();
    mvoxels0 = mvoxels;
    while (mvoxels0 > 25)
    {
        // Load both signal and reference for these volumes
        ProgressiveLoadCandidate* candidate = new ProgressiveLoadCandidate();
        QString mv = QString("%1").arg(mvoxels0);
        // First the full color signal with poor color separation
        *candidate << new ProgressiveFileCompanion("ConsolidatedSignal2_"+mv+".v3dpbd");
        // Second the fourth reference channel to round out the channels
        *candidate << new ProgressiveFileCompanion("Reference2_"+mv+".v3dpbd", CHANNEL_ALPHA); // companion file
        // (lossless version needs no individual color channels)
        *candidate << new ProgressiveFileCompanion("ConsolidatedLabel2_"+mv+".v3dpbd", CHANNEL_LABEL); // companion file
        mvoxels0 /= 2;
        *losslessItem << candidate;
    }
    // Add candidate for loading from NaVolumeData, in case subsampled files
    // are unavailable
    ProgressiveLoadCandidate* volumeDataCandidate = new ProgressiveLoadCandidate();
    const NaVolumeData& volumeData = dataFlowModel->getVolumeData();
    *volumeDataCandidate << new ProgressiveVolumeCompanion(volumeData, CHANNEL_RGB);
    *volumeDataCandidate << new ProgressiveVolumeCompanion(volumeData, CHANNEL_ALPHA);
    *volumeDataCandidate << new ProgressiveVolumeCompanion(volumeData, CHANNEL_LABEL);
    *losslessItem << volumeDataCandidate;
    //
    progressiveLoader << losslessItem;

    return true;
}


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

