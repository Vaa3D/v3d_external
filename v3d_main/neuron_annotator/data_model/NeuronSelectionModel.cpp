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
        Writer selectionWriter(*this);

        neuronHighlight = -1; // No highlight

        // Two overlays
        overlayStatusList.clear();
        overlayStatusList << false << true; // ref off; bkgd on
        // Redundant, but more informative
        // We do not want reference initially loaded.
        setOverlayStatus(DataFlowModel::REFERENCE_MIP_INDEX, false);
        setOverlayStatus(DataFlowModel::BACKGROUND_MIP_INDEX, true);

        for (int n = 0; n < maskStatusList.size(); ++n)
            maskStatusList[n] = true;
        for (int n = 0; n < neuronSelectList.size(); ++n)
            neuronSelectList[n] = false;

        NaVolumeData::Reader volumeReader(volumeData);
        if (volumeReader.hasReadLock()) {
            maskStatusList.clear();
            neuronSelectList.clear();
            for (int i = 0; i < volumeReader.getNumberOfNeurons(); ++i)
            {
                maskStatusList << true; // all neurons visible
                neuronSelectList << false; // no neurons selected
            }
        }
    }
    // qDebug() << "Done initializing NeuronSelectionModel" << __FILE__ << __LINE__;

    emit initialized();
}

bool NeuronSelectionModel::updateOverlay(int index, bool status)
{
    // qDebug() << "NeuronSelectionModel::updateOverlay" << index << status << __FILE__ << __LINE__;
    if (index < 0) return false;
    if (index >= overlayStatusList.size()) return false;
    bool bChanged = false;
    if (overlayStatusList[index] == status) return bChanged; // no change
    {
        Writer selectionWriter(*this);
        overlayStatusList.replace(index, status);
        bChanged = true;
    }
    emit overlayVisibilityChanged(index, status);
    return bChanged;
}

/* slot */
bool NeuronSelectionModel::hideOneNeuron(int index)
{
    return updateNeuronMask(index, false);
}

/* slot */
bool NeuronSelectionModel::showOneNeuron(int index)
{
    return updateNeuronMask(index, true);
}

bool NeuronSelectionModel::updateNeuronMask(int index, bool status)
{
    // qDebug() << "NeuronSelectionModel::updateNeuronMask" << index << status << maskStatusList[index];
    bool bChanged = false;
    if (index >= maskStatusList.size()) return bChanged; // out of sync
    if (maskStatusList[index] == status) {
        // qDebug() << maskStatusList[index] << "equals" << status;
        return bChanged; // no change
    }
    {
        Writer selectionWriter(*this);
        maskStatusList.replace(index, status);
        bChanged = true;
    }
    // qDebug() << "emitting neuronVisibilityChanged()" << this;
    // qDebug() << "NeuronSelectionModel::updateNeuronMask" << index << status << __FILE__ << __LINE__;
    emit neuronVisibilityChanged(index, status);
    return bChanged;
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
    if (bChanged) {
        // qDebug() << "NeuronSelectionModel::showAllNeurons()" << __FILE__ << __LINE__;
        emit multipleVisibilityChanged();
    }
    return bChanged;
}

bool NeuronSelectionModel::showOverlays(const QList<int> overlayList) {
    bool bChanged = false;
    std::vector<bool> newList(overlayStatusList.size(), false);
    for (int i = 0; i < overlayList.size(); ++i)
        newList[overlayList[i]] = true;
    {
        Writer selectionWriter(*this);
        for (int i = 0; i < overlayStatusList.size(); ++i) {
            if (overlayStatusList[i] != newList[i]) {
                overlayStatusList[i] = newList[i];
                bChanged = true;
            }
        }
    }
    if (bChanged) {
        // qDebug() << "NeuronSelectionModel::showOverlays()" << __FILE__ << __LINE__;
        emit multipleVisibilityChanged();
    }
    return bChanged;
}

bool NeuronSelectionModel::showExactlyOneNeuron(int index)
{
    bool bChanged = false;
    if (index < 0) return bChanged;
    if (index >= maskStatusList.size()) return bChanged;
    {
        Writer selectionWriter(*this);
        for (int i = 0; i < maskStatusList.size(); ++i)
            if (maskStatusList[i] != (i == index)) {
                maskStatusList.replace(i, (i == index));
                bChanged = true;
            }
        if (! bChanged) return bChanged;
    }
    // Unselect any previously selected but now invisible neurons
    if (neuronSelectList[index])
        selectExactlyOneNeuron(index);
    else
        clearSelection();
    if (bChanged) {
        // qDebug() << "NeuronSelectionModel::showExactlyOneNeuron" << index << __FILE__ << __LINE__;
        emit multipleVisibilityChanged();
    }
    return bChanged;
}

bool NeuronSelectionModel::showFirstSelectedNeuron()
{
    int index = -1;
    for (int i=0;i<neuronSelectList.size();i++) {
        if (neuronSelectList[i]) {
            index = i;
            break;
        }
    }
    if (index < 0) return false;
    return showExactlyOneNeuron(index);
}

