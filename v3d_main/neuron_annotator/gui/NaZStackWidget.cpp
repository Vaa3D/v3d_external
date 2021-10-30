#include "NaZStackWidget.h"
#include <QPaintEvent>
#include <QDebug>
#include <QPainter>
#include <cmath>
#include <iostream>
#include "../data_model/ZSliceColors.h"
#include "../DataFlowModel.h"
#include "../utility/FooDebug.h"

using namespace std;

#define INF 1e9

NaZStackWidget::NaZStackWidget(QWidget * parent)
    : Na2DViewer(parent)
    , zSliceColors(NULL)
    , volumeData(NULL)
    , cur_z(-1)
    , sx(0), sy(0), sz(0), sc(0)
    , runHDRFILTER(false)
    , viewerContextMenu(NULL)
    , neuronContextMenu(NULL)
    , start_x(0), start_y(0)
    , end_x(20), end_y(20)
    , startMousePos(start_x, start_y)
    , endMousePos(end_x, end_y)
{
    translateMouse_scale = 1;

    b_mouseleft = false;
    b_mouseright = false;
    b_mousemove = false;

    resetHdrBox();

    recNum = 0;
    roiDrawed = false;
    recCopy = 0;

    cur_c = COLOR_RED;
    pre_c = cur_c;
    setFocusPolicy(Qt::ClickFocus);
    updateCursor();

    connect(this, SIGNAL(curColorChannelChanged(NaZStackWidget::Color)), this, SLOT(updateHDRView()));
    connect(this, SIGNAL(mouseLeftDragEvent(int, int, QPoint)),
            this, SLOT(onMouseLeftDragEvent(int, int, QPoint)));
    connect(&cameraModel, SIGNAL(focusChanged(Vector3D)),
            this, SLOT(update()));
    invalidate();
}

NaZStackWidget::~NaZStackWidget() {}

void NaZStackWidget::resetHdrBox()
{
    bMouseCurorIn = false;
    bMouseDone = false;

    for(int i=0; i<NCLRCHNNL; i++)
    {
        recMousePos[i] = false;
        recZ[i] = 0;
        hdrfiltered[i] = false;
    }

    setRedChannel(); // by default
    setHDRCheckState(false);

    m_square_pos.setX(sx/2);
    m_square_pos.setY(sy/2);

    setHdrBoxSize(25);
}

/* virtual */
void NaZStackWidget::setDataFlowModel(const DataFlowModel* dataFlowModelParam)
{
    if (dataFlowModel == dataFlowModelParam)
        return;

    NaViewer::setDataFlowModel(dataFlowModelParam);

    if (NULL == dataFlowModel)
    {
        setZSliceColors(NULL);
        setVolumeData(NULL);
        return;
    }

    setZSliceColors(&dataFlowModel->getZSliceColors());
    setVolumeData(&dataFlowModel->getVolumeData());

    resetHdrBox();
}

void NaZStackWidget::setContextMenus(QMenu* viewerMenuParam, NeuronContextMenu* neuronMenuParam)
{
    if (viewerMenuParam) {
        viewerContextMenu = viewerMenuParam;
    }
    if (neuronMenuParam) {
        neuronContextMenu = neuronMenuParam;
    }
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
}

/* slot */
void NaZStackWidget::showContextMenu(QPoint point)
{
    // Myers index (GUI index) is one less than the volume label field index
    int neuronMyersIx = neuronAt(point);
    // qDebug() << "context menu for neuron" << neuronIx;
    // -1 means click outside of volume
    // 0 means background
    // >=1 means neuron fragment with  index neuronIx-1
    if (neuronMyersIx >= 0) { // neuron clicked
        bool neuronIsVisible = true;
        if (dataFlowModel != NULL)
        {
            NeuronSelectionModel::Reader selectionReader(
                    dataFlowModel->getNeuronSelectionModel());
            if (selectionReader.hasReadLock())
                neuronIsVisible = selectionReader.getMaskStatusList()[neuronMyersIx];
        }
        neuronContextMenu->exec(mapToGlobal(point), neuronMyersIx, neuronIsVisible);
    }
    else {
        // non neuron case
        viewerContextMenu->exec(mapToGlobal(point));
    }
}

int NaZStackWidget::neuronAt(const QPoint& point) const
{
    QPointF v_img = X_img_view * QPointF(point);
    int x = v_img.x();
    int y = v_img.y();
    int z = cur_z;
    int neuronIx = -1;
    if (! volumeData) return neuronIx;
    NaVolumeData::Reader volumeReader(*volumeData);
    if ( !volumeReader.hasReadLock()) return neuronIx;
    const Image4DProxy<My4DImage>& neuronProxy = volumeReader.getNeuronMaskProxy();
    if (   (x < 0) || (x >= neuronProxy.sx)
        || (y < 0) || (y >= neuronProxy.sy)
        || (z < 0) || (z >= neuronProxy.sz) )
    return neuronIx;
    neuronIx = neuronProxy.value_at(x, y, z, 0) - 1;
    return neuronIx;
}

