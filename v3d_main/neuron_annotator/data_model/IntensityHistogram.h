#ifndef INTENSITYHISTOGRAM_H
#define INTENSITYHISTOGRAM_H

#include <vector>
#include "../../v3d/v3d_core.h" // My4DImage

class IntensityHistogram
{
public:
    typedef unsigned short Intensity_t; // Up to 12-bits for now

    IntensityHistogram();
    void populate(const Image4DProxy<My4DImage>& image, int channel_index = 0);
    Intensity_t quantileIntensity(float quantile) const; // domain 0.0-1.0, range 0-max intensity
    void clear();

protected:
    std::vector<size_t> data;
    size_t total_counts;
    Intensity_t min_intensity;
    Intensity_t max_intensity;
};

#endif // INTENSITYHISTOGRAM_H
