#ifdef USE_Qt5
#include <QtCore>
#include <QtGui>
#endif
#include "m_QGLRefSys.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

using namespace teramanager;

QGLRefSys::QGLRefSys(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    xRot = 0;
    yRot = 0;
    zRot = 0;
    xDim = 0.7f;
    yDim = 0.7f;
    zDim = 0.7f;
    ROIxDim = 0.0f;
    ROIyDim = 0.0f;
    ROIzDim = 0.0f;
    ROIxShift = 0.0f;
    ROIyShift = 0.0f;
    ROIzShift = 0.0f;
    filled = true;
    zoom = -15.0;

    setAttribute(Qt::WA_TranslucentBackground,true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );
    setWindowOpacity(0.5);
    bool t=testAttribute ( Qt::WA_TranslucentBackground);
    qDebug()<<"transparency"<<t;
}

QGLRefSys::~QGLRefSys()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);
}

QSize QGLRefSys::minimumSizeHint() const
{
    return QSize(20, 20);
}

QSize QGLRefSys::sizeHint() const
{
    return QSize(400, 400);
}

//static void qNormalizeAngle(int &angle)
//{
//    while (angle < 0)
//        angle += 360 * 16;
//    while (angle > 360 * 16)
//        angle -= 360 * 16;
//}


void QGLRefSys::setDims(int dimX, int dimY, int dimZ,
                        int _ROIxDim /* =0 */, int _ROIyDim /* = 0 */, int _ROIzDim /* = 0 */,
                        int _ROIxShift /* = 0 */, int _ROIyShift /* = 0 */, int _ROIzShift /* = 0 */)
{
    /**/itm::debug(itm::LEV_MAX, strprintf("dim = {%d, %d, %d}, ROIdim = {%d, %d, %d}, ROIshift = {%d, %d, %d}",
                                            dimX, dimY, dimZ, _ROIxDim, _ROIyDim, _ROIzDim, _ROIxShift, _ROIyShift, _ROIzShift).c_str(), __itm__current__function__);

    if(dimX >= dimY && dimX >= dimZ)
    {
        xDim = 1.0;
        yDim = static_cast<float>(dimY)/dimX;
        zDim = static_cast<float>(dimZ)/dimX;
    }
    else if(dimY >= dimX && dimY >= dimZ)
    {
        yDim = 1.0;
        xDim = static_cast<float>(dimX)/dimY;
        zDim = static_cast<float>(dimZ)/dimY;
    }
    else if (dimZ >= dimY && dimZ >= dimX)
    {
        zDim = 1.0;
        yDim = static_cast<float>(dimY)/dimZ;
        xDim = static_cast<float>(dimX)/dimZ;
    }

    if(_ROIxDim && _ROIyDim && _ROIzDim)
    {
        ROIxDim   = (_ROIxDim   * xDim) / dimX;
        ROIxShift = (_ROIxShift * xDim) / dimX;
        ROIyDim   = (_ROIyDim   * yDim) / dimY;
        ROIyShift = (_ROIyShift * yDim) / dimY;
        ROIzDim   = (_ROIzDim   * zDim) / dimZ;
        ROIzShift = (_ROIzShift * zDim) / dimZ;
    }
    updateGL();
}


