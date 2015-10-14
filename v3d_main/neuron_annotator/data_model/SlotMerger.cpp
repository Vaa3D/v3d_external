#include "SlotMerger.h"
#include <QApplication>


////////////////////////
// SlotStatus methods //
////////////////////////

SlotStatus::SlotStatus()
    : isRunning(false)
    , skippedCallCount(0)
{}


////////////////////////
// SlotMerger methods //
////////////////////////

/* explicit */
SlotMerger::SlotMerger(SlotStatus& statusParam)
    : status(statusParam)
{
    bShouldRun = (! status.isRunning);
    if (bShouldRun)
    {
        // forbid others to run while we run this slot
        status.isRunning = true;
        status.skippedCallCount = 0;
        // clear the slot queue of other before proceeding
        QApplication::processEvents();
    }
    else {
        ++status.skippedCallCount;
    }
}

/* virtual */
SlotMerger::~SlotMerger()
{
    if (bShouldRun) {
        // allow future slots to run again
        status.isRunning = false;
    }
}

// Only run if this is the merger that took the lock.
bool SlotMerger::shouldRun() const {return bShouldRun;}

int SlotMerger::skippedCallCount() const {return status.skippedCallCount;}

