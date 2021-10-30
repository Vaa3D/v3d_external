/*
 * VolumeSlicesActor.h
 *
 *  Created on: Nov 26, 2012
 *      Author: Christopher M. Bruns
 */

#ifndef VOLUMESLICESACTOR_H_
#define VOLUMESLICESACTOR_H_

#include "../render/ActorGL.h"
#include "boost/shared_array.hpp"
#include <string>
#include <stdint.h>

// TODO separate into VolumeActor and VolumeTexture classes
class VolumeSlicesActor : public ActorGL
{
public:
    VolumeSlicesActor();
    virtual ~VolumeSlicesActor();
    virtual void destroyGL();
    virtual void initGL();
    bool loadVolume(const std::string& fileName);
    size_t numTextureBytes() const;
    virtual void paintGL();
    void setVoxelColor(int x, int y, int z, uint32_t color);
    void setVoxelMicrometers(double x, double y, double z);

protected:
    void populateTestVolume();

    double voxelMicrometers[3]; // size of one voxel
    // totalPaddedVoxelCount is number of voxels in each dimension.
    // Might be larger than used voxels, padded to power of 2 or multiple of 8
    // In X,Y,Z directions
    int totalPaddedVoxelCount[3]; // including blank padding
    int usedVoxelCount[3]; // number of voxels with actual intensities
    // TODO - Local transform should be in parent (ActorGL) class
    //
    // data storage
    int numChannels;
    int numPixelBytes;
    boost::shared_array<uint8_t> textureData;
    unsigned int textureIdGl;
    unsigned int pixelBufferGl;
    bool bIsInitialized;
    bool bTextureNeedsUpload;
};

#endif /* VOLUMESLICESACTOR_H_ */
