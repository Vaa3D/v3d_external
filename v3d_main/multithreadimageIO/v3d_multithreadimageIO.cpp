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


//by Yang Yu, 20101215

#include "v3d_multithreadimageIO.h"
#include "../basic_c_fun/v3d_message.h"

#include "../plugin_loader/v3d_plugin_loader.h"

#include "../v3d/mainwindow.h"


bool v3d_multithreadimageIO(XFormWidget *curw, const v3d_multithreadimageio_paras & p)
{
	QString mt_load_folder = "plugins/data_IO/multithreaded_image_loading";

	v3d_msg(QString("Now try to load image using the engine [%1]").arg(p.qOperation), 0);
	
	try 
	{
		// find the engine path
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
		//
		if (pluginsDir.cd(mt_load_folder)==false) 
		{
			v3d_msg(QString("Cannot find [%1] directory!").arg(mt_load_folder), 0);
			return false;
		}
		
		QStringList fileList = pluginsDir.entryList(QDir::Files);
		if (fileList.size()<1)
		{
			v3d_msg(QString("Cannot find any file in the [%1] directory!").arg(mt_load_folder), 0);
			return false;
		}
		
		QString fullpath = pluginsDir.absoluteFilePath(fileList.at(0)); //always just use the first file (assume it is the only one) found in the folder as the "correct" dll
		//the real dll name should be "microimaging.dll"
		
    	QPluginLoader* loader = new QPluginLoader(fullpath);
        if (!loader)
        {
        	v3d_msg(QString("ERROR in V3d_PluginLoader::searchPluginFiles the imaging module(%1)").arg(fullpath), 0);
        	return false;
        }
		
		v3d_msg(fullpath, 0);
		
		MainWindow *mainWin = curw->getMainControlWindow();
		mainWin->setCurHiddenSelectedWindow_withoutcheckwinlist(curw); // pass curw
		
		// pluginloader
		V3d_PluginLoader mypluginloader(mainWin);

		curw->setCustomStructPointer((void *)(&p)); //to pass parameters to the imaging plugin
		
		v3d_multithreadimageio_paras *p_customStruct = (v3d_multithreadimageio_paras *)curw->getCustomStructPointer();
		
		mypluginloader.runPlugin(loader, p.qOperation);
		//mypluginloader.runPlugin(loader, "About");
			
	}
	catch (...)
	{
		v3d_msg("Loading the image has a problem", 1);
		return false;
	}
	
	return true;
}



