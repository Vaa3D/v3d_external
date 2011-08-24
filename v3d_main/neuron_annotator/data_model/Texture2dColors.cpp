#include "Texture2dColors.h"
#include "OpenGL/gl.h" // specific to Mac?
#include "OpenGL/glu.h" // specific to Mac?
#include <cassert>

// Texture2dColors is a data source for opengl volume rendering
// using 2D textures.


///////////////////////
// AxisStack methods //
///////////////////////

template<class PixelFormat>
GLenum gl_pixel_format();
template<class PixelFormat>
GLenum gl_pixel_type();

// TODO - test speed with BGRA
template<>
GLenum gl_pixel_format<RGBA8>() {return GL_RGBA;}
// TODO - test speed with GL_UNSIGNED_INT_8_8_8_8_REV
template<>
GLenum gl_pixel_type<RGBA8>() {return GL_UNSIGNED_BYTE;}

template<class PixelFormat>
AxisStack<PixelFormat>::AxisStack(Axis axis1,
          Axis axis2,
          Axis axis3)
{
    assert(axis1 != axis2);
    assert(axis2 != axis3);
    assert(axis3 != axis1);
}

template<class PixelFormat>
AxisStack<PixelFormat>::~AxisStack() {delete [] data;}

// operator[] returns pointer for use as final argument to glTexImage2D
template<class PixelFormat>
const PixelFormat* AxisStack<PixelFormat>::operator[](int sliceIx) const
{
    return data[sliceIx];
}

// Only call uploadOpenGlTextures from your opengl thread!
template<class PixelFormat>
void AxisStack<PixelFormat>::uploadOpenGlTextures() const
{
    for (int sliceIx = 0; sliceIx < sizes[0]; ++sliceIx)
    {
        glBindTexture(GL_TEXTURE_2D, textureIds[sliceIx]);
        glTexImage2D(GL_TEXTURE_2D, // target
                0, // level of detail
                GL_RGBA, // bytes-per-pixel on opengl device
                sizes[1], // width
                sizes[2], // height
                0, // border width
                gl_pixel_format<PixelFormat>(), // image format
                gl_pixel_type<PixelFormat>(), // image type
                (*this)[sliceIx] );
        GLenum id = glGetError();
        if (id != GL_NO_ERROR) {
            qDebug() << gluErrorString(id) << __FILE__ << __LINE__;
        }
    }
}


template class AxisStack<RGBA8>;

/////////////////////////////
// Texture2dColors methods //
/////////////////////////////

/* explicit */
Texture2dColors::Texture2dColors(
        const NaVolumeData& volumeDataParam,
        const DataColorModel& dataColorModelParam,
        const NeuronSelectionModel& neuronSelectionModelParam)
            : xStack(X_AXIS, Z_AXIS, Y_AXIS)
            , yStack(Y_AXIS, Z_AXIS, X_AXIS)
            , zStack(Z_AXIS, Y_AXIS, X_AXIS) // zStack is standard stack
{
}

// Only call uploadOpenGlTextures from your opengl thread!
void Texture2dColors::uploadOpenGlTextures()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // TODO - first upload the one stack the viewer is seeing, then emit
    // a signal to refresh before uploading the unseen axis directions.
    xStack.uploadOpenGlTextures();
    yStack.uploadOpenGlTextures();
    zStack.uploadOpenGlTextures();
}




