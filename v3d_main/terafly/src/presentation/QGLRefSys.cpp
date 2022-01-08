
#include "renderer_gl1.h"

//#if defined(USE_Qt5)
//#include <QtCore>
//#include <QtGui>
//#endif
#include "QGLRefSys.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

using namespace terafly;
// from renderer_obj.cpp
const GLubyte neuron_type_color[ ][3] = {///////////////////////////////////////////////////////
        {255, 255, 255},  // white,   0-undefined
        {20,  20,  0 },  // black,   1-soma
        {200, 20,  0  },  // red,     2-axon
        {0,   20,  200},  // blue,    3-dendrite
        {200, 0,   200},  // purple,  4-apical dendrite
        //the following is Hanchuan's extended color. 090331
        {0,   200, 200},  // cyan,    5
        {220, 200, 0  },  // yellow,  6
        {0,   200, 20 },  // green,   7
        {188, 94,  37 },  // coffee,  8
        {180, 200, 120},  // asparagus,	9
        {250, 100, 120},  // salmon,	10
        {120, 200, 200},  // ice,		11
        {100, 120, 200},  // orchid,	12
        //the following is Hanchuan's further extended color. 111003
        {255, 128, 168},  //	13
        {128, 255, 168},  //	14
        {128, 168, 255},  //	15
        {168, 255, 128},  //	16
        {255, 168, 128},  //	17
        {168, 128, 255}, //	18
         {0, 0, 0}, //19 //totally black. PHC, 2012-02-15
         //the following (20-275) is used for matlab heat map. 120209 by WYN
         {0,0,131}, //20
                                         {0,0,135},
                                         {0,0,139},
                                         {0,0,143},
                                         {0,0,147},
                                         {0,0,151},
                                         {0,0,155},
                                         {0,0,159},
                                         {0,0,163},
                                         {0,0,167},
                                         {0,0,171},
                                         {0,0,175},
                                         {0,0,179},
                                         {0,0,183},
                                         {0,0,187},
                                         {0,0,191},
                                         {0,0,195},
                                         {0,0,199},
                                         {0,0,203},
                                         {0,0,207},
                                         {0,0,211},
                                         {0,0,215},
                                         {0,0,219},
                                         {0,0,223},
                                         {0,0,227},
                                         {0,0,231},
                                         {0,0,235},
                                         {0,0,239},
                                         {0,0,243},
                                         {0,0,247},
                                         {0,0,251},
                                         {0,0,255},
                                         {0,3,255},
                                         {0,7,255},
                                         {0,11,255},
                                         {0,15,255},
                                         {0,19,255},
                                         {0,23,255},
                                         {0,27,255},
                                         {0,31,255},
                                         {0,35,255},
                                         {0,39,255},
                                         {0,43,255},
                                         {0,47,255},
                                         {0,51,255},
                                         {0,55,255},
                                         {0,59,255},
                                         {0,63,255},
                                         {0,67,255},
                                         {0,71,255},
                                         {0,75,255},
                                         {0,79,255},
                                         {0,83,255},
                                         {0,87,255},
                                         {0,91,255},
                                         {0,95,255},
                                         {0,99,255},
                                         {0,103,255},
                                         {0,107,255},
                                         {0,111,255},
                                         {0,115,255},
                                         {0,119,255},
                                         {0,123,255},
                                         {0,127,255},
                                         {0,131,255},
                                         {0,135,255},
                                         {0,139,255},
                                         {0,143,255},
                                         {0,147,255},
                                         {0,151,255},
                                         {0,155,255},
                                         {0,159,255},
                                         {0,163,255},
                                         {0,167,255},
                                         {0,171,255},
                                         {0,175,255},
                                         {0,179,255},
                                         {0,183,255},
                                         {0,187,255},
                                         {0,191,255},
                                         {0,195,255},
                                         {0,199,255},
                                         {0,203,255},

        };//////////////////////////////////////////////////////////////////////////////////
const int neuron_type_color_num = sizeof(neuron_type_color)/(sizeof(GLubyte)*3);

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
    dimXCenter=dimYCenter=dimZCenter=0;
    for(int i=0;i<3;i++)
    {
        dimSm[i]=1;
        dimMax[i]=1;
        dimMin[i]=1;
    }

    zoomFar=30.0;
    zoomNear=5.0;
    zoomInit=6;
    num_res=1;
    curRes=0;

    lenVoxel = 0;
    lenMicron = 0;
    numSegments = 0;
    vx = 0.2; vy = 0.2; vz = 1.0;

    setAttribute(Qt::WA_TranslucentBackground,true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );
    setWindowOpacity(0.5);
    bool t=testAttribute ( Qt::WA_TranslucentBackground);
    qDebug()<<"transparency"<<t;
}

QGLRefSys::~QGLRefSys()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    lenVoxel = 0;
    lenMicron = 0;
    numSegments = 0;
}

