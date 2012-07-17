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
    void setColorModel(DataColorModel*);
    void updateChannelData();

signals:
    void channelChanged(int index);
    void channelColorChanged(int index, /*QRgb*/ int color);
    void channelHdrRangeChanged(int index, qreal min, qreal max);
    void channelGammaChanged(int index, qreal gamma);

public slots:
    void setChannel(int channelIndex);
    void selectColor();
    void setHdrMin(int min);
    void setHdrMax(int max);
    void setGamma(double gamma);
    void resetColors();
    void initializeColors();

protected:
    void updateHdrWidgets();
    void updateDataRange(qreal min, qreal max);
    void updateColor();

private:
    Ui::DynamicRangeTool ui;
    int currentChannelIndex;
    DataColorModel* dataColorModel;
    QColor channelColor;
    int channelHdrMin;
    int channelHdrMax;
    int channelDataMin;
    int channelDataMax;
    double channelGamma;
};

#endif // DYNAMICRANGETOOL_H
