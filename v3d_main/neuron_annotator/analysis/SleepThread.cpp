#include "SleepThread.h"

SleepThread::SleepThread() {
}

void SleepThread::msleep(int milliseconds) {
    QThread::msleep(milliseconds);
}
