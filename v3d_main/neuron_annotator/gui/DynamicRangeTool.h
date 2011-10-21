#ifndef DYNAMICRANGETOOL_H
#define DYNAMICRANGETOOL_H

#include "ui_DynamicRangeTool.h"
#include "../data_model/DataColorModel.h"
#include <QDialog>

class DynamicRangeTool : public QDialog
{
    Q_OBJECT
public:
    explicit DynamicRangeTool(QWidget *parent = 0);
    void setColorModel(DataColorModel&);
    void updateChannelData();

signals:
    void channelChanged(int index);
    void channelColorChanged(int index, /*QRgb*/ int color);
    void channelHdrRangeChanged(int index, qreal min, qreal max);
    void channelGammaChanged(int index, qreal gamma);

public slots:
    void setChannel(int channelIndex);
    void selectColor();

private:
    Ui::DynamicRangeTool ui;
    size_t currentChannelIndex;
    DataColorModel* dataColorModel;
    QColor currentChannelColor;
};

#endif // DYNAMICRANGETOOL_H
