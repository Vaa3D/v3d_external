#include "PrivateVolumeTexture.h"
#include "DataColorModel.h"
#include <QColor>
#include <cassert>


namespace jfrc {


struct LabelSampler
{
    void loop(size_t* dims);
};

////////////////////////
// NeuronLabelTexture //
////////////////////////

NeuronLabelTexture::NeuronLabelTexture()
    : super(GL_TEXTURE3_ARB)
{}

/* virtual */
bool NeuronLabelTexture::uploadTexture() const
{
    // GL_NEAREST ensures that we get an actual non-interpolated label value.
    // Interpolation would be crazy wrong.
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // copy texture from host RAM to GPU device
    glTexImage3D(GL_TEXTURE_3D, // target
                 0, // level
                 GL_INTENSITY16, // texture format
                 (GLsizei)width,
                 (GLsizei)height,
                 (GLsizei)depth,
                 0, // border
                 GL_RED, // image format
                 GL_UNSIGNED_SHORT, // image type
                 &data.front());
}


/////////////////////////
// NeuronSignalTexture //
/////////////////////////

/* virtual */
bool NeuronSignalTexture::uploadTexture() const
{
    // GL_NEAREST ensures that we get an actual non-interpolated label value.
    // Interpolation would be crazy wrong.
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // copy texture from host RAM to GPU device
    glTexImage3D(GL_TEXTURE_3D, // target
                 0, // level
                 GL_RGBA8, // texture format
                 (GLsizei)width,
                 (GLsizei)height,
                 (GLsizei)depth,
                 0, // border
                 GL_BGRA, // image format
                 GL_UNSIGNED_INT_8_8_8_8_REV, // image type
                 &data.front());
}


/////////////////////////////
// NeuronVisibilityTexture //
/////////////////////////////

/* explicit */
NeuronVisibilityTexture::NeuronVisibilityTexture(int maxNeurons)
    : visibilities(maxNeurons, 0x000000ff) // visible, neither selected nor highlighted
    , textureID(0)
    , neuronSelectionModel(NULL)
    , textureUnit(GL_TEXTURE2_ARB)
    , bInitialized(false)
    , bNeedsUpload(true)
{}

/// Connect this texture to the NeuronAnnotator unified data model
void NeuronVisibilityTexture::setNeuronSelectionModel(const NeuronSelectionModel& m)
{
    if (neuronSelectionModel == &m) return; // no change
    neuronSelectionModel = &m;
    update();
}

/// Actions to be taken once, when the GL context is created
/* virtual */
bool NeuronVisibilityTexture::initializeGL()
{
    // clear stale errors
    GLenum glErr;
    while ((glErr = glGetError()) != GL_NO_ERROR)
        qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
    glActiveTextureARB(textureUnit);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &textureID);
    if ((glErr = glGetError()) != GL_NO_ERROR)
        qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
    else
        bInitialized = true;

    glActiveTextureARB(textureUnit);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // MUST use nearest filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // MUST use nearest filter
    // qDebug() << "uploading neuron visibility" << __FILE__ << __LINE__;

    glTexImage2D(GL_TEXTURE_2D, // target
                    0, // level
                    GL_RGBA, // texture format
                    256, // width
                    256, // height
                    0, // border
                    GL_RGBA, // image format
                    GL_UNSIGNED_BYTE, // image type
                    NULL);

    glActiveTextureARB(GL_TEXTURE0_ARB); // restore default
    if ((glErr = glGetError()) != GL_NO_ERROR)
        qDebug() << "OpenGL error" << glErr << __FILE__ << __LINE__;
    return true;
}

bool NeuronVisibilityTexture::update()
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


