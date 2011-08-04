#include "MipFragmentColors.h"

/* explicit */
MipFragmentColors::MipFragmentColors(const MipFragmentData& mipFragmentDataParam,
                  const DataColorModel& colorModelParam,
                  QObject *parentParam /* = NULL */ )
    : NaLockableData(parentParam)
    , mipFragmentData(mipFragmentDataParam)
    , dataColorModel(colorModelParam)
{
    // TODO - connect signals, slots
}