void NaZStackWidget::setZSliceColors(const ZSliceColors * zSliceColorsParam)
{
    zSliceColors = zSliceColorsParam;
    if (NULL == zSliceColors)
        return;
    connect(zSliceColors, SIGNAL(dataChanged()),
            this, SLOT(updatePixmap()));
    connect(zSliceColors, SIGNAL(invalidated()),
            this, SLOT(invalidate()));
}

void NaZStackWidget::setVolumeData(const NaVolumeData * volumeDataParam)
{
    if (volumeData == volumeDataParam)
        return; // no change
    volumeData = volumeDataParam;
    if (NULL == volumeData) {
        sx = sy = sz = sc = 0;
        return;
    }
    connect(volumeData, SIGNAL(dataChanged()),
            this, SLOT(updateVolumeParameters()));
}

// Convert between image data coordinates and screen coordinates.
//  * scale by scale_x, scale_y
//  * translate so image_focus_[xy] is in center of viewport
//  * shift by one pixel so image[0,0] maps to viewport[1,1]
QPointF NaZStackWidget::viewportXYToImageXY(float vx, float vy)
{
    return X_img_view * QPointF(vx, vy);
}

QPoint NaZStackWidget::viewportXYToImageXY(const QPoint& vp)
{
    QPointF ipf = viewportXYToImageXY(vp.x(), vp.y());
    return QPoint(int(ipf.x()), int(ipf.y()));
}

QPointF NaZStackWidget::imageXYToViewportXY(float ix, float iy)
{
    return X_view_img * QPointF(ix, iy);
}

/* slot */
void NaZStackWidget::updateVolumeParameters()
{
    if (! volumeData) return;
    NaVolumeData::Reader volumeReader(*volumeData);
    if (! volumeReader.hasReadLock()) return;
    const Image4DProxy<My4DImage>& volProxy = volumeReader.getOriginalImageProxy();

    bool bChanged = false;
    if (sx != volProxy.sx) bChanged = true;
    if (sy != volProxy.sy) bChanged = true;

    sx = volProxy.sx;
    sy = volProxy.sy;
    sz = volProxy.sz;
    sc = volProxy.sc;
    if (volumeReader.hasReferenceImage())
        sc += 1;// +1, reference channel too

    min_roi.assign(sc, INF);
    max_roi.assign(sc, -INF);
    if (bChanged)
        setSquarePos(QPointF(sx/2, sy/2));
}

void NaZStackWidget::paintEvent(QPaintEvent *event)
{
    // If the zslice viewer is not tied to current data, paint it all plain gray.
    bool paintGray = false;
    if (NULL == zSliceColors)
        paintGray = true;
    else if (! zSliceColors->representsActualData())
        paintGray = true;

    if (paintGray) {
        painter.begin(this);
        painter.fillRect(0, 0, width(), height(), Qt::gray);
        painter.end();
        return;
    }

    updateDefaultScale();
    painter.begin(this);

    // Color background black
    painter.fillRect(0, 0, width(), height(), Qt::black);

    painter.setRenderHint(QPainter::Antialiasing);
    float scale = defaultScale * cameraModel.scale();
    bool showNumbers = (scale > 40); // 40 display pixels per image pixel
    if (! showNumbers)
        // smoothing is nicer on the eyes at low zoom levels, but confusing when numbers  are shown.
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

    transformPainterToCurrentCamera(painter);

    QPointF origin(0, 0);
    painter.drawPixmap(origin, pixmap);

    if (bPaintCrosshair) paintCrosshair(painter);

    if (showNumbers) { // 40 display pixels per image pixel
        paintIntensityNumerals(painter);
    }

    if(runHDRFILTER) { // Z Stack
        // ROI
        drawROI(&painter);
    }

    painter.end();
}

