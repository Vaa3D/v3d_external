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

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/*
 * v3dr_common.h
 *
 *  Created on: Sep 24, 2008
 *      Author: Zongcai Ruan and Hanchuan Peng
 * Last change. 20100406. change ZTHICK_RANGE to 20
 */

#ifndef V3DR_COMMON_H_
#define V3DR_COMMON_H_

//for X11/Qt, qt constant must be included before any header file that defines Status
#include <QtGui>

#include "GLee_r.h" //must before any other OpengGL header file// minor modified glee.h for win32 compatible, by RZC 2008-09-12
#include <QtOpenGL>
//#include <QtTest>

#include <exception>
#include <iostream>

//instead by freeglut_geometry_r.c
//#include "glut32.h" // minor modified 2000 glut.h for mingw

#include "../jba/newmat11/newmatap.h"
#include "../jba/newmat11/newmatio.h"

#include "../basic_c_fun/basic_4dimage.h"
#include "../basic_c_fun/basic_surf_objs.h"
#include "../basic_c_fun/color_xyz.h"
//#include "color_xyz.h" // RGB/RGBA/8/16i/32i/32f, XYZ/XYZW, BoundingBox


//=====================================================================================================================
//#ifdef WIN32  //081001: limited by 3G
//	#define LIMIT_VOLX 512
//	#define LIMIT_VOLY 512
//	#define LIMIT_VOLZ 128
//#else
	#define LIMIT_VOLX 512
	#define LIMIT_VOLY 512
	#define LIMIT_VOLZ 256
//#endif
//	#define LIMIT_VOLX 256
//	#define LIMIT_VOLY 256

#define IS_LESS_64BIT ((sizeof(void*)<8)? true:false)
#define IS_FITTED_VOLUME(dim1,dim2,dim3)  (dim1<=LIMIT_VOLX && dim2<=LIMIT_VOLY && dim3<=LIMIT_VOLZ)

#define ANGLE_X0 (15)			//degree
#define ANGLE_Y0 (360-20)		//degree
#define ANGLE_Z0 (360-2)		//degree
#define ANGLE_TICK 1 			//divided
#define MOUSE_SENSITIVE 1.0f
#define SHIFT_RANGE 100 		//percent of bound
#define ZOOM_RANGE  100         //percent of fov
#define ZOOM_RANGE_RATE 5       //zoom rate of fov
#define CLIP_RANGE  200 		//size of (-100,100)
#define ZTHICK_RANGE 20			//times
#define TRANSPARENT_RANGE 100   //nonlinear divided

#define POPMENU_OPACITY 1

// 081025 by RZC
#define WIN_SIZEX 1024 //800
#define WIN_SIZEY 768  //800
#define CTRL_SIZEX 300
#define MINVIEW_SIZEX 700  //800
#define MINVIEW_SIZEY 700  //800

#define QEvent_Ready (QEvent::User +1)
#define QEvent_OpenFiles (QEvent::User +2)
#define QEvent_DropFiles (QEvent::User +3)
#define QEvent_InitControlValue (QEvent::User +4)
#define QEvent_OpenLocal3DView (QEvent::User +5)


//========================================================================================================================
#ifdef Q_WS_MAC
#define CTRL2_MODIFIER Qt::MetaModifier
#else
#define CTRL2_MODIFIER Qt::ControlModifier
#endif

#define ALT2_MODIFIER Qt::AltModifier
#ifdef Q_WS_X11
#define ATL2_MODIFIER Qt::GroupSwitchModifier
#endif


#define POST_EVENT(pQ, eventType) {	if (pQ!=NULL) QCoreApplication::postEvent(pQ, new QEvent(eventType)); }
#define SEND_EVENT(pQ, eventType) {	if (pQ!=NULL) QCoreApplication::sendEvent(pQ, new QEvent(eventType)); }
#define POST_CLOSE(pQ)	POST_EVENT(pQ, QEvent::Close)
#define ACTIVATE(w)	  { if(w) {QWidget* pQ=(QWidget*)w; pQ->raise(); POST_EVENT(pQ, QEvent::MouseButtonPress);} }

#define SLEEP(t)  { QTime qtime;  qtime.start();  while( qtime.elapsed() < t); }

#define DELETE_AND_ZERO(p)	{ if ((p)!=NULL) delete (p); (p) = NULL; }
#define Q_CSTR(qs)  ( (qs).toStdString().c_str() )
#define QCOLOR_BGRA8(bgra)  ( QColor::fromRgba((unsigned int)(bgra)) )

