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
 ***
	by Yang Yu
	May 3, 2011

 **
 ****************************************************************************/

#include <QtGui>

#include "mainwindow.h"

#include "../basic_c_fun/basic_surf_objs.h"
#include "../plugin_loader/v3d_plugin_loader.h"

#include "v3d_core.h"
#include "../3drenderer/v3dr_mainwindow.h"

#include "../3drenderer/v3dr_glwidget.h"

#ifdef __V3DWSDEVELOP__

bool is3DviewerFunc(char *funcname)
{
	return ( (string(funcname) == "v3dopenfile3dwrot") ||
			(string(funcname) == "v3dopenfile3dwzoom") ||
			(string(funcname) == "v3dopenfile3dwshift") ||
			(string(funcname) == "v3dvolumeColormapDialog") ||
			(string(funcname) == "v3dsurfaceDialogHide") ||
			(string(funcname) == "v3dabsoluteRotPose") ||
			(string(funcname) == "v3dresetZoomShift") ||
			(string(funcname) == "v3dsetBackgroundColor") ||
			(string(funcname) == "v3dsetBright") ||
			(string(funcname) == "v3dtoggleCellName") ||
			(string(funcname) == "v3dtoggleMarkerName") ||
			(string(funcname) == "v3dcreateSurfCurrentR") ||
			(string(funcname) == "v3dcreateSurfCurrentG") ||
			(string(funcname) == "v3dcreateSurfCurrentB") ||
			(string(funcname) == "v3dloadObjectListFromFile") ||
			(string(funcname) == "v3dloadObjectFromFile") ||
			(string(funcname) == "v3dsaveSurfFile") ||
			(string(funcname) == "v3dtogglePolygonMode") ||
			(string(funcname) == "v3dtoggleLineType") ||
			(string(funcname) == "v3dtoggleObjShader") ||
			(string(funcname) == "v3dchangeLineOption") ||
			(string(funcname) == "v3dchangeVolShadingOption") ||
			(string(funcname) == "v3dchangeObjShadingOption") ||
			(string(funcname) == "v3dtoggleTexFilter") ||
			(string(funcname) == "v3dtoggleTex2D3D") ||
			(string(funcname) == "v3dtoggleTexCompression") ||
			(string(funcname) == "v3dtoggleTexStream") ||
			(string(funcname) == "v3dtoggleShader") ||
			(string(funcname) == "v3dshowGLinfo") ||
			(string(funcname) == "v3dupdateWithTriView") ||
			(string(funcname) == "v3dupdateImageData") ||
			(string(funcname) == "v3dreloadData") ||
			(string(funcname) == "v3dcancelSelect") ||
			(string(funcname) == "v3dsetThickness") ||
			(string(funcname) == "v3dsetVolumeTimePoint") ||
			(string(funcname) == "v3dsetCSTransparent") ||
			(string(funcname) == "v3dsetCurChannel") ||
			(string(funcname) == "v3dsurfaceSelectDialog") ||
			(string(funcname) == "v3dsurfaceSelectTab") ||
			(string(funcname) == "v3dsetFrontCut") ||
			(string(funcname) == "v3dsetShowMarkers") ||
			(string(funcname) == "v3dsetShowSurfObjects") ||
			(string(funcname) == "v3dsetMarkerSize") ||
			(string(funcname) == "v3dincVolumeTimePoint") ||
			(string(funcname) == "v3dsetRenderMode_Mip") ||
			(string(funcname) == "v3dsetRenderMode_Alpha") ||
			(string(funcname) == "v3dsetRenderMode_Cs3d") ||
			(string(funcname) == "v3dsetChannelR") ||
			(string(funcname) == "v3dsetChannelG") ||
			(string(funcname) == "v3dsetChannelB") ||
			(string(funcname) == "v3dsetVolCompress") ||
			(string(funcname) == "v3dresetRotation") ||
			(string(funcname) == "v3denableFrontSlice") ||
			(string(funcname) == "v3denableXSlice") ||
			(string(funcname) == "v3denableYSlice") ||
			(string(funcname) == "v3denableZSlice") ||
			(string(funcname) == "v3dsetXCutLock") ||
			(string(funcname) == "v3dsetYCutLock") ||
			(string(funcname) == "v3dsetZCutLock") ||
			(string(funcname) == "v3denableShowAxes") ||
			(string(funcname) == "v3denableShowBoundingBox") ||
			(string(funcname) == "v3denableOrthoView") ||
			(string(funcname) == "v3denableMarkerLabel") ||
			(string(funcname) == "v3denableSurfStretch") ||
			(string(funcname) == "v3dannotationDialog") ||
			(string(funcname) == "v3dmodelRotation") ||
			(string(funcname) == "v3dviewRotation") ||
			(string(funcname) == "v3dsetCut0") ||
			(string(funcname) == "v3dsetCut1") ||
			(string(funcname) == "v3dsetCS") ||
			(string(funcname) == "v3dsetClip0") ||
			(string(funcname) == "v3dsetClip1") ||
			(string(funcname) == "v3dlookAlong") );

}

