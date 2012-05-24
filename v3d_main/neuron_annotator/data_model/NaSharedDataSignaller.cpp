#include "NaSharedDataSignaller.h"

///////////////////////////////////
// NaSharedDataSignaller methods //
///////////////////////////////////

NaSharedDataSignaller::NaSharedDataSignaller() // no parent, because it has its own QThread
    : thread(new QThread(this))
    , lock(QReadWriteLock::NonRecursive)
    , bAbortWrite(false)
{
    thread->start();
    this->moveToThread(thread);
}

/* virtual */
NaSharedDataSignaller::~NaSharedDataSignaller()
{
    bAbortWrite = true;
    thread->quit();
    thread->wait(500);
}

