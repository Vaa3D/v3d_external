#ifndef NEURONSELECTIONMODEL_H
#define NEURONSELECTIONMODEL_H

#include "NaLockableData.h"
#include "NaVolumeData.h"
#include <QList>

class NeuronSelectionModelReader;
class NeuronSelectionModelWriter;

// NeuronSelectionModel maintains lists of neurons/fragments
// that are visible/hidden, selected, or highlighted.
class NeuronSelectionModel : public NaLockableData
{
    Q_OBJECT

public:
    friend class NeuronSelectionModelReader;
    friend class NeuronSelectionModelWriter;
    typedef NeuronSelectionModelReader Reader;
    typedef NeuronSelectionModelWriter Writer;
    typedef int NeuronIndex;

    // When a new data volume is loaded, a new selection model will be initialized.
    explicit NeuronSelectionModel(const NaVolumeData& volumeDataParam);

protected:
    bool neuronMaskIsChecked(int index) const { return maskStatusList.at(index); }
    bool overlayIsChecked(int index) const { return overlayStatusList.at(index); }
    const QList<bool>& getMaskStatusList() const {return maskStatusList;}
    void setNeuronMaskStatus(int index, bool status);
    const QList<bool>& getOverlayStatusList() const {return overlayStatusList;}
    void setOverlayStatus(int index, bool status);
    const QList<bool>& getNeuronSelectList() const {return neuronSelectList;}

    void switchSelectedNeuron(int index);
    void switchSelectedNeuronUniquelyIfOn(int index);
    void clearSelections();

signals:
    void initialized(); // signal that data structures were initialized after new volume load
    // Visibility
    void overlayVisibilityChanged(int index, bool status); // single overlay toggled
    void neuronVisibilityChanged(int index, bool status); // single neuron toggled
    void multipleVisibilityChanged(); // global change
    // Selection
    void selectedNeuronShown(int selectionIndex); // ?
    // TODO highlight signals (not selection)

public slots:
    void initializeSelectionModel();
    void updateNeuronMask(int index, bool status);
    void updateOverlay(int index, bool status);
    void showSelectedNeuron(const QList<int>& overlayList);
    void showAllNeurons(const QList<int>& overlayList);
    void clearAllNeurons();
    void updateNeuronSelectList(int index);

protected:
    const NaVolumeData& volumeData;
    QList<bool> maskStatusList; // neuron visibility
    QList<bool> overlayStatusList; // background/reference visibility
    QList<bool> neuronSelectList; // neuron selection
    int neuronHighlight; // index of hightlighted neuron.  -1 means none.
};


class NeuronSelectionModelReader : public NaLockableData::BaseReadLocker
{
public:
    explicit NeuronSelectionModelReader(
            const NeuronSelectionModel& neuronSelectionModelParam,
            QObject * parentParam = NULL)
                : NaLockableData::BaseReadLocker(neuronSelectionModelParam)
                , neuronSelectionModel(neuronSelectionModelParam)
    {}

    bool neuronMaskIsChecked(int index) const { return neuronSelectionModel.maskStatusList.at(index); }
    bool overlayIsChecked(int index) const { return neuronSelectionModel.overlayStatusList.at(index); }
    const QList<bool>& getMaskStatusList() const {return neuronSelectionModel.maskStatusList;}
    const QList<bool>& getOverlayStatusList() const {return neuronSelectionModel.overlayStatusList;}
    const QList<bool>& getNeuronSelectList() const {return neuronSelectionModel.neuronSelectList;}

protected:
    const NeuronSelectionModel& neuronSelectionModel;
};


class NeuronSelectionModelWriter : public NaLockableData::BaseWriteLocker
{
public:
    explicit NeuronSelectionModelWriter(NeuronSelectionModel& neuronSelectionModelParam)
        : NaLockableData::BaseWriteLocker(neuronSelectionModelParam)
        , neuronSelectionModel(neuronSelectionModelParam)
    {}

    bool neuronMaskIsChecked(int index) const { return neuronSelectionModel.maskStatusList.at(index); }
    bool overlayIsChecked(int index) { return neuronSelectionModel.overlayStatusList.at(index); }
    QList<bool>& getMaskStatusList() {return neuronSelectionModel.maskStatusList;}
    QList<bool>& getOverlayStatusList() {return neuronSelectionModel.overlayStatusList;}
    QList<bool>& getNeuronSelectList() {return neuronSelectionModel.neuronSelectList;}
    void switchSelectedNeuronUniquelyIfOn(int index) {
        neuronSelectionModel.switchSelectedNeuronUniquelyIfOn(index);
    }
    void clearSelections() {
        neuronSelectionModel.clearSelections();
    }
    void updateNeuronSelectList(int index) {
        neuronSelectionModel.updateNeuronSelectList(index);
    }

protected:
    NeuronSelectionModel& neuronSelectionModel;
};


#endif // NEURONSELECTIONMODEL_H
