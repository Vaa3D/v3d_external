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




/**************************************************************************
*
* V3D's plug-in loading support functions
*
* 2009-08-11, Zongcai Ruan
* 2009-08-14. Hanchuan Peng. allow the image size and type change
* 2010-08-01: Hanchuan Peng. add stronger check of the possible change of the data buffer when the data is set back
*
***************************************************************************
*/

#include "../basic_c_fun/v3d_interface.h"
#include "../3drenderer/v3dr_glwidget.h"
#include "../v3d/mainwindow.h"
#include "../v3d/v3d_core.h"
#include "v3d_plugin_loader.h"
#include "pluginDialog.h"


void pumpEvents(int loops=100)
{
	for (int j=1; j<loops; j++) //try to empty all existing events
	{
		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	}
}

//=======================================================
// Qt Pulgin ONLY supports 1 interface in 1 plugin module

QString     v3d_getInterfaceName(QObject *plugin)
{
   	QString name;

	V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
    if (iFilter )  return (name = "V3DSingleImageInterface/1.0");

    V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
    if (iface )  return (name = "V3DPluginInterface/1.1");

    V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
    if (iface2 )  return (name = "V3DPluginInterface/2.0");

    return name;
}

QStringList v3d_getInterfaceMenuList(QObject *plugin)
{
	QStringList qslist;

	V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
    if (iFilter )  return (qslist = iFilter->menulist());

    V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
    if (iface )  return (qslist = iface->menulist());

    V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
    if (iface2 )  return (qslist = iface2->menulist());

    return qslist;
}

QStringList v3d_getInterfaceFuncList(QObject *plugin)
{
	QStringList qslist;

    V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
    if (iface )  return (qslist = iface->funclist());

    V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
    if (iface2 )  return (qslist = iface2->funclist());

    return qslist;
}

//==============================================================

V3d_PluginLoader::V3d_PluginLoader(QMenu* menuPlugin, MainWindow* mainwindow)
{
	this->v3d_menuPlugin = menuPlugin;
	this->v3d_mainwindow = mainwindow;

	pluginList.clear();

	rescanPlugins();
}

V3d_PluginLoader::V3d_PluginLoader(MainWindow* mainwindow)
{
	this->v3d_menuPlugin = 0;
	this->v3d_mainwindow = mainwindow;
}


//QObject * QPluginLoader::instance () returns the root component object of the plugin.
//The plugin is loaded if necessary.
//If the root component object was destroyed, calling this function creates a new instance.
//The root component, returned by this function, is not deleted when the QPluginLoader is destroyed.
//If you want to ensure that the root component is deleted, you should call unload() as soon you don't need to access the core component anymore.
//When the library is finally unloaded, the root component will automatically be deleted.

void V3d_PluginLoader::clear()
{
	plugin_menu.clear();

	foreach (QPluginLoader* loader, pluginList)
    {
        //while (loader->isLoaded())
        	loader->unload();

        delete loader;
    }
	pluginList.clear();

	pluginFilenameList.clear();
}

//	foreach (QPluginLoader* loader, pluginList)
//    {
//        //100805 RZC: try to unload root instance of this plugin
//        while (loader->isLoaded())
//        {
//        	loader->unload();
//        }
////        QObjectList list = loader->staticInstances();
////        for ( ; list.size()>0; )
////        {
////        	loader->unload();
////        	list = loader->staticInstances();
////        }
//    }

void V3d_PluginLoader::rescanPlugins()
{
	clear();

	loadPlugins();

	populateMenus();
}

void V3d_PluginLoader::loadPlugins()
{
	QAction *plugin_manager = new QAction(tr("Plug-in manager"), this);
	connect(plugin_manager, SIGNAL(triggered()), this, SLOT(aboutPlugins()));
	QAction *plugin_rescan = new QAction(tr("Re-scan all plugins"), this);
	connect(plugin_rescan, SIGNAL(triggered()), this, SLOT(rescanPlugins()));
	{
		plugin_menu.addAction(plugin_manager);
		plugin_menu.addAction(plugin_rescan);
		plugin_menu.addSeparator();
	}

	pluginsDir = QDir(qApp->applicationDirPath());
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
    if (pluginsDir.cd("plugins")==false)
    {
    	qDebug("Cannot find ./plugins directory!");
    	return;;
    }

    qDebug("Searching in ./plugins ...... ");
	searchPluginDirs(&plugin_menu);
    searchPluginFiles(&plugin_menu);
    qDebug("Searching ./plugins done.");
}

