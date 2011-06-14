#include "ZoomWidget.h"

ZoomWidget::ZoomWidget(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
    connect(zoomSpinBox, SIGNAL(zoomValueChanged(qreal)),
            this, SIGNAL(zoomValueChanged(qreal)));
}

void ZoomWidget::setZoomValue(qreal doubleValue) {
    zoomSpinBox->setZoomValue(doubleValue);
}

void ZoomWidget::reset() {zoomSpinBox->reset();}



