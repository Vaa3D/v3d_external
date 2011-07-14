#ifndef ANNOTATIONSESSION_H
#define ANNOTATIONSESSION_H

#include <QtCore>
#include "utility/NeuronMaskEntry.h"
#include "v3d_core.h"
#include "MultiColorImageStackNode.h"
#include "NeuronAnnotatorResultNode.h"
#include "FragmentSelectionModel.h"

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
    long getObjectId() { return objectId; }

    bool loadOriginalImageStack();
    bool loadNeuronMaskStack();
    bool loadReferenceStack();
    bool prepareLabelIndex();
    bool populateMipLists();
    QList<QImage>* getNeuronMipList() { return &neuronMipList; }
    QList<QImage>* getOverlayMipList() { return &overlayMipList; }

    void setMultiColorImageStackNode(MultiColorImageStackNode* node) {
        this->multiColorImageStackNode=node;
    }
    void setNeuronAnnotatorResultNode(NeuronAnnotatorResultNode* node) {
        this->neuronAnnotatorResultNode=node;
    }

    // getOriginalImageStackAsMy4DImage() returns a pointer to a My4DImage object
    // maintained by the AnnotationSession class.  Do not delete this pointer,
    // and be aware that it has a lifetime no longer than that of the AnnnotationSession object.
    My4DImage* getOriginalImageStackAsMy4DImage() { return originalImageStack; }
    My4DImage* getReferenceStack() { return referenceStack; }

    My4DImage* getNeuronMaskAsMy4DImage() { return neuronMaskStack; }

    bool neuronMaskIsChecked(int index) { return maskStatusList.at(index); }
    bool overlayIsChecked(int index) { return overlayStatusList.at(index); }
	
    QList<bool> getMaskStatusList(){return maskStatusList;}
    void setNeuronMaskStatus(int index, bool status);
    QList<bool> getOverlayStatusList(){return overlayStatusList;}
    void setOverlayStatus(int index, bool status);
    QList<bool> getNeuronSelectList(){return neuronSelectList;}

    void switchSelectedNeuron(int index);

signals:
    void modelUpdated(QString updateType);
    void neuronMaskStatusSet();
    void scrollBarFocus(FragmentSelectionModel::FragmentIndex index);

public slots:
    void neuronMaskUpdate(int index, bool status);
    void neuronMaskFullUpdate();
    void overlayUpdate(int index, bool status);
    void showSelectedNeuron(QList<int> overlayList);
    void showAllNeurons(QList<int> overlayList);

private:
    long objectId;
    MultiColorImageStackNode* multiColorImageStackNode;
    NeuronAnnotatorResultNode* neuronAnnotatorResultNode;
    My4DImage* originalImageStack;
    My4DImage* neuronMaskStack;
    My4DImage* referenceStack;
    QList<NeuronMaskEntry> maskEntryList;
    QList<QImage> neuronMipList;
    QList<QImage> overlayMipList;
    QList<bool> maskStatusList;
    QList<bool> overlayStatusList;
    QList<bool> neuronSelectList;

};

#endif // ANNOTATIONSESSION_H
