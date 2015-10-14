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
            const DataColorModel& colorModel);

    virtual ~MipFragmentColors();

public slots:
    virtual void update();

protected:
    // input
    const MipFragmentData& mipFragmentData;
    const DataColorModel& dataColorModel;
    // output
    QList<QImage*> fragmentMips; // entry zero(0) is background, entry nFrags+1 is reference/nc82


public:

    class Reader; friend class Reader;
    class Reader : public BaseReadLocker
    {
    public:
        Reader(const MipFragmentColors& mipFragmentColorsParam)
            : BaseReadLocker(mipFragmentColorsParam)
            , mipFragmentColors(mipFragmentColorsParam)
        {}

        size_t getNumImages() const {return mipFragmentColors.fragmentMips.size();}
        const QImage* getImage(int index) {return mipFragmentColors.fragmentMips[index];}

    protected:
        const MipFragmentColors& mipFragmentColors;
    };

    class Writer; friend class Writer;
    class Writer : public BaseWriteLocker
    {
    public:
        Writer(MipFragmentColors& mipFragmentColorsParam)
            : BaseWriteLocker(mipFragmentColorsParam)
            , mipFragmentColors(mipFragmentColorsParam)
        {}

        void allocateImages(int x, int y, int nImages);

    protected:
        MipFragmentColors& mipFragmentColors;
    };


};

#endif // MIPFRAGMENTCOLORS_H
