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


    // An instance of the ReadLocker class is returned by the NaLockableData::acquireReadLock() method.
    // Allocate a NaLockableData::ReadLocker on the stack to manage a read lock in a downstream client of NaLockableData.
    // NaLockableData::ReadLocker is a non-blocking read lock on QReadWriteLock, unlike regular QReadLocker, which is blocking.
    class ReadLocker
    {
    public:
        explicit ReadLocker(QReadWriteLock* lock)
            : m_hasReadLock(false), m_lock(lock), bWarnOnRefreshTime(false)
        {
            refreshLock();
            if (bWarnOnRefreshTime)
                m_intervalTime.start();
        }

        virtual ~ReadLocker()
        {
            if (hasReadLock())
            {
                m_lock->unlock();
                m_hasReadLock = false;
                if (bWarnOnRefreshTime)
                    checkRefreshTime();
            }
        }

        // Check hasReadLock() after allocating a ReadLocker on the stack, to see if it's safe to read.
        bool hasReadLock() const
        {
            return m_hasReadLock;
        }

        // Clients should call refreshLock() every 25 ms or so until done reading.
        // If refreshLock returns "false", stop reading and return, to pop this ReadLocker off the stack.
        bool refreshLock()
        {
            if (hasReadLock())
            {
                m_lock->unlock();
                m_hasReadLock = false;
                if (bWarnOnRefreshTime)
                    checkRefreshTime();
            }
            m_hasReadLock = m_lock->tryLockForRead();
            return m_hasReadLock;
        }

    protected:

        void checkRefreshTime() {
            int interval = m_intervalTime.elapsed();
            if (interval < 5) {
                // TODO - these should be exceptions, so the message can come from the best stack frame.
                qDebug() << "Read lock refresh interval was less than 5 milliseconds (" << interval << " ms)";
            }
            if (interval > 100) {
                qDebug() << "Read lock refresh interval was greater than 100 milliseconds (" << interval << " ms)";
            }
            m_intervalTime.restart();
        }

    private:
        bool m_hasReadLock;
        QReadWriteLock * m_lock;
        bool bWarnOnRefreshTime;
        QTime m_intervalTime;
    };



public:
    explicit NaLockableData(QObject *parent = NULL);

    // override writeData() with data update code.  updateData() method will handle WRITE locking.
    // Returns "true" if data were successfully changed.
    // Remember to refresh any upstream read locks every 25 ms or so within derived implementations of writeData().
    virtual bool writeData() {
        // default implementation does not actually change anything
        return false;
    }

    // to be called from downstream clients in other threads
    ReadLocker acquireReadLock() {
        return ReadLocker(&lock);
    }

signals:
    void dataChanged(); // ready for downstream clients to read all data
    void progressMessage(QString msg);
    void progressAchieved(int); // on a scale of 0-100
    void progressAborted(QString msg); // data update was stopped for some reason

public slots:
    // Connect updateData() slot to upstream data source dataChanged() signals.
    virtual void updateData() // time to rewrite data from upstream sources
    {
        // Always call updateData() for this object from the same thread, to avoid deadlock.
        // Preferably from a dedicated writing thread to which this object has been ".moveToThread()"ed.
        QWriteLocker locker(&lock); // blocking acquisition of write lock, allocated on stack for release safety
        if (writeData()) { // write succeeded...
            locker.unlock(); // Release write lock early, so the following signal is safer
            emit dataChanged(); // Tell downstream clients to start reading.
        }
        // locker was allocated on the stack, so lock will automatically be released when this method returns.
    }

protected:
    QReadWriteLock lock; // used for multiple-read/single-write thread-safe locking
};

#endif // NALOCKABLEDATA_H
