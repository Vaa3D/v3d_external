#include "StagedFileLoader.h"
#include "NaVolumeData.h"
#include <cassert>


////////////////////////////
/// ProgressiveCompanion ///
////////////////////////////

ProgressiveCompanion::ProgressiveCompanion()
    : isCanceled(false)
{}

/* virtual */
ProgressiveCompanion::~ProgressiveCompanion()
{}

/* virtual */
bool ProgressiveCompanion::isFileItem() const
{
    return false;
}

/* virtual */
bool ProgressiveCompanion::isAvailable(QList<QDir> foldersToSearch) const
{
    return false;
}

/* virtual */
bool ProgressiveCompanion::isMpeg4Volume() const
{
    return false;
}


////////////////////////////////
/// ProgressiveLoadCandidate ///
////////////////////////////////

ProgressiveLoadCandidate::ProgressiveLoadCandidate()
    : currentCompanion(NULL)
    , currentCompanionIndex(-1)
{}

/* virtual */
ProgressiveLoadCandidate::~ProgressiveLoadCandidate()
{
    qDeleteAll(begin(), end());
    clear();
}

/* virtual */
bool ProgressiveLoadCandidate::hasFileItem() const // Are any queued items file items?
{
    for (int i = 0; i < size(); ++i)
        if ((*this)[i]->isFileItem())
            return true;
    return false;
}

/* virtual */
bool ProgressiveLoadCandidate::isAvailable(QList<QDir> foldersToSearch) const
{
    if (size() == 0)
        return false;
    // Have we already loaded the whole set?
    if (currentCompanionIndex >= (size()-1))
        return false;
    // Availability of the first item counts as availability of all companions.
    return (*this)[0]->isAvailable(foldersToSearch);
}

/* virtual */
ProgressiveCompanion* ProgressiveLoadCandidate::next(QList<QDir> foldersToSearch)
{
    if (! isAvailable(foldersToSearch))
        return NULL;
    if (currentCompanion == NULL) { // first time
        currentCompanionIndex = 0;
        currentCompanion = (*this)[0];
        return currentCompanion;
    }
    // shift to next companion
    if (currentCompanionIndex >= (size() - 1))
        return NULL; // at end of list
    ++currentCompanionIndex;
    currentCompanion = (*this)[currentCompanionIndex];
    return currentCompanion;
}

/////////////////////////////
//// ProgressiveLoadItem ////
/////////////////////////////

ProgressiveLoadItem::ProgressiveLoadItem()
    : currentCandidate(NULL)
    , currentCandidateIndex(-1)
{}

/* virtual */
ProgressiveLoadItem::~ProgressiveLoadItem()
{
    qDeleteAll(begin(), end());
    clear();
}

/* virtual */
bool ProgressiveLoadItem::hasFileItem() const // Are any queued items file items?
{
    for (int i = 0; i < size(); ++i)
        if ((*this)[i]->hasFileItem())
            return true;
    return false;
}

// Only returns non-NULL if a better candidate has appeared
// EARLIER in the list.
/* virtual */
ProgressiveCompanion* ProgressiveLoadItem::next(QList<QDir> foldersToSearch)
{
    // First see if a better candidate has arrived earlier in the list.
    int maxIndex = currentCandidateIndex - 1;
    if (currentCandidate == NULL) // first try, initialize
        maxIndex = size() - 1;
    for (int i = 0; i <= maxIndex; ++i) {
        if ((*this)[i]->isAvailable(foldersToSearch)) {
            currentCandidateIndex = i;
            currentCandidate = (*this)[i];
            return currentCandidate->next(foldersToSearch);
        }
    }
    // Are there more companions within this same candidate?
    if (currentCandidate != NULL) {
        ProgressiveCompanion* result = currentCandidate->next(foldersToSearch);
        return result;
    }
    return NULL; // found no better candidate
}

/////////////////////////
/// ProgressiveLoader ///
/////////////////////////

ProgressiveLoader::ProgressiveLoader()
    : currentLoadItem(NULL)
    , currentLoadItemIndex(-1)
{
    clearAll();
    connect(&directoryWatcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(reexamineResultDirectory(QString)));
    prodigalFolderPath = "";
}

/* virtual */
ProgressiveLoader::~ProgressiveLoader()
{
    clearAll();
}

/* virtual */
void ProgressiveLoader::clearAll() // Remove all items
{
    qDeleteAll(begin(), end());
    clear();
    currentLoadItem = NULL;
    foldersToSearch.clear();
}

/* virtual */
void ProgressiveLoader::addLoneFile(QString fileName, SignalChannel channel)
{
    ProgressiveLoadItem* item = new ProgressiveLoadItem();
    ProgressiveLoadCandidate* candidate = new ProgressiveLoadCandidate();
    ProgressiveFileCompanion* companion = new ProgressiveFileCompanion(fileName, channel);
    *candidate << companion;
    *item << candidate;
    *this << item;
}


