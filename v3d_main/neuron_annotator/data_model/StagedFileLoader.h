#ifndef STAGEDFILELOADER_H
#define STAGEDFILELOADER_H

#include <QFileSystemWatcher>
#include <QPair>
#include <QString>
#include <QUrl>
#include <QObject>

class NaVolumeData;

/**
 * This structure is a bit complicated.
 * But it satisfies the needs of fast progressive file loading.
 *
 * The general hierarchy is thus:
 *   A ProgressiveLoader contains a sequence of ProgressiveLoadItems
 *   These ProgressiveLoadItems will be loaded in the order in which they appear.
 *   For example, the 3D viewer VolumeTexture class will have one ProgressiveLoader
 *   to manage the loading of a particular directory of volume artifacts.
 *   (NaVolumeData will have a second, simpler ProgressiveLoader for its data)
 *     A ProgressiveLoadItem contains a list of ProgressiveLoadCandidates.
 *     The first *available* ProgressiveLoadCandidate will be loaded.
 *     The other ProgressiveLoadCandidates will be ignored.
 *     For example, if the highest resolution desired image is unavailable,
 *     a lower resolution one will be loaded.
 *       A ProgressiveLoadCandidate contains a group of
 *       ProgressiveCompanion volumes to load.
 *       Every available volume in the group will be loaded.
 *       For example, the group might contain the signal, reference, and label volumes
 *       at a particular resolution, file format, and bit depth.
 *         Each ProgressiveCompanion represents a single volume file or
 *         data structure.
 */

/**
 * ProgressiveCompanion loads a single volume
 */
class ProgressiveCompanion
{
public:
    ProgressiveCompanion();
    virtual ~ProgressiveCompanion();
    virtual bool isAvailable(QList<QUrl> foldersToSearch) const;
    virtual bool isFileItem() const;
    virtual bool isMpeg4Volume() const;

protected:
    volatile bool isCanceled; // for quick shut down
};

/**
 * ProgressiveLoadCandidate represents one possible lump of data that
 * a ProgressiveLoadItem might choose to load.  ProgressiveLoadCandidates
 * will be arranged in order of decreasing desirability.
 * For example, higher resolution images might be preferable to lower
 * resolution images, in which case the higher resolution images will
 * appear first.
 */
class ProgressiveLoadCandidate : public QList<ProgressiveCompanion*>
{
public:
    ProgressiveLoadCandidate();
    virtual ~ProgressiveLoadCandidate();
    virtual bool hasFileItem() const;
    virtual bool isAvailable(QList<QUrl> foldersToSearch) const;
    virtual ProgressiveCompanion* next(QList<QUrl> foldersToSearch);

protected:
    ProgressiveCompanion* currentCompanion;
    int currentCompanionIndex;
};


/**
 * ProgressiveLoadItem is a base class for things that are loaded
 * in a sequence of increasingly accurate instances.
 * Every available item in the sequence will be loaded sequentially.
 * Each ProgressiveLoadItem can contain a series of ProgressiveLoadCandidates
 */
class ProgressiveLoadItem : public QList<ProgressiveLoadCandidate*>
{   
public:
    ProgressiveLoadItem();
    virtual ~ProgressiveLoadItem();
    virtual bool hasFileItem() const;
    virtual ProgressiveCompanion* next(QList<QUrl> foldersToSearch); // only if a better it is available EARLIER in the list

protected:
    ProgressiveLoadCandidate* currentCandidate;
    int currentCandidateIndex;
};


enum SignalChannel {
    CHANNEL_RED   = 2, // 2 not 0 because pixels are in bgra order?
    CHANNEL_GREEN = 1,
    CHANNEL_BLUE  = 0,
    CHANNEL_ALPHA = 3, // reference nc82 channel
    CHANNEL_RGB   = 37, // 3 channel signal data
    CHANNEL_RGBA  = 38,
    CHANNEL_LABEL = 39, // not a color at all
};


/**
 * ProgressiveLoader is a base class for things that load a series
 * of ProgressiveLoadItems
 */
class ProgressiveLoader : public QObject, public QList<ProgressiveLoadItem*>
{
    Q_OBJECT

public:
    ProgressiveLoader();
    virtual ~ProgressiveLoader();

