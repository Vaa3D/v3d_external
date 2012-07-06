#ifndef PRIVATE_VOLUMETEXTURE_H
#define PRIVATE_VOLUMETEXTURE_H

#include <QtGui>
#include "DataColorModel.h"
#include "NeuronSelectionModel.h"
#include "NaVolumeData.h"
#include "Dimension.h"
#include "../DataFlowModel.h"
#include <QObject>
#include <vector>
#include <cassert>
#include <stdint.h>

namespace jfrc {

/// The GLSL shader shader_tex_cmb.txt uses four textures.
/// Each texture uses a different OpenGL texture unit.
/// (though I have read that a single texture unit might be able
///  to have one each of 1D, 2D, 3D textures, I couldn't get it
///  to work that way)
///  1) volume - contains 3D scaled intensity data for up to 4 data channels, in RGBA.
///              Reference intensity is stored in channel 4.
///  2) colormap - 4x256xRGBA 2D texture used to convert data intensities to colors.
///                Fast gamma correction is done with the colormap.
///  3) neuronVisiblity - MAX_NEURON_INDEXxbyte 2D texture encoding visibility of each neuron fragment.
///  4) neuronLabel - 3D label field containing neuron index at each voxel


// templated class should be fully defined in header
template<class DataType>
class Base3DTexture // basis for both NeuronLabelTexture and NeuronSignalTexture
{
public:
    typedef DataType VoxelType;

    explicit Base3DTexture()
        : width(8)
        , height(8)
        , depth(8)
        , bInitialized(false)
    {
        size_t numVoxels = width * height * depth;
        data.assign((size_t)(numVoxels), (VoxelType)0);
        // Test pattern
        for (int i = 0; i < width; ++i)
             for (int j = 0; j < height; ++j)
                 for (int k = 0; k < depth; ++k)
                 {
                     int ix = (int)(k * width * height + j * width + i);
                     assert(ix < numVoxels);
                     assert((ix % 48) >= 0); // number of fragments in realLinkTest + 2
                     data[ix] = (ix % 48);
                 }
    }

    virtual void setValueAt(size_t x, size_t y, size_t z, VoxelType value)
    {
        size_t offset = x + y * width + z * width * height;
        assert(x >= 0); assert(x < width);
        assert(y >= 0); assert(y < height);
        assert(z >= 0); assert(z < depth);
        assert(offset < width * height * depth);
        if (value != 0) {
            int foo = 5;
        }
        data[offset] = value;
    }

    virtual void allocateSize(Dimension paddedTextureSize, Dimension usedTextureSize)
    {
        width = paddedTextureSize.x();
        height = paddedTextureSize.y();
        depth = paddedTextureSize.z();
        paddedSize = paddedTextureSize;
        usedSize = usedTextureSize;
        size_t numVoxels = width * height * depth;
        if (data.size() != numVoxels)
            data.assign((size_t)numVoxels, (VoxelType)0);
        qDebug() << "Allocating" << width << height << depth;
    }

    VoxelType* getData() {return &data[0];}
    const VoxelType* getData() const {return &data[0];}

    size_t getWidth() const {return width;}
    size_t getHeight() const {return height;}
    size_t getDepth() const {return depth;}

    Dimension getUsedSize() const {return usedSize;}
    Dimension getPaddedSize() const {return paddedSize;}

protected:
    std::vector<VoxelType> data;
    size_t width, height, depth;
    Dimension usedSize;
    Dimension paddedSize;
    bool bInitialized;
};


class NeuronLabelTexture : public Base3DTexture<uint16_t>
{};


class NeuronSignalTexture : public Base3DTexture<uint32_t>
{};


// Largest texture size on my Mac is 16384
// So we use a 2D 256x256 texture instead of a 1D 1x65536 texture.
#define MAX_NEURON_INDEX 65536

/// OpenGL texture object for communicating visibility of each neuron fragment to the GLSL shader.
/// 1 x maxNeurons grayscale image with one pixel per neuron fragment.
class NeuronVisibilityTexture
{
public:
    explicit NeuronVisibilityTexture(int maxNeurons = MAX_NEURON_INDEX);
    /// Connect this texture to the NeuronAnnotator unified data model
    void setNeuronSelectionModel(const NeuronSelectionModel& m);
    bool update();
    const uint32_t* getData() const {return &visibilities[0];}

protected:
    const NeuronSelectionModel* neuronSelectionModel;
    std::vector<unsigned int> visibilities;
};


class PrivateVolumeTexture : public QSharedData
{
public:

    typedef unsigned int BGRA; // 32-bit
    typedef BGRA Voxel;

    PrivateVolumeTexture();
    explicit PrivateVolumeTexture(const PrivateVolumeTexture& rhs);

    void initializeSizes(const NaVolumeData::Reader& volumeReader);
    bool subsampleLabelField(const NaVolumeData::Reader& volumeReader);
    bool subsampleColorField(const NaVolumeData::Reader& volumeReader);
    bool subsampleReferenceField(const NaVolumeData::Reader& volumeReader);
    bool populateVolume(const NaVolumeData::Reader& volumeReader, int zBegin, int zEnd);
    bool loadFast3DTexture(int sx, int sy, int sz, const uint8_t* data); // from fast texture
    bool uploadColorMapTextureToVideoCardGL() const;
    bool updateNeuronVisibilityTexture();
    void setNeuronSelectionModel(const NeuronSelectionModel& neuronSelectionModel);
    bool use3DSignalTexture() const {return bUse3DSignalTexture;}
    Dimension originalImageSize; ///< Size of data volume being approximated by this texture set.
    Dimension usedTextureSize; ///< Size of subsection of this texture set containing scaled data volume
    Dimension paddedTextureSize; ///< Total size of this texture set, including empty padded edges to get desired memory alignment
    double subsampleScale; ///< Factor by which a single dimension is subsampled
    // TODO method to load compressed texture directly from file
    const uint32_t* signalData3D() const {return neuronSignalTexture.getData();}
    const uint16_t* labelData3D() const {return neuronLabelTexture.getData();}
    const uint32_t* visibilityData2D() const {return neuronVisibilityTexture.getData();}

protected:
    size_t memoryLimit; // Try not to use more texture memory than this
    int memoryAlignment; // Keep dimensions a multiple of this factor

    NeuronVisibilityTexture neuronVisibilityTexture;
    NeuronLabelTexture neuronLabelTexture;
    NeuronSignalTexture neuronSignalTexture;
    bool bUse3DSignalTexture;
};

} // namespace jfrc

#endif // PRIVATE_VOLUMETEXTURE_H
