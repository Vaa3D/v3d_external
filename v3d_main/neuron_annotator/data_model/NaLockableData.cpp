#include "NaLockableData.h"

NaLockableData::NaLockableData(QObject *parent) :
    QObject(parent)
{
}


////////////////////////////////////////////
// NaLockableData::BaseReadLocker methods //
////////////////////////////////////////////

/* explicit */
NaLockableData::BaseReadLocker::BaseReadLocker(QReadWriteLock* lock)
    : m_hasReadLock(false), m_lock(lock), bWarnOnRefreshTime(false)
{
    refreshLock();
    if (bWarnOnRefreshTime)
        m_intervalTime.start();
}

/* virtual */
NaLockableData::BaseReadLocker::~BaseReadLocker()
{
    if (hasReadLock())
    {
        m_lock->unlock();
        m_hasReadLock = false;
        if (bWarnOnRefreshTime)
            checkRefreshTime();
    }
}

// Check hasReadLock() after allocating a BaseReadLocker on the stack, to see if it's safe to read.
bool NaLockableData::BaseReadLocker::hasReadLock() const
{
    return m_hasReadLock;
}

// Clients should call refreshLock() every 25 ms or so until done reading.
// If refreshLock returns "false", stop reading and return, to pop this BaseReadLocker off the stack.
bool NaLockableData::BaseReadLocker::refreshLock()
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

void NaLockableData::BaseReadLocker::checkRefreshTime() {
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



