#ifndef VOLUMETEXTURE_H
#define VOLUMETEXTURE_H

#include "DataColorModel.h"
#include "NeuronSelectionModel.h"
#include "NaVolumeData.h"
#include "../DataFlowModel.h"
#include <boost/shared_array.hpp>
#ifdef __APPLE__
#include "OpenGL/gl.h"
#else
#include "GL/gl.h"
#endif
#include <QObject>
#include <vector>

namespace vaa3d {

/// The GLSL shader shader_tex_cmb.txt uses four textures.
/// Each texture uses a different OpenGL texture unit.
/// (though I have read that a single texture unit might be able
///  to have one each of 1D, 2D, 3D textures, I couldn't get it
///  to work that way)
///  1) volume - contains 3D scaled intensity data for up to 4 data channels, in RGBA.
///              Reference intensity is stored in channel 4.
///  2) colormap - 4x256xRGBA 2D texture used to convert data intensities to colors.
///                Fast gamma correction is done with the colormap.
///  3) neuronVisiblity - 256xbyte 1D texture encoding visibility of each neuron fragment.
///  4) neuronLabel - 3D label field containing neuron index at each voxel


/// X,Y,Z dimensions of a volume
class Dimension
{
public:
    enum Index {
        X = 0,
        Y = 1,
        Z = 2
    };

    Dimension();
    Dimension(size_t x, size_t y, size_t z);

    size_t numberOfVoxels() const {return x()*y()*z();}
    const size_t& operator[](size_t ix) const;
    size_t& operator[](size_t ix);
    const size_t& x() const;
    const size_t& y() const;
    const size_t& z() const;
    size_t& x();
    size_t& y();
    size_t& z();
    bool operator!=(const Dimension& rhs) const;
    bool operator==(const Dimension& rhs) const;
    /// Compute a possibly smaller size that would fit in a particular GPU memory limit,
    /// assuming 32 bit pixels.
    Dimension sampledSize(size_t memoryLimit = 0) const;
    /// Expand dimensions, if necessary, to be a multiple of 8
    Dimension padToMultipleOf(unsigned int factor = 8);
    /// Compute next multiple of factor greater than or equal to coord
    static size_t padToMultipleOf(size_t coord, unsigned int factor);

protected:
    size_t data[3];
};


/// Abstract base class for shader texture objects
class ShaderTexture : public QObject
{
public:
    virtual void initializeGL() = 0;
    virtual bool uploadPixels() const = 0;
};


class NeuronLabelTexture : public ShaderTexture
{
    Q_OBJECT

public:
    explicit NeuronLabelTexture()
        : textureID(0)
        , textureUnit(GL_TEXTURE3_ARB) // each texture used by a shader might need a separate texture unit
        , width(0)
        , height(0)
        , depth(0)
    {
        // test data for debugging
        width = height = depth = 8; // 8 works better than 5; it's a multiple of 8
        size_t numVoxels = width * height * depth;
        data.assign(numVoxels, 0);
        for (int i = 0; i < width; ++i)
             for (int j = 0; j < height; ++j)
                 for (int k = 0; k < depth; ++k)
                 {
                     int ix = k * width * height + j * width + i;
                     assert(ix < numVoxels);
                     assert((ix % 48) >= 0); // number of fragments in realLinkTest + 2
                     data[ix] = (ix % 48);
                 }
        // data[5] = 1;
        // data[6] = 2;
        // data[7] = 3;
    }

    virtual void initializeGL()
    {
        // clear stale errors
        GLenum glErr;
        while ((glErr = glGetError()) != GL_NO_ERROR)
            qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
        glActiveTextureARB(textureUnit);
        glEnable(GL_TEXTURE_3D);
        glGenTextures(1, &textureID);
        if ((glErr = glGetError()) != GL_NO_ERROR)
            qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
        else
            bInitialized = true;
        glActiveTextureARB(GL_TEXTURE0_ARB); // restore default
    }

    NeuronLabelTexture& setValueAt(size_t x, size_t y, size_t z, unsigned char neuronIndex)
    {
        size_t offset = x + y * width + z * width * height;
        assert(x >= 0); assert(x < width);
        assert(y >= 0); assert(y < height);
        assert(z >= 0); assert(z < depth);
        assert(offset < width * height * depth);
        data[offset] = neuronIndex;
        return *this;
    }

    NeuronLabelTexture& allocateSize(Dimension paddedTextureSize)
    {
        width = paddedTextureSize.x();
        height = paddedTextureSize.y();
        depth = paddedTextureSize.z();
        size_t numVoxels = width * height * depth;
        if (data.size() != numVoxels)
            data.assign(numVoxels, 0);
        return *this;
    }

