#include "NaSharedDataSignaller.h"

///////////////////////////////////
// NaSharedDataSignaller methods //
///////////////////////////////////

NaSharedDataSignaller::NaSharedDataSignaller() // no parent, because it has its own QThread
    : thread(new QThread(this))
    , lock(QReadWriteLock::NonRecursive)
{
    thread->start();
    this->moveToThread(thread);
}

