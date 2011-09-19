#ifndef NEURONSELECTOR_H
#define NEURONSELECTOR_H

#include <QtCore>
#include "AnnotationSession.h"

// cube of NB by NB by NB 
#define NB 3
// Sampling Lattice
#define STEP 5

class NeuronSelector : public QObject
{
    Q_OBJECT

public:
    NeuronSelector(QObject * parent);
    ~NeuronSelector(){}

public:
    int getIndexSelectedNeuron(); // returns index of neuron at selected position.  sets "index" member.
    void setAnnotationSession(AnnotationSession* annotationSession); // wires NeuronSelector to data flow model
    void init();

    void getCurIndexNeuronBoundary(); // populates curNeuronBDxx members
    bool inNeuronMask(V3DLONG x, V3DLONG y, V3DLONG z);
    QList<ImageMarker> highlightIndexNeuron(); // returns a list of points for the index neuron

public slots:
    void onSelectionModelChanged(); // updates highlight based on state of NeuronSelectionModel
    void updateSelectedPosition(double x, double y, double z);

signals:
    void landmarksClearNeeded();
    void landmarksUpdateNeeded(const QList<ImageMarker>);
    void selectionClearNeeded();
    void neuronSelected(int index);

private:
    int index;
    V3DLONG xlc, ylc, zlc; // current mouse left click location in 3D

    AnnotationSession* annotationSession;

    V3DLONG sx, sy, sz;
    V3DLONG curNeuronBDxb, curNeuronBDxe, curNeuronBDyb, curNeuronBDye, curNeuronBDzb, curNeuronBDze;

};

#endif // NEURONSELECTOR_H
