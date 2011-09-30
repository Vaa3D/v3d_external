#include "NeuronSelectionModel.h"
#include "../DataFlowModel.h"

/* explicit */
NeuronSelectionModel::NeuronSelectionModel(const NaVolumeData& volumeDataParam)
    : volumeData(volumeDataParam)
{
    connect(&volumeDataParam, SIGNAL(dataChanged()),
            this, SLOT(initializeSelectionModel()));

    // connect specific signals to more general signals
    connect(this, SIGNAL(initialized()),
            this, SIGNAL(multipleVisibilityChanged()));
    connect(this, SIGNAL(multipleVisibilityChanged()),
            this, SIGNAL(visibilityChanged()));
    connect(this, SIGNAL(neuronVisibilityChanged(int,bool)),
            this, SIGNAL(visibilityChanged()));
    connect(this, SIGNAL(overlayVisibilityChanged(int,bool)),
            this, SIGNAL(visibilityChanged()));
    connect(this, SIGNAL(exactlyOneNeuronShown(int)),
            this, SIGNAL(visibilityChanged()));
    connect(this, SIGNAL(visibilityChanged()),
            this, SIGNAL(dataChanged()));

    connect(this, SIGNAL(exactlyOneNeuronSelected(int)),
            this, SIGNAL(selectionChanged()));
    connect(this, SIGNAL(selectionCleared()),
            this, SIGNAL(selectionChanged()));
    connect(this, SIGNAL(selectionChanged()),
            this, SIGNAL(dataChanged()));
}

/* slot: */
void NeuronSelectionModel::initializeSelectionModel()
{
    // qDebug() << "Initializing NeuronSelectionModel";
    { // curly brackets to restrict scope for locks
        NaVolumeData::Reader volumeReader(volumeData);
        if (! volumeReader.hasReadLock()) return;

        Writer selectionWriter(*this);

        neuronHighlight = -1; // No highlight

        // Two overlays
        overlayStatusList.clear();
        overlayStatusList << false << true; // ref off; bkgd on
        // Redundant, but more informative
        // We do not want reference initially loaded.
        setOverlayStatus(DataFlowModel::REFERENCE_MIP_INDEX, false);
        setOverlayStatus(DataFlowModel::BACKGROUND_MIP_INDEX, true);

        maskStatusList.clear();
        neuronSelectList.clear();
        for (int i = 0; i < volumeReader.getNumberOfNeurons(); ++i)
        {
            maskStatusList << true; // all neurons visible
            neuronSelectList << false; // no neurons selected
        }
    }
    // qDebug() << "Done initializing NeuronSelectionModel";

    emit initialized();
}

void NeuronSelectionModel::updateOverlay(int index, bool status)
{
    if (overlayStatusList[index] == status) return; // no change
    {
        Writer selectionWriter(*this);
        overlayStatusList.replace(index, status);
    }
    emit overlayVisibilityChanged(index, status);
}

void NeuronSelectionModel::updateNeuronMask(int index, bool status)
{
    if (index >= maskStatusList.size()) return; // out of sync
    // qDebug() << "NeuronSelectionModel::updateNeuronMask" << index << status << maskStatusList[index];
    if (maskStatusList[index] == status) {
        // qDebug() << maskStatusList[index] << "equals" << status;
        return; // no change
    }
    {
        Writer selectionWriter(*this);
        maskStatusList.replace(index, status);
    }
    // qDebug() << "emitting neuronVisibilityChanged()" << this;
    emit neuronVisibilityChanged(index, status);
}

void NeuronSelectionModel::setOverlayStatus(int index, bool status)
{
        overlayStatusList.replace(index, status);
}

void NeuronSelectionModel::setNeuronMaskStatus(int index, bool status) {
    maskStatusList.replace(index, status);
}


// switch status of selected neuron
void NeuronSelectionModel::switchSelectedNeuron(int index)
{
    if(neuronSelectList.at(index) == true)
    {
        neuronSelectList.replace(index, false);
    }
    else
    {
        neuronSelectList.replace(index, true);
    }
}

void NeuronSelectionModel::switchSelectedNeuronUniquelyIfOn(int index) {
    bool alreadySelected=neuronSelectList.at(index);
    if (!alreadySelected) {
        // We want to ensure this selection is unique
        for (int i=0;i<neuronSelectList.size();i++) {
            neuronSelectList.replace(i,false);
        }
        neuronSelectList.replace(index, true);
    } else {
        neuronSelectList.replace(index, false);
    }
}


// show all neurons
bool NeuronSelectionModel::showAllNeurons()
{
    bool bChanged = false;
    {
        Writer selectionWriter(*this);
        for (int i=0;i<maskStatusList.size();i++) {
            if (! maskStatusList[i]) {
                maskStatusList.replace(i, true);
                bChanged = true;
            }
        }
    }
    if (bChanged)
        emit multipleVisibilityChanged();
    return bChanged;
}

void NeuronSelectionModel::showOverlays(const QList<int> overlayList) {
    {
        Writer selectionWriter(*this);
        for (int i = 0; i < overlayStatusList.size(); ++i)
            overlayStatusList.replace(i, false);
        for (int i = 0; i < overlayList.size(); ++i)
            overlayStatusList.replace(overlayList[i], true);
    }
    emit multipleVisibilityChanged();
}