void V3d_PluginLoader::searchPluginDirs(QMenu* menu)
{
	if (! menu)  return;

	QStringList dirList = pluginsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (QString dirName, dirList)
    {
		QMenu* submenu = new QMenu(dirName);
		if (! submenu)  return;
		menu->addMenu(submenu);

    	pluginsDir.cd(dirName);

    	searchPluginDirs(submenu);
        searchPluginFiles(submenu);

        pluginsDir.cdUp();
    }
}

void V3d_PluginLoader::searchPluginFiles(QMenu* menu)
{
	if (!menu)  return;

	QStringList fileList = pluginsDir.entryList(QDir::Files);
    foreach (QString fileName, fileList)
    {
    	QString fullpath = pluginsDir.absoluteFilePath(fileName);

    	QPluginLoader* loader = new QPluginLoader(fullpath);
        if (! loader)
        {
        	qDebug("ERROR in V3d_PluginLoader::searchPluginFiles: new QPluginLoader(%s)", qPrintable(fullpath));
        	return;
        }

        QObject *plugin = loader->instance(); //a new instance
        if (plugin)
        {
        	//qDebug()<< "plugin: " << fullpath;
            pluginFilenameList += fullpath;

            pluginList.append(loader); /////

            //--------------------------------------------------
        	QString iname = v3d_getInterfaceName(plugin);
            if (iname.size())
            {
            	//addToMenu(menu, plugin, v3d_getInterfaceFeatures(plugin), SLOT(runPlugin()));
            	addToMenu(menu, loader, v3d_getInterfaceMenuList(plugin), SLOT(runPlugin()));
            }
            //----------------------------------------------------
        }
        else
        {
        	qDebug() << "Fail instantiation: " <<fullpath;
        }

        //unload or left ? is a problem
     	//loader->unload();     //qDebug() << "unload: " <<fileName;
    }
}

void V3d_PluginLoader::addToMenu(QMenu *menu,
		QObject *plugin, const QStringList &texts, const char *member)
{
    foreach (QString text, texts)
    {
        QAction *action = new QAction(text, plugin);
        connect(action, SIGNAL(triggered()), this, member);
        menu->addAction(action);
    }
}

//hook menu to v3d, called by rescanPlugins, MainWindow::updateProcessingMenu
void V3d_PluginLoader::populateMenus()
{
	if (v3d_menuPlugin)
	{
		QAction* a = v3d_menuPlugin->menuAction();
		a->setMenu(&plugin_menu);
	}
}

//=========================================================

void V3d_PluginLoader::aboutPlugins()
{
    PluginDialog dialog("V3D", pluginsDir.path(), pluginFilenameList, v3d_mainwindow);
    dialog.exec();
}

void V3d_PluginLoader::runPlugin(QPluginLoader *loader, const QString & menuString)
{
    if (!loader)
    {
    	v3d_msg("ERROR in V3d_PluginLoader::runPlugin: invalid pointer to the plugin loader detected.");
    	return;
    }
	
    loader->unload(); ///
    QObject *plugin = loader->instance();
    if (!plugin)
    {
    	v3d_msg("ERROR in V3d_PluginLoader::runPlugin: loader->instance()");
    	return;
    }
	
    bool done = false;
    if (!done)  { done = runSingleImageInterface(plugin, menuString); v3d_msg("done with runSingleImageInterface().",0); }
	if (!done)  { done = runPluginInterface(plugin, menuString); v3d_msg("done with runPluginInterface().",0); }
	if (!done)  { done = runPluginInterface2(plugin, menuString); v3d_msg("done with runPluginInterface2().",0); }
	
	v3d_msg(QString("already run! done status=%1").arg(done), 0);
	// 100804 RZC: MUST do not unload plug-ins that has model-less dialog
	//    if (loader->isLoaded())
	//    {
	//    	loader->unload();      qDebug() << "unload: " <<fileName;
	//    }
}

