#include "NaLockableData.h"
#include <QThread>
#include <QDebug>

NaLockableData::NaLockableData()
    : thread(NULL)
    , lock(QReadWriteLock::Recursive)
    , bAbortWrite(false)
{
    thread = new QThread(this); // is this too circular?
    thread->start();
    this->moveToThread(thread);
}

/* virtual */
NaLockableData::~NaLockableData()
{
    invalidate();
    disconnect(this); // Cut all signals
    // Acquire a write lock before we delete this object,
    // so pending Readers have a chance to finish.
    // As usual, acquire the lock in a local block.
    {
        NaLockableDataBaseWriteLocker(*this);
        // Stop our private computing thread.
        thread->quit();
        thread->wait(500);
    }
}

QReadWriteLock * NaLockableData::getLock() const {
    NaLockableData* mutable_this = const_cast<NaLockableData*>(this);
    return &(mutable_this->lock);
}


////////////////////////////////////////////
// NaLockableData::BaseReadLocker methods //
////////////////////////////////////////////

/* explicit */
NaLockableDataBaseReadLocker::NaLockableDataBaseReadLocker(const NaLockableData& lockableDataParam)
    : m_hasReadLock(false), m_lock(lockableDataParam.getLock()), bWarnOnRefreshTime(false)
{
    refreshLock();
    if (bWarnOnRefreshTime)
        m_intervalTime.start();
}

/* virtual */
NaLockableDataBaseReadLocker::~NaLockableDataBaseReadLocker()
{
    if (m_hasReadLock)
    {
        m_lock->unlock();
        m_hasReadLock = false;
        if (bWarnOnRefreshTime)
            checkRefreshTime();
    }
}

// Check hasReadLock() after allocating a BaseReadLocker on the stack, to see if it's safe to read.
bool NaLockableDataBaseReadLocker::hasReadLock() const
{
    return m_hasReadLock;
}

// Clients should call refreshLock() every 25 ms or so until done reading.
// If refreshLock returns "false", stop reading and return, to pop this BaseReadLocker off the stack.
bool NaLockableDataBaseReadLocker::refreshLock()
{
    if (m_hasReadLock)
    {
        m_lock->unlock();
        m_hasReadLock = false;
        if (bWarnOnRefreshTime)
            checkRefreshTime();
    }
    m_hasReadLock = m_lock->tryLockForRead();
    return m_hasReadLock;
}

void NaLockableDataBaseReadLocker::waitForReadLock() { // Be careful how you use this one!
    if (hasReadLock()) return;
    m_lock->lockForRead();
    m_hasReadLock = true;
    return;
}

void NaLockableDataBaseReadLocker::checkRefreshTime() {
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

void NaLockableDataBaseReadLocker::unlock()
{
    if (hasReadLock()) {
        m_lock->unlock();
        m_hasReadLock = false;
    }
}




