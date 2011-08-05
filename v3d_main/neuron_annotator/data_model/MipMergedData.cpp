#include "MipMergedData.h"

/* explicit */
MipMergedData::MipMergedData(const MipFragmentData& mipFragmentDataParam,
                             QObject *parentParam /* = NULL */)
    : NaLockableData(parentParam)
    , mipFragmentData(mipFragmentDataParam)
{
    connect(&mipFragmentData, SIGNAL(dataChanged()),
            this, SLOT(update()));
}

void MipMergedData::update()
{
    // TODO
}