// template function
void MainWindow::switch3dviewercontrol(V3dR_MainWindow *existing_3dviewer)
{
	if(string(pSoapPara->str_func) == "v3dopenfile3dwrot")
	{			
		existing_3dviewer->getGLWidget()->doAbsoluteRot(pSoapPara->v3dmsgxyzf->x, pSoapPara->v3dmsgxyzf->y, pSoapPara->v3dmsgxyzf->z);
	}
	else if(string(pSoapPara->str_func) == "v3dopenfile3dwzoom")
	{
		existing_3dviewer->getGLWidget()->setZoom(pSoapPara->v3dmsgf->s);
	}
	else if(string(pSoapPara->str_func) == "v3dopenfile3dwshift")
	{
		existing_3dviewer->getGLWidget()->setXShift(pSoapPara->v3dmsgxyzf->x);
		existing_3dviewer->getGLWidget()->setYShift(pSoapPara->v3dmsgxyzf->y);
		existing_3dviewer->getGLWidget()->setZShift(pSoapPara->v3dmsgxyzf->z);
	}
	else if(string(pSoapPara->str_func) == "v3dvolumeColormapDialog")
	{
		existing_3dviewer->getGLWidget()->volumeColormapDialog();
	}
	else if(string(pSoapPara->str_func) == "v3dsurfaceDialogHide")
	{
		existing_3dviewer->getGLWidget()->surfaceDialogHide();
	}
	else if(string(pSoapPara->str_func) == "v3dabsoluteRotPose")
	{
		existing_3dviewer->getGLWidget()->absoluteRotPose();
	}
	else if(string(pSoapPara->str_func) == "v3dresetZoomShift")
	{
		existing_3dviewer->getGLWidget()->resetZoomShift();
	}
	else if(string(pSoapPara->str_func) == "v3dsetBackgroundColor")
	{
		existing_3dviewer->getGLWidget()->setBackgroundColor();
	}
	else if(string(pSoapPara->str_func) == "v3dsetBright")
	{
		existing_3dviewer->getGLWidget()->setBright();
	}
	else if(string(pSoapPara->str_func) == "v3dtoggleCellName")
	{
		existing_3dviewer->getGLWidget()->toggleCellName();
	}
	else if(string(pSoapPara->str_func) == "v3dtoggleMarkerName")
	{
		existing_3dviewer->getGLWidget()->toggleMarkerName();
	}
	else if(string(pSoapPara->str_func) == "v3dcreateSurfCurrentR")
	{
		existing_3dviewer->getGLWidget()->createSurfCurrentR();
	}
	else if(string(pSoapPara->str_func) == "v3dcreateSurfCurrentG")
	{
		existing_3dviewer->getGLWidget()->createSurfCurrentG();
	}
	else if(string(pSoapPara->str_func) == "v3dcreateSurfCurrentB")
	{
		existing_3dviewer->getGLWidget()->createSurfCurrentB();
	}
	else if(string(pSoapPara->str_func) == "v3dloadObjectListFromFile")
	{
		existing_3dviewer->getGLWidget()->loadObjectListFromFile();
	}
	else if(string(pSoapPara->str_func) == "v3dloadObjectFromFile")
	{
		existing_3dviewer->getGLWidget()->loadObjectFromFile();
	}
	else if(string(pSoapPara->str_func) == "v3dsaveSurfFile")
	{
		existing_3dviewer->getGLWidget()->saveSurfFile();
	}
	else if(string(pSoapPara->str_func) == "v3dtogglePolygonMode")
	{
		existing_3dviewer->getGLWidget()->togglePolygonMode();
	}
	else if(string(pSoapPara->str_func) == "v3dtoggleLineType")
	{
		existing_3dviewer->getGLWidget()->toggleLineType();
	}
	else if(string(pSoapPara->str_func) == "v3dtoggleObjShader")
	{
		existing_3dviewer->getGLWidget()->toggleObjShader();
	}
	else if(string(pSoapPara->str_func) == "v3dchangeLineOption")
	{
		existing_3dviewer->getGLWidget()->changeLineOption();
	}
	else if(string(pSoapPara->str_func) == "v3dchangeVolShadingOption")
	{
		existing_3dviewer->getGLWidget()->changeVolShadingOption();
	}
	else if(string(pSoapPara->str_func) == "v3dchangeObjShadingOption")
	{
		existing_3dviewer->getGLWidget()->changeObjShadingOption();
	}
	else if(string(pSoapPara->str_func) == "v3dtoggleTexFilter")
	{
		existing_3dviewer->getGLWidget()->toggleTexFilter();
	}
	else if(string(pSoapPara->str_func) == "v3dtoggleTex2D3D")
	{
		existing_3dviewer->getGLWidget()->toggleTex2D3D();
	}
	else if(string(pSoapPara->str_func) == "v3dtoggleTexCompression")
	{
		existing_3dviewer->getGLWidget()->toggleTexCompression();
	}
	else if(string(pSoapPara->str_func) == "v3dtoggleTexStream")
	{
		existing_3dviewer->getGLWidget()->toggleTexStream();
	}
	else if(string(pSoapPara->str_func) == "v3dtoggleShader")
	{
		existing_3dviewer->getGLWidget()->toggleShader();
	}
	else if(string(pSoapPara->str_func) == "v3dshowGLinfo")
	{
		existing_3dviewer->getGLWidget()->showGLinfo();
	}
	else if(string(pSoapPara->str_func) == "v3dupdateWithTriView")
	{
		existing_3dviewer->getGLWidget()->updateWithTriView();
	}
	else if(string(pSoapPara->str_func) == "v3dupdateImageData")
	{
		existing_3dviewer->getGLWidget()->updateImageData();
	}
	else if(string(pSoapPara->str_func) == "v3dreloadData")
	{
		existing_3dviewer->getGLWidget()->reloadData();
	}
	else if(string(pSoapPara->str_func) == "v3dcancelSelect")
	{
		existing_3dviewer->getGLWidget()->cancelSelect();
	}
	else if(string(pSoapPara->str_func) == "v3dsetThickness")
	{
		existing_3dviewer->getGLWidget()->setThickness(pSoapPara->v3dmsgd->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetVolumeTimePoint")
	{
		existing_3dviewer->getGLWidget()->setVolumeTimePoint(pSoapPara->v3dmsgi->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetCSTransparent")
	{
		existing_3dviewer->getGLWidget()->setCSTransparent(pSoapPara->v3dmsgi->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetCurChannel")
	{
		existing_3dviewer->getGLWidget()->setCurChannel(pSoapPara->v3dmsgi->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsurfaceSelectDialog")
	{
		existing_3dviewer->getGLWidget()->surfaceSelectDialog(pSoapPara->v3dmsgi->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsurfaceSelectTab")
	{
		existing_3dviewer->getGLWidget()->surfaceSelectTab(pSoapPara->v3dmsgi->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetFrontCut")
	{
		existing_3dviewer->getGLWidget()->setFrontCut(pSoapPara->v3dmsgi->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetShowMarkers")
	{
		existing_3dviewer->getGLWidget()->setShowMarkers(pSoapPara->v3dmsgi->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetShowSurfObjects")
	{
		existing_3dviewer->getGLWidget()->setShowSurfObjects(pSoapPara->v3dmsgi->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetMarkerSize")
	{
		existing_3dviewer->getGLWidget()->setMarkerSize(pSoapPara->v3dmsgi->s);
	}
	else if(string(pSoapPara->str_func) == "v3dincVolumeTimePoint")
	{
		existing_3dviewer->getGLWidget()->incVolumeTimePoint(pSoapPara->v3dmsgf->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetRenderMode_Mip")
	{
		existing_3dviewer->getGLWidget()->setRenderMode_Mip(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetRenderMode_Alpha")
	{
		existing_3dviewer->getGLWidget()->setRenderMode_Alpha(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetRenderMode_Cs3d")
	{
		existing_3dviewer->getGLWidget()->setRenderMode_Cs3d(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetChannelR")
	{
		existing_3dviewer->getGLWidget()->setChannelR(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetChannelG")
	{
		existing_3dviewer->getGLWidget()->setChannelG(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetChannelB")
	{
		existing_3dviewer->getGLWidget()->setChannelB(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetVolCompress")
	{
		existing_3dviewer->getGLWidget()->setVolCompress(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3dresetRotation")
	{
		existing_3dviewer->getGLWidget()->resetRotation(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3denableFrontSlice")
	{
		existing_3dviewer->getGLWidget()->enableFrontSlice(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3denableXSlice")
	{
		existing_3dviewer->getGLWidget()->enableXSlice(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3denableYSlice")
	{
		existing_3dviewer->getGLWidget()->enableYSlice(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3denableZSlice")
	{
		existing_3dviewer->getGLWidget()->enableZSlice(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetXCutLock")
	{
		existing_3dviewer->getGLWidget()->setXCutLock(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetYCutLock")
	{
		existing_3dviewer->getGLWidget()->setYCutLock(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3dsetZCutLock")
	{
		existing_3dviewer->getGLWidget()->setZCutLock(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3denableShowAxes")
	{
		existing_3dviewer->getGLWidget()->enableShowAxes(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3denableShowBoundingBox")
	{
		existing_3dviewer->getGLWidget()->enableShowBoundingBox(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3denableOrthoView")
	{
		existing_3dviewer->getGLWidget()->enableOrthoView(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3denableMarkerLabel")
	{
		existing_3dviewer->getGLWidget()->enableMarkerLabel(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3denableSurfStretch")
	{
		existing_3dviewer->getGLWidget()->enableSurfStretch(pSoapPara->v3dmsgb->s);
	}
	else if(string(pSoapPara->str_func) == "v3dannotationDialog")
	{
		existing_3dviewer->getGLWidget()->annotationDialog(pSoapPara->v3dmsgad->dataClass, pSoapPara->v3dmsgad->surfaceType, pSoapPara->v3dmsgad->index);
	}
	else if(string(pSoapPara->str_func) == "v3dmodelRotation")
	{
		existing_3dviewer->getGLWidget()->modelRotation(pSoapPara->v3dmsgxyzi->x, pSoapPara->v3dmsgxyzi->y, pSoapPara->v3dmsgxyzi->z);
	}
	else if(string(pSoapPara->str_func) == "v3dviewRotation")
	{
		existing_3dviewer->getGLWidget()->viewRotation(pSoapPara->v3dmsgxyzi->x, pSoapPara->v3dmsgxyzi->y, pSoapPara->v3dmsgxyzi->z);
	}
	else if(string(pSoapPara->str_func) == "v3dsetCut0")
	{
		existing_3dviewer->getGLWidget()->setXCut0(pSoapPara->v3dmsgxyzi->x);
		existing_3dviewer->getGLWidget()->setYCut0(pSoapPara->v3dmsgxyzi->y);
		existing_3dviewer->getGLWidget()->setZCut0(pSoapPara->v3dmsgxyzi->z);
	}
	else if(string(pSoapPara->str_func) == "v3dsetCut1")
	{
		existing_3dviewer->getGLWidget()->setXCut1(pSoapPara->v3dmsgxyzi->x);
		existing_3dviewer->getGLWidget()->setYCut1(pSoapPara->v3dmsgxyzi->y);
		existing_3dviewer->getGLWidget()->setZCut1(pSoapPara->v3dmsgxyzi->z);
	}
	else if(string(pSoapPara->str_func) == "v3dsetCS")
	{
		existing_3dviewer->getGLWidget()->setXCS(pSoapPara->v3dmsgxyzi->x);
		existing_3dviewer->getGLWidget()->setYCS(pSoapPara->v3dmsgxyzi->y);
		existing_3dviewer->getGLWidget()->setZCS(pSoapPara->v3dmsgxyzi->z);
	}
	else if(string(pSoapPara->str_func) == "v3dsetClip0")
	{
		existing_3dviewer->getGLWidget()->setXClip0(pSoapPara->v3dmsgxyzi->x);
		existing_3dviewer->getGLWidget()->setYClip0(pSoapPara->v3dmsgxyzi->y);
		existing_3dviewer->getGLWidget()->setZClip0(pSoapPara->v3dmsgxyzi->z);
	}
	else if(string(pSoapPara->str_func) == "v3dsetClip1")
	{
		existing_3dviewer->getGLWidget()->setXClip1(pSoapPara->v3dmsgxyzi->x);
		existing_3dviewer->getGLWidget()->setYClip1(pSoapPara->v3dmsgxyzi->y);
		existing_3dviewer->getGLWidget()->setZClip1(pSoapPara->v3dmsgxyzi->z);
	}
	else if(string(pSoapPara->str_func) == "v3dlookAlong")
	{
		existing_3dviewer->getGLWidget()->modelRotation(pSoapPara->v3dmsgxyzf->x, pSoapPara->v3dmsgxyzf->y, pSoapPara->v3dmsgxyzf->z);
	}
	
}

void MainWindow::do3dfunc()
{
	QString fileName(pSoapPara->str_message);
	
	if (!fileName.isEmpty()) 
	{
		// find triview window
		XFormWidget *existing_imgwin = findMdiChild(fileName);	
		
		// find 3d main window
		V3dR_MainWindow *existing_3dviewer = find3DViewer(fileName);
		
		// handling
		if(existing_imgwin)
		{
			if(!existing_3dviewer)
			{
				existing_imgwin->doImage3DView();
				existing_3dviewer = find3DViewer(fileName);
			}
			
			switch3dviewercontrol(existing_3dviewer);
		}
		else
		{
			this->loadV3DFile(pSoapPara->str_message, true, true);
			
			existing_imgwin = findMdiChild(fileName);
			if(!existing_imgwin) 
			{
				// try open image file in currrent dir
				QString tryFileName = QDir::currentPath() + "/" + fileName;
				v3d_msg(QString("Try to open the file [%1].").arg(tryFileName), 1);
				
				this->loadV3DFile(tryFileName, true, true);
				
				if(!existing_imgwin) 
				{
					v3d_msg(QString("The file [%1] does not exist! Do nothing.").arg(fileName), 1);
					return;	
				}
			}
			
			if(!existing_3dviewer)
			{
				existing_imgwin->doImage3DView();
				existing_3dviewer = find3DViewer(fileName);
			}
			
			switch3dviewercontrol(existing_3dviewer);
		}
		
	}
	else
	{
		v3d_msg(QString("The file [%1] does not exist! Do nothing.").arg(fileName), 1);
		return;	
	}
}


// slot function for init web service thread
void MainWindow::initWebService(V3DWebService *pws)
{
	connect(pws, SIGNAL(finished()), pws, SLOT(deleteLater()));
	pws->start();
}

// slot function for quit web service thread
void MainWindow::quitWebService(V3DWebService *pws)
{	
	pws->exit();
}

// slot function for passing soap parameters
void MainWindow::setSoapPara(soappara *pSoapParaInput)
{
	pSoapPara = pSoapParaInput;
}

// slot function for response web service
void MainWindow::webserviceResponse()
{	
	qDebug()<<"web service response here ...";
	
	this->setSoapPara(v3dws->getSoapPara());
	
	if(pSoapPara)
	{
		if(string(pSoapPara->str_func) == "helloworld")
		{
			QMessageBox::information((QWidget *)0, QString("title: v3d web service"), QString(pSoapPara->str_message));
		}
		else if(string(pSoapPara->str_func) == "v3dopenfile")
		{
			this->loadV3DFile(pSoapPara->str_message, true, false);
		}
		else if(is3DviewerFunc(pSoapPara->str_func))
		{			
			do3dfunc();
		}
		else if(string(pSoapPara->str_func) == "v3dsavefile") // plugin method call
		{
			// is the same file name, ignore cases
			if(QString(pSoapPara->v3dmsgsave->saveName).toUpper() == QString(pSoapPara->str_message).toUpper())
			{
				v3d_msg(QString("The saving file [%1] is the same to the original file [%2].").arg(pSoapPara->v3dmsgsave->saveName).arg(pSoapPara->str_message), 1);
				return;
			}
			
			//
			QString fileName(pSoapPara->str_message);
			
			// load image
			if (!fileName.isEmpty()) 
			{
				// replace the existing file ?
				if (QFile::exists(pSoapPara->v3dmsgsave->saveName))
				{
					if(QMessageBox::No == QMessageBox::question (0, "", QString("Do you want to replace the existing file [%1]?").arg(pSoapPara->v3dmsgsave->saveName), QMessageBox::Yes, QMessageBox::No))
					{
						return;
					}
				}
				
				// find triview window
				XFormWidget *existing_imgwin = findMdiChild(fileName);	
				
				// handling
				if(!existing_imgwin)
				{
					this->loadV3DFile(pSoapPara->str_message, true, false); // 3d viewer is not opened by default
					
					existing_imgwin = findMdiChild(fileName);
					if(!existing_imgwin) 
					{
						// try open image file in currrent dir
						QString tryFileName = QDir::currentPath() + "/" + fileName;
						v3d_msg(QString("Try to open the file [%1].").arg(tryFileName), 1);
						
						this->loadV3DFile(tryFileName, true, false);
						
						if(!existing_imgwin) 
						{
							v3d_msg(QString("The file [%1] does not exist!.").arg(fileName), 1);
							return;	
						}
					}					
				}
				
				//save
				if (activeMdiChild()->saveFile(pSoapPara->v3dmsgsave->saveName))
				{
					setCurrentFile(pSoapPara->v3dmsgsave->saveName);
					statusBar()->showMessage(tr("File saved"), 2000);
				}
			}
			else
			{
				v3d_msg(QString("The file [%1] does not exist! Do nothing.").arg(fileName), 1);
				return;	
			}
		}		
		else if(string(pSoapPara->str_func) == "v3dwscallpluginmethod") // plugin method call
		{			
			QString fileName(pSoapPara->str_message);
			
			// setting plugin name and method call
			setBooleanCLplugin(true);
			setPluginName(pSoapPara->v3dpluginm->pluginName);
			setPluginMethod(pSoapPara->v3dpluginm->pluginMethod);
			
			// load image
			if (!fileName.isEmpty()) 
			{
				// find triview window
				XFormWidget *existing_imgwin = findMdiChild(fileName);	
				
				// handling
				if(!existing_imgwin)
				{
					this->loadV3DFile(pSoapPara->str_message, true, false); // 3d viewer is not opened by default
					
					existing_imgwin = findMdiChild(fileName);
					if(!existing_imgwin) 
					{
						// try open image file in currrent dir
						QString tryFileName = QDir::currentPath() + "/" + fileName;
						v3d_msg(QString("Try to open the file [%1].").arg(tryFileName), 1);
						
						this->loadV3DFile(tryFileName, true, false);
						
						if(!existing_imgwin) 
						{
							v3d_msg(QString("The file [%1] does not exist! Try run plugin directly.").arg(fileName), 1);
							emit imageLoaded2Plugin();
							return;	
						}
					}
				}
				
				// call plugin
				emit imageLoaded2Plugin();
			}
			else
			{
				v3d_msg(QString("The file [%1] does not exist! Do nothing.").arg(fileName), 1);
				return;	
			}
		}
		else
		{
			QMessageBox::information((QWidget *)0, QString("title: v3d web service"), QString("Wrong function to invoke!"));
		}
	}
	
}

#endif //__v3dwebservice__