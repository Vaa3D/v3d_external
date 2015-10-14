#include "NaSharedDataSignaller.h"

///////////////////////////////////
// NaSharedDataSignaller methods //
///////////////////////////////////

NaSharedDataSignaller::NaSharedDataSignaller() // no parent, because it has its own QThread
    : thread(new QThread(this))
    , lock(QReadWriteLock::Recursive)
    , bAbortWrite(false)
{
    thread->start();
    this->moveToThread(thread);
}

/* virtual */
NaSharedDataSignaller::~NaSharedDataSignaller()
{
    bAbortWrite = true;
    {
        // acquire write lock to wait for reading clients to finish
        QWriteLocker writeLock(&lock);
        thread->quit();
        thread->wait(500);
    }
}

