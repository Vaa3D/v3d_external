#include "SampledVolumeMetadata.h"
#include "../utility/url_tools.h"
#include <fstream>
#include <iostream>
#include <sstream>

SampledVolumeMetadata::SampledVolumeMetadata()
    : channelGamma((size_t)4, 1.0)
    , channelHdrMinima((size_t)4, (uint32_t)0)
    , channelHdrMaxima((size_t)4, (uint32_t)4095)
{}

bool SampledVolumeMetadata::loadFromUrl(QUrl fileUrl, int channel_offset)
{
    UrlStream stream(fileUrl);
    if (stream.io() == NULL)
        return false;
    char lineBuffer[1024];
    bool bChanged = false;
    while (stream.io()->readLine(lineBuffer, 1024))
    {
        std::string line(lineBuffer);
        std::istringstream lineStream(line);
        std::string recordType;
        lineStream >> recordType;
        if (recordType.length() < 1)
            continue;
        if (recordType == "CHANNEL_RESCALE") {
            // CHANNEL_RESCALE         0       9       4276
            int channel;
            uint32_t min, max;
            lineStream >> channel;
            channel += channel_offset;
            lineStream >> min;
            lineStream >> max;
            if (min != channelHdrMinima[channel]) {
                channelHdrMinima[channel] = min;
                bChanged = true;
            }
            if (max != channelHdrMaxima[channel]) {
                channelHdrMaxima[channel] = max;
                bChanged = true;
            }
        }
        else if (recordType == "CHANNEL_GAMMA") {
            // CHANNEL_GAMMA   0       0.46
            int channel;
            float gamma;
            lineStream >> channel;
            channel += channel_offset;
            lineStream >> gamma;
            if (gamma != channelGamma[channel]) {
                channelGamma[channel] = gamma;
                bChanged = true;
            }
        }
        else if (recordType == "PARENT_DIMS") {
            // PARENT_DIMS     512     512     445
            size_t x, y, z;
            lineStream >> x >> y >> z;
            if (x*y*z < 1)
                continue;
            jfrc::Dimension d(x, y, z);
            if (d != originalImageSize) {
                originalImageSize = d;
                bChanged = true;
            }
        }
        else if (recordType == "RESAMPLED_DIMS") {
            // RESAMPLED_DIMS  392     392     336
            size_t x, y, z;
            lineStream >> x >> y >> z;
            if (x*y*z < 1)
                continue;
            jfrc::Dimension d(x, y, z);
            if (d != paddedImageSize) {
                paddedImageSize = d;
                bChanged = true;
            }
        }
        else if (recordType == "USED_DIMS") {
            // USED_DIMS  386     386     336
            size_t x, y, z;
            lineStream >> x >> y >> z;
            if (x*y*z < 1)
                continue;
            jfrc::Dimension d(x, y, z);
            if (d != usedImageSize) {
                usedImageSize = d;
                bChanged = true;
            }
        }
        if (stream.io()->atEnd())
            break;
    }
    return bChanged;
}

bool SampledVolumeMetadata::operator==(const SampledVolumeMetadata& rhs) const
{
    return ! (*this != rhs);
}

bool SampledVolumeMetadata::operator!=(const SampledVolumeMetadata& rhs) const
{
    if (originalImageSize != rhs.originalImageSize)
        return true;
    if (paddedImageSize != rhs.paddedImageSize)
        return true;
    if (usedImageSize != rhs.usedImageSize)
        return true;
    if (channelGamma.size() != rhs.channelGamma.size())
        return true;
    for (int c = 0; c < channelGamma.size(); ++c) {
        if (channelGamma[c] != rhs.channelGamma[c])
            return true;
        if (channelHdrMinima[c] != rhs.channelHdrMinima[c])
            return true;
        if (channelHdrMaxima[c] != rhs.channelHdrMaxima[c])
            return true;
    }

    return false;
}