QSize QGLRefSys::minimumSizeHint() const
{
    return QSize(20, 20);
}

QSize QGLRefSys::sizeHint() const
{
    return QSize(400, 400);
}

//XYZ QGLRefSys::get3Dpoint(int x, int y)
//{
//    Renderer_gl1::MarkerPos pos;
//    pos.x = x;
//    pos.y = y;
//    for (int i=0; i<4; i++)
//            pos.view[i] = renderer->viewport[i];
//    for (int i=0; i<16; i++)
//    {
//        pos.P[i]  = renderer->projectionMatrix[i];
//        pos.MV[i] = renderer->markerViewMatrix[i];
//    }
//    return renderer->getCenterOfMarkerPos(pos);
//}

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

    bool dimGlEnable[]={false,false,false};

    //
    lenVoxel = 0;
    lenMicron = 0;
    numSegments = 0;

    //
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
//            qDebug("dimSm %f and dimMax %f and dimmin %f",dimSm[i],dimMax[i],dimMin[i]);
        }
        if(dimSm[0]>dimSm[1]&&dimSm[0]>dimSm[2])
        {
            xDim=1.0;
            yDim=static_cast<float> (dimSm[1])/dimSm[0];
            zDim=static_cast<float> (dimSm[2])/dimSm[0];
        }
        if(dimSm[1]>dimSm[0]&&dimSm[1]>dimSm[2])
        {
            yDim=1.0;
            xDim=static_cast<float> (dimSm[0])/dimSm[1];
            zDim=static_cast<float> (dimSm[2])/dimSm[1];
        }
        if(dimSm[2]>dimSm[1]&&dimSm[2]>dimSm[0])
        {
            zDim=1.0;
            yDim=static_cast<float> (dimSm[1])/dimSm[2];
            xDim=static_cast<float> (dimSm[0])/dimSm[2];
        }

        //
        double vx2 = vx*vx;
        double vy2 = vy*vy;
        double vz2 = vz*vz;

//        NeuronTree ntc;
//        ntc.deepCopy(nt);
        nt_init.deepCopy(nt);

        for(int i=0; i<nt.listNeuron.size();i++)
        {
            // length
            NeuronSWC curr = nt_init.listNeuron.at(i);
            if(curr.pn >= 0)
            {
                int parent = nt_init.hashNeuron.value(curr.pn);
                NeuronSWC parentNode = nt_init.listNeuron.at(parent);
                double l = sqrt((curr.x-parentNode.x)*(curr.x-parentNode.x)+(curr.y-parentNode.y)*(curr.y-parentNode.y)+(curr.z-parentNode.z)*(curr.z-parentNode.z));
                double lr = sqrt((curr.x-parentNode.x)*(curr.x-parentNode.x)*vx2+(curr.y-parentNode.y)*(curr.y-parentNode.y)*vy2+(curr.z-parentNode.z)*(curr.z-parentNode.z)*vz2);
                lenVoxel += l;
                lenMicron += lr;
            }

            //
            nt.listNeuron[i].x=(nt.listNeuron[i].x-dimMin[0])/dimSm[0];
            nt.listNeuron[i].y=(dimMax[1]-nt.listNeuron[i].y)/dimSm[1];
            nt.listNeuron[i].z=(dimMax[2]-nt.listNeuron[i].z)/dimSm[2];
            /*nt.listNeuron[i].x = nt.listNeuron[i].x / dimX;
            nt.listNeuron[i].y = (dimY-nt.listNeuron[i].y)  / dimY;
            nt.listNeuron[i].z = (dimZ-nt.listNeuron[i].z) / dimZ;*/

        }
        //update RoiDim and RoiShift for miniMap.added by shengdian 20180513
        {
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

            // calculate neuron stroks
            V_NeuronSWC_list nt_decomposed = NeuronTree__2__V_NeuronSWC_list(nt);
            numSegments = nt_decomposed.nsegs();
        }
        else
            miniMapCurBox=false;
    }
    else
        alreadyLoadSwc=false;
    /*xSoma=ySoma=zSoma=0;
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
    }*/
    //qDebug("xyzDim is %f and %f and %f.",xDim,yDim,zDim);
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
    glFrustum( -1.0, 1.0, -1.0, 1.0, zoomNear, zoomFar);
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
            glColor3f(0,0.0,0.0);
            glLineWidth(2.0);
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
        /*if(xSoma && ySoma && zSoma&&alreadyLoadSwc)
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
        }*/

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
        float delta = 0.1;
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

    //
    if(nt.listNeuron.size()>0)
    {
        glDisable(GL_DEPTH_TEST);
        for(int i=0; i<nt.listNeuron.size(); i++)
        {
            GLfloat ntr=neuron_type_color[(nt.listNeuron[i].type>=0&&nt.listNeuron[i].type<=neuron_type_color_num)? (nt.listNeuron[i].type):0][0];
            GLfloat ntg=neuron_type_color[(nt.listNeuron[i].type>=0&&nt.listNeuron[i].type<=neuron_type_color_num)? (nt.listNeuron[i].type):0][1];
            GLfloat ntb=neuron_type_color[(nt.listNeuron[i].type>=0&&nt.listNeuron[i].type<=neuron_type_color_num)? (nt.listNeuron[i].type):0][2];
            glColor3f(ntr/255,ntg/255,ntb/255);
            glPointSize(2);
            glBegin(GL_POINTS);
            glVertex3f(2*xDim*nt.listNeuron[i].x-xDim,2*yDim*nt.listNeuron[i].y-yDim,2*zDim*nt.listNeuron[i].z-zDim);
            glEnd();
        }
    }

    // display total length of swc
    char str[256];
    sprintf(str, "total length: %0.2lf voxels / %0.2lf um \nnumber of segments: %d", lenVoxel, lenMicron, numSegments);
    emit neuronInfoChanged(QString(str));
}

