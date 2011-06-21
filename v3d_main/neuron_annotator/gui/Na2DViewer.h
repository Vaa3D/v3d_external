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
public:
    Na2DViewer(QWidget * parent = NULL) : QWidget(parent) {}

protected:
    QPixmap pixmap;
    QPainter painter;
    QTransform X_img_view;
    QTransform X_view_img;
};

#endif // NA2DVIEWER_H
