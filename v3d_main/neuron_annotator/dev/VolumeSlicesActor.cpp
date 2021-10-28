/*
 * VolumeSlicesActor.cpp
 *
 *  Created on: Nov 26, 2012
 *      Author: Christopher M. Bruns
 */

#include "VolumeSlicesActor.h"
#include "../../3drenderer/GLee_r.h"
#include <fstream>
#include <iostream>
#include <string>
#include <stdint.h>

using namespace std;

VolumeSlicesActor::VolumeSlicesActor()
    : numChannels(0)
    , textureIdGl(0)
    , pixelBufferGl(0)
    , bIsInitialized(false)
    , bTextureNeedsUpload(false)
{
    // Set parameters to default values
    for (int i = 0; i < 3; ++i)
    {
        voxelMicrometers[i] = 1.0;
        totalPaddedVoxelCount[i] = 0;
        usedVoxelCount[i] = 0;
    }
}

VolumeSlicesActor::~VolumeSlicesActor()
{
}

/* virtual */
void VolumeSlicesActor::destroyGL()
{
    if (bIsInitialized) {
        glDeleteTextures(1, &textureIdGl);
        glDeleteBuffers(1, &pixelBufferGl);
    }
    textureIdGl = 0;
    pixelBufferGL = 0;
}

/* virtual */
void VolumeSlicesActor::initGL()
{
    // TODO refactor initialization check into parent class
    if (bIsInitialized)
        return;

    // Create 3D texture for volume image
    glGenBuffers(1, &pixelBufferGl);
    glGenTextures(1, &textureIdGl);

    glBindTexture(GL_TEXTURE_3D, textureIdGl);
    int internalFormat = GL_RGBA8;
    int format = GL_RGBA;
    if (numChannels == 1) {
        internalFormat = format = GL_RED;
    }
    else if (numChannels == 2) {
        internalFormat = format = GL_RG;
    }
    else if (numChannels == 3) {
        internalFormat = format = GL_RGB;
    }
    else if (numChannels == 4) {
        internalFormat = format = GL_RGBA;
    }
    // 16-bit data
    int type = GL_UNSIGNED_BYTE;
    if (numPixelBytes == 2)
        type = GL_UNSIGNED_SHORT;
    // Allocate on device
    glTexImage3D(GL_TEXTURE_3D,
                0, // mipmap level
                internalFormat, // bytes per pixel, plus somehow srgb info
                totalPaddedVoxelCount[0], // width
                totalPaddedVoxelCount[1], // height
                totalPaddedVoxelCount[2], // depth
                0, // border
                format, // voxel component order
                type, // voxel component type
                NULL);
    GLuint error = glGetError();
    if (error != GL_NO_ERROR) {
        cerr << "OpenGL error " << error << endl;
    }

    // Copy
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixelBufferGl);
    // Allocate device buffer
    glBufferData(GL_PIXEL_UNPACK_BUFFER,
            numTextureBytes(),
            NULL,
            GL_STATIC_DRAW);
    uint8_t* ptr = (uint8_t*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    // Copy texture
    // cerr << "Copying " << textureBytes << " bytes to pixel buffer object" << endl;
    memcpy(ptr, textureData.get(), numTextureBytes());
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindTexture(GL_TEXTURE_3D, textureIdGl);
    // asynchronous copy
    glTexSubImage3D(GL_TEXTURE_3D,
                0, // mipmap level
                0, 0, 0, // x/y/z offset
                totalPaddedVoxelCount[0], // width
                totalPaddedVoxelCount[1], // height
                totalPaddedVoxelCount[2], // depth
                format, // voxel component order
                type, // voxel component type
                0); // zero -> use pixel buffer object...
    // Restore state
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    bIsInitialized = true;
}

