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
    // NaSharedDataModel& operator=(const NaSharedDataModel<P>& rhs);
    virtual ~NaSharedDataModel();
    bool readerIsStale(const BaseReader& reader) const;

protected:
    // single data member, d, to follow QSharedData pattern
    // see http://doc.qt.nokia.com/latest/qshareddatapointer.html
    QSharedDataPointer<P> d;


public:
    class BaseReader
    {
    public:
        friend class NaSharedDataModel<P>;
        explicit BaseReader(
                const NaSharedDataModel& naSharedDataModel,
                bool waitForReadLock = false);
        virtual ~BaseReader();

    protected:
        QSharedDataPointer<const P> d; // const P can never cause copy-on-write.
    };
    friend class BaseReader;


protected: // Only the original owner should ever write to the data.
    class BaseWriter : public QWriteLocker
    {
    public:
        friend class NaSharedDataModel<P>;
        explicit BaseWriter(NaSharedDataModel& naSharedDataModel)
            : QWriteLocker(naSharedDataModel.getLock()) {}
        virtual ~BaseWriter();
    };
};

#endif // NASHAREDDATAMODEL_H
