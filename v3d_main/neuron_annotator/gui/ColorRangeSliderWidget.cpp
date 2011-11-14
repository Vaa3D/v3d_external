#include "ColorRangeSliderWidget.h"
#include <cmath>
#include <QMouseEvent>
#include <QDebug>
#include <cstdlib>

ColorRangeSliderWidget::ColorRangeSliderWidget(QWidget *parent)
    : QWidget(parent)
    , dataMin(0)
    , dataMax(0)
    , hdrMin(0)
    , hdrMax(0)
    , gamma(0)
    , highlightControl(CONTROL_NONE)
{
    triangle << QPointF(0, 0) << QPointF(-8, 12) << QPointF(8, 12);
    setMouseTracking(true);
}

void ColorRangeSliderWidget::setHighlightControl(Control c)
{
    if (highlightControl == c) return; // no change
    highlightControl = c;
    update();
}

ColorRangeSliderWidget::Control ColorRangeSliderWidget::getControlAtPosition(int pos)
{
    int minPos = getMinPos();
    int gammaPos = getGammaPos();
    int maxPos = getMaxPos();
    // Apparently std::abs(int) is in <cstdlib>, not <cmath> on Linux.  Weird.
    if (std::abs((pos - gammaPos)) < 10)
    {
        // qDebug() << "gamma";
        return CONTROL_GAMMA;
    }
    else if (std::abs(pos - minPos) < 10)
    {
        // qDebug() << "hdrMin";
        return CONTROL_MIN;
    }
    else if (std::abs(pos - maxPos) < 10)
    {
        // qDebug() << "hdrMax";
        return CONTROL_MAX;
    }
    else
    {
        return CONTROL_NONE;
    }
}

/* virtual */
void ColorRangeSliderWidget::mousePressEvent(QMouseEvent * mouseEvent)
{
    draggingControl = getControlAtPosition(mouseEvent->x());
}

/* virtual */
void ColorRangeSliderWidget::mouseReleaseEvent(QMouseEvent * event)
{
    draggingControl = CONTROL_NONE;
}

/* virtual */
void ColorRangeSliderWidget::mouseMoveEvent(QMouseEvent * mouseEvent)
{
    // qDebug() << "mouse move";
    if (Qt::NoButton == mouseEvent->buttons())
    {
        // hover - highlight drag triangle
        setHighlightControl(getControlAtPosition(mouseEvent->x()));
    }
    else // dragging
    {
        setControlPosition(draggingControl, mouseEvent->x());
    }
}

void ColorRangeSliderWidget::setControlPosition(Control control, int pos)
{
    if (CONTROL_NONE == control) return;
    if (pos < 0) pos = 0;
    if (pos > width()) pos = width();
    qreal dataRange = dataMax - dataMin;
    if (dataRange <= 0) return;
    double newGamma;
    switch(control)
    {
    case CONTROL_MIN:
        if (pos == getMinPos()) return; // no change
        pos = std::min(pos, getMaxPos() - 10); // too close to max
        setHdrMin(dataMin + (pos / (qreal)width()) * dataRange);
        break;
    case CONTROL_MAX:
        if (pos == getMaxPos()) return; // no change
        pos = std::max(pos, getMinPos() + 10); // too close to min
        setHdrMax(dataMin + (pos / (qreal)width()) * dataRange);
        break;
    case CONTROL_GAMMA:
        if (pos == getGammaPos()) return; // no change
        pos = std::min(pos, getMaxPos() - 5); // too close to max
        pos = std::max(pos, getMinPos() + 5); // too close to min
        newGamma = (pos - getMinPos()) / (double)(getMaxPos() - getMinPos());
        setGamma( std::log(0.5) / std::log(newGamma) );
        break;
    default:
        qDebug() << "TODO - implement control dragging";
        break;
    }
}

void ColorRangeSliderWidget::setHdrMin(int min)
{
    if(min == hdrMin) return;
    if(min < dataMin) return;
    if(min > dataMax) return;
    hdrMin = min;
    update();
    emit(hdrMinChanged(hdrMin));
}

void ColorRangeSliderWidget::setHdrMax(int max)
{
    if(max == hdrMax) return;
    if(max < dataMin) return;
    if(max > dataMax) return;
    hdrMax = max;
    update();
    emit(hdrMaxChanged(hdrMax));
}

void ColorRangeSliderWidget::setGamma(double gammaParam)
{
    if (gammaParam == gamma) return;
    if (gammaParam < 0.125) return;
    if (gammaParam > 8.0) return;
    gamma = gammaParam;
    update();
    emit(gammaChanged(gamma));
}

int ColorRangeSliderWidget::getMinPos() const
{
    qreal relMin = 0.0;
    qreal dataRange = dataMax - dataMin;
    if (dataRange > 0)
        relMin = (hdrMin - dataMin) / dataRange;
    return (int)(relMin * width());
}

int ColorRangeSliderWidget::getMaxPos() const
{
    qreal relMax = 1.0;
    qreal dataRange = dataMax - dataMin;
    if (dataRange > 0)
        relMax = (hdrMax - dataMin) / dataRange;
    return (int)(relMax * width());
}

int ColorRangeSliderWidget::getGammaPos() const
{
    qreal relGamma = 0.5;
    qreal dataRange = dataMax - dataMin;
    if (dataRange > 0)
    {
        relGamma = std::pow(0.5, 1.0/gamma); // output brightness is average of max, min
        int min = getMinPos();
        int max = getMaxPos();
        relGamma = min + relGamma * (max - min);
    }
    return (int)(relGamma);
}

/* virtual */
void ColorRangeSliderWidget::paintEvent(QPaintEvent* paintEvent)
{
    painter.begin(this);

    QBrush highlightBrush(Qt::yellow);

    // Triangle at min
    painter.resetTransform();
    painter.translate(getMinPos(), 0);
    if (CONTROL_MIN == highlightControl) painter.setBrush(highlightBrush);
    else painter.setBrush(Qt::NoBrush);
    painter.drawConvexPolygon(triangle);

    // Triangle at gamma
    painter.resetTransform();
    painter.translate(getGammaPos(), 0);
    if (CONTROL_GAMMA == highlightControl) painter.setBrush(highlightBrush);
    else painter.setBrush(Qt::NoBrush);
    painter.drawConvexPolygon(triangle);

    // Triangle at max
    painter.resetTransform();
    painter.translate(getMaxPos(), 0);
    if (CONTROL_MAX == highlightControl) painter.setBrush(highlightBrush);
    else painter.setBrush(Qt::NoBrush);
    painter.drawConvexPolygon(triangle);

    painter.end();
}

