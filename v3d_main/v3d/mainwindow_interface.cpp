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




/****************************************************************************
**
** by Hanchuan Peng
** 2010
* separate from mainwindow.h
*  100803 RZC: and some XFormWidget:: separated from v3d_core.h
**
****************************************************************************/

#include <QtGui>

#include "mainwindow.h"
#include "v3d_core.h"
#include "../3drenderer/v3dr_mainwindow.h"
#include "../3drenderer/v3dr_glwidget.h"


XFormWidget* MainWindow::currentImageWindow() {return activeMdiChild();}

My4DImage* MainWindow::currentImage()
{
	XFormWidget* w=currentImageWindow();
	if (w) return w->getImageData(); else return 0;
}



QList<void*> MainWindow::allWindowList()
{
	QList<void*> list;
	foreach (QWidget* w, workspace->windowList()) list << w;
	//qDebug()<<"MainWindow allWindowList: " << list;
	return list;
}
XFormWidget* MainWindow::validateImageWindow(void* window)
{
	//because createMdiChild() DOESNOT update the workspace->windowList(), so cannot to validate
//	XFormWidget* w = 0;
//	QList<void*> list = allWindowList();
//	for (int i=0; i<list.size(); i++)  if (list[i]==window) {w = (XFormWidget*)window; break;}
//	if (!w) qDebug()<<" invalidated window: "<< window;
//	return (XFormWidget*)w;
	return (XFormWidget*)window;
}
QString MainWindow::getWindowName(void* window)
{
	XFormWidget* w = validateImageWindow(window);
	QString name; if (w) name = w->windowTitle(); return name;
}
QStringList MainWindow::allWindowNameList()
{
	QStringList namelist;
	QList<void*> list = allWindowList();
	for (int i=0; i<list.size(); i++) namelist << getWindowName(list[i]);
	return namelist;
}

XFormWidget* MainWindow::newImageWindow(const QString &name)
{
	XFormWidget* w = createMdiChild();
	//qDebug()<<" allWindowList: "<<allWindowList(); //here is VERY STRANGE, workspace->addWindow(child) DOESNOT update the workspace->windowList()
	if (w)
	{
		//w->show();
		w->setCurrentFileName(name);
		qDebug()<<"MainWindow newImageWindow: "<< w->userFriendlyCurrentFile();
	}
	return w;
}
XFormWidget* MainWindow::updateImageWindow(void* window)
{
	XFormWidget* w = validateImageWindow(window);
	if (w)
	{
		w->show();
		w->updateViews();
		updateWorkspace();
	}
	return w;
}
XFormWidget* MainWindow::setImageName(void* window, const QString &name)
{
	XFormWidget* w = validateImageWindow(window);
	if (w)
	{
		w->setCurrentFileName(name);
		qDebug()<<"MainWindow setImageName: "<< w->userFriendlyCurrentFile();
	}
	return w;
}

My4DImage* MainWindow::getImage(void* window)
{
	XFormWidget* w = validateImageWindow(window);
	if (! w) return 0;
	return w->getImageData();
}
bool MainWindow::setImage(void* window, Image4DSimple *image)
{
	XFormWidget* w = validateImageWindow(window);
	if (w)
	{
		qDebug()<<"MainWindow setImage now: "<< w << image;
//image->data1d[100]=255;
printf("[%p]\n", image->getRawData());
		return w->transferImageData(image, image->getRawData())
				&& w->setCurrentFileName(w->userFriendlyCurrentFile());
	}
	else return false;
}

QList<LocationSimple> MainWindow::getLandmark(void* window)
{
	XFormWidget* w = validateImageWindow(window);
	if (! w) return QList<LocationSimple>();
	return w->getImageData()->listLandmarks;
}
bool MainWindow::setLandmark(void* window, QList<LocationSimple>& landmark_list)
{
	XFormWidget* w = validateImageWindow(window);
	if (w)
	{
		qDebug()<<"MainWindow setLandmark: "<< w << &landmark_list;
		w->getImageData()->listLandmarks = landmark_list;
		//missing update?, by Hanchuan Peng, 100602 //updateImageWindow
		return true;
	}
	else return false;
}

