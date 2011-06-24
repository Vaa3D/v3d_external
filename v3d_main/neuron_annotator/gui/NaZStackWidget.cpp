#include "NaZStackWidget.h"
#include <QPaintEvent>
#include <QDebug>
#include <QPainter>
#include <cmath>

#define INF 1e9

// funcs for converting data and create pixmap
// func converting kernel
template <class Tpre, class Tpost>
void converting(Tpre *pre1d, Tpost *pPost, V3DLONG imsz) {
    for(V3DLONG i=0; i<imsz; i++)
        pPost[i] = (Tpost) pre1d[i];
}

template <class T> QImage& getXYPlane(QImage& displayImage, const T * pdata, V3DLONG sx, V3DLONG sy, V3DLONG sz, V3DLONG sc, V3DLONG curz, float *p_vmax, float *p_vmin)
{
    displayImage = QImage(sx, sy, QImage::Format_RGB32);

    int tr,tg,tb;

    V3DLONG i,j;
    float tmpr,tmpg,tmpb;
    float tmpr_min, tmpg_min, tmpb_min;

    if (sc>=3)
    {
        tmpb = p_vmax[2]-p_vmin[2]; tmpb = (tmpb==0)?1:tmpb;
        tmpb_min = p_vmin[2];
    }

    if (sc>=2)
    {
        tmpg = p_vmax[1]-p_vmin[1]; tmpg = (tmpg==0)?1:tmpg;
        tmpg_min = p_vmin[1];
    }

    if (sc>=1)
    {
        tmpr = p_vmax[0]-p_vmin[0]; tmpr = (tmpr==0)?1:tmpr;
        tmpr_min = p_vmin[0];
    }
    int pagesz = sx*sy*sz;
    long offset_k = sx*sy;

    switch (sc)
    {
        case 1:

            for (long j = 0; j < sy; j ++)
            {
                long offset = curz*offset_k + j*sx;
                for (long i=0; i<sx; i++)
                {
                    long idx = offset + i;

                    tb = tg = tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
                    displayImage.setPixel(i, j, qRgb(tr, tg, tb));
                }
            }
            break;

        case 2:

            tb = 0;
            for (long j = 0; j < sy; j ++)
            {
                long offset = curz*offset_k + j*sx;
                for (long i=0; i<sx; i++)
                {
                    long idx = offset + i;
                    tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
                    tg = floor((pdata[idx+pagesz]-tmpg_min)/tmpg*255.0);
                    displayImage.setPixel(i, j, qRgb(tr, tg, tb));
                }
            }
            break;

        case 3:
        case 4:

            for (long j = 0; j < sy; j ++)
            {
                long offset = curz*offset_k + j*sx;
                for (long i=0; i<sx; i++)
                {
                    long idx = offset + i;

                    tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
                    tg = floor((pdata[idx+pagesz]-tmpg_min)/tmpg*255.0);
                    tb = floor((pdata[idx+2*pagesz]-tmpb_min)/tmpb*255.0);
                    displayImage.setPixel(i, j, qRgb(tr, tg, tb));
                }
            }
            break;

        default:
            break;
    }
    return displayImage;

}

NaZStackWidget::NaZStackWidget(QWidget * parent)
    : Na2DViewer(parent)
    , originalImage(NULL)
{
    pData1d = NULL;
    pDispData1d = NULL;

    displayImage = QImage(256, 256, QImage::Format_RGB32);
    displayImage.fill(Qt::black);
    pixmap = QPixmap::fromImage(displayImage);
    // updateDefaultScale();

    // scale_x = 1.0; scale_y = 1.0;
    // dispscale = 0.9;

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
    connect(this, SIGNAL(roiChanged()), this, SLOT(updatePixmap()));
    connect(&cameraModel, SIGNAL(focusChanged(Vector3D)),
            this, SLOT(onCameraFocusChanged(Vector3D)));
    connect(this, SIGNAL(mouseLeftDragEvent(int, int, QPoint)),
            this, SLOT(onMouseLeftDragEvent(int, int, QPoint)));
}

