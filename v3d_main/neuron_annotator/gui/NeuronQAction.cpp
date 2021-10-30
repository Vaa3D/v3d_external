#include "NeuronQAction.h"

NeuronQAction::NeuronQAction(const QString& textParam, QObject *parentParam /* = 0 */) :
    QAction(textParam, parentParam)
{
    setVisible(true);
    setIconVisibleInMenu(true);
}
