#ifndef VOLUMECOLORS_H
#define VOLUMECOLORS_H

#include "NeuronSelectionModel.h"
#include "DataColorModel.h"

// VolumeColors creates volume data for the 3D viewer.
// RGBA, with the reference channel in the A/alpha channel.
// Colorized data channels in RGB.
class VolumeColors : public NaLockableData
{
    Q_OBJECT

public:
    explicit VolumeColors(
            const NaVolumeData& volumeData,
            const DataColorModel& dataColorModel,
            const NeuronSelectionModel& neuronSelectionModel);
    virtual ~VolumeColors();

public slots:
    virtual void update();

protected:
    // input
    const NaVolumeData& volumeData;
    const DataColorModel& dataColorModel;
    const NeuronSelectionModel& neuronSelectionModel;
    // output
    My4DImage* volumeColorsImage;
    My4DImage* emptyImage;
    Image4DProxy<My4DImage> volumeColorsProxy;

public:
    class Reader : public BaseReadLocker
    {
    public:
        Reader(const VolumeColors& volumeColorsParam)
            : BaseReadLocker(volumeColorsParam)
            , volumeColors(volumeColorsParam)
        {}

    protected:
        const VolumeColors& volumeColors;
    };


    class Writer : public BaseWriteLocker
    {
    public:
        Writer(VolumeColors& volumeColorsParam)
            : BaseWriteLocker(volumeColorsParam)
            , volumeColors(volumeColorsParam)
        {}
        void clearData();

    protected:
        VolumeColors& volumeColors;
    };
};

#endif // VOLUMECOLORS_H
