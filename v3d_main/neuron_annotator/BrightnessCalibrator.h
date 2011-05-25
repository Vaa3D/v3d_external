#ifndef BRIGHTNESSCALIBRATOR_H
#define BRIGHTNESSCALIBRATOR_H

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
    // Return a corrected value between zero and 1.0
    float getCorrectedIntensity(ValueType value) const;
    unsigned char getCorrectedByte(ValueType value) const;

protected:
    ValueType displayMin; // min display value for HDR
    ValueType displayMax; // max display value for HDR
    float displayGamma; // gamma correction between displayMin and displayMax
    // Gamma lookup is kept in a small table, as a compromise between speed and memory use.
    float gammaTable[256]; // lookup table for gamma correction
    float dGammaTable[256]; // to help interpolation, especially for high dynamic range data.
};

#endif // BRIGHTNESSCALIBRATOR_H
