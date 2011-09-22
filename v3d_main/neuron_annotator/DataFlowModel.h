#ifndef DATAFLOWMODEL_H
#define DATAFLOWMODEL_H

#include <QtCore>
#include "utility/NeuronMaskEntry.h"
#include "v3d_core.h"
#include "MultiColorImageStackNode.h"
#include "NeuronAnnotatorResultNode.h"
#include "data_model/NeuronFragmentData.h"
#include "data_model/GalleryMipImages.h"
#include "data_model/NeuronSelectionModel.h"
#include "data_model/MipMergedData.h"
#include "data_model/ZSliceColors.h"
#include "data_model/VolumeColors.h"

class DataFlowModel : public QObject
{

    Q_OBJECT

public:
    DataFlowModel(QObject* parentParam = NULL);
    ~DataFlowModel();

    const static int REFERENCE_MIP_INDEX;
    const static int BACKGROUND_MIP_INDEX;

    bool save();
    bool load(long annotationSessionID);
    long getObjectId() const { return objectId; }

    bool loadVolumeData();

    bool loadLsmMetadata();

    double getZRatio() const { return zRatio; }
    void setZRatio(double ZRatioParam) { zRatio=ZRatioParam; }

    void setMultiColorImageStackNode(MultiColorImageStackNode* node) {
        this->multiColorImageStackNode=node;
    }
    void setNeuronAnnotatorResultNode(NeuronAnnotatorResultNode* node) {
        this->neuronAnnotatorResultNode=node;
    }

    MultiColorImageStackNode* getMultiColorImageStackNode() { return multiColorImageStackNode; }

    // Data flow accessors
    NaVolumeData& getVolumeData() {return volumeData;}
    NeuronSelectionModel& getNeuronSelectionModel() {return neuronSelectionModel;}
    DataColorModel& getDataColorModel() {return dataColorModel;}
    MipFragmentData& getMipFragmentData() {return mipFragmentData;}
    MipFragmentColors& getMipFragmentColors() {return mipFragmentColors;}
    GalleryMipImages& getGalleryMipImages() {return galleryMipImages;}
    MipMergedData& getMipMergedData() {return mipMergedData;}
    // VolumeColors& getVolumeColors() {return volumeColors;}

    // Data flow accessors (const versions)
    const NaVolumeData& getVolumeData() const {return volumeData;}
    const NeuronSelectionModel& getNeuronSelectionModel() const {return neuronSelectionModel;}
    const DataColorModel& getDataColorModel() const {return dataColorModel;}
    const NeuronFragmentData& getNeuronFragmentData() const {return neuronFragmentData;}
    const MipFragmentData& getMipFragmentData() const {return mipFragmentData;}
    const MipFragmentColors& getMipFragmentColors() const {return mipFragmentColors;}
    const GalleryMipImages& getGalleryMipImages() const {return galleryMipImages;}
    const MipMergedData& getMipMergedData() const {return mipMergedData;}
    const ZSliceColors& getZSliceColors() const {return zSliceColors;}
    const DataColorModel& getFast3DColorModel() const {return fast3DColorModel;}
    // const VolumeColors& getVolumeColors() const {return volumeColors;}

signals:
    void modelUpdated(QString updateType); // Tell 3D viewer how to update
    void scrollBarFocus(NeuronSelectionModel::NeuronIndex index);
    void deselectNeuron();
    void volumeDataNeeded(); // stimulate VolumeData to load images from disk

public slots:
    void updateNeuronMaskFull();

protected slots:
    // TODO - these slots should be moved to 3D viewer
    void updateNeuronMask(int index, bool status);

private:
    long objectId;
    MultiColorImageStackNode* multiColorImageStackNode;
    NeuronAnnotatorResultNode* neuronAnnotatorResultNode;
    // Data flow objects.
    // Be aware that the order of declaration here is the order of initialization,
    // which in turn affects the order in which signals get serviced.
    NaVolumeData volumeData;
    NeuronSelectionModel neuronSelectionModel;
    DataColorModel dataColorModel;
    NeuronFragmentData neuronFragmentData;
    ZSliceColors zSliceColors;
    MipFragmentData mipFragmentData;
    MipFragmentColors mipFragmentColors;
    GalleryMipImages galleryMipImages;
    MipMergedData mipMergedData;
    DataColorModel slow3DColorModel;
    DataColorModel fast3DColorModel; // for fast but approximate 3D viewer color updates

    // TODO - move zRatio into VolumeData
    double zRatio;
};

#endif // DATAFLOWMODEL_H
