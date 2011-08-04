#include "DataColorModel.h"

DataColorModel::DataColorModel(
        const NaVolumeData& volumeDataParam,
        QObject *parentParam /* = NULL */)
    : NaLockableData(parentParam)
    , volumeData(volumeDataParam)
{
    connect(&volumeData, SIGNAL(dataChanged()),
            this, SLOT(resetColors()));
}

void DataColorModel::resetColors()
{
    NaVolumeData::Reader volumeReader(volumeData);
    if (! volumeReader.hasReadLock()) return;
    // Add one for reference channel
    int numChannels = volumeReader.getOriginalImageProxy().sc + 1;

    Writer colorWriter(*this);
    channelColors.clear();
    for (int c = 0; c < numChannels; ++c)
    {
        // single channel gets colored white
        QRgb color = qRgb(255, 255, 255);
        if (numChannels > 1) {
            int r = numChannels % 4; // cycle through red, green, blue, gray
            if (r == 0) color = qRgb(255, 0, 0); // red
            else if (r == 1) color = qRgb(0, 255, 0); // green
            else if (r == 2) color = qRgb(0, 0, 255); // blue
            else color = qRgb(255, 255, 255); // white
        }
        channelColors.push_back(ChannelColorModel(color));
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

void DataColorModel::setChannelHdrRange(int index, float min, float max)
{
    if ( (channelColors[index].hdrMin == min)
       &&(channelColors[index].hdrMax == max) )
        return; // no change
    Writer colorWriter(*this);
    channelColors[index].setHdrRange(min, max);
    colorWriter.unlock();
    emit dataChanged();
}

void DataColorModel::setChannelGamma(int index, float gamma)
{
    if (channelColors[index].gamma == gamma)
        return; // no change
    Writer colorWriter(*this);
    channelColors[index].setGamma(gamma);
    colorWriter.unlock();
    emit dataChanged();
}


