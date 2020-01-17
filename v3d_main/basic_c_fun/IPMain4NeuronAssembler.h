#ifndef IPMAIN4NEURONASSEMBLER_H
#define IPMAIN4NEURONASSEMBLER_H

#include <QtCore>

#include "v3d_core.h"

class IPMain4NeuronAssembler
{
public:
	virtual ~IPMain4NeuronAssembler() {}

	virtual void getNAVersionNum() = 0;

	virtual bool markerMonitorStatus() = 0;
	virtual void sendSelectedMarkers2NA(const QList<ImageMarker>& selectedMarkerList, const QList<ImageMarker>& selectedLocalMarkerList) = 0;

	virtual vector<V_NeuronSWC> eraserSegProcess(const vector<V_NeuronSWC>& currSegDisplay, const float nodeCoords[]) = 0;
};

Q_DECLARE_INTERFACE(IPMain4NeuronAssembler, "MK.PMainInterface_for_NeuronAssemblerUI/1.0");

#endif