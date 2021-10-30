#ifndef NEURONCONTEXTMENU_H
#define NEURONCONTEXTMENU_H

#include <QMenu>
#include "NeuronQAction.h"
#include "../data_model/NeuronSelectionModel.h"

class NeuronContextMenu : public QMenu
{
    Q_OBJECT
public:
    explicit NeuronContextMenu(QWidget *parent = 0);
    QAction* exec(const QPoint& pos, int neuronIndex, bool isShown = true);
    void connectActions(const NeuronSelectionModel& neuronSelectionModel);

signals:

public slots:

protected:
    //
    QAction* neuronTitleAction;
    NeuronQAction* hideThisNeuronAction;
    NeuronQAction* showThisNeuronAction;
    NeuronQAction* showOnlyThisNeuronAction;
    NeuronQAction* showOnlyThisNeuronWithBackgroundAction;
    NeuronQAction* showOnlyThisNeuronWithReferenceAction;
    NeuronQAction* showOnlyThisNeuronWithBackgroundAndReferenceAction;
};

#endif // NEURONCONTEXTMENU_H
