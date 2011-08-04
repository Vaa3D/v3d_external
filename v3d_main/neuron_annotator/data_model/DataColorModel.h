#ifndef DATACOLORMODEL_H
#define DATACOLORMODEL_H

#include "NaLockableData.h"
#include "NaVolumeData.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <qrgb.h>

// DataColorModel converts multichannel 16-bit intensities
// into rgb colors by combining:
//  1 - a color for each data channel
//  2 - min and max "HDR" limits for each channel
//  3 - gamma brightness correction for each channel
// TODO - deprecate BrightnessCalibrator for DataColorModel
class DataColorModel : public NaLockableData
{
    Q_OBJECT

public:
    explicit DataColorModel(
            const NaVolumeData& volumeDataParam,
            QObject *parentParam = NULL);

public slots:
    void resetColors();
    void setChannelColor(int index, QRgb color);
    void setChannelHdrRange(int index, float min, float max);
    void setChannelGamma(int index, float gamma);

public:
    // ChannelColorModel specifies the colorization parameters for a single data channel
    class ChannelColorModel
    {
    public:
        friend class DataColorModel;

        ChannelColorModel(QRgb channelColorParam)
            : blackColor(qRgb(0, 0, 0))
        {
            setColor(channelColorParam);
            setGamma(1.0f);
            setHdrRange(0.0f, 4095.0f);
        }

        void setColor(QRgb channelColorParam)
        {
            channelColor = channelColorParam;
            colorRed = qRed(channelColor);
            colorGreen = qGreen(channelColor);
            colorBlue = qBlue(channelColor);
        }

        void setHdrRange(float hdrMinParam, float hdrMaxParam)
        {
            hdrMin = hdrMinParam;
            hdrMax = hdrMaxParam;
            hdrRange = std::max(1.0f, hdrMax - hdrMin);
        }

        void setGamma(float gammaParam)
        {
            gamma = gammaParam;
            gammaIsNotUnity = (gamma != 1.0f); // premature optimization
            // populate gamma lookup table
            float previous_i_out = 0.0f;
            for (int i = 0; i < 256; ++i) {
                float i_in = i / 255.0f; // range 0.0-1.0
                float i_out = std::pow(i_in, gamma);
                gammaTable[i] = i_out;
                if (i > 0)
                    dGammaTable[i - 1] = i_out - previous_i_out;
                previous_i_out = i_out;
            }
            dGammaTable[255] = 0.0f; // final unused entry for neatness
        }

        // getColor() definition is in header file so it can be inlined
        QRgb getColor(float intensity) const
        {
            // 1) Apply hdr interval
            float i = (intensity - hdrMin)/hdrRange;
            if (i <= 0.0) return blackColor; // clamp below
            if (i >= 1.0) return channelColor; // clamp above
            // 2) Apply gamma correction
            if (gammaIsNotUnity)
            {
               float gf = i * 255.0f; // index for lookup table, including decimal fraction
               int gi = int(gf); // index for lookup table
               i = gammaTable[gi] + dGammaTable[gi] * (gf - gi); // Taylor series interpolation
            }
            // 3) scale channel color
            return qRgb(int(i * colorRed), int(i * colorGreen), int(i * colorBlue));
        }

    protected:
        QRgb blackColor;
        QRgb channelColor; // color of channel
        int colorRed; // red component of channelColor, for efficiency
        int colorGreen;
        int colorBlue;
        float hdrMin; // minimum intensity to scale
        float hdrMax; // maximum intensity to scale
        float hdrRange; // hdrMax - hdrMin
        float gamma; // exponential brightness correction
        bool gammaIsNotUnity; // precompute gamma != 1.0
        // lookup table for faster gamma transform
        float gammaTable[256]; // cache for efficiency
        float dGammaTable[256]; // first derivative of gammaTable values
    };


    class Reader; friend class Reader;
    class Reader : public BaseReadLocker
    {
    public:
        Reader(const DataColorModel& colorModelParam)
            : BaseReadLocker(colorModelParam.getLock())
            , colorModel(colorModelParam)
        {}

        // TODO - create more accessor methods for downstream clients
        int getNumberOfDataChannels() const {
            return colorModel.channelColors.size(); // +1 for reference
        }

        QRgb blend(double channelIntensities[]) const {
            int red, green, blue;
            red = green = blue = 0;
            int sc = colorModel.channelColors.size();
            for (int c = 0; c < sc; ++c) {
                if (channelIntensities[c] == 0.0)
                    continue;
                const ChannelColorModel& ccm = colorModel.channelColors[c];
                QRgb channelColor = ccm.getColor(channelIntensities[c]);
                red += qRed(channelColor);
                green += qGreen(channelColor);
                blue += qBlue(channelColor);
            }
            red = std::min(255, red);
            green = std::min(255, green);
            blue = std::min(255, blue);
            return qRgb(red, green, blue);
        }

    private:
        const DataColorModel& colorModel;
    };


    class Writer : public QWriteLocker
    {
    public:
        Writer(DataColorModel& colorModelParam)
            : QWriteLocker(colorModelParam.getLock())
            , colorModel(colorModelParam)
        {}

    protected:
        DataColorModel& colorModel;
    };


protected:
    const NaVolumeData& volumeData;
    std::vector<ChannelColorModel> channelColors;
};

#endif // DATACOLORMODEL_H