    virtual bool uploadPixels() const
    {
        if (data.size() < 1) return false;
        glActiveTextureARB(textureUnit);
        glBindTexture(GL_TEXTURE_3D, textureID);
        // I want off-texture position to be zero - meaning background non-neuron classification
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        // GL_NEAREST ensures that we get an actual non-interpolated label value.
        // Interpolation would be crazy wrong.
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // copy texture from host RAM to GPU device
        glTexImage3D(GL_TEXTURE_3D, // target
                        0, // level
                        GL_INTENSITY8, // texture format
                        width,
                        height,
                        depth,
                        0, // border
                        GL_RED, // image format
                        GL_UNSIGNED_BYTE, // image type
                        &data.front());
        bool bSucceeded = true;
        GLenum glErr;
        if ((glErr = glGetError()) != GL_NO_ERROR)
        {
            qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
            bSucceeded = false;
        }
        glActiveTextureARB(GL_TEXTURE0_ARB); // restore default
        if (bSucceeded)
            emit textureChanged();
        return bSucceeded;
    }

signals:
    void textureChanged() const;

private:
    GLuint textureID;
    GLenum textureUnit;
    std::vector<unsigned char> data;
    size_t width, height, depth;
    bool bInitialized;
};


/// OpenGL texture object for communicating visibility of each neuron fragment to the GLSL shader.
/// 1 x maxNeurons grayscale image with one pixel per neuron fragment.
class NeuronVisibilityTexture : public ShaderTexture
{
    Q_OBJECT
public:
    explicit NeuronVisibilityTexture(int maxNeurons = 256)
        : visibilities(maxNeurons, 255)
        , textureID(0)
        , neuronSelectionModel(NULL)
        , textureUnit(GL_TEXTURE2_ARB)
        , bInitialized(false)
        , bNeedsUpload(true)
    {}

    /// Connect this texture to the NeuronAnnotator unified data model
    void setNeuronSelectionModel(const NeuronSelectionModel& m)
    {
        neuronSelectionModel = &m;
        connect(neuronSelectionModel, SIGNAL(visibilityChanged()),
                this, SLOT(update()));
    }

    /// Actions to be taken once, when the GL context is created
    virtual void initializeGL()
    {
        // clear stale errors
        GLenum glErr;
        while ((glErr = glGetError()) != GL_NO_ERROR)
            qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
        glActiveTextureARB(textureUnit);
        glEnable(GL_TEXTURE_1D);
        glGenTextures(1, &textureID);
        if ((glErr = glGetError()) != GL_NO_ERROR)
            qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
        else
            bInitialized = true;

        glActiveTextureARB(textureUnit);
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, textureID);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // MUST use nearest filter
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // MUST use nearest filter
        // qDebug() << "uploading neuron visibility" << __FILE__ << __LINE__;

        glTexImage1D(GL_TEXTURE_1D, // target
                        0, // level
                        GL_INTENSITY8, // texture format
                        visibilities.size(), // width
                        0, // border
                        GL_RED, // image format
                        GL_UNSIGNED_BYTE, // image type
                        NULL);

        glActiveTextureARB(GL_TEXTURE0_ARB); // restore default
        if ((glErr = glGetError()) != GL_NO_ERROR)
            qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
    }

signals:
    void textureChanged() const; /// Texture was modified.  This might be a good time to update the view.

public slots:
    // Synchronize with list of fragment visibilities in NeuronSelectionModel
    void update()
    {
        if(! neuronSelectionModel) return;
        bool bChanged = false; // For efficiency, keep track of whether the data actually change.
        {
            NeuronSelectionModel::Reader selectionReader(*neuronSelectionModel);
            if (! selectionReader.hasReadLock())
                return;
            // Check each neuron fragment
            const QList<bool>& visList = selectionReader.getMaskStatusList();
            for(int i = 0; i < visList.size(); ++i)
            {
                int ix = i + 1; // position zero is for background signal
                if (ix >= visibilities.size()) break;
                int val = visList[i] ? 255 : 0;
                if (val != visibilities[ix]) {
                    visibilities[ix] = val;
                    bChanged = true;
                }
            }
            // Background signal is found in overlay list, not neuron fragment list.
            const QList<bool>& overlayList = selectionReader.getOverlayStatusList();
            if ( (overlayList.size() > DataFlowModel::BACKGROUND_MIP_INDEX)
                && (visibilities.size() > 0) )
            {
                int val = overlayList[DataFlowModel::BACKGROUND_MIP_INDEX] ? 255 : 0;
                if (val != visibilities[0]) {
                    visibilities[0] = val;
                    bChanged = true;
                }
            }
        } // release read lock
        if (bChanged) {
            bNeedsUpload = true;
            emit textureChanged();
            // uploadPixels();
        }
    }

