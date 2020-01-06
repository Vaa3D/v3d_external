#ifndef IPMAIN4NEURONASSEMBLER_H
#define IPMAIN4NEURONASSEMBLER_H

#include <QtCore>

class IPMain4NeuronAssembler
{
public:
	virtual void getNAVersionNum() = 0;

	virtual void switchMarkerMonitor_fromPMain(bool on_off) = 0;

};

Q_DECLARE_INTERFACE(IPMain4NeuronAssembler, "MK.PMainInterface_for_NeuronAssemblerUI/1.0");

#endif