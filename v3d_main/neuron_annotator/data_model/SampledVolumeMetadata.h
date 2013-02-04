#ifndef SAMPLEDVOLUMEMETADATA_H
#define SAMPLEDVOLUMEMETADATA_H


#include "Dimension.h"
#include <QUrl>
#include <vector>
#include <stdint.h>


// Data structure to store relationship between sampled volume and original image
class SampledVolumeMetadata
{
public:
    SampledVolumeMetadata();
    bool loadFromUrl(QUrl fileUrl, int channel_offset);
    bool operator==(const SampledVolumeMetadata& rhs) const;
    bool operator!=(const SampledVolumeMetadata& rhs) const;

    // Image metadata from .sizes/.colors file
    jfrc::Dimension originalImageSize;
    jfrc::Dimension paddedImageSize;
    jfrc::Dimension usedImageSize;
    std::vector<float> channelGamma;
    std::vector<uint32_t> channelHdrMinima;
    std::vector<uint32_t> channelHdrMaxima;
};


#endif // SAMPLEDVOLUMEMETADATA_H