void NeuronSelectionModel::showExactlyOneNeuron(int index)
{
    if (index < 0) return;
    if (index >= maskStatusList.size()) return;
    {
        bool bChanged = false;
        Writer selectionWriter(*this);
        for (int i = 0; i < maskStatusList.size(); ++i)
            if (maskStatusList[i] != (i == index)) {
                maskStatusList.replace(i, (i == index));
                bChanged = true;
            }
        if (! bChanged) return;
    }
    // Unselect any previously selected but now invisible neurons
    if (neuronSelectList[index])
        selectExactlyOneNeuron(index);
    else
        clearSelection();
    emit multipleVisibilityChanged();
}

void NeuronSelectionModel::showFirstSelectedNeuron()
{
    int index = -1;
    for (int i=0;i<neuronSelectList.size();i++) {
        if (neuronSelectList[i]) {
            index = i;
            break;
        }
    }
    if (index < 0) return;
    showExactlyOneNeuron(index);
}

// clear all neurons
// affects both selection and visibility, including overlays
void NeuronSelectionModel::clearAllNeurons()
{
    {
        Writer selectionWriter(*this);
        // deselect background and reference
        for (int i=0;i<overlayStatusList.size();i++) {
            overlayStatusList.replace(i, false);
        }
        for (int i=0;i<maskStatusList.size();i++) {
            maskStatusList.replace(i, false);
        }
    }
    clearSelection();
    emit multipleVisibilityChanged();
}

/* slot */
bool NeuronSelectionModel::showAllNeuronsInEmptySpace()
{
    bool bChanged = false;
    bool oldBlockSignals = signalsBlocked();
    qDebug() << "NeuronSelectionModel::showAllNeuronsInEmptySpace()";
    blockSignals(true);
    if (showAllNeurons())
        bChanged = true;
    for (int i = 0; i < overlayStatusList.size(); ++i) {
        if (overlayStatusList[i]) {
            overlayStatusList[i] = false;
            bChanged = true;
        }
    }
    blockSignals(oldBlockSignals);
    if (bChanged)
        emit multipleVisibilityChanged();
    return bChanged;
}

/* slot */
void NeuronSelectionModel::showNothing()
{
    bool oldBlockSignals = signalsBlocked();
    blockSignals(true);
    clearAllNeurons();
    for (int i = 0; i < overlayStatusList.size(); ++i)
        overlayStatusList[i] = false;
    blockSignals(oldBlockSignals);
    // Unselect anything that is now hidden
    // TODO - do this for all potentially hiding visibility operations.
    qDebug() << "Clearing selection after hiding all";
    clearSelection();
    emit multipleVisibilityChanged();
}

/* slot */
void NeuronSelectionModel::showExactlyOneNeuronInEmptySpace(int ix)
{
    bool oldBlockSignals = signalsBlocked();
    blockSignals(true);
    showNothing();
    showExactlyOneNeuron(ix);
    blockSignals(oldBlockSignals);
    emit multipleVisibilityChanged();
}

/* slot */
void NeuronSelectionModel::showExactlyOneNeuronWithBackground(int ix)
{
    bool oldBlockSignals = signalsBlocked();
    blockSignals(true);
    showExactlyOneNeuron(ix);
    QList<int> overlayList;
    overlayList << DataFlowModel::BACKGROUND_MIP_INDEX;
    showOverlays(overlayList);
    blockSignals(oldBlockSignals);
    emit multipleVisibilityChanged();
}

/* slot */
void NeuronSelectionModel::showExactlyOneNeuronWithReference(int ix)
{
    bool oldBlockSignals = signalsBlocked();
    blockSignals(true);
    showExactlyOneNeuron(ix);
    QList<int> overlayList;
    overlayList << DataFlowModel::REFERENCE_MIP_INDEX;
    showOverlays(overlayList);
    blockSignals(oldBlockSignals);
    emit multipleVisibilityChanged();
}

/* slot */
void NeuronSelectionModel::showExactlyOneNeuronWithBackgroundAndReference(int ix)
{
    bool oldBlockSignals = signalsBlocked();
    blockSignals(true);
    showExactlyOneNeuron(ix);
    QList<int> overlayList;
    overlayList << DataFlowModel::REFERENCE_MIP_INDEX << DataFlowModel::BACKGROUND_MIP_INDEX;
    showOverlays(overlayList);
    blockSignals(oldBlockSignals);
    emit multipleVisibilityChanged();
}

// update Neuron Select List
void NeuronSelectionModel::selectExactlyOneNeuron(int index)
{
    // qDebug() << "NeuronSelectionModel::selectExactlyOneNeuron" << index;
    if (index >= neuronSelectList.size()) return;
    bool changed = false;
    if (! neuronSelectList[index])
        changed = true;
    for (int i = 0; i < neuronSelectList.size(); ++i)
    {
        if (neuronSelectList[i] && (i != index))
            changed = true;
    }
    if (! changed) return;

    {
        Writer selectionWriter(*this);
        for (int i=0;i<neuronSelectList.size();i++) {
            neuronSelectList.replace(i, false);
        }
        neuronSelectList.replace(index, true);
    }
    emit exactlyOneNeuronSelected(index);
}

void NeuronSelectionModel::clearSelection()
{
    {
        bool bChanged = false;
        Writer selectionWriter(*this);
        for (int i=0;i<neuronSelectList.size();i++)
            if (neuronSelectList[i]) {
                neuronSelectList.replace(i, false);
                bChanged = true;
            }
        if (!bChanged) return;
    }
    qDebug() << "emit selectionCleared";
    emit selectionCleared();
}


