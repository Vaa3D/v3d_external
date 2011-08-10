#include "NeuronSelectionModel.h"
#include "../AnnotationSession.h"

/* explicit */
NeuronSelectionModel::NeuronSelectionModel(const NaVolumeData& volumeDataParam)
    : volumeData(volumeDataParam)
{
    connect(&volumeDataParam, SIGNAL(dataChanged()),
            this, SLOT(initializeSelectionModel()));
}

/* slot: */
void NeuronSelectionModel::initializeSelectionModel()
{
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
        setOverlayStatus(AnnotationSession::REFERENCE_MIP_INDEX, false);
        setOverlayStatus(AnnotationSession::BACKGROUND_MIP_INDEX, true);

        maskStatusList.clear();
        neuronSelectList.clear();
        for (int i = 0; i < volumeReader.getNumberOfNeurons(); ++i)
        {
            maskStatusList << true; // all neurons visible
            neuronSelectList << false; // no neurons selected
        }
    }

    emit initialized();
    emit multipleVisibilityChanged();
    emit dataChanged();
}

void NeuronSelectionModel::updateOverlay(int index, bool status)
{
    if (overlayStatusList[index] == status) return; // no change
    {
        Writer selectionWriter(*this);
        overlayStatusList.replace(index, status);
    }
    emit overlayVisibilityChanged(index, status);
    emit dataChanged();
}

void NeuronSelectionModel::updateNeuronMask(int index, bool status)
{
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
    emit dataChanged();
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

// show selected neuron
void NeuronSelectionModel::showSelectedNeuron(const QList<int>& overlayList)
{
    int selectionIndex=-1;
    for (int i=0;i<neuronSelectList.size();i++) {
        if (neuronSelectList.at(i)) {
            selectionIndex=i;
            break;
        }
    }
    if (selectionIndex<0 || selectionIndex>=maskStatusList.size()) {
        // nothing to do
        return;
    }
    {
        Writer selectionWriter(*this);
        for (int i=0;i<overlayStatusList.size();i++) {
            overlayStatusList.replace(i, false);
        }
        for (int i=0;i<overlayList.size();i++) {
            overlayStatusList.replace(overlayList.at(i), true);
        }
        for (int i=0;i<maskStatusList.size();i++) {
            maskStatusList.replace(i, false);
        }
        maskStatusList.replace(selectionIndex, true);
    }
    emit selectedNeuronShown(selectionIndex);
    emit multipleVisibilityChanged();
    emit dataChanged();
}

// show all neurons
void NeuronSelectionModel::showAllNeurons(const QList<int>& overlayList)
{
    {
        Writer selectionWriter(*this);
        for (int i=0;i<overlayStatusList.size();i++) {
            overlayStatusList.replace(i, false);
        }
        for (int i=0;i<overlayList.size();i++) {
            overlayStatusList.replace(overlayList.at(i), true);
        }
        for (int i=0;i<maskStatusList.size();i++) {
            maskStatusList.replace(i, true);
        }
    }
    emit multipleVisibilityChanged();
    emit dataChanged();
}

// clear all neurons
void NeuronSelectionModel::clearAllNeurons()
{
    {
        Writer selectionWriter(*this);
        // deselect background and reference
        for (int i=0;i<overlayStatusList.size();i++) {
            overlayStatusList.replace(i, false);
        }

        // deselect neurons
        for (int i=0;i<neuronSelectList.size();i++) {
            neuronSelectList.replace(i, false);
        }
        for (int i=0;i<maskStatusList.size();i++) {
            maskStatusList.replace(i, false);
        }
    }
    emit multipleVisibilityChanged();
    emit dataChanged();
}

// update Neuron Select List
void NeuronSelectionModel::updateNeuronSelectList(int index)
{

    {
        Writer selectionWriter(*this);
        for (int i=0;i<neuronSelectList.size();i++) {
            neuronSelectList.replace(i, false);
            maskStatusList.replace(i, false);
        }

        neuronSelectList.replace(index, true);
        maskStatusList.replace(index, true);
    }

    emit multipleVisibilityChanged();
    emit dataChanged();
}

void NeuronSelectionModel::clearSelections()
{
    {
        Writer selectionWriter(*this);
        for (int i=0;i<neuronSelectList.size();i++) {
            neuronSelectList.replace(i, false);
        }
    }
    emit dataChanged();
}


