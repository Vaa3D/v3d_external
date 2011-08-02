#ifndef ANNOTATIONSESSION_H
#define ANNOTATIONSESSION_H

#include <QtCore>
#include "utility/NeuronMaskEntry.h"
#include "v3d_core.h"
#include "MultiColorImageStackNode.h"
#include "NeuronAnnotatorResultNode.h"
#include "FragmentSelectionModel.h"
#include "data_model/NaVolumeData.h"

class AnnotationSession : public QObject
{

    Q_OBJECT

public:
    AnnotationSession();
    ~AnnotationSession();

    const static int REFERENCE_MIP_INDEX;
    const static int BACKGROUND_MIP_INDEX;

    bool save();
    bool load(long annotationSessionID);
    long getObjectId() const { return objectId; }

    // bool loadOriginalImageStack();
    // bool loadNeuronMaskStack();
    // bool loadReferenceStack();
    bool loadVolumeData();

    bool prepareLabelIndex();
    bool populateMipLists();
    bool loadLsmMetadata();

    double getZRatio() const { return zRatio; }
    void setZRatio(double ZRatioParam) { zRatio=ZRatioParam; }

    QList<QImage*>* getNeuronMipList() { return &neuronMipList; }
    QList<QImage*>* getOverlayMipList() { return &overlayMipList; }

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
    NaVolumeData * getVolumeData() {return &volumeData;}

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
    NaVolumeData volumeData;
    QThread volumeDataThread;
    QList<QImage*> neuronMipList;
    QList<QImage*> overlayMipList;
    QList<bool> maskStatusList;
    QList<bool> overlayStatusList;
    QList<bool> neuronSelectList;
    double zRatio;

};

#endif // ANNOTATIONSESSION_H
