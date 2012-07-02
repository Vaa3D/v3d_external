#ifndef PRIVATE_VOLUMETEXTURE_H
#define PRIVATE_VOLUMETEXTURE_H

#include <QtGui>
#include "../../3drenderer/GLee_r.h"
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

    explicit Base3DTexture(GLenum textureUnitParam=GL_TEXTURE0_ARB)
        : textureID(0)
        , textureUnit(textureUnitParam) // each texture used by a shader might need a separate texture unit
        , width(8)
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

    virtual unsigned int getTextureId() const {return textureID;}

    virtual bool initializeGL()
    {
        glPushAttrib(GL_TEXTURE_BIT); // remember previous OpenGL state
        {
            glActiveTextureARB(textureUnit);
            glEnable(GL_TEXTURE_3D);
            if (0 == textureID)
                glGenTextures(1, &textureID);
            bInitialized = true;
        }
        glPopAttrib();
        return true;
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

    virtual void allocateSize(Dimension paddedTextureSize)
    {
        width = paddedTextureSize.x();
        height = paddedTextureSize.y();
        depth = paddedTextureSize.z();
        size_t numVoxels = width * height * depth;
        if (data.size() != numVoxels)
            data.assign((size_t)numVoxels, (VoxelType)0);
    }

    virtual bool uploadPixels() const
    {
        if (data.size() < 1) return false;
        GLenum glErr;
        bool bSucceeded = true;
        while ((glErr = glGetError()) != GL_NO_ERROR)
            qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
        // Store active texture unit because glPushAttrib(GL_TEXTURE_BIT);...glPopAttrib(); causes texture binding to be forgotten;
        GLint previousActiveTextureUnit;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTextureUnit);
        {
            glActiveTextureARB(textureUnit);
            glEnable(GL_TEXTURE_3D);
            assert(0 != textureID); // run initializeGL() first
            glBindTexture(GL_TEXTURE_3D, textureID);
            // I want off-texture position to be zero - meaning background non-neuron classification
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            uploadTexture(); // use derived-class-specific method
            GLenum glErr;
            while ((glErr = glGetError()) != GL_NO_ERROR)
            {
                qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
                bSucceeded = false;
            }
        }
        glActiveTextureARB(previousActiveTextureUnit); // restore default
        return bSucceeded;
    }

protected:
    virtual bool uploadTexture() const = 0;

    GLuint textureID;
    GLenum textureUnit;
    std::vector<VoxelType> data;
    size_t width, height, depth;
    bool bInitialized;
};


class NeuronLabelTexture : public Base3DTexture<uint16_t>
{
private:
    typedef Base3DTexture<uint16_t> super;

public:
    NeuronLabelTexture();

protected:
    virtual bool uploadTexture() const;
};


class NeuronSignalTexture : public Base3DTexture<uint32_t>
{
protected:
    virtual bool uploadTexture() const;
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
    /// Actions to be taken once, when the GL context is created
    virtual bool initializeGL();
    bool update();
    virtual bool uploadPixels() const;

    bool bNeedsUpload;

protected:
    const NeuronSelectionModel* neuronSelectionModel;
    GLuint textureID;
    GLenum textureUnit;
    std::vector<unsigned int> visibilities;
    bool bInitialized;
};


class PrivateVolumeTexture : public QSharedData
{
public:

    typedef unsigned int BGRA; // 32-bit
    typedef BGRA Voxel;

    /// One stack of textures, out of three used for basic volume rendering
    class Stack
    {
    public:
        enum StackSet {X, Y, Z};

        Stack();
        explicit Stack(const Stack& rhs);
        explicit Stack(Dimension sizeParam);
        virtual ~Stack();
        bool initializeGL();
        Stack& setSize(Dimension sizeParam);
        Stack& setValueAt(size_t x, size_t y, size_t z, Voxel color);
        const Voxel* getSlice(int sliceIx) const;
        bool populateGLTextures() const;
        const Voxel* getDataPtr() const;
        const GLuint* getTexIDPtr() const;

    protected:
        std::vector<Voxel> data;
        Dimension size;
        std::vector<GLuint> textureIDs; ///< openGL texture names from glGenTextures
        bool bHasTextureIDs;

    private:
        Stack& operator=(const Stack& rhs);
    };


    PrivateVolumeTexture();
    explicit PrivateVolumeTexture(const PrivateVolumeTexture& rhs);

    void initializeSizes(const NaVolumeData::Reader& volumeReader);
    bool populateVolume(const NaVolumeData::Reader& volumeReader, int zBegin, int zEnd);
    bool uploadVolumeTexturesToVideoCardGL() const;
    bool uploadNeuronVisibilityTextureToVideoCardGL() const;
    bool uploadColorMapTextureToVideoCardGL() const;
    bool updateNeuronVisibilityTexture();
    void setNeuronSelectionModel(const NeuronSelectionModel& neuronSelectionModel);
    const Voxel* getDataPtr(Stack::StackSet s) const;
    /// Access list of OpenGL texture IDs for one of the three texture stacks
    const GLuint* getTexIDPtr(Stack::StackSet s) const;
    bool initializeGL();
    bool use3DSignalTexture() const {return bUse3DSignalTexture;}
    unsigned int signal3DTextureId() const {return neuronSignalTexture.getTextureId();}

    Dimension originalImageSize; ///< Size of data volume being approximated by this texture set.
    Dimension usedTextureSize; ///< Size of subsection of this texture set containing scaled data volume
    Dimension paddedTextureSize; ///< Total size of this texture set, including empty padded edges to get desired memory alignment
    double subsampleScale; ///< Factor by which a single dimension is subsampled

    // TODO method to load compressed texture directly from file

protected:
    size_t memoryLimit; // Try not to use more texture memory than this
    int memoryAlignment; // Keep dimensions a multiple of this factor

    Stack slicesZxy;
    Stack slicesXyz;
    Stack slicesYzx;
    NeuronVisibilityTexture neuronVisibilityTexture;
    NeuronLabelTexture neuronLabelTexture;
    NeuronSignalTexture neuronSignalTexture;
    bool bUse3DSignalTexture;
};

} // namespace jfrc

#endif // PRIVATE_VOLUMETEXTURE_H
