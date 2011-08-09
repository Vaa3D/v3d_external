#ifndef MIPMERGEDDATA_H
#define MIPMERGEDDATA_H

#include "NaLockableData.h"
#include "MipFragmentData.h"
#include "DataColorModel.h"
#include "NeuronSelectionModel.h"

class MipMergeLayer : public QObject
{
    Q_OBJECT

public:

signals:
    void dataChanged();

public slots:
    void setVisiblity(bool bIsVisibleParam);
    void update(); // emits dataChanged() if data changes.
    void updateWithoutSignal(); // for manual full updates

protected:
    bool bIsVisible();
    int index; // into z-slice of MipMergedData::layerZValues
};

// MipMergedData combines a subset of neuron fragments and overlays into
// a blended image, for use in the NaLargeMipWidget.
class MipMergedData : public NaLockableData
{
    Q_OBJECT
public:
    explicit MipMergedData(
            const MipFragmentData& mipFragmentData,
            const DataColorModel& dataColorModel,
            const NeuronSelectionModel& neuronSelectionModel);
    virtual ~MipMergedData();

public slots:
    virtual void update(); // full update, on mipFragmentData.dataChanged
    void toggleNeuronVisibility(int index, bool status); // update a single neuron, on neuronSelectionModel.neuronMaskUpdated, O(log nfrags)
    void toggleOverlayVisibility(int index, bool status);
    void updateNeuronVisibility(); // remerge all neurons O(nfrags), on neuronSelectionModel.visibilityChanged? or dirty partial update.
    void updateColors(); // on dataColorModel.dataChanged, or mergedImage change

protected:
    const MipFragmentData& mipFragmentData;
    const DataColorModel& dataColorModel;
    const NeuronSelectionModel& neuronSelectionModel;
    // layerZValues represents binary tree of merged fragment mips.
    // Values in layerZValues are the z-coordinate of the mip voxel in the original volumeData.
    // Z-coordinates in layerZValues represent fragment indices for Z-values in the range 0->(nfrags-1).
    // Larger Z-coordinates represent various merged layers in the binary tree.
    My4DImage* layerZValues;
    int mergedIndex; // Z-coordinate index of final merged image
    QImage * mergedImage; // colorized output image
    QList<MipMergeLayer*> layers;

public:
    class Reader : public BaseReadLocker
    {
    public:
        explicit Reader(const MipMergedData& mipMergedDataParam)
            : BaseReadLocker(mipMergedDataParam)
            , mipMergedData(mipMergedDataParam)
        {}

        const QImage * getImage() const {return mipMergedData.mergedImage;}

    protected:
        const MipMergedData& mipMergedData;
    };


    class Writer : public BaseWriteLocker
    {
        explicit Writer(MipMergedData& mipMergedDataParam)
            : BaseWriteLocker(mipMergedDataParam)
        {}
    };
};

#endif // MIPMERGEDDATA_H
