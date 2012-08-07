#ifndef NALOCKABLEDATA_H
#define NALOCKABLEDATA_H

#include <QObject>
#include <QReadWriteLock>
#include <QTime>
#include "SlotMerger.h"

class NaLockableDataBaseReadLocker;
class NaLockableDataBaseWriteLocker;

// NaLockableData is intended to be a base class for NeuronAnnotator
// data flow objects such as VolumeData, VolumeColors, MIPData,
// CurrentZSlice, FragmentMIPs, and MergedMIP.
// NaLockableData provides a QReadWriteLock mechanism for synchronizing
// writes and reads to data.  In exchange, clients are expected to recheck
// the status of read locks every 25 milliseconds or so.
class NaLockableData : public QObject
{
    Q_OBJECT

public:
    friend class NaLockableDataBaseReadLocker;
    friend class NaLockableDataBaseWriteLocker;
    typedef NaLockableDataBaseReadLocker BaseReadLocker;
    typedef NaLockableDataBaseWriteLocker BaseWriteLocker;

public:
    // NaLockableData automatically creates a separate QThread for itself.
    explicit NaLockableData();
    virtual ~NaLockableData();
    bool representsActualData() const {return ! bAbortWrite;}

signals:
    void dataChanged(); // ready for downstream clients to read all data
    void progressMessageChanged(QString msg);
    void progressValueChanged(int); // on a scale of 0-100
    void progressAborted(QString msg); // data update was stopped for some reason
    void progressCompleted(); // successful completion
    void invalidated();

public slots:
    virtual void update() {} // recreate everything from upstream data
    virtual void invalidate() {
        if (bAbortWrite) return;
        bAbortWrite = true;
        emit invalidated();
    }

protected:
    void setRepresentsActualData() {bAbortWrite = false;}

    // Special const access to QReadWriteLock.  Ouch.  Use carefully!
    QReadWriteLock * getLock() const;

    QReadWriteLock lock; // used for multiple-read/single-write thread-safe locking
    QThread * thread; // call constructor with NULL parent to automatically create a thread for NaLocaableData object.

private:
    volatile bool bAbortWrite; // hint to abandon writing because destructor is waiting


protected:
    // Optionally slow down those too-fast update() slot calls
    // like this:
    //     void MyNaLockableData::update() {
    //         SlotMerger updateMerger(statusOfUpdateSlot);
    //         if (! updateMerger.shouldRun()) return;
    //         ...
    SlotStatus statusOfUpdateSlot;

};


// Allocate a NaLockableData::BaseReadLocker on the stack to manage a read lock in a downstream client of NaLockableData.
// NaLockableData::BaseReadLocker is a non-blocking read lock on QReadWriteLock, unlike regular QReadLocker, which is blocking.
class NaLockableDataBaseReadLocker
{
public:
    explicit NaLockableDataBaseReadLocker(const NaLockableData& lockableDataParam);
    virtual ~NaLockableDataBaseReadLocker();
    // Check hasReadLock() after allocating a BaseReadLocker on the stack, to see if it's safe to read.
    bool hasReadLock() const;
    void waitForReadLock(); // Be careful how you use this one!
    // Clients should call refreshLock() every 25 ms or so until done reading.
    // If refreshLock returns "false", stop reading and return, to pop this BaseReadLocker off the stack.
    bool refreshLock();
    // unlock() method for manual early termination, for optimization.
    void unlock();

protected:
    void checkRefreshTime();

private:
    bool m_hasReadLock;
    QReadWriteLock * m_lock;
    bool bWarnOnRefreshTime;
    QTime m_intervalTime;
};


// Allocate a NaLockableData::BaseWriteLocker on the stack while modifying a NaLockableData.
class NaLockableDataBaseWriteLocker : public QWriteLocker
{
public:
    explicit NaLockableDataBaseWriteLocker(NaLockableData& lockableDataParam)
        : QWriteLocker(lockableDataParam.getLock()) {}
};


#endif // NALOCKABLEDATA_H
