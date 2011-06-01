#ifndef BRIGHTNESSCALIBRATOR_H
#define BRIGHTNESSCALIBRATOR_H

#include <cassert>

// BrightnessCalibrator class helps convert 2D 16-bit multichannel data into
// 2D 8-bit RGB data.  Gamma correction and HDR data range parameters
// influence the output.
template<class ValueType>
class BrightnessCalibrator
{
public:
    BrightnessCalibrator();
    float getGamma() const;
    void setGamma(float gamma);
    void setHdrRange(ValueType min, ValueType max);
    // Define frequently used methods in header, so compiler can inline them.
    // Return a corrected value between zero and 1.0
    float getCorrectedIntensity(ValueType value) const {
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
    unsigned char getCorrectedByte(ValueType value) const {
        return (unsigned char)((getCorrectedIntensity(value) * 255.0) + 0.4999);
    }

protected:
    ValueType displayMin; // min display value for HDR
    ValueType displayMax; // max display value for HDR
    float displayGamma; // gamma correction between displayMin and displayMax
    // Gamma lookup is kept in a small table, as a compromise between speed and memory use.
    float gammaTable[256]; // lookup table for gamma correction
    float dGammaTable[256]; // to help interpolation, especially for high dynamic range data.
};

#endif // BRIGHTNESSCALIBRATOR_H
