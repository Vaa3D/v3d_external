#ifndef NA2DVIEWER_H
#define NA2DVIEWER_H

#include <QWidget>
#include <QPainter>
#include <QTransform>
#include <QPixmap>
#include "NaViewer.h"

// Na2DViewer is the common base class of NaLargeMipWidget and NaZStackWidget
class Na2DViewer : public QWidget, public NaViewer
{
   // Q_OBJECT

public:
    Na2DViewer(QWidget * parent = NULL);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    // double click to center
    virtual void mouseDoubleClickEvent(QMouseEvent * event);

    // Screen Y-axis is flipped with respect to data Y-axis in V3D 3D viewer
    static const int flip_X =  1;
    static const int flip_Y = -1;
    static const int flip_Z = -1;

//signals:
    void mouseLeftDragEvent(int dx, int dy, QPoint pos);

//public slots:
    void showCrosshair(bool b) {NaViewer::showCrosshair(b); update();}
    virtual void invalidate() {NaViewer::invalidate();}

//protected slots:
    void translateImage(int dx, int dy);

protected:
    void transformPainterToCurrentCamera(QPainter& painter);
    void updateDefaultScale();
    void paintCrosshair(QPainter& painter);

    QPixmap pixmap;
    QPainter painter;
    QTransform X_img_view;
    QTransform X_view_img;
};

#endif // NA2DVIEWER_H