QList<QPolygon> MainWindow::getROI(void* window)
{
	XFormWidget* w = validateImageWindow(window);
	if (! w) return QList<QPolygon>();
	return w->get3ViewROI();
}
bool MainWindow::setROI(void* window, QList<QPolygon>& roi_list)
{
	XFormWidget* w = validateImageWindow(window);
	if (w)
	{
		qDebug()<<"MainWindow setROI: "<< w << &roi_list;
		return w->set3ViewROI(roi_list);
	}
	else return false;
}

NeuronTree MainWindow::getSWC(void* window)
{
	XFormWidget* w = validateImageWindow(window);
	if (! w) return NeuronTree();
	return V_NeuronSWC_list__2__NeuronTree(w->getImageData()->tracedNeuron);
}
bool MainWindow::setSWC(void* window, NeuronTree & nt)
{
	XFormWidget* w = validateImageWindow(window);
	if (w)
	{
		qDebug()<<"MainWindow set SWC: "<< w << &nt;
		w->getImageData()->tracedNeuron = NeuronTree__2__V_NeuronSWC_list(nt);
		//missing update?, by Hanchuan Peng, 100602 //updateImageWindow
		return true;
	}
	else return false;
}


V3D_GlobalSetting MainWindow::getGlobalSetting()
{
	return global_setting;
}
bool MainWindow::setGlobalSetting( V3D_GlobalSetting &gs )
{
	global_setting = gs;
	return true;
}

//void MainWindow::setFocusLocation(void* window, V3DLONG cx, V3DLONG cy, V3DLONG cz)
//{
//	qDebug()<<"setFocusLocation ...";
//	
//	//	XFormWidget* w = this->p_mainWindow->validateImageWindow(window);
//	//	if (w)
//	//	{
//	//		w->forceToChangeFocus((int)cx, (int)cy, (int)cz);
//	//	}
//	
//	return;
//}

/////////////////////////////////////////////////////////////////////////////////
#define __XFormWidget_interface__

QList<QPolygon> XFormWidget::get3ViewROI()
{
	QList<QPolygon> roi_list;
	My4DImage* img = getImageData();
	if (img)
	{
		roi_list << img->p_xy_view->roiPolygon;
		roi_list << img->p_yz_view->roiPolygon;
		roi_list << img->p_zx_view->roiPolygon;
	}
	return roi_list;
}
bool XFormWidget::set3ViewROI(QList<QPolygon> & roi_list)
{
	if (roi_list.size() !=3) return false;
	My4DImage* img = getImageData();
	if (img)
	{
		img->p_xy_view->roiPolygon = roi_list[0];
		img->p_yz_view->roiPolygon = roi_list[1];
		img->p_zx_view->roiPolygon = roi_list[2];
		return true;
	}
	else return false;
}

bool XFormWidget::transferImageData(Image4DSimple *img, unsigned char *a) // FIXME: Why is "a" not used ?
{
	if (! img || !img->valid())  return false;

  bool result = this->setImageData(
    img->getRawData(),
    img->getXDim(),
    img->getYDim(),
    img->getZDim(),
    img->getCDim(),
    img->getDatatype() );

	if (result)
	{
		if (! this->imgData)  return false;

	  this->imgData->setTDim( img->getTDim() );
	  this->imgData->setTimePackType( img->getTimePackType() );
	  this->imgData->setRezX( img->getRezX() ); //100626
	  this->imgData->setRezY( img->getRezY() );
	  this->imgData->setRezZ( img->getRezZ() );
	  this->imgData->setOriginX( img->getOriginX() ); //101007
	  this->imgData->setOriginY( img->getOriginY() );
	  this->imgData->setOriginZ( img->getOriginZ() );
	  this->imgData->setCustomStructPointer( img->getCustomStructPointer() );
		
	  img->setRawDataPointerToNull();

    return true;
	}
	else return false;
} 