NaZStackWidget::~NaZStackWidget() {}

void NaZStackWidget::paintEvent(QPaintEvent *event)
{
    updateDefaultScale();
    painter.begin(this);

    // Color background black
    painter.fillRect(0, 0, width(), height(), Qt::black);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    transformPainterToCurrentCamera(painter);

    /*
    // TODO - these values do not need to be recalculated every paint.
    // ...but after either widget resize or image load.
    // scale image to to viewport
    float temp_scale_x = (float)width()/(float)sx;
    float temp_scale_y = (float)height()/(float)sy;
    // fit to window; keep aspect ratio
    float scale = temp_scale_x < temp_scale_y ? temp_scale_x : temp_scale_y;
    scale_x = scale;
    scale_y = scale;
    // translate to center image in viewport
    image_focus_x = sx/2;
    image_focus_y = sy/2;

    painter.translate(width()/2 - scale_x*image_focus_x, height()/2 - scale_y*image_focus_y);
    painter.scale(scale_x, scale_y);
    */

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
    if (! originalImage ) return;
    const Image4DProxy<My4DImage> imgProxy(const_cast<My4DImage*>(originalImage));

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
    int nC = originalImage->getCDim();
    float lineHeight = scale / (nC + 1.0);
    for (int x = int(v_img_upleft.x() - 0.5); x <= int(v_img_downright.x() + 0.5); ++x) {
        // qDebug() << "x = " << x;
        if (x < 0)
            continue;
        if (x >= displayImage.size().width())
            continue;
        for (int y = int(v_img_upleft.y() - 0.5); y <= int(v_img_downright.y() + 0.5); ++y) {
            // qDebug() << "y = " << y;
            if (y < 0)
                continue;
            if (y >= displayImage.size().height())
                continue;
            // Transform image pixel coordinates back to viewport coordinates
            QPointF v = X_view_img * QPointF(x, y);
            // qDebug() << x << ", " << y << "; " << v.x() << ", " << v.y();
            // Print original data intensity, not displayed intensity
            // But choose font color based on displayed intensity.
            unsigned int red = qRed(displayImage.pixel(x, y));
            unsigned int green = qGreen(displayImage.pixel(x, y));
            unsigned int blue = qBlue(displayImage.pixel(x, y));
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

void NaZStackWidget::onCameraFocusChanged(const Vector3D& focus)
{
    if (sz <= 0) return;
    // Drive to the corresponding z slice
    // qDebug() << "focus.z() = " << focus.z();
    int z = int(floor(focus.z() + 0.5));
    // qDebug() << "z0 = " << z;
    float midZ = sz / 2.0f;
    z = int(midZ + flip_Z * (z - midZ));
    // qDebug() << "z1 = " << z;
    // qDebug() << "midZ = " << midZ;
    // qDebug() << "flip_Z = " << flip_Z;
    // qDebug() << "camera focus set to " << QString("%1, %2, %3").arg(focus.x()).arg(focus.y()).arg(focus.z());
    setCurrentZSlice(z + 1);
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
    if (flip_cur_z != camZ)
        cameraModel.setFocus(Vector3D(f.x(), f.y(), flip_cur_z));

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
    setCurrentZSlice(getCurrentZSlice() + numTicks);
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
void NaZStackWidget::do_HDRfilter()
{
    // widget might not be initialized
    if (cur_c < 1) return;
    if (cur_c > 3) return;
    if (!pData1d) return;
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

    if(end_x<=start_x || end_y<=start_y) return;

    V3DLONG pagesz = sx*sy*sz;

    // min_max
    V3DLONG c = cur_c-1;

    min_roi[c] = INF;
    max_roi[c] = -INF;

    V3DLONG offset_c = c*pagesz;

    V3DLONG offset_z = cur_z * sx * sy; // current slice

    for(V3DLONG j=start_y; j<end_y; j++)
    {
        V3DLONG offset_j = offset_c + offset_z + j*sx;
        for(V3DLONG i=start_x; i<end_x; i++)
        {
            V3DLONG idx = offset_j + i;

            float curval;

            if(datatype == V3D_UINT8)
                curval = (float)( ((unsigned char*)pData1d)[idx] );
            else if(datatype == V3D_UINT16)
                curval = (float)( ((unsigned short int*)pData1d)[idx] );
            else if(datatype == V3D_FLOAT32)
                curval = (float)( ((float*)pData1d)[idx] );
            else {
                printf("Datatype is not supported.\n");
                return;
            }

            if(min_roi[c]>curval) min_roi[c] = curval;
            if(max_roi[c]<curval) max_roi[c] = curval;
        }
    }

    scale_roi[c] = max_roi[c] - min_roi[c];

    // filter
    do_HDRfilter_zslice();

    //
    emit roiChanged();
}

// z slice filter
void NaZStackWidget::do_HDRfilter_zslice()
{
    if (!pData1d) return; // data not initialized yet.
    if (!pDispData1d) return; // race condition?

    V3DLONG pagesz = sx*sy;
    V3DLONG channelsz = pagesz*sz;
    //V3DLONG c = cur_c-1;
    V3DLONG offset_z = cur_z * pagesz;

    for(V3DLONG c=0; c<sc; c++)
    {
        if(!hdrfiltered[c]) continue;

        V3DLONG offset_c = c*channelsz;

        for(V3DLONG i=0; i<pagesz; i++)
        {
            V3DLONG idx = offset_c + offset_z + i;

            float curval;

            if(datatype == V3D_UINT8){
                curval = (float)( ((unsigned char*)pData1d)[idx] );

                if(curval<min_roi[c])
                {
                    ((unsigned char*)pDispData1d)[idx] = (unsigned char)min_roi[c];
                }
                else if(curval>max_roi[c])
                {
                    ((unsigned char*)pDispData1d)[idx] = (unsigned char)max_roi[c];
                }
                else
                {
                    ((unsigned char*)pDispData1d)[idx] = (unsigned char)curval;
                }
            }
            else if(datatype == V3D_UINT16){
                curval = (float)( ((unsigned short int*)pData1d)[idx] );

                if(curval<min_roi[c])
                {
                    ((unsigned short int*)pDispData1d)[idx] = (unsigned short int)min_roi[c];
                }
                else if(curval>max_roi[c])
                {
                    ((unsigned short int*)pDispData1d)[idx] = (unsigned short int)max_roi[c];
                }
                else
                {
                    ((unsigned short int*)pDispData1d)[idx] = (unsigned short int)curval;
                }
            }
            else if(datatype == V3D_FLOAT32){
                curval = (float)( ((float*)pData1d)[idx] );

                if(curval<min_roi[c])
                {
                    ((float*)pDispData1d)[idx] = min_roi[c];
                }
                else if(curval>max_roi[c])
                {
                    ((float*)pDispData1d)[idx] = max_roi[c];
                }
                else
                {
                    ((float*)pDispData1d)[idx] = curval;
                }
            }
            else {
                printf("Datatype is not supported.\n");
                return;
            }
        }
    }

}

// copy data
void NaZStackWidget::copydata2disp() // legacy function
{
    V3DLONG c = cur_c - 1;

    V3DLONG pagesz = sx*sy*sz;

    for(V3DLONG ic = 0; ic<sc; ic++)
    {
        if(ic==c) continue;

        V3DLONG offset_c = ic*pagesz;

        if(recCopy==0)
        {
            for(V3DLONG i=0; i<pagesz; i++)
            {
                V3DLONG idx = offset_c + i;

                if(datatype == V3D_UINT8){
                    ((unsigned char *)pDispData1d)[idx] = ((unsigned char*)pData1d)[idx];
                }
                else if(datatype == V3D_UINT16){
                    ((unsigned short int *)pDispData1d)[idx] = ((unsigned short int*)pData1d)[idx];
                }
                else if(datatype == V3D_FLOAT32){
                    ((float *)pDispData1d)[idx] = ((float*)pData1d)[idx];
                }
                else {
                    printf("Datatype is not supported.\n");
                    return;
                }
            }

            min_roi[ic] = min_img[ic];
            max_roi[ic] = max_img[ic];
            scale_roi[ic] = scale_img[ic];
        }

    }
    recCopy++;
}

// update pixelmap
void NaZStackWidget::updatePixmap()
{
    if(runHDRFILTER){
        // race condition?
        if (!pDispData1d) return;
        if(datatype == V3D_UINT8)
            getXYPlane(displayImage, (unsigned char *)pDispData1d, sx, sy, sz, sc, cur_z, max_roi, min_roi);
        else if(datatype == V3D_UINT16)
            getXYPlane(displayImage, (unsigned short int *)pDispData1d, sx, sy, sz, sc, cur_z, max_roi, min_roi);
        else if(datatype == V3D_FLOAT32)
            getXYPlane(displayImage, (float *)pDispData1d, sx, sy, sz, sc, cur_z, max_roi, min_roi);
        else {
            printf("Datatype is not supported.\n");
            return;
        }
        pixmap = QPixmap::fromImage(displayImage);
    }
    else{
        if (!pData1d) return;
        if(datatype == V3D_UINT8)
            getXYPlane(displayImage, (unsigned char *)pData1d, sx, sy, sz, sc, cur_z, max_img, min_img);
        else if(datatype == V3D_UINT16)
            getXYPlane(displayImage, (unsigned short int *)pData1d, sx, sy, sz, sc, cur_z, max_img, min_img);
        else if(datatype == V3D_FLOAT32)
            getXYPlane(displayImage, (float *)pData1d, sx, sy, sz, sc, cur_z, max_img, min_img);
        else {
            printf("Datatype is not supported.\n");
            return;
        }
        pixmap = QPixmap::fromImage(displayImage);

        for(int j=0; j<pixmap.height(); j++)
        {
            for(int i=0; i<pixmap.width(); i++)
            {
                QRgb qrgb = displayImage.pixel(i, j);

                int tr = (int)((brightnessCalibrator.getCorrectedIntensity((float)(qRed(qrgb))) * 255.0f) + 0.4999);
                int tg = (int)((brightnessCalibrator.getCorrectedIntensity((float)(qGreen(qrgb))) * 255.0f) + 0.4999);
                int tb = (int)((brightnessCalibrator.getCorrectedIntensity((float)(qBlue(qrgb))) * 255.0f) + 0.4999);

                displayImage.setPixel(i, j, qRgb(tr, tg, tb));

            }
        }

        pixmap = QPixmap::fromImage(displayImage);
    }
    // updateDefaultScale();
    update();
}

// Load image data from an in-memory image
bool NaZStackWidget::loadMy4DImage(const My4DImage* img, const My4DImage* neuronMaskImage)
{
    V3DLONG imageSize[4] = {img->getXDim(), img->getYDim(), img->getZDim(), img->getCDim()};
    initHDRViewer(imageSize, img->getRawData(), img->getDatatype());
    originalImage = img;
    return true;
}

void NaZStackWidget::initHDRViewer(const V3DLONG *imgsz, const unsigned char *data1d, ImagePixelType imgdatatype)
{
    sx = imgsz[0];
    sy = imgsz[1];
    sz = imgsz[2];
    sc = imgsz[3];

    datatype = imgdatatype;

    ratio_x2y = (float)sx/(float)sy;

    cur_z = sz/2; // init here

    V3DLONG pagesz = sx*sy*sz;
    V3DLONG imagesz = pagesz*sc;

    try
    {

        if(datatype == V3D_UINT8)
        {
            pData1d = (void *)data1d;
            pDispData1d = new unsigned char [imagesz];

            converting<unsigned char, unsigned char>((unsigned char *)pData1d, (unsigned char *)pDispData1d, imagesz); // copy data

            // min_max
            for (V3DLONG c=0; c<sc; c++)
            {
                min_img[c] = INF;
                max_img[c] = -INF;

                V3DLONG offset_c = c*pagesz;
                for(V3DLONG i=0; i<pagesz; i++)
                {
                    float val = (float)( data1d[i+offset_c] );

                    if(min_img[c]>val) min_img[c] = val;
                    if(max_img[c]<val) max_img[c] = val;
                }
                scale_img[c] = max_img[c] - min_img[c];

                min_roi[c] = min_img[c];
                max_roi[c] = max_img[c];
                scale_roi[c] = scale_img[c];
            }

            getXYPlane(displayImage, (unsigned char *)pData1d, sx, sy, sz, sc, cur_z, max_img, min_img); // initial focus plane
            pixmap = QPixmap::fromImage(displayImage);
            // updateDefaultScale();

            update();
            //repaint();

        }
        else if(datatype == V3D_UINT16)
        {
            pData1d = (void *)data1d;
            pDispData1d = new unsigned short int [imagesz];

            converting<unsigned short int, unsigned short int>((unsigned short int*)pData1d, (unsigned short int*)pDispData1d, imagesz); // copy data

            // min_max
            for (V3DLONG c=0; c<sc; c++)
            {
                min_img[c] = INF;
                max_img[c] = -INF;

                V3DLONG offset_c = c*pagesz;
                for(V3DLONG i=0; i<pagesz; i++)
                {
                    float val = (float)( ((unsigned short *)data1d)[i+offset_c] );

                    if(min_img[c]>val) min_img[c] = val;
                    if(max_img[c]<val) max_img[c] = val;
                }
                scale_img[c] = max_img[c] - min_img[c];

                min_roi[c] = min_img[c];
                max_roi[c] = max_img[c];
                scale_roi[c] = scale_img[c];
            }

            getXYPlane(displayImage, (unsigned short int*)pData1d, sx, sy, sz, sc, cur_z, max_img, min_img); // initial focus plane
            pixmap = QPixmap::fromImage(displayImage);
            // updateDefaultScale();

            update();
        }
        else if(datatype == V3D_FLOAT32)
        {
            pData1d = (void *)data1d;
            pDispData1d = new float [imagesz];

            converting<float, float>((float *)pData1d, (float *)pDispData1d, imagesz); // copy data

            // min_max
            for (V3DLONG c=0; c<sc; c++)
            {
                min_img[c] = INF;
                max_img[c] = -INF;

                V3DLONG offset_c = c*pagesz;
                for(V3DLONG i=0; i<pagesz; i++)
                {
                    float val = (float)( ((float *)data1d)[i+offset_c] );

                    if(min_img[c]>val) min_img[c] = val;
                    if(max_img[c]<val) max_img[c] = val;
                }
                scale_img[c] = max_img[c] - min_img[c];

                min_roi[c] = min_img[c];
                max_roi[c] = max_img[c];
                scale_roi[c] = scale_img[c];

            }

            getXYPlane(displayImage, (float *)pData1d, sx, sy, sz, sc, cur_z, max_img, min_img); // initial focus plane
            pixmap = QPixmap::fromImage(displayImage);
            // updateDefaultScale();

            update();
        }

    }
    catch(...)
    {
        printf("Error allocating memory. \n");
    }
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

void NaZStackWidget::annotationModelUpdate(QString updateType) {
    // Stub
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
    updatePixmap();
}

void NaZStackWidget::setGammaBrightness(qreal gamma){
    if (gamma == brightnessCalibrator.getGamma()) return;
    brightnessCalibrator.setGamma(gamma);

    updatePixmap();
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
        do_HDRfilter_zslice();

        updatePixmap();
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
