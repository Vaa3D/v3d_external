#ifndef ANNOTATIONSESSION_H
#define ANNOTATIONSESSION_H

#include <QtCore>
#include "utility/NeuronMaskEntry.h"
#include "v3d_core.h"
#include "MultiColorImageStackNode.h"
#include "NeuronAnnotatorResultNode.h"
#include "FragmentSelectionModel.h"
#include "data_model/GalleryMipImages.h"

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

    bool prepareLabelIndex();
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

    bool neuronMaskIsChecked(int index) const { return maskStatusList.at(index); }
    bool overlayIsChecked(int index) const { return overlayStatusList.at(index); }
	
    QList<bool> getMaskStatusList() const {return maskStatusList;}
    void setNeuronMaskStatus(int index, bool status);
    QList<bool> getOverlayStatusList() const {return overlayStatusList;}
    void setOverlayStatus(int index, bool status);
    QList<bool> getNeuronSelectList() const {return neuronSelectList;}

    void switchSelectedNeuron(int index);
    void switchSelectedNeuronUniquelyIfOn(int index);
    void clearSelections();

    // Data flow accessors
    NaVolumeData& getVolumeData() {return volumeData;}
    GalleryMipImages& getGalleryMipImages() {return galleryMipImages;}
    DataColorModel& getDataColorModel() {return dataColorModel;}

    // Data flow accessors (const versions)
    const NaVolumeData& getVolumeData() const {return volumeData;}
    const GalleryMipImages& getGalleryMipImages() const {return galleryMipImages;}
    const DataColorModel& getDataColorModel() const {return dataColorModel;}

signals:
    void modelUpdated(QString updateType);
    void scrollBarFocus(FragmentSelectionModel::FragmentIndex index);
    void deselectNeuron();
    void volumeDataNeeded(); // stimulate VolumeData to load images from disk

public slots:
    void neuronMaskUpdate(int index, bool status);
    void neuronMaskFullUpdate();
    void overlayUpdate(int index, bool status);
    void showSelectedNeuron(QList<int> overlayList);
    void showAllNeurons(QList<int> overlayList);
    void clearAllNeurons();
    void updateNeuronSelectList(int index);

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
    //
    QList<bool> maskStatusList;
    QList<bool> overlayStatusList;
    QList<bool> neuronSelectList;
    double zRatio;
};

#endif // ANNOTATIONSESSION_H
