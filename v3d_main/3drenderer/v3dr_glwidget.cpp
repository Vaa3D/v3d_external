/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets, Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model, Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/****************************************************************************
** by Hanchuan Peng
** 2006-08-09
** 2006-09-03
** 2007-06-18: add a "HIGHLIGHT" class in the ano file which indicates which cell should be highlighted
** 2007-06-19: change the colors as 11 colors with names "COLOR00" to "COLOR10"
** 2007-08-08: add an automatic switch between the scale of worm atlas and fly embryo atlas if find the x-scale changes dramatically. wano v0.153
** 2008-07-06: line 282 add the second para as 0, for temporary convenience. Need to fix later
** 2008-09-27: add an exception catch for loading surface objects
** 2008-10-03: PHC change the GL version check for autosetting of 3D viewer, for compiling on Maci, Tiger with Qt 4.1.4
** Last update: 090221, PHC add the surface obj geo dialog
****************************************************************************/

#include "GLee2glew.h" ////2020-2-10
#include<QAction>
#include <QTimer>

#include "v3dr_glwidget.h"
#include "v3dr_surfaceDialog.h"
#include "v3dr_colormapDialog.h"
#include "v3dr_mainwindow.h"
#include "../terafly/src/control/CPlugin.h"
#include "../terafly/src/presentation/PMain.h"
#include "../v3d/vr_vaa3d_call.h"
// Dynamically choice a renderer
#include "renderer.h"
#include "renderer_gl1.h"
#include "renderer_gl2.h"
#include <QtGui>
#include <QCloseEvent>
#include <QDialog>

//#include <QGLFormat>

#include<QWidget>
#include <QMessageBox>
#include <QOpenGLFunctions_1_1>
bool V3dR_GLWidget::disableUndoRedo = false;
bool V3dR_GLWidget::skipFormat = false; // 201602 TDP: allow skip format to avoid ASSERT q_ptr error on closing window
#ifdef __ALLOW_VR_FUNCS__
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
bool V3dR_GLWidget::resumeCollaborationVR=false;
#endif
#include <QWheelEvent>
//PROGRESS_DIALOG("", 0)
V3dr_colormapDialog *V3dR_GLWidget::colormapDlg = 0;
V3dr_surfaceDialog *V3dR_GLWidget::surfaceDlg = 0;

///////////////////////////////////////////////////////////////////////////////////////////////
#if defined(USE_Qt5)
 #define POST_updateGL update  // prevent direct updateGL when rotation which causes re-entering shack problems, by RZC 080925
 #define DO_updateGL	update  // macro to control direct updateGL or post update, by RZC 081007
#else
#define POST_updateGL QOpenGLWidget::update  // prevent direct updateGL when rotation which causes re-entering shack problems, by RZC 080925
// #define DO_updateGL	updateGL  // macro to control direct updateGL or post update, by RZC 081007
//修改了
#define DO_updateGL	QOpenGLWidget::update
#endif
///////////////////////////////////////////////////////////////////////////////////////////////

static bool _isMultipleSampleSupported()
{
    bool b = supported_MultiSample();
#ifdef MACI_TIGER
    b = false;
#endif
#ifdef Q_WS_MAC
    if ((QSysInfo::MacintoshVersion == QSysInfo::MV_10_4)
        && QString((char*)glGetString(GL_RENDERER)).contains("GeForce 7300GT", Qt::CaseInsensitive))
    {
        b = false;
    }
#endif
    return b;
}

void V3dR_GLWidget::closeEvent(QCloseEvent* e)
{
    qDebug("V3dR_GLWidget::closeEvent");  // never run to here for non-frame window, unless called directly, by RZC 080814
    deleteRenderer();

    /////////////////////////////////////////////////////
    //deleteLater(); //Schedules this object for deletion


    //QWidget::closeEvent(e); //accept
    closeEvent(e);
    /////////////////////////////////////////////////////
}

V3dR_GLWidget::~V3dR_GLWidget()
{
    qDebug("V3dR_GLWidget::~V3dR_GLWidget =======================================");
    _in_destructor = true;

    //if (colormapDlg && colormapDlg->DecRef(this)<1) {}//colormapDlg = 0 safely called in destructor of colormapDlg;
    //if (surfaceDlg && surfaceDlg->DecRef(this)<1) {} //surfaceDlg = 0  safely called in destructor of surfaceDlg;

    deleteRenderer(); //090711 RZC: maybe too late, because some version Qt destroyed GL context before here.

    POST_CLOSE(mainwindow);
    QCoreApplication::sendPostedEvents(0, 0); // process all blocked events
}

V3dR_GLWidget::V3dR_GLWidget(iDrawExternalParameter* idep, QWidget* mainWindow, QString title)
    : QOpenGLWidget(mainWindow)
{
    qDebug("V3dR_GLWidget::V3dR_GLWidget ========================================");


    this->_idep = idep;
    this->mainwindow = mainWindow;
    this->data_title = title;
    this->renderer = 0;
    this->show_progress_bar = true;

    ///////////////////////////////////////////////////////////////
    init_members();
    ///////////////////////////////////////////////////////////////

    //setFormat(QGLFormat(0));
    //qDebug()<<"init_members 之后";


    makeCurrent(); //090729: this make sure created GL context

    static int isGLinfoDetected = 0;
    if (isGLinfoDetected==0) // only once when multiple views
    {
#ifdef test_main_cpp
        GLinfoDetect(0); //print to console
#endif
        isGLinfoDetected = 1;
    }


    QSurfaceFormat f; // = QGLFormat::defaultFormat();
                //= format();

    f.setSamples(4); // (1,2,4), For ATI must force samples, by RZC 081001
    f.setDepthBufferSize(24);
    f.setStencilBufferSize(8);


    //dynamic choice GLFormat
    if (!skipFormat)
        setFormat(f);
    QSurfaceFormat::setDefaultFormat(f);

    ///////////////////////////////////////////////////////////////
    makeCurrent(); //090729: this make sure created GL context


    //setFocusPolicy(Qt::WheelFocus); // accept KeyPressEvent when mouse wheel move, by RZC 080831
    setFocusPolicy(Qt::StrongFocus); // accept KeyPressEvent when mouse click, by RZC 081028
    //setFocusProxy(mainWindow);

    //qDebug("V3dR_GLWidget::V3dR_GLWidget ----- end    in v3dr_glwidget.cpp");
    currentPluginState = -1; // May 29, 2012 by Hang
}

//////////////////////////////////////////////////////
void V3dR_GLWidget::deleteRenderer() {makeCurrent(); DELETE_AND_ZERO(renderer);} //090710 RZC: to delete renderer before ~V3dR_GLWidget()
void V3dR_GLWidget::createRenderer() {makeCurrent(); deleteRenderer(); initializeGL();} //090710 RZC: to create renderer at any time

void V3dR_GLWidget::choiceRenderer()
{
    qDebug("V3dR_GLWidget::choiceRenderer");
    _isSoftwareGL = false;
    GLeeInit();

    //==============================================================================
    // OpenGL hardware supporting detection
    // standard method
    const char* glversion = (const char*)glGetString(GL_VERSION);
    //if (strlen(glversion)>3 && glversion[0]=='1' && glversion[1]=='.' && glversion[2]<'3')	_isSoftwareGL = true;
    // GLee method
    if (! GLEE_VERSION_1_3)	_isSoftwareGL = true;

    if (_isSoftwareGL)
    {
        qDebug("   *** You OpenGL version (%s) is under 1.3, switch to Cross-Section type.", glversion);

        PROGRESS_DIALOG( "Preparing 3D View", NULL);
       // PROGRESS_TEXT( ("Preparing 3D View\n\n"
               // "Warning: You OpenGL version (%1) is under 1.3, NO enough graphics hardware support!\n\n"
                //"Now switch to Cross-Section display type, other display type will be very slow.").arg(QString(glversion)).toStdString());
        PROGRESS_PERCENT(1); // 090730: this MUST be here for displaying since Qt 4.5
        //SLEEP(5000);
    }

    //============================================================================
    // dynamic choice different Renderer version according to OpenGL version
    renderer = 0;
    //if (strlen(glversion)>3 && glversion[0]>='2' && glversion[1]=='.' && glversion[2]>='0')
    if (1 && supported_GLSL())
    {
        renderer = new Renderer_gl2(this);
    }
    else  // 081215: this comment for special version without GL 2.0 support
    if (1) //strlen(glversion)>3 && glversion[0]>='1' && glversion[1]=='.' && glversion[2]>='0')
    {
        renderer = new Renderer_gl1(this);
    }
    else
    {
        renderer = new Renderer(this);
    }
   // if (renderer) renderer->selectMode = Renderer::defaultSelectMode;
    //if (renderer) renderer->widget = (void*)this; //081025 //100827 move to constructor parameter
}

// 091007 RZC: extract to function
void V3dR_GLWidget::settingRenderer() // before renderer->setupData & init
{
    qDebug("V3dR_GLWidget::settingRenderer");

    //by PHC, 090618: set up some default rendering options
#ifndef test_main_cpp
    if (_idep && _idep->V3Dmainwindow && renderer)
    {
        renderer->bShowBoundingBox = (_idep->V3Dmainwindow->global_setting.b_autoDispBoundingBox);
        renderer->bShowAxes        = (_idep->V3Dmainwindow->global_setting.b_autoDispXYZAxes);

        renderer->tryTexCompress = (_idep->V3Dmainwindow->global_setting.b_autoVideoCardCompress);
        renderer->tryTex3D       = (_idep->V3Dmainwindow->global_setting.b_autoVideoCard3DTex);
        renderer->tryTexNPT      = (_idep->V3Dmainwindow->global_setting.b_autoVideoCardNPTTex);
        renderer->tryTexStream   = (_idep->V3Dmainwindow->global_setting.autoVideoCardStreamMode);

        if (_idep->b_local){
            renderer->tryTexCompress = 0;
        }
        if (_idep->b_use_512x512x256 == false){
            renderer->tryTexStream = -1; //091016, 100719: use data of full resolution
            qDebug("	Don't use 512x512x256. tryTexStream = -1, use data of full resolution");
            //renderer->tryTex3D = 0; // 3D texture may cause overflow on some machine
        }

        renderer->lineType   = (_idep->V3Dmainwindow->global_setting.b_autoSWCLineMode)?1:0;
    }
#endif
}

void V3dR_GLWidget::preparingRenderer() // renderer->setupData & init, 100719 extracted to a function
{
    qDebug("V3dR_GLWidget::preparingRenderer");

    if (_isSoftwareGL) setRenderMode_Cs3d(true); //090724 set renderer mode before paint

    //=============================================================================
    PROGRESS_DIALOG( "Preparing 3D View", NULL);
    if(this->show_progress_bar)
    {
        PROGRESS_PERCENT(10);
    }
    {
        if(this->show_progress_bar)
        {
            PROGRESS_PERCENT(30);
        }
        if (renderer)
        {
            renderer->setupData(this->_idep);
            if (renderer->hasError())	POST_CLOSE(this);
            renderer->getLimitedDataSize(_data_size); //for update slider size
        }

        if(this->show_progress_bar)
        {
            PROGRESS_PERCENT(70);
        }
        if (renderer)
        {
            renderer->initialize(renderer->class_version()); //090705 RZC
            if (renderer->hasError())	POST_CLOSE(this);
        }
    }
    if(this->show_progress_bar)
    {
        PROGRESS_PERCENT(100);
    }
    //=============================================================================

    // when initialize done, update status of control widgets
    SEND_EVENT(this, QEvent::Type(QEvent_InitControlValue)); // use event instead of signal
    if (_isSoftwareGL)
    {
        emit changeDispType_cs3d(true);  // 081215, set check-box must after changeVolumeCutRange()
    }
    if (supported_TexCompression())
    {
        qDebug("	GL texture compression supported, enable texture compression function");
        emit changeEnableVolCompress(true);
    }
    if (supported_GLSL())
    {
        qDebug("	GL shading language supported, enable volume colormap function");
        emit changeEnableVolColormap(true);
    }

    POST_EVENT(this, QEvent::Type(QEvent_OpenFiles));
    POST_EVENT(this, QEvent::Type(QEvent_Ready)); //081124

    //updateTool(); //081222   //110722, no need, called by V3dR_MainWindow::changeEvent(ActivationChange)
    // 081122, CAUTION: call updateGL from initializeGL will cause infinite loop call
}

void V3dR_GLWidget::initializeGL()
{
    qDebug("V3dR_GLWidget::initializeGL");
    // Qt OpenGL context format detection
    this->initializeOpenGLFunctions();
    makeCurrent();

    QSurfaceFormat format;
    format.setVersion(2, 0);
    format.setStencilBufferSize(8);
    format.setDepthBufferSize(24);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    setFormat(format);


    //choice renderer according to OpenGl version
    choiceRenderer();

    settingRenderer(); //091007, 100719 moved to position before renderer->setupData

    preparingRenderer();
}


void V3dR_GLWidget::resizeGL(int width, int height)
{
    //qDebug(" renderer->setupView( %d, %d )", width, height);
    makeCurrent();
    viewW = width; viewH = height;
    if (renderer) renderer->setupView(width,height);
}

void V3dR_GLWidget::paintGL()
{
    makeCurrent();
    if (renderer && renderer->hasError())  POST_CLOSE(this);

    //QTime qtime; qtime.start();

    //the following translation & rotation operations are carried out in view space
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //the order is important

    //GET current rotation pose by GL matrix stack
    glPushMatrix();
    {
        glLoadIdentity();
        //last absolute rotation pose
        glMultMatrixd(mRot);
        //current relative small rotation, always around center of model
        {
            XYZ R(dxRot, dyRot, dzRot);  					//qDebug("R= %f %f %f", R.x, R.y, R.z);
            dxRot=dyRot=dzRot=0;  // clear relative rotation step

            double angle = norm(R)/(float)ANGLE_TICK;       //qDebug("angle=%f", angle);
            if (angle)
            {
                normalize(R);          						//qDebug("R= %f %f %f", R.x, R.y, R.z);
                glRotated( angle,  R.x, R.y, R.z);
            }
        }
        //save current absolute rotation pose
        glGetDoublev(GL_MODELVIEW_MATRIX, mRot);
        for (int i=0; i<3; i++)
            mRot[i*4 +3]=mRot[3*4 +i]=0; mRot[3*4 +3]=1; // only reserve rotation, remove translation in mRot
    }
    glPopMatrix();

    //SET translation

        //absolute translation
        XYZ T(_xShift, _yShift, _zShift);  				//qDebug("T= %f %f %f", T.x, T.y, T.z);
        //XYZ T(_xShift, _yShift, 0); // force zShift=0
        dxShift=dyShift=dzShift=0;  // clear relative shift step

        double s = 1.4/(float)SHIFT_RANGE;  // 1.4 ~ sqrt(2);
        T = T*s;

//		// alternate rotation center is a fixed point
//		if (alt_rotation)
//		{
//			// shift position as alternate rotation center
//			//vAltC[0]=T.x; vAltC[1]=T.y; vAltC[2]=T.z;
//			//vAltC[0]=-1; vAltC[1]=-1; vAltC[2]=-1;
//
//			double aR[4][4];
//			for (int i=0; i<4; i++)
//				for (int j=0; j<4; j++)
//				{
//					int k = i*4 +j;
//					aR[i][j] = ((i==j)? 1-mRot[k] : -mRot[k]);
//				}
//			XYZ aT;
//			aT.x = mAltC[0]*aR[0][0] + mAltC[1]*aR[1][0] + mAltC[2]*aR[2][0];
//			aT.y = mAltC[0]*aR[0][1] + mAltC[1]*aR[1][1] + mAltC[2]*aR[2][1];
//			aT.z = mAltC[0]*aR[0][2] + mAltC[1]*aR[1][2] + mAltC[2]*aR[2][2];
//
//			T = T + aT; //merge translation
//		}

        glTranslated( T.x, T.y, T.z );


    //SET current absolute rotation pose at alternate rotation center
    if (alt_rotation)	glTranslated( mAltC[0]*flip_X, mAltC[1]*flip_Y, mAltC[2]*flip_Z );
    glMultMatrixd(mRot);
    if (alt_rotation)	glTranslated( -mAltC[0]*flip_X, -mAltC[1]*flip_Y, -mAltC[2]*flip_Z );


    glScaled(flip_X,flip_Y,flip_Z); // make y-axis downward conformed with image coordinate

    //glScaled(1,1, _thickness); // here may be out of view clip-space, not used

    //=========================================================================
    // normalized space of [-1,+1]^3;
    if (renderer)  renderer->paint();
    //=========================================================================

    // changed in setBright dialog, 081101
    //if (sUpdate_bright)
    {
        blendBrighten(_Bright/100.f, (_Contrast+100)/100.f); // fast 8-bit precision
    }

    //qDebug("paint frame cost time = %g sec", qtime.elapsed()*0.001);

    //CHECK_GLError_print(); //090715,090723
}

/////////////////////////////////////////////////////////////
#define __event_handlers__

void V3dR_GLWidget::customEvent(QEvent* e)
{
    qDebug("V3dR_GLWidget::customEvent( ? )");
    switch(e->type())
    {
    case QEvent_OpenFiles:
        qDebug("	( QEvent_OpenFiles )");
        loadObjectListFromFile();
        break;

    case QEvent_DropFiles:  // not use this
        qDebug("	( QEvent_DropFiles )");
        if (renderer)  renderer->loadObjectFromFile( Q_CSTR(dropUrl) );
        break;

    case QEvent_InitControlValue:
        qDebug("	( QEvent_InitControlValue )");
        emit signalInitControlValue(); // V3dR_MainWindow->initControlValue
        break;

    case QEvent_Ready:



        qDebug("	( QEvent_Ready )");
        if (renderer && !renderer->has_image())
        {
            emit signalOnlySurfaceObj(); // V3dR_MainWindow->onlySurfaceObjTab
        }
        qDebug("-------------------------------------------------------------- Ready");
        break;

    }
    POST_updateGL();
}

bool V3dR_GLWidget::event(QEvent* e) //090427 RZC
{
    setAttribute(Qt::WA_Hover); // this control the QEvent::ToolTip and QEvent::HoverMove

    bool event_tip = false;
    QPoint pos;
    switch (e->type())
    {
    case QEvent::ToolTip: {// not work under Mac 64bit, because default not set WA_Hover
//		qDebug("QEvent::ToolTip in V3dR_GLWidget");
        QHelpEvent* he = (QHelpEvent*)e;
        pos = he->pos(); //globalPos();
        event_tip = true;
        break;
        }
    //case QEvent::HoverMove: {
////		qDebug("QEvent::HoverMove in V3dR_GLWidget");
//		QHoverEvent* he = (QHoverEvent*)e;
//		pos = he->pos();
//		event_tip = true;
//		break;
//	}
//	case QEvent::MouseMove: {
////		qDebug("QEvent::MouseMove in V3dR_GLWidget");
//		QMouseEvent* he = (QMouseEvent*)e;
//		pos = he->pos();
//		event_tip = true;
//		break;
//		}
    }

    if (event_tip && renderer)
    {
        //qDebug()<<"cur_node.x="<<pos.x()<<" "<<"cur_node.y="<<pos.y();

        QPoint gpos = mapToGlobal(pos);
        //qDebug()<<"gpos.x="<<gpos.x()<<" "<<"gpos.y="<<gpos.y();

        tipBuf[0] = '\0';
        if (renderer->selectObj(pos.x(), pos.y(), false, tipBuf))
//        #ifdef _ENABLE_MACX_DRAG_DROP_FIX_
//            if (renderer->selectObj(2*pos.x(), 2*pos.y(), false, tipBuf))
//        #endif
            {} //a switch to turn on/off hover tip, because processHit always return 0 for tipBuf!=0
        {
            QToolTip::showText(gpos, QString(tipBuf), this);
        }
    }

    int i = int(e->type())-QEvent::User;
    if (i>0)
    {
        qDebug() << "++++++++++++ customEvent: " << i;
    }

    return QOpenGLWidget_proxy::event(e);
}

