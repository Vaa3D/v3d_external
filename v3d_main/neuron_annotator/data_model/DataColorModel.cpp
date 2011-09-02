#include "DataColorModel.h"
#include "PrivateDataColorModel.h"
#include "NaSharedDataModel.cpp"

template class NaSharedDataModel<PrivateDataColorModel>;

////////////////////////////
// DataColorModel methods //
////////////////////////////

DataColorModel::DataColorModel(const NaVolumeData& volumeDataParam)
    : volumeData(volumeDataParam)
{
    connect(&volumeData, SIGNAL(dataChanged()),
            this, SLOT(resetColors()));
}

void DataColorModel::resetColors()
{
    qDebug() << "Resetting DataColorModel";
    {
        NaVolumeData::Reader volumeReader(volumeData);
        if (! volumeReader.hasReadLock()) return;
        Writer colorWriter(*this);
        if (! d->resetColors(volumeReader)) return;
    } // release locks
    qDebug() << "Done resetting DataColorModel";
    emit dataChanged();
}

void DataColorModel::setChannelColor(int index, QRgb color)
{
    {
        Writer colorWriter(*this);
        if (! d->setChannelColor(index, color)) return;
    } // release lock
    emit dataChanged();
}

void DataColorModel::setChannelHdrRange(int index, qreal minParam, qreal maxParam)
{
    {
        Writer colorWriter(*this);
        d->setChannelHdrRange(index, minParam, maxParam);
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
        Writer colorWriter(*this);
        if (! d->setGamma(gamma)) return;
    } // release lock
    emit dataChanged();
}

void DataColorModel::setChannelGamma(int index, qreal gamma)
{
    {
        Writer colorWriter(*this);
        if (! d->setChannelGamma(index, gamma)) return;
    } // release lock
    emit dataChanged();
}


////////////////////////////////////
// DataColorModel::Reader methods //
////////////////////////////////////

DataColorModel::Reader::Reader(const DataColorModel& colorModelParam)
    : BaseReader(colorModelParam)
{}

int DataColorModel::Reader::getNumberOfDataChannels() const {
    return d->getNumberOfDataChannels(); // note special "->" operator
}

QRgb DataColorModel::Reader::blend(const double channelIntensities[]) const {
    return d->blend(channelIntensities);
}

QRgb DataColorModel::Reader::blend(const std::vector<double>& channelIntensities) const {
    return d->blend(channelIntensities);
}

qreal DataColorModel::Reader::getReferenceScaledIntensity(qreal raw_intensity) const {
    return d->getReferenceScaledIntensity(raw_intensity);
}

////////////////////////////////////
// DataColorModel::Writer methods //
////////////////////////////////////

DataColorModel::Writer::Writer(DataColorModel& colorModelParam)
    : BaseWriter(colorModelParam)
{}


