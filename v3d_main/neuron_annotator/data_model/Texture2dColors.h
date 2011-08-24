#ifndef TEXTURE2DCOLORS_H
#define TEXTURE2DCOLORS_H

#include "DataColorModel.h"
#include "NeuronSelectionModel.h"

// class Texture2dColors is a data source for opengl volume rendering
// using 2D textures.

// AxisStack class holds the displayed textures along one dimension
template<class PixelFormat> // e.g. RGBA8
class AxisStack {
public:
    enum Axis {X_AXIS, Y_AXIS, Z_AXIS};

    AxisStack(Axis axis1,
              Axis axis2,
              Axis axis3);
    ~AxisStack();
    // operator[] returns pointer for use as final argument to glTexImage2D
    const PixelFormat* operator[](int sliceIx) const;
    // Only call uploadOpenGlTextures from your opengl thread!
    void uploadOpenGlTextures() const;

private:
    PixelFormat** data;
    Axis axes[3];
    size_t sizes[3];
    // TODO - populate textureIds
    std::vector<unsigned int> textureIds;
};

class Texture2dColors : public NaLockableData
{
    Q_OBJECT

public:
    typedef RGBA8 PixelType;
    typedef AxisStack<PixelType> Stack;
    static const AxisStack<PixelType>::Axis X_AXIS = Stack::X_AXIS;
    static const AxisStack<PixelType>::Axis Y_AXIS = Stack::Y_AXIS;
    static const AxisStack<PixelType>::Axis Z_AXIS = Stack::Z_AXIS;

    explicit Texture2dColors(
            const NaVolumeData& volumeDataParam,
            const DataColorModel& dataColorModelParam,
            const NeuronSelectionModel& neuronSelectionModelParam);

protected:
    // Only call uploadOpenGlTextures from your opengl thread!
    // (thus this one operation cannot be parallelized)
    void uploadOpenGlTextures();

protected:
    Stack xStack;
    Stack yStack;
    Stack zStack;
    int xProgress, yProgress, zProgress;
};

#endif // TEXTURE2DCOLORS_H