void V3d_PluginLoader::runPlugin()
{
    QAction *action = qobject_cast<QAction *>(sender());
    //V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(action->parent());
    QPluginLoader *loader = qobject_cast<QPluginLoader *>(action->parent());
    if (!loader)
    {
    	v3d_msg("ERROR in V3d_PluginLoader::runPlugin: qobject_cast<QPluginLoader *>");
    	return;
    }
	
	return runPlugin(loader, action->text());
}

bool V3d_PluginLoader::runSingleImageInterface(QObject* plugin, const QString &command)
{
    V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
    bool done = (iFilter != 0);
    if (iFilter && v3d_mainwindow)
    {
        My4DImage* image = v3d_mainwindow->currentImage();
		if (!image)
		{
			v3d_msg("No image is open.");
			return done;
		}

		//make a copy of the property of the image input.
		V3DLONG szx0 = image->getXDim(), szy0 = image->getYDim(), szz0 = image->getZDim(), szc0 = image->getCDim();
		ImagePixelType datatype0 = image->getDatatype();
		unsigned char *dataptr0 = image->getRawData();

        try
        {
        	iFilter->processImage(command, (Image4DSimple*)image, (QWidget*)0); //do not pass the mainwindow widget
        }
        catch (...)
        {
        	v3d_msg(QString("The plugin [%1] fails to run. Check your plugin code please.").arg(command));
        }

		//check if any of the image properties changes; if yes, then also regenerate all 4D image pointers, etc
		V3DLONG szx_new = image->getXDim(), szy_new = image->getYDim(), szz_new = image->getZDim(), szc_new = image->getCDim();
		ImagePixelType datatype_new = image->getDatatype();
		unsigned char *dataptr_new = image->getRawData();
		if (dataptr0!=dataptr_new || szx0!=szx_new || szy0!=szy_new || szz0!=szz_new || szc0!=szc_new || datatype0!=datatype_new)
		{
			try
			{
				unsigned char * datanew = new unsigned char [image->getTotalBytes()];
				memcpy(datanew, image->getRawData(), image->getTotalBytes());

				//has to set the original size back first, otherwise the delete 4d pointer will not be correct
				image->setXDim(szx0);image->setYDim(szy0);image->setZDim(szz0);image->setCDim(szc0);image->setDatatype(datatype0);
				if (image->setNewImageData(datanew, szx_new, szy_new, szz_new, szc_new, datatype_new)==false)
				{
					v3d_msg("Fail to update the new image content returned by the plugin to the window.");
					return done;
				}
			}
			catch (...)
			{
				v3d_msg("Fail to allocate temporary memory of some other errors for handling the returned image contents of the plugin.");
				return done;
			}
		}
		else 
		{
			image->updateminmaxvalues(); //since the data have been changed, thus the min max values should be updated even the sizes remain the same
		}

		//v3d_msg(QString("after %1 %2 %3 %4").arg(image->getXDim()).arg(image->getYDim()).arg(image->getZDim()).arg(image->getCDim()));

        if (image)  image->updateViews();
        return done;
    }
    return done;
}

bool V3d_PluginLoader::runPluginInterface(QObject* plugin, const QString& command)
{
    V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
	V3DPluginCallback *callback = dynamic_cast<V3DPluginCallback *>(this);
	if (iface && callback)
    {
        try
        {
        	iface->domenu(command, *callback, (QWidget*)0); //do not pass the mainwindow widget
        }
        catch (...)
        {
        	v3d_msg(QString("The plugin fails to run [%1] . Check your plugin code please.").arg(command));
        }
        return true;
    }
	return false;
}

