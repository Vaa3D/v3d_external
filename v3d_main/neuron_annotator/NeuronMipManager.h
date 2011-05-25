#ifndef NEURONMIPMANAGER_H
#define NEURONMIPMANAGER_H

#include <QObject>
#include "ImageColorizer.h"

class NeuronMipManager : public QObject
{
    Q_OBJECT

public:
    explicit NeuronMipManager(QObject *parent = 0);
    const QImage& getCombinedMipImage();

signals:

public slots:
    void setGamma(double);

protected:
    ImageColorizer colorizer;
};

#endif // NEURONMIPMANAGER_H