#define MESSAGE(s) \
{\
	QMessageBox::information(0, "MESSAGE", QObject::tr("%1 \n\n in file(%2) at line(%3)").arg(s).arg(__FILE__).arg(__LINE__)); \
}
#define MESSAGE_ASSERT(s) \
{\
	if (!(s)) \
		QMessageBox::critical(0, "ASSERT", QObject::tr("ASSERT(%1) in file(%2) at line(%3)").arg(#s).arg(__FILE__).arg(__LINE__)); \
	Q_ASSERT(s); \
}

#define CATCH_TO_QString( type, msg ) \
	catch (std::exception& e) { \
		type = "[std]"; \
		msg = e.what(); \
	} \
	catch (int id) { \
		type = "[int]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (V3DLONG id) { \
		type = "[V3DLONG]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (unsigned int id) { \
		type = "[uint]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (unsigned V3DLONG id) { \
		type = "[ulong]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (char* str) { \
		type = "[char]"; \
		msg = (const char*)str; \
	} \
	catch (const char* str) { \
		type = "[cchar]"; \
		msg = str; \
	} \
	catch (...) { \
		type = "[unknown]"; \
		msg = QString("unknown exception in file(%1) at line(%2)").arg(__FILE__).arg(__LINE__)); \
	}


#define PROGRESS_DIALOG(text, widget)  	QProgressDialog progress( QString(text), 0, 0, 100, (QWidget*)widget, Qt::Tool | Qt::WindowStaysOnTopHint);
extern QProgressDialog progress;
#define PROGRESS_PARENT(widget)   progress.setParent( (QWidget*)widget ); //Qt::WShowModal
#define PROGRESS_TEXT(text)   { QApplication::setActiveWindow(&progress);  progress.setLabelText( QString(text) );  progress.repaint();}
#define PROGRESS_PERCENT(i)	  { QApplication::setActiveWindow(&progress);  progress.setValue(i);  progress.repaint(); \
								QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);} //exclude user input is more safe

//Each QThread can have its own event loop, exec(). It makes it possible to connect signals from other threads to slots in this threads.
//It also makes it possible to use classes that require the event loop, such as QTimer and QTcpSocket.
//However, that is is not possible to use any widget classes in the thread.
//class ProgressThread : public QThread // Cannot contain widget!!!
//{
//	Q_OBJECT
//public:
//	ProgressThread(QObject *parent = 0) : QThread(parent)
//	{	}
//public slots:
//	setPercent(int i)
//	{
//		;
//	}
//protected:
//	void run()
//	{
//		;
//	}
//}

// 090424RZC: because QColorDialog::getColor cannot handle correctly when user clicks Cancel
// this function is called in ColorEditor::color, LIST_COLOR(Renderer_tex2::processHit), V3dr_surfaceDialog::selectedColor
inline bool v3dr_getColorDialog( QColor *color, QWidget *parent=0)
{
	QRgb input = 0xff000000;
	if (color)	input = color->rgba();
	bool ok;
	QRgb ouput = QColorDialog::getRgba(input, &ok, parent);
	if (ok && color)  *color = QColor::fromRgba( ouput );
	return ok;
}


//===================================================================================================================
#ifndef test_main_cpp

#include "../v3d/v3d_core.h"
#include "../v3d/mainwindow.h"

inline
My4DImage* v3dr_getImage4d(void* idep)
{
	My4DImage* image4d = 0L;
	if (idep)
	{
		image4d = ( ((iDrawExternalParameter*)idep)->image4d );
	}
	return (image4d); // && image4d->valid())? image4d : 0L;
}
inline
XFormWidget* v3dr_getXWidget(void* idep)
{
	My4DImage* image4d = 0L;
	XFormWidget* xwidget = 0L;
	if (idep)
	{
		image4d = ( ((iDrawExternalParameter*)idep)->image4d );
		xwidget = ( ((iDrawExternalParameter*)idep)->xwidget );
	}
	return (xwidget && (xwidget->getImageData()==image4d))? xwidget : 0L;
}
inline
MainWindow* v3dr_getV3Dmainwindow(void* idep)
{
	MainWindow* window = 0L;
	if (idep)
	{
		window = ( ((iDrawExternalParameter*)idep)->V3Dmainwindow );
	}
	return window;
}
inline
QList<V3dR_MainWindow*>* v3dr_getV3Dview_plist(void* idep)
{
	QList<V3dR_MainWindow*>* plist = 0L;
	if (idep)
	{
		plist = ( ((iDrawExternalParameter*)idep)->p_list_3Dview_win );
	}
	return plist;
}

#else

#define iDrawExternalParameter char
#define My4DImage char
#define MainWindow char
#define v3d_msg(a,b) qDebug()<<a

#endif


#endif /* V3DR_COMMON_H_ */
