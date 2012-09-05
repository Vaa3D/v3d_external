#ifndef STAGEDFILELOADER_H
#define STAGEDFILELOADER_H

#include <QObject>
#include <QString>
#include <QDir>


/**
 * ProgressiveLoadItem is a base class for things that are loaded
 * in a sequence of increasingly accurate instances.
 * Every available item in the sequence will be loaded sequentially.
 */
class ProgressiveLoadItem
{   
public:
    virtual ~ProgressiveLoadItem() {}
    virtual bool isAvailable() const = 0;
    virtual bool load() = 0;
    virtual void cancel() = 0;

protected:
    volatile bool isCanceled; // for quick shut down
};


/**
 * ProgressiveLoader is a base class for things that load a series
 * of ProgressiveLoadItems
 */
class ProgressiveLoader
{
public:
    ProgressiveLoader();
    virtual ~ProgressiveLoader() {}

    // Loads the next available item in the sequence
    virtual bool addItem(ProgressiveLoadItem*);
    void clear(); // Remove all items
    virtual bool loadNextItem();
    virtual void reset() {currentLoadItem = NULL;} // restart from the beginning

protected:
    // Every available item in loadSequence will be loaded
    QList<ProgressiveLoadItem*> loadSequence;
    // loadIncrementalItem() will only attempt to load items appearing AFTER the currentLoadItem
    ProgressiveLoadItem* currentLoadItem; // NULL means nothing is loaded

    // loadIncrementalItem() will only attempt to load items appearing AFTER the latestFailedItem?
    // TODO - but maybe the item is available now?
    ProgressiveLoadItem* latestFailedItem; // So we don't repeat our mistakes
};


class ProgressiveFileItem : public virtual ProgressiveLoadItem
{
public:
    enum Channel {
        CHANNEL_RED   = 2, // 2 not 0 because pixels are in bgra order?
        CHANNEL_GREEN = 1,
        CHANNEL_BLUE  = 0,
        CHANNEL_ALPHA = 3,
        CHANNEL_RGB   = 37,
        CHANNEL_RGBA  = 38
    };

    ProgressiveFileItem(QList<QString> fileNames, Channel channel = CHANNEL_RGB)
        : fileNames(fileNames)
        , channel(channel)
    {}

    void setDirectoriesToCheck(QList<QDir> dirs) {
        directoriesToCheck = dirs;
    }

protected:
    // The first available file name in the list will be used
    QList<QString> fileNames;
    Channel channel;
    QList<QDir> directoriesToCheck;
};


#endif // STAGEDFILELOADER_H