void NaZStackWidget::paintIntensityNumerals(QPainter& painter)
{
    if (cur_z < 0) return;
    if (! zSliceColors) return;
    ZSliceColors::Reader zReader(*zSliceColors);
    if (! zReader.hasReadLock()) return;
    const QImage * displayImage = zReader.getImage();
    if (! displayImage) return;

    if (! volumeData) return;
    NaVolumeData::Reader volumeReader(*volumeData);
    if (! volumeReader.hasReadLock()) return;
    const Image4DProxy<My4DImage>& imgProxy = volumeReader.getOriginalImageProxy();

    // qDebug() << "numerals";
    QPointF v_img_upleft = X_img_view * painter.viewport().topLeft();
    QPointF v_img_downright = X_img_view * painter.viewport().bottomRight();
    // qDebug() << v_img_upleft;
    // qDebug() << v_img_downright;

    // clear transform for text rendering, otherwise font size is harder to manage
    painter.resetTransform();

    QFont font = painter.font();
    float scale = defaultScale * cameraModel.scale();
    font.setPixelSize(scale/4.0);
    font.setStyleStrategy(QFont::NoAntialias); // text rendering can be slow
    painter.setFont(font);

    // qDebug() << "nColumns = " << mipImage->originalData.nColumns();
    // qDebug() << "nRows = " << mipImage->originalData.nRows();

    // Iterate over only the image pixels that are visible
    int nC = imgProxy.sc;
    float lineHeight = scale / (nC + 1.0);
    for (int x = int(v_img_upleft.x() - 0.5); x <= int(v_img_downright.x() + 0.5); ++x) {
        // qDebug() << "x = " << x;
        if (x < 0)
            continue;
        if (x >= displayImage->width())
            continue;
        for (int y = int(v_img_upleft.y() - 0.5); y <= int(v_img_downright.y() + 0.5); ++y) {
            // qDebug() << "y = " << y;
            if (y < 0)
                continue;
            if (y >= displayImage->height())
                continue;
            // Transform image pixel coordinates back to viewport coordinates
            QPointF v = X_view_img * QPointF(x, y);
            // qDebug() << x << ", " << y << "; " << v.x() << ", " << v.y();
            // Print original data intensity, not displayed intensity
            // But choose font color based on displayed intensity.
            unsigned int red = qRed(displayImage->pixel(x, y));
            unsigned int green = qGreen(displayImage->pixel(x, y));
            unsigned int blue = qBlue(displayImage->pixel(x, y));
            // human color perception is important here
            float displayIntensity = 0.30 * red + 0.58 * green + 0.12 * blue;
            if (displayIntensity < 128)
                painter.setPen(Qt::white);
            else
                painter.setPen(Qt::black);

            // Write a neat little column of numbers inside each pixel
            for (int c = 0; c < nC; ++c) {
                double val = imgProxy.value_at(x, y, cur_z, c);
                painter.drawText(QRectF(v.x(), v.y() + (c + 0.5) * lineHeight, scale, lineHeight),
                                 Qt::AlignHCenter | Qt::AlignVCenter,
                                 QString("%1").arg(val));
            }
        }
    }
    // restore coordinate system
    transformPainterToCurrentCamera(painter);
}

void NaZStackWidget::drawROI(QPainter *painter)
{
    if (bMouseCurorIn || recNum || roiDrawed)
    {
        painter->setPen(Qt::yellow );
        painter->setBrush(Qt::NoBrush);

        if(b_mousemove)
        {
            if(b_mouseright)
            {
                QRect r( startMousePosR, curMousePosR );
                painter->drawRect( r );
            }
            else if(b_mouseleft)
            {
                if(bMouseDone)
                {
                    QSizeF sz(hdrBoxSize, hdrBoxSize);
                    QRectF square = rectangle_around(m_square_pos, sz);
                    painter->drawRect(square);
                }
            }
        }
        else if(bMouseDone)
        {
            QSizeF sz(hdrBoxSize, hdrBoxSize);
            QRectF square = rectangle_around(m_square_pos,sz);
            painter->drawRect(square);
        }

        if( checkROIchanged() )
        {
            do_HDRfilter(); // HDR filtering
        }
    }
    else
    {
        roiDrawed = true;

        // init a square
        cx = sx/2;
        cy = sy/2;

        setHdrBoxSize(25);

        int temp_cr = (hdrBoxSize - 1) / 2;
        start_x = cx - temp_cr;
        end_x = cx + temp_cr;

        if(start_x<0) start_x = 0;
        if(end_x>=sx) end_x = sx-1;

        start_y = cy - temp_cr;
        end_y = cy + temp_cr;

        if(start_y<0) start_y = 0;
        if(end_y>=sy) end_y = sy-1;

        startMousePos.setX(start_x);
        startMousePos.setY(start_y);

        endMousePos.setX(end_x);
        endMousePos.setY(end_y);

        m_square_pos.setX( startMousePos.x() + (endMousePos.x() - startMousePos.x())/2 );
        m_square_pos.setY( startMousePos.y() + (endMousePos.y() - startMousePos.y())/2 );

        QSizeF sz(hdrBoxSize, hdrBoxSize);
        QRectF square = rectangle_around(m_square_pos,sz);
        painter->drawRect(square);
    }

}

