#include "NaSharedDataModel.h"
#include <QDebug>

//////////////////////////
// NaSharedData methods //
//////////////////////////

template<class P>
NaSharedDataModel<P>::NaSharedDataModel() // creates empty volume
{
    d = new P;
}

// shared data always uses shallow copy, until a write operation occurs
template<class P>
/* explicit */
NaSharedDataModel<P>::NaSharedDataModel(const NaSharedDataModel<P>& other) // copy constructor
    : d(other.d) // no slicing danger, because d (and P) aready have the exact type we want
{
    qDebug() << "Info: copying NaSharedDataModel object" << __FILE__ << __LINE__;
}

/* virtual */
template<class P>
NaSharedDataModel<P>::~NaSharedDataModel()
{
    // qDebug() << "Info: deleting NaSharedDataModel object" << __FILE__ << __LINE__;
}

template<class P>
bool NaSharedDataModel<P>::readerIsStale(const BaseReader& reader) const
{
    if (reader.d == NULL)
        return true; // null readers are always stale
    else if (! representsActualData())
        return true; // we are trying to quickly tear down
    else if (reader.d == d)
        return false; // reader has a current copy
    else
        return true; // copy is stale
}


//////////////////////////////////////
// NaSharedData::BaseReader methods //
//////////////////////////////////////

template<class P>
/* explicit */
NaSharedDataModel<P>::BaseReader::BaseReader(
        const NaSharedDataModel& naSharedDataModel,
        bool waitForReadLock /* = false */ )
{
    // Only copy pointer if we can acquire a read lock
    if (waitForReadLock)
    {
        // Only hold the read lock for a moment, just to make sure it is not writing right now.
        naSharedDataModel.getLock()->lockForRead(); // block until read is available
        d = naSharedDataModel.d;
        naSharedDataModel.getLock()->unlock();
    }
    else if ( naSharedDataModel.getLock()->tryLockForRead() )
    {
        // Only hold the read lock for a moment, just to make sure it is not writing right now.
        d = naSharedDataModel.d;
        naSharedDataModel.getLock()->unlock();
    }
    else
        d = NULL;
}

template<class P>
NaSharedDataModel<P>::BaseReader::~BaseReader()
{}


//////////////////////////////////////
// NaSharedData::BaseWriter methods //
//////////////////////////////////////

/* virtual */
template<class P>
NaSharedDataModel<P>::BaseWriter::~BaseWriter() {}

