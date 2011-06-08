#ifndef NA_ZSTACK_WIDGET_H
#define NA_ZSTACK_WIDGET_H

#include <QObject>
#include <QWidget>
#include <QSize>
class QPaintEvent;
class QPainter;
class QEvent;
class QPointF;
class QSizeF;
class QRectF;

#include "../basic_c_fun/v3d_basicdatatype.h"
#include "../basic_c_fun/basic_4dimage.h"
#include "../v3d/v3d_core.h"
#include "NaViewer.h"
#include "BrightnessCalibrator.h"

// NaZStackWidget is a viewer for successive slices of a 3D volume.
// NaZStackWidget is based on HDRViewer class created by Yang Yu,
// refactored to be a standalone class for use in Neuron Annotator.
// NOTE - It does not work yet, as of April 25, 2011 - CMB

class NaZStackWidget : public QWidget, public NaViewer
{
    Q_OBJECT

public:
    enum Color {
        COLOR_RED = 1,
        COLOR_GREEN = 2,
        COLOR_BLUE = 3
    };

    NaZStackWidget(QWidget* parent);
    virtual ~NaZStackWidget();
    void paintEvent(QPaintEvent *event);
    // ROI controller functions
    void drawROI(QPainter *painter);

    void enterEvent (QEvent * e); // mouse found
    void mouseLeftButtonPressEvent(QMouseEvent *e);
    void mouseRightButtonPressEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent * e); // mouse wheel
    void mouseMoveEvent (QMouseEvent * e); // mouse move
    void mousePressEvent(QMouseEvent *e); // mouse button press
    void mouseReleaseEvent(QMouseEvent * e); // mouse button release

    void setSquarePos(const QPointF &pos);
    QRectF rectangle_around(const QPointF &p, const QSizeF &size = QSize(25, 25));
    bool checkROIchanged();
    // Load image data from an in-memory image
    bool loadMy4DImage(const My4DImage* my4DImage, const My4DImage* neuronMaskImage = NULL);
    int getCurrentZSlice(); // 1-based slice index
	int getCurrentBoxSize();

    // Convert between image data coordinates and screen coordinates.
    //  * scale by scale_x, scale_y
    //  * translate so image_focus_[xy] is in center of viewport
    //  * shift by one pixel so image[0,0] maps to viewport[1,1]
    QPointF viewportXYToImageXY(float vx, float vy)
    {
        float ix = (vx - width()/2.0 - 1) / scale_x + image_focus_x;
        float iy = (vy - height()/2.0 - 1) / scale_y + image_focus_y;
        // float ix = (vx - 1) / scale_x;
        // float iy = (vy - 1) / scale_y;
        return QPointF(ix, iy);
    }
    QPoint viewportXYToImageXY(const QPoint& vp)
    {
        QPointF ipf = viewportXYToImageXY(vp.x(), vp.y());
        return QPoint(int(ipf.x()), int(ipf.y()));
    }
    QPointF imageXYToViewportXY(float ix, float iy) {
        float vx = (ix - image_focus_x) * scale_x + width()/2.0 + 1;
        float vy = (iy - image_focus_y) * scale_y + height()/2.0 + 1;
        // float vx = (ix) * scale_x + 1;
        // float vy = (iy) * scale_y + 1;
        return QPointF(vx, vy);
    }
	
    void initHDRViewer(const V3DLONG *imgsz, const unsigned char *data1d, ImagePixelType imgdatatype);
    void recordColorChannelROIPos();

public slots:
    void do_HDRfilter();
    void do_HDRfilter_zslice();
    void copydata2disp(); // legacy func
    void updatePixmap();
    void setRedChannel();
    void setGreenChannel();
    void setBlueChannel();
    void setCurrentZSlice(int sliceNum);
    void updateROIsize(int boxSize);
    void annotationModelUpdate(QString updateType);
    void setHDRCheckState(int state);

    void setGammaBrightness(double gamma);
    void updateHDRView();

signals:
    void roiChanged();
    void curZsliceChanged(int);
    void curColorChannelChanged(NaZStackWidget::Color);
    void boxSizeChanged(int boxSize);
    void changedHDRCheckState(bool state);

public:
    V3DLONG sx, sy, sz, sc;

protected:
    void setColorChannel(NaZStackWidget::Color col);

    V3DLONG roi_top, roi_left, roi_bottom, roi_right; // ROI boundary of the search box
    float roi_min, roi_max; // local min and max in the search box

    // mouse events handler
    bool b_mouseleft, b_mouseright;
    bool b_mousemove;

    bool bMouseCurorIn, bMouseDone;

    QPoint startMousePosL, curMousePosL, startMousePosR, curMousePosR;
    QPoint endMousePosL, endMousePosR;
    QPoint startMousePos, endMousePos;
    int translateMouse_x, translateMouse_y;
    float translateMouse_scale;

    QPoint recStartMousePos[5], recEndMousePos[5]; //
    bool recMousePos[5], recCr[5];
    int recNum, recCopy;
    V3DLONG recZ[5];

    QPointF m_square_pos;
    QPointF m_offset;

    // disp geometry
    // int dispwidth, dispheight;
    // float dispscale;

    V3DLONG cx, cy, cz, cc, cr;
    V3DLONG cur_x, cur_y, cur_z, cur_c, pre_c;
    ImagePixelType datatype;

    void *pDispData1d; // display
    void *pData1d; // ori

    QPixmap pixmap;

    float min_img[5], max_img[5], scale_img[5]; // assume max color channel is 3

    // roi
    float min_roi[5], max_roi[5], scale_roi[5]; //
    V3DLONG start_x, end_x, start_y, end_y;

    // gui
    float ratio_x2y; // x/y
    // parameters for mapping screen coordinates to image coordinates
    // scale is viewport pixels per image voxel
    float scale_x, scale_y; // assume scale_x = scale_y, i.e. keep the ratio x to y
    // image coordinates of image point in center of viewport
    float image_focus_x, image_focus_y;

    // on/off HDR filter
    bool runHDRFILTER;

    BrightnessCalibrator<float> brightnessCalibrator; // gamma correction
};

#endif // NA_ZSTACK_WIDGET_H
