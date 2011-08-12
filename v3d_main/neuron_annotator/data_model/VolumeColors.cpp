#include "VolumeColors.h"

/* explicit */
VolumeColors::VolumeColors(
                const NaVolumeData& volumeDataParam,
                const DataColorModel& dataColorModelParam,
                const NeuronSelectionModel& neuronSelectionModelParam)
    : volumeData(volumeDataParam)
    , dataColorModel(dataColorModelParam)
    , neuronSelectionModel(neuronSelectionModelParam)
{
}

/* virtual */
VolumeColors::~VolumeColors() {}