void V3dR_GLWidget::enterEvent(QEvent*)
{
    // qDebug("V3dR_GLWidget::enterEvent in v3dr_glwidget.cpp");
    mouse_in_view = 1;
    //setFocus();
}
void V3dR_GLWidget::leaveEvent(QEvent*)
{
    mouse_in_view = 0;
    update();  //DLC ADD
}
void V3dR_GLWidget::focusInEvent(QFocusEvent*)
{
    qDebug("V3dR_GLWidget::focusInEvent");
    //_stillpaint_disable = false;
}
void V3dR_GLWidget::focusOutEvent(QFocusEvent*)
{
    qDebug("V3dR_GLWidget::focusOutEvent");
    //if (_mouse_in_view)
    //	_stillpaint_disable = true;
}

//091015: use still_timer instead
//#define DELAY_STILL_PAINT()  {QTimer::singleShot(1000, this, SLOT(stillPaint())); _still = false;}

void V3dR_GLWidget::paintEvent(QPaintEvent *event)
{
    //QOpenGLWidget_proxy::paintEvent(event);
//	if (! mouse_in_view) //TODO:  use change of viewing matrix
//	{
//		_still = true;
//		QOpenGLWidget_proxy::paintEvent(event);
//		_still = false;
//	}
//	else
    {
        _still = false;
        QOpenGLWidget::paintEvent(event);

        if (needStillPaint()) //pending
        {
            _stillpaint_pending=true;
        }
    }
}

bool V3dR_GLWidget::needStillPaint()
{
    return  (renderer && renderer->tryTexStream == 1);
}
//注释了
void V3dR_GLWidget::stillPaint()
{
    if (_still)  return; // avoid re-enter
    if (! _stillpaint_pending) return;

//	if (QCoreApplication::hasPendingEvents())
//	{
//		_still = false;
//		_stillpaint_pending = true;
//		return;    //continue pending if event loop is busy
//	}
//	else // here system must be idle
    {
        qDebug()<<"debug in v3dr_glwidget.cpp V3dR_GLWidget::stillPaint()";
        still_timer.stop();
        _still = true;
            DO_updateGL(); // update at once, stream texture for full-resolution
        _still = false;
        _stillpaint_pending = false;
        still_timer.start(still_timer_interval); //restart timer
    }
}


#define KM  QApplication::keyboardModifiers()
#define IS_CTRL_MODIFIER		((KM==Qt::ControlModifier) || (KM==CTRL2_MODIFIER))
#define WITH_CTRL_MODIFIER		((KM & Qt::ControlModifier) || (KM & CTRL2_MODIFIER))
#define IS_ALT_MODIFIER			((KM==Qt::AltModifier) || (KM==ALT2_MODIFIER))
#define WITH_ALT_MODIFIER		((KM & Qt::AltModifier) || (KM & ALT2_MODIFIER))
#define IS_SHIFT_MODIFIER		((KM==Qt::ShiftModifier))
#define WITH_SHIFT_MODIFIER		((KM & Qt::ShiftModifier))

#define IS_TRANSLATE_MODIFIER		IS_SHIFT_MODIFIER
#define WITH_TRANSLATE_MODIFIER		WITH_SHIFT_MODIFIER
#define IS_MODEL_MODIFIER			IS_CTRL_MODIFIER
#define WITH_MODEL_MODIFIER			WITH_CTRL_MODIFIER

#define MOUSE_SHIFT(dx, D)  (int(SHIFT_RANGE*2* float(dx)/D))
#define MOUSE_ROT(dr, D)    (int(MOUSE_SENSITIVE*270* float(dr)/D) *ANGLE_TICK)

//#define INTERACT_BUTTON     (event->buttons())// & Qt::LeftButton)

void V3dR_GLWidget::mousePressEvent(QMouseEvent *event)
{
    //091025: use QMouseEvent::button()== not buttonS()&
    qDebug("V3dR_GLWidget::mousePressEvent  button = %d", event->button());

    mouse_held = 1;

    if (event->button()==Qt::LeftButton)
    {
        lastPos = event->pos();
        t_mouseclick_left = clock();
        if(pluginLeftMouseFuncs.find(currentPluginState) != pluginLeftMouseFuncs.end())
        {
            void(*mouse_func)(void*);
            mouse_func = pluginLeftMouseFuncs[currentPluginState];
            (*mouse_func)((void*)this);
        }
    }

    if (!isoperating&&event->button()==Qt::RightButton && renderer) //right-click
    {
        int x = event->x();
        int y = event->y();
        #ifdef _ENABLE_MACX_DRAG_DROP_FIX_
        x = 2 * x;
        y = 2 * y;
        #endif

        if (renderer->hitPoint(x,y))  //pop-up menu (selectObj) or marker definition (hitPen)
        {
           updateTool();
           //POST_updateGL();
        }
        //updateTool();
        POST_updateGL(); //display result after menu   //csz solve the blue screen don't know the reason
    }
}

void V3dR_GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //091025: use 'QMouseEvent::button()==' instead of 'buttons()&'
    //qDebug("V3dR_GLWidget::mouseReleaseEvent  button = %d", event->button());

    mouse_held = 0;

    if (event->button()==Qt::RightButton && renderer) //right-drag end
    {
        (renderer->movePen(event->x(), event->y(), false)); //create curve or nothing
        //qDebug() << "done drawing\n";
        updateTool();

        POST_updateGL(); //update display of curve
    }
}

void V3dR_GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    //091025: use 'QMouseEvent::buttons()&' instead of 'button()=='
    //qDebug()<<"V3dR_GLWidget::mouseMoveEvent  buttons = "<< event->buttons();

    // setFocus(); // accept KeyPressEvent, by RZC 080831

    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();
    lastPos = event->pos();

    if ((event->buttons() & Qt::RightButton) && renderer) //right-drag for 3d curve
        if ( ABS(dx) + ABS(dy) >=2 )
    {
        (renderer->movePen(event->x(), event->y(), true));
//        QPaintEvent *pEvent;
//        paintEvent(pEvent);
        DO_updateGL(); // instantly display pen track, dlc comment for bug


        return;
    }

    if (event->buttons() & Qt::LeftButton)
    {
        //qDebug()<<"MoveEvent LeftButton";
        int xRotStep = MOUSE_ROT(dy, MIN(viewW,viewH));
        int yRotStep = MOUSE_ROT(dx, MIN(viewW,viewH));
        int xShiftStep = MOUSE_SHIFT(dx, viewW);
        int yShiftStep = MOUSE_SHIFT(dy, viewH);

        alt_rotation = (IS_ALT_MODIFIER); // alt+mouse control alternate marker center rotation, 081104

        // mouse control view space, transformed to model space, 081026
        if (IS_TRANSLATE_MODIFIER) // shift+mouse control view space translation, 081104
        {
            setXShift(_xShift + xShiftStep);// move +view -model
            setYShift(_yShift - yShiftStep);// move -view +model
        }
        else if (IS_MODEL_MODIFIER) // ctrl+mouse control model space rotation, 081104
        {
            //modelRotation(yRotStep, xRotStep, 0); //swap y,x
            modelRotation(xRotStep, yRotStep, 0);
        }
        else // default mouse controls view space rotation
        {
            viewRotation(xRotStep, yRotStep, 0);
        }
    }
}

//滚轮事件注释
void V3dR_GLWidget::wheelEvent(QWheelEvent *event)
{

    //20170804 RZC: add zoomin_sign in global_setting.b_scrollupZoomin
    //-1 : scrolldown zoomin
    //+1 : scrollup zoomin
    int zoomin_sign = -1;  //default
#ifndef test_main_cpp
    if (_idep && _idep->V3Dmainwindow)
    {
        zoomin_sign = (_idep->V3Dmainwindow->global_setting.b_scrollupZoomin)? +1 : -1;
    }
#endif

    setFocus(); // accept KeyPressEvent, by RZC 081028

    QPoint numDegress = event->angleDelta()/100;
    //float d = (event->delta())/100;  // ~480
    //qDebug("V3dR_GLWidget::wheelEvent->delta = %g",d);
    #define MOUSE_ZOOM(dz)    (int(dz*4* MOUSE_SENSITIVE));
    #define MOUSE_ZROT(dz)    (int(dz*8* MOUSE_SENSITIVE));

    int zoomStep = MOUSE_ZOOM(numDegress.y());
    int zRotStep = MOUSE_ZROT(numDegress.y());

    if (IS_TRANSLATE_MODIFIER) // shift+mouse control view space translation, 081104
    {
        viewRotation(0, 0, zRotStep);
    }
    else if (IS_MODEL_MODIFIER) // alt+mouse control model space rotation, 081104
    {
        modelRotation(0, 0, zRotStep);
    }
    else // default
    {
        (renderer->hitWheel(event->angleDelta().x(), event->angleDelta().y())); //by PHC, 130424. record the wheel location when zoom-in or out

#ifdef _NEURON_ASSEMBLER_
        // As Alessandro points out in [CViewer::invokedFromVaa3D], Vaa3D somehow does not really disconnect setZoom slot
        // when scrolling (zooming with mouse wheel) in terafly environment. This additional if-else selection aims to avoid
        // Vaa3D's zooming action when the cursor is in the erasing mode of Neuron Assembler.             -- MK, Jan, 2020
        if (!terafly::PMain::isInstantiated()) setZoom((zoomin_sign * zoomStep) + _zoom);
        else
        {
            if (!terafly::CImport::instance()->isEmpty() && terafly::PMain::getInstance()->FragTracerPluginLoaderPtr != nullptr)
            {
                if (terafly::CViewer::getCurrent()->editingMode.compare("erase") && terafly::CViewer::getCurrent()->editingMode.compare("connect"))
                    setZoom((zoomin_sign * zoomStep) + _zoom);
            }
            else setZoom((zoomin_sign * zoomStep) + _zoom);
        }
#else
        setZoom((zoomin_sign * zoomStep) + _zoom);  //20170804 RZC: add zoomin_sign in global_setting.b_scrollupZoomin
#endif

    }

    event->accept();
}

