#ifndef ZSLICECOLORS_H
#define ZSLICECOLORS_H

#include "NaVolumeData.h"
#include "DataColorModel.h"
#include "NeuronSelectionModel.h"
#include "../geometry/Vector3D.h"

// ZSliceColors creates a QImage for use in the z stack viewer.
class ZSliceColors : public NaLockableData
{
    Q_OBJECT

public:
    explicit ZSliceColors(const NaVolumeData& volumeData,
                          const DataColorModel& dataColorModel,
                          const NeuronSelectionModel& neuronSelectionModel);
    virtual ~ZSliceColors();

public slots:
    virtual void update();
    void setZIndex(int z);
    void onCameraFocusChanged(const Vector3D& focus);

protected:
    // input
    const NaVolumeData& volumeData;
    const DataColorModel& dataColorModel;
    const NeuronSelectionModel& neuronSelectionModel;
    int currentZIndex;
    // output
    QImage * image;

public:
    class Reader : public BaseReadLocker
    {
    public:
        Reader(const ZSliceColors& zSliceColorsParam)
            : BaseReadLocker(zSliceColorsParam)
            , zSliceColors(zSliceColorsParam)
        {}

        const QImage* getImage() const {return zSliceColors.image;}
        int getZIndex() const {return zSliceColors.currentZIndex;}

    protected:
        const ZSliceColors& zSliceColors;
    };
    friend class Reader;


    class Writer : public BaseWriteLocker
    {
    public:
        Writer(ZSliceColors& zSliceColorsParam)
            : BaseWriteLocker(zSliceColorsParam)
            , zSliceColors(zSliceColorsParam)
        {}

        void clearData();

    protected:
        ZSliceColors& zSliceColors;
    };
    friend class Writer;
};

#endif // ZSLICECOLORS_H