    // Loads the next available item in the sequence
    virtual void addLoneFile(QString fileName, SignalChannel channel=CHANNEL_RGB);
    virtual void addSearchFolder(QUrl folder);
    virtual void clearAll(); // Remove all items
    virtual const QList<QUrl>& getFoldersToSearch() const {
        return foldersToSearch;
    }
    virtual bool hasFileItem() const; // Are any queued items file items?
    virtual ProgressiveCompanion* next();
    virtual void queueSeparationFolder(QUrl url);
    virtual void reset(); // restart from the beginning

signals:
    void newFoldersFound();

public slots:
    void reexamineResultDirectory(QString dirName);

protected:
    // loadIncrementalItem() will only attempt to load items appearing AFTER the currentLoadItem
    ProgressiveLoadItem* currentLoadItem; // NULL means nothing is loaded
    int currentLoadItemIndex;
    QList<QUrl> foldersToSearch;
    QFileSystemWatcher directoryWatcher;
    QUrl prodigalFolderPath;

    // loadIncrementalItem() will only attempt to load items appearing AFTER the latestFailedItem?
    // TODO - but maybe the item is available now?
    // ProgressiveLoadItem* latestFailedItem; // So we don't repeat our mistakes
};

struct ProgressiveFileElement
{
    QString file_name;
    SignalChannel channel;
    bool flipped_in_y;

    ProgressiveFileElement(): file_name(""), channel(CHANNEL_RGB), flipped_in_y(false) {}
    ProgressiveFileElement(QString name, SignalChannel ch, bool flipped): 
    file_name(name), channel(ch), flipped_in_y(flipped) {}
};
// Each ProgressiveFileCandidate can contain a set of companion files that
// should be loaded together
class ProgressiveFileCompanion
    : public ProgressiveCompanion
{
public:
    ProgressiveFileCompanion() {}
    virtual QUrl getFileUrl(QList<QUrl> foldersToSearch, int& index_of_file) const = 0;
    virtual bool isAvailable(QList<QUrl> foldersToSearch) const = 0;
    virtual bool isFileItem() const {return true;}
    virtual int count() const = 0;
    virtual bool isMpeg4Volume() const = 0;
    virtual ProgressiveFileElement const& operator[](int i) const = 0;

};


class ProgressiveSingleFileCompanion
    : public ProgressiveFileCompanion
{
public:
    ProgressiveSingleFileCompanion(QString fileName, SignalChannel channel = CHANNEL_RGB)
        : ProgressiveFileCompanion(), _element(fileName, channel, false)
    {}
    QUrl getFileUrl(QList<QUrl> foldersToSearch, int& index_of_file) const;
    bool isAvailable(QList<QUrl> foldersToSearch) const;
    int count() const { return 1; }
    bool isMpeg4Volume() const;
    ProgressiveFileElement const& operator[](int i) const { return _element; }
protected:
    ProgressiveFileElement _element;
};


// Each ProgressiveFileCandidate can contain a set of companion files that
// should be loaded together
class ProgressiveFileChoiceCompanion
    : public ProgressiveFileCompanion
    , public QList< ProgressiveFileElement* >
{
public:
    ProgressiveFileChoiceCompanion(): ProgressiveFileCompanion() {}
    QUrl getFileUrl(QList<QUrl> foldersToSearch, int& index_of_file) const;
    bool isAvailable(QList<QUrl> foldersToSearch) const;
    int count() const { return size(); }
    bool isMpeg4Volume() const;
    ProgressiveFileElement const& operator[](int i) const { return *((*this).QList< ProgressiveFileElement* >::operator[](i)); }
};


class ProgressiveVolumeCompanion : public ProgressiveCompanion
{
public:
     ProgressiveVolumeCompanion(const NaVolumeData& volumeData, SignalChannel channel)
         : volumeData(volumeData)
         , channel(channel)
     {}
     virtual bool isAvailable(QList<QUrl> foldersToSearch) const;

protected:
     const NaVolumeData& volumeData;
     SignalChannel channel;
};



#endif // STAGEDFILELOADER_H
