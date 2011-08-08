#ifndef ANNOTATIONSESSION_H
#define ANNOTATIONSESSION_H

#include <QtCore>
#include "utility/NeuronMaskEntry.h"
#include "v3d_core.h"
#include "MultiColorImageStackNode.h"
#include "NeuronAnnotatorResultNode.h"
#include "data_model/GalleryMipImages.h"
#include "data_model/NeuronSelectionModel.h"

class AnnotationSession : public QObject
{

    Q_OBJECT

public:
    AnnotationSession(QObject* parentParam = NULL);
    ~AnnotationSession();

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

    // getOriginalImageStackAsMy4DImage() returns a pointer to a My4DImage object
    // maintained by the AnnotationSession class.  Do not delete this pointer,
    // and be aware that it has a lifetime no longer than that of the AnnnotationSession object.
    // TODO - deprecate these accessors in favor of multithreaded data flow objects.
    My4DImage* getOriginalImageStackAsMy4DImage() { return volumeData.getOriginalImageStackAsMy4DImage(); }
    My4DImage* getReferenceStack() { return volumeData.getReferenceStack(); }
    My4DImage* getNeuronMaskAsMy4DImage() { return volumeData.getNeuronMaskAsMy4DImage(); }
    const My4DImage* getOriginalImageStackAsMy4DImage() const { return volumeData.getOriginalImageStackAsMy4DImage(); }
    const My4DImage* getReferenceStack() const { return volumeData.getReferenceStack(); }
    const My4DImage* getNeuronMaskAsMy4DImage() const { return volumeData.getNeuronMaskAsMy4DImage(); }

    // Data flow accessors
    NaVolumeData& getVolumeData() {return volumeData;}
    MipFragmentData& getMipFragmentData() {return mipFragmentData;}
    MipFragmentColors& getMipFragmentColors() {return mipFragmentColors;}
    GalleryMipImages& getGalleryMipImages() {return galleryMipImages;}
    DataColorModel& getDataColorModel() {return dataColorModel;}
    NeuronSelectionModel& getNeuronSelectionModel() {return neuronSelectionModel;}

    // Data flow accessors (const versions)
    const NaVolumeData& getVolumeData() const {return volumeData;}
    const MipFragmentData& getMipFragmentData() const {return mipFragmentData;}
    const MipFragmentColors& getMipFragmentColors() const {return mipFragmentColors;}
    const GalleryMipImages& getGalleryMipImages() const {return galleryMipImages;}
    const DataColorModel& getDataColorModel() const {return dataColorModel;}
    const NeuronSelectionModel& getNeuronSelectionModel() const {return neuronSelectionModel;}

signals:
    void modelUpdated(QString updateType); // Tell 3D viewer how to update
    void scrollBarFocus(NeuronSelectionModel::NeuronIndex index);
    void deselectNeuron();
    void volumeDataNeeded(); // stimulate VolumeData to load images from disk

public slots:
    void updateNeuronMaskFull();

protected slots:
    // TODO - these slots should be moved to 3D viewer
    void updateOverlay(int index, bool status);
    void updateNeuronMask(int index, bool status);
    void showSelectedNeuron(int selectionIndex);

private:
    long objectId;
    MultiColorImageStackNode* multiColorImageStackNode;
    NeuronAnnotatorResultNode* neuronAnnotatorResultNode;
    // Data flow objects
    NaVolumeData volumeData;
    MipFragmentData mipFragmentData;
    DataColorModel dataColorModel;
    MipFragmentColors mipFragmentColors;
    GalleryMipImages galleryMipImages;
    NeuronSelectionModel neuronSelectionModel;

    // TODO - move zRatio into VolumeData
    double zRatio;
};

#endif // ANNOTATIONSESSION_H