void ProgressiveLoader::addSearchFolder(QDir folder)
{
    if (! foldersToSearch.contains(folder))
        foldersToSearch << folder;
}

/* virtual */
bool ProgressiveLoader::hasFileItem() const // Are any queued items file items?
{
    for (int i = 0; i < size(); ++i)
        if ((*this)[i]->hasFileItem())
            return true;
    return false;
}

/* virtual */
ProgressiveCompanion* ProgressiveLoader::next()
{
    if (size() == 0)
        return NULL;
    ProgressiveCompanion* result = NULL;
    ProgressiveLoadItem* item = currentLoadItem;
    int index = currentLoadItemIndex;
    if (item == NULL) { // uninitialized, go to first item
        index = 0;
        item = (*this)[index];
    }
    // Search for the next Companion to load
    while (true) {
        // Is there a better candidate in this item?
        result = item->next(foldersToSearch);
        if (result != NULL) {
            break;
        }
        // Try the next item in the series
        ++index;
        if (index >= size()) { // no more items to check
            break; // There was nothing to load, anywhere
        }
        item = (*this)[index];
    }
    // If nothing is found, retain initial state
    if (result != NULL) {
        currentLoadItem = item;
        currentLoadItemIndex = index;
    }
    return result;
}

/* virtual */
void ProgressiveLoader::queueSeparationFolder(QDir folder)
{
    // search in
    //   1) <dirname>
    //   2) <dirname>/fastLoad
    //   3) <dirname>/archive
    //   4) <dirname>/archive/fastLoad
    clearAll();
    // Always look in the primary folder for items
    addSearchFolder(folder);
    // If "fastLoad" directory exists, look there too.
    if (folder.exists("fastLoad")) {
        addSearchFolder(QDir(folder.filePath("fastLoad")));
    }
    // The "archive" subfolder might spring into existence later
    if (folder.exists("archive")) {
        QDir folder2(QDir(folder.filePath("archive")));
        addSearchFolder(folder2);
        if (folder2.exists("fastLoad")) {
            addSearchFolder(QDir(folder2.filePath("fastLoad")));
        }
        else {
            prodigalFolderPath = folder2.absoluteFilePath("fastLoad");
            directoryWatcher.addPath(folder2.canonicalPath());
        }
    }
    else {
        prodigalFolderPath = folder.absoluteFilePath("archive");
        directoryWatcher.addPath(folder.canonicalPath());
    }
}

/* slot */
void ProgressiveLoader::reexamineResultDirectory(QString dirName)
{
    QDir newDir(prodigalFolderPath);
    if (! newDir.exists())
        return;
    // TODO
    assert(false);
}

/* virtual */
void ProgressiveLoader::reset()
{
    currentLoadItem = NULL;
}


///////////////////////////////////////

// ProgressiveFileCompanion methods

/* virtual */
bool ProgressiveFileCompanion::isAvailable(QList<QDir> foldersToSearch) const
{
    QString fileName = getFileName(foldersToSearch);
    return (! fileName.isEmpty());
}

/* virtual */
QString ProgressiveFileCompanion::getFileName(QList<QDir> foldersToSearch) const
{
    QString fileName = first;
    for (int f = 0; f < foldersToSearch.size(); ++f)
        if (foldersToSearch[f].exists(fileName))
            return foldersToSearch[f].absoluteFilePath(fileName);
    return "";
}

/* virtual */
bool ProgressiveFileCompanion::isMpeg4Volume() const
{
#ifdef USE_FFMPEG
    QString fileName = first;
    QString extension = QFileInfo(fileName).suffix().toUpper();
        if (extension == "MP4")
            return true;
#endif
    return false;
}

///////////////////

// TODO - ProgressiveVolumeCompanion methods
/* virtual */
bool ProgressiveVolumeCompanion::isAvailable(QList<QDir> foldersToSearch) const
{
    NaVolumeData::Reader volumeReader(volumeData);
    if (! volumeReader.hasReadLock())
        return false;
    switch (channel)
    {
    case CHANNEL_LABEL:
    {
        const Image4DProxy<My4DImage>& proxy = volumeReader.getNeuronMaskProxy();
        if (proxy.data_p == NULL) return false;
        if (proxy.stride_c < 1) return false;
        return true;
        break;
    }
    case CHANNEL_ALPHA: // reference channel
    {
        const Image4DProxy<My4DImage>& proxy =
                volumeReader.getReferenceImageProxy();
        if (proxy.data_p == NULL) return false;
        if (proxy.stride_c < 1) return false;
        return true;
        break;
    }
    case CHANNEL_RED:
    case CHANNEL_GREEN:
    case CHANNEL_BLUE:
    case CHANNEL_RGB:
    case CHANNEL_RGBA:
    {
        const Image4DProxy<My4DImage>& proxy =
                volumeReader.getOriginalImageProxy(); // signal
        if (proxy.data_p == NULL) return false;
        if (proxy.stride_c < 1) return false;
        return true;
        break;
    }
    default:
        assert(false); // TODO - missing channel
        break;
    }
    return false;
}

