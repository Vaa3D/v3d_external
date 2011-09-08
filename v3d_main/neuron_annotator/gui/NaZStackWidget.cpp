#include "NaZStackWidget.h"
#include <QPaintEvent>
#include <QDebug>
#include <QPainter>
#include <cmath>
#include <iostream>

using namespace std;

#define INF 1e9

NaZStackWidget::NaZStackWidget(QWidget * parent)
    : Na2DViewer(parent)
    , zSliceColors(NULL)
    , volumeData(NULL)
    , cur_z(-1)
{
    translateMouse_scale = 1;

    b_mouseleft = false;
    b_mouseright = false;
    b_mousemove = false;

    bMouseCurorIn = false;
    bMouseDone = false;

    for(int i=0; i<NCLRCHNNL; i++)
    {
        recMousePos[i] = false;
        recZ[i] = 0;
        hdrfiltered[i] = false;
    }

    setRedChannel(); // by default

    m_square_pos.setX(sx/2);
    m_square_pos.setY(sy/2);
    cr = 12;

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
}

NaZStackWidget::~NaZStackWidget() {}

/* slot */
void NaZStackWidget::updateVolumeParameters()
{
    if (! volumeData) return;
    NaVolumeData::Reader volumeReader(*volumeData);
    if (! volumeReader.hasReadLock()) return;
    const Image4DProxy<My4DImage>& volProxy = volumeReader.getOriginalImageProxy();
    sx = volProxy.sx;
    sy = volProxy.sy;
    sz = volProxy.sz;
    sc = volProxy.sc + 1; // +1, reference channel too
    min_roi.assign(sc, INF);
    max_roi.assign(sc, -INF);
}

void NaZStackWidget::paintEvent(QPaintEvent *event)
{
    updateDefaultScale();
    painter.begin(this);

    // Color background black
    painter.fillRect(0, 0, width(), height(), Qt::black);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    transformPainterToCurrentCamera(painter);

    painter.drawPixmap(0, 0, pixmap);

    if (bPaintCrosshair) paintCrosshair(painter);

    float scale = defaultScale * cameraModel.scale();
    if (scale > 40) { // 40 display pixels per image pixel
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
                    QSizeF sz(2*cr+1, 2*cr+1);
                    QRectF square = rectangle_around(m_square_pos, sz);
                    painter->drawRect(square);
                }
            }
        }
        else if(bMouseDone)
        {
            QSizeF sz(2*cr+1, 2*cr+1);
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
        cr = 12;

        start_x = cx - cr;
        end_x = cx + cr;

        if(start_x<0) start_x = 0;
        if(end_x>=sx) end_x = sx-1;

        start_y = cy -cr;
        end_y = cy + cr;

        if(start_y<0) start_y = 0;
        if(end_y>=sy) end_y = sy-1;

        startMousePos.setX(start_x);
        startMousePos.setY(start_y);

        endMousePos.setX(end_x);
        endMousePos.setY(end_y);

        m_square_pos.setX( startMousePos.x() + (endMousePos.x() - startMousePos.x())/2 );
        m_square_pos.setY( startMousePos.y() + (endMousePos.y() - startMousePos.y())/2 );

        QSizeF sz(2*cr+1, 2*cr+1);
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
    return cur_z + 1; // 1-based index for API; 0-based internal
}

int NaZStackWidget::getCurrentBoxSize() {
    return cr; // radius
}