void V3dR_GLWidget::handleKeyPressEvent(QKeyEvent * e)  //090428 RZC: make public function to finally overcome the crash problem of hook MainWindow
{
    switch (e->key())
    {
        case Qt::Key_1:		_holding_num[1] = true; 	break;
        case Qt::Key_2:		_holding_num[2] = true; 	break;
        case Qt::Key_3:		_holding_num[3] = true; 	break;
        case Qt::Key_4:		_holding_num[4] = true; 	break;
        case Qt::Key_5:		_holding_num[5] = true; 	break;
        case Qt::Key_6:		_holding_num[6] = true; 	break;
        case Qt::Key_7:		_holding_num[7] = true; 	break;
        case Qt::Key_8:		_holding_num[8] = true; 	break;
        case Qt::Key_9:		_holding_num[9] = true; 	break;

        case Qt::Key_BracketLeft:
            {
                if (IS_MODEL_MODIFIER) // alt-mouse to control model space rotation, 081104
                    modelRotation(0, 0, +5);
                else
                    viewRotation(0, 0, +5);
            }
            break;
        case Qt::Key_BracketRight:
            {
                if (IS_MODEL_MODIFIER) // alt-mouse to control model space rotation, 081104
                    modelRotation(0, 0, -5);
                else
                    viewRotation(0, 0, -5);
            }
            break;
        case Qt::Key_Left: //100802: arrows key must use WITH_?_MODIFIER
            {
                if (WITH_MODEL_MODIFIER)
                    modelRotation(0, -5, 0);
                else if (WITH_TRANSLATE_MODIFIER)
                    setXShift(_xShift -1);// move -model
                else
                    setXShift(_xShift +1);// move +view
            }
            break;
        case Qt::Key_Right:
            {
                if (WITH_MODEL_MODIFIER)
                    modelRotation(0, +5, 0);
                else if (WITH_TRANSLATE_MODIFIER)
                    setXShift(_xShift +1);// move +model
                else
                    setXShift(_xShift -1);// move -view
            }
            break;
        case Qt::Key_Up:
            {
                if (WITH_MODEL_MODIFIER)
                    modelRotation(-5, 0, 0);
                else if (WITH_TRANSLATE_MODIFIER)
                    setYShift(_yShift +1);// move +model
                else
                    setYShift(_yShift -1);// move -view
            }
            break;
        case Qt::Key_Down:
            {
                if (WITH_MODEL_MODIFIER)
                    modelRotation(+5, 0, 0);
                else if (WITH_TRANSLATE_MODIFIER)
                    setYShift(_yShift -1);// move -model
                else
                    setYShift(_yShift +1);// move +view
            }
            break;
        case Qt::Key_Minus:
            {
                setZoom(_zoom - 10); // zoom out
            }
            break;
        case Qt::Key_Equal:
            {
                setZoom(_zoom + 10); // zoom in
            }
            break;
        case Qt::Key_Underscore:
            {
                emit changeMarkerSize(_markerSize - 1);
            }
            break;
        case Qt::Key_Plus:
            {
                emit changeMarkerSize(_markerSize + 1);
            }
            break;
        case Qt::Key_Backspace:
            {
                resetZoomShift();
            }
            break;
        case Qt::Key_Backslash:
            if (IS_CTRL_MODIFIER)
            {
                emit changeOrthoView(!_orthoView);
            }
            else
            {
                resetRotation();
            }
            break;
        case Qt::Key_Comma:
            {
                emit changeFrontCut(_fCut - 1);
            }
            break;
        case Qt::Key_Period:
            {
                emit changeFrontCut(_fCut + 1);
            }
            break;
        case Qt::Key_Slash:
            {
                emit changeXCSSlider((dataDim1()-1)/2);
                emit changeYCSSlider((dataDim2()-1)/2);
                emit changeZCSSlider((dataDim3()-1)/2);
                emit changeFrontCut(0);
            }
            break;
        case Qt::Key_Less:
            {
                emit changeVolumeTimePoint(_volumeTimePoint - 1);
            }
            break;
        case Qt::Key_Greater:
            {
                emit changeVolumeTimePoint(_volumeTimePoint + 1);
            }
            break;
        case Qt::Key_Question:
            {
                emit changeVolumeTimePoint(0);
            }
            break;

            //// button shortcut //////////////////////////////////////////////////////////////////
        case Qt::Key_B:
            if (IS_CTRL_MODIFIER)
            {
                setBright();
            }else if (IS_ALT_MODIFIER)
            {
                emit changeEditinput("Alt+B: Drawing BBox");
                if(!isoperating)
                    isoperating=true;
                callStrokeCurveDrawingBBoxes();//For serial BBoxes curve drawing shortcut, by ZZ,02212018
            }
            break;
        case Qt::Key_R:
            if (IS_CTRL_MODIFIER)
            {
                reloadData();
            }
            else
            {
                returncheckmode();
            }
            break;
        case Qt::Key_U:
            if (IS_CTRL_MODIFIER)
            {
                updateWithTriView();
            }
            break;
//		case Qt::Key_I:
//		    if (IS_CTRL_MODIFIER)
//		    {
//		    	if (colormapDlg && !colormapDlg->isHidden()) colormapDlg->hide();
//		    	else volumeColormapDialog();
//			}
//	  		break;
//		case Qt::Key_O:
//		    if (IS_CTRL_MODIFIER)
//		    {
//		    	if (surfaceDlg && !surfaceDlg->isHidden()) surfaceDlg->hide();
//		    	else surfaceSelectDialog();
//			}
//	  		break;

            ///// advanced OpenGL shortcut // use & instead of == //////////////////////////////////////////////////////
        case Qt::Key_I:
            if ( WITH_SHIFT_MODIFIER && //advanced
                    WITH_CTRL_MODIFIER
                )
            {
                showGLinfo();
            }
            else if (renderer)
            {

                Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
                if (thisRenderer->selectMode == Renderer::smDeleteMultiNeurons)
                {

                    thisRenderer->setDeleteKey(1);
                    thisRenderer->deleteMultiNeuronsByStroke();
                }
            }
            break;
        case Qt::Key_G:
            if ( WITH_SHIFT_MODIFIER && //advanced
                 WITH_CTRL_MODIFIER
                 )
            {
                toggleShader();
            }else if (IS_ALT_MODIFIER)
            {
                emit changeEditinput("Alt+G: Drawing Global");
                if(!isoperating)
                    isoperating=true;
                callStrokeCurveDrawingGlobal();//For Global optimal curve drawing shortcut, by ZZ,02212018
            }else
            {
                emit changeEditinput("Alt+G: GDTraing");
                if(!isoperating)
                    isoperating=true;
                callGDTracing();
            }
            break;

            ///// volume texture operation //////////////////////////////////////////////////////
        case Qt::Key_F:
            if (IS_CTRL_MODIFIER)
            {
                toggleTexFilter();
            }
            else if (IS_ALT_MODIFIER)
            {
#ifdef _NEURON_ASSEMBLER_
                if (terafly::PMain::isInstantiated())
                {
                    terafly::PMain& pMain = *(terafly::PMain::getInstance());
                    if (!pMain.fragTracePluginInstance)
                    {
#ifdef _NEURON_ASSEMBLER_DEBUG_
                        QPluginLoader* loader = new QPluginLoader("D:\\Vaa3D_2013_Qt486\\v3d_external\\bin\\plugins\\Fragmented_Auto-trace\\Fragmented_Auto-trace.dll");
#else
                        QPluginLoader* loader = new QPluginLoader(".\\plugins\\Fragmented_Auto-trace\\Fragmented_Auto-trace.dll");
#endif
                        pMain.FragTracerQPluginPtr = loader;
                        if (!loader->isLoaded())
                        {
                            v3d_msg("Neuron Assembler module not found. Do nothing.");
                            return;
                        }
                        XFormWidget* curXWidget = v3dr_getXWidget(_idep);
                        pMain.FragTracerPluginLoaderPtr = curXWidget->getMainControlWindow()->pluginLoader;

                        // Cast instantiated CViewer into INeuronAssembler and hold in V3D_PluginCallback2, so that [callback] object can access it.
                        curXWidget->getMainControlWindow()->pluginLoader->castCViewer = tf::PluginInterface::getTeraflyCViewer();
                        curXWidget->getMainControlWindow()->pluginLoader->runPlugin(loader, "settings");

                        // No need to delete QPlugin root component. It handles itself once getting out of the scope;
                        // However, this [return] seems to prevent NA from crashing, although exaxtly why is still unknown.
                        return;
                    }
                    else
                    {
                        //v3d_msg("Neuron Assembler plugin instance already exists.");
                        V3DPluginArgList pluginInputList, pluginOutputList;
                        V3DPluginArgItem dummyInput, inputParam, dummyOutput;
                        vector<char*> pluginInputArgList;
                        vector<char*> pluginOutputArgList;
                        dummyInput.type = "dummy";
                        dummyInput.p = (void*)(&pluginInputArgList);
                        inputParam.type = "dummy";
                        inputParam.p = (void*)(&pluginInputArgList);
                        pluginInputList.push_back(dummyInput);
                        pluginInputList.push_back(inputParam);
                        dummyOutput.type = "dummy";
                        dummyOutput.p = (void*)(&pluginOutputArgList);
                        XFormWidget* curXWidget = v3dr_getXWidget(_idep);
                        curXWidget->getMainControlWindow()->pluginLoader->callPluginFunc("Fragmented_Auto-trace", "bring_to_the_front", pluginInputList, pluginOutputList);
                    }
                }
                else
                {
#ifdef _NEURON_ASSEMBLER_DEBUG_
                    QPluginLoader* loader = new QPluginLoader("D:\\Vaa3D_2013_Qt486\\v3d_external\\bin\\plugins\\Fragmented_Auto-trace\\Fragmented_Auto-trace.dll");
#else
                    QPluginLoader* loader = new QPluginLoader(".\\plugins\\Fragmented_Auto-trace\\Fragmented_Auto-trace.dll");
#endif
                    XFormWidget* curXWidget = v3dr_getXWidget(_idep);
                    if (!loader)
                    {
                        v3d_msg("Neuron Assembler module not found. Do nothing.");
                        return;
                    }
                    if (curXWidget == 0L)
                    {
                        v3d_msg("Neuron Assembler module does not work with non-image 3D viewer. Exit.");
                        return;
                    }
                    curXWidget->getMainControlWindow()->pluginLoader->castCViewer = nullptr;
                    curXWidget->getMainControlWindow()->pluginLoader->runPlugin(loader, "settings");

                    return;
                }
#endif
            }
            break;
#ifdef _NEURON_ASSEMBLER_
        case Qt::Key_F1:
            if (this->getRenderer())
            {
                Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
                My4DImage* curImg = 0;
                if (this) curImg = v3dr_getImage4d(_idep);

                terafly::PMain& pMain = *(terafly::PMain::getInstance());
                if (pMain.fragTracePluginInstance)
                {
                    V3DPluginArgList pluginInputList, pluginOutputList;
                    V3DPluginArgItem dummyInput, inputParam, dummyOutput;
                    vector<char*> pluginInputArgList;
                    vector<char*> pluginOutputArgList;
                    dummyInput.type = "dummy";
                    dummyInput.p = (void*)(&pluginInputArgList);
                    inputParam.type = "show-hide";

                    inputParam.p = (void*)(&pluginInputArgList);
                    pluginInputList.push_back(dummyInput);
                    pluginInputList.push_back(inputParam);
                    dummyOutput.type = "dummy";
                    dummyOutput.p = (void*)(&pluginOutputArgList);
                    XFormWidget* curXWidget = v3dr_getXWidget(_idep);
                    curXWidget->getMainControlWindow()->pluginLoader->callPluginFunc("Fragmented_Auto-trace", "hotKey", pluginInputList, pluginOutputList);
                }
            }
            break;

        case Qt::Key_F2:
            if (this->getRenderer())
            {
                Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
                My4DImage* curImg = 0;
                if (this) curImg = v3dr_getImage4d(_idep);

                terafly::PMain& pMain = *(terafly::PMain::getInstance());
                if (pMain.fragTracePluginInstance)
                {
                    V3DPluginArgList pluginInputList, pluginOutputList;
                    V3DPluginArgItem dummyInput, inputParam, dummyOutput;
                    vector<char*> pluginInputArgList;
                    vector<char*> pluginOutputArgList;
                    dummyInput.type = "dummy";
                    dummyInput.p = (void*)(&pluginInputArgList);
                    inputParam.type = "increase_show";

                    inputParam.p = (void*)(&pluginInputArgList);
                    pluginInputList.push_back(dummyInput);
                    pluginInputList.push_back(inputParam);
                    dummyOutput.type = "dummy";
                    dummyOutput.p = (void*)(&pluginOutputArgList);
                    XFormWidget* curXWidget = v3dr_getXWidget(_idep);
                    curXWidget->getMainControlWindow()->pluginLoader->callPluginFunc("Fragmented_Auto-trace", "hotKey", pluginInputList, pluginOutputList);
                }
            }
            break;

        case Qt::Key_F3:
            if (this->getRenderer())
            {
                Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
                My4DImage* curImg = 0;
                if (this) curImg = v3dr_getImage4d(_idep);

                terafly::PMain& pMain = *(terafly::PMain::getInstance());
                if (pMain.fragTracePluginInstance)
                {
                    V3DPluginArgList pluginInputList, pluginOutputList;
                    V3DPluginArgItem dummyInput, inputParam, dummyOutput;
                    vector<char*> pluginInputArgList;
                    vector<char*> pluginOutputArgList;
                    dummyInput.type = "dummy";
                    dummyInput.p = (void*)(&pluginInputArgList);
                    inputParam.type = "decrease_show";

                    inputParam.p = (void*)(&pluginInputArgList);
                    pluginInputList.push_back(dummyInput);
                    pluginInputList.push_back(inputParam);
                    dummyOutput.type = "dummy";
                    dummyOutput.p = (void*)(&pluginOutputArgList);
                    XFormWidget* curXWidget = v3dr_getXWidget(_idep);
                    curXWidget->getMainControlWindow()->pluginLoader->callPluginFunc("Fragmented_Auto-trace", "hotKey", pluginInputList, pluginOutputList);
                }
            }
            break;
#endif

         case Qt::Key_E:
            if (IS_ALT_MODIFIER)
            {
                emit changeEditinput("Alt+E");
                if(!isoperating)
                    isoperating=true;
                toggleEditMode();
            }
            else if (IS_SHIFT_MODIFIER)
            {
#ifdef _NEURON_ASSEMBLER_
                if (this->getRenderer())
                {
                    Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
                    My4DImage* curImg = 0;
                    if (this) curImg = v3dr_getImage4d(_idep);

                    terafly::PMain& pMain = *(terafly::PMain::getInstance());
                    if (pMain.fragTracePluginInstance)
                    {
                        thisRenderer->NAeditingMode = true;
                        V3DPluginArgList pluginInputList, pluginOutputList;
                        V3DPluginArgItem dummyInput, inputParam, dummyOutput;
                        vector<char*> pluginInputArgList;
                        vector<char*> pluginOutputArgList;
                        dummyInput.type = "dummy";
                        dummyInput.p = (void*)(&pluginInputArgList);

                        if (thisRenderer->editinput == 0) inputParam.type = "shift_e";
                        else
                        {
                            cancelSelect();
                            inputParam.type = "shift_e_already";
                        }
                        thisRenderer->editinput = 97; // This is a special number I chose for Neuron Assembler editing mode. MK, June, 2020.

                        inputParam.p = (void*)(&pluginInputArgList);
                        pluginInputList.push_back(dummyInput);
                        pluginInputList.push_back(inputParam);
                        dummyOutput.type = "dummy";
                        dummyOutput.p = (void*)(&pluginOutputArgList);
                        XFormWidget* curXWidget = v3dr_getXWidget(_idep);
                        curXWidget->getMainControlWindow()->pluginLoader->callPluginFunc("Fragmented_Auto-trace", "hotKey", pluginInputList, pluginOutputList);
                    }
                }
#endif
            }
            else
            {
                callcheckmode();
            }
            break;

        case Qt::Key_T:
            if ( WITH_SHIFT_MODIFIER && //advanced
                    WITH_CTRL_MODIFIER
                )
            {
                toggleTex2D3D();
            }else if (IS_ALT_MODIFIER)
            {
                emit changeEditinput("Alt+T");
                if(!isoperating)
                    isoperating=true;
                callStrokeRetypeMultiNeurons();//For multiple segments retyping shortcut, by ZZ,02212018
            }
            else if (WITH_ALT_MODIFIER && WITH_SHIFT_MODIFIER)
            {
                //callShowSubtree(); // temporarily disabled, MK, July 2018
            }
            else if (renderer)
            {
                Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());

                if (thisRenderer->selectMode == Renderer::smDeleteMultiNeurons)
                {
                    thisRenderer->setDeleteKey(2);
                    thisRenderer->deleteMultiNeuronsByStroke();
                }
            }
            else
                callAutoTracers();
            break;
        case Qt::Key_D:
            if (IS_ALT_MODIFIER)
            {
                emit changeEditinput("Alt+D: Deleting");
                if(!isoperating)
                    isoperating=true;
                callStrokeDeleteMultiNeurons(); //For multiple segments deleting shortcut, by ZZ,02212018
            }
            else
            {
                // delete connected segments that have been highlighted, MK, July, 2018
                Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
                if (thisRenderer->selectMode == Renderer::smShowSubtree)
                {
                    My4DImage* curImg = 0;
                    if (this) curImg = v3dr_getImage4d(_idep);
                    if (thisRenderer->originalSegMap.empty()) return;

                    for (set<size_t>::iterator segIDit = thisRenderer->subtreeSegs.begin(); segIDit != thisRenderer->subtreeSegs.end(); ++segIDit)
                        curImg->tracedNeuron.seg[*segIDit].to_be_deleted = true;

                    thisRenderer->escPressed_subtree();

                    curImg->update_3drenderer_neuron_view(this, thisRenderer);
                    curImg->proj_trace_history_append();
                }
            }
            break;
        case Qt::Key_S:
            if (IS_ALT_MODIFIER)
            {
                emit changeEditinput("Alt+S");
                if(!isoperating)
                    isoperating=true;
                callStrokeSplitMultiNeurons();//For multiple segments spliting shortcut, by ZZ,02212018
            }
#ifdef _NEURON_ASSEMBLER_
            else if (IS_SHIFT_MODIFIER)
            {
                if (this->getRenderer())
                {
                    Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
                    My4DImage* curImg = 0;
                    if (this) curImg = v3dr_getImage4d(_idep);

                    terafly::PMain& pMain = *(terafly::PMain::getInstance());
                    if (pMain.fragTracePluginInstance)
                    {
                        map<int, vector<int> > labeledSegs;
                        for (vector<V_NeuronSWC>::iterator segIt = curImg->tracedNeuron.seg.begin(); segIt != curImg->tracedNeuron.seg.end(); ++segIt)
                            if (segIt->row.begin()->type == 16) segIt->to_be_deleted = true;

                        curImg->update_3drenderer_neuron_view(this, thisRenderer);
                        curImg->proj_trace_history_append();
                    }
                }
            }
            else if (WITH_SHIFT_MODIFIER && WITH_ALT_MODIFIER)
            {
                if (this->getRenderer())
                {
                    Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
                    My4DImage* curImg = 0;
                    if (this) curImg = v3dr_getImage4d(_idep);

                    terafly::PMain& pMain = *(terafly::PMain::getInstance());
                    if (pMain.fragTracePluginInstance)
                    {
                        map<int, vector<int> > labeledSegs;
                        for (vector<V_NeuronSWC>::iterator segIt = curImg->tracedNeuron.seg.begin(); segIt != curImg->tracedNeuron.seg.end(); ++segIt)
                            if (segIt->row.begin()->type == 18) segIt->to_be_deleted = true;

                        curImg->update_3drenderer_neuron_view(this, thisRenderer);
                        curImg->proj_trace_history_append();
                    }
                }
            }
#endif
            else
            {
                if(_idep && _idep->window3D)
                {
                    int sShowSurf = (renderer->sShowSurfObjects==0)?2:0;
                    setShowSurfObjects(sShowSurf);
                    switch(sShowSurf)
                    {
                    case 0: _idep->window3D->checkBox_displaySurf->setCheckState(Qt::Unchecked);break;
                    case 2: _idep->window3D->checkBox_displaySurf->setChecked(Qt::Checked);break;
                    }
                }
            }
            break;
        case Qt::Key_C:
            if ( WITH_SHIFT_MODIFIER && //advanced
                    WITH_CTRL_MODIFIER
                )
            {
                toggleTexCompression();
            }else if (IS_ALT_MODIFIER)
            {
                emit changeEditinput("Alt+C: Connecting");
                if(!isoperating)
                    isoperating=true;
                callStrokeConnectMultiNeurons();//For multiple segments connection shortcut, by ZZ,02212018
            }
            else if (IS_SHIFT_MODIFIER)
            {
#ifdef _NEURON_ASSEMBLER_
                if (this->getRenderer())
                {
                    Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
                    My4DImage* curImg = 0;
                    if (this) curImg = v3dr_getImage4d(_idep);
                    terafly::PMain& pMain = *(terafly::PMain::getInstance());
                    if (pMain.fragTracePluginInstance)
                    {
                        thisRenderer->NAeditingMode = true;
                        V3DPluginArgList pluginInputList, pluginOutputList;
                        V3DPluginArgItem dummyInput, inputParam, dummyOutput;
                        vector<char*> pluginInputArgList;
                        vector<char*> pluginOutputArgList;
                        dummyInput.type = "dummy";
                        dummyInput.p = (void*)(&pluginInputArgList);

                        if (this->getRenderer()->editinput == 0) inputParam.type = "shift_c";
                        else
                        {
                            cancelSelect();
                            inputParam.type = "shift_c_already";
                        }
                        thisRenderer->editinput = 97; // This is a special number I chose for Neuron Assembler editing mode. MK, June, 2020.

                        inputParam.p = (void*)(&pluginInputArgList);
                        pluginInputList.push_back(dummyInput);
                        pluginInputList.push_back(inputParam);
                        dummyOutput.type = "dummy";
                        dummyOutput.p = (void*)(&pluginOutputArgList);
                        XFormWidget* curXWidget = v3dr_getXWidget(_idep);
                        curXWidget->getMainControlWindow()->pluginLoader->callPluginFunc("Fragmented_Auto-trace", "hotKey", pluginInputList, pluginOutputList);
                    }
                }
#endif
            }
            else
            {
                neuronColorMode = (neuronColorMode==0)?5:0; //0 default display mode, 5 confidence level mode by ZZ 06192018
                updateColorMode(neuronColorMode);
            }
            break;
        case Qt::Key_V:
            if ( WITH_SHIFT_MODIFIER && //advanced
                    WITH_CTRL_MODIFIER
                )
            {
                toggleTexStream();
            }
            else if (IS_ALT_MODIFIER)
            {
                emit changeEditinput("Alt+V");
                if(!isoperating)
                    isoperating=true;
                changeVolShadingOption();
            }
            else if (IS_CTRL_MODIFIER)
            {
                emit changeEditinput("Ctl+T");
                if(!isoperating)
                    isoperating=true;
                updateImageData();
            }
            else
            {
                callLoadNewStack();//by ZZ 02012018
            }
            break;
            /////  object operation //////////////////////////////////////////////////////
        case Qt::Key_P:
            if ( WITH_SHIFT_MODIFIER && //advanced
                    WITH_CTRL_MODIFIER
                )
            {
                toggleObjShader();
            }
            else if (IS_CTRL_MODIFIER)
            {
                emit changeEditinput("Ctl+P");
                if(!isoperating)
                    isoperating=true;
                togglePolygonMode();
            }
            else if (IS_ALT_MODIFIER)
            {
                emit changeEditinput("Alt+P");
                if(!isoperating)
                    isoperating=true;
                changeObjShadingOption();
            }
            break;

        case Qt::Key_Y:
            if (IS_ALT_MODIFIER)
            {
                callDefine3DPolyline();//For 3D polyline shortcut, by ZZ,03262018
            }
            break;

        case Qt::Key_Q:
            if(IS_CTRL_MODIFIER){
                qDebug()<<"call special marker---------------------------------------------";
                callCreateSpecialMarkerNearestNode(); //add special marker, by XZ, 20190720
            }
            else
            {
                callCreateMarkerNearestNode();
            }
            break;

            ///// marker operation //////////////////////////////////////////////////////
        case Qt::Key_Escape:
            {
                emit changeEditinput("Esc: cancelled selection");
                if(isoperating)
                    isoperating=false;
                cancelSelect();

#ifdef _NEURON_ASSEMBLER_
                Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
                My4DImage* curImg = 0;
                if (this) curImg = v3dr_getImage4d(_idep);

                if (terafly::PMain::isInstantiated())
                {
                    if (terafly::PMain::getInstance()->fragTracePluginInstance)
                    {
                        thisRenderer->NAeditingMode = false;
                        V3DPluginArgList pluginInputList, pluginOutputList;
                        V3DPluginArgItem dummyInput, inputParam, dummyOutput;
                        vector<char*> pluginInputArgList;
                        vector<char*> pluginOutputArgList;
                        dummyInput.type = "dummy";
                        dummyInput.p = (void*)(&pluginInputArgList);
                        inputParam.type = "escape";
                        inputParam.p = (void*)(&pluginInputArgList);
                        pluginInputList.push_back(dummyInput);
                        pluginInputList.push_back(inputParam);
                        dummyOutput.type = "dummy";
                        dummyOutput.p = (void*)(&pluginOutputArgList);
                        XFormWidget* curXWidget = v3dr_getXWidget(_idep);
                        curXWidget->getMainControlWindow()->pluginLoader->callPluginFunc("Fragmented_Auto-trace", "hotKey", pluginInputList, pluginOutputList);
                    }
                }
#endif
            }
            break;

            ///// cell operation ///////////////////////////////////////////////////////
        case Qt::Key_N:
            if (IS_CTRL_MODIFIER)
            {
                emit changeEditinput("Ctl+N: toggle cell name");
                if(!isoperating)
                    isoperating=true;
                toggleCellName();
            }
            else if (IS_SHIFT_MODIFIER) // toggle marker name display. by Lei Qu, 110425
            {
                toggleMarkerName();
            }
            else if (IS_ALT_MODIFIER)
            {
                emit changeEditinput("Alt+N: show connected segs");
                if(!isoperating)
                    isoperating=true;
                callShowConnectedSegs();
            }
            else if (WITH_ALT_MODIFIER && WITH_CTRL_MODIFIER)
            {
                cout << "wp_debug" << __LINE__ << __FUNCTION__ << endl;
                callShowBreakPoints();
            }
            break;

            ///// neuron operation //////////////////////////////////////////////////////
        case Qt::Key_L:
            if (IS_CTRL_MODIFIER)
            {
                toggleLineType();
            }
            else if (IS_ALT_MODIFIER)
            {
                changeLineOption();
            }
            else if(IS_SHIFT_MODIFIER)
            {
                toggleNStrokeCurveDrawing(); // For n-right-strokes curve shortcut ZJL 110920
            }
            else
            {
                //callCurveLineDetector(0); //the 0 option is for a fixed 32 window
                callCurveLineDetector(1);//by PHC 20170531. // the 1 option is for calling the curveline detector using its infinite loop mode

                Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
                if (thisRenderer->selectMode == Renderer::smShowSubtree)
                {
                    My4DImage* curImg = 0;
                    if (this) curImg = v3dr_getImage4d(_idep);

                thisRenderer->loopDetection();
                }
            }
            break;

          case Qt::Key_W:
            if (IS_ALT_MODIFIER)
            {
                setDragWinSize(+2);
            }
            else if(IS_SHIFT_MODIFIER)
            {
                setDragWinSize(-2);
            }
            else
            {
                Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
                if (thisRenderer->selectMode == Renderer::smShowSubtree)
                {
                    My4DImage* curImg = 0;
                    if (this) curImg = v3dr_getImage4d(_idep);

                    for (set<size_t>::iterator segIDit = thisRenderer->subtreeSegs.begin(); segIDit != thisRenderer->subtreeSegs.end(); ++segIDit)
                    {
                        for (vector<V_NeuronSWC_unit>::iterator nodeIt = thisRenderer->originalSegMap[*segIDit].begin(); nodeIt != thisRenderer->originalSegMap[*segIDit].end(); ++nodeIt)
                            nodeIt->type = 0;

                        curImg->tracedNeuron.seg[*segIDit].row = thisRenderer->originalSegMap[*segIDit];
                    }

                    curImg->update_3drenderer_neuron_view(this, thisRenderer);
                    curImg->proj_trace_history_append();
                }
            }
            break;

#ifndef test_main_cpp
        case Qt::Key_Z:

            // @ADDED by Alessandro on 2015-05-23. Also allow redo with CTRL+SHIFT+Z
            if (KM.testFlag(Qt::ShiftModifier) && (KM.testFlag(Qt::ControlModifier) || KM.testFlag(Qt::MetaModifier)))
            {
                if (!V3dR_GLWidget::disableUndoRedo && v3dr_getImage4d(_idep) && renderer)
                {
                    v3dr_getImage4d(_idep)->proj_trace_history_redo();
                    v3dr_getImage4d(_idep)->update_3drenderer_neuron_view(this, (Renderer_gl1*)renderer);//090924
                }
            }
            //undo the last tracing step if possible. by PHC, 090120
            else if (IS_CTRL_MODIFIER)
            {
                if (!V3dR_GLWidget::disableUndoRedo && v3dr_getImage4d(_idep) && renderer)
                {
                    v3dr_getImage4d(_idep)->proj_trace_history_undo();
                    v3dr_getImage4d(_idep)->update_3drenderer_neuron_view(this, (Renderer_gl1*)renderer);//090924
                }
            }
            break;

        case Qt::Key_X: //090924 RZC: redo
            if (IS_CTRL_MODIFIER)
            {
                if (!V3dR_GLWidget::disableUndoRedo && v3dr_getImage4d(_idep) && renderer)
                {
                    v3dr_getImage4d(_idep)->proj_trace_history_redo();
                    v3dr_getImage4d(_idep)->update_3drenderer_neuron_view(this, (Renderer_gl1*)renderer);//090924
                }
            }
            break;

#endif
            //////////////////////////////////////////////////////////////////////////////
        default:
            QOpenGLWidget_proxy::keyPressEvent(e);
            break;
    }
    update(); //091030: must be here for correct MarkerPos's view matrix
}

