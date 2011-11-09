#ifndef ZOOMWIDGET_H
#define ZOOMWIDGET_H

#include <QWidget>
#include "ui_ZoomWidget.h"

class ZoomWidget : public QWidget, public Ui::ZoomWidget
{
    Q_OBJECT
public:
    explicit ZoomWidget(QWidget *parent = 0);

signals:
    void zoomValueChanged(qreal zoomValue);

public slots:
    void setZoomValue(qreal doubleValue);
    void reset();

protected slots:
    void onDialValueChanged(int);
    void onSpinBoxValueChanged(qreal);
};

#endif // ZOOMWIDGET_H
