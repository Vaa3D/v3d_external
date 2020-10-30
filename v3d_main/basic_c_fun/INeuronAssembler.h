#ifndef INEURONASSEMBLER_H
#define INEURONASSEMBLER_H

#include <QtCore>

#include <boost/container/flat_map.hpp>

#include "IPMain4NeuronAssembler.h"

using namespace std;

class INeuronAssembler
{
public:
	virtual ~INeuronAssembler() {}

	virtual bool teraflyImgInstance() = 0;

	virtual void sendCastNAUI2PMain(IPMain4NeuronAssembler* NAportal) = 0;
	//virtual void forceCViewerPortalUpdate() = 0;
	virtual bool checkFragTraceStatus() = 0;
	virtual void changeFragTraceStatus(bool newStatus) = 0;
	virtual int getViewerID() = 0;
	virtual void printOutCViewerAddress() = 0;

	virtual string getCviewerWinTitle() = 0;
	virtual void printoutWinTitle() = 0;
	virtual int getTeraflyTotalResLevel() = 0;
	virtual int getTeraflyResLevel() = 0;
	virtual int getZoomingFactor() = 0;
	virtual bool getXlockStatus() = 0;
	virtual bool getYlockStatus() = 0;
	virtual bool getZlockStatus() = 0;
	virtual bool getPartialVolumeCoords(int globalCoords[], int localCoords[], int displayingVolDims[]) = 0;

	virtual void refreshSelectedMarkers() = 0;
	virtual void pushMarkersfromTester(const set<vector<float> >& markerCoords, RGBA8 color) = 0;

	virtual vector<V_NeuronSWC>* getDisplayingSegs() = 0;
	virtual void updateDisplayingSegs() = 0;
	virtual void editingModeInit() = 0;
	virtual void setEraserSize(int newEraserSize) = 0;
	virtual int getEraserSize() = 0;
	virtual void setConnectorSize(int newConnectorSize) = 0;
	virtual int getConnectorSize() = 0;
	virtual void segEditing_setCursor(string action) = 0;

	////‘>>’ should be ‘> >’ within a nested template argument list
	virtual int getNearestSegEndClusterCentroid(const boost::container::flat_map<int, vector<float> >& segEndClusterCentroidMap) = 0;
	virtual void convertLocalCoord2windowCoord(const float localCoord[], float windowCoord[]) = 0;
	virtual void convertWindowCoord2likelyLocalCoord(const int mouseX, const int mouseY, float putativeCoord[]) = 0;

	virtual void getParamsFromFragTraceUI(const string& keyName, const float& value) = 0;
};

// INeuronAssembler is not a Q_OBJECT, hence Q_DECLARE_INTERFACE is needed for CViewer class to inherit.
Q_DECLARE_INTERFACE(INeuronAssembler, "MK.NeuronAssemblerInterface_for_CViewerClass/1.0");


#endif
