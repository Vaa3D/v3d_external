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

	virtual void updateCViewerPortal() = 0;

	virtual bool markerMonitorStatus() = 0;
	virtual void sendSelectedMarkers2NA(const QList<ImageMarker>& selectedMarkerList, const QList<ImageMarker>& selectedLocalMarkerList) = 0;

	////‘>>’ should be ‘> >’ within a nested template argument list
	virtual void eraserSegProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC> >& seg2includedNodeMap) = 0;
	virtual void connectSegProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC> >& seg2includedNodeMap) = 0;
	virtual void exitNAeditingMode() = 0;

	virtual bool changeAssociatedSegsClicked() = 0;
	virtual void signalNA2retypeConnectedSegs(V_NeuronSWC_list& displayingSegs, const int retypedSeg, const int type) = 0;
	virtual void signalNA2retypeConnectedSegs(V_NeuronSWC_list& displayingSegs, const set<int>& retypedSegs, const int type) = 0;
};

Q_DECLARE_INTERFACE(IPMain4NeuronAssembler, "MK.PMainInterface_for_NeuronAssemblerUI/1.0");

#endif
