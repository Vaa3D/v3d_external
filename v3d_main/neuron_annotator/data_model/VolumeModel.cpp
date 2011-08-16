#include "VolumeModel.h"
#include "PrivateVolumeModelData.h"
#include "NaSharedDataModel.cpp"

VolumeModel::VolumeModel()
    : super()
{}

/* explicit */
VolumeModel::VolumeModel(const QDir& dataDirectory) // populates from multichannel node directory
    : super()
{
    // TODO construct using d-> operator
}

// copy is always shallow for shared data
/* explicit */
VolumeModel::VolumeModel(const VolumeModel& other) // copy constructor
    : super(other)
{}

