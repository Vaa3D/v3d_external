#ifndef VOLUMECOLORS_H
#define VOLUMECOLORS_H

#include "NeuronSelectionModel.h"
#include "DataColorModel.h"

class VolumeColors : public NaLockableData
{
    Q_OBJECT
public:
    explicit VolumeColors(
            const NaVolumeData& volumeData,
            const DataColorModel& dataColorModel,
            const NeuronSelectionModel& neuronSelectionModel);
    virtual ~VolumeColors();

signals:

public slots:

protected:
    const NaVolumeData& volumeData;
    const DataColorModel& dataColorModel;
    const NeuronSelectionModel& neuronSelectionModel;
};

#endif // VOLUMECOLORS_H
