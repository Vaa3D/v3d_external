#include "DataColorModel.h"
#include "PrivateDataColorModel.h"
#include "NaSharedDataModel.cpp"

template class NaSharedDataModel<PrivateDataColorModel>;

////////////////////////////
// DataColorModel methods //
////////////////////////////

DataColorModel::DataColorModel()
    : volumeData(NULL)
    , desiredColors(NULL)
    , currentColors(NULL)
{
}

DataColorModel::DataColorModel(const NaVolumeData& volumeDataParam)
    : volumeData(&volumeDataParam)
    , desiredColors(NULL)
    , currentColors(NULL)
{
    connect(volumeData, SIGNAL(dataChanged()),
            this, SLOT(initialize()));
}

void DataColorModel::setIncrementalColorSource(const DataColorModel& desiredColorsParam, const DataColorModel& currentColorsParam)
{
    desiredColors = &desiredColorsParam;
    currentColors = &currentColorsParam;
    connect(desiredColors, SIGNAL(dataChanged()),
            this, SLOT(colorizeIncremental()));
    connect(currentColors, SIGNAL(dataChanged()),
            this, SLOT(colorizeIncremental()));
}

/* slot */
void DataColorModel::colorizeIncremental()
{
    // qDebug() << "DataColorModel::colorizeIncremental()" << __FILE__ << __LINE__;
    if (! desiredColors) return;
    if (! currentColors) return;
    {
        DataColorModel::Reader desiredReader(*desiredColors);
        if (desiredColors->readerIsStale(desiredReader)) return;
        DataColorModel::Reader currentReader(*currentColors);
        if (currentColors->readerIsStale(currentReader)) return;
        if (desiredReader.getNumberOfDataChannels() != currentReader.getNumberOfDataChannels())
            return; // color models are out of sync
        Writer(*this);
        d->colorizeIncremental(desiredReader, currentReader);
    } // release locks
    // qDebug() << "DataColorModel::colorizeIncremental()" << __FILE__ << __LINE__;
    emit dataChanged();
}

void DataColorModel::initialize()
{
    if (! volumeData) return;
    // qDebug() << "Resetting DataColorModel";
    {
        NaVolumeData::Reader volumeReader(*volumeData);
        if (! volumeReader.hasReadLock()) return;
        Writer colorWriter(*this);
        if (! d->initialize(volumeReader)) return;
    } // release locks
    // qDebug() << "Done resetting DataColorModel";
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

/* slot */
void DataColorModel::setChannelVisibility(int channel, bool isVisible) {
    {
        Writer colorWriter(*this);
        if (! d->setChannelVisibility(channel, isVisible)) return;
    } // release lock before emit
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

QRgb DataColorModel::Reader::getChannelColor(int channelIndex) const {
    return d->getChannelColor(channelIndex);
}

qreal DataColorModel::Reader::getChannelScaledIntensity(int channel, qreal raw_intensity) const {
    return d->getChannelScaledIntensity(channel, raw_intensity);
}

qreal DataColorModel::Reader::getChannelGamma(int channel) const {
    return d->getChannelGamma(channel);
}

qreal DataColorModel::Reader::getChannelHdrMin(int channel) const
{
    return d->getChannelHdrMin(channel);
}

qreal DataColorModel::Reader::getChannelHdrMax(int channel) const
{
    return d->getChannelHdrMax(channel);
}

bool DataColorModel::Reader::getChannelVisibility(int channel) const
{
    return d->getChannelVisibility(channel);
}

////////////////////////////////////
// DataColorModel::Writer methods //
////////////////////////////////////

DataColorModel::Writer::Writer(DataColorModel& colorModelParam)
    : BaseWriter(colorModelParam)
{}