void NaZStackWidget::enterEvent (QEvent * e) // mouse found
{
    bMouseCurorIn = true;
    update();
}

void NaZStackWidget::mouseLeftButtonPressEvent(QMouseEvent *e) // mouse left button
{
    if (bMouseCurorIn)
    {
        b_mouseleft = true;
        b_mouseright = false;
        b_mousemove = false;
        bMouseDone = true;

        startMousePosL = e->pos();

        startMousePosL = viewportXYToImageXY(startMousePosL);

        QRectF square = rectangle_around(m_square_pos);

        if (square.contains(startMousePosL))
        {
            m_offset = square.center() - startMousePosL;
        }

        // setCursor(Qt::CrossCursor);
        update();
    }
}

void NaZStackWidget::mouseRightButtonPressEvent(QMouseEvent *e) // mouse right button
{
    if (bMouseCurorIn)
    {
        b_mouseright = true;
        b_mouseleft = false;
        b_mousemove = false;
        bMouseDone = false;

        startMousePosR = e->pos(); //

        startMousePosR = viewportXYToImageXY(startMousePosR);

        // setCursor(Qt::CrossCursor);
        update();
    }
}

int NaZStackWidget::getCurrentZSlice() {
    return cur_z; // 0-based index for API; 0-based internal
}

int NaZStackWidget::getCurrentBoxSize() {
    return hdrBoxSize; // radius
}

void NaZStackWidget::setCurrentZSlice(int slice)
{
    if (slice < 0) return; // value too small
    if (cur_z == slice) return; // no change; ignore
    // Might want to update volume information just-in-time
    if ( (sz < 1) && (NULL != volumeData) )
        updateVolumeParameters();
    if (slice >= sz) return; // value too big
    cur_z = slice;
    // qDebug() << "setting z slice to " << slice;

    updateHDRView();

    // Consider updating camera focus to match latest z-slice
    const Vector3D& f = cameraModel.focus();
    int camZ = int(floor(f.z() + 0.5));
    float midZ = (sz - 1) / 2.0f;
    int flip_cur_z = int(midZ + flip_Z * (cur_z - midZ));
    if (flip_cur_z != camZ) {
        Vector3D newFocus(f.x(), f.y(), flip_cur_z);
        // cerr << newFocus << __LINE__ << __FILE__ << "; " << slice << ", " <<  f.z() << ", " << camZ << ", " << midZ << ", " << sz << ", " << flip_cur_z << endl;
        cameraModel.setFocus(newFocus);
    }

    emit curZsliceChanged(slice);
}

void NaZStackWidget::wheelEvent(QWheelEvent * e) // mouse wheel
{
    b_mousemove = false;

    int numDegrees = e->delta()/8;
    int numTicks = numDegrees/15;
    // Some mouse wheels have a finer scroll increment; at least move a bit.
    if ((e->delta() != 0) && (numTicks == 0))
        numTicks = e->delta() > 0 ? 1 : -1;

    // qDebug() << "wheel";
    // cerr << "wheel event " << __LINE__ << __FILE__ << endl;
    // CMB 12-Aug-2011 - reverse sign to make direction match scroll wheel on scroll bar
    if (e->modifiers() & Qt::ShiftModifier) // shift-scroll to zoom
    {
        // setCurrentZSlice(getCurrentZSlice() - numTicks); // z-scan
        wheelZoom(e->delta()); // zoom
    }
    else if (runHDRFILTER && (
               (e->modifiers() & Qt::AltModifier)
            || (e->modifiers() & Qt::MetaModifier)
            || (e->modifiers() & Qt::ControlModifier)
            ))
    { // ctrl-scroll to shift HDR color/data channel
        // qDebug() << "modified scroll" << sc;
        const int min_color = COLOR_RED;
        // const int max_color = min((int)sc, (int)COLOR_BLUE);
        const int max_color = (int)sc;
        if (max_color <= 1) return; // no other colors to change to
        int old_color = cur_c;
        int new_color = old_color;
        if (numTicks == 0)
            return;
        else if (numTicks < 0)
            --new_color;
        else
            ++new_color;
        // wrap around channel-list boundaries
        while (new_color > max_color)
            new_color -= max_color;
        while (new_color < min_color)
            new_color += max_color;
        setColorChannel((Color)new_color);
        // qDebug() << new_color << __FILE__ << __LINE__;
    }
    else // regular-scroll to z-scan
    {
        setCurrentZSlice(getCurrentZSlice() - numTicks); // z-scan
        // wheelZoom(e->delta());
    }
}

