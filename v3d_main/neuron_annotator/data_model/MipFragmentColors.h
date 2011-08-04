#ifndef MIPFRAGMENTCOLORS_H
#define MIPFRAGMENTCOLORS_H

#include "NaLockableData.h"
#include "MipFragmentData.h"
#include "DataColorModel.h"
#include <QImage>
#include <QList>

// MipFragmentColors creates a set of 24-bit(rgb8-8-8) color QImage
// maximum intensity projections for a set of neuron fragments plus
// background and reference/nc82.
// MipFragmentColors combines 16-bit multichannel MipFragmentData
// with a DataColorModel to create color images.
class MipFragmentColors : public NaLockableData
{
    Q_OBJECT
public:
    explicit MipFragmentColors(
            const MipFragmentData& mipFragmentData,
            const DataColorModel& colorModel,
            QObject *parentParam = NULL);

signals:

public slots:

protected:
    // input
    const MipFragmentData& mipFragmentData;
    const DataColorModel& dataColorModel;
    // output
    QList<QImage*> fragmentMips; // entry zero(0) is background, entry nFrags+1 is reference/nc82
};

#endif // MIPFRAGMENTCOLORS_H
