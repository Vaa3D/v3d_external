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

#ifdef __v3dwebservice__

// template function
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
		else if(string(pSoapPara->str_func) == "v3dopenfile3dwrot")
		{			
			do3dfunc();
		}
		else if(string(pSoapPara->str_func) == "v3dopenfile3dwzoom")
		{			
			do3dfunc();
		}
		else if(string(pSoapPara->str_func) == "v3dopenfile3dwshift")
		{	
			do3dfunc();
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
				if(existing_imgwin)
				{
					emit imageLoaded2Plugin();
				}
				else
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