#include "VolumeTexture.h"
#include "DataColorModel.h"
#include <QColor>
#include <cassert>


namespace vaa3d {

///////////////////////////
// VolumeTexture methods //
///////////////////////////

VolumeTexture::VolumeTexture(QObject *parent)
    : QObject(parent)
    , memoryLimit(1e9)
    , memoryAlignment(8)
{
}

// Create host texture memory for data volume
bool VolumeTexture::populateVolume(const NaVolumeData::Reader& volumeReader,
                                   const DataColorModel::Reader& colorReader)
{
    QTime stopwatch;
    stopwatch.start();
    const Image4DProxy<My4DImage>& imageProxy = volumeReader.getOriginalImageProxy();
    const Image4DProxy<My4DImage>& referenceProxy = volumeReader.getReferenceImageProxy();
    const Image4DProxy<My4DImage>& labelProxy = volumeReader.getNeuronMaskProxy();
    Dimension inputSize(imageProxy.sx, imageProxy.sy, imageProxy.sz);
    /// If originalImageSize is unchanged, assume all size parameters are already correct.
    if (inputSize != originalImageSize) // new/changed volume size
    {
        originalImageSize = inputSize;
        usedTextureSize = inputSize.sampledSize(memoryLimit/3);
        paddedTextureSize = usedTextureSize.padToMultipleOf(memoryAlignment);
        slicesXyz.setSize(Dimension(paddedTextureSize.x(), paddedTextureSize.y(), paddedTextureSize.z()));
        slicesYzx.setSize(Dimension(paddedTextureSize.y(), paddedTextureSize.x(), paddedTextureSize.z()));
        slicesZxy.setSize(Dimension(paddedTextureSize.z(), paddedTextureSize.x(), paddedTextureSize.y()));
        neuronLabelTexture.allocateSize(paddedTextureSize);
    }

    // Scale RGBA channel colors to actual data range of input
    // (Final coloring will be handled in the shader, so don't use colorReader.blend())
    // Precompute coefficients for scaling.
    int minData[4] = {0, 0, 0, 0}; // minimum data value of channel
    double rangeData[4] = {1.0, 1.0, 1.0, 1.0}; // 255.0 divided by data range of channel
    for (int c = 0; c < colorReader.getNumberOfDataChannels(); ++c)
    {
        minData[c] = colorReader.getChannelDataMin(c);
        rangeData[c] = 255.0 / (colorReader.getChannelDataMax(c) - colorReader.getChannelDataMin(c));
    }
    // Use stupid box filter for now.  Once that's working, use Lanczos for better sampling.
    // TODO
    double xScale = (double)originalImageSize.x() / (double)usedTextureSize.x();
    double yScale = (double)originalImageSize.y() / (double)usedTextureSize.y();
    double zScale = (double)originalImageSize.z() / (double)usedTextureSize.z();
    // qDebug() << "x, y, z Scale =" << xScale << yScale << zScale << __FILE__ << __LINE__;
    std::vector<double> channelIntensities(imageProxy.sc + 1, 0.0); // For colorReader::blend() interface; +1 for reference channel
    size_t refIx = imageProxy.sc; // index of reference channel
    for(int z = 0; z < usedTextureSize.z(); ++z)
    {
        // if (! z%10)
            // qDebug() << "VolumeTexture::populateVolume()" << z;
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
                                channelIntensities[c] += imageProxy.value_at(x, y, z, c);
                            channelIntensities[refIx] += referenceProxy.value_at(x, y, z, 0);
                            if (neuronIndex == 0) // take first non-zero value
                                neuronIndex = (int)labelProxy.value_at(x, y, z, 0);
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
    qDebug() << "Sampling 3D volume for 3D viewer took" << stopwatch.elapsed() / 1000.0 << "seconds";

    if (! slicesZxy.populateGLTextures()) return false;
    if (! slicesYzx.populateGLTextures()) return false;
    if (! slicesXyz.populateGLTextures()) return false;
    if (! neuronLabelTexture.uploadPixels()) return false;

    return true;
}


//////////////////////////////////
// VolumeTexture::Stack methods //
//////////////////////////////////

// Run once to create (but not populate/update) opengl textures
bool VolumeTexture::Stack::initializeGLTextures()
{
    QTime stopwatch;
    stopwatch.start();
    // qDebug() << "VolumeTexture::Stack::initializeGLTextures()" << __FILE__ << __LINE__;
    int numberOfSlices = size.x();
    int width = size.y();
    int height = size.z();
    if (numberOfSlices <= 0) return false;
    if (width <= 0) return false;
    if (height <= 0) return false;

    // Clear GL errors
    {
        int count = 0;
        while (glGetError() != GL_NO_ERROR)
        {
            ++count;
            if (count > 1000) break;
        }
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            return false;
        }
    }

    if (textureIDs.size() != (numberOfSlices + 1))
    {
         if (textureIDs.size() > 0) {
            glDeleteTextures(textureIDs.size(), &textureIDs.front());
            // textureIDs.clear();
        }
        textureIDs.assign(numberOfSlices + 1, 0);
        glGenTextures(numberOfSlices + 1, &textureIDs.front());
        // Check for GL errors
        {
            GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
                return false;
            }
        }
    }
    int border = 0;
    for (int i = 0; i < numberOfSlices + 1; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
        glTexImage2D(GL_TEXTURE_2D,
            0, ///< mipmap level; zero means base level
            GL_RGBA8, ///< texture format, in bytes per pixel
            width,
            height,
            border,
            GL_BGRA, // image format
            GL_UNSIGNED_INT_8_8_8_8_REV, // image type
            NULL); ///< NULL means initialize but don't populate
        // Check for GL errors
        {
            GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
                return false;
            }
        }
    }

    // fill plane zero with empty image (for testing)
    std::vector<unsigned int> blankPlane(width*height, 0);
    glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
    glTexSubImage2D(GL_TEXTURE_2D, // target
            0, // level
            0,0,  // offset
            width, // sub width
            height, // sub height
            GL_BGRA, // image format
            GL_UNSIGNED_INT_8_8_8_8_REV, // image type
            &blankPlane.front());
    // Check for GL errors
    {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            qDebug() << "OpenGL error" << err << __FILE__ << __LINE__;
            return false;
        }
    }

    // qDebug() << "Initialize textures took" << stopwatch.elapsed() / 1000.0 << "seconds";
    return true;
}

