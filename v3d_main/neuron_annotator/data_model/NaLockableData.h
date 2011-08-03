#ifndef NALOCKABLEDATA_H
#define NALOCKABLEDATA_H

#include <QObject>
#include <QReadWriteLock>
#include <QTime>
#include <QDebug>

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
    // An instance of the BaseReadLocker class is returned by the NaLockableData::acquireReadLock() method.
    // Allocate a NaLockableData::BaseReadLocker on the stack to manage a read lock in a downstream client of NaLockableData.
    // NaLockableData::BaseReadLocker is a non-blocking read lock on QReadWriteLock, unlike regular QReadLocker, which is blocking.
    class BaseReadLocker
    {
    public:
        explicit BaseReadLocker(QReadWriteLock* lock);
        virtual ~BaseReadLocker();
        // Check hasReadLock() after allocating a BaseReadLocker on the stack, to see if it's safe to read.
        bool hasReadLock() const;
        // Clients should call refreshLock() every 25 ms or so until done reading.
        // If refreshLock returns "false", stop reading and return, to pop this BaseReadLocker off the stack.
        bool refreshLock();
        void unlock() {
            if (hasReadLock()) {
                m_lock->unlock();
                m_hasReadLock = false;
            }
        }

    protected:
        void checkRefreshTime();

    private:
        bool m_hasReadLock;
        QReadWriteLock * m_lock;
        bool bWarnOnRefreshTime;
        QTime m_intervalTime;
    };


public:
    explicit NaLockableData(QObject *parent = NULL);
    virtual ~NaLockableData() {}

signals:
    void dataChanged(); // ready for downstream clients to read all data
    void progressMessage(QString msg);
    void progressAchieved(int); // on a scale of 0-100
    void progressAborted(QString msg); // data update was stopped for some reason

protected:
    // Special const access to QReadWriteLock.  Ouch.  Use carefully!
    QReadWriteLock * getLock() const {
        NaLockableData* mutable_this = const_cast<NaLockableData*>(this);
        return &(mutable_this->lock);
    }

    QReadWriteLock lock; // used for multiple-read/single-write thread-safe locking
};

#endif // NALOCKABLEDATA_H