void NaZStackWidget::onMouseLeftDragEvent(int dx, int dy, QPoint pos) {
    // translate image when not in HDR mode
    if (runHDRFILTER) return;
    translateImage(dx, dy);
}

static bool downRightDragR = true;
void NaZStackWidget::mouseMoveEvent (QMouseEvent * e) // mouse move
{
    super::mouseMoveEvent(e);
    if (Qt::NoButton == e->buttons())
    {
        // hover, not drag
        // Hover to show ([neuron], x, y, z, value) in status bar
        QPointF v_img = X_img_view * QPointF(e->pos());
        int x = int( floor(v_img.x()) );
        int y = int( floor(v_img.y()) );
        int z = cur_z;
        int neuronIx = -1;
        QString value("<None>"); // default value
        if (volumeData)
        {
            NaVolumeData::Reader volumeReader(*volumeData);
            if (volumeReader.hasReadLock()) {
                const Image4DProxy<My4DImage>& dataProxy = volumeReader.getOriginalImageProxy();
                const Image4DProxy<My4DImage>& neuronProxy = volumeReader.getNeuronMaskProxy();
                if (   (x >= 0) && (x < dataProxy.sx)
                    && (y >= 0) && (y < dataProxy.sy)
                    && (z >= 0) && (z < dataProxy.sz) )
                {
                    value = "";
                    int nC = dataProxy.sc;
                    if (nC > 1) value += "[";
                    for (int c = 0; c < nC; ++c) {
                        if (c > 0) value += ", ";
                        float val = dataProxy.value_at(x, y, z, c);
                        value += QString("%1").arg(val, 4);
                    }
                    if (nC > 1) value += "]";
                    if (volumeReader.hasNeuronMask())
                        neuronIx = neuronProxy.value_at(x, y, z, 0) - 1;
                }
            }
        }
        QString msg = QString("x =%1, y =%2, z =%3, value =%4")
                      .arg(x, 3)
                      .arg(y, 3)
                      .arg(z, 3)
                      .arg(value);
        if (neuronIx >= 0) { // Zero means background in label field, so -1 in Myers index
            msg = QString("Neuron fragment %1; ").arg(neuronIx, 2) + msg;
        }
        emit statusMessage(msg);

        return;
    }
    if (bMouseCurorIn)
    {
        b_mousemove = true;

        if(b_mouseright)
        {
            bMouseDone = false;

            curMousePosR = e->pos();
            curMousePosR = viewportXYToImageXY(curMousePosR);

            if (curMousePosR.x() < 0)
            {
                curMousePosR.setX(0);
            }
            else if (curMousePosR.x() > sx)
            {
                curMousePosR.setX(sx);
            }

            if (curMousePosR.y()<0)
            {
                curMousePosR.setY(0);
            }
            else if (curMousePosR.y() > sy)
            {
                curMousePosR.setY(sy);
            }
            downRightDragR = ((curMousePosR.x() - startMousePosR.x()) * (curMousePosR.y() - startMousePosR.y()) >= 0);
            updateCursor();
        }

        if(b_mouseleft)
        {
            bMouseDone = true;

            curMousePosL = e->pos();
            curMousePosL = viewportXYToImageXY(curMousePosL);

            if (curMousePosL.x() < 0)
            {
                curMousePosL.setX(0);
            }
            else if (curMousePosL.x() > sx)
            {
                curMousePosL.setX(sx);
            }

            if (curMousePosL.y()<0)
            {
                curMousePosL.setY(0);
            }
            else if (curMousePosL.y() > sy)
            {
                curMousePosL.setY(sy);
            }

            setSquarePos(curMousePosL + m_offset);
        }

    }
    update();
}

void NaZStackWidget::mousePressEvent(QMouseEvent *e) // mouse button press
{
    super::mousePressEvent(e);
    updateCursor();
    switch (e->button())
    {
        case Qt::LeftButton:
            mouseLeftButtonPressEvent(e);
            break;
        case Qt::RightButton:
            mouseRightButtonPressEvent(e);
        default:
            break;
    }
}

void NaZStackWidget::updateCursor()
{
    if (runHDRFILTER) {
        // right drag to resize HDR box
        if (QApplication::mouseButtons() & Qt::RightButton) {
            // Resize arrow direction depends on where the user is dragging the box
            if (downRightDragR)
                setCursor(Qt::SizeFDiagCursor);
            else
                setCursor(Qt::SizeBDiagCursor);
        }
        else // left drag to move HDR box
            setCursor(Qt::CrossCursor);
    }
    else // drag to translate
    {
        if (QApplication::mouseButtons() & Qt::LeftButton)
            setCursor(Qt::ClosedHandCursor); // dragging
        else
            setCursor(Qt::OpenHandCursor); // hovering
    }
}