bool VolumeTexture::Stack::populateGLTextures()
{
    QTime stopwatch;
    stopwatch.start();
    // qDebug() << "VolumeTexture::Stack::populateGLTextures()" << __FILE__ << __LINE__;
    int numberOfSlices = size.x();
    int width = size.y();
    int height = size.z();
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
    qDebug() << "Populate textures took" << stopwatch.elapsed() / 1000.0 << "seconds";
    return true;
}


////////////////////////
// Dimension methods //
///////////////////////

Dimension::Dimension()
{
    data[0] = 0; data[1] = 0; data[2] = 0;
}

Dimension::Dimension(size_t x, size_t y, size_t z)
{
    data[0] = x; data[1] = y; data[2] = z;
}

const size_t& Dimension::operator[](size_t ix) const
{return data[ix];}

size_t& Dimension::operator[](size_t ix)
{return data[ix];}

const size_t& Dimension::x() const
{return data[X];}

const size_t& Dimension::y() const
{return data[Y];}

const size_t& Dimension::z() const
{return data[Z];}

size_t& Dimension::x()
{return data[X];}

size_t& Dimension::y()
{return data[Y];}

size_t& Dimension::z()
{return data[Z];}

bool Dimension::operator!=(const Dimension& rhs) const {
    const Dimension& lhs = *this;
    for (int i = 0; i < 3; ++i)
        if (lhs[i] != rhs[i]) return true;
    return false;
}

bool Dimension::operator==(const Dimension& rhs) const {
    const Dimension& lhs = *this;
    return ! (lhs != rhs);
}

/// Compute a possibly smaller size that would fit in a particular GPU memory limit,
/// assuming 32 bit pixels.
Dimension Dimension::sampledSize(size_t memoryLimit) const
{
    if (memoryLimit <= 0) return *this; // zero means no limit
    size_t memoryUse = x() * y() * z() * 2; // bytes
    if (memoryUse <= memoryLimit) return *this; // already fits into memory as-is
    double sampleFactor = (double)memoryLimit / (double)memoryUse;
    return Dimension((int)(x() * sampleFactor), // scale all dimensions
                     (int)(y() * sampleFactor),
                     (int)(z() * sampleFactor));
}

/// Expand dimensions, if necessary, to be a multiple of 8
Dimension Dimension::padToMultipleOf(unsigned int factor)
{
    return Dimension(padToMultipleOf(x(), factor),
                     padToMultipleOf(y(), factor),
                     padToMultipleOf(z(), factor));
}

/// Compute next multiple of factor greater than or equal to coord
/* static */
size_t Dimension::padToMultipleOf(size_t coord, unsigned int factor)
{
    const int remainder = coord % factor;
    if (0 == remainder) return coord;
    size_t result = coord + factor - remainder;
    assert(0 == (result % factor));
    return result;
}


} // namespace vaa3d

