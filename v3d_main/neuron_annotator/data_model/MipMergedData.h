#ifndef MIPMERGEDDATA_H
#define MIPMERGEDDATA_H

#include "NaLockableData.h"
#include "MipFragmentData.h"

// MipMergedData combines a subset of neuron fragments and overlays into
// a blended image, for use in the NaLargeMipWidget.
class MipMergedData : public NaLockableData
{
    Q_OBJECT
public:
    explicit MipMergedData(
            const MipFragmentData& mipFragmentData,
            QObject *parent = NULL);

public slots:
    void update();

protected:
    const MipFragmentData& mipFragmentData;


public:
    class Reader : public BaseReadLocker
    {
    };


    class Writer : public QWriteLocker
    {
    };
};

#endif // MIPMERGEDDATA_H
