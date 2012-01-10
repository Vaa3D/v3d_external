#ifndef PRIVATE_VOLUMETEXTURE_H
#define PRIVATE_VOLUMETEXTURE_H

#include "DataColorModel.h"
#include "NeuronSelectionModel.h"
#include "NaVolumeData.h"
#include "Dimension.h"
#include "../DataFlowModel.h"
#include "../../3drenderer/GLee_r.h"
#include <QObject>
#include <vector>
#include <cassert>

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
///  3) neuronVisiblity - 256xbyte 1D texture encoding visibility of each neuron fragment.
///  4) neuronLabel - 3D label field containing neuron index at each voxel


class NeuronLabelTexture
{

public:
    NeuronLabelTexture()
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
        return bSucceeded;
    }

private:
    GLuint textureID;
    GLenum textureUnit;
    std::vector<unsigned char> data;
    size_t width, height, depth;
    bool bInitialized;
};


/// OpenGL texture object for communicating visibility of each neuron fragment to the GLSL shader.
/// 1 x maxNeurons grayscale image with one pixel per neuron fragment.
class NeuronVisibilityTexture
{
public:
    explicit NeuronVisibilityTexture(int maxNeurons = 256)
        : visibilities(maxNeurons, 0x000000ff) // visible, neither selected nor highlighted
        , textureID(0)
        , neuronSelectionModel(NULL)
        , textureUnit(GL_TEXTURE2_ARB)
        , bInitialized(false)
        , bNeedsUpload(true)
    {}

    /// Connect this texture to the NeuronAnnotator unified data model
    void setNeuronSelectionModel(const NeuronSelectionModel& m)
    {
        if (neuronSelectionModel == &m) return; // no change
        neuronSelectionModel = &m;
        update();
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
                        GL_RGBA, // texture format
                        visibilities.size(), // width
                        0, // border
                        GL_RGBA, // image format
                        GL_UNSIGNED_BYTE, // image type
                        NULL);

        glActiveTextureARB(GL_TEXTURE0_ARB); // restore default
        if ((glErr = glGetError()) != GL_NO_ERROR)
            qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
    }

    bool update()
    {
        if(! neuronSelectionModel) return false;
        bool bChanged = false; // For efficiency, keep track of whether the data actually change.
        {
            NeuronSelectionModel::Reader selectionReader(*neuronSelectionModel);
            if (! selectionReader.hasReadLock())
                return false;
            // Check each neuron fragment
            const QList<bool>& visList = selectionReader.getMaskStatusList();
            const QList<bool>& selList = selectionReader.getNeuronSelectList();
            assert(visList.size() == selList.size());
            for(int i = 0; i < visList.size(); ++i)
            {
                int ix = i + 1; // position zero is for background signal
                if (ix >= visibilities.size()) break;
                // Visiblity
                unsigned int val;
                if (visList[i]) { // visible
                    val = visibilities[ix] | 0x000000ff; // 0xAABBGGRR, turn on red/visibility
                }
                else { // not visible
                    val = visibilities[ix] & 0xffffff00; // 0xAABBGGRR, turn off red/visibility
                }
                if (val != visibilities[ix]) {
                    visibilities[ix] = val;
                    bChanged = true;
                }
                // Selection
                if (selList[i]) { // selected
                    val = visibilities[ix] | 0x0000ff00; // 0xAABBGGRR, turn on green/selection
                }
                else { // not selected
                    val = visibilities[ix] & 0xffff00ff; // 0xAABBGGRR, turn off green/selection
                }
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
                int val;
                if (overlayList[DataFlowModel::BACKGROUND_MIP_INDEX])
                    val = 0x000000ff; // visible, never selected nor highlighted
                else
                    val = 0x00000000; // not visible
                if (val != visibilities[0]) {
                    visibilities[0] = val;
                    bChanged = true;
                }
            }
        } // release read lock
        if (bChanged) {
            bNeedsUpload = true;
        }
        return bChanged;
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
                        GL_RGBA, // texture format
                        visibilities.size(), // width
                        0, // border
                        GL_RGBA, // image format
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

        Stack()
            : bHasTextureIDs(false)
        {}

        bool initializeGL()
        {
            // Allocate texture names from OpenGL
            if (textureIDs.size() != size.x())
            {
                if (bHasTextureIDs && (textureIDs.size() > 0))
                    glDeleteTextures(textureIDs.size(), &textureIDs[0]);
                textureIDs.assign(size.x(), 0);
                bHasTextureIDs = false;
            }
            if (! bHasTextureIDs) {
                glGenTextures(textureIDs.size(), &textureIDs[0]);
            }
            // Check for GL errors
            {
                GLenum err = glGetError();
                if (err != GL_NO_ERROR) {
                    qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
                    return false;
                }
            }
        }

        explicit Stack(const Stack& rhs)
            : data(rhs.data)
            , size(rhs.size)
            , textureIDs(rhs.textureIDs)
            , bHasTextureIDs(false)
        {
            qDebug() << "Stack data being copied.  This is probably not a good idea...";
        }

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
                // initializeGLTextures();
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
        bool populateGLTextures() const;
        const Voxel* getDataPtr() const {return &data.front();}
        const GLuint* getTexIDPtr() const {return &textureIDs.front();}

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
    bool uploadNeuronVisibilityTextureToVideoCardGL() const {
        return neuronVisibilityTexture.uploadPixels();
    }
    bool uploadColorMapTextureToVideoCardGL() const {
        qDebug() << "ColorMap not yet incorporated into VolumeTexture" << __FILE__ << __LINE__;
        return true;
    }
    bool updateNeuronVisibilityTexture()
    {
        return neuronVisibilityTexture.update();
    }

    void setNeuronSelectionModel(const NeuronSelectionModel& neuronSelectionModel);
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

    bool initializeGL()
    {
        neuronVisibilityTexture.initializeGL();
        neuronLabelTexture.initializeGL();
        slicesXyz.initializeGL();
        slicesYzx.initializeGL();
        slicesZxy.initializeGL();
    }

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
};

} // namespace jfrc

#endif // PRIVATE_VOLUMETEXTURE_H
