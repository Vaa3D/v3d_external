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
#include "../v3d/mainwindow.h"
#include "../terafly/src/control/CPlugin.h"
//#include "../3drenderer/v3dr_mainwindow.h"
//#include "../v3d/v3d_core.h"
#ifdef __WIN32 
// #include "../sub_projects/imaging_piezo/microimaging.h"
#endif
bool v3d_imaging(MainWindow* mainwindow, const v3d_imaging_paras & p)
{
    v3d_msg(QString("Now try to do imaging or other plugin functions [%1]").arg(p.OPS), 0);

	try 
	{
		const char* filename=p.imgp->getFileName();
        //v3d_msg(QString("[")+filename+"]");
        XFormWidget *curw = 0;
        if (filename)
        {
            curw = mainwindow->findMdiChild(QString(filename)); //search window using name. ZJL
            //unsigned char* rawdata=p.imgp->getRawData();
            //QList <V3dR_MainWindow *> winlist=mainwindow->list_3Dview_win;
            //V3dR_MainWindow *curwin;
            //for(int i=0;i<winlist.size();i++)
            //{
            //	unsigned char* winrawdata=winlist.at(i)->getGLWidget()->getiDrawExternalParameter()->image4d->getRawData();
            //	if(rawdata==winrawdata)
            //	{
            //		curwin=winlist.at(i);
            //		continue;
            //	}
            //}
            if (!curw)
            {
                v3d_msg(QString("No window open yet (error detected in v3d_imaging() main entry point [filename={%1}]).").arg(filename));
                return false;
            }
        }
        else
        {
            if (p.OPS != "Load file using Vaa3D data IO manager") //only allow data IO manager to pass an empty file name parameter here
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
        if (p.OPS == "Vaa3D-Neuron2-APP2")
        {
            if (pluginsDir.cd("plugins/neuron_tracing/Vaa3D_Neuron2")==false)
            {
                v3d_msg("Cannot find ./plugins/neuron_tracing/Vaa3D_Neuron2 directory!");
                return false;
            }            
        }
        else if (p.OPS == "GD Curveline" || p.OPS == "GD Curveline infinite") //PHC 20170530
        {
            if (pluginsDir.cd("plugins/line_detector")==false)
            {
                v3d_msg("Cannot find ./plugins/line_detector directory!",0);
                return false;
            }
        }
        else if (p.OPS == "load_new_stack") //ZZ, 02012018
        {
            if (pluginsDir.cd("plugins/neuron_utilities/assemble_neuron_live")==false)
            {
                v3d_msg("Cannot find ./plugins/neuron_utilities/assemble_neuron_live!",0);
                return false;
            }
        }

        else if (p.OPS == "actRetrace" || p.OPS == "*app2Convenient " || p.OPS == "app2Terafly" || p.OPS == "app2MultiTerafly" || p.OPS == "app2TeraflyWithPara" || p.OPS == "app2MultiTeraflyWithPara") //ZZ, 02012018
        {
            if (pluginsDir.cd("plugins/Retrace")==false)
            {
                v3d_msg("Cannot find ./plugins/Retrace",0);
                return false;
            }
        }
        else if (p.OPS == "trace_APP2") //ZZ, 05142018
        {
            if (pluginsDir.cd("plugins/neuron_tracing/segment_maker")==false)
            {
                v3d_msg("Cannot find ./plugins/neuron_tracing/segment_maker!",0);
                return false;
            }
        }
        else if (p.OPS == "checked_to_mark") //OY, 26102018
        {
            if (pluginsDir.cd("plugins/neuron_tracing/checked_mark")==false)
            {
                v3d_msg("Cannot find ./plugins/neuron_tracing/checked_mark!",0);
                return false;
            }
        }
        else if (p.OPS == "checked_return") //OY, 26102018
        {
            if (pluginsDir.cd("plugins/neuron_tracing/checked_mark")==false)
            {
                v3d_msg("Cannot find ./plugins/neuron_tracing/checked_mark!",0);
                return false;
            }
        }
        else if (p.OPS == "Fetch Highrez Image Data from File")
        {
            // @FIXED by Alessandro on 2015-09-30.
            // Since TeraFly is part of Vaa3D, here we can directly call TeraFly's domenu function w/o using QPluginLoader.
            curw->getImageData()->setCustomStructPointer((void *)(&p)); //to pass parameters to the imaging plugin
            tf::TeraFly::doaction(p.OPS);
            return true;
        }
        else if (p.OPS == "Load file using Vaa3D data IO manager")
        {
            if (pluginsDir.cd("plugins/data_IO/data_IO_manager")==false)
            {
                v3d_msg("Cannot find ./plugins/data_IO/data_IO_manager directory!");
                return false;
            }
        }
        else
        {
            if (pluginsDir.cd("plugins/smartscope_controller")==false) 
            {
                v3d_msg("Cannot find ./plugins/smartscope_controller directory!");
                return false;
            }
        }
		
		QStringList fileList = pluginsDir.entryList(QDir::Files);
		if (fileList.size()<1)
		{
			v3d_msg("Cannot find any file in the ./plugins/smartscope_controller directory!");
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

        if (curw)
        {
            //mypluginloader.runPlugin(loader, QString("about this plugin"));
            curw->getImageData()->setCustomStructPointer((void *)(&p)); //to pass parameters to the imaging plugin

            QTime t;
           // t.start();
            mypluginloader.runPlugin(loader, p.OPS);
            //mypluginloader.runPlugin(loader, "about");
           // v3d_msg(QString("** invoke time for the plugin is [%1] ms.").arg(t.elapsed()*1000), 0);
        }
        else //for data IO manager in which case curw is NULL
        {
            QTime t;
           // t.start();
            mypluginloader.runPlugin(loader, p.datafilename); //the data IO manager is specially designed to pass the datafilename parameter in this way. 2013-12-02. by PHC. what an ugly design here!
            //v3d_msg(QString("** invoke time for the plugin is [%1] ms.").arg(t.elapsed()*1000), 0);
        }
	}
	catch (...)
	{
		v3d_msg("Catch a problem in v3d_imaging() wrapper function.", 1);
		return false;
	}
	
	return true;
}
