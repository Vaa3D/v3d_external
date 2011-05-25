#include "BrightnessCalibrator.h"

#include <cassert>
#include <cmath>

template<class ValueType>
BrightnessCalibrator<ValueType>::BrightnessCalibrator()
    : displayMin(0)
    , displayMax(255)
    , displayGamma(1.0)
{
    // Initialize gamma lookup table, even though it won't be used for gamma==1.0
    for (int i = 0; i < 256; ++i)
    {
        gammaTable[i] = i / 255.0f;
        dGammaTable[i] = 1.0f / 255.0f;
    }
    dGammaTable[255] = 0;
}

template<class ValueType>
float BrightnessCalibrator<ValueType>::getGamma() const {
    return displayGamma;
}

template<class ValueType>
void BrightnessCalibrator<ValueType>::setGamma(float gamma)
{
    assert(gamma > 0.0f);
    if (gamma == displayGamma) return;
    float previous_i_out = 0.0; // for slope interpolation
    for (int i = 0; i < 256; ++i) { // gamma table entries
        float i_in = i/255.0; // range 0.0-1.0
        float i_out = std::pow(i_in, gamma);
        assert(i_out <= 1.0);
        assert(i_out >= 0.0);
        gammaTable[i] = i_out;
        if (i > 0) {
            // first derivative of gamma table for interpolation
            // stepwise linear interpolation
            dGammaTable[i - 1] = i_out - previous_i_out;
        }
        previous_i_out = i_out;
    }
    dGammaTable[255] = 0.0; // final unused entry for neatness
    displayGamma = gamma;
}

template<class ValueType>
void BrightnessCalibrator<ValueType>::setHdrRange(ValueType min, ValueType max) {
    if (min != displayMin) displayMin = min;
    if (max != displayMax) displayMax = max;
}

// Return a corrected value between zero and 1.0
template<class ValueType>
float BrightnessCalibrator<ValueType>::getCorrectedIntensity(ValueType value) const
{
    // 1 - apply HDR clamp
    if (value <= displayMin) return 0.0;
    if (value >= displayMax) return 1.0;
    // 2 - apply HDR ramp
    float i_out = (value - displayMin) / float(displayMax - displayMin);
    // 3 - apply gamma correction using lookup table
    if (displayGamma != 1.0) {
        float indexF = 255.0 * i_out;
        int ix = int(indexF);
        float d_ix = indexF - ix;
        assert(d_ix >= 0.0);
        assert(d_ix <= 1.0);
        i_out = gammaTable[ix] + d_ix * dGammaTable[ix];
    }
    assert(i_out >= 0.0f);
    assert(i_out <= 1.0f);
    return i_out;
}

template<class ValueType>
unsigned char BrightnessCalibrator<ValueType>::getCorrectedByte(ValueType value) const
{
    return (unsigned char)((getCorrectedIntensity(value) * 255.0) + 0.4999);
}

template class BrightnessCalibrator<float>;
template class BrightnessCalibrator<unsigned short>;
template class BrightnessCalibrator<int>;
template class BrightnessCalibrator<unsigned char>;
