#ifndef PRIVATEDATACOLORMODEL_H
#define PRIVATEDATACOLORMODEL_H

#include <QSharedData>
#include <vector>
#include <qrgb.h>
#include "NaVolumeData.h"

class PrivateDataColorModel : public QSharedData
{
public:
    class ChannelColorModel;

    PrivateDataColorModel();
    PrivateDataColorModel(const PrivateDataColorModel& rhs);
    virtual ~PrivateDataColorModel();

    int getNumberOfDataChannels() const {
        return channelColors.size(); // +1 for reference
    }

    bool resetColors(const NaVolumeData::Reader& volumeReader);
    const ChannelColorModel& getReferenceChannel() const;
    QRgb blend(const double channelIntensities[]) const;
    QRgb blend(const std::vector<double>& channelIntensities) const;
    bool setChannelColor(int index, QRgb color);
    bool setChannelHdrRange(int index, qreal minParam, qreal maxParam);
    bool hasChannelHdrRange(int index, qreal minParam, qreal maxParam) const;
    bool setGamma(qreal gammaParam);
    bool setChannelGamma(int index, qreal gamma);
    qreal getReferenceScaledIntensity(qreal raw_intensity) const {
        return getReferenceChannel().getScaledIntensity(raw_intensity);
    }

private:
    std::vector<ChannelColorModel> channelColors;


public:
    // getRendererNa specifies the colorization parameters for a single data channel
    class ChannelColorModel
    {
    public:
        friend class PrivateDataColorModel;

        ChannelColorModel(QRgb channelColorParam);
        void setColor(QRgb channelColorParam);
        QRgb getColor() const;
        void setHdrRange(qreal hdrMinParam, qreal hdrMaxParam);
        void setGamma(qreal gammaParam);
        // Returns a value in the range 0.0-1.0
        qreal getScaledIntensity(qreal raw_intensity) const;
        // getColor() definition is in header file so it can be inlined
        QRgb getColor(qreal raw_intensity) const;

    protected:
        QRgb blackColor;
        QRgb channelColor; // color of channel
        int colorRed; // red component of channelColor, for efficiency
        int colorGreen;
        int colorBlue;
        qreal hdrMin; // minimum intensity to scale
        qreal hdrMax; // maximum intensity to scale
        qreal hdrRange; // hdrMax - hdrMin
        qreal gamma; // exponential brightness correction
        bool gammaIsNotUnity; // precompute gamma != 1.0
        // lookup table for faster gamma transform
        qreal gammaTable[256]; // cache for efficiency
        qreal dGammaTable[256]; // first derivative of gammaTable values
    };
};

#endif // PRIVATEDATACOLORMODEL_H
