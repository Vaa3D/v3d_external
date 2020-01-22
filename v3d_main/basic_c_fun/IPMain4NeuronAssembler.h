#ifndef IPMAIN4NEURONASSEMBLER_H
#define IPMAIN4NEURONASSEMBLER_H

#include <set>

#include <QtCore>

#include "v3d_core.h"

using namespace std;

class IPMain4NeuronAssembler
{
public:
	virtual ~IPMain4NeuronAssembler() {}

	virtual void getNAVersionNum() = 0;

	virtual bool markerMonitorStatus() = 0;
	virtual void sendSelectedMarkers2NA(const QList<ImageMarker>& selectedMarkerList, const QList<ImageMarker>& selectedLocalMarkerList) = 0;

	virtual void eraserSegProcess(V_NeuronSWC_list& displayingSegs, const float nodeCoords[], map<int, set<int>>& seg2Bedited) = 0;
};

Q_DECLARE_INTERFACE(IPMain4NeuronAssembler, "MK.PMainInterface_for_NeuronAssemblerUI/1.0");

#endif