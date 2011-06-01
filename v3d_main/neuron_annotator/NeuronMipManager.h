#ifndef NEURONMIPMANAGER_H
#define NEURONMIPMANAGER_H

#include <QObject>
#include <QImage>
#include "../v3d/v3d_core.h" /* My4DImage */
#include "ImageColorizer.h"
#include "BrightnessCalibrator.h"

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
    My4DImage * volume4DImage;
    My4DImage * mask4DImage;
    unsigned int numNeurons;
    ImageColorizer colorizer;
    BrightnessCalibrator brightnessCalibrator;
    QImage * combinedMipImage;
};

#endif // NEURONMIPMANAGER_H
