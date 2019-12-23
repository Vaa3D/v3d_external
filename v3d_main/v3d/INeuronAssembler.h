#ifndef INEURONASSEMBLER_H
#define INEURONASSEMBLER_H

#include <QtCore>

class INeuronAssembler
{
public:
	virtual ~INeuronAssembler() {}

	virtual string getCviewerWinTitle() = 0;


};


Q_DECLARE_INTERFACE(INeuronAssembler, "MK.teraflyInterfaceTest1/1.0");


#endif