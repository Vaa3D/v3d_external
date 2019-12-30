#ifndef INEURONASSEMBLER_H
#define INEURONASSEMBLER_H

#include <QtCore>

class INeuronAssembler
{
public:
	virtual ~INeuronAssembler() {}

	virtual bool checkFragTraceStatus() = 0;
	virtual string getCviewerWinTitle() = 0;
	virtual int getTeraflyResLevel() = 0;

	virtual void getSelectedMarkerList(QList<ImageMarker>& selectedMarkerList, QList<ImageMarker>& selectedLocalMarkerList) = 0;
	virtual void refreshSelectedMarkers() = 0;

	virtual void segEditing_setCursor(string mode) = 0;

	virtual void getParamsFromFragTraceUI(const string& keyName, const float& value) = 0;
};

Q_DECLARE_INTERFACE(INeuronAssembler, "MK.teraflyInterfaceTest1/1.0");


#endif