void NaZStackWidget::mouseReleaseEvent(QMouseEvent * e) // mouse button release
{
    super::mouseReleaseEvent(e);
    updateCursor();
    b_mousemove = false;

    if (bMouseCurorIn)
    {
        if(b_mouseright)
        {
            endMousePosR = e->pos();
            endMousePosR = viewportXYToImageXY(endMousePosR);
            // setCursor(Qt::ArrowCursor);

            if (endMousePosR.x() < 0)
            {
                endMousePosR.setX(0);
            }
            else if (endMousePosR.x() > sx)
            {
                endMousePosR.setX(sx);
            }

            if (endMousePosR.y()<0)
            {
                endMousePosR.setY(0);
            }
            else if (endMousePosR.y() > sy)
            {
                endMousePosR.setY(sy);
            }

            // adjust pos
            float top_x, top_y, bottom_x, bottom_y;

            top_x = qMin( startMousePosR.x(), endMousePosR.x() );
            top_y = qMin( startMousePosR.y(), endMousePosR.y() );

            bottom_x = qMax( startMousePosR.x(), endMousePosR.x() );
            bottom_y = qMax( startMousePosR.y(), endMousePosR.y() );

            startMousePos.setX( top_x );
            startMousePos.setY( top_y );

            endMousePos.setX( bottom_x );
            endMousePos.setY( bottom_y );

            m_square_pos.setX( startMousePos.x() + (endMousePos.x() - startMousePos.x())/2 );
            m_square_pos.setY( startMousePos.y() + (endMousePos.y() - startMousePos.y())/2 );

            int temp_cr = qMax((endMousePos.x() - startMousePos.x())/2, (endMousePos.y() - startMousePos.y())/2);
            setHdrBoxSize(2 * temp_cr + 1);

            bMouseDone = true;

            //
            b_mouseright = false;
        }

        if(b_mouseleft)
        {
            // setCursor(Qt::ArrowCursor);

            //
            b_mouseleft = false;
            bMouseDone = true;
        }

    }

    update();
}

void NaZStackWidget::setSquarePos(const QPointF &pos)
{
    const QRect oldRect = rectangle_around(m_square_pos).toAlignedRect();
    m_square_pos = pos;

    //
    float top_x, top_y, bottom_x, bottom_y;

    int temp_cr = (hdrBoxSize - 1) / 2;
    top_x = m_square_pos.x() - temp_cr;
    top_y = m_square_pos.y() - temp_cr;

    bottom_x = m_square_pos.x() + temp_cr;
    bottom_y = m_square_pos.y() + temp_cr;

    if(top_x<1) top_x = 1;
    if(top_y<1) top_y = 1;

    if(bottom_x>sx) bottom_x = sx;
    if(bottom_y>sy) bottom_y = sy;

    startMousePos.setX( top_x );
    startMousePos.setY( top_y );

    endMousePos.setX( bottom_x );
    endMousePos.setY( bottom_y );

}

QRectF NaZStackWidget::rectangle_around(const QPointF &p, const QSizeF &size)
{
    QRectF rect(p, size);
    rect.translate(-size.width()/2, -size.height()/2);
    return rect;
}

//
bool NaZStackWidget::checkROIchanged()
{
    bool flag = false;

    // recordColorChannelROIPos();

    if(start_x != startMousePos.x()-1)
    {
        start_x = startMousePos.x()-1;
        flag = true;
    }

    if(end_x != endMousePos.x()-1)
    {
        end_x = endMousePos.x()-1;
        flag = true;
    }

    if(start_y != startMousePos.y()-1)
    {
        start_y = startMousePos.y()-1;
        flag = true;
    }

    if(end_y != endMousePos.y()-1)
    {
        end_y = endMousePos.y()-1;
        flag = true;
    }

    //
    return flag;
}

