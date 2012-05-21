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

DataColorModel::DataColorModel(const NaVolumeData& volumeDataParam, bool bAutoCorrectParam)
    : volumeData(&volumeDataParam)
    , desiredColors(NULL)
    , currentColors(NULL)
    , bAutoCorrect(bAutoCorrectParam)
{
    connect(volumeData, SIGNAL(dataChanged()),
            this, SLOT(initialize()));
}

bool DataColorModel::setChannelUseSharedGamma(int index, bool useIt)
{
    bool bChanged = false;
    if (useIt == d.constData()->getChannelUseSharedGamma(index))
        return false; // no change
    {
        Writer(*this);
        bChanged = d->setChannelUseSharedGamma(index, useIt);
    }
    if (bChanged)
        emit dataChanged();
    return bChanged;
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
        if (bAutoCorrect)
            d->autoCorrect(volumeReader);
    } // release locks
    // qDebug() << "Done resetting DataColorModel";
    emit colorsInitialized();
    emit dataChanged();
}

void DataColorModel::setChannelColor(int index, /*QRgb*/ int color)
{
    if (d.constData()->getChannelColor(index) == color) return;
    {
        Writer colorWriter(*this);
        if (! d->setChannelColor(index, color)) return;
    } // release lock
    emit dataChanged();
}

void DataColorModel::setChannelHdrRange(int index, qreal minParam, qreal maxParam)
{
    if (d.constData()->hasChannelHdrRange(index, minParam, maxParam)) return;
    {
        Writer colorWriter(*this);
        d->setChannelHdrRange(index, minParam, maxParam);
    } // release lock
    emit dataChanged();
}

void DataColorModel::setSharedGamma(qreal gammaParam) // for all channels
{
    if (d.constData()->getSharedGamma() == gammaParam) return;
    // qDebug() << "DataColorModel::setGamma" << gammaParam;
    // Combine backlog of setGamma signals
    latestGamma = gammaParam;  // back up new value before aborting
    SlotMerger gammaMerger(statusOfSetGammaSlot);
    if (! gammaMerger.shouldRun()) return;
    qreal gamma = latestGamma;
    {
        Writer colorWriter(*this);
        if (! d->setSharedGamma(gamma)) return;
    } // release lock
    emit dataChanged();
}

void DataColorModel::setReferenceGamma(qreal gamma)
{
    setChannelGamma(3, gamma);
}

void DataColorModel::setChannelGamma(int index, qreal gamma)
{
    if (d.constData()->getChannelGamma(index) == gamma) return;
    {
        Writer colorWriter(*this);
        if (! d->setChannelGamma(index, gamma)) return;
    } // release lock
    emit dataChanged();
}

/* slot */
void DataColorModel::setChannelVisibility(int channel, bool isVisible) {
    if (d.constData()->getChannelVisibility(channel) == isVisible) return;
    {
        Writer colorWriter(*this);
        if (! d->setChannelVisibility(channel, isVisible)) return;
    } // release lock before emit
    emit dataChanged();
}

/* slot */
void DataColorModel::resetColors() {
    {
        Writer colorWriter(*this);
        d->resetColors();
    }
    // qDebug() << "DataColorModel::resetColors()" << __FILE__ << __LINE__;
    emit dataChanged();
}

////////////////////////////////////
// DataColorModel::Reader methods //
////////////////////////////////////

DataColorModel::Reader::Reader(const DataColorModel& colorModelParam)
    : BaseReader(colorModelParam)
{}

int DataColorModel::Reader::getNumberOfDataChannels() const {
    return d.constData()->getNumberOfDataChannels(); // note special "->" operator
}

QRgb DataColorModel::Reader::blend(const double channelIntensities[]) const {
    return d.constData()->blend(channelIntensities);
}

QRgb DataColorModel::Reader::blendInvisible(const double channelIntensities[]) const {
    return d.constData()->blendInvisible(channelIntensities);
}

QRgb DataColorModel::Reader::blend(const std::vector<double>& channelIntensities) const {
    return d.constData()->blend(channelIntensities);
}

qreal DataColorModel::Reader::getReferenceScaledIntensity(qreal raw_intensity) const {
    return d.constData()->getReferenceScaledIntensity(raw_intensity);
}

QRgb DataColorModel::Reader::getChannelColor(int channelIndex) const {
    return d.constData()->getChannelColor(channelIndex);
}

qreal DataColorModel::Reader::getChannelScaledIntensity(int channel, qreal raw_intensity) const {
    return d.constData()->getChannelScaledIntensity(channel, raw_intensity);
}

qreal DataColorModel::Reader::getChannelGamma(int channel) const {
    return d.constData()->getChannelGamma(channel);
}

qreal DataColorModel::Reader::getSharedGamma() const {
    return d.constData()->getSharedGamma();
}

qreal DataColorModel::Reader::getChannelHdrMin(int channel) const
{
    return d.constData()->getChannelHdrMin(channel);
}

qreal DataColorModel::Reader::getChannelHdrMax(int channel) const
{
    return d.constData()->getChannelHdrMax(channel);
}

qreal DataColorModel::Reader::getChannelDataMin(int channel) const
{
    return d.constData()->getChannelDataMin(channel);
}

qreal DataColorModel::Reader::getChannelDataMax(int channel) const
{
    return d.constData()->getChannelDataMax(channel);
}

bool DataColorModel::Reader::getChannelVisibility(int channel) const
{
    return d.constData()->getChannelVisibility(channel);
}

bool DataColorModel::Reader::getChannelUseSharedGamma(int index) const
{
    return d.constData()->getChannelUseSharedGamma(index);
}

////////////////////////////////////
// DataColorModel::Writer methods //
////////////////////////////////////

DataColorModel::Writer::Writer(DataColorModel& colorModelParam)
    : BaseWriter(colorModelParam)
{
    // qDebug() << "DataColorModel::Writer constructor";
}


