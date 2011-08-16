#ifndef VOLUMEMODEL_H
#define VOLUMEMODEL_H

#include "NaSharedDataModel.h"
#include <QDir>
#include "../v3d/v3d_core.h"

// Sketch of how to use Qt copy-on-write methods to implement unified data model data flow objects.

class PrivateVolumeModelData; // forward declaration of non-API class

// VolumeModel corresponds to NaVolumeData in the previous QReadWriteLock unified data model paradigm.
class VolumeModel : public NaSharedDataModel<PrivateVolumeModelData>
{
public:
    VolumeModel(); // creates empty volume
    explicit VolumeModel(const QDir& dataDirectory); // populates from multichannel node directory
    explicit VolumeModel(const VolumeModel& other); // copy constructor

public:
    class Reader : public BaseReader // read-only accessor will never copy-on-write
    {
    public:
        Reader(const VolumeModel& volumeModel);
        const Image4DProxy<My4DImage>& getNeuronMaskProxy() const;
        const Image4DProxy<My4DImage>& getOriginalImageProxy() const;
        const Image4DProxy<My4DImage>& getReferenceImageProxy() const;
        ImagePixelType getOriginalDatatype() const;
        int getNumberOfNeurons() const;
    };

private:
    typedef NaSharedDataModel<PrivateVolumeModelData> super; // semantic sugar
};

#endif // VOLUMEMODEL_H
