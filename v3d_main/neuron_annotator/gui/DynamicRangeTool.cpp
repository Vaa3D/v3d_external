#include "DynamicRangeTool.h"
#include <QColorDialog>

DynamicRangeTool::DynamicRangeTool(QWidget *parent)
    : QDialog(parent)
    , dataColorModel(NULL)
    , currentChannelIndex(-1)
    , channelHdrMin(0)
    , channelHdrMax(0)
    , channelGamma(0)
    , channelDataMin(0)
    , channelDataMax(0)
{
    ui.setupUi(this);
    connect(ui.comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setChannel(int)));
    connect(ui.changeColorButton, SIGNAL(clicked()),
            this, SLOT(selectColor()));
    connect(ui.channelMax_spinBox, SIGNAL(valueChanged(int)),
            this, SLOT(setHdrMax(int)));
    connect(ui.channelMin_spinBox, SIGNAL(valueChanged(int)),
            this, SLOT(setHdrMin(int)));
    connect(ui.channelGamma_doubleSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setGamma(double)));
    connect(ui.colorRangeSlider, SIGNAL(hdrMinChanged(int)),
            this, SLOT(setHdrMin(int)));
    connect(ui.colorRangeSlider, SIGNAL(hdrMaxChanged(int)),
            this, SLOT(setHdrMax(int)));
    connect(ui.colorRangeSlider, SIGNAL(gammaChanged(double)),
            this, SLOT(setGamma(double)));
    connect(ui.buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
            this, SLOT(resetColors()));
}

/* slot */
void DynamicRangeTool::resetColors()
{
    setGamma(1.0);
    setHdrMin(channelDataMin);
    setHdrMax(channelHdrMax);
}

/* slot */
void DynamicRangeTool::selectColor()
{
    QColor color = QColorDialog::getColor(channelColor,
                                          this,
                                          "Select new data channel color");
    if (color == channelColor) return;
    if ( ! color.isValid() ) return;
    channelColor = color;
    updateColor();
    emit channelColorChanged(currentChannelIndex, color.rgb());
}

void DynamicRangeTool::setColorModel(DataColorModel* modelParam)
{
    if (modelParam == dataColorModel) return;
    dataColorModel = modelParam;
    if (dataColorModel == NULL)
        return;

    initializeColors();

    connect(this, SIGNAL(channelColorChanged(int,int)),
            modelParam, SLOT(setChannelColor(int,int)));
    connect(this, SIGNAL(channelHdrRangeChanged(int,qreal,qreal)),
            modelParam, SLOT(setChannelHdrRange(int,qreal,qreal)));
    connect(this, SIGNAL(channelGammaChanged(int,qreal)),
            modelParam, SLOT(setChannelGamma(int,qreal)));
    connect(modelParam, SIGNAL(colorsInitialized()),
            this, SLOT(initializeColors()));
}

/* slot */
void DynamicRangeTool::initializeColors()
{
    if (dataColorModel == NULL)
        return;
    int numChannels = 0;
    {
        DataColorModel::Reader colorReader(*dataColorModel);
        if (dataColorModel->readerIsStale(colorReader))
            return;
        numChannels = colorReader.getNumberOfDataChannels();
        if (numChannels < 1)
            return;
    } // release read lock
    // Ensure that initial index is an actual color channel
    if (currentChannelIndex < 0)
        currentChannelIndex = 0;
    if (currentChannelIndex >= numChannels)
        currentChannelIndex = numChannels - 1;
    // Populate channel combobox
    // Ensure that channel box has an entry for each channel
    if (ui.comboBox->count() != numChannels) {
        ui.comboBox->clear();
        for(size_t c = 0; c < numChannels; ++c)
            ui.comboBox->addItem(QString("Channel %1").arg(c + 1));
    }
    ui.comboBox->setCurrentIndex(currentChannelIndex);
    updateChannelData();
}

void DynamicRangeTool::setChannel(int channelIndex)
{
    if (channelIndex == currentChannelIndex) return;
    currentChannelIndex = channelIndex;
    updateChannelData();
    emit channelChanged(currentChannelIndex);
}

void DynamicRangeTool::setHdrMin(int min)
{
    if (min == channelHdrMin) return;
    channelHdrMin = min;
    updateHdrWidgets();
    emit channelHdrRangeChanged(currentChannelIndex, channelHdrMin, channelHdrMax);
}

void DynamicRangeTool::setGamma(double gamma)
{
    if (gamma == channelGamma) return;
    channelGamma = gamma;
    updateHdrWidgets();
    emit channelGammaChanged(currentChannelIndex, channelGamma);
}

void DynamicRangeTool::setHdrMax(int max)
{
    if (max == channelHdrMax) return;
    channelHdrMax = max;
    updateHdrWidgets();
    // qDebug() << "emitting channelHdrRangeChanged" << currentChannelIndex << channelHdrMin << channelHdrMax;
    emit channelHdrRangeChanged(currentChannelIndex, channelHdrMin, channelHdrMax);
}

void DynamicRangeTool::updateDataRange(qreal min, qreal max)
{
    channelDataMin = min;
    channelDataMax = 2.0 * max; // allow dimming by going above actual max
    ui.channelMin_spinBox->setMinimum(channelDataMin);
    ui.channelMin_spinBox->setMaximum(channelDataMax);
    ui.channelMax_spinBox->setMinimum(channelDataMin);
    ui.channelMax_spinBox->setMaximum(channelDataMax);
    ui.colorRangeSlider->setDataRange(channelDataMin, channelDataMax);
}

void DynamicRangeTool::updateHdrWidgets()
{
    ui.channelGamma_doubleSpinBox->setValue(channelGamma);
    ui.channelMin_spinBox->setValue(channelHdrMin);
    ui.channelMax_spinBox->setValue(channelHdrMax);
    ui.colorRangeSlider->setHdrMin(channelHdrMin);
    ui.colorRangeSlider->setHdrMax(channelHdrMax);
    ui.colorRangeSlider->setGamma(channelGamma);
    update();
}

void DynamicRangeTool::updateColor()
{
    QPalette palette = ui.colorFrame->palette();
    palette.setColor(backgroundRole(), channelColor);
    ui.colorFrame->setPalette(palette);
}

void DynamicRangeTool::updateChannelData()
{
    if (! dataColorModel) return;
    // Draw channel color in box
    {
        DataColorModel::Reader colorReader(*dataColorModel);
        if (dataColorModel->readerIsStale(colorReader))
            return;
        size_t numChannels = colorReader.getNumberOfDataChannels();
        if (numChannels < 1)
            return; // can't do much with a color model with no colors
        if (currentChannelIndex < 0)
            return; // not a valid channel
        if (currentChannelIndex >= numChannels)
            return;
        channelColor = colorReader.getChannelColor(currentChannelIndex);
        channelHdrMin = colorReader.getChannelHdrMin(currentChannelIndex);
        channelHdrMax = colorReader.getChannelHdrMax(currentChannelIndex);
        channelGamma = colorReader.getChannelGamma(currentChannelIndex);
        channelDataMin = (int)colorReader.getChannelDataMin(currentChannelIndex);
        channelDataMax = (int)colorReader.getChannelDataMax(currentChannelIndex);
    }
    updateColor();
    updateDataRange(channelDataMin, channelDataMax);
    updateHdrWidgets();
    // TODO - everything else
}