void V3dR_GLWidget::handleKeyReleaseEvent(QKeyEvent * e)  //090428 RZC: make public function to finally overcome the crash problem of hook MainWindow
{
    switch (e->key())
    {
        case Qt::Key_1:		_holding_num[1] = false; 	break;
        case Qt::Key_2:		_holding_num[2] = false; 	break;
        case Qt::Key_3:		_holding_num[3] = false; 	break;
        case Qt::Key_4:		_holding_num[4] = false; 	break;
        case Qt::Key_5:		_holding_num[5] = false; 	break;
        case Qt::Key_6:		_holding_num[6] = false; 	break;
        case Qt::Key_7:		_holding_num[7] = false; 	break;
        case Qt::Key_8:		_holding_num[8] = false; 	break;
        case Qt::Key_9:		_holding_num[9] = false; 	break;

        default:
            QOpenGLWidget_proxy::keyReleaseEvent(e);
            break;
    }
    update(); //091030: must be here for correct MarkerPos's view matrix
    return;
}

//应该是做切片的功能
QString V3dR_GLWidget::Cut_altTip(int dim_i, int v, int minv, int maxv, int offset)
{
    if (!getRenderer() || getRenderer()->class_version()<2) return "";
    Renderer_gl1* r = (Renderer_gl1*)getRenderer();
    BoundingBox DB = r->getDataBox();

    float minw, maxw;
    switch(dim_i)
    {
        case 1:	 minw = DB.x0;  maxw = DB.x1;  break;
        case 2:	 minw = DB.y0;  maxw = DB.y1;  break;
        case 3:	 minw = DB.z0;  maxw = DB.z1;  break;
        default: return "";
    }
    if (maxv - minv == 0 || maxv - minv == maxw - minw) return "";

    float w = minw + (v - minv)*(maxw - minw)/(maxv - minv);
    QString tip = QString(" : %1(%2~%3)").arg(w+offset).arg(minw+offset).arg(maxw+offset);
    //qDebug()<<"		Cut_altTip "<<tip;
    return tip;
}



///////////////////////////////////////////////////////////////////////////////////////////
#define VIEW3DCONTROL
#define __begin_view3dcontrol_interface__
///////////////////////////////////////////////////////////////////////////////////////////

int V3dR_GLWidget::setVolumeTimePoint(int t)
{
    //qDebug("V3dR_GLWidget::setVolumeTimePoint = %d", t);
    if (t<0) t = 0;
    if (t>=dataDim5()) t = dataDim5()-1;

    if (_volumeTimePoint != t) {
        _volumeTimePoint = t;
        if (renderer) _volumeTimePoint = renderer->setVolumeTimePoint(t);

        POST_updateGL(); //090805: make slider dragging more smooth for Win32
    }
    emit changeVolumeTimePoint(t);
    return _volumeTimePoint;
}

void V3dR_GLWidget::incVolumeTimePoint(float step)
{
    float t = _volumeTimePoint + volumeTimPoint_fraction + step;
    if (t<0) t += dataDim5();
    if (t>=dataDim5()) t -= dataDim5();

    volumeTimPoint_fraction = t-floor(t);
    setVolumeTimePoint(int(t));
}

void V3dR_GLWidget::setRenderMode_Mip(bool b, bool useMin)
{
    //qDebug("V3dR_GLWidget::setRenderMode_Mip = %i",b);
    if (b) {
        if (!useMin) {	//max IP
            _renderMode = int(Renderer::rmMaxIntensityProjection);
            if (renderer) renderer->setRenderMode(Renderer::rmMaxIntensityProjection);
        } else {
            //mIP
            _renderMode = int(Renderer::rmMinIntensityProjection);
            if (renderer) renderer->setRenderMode(Renderer::rmMinIntensityProjection);
        }
        // restore renderer->Cut0
        if (renderer) renderer->setXCut0(_xCut0);
        if (renderer) renderer->setYCut0(_yCut0);
        if (renderer) renderer->setZCut0(_zCut0);
        POST_updateGL();
    }

    if (!useMin)
        emit changeDispType_maxip(b);
    else
        emit changeDispType_minip(b);

    emit changeTransparentSliderLabel("Threshold");
    emit changeEnableCut0Slider(b);
    emit changeEnableCut1Slider( !b);
    if (b) emit changeCurrentTabCutPlane(0);
    emit changeEnableTabCutPlane(0, b);
    emit changeEnableTabCutPlane(1, !b);
}

void V3dR_GLWidget::setRenderMode_Alpha(bool b)
{
    //qDebug("V3dR_GLWidget::setRenderMode_Alpha = %i",b);
    if (b) {
        _renderMode = int(Renderer::rmAlphaBlendingProjection);
        if (renderer) renderer->setRenderMode(Renderer::rmAlphaBlendingProjection);
        // restore renderer->Cut0
        if (renderer) renderer->setXCut0(_xCut0);
        if (renderer) renderer->setYCut0(_yCut0);
        if (renderer) renderer->setZCut0(_zCut0);
        POST_updateGL();
    }
    emit changeDispType_alpha(b);

    emit changeTransparentSliderLabel("Threshold");
    emit changeEnableCut0Slider(b);
    emit changeEnableCut1Slider( !b);
    if (b) emit changeCurrentTabCutPlane(0);
    emit changeEnableTabCutPlane(0, b);
    emit changeEnableTabCutPlane(1, !b);
}

void V3dR_GLWidget::setRenderMode_Cs3d(bool b)
{
    //qDebug("V3dR_GLWidget::setRenderMode_Cs3d = %i",b);
    if (b) {
        _renderMode = int(Renderer::rmCrossSection);
        if (renderer) renderer->setRenderMode(Renderer::rmCrossSection);
        // using widget->Cut1 to control renderer->Cut0
//		if (renderer) renderer->setXCut0(_xCut1);
//		if (renderer) renderer->setYCut0(_yCut1);
//		if (renderer) renderer->setZCut0(_zCut1);
        if (renderer) renderer->setXCut0(_xCS);
        if (renderer) renderer->setYCut0(_yCS);
        if (renderer) renderer->setZCut0(_zCS);
        POST_updateGL();
    }
    emit changeDispType_cs3d(b);

    emit changeTransparentSliderLabel("Transparency");
    emit changeEnableCut0Slider( !b);
    emit changeEnableCut1Slider(b);
    if (b) emit changeCurrentTabCutPlane(1);
    emit changeEnableTabCutPlane(0, !b);
    emit changeEnableTabCutPlane(1, b);
}

