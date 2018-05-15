#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
#include <QtCore>
#include <QtGui>
#endif
#include "QGLRefSys.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

using namespace terafly;

QGLRefSys::QGLRefSys(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

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
    miniMapCurBox=false;
    alreadyLoadSwc=false;
    miniROIxDim = 0.0f;
    miniROIyDim = 0.0f;
    miniROIzDim = 0.0f;
    miniROIxShift = 0.0f;
    miniROIyShift = 0.0f;
    miniROIzShift = 0.0f;

    setAttribute(Qt::WA_TranslucentBackground,true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );
    setWindowOpacity(0.5);
    bool t=testAttribute ( Qt::WA_TranslucentBackground);
    qDebug()<<"transparency"<<t;
}

QGLRefSys::~QGLRefSys()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);
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
    /**/tf::debug(tf::LEV_MAX, strprintf("dim = {%d, %d, %d}, ROIdim = {%d, %d, %d}, ROIshift = {%d, %d, %d}",
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

    float dimMin[]={1,1,1};
    float dimMax[]={1,1,1};
    float dimSm[]={1,1,1};
    bool dimGlEnable[]={false,false,false};
    if(nt.listNeuron.size()>0)
    {
        alreadyLoadSwc=true;
        dimMin[0]=nt.listNeuron[0].x;
        dimMax[0]=nt.listNeuron[0].x;
        dimMin[1]=nt.listNeuron[0].y;
        dimMax[1]=nt.listNeuron[0].y;
        dimMin[2]=nt.listNeuron[0].z;
        dimMax[2]=nt.listNeuron[0].z;
        for(int i=1;i<nt.listNeuron.size();i=i+5)
        {
            if(dimMin[0]>nt.listNeuron[i].x)
                dimMin[0]=nt.listNeuron[i].x;
            if(dimMax[0]<nt.listNeuron[i].x)
                dimMax[0]=nt.listNeuron[i].x;
            if(dimMin[1]>nt.listNeuron[i].y)
                dimMin[1]=nt.listNeuron[i].y;
            if(dimMax[1]<nt.listNeuron[i].y)
                dimMax[1]=nt.listNeuron[i].y;
            if(dimMin[2]>nt.listNeuron[i].z)
                dimMin[2]=nt.listNeuron[i].z;
            if(dimMax[2]<nt.listNeuron[i].z)
                dimMax[2]=nt.listNeuron[i].z;
        }
        for(int i=0;i<3;i++)
        {
            dimSm[i]=dimMax[i]-dimMin[i];
        }
        for(int i=0; i<nt.listNeuron.size();i++)
        {
            nt.listNeuron[i].x=(nt.listNeuron[i].x-dimMin[0])/dimSm[0];
            nt.listNeuron[i].y=(dimMax[1]-nt.listNeuron[i].y)/dimSm[1];
            nt.listNeuron[i].z=(dimMax[2]-nt.listNeuron[i].z)/dimSm[2];
            /*nt.listNeuron[i].x = nt.listNeuron[i].x / dimX;
            nt.listNeuron[i].y = (dimY-nt.listNeuron[i].y)  / dimY;
            nt.listNeuron[i].z = (dimZ-nt.listNeuron[i].z) / dimZ;*/

        }
        //update RoiDim and RoiShift for miniMap.added by shengdian 20180513
        if(_ROIxShift<=dimMin[0]&&(_ROIxShift+_ROIxDim)>=dimMin[0]&&(_ROIxShift+_ROIxDim)<=dimMax[0])
        {
            dimGlEnable[0]=true;
            _ROIxDim=_ROIxDim-(dimMin[0]-_ROIxShift);
            _ROIxShift=dimMin[0];
        }
        else if(_ROIxShift>=dimMin[0]&&(_ROIxShift+_ROIxDim)<=dimMax[0])
        {
            dimGlEnable[0]=true;
        }
        else if(_ROIxShift<dimMin[0]&&(_ROIxDim+_ROIxShift)>dimMax[0])
        {
            dimGlEnable[0]=true;
            _ROIxDim=dimSm[0];
            _ROIxShift=dimMin[0];
        }
        else if(_ROIxShift<=dimMax[0]&&(_ROIxShift+_ROIxDim)>dimMax[0])
        {
            dimGlEnable[0]=true;
            _ROIxDim=_ROIxDim-(_ROIxShift+_ROIxDim-dimMax[0]);
        }
        else
        {
            dimGlEnable[0]=false;
        }
        //RoiyDim
        if(_ROIyShift<=dimMin[1]&&(_ROIyShift+_ROIyDim)>dimMin[1]&&(_ROIyShift+_ROIyDim)<=dimMax[1])
        {
            dimGlEnable[1]=true;
            _ROIyDim=_ROIyDim-(dimMin[1]-_ROIyShift);
            _ROIyShift=dimMin[1];
            qDebug()<<"roiydim"<<_ROIyDim<<","<<_ROIyShift;
        }
        else if(_ROIyShift>=dimMin[1]&&(_ROIyShift+_ROIyDim)<=dimMax[1])
        {
            dimGlEnable[1]=true;
        }
        else if(_ROIyShift<dimMin[1]&&(_ROIyDim+_ROIyShift)>dimMax[1])
        {
            dimGlEnable[1]=true;
            _ROIyDim=dimSm[1];
            _ROIyShift=dimMin[1];
        }
        else if(_ROIyShift<dimMax[1]&&(_ROIyShift+_ROIyDim)>dimMax[1])
        {
            dimGlEnable[1]=true;
            _ROIyDim=_ROIyDim-(_ROIyShift+_ROIyDim-dimMax[1]);
        }
        else
        {
            dimGlEnable[1]=false;
        }
        //RoizDim
        if(_ROIzShift<=dimMin[2]&&(_ROIzShift+_ROIzDim)>=dimMin[2]&&(_ROIzShift+_ROIzDim)<=dimMax[2])
        {
            dimGlEnable[2]=true;
            _ROIzDim=_ROIzDim-(dimMin[2]-_ROIzShift);
            _ROIzShift=dimMin[2];
        }
        else if(_ROIzShift>=dimMin[2]&&(_ROIzShift+_ROIzDim)<=dimMax[2])
        {
            dimGlEnable[2]=true;
        }
        else if(_ROIzShift<dimMin[2]&&(_ROIzDim+_ROIzShift)>dimMax[2])
        {
            dimGlEnable[2]=true;
            _ROIzDim=dimSm[2];
            _ROIzShift=dimMin[2];
        }
        else if(_ROIzShift<=dimMax[2]&&(_ROIzShift+_ROIzDim)>dimMax[2])
        {
            dimGlEnable[2]=true;
            _ROIzDim=_ROIzDim-(_ROIzShift+_ROIzDim-dimMax[2]);
        }
        else
        {
            dimGlEnable[2]=false;
        }
        if(dimGlEnable[0]&&dimGlEnable[1]&&dimGlEnable[2])
        {
            miniMapCurBox=true;
            miniROIxDim   = (_ROIxDim) / dimSm[0];
            miniROIxShift = ((_ROIxShift-dimMin[0])) / dimSm[0];
            miniROIyDim   = ((_ROIyDim  ) / dimSm[1]);
            miniROIyShift = ((-dimMin[1]+_ROIyShift)) / dimSm[1];
            miniROIzDim   = (_ROIzDim   ) / dimSm[2];
            miniROIzShift = ((-dimMin[2]+_ROIzShift)) / dimSm[2];
        }
        else
            miniMapCurBox=false;
    }
    else
        alreadyLoadSwc=false;
    xSoma=ySoma=zSoma=0;
    if(markList.size()>0)
    {
        for(int i=0; i< markList.size();i++)
        {
            if(markList.at(i).comments == "soma")
            {
                xSoma = ((markList.at(i).x-dimMin[0]) * xDim) / dimSm[0];
                ySoma = ((markList.at(i).y-dimMin[1]) * yDim) / dimSm[1];
                zSoma = ((markList.at(i).z-dimMin[2]) * zDim) / dimSm[2];
            }
        }
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
    else if(miniROIxDim && miniROIyDim && miniROIzDim)
    {
        // ROI facesdimX
        if(miniMapCurBox&&alreadyLoadSwc)
        {
            glColor3f(1.00,1.00,1.00);
            glBegin(GL_QUADS);
                //---------front-----------//
                glVertex3f(2*xDim*miniROIxShift-xDim,           -2*yDim*miniROIyShift+yDim,            -2*zDim*miniROIzShift+zDim);
                glVertex3f(2*xDim*(miniROIxShift+miniROIxDim)-xDim, -2*yDim*miniROIyShift+yDim,            -2*zDim*miniROIzShift+zDim);
                glVertex3f(2*xDim*(miniROIxShift+miniROIxDim)-xDim, -2*(miniROIyShift+miniROIyDim)*yDim+yDim,  -2*zDim*miniROIzShift+zDim);
                glVertex3f(2*xDim*miniROIxShift-xDim,          -2*(miniROIyShift+miniROIyDim)*yDim+yDim,  -2*zDim*miniROIzShift+zDim);
                //---------back----------//
                glVertex3f(2*xDim*miniROIxShift-xDim,           -2*yDim*miniROIyShift+yDim,            -2*zDim*(miniROIzDim+miniROIzShift)+zDim);
                glVertex3f(2*xDim*(miniROIxShift+miniROIxDim)-xDim, -2*yDim*miniROIyShift+yDim,            -2*zDim*(miniROIzDim+miniROIzShift)+zDim);
                glVertex3f(2*xDim*(miniROIxShift+miniROIxDim)-xDim, -2*(miniROIyShift+miniROIyDim)*yDim+yDim,  -2*zDim*(miniROIzDim+miniROIzShift)+zDim);
                glVertex3f(2*xDim*miniROIxShift-xDim,          -2*(miniROIyShift+miniROIyDim)*yDim+yDim,  -2*zDim*(miniROIzDim+miniROIzShift)+zDim);
                //---------top-----------//
                glVertex3f(2*xDim*miniROIxShift-xDim,           -2*yDim*miniROIyShift+yDim,            -2*zDim*miniROIzShift+zDim);
                glVertex3f(2*xDim*(miniROIxShift+miniROIxDim)-xDim, -2*yDim*miniROIyShift+yDim,            -2*zDim*miniROIzShift+zDim);
                glVertex3f(2*xDim*(miniROIxShift+miniROIxDim)-xDim,-2*yDim*miniROIyShift+yDim,  -2*zDim*(miniROIzDim+miniROIzShift)+zDim);
                glVertex3f(2*xDim*miniROIxShift-xDim,           -2*yDim*miniROIyShift+yDim,  -2*zDim*(miniROIzDim+miniROIzShift)+zDim);
                //-------bottom----------//
                glVertex3f(2*xDim*miniROIxShift-xDim,           -2*(miniROIyShift+miniROIyDim)*yDim+yDim,            -2*zDim*miniROIzShift+zDim);
                glVertex3f(2*xDim*(miniROIxShift+miniROIxDim)-xDim, -2*(miniROIyShift+miniROIyDim)*yDim+yDim,          -2*zDim*miniROIzShift+zDim);
                glVertex3f(2*xDim*(miniROIxShift+miniROIxDim)-xDim, -2*(miniROIyShift+miniROIyDim)*yDim+yDim,  -2*zDim*(miniROIzDim+miniROIzShift)+zDim);
                glVertex3f(2*xDim*miniROIxShift-xDim,          -2*(miniROIyShift+miniROIyDim)*yDim+yDim,  -2*zDim*(miniROIzDim+miniROIzShift)+zDim);
                //--------right----------//
                glVertex3f(2*xDim*miniROIxShift-xDim,           -2*yDim*miniROIyShift+yDim,            -2*zDim*miniROIzShift+zDim);
                glVertex3f(2*xDim*miniROIxShift-xDim,           -2*yDim*miniROIyShift+yDim,  -2*zDim*(miniROIzDim+miniROIzShift)+zDim);
                glVertex3f(2*xDim*miniROIxShift-xDim,           -2*(miniROIyShift+miniROIyDim)*yDim+yDim,  -2*zDim*(miniROIzDim+miniROIzShift)+zDim);
                glVertex3f(2*xDim*miniROIxShift-xDim,           -2*(miniROIyShift+miniROIyDim)*yDim+yDim,            -2*zDim*miniROIzShift+zDim);
                //---------left----------//
                glVertex3f(2*xDim*(miniROIxShift+miniROIxDim)-xDim, -2*yDim*miniROIyShift+yDim,            -2*zDim*miniROIzShift+zDim);
                glVertex3f(2*xDim*(miniROIxShift+miniROIxDim)-xDim,-2*yDim*miniROIyShift+yDim,  -2*zDim*(miniROIzDim+miniROIzShift)+zDim);
                glVertex3f(2*xDim*(miniROIxShift+miniROIxDim)-xDim, -2*(miniROIyShift+miniROIyDim)*yDim+yDim, -2*zDim*(miniROIzDim+miniROIzShift)+zDim);
                glVertex3f(2*xDim*(miniROIxShift+miniROIxDim)-xDim,-2*(miniROIyShift+miniROIyDim)*yDim+yDim,            -2*zDim*miniROIzShift+zDim);
            glEnd();

            // ROI contour
            glColor3f(255.0,0.0,0.0);
            glLineWidth(1.0);
            glBegin(GL_LINES);
                //-------top lines---------//
                glVertex3f(-xDim+2*xDim*(miniROIxShift+miniROIxDim), yDim-2*yDim*miniROIyShift,           zDim-2*zDim*(miniROIzDim+miniROIzShift));
                glVertex3f(-xDim+2*xDim*miniROIxShift,           yDim-2*yDim*miniROIyShift,           zDim-2*zDim*(miniROIzDim+miniROIzShift));
                glVertex3f(-xDim+2*xDim*miniROIxShift,           yDim-2*yDim*miniROIyShift,           zDim-2*zDim*(miniROIzDim+miniROIzShift));
                glVertex3f(-xDim+2*xDim*miniROIxShift,           yDim-2*yDim*miniROIyShift,           zDim-2*miniROIzShift*zDim);
                glVertex3f(-xDim+2*xDim*miniROIxShift,           yDim-2*yDim*miniROIyShift,           zDim-2*miniROIzShift*zDim);
                glVertex3f(-xDim+2*xDim*(miniROIxShift+miniROIxDim), yDim-2*yDim*miniROIyShift,           zDim-2*miniROIzShift*zDim);
                glVertex3f(-xDim+2*xDim*(miniROIxShift+miniROIxDim), yDim-2*yDim*miniROIyShift,           zDim-2*miniROIzShift*zDim);
                glVertex3f(-xDim+2*xDim*(miniROIxShift+miniROIxDim), yDim-2*yDim*miniROIyShift,           zDim-2*zDim*(miniROIzDim+miniROIzShift));
                //------bottom lines-------//
                glVertex3f(-xDim+2*xDim*(miniROIxShift+miniROIxDim), yDim-2*(miniROIyShift+miniROIyDim)*yDim, zDim-2*zDim*(miniROIzDim+miniROIzShift));
                glVertex3f(-xDim+2*xDim*miniROIxShift,           yDim-2*(miniROIyShift+miniROIyDim)*yDim, zDim-2*zDim*(miniROIzDim+miniROIzShift));
                glVertex3f(-xDim+2*xDim*miniROIxShift,           yDim-2*(miniROIyShift+miniROIyDim)*yDim, zDim-2*zDim*(miniROIzDim+miniROIzShift));
                glVertex3f(-xDim+2*xDim*miniROIxShift,           yDim-2*(miniROIyShift+miniROIyDim)*yDim, zDim-2*miniROIzShift*zDim);
                glVertex3f(-xDim+2*xDim*miniROIxShift,           yDim-2*(miniROIyShift+miniROIyDim)*yDim, zDim-2*miniROIzShift*zDim);
                glVertex3f(-xDim+2*xDim*(miniROIxShift+miniROIxDim), yDim-2*(miniROIyShift+miniROIyDim)*yDim, zDim-2*miniROIzShift*zDim);
                glVertex3f(-xDim+2*xDim*(miniROIxShift+miniROIxDim), yDim-2*(miniROIyShift+miniROIyDim)*yDim, zDim-2*miniROIzShift*zDim);
                glVertex3f(-xDim+2*xDim*(miniROIxShift+miniROIxDim), yDim-2*(miniROIyShift+miniROIyDim)*yDim, zDim-2*zDim*(miniROIzDim+miniROIzShift));
                //--------side lines-------//
                glVertex3f(-xDim+2*xDim*(miniROIxShift+miniROIxDim), yDim-2*yDim*miniROIyShift,           zDim-2*zDim*(miniROIzDim+miniROIzShift));
                glVertex3f(-xDim+2*xDim*(miniROIxShift+miniROIxDim), yDim-2*(miniROIyShift+miniROIyDim)*yDim, zDim-2*zDim*(miniROIzDim+miniROIzShift));
                glVertex3f(-xDim+2*xDim*miniROIxShift,           yDim-2*yDim*miniROIyShift,           zDim-2*zDim*(miniROIzDim+miniROIzShift));
                glVertex3f(-xDim+2*xDim*miniROIxShift,           yDim-2*(miniROIyShift+miniROIyDim)*yDim, zDim-2*zDim*(miniROIzDim+miniROIzShift));
                glVertex3f(-xDim+2*xDim*miniROIxShift,           yDim-2*yDim*miniROIyShift,           zDim-2*miniROIzShift*zDim);
                glVertex3f(-xDim+2*xDim*miniROIxShift,           yDim-2*(miniROIyShift+miniROIyDim)*yDim, zDim-2*miniROIzShift*zDim);
                glVertex3f(-xDim+2*xDim*(miniROIxShift+miniROIxDim), yDim-2*yDim*miniROIyShift,           zDim-2*miniROIzShift*zDim);
                glVertex3f(-xDim+2*xDim*(miniROIxShift+miniROIxDim), yDim-2*(miniROIyShift+miniROIyDim)*yDim, zDim-2*miniROIzShift*zDim);
            glEnd(); // GL_LINES
        }
        else if(!alreadyLoadSwc)
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
                    glColor3f(255.0,0.0,0.0);
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
        if(xSoma && ySoma && zSoma&&alreadyLoadSwc)
        {
            glDisable(GL_DEPTH_TEST);
            // Soma faces
            glColor3f(1.00,1.00,1.00);
            glBegin(GL_QUADS);
                //---------front-----------//
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma,            zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma+0.04,      yDim-2*ySoma,            zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma+0.04,      yDim-2*ySoma-0.04,       zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma-0.04,       zDim-2*zSoma);
                //---------back----------//
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma,            zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma+0.04,      yDim-2*ySoma,            zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma+0.04,      yDim-2*ySoma-0.04,       zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma-0.04,       zDim-2*zSoma-0.04);
                //---------top-----------//
                glVertex3f(-xDim+2*xSoma+0.04,      yDim-2*ySoma,            zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma,            zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma,            zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma+0.04,      yDim-2*ySoma,            zDim-2*zSoma-0.04);
                //-------bottom----------//
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma-0.04,       zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma-0.04,       zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma+0.04,      yDim-2*ySoma-0.04,       zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma+0.04,      yDim-2*ySoma-0.04,       zDim-2*zSoma);
                //--------right----------//
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma,            zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma-0.04,       zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma-0.04,       zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma,            zDim-2*zSoma-0.04);
                //---------left----------//
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma,            zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma-0.04,       zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma-0.04,       zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma,            zDim-2*zSoma-0.04);
            glEnd();

            // Soma contour
            glColor3f(0.0,0.0,0.0);
            glLineWidth(1.0);
            glBegin(GL_LINES);
                //-------top lines---------//
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma,           zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma,           zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma,           zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma,           zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma,           zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma,           zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma,           zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma,           zDim-2*zSoma-0.04);
                //------bottom lines-------//
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma-0.04, zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma-0.04, zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma-0.04, zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma-0.04, zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma-0.04, zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma-0.04, zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma-0.04, zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma-0.04, zDim-2*zSoma-0.04);
                //--------side lines-------//
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma,           zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma-0.04, zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma,           zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma-0.04, zDim-2*zSoma-0.04);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma,           zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma,           yDim-2*ySoma-0.04, zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma,           zDim-2*zSoma);
                glVertex3f(-xDim+2*xSoma+0.04, yDim-2*ySoma-0.04, zDim-2*zSoma);
            glEnd(); // GL_LINES
        }

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


    if(nt.listNeuron.size()>0)
    {
        glDisable(GL_DEPTH_TEST);
        for(int i=0; i<nt.listNeuron.size();i+=2)
        {
            //color info need to complete
            /*"\n 0 -- undefined (white)"
              "\n 1 -- soma (black)"
              "\n 2 -- axon (red)"
              "\n 3 -- dendrite (blue)"
              "\n 4 -- apical dendrite (purple)"
              "\n else -- custom \n"),*/
            switch(nt.listNeuron[i].type)
            {
            case 0:
                glColor3f(255.0,255.0,255.0);
                break;
            case 1:
                glColor3f(0,0,0);
                break;
            case 2:
                glColor3f(255,0,0);
                break;
            case 3:
                glColor3f(0.0,0.0,255);
                break;
            case 4:
                glColor3f(128,0,128);
                break;
            default:
                glColor3f(255.0,255.0,0);
            }
            glPointSize(2);
            glBegin(GL_POINTS);
            glVertex3f(2*xDim*nt.listNeuron[i].x-xDim,2*yDim*nt.listNeuron[i].y-yDim,2*zDim*nt.listNeuron[i].z-zDim);
            glEnd();
        }
        /*glColor3f(0.0,0.0,255.0);
        glPointSize(2);
        glBegin(GL_POINTS);
        for(int i=0; i<nt.listNeuron.size();i+=5)
        {
            nt.listNeuron[i].color.r;
            glVertex3f(2*xDim*nt.listNeuron[i].x-xDim,2*yDim*nt.listNeuron[i].y-yDim,2*zDim*nt.listNeuron[i].z-zDim);
        }
        glEnd();*/
    }

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
