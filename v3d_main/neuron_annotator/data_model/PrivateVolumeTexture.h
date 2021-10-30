#ifndef PRIVATE_VOLUMETEXTURE_H
#define PRIVATE_VOLUMETEXTURE_H

#include "DataColorModel.h"
#include "NeuronSelectionModel.h"
#include "NaVolumeData.h"
#include "Dimension.h"
#include "../DataFlowModel.h"
#include "Fast3DTexture.h"
#include <vector>
#include <cassert>
#include <stdint.h>
// #include <QVector>
#include <QtGui>
#include <QObject>

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
        // , bInitialized(false)
    {
        size_t numVoxels = width * height * depth;
        data.assign(numVoxels, 0);
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
        if (paddedSize == paddedTextureSize)
            return; // already allocated
        width = paddedTextureSize.x();
        height = paddedTextureSize.y();
        depth = paddedTextureSize.z();
        paddedSize = paddedTextureSize;
        usedSize = usedTextureSize;
        size_t numVoxels = width * height * depth;
        data.assign(numVoxels, 0);
        // qDebug() << "Allocating" << width << height << depth;
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
    // size_t data_size;
    size_t width, height, depth;
    Dimension usedSize;
    Dimension paddedSize;
    // bool bInitialized;
};


class NeuronLabelTexture : public Base3DTexture<uint16_t>
{
public:
    bool loadFromPbdFile(QUrl fileUrl);
};


class NeuronSignalTexture : public Base3DTexture<uint32_t>
{
public:
    bool loadReferenceFromRawFile(QUrl fileUrl);
    bool loadSignalFromRawFile(QUrl fileUrl);
};


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


class ColorMapTexture
{
public:
    ColorMapTexture();
    ColorMapTexture( ColorMapTexture const& rhs );
    virtual ~ColorMapTexture() {}
    const uint32_t* getData() const {return &colors[0];}
    bool update();
    void setDataColorModel(const DataColorModel& cm);

protected:
    const DataColorModel* dataColorModel;
    std::vector<uint32_t> colors;
};

class PrivateVolumeTexture : public QSharedData
{
public:

    typedef unsigned int BGRA; // 32-bit
    typedef BGRA Voxel;

    PrivateVolumeTexture();
    explicit PrivateVolumeTexture(const PrivateVolumeTexture& rhs);
    virtual ~PrivateVolumeTexture();

    void initializeSizes(const NaVolumeData::Reader& volumeReader);
    bool subsampleLabelField(const NaVolumeData::Reader& volumeReader);
    bool subsampleColorField(const NaVolumeData::Reader& volumeReader);
    bool subsampleReferenceField(const NaVolumeData::Reader& volumeReader);
    bool populateVolume(const NaVolumeData::Reader& volumeReader, int zBegin, int zEnd);
    bool loadFast3DTexture(int sx, int sy, int sz, const uint8_t* data); // from fast texture
    bool updateNeuronVisibilityTexture();
    bool updateColorMapTexture();
    void setNeuronSelectionModel(const NeuronSelectionModel& neuronSelectionModel);
    void setDataColorModel(const DataColorModel& dataColorModel);
    bool use3DSignalTexture() const {return bUse3DSignalTexture;}
    // Dimension originalImageSize; ///< Size of data volume being approximated by this texture set.
    // Dimension usedTextureSize; ///< Size of subsection of this texture set containing scaled data volume
    // Dimension paddedTextureSize; ///< Total size of this texture set, including empty padded edges to get desired memory alignment
    const Dimension& getOriginalImageSize() const {return metadata.originalImageSize;}
    const Dimension& getUsedTextureSize() const {return metadata.usedImageSize;}
    const Dimension& getPaddedTextureSize() const {return metadata.paddedImageSize;}
    double subsampleScale; ///< Factor by which a single dimension is subsampled
    // TODO method to load compressed texture directly from file
    const uint32_t* signalData3D() const {return neuronSignalTexture.getData();}
    const uint16_t* labelData3D() const {return neuronLabelTexture.getData();}
    const uint32_t* visibilityData2D() const {return neuronVisibilityTexture.getData();}
    const uint32_t* colorMapData2D() const {return colorMapTexture.getData();}
    const SampledVolumeMetadata& getMetadata() const {return metadata;}
    // void setMetadata(const SampledVolumeMetadata& md) {metadata = md;}
    bool loadLabelPbdFile(QUrl fileUrl)
    {
        return neuronLabelTexture.loadFromPbdFile(fileUrl);
    }
    bool loadSignalRawFile(QUrl fileUrl);
    bool loadReferenceRawFile(QUrl fileUrl) {
        return neuronSignalTexture.loadReferenceFromRawFile(fileUrl);
    }
    void setMetadata(const SampledVolumeMetadata& m) {
        metadata = m;
        subsampleScale = 1.0;
        if (m.originalImageSize.x() != 0.0)
            subsampleScale = m.usedImageSize.x()/m.originalImageSize.x();
    }

protected:
    SampledVolumeMetadata metadata;
    int memoryAlignment; // Keep dimensions a multiple of this factor

    NeuronVisibilityTexture neuronVisibilityTexture;
    NeuronLabelTexture neuronLabelTexture;
    NeuronSignalTexture neuronSignalTexture;
    ColorMapTexture colorMapTexture;
    bool bUse3DSignalTexture;
};

} // namespace jfrc

#endif // PRIVATE_VOLUMETEXTURE_H
