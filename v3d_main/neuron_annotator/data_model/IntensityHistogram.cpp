#include "IntensityHistogram.h"
#include <iostream>
#include <cassert>
#include <limits>

using namespace std;

IntensityHistogram::IntensityHistogram()
{
    clear();
}

void IntensityHistogram::populate(const Image4DProxy<My4DImage>& image, int channel_index)
{
    V3DLONG c = channel_index;
    // assert(image.has_minmax());
    min_intensity = image.vmin[c];
    max_intensity = image.vmax[c];
    // Initialize to zero
    data.assign((size_t)max_intensity, (size_t)0);
    total_counts = 0;
    for (V3DLONG z = 0; z < image.sz; ++z) {
        for (V3DLONG y = 0; y < image.sy; ++y) {
            for (V3DLONG x = 0; x < image.sx; ++x) {
                v3d_uint16 value = (v3d_uint16)(image.value_at(x, y, z, c));
                assert(value >= min_intensity);
                assert(value <= max_intensity);
                data[value] += 1;
                total_counts += 1;
            }
        }
    }
}

IntensityHistogram::Intensity_t IntensityHistogram::quantileIntensity(float quantile) const // domain 0.0-1.0, range 0-max intensity
{
    if (0 == total_counts) return 0;
    if (quantile <= 0.0) return min_intensity;
    if (quantile >= 1.0) return max_intensity;
    size_t threshold_count = (size_t)(quantile * total_counts);
    size_t integrated_count = 0;
    Intensity_t result;
    for(result = min_intensity; result <= max_intensity; ++result)
    {
        integrated_count += data[result];
        if (integrated_count >= threshold_count)
            return result;
    }
    cerr << "Error finding quantileIntensity " << __FILE__ << __LINE__ << endl;
    assert(false); // should not get this far
    return max_intensity; // try to be fail gracefully on error in release mode...
}

void IntensityHistogram::clear()
{
    data.assign((size_t)data.size(), (size_t)0);
    total_counts = 0;
    max_intensity = 0;
    // Integer types do not have infinity representation, but do have max
    min_intensity = numeric_limits<Intensity_t>::max();
}

