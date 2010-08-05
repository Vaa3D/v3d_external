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
#include "v3d_plugin_loader.h"
#include "pluginDialog.h"
#include "../v3d/mainwindow.h"
#include "../v3d/v3d_core.h"

void pumpEvents()
{
	for (int j=1; j<1000; j++) //try to empty all existing events
	{
		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	}
}

//=======================================================

QString     v3d_getInterfaceName(QObject *plugin)
{
   	QString name;

	V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
    if (iFilter )  return (name = "V3DSingleImageInterface/1.0");

    V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
    if (iface )  return (name = "V3DPluginInterface/1.1");

    return name;
}

QStringList v3d_getInterfaceFeatures(QObject *plugin)
{
	QStringList qslist;

	V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
    if (iFilter )  return (qslist = iFilter->menulist());

    V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
    if (iface )  return (qslist = iface->menulist());

    return qslist;
}

//==============================================================

V3d_PluginLoader::V3d_PluginLoader(QMenu* menuPlugin, MainWindow* mainwindow)
{
	this->v3d_menuPlugin = menuPlugin;
	this->v3d_mainwindow = mainwindow;

	plugin_menu.clear();

	rescanPlugins();
}

void V3d_PluginLoader::clear()
{
	plugin_menu.clear();

	foreach (QPluginLoader* loader, pluginList)
    {
		//100805 RZC: this MUST don't called because it may cause model-less plugin to crash.
//        if (loader->isLoaded())
//        {
//        	loader->unload();
//        }
		delete loader;
    }
	pluginList.clear();

	pluginFilenameList.clear();
}

void V3d_PluginLoader::rescanPlugins()
{
	clear();

	loadPlugins();

	populateMenus();

	foreach (QPluginLoader* loader, pluginList)
    {
        //100805 RZC: try to unload root instance of this plugin
        while (loader->isLoaded())
        {
        	loader->unload();
        }
//        QObjectList list = loader->staticInstances();
//        for ( ; list.size()>0; )
//        {
//        	loader->unload();
//        	list = loader->staticInstances();
//        }
    }
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
        if (! loader) return;

        QObject *plugin = loader->instance();
        if (plugin)
        {
        	qDebug()<< "plugin: " << fullpath;
            pluginFilenameList += fullpath;

            pluginList.append(loader); /////

            //--------------------------------------------------
        	QString iname = v3d_getInterfaceName(plugin);
            if (iname.size())
            {
            	//addToMenu(menu, plugin, v3d_getInterfaceFeatures(plugin), SLOT(runPlugin()));
            	addToMenu(menu, loader, v3d_getInterfaceFeatures(plugin), SLOT(runPlugin()));
            }
            //----------------------------------------------------
        }
        else
        {
        	qDebug() << "fail instantiation: " <<fullpath;
        }

        if (loader->isLoaded())
        {
        	loader->unload();     //qDebug() << "unload: " <<fileName;
        }
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

void V3d_PluginLoader::runPlugin()
{
    QAction *action = qobject_cast<QAction *>(sender());
    //V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(action->parent());
    QPluginLoader *loader = qobject_cast<QPluginLoader *>(action->parent());

    QObject *plugin = loader->instance();

    runSingleImageInterface(plugin, action->text());

    runPluginInterface(plugin, action->text());

// 100804 RZC: MUST do not unload plug-ins that has model-less dialog
//    if (loader->isLoaded())
//    {
//    	loader->unload();      qDebug() << "unload: " <<fileName;
//    }
}

void V3d_PluginLoader::runSingleImageInterface(QObject* plugin, const QString &command)
{
    V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
    if (iFilter && v3d_mainwindow)
    {
        My4DImage* image = v3d_mainwindow->currentImage();

		if (!image)
		{
			v3d_msg("No image is open.");
			return;
		}

		//make a copy of the property of the image input.
		V3DLONG szx0 = image->getXDim(), szy0 = image->getYDim(), szz0 = image->getZDim(), szc0 = image->getCDim();
		ImagePixelType datatype0 = image->getDatatype();
		unsigned char *dataptr0 = image->getRawData();

        try
        {
        	//iFilter->processImage(command, (Image4DSimple*)image, (QWidget*)v3d_mainwindow);
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
//				image->cleanExistData_only4Dpointers();
//				image->setupData4D();

				unsigned char * datanew = new unsigned char [image->getTotalBytes()];
				memcpy(datanew, image->getRawData(), image->getTotalBytes());

				//has to set the original size back first, otherwise the delete 4d pointer will not be correct
				image->setXDim(szx0);image->setYDim(szy0);image->setZDim(szz0);image->setCDim(szc0);image->setDatatype(datatype0);
				if (image->setNewImageData(datanew, szx_new, szy_new, szz_new, szc_new, datatype_new)==false)
				{
					v3d_msg("Fail to update the new image content returned by the plugin to the window.");
					return;
				}

			}
			catch (...)
			{
				v3d_msg("Fail to allocate temporary memory of some other errors for handling the returned image contents of the plugin.");
				return;
			}
		}
		else //
		{
			image->updateminmaxvalues(); //since the data have been changed, thus the min max values should be updated even the sizes remain the same
		}

		//v3d_msg(QString("after %1 %2 %3 %4").arg(image->getXDim()).arg(image->getYDim()).arg(image->getZDim()).arg(image->getCDim()));

        if (image)  image->updateViews();
    }
}

