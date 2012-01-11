#include "PrivateVolumeTexture.h"
#include "DataColorModel.h"
#include <QColor>
#include <cassert>


namespace jfrc {


///////////////////////////
// PrivateVolumeTexture methods //
///////////////////////////

PrivateVolumeTexture::PrivateVolumeTexture()
    : memoryLimit(5e8) // 500 MB for volume texture
    , memoryAlignment(8)
    , subsampleScale(1.0)
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
    }
}

// Create host texture memory for data volume
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
    minData[refIx] = referenceProxy.vmin[0];
    rangeData[refIx] = 255.0 / (referenceProxy.vmax[0] / REF_CHANNEL_DIMNESS_FACTOR - referenceProxy.vmin[0]);

    // Use stupid box filter for now.  Once that's working, use Lanczos for better sampling.
    // TODO
    double xScale = (double)originalImageSize.x() / (double)usedTextureSize.x();
    double yScale = (double)originalImageSize.y() / (double)usedTextureSize.y();
    double zScale = (double)originalImageSize.z() / (double)usedTextureSize.z();
    // qDebug() << "x, y, z Scale =" << xScale << yScale << zScale << __FILE__ << __LINE__;
    std::vector<double> channelIntensities(imageProxy.sc + 1, 0.0); // For colorReader::blend() interface; +1 for reference channel
    if (zEnd < 0) // -1 means actual final z
        zEnd = usedTextureSize.z();
    if (zEnd > usedTextureSize.z())
        zEnd = usedTextureSize.z();
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
                channelIntensities.assign(refIx + 1, 0.0);
                for(int sx = x0; sx < x1; ++sx)
                    for(int sy = y0; sy < y1; ++sy)
                        for(int sz = z0; sz < z1; ++sz)
                        {
                            for (int c = 0; c < imageProxy.sc; ++c)
                                channelIntensities[c] += imageProxy.value_at(sx, sy, sz, c);
                            channelIntensities[refIx] += referenceProxy.value_at(sx, sy, sz, 0);
                            if (neuronIndex == 0) // take first non-zero value
                                neuronIndex = (int)labelProxy.value_at(sx, sy, sz, 0);
                            weight += 1.0;
                        }
                for (int c = 0; c <= refIx; ++c) // Normalize
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
            }
        }
    }
    // qDebug() << "Sampling 3D volume for 3D viewer took" << stopwatch.elapsed() / 1000.0 << "seconds";
    return true;
}

bool PrivateVolumeTexture::uploadVolumeTexturesToVideoCardGL() const
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
    if (! neuronLabelTexture.uploadPixels()) {
        return false;
    }
    return true;
}


/////////////////////////////////////////
// PrivateVolumeTexture::Stack methods //
/////////////////////////////////////////

// Run once to create (but not populate/update) opengl textures

bool PrivateVolumeTexture::Stack::populateGLTextures() const
{
    QTime stopwatch;
    stopwatch.start();
    // qDebug() << "PrivateVolumeTexture::Stack::populateGLTextures()" << __FILE__ << __LINE__;
    int numberOfSlices = size.x();
    int width = size.y();
    int height = size.z();

    // Double check that we have the right number of texture names
    if (textureIDs.size() != (numberOfSlices + 1))
    {
        if (textureIDs.size() > 0) {
            glDeleteTextures(textureIDs.size(), &textureIDs.front());
            // textureIDs.clear();
        }
        const_cast<std::vector<GLuint>& >(textureIDs).assign(numberOfSlices + 1, 0);
        glGenTextures(numberOfSlices + 1, const_cast<GLuint*>(&textureIDs.front()));
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

