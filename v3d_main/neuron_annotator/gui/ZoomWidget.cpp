#include "ZoomWidget.h"
#include <QDebug>

ZoomWidget::ZoomWidget(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
    connect(dial, SIGNAL(valueChanged(int)),
            this, SLOT(onDialValueChanged(int)));
    connect(zoomSpinBox, SIGNAL(zoomValueChanged(qreal)),
            this, SLOT(onSpinBoxValueChanged(qreal)));
}

void ZoomWidget::setZoomValue(qreal doubleValue) {
    zoomSpinBox->setZoomValue(doubleValue);
}

void ZoomWidget::onDialValueChanged(int val)
{
    // qDebug() << "dial value changed " << val;
    int v0 = zoomSpinBox->value();
    // qDebug() << "box value = " << v0;
    // dial value is a remainder; an incremental value
    int boxVal = v0 % 100;
    if (val == boxVal)
        return; // no change needed, values are already in sync
    int dVal = val - boxVal;
    // assume the change is as small as possible
    while (dVal > 50) dVal -= 100;
    while (dVal < -50) dVal += 100;
    zoomSpinBox->setValue(v0 + dVal);
}

void ZoomWidget::onSpinBoxValueChanged(qreal val)
{
    // qDebug() << "SpinBox value changed " << val;
    int boxValue = zoomSpinBox->value();
    int dialValue = boxValue % 100;
    while(dialValue < 0) dialValue += 100;
    while(dialValue >= 100) dialValue -= 100;
    // qDebug() << "new dial value = " << dialValue;
    dial->setValue(dialValue);
    emit zoomValueChanged(val);
}

void ZoomWidget::reset() {zoomSpinBox->reset();}



