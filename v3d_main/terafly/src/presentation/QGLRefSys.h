#ifndef QGLREFSYS_H
#define QGLREFSYS_H

//#include "renderer_gl1.h"

#include <QGLWidget>
#include "../control/CPlugin.h"

class terafly::QGLRefSys : public QGLWidget
{
    Q_OBJECT

    private:

        int xRot;               //rotation angle (in degrees) along the X axis
        int yRot;               //rotation angle (in degrees) along the Y axis
        int zRot;               //rotation angle (in degrees) along the Z axis
        float xDim;             //x dimension (between 0.0 and 1.0)
        float yDim;             //y dimension (between 0.0 and 1.0)
        float zDim;             //z dimension (between 0.0 and 1.0)
        float ROIxDim;          //ROI x dimension (between 0.0 and 1.0)
        float ROIyDim;          //ROI y dimension (between 0.0 and 1.0)
        float ROIzDim;          //ROI z dimension (between 0.0 and 1.0)
        float ROIxShift;        //ROI x shift (between 0.0 and 1.0)
        float ROIyShift;        //ROI y shift (between 0.0 and 1.0)
        float ROIzShift;        //ROI z shift (between 0.0 and 1.0)
        QPoint lastPos;         //previous location of the mouse cursor to determine how much the object in the scene should be rotated, and in which direction
        double zoom;
        bool filled;            //equivalent to draw parallelepipedon faces (=true) or lines only (=false)
        float xSoma;           //First soma x location (between 0.0 and 1.0)
        float ySoma;           //First soma y location (between 0.0 and 1.0)
        float zSoma;           //First soma z location (between 0.0 and 1.0)
        //for miniMap.added by shengdian.20180513
        bool miniMapCurBox;
        bool alreadyLoadSwc;
        float miniROIxDim;          //ROI x dimension (between 0.0 and 1.0)
        float miniROIyDim;          //ROI y dimension (between 0.0 and 1.0)
        float miniROIzDim;          //ROI z dimension (between 0.0 and 1.0)
        float miniROIxShift;        //ROI x shift (between 0.0 and 1.0)
        float miniROIyShift;        //ROI y shift (between 0.0 and 1.0)
        float miniROIzShift;        //ROI z shift (between 0.0 and 1.0)
        float dimMin[3];
        float dimMax[3];
        float dimSm[3];
        //for miniMap zoom (double click)
        double zoomNear;            //init zoom near
        double zoomFar;             //init zoom far
        double vx, vy, vz;          // voxelsize


    public:

        QGLRefSys(QWidget *parent = 0);
        ~QGLRefSys();

        QSize minimumSizeHint() const;
        QSize sizeHint() const;
        int getXRot(){return xRot;}
        int getYRot(){return yRot;}
        int getZRot(){return zRot;}
        void setDims(int dimX, int dimY, int dimZ, int _ROIxDim=0, int _ROIyDim=0, int _ROIzDim=0, int _ROIxShift=0, int _ROIyShift=0, int _ROIzShift=0);
        void setRender(Renderer_gl1* _gl1){renderer=_gl1;}
        void setFilled(bool _filled){filled = _filled; nt.listNeuron.clear(); /*markList.clear();*/updateGL();}
        void setZoom(double _zoom){zoom = _zoom;}
        void resetZoom(){zoom = -15.0; updateGL();}
        void setVoxelSize(double x, double y, double z){vx = x; vy = y; vz = z;}
        Renderer_gl1 *renderer;
        NeuronTree nt,nt_init;
        //LandmarkList markList;
        int dimXCenter;
        int dimYCenter;
        int dimZCenter;
        int curRes;
        int num_res;
        double zoomInit;
        double lenVoxel, lenMicron;
        int numSegments;

    public slots:

        // can be connected to a signal to syncronize rotation
        void setXRotation(int angle);
        void setYRotation(int angle);
        void setZRotation(int angle);

    signals:

        // signals emitted when rotation change
        void xRotationChanged(int angle);
        void yRotationChanged(int angle);
        void zRotationChanged(int angle);

        void mouseReleased();
        void neuronInfoChanged(QString str);
        void reset();

    protected:

        // OpenGL initialization, viewport resizing, and painting methods MUST be overriden
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);

        // Qt mouse event handlers to control rotation and zoom-in/out with the mouse
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void wheelEvent(QWheelEvent *event);
        void enterEvent(QEvent *event);
        //XYZ get3Dpoint(int x,int y);


        int heightForWidth( int w ) { return w; }
};

#endif // QGLREFSYS_H