void NaZStackWidget::setCurrentZSlice(int slice)
{
    if (slice < 1) return; // value too small
    if (slice > sz) return; // value too big
    if (cur_z == slice - 1) return; // no change; ignore
    cur_z = slice - 1;
    // qDebug() << "setting z slice to " << slice;

    updateHDRView();

    // Consider updating camera focus to match latest z-slice
    const Vector3D& f = cameraModel.focus();
    int camZ = int(floor(f.z() + 0.5));
    float midZ = sz / 2.0f;
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
    setCurrentZSlice(getCurrentZSlice() - numTicks);
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
    if (Qt::NoButton == e->buttons()) {
        // hover, not drag
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

            int old_cr = cr;
            cr = qMax((endMousePos.x() - startMousePos.x())/2, (endMousePos.y() - startMousePos.y())/2);

            setSearchBoxSize();

            bMouseDone = true;

            //
            b_mouseright = false;
            if (old_cr != cr) {
                old_cr = cr;
                emit boxSizeChanged(2*cr+1);
            }
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

    top_x = m_square_pos.x() - cr;
    top_y = m_square_pos.y() - cr;

    bottom_x = m_square_pos.x() + cr;
    bottom_y = m_square_pos.y() + cr;

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

    recordColorChannelROIPos();

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

    // qDebug() << "emitting hdrRangeChanged" << c << min_roi << max_roi;
    emit hdrRangeChanged(c, min_roi[c], max_roi[c]);
}

void NaZStackWidget::updatePixmap()
{
    if (! zSliceColors) return;
    QTime stopwatch;
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
        cur_z = zReader.getZIndex();
    }
    // qDebug() << "NaStackWidget pixmap updated";
    // qDebug() << "NaZStackWidget updatePixmap took" << stopwatch.elapsed() << "milliseconds";
    update();
}

void NaZStackWidget::setColorChannel(NaZStackWidget::Color col)
{
    // Ensure there are enough channels to support this color
    if ( (int)col > sc )
        return; // outside of range of available color channels
    pre_c = cur_c;
    cur_c = col;

    emit curColorChannelChanged(col);
}

void NaZStackWidget::setRedChannel() {
    recNum = 0;
    hdrfiltered[0] = true;
    setColorChannel(COLOR_RED);
}

void NaZStackWidget::setGreenChannel() {
    recNum = 0;
    hdrfiltered[1] = true;
    setColorChannel(COLOR_GREEN);
}

void NaZStackWidget::setBlueChannel() {
    recNum = 0;
    hdrfiltered[2] = true;
    setColorChannel(COLOR_BLUE);
}

void NaZStackWidget::updateROIsize(int boxSize)
{
    if (cr == boxSize) return; // no change => ignore
    cr = (boxSize-1)/2;

    updateHDRView();

    emit boxSizeChanged(boxSize);
}

void NaZStackWidget::setHDRCheckState(int state) {
    if(state){
        runHDRFILTER = true;
        emit changedHDRCheckState(false); // hide gamma widget
    }
    else{
        runHDRFILTER = false;
        emit changedHDRCheckState(true); // show gamma widget
    }
    updateCursor();
    update();
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

            recCr[pre_c-1] = cr;
            recZ[pre_c-1] = cur_z;
        }

        if(recMousePos[cur_c-1])
        {
            cr = recCr[cur_c-1];
            cur_z = recZ[cur_c-1];

            startMousePos.setX(recStartMousePos[cur_c-1].x());
            startMousePos.setY(recStartMousePos[cur_c-1].y());

            endMousePos.setX(recEndMousePos[cur_c-1].x());
            endMousePos.setY(recEndMousePos[cur_c-1].y());

        }

        start_x = startMousePos.x()-1;
        end_x = endMousePos.x()-1;

        start_y = startMousePos.y()-1;
        end_y = endMousePos.y()-1;

        m_square_pos.setX( startMousePos.x() + (endMousePos.x() - startMousePos.x())/2 );
        m_square_pos.setY( startMousePos.y() + (endMousePos.y() - startMousePos.y())/2 );

        int old_cr = cr;
        cr = qMax((endMousePos.x() - startMousePos.x())/2, (endMousePos.y() - startMousePos.y())/2);

        setSearchBoxSize();
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

void NaZStackWidget::setSearchBoxSize(){

    if(cr<MINSZBOX)
    {
        cr = MINSZBOX;

        int boxsz = 2*cr+1;

        endMousePos.setX(startMousePos.x() + boxsz);
        endMousePos.setY(startMousePos.y() + boxsz);

        updateHDRView();
    }

}
