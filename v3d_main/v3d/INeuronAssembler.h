#ifndef INEURONASSEMBLER_H
#define INEURONASSEMBLER_H

#include <QtCore>

#include "IPMain4NeuronAssembler.h"

class INeuronAssembler
{
public:
	virtual ~INeuronAssembler() {}

	virtual bool teraflyImgInstance() = 0;

	virtual void sendCastNAUI2PMain(IPMain4NeuronAssembler* NAportal) = 0;
	virtual bool checkFragTraceStatus() = 0;
	virtual void changeFragTraceStatus(bool newStatus) = 0;

	virtual string getCviewerWinTitle() = 0;
	virtual int getTeraflyTotalResLevel() = 0;
	virtual int getTeraflyResLevel() = 0;
	virtual bool getXlockStatus() = 0;
	virtual bool getYlockStatus() = 0;
	virtual bool getZlockStatus() = 0;
	virtual bool getPartialVolumeCoords(int globalCoords[], int localCoords[], int displayingVolDims[]) = 0;

	virtual void getSelectedMarkerList(QList<ImageMarker>& selectedMarkerList, QList<ImageMarker>& selectedLocalMarkerList) = 0;
	virtual void refreshSelectedMarkers() = 0;

	virtual void segEditing_setCursor(string mode) = 0;

	virtual void getParamsFromFragTraceUI(const string& keyName, const float& value) = 0;
};

Q_DECLARE_INTERFACE(INeuronAssembler, "MK.NeuronAssemblerInterface_for_CViewerClass/1.0");


#endif