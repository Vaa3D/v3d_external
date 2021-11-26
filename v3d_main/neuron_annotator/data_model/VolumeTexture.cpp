#include "PrivateVolumeTexture.h"
#include "VolumeTexture.h"
#include "PrivateDataColorModel.h" // avoid compile error on msvc?
#include "PrivateNeuronFragmentData.h" // avoid compile error on msvc?
#include "NaSharedDataModel.cpp"
#include "../DataFlowModel.h"
#include "Fast3DTexture.h"
#include "../utility/FooDebug.h"
#include "../utility/url_tools.h"

template class NaSharedDataModel<jfrc::PrivateVolumeTexture>;

namespace jfrc {

///////////////////////////
// VolumeTexture methods //
///////////////////////////

VolumeTexture::VolumeTexture()
    : dataFlowModel(NULL)
    , bLoadedFromNaVolumeData(false)
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
    // connect(volumeData, SIGNAL(dataChanged()),
    //         this, SLOT(updateVolume()), Qt::UniqueConnection);

    // Q: What is this for?? Oct 16 2012 CMB
    // A: loadStagedVolumes will call updateVolume() IFF NaVolumeData has the best version of the volume.
    connect(volumeData, SIGNAL(channelsLoaded(int)),
            this, SLOT(loadStagedVolumes()), Qt::UniqueConnection);

    // Needed for loading single volume files
    // connect(volumeData, SIGNAL(channelsLoaded(int)),
    //        this, SLOT(updateVolume()), Qt::UniqueConnection);

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
    connect(&progressiveLoader, SIGNAL(newFoldersFound()),
            this, SLOT(loadStagedVolumes()));

#ifdef USE_FFMPEG
    const Fast3DTexture* fast3DTexture = &dataFlowModel->getFast3DTexture();
    connect(this, SIGNAL(signalTextureChanged()),
            fast3DTexture, SLOT(loadNextVolume())); // Now that previous volume was safely copied
    connect(fast3DTexture, SIGNAL(volumeUploadRequested(int,int,int,void*)),
            this, SLOT(loadFast3DTexture()));
    // Delegate loading of mpeg files to Fast3DTexture
    // Add a mp4 volume to the queue
    connect(this, SIGNAL(mpegQueueRequested(QUrl,int)),
            fast3DTexture, SLOT(queueVolume(QUrl,int)));
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
    if (labelPbdFileUrl.isEmpty())
        return false;
    if (! exists(labelPbdFileUrl))
        return false;
    if (! representsActualData())
        return false;
    {
        Writer textureWriter(*this);
        if (! d->loadLabelPbdFile(labelPbdFileUrl))
            return false;
    }
    emit labelTextureChanged();
    return true;
}

// TODO - load metadata if needed
bool VolumeTexture::loadSignalRawFile(QUrl fileUrl)
{
    if (fileUrl.isEmpty())
        return false;
    if (! exists(fileUrl))
        return false;
    {
        Writer textureWriter(*this);
        if (! d->loadSignalRawFile(fileUrl))
            return false;
        // fooDebug() << d.constData() << d.constData()->getPaddedTextureSize().x() << __FILE__ << __LINE__;
    }
    setRepresentsActualData();
    emit signalTextureChanged();
    return true;
}