// HDR filter
// This method appears to measure the range of intensities within the HDR box
void NaZStackWidget::do_HDRfilter()
{
    // qDebug() << "do_HDRfilter";

    // widget might not be initialized
    // qDebug() << cur_c << sc << runHDRFILTER;
    if (cur_c < 1) return;
    if (cur_c > sc) return;
    if(!runHDRFILTER) return;

    //
    start_x = startMousePos.x()-1;
    end_x = endMousePos.x()-1;

    start_y = startMousePos.y()-1;
    end_y = endMousePos.y()-1;

    V3DLONG tmpsum = start_x + end_x;
    start_x = start_x<=end_x?start_x:end_x;
    end_x = tmpsum - start_x;

    tmpsum = start_y + end_y;
    start_y = start_y<=end_y?start_y:end_y;
    end_y = tmpsum - start_y;

    if(start_x<0) start_x = 0;
    if(start_y<0) start_y = 0;

    if(end_x>sx) end_x = sx-1;
    if(end_y>sy) end_y = sy-1;

    // qDebug() << end_x << start_x << end_y << start_y;
    if(end_x<=start_x || end_y<=start_y) return;

    // min_max
    V3DLONG c = cur_c - 1; // channel index
    if (c < 0) return;

    // qDebug() << volumeData;
    if (! volumeData) return;
    {
        NaVolumeData::Reader volumeReader(*volumeData);
        if (! volumeReader.hasReadLock()) return;
        const Image4DProxy<My4DImage>& volProxy = volumeReader.getOriginalImageProxy();
        const Image4DProxy<My4DImage>& refProxy = volumeReader.getReferenceImageProxy();

        if (c > volProxy.sc) return; // ==sc means reference channel

        min_roi[c] = INF;
        max_roi[c] = -INF;

        for(V3DLONG j=start_y; j<end_y; j++)
        {
            for(V3DLONG i=start_x; i<end_x; i++)
            {
                float curval;
                if (c < volProxy.sc) // regular data channel
                    curval = volProxy.value_at(i, j, cur_z, c);
                else // reference channel
                    curval = refProxy.value_at(i, j, cur_z, 0);

                if(min_roi[c] > curval) min_roi[c] = curval;
                if(max_roi[c] < curval) max_roi[c] = curval;
            }
        }
    } // release read locks

    // qDebug() << "emitting hdrRangeChanged" << c << min_roi[c] << max_roi[c];
    emit hdrRangeChanged(c, min_roi[c], max_roi[c]);
}

void NaZStackWidget::updatePixmap()
{
    if (! zSliceColors) {
        invalidate();
        return;
    }
    if (! zSliceColors->representsActualData()) {
        invalidate();
        return;
    }
    setRepresentsActualData();
    QTime stopwatch;
    int newZ = -1;
    stopwatch.start();
    {
        ZSliceColors::Reader zReader(*zSliceColors);
        if (! zReader.hasReadLock()) {
            // return; // return is what most Readers should usually do in this case.
            // In this one case it is OK to call waitForReadLock() because:
            // 1) ZSliceColors takes only 30ms to update.
            // 2) ZSliceColors has been specially modified to discard excess accumulated update() events.
            // Don't call waitForReadLock() casually, especially from the GUI thread (like this!).
            // I'm calling waitForReadLock() here so the user can get continuous visual feedback while
            // dragging the z-scrollbar.  The ZSliceColors constantly write-locks during fast dragging.
            zReader.waitForReadLock();
        }
        pixmap = QPixmap::fromImage(*zReader.getImage());
        sx = pixmap.width();
        sy = pixmap.height();
        newZ = zReader.getZIndex();
    }
    setCurrentZSlice(newZ);
    // qDebug() << "NaStackWidget pixmap updated";
    // qDebug() << "NaZStackWidget updatePixmap took" << stopwatch.elapsed() << "milliseconds";
    update();
}

void NaZStackWidget::setColorChannel(NaZStackWidget::Color col)
{
    // Ensure there are enough channels to support this color
    if ( (int)col > sc )
        return; // outside of range of available color channels
    setHDRCheckState(true); // Turn on HDR mode when a color channel is selected
    if (col == cur_c)
        return; // No change

    // Store hdr box parameters for previous channel
    int c_ix = cur_c - 1;
    if (hdrBoxSize >= NaZStackWidget::minHdrBoxSize)
    {
        recStartMousePos[c_ix] = startMousePos;
        recEndMousePos[c_ix] = endMousePos;
        recCr[c_ix] = hdrBoxSize;
        // fooDebugggggggggggggggggggggggggggggggggggggggggg() << recCr[c_ix] << c_ix << __FILE__ << __LINE__;
        recZ[c_ix] = cur_z;
        recMousePos[c_ix] = true;
    }
    // Recall hdr box parameters for current channel
    cur_c = col;
    c_ix = cur_c - 1;
    if (recMousePos[c_ix])
    {
        startMousePos = recStartMousePos[c_ix];
        endMousePos = recEndMousePos[c_ix];
        cur_z = recZ[c_ix]; // TODO REALLY!?!?!
        // fooDebug() << recCr[c_ix] << c_ix << __FILE__ << __LINE__;
        setHdrBoxSize(recCr[c_ix]);
        // TODO - start_x and startMousePos.x() are redundant
        start_x = startMousePos.x()-1;
        end_x = endMousePos.x()-1;
        start_y = startMousePos.y()-1;
        end_y = endMousePos.y()-1;
        m_square_pos.setX( startMousePos.x() + (endMousePos.x() - startMousePos.x())/2 );
        m_square_pos.setY( startMousePos.y() + (endMousePos.y() - startMousePos.y())/2 );
    }

    recNum = 0;
    hdrfiltered[(int)col - 1] = true;
    pre_c = cur_c;
    // cur_c = col;

    emit curColorChannelChanged(col);
}

