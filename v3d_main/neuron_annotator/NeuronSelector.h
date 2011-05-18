#ifndef NEURONSELECTOR_H
#define NEURONSELECTOR_H

#include <QtCore>
#include "AnnotationSession.h"

// cube of NB by NB by NB 
#define NB 3

class NeuronSelector : public QObject
{

    Q_OBJECT

public:
	NeuronSelector(){}
	~NeuronSelector(){}
	
public:
	void switchSelectedNeuron(int index);
	int getIndexSetectedNeuron(); // find nearest neuron
	
	void setAnnotationSession(AnnotationSession* annotationSession);
	
	void init();
	
	void highlightSelectedNeuron();
	
public slots:
	void updateSelectedPosition(double x, double y, double z);
	
private:
	int index;
	
	V3DLONG xlc, ylc, zlc; // current mouse left click location in 3D
	
	AnnotationSession* annotationSession;

};

#endif // NEURONSELECTOR_H