bool V3d_PluginLoader::runPluginInterface2(QObject* plugin, const QString& command)
{
    V3DPluginInterface2 *iface = qobject_cast<V3DPluginInterface2 *>(plugin);
	V3DPluginCallback2 *callback = dynamic_cast<V3DPluginCallback2 *>(this);
	if (iface && callback)
    {
        try
        {
        	iface->domenu(command, *callback, (QWidget*)0); //do not pass the mainwindow widget
        }
        catch (...)
        {
        	v3d_msg(QString("The plugin fails to run [%1]. Check your plugin code please.").arg(command));
        }
        return true;
    }
	return false;
}

//====================================================================

bool V3d_PluginLoader::callPluginFunc(const QString &plugin_name,
		const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output)
{
	QString fullpath = pluginsDir.absoluteFilePath(plugin_name);
	qDebug()<<"callPluginFunc fullpath: " <<fullpath;
	int idx = pluginFilenameList.indexOf(fullpath);
	//qDebug()<<"callPluginFunc idx: " <<idx;
	if (idx < 0)
	{
		qDebug()<<QString("ERROR: callPluginFunc cannot find this plugin_name: '%1'").arg(plugin_name);
		return false;
	}

	Q_ASSERT(idx>=0 && idx<pluginList.size());
	QPluginLoader *loader = pluginList.at(idx);

	loader->unload(); ///
    QObject *plugin = loader->instance();
    if (! plugin)
    {
    	qDebug("ERROR in V3d_PluginLoader::callPluginFunc: loader->instance()");
    	return false;
    }

//	QStringList funclist = v3d_getInterfaceFuncList(plugin);
//	if (! funclist.contains(func_name))
//	{
//		qDebug()<<Qstring("ERROR: callPluginFunc cannot find this func_name: '%1' in '%2'").arg(func_name).arg(plugin_name);
//		return false;
//	}

	V3DPluginInterface2 *iface = qobject_cast<V3DPluginInterface2 *>(plugin);
	V3DPluginCallback2 *callback = dynamic_cast<V3DPluginCallback2 *>(this);
	if (! (iface && callback) )
	{
		qDebug()<<QString("ERROR: callPluginFunc cannot cast (V3DPluginInterface2) of plugin '%1'").arg(plugin_name);
		return false;
	}

    try
    {
    	return iface->dofunc(func_name, input, output, *callback, (QWidget*)0);
    }
    catch (...)
    {
    	v3d_msg(QString("The plugin fails to call [%1]. Check your plugin code please.").arg(func_name));
		return false;
    }
	return true;
}

//==================================================================


#define __get_or_push_with_v3d__
v3dhandleList V3d_PluginLoader::getImageWindowList() const
{
	v3dhandleList list;
	if (v3d_mainwindow)
	{
		list = v3d_mainwindow->allWindowList();
	}
	return list;
}
v3dhandle V3d_PluginLoader::currentImageWindow()
{
	XFormWidget* w = 0;
	if (v3d_mainwindow)
	{
		w = v3d_mainwindow->currentImageWindow();
	}
	return v3dhandle(w);
}
v3dhandle V3d_PluginLoader::curHiddenSelectedWindow()
{
	XFormWidget* w = 0;
	if (v3d_mainwindow)
	{
		w = v3d_mainwindow->curHiddenSelectedWindow();
	}
	return v3dhandle(w);
}
v3dhandle V3d_PluginLoader::newImageWindow(QString name)
{
	XFormWidget* w = 0;
	if (v3d_mainwindow)
	{
		w = v3d_mainwindow->newImageWindow(name);
		qDebug() << "V3d_PluginLoader newImageWindow: " << w;
	}
	return v3dhandle(w);
}
void V3d_PluginLoader::updateImageWindow(v3dhandle image_window)
{
	if (v3d_mainwindow)
	{
		XFormWidget* w = v3d_mainwindow->updateImageWindow(image_window);
		qDebug() << "V3d_PluginLoader updateImageWindow: " << w <<"/"<< image_window;
	}

	pumpEvents(); //100804 RZC
}


