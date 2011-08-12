#include "DataColorModel.h"

DataColorModel::DataColorModel(const NaVolumeData& volumeDataParam)
    : volumeData(volumeDataParam)
{
    connect(this, SIGNAL(dataChanged()),
            this, SLOT(storeModificationTime()));

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
    {
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
        statusOfSetChannelHdrSlot.assign(numChannels, SlotStatus());
        latestChannelHdrMin.assign(numChannels, 0);
        latestChannelHdrMax.assign(numChannels, 0);
    } // release lock
    emit dataChanged();
}

void DataColorModel::setChannelColor(int index, QRgb color)
{
    if (channelColors[index].channelColor == color)
        return; // no change
    {
        Writer colorWriter(*this);
        channelColors[index].setColor(color);
    } // release lock
    emit dataChanged();
}

void DataColorModel::setChannelHdrRange(int index, qreal minParam, qreal maxParam)
{
    // Combine backlog of setChannelHdrRange signal.
    // cache args before checking.
    latestChannelHdrMin[index] = minParam;
    latestChannelHdrMax[index] = maxParam;

    SlotMerger slotMerger(statusOfSetChannelHdrSlot[index]);
    if (! slotMerger.shouldRun()) return;

    qreal min = latestChannelHdrMin[index];
    qreal max = latestChannelHdrMax[index];
    if ( (channelColors[index].hdrMin == min)
       &&(channelColors[index].hdrMax == max) )
        return; // no change
    qDebug() << "setChannelHdrRange" << index << min << max;
    {
        Writer colorWriter(*this);
        channelColors[index].setHdrRange(min, max);
    } // release lock
    emit dataChanged();
}

void DataColorModel::setGamma(qreal gammaParam) // for all channels
{
    // qDebug() << "DataColorModel::setGamma" << gammaParam;
    // Combine backlog of setGamma signals
    latestGamma = gammaParam;  // back up new value before aborting
    SlotMerger gammaMerger(statusOfSetGammaSlot);
    if (! gammaMerger.shouldRun()) return;
    qreal gamma = latestGamma;
    {
        Reader colorReader(*this);
        if (! colorReader.hasReadLock()) return;
        bool bChanged = false;
        for (int c = 0; c < channelColors.size(); ++c) {
            if (channelColors[c].gamma != gamma)
                bChanged = true;
        }
        if (! bChanged) return;
    } // release lock
    {
        // qDebug() << "setting gamma to" << gamma;
        Writer colorWriter(*this);
        for (int c = 0; c < channelColors.size(); ++c)
            channelColors[c].setGamma(gamma);
    } // release lock
    emit dataChanged();
}

void DataColorModel::setChannelGamma(int index, qreal gamma)
{
    if (channelColors[index].gamma == gamma)
        return; // no change
    {
        Writer colorWriter(*this);
        channelColors[index].setGamma(gamma);
    } // release lock
    emit dataChanged();
}