    virtual bool uploadPixels() const
    {
        if (! bInitialized) {
            qDebug() << "Neuron visibility texture has not been initialized";
            return false;
        }
        GLenum glErr;
        glActiveTextureARB(textureUnit);
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, textureID);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GLSL will replace TexEnv
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // MUST use nearest filter
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // MUST use nearest filter
        // qDebug() << "uploading neuron visibility" << __FILE__ << __LINE__;

        glTexImage1D(GL_TEXTURE_1D, // target
                        0, // level
                        GL_INTENSITY8, // texture format
                        visibilities.size(), // width
                        0, // border
                        GL_RED, // image format
                        GL_UNSIGNED_BYTE, // image type
                        &visibilities.front());

        /*
        glTexSubImage1D(GL_TEXTURE_1D, // target
                        0, // level
                        0, // offset
                        visibilities.size(), // width
                        GL_RED, // image format
                        GL_UNSIGNED_BYTE, // image type
                        &visibilities.front());
                        */

        glActiveTextureARB(GL_TEXTURE0_ARB); // restore default
        if ((glErr = glGetError()) != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
            return false;
        }
        else {
            const_cast<NeuronVisibilityTexture*>(this)->bNeedsUpload = false;
            // emit textureChanged();
            return true;
        }
    }

public:
    bool bNeedsUpload;

protected:

    const NeuronSelectionModel* neuronSelectionModel;
    GLuint textureID;
    GLenum textureUnit;
    std::vector<unsigned char> visibilities;
    bool bInitialized;
};


class VolumeTexture : public QObject
{
    Q_OBJECT
public:

    typedef unsigned int BGRA; // 32-bit
    typedef BGRA Voxel;

    /// One stack of textures, out of three used for basic volume rendering
    class Stack
    {
    public:
        enum StackSet {X, Y, Z};

        Stack()
        {}

        explicit Stack(Dimension sizeParam)
            : size(sizeParam)
        {
            setSize(sizeParam);
        }

        Stack& setSize(Dimension sizeParam)
        {
            size = sizeParam;
            int numVoxels = sizeParam.numberOfVoxels();
            if (data.size() != numVoxels) {
                data.assign(numVoxels, 0);
                initializeGLTextures();
            }
            return *this;
        }

        Stack& setValueAt(size_t x, size_t y, size_t z, Voxel color)
        {
            size_t index = y + z * size.y() + x * size.z() * size.y();
            assert(data.size() > index);
            data[index] = color;
            return *this;
        }
        const Voxel* getSlice(int sliceIx) const {
            return &data[sliceIx * size.z() * size.y()];
        }
        bool initializeGLTextures();
        bool populateGLTextures();
        const Voxel* getDataPtr() const {return &data.front();}
        const GLuint* getTexIDPtr() const {return &textureIDs.front();}

    protected:
        std::vector<Voxel> data;
        Dimension size;
        std::vector<GLuint> textureIDs; ///< openGL texture names from glGenTextures

    private:
        Stack& operator=(const Stack& rhs);
        Stack(const Stack& rhs);
    };


    explicit VolumeTexture(QObject *parent = 0);
    bool populateVolume(const NaVolumeData::Reader& volumeReader,
                        const DataColorModel::Reader& colorReader);
    const Voxel* getDataPtr(Stack::StackSet s) const {
        switch(s) {
        case Stack::X:
            return slicesXyz.getDataPtr();
        case Stack::Y:
            return slicesYzx.getDataPtr();
        case Stack::Z:
            return slicesZxy.getDataPtr();
        }
    }
    /// Access list of OpenGL texture IDs for one of the three texture stacks
    const GLuint* getTexIDPtr(Stack::StackSet s) const
    {
        switch(s) {
        case Stack::X:
            return slicesXyz.getTexIDPtr();
        case Stack::Y:
            return slicesYzx.getTexIDPtr();
        case Stack::Z:
            return slicesZxy.getTexIDPtr();
        }
    }

    Dimension originalImageSize; ///< Size of data volume being approximated by this texture set.
    Dimension usedTextureSize; ///< Size of subsection of this texture set containing scaled data volume
    Dimension paddedTextureSize; ///< Total size of this texture set, including empty padded edges to get desired memory alignment

    // TODO method to load compressed texture directly from file

signals:

public slots:

public:
    // GLuint colorMapTextureID;
    NeuronLabelTexture neuronLabelTexture;

protected:
    size_t memoryLimit; // Try not to use more texture memory than this
    int memoryAlignment; // Keep dimensions a multiple of this factor

    Stack slicesZxy;
    Stack slicesXyz;
    Stack slicesYzx;
};

} // namespace vaa3d

#endif // VOLUMETEXTURE_H
