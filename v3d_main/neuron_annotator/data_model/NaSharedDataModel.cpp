#include "NaSharedDataModel.h"


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
{}

template<class P>
bool NaSharedDataModel<P>::readerIsStale(const BaseReader& reader)
{
    if (reader == NULL)
        return true; // null readers are always stale
    else if (reader == d)
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
        naSharedDataModel.lock.lockForRead(); // block until read is available
        *this = naSharedDataModel.d;
        naSharedDataModel.lock.unlock();
    }
    else if ( naSharedDataModel.lock.tryLockForRead() )
    {
        // Only hold the read lock for a moment, just to make sure it is not writing right now.
        *this = naSharedDataModel.d;
        naSharedDataModel.lock.unlock();
    }
    else
        *this = NULL;
}

