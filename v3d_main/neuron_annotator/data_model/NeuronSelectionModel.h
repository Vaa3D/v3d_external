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

    explicit NeuronSelectionModel(const NaVolumeData& volumeDataParam,
                                  QObject *parentParam = NULL);

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

public:
signals:
    void initialized();
    void overlayUpdated(int index, bool status);
    void neuronMaskUpdated(int index, bool status);
    void selectedNeuronShown(int selectionIndex);
    void allNeuronsShown();
    void allNeuronsCleared();

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
                : NaLockableData::BaseReadLocker(neuronSelectionModelParam.getLock())
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


class NeuronSelectionModelWriter : public QWriteLocker
{
public:
    explicit NeuronSelectionModelWriter(
            NeuronSelectionModel& neuronSelectionModelParam)
        : QWriteLocker(neuronSelectionModelParam.getLock())
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
