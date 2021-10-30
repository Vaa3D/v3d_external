#ifndef NEURONQACTION_H
#define NEURONQACTION_H

#include <QAction>

// QAction that applies to a particular neuron with an integer index.
class NeuronQAction : public QAction
{
    Q_OBJECT
public:
    explicit NeuronQAction(const QString& text, QObject *parent = 0);

signals:
    void triggeredWithIndex(int);

public slots:
    void triggerWithIndex(int i) {
        emit triggeredWithIndex(i);
    }
};

#endif // NEURONQACTION_H
