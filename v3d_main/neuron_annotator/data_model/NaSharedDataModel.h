#ifndef NASHAREDDATAMODEL_H
#define NASHAREDDATAMODEL_H

#include <QSharedData>
// template classes like NaSharedDataModel cannot have Q_OBJECT, so create
// base class NaSharedDataSignaller .
#include "NaSharedDataSignaller.h"

// Sketch of how to use Qt copy-on-write methods to implement unified data model data flow objects.
// Uses both QReadWriteLock and QSharedDataPointer to manage access to shared data.
// QSharedDataPointer part is based on Employee example at
// http://doc.qt.nokia.com/latest/qshareddatapointer.html

// NaSharedDataModel corresponds to NaLockableData in the previous QReadWriteLock unified data model paradigm.
template<class P> // P is a private class derived from QSharedData
class NaSharedDataModel : public NaSharedDataSignaller
{
public:
    class BaseReader;

    NaSharedDataModel(); // creates empty volume
    explicit NaSharedDataModel(const NaSharedDataModel<P>& other); // copy constructor
    bool readerIsStale(const BaseReader& reader);

protected:
    // single data member, d, to follow QSharedData pattern
    // see http://doc.qt.nokia.com/latest/qshareddatapointer.html
    QSharedDataPointer<P> d;


public:
    class BaseReader : public QSharedDataPointer<const P> // const P can never cause copy-on-write.
    {
    public:
        explicit BaseReader(
                const NaSharedDataModel& naSharedDataModel,
                bool waitForReadLock = false);
    };
    friend class BaseReader;


    class BaseWriter : public QWriteLocker
    {
    public:
        explicit BaseWriter(NaSharedDataModel& naSharedDataModel)
            : QWriteLocker(naSharedDataModel.lock) {}
    };
};

#endif // NASHAREDDATAMODEL_H