void NaZStackWidget::setRedChannel() {
    setColorChannel(COLOR_RED);
}

void NaZStackWidget::setGreenChannel() {
    setColorChannel(COLOR_GREEN);
}

void NaZStackWidget::setBlueChannel() {
    setColorChannel(COLOR_BLUE);
}

void NaZStackWidget::setNc82Channel() {
    setColorChannel(COLOR_NC82);
}

bool NaZStackWidget::setHdrBoxSize(int boxSize)
{
    if (boxSize < NaZStackWidget::minHdrBoxSize)
        boxSize = NaZStackWidget::minHdrBoxSize;
    if (hdrBoxSize == boxSize)
        return false; // no change => ignore
    hdrBoxSize = boxSize;
    endMousePos.setX(startMousePos.x() + boxSize);
    endMousePos.setY(startMousePos.y() + boxSize);
    // updateHDRView(); // only on mouse drag
    emit hdrBoxSizeChanged(boxSize);
    update();
    return true;
}

void NaZStackWidget::setHDRCheckState(bool state)
{
    if (state == runHDRFILTER) return;
    runHDRFILTER = state;
    if(state){
        setContextMenuPolicy(Qt::NoContextMenu); // because hdr right-click does not work with context menu
        setToolTip("drag to move HDR window\nright-drag to change HDR window size\nalt-scroll to change HDR channel\nshift-scroll to zoom\nscroll to z-scan\ndouble-click to recenter");
    }
    else{
        setContextMenuPolicy(Qt::CustomContextMenu);
        setToolTip("drag to pan\nshift-scroll to zoom\nscroll to z-scan\ndouble-click to recenter");
    }
    updateCursor();
    update();
    emit changedHDRCheckState(state); // hide gamma widget
}

// record previous color channel ROI
void NaZStackWidget::recordColorChannelROIPos(){

    if(recNum<1 && pre_c!=cur_c){
        recNum++;

        if(pre_c!=cur_c)
        {
            recMousePos[pre_c-1] = true;

            recStartMousePos[pre_c-1].setX( startMousePos.x() );
            recStartMousePos[pre_c-1].setY( startMousePos.y() );

            recEndMousePos[pre_c-1].setX( endMousePos.x() );
            recEndMousePos[pre_c-1].setY( endMousePos.y() );

            // fooDebug() << recCr[pre_c-1] << pre_c-1 << __FILE__ << __LINE__;
            recCr[pre_c-1] = hdrBoxSize;
            // fooDebug() << recCr[pre_c-1] << pre_c-1 << __FILE__ << __LINE__;
            recZ[pre_c-1] = cur_z;
        }

        if(recMousePos[cur_c-1])
        {
            cur_z = recZ[cur_c-1];

            startMousePos.setX(recStartMousePos[cur_c-1].x());
            startMousePos.setY(recStartMousePos[cur_c-1].y());

            endMousePos.setX(recEndMousePos[cur_c-1].x());
            endMousePos.setY(recEndMousePos[cur_c-1].y());
            // fooDebug() << recCr[cur_c-1] << cur_c-1 << __FILE__ << __LINE__;
            setHdrBoxSize(recCr[cur_c-1]);
        }

        start_x = startMousePos.x()-1;
        end_x = endMousePos.x()-1;

        start_y = startMousePos.y()-1;
        end_y = endMousePos.y()-1;

        m_square_pos.setX( startMousePos.x() + (endMousePos.x() - startMousePos.x())/2 );
        m_square_pos.setY( startMousePos.y() + (endMousePos.y() - startMousePos.y())/2 );

        int temp_cr = qMax((endMousePos.x() - startMousePos.x())/2, (endMousePos.y() - startMousePos.y())/2);
        setHdrBoxSize(2*temp_cr + 1);
    }

}

void NaZStackWidget::updateHDRView(){

    if( checkROIchanged() )
    {
        do_HDRfilter();
    }
    else
    {
        // do_HDRfilter_zslice();
        update();
    }
}