void V3d_PluginLoader::runPluginInterface(QObject* plugin, const QString& command)
{
    V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
	V3DPluginCallback *callback = dynamic_cast<V3DPluginCallback *>(this);

	if (iface && callback)
    {
        try
        {
        	//iFilter->domenu(command, *callback, (QWidget*)v3d_mainwindow);
        	iface->domenu(command, *callback, (QWidget*)0); //do not pass the mainwindow widget
        }
        catch (...)
        {
        	v3d_msg(QString("The plugin [%1] fails to run. Check your plugin code please.").arg(command));
        }

    }
}

//====================================================================

bool V3d_PluginLoader::callPluginFunc(const QString &plugin_name,
		const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output)
{
	Q_UNUSED(plugin_name); Q_UNUSED(func_name); Q_UNUSED(input); Q_UNUSED(output);
	return false;
}

//==================================================================

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
			return v3d_mainwindow->setImage(image_window, image);
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

void V3d_PluginLoader::open3DWindow(v3dhandle image_window)
{
	if (v3d_mainwindow)
	{
		XFormWidget* w = v3d_mainwindow->open3DWindow(image_window);
		qDebug() << "V3d_PluginLoader open3DWindow: " << w <<"/"<< image_window;
	}
}
void V3d_PluginLoader::close3DWindow(v3dhandle image_window)
{
	if (v3d_mainwindow)
	{
		XFormWidget* w = v3d_mainwindow->close3DWindow(image_window);
		qDebug() << "V3d_PluginLoader close3DWindow: " << w <<"/"<< image_window;

		pumpEvents(); //100804 RZC
	}
}
void V3d_PluginLoader::openROI3DWindow(v3dhandle image_window)
{
	if (v3d_mainwindow)
	{
		XFormWidget* w = v3d_mainwindow->openROI3DWindow(image_window);
		qDebug() << "V3d_PluginLoader openROI3DWindow: " << w <<"/"<< image_window;
	}
}
void V3d_PluginLoader::closeROI3DWindow(v3dhandle image_window)
{
	if (v3d_mainwindow)
	{
		XFormWidget* w = v3d_mainwindow->closeROI3DWindow(image_window);
		qDebug() << "V3d_PluginLoader closeROI3DWindow: " << w <<"/"<< image_window;

		pumpEvents(); //100804 RZC
	}
}

void V3d_PluginLoader::pushObjectIn3DWindow(v3dhandle image_window)
{
	if (v3d_mainwindow)
	{
		XFormWidget* w = v3d_mainwindow->pushObjectIn3DWindow(image_window);
		qDebug() << "V3d_PluginLoader pushObjectIn3DWindow: " << w <<"/"<< image_window;
	}
}

void V3d_PluginLoader::pushImageIn3DWindow(v3dhandle image_window)
{
	if (v3d_mainwindow)
	{
		XFormWidget* w = v3d_mainwindow->pushImageIn3DWindow(image_window);
		qDebug() << "V3d_PluginLoader pushImageIn3DWindow: " << w <<"/"<< image_window;
	}
}

int V3d_PluginLoader::pushTimepointIn3DWindow(v3dhandle image_window, int timepoint)
{
	int t=0;
	if (v3d_mainwindow)
	{
		t = v3d_mainwindow->pushTimepointIn3DWindow(image_window, timepoint);
		qDebug() << "V3d_PluginLoader pushTimepointIn3DWindow: " << t <<"/"<< timepoint;
	}
	return t;
}

bool V3d_PluginLoader::screenShot3DWindow(v3dhandle image_window, QString filename)
{
	bool r =false;
	XFormWidget* w=0;
	if (v3d_mainwindow && (w = v3d_mainwindow->validateImageWindow(image_window)))
	{
		r = w->screenShot3DWindow(filename);
		qDebug() << "V3d_PluginLoader screenShotIn3DWindow: " << r <<"/"<< filename;
	}
}
bool V3d_PluginLoader::screenShotROI3DWindow(v3dhandle image_window, QString filename)
{
	bool r =false;
	XFormWidget* w=0;
	if (v3d_mainwindow && (w = v3d_mainwindow->validateImageWindow(image_window)))
	{
		r = w->screenShotROI3DWindow(filename);
		qDebug() << "V3d_PluginLoader screenShotROI3DWindow: " << r <<"/"<< filename;
	}
}

