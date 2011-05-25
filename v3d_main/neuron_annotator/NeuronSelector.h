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
	NeuronSelector(){}
	~NeuronSelector(){}
	
public:

	int getIndexSetectedNeuron(); // find nearest neuron
	
	void setAnnotationSession(AnnotationSession* annotationSession);
	
	void init();
	
	void getCurNeuronBoundary();
	
	bool inNeuronMask(V3DLONG x, V3DLONG y, V3DLONG z);
	
	void highlightSelectedNeuron();
	
public slots:
	void updateSelectedPosition(double x, double y, double z);
	
signals:
	void neuronHighlighted(bool b);
	
private:
	int index;
	
	V3DLONG xlc, ylc, zlc; // current mouse left click location in 3D
	
	AnnotationSession* annotationSession;
	
	V3DLONG sx, sy, sz;
	
	V3DLONG curNeuronBDxb, curNeuronBDxe, curNeuronBDyb, curNeuronBDye, curNeuronBDzb, curNeuronBDze;

};

#endif // NEURONSELECTOR_H
