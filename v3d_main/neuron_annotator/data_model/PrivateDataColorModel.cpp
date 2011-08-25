#include "PrivateDataColorModel.h"
#include <algorithm>
#include <cmath>

PrivateDataColorModel::PrivateDataColorModel()
{}

PrivateDataColorModel::PrivateDataColorModel(const PrivateDataColorModel& rhs)
{
    channelColors = rhs.channelColors;
}

/* virtual */
PrivateDataColorModel::~PrivateDataColorModel() {}

bool PrivateDataColorModel::resetColors(const NaVolumeData::Reader& volumeReader)
{
    if (! volumeReader.hasReadLock()) return false;
    const Image4DProxy<My4DImage>& volProxy = volumeReader.getOriginalImageProxy();
    if (volProxy.sx <= 0) return false; // data not populated yet?
    const Image4DProxy<My4DImage>& refProxy = volumeReader.getReferenceImageProxy();
    // Add one for reference channel
    int numChannels = volProxy.sc + 1;
    channelColors.clear();
    for (int channel = 0; channel < numChannels; ++channel)
    {
        // Channel color
        // single channel gets colored white
        QRgb color = qRgb(255, 255, 255);
        if (numChannels > 1) {
            int remainder = channel % 4; // cycle through red, green, blue, gray
            if (remainder == 0) {
                color = qRgb(255, 0, 0); // red
            }
            else if (remainder == 1) {
                color = qRgb(0, 255, 0); // green
            }
            else if (remainder == 2) {
                color = qRgb(0, 0, 255); // blue
            }
            else {
                color = qRgb(255, 255, 255); // white
            }
        }
        ChannelColorModel channelModel(color);
        // Initialize intensity range to entire observed range
        if (channel < volProxy.sc) // fragments and background
            channelModel.setHdrRange(volProxy.vmin[channel], volProxy.vmax[channel]);
        else // reference channel
            channelModel.setHdrRange(refProxy.vmin[0], refProxy.vmax[0]);
        channelColors.push_back(channelModel);
        // qDebug() << "Channel color " << channelColors.size() << qRed(color) << qGreen(color) << qBlue(color);
    }
    return true;
}

const PrivateDataColorModel::ChannelColorModel& PrivateDataColorModel::getReferenceChannel() const {
    return channelColors.back();
}

QRgb PrivateDataColorModel::blend(const double channelIntensities[]) const {
    int red, green, blue;
    red = green = blue = 0;
    int sc = channelColors.size();
    for (int c = 0; c < sc; ++c) {
        if (channelIntensities[c] == 0.0)
            continue;
        const ChannelColorModel& ccm = channelColors[c];
        QRgb channelColor = ccm.getColor(channelIntensities[c]);
        red += qRed(channelColor);
        green += qGreen(channelColor);
        blue += qBlue(channelColor);
    }
    red = min(255, red);
    green = min(255, green);
    blue = min(255, blue);
    return qRgb(red, green, blue);
}

QRgb PrivateDataColorModel::blend(const std::vector<double>& channelIntensities) const {
    return blend(&channelIntensities[0]);
}

bool PrivateDataColorModel::setChannelColor(int index, QRgb color)
{
    if (channelColors[index].channelColor == color)
        return false; // no change
    channelColors[index].setColor(color);
    return true;
}

bool PrivateDataColorModel::setChannelHdrRange(int index, qreal minParam, qreal maxParam)
{
    if ( (channelColors[index].hdrMin == minParam)
       &&(channelColors[index].hdrMax == maxParam) )
        return false; // no change
    channelColors[index].setHdrRange(minParam, maxParam);
    return true;
}

bool PrivateDataColorModel::setGamma(qreal gamma) // for all channels
{
    bool bChanged = false;
    for (int c = 0; c < channelColors.size(); ++c) {
        if (channelColors[c].gamma != gamma)
            bChanged = true;
    }
    if (! bChanged) return false;
    for (int c = 0; c < channelColors.size(); ++c)
        channelColors[c].setGamma(gamma);
    return true;
}

bool PrivateDataColorModel::setChannelGamma(int index, qreal gamma)
{
    if (channelColors[index].gamma == gamma)
        return false; // no change
    channelColors[index].setGamma(gamma);
    return true;
}


//////////////////////////////////////////////////////
// PrivateDataColorModel::ChannelColorModel methods //
//////////////////////////////////////////////////////

PrivateDataColorModel::ChannelColorModel::ChannelColorModel(QRgb channelColorParam)
    : blackColor(qRgb(0, 0, 0))
{
    setColor(channelColorParam);
    setGamma(1.0f);
    setHdrRange(0.0f, 4095.0f);
}

void PrivateDataColorModel::ChannelColorModel::setColor(QRgb channelColorParam)
{
    channelColor = channelColorParam;
    colorRed = qRed(channelColor);
    colorGreen = qGreen(channelColor);
    colorBlue = qBlue(channelColor);
}

QRgb PrivateDataColorModel::ChannelColorModel::getColor() const {return channelColor;}

void PrivateDataColorModel::ChannelColorModel::setHdrRange(qreal hdrMinParam, qreal hdrMaxParam)
{
    hdrMin = hdrMinParam;
    hdrMax = hdrMaxParam;
    //hdrRange = std::max(qreal(1.0), hdrMax - hdrMin);
    hdrRange = max(qreal(1.0), hdrMax - hdrMin);
}

void PrivateDataColorModel::ChannelColorModel::setGamma(qreal gammaParam)
{
    gamma = gammaParam;
    gammaIsNotUnity = (gamma != 1.0f); // premature optimization
    // populate gamma lookup table
    qreal previous_i_out = 0.0f;
    for (int i = 0; i < 256; ++i) {
        qreal i_in = i / 255.0f; // range 0.0-1.0
        qreal i_out = std::pow(i_in, gamma);
        gammaTable[i] = i_out;
        if (i > 0)
            dGammaTable[i - 1] = i_out - previous_i_out;
        previous_i_out = i_out;
    }
    dGammaTable[255] = 0.0f; // final unused entry for neatness
}

// Returns a value in the range 0.0-1.0
qreal PrivateDataColorModel::ChannelColorModel::getScaledIntensity(qreal raw_intensity) const
{
    if (raw_intensity <= hdrMin) return 0.0; // clamp below
    if (raw_intensity >= hdrMax) return 1.0; // clamp above
    // 1) Apply hdr interval
    qreal i = (raw_intensity - hdrMin)/hdrRange;
    // 2) Apply gamma correction
    if (gammaIsNotUnity)
    {
       qreal gf = i * 255.0f; // index for lookup table, including decimal fraction
       int gi = int(gf); // index for lookup table
       i = gammaTable[gi] + dGammaTable[gi] * (gf - gi); // Taylor series interpolation
    }
    return i;
}

// getColor() definition is in header file so it can be inlined
QRgb PrivateDataColorModel::ChannelColorModel::getColor(qreal raw_intensity) const
{
    if (raw_intensity <= hdrMin) return blackColor; // clamp below
    if (raw_intensity >= hdrMax) return channelColor; // clamp above
    const qreal i = getScaledIntensity(raw_intensity);
    // 3) scale channel color
    return qRgb(int(i * colorRed), int(i * colorGreen), int(i * colorBlue));
}



