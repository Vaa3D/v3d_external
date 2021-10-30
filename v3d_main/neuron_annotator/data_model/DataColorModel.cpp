#include "PrivateNeuronFragmentData.h" // avoid compile error on msvc?
#include "PrivateVolumeTexture.h" // avoid compile error on msvc?

#include "NaSharedDataModel.cpp"
#include "PrivateDataColorModel.h"
template class NaSharedDataModel<PrivateDataColorModel>;

#include "DataColorModel.h"
#include "NaVolumeData.h"
#include "../DataFlowModel.h"
#include "NeuronFragmentData.h"


////////////////////////////
// DataColorModel methods //
////////////////////////////

DataColorModel::DataColorModel()
    : volumeData(NULL)
    , desiredColors(NULL)
    , currentColors(NULL)
    , dataFlowModel(NULL)
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

/* virtual */
DataColorModel::~DataColorModel()
{}

void DataColorModel::setDataFlowModel(const DataFlowModel * dfm)
{
    if (dfm == dataFlowModel)
        return;
    dataFlowModel = dfm;
    if (NULL == dataFlowModel)
        return;
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
        int c1 = desiredReader.getNumberOfDataChannels();
        int c2 = currentReader.getNumberOfDataChannels();
        int numChannels = std::min(c1, c2);
        if (numChannels < 1)
            return;
        // if (desiredReader.getNumberOfDataChannels() != currentReader.getNumberOfDataChannels())
        //     return; // color models are out of sync
        Writer(*this);
        d->colorizeIncremental(desiredReader, currentReader);
    } // release locks
    // qDebug() << "DataColorModel::colorizeIncremental()" << __FILE__ << __LINE__;
    emit dataChanged();
}

void DataColorModel::initialize()
{
    if (! volumeData) return;

    // Don't initialize colors if NaVolumeData is not the first receiver of volume data.
    // i.e. during fast load.  So use bDoUpdateSignalTexture as a proxy for
    // primacy of NaVolumeData in this run.
    // if (! volumeData->bDoUpdateSignalTexture)
    //    return;

    // qDebug() << "Resetting DataColorModel";
    {
        NaVolumeData::Reader volumeReader(*volumeData);
        if (! volumeReader.hasReadLock()) return;
        Writer colorWriter(*this);
        if (! d->initialize(volumeReader)) return;
    } // release locks
    // qDebug() << "Done resetting DataColorModel";
    emit colorsInitialized();
    emit dataChanged();
}

/* slot */
void DataColorModel::updateVolumeTextureMetadata()
{
    // qDebug() << "updateVolumeTextureMetaData()" << __FILE__ << __LINE__;
    if (NULL == dataFlowModel)
        return;
    {
        jfrc::VolumeTexture::Reader textureReader(dataFlowModel->getVolumeTexture());
        if (dataFlowModel->getVolumeTexture().readerIsStale(textureReader))
            return;
        const SampledVolumeMetadata& metadata = textureReader.metadata();
        Writer colorWriter(*this);
        for (int c = 0; c < metadata.channelGamma.size(); ++c) {
            // qDebug() << "setting channel gamma" << c << metadata.channelGamma[c];
            d->setChannelGamma(c, metadata.channelGamma[c]);
            // TODO - or should this be setting the data range, or the hdr range?
            d->setChannelHdrRange(c,
                                  metadata.channelHdrMinima[c],
                                  metadata.channelHdrMaxima[c]);
            // qDebug() << "slow 3d hdr max =" << metadata.channelHdrMaxima[c] << c << __FILE__ << __LINE__;
            if (metadata.channelHdrMaxima[c] > 16000.0)
                d->setChannelDataRange(c, 0, 65535); // 16 bit
            else if (metadata.channelHdrMaxima[c] > 1000.0)
                d->setChannelDataRange(c, 0, 4095); // 12 bit
            else
                d->setChannelDataRange(c, 0, 255); // 8 bit
        }
    }
    emit dataChanged();
}

bool DataColorModel::initializeRgba32()
{
    {
        Writer colorWriter(*this);
        if (! d->initializeRgba32())
            return false;
    }
    emit colorsInitialized();
    emit dataChanged();
    return true;
}

bool DataColorModel::initializeRgba48()
{
    {
        Writer colorWriter(*this);
        if (! d->initializeRgba48())
            return false;
    }
    emit colorsInitialized();
    emit dataChanged();
    return true;
}

void DataColorModel::setChannelColor(int index, /*QRgb*/ int color)
{
    if (d.constData()->getNumberOfDataChannels() <= index) return;
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

void DataColorModel::setChannelDataRange(int index, qreal minParam, qreal maxParam)
{
    if (d.constData()->hasChannelDataRange(index, minParam, maxParam)) return;
    {
        Writer colorWriter(*this);
        d->setChannelDataRange(index, minParam, maxParam);
    } // release lock
    emit dataChanged();
}

void DataColorModel::setSharedGamma(qreal gammaParam) // for all channels
{
    if (d.constData()->getSharedGamma() == gammaParam) return;
    // qDebug() << "DataColorModel::setGamma" << gammaParam << (long)this;
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
void DataColorModel::setChannelVisibility(int channel, bool isVisible)
{
    // qDebug() << "DataColorModel::setChannelVisibility()" << channel << isVisible << __FILE__ << __LINE__;
    if (d.constData()->getChannelVisibility(channel) == isVisible) return;
    {
        Writer colorWriter(*this);
        // qDebug() << "DataColorModel::setChannelVisibility()" << channel << isVisible << __FILE__ << __LINE__;
        if (! d->setChannelVisibility(channel, isVisible))
            return;
    } // release lock before emit
    // qDebug() << "visiblity changed";
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
{
    // qDebug() << "DataColorModel::Reader constructor";
    if (! colorModelParam.readerIsStale(*this)) {
        // qDebug() << "  got read lock";
    }
}

DataColorModel::Reader::~Reader()
{
    // qDebug() << "DataColorModel::Reader destructor";
}

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

DataColorModel::Writer::~Writer()
{
    // qDebug() << "DataColorModel::Writer destructor";
}