void QGLRefSys::mousePressEvent(QMouseEvent *event)
{

    //const double PI = 3.1415926535897;
    lastPos = event->pos();
    QRect parentWidgetRect= this->geometry();
    int centerWidth=parentWidgetRect.width()/2;
    int centerHeight=parentWidgetRect.height()/2;
    bool findSWCNode;
    float curClickPosx,curClickPosy,minProjectionx,minProjectiony,curSwcPosx,curSwcPosy,maxProjectionx,maxProjectiony,curProjectionPosx,curProjectionPosy;
    //NeuronSWC cur_node;
    if(event->type()==QEvent::MouseButtonDblClick)
    {
        if(event->button()==Qt::LeftButton&&alreadyLoadSwc)
        {
            if(xRot%180==0&&yRot%180==0&&zRot%180==0)
            {
                //qDebug("mouse position %d and y %d",lastPos.x(),lastPos.y());
//                XYZ mousepostiontest=get3Dpoint(lastPos.x(),lastPos.y());
//                qDebug("test 3d point of mouse position %d and %d and %d",mousepostiontest.x,mousepostiontest.y,mousepostiontest.z);
                curClickPosx=(float)(lastPos.x()-centerWidth)/centerWidth;
                curClickPosy=(float)(-lastPos.y()+centerHeight)/centerHeight;
                findSWCNode=false;
                //qDebug("cur mouse position %6f and y %6f",curClickPosx,curClickPosy);
//                curClickPosx/=xDim;
//                curClickPosy/=yDim;
//                qDebug("cur mouse position 2 %6f and y %6f",curClickPosx,curClickPosy);
                if(nt.listNeuron.size()>0&&nt_init.listNeuron.size()>0)
                {
                    minProjectionx=(abs(zoom)-zDim)*curClickPosx/(zoomNear);
                    maxProjectionx=(abs(zoom)+zDim)*curClickPosx/(zoomNear);
                    //minProjectionx=((abs(zoom)-zDim)/(abs(zoom)*xDim))*curClickPosx;
                    minProjectiony=((abs(zoom)-zDim)*curClickPosy/((zoomNear)));
                    maxProjectiony=((abs(zoom)+zDim)*curClickPosy/((zoomNear)));
                    //minProjectiony=(zoomNear/((abs(zoom)+zDim)*yDim))*curClickPosy;
                    //maxProjectionx=((abs(zoom)+zDim)/(abs(zoom)*xDim))*curClickPosx;
                    maxProjectionx=(abs(maxProjectionx)<=xDim)?maxProjectionx:((maxProjectionx>=0?1:(-1))*xDim);
                    //maxProjectiony=(zoomNear/((abs(zoom)-zDim)*yDim))*curClickPosy;
                    maxProjectiony=(abs(maxProjectiony)<=yDim)?maxProjectiony:((maxProjectiony>=0?1:(-1))*yDim);
                    //curProjectionPosx=-(zoom/zoomNear)*curClickPosx;//(cos(yRot*PI/180));
                    //curProjectionPosy=-(zoom/zoomNear)*curClickPosy;//(cos(xRot*PI/180));
                    //qDebug("cur mouse projection position min %6f and y %6f,and max %6f, %6f",minProjectionx,minProjectiony,maxProjectionx,maxProjectiony);

                    if(abs(minProjectionx)<=xDim&&
                            abs(minProjectiony)<=yDim&&
                            abs(maxProjectionx)<=xDim&&
                            abs(maxProjectiony)<=yDim)//
                    {
                        if(!renderer)return;
                        for(float ix=abs(maxProjectionx-minProjectionx)/5;ix<abs(maxProjectionx-minProjectionx);ix=ix+abs(maxProjectionx-minProjectionx)/5)
                        {
                            //if(findSWCNode) break;
                            for(float iy=abs(maxProjectiony-minProjectiony)/5;iy<abs(maxProjectiony-minProjectiony);iy=iy+abs(maxProjectiony-minProjectiony)/5)
                            {
                                //if(findSWCNode)break;
                                curProjectionPosx=maxProjectionx-ix;
                                curProjectionPosy=maxProjectiony-iy;
                                curSwcPosx=dimMin[0]+dimSm[0]*(xDim+curProjectionPosx)/(2*xDim);
                                curSwcPosy=dimMin[1]+dimSm[1]*(yDim-curProjectionPosy)/(2*yDim);
                                //qDebug("cur swc projection position %6f and y %6f",curSwcPosx,curSwcPosy);
                                //qDebug("dimxyz center is %d and %d and %d",dimXCenter,dimYCenter,dimZCenter);
                                /*V3DLONG i;*/double pow_x=2;double pow_xy=pow(pow_x,curRes+1-num_res);
                                for(V3DLONG i=0;i<nt_init.listNeuron.size();i++)
                                {
                                    NeuronSWC tempNeuron=nt_init.listNeuron.at(i);
                                    if((abs(tempNeuron.x-curSwcPosx)+abs(tempNeuron.y-curSwcPosy))<50)
                                    {
                                        //qDebug("cur swc projection position %6f and y %6f",curSwcPosx,curSwcPosy);
                                        XYZ cur_node_xyz = XYZ((V3DLONG)tempNeuron.x*pow_xy-dimXCenter*pow_xy,
                                                               (V3DLONG)tempNeuron.y*pow_xy-dimYCenter*pow_xy,
                                                               (V3DLONG)tempNeuron.z*pow_xy-dimZCenter*pow_xy);

                                        vector <XYZ> loc_vec;
                                        loc_vec.push_back(cur_node_xyz);
                                        findSWCNode=true;
                                        //qDebug("find one");
                                        renderer->b_grabhighrez = true;
                                        renderer->produceZoomViewOf3DRoi(loc_vec,0);
                                        break;
                                    }
                                }
                                if(findSWCNode)break;
                            }
                            if(findSWCNode)break;
                        }
                    }
//                    if(abs(minProjectionx)<maxProjectionx&&abs(minProjectiony)<maxProjectiony)
//                    {
//                        curSwcPosx=dimMin[0]+dimSm[0]*(xDim+curProjectionPosx)/(2*xDim);
//                        curSwcPosy=dimMin[1]+dimSm[1]*(yDim-curProjectionPosy)/(2*yDim);
//                        qDebug("cur swc projection position %6f and y %6f",curSwcPosx,curSwcPosy);
//                        //qDebug("dimxyz center is %d and %d and %d",dimXCenter,dimYCenter,dimZCenter);
//                        /*V3DLONG i;*/double pow_x=2;double pow_xy=pow(pow_x,curRes+1-num_res);
//                        for(V3DLONG i=0;i<nt_init.listNeuron.size();i++)
//                        {
//                            NeuronSWC tempNeuron=nt_init.listNeuron.at(i);
//                            if((abs(tempNeuron.x-curSwcPosx)+abs(tempNeuron.y-curSwcPosy))<150)
//                            {
//                                XYZ cur_node_xyz = XYZ((V3DLONG)tempNeuron.x*pow_xy-dimXCenter*pow_xy,
//                                                       (V3DLONG)tempNeuron.y*pow_xy-dimYCenter*pow_xy,
//                                                       (V3DLONG)tempNeuron.z*pow_xy-dimZCenter*pow_xy);

//                                vector <XYZ> loc_vec;
//                                loc_vec.push_back(cur_node_xyz);
//                                qDebug("find one");
//                                //renderer->b_grabhighrez = true;
//                                //renderer->produceZoomViewOf3DRoi(loc_vec,0);
//                                break;
//                            }
//                        }

//                    }
//                    else
//                        qDebug("worng position");

                }

            }
            else
            {
                xRot=0;//((abs(xRot%360)<=90)&&abs(xRot%360)>=270)?0:180;
                yRot=0;//(abs(yRot%360)<=90&&abs(yRot%360)>=270)?0:180;
                zRot=0;//(abs(zRot%360)<=90&&abs(zRot%360)>=270)?0:180;
                zoom=-zoomInit;
                updateGL();
            }
        }
    }
}

void QGLRefSys::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 4 * dy);
        setYRotation(yRot + 4 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 4 * dy);
        setZRotation(zRot + 4 * dx);
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
    zoom = std::min(zoom, -zoomInit);
//    printf("zoomFactor = %.0f\n", zoom);
    updateGL();
}

void QGLRefSys::enterEvent(QEvent *event)
{
    // emit reset();
}


