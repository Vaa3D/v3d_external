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
    connect(&volumeData, SIGNAL(dataChanged()),
            this, SLOT(update()));
    connect(&dataColorModel, SIGNAL(dataChanged()),
            this, SLOT(update()));
    connect(&neuronSelectionModel, SIGNAL(dataChanged()),
            this, SLOT(update()));
}

/* virtual */
VolumeColors::~VolumeColors() {}

/* virtual */
void VolumeColors::update()
{
    qDebug() << "VolumeColors::update()";
    // TODO
}

