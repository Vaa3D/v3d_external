#include "MipFragmentData.h"

/* explicit */
MipFragmentData::MipFragmentData(const NaVolumeData& volumeDataParam, QObject *parent /* = 0 */)
    : NaLockableData(parent)
    , volumeData(volumeDataParam)
{
}

/* virtual */
MipFragmentData::~MipFragmentData()
{ // TODO
}

void MipFragmentData::updateFromVolumeData()
{ // TODO
}

