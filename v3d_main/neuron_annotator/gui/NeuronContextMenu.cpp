#include "NeuronContextMenu.h"

NeuronContextMenu::NeuronContextMenu(QWidget *parent) :
    QMenu(parent)
{
    neuronTitleAction = new QAction("(no neuron specified)", this);
    neuronTitleAction->setEnabled(false);

    hideThisNeuronAction = new NeuronQAction(
            "Hide this neuron",
            this);

    showThisNeuronAction = new NeuronQAction(
            "Show this neuron",
            this);

    showOnlyThisNeuronAction = new NeuronQAction(
            "View only this neuron in empty space",
            this);
    showOnlyThisNeuronAction->setIcon(QIcon(":/icons/neuronwobg.png"));

    showOnlyThisNeuronWithBackgroundAction = new NeuronQAction(
            "View only this neuron with background",
            this);
    showOnlyThisNeuronWithBackgroundAction->setIcon(QIcon(":/icons/neuronwbg.png"));

    showOnlyThisNeuronWithReferenceAction = new NeuronQAction(
            "View only this neuron with reference",
            this);
    showOnlyThisNeuronWithReferenceAction->setIcon(QIcon(":/icons/neuronwref.png"));

    showOnlyThisNeuronWithBackgroundAndReferenceAction = new NeuronQAction(
            "View only this neuron with background and reference",
            this);
    showOnlyThisNeuronWithBackgroundAndReferenceAction->setIcon(QIcon(":/icons/neuronwbgref.png"));

    addAction(neuronTitleAction);
    addAction(hideThisNeuronAction);
    addAction(showThisNeuronAction);
    addAction(showOnlyThisNeuronAction);
    addAction(showOnlyThisNeuronWithBackgroundAction);
    addAction(showOnlyThisNeuronWithReferenceAction);
    addAction(showOnlyThisNeuronWithBackgroundAndReferenceAction);
}

QAction* NeuronContextMenu::exec(const QPoint& pos, int neuronIndex, bool isShown)
{
    neuronTitleAction->setText(QString("Neuron fragment %1").arg(neuronIndex));
    if (isShown) {
        hideThisNeuronAction->setEnabled(true);
        hideThisNeuronAction->setVisible(true);
        showThisNeuronAction->setVisible(false);
    }
    else {
        showThisNeuronAction->setEnabled(true);
        showThisNeuronAction->setVisible(true);
        hideThisNeuronAction->setVisible(false);
    }
    QAction* act = QMenu::exec(pos, NULL);
    NeuronQAction* nact = dynamic_cast<NeuronQAction*>(act);
    if (nact)
        nact->triggerWithIndex(neuronIndex);
    return act;
}

void NeuronContextMenu::connectActions(const NeuronSelectionModel& neuronSelectionModel)
{
    connect(hideThisNeuronAction, SIGNAL(triggeredWithIndex(int)),
            &neuronSelectionModel, SLOT(hideOneNeuron(int)));
    connect(showThisNeuronAction, SIGNAL(triggeredWithIndex(int)),
            &neuronSelectionModel, SLOT(showOneNeuron(int)));
    connect(showOnlyThisNeuronAction, SIGNAL(triggeredWithIndex(int)),
            &neuronSelectionModel, SLOT(showExactlyOneNeuronInEmptySpace(int)));
    connect(showOnlyThisNeuronWithBackgroundAction, SIGNAL(triggeredWithIndex(int)),
            &neuronSelectionModel, SLOT(showExactlyOneNeuronWithBackground(int)));
    connect(showOnlyThisNeuronWithReferenceAction, SIGNAL(triggeredWithIndex(int)),
            &neuronSelectionModel, SLOT(showExactlyOneNeuronWithReference(int)));
    connect(showOnlyThisNeuronWithBackgroundAndReferenceAction, SIGNAL(triggeredWithIndex(int)),
            &neuronSelectionModel, SLOT(showExactlyOneNeuronWithBackgroundAndReference(int)));
}



