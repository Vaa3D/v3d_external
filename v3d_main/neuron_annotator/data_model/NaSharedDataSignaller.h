#ifndef NASHAREDDATASIGNALLER_H
#define NASHAREDDATASIGNALLER_H

#include <QObject>
#include <QReadWriteLock>
#include <QThread>
#include "SlotMerger.h"

// template classes like NaSharedDataModel cannot have Q_OBJECT, so create
// base class NaSharedDataSignaller .
class NaSharedDataSignaller : public QObject
{
    Q_OBJECT

public:
    NaSharedDataSignaller(); // no parent, because it has its own QThread
    virtual ~NaSharedDataSignaller();
    bool representsActualData() const {return ! bAbortWrite;}

signals:
    void dataChanged(); // ready for downstream clients to read all data
    void progressMessageChanged(QString msg);
    void progressValueChanged(int); // on a scale of 0-100
    void progressAborted(QString msg); // data update was stopped for some reason
    void progressComplete();
    void invalidated();
    void actualDataRepresented();

public slots:
    virtual void update() {} // recreate everything from upstream data
    virtual void invalidate() {
        if (bAbortWrite) return;
        bAbortWrite = true;
        emit invalidated();
    }

protected:
    void setRepresentsActualData() {
        if (! bAbortWrite) return; // no change
        bAbortWrite = false;
        emit actualDataRepresented();
    }

    QReadWriteLock* getLock() const {return const_cast<QReadWriteLock*>(&lock);}

    // Optionally slow down those too-fast update() slot calls
    // like this:
    //     void MyNaSharedDataSignaller::update()
    //     {
    //         <store any local state of this call, such as if update() was passed arguments>
    //         SlotMerger updateMerger(statusOfUpdateSlot);
    //         if (! updateMerger.shouldRun()) return;
    //         <proceed with update>
    //         ...
    SlotStatus statusOfUpdateSlot;

    QReadWriteLock lock; // used for multiple-read/single-write thread-safe locking
    QThread * thread;

private:
    volatile bool bAbortWrite; // flag hint to stop writing, even if data is in an inconsistent state, because destructor is waiting.
};


#endif // NASHAREDDATASIGNALLER_H
