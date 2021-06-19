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
#include "data_model/Fast3DTexture.h"
#include "data_model/VolumeTexture.h"


class DataFlowModel : public QObject
{

    //Q_OBJECT

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

    double getZRatio() const {
        return zRatio;
    }
    void setZRatio(double ZRatioParam) {
        if (zRatio == ZRatioParam) return;
        zRatio=ZRatioParam;
        emit zRatioChanged(zRatio);
    }

    void setMultiColorImageStackNode(MultiColorImageStackNode* node)
    {
        if (node == multiColorImageStackNode)
            return;
        if (NULL != multiColorImageStackNode)
            delete multiColorImageStackNode;
        multiColorImageStackNode = node;
    }

    void setNeuronAnnotatorResultNode(NeuronAnnotatorResultNode* node)
    {
        if (node == neuronAnnotatorResultNode)
            return;
        if (NULL != neuronAnnotatorResultNode)
            delete neuronAnnotatorResultNode;
        neuronAnnotatorResultNode = node;
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
    ZSliceColors& getZSliceColors() {return zSliceColors;}
    DataColorModel& getFast3DColorModel() {return fast3DColorModel;}
    DataColorModel& getSlow3DColorModel() {return slow3DColorModel;}
    jfrc::VolumeTexture& getVolumeTexture() {return volumeTexture;}
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
    const jfrc::VolumeTexture& getVolumeTexture() const {return volumeTexture;}
    // const VolumeColors& getVolumeColors() const {return volumeColors;}

#ifdef USE_FFMPEG
    Fast3DTexture& getFast3DTexture() {return fast3DTexture;}
    const Fast3DTexture& getFast3DTexture() const {return fast3DTexture;}
#endif

//signals:
    void scrollBarFocus(NeuronSelectionModel::NeuronIndex index);
    void deselectNeuron();
    void volumeDataNeeded(); // stimulate VolumeData to load images from disk
    void benchmarkTimerResetRequested();
    void benchmarkTimerPrintRequested(QString);
    void zRatioChanged(double ratio);

//public slots:
    void cancel();
    void debugColorModel();
    void synchronizeColorModels();

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
    jfrc::VolumeTexture volumeTexture;
#ifdef USE_FFMPEG
    Fast3DTexture fast3DTexture;
#endif

    // TODO - move zRatio into VolumeData
    double zRatio;
};

#endif // DATAFLOWMODEL_H
