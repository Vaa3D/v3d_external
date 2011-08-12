#include "DataColorModel.h"

DataColorModel::DataColorModel(const NaVolumeData& volumeDataParam)
    : volumeData(volumeDataParam)
{
    connect(&volumeData, SIGNAL(dataChanged()),
            this, SLOT(resetColors()));
}

void DataColorModel::resetColors()
{
    NaVolumeData::Reader volumeReader(volumeData);
    if (! volumeReader.hasReadLock()) return;
    const Image4DProxy<My4DImage>& volProxy = volumeReader.getOriginalImageProxy();
    if (volProxy.sx <= 0) return; // data not populated yet?
    const Image4DProxy<My4DImage>& refProxy = volumeReader.getReferenceImageProxy();
    // Add one for reference channel
    int numChannels = volProxy.sc + 1;

    Writer colorWriter(*this);
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
    colorWriter.unlock();
    emit dataChanged();
}

void DataColorModel::setChannelColor(int index, QRgb color)
{
    if (channelColors[index].channelColor == color)
        return; // no change
    Writer colorWriter(*this);
    channelColors[index].setColor(color);
    colorWriter.unlock();
    emit dataChanged();
}

void DataColorModel::setChannelHdrRange(int index, qreal min, qreal max)
{
    if ( (channelColors[index].hdrMin == min)
       &&(channelColors[index].hdrMax == max) )
        return; // no change
    qDebug() << "setChannelHdrRange" << index << min << max;
    Writer colorWriter(*this);
    channelColors[index].setHdrRange(min, max);
    colorWriter.unlock();
    emit dataChanged();
}

void DataColorModel::setGamma(qreal gamma)
{
    bool bChanged = false;
    for (int c = 0; c < channelColors.size(); ++c) {
        if (channelColors[c].gamma != gamma)
            bChanged = true;
    }
    if (! bChanged) return;
    Writer colorWriter(*this);
    for (int c = 0; c < channelColors.size(); ++c)
        channelColors[c].setGamma(gamma);
    colorWriter.unlock();
    emit dataChanged();
}

void DataColorModel::setChannelGamma(int index, qreal gamma)
{
    if (channelColors[index].gamma == gamma)
        return; // no change
    Writer colorWriter(*this);
    channelColors[index].setGamma(gamma);
    colorWriter.unlock();
    emit dataChanged();
}