void V3dR_GLWidget::setCSTransparent(int t)
{
    if (_CStransparency != t) {
        _CStransparency = t;
        if (renderer) renderer->CSbeta = CLAMP(0,1, float(t)/TRANSPARENT_RANGE);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setContrast(int t)
{
    Renderer_gl2* curr_renderer = (Renderer_gl2*)(getRenderer());

    if (curr_renderer)
    {
        float exp_val = pow(10.0f, t/100.0f);
        for(int j=0; j<255; j++)
        {
            // This is the value being manipulated
            int val= (int)(pow(j/255.0f, exp_val) * 255.0f);

            (curr_renderer->colormap[0][j]).r = (unsigned char)255;
            (curr_renderer->colormap[0][j]).g = (unsigned char)0;
            (curr_renderer->colormap[0][j]).b = (unsigned char)0;
            (curr_renderer->colormap[0][j]).a = (unsigned char)val;

            (curr_renderer->colormap[1][j]).r = (unsigned char)0;
            (curr_renderer->colormap[1][j]).g = (unsigned char)255;
            (curr_renderer->colormap[1][j]).b = (unsigned char)0;
            (curr_renderer->colormap[1][j]).a = (unsigned char)val;

            (curr_renderer->colormap[2][j]).r = (unsigned char)0;
            (curr_renderer->colormap[2][j]).g = (unsigned char)0;
            (curr_renderer->colormap[2][j]).b = (unsigned char)255;
            (curr_renderer->colormap[2][j]).a = (unsigned char)val;

            (curr_renderer->colormap[3][j]).r = (unsigned char)205;
            (curr_renderer->colormap[3][j]).g = (unsigned char)205;
            (curr_renderer->colormap[3][j]).b = (unsigned char)205;
            (curr_renderer->colormap[3][j]).a = (unsigned char)205;
        }
        for (int ch=0; ch<3; ch++)
        {
            for (int j=0; j<4; j++) // RGBA
            {
                curr_renderer->colormap_curve[ch][j].clear();
                int y;
                y = curr_renderer->colormap[ch][0].c[j];	set_colormap_curve(curr_renderer->colormap_curve[ch][j],  0.0,  y);
                y = curr_renderer->colormap[ch][255].c[j];	set_colormap_curve(curr_renderer->colormap_curve[ch][j],  1.0,  y);
            }
        }
        POST_updateGL();
    }
}

void V3dR_GLWidget::setContrast1(int t){
        Renderer_gl2* curr_renderer = (Renderer_gl2*)(getRenderer());

        if (curr_renderer)
        {
            float exp_val = pow(10.0f, t/100.0f);
            for(int j=0; j<255; j++)
            {
                // This is the value being manipulated
                int val= (int)(pow(j/255.0f, exp_val) * 255.0f);

                (curr_renderer->colormap[0][j]).r = (unsigned char)255;
                (curr_renderer->colormap[0][j]).g = (unsigned char)0;
                (curr_renderer->colormap[0][j]).b = (unsigned char)0;
                (curr_renderer->colormap[0][j]).a = (unsigned char)val;

                (curr_renderer->colormap[1][j]).r = (unsigned char)0;
                (curr_renderer->colormap[1][j]).g = (unsigned char)255;
                (curr_renderer->colormap[1][j]).b = (unsigned char)0;
                (curr_renderer->colormap[1][j]).a = (unsigned char)val;

                (curr_renderer->colormap[2][j]).r = (unsigned char)0;
                (curr_renderer->colormap[2][j]).g = (unsigned char)0;
                (curr_renderer->colormap[2][j]).b = (unsigned char)255;
                (curr_renderer->colormap[2][j]).a = (unsigned char)val;

                (curr_renderer->colormap[3][j]).r = (unsigned char)205;
                (curr_renderer->colormap[3][j]).g = (unsigned char)205;
                (curr_renderer->colormap[3][j]).b = (unsigned char)205;
                (curr_renderer->colormap[3][j]).a = (unsigned char)205;
            }
            for (int ch=0; ch<3; ch++)
            {
                for (int j=0; j<4; j++) // RGBA
                {
                    curr_renderer->colormap_curve[ch][j].clear();
                    int y;
                    y = curr_renderer->colormap[ch][0].c[j];	set_colormap_curve(curr_renderer->colormap_curve[ch][j],  0.0,  y);
                    y = curr_renderer->colormap[ch][255].c[j];	set_colormap_curve(curr_renderer->colormap_curve[ch][j],  1.0,  y);
                }
            }
            POST_updateGL();
        }
    }


void V3dR_GLWidget::setThickness(double t) //added by PHC, 090215
{
    if (_thickness != t) {
        _thickness = CLAMP(1, ZTHICK_RANGE, t);
        if (renderer) renderer->setThickness(_thickness);
        POST_updateGL();
    }

    Renderer_gl1* rendererGL1Ptr = static_cast<Renderer_gl1*>(this->getRenderer());
    rendererGL1Ptr->zThick = t;
}

void V3dR_GLWidget::setCurChannel(int t) //100802
{
    if (_curChannel != t) {
        _curChannel = t;
        if (renderer) renderer->curChannel = (_curChannel-1); //0-based
        POST_updateGL();
    }
}

void V3dR_GLWidget::setChannelR(bool s)
{
    if (renderer)
    {
        renderer->color_proxy.r = ((s)? 1 : 0);
        POST_updateGL();
    }
}
void V3dR_GLWidget::setChannelG(bool s)
{
    if (renderer)
    {
        renderer->color_proxy.g = ((s)? 1 : 0);
        POST_updateGL();
    }
}
void V3dR_GLWidget::setChannelB(bool s)
{
    if (renderer)
    {
        renderer->color_proxy.b = ((s)? 1 : 0);
        POST_updateGL();
    }
}
void V3dR_GLWidget::setVolCompress(bool s)
{
    if (renderer)
    {
        _volCompress = (renderer->tryTexCompress >0);
        if (_volCompress != s)
        {
            toggleTexCompression();
            POST_updateGL();
        }
    }
}

void V3dR_GLWidget::enableFrontSlice(bool s)
{
    if (renderer)
    {
        renderer->bFSlice = (s);
        POST_updateGL();
    }
}
void V3dR_GLWidget::enableXSlice(bool s)
{
    if (renderer)
    {
        renderer->bXSlice = (s);
        POST_updateGL();
    }
}
void V3dR_GLWidget::enableYSlice(bool s)
{
    if (renderer)
    {
        renderer->bYSlice = (s);
        POST_updateGL();
    }
}
void V3dR_GLWidget::enableZSlice(bool s)
{
    if (renderer)
    {
        renderer->bZSlice = (s);
        POST_updateGL();
    }
}

///////////////////////////////////////////////////////////////////////////////
#define __shared_tool_dialogs__ //dummy, just for locating

void V3dR_GLWidget::showTool()
{
    if (surfaceDlg)  surfaceDlg->show();
    if (colormapDlg)  colormapDlg->show();
}
void V3dR_GLWidget::hideTool()
{
    if (surfaceDlg)  surfaceDlg->hide();
    if (colormapDlg)  colormapDlg->hide();
}
void V3dR_GLWidget::updateTool()
{
    qDebug("V3dR_GLWidget::updateTool (surfaceDlg=%p) (colormapDlg=%p) v3dr_glwidget.cpp", surfaceDlg, colormapDlg);

    if (surfaceDlg && !(surfaceDlg->isHidden()) ) //081215
    {
        //int i = surfaceDlg->getCurTab();
        surfaceDlg->linkTo(this);
        surfaceDlg->setCurTab(-1);  //-1 = last tab
    }
    if (colormapDlg && !(colormapDlg->isHidden()) ) //081219
    {
        colormapDlg->linkTo(this);
    }
}

void V3dR_GLWidget::volumeColormapDialog()
{
    // Caution: there renderer must be Renderer_gl2* at least
    if (! renderer || renderer->class_version()<2) return;

    if (! colormapDlg)
        colormapDlg = new V3dr_colormapDialog(this); //mainwindow);
    else
        colormapDlg->linkTo(this);   //except isHidden, linkTo in updateTool triggered by ActivationChange event

    if (colormapDlg)
    {

        colormapDlg->show();
        this->raise();        //110713
        colormapDlg->raise(); //090710
    }
}

void V3dR_GLWidget::surfaceSelectDialog(int curTab)
{
    // Caution: there renderer must be Renderer_gl1* at least
    if (! renderer || renderer->class_version()<1) return;

    PROGRESS_DIALOG("collecting data for table", 0);
    PROGRESS_PERCENT(20);

    if (! surfaceDlg)
        surfaceDlg = new V3dr_surfaceDialog(this); //, mainwindow);
    else
        surfaceDlg->linkTo(this);  //except isHidden, linkTo in updateTool triggered by ActivationChange event


    if (surfaceDlg)
    {
        surfaceDlg->show();
        this->raise();       //110713
        surfaceDlg->raise(); //090710

        surfaceSelectTab(-1);  //-1 = last tab
    }
}

void V3dR_GLWidget::surfaceSelectTab(int curTab)
{
    if (surfaceDlg)
    {
        surfaceDlg->setCurTab(curTab);
    }
}

void V3dR_GLWidget::surfaceDialogHide()
{
    if (surfaceDlg)
    {
        surfaceDlg->hide();
    }
}

void V3dR_GLWidget::annotationDialog(int dc, int st, int i)
{
    qDebug("V3dR_GLWidget::annotationDialog (%d, %d, %d) for annotation", dc, st, i);

    if (renderer) renderer->editSurfaceObjAnnotation(dc, st, i);
}

#define __VR_FUNCS_a__
#ifdef __ALLOW_VR_FUNCS__
void V3dR_GLWidget::doimage3DVRView(bool bCanCoMode)
{
    qDebug()<<"PMain::doCollaborationVRView()";
    try
    {
            //this->setWindowState(Qt::WindowMinimized);
            //this->hide();

            int maxresindex = 1;
            My4DImage *img4d = this->getiDrawExternalParameter()->image4d;

            this->collaborationMaxResolution = XYZ(img4d->getXDim(),img4d->getYDim(),img4d->getZDim());
            this->Resindex = -1;
            cout<<"CViewer::getCurrent()->volResIndex;   "<<this->Resindex<<endl;
            this->doimageVRView(true);
            //cur_win->storeAnnotations();
            //this->show();

    }
    catch(...)
    {
        qDebug()<<"???do3DVRView()";
    }
}
void V3dR_GLWidget::process3Dwindow(bool show)
{
    static bool clearwindowlist = true;// if process3Dwindow is not used in pair ,set it true
    XFormWidget* curXWidget = v3dr_getXWidget(_idep);
    if(!curXWidget)
    {cout<<"curXWidget is nor exist";clearwindowlist =false;return; }
    if(!curXWidget->getMainControlWindow())
        cout<<"main window is nor exist";
    V3d_PluginLoader mypluginloader(curXWidget->getMainControlWindow());
    QList<V3dR_MainWindow*> windowList = mypluginloader.getListAll3DViewers();
    static QVector<int> visibility;

    if(show)
    {
        cout<<"process3Dwindow show"<<endl;
        cout<<"windowList"<<windowList.size();
        for(int i=0;i<visibility.size();++i)
        {
            windowList[visibility[i]]->show();
        }

        clearwindowlist = true;
    }
    else
    {
        cout<<"process3Dwindow hide"<<endl;
        if(clearwindowlist)
        visibility.clear();
        for(int i=0;i<windowList.size();++i)
        {
            if(windowList[i]->isVisible())
                {
                    visibility.push_back(i);
                    windowList[i]->hide();
                }
        }
    }

}
//VR注释
void V3dR_GLWidget::doimageVRView(bool bCanCoMode)//0518
{
    Renderer_gl1* tempptr = (Renderer_gl1*)renderer;
    QList <NeuronTree> * listNeuronTrees = tempptr->getHandleNeuronTrees();
    cout<<"vr listNeuronTrees.size()"<<listNeuronTrees->size();
    My4DImage *img4d = this->getiDrawExternalParameter()->image4d;
    //this->getMainWindow()->hide();
    //process3Dwindow(false);
    //this->getiDrawExternalParameter()->V3Dmainwindow->showMinimized();
    QMessageBox::StandardButton reply;
    if(bCanCoMode&&(!resumeCollaborationVR))// get into collaboration  first time
        reply = QMessageBox::question(this, "Vaa3D VR", "Collaborative mode?", QMessageBox::Yes|QMessageBox::No);
    else if(resumeCollaborationVR)	//if resume collaborationVR ,reply = yes and no question message box
        reply = QMessageBox::Yes;
    else
        reply = QMessageBox::No;
    if (reply == QMessageBox::Yes)
    {
        if(VRClientON==false)
        {
            VRClientON = true;
            if(myvrwin)
                delete myvrwin;
            myvrwin = 0;
            myvrwin = new VR_MainWindow();
            myvrwin->setWindowTitle("VR MainWindow");
            //bool linkerror = myvrwin->SendLoginRequest(resumeCollaborationVR);

            if(!TeraflyCommunicator)  // there is error with linking ,linkerror = 0
            {qDebug()<<"can't connect to server .unknown wrong ";this->getMainWindow()->show(); VRClientON = false;return;}
            connect(myvrwin,SIGNAL(VRSocketDisconnect()),this,SLOT(OnVRSocketDisConnected()));
            QString VRinfo = this->getDataTitle();
            qDebug()<<"VR get data_title = "<<VRinfo;
            resumeCollaborationVR = false;//reset resumeCollaborationVR
            myvrwin->ResIndex = Resindex;
            int _call_that_func = myvrwin->StartVRScene(listNeuronTrees,img4d,(MainWindow *)(this->getMainWindow()),1,VRinfo,CollaborationCreatorRes,&teraflyZoomInPOS,&CollaborationCreatorPos,collaborationMaxResolution);

            qDebug()<<"result is "<<_call_that_func;
            qDebug()<<"xxxxxxxxxxxxx ==%1 y ==%2 z ==%3"<<teraflyZoomInPOS.x<<teraflyZoomInPOS.y<<teraflyZoomInPOS.z;
#ifdef __ALLOW_VR_FUNCS_
            UpdateVRcollaInfo();
#endif
            updateWithTriView();

            if (_call_that_func > 0)
            {
                resumeCollaborationVR = true;
                emit(signalCallTerafly(_call_that_func));
            }
            else if(_call_that_func == -1)
            {
                call_neuron_assembler_live_plugin((MainWindow *)(this->getMainWindow()));
            }
        }
        else
        {
            v3d_msg("The ** client is running.Failed to start VR client.");
            //this->getMainWindow()->show();
        }
    }
    else
    {
        // bool _Call_ZZ_Plugin = startStandaloneVRScene(listNeuronTrees, img4d, (MainWindow *)(this->getMainWindow())); // both nt and img4d can be empty.

        int _call_that_func = startStandaloneVRScene(listNeuronTrees, img4d, (MainWindow *)(this->getMainWindow()),this->getvrwidget(),&teraflyZoomInPOS); // both nt and img4d can be empty.

        qDebug()<<"result is "<<_call_that_func;
        qDebug()<<"xxxxxxxxxxxxx ==%1 y ==%2 z ==%3"<<teraflyZoomInPOS.x<<teraflyZoomInPOS.y<<teraflyZoomInPOS.z;
        updateWithTriView();
        if (_call_that_func > 0)
        {
            emit(signalCallTerafly(_call_that_func));
        }
        else if(_call_that_func == -1)
        {
            call_neuron_assembler_live_plugin((MainWindow *)(this->getMainWindow()));
        }

//        this->getMainWindow()->show();
//         if(_Call_ZZ_Plugin)
//         {
//            // call_neuron_assembler_live_plugin((MainWindow *)(this->getMainWindow()));
//            emit(signalCallTerafly());
//         }
    }
        //this->getiDrawExternalParameter()->V3Dmainwindow->showNormal();
        //process3Dwindow(true);

}
void V3dR_GLWidget::doclientView(bool check_flag)
{

    /*if(check_flag)
    {
    qDebug()<<"run true.";
    if(VRClientON==false)
    {
    v3d_msg("Now start Collaboration.");
    VRClientON = true;
    Renderer_gl1* tempptr = (Renderer_gl1*)renderer;
    QList <NeuronTree> * listNeuronTrees = tempptr->getHandleNeuronTrees();
    if(myclient)
    delete myclient;
    myclient = 0;
    myclient =new V3dR_Communicator(&this->VRClientON, listNeuronTrees);
    bool linkerror = myclient->SendLoginRequest();
    XFormWidget *curXWidget = v3dr_getXWidget(_idep);
    if (!curXWidget)
    {
    cout << "curXWidget is nor exist";
    return;
    }
    if (!curXWidget->getMainControlWindow())
    cout << "main window is nor exist";
    V3d_PluginLoader mypluginloader(curXWidget->getMainControlWindow());
    QList<V3dR_MainWindow *> windowList = mypluginloader.getListAll3DViewers();

    if(!linkerror)
    {
    for (int i = 0; i < windowList.size(); ++i)
    {
    if (windowList[i]->rotCView->isChecked())
    {
    windowList[i]->rotCView->setChecked(false);
    }
    }
    v3d_msg("Error!Cannot link to server!");
    myclient = 0;
    VRClientON = false;
    }
    else
    {v3d_msg("Successed linking to server! ");
    myclient->CollaborationMainloop();}
    }
    else
    {

    v3d_msg("The VR client is running.Failed to start ** client.");
    }
    }
    else
    {
    qDebug()<<"run false.";
    if(myclient)
    {
    qDebug()<<"run disc.";
    delete myclient;
    myclient = 0;
    }
    VRClientON=false;
    }*/
}

void V3dR_GLWidget::OnVRSocketDisConnected()
{
    qDebug()<<"V3dR_GLWidget::OnVRSocketDisConnected()";
    VRClientON=false;
}


#endif

///////////////////////////////////////////////////////////////////////////////
#define __interaction_controls__

#define NORMALIZE_angle( angle ) \
{ \
    while (angle < 0)                  angle += 360 * ANGLE_TICK; \
    while (angle > 360 * ANGLE_TICK)   angle -= 360 * ANGLE_TICK; \
}
#define NORMALIZE_angleStep( angle ) \
{ \
    while (angle < -180 * ANGLE_TICK)   angle += 360 * ANGLE_TICK; \
    while (angle >  180 * ANGLE_TICK)   angle -= 360 * ANGLE_TICK; \
}

void V3dR_GLWidget::setXRotation(int angle)
{
    NORMALIZE_angle( angle );
    if (angle != _xRot) {
        _absRot = false;
        dxRot = angle-_xRot;        //qDebug("dxRot=%d",dxRot);
        NORMALIZE_angleStep(dxRot);  //qDebug("dxRot=%d",dxRot);
        _xRot = angle;

        emit xRotationChanged(angle);
        POST_updateGL(); // post update to prevent shaking, by RZC 080910
    }
}

//实现鼠标移动的函数
void V3dR_GLWidget::setXRotation(float angle)
{
    NORMALIZE_angle( angle );
    if (angle != _xRot) {
        _absRot = false;
        dxRot = angle-_xRot;        //qDebug("dxRot=%d",dxRot);
        NORMALIZE_angleStep(dxRot);  //qDebug("dxRot=%d",dxRot);
        _xRot = angle;

        emit xRotationChanged(angle);
        POST_updateGL(); // post update to prevent shaking, by RZC 080910
    }
}

void V3dR_GLWidget::setYRotation(int angle)
{
    NORMALIZE_angle( angle );
    if (angle != _yRot) {
        _absRot = false;
        dyRot = angle-_yRot;       //qDebug("dyRot=%d",dyRot);
        NORMALIZE_angleStep(dyRot); //qDebug("dyRot=%d",dyRot);
        _yRot = angle;

        emit yRotationChanged(angle);
        POST_updateGL(); // post update to prevent shaking, by RZC 080910
    }
}

void V3dR_GLWidget::setYRotation(float angle)
{
    NORMALIZE_angle( angle );
    if (angle != _yRot) {
        _absRot = false;
        dyRot = angle-_yRot;       //qDebug("dyRot=%d",dyRot);
        NORMALIZE_angleStep(dyRot); //qDebug("dyRot=%d",dyRot);
        _yRot = angle;

        emit yRotationChanged(angle);
        POST_updateGL(); // post update to prevent shaking, by RZC 080910
    }
}

void V3dR_GLWidget::setZRotation(int angle)
{
    NORMALIZE_angle( angle );
    if (angle != _zRot) {
        _absRot = false;
        dzRot = angle-_zRot;       //qDebug("dzRot=%d",dzRot);
        NORMALIZE_angleStep(dzRot); //qDebug("dzRot=%d",dzRot);
        _zRot = angle;

        emit zRotationChanged(angle);
        POST_updateGL(); // post update to prevent shaking, by RZC 080910
    }
}

void V3dR_GLWidget::setZRotation(float angle)
{
    NORMALIZE_angle( angle );
    if (angle != _zRot) {
        _absRot = false;
        dzRot = angle-_zRot;       //qDebug("dzRot=%d",dzRot);
        NORMALIZE_angleStep(dzRot); //qDebug("dzRot=%d",dzRot);
        _zRot = angle;

        emit zRotationChanged(angle);
        POST_updateGL(); // post update to prevent shaking, by RZC 080910
    }
}

void V3dR_GLWidget::resetRotation(bool b_emit)
{
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            mRot[i*4 +j] = ((i==j)? 1 : 0); // up-left 3x3 Identity matrix

    dxRot=dyRot=dzRot= 0;
    _xRot=_yRot=_zRot= 0;

    if (b_emit) //100720: for abs?Rotation's correct cursor position in spinBox
    {
        emit xRotationChanged(0);
        emit yRotationChanged(0);
        emit zRotationChanged(0);
        POST_updateGL();
    }
}

void V3dR_GLWidget::modelRotation(int xRotStep, int yRotStep, int zRotStep)
{
    if (xRotStep) setXRotation(_xRot + xRotStep);
    if (yRotStep) setYRotation(_yRot + yRotStep);
    if (zRotStep) setZRotation(_zRot + zRotStep);

    DO_updateGL(); // direct updateGL for no-event-loop animation, by RZC 080930
}

void V3dR_GLWidget::viewRotation(int xRotStep, int yRotStep, int zRotStep)
{
    double rx,ry,rz;
    rx = xRotStep; ry = yRotStep; rz = zRotStep;
    ViewRotToModel(mRot, rx, ry, rz);

    xRotStep = round(rx); yRotStep = round(ry); zRotStep = round(rz); // -rz for mouse gesture of z-rot
    modelRotation(xRotStep, yRotStep, zRotStep);
}



void V3dR_GLWidget::absoluteRotPose() //100723 RZC
{
    //mRot --> (xRot,yRot,zRot)
    double M[4][4];
    MAT16_TO_MAT4x4( mRot, M );
    // NOTE:  M is column-first-index & 0-based

#define A(i,j)   M[j-1][i-1]
#define PI 3.14159265
    double rx, ry, rz;

    if (A(1,3) == -1)
    {
        ry = PI*0.5;
        rz = 0;
        //rx = atan2( A(2,1), A(3,1) ) + rz; // this is wrong
        rx = asin( A(2,1) );
    }
    if (A(1,3) == 1)
    {
        ry = PI*1.5;
        rz = 0;
        //rx = atan2( A(2,1), A(3,1) ) - rz; // this is wrong
        rx = asin( -A(2,1) );
    }
    else
    {
        ry = asin( -A(1,3) );
        rz = atan2( A(1,2), A(1,1) );
        rx = atan2( A(2,3), A(3,3) );
    }

    int xRot = round(- rx/PI*180); //must need be negative?
    int yRot = round(- ry/PI*180);
    int zRot = round(- rz/PI*180);
    NORMALIZE_angle(xRot);
    NORMALIZE_angle(yRot);
    NORMALIZE_angle(zRot);

    doAbsoluteRot(xRot, yRot, zRot);
}

void V3dR_GLWidget::doAbsoluteRot(int xRot, int yRot, int zRot) //100723 RZC
{
    NORMALIZE_angle(xRot);
    NORMALIZE_angle(yRot);
    NORMALIZE_angle(zRot);

    emit xRotationChanged(xRot);
    emit yRotationChanged(yRot);
    emit zRotationChanged(zRot);

    _xRot = xRot;
    _yRot = yRot;
    _zRot = zRot;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
        // rotation order X--Y--Z
        glRotated( _xRot,  1,0,0);
        glRotated( _yRot,  0,1,0);
        glRotated( _zRot,  0,0,1);
    glGetDoublev(GL_MODELVIEW_MATRIX, mRot);
    glPopMatrix();

    dxRot=dyRot=dzRot= 0;
    _absRot = true;

    POST_updateGL();
}

void V3dR_GLWidget::doAbsoluteRot(float xRot, float yRot, float zRot) // 2011 Feb 08 CMB
{
    NORMALIZE_angle(xRot);
    NORMALIZE_angle(yRot);
    NORMALIZE_angle(zRot);

    emit xRotationChanged(xRot);
    emit yRotationChanged(yRot);
    emit zRotationChanged(zRot);

    _xRot = xRot;
    _yRot = yRot;
    _zRot = zRot;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
        // rotation order X--Y--Z
        glRotated( _xRot,  1,0,0);
        glRotated( _yRot,  0,1,0);
        glRotated( _zRot,  0,0,1);
    glGetDoublev(GL_MODELVIEW_MATRIX, mRot);
    glPopMatrix();

    dxRot=dyRot=dzRot= 0;
    _absRot = true;

    POST_updateGL();
}

void V3dR_GLWidget::lookAlong(float xLook, float yLook, float zLook) //100812 RZC
{
    if (!renderer)  return;

    //XYZ view(-xLook, -yLook, -zLook);
    XYZ view(-xLook*flip_X, -yLook*flip_Y, -zLook*flip_Z);
    normalize(view);
    XYZ eye = view * (renderer->getViewDistance());
    XYZ at(0,0,0);
    XYZ up(0,1,0);
    if (cross(up, view)==0) up = up + 0.01;   //make sure that cross(up,view)!=0

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
        gluLookAt(eye.x,eye.y,eye.z,
                at.x,at.y,at.z,
                up.x,up.y,up.z);
    glGetDoublev(GL_MODELVIEW_MATRIX, mRot);
    glPopMatrix();

    absoluteRotPose();
}

void V3dR_GLWidget::resetAltCenter()
{
    for (int i=0; i<4; i++)
            mAltC[i] = ((i==3)? 1 : 0);		// Original point
}

void V3dR_GLWidget::setAltCenter(float xC, float yC, float zC)
{
    mAltC[0] = xC; mAltC[1] = yC; mAltC[2] = zC; mAltC[3] = 1;
}


void V3dR_GLWidget::resetZoomShift()
{
    setZoom(0);
    setXShift(0);  dxShift=0;
    setYShift(0);  dyShift=0;
    setZShift(0);  dzShift=0;

    for (int i=0; i<3; i++)	mRot[i*4 +3] = mRot[3*4 +i] = 0; mRot[3*4 +3] = 1; // translation clear
}

void V3dR_GLWidget::setZoom(int zr)
{
    //qDebug("V3dR_GLWidget::setZoom = %i",zr);
    zr = CLAMP(-ZOOM_RANGE, ZOOM_RANGE, zr);
    if (int(_zoom) != zr) {
        _zoom = zr;
        if (renderer)
        {
            if (zr>=100) //40
            {
                //v3d_msg("Now prepare to enter the zr>40 wheel event!");
                //check if terafly exists
                QDir pluginsDir = QDir(qApp->applicationDirPath());
            #if defined(Q_OS_WIN)
                if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
                    pluginsDir.cdUp();
            #elif defined(Q_OS_MAC)
                if (pluginsDir.dirName() == "MacOS") {
                    pluginsDir.cdUp();
                    pluginsDir.cdUp();
                    pluginsDir.cdUp();
                }
            #endif

                QDir pluginsDir1 = pluginsDir;
                if (pluginsDir1.cd("plugins/teramanager")==true)
                {
                    renderer->zoomview_wheel_event();
                }
                else
                    renderer->setZoom( +float(zr)/100.f * ZOOM_RANGE_RATE); //sign can switch zoom orientation
            }
            else
                renderer->setZoom( +float(zr)/100.f * ZOOM_RANGE_RATE); //sign can switch zoom orientation
        }
        emit zoomChanged(zr);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setZoom(float zr)
{
    //qDebug("V3dR_GLWidget::setZoom = %i",zr);
    zr = CLAMP(-ZOOM_RANGE, ZOOM_RANGE, zr);
    if (_zoom != zr) {
        _zoom = zr;
        if (renderer)
        {
            if (zr>=100) //40
            {
                //v3d_msg("Now prepare to enter the zr>40 wheel event!");
                //check if terafly exists
                QDir pluginsDir = QDir(qApp->applicationDirPath());
            #if defined(Q_OS_WIN)
                if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
                    pluginsDir.cdUp();
            #elif defined(Q_OS_MAC)
                if (pluginsDir.dirName() == "MacOS") {
                    pluginsDir.cdUp();
                    pluginsDir.cdUp();
                    pluginsDir.cdUp();
                }
            #endif

                QDir pluginsDir1 = pluginsDir;
                if (pluginsDir1.cd("plugins/teramanager")==true)
                {
                    renderer->zoomview_wheel_event();
                }
                else
                    renderer->setZoom( +float(zr)/100.f * ZOOM_RANGE_RATE); //sign can switch zoom orientation
            }
            else
                renderer->setZoom( +float(zr)/100.f * ZOOM_RANGE_RATE); //sign can switch zoom orientation
        }
        emit zoomChanged(int(zr));
        POST_updateGL();
    }
}

void V3dR_GLWidget::setXShift(int s)
{
    s = CLAMP(-SHIFT_RANGE, SHIFT_RANGE, s);
    if (int(_xShift) != s) {
        dxShift = s-_xShift;

        _xShift = s;
        emit xShiftChanged(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setXShift(float s)
{
    s = CLAMP(-SHIFT_RANGE, SHIFT_RANGE, s);
    if (_xShift != s) {
        dxShift = s-_xShift;

        _xShift = s;
        emit xShiftChanged(int(s));
        POST_updateGL();
    }
}

void V3dR_GLWidget::setYShift(int s)
{
    s = CLAMP(-SHIFT_RANGE, SHIFT_RANGE, s);
    if (int(_yShift) != s) {
        dyShift = s-_yShift;

        _yShift = s;
        emit yShiftChanged(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setYShift(float s)
{
    s = CLAMP(-SHIFT_RANGE, SHIFT_RANGE, s);
    if (_yShift != s) {
        dyShift = s-_yShift;

        _yShift = s;
        emit yShiftChanged(int(s));
        POST_updateGL();
    }
}

void V3dR_GLWidget::setZShift(int s)
{
    if (int(_zShift) != s) {
        dzShift = s-_zShift;

        _zShift = s;
        emit zShiftChanged(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setZShift(float s)
{
    if (_zShift != s) {
        dzShift = s-_zShift;

        _zShift = s;
        emit zShiftChanged(int(s));
        POST_updateGL();
    }
}

void V3dR_GLWidget::setFrontCut(int s)
{
    if (_fCut != s) {
        _fCut = s;
        if (renderer) renderer->setViewClip(s/(float)CLIP_RANGE);

        emit changeFrontCut(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setXCut0(int s)
{
    if (_xCut0 != s) {
        int DX = MAX(0, dataDim1()-1);
        if (s+dxCut>DX)  s = DX-dxCut;

        _xCut0 = s;
        if (renderer) renderer->setXCut0(s);

        if (_xCut0+dxCut>_xCut1)	setXCut1(_xCut0+dxCut); //081029,100913
        if (lockX && _xCut0+dxCut<_xCut1)	setXCut1(_xCut0+dxCut); //100913, 110713
        setXYZSurfure(renderer->b_surfZLock);

        emit changeXCut0(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setXCut1(int s)
{
    if (_xCut1 != s) {
        if (s-dxCut<0)  s = dxCut;

        _xCut1 = s;
        if (renderer) renderer->setXCut1(s);

        if (_xCut0>_xCut1-dxCut)	setXCut0(_xCut1-dxCut);
        if (lockX && _xCut0<_xCut1-dxCut)	setXCut0(_xCut1-dxCut); //100913,110713
        setXYZSurfure(renderer->b_surfZLock);

        emit changeXCut1(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setYCut0(int s)
{
    if (_yCut0 != s) {
        int DY = MAX(0, dataDim2()-1);
        if (s+dyCut>DY)  s = DY-dyCut;

        _yCut0 = s;
        if (renderer) renderer->setYCut0(s);

        if (_yCut0+dyCut>_yCut1)	setYCut1(_yCut0+dyCut);
        if (lockY && _yCut0+dyCut<_yCut1)	setYCut1(_yCut0+dyCut);
        setXYZSurfure(renderer->b_surfZLock);
        emit changeYCut0(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setYCut1(int s)
{
    if (_yCut1 != s) {
        if (s-dyCut<0)  s = dyCut;

        _yCut1 = s;
        if (renderer) renderer->setYCut1(s);

        if (_yCut0>_yCut1-dyCut)	setYCut0(_yCut1-dyCut);
        if (lockY && _yCut0<_yCut1-dyCut)	setYCut0(_yCut1-dyCut);
        setXYZSurfure(renderer->b_surfZLock);
        emit changeYCut1(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setZCut0(int s)
{
    if (_zCut0 != s) {
        int DZ = MAX(0, dataDim3()-1);
        if (s+dzCut>DZ)  s = DZ-dzCut;

        _zCut0 = s;
        if (renderer) renderer->setZCut0(s);

        if (_zCut0+dzCut>_zCut1)	setZCut1(_zCut0+dzCut);
        if (lockZ && _zCut0+dzCut<_zCut1)	setZCut1(_zCut0+dzCut);
        setXYZSurfure(renderer->b_surfZLock);

        emit changeZCut0(_zCut0);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setZCut1(int s)
{
    if (_zCut1 != s) {
        if (s-dzCut<0)  s = dzCut;

        _zCut1 = s;
        if (renderer) renderer->setZCut1(s);

        if (_zCut0>_zCut1-dzCut)	setZCut0(_zCut1-dzCut);
        if (lockZ && _zCut0<_zCut1-dzCut)	setZCut0(_zCut1-dzCut);
        setXYZSurfure(renderer->b_surfZLock);

        emit changeZCut1(_zCut1);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setXYZSurfure(bool b)
{
    if(b)
    {
        Renderer_gl2* curr_renderer = (Renderer_gl2*)(getRenderer());
        curr_renderer->setBBXYZ((float) _idep->window3D->xcminSlider->value()-2, (float) _idep->window3D->xcmaxSlider->value()+2,
                                (float) _idep->window3D->ycminSlider->value()-2, (float) _idep->window3D->ycmaxSlider->value()+2,
                                (float) _idep->window3D->zcminSlider->value()-2, (float) _idep->window3D->zcmaxSlider->value()+2);
    }
}


void V3dR_GLWidget::setXCutLock(bool b)
{
    if (b)  dxCut = _xCut1-_xCut0;
    else    dxCut = 0;
    lockX = b? 1:0;  //110714
}
void V3dR_GLWidget::setYCutLock(bool b)
{
    if (b)	dyCut = _yCut1-_yCut0;
    else    dyCut = 0;
    lockY = b? 1:0;  //110714
}
void V3dR_GLWidget::setZCutLock(bool b)
{
    if (b)	dzCut = _zCut1-_zCut0;
    else    dzCut = 0;
    lockZ = b? 1:0;  //110714
}

void V3dR_GLWidget::setXCS(int s)
{
    if (_xCS != s)
    {
        _xCS = s;
        if (_renderMode==Renderer::rmCrossSection && renderer) renderer->setXCut0(s);
        POST_updateGL();
    }
}
void V3dR_GLWidget::setYCS(int s)
{
    if (_yCS != s)
    {
        _yCS = s;
        if (_renderMode==Renderer::rmCrossSection && renderer) renderer->setYCut0(s);
        POST_updateGL();
    }
}
void V3dR_GLWidget::setZCS(int s)
{
    if (_zCS != s)
    {
        _zCS = s;
        if (_renderMode==Renderer::rmCrossSection && renderer) renderer->setZCut0(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setXClip0(int s)
{
    if (_xClip0 != s) {
        _xClip0 = s;
        if (renderer) renderer->setXClip0(s/(float)CLIP_RANGE);

        if (_xClip0>_xClip1)	setXClip1(_xClip0); //081031
        emit changeXClip0(s);
        POST_updateGL();
    }
}
void V3dR_GLWidget::setXClip1(int s)
{
    if (_xClip1 != s) {
        _xClip1 = s;
        if (renderer) renderer->setXClip1(s/(float)CLIP_RANGE);

        if (_xClip0>_xClip1)	setXClip0(_xClip1); //081031
        emit changeXClip1(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setYClip0(int s)
{
    if (_yClip0 != s) {
        _yClip0 = s;
        if (renderer) renderer->setYClip0(s/(float)CLIP_RANGE);

        if (_yClip0>_yClip1)	setYClip1(_yClip0); //081031
        emit changeYClip0(s);
        POST_updateGL();
    }
}
void V3dR_GLWidget::setYClip1(int s)
{
    if (_yClip1 != s) {
        _yClip1 = s;
        if (renderer) renderer->setYClip1(s/(float)CLIP_RANGE);

        if (_yClip0>_yClip1)	setYClip0(_yClip1); //081031
        emit changeYClip1(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setZClip0(int s)
{
    if (_zClip0 != s) {
        _zClip0 = s;
        if (renderer) renderer->setZClip0(s/(float)CLIP_RANGE);

        if (_zClip0>_zClip1)	setZClip1(_zClip0); //081031
        emit changeZClip0(s);
        POST_updateGL();
    }
}
void V3dR_GLWidget::setZClip1(int s)
{
    if (_zClip1 != s) {
        _zClip1 = s;
        if (renderer) renderer->setZClip1(s/(float)CLIP_RANGE);

        if (_zClip0>_zClip1)	setZClip0(_zClip1); //081031
        emit changeZClip1(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::confidenceDialog()
{
    QString qtitle = "Confidence level";

    QDialog d(this);
    QGridLayout *formLayout = new QGridLayout;
    QScrollBar* confSlider = new QScrollBar(Qt::Horizontal,0);
    confSlider->setRange(0, 100);
    confSlider->setSingleStep(1);
    if(renderer->dispConfLevel==INT_MAX)
        confSlider->setValue(0);
    else
        confSlider->setValue(100-(100*(renderer->dispConfLevel-20)/255));
    confSlider->setPageStep(10);
    formLayout->addWidget(new QLabel("Confidence scores\n(range 0%~100%:)"), 1, 0, 1, 5);
    formLayout->addWidget(confSlider, 1, 5, 1, 15);
    QPushButton* cancel = new QPushButton("Close");
    formLayout->addWidget(cancel, 2, 15, 1, 5);
    d.setLayout(formLayout);
    d.setWindowTitle(qtitle);

    d.connect(cancel, SIGNAL(clicked()), &d, SLOT(reject()));
    d.connect(this, SIGNAL(changeConfCut(int)), confSlider, SLOT(setValue(int)));
    d.connect(confSlider, SIGNAL(valueChanged(int)), this, SLOT(setConfCut(int)));

    do
    {
        int ret = d.exec();
        if (ret==QDialog::Rejected)
            break;
        update();

    }
    while (true);
    POST_updateGL();
}

void V3dR_GLWidget::setConfCut(int s)
{
    if(s==0)
        renderer->dispConfLevel=INT_MAX;
    else
        renderer->dispConfLevel=(255*(100-s)/100)+20;
    emit changeConfCut(s);
    POST_updateGL();
}

void V3dR_GLWidget::setBright()
{
    QString qtitle = "Brighten/Darken the whole view";
    sUpdate_bright = 1;

    QDialog d(this);
    QFormLayout *formLayout = new QFormLayout;
    QSpinBox* spinBright = new QSpinBox(); spinBright->setRange(-100,100);
    QSpinBox* spinSlope = new QSpinBox(); spinSlope->setRange(0,200);
    formLayout->addRow(QObject::tr(    "Brightness\n (default 0, range -100~+100)%: "), spinBright);
    formLayout->addRow(QObject::tr("Contrast Slope\n (default 100,   range 0~200)%: "), spinSlope);
    QPushButton* ok     = new QPushButton("OK");
    QPushButton* cancel = new QPushButton("Close");
    QPushButton* reset  = new QPushButton("Reset");
    QFormLayout* button_right = new QFormLayout;
    button_right->addRow(reset, cancel);
    formLayout->addRow(ok, button_right);
    d.setLayout(formLayout);
    d.setWindowTitle(qtitle);

    d.connect(ok,     SIGNAL(clicked()), &d, SLOT(accept()));
    d.connect(cancel, SIGNAL(clicked()), &d, SLOT(reject()));

    //d.connect(reset,  SIGNAL(clicked()), &d, SIGNAL(done(10))); //  connect signal to slot with constant parameter
    QSignalMapper mapper(this);
    mapper.setMapping(reset, 10); connect(reset, SIGNAL(clicked()), &mapper, SLOT(map()));
    connect(&mapper, SIGNAL(mapped(int)), &d, SLOT(done(int)));

    QPoint pos = d.pos();
    do
    {
        spinBright->setValue(_Bright);
        spinSlope->setValue(_Contrast+100);

        d.move(pos);
        int ret = d.exec();
        if (ret==QDialog::Rejected)
            break;
        pos = d.pos();

        _Bright = spinBright->value();
        _Contrast = spinSlope->value()-100;
        if (ret==10) //reset
        {
            _Bright = _Contrast = 0;
        }
        DO_updateGL();

    }
    while (true); //(ret==QMessageBox::Retry);
    sUpdate_bright = 0;
    POST_updateGL();
}

void V3dR_GLWidget::setBackgroundColor()
{
    QAction *actBackgroundColor=0, *actLineColor=0;
    QMenu colorMenu;

    actBackgroundColor = new QAction(tr("&Background Color"), this);
    colorMenu.addAction(actBackgroundColor);

    actLineColor = new QAction(tr("&Bounding Line Color"), this);
    colorMenu.addAction(actLineColor);

    QAction *act = colorMenu.exec(QCursor::pos());

    if (! renderer) return;
    else if (act==actBackgroundColor)
    {
        RGBA8 c;
        c = XYZW(renderer->color_background)*255;
        QColor qcolor = QColorFromRGBA8(c);

        if (v3dr_getColorDialog(&qcolor))
        {
            c = RGBA8FromQColor(qcolor);
            renderer->color_background = XYZW(c)/255.f;
        }
    }
    else if (act==actLineColor)
    {
        RGBA8 c;
        c = XYZW(renderer->color_line)*255;
        QColor qcolor = QColorFromRGBA8(c);

        if (v3dr_getColorDialog(&qcolor))
        {
            c = RGBA8FromQColor(qcolor);
            renderer->color_line = XYZW(c)/255.f;
        }
    }
    POST_updateGL();
}

void V3dR_GLWidget::switchBackgroundColor()
{
    RGBA8 c;
    c = XYZW(renderer->color_background)*255;
    QColor qcolor = QColorFromRGBA8(c);

    int diff = qcolor.red() + qcolor.green() + qcolor.blue();

    if(diff == 0) // black
    {
        renderer->color_background = backgroundColor/255.f;
    }
    else // non-black
    {
        backgroundColor = c;
        renderer->color_background = XYZW(0.f)/255.f;
    }

    POST_updateGL();
}

#ifdef _NEURON_ASSEMBLER_
void V3dR_GLWidget::getXlockStatus(bool status)
{
    if (this->getRenderer())
    {
        Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
        My4DImage* curImg = 0;
        if (this) curImg = v3dr_getImage4d(_idep);

        if (terafly::PMain::isInstantiated())
        {
            terafly::PMain& pMain = *(terafly::PMain::getInstance());
            if (pMain.fragTracePluginInstance) pMain.xLockStatus = status;
        }
    }
}

void V3dR_GLWidget::getYlockStatus(bool status)
{
    if (this->getRenderer())
    {
        Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
        My4DImage* curImg = 0;
        if (this) curImg = v3dr_getImage4d(_idep);

        if (terafly::PMain::isInstantiated())
        {
            terafly::PMain& pMain = *(terafly::PMain::getInstance());
            if (pMain.fragTracePluginInstance) pMain.yLockStatus = status;
        }
    }
}

void V3dR_GLWidget::getZlockStatus(bool status)
{
    if (this->getRenderer())
    {
        Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
        My4DImage* curImg = 0;
        if (this) curImg = v3dr_getImage4d(_idep);

        if (terafly::PMain::isInstantiated())
        {
            terafly::PMain& pMain = *(terafly::PMain::getInstance());
            if (pMain.fragTracePluginInstance) pMain.zLockStatus = status;
        }
    }
}
#endif

void V3dR_GLWidget::setVoxSize()
{
    //注释
    //terafly::CImport* importCheckPtr = terafly::CImport::instance();
//	if (importCheckPtr->getVMapRawData() != 0)
//	{
//		v3d_msg(tr("There is a terafly UI inhereted from this Vaa3D controls.\n Please adjust the voxel size from terafly control panel instead."));
//		return;
//	}

    QSettings voxSettings("SEU-Allen", "scaleBar_nonTerafly");

    this->setVoxSizeDlg = new QDialog;
    this->setVoxDlgPtr = new Ui::setVoxSizeDialog;
    this->setVoxDlgPtr->setupUi(this->setVoxSizeDlg);
    this->setVoxDlgPtr->doubleSpinBox->setValue(voxSettings.value("x").toDouble());
    this->setVoxDlgPtr->doubleSpinBox_2->setValue(voxSettings.value("y").toDouble());
    this->setVoxDlgPtr->doubleSpinBox_3->setValue(voxSettings.value("z").toDouble());
    this->setVoxSizeDlg->exec();

    voxSettings.setValue("x", this->setVoxDlgPtr->doubleSpinBox->value());
    voxSettings.setValue("y", this->setVoxDlgPtr->doubleSpinBox_2->value());
    voxSettings.setValue("z", this->setVoxDlgPtr->doubleSpinBox_3->value());

    renderer->paint();
}

void V3dR_GLWidget::callUpBrainAtlas()
{
    if (renderer)
    {
        renderer->editinput = 13;
        renderer->drawEditInfo();
        Renderer_gl1* rendererGL1 = static_cast<Renderer_gl1*>(this->getRenderer());
        cout << "test1" << endl;

        QPluginLoader* loader = new QPluginLoader("plugins/BrainAtlas/BrainAtlas.dll");
        if (!loader) v3d_msg("BrainAtlas module not found. Do nothing.");
        cout << "test2" << endl;

        XFormWidget* curXWidget = v3dr_getXWidget(_idep);
        cout << "test3" << endl;
        V3d_PluginLoader mypluginloader(curXWidget->getMainControlWindow());
        cout << "test4" << endl;
        mypluginloader.runPlugin(loader, "menu1");
    }
}

void V3dR_GLWidget::enableShowAxes(bool s)
{
    //qDebug("V3dR_GLWidget::setShowAxes = %i",s);
    if (renderer)
    {
        renderer->bShowAxes = _showAxes = (s>0);
        POST_updateGL();
    }
}

void V3dR_GLWidget::enableClipBoundingBox(bool b)  //141013 Hanbo Chen
{
    if (renderer)
    {
        renderer->b_useClipBoxforSubjectObjs = _clipBoxEnable = (b>0);
    }
}

void V3dR_GLWidget::enableShowBoundingBox(bool s)
{
    //qDebug("V3dR_GLWidget::setShowBoundingBox = %i",s);
    if (renderer)
    {
        renderer->bShowBoundingBox = _showBoundingBox = (s>0);
        POST_updateGL();
    }
}

void V3dR_GLWidget::enableOrthoView(bool s)
{
    //qDebug("V3dR_GLWidget::enableOrthoView = %i",s);
    if (s != _orthoView)
    if (renderer)
    {
        renderer->bOrthoView = _orthoView = (s);
        renderer->setupView(viewW, viewH);
        emit changeOrthoView(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setShowMarkers(int s)
{
    //qDebug("V3dR_GLWidget::setShowMarkers = %i",s);
    if (renderer)
    {
        switch(s)
        {
        case Qt::Unchecked: 		s = 0; break;
        case Qt::PartiallyChecked:	s = 1; break;
        default: s = 2; break;
        }
        renderer->sShowMarkers = s;
        POST_updateGL();
    }
}

void V3dR_GLWidget::setShowSurfObjects(int s)
{
    //qDebug("V3dR_GLWidget::setShowSurfObjects = %i",s);
    if (renderer)
    {
        switch(s)
        {
        case Qt::Unchecked: 		s = 0; break;
        case Qt::PartiallyChecked:	s = 1; break;
        default: s = 2; break;
        }
        renderer->sShowSurfObjects = s;
        POST_updateGL();
    }
}

void V3dR_GLWidget::enableMarkerLabel(bool s)
{
    //qDebug("V3dR_GLWidget::enableMarkerLabel = %i",s);
    if (renderer)
    {
        renderer->b_showMarkerLabel = s;
        //renderer->b_showMarkerName = s; //added by PHC, 110426
        POST_updateGL();
    }
}

void V3dR_GLWidget::setMarkerSize(int s)
{
    //qDebug("V3dR_GLWidget::setMarkerSize = %i",s);
    if (_markerSize != s) {
        _markerSize = s;
        if (renderer)	renderer->markerSize = s;
        emit changeMarkerSize(s); // Go back to the spin box and set the new value.
        POST_updateGL();
    }
}

void V3dR_GLWidget::enableSurfStretch(bool s)
{
    if (renderer)
    {
        renderer->b_surfStretch = s;
        POST_updateGL();
    }
}

void V3dR_GLWidget::enableSurfZLock(bool s)
{
    if (renderer && _idep && _idep->window3D)
    {
        renderer->b_surfZLock = s;
        Renderer_gl2* curr_renderer = (Renderer_gl2*)(getRenderer());
        if(curr_renderer->zMin == -1.0 && curr_renderer->zMax == 1.0)
        {
            Renderer_gl1* curr_renderer = (Renderer_gl1*)(getRenderer());
            curr_renderer->cuttingXYZ = true;
            setXYZSurfure(s);
            return;
        }

        curr_renderer->setBBcutFlag(s);
        _idep->window3D->zcminSlider->setValue(_idep->window3D->zcminSlider->value());
        _idep->window3D->zcmaxSlider->setValue(_idep->window3D->zcmaxSlider->value());
        _idep->window3D->xcminSlider->setValue(_idep->window3D->xcminSlider->value());
        _idep->window3D->xcmaxSlider->setValue(_idep->window3D->xcmaxSlider->value());
        setXYZSurfure(s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::toggleCellName()
{
    if (renderer)
    {
        renderer->b_showCellName = !(renderer->b_showCellName);
        POST_updateGL();
    }
}
void V3dR_GLWidget::toggleMarkerName() // toggle marker name display. by Lei Qu, 110425
{
    if (renderer)
    {
        renderer->b_showMarkerName = !(renderer->b_showMarkerName);
        //renderer->b_showMarkerLabel = !(renderer->b_showMarkerName);
        POST_updateGL();
    }
}


void V3dR_GLWidget::createSurfCurrentR()
{
    //qDebug("V3dR_GLWidget::createSurfCurrentR");
    if (renderer)
    {
        renderer->createSurfCurrent(0);
        POST_updateGL();
    }
}
void V3dR_GLWidget::createSurfCurrentG()
{
    //qDebug("V3dR_GLWidget::createSurfCurrentG");
    if (renderer)
    {
        renderer->createSurfCurrent(1);
        POST_updateGL();
    }
}
void V3dR_GLWidget::createSurfCurrentB()
{
    //qDebug("V3dR_GLWidget::createSurfCurrentB");
    if (renderer)
    {
        renderer->createSurfCurrent(2);
        POST_updateGL();
    }
}

void V3dR_GLWidget::updateColorMode(int colorMode){

    Renderer_gl2* curr_renderer = (Renderer_gl2*)getRenderer();
    curr_renderer->neuronColorMode = colorMode;
    POST_updateGL();

}


//defined for Katie's need to export the local 3D viewer starting and local locations //140811
int V3dR_GLWidget::getLocalStartPosX()
{
    if (_idep)
        return _idep->local_start.x;
    else
        return -1;
}

int V3dR_GLWidget::getLocalStartPosY()
{
    if (_idep){
        return _idep->local_start.y;
    }else{
        return -1;
    }
}

int V3dR_GLWidget::getLocalStartPosZ()
{
    if (_idep)
        return _idep->local_start.z;
    else
        return -1;
}

int V3dR_GLWidget::getLocalEndPosX()
{
if (_idep)
    return _idep->local_start.x + _idep->local_size.x - 1;
else
    return -1;
}

int V3dR_GLWidget::getLocalEndPosY()
{
    if (_idep)
        return _idep->local_start.y + _idep->local_size.y - 1;
    else
        return -1;
}

int V3dR_GLWidget::getLocalEndPosZ()
{
    if (_idep)
        return _idep->local_start.z + _idep->local_size.z - 1;
    else
        return -1;
}


//void V3dR_GLWidget::loadObjectFromFile()
//{
//	if (renderer)
//	{
//		renderer->loadObjectFromFile();
//		updateTool();
//		POST_updateGL();
//	}
//}
void V3dR_GLWidget::loadObjectFromFile(QString url)
{
    if (renderer)
    {
        if (url.size())
            renderer->loadObjectFromFile(Q_CSTR(url));
        else
            renderer->loadObjectFromFile(0);
        updateTool();
        POST_updateGL();
    }
}
void V3dR_GLWidget::loadObjectListFromFile()
{
    if (renderer)
    {
        renderer->loadObjectListFromFile();
        updateTool();
        POST_updateGL();
    }
}

void V3dR_GLWidget::saveSurfFile()
{
    //qDebug("V3dR_GLWidget::saveSurfFile");
    if (renderer) renderer->saveSurfFile();
}

///////////////////////////////////////////////////////////////
#define __renderer_state_option__

void V3dR_GLWidget::changeLineOption()
{
    if (! renderer) return;
    {
        int line_width = renderer->lineWidth;
        int node_size = renderer->nodeSize;
        int root_size = renderer->rootSize;

        QDialog d(this);
        QSpinBox* spinWidth = new QSpinBox(); spinWidth->setRange(1,20); spinWidth->setValue(line_width);
        QSpinBox* spinNode  = new QSpinBox(); spinNode->setRange(0,20); spinNode->setValue(node_size);
        QSpinBox* spinRoot  = new QSpinBox(); spinRoot->setRange(0,20); spinRoot->setValue(root_size);

        QFormLayout *formLayout = new QFormLayout;
        formLayout->addRow(QObject::tr("skeleton width: "), spinWidth);
        formLayout->addRow(QObject::tr("node point size: "), spinNode);
        formLayout->addRow(QObject::tr("root point size: "), spinRoot);
        QPushButton* ok     = new QPushButton("OK");
        QPushButton* cancel = new QPushButton("Cancel");
        formLayout->addRow(ok, cancel);
        d.setLayout(formLayout);
        d.setWindowTitle(QString("Skeleton Options for Neuron Structure"));

        d.connect(ok,     SIGNAL(clicked()), &d, SLOT(accept()));
        d.connect(cancel, SIGNAL(clicked()), &d, SLOT(reject()));
        if (d.exec()!=QDialog::Accepted)
            return;

        renderer->lineWidth = spinWidth->value();
        renderer->nodeSize = spinNode->value();
        renderer->rootSize = spinRoot->value();

        POST_updateGL();
    }
}

void V3dR_GLWidget::changeVolShadingOption()
{
    if (! renderer) return;
    {
        int tex_comp = renderer->tryTexCompress;
        int tex_3d = renderer->tryTex3D;
        int tex_npt = renderer->tryTexNPT;
        int tex_stream = renderer->tryTexStream;
        int shader = renderer->tryVolShader;

        QDialog d(this);
        QCheckBox* qComp  = new QCheckBox(); qComp->setChecked(tex_comp);
        QCheckBox* qT3D = new QCheckBox(); qT3D->setChecked(tex_3d);
        QCheckBox* qNPT = new QCheckBox(); qNPT->setChecked(tex_npt);
        QSpinBox* qStream  = new QSpinBox();  qStream->setRange(-1,2);  qStream->setValue(tex_stream);
        QCheckBox* qShader  = new QCheckBox(); qShader->setChecked(shader);

        qComp->setEnabled(supported_TexCompression());  if (!supported_TexCompression())  qComp->setChecked(0);
        qT3D->setEnabled(supported_Tex3D());			if (!supported_Tex3D())  qT3D->setChecked(0);
        qNPT->setEnabled(supported_TexNPT());			if (!supported_TexNPT())  qNPT->setChecked(0);
                                                        if (!supported_PBO())  qStream->setMaximum(0);
        qShader->setEnabled(supported_GL2());			if (!supported_GL2())  qShader->setChecked(0);

        QFormLayout *formLayout = new QFormLayout;
        formLayout->addRow(QObject::tr("Compressed Resident texture: "), qComp);
        formLayout->addRow(QObject::tr("3D Resident texture: "), qT3D);
        formLayout->addRow(QObject::tr("(preferable to Compressed) Non-power-of-two size texture: \n"), qNPT);
        formLayout->addRow(QObject::tr("Stream texture mode %1: \n"
                " [0] -- 512x512x256 Down-sampled data -> Down-sampled Resident texture      \n"
                " [1] -- Full resolution data -> Adaptive (stream && resident) texture       \n"
                " [2] -- Full resolution data -> Full resolution Stream texture              \n"
                "[-1] -- Full resolution data -> Full resolution Resident texture            \n"
                "         (prefer checking off '3D Resident texture' for [-1] mode, otherwise\n"
                "         it may cause crash due to exceeding the limit of your video card!)\n"
                ).arg(supported_PBO()? "(with PBO support)": "(without PBO support)"), qStream);
        formLayout->addRow(QObject::tr("(volume colormap) GLSL shader: "), qShader);
        QPushButton* ok     = new QPushButton("OK");
        QPushButton* cancel = new QPushButton("Cancel");
        formLayout->addRow(ok, cancel);
        formLayout->addRow(new QLabel("----------------------------------------------------------\n"
                "Note: some combination may cause wrong display or dead lock on low-end video card."
                ));
        d.setLayout(formLayout);
        d.setWindowTitle(QString("Volume Advanced Options about Texture/Shader "));

        d.connect(ok,     SIGNAL(clicked()), &d, SLOT(accept()));
        d.connect(cancel, SIGNAL(clicked()), &d, SLOT(reject()));
        if (d.exec()!=QDialog::Accepted)
            return;

        qDebug("V3dR_GLWidget::changeVolShadingOption begin %s", renderer->try_vol_state());

        renderer->tryTexCompress = qComp->isChecked();
        renderer->tryTex3D = qT3D->isChecked();
        renderer->tryTexNPT = qNPT->isChecked();
        renderer->tryTexStream = qStream->value();
        renderer->tryVolShader = qShader->isChecked();

        if (   tex_comp != renderer->tryTexCompress
            || tex_3d != renderer->tryTex3D
//#if BUFFER_NPT
            || tex_npt != renderer->tryTexNPT   //no need when always use power_of_two buffer
//#endif
            || ((tex_stream != renderer->tryTexStream)
                    && !(tex_stream==1 && renderer->tryTexStream==2)
                    && !(tex_stream==2 && renderer->tryTexStream==1))
            //|| shader != renderer->tryVolShader   //no need of reloading texture
            )
        {

            //=============================================================================
            PROGRESS_DIALOG( "Update Volume Shading Option", NULL);
            PROGRESS_PERCENT(10);
            {

                ////100720: re-preparing data only if need
                if (   (renderer->tryTexStream != 0 && renderer->beLimitedDataSize())
                    || (renderer->tryTexStream == 0 && ! renderer->beLimitedDataSize()))
                {
                    PROGRESS_PERCENT(30);

                    renderer->setupData(this->_idep);
                    if (renderer->hasError())	POST_CLOSE(this);
                    renderer->getLimitedDataSize(_data_size); //for updating slider size
                }

                PROGRESS_PERCENT(70);
                {
                    renderer->reinitializeVol(renderer->class_version()); //100720
                    if (renderer->hasError())	POST_CLOSE(this);
                }
            }
            PROGRESS_PERCENT(100);
            //=============================================================================

            // when initialize done, update status of control widgets
            //SEND_EVENT(this, QEvent::Type(QEvent_InitControlValue)); // use event instead of signal
            emit signalVolumeCutRange(); //100809

        }

        qDebug("V3dR_GLWidget::changeVolShadingOption end %s", renderer->try_vol_state());
        POST_updateGL();

    }
}

void V3dR_GLWidget::changeObjShadingOption()
{
    if (! renderer) return;
    {
        int poly_mode = renderer->polygonMode;
        int shader = renderer->tryObjShader;

        QDialog d(this);
        QComboBox* qMode = new QComboBox();
        qMode->addItem("Filled");
        qMode->addItem("Line");
        qMode->addItem("Point");
        qMode->addItem("Transparent");
        qMode->setCurrentIndex(poly_mode);
        QCheckBox* qShader  = new QCheckBox(); qShader->setChecked(shader);

        if (! supported_GL2())  qShader->setChecked(0);
        qShader->setEnabled(supported_GL2());

        QFormLayout *formLayout = new QFormLayout;
        formLayout->addRow(QObject::tr("Polygon drawing mode: "), qMode);
        formLayout->addRow(QObject::tr("Polygon outline mode \n(need OpenGL 2.0) "), qShader);
        QPushButton* ok     = new QPushButton("OK");
        QPushButton* cancel = new QPushButton("Cancel");
        formLayout->addRow(ok, cancel);
        formLayout->addRow(new QLabel("----------------------------------------------------------\n"
                "Note: transparent & outline mode may cause object selection difficult\n due to no depth information."
                ));
        d.setLayout(formLayout);
        d.setWindowTitle(QString("/Object Advanced Options "));

        d.connect(ok,     SIGNAL(clicked()), &d, SLOT(accept()));
        d.connect(cancel, SIGNAL(clicked()), &d, SLOT(reject()));
        if (d.exec()!=QDialog::Accepted)
            return;

        renderer->polygonMode = qMode->currentIndex();
        renderer->tryObjShader = qShader->isChecked();

        qDebug("V3dR_GLWidget::changeObjShadingOption to: (fill/line/point = %d) (shader = %d)",
                renderer->polygonMode, renderer->tryObjShader);

        POST_updateGL();
    }
}

void V3dR_GLWidget::updateControl()
{
    //qDebug("V3dR_GLWidget::updateControl");
    if (renderer)
    {
        emit changeVolCompress(renderer->tryTexCompress>0);
    }
}

void V3dR_GLWidget::togglePolygonMode()
{
    //qDebug("V3dR_GLWidget::togglePolygonMode");
    if (renderer)
    {
        renderer->togglePolygonMode();
        POST_updateGL();
    }
}

// For n-right-strokes curve shortcut ZJL 110920
void V3dR_GLWidget::toggleNStrokeCurveDrawing()
{
     if (renderer)
    {
        renderer->toggleNStrokeCurveDrawing();
        POST_updateGL();
    }
}

// five shortcuts, by ZZ,02212018
void V3dR_GLWidget::callStrokeCurveDrawingBBoxes()
{
    if (renderer && _idep && v3dr_getImage4d(_idep))
    {
        if (v3dr_getImage4d(_idep)->get_xy_view())
        {
            renderer->callStrokeCurveDrawingBBoxes();
            POST_updateGL();
        }
    }
}

void V3dR_GLWidget::callStrokeCurveDrawingGlobal()
{
    if (renderer && _idep && v3dr_getImage4d(_idep))
    {
        if (v3dr_getImage4d(_idep)->get_xy_view())
        {
            renderer->callStrokeCurveDrawingGlobal();
            POST_updateGL();
        }
    }
}
void V3dR_GLWidget::callStrokeRetypeMultiNeurons()
{
    if (renderer)
    {
        renderer->callStrokeRetypeMultiNeurons();
        POST_updateGL();
    }
}

void V3dR_GLWidget::callStrokeDeleteMultiNeurons()
{
    if (renderer)
    {
        renderer->callStrokeDeleteMultiNeurons();
        update();
    }
}

void V3dR_GLWidget::callStrokeSplitMultiNeurons()
{
    if (renderer)
    {
        renderer->callStrokeSplitMultiNeurons();
        POST_updateGL();
    }
}

void V3dR_GLWidget::callStrokeConnectMultiNeurons()
{
    if (renderer)
    {
        renderer->callStrokeConnectMultiNeurons();
        POST_updateGL();
    }
}

void V3dR_GLWidget::callShowBreakPoints()
{
    if (renderer)
    {
        renderer->callShowBreakPoints();
        POST_updateGL();
    }
}

void V3dR_GLWidget::callShowSubtree()
{
    if (renderer)
    {
        renderer->callShowSubtree();
        POST_updateGL();
    }
}

void V3dR_GLWidget::callShowConnectedSegs()
{
    if (renderer)
    {
        renderer->callShowConnectedSegs();
        POST_updateGL();
    }
}

void V3dR_GLWidget::subtreeHighlightModeMonitor()
{
    // This method fires signal to check if the renderer is in [highlighting subtree] mode every 50 ms.
    // If yes, do nothing; if no, restore the highlited color to its original color and call to Renderer_gl1::escPressed_subtree() to exit [highlighting subtree] mode.
    // -- MK, June, 2018

    if (!this->getRenderer()) return;

    Renderer_gl1* thisRenderer = static_cast<Renderer_gl1*>(this->getRenderer());
    if (thisRenderer->selectMode != Renderer::smShowSubtree) thisRenderer->escPressed_subtree();
    else
    {
        int pressedNumber = this->getNumKeyHolding();
        //cout << pressedNumber << " ";

        if (thisRenderer->connectEdit == Renderer::loopEdit)
        {
            if (pressedNumber > -1)
            {
                cout << "  --> assigned color(type) to looped segments:" << pressedNumber << " " << endl;
                My4DImage* curImg = 0;
                if (this) curImg = v3dr_getImage4d(_idep);

                if (thisRenderer->finalizedLoopsSet.empty()) return;
                for (set<set<size_t> >::iterator loopIt = thisRenderer->finalizedLoopsSet.begin(); loopIt != thisRenderer->finalizedLoopsSet.end(); ++loopIt)
                {
                    set<size_t> thisLoop = *loopIt;
                    for (set<size_t>::iterator it = thisLoop.begin(); it != thisLoop.end(); ++it)
                    {
                        for (vector<V_NeuronSWC_unit>::iterator unitIt = curImg->tracedNeuron.seg[*it].row.begin(); unitIt != curImg->tracedNeuron.seg[*it].row.end(); ++unitIt)
                            unitIt->type = pressedNumber;

                        thisRenderer->originalSegMap.erase(*it);
                    }
                }

                if (!thisRenderer->nonLoopErrors.empty())
                {
                    for (set<set<size_t> >::iterator loopIt = thisRenderer->nonLoopErrors.begin(); loopIt != thisRenderer->nonLoopErrors.end(); ++loopIt)
                    {
                        set<size_t> thisLoop = *loopIt;
                        for (set<size_t>::iterator it = thisLoop.begin(); it != thisLoop.end(); ++it)
                        {
                            for (vector<V_NeuronSWC_unit>::iterator unitIt = curImg->tracedNeuron.seg[*it].row.begin(); unitIt != curImg->tracedNeuron.seg[*it].row.end(); ++unitIt)
                                unitIt->type = 20;

                            thisRenderer->originalSegMap.erase(*it);
                        }
                    }
                }

                curImg->update_3drenderer_neuron_view(this, thisRenderer);
                curImg->proj_trace_history_append();
            }
        }
        else
        {
            if (pressedNumber > -1)
            {
                cout << "  --> assigned color(type) to highlighted neuron:" << pressedNumber << " " << endl;
                My4DImage* curImg = 0;
                if (this) curImg = v3dr_getImage4d(_idep);
                if (thisRenderer->originalSegMap.empty()) return;

                for (set<size_t>::iterator segIDit = thisRenderer->subtreeSegs.begin(); segIDit != thisRenderer->subtreeSegs.end(); ++segIDit)
                {
                    //for (map<size_t, vector<V_NeuronSWC_unit> >::iterator it = thisRenderer->originalSegMap.begin(); it != thisRenderer->originalSegMap.end(); ++it)
                    //{
                    for (vector<V_NeuronSWC_unit>::iterator nodeIt = thisRenderer->originalSegMap[*segIDit].begin(); nodeIt != thisRenderer->originalSegMap[*segIDit].end(); ++nodeIt)
                        nodeIt->type = pressedNumber;

                    curImg->tracedNeuron.seg[*segIDit].row = thisRenderer->originalSegMap[*segIDit];
                    //}
                }

                curImg->update_3drenderer_neuron_view(this, thisRenderer);
                curImg->proj_trace_history_append();
            }
        }

        QTimer::singleShot(50, this, SLOT(subtreeHighlightModeMonitor()));
    }
}



void V3dR_GLWidget::callDefine3DPolyline()
{
    if (renderer && _idep && v3dr_getImage4d(_idep))
    {
        if (v3dr_getImage4d(_idep)->get_xy_view())
        {
            renderer->callDefine3DPolyline();
            POST_updateGL();
        }
    }
}

void V3dR_GLWidget::callGDTracing()
{
    if (renderer && _idep && v3dr_getImage4d(_idep))
    {
        if (v3dr_getImage4d(_idep)->get_xy_view())
        {
            renderer->callGDTracing();
            POST_updateGL();
        }
    }
}

void V3dR_GLWidget::callCreateMarkerNearestNode()
{
    if (renderer)
    {
        QPoint gpos = mapFromGlobal(cursor().pos());
        renderer->callCreateMarkerNearestNode(gpos.x(),gpos.y());
        POST_updateGL();
    }
}
//end five shortcuts

void V3dR_GLWidget::callCreateSpecialMarkerNearestNode()
{
    if(renderer)
    {
        QPoint gpos = mapFromGlobal(cursor().pos());
        renderer->callCreateSpecialMarkerNearestNode(gpos.x(),gpos.y());
    }
}
//add special marker, by XZ, 20190720


// For curveline detection , by PHC 20170531
void V3dR_GLWidget::callCurveLineDetector(int option)
{
    if (renderer && _idep && v3dr_getImage4d(_idep))
    {
        if (v3dr_getImage4d(_idep)->get_xy_view())
        {
            if (option==0)
                v3dr_getImage4d(_idep)->get_xy_view()->popupImageProcessingDialog(QString(" -- GD Curveline"));
            else
                v3dr_getImage4d(_idep)->get_xy_view()->popupImageProcessingDialog(QString(" -- GD Curveline infinite"));
            POST_updateGL();
        }
    }
}

//For new stack loading, by ZZ 01212018
void V3dR_GLWidget::callLoadNewStack()
{
    if (renderer && _idep && v3dr_getImage4d(_idep))
    {
        if (v3dr_getImage4d(_idep)->get_xy_view())
        {
            v3dr_getImage4d(_idep)->get_xy_view()->popupImageProcessingDialog(QString(" -- Load New Stack"));
            POST_updateGL();
        }
    }
}

//for calling different auto tracers in terafly, by ZZ, 05142018
void V3dR_GLWidget::callAutoTracers()
{
    if (renderer && _idep && v3dr_getImage4d(_idep))
    {
        if (v3dr_getImage4d(_idep)->get_xy_view())
        {
            v3dr_getImage4d(_idep)->get_xy_view()->popupImageProcessingDialog(QString(" -- Call Auto Tracers"));
            POST_updateGL();
        }
    }
}
void V3dR_GLWidget::callcheckmode()
{
    if (renderer && _idep && v3dr_getImage4d(_idep))
    {
        if (v3dr_getImage4d(_idep)->get_xy_view())
        {
            v3dr_getImage4d(_idep)->get_xy_view()->popupImageProcessingDialog(QString(" -- Call Check Mode"));
            POST_updateGL();
        }
    }
}
void V3dR_GLWidget::returncheckmode()
{
    if (renderer && _idep && v3dr_getImage4d(_idep))
    {
        if (v3dr_getImage4d(_idep)->get_xy_view())
        {
            v3dr_getImage4d(_idep)->get_xy_view()->popupImageProcessingDialog(QString(" -- Return Check Mode"));
            POST_updateGL();
        }
    }
}

void V3dR_GLWidget::setDragWinSize(int csize)
{
     if (renderer)
    {
        renderer->setDragWinSize(csize);
        POST_updateGL();
    }
}

void V3dR_GLWidget::toggleLineType()
{
    //qDebug("V3dR_GLWidget::toggleLineType");
    if (renderer)
    {
        renderer->toggleLineType();
        POST_updateGL();
    }
}

void V3dR_GLWidget::toggleEditMode()
{
    //qDebug("V3dR_GLWidget::toggleEditMode");
    if (renderer)
    {
        renderer->toggleEditMode();
        POST_updateGL();
    }
}

void V3dR_GLWidget::setEditMode()
{
    if (renderer)
    {
        renderer->setEditMode();
        update();
    }
}

void V3dR_GLWidget::toggleTexFilter()
{
    //qDebug("V3dR_GLWidget::toggleTexFilter");
    if (renderer)
    {
        renderer->toggleTexFilter();
        POST_updateGL();
    }
    updateControl();
}

void V3dR_GLWidget::toggleTex2D3D()
{
    //qDebug("V3dR_GLWidget::toggleTex2D3D");
    if (renderer)
    {
        renderer->toggleTex2D3D();
        POST_updateGL();
    }
    updateControl();
}

void V3dR_GLWidget::toggleTexCompression()
{
    //qDebug("V3dR_GLWidget::toggleTexCompression");
    if (renderer)
    {
        renderer->toggleTexCompression();
        _volCompress = (renderer->tryTexCompress>0);
        POST_updateGL();
    }
    updateControl();
}

void V3dR_GLWidget::toggleTexStream()
{
    //qDebug("V3dR_GLWidget::toggleTexStream");
    if (renderer)
    {
        renderer->toggleTexStream();
        POST_updateGL();
    }
    updateControl();
}

void V3dR_GLWidget::toggleShader()
{
    //qDebug("V3dR_GLWidget::toggleShader");
    if (renderer)
    {
        renderer->toggleShader();
        POST_updateGL();
    }
    updateControl();
}
void V3dR_GLWidget::toggleObjShader()
{
    //qDebug("V3dR_GLWidget::toggleObjShader");
    if (renderer)
    {
        renderer->toggleObjShader();
        POST_updateGL();
    }
    updateControl();
}

void V3dR_GLWidget::showGLinfo()
{
    string info;
    GLinfoDetect(&info);
    QString qinfo = QString::fromStdString(info);

    // Qt OpenGL context format detection
//#if defined(USE_Qt5)
//#else
//#if (QT_VERSION > 0x040200)
//		QGLFormat f = format();
//		char buf[1024];
//		sprintf(buf,"   GLformat: (version = 0x%x) (samples double-buffer stereo plane overlay = %d %d %d %d %d)\n",
//				int(QGLFormat::openGLVersionFlags()),
//				f.samples(), f.doubleBuffer(), f.stereo(), f.plane(), f.hasOverlay());
//		qinfo += buf;
//		sprintf(buf,"   GLformat: (r g b a = %d %d %d %d) + (depth stencil accum = %d %d %d)\n",
//				f.redBufferSize(), f.greenBufferSize(), f.blueBufferSize(), f.alphaBufferSize(),
//				f.depthBufferSize(), f.stencilBufferSize(), f.accumBufferSize());
//		qinfo += buf;
//#endif
//#endif

    //QLabel *p = new QLabel(qinfo);
    QTextEdit *p = new QTextEdit(); //no parent, otherwise will be ghost
    p->setPlainText(qinfo);
    p->setReadOnly(true);
    //p->setTabStopWidth(8);
    p->setWordWrapMode(QTextOption::NoWrap);
//	p->currentFont().setFixedPitch(true);
    p->show();
    p->resize(700, 700);
}


void V3dR_GLWidget::updateWithTriView()
{
    if (renderer)
    try //080927
    {
        renderer->updateLandmark();
        renderer->updateTracedNeuron();
        //updateTool(); //assume has called in above functions
        update();
    }
    catch(...)
    {
        printf("Fail to run the V3dR_GLWidget::updateLandmark() function.\n");
    }
}

void V3dR_GLWidget::updateLandmark() //141018 Hanbo Chen
{
    if (renderer)
    try
    {
        renderer->updateLandmark();
        POST_updateGL();
    }
    catch(...)
    {
        printf("Fail to run the V3dR_GLWidget::updateLandmark() function.\n");
    }
}

void V3dR_GLWidget::updateImageData()
{
    qDebug("V3dR_GLWidget::updateImageData -----------------------------------------");


    PROGRESS_DIALOG( "Updating image", this);
    if(this->show_progress_bar)
    {
        PROGRESS_PERCENT(10);
    }
    {
        {
            if(this->show_progress_bar)
            {
                PROGRESS_PERCENT(30);
            }

            renderer->setupData(this->_idep);
            if (renderer->hasError())	POST_CLOSE(this);
            renderer->getLimitedDataSize(_data_size); //for update slider size
        }

        if(this->show_progress_bar)
        {
            PROGRESS_PERCENT(70);
        }
        {
            renderer->reinitializeVol(renderer->class_version()); //100720
            if (renderer->hasError())	POST_CLOSE(this);
        }
    }
    if(this->show_progress_bar)
    {
        PROGRESS_PERCENT(100);
    }
    //=============================================================================

    // when initialize done, update status of control widgets
    //SEND_EVENT(this, QEvent::Type(QEvent_InitControlValue)); // use event instead of signal
    emit signalVolumeCutRange(); //100809

    POST_updateGL();
}

void V3dR_GLWidget::reloadData()
{

    QString qtitle = "Reload data";
    if (QMessageBox::question(0, qtitle,
                        tr("Are you sure to RELOAD the initial data set? \n\n"
                           "(return to the initial data set, drop other data, but user-controlled parameters/users generated data structures won't be changed)."),
                        QMessageBox::No | QMessageBox::Yes,
                        QMessageBox::Yes)
        ==QMessageBox::No)
        return;

    v3d_msg("V3dR_GLWidget::reloadData -----------------------------------------");

    //reset by Hanchuan Peng 20140710
    this->_idep->labelfield_file.clear();
    this->_idep->swc_file_list.clear();
    this->_idep->surface_file.clear();
    this->_idep->pointcloud_file_list.clear();

    makeCurrent(); //ensure right context when concurrent animation, 081025 //090705 delete

    PROGRESS_DIALOG( "Reloading", this);
    if(this->show_progress_bar)
    {
        PROGRESS_PERCENT(10);
    }
    {
        //if (renderer)	renderer->cleanData(); //090705 delete this line

        if(this->show_progress_bar)
        {
            PROGRESS_PERCENT(30);
        }
        //=============================================================================
        if (renderer)
        {
            renderer->setupData(this->_idep);
            if (renderer->hasError())	POST_CLOSE(this);
            renderer->getLimitedDataSize(_data_size); //for update slider size
        }

        if(this->show_progress_bar)
        {
            PROGRESS_PERCENT(70);
        }
        if (renderer)
        {
            renderer->initialize(1); //090705 RZC: only treat as class Renderer_gl1
            if (renderer->hasError())	POST_CLOSE(this);
        }
        //=============================================================================
    }
    if(this->show_progress_bar)
    {
        PROGRESS_PERCENT(100);
    }

    emit signalVolumeCutRange(); //100809

    POST_EVENT(this, QEvent::Type(QEvent_OpenFiles)); // open objects after loading volume, 081025
    POST_EVENT(this, QEvent::Type(QEvent_Ready));  //081124

    updateTool(); //081222
    POST_updateGL();

}

void V3dR_GLWidget::cancelSelect()
{
    if (renderer) renderer->endSelectMode();
}


#ifdef __ALLOW_VR_FUNCS_
void V3dR_GLWidget::UpdateVRcollaInfo()
{
    if(myvrwin->VROutinfo.deletedcurvespos.size())
    {
        NeuronTree  nt = terafly::PluginInterface::getSWC();
        std::vector<XYZ> deletedcurves = myvrwin->VROutinfo.deletedcurvespos;
        std::vector<int> deletedcurvesindex;
        int deleteindex = 0;
        qDebug()<<"deletedcurves.size()"<<deletedcurves.size();
        for(int i=0;i<deletedcurves.size();++i)//   get global delete curve  index
        {
            XYZ dPOS(deletedcurves.at(i).x,deletedcurves.at(i).y,deletedcurves.at(i).z);
            for(int j=0;j < nt.listNeuron.size();++j)
            {
                NeuronSWC SS0;
                SS0 = nt.listNeuron.at(j);
                float dist = glm::sqrt((dPOS.x-SS0.x)*(dPOS.x-SS0.x)+(dPOS.y-SS0.y)*(dPOS.y-SS0.y)+(dPOS.z-SS0.z)*(dPOS.z-SS0.z));
                qDebug()<<"SSO POS"<<SS0.x<<" "<<SS0.y<<" "<<SS0.z<<" ";
                qDebug()<<"SSO n"<<SS0.n<<"   pn   "<<SS0.pn;
                //call the dist between pos & current node'position, then compare with the threshold
                if(dist <8.0)      //this function is copy from v3dr_gl_vr.h   2 is an experienced value
                {
                    int findrootindex = j;
                    qDebug()<<"findrootindex"<<findrootindex;
                    while(SS0.pn!=-1)
                    {
                        findrootindex-=1;
                        SS0 = nt.listNeuron[findrootindex];
                        qDebug()<<" nt.listNeuron[findrootindex].pn"<< nt.listNeuron[findrootindex].pn;
                        qDebug()<<"findrootindex"<<findrootindex;
                    }
                    deleteindex = findrootindex;
                    deletedcurvesindex.push_back(deleteindex);
                    qDebug()<<"deleteindex  liqiqqq"<<deleteindex;
                    break;
                }
                qDebug()<<dist<<"     test delete distance liqqqqq";
            }
            qDebug()<<"find  delete point done "<<i+1;
        }
        std::sort(deletedcurvesindex.begin(),deletedcurvesindex.end());//sort use default <
        deletedcurvesindex.erase(unique(deletedcurvesindex.begin(),deletedcurvesindex.end()), deletedcurvesindex.end());
        NeuronTree NewNT;
        qDebug()<<"begin create new nt";
        for(int indexinNewNT=0,deleteindex=0;deleteindex<deletedcurvesindex.size();)//  create new tree
        {

            if(indexinNewNT==deletedcurvesindex[deleteindex])
            {
                qDebug()<<"nt listNeuron index pn = "<<nt.listNeuron[indexinNewNT].pn;
                if(nt.listNeuron[indexinNewNT].pn!=-1)
                    qDebug()<<"algorithm is wrong check ur code!!";
                ++indexinNewNT;
                deleteindex++;
                while(indexinNewNT<nt.listNeuron.size())
                {
                    if(nt.listNeuron[indexinNewNT].pn!=-1)
                    ++indexinNewNT;
                    else
                        break;
                    qDebug()<<"pos 1 indexinNewNt"<<indexinNewNT;
                }
                qDebug()<<"skip indexNewNT = "<<indexinNewNT;
                qDebug()<<"skip deleteindex = "<<deleteindex;
            }
            else
            {
                qDebug()<<"NewNT.listNeuron.append"<<indexinNewNT;
                NewNT.listNeuron.append(nt.listNeuron.at(indexinNewNT));
                NewNT.hashNeuron.insert(nt.listNeuron.at(indexinNewNT).n, nt.listNeuron.size()-1);
                ++indexinNewNT;
            }
            qDebug()<<"pos 2";
            if(deleteindex>=deletedcurvesindex.size()&&indexinNewNT<nt.listNeuron.size())//process last delete point then copy rest SWC into NewNT
            {
                qDebug()<<"pos 3";
                while(indexinNewNT<nt.listNeuron.size())
                {
                qDebug()<<"NewNT.listNeuron.append"<<indexinNewNT;
                NewNT.listNeuron.append(nt.listNeuron.at(indexinNewNT));
                NewNT.hashNeuron.insert(nt.listNeuron.at(indexinNewNT).n, nt.listNeuron.size()-1);
                ++indexinNewNT;
                }
            }
        }
        if(NewNT.listNeuron.size())
        terafly::PluginInterface::setSWC(NewNT);

        qDebug()<<"update collaboration Infor done";
    }

}
#endif
///////////////////////////////////////////////////////////////////////////////////////////
#define __end_view3dcontrol_interface__
///////////////////////////////////////////////////////////////////////////////////////////
