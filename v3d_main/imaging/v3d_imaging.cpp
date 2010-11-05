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


//by Hanchuan Peng, 20101008

#include "v3d_imaging.h"
#include "../basic_c_fun/v3d_message.h"

#include "../plugin_loader/v3d_plugin_loader.h"

//#include "../v3d/v3d_core.h"
#include "../v3d/mainwindow.h"

#ifdef __WIN32 
// #include "../sub_projects/imaging_piezo/microimaging.h"
#endif

bool v3d_imaging(MainWindow* mainwindow, const v3d_imaging_paras & p)
{
	v3d_msg(QString("Now try to do imaging [%1]").arg(p.OPS), 0);
	
	try 
	{
		XFormWidget *curw = mainwindow->activeMdiChild();
		if (!curw)
		{
			v3d_msg("No window open yet.");
			return false;
		}

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
		//if (pluginsDir.cd("plugins/64bit/Canvas_Eraser")==false)
		if (pluginsDir.cd("plugins/imaging_piezo")==false) 
		{
			v3d_msg("Cannot find ./plugins/imaging_piezo directory!");
			return false;
		}
		
		QStringList fileList = pluginsDir.entryList(QDir::Files);
		if (fileList.size()<1)
		{
			v3d_msg("Cannot find any file in the ./plugins/imaging_piezo directory!");
			return false;
		}
		
		QString fullpath = pluginsDir.absoluteFilePath(fileList.at(0)); //always just use the first file (assume it is the only one) found in the folder as the "correct" dll
		//the real dll name should be "microimaging.dll"
		
    	QPluginLoader* loader = new QPluginLoader(fullpath);
        if (!loader)
        {
        	qDebug("ERROR in V3d_PluginLoader::searchPluginFiles the imaging module(%s)", qPrintable(fullpath));
        	return false;
        }
		
		v3d_msg(fullpath, 0);
		
		V3d_PluginLoader mypluginloader(mainwindow);
		//mypluginloader.runPlugin(loader, QString("about this plugin"));

		curw->getImageData()->setCustomStructPointer((void *)(&p)); //to pass parameters to the imaging plugin
		
		mypluginloader.runPlugin(loader, p.OPS);
			//mypluginloader.runPlugin(loader, "about");
			
	}
	catch (...)
	{
		v3d_msg("Imaging has a problem", 1);
		return false;
	}
	
	return true;
}



