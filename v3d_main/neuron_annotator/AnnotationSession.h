#ifndef ANNOTATIONSESSION_H
#define ANNOTATIONSESSION_H

#include <QtCore>
#include "NeuronMaskEntry.h"
#include "v3d_core.h"
#include "MultiColorImageStackNode.h"
#include "NeuronAnnotatorResultNode.h"

class AnnotationSession : public QObject
{

    Q_OBJECT

public:
    AnnotationSession();
    ~AnnotationSession();
    bool save();
    bool load(long annotationSessionID);
    long getObjectId() { return objectId; }

    bool loadOriginalImageStack();
    bool loadNeuronMaskStack();
    bool loadMaskLabelIndexFile();
    bool populateMaskMipList();
    QList<QPixmap> * getMaskMipList() { return &maskMipList; }

    void setMultiColorImageStackNode(MultiColorImageStackNode* multiColorImageStackNode) {
        this->multiColorImageStackNode=multiColorImageStackNode;
    }
    void setNeuronAnnotatorResultNode(NeuronAnnotatorResultNode* neuronAnnotatorResultNode) {
        this->neuronAnnotatorResultNode=neuronAnnotatorResultNode;
    }

    // getOriginalImageStackAsMy4DImage() returns a pointer to a My4DImage object
    // maintained by the AnnotationSession class.  Do not delete this pointer,
    // and be aware that it has a lifetime no longer than that of the AnnnotationSession object.
    My4DImage* getOriginalImageStackAsMy4DImage() { return originalImageStack; }

    My4DImage* getNeuronMaskAsMy4DImage() { return neuronMaskStack; }

    bool neuronMaskIsChecked(int index) { return maskStatusList.at(index); }
    void setNeuronMaskStatus(int index, bool status);
	
	QList<bool> getMaskStatusList(){return maskStatusList;}
	QList<bool> getNeuronSelectList(){return neuronSelectList;}

        void switchSelectedNeuron(int index);

signals:
    void modelUpdated(QString updateType);
    void neuronMaskStatusSet();
    void scrollBarFocus(int index);

public slots:
    void neuronMaskUpdate(int index, bool status);
    void showSelectedNeuron(bool background);
    void showAllNeurons(bool background);

private:
    long objectId;
    MultiColorImageStackNode* multiColorImageStackNode;
    NeuronAnnotatorResultNode* neuronAnnotatorResultNode;
    My4DImage* originalImageStack;
    My4DImage* neuronMaskStack;
    QList<NeuronMaskEntry> maskEntryList;
    QList<QPixmap> maskMipList;
    QList<bool> maskStatusList;
    QList<bool> neuronSelectList;

};

#endif // ANNOTATIONSESSION_H
