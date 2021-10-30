#include "Na2DViewer.h"

Na2DViewer::Na2DViewer(QWidget * parent /* = NULL */)
    : QWidget(parent)
{
    setMouseTracking(true); // respond to mouse hover events
}

void Na2DViewer::paintCrosshair(QPainter& painter)
{
    float scale = defaultScale * cameraModel.scale();
    QBrush brush1(Qt::black);
    QBrush brush2(QColor(255, 255, 180));
    QPen pen1(brush1, 2.0/scale);
    QPen pen2(brush2, 1.0/scale);
    // qDebug() << "paint crosshair";
    // Q: Why all this complicated math instead of just [width()/2, height()/2]?
    // A: This helps debug/document placement of image focus
    qreal w2 = (pixmap.width() - 1.0) / 2.0; // origin at pixel center, not corner
    qreal h2 = (pixmap.height() - 1.0) / 2.0; // origin at pixel center, not corner
    qreal cx = w2 + flip_X * (cameraModel.focus().x() - w2) + 0.5;
    qreal cy = h2 + flip_Y * (cameraModel.focus().y() - h2) + 0.5;
    QPointF f(cx, cy);
    QPointF dx1(4.0 / scale, 0);
    QPointF dy1(0, 4.0 / scale);
    QPointF dx2(10.0 / scale, 0); // crosshair size is ten pixels
    QPointF dy2(0, 10.0 / scale);
    painter.setPen(pen1);
    painter.drawLine(f + dx1, f + dx2);
    painter.drawLine(f - dx1, f - dx2);
    painter.drawLine(f + dy1, f + dy2);
    painter.drawLine(f - dy1, f - dy2);
    painter.setPen(pen2);
    painter.drawLine(f + dx1, f + dx2);
    painter.drawLine(f - dx1, f - dx2);
    painter.drawLine(f + dy1, f + dy2);
    painter.drawLine(f - dy1, f - dy2);
}

    void Na2DViewer::transformPainterToCurrentCamera(QPainter& painter)
{
    // adjust painter coordinate system to place image correctly
    float scale = defaultScale * cameraModel.scale();
    // origin at pixel center, not corner
    qreal w2 = (pixmap.width() - 1.0) / 2.0;
    qreal h2 = (pixmap.height() - 1.0) / 2.0;
    qreal tx = w2 + flip_X * (cameraModel.focus().x() - w2) + 0.5;
    qreal ty = h2 + flip_Y * (cameraModel.focus().y() - h2) + 0.5;
    painter.translate(width()/2.0 - tx * scale, height()/2.0 - ty * scale);
    painter.scale(scale, scale);

    // I want to convert screen coordinates to image coordinates;
    // The QPainter object knows this transformation.
    // This nomenclature for the transforms, e.g. X_view_img , comes from the
    // advanced dynamics community at Stanford, specifically the disciples of Thomas Kane.
    X_view_img = painter.transform();
    X_img_view = painter.transform().inverted();
}

// Mouse drag
/* virtual */
void Na2DViewer::mouseMoveEvent(QMouseEvent * event)
{
    // Notice statement "setMouseTracking(true)" in constructor.
    // hover
    if (Qt::NoButton == event->buttons())
    {
        bMouseIsDragging = false;
        return;
    }

    bool isTranslateDrag = ( (event->buttons() & Qt::LeftButton)
                          || (event->buttons() & Qt::MidButton) );

    if (! isTranslateDrag ) {
        // qDebug() << "Not left button...";
        bMouseIsDragging = false;
        return;
    }

    int dx = event->pos().x() - oldDragX;
    int dy = event->pos().y() - oldDragY;
    oldDragX = event->pos().x();
    oldDragY = event->pos().y();

    // Do nothing until the second drag point is reached
    if (!bMouseIsDragging) {
        bMouseIsDragging = true;
        return;
    }

    emit mouseLeftDragEvent(dx, dy, event->pos());
}

/* virtual */
void Na2DViewer::mousePressEvent(QMouseEvent * event)
{
    mouseClickManager.mousePressEvent(event);
    // Consider starting a translation drag operation
    if (event->buttons() & Qt::LeftButton) {
        bMouseIsDragging = true;
        oldDragX = event->pos().x();
        oldDragY = event->pos().y();
    }
    else {
        bMouseIsDragging = false;
    }
}

/* virtual */
void Na2DViewer::mouseReleaseEvent(QMouseEvent * event)
{
    // End any drag event
    bMouseIsDragging = false;
    mouseClickManager.mouseReleaseEvent(event);
}

void Na2DViewer::mouseDoubleClickEvent(QMouseEvent * event)
{
    mouseClickManager.mouseDoubleClickEvent(event);
    if (event->button() != Qt::LeftButton)
        return;
    double dx = event->pos().x() - width()/2.0;
    double dy = event->pos().y() - height()/2.0;
    translateImage(-dx, -dy);
}


void Na2DViewer::translateImage(int dx, int dy)
{
    if (!dx && !dy) return;
    float scale = defaultScale * cameraModel.scale();
    Vector3D newFocus = cameraModel.focus() - Vector3D(flip_X * dx/scale, flip_Y * dy/scale, 0);
    // cerr << newFocus << __LINE__ << __FILE__;
    cameraModel.setFocus(newFocus);
    update();
}

void Na2DViewer::updateDefaultScale()
{
    float screenWidth = width();
    float screenHeight = height();
    float objectWidth = pixmap.size().width();
    float objectHeight = pixmap.size().height();

    if (screenWidth < 1) return;
    if (screenHeight < 1) return;
    if (objectWidth < 1) return;
    if (objectHeight < 1) return;
    float scaleX = screenWidth / objectWidth;
    float scaleY = screenHeight / objectHeight;
    // fit whole pixmap in window, with bars if necessary
    defaultScale = scaleX > scaleY ? scaleY : scaleX;
}




