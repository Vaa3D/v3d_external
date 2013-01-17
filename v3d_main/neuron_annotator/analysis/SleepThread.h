#ifndef SLEEPTHREAD_H
#define SLEEPTHREAD_H

#include <QtCore>

class SleepThread : QThread {

public:

    SleepThread();

    void msleep(int milliseconds);

};
#endif // SLEEPTHREAD_H
