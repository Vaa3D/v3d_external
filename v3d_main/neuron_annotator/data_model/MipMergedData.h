#ifndef MIPMERGEDDATA_H
#define MIPMERGEDDATA_H

#include "NaLockableData.h"
#include "MipFragmentData.h"
#include "DataColorModel.h"
#include "NeuronSelectionModel.h"


class MipMergeLayer; // forward declaration


// MipMergedData combines a subset of neuron fragments and overlays into
// a blended image, for use in the NaLargeMipWidget.
class MipMergedData : public NaLockableData
{
    Q_OBJECT
public:
    friend class MipMergeLayer;

    explicit MipMergedData(
            const NaVolumeData& volumeDataParam,
            const MipFragmentData& mipFragmentData,
            const DataColorModel& dataColorModel,
            const NeuronSelectionModel& neuronSelectionModel);
    virtual ~MipMergedData();

public slots:
    virtual void update(); // full update, on mipFragmentData.dataChanged
    void toggleNeuronVisibility(int index, bool status); // update a single neuron, on neuronSelectionModel.neuronMaskUpdated, O(log nfrags)
    void toggleOverlayVisibility(int index, bool status);
    void updateNeuronVisibility(); // remerge all neurons O(nfrags), on neuronSelectionModel.visibilityChanged? or dirty partial update.
    void colorizeImage(); // on dataColorModel.dataChanged, or mergedImage change

protected:
    bool recomputeLayerTree();
    bool computeMergedImage();

    const NaVolumeData& volumeData;
    const MipFragmentData& mipFragmentData;
    const DataColorModel& dataColorModel;
    const NeuronSelectionModel& neuronSelectionModel;
    // layerZValues represents binary tree of merged fragment mips.
    // Values in layerZValues are the z-coordinate of the mip voxel in the original volumeData.
    // Z-coordinates in layerZValues represent fragment indices for Z-values in the range 0->(nfrags-1).
    // Larger Z-coordinates represent various merged layers in the binary tree.
    My4DImage* layerZValues;
    My4DImage* layerIntensities;
    My4DImage* layerData;
    My4DImage* layerNeurons;
    My4DImage* emptyMy4DImage; // helper for initializing Image4DProxys
    Image4DProxy<My4DImage> layerZProxy;
    Image4DProxy<My4DImage> layerIntensityProxy;
    Image4DProxy<My4DImage> layerDataProxy;
    Image4DProxy<My4DImage> layerNeuronProxy;
    QList<MipMergeLayer*> layers;
    bool bShowReferenceChannel;
    QImage * mergedImage; // colorized output image

public:
    class Reader : public BaseReadLocker
    {
    public:
        explicit Reader(const MipMergedData& mipMergedDataParam)
            : BaseReadLocker(mipMergedDataParam)
            , mipMergedData(mipMergedDataParam)
        {}

        int getMergedImageLayerIndex();
        const Image4DProxy<My4DImage>& getLayerDataProxy() {return mipMergedData.layerDataProxy;}
        const Image4DProxy<My4DImage>& getLayerNeuronProxy() {return mipMergedData.layerNeuronProxy;}
        const Image4DProxy<My4DImage>& getLayerZProxy() {return mipMergedData.layerZProxy;}

        const QImage * getImage() const {return mipMergedData.mergedImage;}

    protected:
        const MipMergedData& mipMergedData;
    };


    class Writer : public BaseWriteLocker
    {
    public:

        explicit Writer(MipMergedData& mipMergedDataParam)
            : BaseWriteLocker(mipMergedDataParam)
            , mipMergedData(mipMergedDataParam)
        {}

        void clearData(); // deletes everything in mipMergedData

    protected:
        MipMergedData& mipMergedData;
    };
};


// MipMergeLayer represents one node in a binary tree of mip images.  The leaves of the tree
// are individual neuron mips, and the root is a combined mip.
class MipMergeLayer : public QObject
{
    Q_OBJECT

public:
    explicit MipMergeLayer(MipMergedData& mipMergedDataParam,
                           int indexParam, bool isVisibleParam,
                           MipMergeLayer * child1Param, MipMergeLayer * child2Param);
    int getIndex() const {return index;} // slice number containing this layer's data
    int getProxyIndex() const {return proxyIndex;}
    bool isVisible() const {return bIsVisible;}
    bool updateWithoutSignal();
    void setVisibility(bool bIsVisibleParam);

signals:
    void dataChanged();

public slots:
    void update(); // emits dataChanged() if data changes.

protected:
    bool copyChildState(MipMergeLayer * child);

    MipMergedData& mipMergedData;
    bool bIsVisible;
    int index; // into z-slice of MipMergedData::layerZValues
    int proxyIndex; // this layer may be represented by a layer closer to the leaves of the tree
    MipMergeLayer * child1;
    MipMergeLayer * child2;
};


#endif // MIPMERGEDDATA_H
