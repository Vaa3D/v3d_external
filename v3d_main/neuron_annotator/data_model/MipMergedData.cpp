#include "MipMergedData.h"


///////////////////////////
// MipMergeLayer methods //
///////////////////////////

void MipMergeLayer::setVisiblity(bool bIsVisibleParam)
{}

void MipMergeLayer::update() // emits dataChanged() if data changes.
{}

void MipMergeLayer::updateWithoutSignal() // for manual full updates
{}


///////////////////////////
// MipMergedData methods //
///////////////////////////

/* explicit */
MipMergedData::MipMergedData(
                const MipFragmentData& mipFragmentDataParam,
                const DataColorModel& dataColorModelParam,
                const NeuronSelectionModel& neuronSelectionModelParam)
        : mipFragmentData(mipFragmentDataParam)
        , dataColorModel(dataColorModelParam)
        , neuronSelectionModel(neuronSelectionModelParam)
{
    connect(&mipFragmentData, SIGNAL(dataChanged()),
            this, SLOT(update()));
    connect(&dataColorModel, SIGNAL(dataChanged()),
            this, SLOT(updateColors()));
    connect(&neuronSelectionModel, SIGNAL(neuronVisibilityChanged(int,bool)),
            this, SLOT(toggleNeuronVisibility(int,bool)));
    connect(&neuronSelectionModel, SIGNAL(overlayVisibilityChanged(int,bool)),
            this, SLOT(toggleOverlayVisibility(int,bool)));
    connect(&neuronSelectionModel, SIGNAL(multipleVisibilityChanged()),
            this, SLOT(updateNeuronVisibility()));
}

/* virtual */
MipMergedData::~MipMergedData() {}

/* virtual */
void MipMergedData::update()
{
    // TODO
    qDebug() << "WARNING: MipMergedData::update() method not yet written.";
}

void MipMergedData::toggleNeuronVisibility(int index, bool status) // update a single neuron, on neuronSelectionModel.neuronMaskUpdated, O(log nfrags)
{}

void MipMergedData::toggleOverlayVisibility(int index, bool status)
{}

void MipMergedData::updateNeuronVisibility() // remerge all neurons O(nfrags), on neuronSelectionModel.visibilityChanged? or dirty partial update.
{}

void MipMergedData::updateColors() // on dataColorModel.dataChanged, or mergedImage change
{}


