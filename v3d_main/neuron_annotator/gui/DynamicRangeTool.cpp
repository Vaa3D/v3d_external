#include "DynamicRangeTool.h"

DynamicRangeTool::DynamicRangeTool(QWidget *parent)
    : QDialog(parent)
    , dataColorModel(NULL)
    , currentChannelIndex(-1)
{
    ui.setupUi(this);
    connect(ui.comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setChannel(int)));
    connect(ui.changeColorButton, SIGNAL(clicked()),
            this, SLOT(selectColor()));
}

/* slot */
void DynamicRangeTool::selectColor()
{
    QColor color = QColorDialog::getColor(currentChannelColor,
                                          this,
                                          "Select new data channel color");
    if (color == currentChannelColor) return;
    if ( ! color.isValid() ) return;
    emit channelColorChanged(currentChannelIndex, color.rgb());
}

void DynamicRangeTool::setColorModel(DataColorModel& modelParam)
{
    if (&modelParam == dataColorModel) return;
    dataColorModel = &modelParam;

    size_t numChannels = 0;
    {
        DataColorModel::Reader colorReader(*dataColorModel);
        numChannels = colorReader.getNumberOfDataChannels();
    } // release read lock
    // Populate channel combobox
    ui.comboBox->clear();
    for(size_t c = 0; c < numChannels; ++c)
        ui.comboBox->addItem(QString("Channel %1").arg(c + 1));

    if ( (currentChannelIndex < 0) || (currentChannelIndex >= numChannels) )
        currentChannelIndex = 0;
    updateChannelData();

    connect(this, SIGNAL(channelColorChanged(int,int)),
            &modelParam, SLOT(setChannelColor(int,int)));
}

void DynamicRangeTool::setChannel(int channelIndex)
{
    if (channelIndex == currentChannelIndex) return;
    currentChannelIndex = channelIndex;
    updateChannelData();
    emit channelChanged(currentChannelIndex);
}

void DynamicRangeTool::updateChannelData()
{
    if (! dataColorModel) return;
    // Draw channel color in box
    {
       DataColorModel::Reader colorReader(*dataColorModel);
       currentChannelColor = colorReader.getChannelColor(currentChannelIndex);
    }
    QPalette palette = ui.colorFrame->palette();
    palette.setColor(backgroundRole(), currentChannelColor);
    ui.colorFrame->setPalette(palette);
    // TODO - everything else
}