// clear all neurons
// affects both selection and visibility, including overlays
bool NeuronSelectionModel::clearAllNeurons()
{
    // qDebug() << "NeuronSelectionModel::clearAllNeurons";
    bool bChanged = false;
    {
        Writer selectionWriter(*this);
        // deselect background and reference
        for (int i=0;i<overlayStatusList.size();i++) {
            if (overlayStatusList[i]) {
                overlayStatusList[i] = false;
                bChanged = true;
            }
        }
        for (int i=0;i<maskStatusList.size();i++) {
            if (maskStatusList[i]) {
                maskStatusList[i] = false;
                bChanged = true;
            }
        }
    }
    if (clearSelection())
        bChanged = true;
    if (bChanged) {
        // qDebug() << "NeuronSelectionModel::clearAllNeurons" << __FILE__ << __LINE__;
        emit multipleVisibilityChanged();
    }
    return bChanged;
}

/* slot */
bool NeuronSelectionModel::showAllNeuronsInEmptySpace()
{
    bool bChanged = false;
    bool oldBlockSignals = signalsBlocked();
    // qDebug() << "NeuronSelectionModel::showAllNeuronsInEmptySpace()";
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
    if (bChanged) {
        // qDebug() << "NeuronSelectionModel::showAllNeuronsInEmptySpace" << __FILE__ << __LINE__;
        emit multipleVisibilityChanged();
    }
    return bChanged;
}

/* slot */
bool NeuronSelectionModel::showNothing()
{
    // hides all neurons and overlays.
    // we deselect everything that's hidden, so showNothing() is equivalent to clearAllNeurons()
    return clearAllNeurons();
}

/* slot */
bool NeuronSelectionModel::showExactlyOneNeuronInEmptySpace(int ix)
{
    bool bChanged = false;
    bool oldBlockSignals = signalsBlocked();
    blockSignals(true);
    for (int i = 0; i < overlayStatusList.size(); ++i)
        if (overlayStatusList[i]) {
            overlayStatusList[i] = false;
            bChanged = true;
        }
    if (showExactlyOneNeuron(ix))
        bChanged = true;
    blockSignals(oldBlockSignals);
    if (bChanged)
        emit multipleVisibilityChanged();
    return bChanged;
}

/* slot */
bool NeuronSelectionModel::showExactlyOneNeuronWithBackground(int ix)
{
    bool bChanged = false;
    bool oldBlockSignals = signalsBlocked();
    blockSignals(true);
    if (showExactlyOneNeuron(ix))
        bChanged = true;
    QList<int> overlayList;
    overlayList << DataFlowModel::BACKGROUND_MIP_INDEX;
    if (showOverlays(overlayList))
        bChanged = true;
    blockSignals(oldBlockSignals);
    if (bChanged)
        emit multipleVisibilityChanged();
    return bChanged;
}

/* slot */
bool NeuronSelectionModel::showExactlyOneNeuronWithReference(int ix)
{
    bool bChanged = false;
    bool oldBlockSignals = signalsBlocked();
    blockSignals(true);
    if (showExactlyOneNeuron(ix))
        bChanged = true;
    QList<int> overlayList;
    overlayList << DataFlowModel::REFERENCE_MIP_INDEX;
    if (showOverlays(overlayList))
        bChanged = true;
    blockSignals(oldBlockSignals);
    if (bChanged)
        emit multipleVisibilityChanged();
    return bChanged;
}

/* slot */
bool NeuronSelectionModel::showExactlyOneNeuronWithBackgroundAndReference(int ix)
{
    bool bChanged = false;
    bool oldBlockSignals = signalsBlocked();
    blockSignals(true);
    if (showExactlyOneNeuron(ix))
        bChanged = true;
    QList<int> overlayList;
    overlayList << DataFlowModel::REFERENCE_MIP_INDEX << DataFlowModel::BACKGROUND_MIP_INDEX;
    if (showOverlays(overlayList))
        bChanged = true;
    blockSignals(oldBlockSignals);
    if (bChanged)
        emit multipleVisibilityChanged();
    return bChanged;
}

// update Neuron Select List
bool NeuronSelectionModel::selectExactlyOneNeuron(int index)
{
    bool changed = false;
    // qDebug() << "NeuronSelectionModel::selectExactlyOneNeuron" << index;
    if (index >= neuronSelectList.size()) return changed;
    if (! neuronSelectList[index])
        changed = true;
    for (int i = 0; i < neuronSelectList.size(); ++i)
    {
        if (neuronSelectList[i] && (i != index))
            changed = true;
    }
    if (! changed) return changed;
    {
        Writer selectionWriter(*this);
        for (int i=0;i<neuronSelectList.size();i++) {
            neuronSelectList.replace(i, false);
        }
        neuronSelectList.replace(index, true);
    }
    if (changed)
        // qDebug() << "emitting exactlyOneNeuronSelected" << index;
        emit exactlyOneNeuronSelected(index);
    return changed;
}

bool NeuronSelectionModel::clearSelection()
{
    // qDebug() << "NeuronSelectionModel::clearSelection()" << __FILE__ << __LINE__;
    bool bChanged = false;
    for (int i=0;i<neuronSelectList.size();i++) {
        if (neuronSelectList[i]) {
            bChanged = true;
            break;
        }
    }
    if (!bChanged) return bChanged;
    {
        Writer selectionWriter(*this);
        for (int i=0;i<neuronSelectList.size();i++)
            neuronSelectList[i] = false;
    }
    // qDebug() << "emit selectionCleared";
    emit selectionCleared();
    return bChanged;
}