void XFormWidget::open3DWindow()
{
	doImage3DView();
}
void XFormWidget::openROI3DWindow()
{
	doImage3DLocalRoiView();
}
void XFormWidget::close3DWindow()
{
	if (mypara_3Dview.b_still_open && mypara_3Dview.window3D)
	{
		mypara_3Dview.window3D->postClose();
	}
}
void XFormWidget::closeROI3DWindow()
{
	if (mypara_3Dlocalview.b_still_open && mypara_3Dlocalview.window3D)
	{
		mypara_3Dlocalview.window3D->postClose();
	}
}

void XFormWidget::pushObjectIn3DWindow()
{
	V3dR_GLWidget* w = 0;
	if (mypara_3Dview.b_still_open && mypara_3Dview.window3D
			&& (w = mypara_3Dview.window3D->getGLWidget()))
	{
		w->updateWithTriView();
	}
	if (mypara_3Dlocalview.b_still_open && mypara_3Dlocalview.window3D
			&& (w = mypara_3Dlocalview.window3D->getGLWidget()))
	{
		w->updateWithTriView();
	}
}
void XFormWidget::pushImageIn3DWindow()
{
	V3dR_GLWidget* w = 0;
	if (mypara_3Dview.b_still_open && mypara_3Dview.window3D
			&& (w = mypara_3Dview.window3D->getGLWidget()))
	{
		w->updateImageData();
	}
	if (mypara_3Dlocalview.b_still_open && mypara_3Dlocalview.window3D
			&& (w = mypara_3Dlocalview.window3D->getGLWidget()))
	{
		w->updateImageData();
	}
}
int XFormWidget::pushTimepointIn3DWindow(int timepoint)
{
	V3dR_GLWidget* w = 0;
	if (mypara_3Dview.b_still_open && mypara_3Dview.window3D
			&& (w = mypara_3Dview.window3D->getGLWidget()))
	{
		return w->setVolumeTimePoint(timepoint);
	}
	if (mypara_3Dlocalview.b_still_open && mypara_3Dlocalview.window3D
			&& (w = mypara_3Dlocalview.window3D->getGLWidget()))
	{
		return w->setVolumeTimePoint(timepoint);
	}
}

bool XFormWidget::screenShot3DWindow(QString filename)
{
	V3dR_GLWidget* w = 0;
	if (mypara_3Dview.b_still_open && mypara_3Dview.window3D
			&& (w = mypara_3Dview.window3D->getGLWidget()))
	{
		return w->screenShot(filename);
	}
}

bool XFormWidget::screenShotROI3DWindow(QString filename)
{
	V3dR_GLWidget* w = 0;
	if (mypara_3Dlocalview.b_still_open && mypara_3Dlocalview.window3D
			&& (w = mypara_3Dlocalview.window3D->getGLWidget()))
	{
		return w->screenShot(filename);
	}
}


V3dR_GLWidget * XFormWidget::getView3D()
{
	V3dR_GLWidget* w = 0;
	if (mypara_3Dview.b_still_open && mypara_3Dview.window3D)
			w = mypara_3Dview.window3D->getGLWidget();
	return w;
}
V3dR_GLWidget * XFormWidget::getLocalView3D()
{
	V3dR_GLWidget* w = 0;
	if (mypara_3Dlocalview.b_still_open && mypara_3Dlocalview.window3D)
			w = mypara_3Dlocalview.window3D->getGLWidget();
	return w;
}

//View3DControl * XFormWidget::getView3DControl()
//{
//	V3dR_GLWidget* w = 0;
//	if (mypara_3Dview.b_still_open && mypara_3Dview.window3D)
//			w = mypara_3Dview.window3D->getGLWidget();
//	if (w)
//		return w->getView3DControl();
//	else
//		return 0;
//}
//View3DControl * XFormWidget::getLocalView3DControl()
//{
//	V3dR_GLWidget* w = 0;
//	if (mypara_3Dlocalview.b_still_open && mypara_3Dlocalview.window3D)
//			w = mypara_3Dlocalview.window3D->getGLWidget();
//	if (w)
//		return w->getView3DControl();
//	else
//		return 0;
//}