QString V3d_PluginLoader::getImageName(v3dhandle image_window) const
{
	QString name;
	if (v3d_mainwindow)
	{
		name = v3d_mainwindow->getWindowName(image_window);
	}
	return name;
}
void V3d_PluginLoader::setImageName(v3dhandle image_window, QString name)
{
	if (v3d_mainwindow)
	{
		XFormWidget* w = v3d_mainwindow->setImageName(image_window, name);
		qDebug() << "V3d_PluginLoader setImageName: " << w <<"/"<< image_window;
	}
}


Image4DSimple* V3d_PluginLoader::getImage(v3dhandle image_window)
{
	My4DImage* image = 0;
	if (v3d_mainwindow)
	{
		image = v3d_mainwindow->getImage(image_window);
	}
	return (Image4DSimple*)image;
}
bool V3d_PluginLoader::setImage(v3dhandle image_window, Image4DSimple* image)
{
	if (!image)
	{
		v3d_msg("The new image pointer is invalid in V3d_PluginLoader::setImage(). Do nothing.\n",0);
		return false;
	}
	if (v3d_mainwindow)
	{
		qDebug() << "V3d_PluginLoader setImage: " << image_window << image;
		Image4DSimple *targetP = v3d_mainwindow->getImage(image_window);
		if (!targetP) //in this case, the pointers cannot be the same, and thus directly set data
    {
			return v3d_mainwindow->setImage(image_window, image);
    }
		else
		{
			if (targetP==image ||     // in this case no need for further judgment as the data MUST be the same
	                    image->isSameDataBuffer(targetP) )
			{
				v3d_msg("You try to set the same image pointer to itself; thus nothing is needed. \n",0);
				return true;
			}  //in this case the data buffer would be updated directly, and thus no need to update the actual data
                           //note that a possible bug is that if the plugin developer change the dimensions/datatype of the data
                           //buffer, but still keep the buffer pointer unchanged. Then here we should recreate the 4d pointers.
                           // This is a to-do in the near future. by PHC. 2010-08-01.

			return v3d_mainwindow->setImage(image_window, image);
		}
	}
	return false;
}

bool V3d_PluginLoader::setImageTest(v3dhandle image_window, Image4DSimple* image, unsigned char *a)
{
	if (v3d_mainwindow)
	{
		qDebug() << "V3d_PluginLoader setImage: " << image_window << image;

		return v3d_mainwindow->setImage(image_window, image);
	}
	return false;
}


LandmarkList  V3d_PluginLoader::getLandmark(v3dhandle image_window)
{
	if (v3d_mainwindow)
	{
		return v3d_mainwindow->getLandmark(image_window);
	}
	return LandmarkList();
}
bool V3d_PluginLoader::setLandmark(v3dhandle image_window, LandmarkList& landmark_list)
{
	if (v3d_mainwindow)
	{
		return v3d_mainwindow->setLandmark(image_window, landmark_list);
	}
	return false;
}


ROIList V3d_PluginLoader::getROI(v3dhandle image_window)
{
	if (v3d_mainwindow)
	{
		return v3d_mainwindow->getROI(image_window);
	}
	return ROIList();
}
bool V3d_PluginLoader::setROI(v3dhandle image_window, ROIList & roi_list)
{
	if (v3d_mainwindow)
	{
		return v3d_mainwindow->setROI(image_window, roi_list);
	}
	return false;
}

NeuronTree V3d_PluginLoader::getSWC(v3dhandle image_window)
{
	if (v3d_mainwindow)
	{
		return v3d_mainwindow->getSWC(image_window);
	}
	return NeuronTree();
}
bool V3d_PluginLoader::setSWC(v3dhandle image_window, NeuronTree & nt)
{
	if (v3d_mainwindow)
	{
		return v3d_mainwindow->setSWC(image_window, nt);
	}
	return false;
}


