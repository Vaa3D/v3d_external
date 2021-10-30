#ifndef NA_ZSTACK_WIDGET_H
#define NA_ZSTACK_WIDGET_H

class ZSliceColors;

#include <QObject>
#include <QWidget>
#include <QSize>
class QPaintEvent;
class QPainter;
class QEvent;
class QPointF;
class QSizeF;
class QRectF;

#include "Na2DViewer.h"
#include "NeuronContextMenu.h"

// NaZStackWidget is a viewer for successive slices of a 3D volume.
// NaZStackWidget is based on HDRViewer class created by Yang Yu,
// refactored to be a standalone class for use in Neuron Annotator.

#define NCLRCHNNL 5

class NaZStackWidget : public Na2DViewer
{
    Q_OBJECT

public:
    enum Color {
        COLOR_RED = 1,
        COLOR_GREEN = 2,
        COLOR_BLUE = 3,
        COLOR_NC82 = 4
    };

    static const int minHdrBoxSize = 3;

    NaZStackWidget(QWidget* parent);
    virtual ~NaZStackWidget();
    void setContextMenus(QMenu* viewerMenu, NeuronContextMenu* neuronMenu);
    int neuronAt(const QPoint& point) const;
    void setZSliceColors(const ZSliceColors * zSliceColorsParam);
    void setVolumeData(const NaVolumeData * volumeDataParam);
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
    int getCurrentZSlice(); // 1-based slice index
    int getCurrentBoxSize();
    // Convert between image data coordinates and screen coordinates.
    //  * scale by scale_x, scale_y
    //  * translate so image_focus_[xy] is in center of viewport
    //  * shift by one pixel so image[0,0] maps to viewport[1,1]
    QPointF viewportXYToImageXY(float vx, float vy);
    QPoint viewportXYToImageXY(const QPoint& vp);
    QPointF imageXYToViewportXY(float ix, float iy);
    void recordColorChannelROIPos();
    virtual void setDataFlowModel(const DataFlowModel* dataFlowModel);

public slots:
    void do_HDRfilter();
    void updatePixmap();
    void setRedChannel();
    void setGreenChannel();
    void setBlueChannel();
    void setNc82Channel();
    void setCurrentZSlice(int sliceNum);
    bool setHdrBoxSize(int boxSize);
    void setHDRCheckState(bool state);
    void updateVolumeParameters();
    void updateHDRView();
    void showContextMenu(QPoint);

signals:
    void curZsliceChanged(int);
    void curColorChannelChanged(NaZStackWidget::Color);
    void hdrBoxSizeChanged(int boxSize);
    void changedHDRCheckState(bool state);
    void hdrRangeChanged(int channel, qreal min, qreal max);
    void statusMessage(const QString&);

protected slots:
    void onMouseLeftDragEvent(int dx, int dy, QPoint pos);
    void resetHdrBox();

protected:
    void updateCursor();
    void setColorChannel(NaZStackWidget::Color col);
    void paintIntensityNumerals(QPainter& painter);

    const ZSliceColors * zSliceColors;
    const NaVolumeData * volumeData;

    V3DLONG roi_top, roi_left, roi_bottom, roi_right; // ROI boundary of the search box
    bool roiDrawed;

    // mouse events handler
    bool b_mouseleft, b_mouseright;
    bool b_mousemove;

    bool bMouseCurorIn, bMouseDone;

    QPoint startMousePosL, curMousePosL, startMousePosR, curMousePosR;
    QPoint endMousePosL, endMousePosR;
    QPoint startMousePos, endMousePos;
    int translateMouse_x, translateMouse_y;
    float translateMouse_scale;

    QPoint recStartMousePos[NCLRCHNNL], recEndMousePos[NCLRCHNNL]; //
    bool recMousePos[NCLRCHNNL]; // whether box parameters are saved for each channel
    int recCr[NCLRCHNNL]; // box size for each channel
    int recNum, recCopy;
    V3DLONG recZ[NCLRCHNNL];

    QPointF m_square_pos;
    QPointF m_offset;

    // disp geometry
    // int dispwidth, dispheight;
    // float dispscale;

    V3DLONG sx, sy, sz, sc; // dimensions of image
    V3DLONG cx, cy, cz, cc;
    // , cr;
    V3DLONG cur_x, cur_y, cur_z, cur_c, pre_c;
    V3DLONG hdrBoxSize;

    // roi
    std::vector<qreal> min_roi, max_roi;
    V3DLONG start_x, end_x, start_y, end_y;

    // on/off HDR filter
    bool runHDRFILTER;
    bool hdrfiltered[NCLRCHNNL];

    QMenu* viewerContextMenu;
    NeuronContextMenu* neuronContextMenu;

private:
    typedef Na2DViewer super;
};

#endif // NA_ZSTACK_WIDGET_H
