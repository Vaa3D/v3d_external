#ifndef PRIVATEDATACOLORMODEL_H
#define PRIVATEDATACOLORMODEL_H

#include <QSharedData>
#include <QVector>
#include <vector>
#include <qrgb.h>
#include "NaVolumeData.h"
#include "DataColorModel.h"

class PrivateDataColorModel : public QSharedData
{
public:
    class ChannelColorModel;

    PrivateDataColorModel();
    PrivateDataColorModel(const PrivateDataColorModel& rhs);
    virtual ~PrivateDataColorModel();

    int getNumberOfDataChannels() const {
        return (int)channelColors.size(); // +1 for reference
    }
    bool initialize(const NaVolumeData::Reader& volumeReader);
    bool initializeRgba32();
    bool initializeRgba48();
    const ChannelColorModel& getReferenceChannel() const;
    QRgb blend(const double channelIntensities[]) const;
    QRgb blendInvisible(const double channelIntensities[]) const; // Ignores channel visibility
    QRgb blend(const std::vector<double>& channelIntensities) const;
    bool setChannelColor(int index, QRgb color);
    bool setChannelHdrRange(int index, qreal minParam, qreal maxParam);
    bool hasChannelHdrRange(int index, qreal minParam, qreal maxParam) const;
    bool hasChannelDataRange(int index, qreal minParam, qreal maxParam) const;
    bool setChannelDataRange(int index, qreal minParam, qreal maxParam);
    bool setChannelVisibility(int index, bool);
    bool getChannelVisibility(int index) const;
    bool setSharedGamma(qreal gammaParam);
    bool setChannelGamma(int index, qreal gamma);
    bool setChannelUseSharedGamma(int index, bool useIt);
    bool getChannelUseSharedGamma(int index) const;
    qreal getReferenceScaledIntensity(qreal raw_intensity) const;
    qreal getChannelScaledIntensity(int channel, qreal raw_intensity) const;
    qreal getChannelGamma(int channel) const;
    qreal getSharedGamma() const;
    qreal getChannelHdrMin(int channel) const;
    qreal getChannelHdrMax(int channel) const;
    qreal getChannelDataMin(int channel) const;
    qreal getChannelDataMax(int channel) const;
    void colorizeIncremental(
            const DataColorModel::Reader& desiredColorReader,
            const DataColorModel::Reader& currentColorReader);
    QRgb getChannelColor(int channelIndex) const;
    void resetColors()
    {
        resetGamma();
        // Don't change visibility of reference channel
        bool referenceVisibility = false;
        const int refChannel = 3;
        if (channelColors.size() > refChannel)
            referenceVisibility = channelColors[refChannel].showChannel;
        resetChannelVisibility();
        resetChannelColors();
        resetHdrRange();
        // restore reference visibility
        if (channelColors.size() > refChannel)
            channelColors[refChannel].showChannel = referenceVisibility;
    }
    void resetChannelColors();
    void resetGamma(qreal = 1.0);
    void resetChannelCount(int count=4);
    void resetHdr(qreal max=4095.0);
    void resetHdrRange() {
        for (int c = 0; c < channelColors.size(); ++c)
            channelColors[c].resetHdrRange();
    }
    void resetChannelVisibility();

private:
    // std::vector<ChannelColorModel> channelColors;
    QVector<ChannelColorModel> channelColors;
    qreal sharedGamma;

public:
    // getRendererNa specifies the colorization parameters for a single data channel
    class ChannelColorModel
    {
    public:
        friend class PrivateDataColorModel;

        ChannelColorModel(QRgb channelColorParam = Qt::magenta);
        ChannelColorModel(const ChannelColorModel& rhs);
        void setColor(QRgb channelColorParam);
        QRgb getColor() const;
        void setDataRange(qreal dataMinParam, qreal dataMaxParam);
        void setHdrRange(qreal hdrMinParam, qreal hdrMaxParam);
        void resetHdrRange();
        void setGamma(qreal gammaParam);
        // Returns a value in the range 0.0-1.0
        qreal getScaledIntensity(qreal raw_intensity) const; // Applies visibility toggle
        qreal getInvisibleScaledIntensity(qreal raw_intensity) const; // Ignores visibility toggle
        QRgb getColor(qreal raw_intensity) const;
        QRgb getInvisibleColor(qreal raw_intensity) const; // Ignores visibility toggle
        qreal getGamma() const {return gamma;}
        qreal getHdrMin() const {return hdrMin;}
        qreal getHdrMax() const {return hdrMax;}
        qreal getDataMin() const {return dataMin;}
        qreal getDataMax() const {return dataMax;}
        void resetColors() {
            setGamma(1.0);
            resetHdrRange();
            showChannel = true;
        }

    protected:
        bool showChannel;
        QRgb blackColor;
        QRgb channelColor; // color of channel
        int colorRed; // red component of channelColor, for efficiency
        int colorGreen;
        int colorBlue;
        qreal hdrMin; // minimum intensity to scale
        qreal hdrMax; // maximum intensity to scale
        qreal hdrRange; // hdrMax - hdrMin
        qreal dataMin; // actual minimum intensity of data
        qreal dataMax; // actual maximum intensity of data
        qreal gamma; // exponential brightness correction
        bool gammaIsNotUnity; // precompute gamma != 1.0
        // lookup table for faster gamma transform
        qreal gammaTable[256]; // cache for efficiency
        qreal dGammaTable[256]; // first derivative of gammaTable values
        bool bUseSharedGamma; // flag that permits reference channel to not use shared gamma
    };
};

#endif // PRIVATEDATACOLORMODEL_H