V3D_GlobalSetting V3d_PluginLoader::getGlobalSetting()
{
	if (v3d_mainwindow)
	{
		return v3d_mainwindow->getGlobalSetting();
	}
	return V3D_GlobalSetting();
}
bool V3d_PluginLoader::setGlobalSetting( V3D_GlobalSetting & gs )
{
	if (v3d_mainwindow)
	{
		return v3d_mainwindow->setGlobalSetting(gs);
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////
//100810 RZC: add this macro
#define if_XFormWidget(w, image_window) \
	XFormWidget* w=0; \
	if (v3d_mainwindow && (w = v3d_mainwindow->validateImageWindow(image_window)))


void V3d_PluginLoader::open3DWindow(v3dhandle image_window)
{
	if_XFormWidget(w, image_window)
	{
		w->open3DWindow();
		qDebug() << "V3d_PluginLoader open3DWindow: " << w <<"/"<< image_window;
	}
}
void V3d_PluginLoader::close3DWindow(v3dhandle image_window)
{
	if_XFormWidget(w, image_window)
	{
		w->close3DWindow();
		qDebug() << "V3d_PluginLoader close3DWindow: " << w <<"/"<< image_window;

		pumpEvents(); //100804 RZC
	}
}
void V3d_PluginLoader::openROI3DWindow(v3dhandle image_window)
{
	if_XFormWidget(w, image_window)
	{
		w->openROI3DWindow();
		qDebug() << "V3d_PluginLoader openROI3DWindow: " << w <<"/"<< image_window;
	}
}
void V3d_PluginLoader::closeROI3DWindow(v3dhandle image_window)
{
	if_XFormWidget(w, image_window)
	{
		w->closeROI3DWindow();
		qDebug() << "V3d_PluginLoader closeROI3DWindow: " << w <<"/"<< image_window;

		pumpEvents(); //100804 RZC
	}
}

void V3d_PluginLoader::pushObjectIn3DWindow(v3dhandle image_window)
{
	if_XFormWidget(w, image_window)
	{
		w->pushObjectIn3DWindow();
		qDebug() << "V3d_PluginLoader pushObjectIn3DWindow: " << w <<"/"<< image_window;
	}
}

void V3d_PluginLoader::pushImageIn3DWindow(v3dhandle image_window)
{
	if_XFormWidget(w, image_window)
	{
		w->pushImageIn3DWindow();
		qDebug() << "V3d_PluginLoader pushImageIn3DWindow: " << w <<"/"<< image_window;
	}
}

int V3d_PluginLoader::pushTimepointIn3DWindow(v3dhandle image_window, int timepoint)
{
	int t=0;
	if_XFormWidget(w, image_window)
	{
		t = w->pushTimepointIn3DWindow(timepoint);
		qDebug() << "V3d_PluginLoader pushTimepointIn3DWindow: " << t <<"/"<< timepoint;
	}
	return t;
}

bool V3d_PluginLoader::screenShot3DWindow(v3dhandle image_window, QString filename)
{
	bool r =false;
	if_XFormWidget(w, image_window)
	{
		r = w->screenShot3DWindow(filename);
		qDebug() << "V3d_PluginLoader screenShotIn3DWindow: " << r <<"/"<< filename;
	}

	return r;
}
bool V3d_PluginLoader::screenShotROI3DWindow(v3dhandle image_window, QString filename)
{
	bool r =false;
	if_XFormWidget(w, image_window)
	{
		r = w->screenShotROI3DWindow(filename);
		qDebug() << "V3d_PluginLoader screenShotROI3DWindow: " << r <<"/"<< filename;
	}

	return r;
}


//V3DPluginCallback2

View3DControl * V3d_PluginLoader::getView3DControl(v3dhandle image_window)
{
	View3DControl * vi = 0;
	if_XFormWidget(w, image_window)
	{
		//vi = w->getView3DControl();
		vi = dynamic_cast<View3DControl *>(w->getView3D());
		qDebug() << "V3d_PluginLoader getView3DControl = "<< vi <<" : " << w <<"/"<< image_window;
	}
	return vi;
}
View3DControl * V3d_PluginLoader::getLocalView3DControl(v3dhandle image_window)
{
	View3DControl * vi = 0;
	if_XFormWidget(w, image_window)
	{
		//vi = w->getLocalView3DControl();
		vi = dynamic_cast<View3DControl *>(w->getLocalView3D());
		qDebug() << "V3d_PluginLoader getLocalView3DControl= "<< vi <<" : " << w <<"/"<< image_window;
	}
	return vi;
}