/* virtual */
bool NeuronVisibilityTexture::uploadPixels() const
{
    if (! bInitialized) {
        qDebug() << "Neuron visibility texture has not been initialized";
        return false;
    }
    GLenum glErr;
    glActiveTextureARB(textureUnit);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GLSL will replace TexEnv
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // MUST use nearest filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // MUST use nearest filter
    // qDebug() << "uploading neuron visibility" << __FILE__ << __LINE__;

    glTexImage2D(GL_TEXTURE_2D, // target
                    0, // level
                    GL_RGBA, // texture format
                    256, // width
                    256, // height
                    0, // border
                    GL_RGBA, // image format
                    GL_UNSIGNED_BYTE, // image type
                    &visibilities.front());

    /*
    glTexSubImage2D(GL_TEXTURE_2D, // target
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


//////////////////////////
// PrivateVolumeTexture //
//////////////////////////

PrivateVolumeTexture::PrivateVolumeTexture()
    : memoryLimit(5e8) // 500 MB for volume texture
    , memoryAlignment(8)
    , subsampleScale(1.0)
    , bUse3DSignalTexture(true)
{
}

/* explicit */
PrivateVolumeTexture::PrivateVolumeTexture(const PrivateVolumeTexture& rhs)
    : memoryLimit(rhs.memoryLimit)
    , memoryAlignment(rhs.memoryAlignment)
    , subsampleScale(rhs.subsampleScale)
    , originalImageSize(rhs.originalImageSize)
    , usedTextureSize(rhs.usedTextureSize)
    , paddedTextureSize(rhs.paddedTextureSize)
    , neuronLabelTexture(rhs.neuronLabelTexture)
    , neuronVisibilityTexture(rhs.neuronVisibilityTexture)
    , slicesXyz(rhs.slicesXyz)
    , slicesYzx(rhs.slicesYzx)
    , slicesZxy(rhs.slicesZxy)
    , neuronSignalTexture(rhs.neuronSignalTexture)
    , bUse3DSignalTexture(rhs.bUse3DSignalTexture)
{
}

void PrivateVolumeTexture::setNeuronSelectionModel(const NeuronSelectionModel& neuronSelectionModel)
{
    neuronVisibilityTexture.setNeuronSelectionModel(neuronSelectionModel);
}

void PrivateVolumeTexture::initializeSizes(const NaVolumeData::Reader& volumeReader)
{
    const Image4DProxy<My4DImage>& imageProxy = volumeReader.getOriginalImageProxy();
    Dimension inputSize(imageProxy.sx, imageProxy.sy, imageProxy.sz);
    if (inputSize != originalImageSize) // new/changed volume size
    {
        originalImageSize = inputSize;
        subsampleScale = inputSize.computeLinearSubsampleScale(memoryLimit/3);
        usedTextureSize = inputSize.sampledSize(memoryLimit/3);
        paddedTextureSize = usedTextureSize.padToMultipleOf(memoryAlignment);
        slicesXyz.setSize(Dimension(paddedTextureSize.x(), paddedTextureSize.y(), paddedTextureSize.z()));
        slicesYzx.setSize(Dimension(paddedTextureSize.y(), paddedTextureSize.x(), paddedTextureSize.z()));
        slicesZxy.setSize(Dimension(paddedTextureSize.z(), paddedTextureSize.x(), paddedTextureSize.y()));
        neuronLabelTexture.allocateSize(paddedTextureSize);
        neuronSignalTexture.allocateSize(paddedTextureSize);
    }
}

// Create host texture memory for data volume
// TODO fill 3D texture
bool PrivateVolumeTexture::populateVolume(const NaVolumeData::Reader& volumeReader, int zBegin, int zEnd)
{
    // qDebug() << "Populating volume textures for 3D renderer..." << __FILE__ << __LINE__;
    QTime stopwatch;
    stopwatch.start();
    const Image4DProxy<My4DImage>& imageProxy = volumeReader.getOriginalImageProxy();
    const Image4DProxy<My4DImage>& referenceProxy = volumeReader.getReferenceImageProxy();
    const Image4DProxy<My4DImage>& labelProxy = volumeReader.getNeuronMaskProxy();
    initializeSizes(volumeReader);
    // Scale RGBA channel colors to actual data range of input
    // (Final coloring will be handled in the shader, so don't use colorReader.blend())
    // Precompute coefficients for scaling.
    int minData[4] = {0, 0, 0, 0}; // minimum data value of channel
    double rangeData[4] = {1.0, 1.0, 1.0, 1.0}; // 255.0 divided by data range of channel
    size_t refIx = imageProxy.sc; // index of reference channel
    assert(imageProxy.sc <= 3); // that's what I'm assuming for now...
    for (int c = 0; c < imageProxy.sc; ++c)
    {
        minData[c] = imageProxy.vmin[c];
        rangeData[c] = 255.0 / (imageProxy.vmax[c] - imageProxy.vmin[c]);
        // elide use of colorReader
        // minData[c] = colorReader.getChannelDataMin(c);
        // rangeData[c] = 255.0 / (colorReader.getChannelDataMax(c) - colorReader.getChannelDataMin(c));
    }
    if (volumeReader.hasReferenceImage()) {
        minData[refIx] = referenceProxy.vmin[0];
        rangeData[refIx] = 255.0 / (referenceProxy.vmax[0] - referenceProxy.vmin[0]);
    }

    // Use stupid box filter for now.  Once that's working, use Lanczos for better sampling.
    // TODO
    double xScale = (double)originalImageSize.x() / (double)usedTextureSize.x();
    double yScale = (double)originalImageSize.y() / (double)usedTextureSize.y();
    double zScale = (double)originalImageSize.z() / (double)usedTextureSize.z();
    // qDebug() << "x, y, z Scale =" << xScale << yScale << zScale << __FILE__ << __LINE__;
    std::vector<double> channelIntensities(imageProxy.sc + 1, 0.0); // For colorReader::blend() interface; +1 for reference channel
    if (zEnd < 0) // -1 means actual final z
        zEnd = (int)usedTextureSize.z();
    if (zEnd > usedTextureSize.z())
        zEnd = (int)usedTextureSize.z();
    int channelCount = imageProxy.sc;
    if (volumeReader.hasReferenceImage())
        channelCount += 1;
    for(int z = zBegin; z < zEnd; ++z)
    {
        // qDebug() << z << __FILE__ << __LINE__;
        int z0 = (int)(z * zScale + 0.49);
        int z1 = (int)((z + 1) * zScale + 0.49);
        for(int y = 0; y < usedTextureSize.y(); ++y)
        {
            int y0 = (int)(y * yScale + 0.49);
            int y1 = (int)((y + 1) * yScale + 0.49);
            for(int x = 0; x < usedTextureSize.x(); ++x)
            {
                int x0 = (int)(x * xScale + 0.49);
                int x1 = (int)((x + 1) * xScale + 0.49);
                float weight = 0.0;
                // Choose exactly one neuron index for this voxel.  Default to zero (background),
                // but accept any non-background value in its place.
                int neuronIndex = 0;
                // Average over multiple voxels in input image
                channelIntensities.assign(channelCount, 0.0);
                for(int sx = x0; sx < x1; ++sx)
                    for(int sy = y0; sy < y1; ++sy)
                        for(int sz = z0; sz < z1; ++sz)
                        {
                            for (int c = 0; c < imageProxy.sc; ++c)
                                channelIntensities[c] += imageProxy.value_at(sx, sy, sz, c);
                            if (volumeReader.hasReferenceImage())
                                channelIntensities[refIx] += referenceProxy.value_at(sx, sy, sz, 0);
                            if (volumeReader.hasNeuronMask()) {
                                if (neuronIndex == 0) // take first non-zero value
                                    neuronIndex = (int)labelProxy.value_at(sx, sy, sz, 0);
                            }
                            weight += 1.0;
                        }
                for (int c = 0; c < channelCount; ++c) // Normalize
                    if (weight > 0)
                        channelIntensities[c]  = ((channelIntensities[c] / weight) - minData[c]) * rangeData[c];
                // Swap red and blue from RGBA to BGRA, for Windows texture efficiency
                // Create unsigned int with #AARRGGBB pattern
                BGRA color = 0;
                color |= (((int)channelIntensities[2]));       // #000000BB blue  : channel 3
                color |= (((int)channelIntensities[1]) << 8);  // #0000GGBB green : channel 2
                color |= (((int)channelIntensities[0]) << 16); // #00RRGGBB red   : channel 1
                color |= (((int)channelIntensities[3]) << 24); // #AARRGGBB white : reference
                neuronLabelTexture.setValueAt(x, y, z, neuronIndex);
                slicesXyz.setValueAt(x, y, z, color);
                slicesYzx.setValueAt(y, x, z, color);
                slicesZxy.setValueAt(z, x, y, color);
                neuronSignalTexture.setValueAt(x, y, z, color);
            }
        }
    }
    // qDebug() << "Sampling 3D volume for 3D viewer took" << stopwatch.elapsed() / 1000.0 << "seconds";
    return true;
}

bool PrivateVolumeTexture::uploadVolumeTexturesToVideoCardGL() const
{
    if (bUse3DSignalTexture) {
        if (! neuronSignalTexture.uploadPixels())
            return false;
    }
    else
    {
        if (! slicesZxy.populateGLTextures()) {
            return false;
        }
        if (! slicesYzx.populateGLTextures()) {
            return false;
        }
        if (! slicesXyz.populateGLTextures()) {
            return false;
        }
    }
    if (! neuronLabelTexture.uploadPixels()) {
        return false;
    }
    return true;
}

bool PrivateVolumeTexture::uploadNeuronVisibilityTextureToVideoCardGL() const
{
    return neuronVisibilityTexture.uploadPixels();
}

bool PrivateVolumeTexture::uploadColorMapTextureToVideoCardGL() const
{
    qDebug() << "ColorMap not yet incorporated into VolumeTexture" << __FILE__ << __LINE__;
    return true;
}

bool PrivateVolumeTexture::updateNeuronVisibilityTexture()
{
    return neuronVisibilityTexture.update();
}

const PrivateVolumeTexture::Voxel* PrivateVolumeTexture::getDataPtr(Stack::StackSet s) const
{
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
const GLuint* PrivateVolumeTexture::getTexIDPtr(Stack::StackSet s) const
{
    switch(s) {
    case Stack::X:
        return slicesXyz.getTexIDPtr();
    case Stack::Y:
        return slicesYzx.getTexIDPtr();
    case Stack::Z:
        return slicesZxy.getTexIDPtr();
    }
    return slicesXyz.getTexIDPtr();
}

bool PrivateVolumeTexture::initializeGL() const
{
    PrivateVolumeTexture& mutableThis =
            const_cast<PrivateVolumeTexture&>(*this);
    bool result = true;
    if (!mutableThis.neuronVisibilityTexture.initializeGL())
        result = false;
    if (!mutableThis.neuronLabelTexture.initializeGL())
        result = false;
    if (bUse3DSignalTexture) {
        if (! mutableThis.neuronSignalTexture.initializeGL())
            result = false;
    }
    else
    {
        if (!mutableThis.slicesXyz.initializeGL())
            result = false;
        if (!mutableThis.slicesYzx.initializeGL())
            result = false;
        if (!mutableThis.slicesZxy.initializeGL())
            result = false;
    }
    return result;
}


/////////////////////////////////////////
// PrivateVolumeTexture::Stack methods //
/////////////////////////////////////////

PrivateVolumeTexture::Stack::Stack()
    : bHasTextureIDs(false)
{}

/* explicit */
PrivateVolumeTexture::Stack::Stack(const Stack& rhs)
    : data(rhs.data)
    , size(rhs.size)
    , textureIDs(rhs.textureIDs)
    , bHasTextureIDs(false)
{
    qDebug() << "Stack data being copied.  This is probably not a good idea...";
}

/* explicit */
PrivateVolumeTexture::Stack::Stack(Dimension sizeParam)
    : size(sizeParam)
{
    setSize(sizeParam);
}

/* virtual */
PrivateVolumeTexture::Stack::~Stack()
{
    if (textureIDs.size() > 0) {
        glDeleteTextures((GLsizei)textureIDs.size(), &textureIDs.front());
        textureIDs.clear();
    }
}

bool PrivateVolumeTexture::Stack::initializeGL()
{
    if (size.x() < 1) // Can't initialize that!
        return false;
    // Allocate texture names from OpenGL
    if (textureIDs.size() != size.x())
    {
        if (bHasTextureIDs && (textureIDs.size() > 0))
            glDeleteTextures((GLsizei)textureIDs.size(), &textureIDs[0]);
        textureIDs.assign((size_t)size.x(), (GLuint)0);
        bHasTextureIDs = false;
    }
    if (! bHasTextureIDs) {
        glGenTextures((GLsizei)textureIDs.size(), &textureIDs[0]);
    }
    // Check for GL errors
    {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            return false;
        }
    }
                return true;
}

PrivateVolumeTexture::Stack& PrivateVolumeTexture::Stack::setSize(Dimension sizeParam)
{
    if (size == sizeParam)
        return *this; // no change
    size = sizeParam;
    size_t numVoxels = sizeParam.numberOfVoxels();
    if (data.size() != numVoxels) {
        // explicit cast to avoid iterator interpretation in MSVC
        data.assign(numVoxels, (PrivateVolumeTexture::Voxel)0);
        // initializeGL();
    }
    return *this;
}

PrivateVolumeTexture::Stack&
PrivateVolumeTexture::Stack::setValueAt(size_t x, size_t y, size_t z, PrivateVolumeTexture::Voxel color)
{
    size_t index = y + z * size.y() + x * size.z() * size.y();
    assert(data.size() > index);
    data[index] = color;
    return *this;
}

const PrivateVolumeTexture::Voxel* PrivateVolumeTexture::Stack::getSlice(int sliceIx) const
{
    return &data[sliceIx * size.z() * size.y()];
}

const PrivateVolumeTexture::Voxel* PrivateVolumeTexture::Stack::getDataPtr() const
{
    return &data.front();
}

const GLuint* PrivateVolumeTexture::Stack::getTexIDPtr() const
{
    return &textureIDs.front();
}

// Run once to create (but not populate/update) opengl textures
bool PrivateVolumeTexture::Stack::populateGLTextures() const
{
    QTime stopwatch;
    stopwatch.start();
    // qDebug() << "PrivateVolumeTexture::Stack::populateGLTextures()" << __FILE__ << __LINE__;
    size_t numberOfSlices = size.x();
    GLsizei width = (GLsizei)size.y();
    GLsizei height = (GLsizei)size.z();

    // Double check that we have the right number of texture names
    if (textureIDs.size() != (numberOfSlices + 1))
    {
        if (textureIDs.size() > 0) {
            glDeleteTextures((GLsizei)textureIDs.size(), &textureIDs.front());
            // textureIDs.clear(); // non const method
        }
        const_cast<std::vector<GLuint>& >(textureIDs).assign(numberOfSlices + 1, 0);
        glGenTextures((GLsizei)(numberOfSlices + 1), const_cast<GLuint*>(&textureIDs.front()));
        // Check for GL errors
        {
            GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
                return false;
            }
        }
    }

    for (int i = 0; i < numberOfSlices; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textureIDs[i + 1]);
        // Check for GL errors
        {
            GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
                return false;
            }
        }

        const int border = 0;
        glTexImage2D(GL_TEXTURE_2D,
            0, ///< mipmap level; zero means base level
            GL_RGBA8, ///< texture format, in bytes per pixel
            width,
            height,
            border,
            GL_BGRA, // image format
            GL_UNSIGNED_INT_8_8_8_8_REV, // image type
            getSlice(i)); ///< NULL means initialize but don't populate

        /*
        glTexSubImage2D(GL_TEXTURE_2D, // target
                0, // level
                0,0,  // offset
                width, // sub width
                height, // sub height
                GL_BGRA, // image format
                GL_UNSIGNED_INT_8_8_8_8_REV, // image type
                slices[i][0]);
         */

        // Check for GL errors
        {
            GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
                return false;
            }
        }
    }
    // qDebug() << width << height;
    qDebug() << "Populate textures took" << stopwatch.elapsed() / 1000.0 << "seconds";
    return true;
}


} // namespace jfrc

