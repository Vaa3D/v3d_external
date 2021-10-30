#ifndef COLORRANGESLIDERWIDGET_H
#define COLORRANGESLIDERWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPolygonF>

class ColorRangeSliderWidget : public QWidget
{
    Q_OBJECT

public:

    enum Control {
        CONTROL_MIN,
        CONTROL_MAX,
        CONTROL_GAMMA,
        CONTROL_NONE
    };

    explicit ColorRangeSliderWidget(QWidget *parent = 0);
    virtual void paintEvent(QPaintEvent*);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    void setDataRange(qreal min, qreal max) {dataMin = min; dataMax = max;}
    void setHighlightControl(Control);

signals:
    void hdrMinChanged(int min);
    void hdrMaxChanged(int max);
    void gammaChanged(double gamma);

public slots:
    void setHdrMin(int min);
    void setHdrMax(int max);
    void setGamma(double gamma);

protected:
    int getMinPos() const;
    int getMaxPos() const;
    int getGammaPos() const;
    Control getControlAtPosition(int pos);
    void setControlPosition(Control control, int pos);

    QPainter painter;
    QPolygonF triangle;
    int dataMin;
    int dataMax;
    int hdrMin;
    int hdrMax;
    double gamma;
    Control highlightControl;
    Control draggingControl;
};

#endif // COLORRANGESLIDERWIDGET_H