void QGLRefSys::setXRotation(int angle)
{
    //qNormalizeAngle(angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void QGLRefSys::setYRotation(int angle)
{
    //qNormalizeAngle(angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void QGLRefSys::setZRotation(int angle)
{
    //qNormalizeAngle(angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void QGLRefSys::initializeGL()
{
    qglClearColor(QWidget::palette().color(QWidget::backgroundRole())); // background color
    glEnable(GL_DEPTH_TEST);       // activate the depth buffer
    glShadeModel(GL_SMOOTH);       // select shade model (can be either smooth or flat)
    glEnable(GL_MULTISAMPLE);      // activate multisample

    glEnable (GL_LINE_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
}


/*
 *  Sets up the OpenGL viewport, projection, etc. Gets called whenever the widget has been resized
 *  (and also when it is shown for the first time because all newly created widgets get a resize event automatically).
 */
void QGLRefSys::resizeGL (int width, int height)
{
    // define a square viewport based on the length of the smallest side of the widget to ensure that
    // the scene is not distorted if the widget has sides of unequal length
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 30.0);
    glMatrixMode(GL_MODELVIEW);
}

/*
 * Renders the OpenGL scene. Gets called whenever the widget needs to be updated.
 */
void QGLRefSys::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //delete color and depth buffer
    glLoadIdentity();                                   //resets projection and modelview matrices
    glTranslatef(0.0, 0.0, zoom);                      //move along z-axis
    glRotatef(xRot, 1.0, 0.0, 0.0);                     //rotate around x-axis
    glRotatef(yRot, 0.0, 1.0, 0.0);                     //rotate around y-axis
    glRotatef(zRot, 0.0, 0.0, 1.0);                     //rotate around z-axis

    // CUBE FACES
    if(this->isEnabled())
        glColor3f(1,1,1);
    else
        glColor3f(0.75,0.75,0.75);
    if(filled)
    {
        glBegin(GL_QUADS);
            //---------front-----------//
            glVertex3f(xDim,yDim,zDim);
            glVertex3f(-xDim,yDim,zDim);
            glVertex3f(-xDim,-yDim,zDim);
            glVertex3f(xDim,-yDim,zDim);
            //---------back----------//
            glVertex3f(xDim,yDim,-zDim);
            glVertex3f(-xDim,yDim,-zDim);
            glVertex3f(-xDim,-yDim,-zDim);
            glVertex3f(xDim,-yDim,-zDim);
            //---------top-----------//
            glVertex3f(-xDim,yDim,zDim);
            glVertex3f(xDim,yDim,zDim);
            glVertex3f(xDim,yDim,-zDim);
            glVertex3f(-xDim,yDim,-zDim);
            //-------bottom----------//
            glVertex3f(xDim,-yDim,zDim);
            glVertex3f(xDim,-yDim,-zDim);
            glVertex3f(-xDim,-yDim,-zDim);
            glVertex3f(-xDim,-yDim,zDim);
            //--------right----------//
            glVertex3f(xDim,yDim,zDim);
            glVertex3f(xDim,-yDim,zDim);
            glVertex3f(xDim,-yDim,-zDim);
            glVertex3f(xDim,yDim,-zDim);
            //---------left----------//
            glVertex3f(-xDim,yDim,zDim);
            glVertex3f(-xDim,-yDim,zDim);
            glVertex3f(-xDim,-yDim,-zDim);
            glVertex3f(-xDim,yDim,-zDim);
        glEnd();
    }
    else if(ROIxDim && ROIyDim && ROIzDim)
    {
        // ROI faces
        glColor3f(1.00,1.00,1.00);
        glBegin(GL_QUADS);
            //---------front-----------//
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift,            zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift,            zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift-2*ROIyDim,  zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift-2*ROIyDim,  zDim-2*ROIzShift);
            //---------back----------//
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift,            zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift,            zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift-2*ROIyDim,  zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift-2*ROIyDim,  zDim-2*ROIzShift-2*ROIzDim);
            //---------top-----------//
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift,            zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift,            zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift,            zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift,            zDim-2*ROIzShift-2*ROIzDim);
            //-------bottom----------//
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift-2*ROIyDim,  zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift-2*ROIyDim,  zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift-2*ROIyDim,  zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift-2*ROIyDim,  zDim-2*ROIzShift);
            //--------right----------//
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift,            zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift-2*ROIyDim,  zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift-2*ROIyDim,  zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift,            zDim-2*ROIzShift-2*ROIzDim);
            //---------left----------//
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift,            zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift-2*ROIyDim,  zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift-2*ROIyDim,  zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift,            zDim-2*ROIzShift-2*ROIzDim);
        glEnd();

        // ROI contour
        glColor3f(0.0,0.0,0.0);
        glLineWidth(1.0);
        glBegin(GL_LINES);
            //-------top lines---------//
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift,           zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift,           zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift,           zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift,           zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift,           zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift,           zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift,           zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift,           zDim-2*ROIzShift-2*ROIzDim);
            //------bottom lines-------//
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift-2*ROIyDim, zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift-2*ROIyDim, zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift-2*ROIyDim, zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift-2*ROIyDim, zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift-2*ROIyDim, zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift-2*ROIyDim, zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift-2*ROIyDim, zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift-2*ROIyDim, zDim-2*ROIzShift-2*ROIzDim);
            //--------side lines-------//
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift,           zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift-2*ROIyDim, zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift,           zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift-2*ROIyDim, zDim-2*ROIzShift-2*ROIzDim);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift,           zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift,           yDim-2*ROIyShift-2*ROIyDim, zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift,           zDim-2*ROIzShift);
            glVertex3f(-xDim+2*ROIxShift+2*ROIxDim, yDim-2*ROIyShift-2*ROIyDim, zDim-2*ROIzShift);
        glEnd(); // GL_LINES
    }

    // CUBE CONTOUR
    if(this->isEnabled())
        glColor3f(0.0,0.0,0.0);
    else
        glColor3f(0.5,0.5,0.5);
    glLineWidth(1.0);
    glBegin(GL_LINES);
        //-------top lines---------//
        glVertex3f(-xDim, yDim, -zDim);
        glVertex3f( xDim, yDim, -zDim);
        glVertex3f( xDim, yDim, -zDim);
        glVertex3f( xDim, yDim,  zDim);
        if(filled)
        {
            glVertex3f( xDim, yDim,  zDim);
            glVertex3f(-xDim, yDim,  zDim);
            glVertex3f(-xDim, yDim,  zDim);
            glVertex3f(-xDim, yDim, -zDim);
        }
        //------bottom lines-------//
        glVertex3f(-xDim, -yDim, -zDim);
        glVertex3f( xDim, -yDim, -zDim);
        glVertex3f( xDim, -yDim, -zDim);
        glVertex3f( xDim, -yDim,  zDim);
        glVertex3f( xDim, -yDim,  zDim);
        glVertex3f(-xDim, -yDim,  zDim);
        glVertex3f(-xDim, -yDim,  zDim);
        glVertex3f(-xDim, -yDim, -zDim);
        //--------side lines-------//
        glVertex3f(-xDim, yDim,  -zDim);
        glVertex3f(-xDim, -yDim, -zDim);
        glVertex3f( xDim, yDim,  -zDim);
        glVertex3f( xDim, -yDim, -zDim);
        glVertex3f( xDim, yDim,  zDim);
        glVertex3f( xDim, -yDim, zDim);
        if(filled)
        {
            glVertex3f(-xDim, yDim,  zDim);
            glVertex3f(-xDim, -yDim, zDim);
        }
    glEnd(); // GL_LINES

    // AXES
    double radius=0.2;
    double height=2.39;
    float headRadius = 2.3*radius;
    float headHeight = 0.7;
    const double PI = 3.1415926535897;
    double resolution = PI/200;
    double shift = 0.01;
    if(filled)
    {
        if(isEnabled())
            glColor3f(0.0,200.0/255,0.0);
        else
            glColor3f(0.6,0.6,0.6);
        //----------------------------------Y-head----------------------------------//
        glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0, -shift-height-headHeight, 0);  /* center */
            for (double i = 0; i <= 2 * PI; i += resolution)
                glVertex3f(headRadius * cos(i), -shift-height, headRadius * sin(i));
        glEnd();
        //----------------------------------Y-base----------------------------------//
    //    glBegin(GL_TRIANGLE_FAN);
    //        glVertex3f(0, shift, 0);  /* center */
    //        for (double i = 2 * PI; i >= 0; i -= resolution)
    //            glVertex3f(radius * cos(i), shift, radius * sin(i));
    //        glVertex3f(radius, shift+height, 0);
    //    glEnd();
        //-------------------------------Y-middle tube------------------------------//
        glBegin(GL_QUAD_STRIP);
            for (double i = 0; i <= 2 * PI; i += resolution)
            {
                glVertex3f(radius * cos(i), -shift-yDim, radius * sin(i));
                glVertex3f(radius * cos(i), -shift-height, radius * sin(i));
            }
            glVertex3f(radius, -shift-yDim, 0);
            glVertex3f(radius, -shift-height, 0);
        glEnd();


        if(isEnabled())
            glColor3f(1.0,0.0,0.0);
        else
            glColor3f(0.6,0.6,0.6);
        //----------------------------------X-head----------------------------------//
        glBegin(GL_TRIANGLE_FAN);
            glVertex3f(shift+height+headHeight, 0, 0);  /* center */
            for (double i = 0; i <= 2 * PI; i += resolution)
                glVertex3f(shift+height, headRadius * cos(i), headRadius * sin(i));
        glEnd();
        //----------------------------------X-base----------------------------------//
    //    glBegin(GL_TRIANGLE_FAN);
    //        glVertex3f(shift, 0, 0);  /* center */
    //        for (double i = 2 * PI; i >= 0; i -= resolution)
    //            glVertex3f(shift, radius * cos(i), radius * sin(i));
    //        glVertex3f(shift+height, radius, 0);
    //    glEnd();
        //-------------------------------X-middle tube------------------------------//
        glBegin(GL_QUAD_STRIP);
            for (double i = 0; i <= 2 * PI; i += resolution)
            {
                glVertex3f(shift+xDim, radius * cos(i), radius * sin(i));
                glVertex3f(shift+height, radius * cos(i), radius * sin(i));
            }
            /* close the loop back to zero degrees */
            glVertex3f(shift+xDim, radius, 0);
            glVertex3f(shift+height, radius, 0);
        glEnd();

        if(isEnabled())
            glColor3f(0.0,0.0,1.0);
        else
            glColor3f(0.6,0.6,0.6);
        //----------------------------------Z-head----------------------------------//
        glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0, 0, -shift-height-headHeight);  /* center */
            for (double i = 0; i <= 2 * PI; i += resolution)
                glVertex3f(headRadius * cos(i), headRadius * sin(i), -shift-height);
        glEnd();
        //----------------------------------Z-base----------------------------------//
    //    glBegin(GL_TRIANGLE_FAN);
    //        glVertex3f(0, shift, 0);  /* center */
    //        for (double i = 2 * PI; i >= 0; i -= resolution)
    //            glVertex3f(radius * cos(i), shift, radius * sin(i));
    //        glVertex3f(radius, shift+height, 0);
    //    glEnd();
        //-------------------------------Z-middle tube------------------------------//
        glBegin(GL_QUAD_STRIP);
            for (double i = 0; i <= 2 * PI; i += resolution)
            {
                glVertex3f(radius * cos(i), radius * sin(i), -shift-zDim);
                glVertex3f(radius * cos(i), radius * sin(i), -shift-height);
            }
            glVertex3f(radius, 0, -shift-zDim);
            glVertex3f(radius, 0, -shift-height);
        glEnd();
    }


    // INVERSE AXES

    glLineWidth(3.0);
    glBegin(GL_LINES);
    if(filled)
    {
        //----------------Y-axis--------------//
        if(isEnabled())
            glColor3f(0.0,200.0/255,0.0);
        else
            glColor3f(0.6,0.6,0.6);
        glVertex3f(0,shift+yDim,0);
        glVertex3f(0,shift+headHeight+height,0);
        //----------------X-axis--------------//
        if(isEnabled())
            glColor3f(1.0,0.0,0.0);
        else
            glColor3f(0.6,0.6,0.6);
        glVertex3f(-shift-xDim,0,0);
        glVertex3f(-shift-headHeight-height,0,0);
        //----------------Z-axis--------------//
        if(isEnabled())
            glColor3f(0.0,0.0,1.0);
        else
            glColor3f(0.6,0.6,0.6);
        glVertex3f(0,0,shift+zDim);
        glVertex3f(0,0,shift+headHeight+height);
    }
    else
    {
        float delta = 0.5;
        glLineWidth(4.0);
        //----------------Y-axis--------------//
        if(isEnabled())
            glColor3f(0.0,200.0/255,0.0);
        else
            glColor3f(0.6,0.6,0.6);
        glVertex3f(-xDim, yDim,  zDim);
        glVertex3f(-xDim, -yDim-delta, zDim);
        //----------------X-axis--------------//
        if(isEnabled())
            glColor3f(1.0,0.0,0.0);
        else
            glColor3f(0.6,0.6,0.6);
        glVertex3f( xDim+delta, yDim,  zDim);
        glVertex3f(-xDim, yDim,  zDim);

        //----------------Z-axis--------------//
        if(isEnabled())
            glColor3f(0.0,0.0,1.0);
        else
            glColor3f(0.6,0.6,0.6);
        glVertex3f(-xDim, yDim,  zDim);
        glVertex3f(-xDim, yDim, -zDim-delta);


    }
    glEnd();

}

void QGLRefSys::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void QGLRefSys::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 8 * dy);
        setZRotation(zRot + 8 * dx);
    }
    lastPos = event->pos();
}


void QGLRefSys::mouseReleaseEvent(QMouseEvent *event)
{
    emit mouseReleased();
}


void QGLRefSys::wheelEvent(QWheelEvent *event)
{
    zoom += event->delta() < 0 ? 1.0 : -1.0;
    zoom = std::max(zoom, -29.0);
    zoom = std::min(zoom, -6.0);
//    printf("zoomFactor = %.0f\n", zoom);
    updateGL();
}