// TODO - load metadata if needed
bool VolumeTexture::loadReferenceRawFile(QUrl fileUrl)
{
    if (fileUrl.isEmpty())
        return false;
    if (! exists(fileUrl))
        return false;
    {
        Writer textureWriter(*this);
        if (! d->loadReferenceRawFile(fileUrl))
            return false;
    }
    setRepresentsActualData();
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

// Loads a non-mpeg4 volume
void VolumeTexture::loadOneVolume(ProgressiveCompanion* item, QList<QUrl> foldersToSearch)
{
   if ( item->isFileItem() )
   {
      ProgressiveFileCompanion *fileItem =
          dynamic_cast< ProgressiveFileCompanion * >( item );
      for ( int i = 0; i < fileItem->count(); ++i )
      {

         QUrl fileUrl = fileItem->getFileUrl( foldersToSearch, i );
         // qDebug() << "VolumeTexture::loadOneVolume" << fileUrl << __FILE__ << __LINE__;
         // fooDebug() << "Loading" << fileName << __FILE__ << __LINE__;
         SignalChannel channel = ( *fileItem )[ i ].channel;
         if ( channel == CHANNEL_LABEL )
         {
            setLabelPbdFileUrl( fileUrl );
            loadLabelPbdFile();
         }
         else if ( channel == CHANNEL_ALPHA )
         {
            loadReferenceRawFile( fileUrl );
         }
         else
         {
            loadSignalRawFile( fileUrl );
         }
      }
    }
    else {
        // TODO - non-file volumes
        // assert(false);
        updateVolume();
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
            int index = 0;
            QUrl fileUrl = fileItem->getFileUrl(progressiveLoader.getFoldersToSearch(), index);
            // Only get here for a single file companion
            SignalChannel channel = (*fileItem)[0].channel;
            emit mpegQueueRequested(fileUrl, channel);
        }
        // Flush mpeg-4 queue upon observing any non mpeg-4 file
        else if (mpegVolumesAreQueued)
        {
            pendingCompanion = item;
            break;
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
    else if (! bLoadedFromNaVolumeData) {
        emit volumeLoadSequenceCompleted();
    }
}

/* slot */
bool VolumeTexture::updateVolume()
{
    if (bLoadedFromNaVolumeData)
        return false; // already loaded
    // fooDebug() << "VolumeTexture::updateVolume()" << __FILE__ << __LINE__;
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
            // if (volumeReader.doUpdateSignalTexture()) {
            if (true) {
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
        bLoadedFromNaVolumeData = true;
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
        // d->setMetadata(md);
        // d->originalImageSize = md.originalImageSize;
        // d->paddedTextureSize = md.paddedImageSize;
        // d->usedTextureSize = md.usedImageSize;
        // d->subsampleScale = md.usedImageSize.x()/md.originalImageSize.x();
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

bool VolumeTexture::queueSeparationFolder(QUrl url) // using new staged loader
{
    progressiveLoader.queueSeparationFolder(url);

    /// Loading sequence: ///

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

    ProgressiveLoadItem* losslessItem = new ProgressiveLoadItem();

// Only mp4 files in this block
#ifdef USE_FFMPEG
    bool useFastLoad = true; // by default, but user preference might say otherwise
    QVariant flval = settings.value("NaBUseFastLoad3D");
    if (flval.isValid())
        useFastLoad = flval.toBool();
    if (useFastLoad)
    {
        // 1) The smallest, fastest mpeg4 signal file we can find
        progressiveLoader.addLoneFile("ConsolidatedSignal2_25.mp4");

        ProgressiveLoadItem* fullSizeItem = new ProgressiveLoadItem();
        // File candidates are ordered by decreasing size, starting with the
        // largest that could fit in video memory.
        while (mvoxels0 > 25)
        {
            // Load both signal and reference for these volumes
            ProgressiveLoadCandidate* candidate = new ProgressiveLoadCandidate();
            QString mv = QString("%1").arg(mvoxels0);
            // First the full color signal with poor color separation
            *candidate << new ProgressiveSingleFileCompanion("ConsolidatedSignal2_"+mv+".mp4");
            // Second the fourth reference channel to round out the channels
            *candidate << new ProgressiveSingleFileCompanion("Reference2_"+mv+".mp4", CHANNEL_ALPHA); // companion file
            // Next the individual color channels, to sharpen the colors
            *candidate << new ProgressiveSingleFileCompanion("ConsolidatedSignal2Red_"+mv+".mp4", CHANNEL_RED); // companion file
            *candidate << new ProgressiveSingleFileCompanion("ConsolidatedSignal2Green_"+mv+".mp4", CHANNEL_GREEN); // companion file
            *candidate << new ProgressiveSingleFileCompanion("ConsolidatedSignal2Blue_"+mv+".mp4", CHANNEL_BLUE); // companion file
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

        // Feb 2013, BUT loading lossless v3dpbd into VolumeTexture is not
        // working correctly for some reason. So leave it out for non-FFMPEG
        // case for now...

        // 4) The largest lossless signal file that will fit in texture memory
        mvoxels0 = mvoxels;
        while (mvoxels0 > 25)
        {
            // Load both signal and reference for these volumes
            ProgressiveLoadCandidate* candidate = new ProgressiveLoadCandidate();
            QString mv = QString("%1").arg(mvoxels0);
            // First the full color signal with poor color separation
            *candidate << new ProgressiveSingleFileCompanion("ConsolidatedSignal2_"+mv+".v3dpbd");
            // Second the fourth reference channel to round out the channels
            *candidate << new ProgressiveSingleFileCompanion("Reference2_"+mv+".v3dpbd", CHANNEL_ALPHA); // companion file
            // (lossless version needs no individual color channels)
            *candidate << new ProgressiveSingleFileCompanion("ConsolidatedLabel2_"+mv+".v3dpbd", CHANNEL_LABEL); // companion file
            mvoxels0 /= 2;
            *losslessItem << candidate;
        }

        // Add candidate for loading from NaVolumeData, in case subsampled files
        // are unavailable
        /*
           ProgressiveLoadCandidate* volumeDataCandidate = new ProgressiveLoadCandidate();
           const NaVolumeData& volumeData = dataFlowModel->getVolumeData();
           *volumeDataCandidate << new ProgressiveVolumeCompanion(volumeData, CHANNEL_RGB);
           *volumeDataCandidate << new ProgressiveVolumeCompanion(volumeData, CHANNEL_ALPHA);
           *volumeDataCandidate << new ProgressiveVolumeCompanion(volumeData, CHANNEL_LABEL);
           *losslessItem << volumeDataCandidate;
        */
    }
#endif

    queueVolumeData(*losslessItem); // last resort load from NaVolumeData

    progressiveLoader << losslessItem;

    return true;
}

void VolumeTexture::queueVolumeData(ProgressiveLoadItem& losslessItem)
{
    // Add candidate for loading from NaVolumeData, in case subsampled files
    // are unavailable
    ProgressiveLoadCandidate* volumeDataCandidate = new ProgressiveLoadCandidate();
    const NaVolumeData& volumeData = dataFlowModel->getVolumeData();
    *volumeDataCandidate << new ProgressiveVolumeCompanion(volumeData, CHANNEL_RGB);
    *volumeDataCandidate << new ProgressiveVolumeCompanion(volumeData, CHANNEL_ALPHA);
    *volumeDataCandidate << new ProgressiveVolumeCompanion(volumeData, CHANNEL_LABEL);
    losslessItem << volumeDataCandidate;
}

void VolumeTexture::queueVolumeData()
{
    ProgressiveLoadItem* losslessItem = new ProgressiveLoadItem();
    queueVolumeData(*losslessItem);
    progressiveLoader << losslessItem;
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
    return d.constData()->getOriginalImageSize();
}

const jfrc::Dimension& VolumeTexture::Reader::usedTextureSize() const
{
    return d.constData()->getUsedTextureSize();
}

const jfrc::Dimension& VolumeTexture::Reader::paddedTextureSize() const
{
    // fooDebug() << d.constData() << d.constData()->getPaddedTextureSize().x() << __FILE__ << __LINE__;
    return d.constData()->getPaddedTextureSize();
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