bool VolumeSlicesActor::loadVolume(const std::string& fileName)
{
    // TODO
    cerr << "loadVolume " << fileName;
    cerr << " " << __FILE__ << __LINE__;
    cerr << endl;

    // Assume v3draw for now
    ifstream in(fileName.c_str());
    if (! in.good())
        return false;

    // Read 43 byte header
    // Magic file identification string
    const std::string expected_key("raw_image_stack_by_hpeng");
    char file_key[128];
    in.read(file_key, expected_key.size());
    file_key[expected_key.size()] = '\0';
    if (expected_key != file_key)
        return false;
    // Big-endian or little endian data format?
    char dataEndian;
    in >> dataEndian;
    if (dataEndian != 'L')
        return false;
    // 1 or 2 bytes per intensity value?
    uint16_t pixelBytes;
    in.read((char*) &pixelBytes, 2);
    if ((pixelBytes < 1) || (pixelBytes > 2))
        return false;
    // Pixel extents in X, Y, Z, and color dimensions
    uint32_t dim[4];
    for (int i = 0; i < 4; ++i)
    {
        dim[i] = 0;
        in.read((char*) &dim[i], 4);
        if (i < 3) {
            totalPaddedVoxelCount[i] = dim[i];
            usedVoxelCount[i] = dim[i];
        }
    }
    numChannels = dim[3];
    numPixelBytes = pixelBytes;

    // Read one slice at a time
    size_t numSliceBytes = pixelBytes * dim[0] * dim[1];
    size_t totalBytes = numSliceBytes * dim[2] * dim[3];
    // cerr << "Allocating " << totalBytes << " bytes for texture" << endl;
    textureData.reset(new uint8_t[totalBytes]);
    boost::shared_array<uint8_t> slice(new uint8_t[numSliceBytes]);
    for (int c = 0; c < dim[3]; ++c) {
        for (int z = 0; z < dim[2]; ++z) {
            in.read((char*)slice.get(), numSliceBytes);
            // TODO process that slice
        }
    }

    cerr << "Finished loading volume" << endl;

    return true;
}

size_t VolumeSlicesActor::numTextureBytes() const
{
    return totalPaddedVoxelCount[0]
        * totalPaddedVoxelCount[1]
        * totalPaddedVoxelCount[2]
        * numChannels
        * numPixelBytes;
}

/* virtual */
void VolumeSlicesActor::paintGL()
{
    if (! bIsInitialized)
        initGL();
    // TODO - paint cube bounding box for initial implementation
    glColor3f(1.00, 1.00, 0.50);

    const float x = 0.5 * voxelMicrometers[0] * usedVoxelCount[0];
    const float y = 0.5 * voxelMicrometers[1] * usedVoxelCount[1];
    const float z = 0.5 * voxelMicrometers[2] * usedVoxelCount[2];

    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_LINE_STRIP);
    glVertex3f( x,  y,  z);
    glVertex3f(-x,  y,  z);
    glVertex3f(-x, -y,  z);
    glVertex3f( x, -y,  z);
    glVertex3f( x,  y,  z);
    glVertex3f( x,  y, -z);
    glVertex3f(-x,  y, -z);
    glVertex3f(-x, -y, -z);
    glVertex3f( x, -y, -z);
    glVertex3f( x,  y, -z);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(-x,  y,  z);
    glVertex3f(-x,  y, -z);
    glVertex3f(-x, -y,  z);
    glVertex3f(-x, -y, -z);
    glVertex3f( x, -y,  z);
    glVertex3f( x, -y, -z);
    glEnd();
}

void VolumeSlicesActor::populateTestVolume()
{
    // Regular RGBA
    numChannels = 4;
    numPixelBytes = 1;
    // 2x1x3
    voxelMicrometers[0] = 30.0;
    voxelMicrometers[1] = 40.0;
    voxelMicrometers[2] = 20.0;
    totalPaddedVoxelCount[0] = totalPaddedVoxelCount[1] = totalPaddedVoxelCount[2] = 8;
    usedVoxelCount[0] = 2;
    usedVoxelCount[1] = 1;
    usedVoxelCount[2] = 3;
    textureData.reset(new uint8_t[numTextureBytes()]);
    memset(textureData.get(), 0, numTextureBytes());
    setVoxelColor(0,0,0, 0x11110000); // ghostly red
    setVoxelColor(0,0,1, 0x44004400);
    setVoxelColor(0,0,2, 0x77000077);
    setVoxelColor(0,1,0, 0xaaaa0000);
    setVoxelColor(0,1,1, 0xdd00dd00);
    setVoxelColor(0,1,2, 0xff0000ff); // opaque blue
    bTextureNeedsUpload = true;
}

void VolumeSlicesActor::setVoxelColor(int x, int y, int z, uint32_t color)
{
    size_t index = x * numPixelBytes;
    index += y * totalPaddedVoxelCount[0] * numPixelBytes;
    index += z * totalPaddedVoxelCount[1] * totalPaddedVoxelCount[0] * numPixelBytes;
    uint8_t   red = (0x00ff0000 & color) >> 16;
    textureData[index] = red;
    if (numChannels < 2) return;
    uint8_t green = (0x0000ff00 & color) >> 8;
    textureData[index+1] = green;
    if (numChannels < 3) return;
    uint8_t  blue = (0x000000ff & color) >> 0;
    textureData[index+2] = blue;
    if (numChannels < 4) return;
    uint8_t alpha = (0xff000000 & color) >> 24;
    textureData[index+3] = alpha;
}

void VolumeSlicesActor::setVoxelMicrometers(double x, double y, double z)
{
    voxelMicrometers[0] = x;
    voxelMicrometers[1] = y;
    voxelMicrometers[2] = z;
}
