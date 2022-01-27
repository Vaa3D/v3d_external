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

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) ‚ÄúV3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,‚Äù Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) ‚ÄúAutomatic reconstruction of 3D neuron structures using a graph-augmented deformable model,‚Äù Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

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

#include "../3drenderer/v3dr_glwidget.h"
#include "../basic_c_fun/v3d_interface.h"
#include "../3drenderer/renderer_gl1.h"
#include "../3drenderer/v3dr_mainwindow.h"
#include "../v3d/mainwindow.h"
#include "../v3d/v3d_core.h"
#include "v3d_plugin_loader.h"
#include "pluginDialog.h"
#include "../terafly/src/control/CPlugin.h"
#include <QtGlobal>


void pumpEvents(int loops=100)
{
	for (int j=1; j<loops; j++) //try to empty all existing events
	{
		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	}
}

//=======================================================
// Qt Plugin ONLY supports 1 interface in 1 plugin module

QString     v3d_getInterfaceName(QObject *plugin)
{
   	QString name;

    // Derived class must appear first, to be selected
	V3DSingleImageInterface2_1 *iFilter2_1 = qobject_cast<V3DSingleImageInterface2_1 *>(plugin);
    if (iFilter2_1 )  return (name = "V3DSingleImageInterface/2.1");

    // Base class must appear later, so derived class has a chance
	V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
    if (iFilter )  return (name = "V3DSingleImageInterface/1.0");

    V3DPluginInterface2_1 *iface2_1 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
    if (iface2_1 )  return (name = "V3DPluginInterface/2.1");

    V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
    if (iface2 )  return (name = "V3DPluginInterface/2.0");

    V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
    if (iface )  return (name = "V3DPluginInterface/1.1");

    return name;
}

QStringList v3d_getInterfaceMenuList(QObject *plugin)
{
	QStringList qslist;

	V3DSingleImageInterface2_1 *iFilter2_1 = qobject_cast<V3DSingleImageInterface2_1 *>(plugin);
    if (iFilter2_1 )  return (qslist = iFilter2_1->menulist());

	V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
    if (iFilter )  return (qslist = iFilter->menulist());

    V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
    if (iface )  return (qslist = iface->menulist());

    V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
    if (iface2 )  return (qslist = iface2->menulist());

    V3DPluginInterface2_1 *iface2_1 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
    if (iface2_1 )  return (qslist = iface2_1->menulist());

    return qslist;
}

QStringList v3d_getInterfaceFuncList(QObject *plugin)
{
	QStringList qslist;

    V3DPluginInterface *iface = qobject_cast<V3DPluginInterface *>(plugin);
    if (iface )  return (qslist = iface->funclist());

    V3DPluginInterface2 *iface2 = qobject_cast<V3DPluginInterface2 *>(plugin);
    if (iface2 )  return (qslist = iface2->funclist());

    V3DPluginInterface2_1 *iface2_1 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
    if (iface2_1 )  return (qslist = iface2_1->funclist());

    return qslist;
}

//==============================================================

//void Vaa3DPluginMenu::mousePressEvent ( QMouseEvent * e )
//{
//    v3d_msg("enter mouse press");
//    if (ploader)
//        ploader->rescanPlugins();
//    QMenu::mousePressEvent(e);
//};

//===============================================================

V3d_PluginLoader::V3d_PluginLoader(QMenu* menuPlugin, MainWindow* mainwindow)
{
	this->v3d_menuPlugin = menuPlugin;
	this->v3d_mainwindow = mainwindow;

	pluginList.clear();

	rescanPlugins();
}

V3d_PluginLoader::V3d_PluginLoader(MainWindow* mainwindow)
{
    if (mainwindow) //20130904
    {
        this->v3d_mainwindow = mainwindow;
        this->v3d_menuPlugin = mainwindow->pluginProcMenu; //changed from 0, 20130904, PHC
    }
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

// Return a list of directories that will be searched for plugins
QList<QDir> V3d_PluginLoader::getPluginsDirList()
{
    QList<QDir> pluginsDirList;

    pluginsDirList.clear();
	QDir testPluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
    if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
        testPluginsDir.cdUp();

	qDebug() << testPluginsDir.absolutePath();
#elif defined(Q_OS_MAC)
    // In a Mac app bundle, plugins directory could be either
    //  a - below the actual executable i.e. v3d.app/Contents/MacOS/plugins/
    //  b - parallel to v3d.app i.e. foo/v3d.app and foo/plugins/
    if (testPluginsDir.dirName() == "MacOS") {
        QDir testUpperPluginsDir = testPluginsDir;
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
        if (testUpperPluginsDir.cd("plugins"))
            pluginsDirList.append(testUpperPluginsDir);
    }
#endif
    if (testPluginsDir.cd("plugins"))
        pluginsDirList.append(testPluginsDir);

    return pluginsDirList;
}

void V3d_PluginLoader::loadPlugins()
{
	QAction *plugin_manager = new QAction(tr("Plug-in manager"), this);
	connect(plugin_manager, SIGNAL(triggered()), this, SLOT(aboutPlugins()));
    QAction *plugin_rescan = new QAction(tr("Re-scan all plugins"), this);
	connect(plugin_rescan, SIGNAL(triggered()), this, SLOT(rescanPlugins()));
    QAction * plugin_clear = new QAction(tr("Clear used plugins history"),this);
    connect(plugin_clear, SIGNAL(triggered()), this, SLOT(clear_recentPlugins()));
	{
		plugin_menu.addAction(plugin_manager);
		plugin_menu.addAction(plugin_rescan);

        addrecentPlugins(&plugin_menu); //added by Zhi Z 20140721
        updated_recentPlugins();

        plugin_menu.addAction(plugin_clear);
		plugin_menu.addSeparator();
	}

    QList<QDir> pluginsDirList = getPluginsDirList();

    if (pluginsDirList.size() == 0)
    {
    	qDebug("Cannot find ./plugins directory!");
        return;
    }

    qDebug("Searching in ./plugins ...... ");
    foreach (const QDir& pluginsDir, pluginsDirList)
    {
    	searchPluginDirs(&plugin_menu, pluginsDir);
        searchPluginFiles(&plugin_menu, pluginsDir);
        qDebug("Searching ./plugins done.");
    }
}

//added by Zhi Z 20140721
void V3d_PluginLoader::addrecentPlugins(QMenu* menu)
{
    if (! menu)  return;
    QSettings settings("HHMI", "Vaa3D");

    recentpluginsList = settings.value("recentPluginList").toStringList();
    recentpluginsIndex = settings.value("recentPluginIndex").toList();

    plugin_recent = new QMenu(tr("Recently used plugins"));
    plugin_most = new QMenu(tr("Most used plugins"));
    menu->addMenu(plugin_recent);
    menu->addMenu(plugin_most);

}
void V3d_PluginLoader::updated_recentPlugins()
{
    plugin_recent->clear();
    plugin_most->clear();

    if(recentpluginsIndex.size()>0)
    {
        QList<QVariant> recentpluginsIndex_temp = recentpluginsIndex;
        QList <int> sort_index;

        for(V3DLONG i = 0; i < recentpluginsList.size(); i++)
            sort_index.append(i);

        if(recentpluginsIndex.size()>1)
        {
            for(V3DLONG i = 0; i < recentpluginsList.size(); i++)
            {
                if (i > 0)
                {
                    V3DLONG j = i;
                    while(j > 0 && recentpluginsIndex_temp.at(j-1).toInt()<recentpluginsIndex_temp.at(j).toInt())
                    {
                        recentpluginsIndex_temp.swap(j,j-1);
                        sort_index.swap(j,j-1);
                        j--;
                    }
                }
            }
        }

        QRegExp reg("%");
        for(int i = 0; i < recentpluginsList.size(); i++)
        {
            QStringList plugininfo = recentpluginsList.at(i).split(reg);
            QAction *action = new QAction(plugininfo.at(0),this);
            connect(action, SIGNAL(triggered()), this, SLOT(runRecentPlugin()));
            plugin_recent->addAction(action);

            QStringList plugininfo_index = recentpluginsList.at(sort_index[i]).split(reg);
            QAction *action_index = new QAction(plugininfo_index.at(0),this);
            connect(action_index, SIGNAL(triggered()), this, SLOT(runRecentPlugin()));
            plugin_most->addAction(action_index);
        }
    }
}

void V3d_PluginLoader::runRecentPlugin()
{
    QAction *action = qobject_cast<QAction *>(sender());
    int i;
    QRegExp reg("%");
    QStringList plugininfo;
    for(i = 0; i< recentpluginsList.size(); i++)
    {
        plugininfo = recentpluginsList.at(i).split(reg);
        if(plugininfo.at(0) == action->text())
            break;
    }
    QPluginLoader* loader = new QPluginLoader(plugininfo.at(1));
    return runPlugin(loader,plugininfo.at(0));
}

void V3d_PluginLoader::clear_recentPlugins()
{
    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to clear the history of used plugins?"), QMessageBox::Yes, QMessageBox::No))
    {

        QSettings settings("HHMI", "Vaa3D");
        settings.remove("recentPluginList");
        settings.remove("recentPluginIndex");

        plugin_recent->clear();
        plugin_most->clear();
    }

}

void V3d_PluginLoader::searchPluginDirs(QMenu* menu, const QDir& pluginsDir)
{
	if (! menu)  return;

    QStringList dirList = pluginsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (QString dirName, dirList)
    {
        QMenu* submenu = new QMenu(dirName);
        if (! submenu)  return;
        menu->addMenu(submenu);

        QDir subDir = pluginsDir;
        subDir.cd(dirName);

        searchPluginDirs(submenu, subDir);
        searchPluginFiles(submenu, subDir);
    }
}

void V3d_PluginLoader::searchPluginFiles(QMenu* menu, const QDir& pluginsDir)
{
	if (!menu)  return;

    QStringList fileList = pluginsDir.entryList(QDir::Files);
    foreach (QString fileName, fileList)
    {
        QString fullpath = pluginsDir.absoluteFilePath(fileName);

		// Skip older versions that have been backed up with ".old" suffix.
		if (fullpath.endsWith(".old")) continue;
		if (fullpath.endsWith(".new")) continue;

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
            qDebug() << "Fail instantiation("<<loader->errorString().toStdString().c_str()<<"): " <<fullpath;

            //qDebug() << "Fail instantiation: " <<fullpath;
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
		if (text.startsWith("HIDDEN"))
			continue; //do not add some hidden menu items // by PHC, 2010-Dec-16
			
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

void V3d_PluginLoader::aboutPlugins()
{
    QList<QDir> pluginsDirList = getPluginsDirList();
    PluginDialog dialog("Vaa3D", pluginsDirList, pluginFilenameList, v3d_mainwindow);
    dialog.exec();
}

void V3d_PluginLoader::runPlugin(QPluginLoader *loader, const QString & menuString)
{
    if (!loader)
    {
    	v3d_msg("ERROR in Vaa3D_PluginLoader::runPlugin: invalid pointer to the plugin loader detected.");
    	return;
    }
	

#if QT_VERSION < 0x040806 // MK, 09242017, attempting to solve plugin issue on Windows with Qt4.8 and higher. Still not sure why it's ok with Qt4.7.
	loader->unload();
#endif

	QObject *plugin = loader->instance();
    if (!plugin)
    {
    	v3d_msg("ERROR in Vaa3D_PluginLoader::runPlugin: loader->instance()");
    	return;
    }
	
    //added by Zhi Z, 20140724
    QSettings settings("HHMI", "Vaa3D");
    recentpluginsList = settings.value("recentPluginList").toStringList();
    recentpluginsIndex = settings.value("recentPluginIndex").toList();

    QString CurrentpluginInfo = menuString + "%" + loader->fileName();
    int flag = 0;
    for(int i = 0; i < pluginFilenameList.size();i++)
    {
        if(pluginFilenameList.at(i) == loader->fileName())
        {
            flag = 1;
            break;
        }
    }
	
    if(flag == 1)
    {
        int currentIndex = 0;

        if(recentpluginsIndex.size() > 0)
        {
            for(int i=0; i< recentpluginsList.size(); i++)
            {
                if(recentpluginsList.at(i) == CurrentpluginInfo)
                {
                    currentIndex = recentpluginsIndex.at(i).toInt();
                    recentpluginsList.removeAt(i);
                    recentpluginsIndex.removeAt(i);
                    break;
                }
            }
        }
        recentpluginsList.prepend(CurrentpluginInfo);
        recentpluginsIndex.prepend(currentIndex + 1);

        settings.setValue("recentPluginList", recentpluginsList);
        settings.setValue("recentPluginIndex", recentpluginsIndex);

        updated_recentPlugins();
        plugin_menu.update();
    }

    bool done = false;
	if (!done)  { done = runPluginInterface2_1(plugin, menuString); v3d_msg("done with runPluginInterface2_1().",0); }
	if (!done)  { done = runPluginInterface2(plugin, menuString); v3d_msg("done with runPluginInterface2().",0); }
	if (!done)  { done = runPluginInterface(plugin, menuString); v3d_msg("done with runPluginInterface().",0); }
    // runSingleImageInterface works with both 1.0 and 2.1
    if (!done)  { done = runSingleImageInterface(plugin, menuString); v3d_msg("done with runSingleImageInterface().",0); }
	//
    if (!done)  {v3d_msg("No interface found.",0);}

	
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
    // if (plugin == NULL) {v3d_msg("plugin is null", 0);}
    // V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);
    // if (iFilter == NULL) {v3d_msg("plugin cannot be cast to V3DSingleImageInterface*", 0);}
    // V3DSingleImageInterface2_1 *iFilter2 = qobject_cast<V3DSingleImageInterface2_1 *>(plugin);
    // if (iFilter2 == NULL) {v3d_msg("plugin cannot be cast to V3DSingleImageInterface2_1*", 0);}
    // V3DSingleImageInterface *iFilter3 = dynamic_cast<V3DSingleImageInterface *>(plugin);
    // if (iFilter2 == NULL) {v3d_msg("plugin cannot be dynamic_cast to V3DSingleImageInterface*", 0);}
    
    // For some reason dynamic_cast works, but qobject_cast fails, when plugin is V3DSingleImageInterface2_1 //this should be wrong. by PHC 110705

    V3DSingleImageInterface2_1 *iFilter21 = qobject_cast<V3DSingleImageInterface2_1 *>(plugin);
    V3DSingleImageInterface *iFilter = qobject_cast<V3DSingleImageInterface *>(plugin);

    if (!iFilter21 && !iFilter)
    {
       return false;
    }

    bool done = true;

    qDebug()<<"Vaa3DSingleImageInterface or Vaa3DSingleImageInterface2_1..."<<iFilter;

    if (v3d_mainwindow)
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
        	if (iFilter21)
               	iFilter21->processImage(command, (Image4DSimple*)image, (QWidget*)0); //do not pass the mainwindow widget
            else
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

        qDebug()<<"runPluginInterface ..."<<iface;

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

        qDebug()<<"runPluginInterface2 ..."<<iface;

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

bool V3d_PluginLoader::runPluginInterface2_1(QObject* plugin, const QString& command)
{
    V3DPluginInterface2_1 *iface = qobject_cast<V3DPluginInterface2_1 *>(plugin);
	V3DPluginCallback2 *callback = dynamic_cast<V3DPluginCallback2 *>(this);

        qDebug()<<"runPluginInterface2_1 ..."<<iface;

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

bool V3d_PluginLoader::callPluginFunc(const QString &plugin_name,
		const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output)
{
    if (pluginFilenameList.isEmpty()) //added by PHC 20130904 to avoid duplicated menu of YuY's code below
        loadPlugins(); // ensure pluginFilenameList unempty 20110520 YuY
	
	QString fullpath;
    QList<QDir> pluginsDirList = getPluginsDirList();

    QStringList existingPluginsList = getPluginNameList();

    foreach (const QDir& pluginsDir, pluginsDirList)
    {
//        // Find the first plugin directory with such a file
//        if (pluginsDir.exists(plugin_name)) {
//            fullpath = pluginsDir.absoluteFilePath(plugin_name);
//            break;
//        }

//the following was sugegsted by Zhi Zhou to do partial name match. 20130706

        std::cout << pluginsDir.dirName().toStdString() << std::endl;
        // Find the first plugin directory with such a file name or partial name by  Zhi Zhou 20130705
        foreach (QString file, existingPluginsList )
        if (file.contains(plugin_name))
        {
            fullpath = pluginsDir.absoluteFilePath(file);
            break;
        }
    }
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
	V3DPluginInterface2_1 *iface2_1 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
	V3DPluginCallback2 *callback = dynamic_cast<V3DPluginCallback2 *>(this);
    if (iface && callback)
    {
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
    else if (iface2_1 && callback) {
         try
        {
            return iface2_1->dofunc(func_name, input, output, *callback, (QWidget*)0);
        }
        catch (...)
        {
            v3d_msg(QString("The plugin fails to call [%1]. Check your plugin code please.").arg(func_name));
            return false;
        }
        return true;
    }
    else // (! (iface && callback) )
	{
		qDebug()<<QString("ERROR: callPluginFunc cannot cast (Vaa3DPluginInterface2_1) of plugin '%1'").arg(plugin_name);
		return false;
	}

}

////a bug caused in v2.823?
/*
bool V3d_PluginLoader::callPluginMenu(const QString &plugin_name,
                                      const QString &menu_name)
{
    
	loadPlugins(); // ensure pluginFilenameList unempty 20110520 YuY
	
	QString fullpath;
    QList<QDir> pluginsDirList = getPluginsDirList();
    foreach (const QDir& pluginsDir, pluginsDirList)
    {
        // Find the first plugin directory with such a file
        if (pluginsDir.exists(plugin_name)) {
            fullpath = pluginsDir.absoluteFilePath(plugin_name);
            break;
        }
    }
	qDebug()<<"callPluginMenu fullpath: " <<fullpath;
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
    	qDebug("ERROR in V3d_PluginLoader::callPluginMenu: loader->instance()");
    	return false;
    }
    
	V3DPluginInterface2 *iface = qobject_cast<V3DPluginInterface2 *>(plugin);
	V3DPluginInterface2_1 *iface2_1 = qobject_cast<V3DPluginInterface2_1 *>(plugin);
	V3DPluginCallback2 *callback = dynamic_cast<V3DPluginCallback2 *>(this);
    if (iface && callback) {
        try
        {
            iface->domenu(menu_name, *callback, (QWidget*)0);
            return true;
        }
        catch (...)
        {
            v3d_msg(QString("The plugin fails to call [%1]. Check your plugin code please.").arg(menu_name));
            return false;
        }
        return true;
    }
    else if (iface2_1 && callback) {
        try
        {
            iface2_1->domenu(menu_name, *callback, (QWidget*)0);
            return true;
        }
        catch (...)
        {
            v3d_msg(QString("The plugin fails to call [%1]. Check your plugin code please.").arg(menu_name));
            return false;
        }
        return true;
    }
    else // (! (iface && callback) )
	{
		qDebug()<<QString("ERROR: callPluginMenu cannot cast (Vaa3DPluginInterface2_1) of plugin '%1'").arg(plugin_name);
		return false;
	}
}

*/

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
void V3d_PluginLoader::updateImageWindow(v3dhandle image_window, bool b_forceUpdateChannelMinMaxValues) //by PHC, 20120412
{
	if (v3d_mainwindow)
	{
		XFormWidget* w = v3d_mainwindow->updateImageWindow(image_window, b_forceUpdateChannelMinMaxValues);
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

int V3d_PluginLoader::setSWC_noDecompose(V3dR_MainWindow* window, const char* fileName)
{
	if (v3d_mainwindow)
	{
		return v3d_mainwindow->setSWC_noDecompose(window, fileName);
	}
	return -1;
}

bool V3d_PluginLoader::hideSWC(V3dR_MainWindow* window, int treeIndex)
{
	if (v3d_mainwindow)
	{
		return v3d_mainwindow->hideSWC(window, treeIndex);
	}
	return false;
}

bool V3d_PluginLoader::displaySWC(V3dR_MainWindow* window, int treeIndex)
{
	if (v3d_mainwindow)
	{
		return v3d_mainwindow->displaySWC(window, treeIndex);
	}
	return false;
}

QList<NeuronTree> V3d_PluginLoader::loadedNeurons(V3dR_MainWindow* window, QList<string>& loadedSurfaces)
{
	QList<NeuronTree> emptyList;
	if (v3d_mainwindow)
	{
		return v3d_mainwindow->loadedNeurons(window, loadedSurfaces);
	}
	return emptyList;
}

Image4DSimple * V3d_PluginLoader::loadImage(char *filename)  //2013-08-09. two more functions for simplied calls to use Vaa3D's image loading and saving functions without linking to additional libs
{
    if (!filename)
        return 0;

    Image4DSimple * myimg = new Image4DSimple;
    myimg->loadImage(filename, false); //first try libtiff
    if (myimg->valid()==false) //add this double-loading as of 140630 to cope with Zhi's request
    {
        myimg->loadImage(filename, true); //add using mylib support 20131105
        if (myimg->valid())
            return myimg;
        else
            return 0;
    }
    else
        return myimg;
}

Image4DSimple * V3d_PluginLoader::loadImage(char *filename, V3DLONG zsliceno)  //2013-11-02
{
    if (!filename)
        return 0;

    if (zsliceno<0)
        return loadImage(filename);

    bool b_UseMyLib=true;
    Image4DSimple * myimg = new Image4DSimple;
    myimg->loadImage_slice(filename, b_UseMyLib, zsliceno);
    if (myimg->valid())
        return myimg;
    else
        return 0;
}


bool V3d_PluginLoader::saveImage(Image4DSimple * img, char *filename)
{
    if (!img || !filename || !img->valid())
        return false;

    return img->saveImage(filename);
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

QStringList V3d_PluginLoader::getPluginNameList()
{
    return pluginFilenameList;
}

void V3d_PluginLoader::pushImageToTeraWin(v3dhandle data)
{
    tf::PluginInterface::pushImageToTeraWin(data);
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

V3dR_MainWindow * V3d_PluginLoader::open3DViewerForSingleSurfaceFile(QString fileName) //By PHC 20150210
{
    if (v3d_mainwindow)
    {
        QFileInfo curfile_info(fileName);
        QString cur_suffix = curfile_info.suffix().toUpper();
        if (cur_suffix=="APO" ||
                 cur_suffix=="SWC" ||
                 cur_suffix=="ESWC" ||
                 cur_suffix=="OBJ" ||
                 cur_suffix=="VAA3DS" ||
                 cur_suffix=="V3DS" ||
                 cur_suffix=="NULL3DVIEWER" || fileName=="NULL3DVIEWER")
        {
            v3d_mainwindow->loadV3DFile(fileName, true, this->v3d_mainwindow->global_setting.b_autoOpenImg3DViewer);
            return v3d_mainwindow->find3DViewer(fileName);
        }
    }

    return 0;
}

V3dR_MainWindow * V3d_PluginLoader::open3DViewerForLinkerFile(QString fileName) //By PHC 20150210
{
    if (v3d_mainwindow)
    {
        QFileInfo curfile_info(fileName);
        QString cur_suffix = curfile_info.suffix().toUpper();
        if (cur_suffix=="ANO")
        {
            v3d_mainwindow->loadV3DFile(fileName, true, this->v3d_mainwindow->global_setting.b_autoOpenImg3DViewer);
            return v3d_mainwindow->find3DViewer(fileName);
        }
    }

    return 0;
}

V3dR_MainWindow * V3d_PluginLoader::createEmpty3DViewer() //By PHC 20150210
{
    if (v3d_mainwindow)
    {
        V3dR_MainWindow *p = open3DViewerForSingleSurfaceFile("noname.NULL3DVIEWER");
        return p;
    }

    return 0;
}

void V3d_PluginLoader::moveWindow(V3dR_MainWindow *w, int x, int y)
{
    if (v3d_mainwindow )
    {
      w->move(x,y);
    }
}

void V3d_PluginLoader::resizeWindow(V3dR_MainWindow *w, int x, int y)
{
    if (v3d_mainwindow )
    {
      w->resize(x,y);
    }
}
void V3d_PluginLoader::setHideDisplayControlButton(V3dR_MainWindow *w)
{
    if(v3d_mainwindow)
    {
        w->hideDisplayControls();
    }
}

//void V3d_PluginLoader::setResizeEvent(V3dR_MainWindow *w, int x, int y)
//{
//    if(v3d_mainwindow)
//    {
//        w->resizeEvent((QResizeEvent *)QSize(x,y));
//    }
//}

#ifdef _NEURON_ASSEMBLER_
int V3d_PluginLoader::getSurfaceType(V3dR_MainWindow* w)
{
	V3dR_GLWidget* vi = w->getGLWidget();
	Renderer_gl1* thisRenderer = (Renderer_gl1*)(vi->getRenderer());
	return int(thisRenderer->surType);
}

void V3d_PluginLoader::set3DViewerMarkerDetectorStatus(bool on_off, V3dR_MainWindow* w)
{
	V3dR_GLWidget* vi = w->getGLWidget();
	Renderer_gl1* thisRenderer = (Renderer_gl1*)(vi->getRenderer());
	thisRenderer->FragTraceMarkerDetector3Dviewer = on_off;
	//cout << "marker detector test: " << thisRenderer->listMarker.size() << endl;
	//cout << "apo detector test: " << thisRenderer->listCell.size() << endl;

	//for (QList<CellAPO>::iterator it = thisRenderer->listCell.begin(); it != thisRenderer->listCell.end(); ++it)
	//	cout << it->name.toStdString() << ": " << it->selected << endl;
}

QList<ImageMarker> V3d_PluginLoader::send3DviewerMarkerList(V3dR_MainWindow* w)
{
	V3dR_GLWidget* vi = w->getGLWidget();
	Renderer_gl1* thisRenderer = (Renderer_gl1*)(vi->getRenderer());
	return thisRenderer->listMarker;
}

QList<CellAPO> V3d_PluginLoader::send3DviewerApoList(V3dR_MainWindow* w)
{
	V3dR_GLWidget* vi = w->getGLWidget();
	Renderer_gl1* thisRenderer = (Renderer_gl1*)(vi->getRenderer());
	return thisRenderer->listCell;
}

void V3d_PluginLoader::refreshSelectedMarkers(V3dR_MainWindow* w)
{
	V3dR_GLWidget* vi = w->getGLWidget();
	Renderer_gl1* thisRenderer = (Renderer_gl1*)(vi->getRenderer());

	if (thisRenderer->surType == stImageMarker)
	{
		for (QList<ImageMarker>::iterator it = thisRenderer->listMarker.begin(); it != thisRenderer->listMarker.end(); ++it)
			it->selected = false;
	}
	else if (thisRenderer->surType == stPointCloud)
	{
		for (QList<CellAPO>::iterator it = thisRenderer->listCell.begin(); it != thisRenderer->listCell.end(); ++it)
			it->selected = false;
	}	
}
#endif

void V3d_PluginLoader::setWindowDataTitle(V3dR_MainWindow * w, QString title)
{
    if (v3d_mainwindow )
    {
      w->setDataTitle(title);
    }
}

QString V3d_PluginLoader::getWindowDataTitle(V3dR_MainWindow * w)
{
    if (v3d_mainwindow )
    {
      return w->getDataTitle();
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

//added Dec. 02,2013 by Zhi Zhou

void V3d_PluginLoader::screenShot_Any3DViewer(V3dR_MainWindow *w,QString filename)
{
    if (w)
    {
        V3dR_GLWidget * v = w->getGLWidget();
        if (v)
            v->screenShot(filename);
    }
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

//added Oct. 08, 2014 by Hanbo Chen
void V3d_PluginLoader::update_NeuronBoundingBox(V3dR_MainWindow *w)
{
    if(w && w->getGLWidget())
    {
        V3dR_GLWidget * g = w->getGLWidget();
        Renderer_gl1 * gp = (Renderer_gl1 *) g->getRenderer();
        gp->updateNeuronBoundingBox();
        gp->updateBoundingBox();
    }
}

LandmarkList * V3d_PluginLoader::getHandleLandmarkList_Any3DViewer(V3dR_MainWindow *w) //Oct. 16, 2014 by Hanbo Chen
{
    if (!w) return 0;
    V3dR_GLWidget * vi = w->getGLWidget();
    if (!vi) return 0;
    else
    {
        Renderer_gl1 * gp = (Renderer_gl1 *)(vi->getRenderer());
        if (!gp)
            return 0;
        else
            return (gp->getHandleLandmark());
    }
}

bool V3d_PluginLoader::setHandleLandmarkList_Any3DViewer(V3dR_MainWindow *w, LandmarkList & landmark_list)//Aug. 2, 2017 by Zhi Zhou
{
    if (!w) return false;
    V3dR_GLWidget * vi = w->getGLWidget();
    if (!vi) return false;
    else
    {
        Renderer_gl1 * gp = (Renderer_gl1 *)(vi->getRenderer());
        if (!gp)
            return false;
        else
        {
            gp->setHandleLandmark(landmark_list);
            gp->updateLandmark();
            return true;
        }
    }
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

//added Dec. 02,2013 by Zhi Zhou

View3DControl * V3d_PluginLoader::getView3DControl_Any3DViewer(V3dR_MainWindow *w)
{
    View3DControl * vi = 0;
    if (w)
    {
        return (vi = dynamic_cast<View3DControl *>(w->getGLWidget()));
    }
}

View3DControl * V3d_PluginLoader::getLocalView3DControl(v3dhandle image_window)
{
	View3DControl * vi = 0;
	if_XFormWidget(w, image_window)
	{
		//vi = w->getLocalView3DControl();
		vi = dynamic_cast<View3DControl *>(w->getLocalView3D());
		qDebug() << "V3d_PluginLoader getLocalView3DControl = "<< vi <<" : " << w <<"/"<< image_window;
	}
	return vi;
}

TriviewControl * V3d_PluginLoader::getTriviewControl(v3dhandle image_window)
{
	TriviewControl * tvi = 0;
	if_XFormWidget(w, image_window)
	{
		tvi = dynamic_cast<TriviewControl *>(w); // w->getTriview()
		qDebug() << "V3d_PluginLoader getTriviewControl = "<< tvi <<" : " << w <<"/"<< image_window;
	}
	return tvi;
}

//added PHC 20120406. add a main window handle, to allow access everything in Vaa3D

MainWindow * V3d_PluginLoader::getVaa3DMainWindow()
{
    return v3d_mainwindow;
}

QList <V3dR_MainWindow *> V3d_PluginLoader::getListAll3DViewers()
{
    QList <V3dR_MainWindow *> mylist;
    if (v3d_mainwindow)
        return v3d_mainwindow->list_3Dview_win;
    else
        return mylist;
}

V3dR_MainWindow * V3d_PluginLoader::find3DViewerByName(QString fileName) 
{
    if (v3d_mainwindow)
        return v3d_mainwindow->find3DViewer(fileName);
    else
        return 0;
}

void V3d_PluginLoader::update_3DViewer(V3dR_MainWindow *w) //Dec, 9, 2013. PHC
{
    if (w)
    {
        V3dR_GLWidget * v = w->getGLWidget();
        if (v)
            v->update();
    }

    pumpEvents(); //131209 PHC

}

//the following 12 functions are added PHC 20120406 to allow uses to access the surface data objects in a 3D viewer

QList <NeuronTree> * getHandleNeuronTrees_3DGLWidget(V3dR_GLWidget *vi) //a utility function
{
    if (!vi) return 0;
    else
    {
        Renderer_gl1 * gp = (Renderer_gl1 *)(vi->getRenderer());
        if (!gp)
            return 0;
        else
            return gp->getHandleNeuronTrees();
    }    
}

QList <NeuronTree> * V3d_PluginLoader::getHandleNeuronTrees_3DGlobalViewer(v3dhandle image_window)
{
	V3dR_GLWidget * vi = (V3dR_GLWidget *)(getView3DControl(image_window));
    return getHandleNeuronTrees_3DGLWidget(vi);
}

QList <NeuronTree> * V3d_PluginLoader::getHandleNeuronTrees_Any3DViewer(V3dR_MainWindow *w)
{
    if (!w) return 0;
	V3dR_GLWidget * vi = w->getGLWidget();
    return getHandleNeuronTrees_3DGLWidget(vi);
}

//=======

QList <CellAPO>    * getHandleAPOCellList_3DGLWidget(V3dR_GLWidget *vi) //a utility function
{
    if (!vi) return 0;
    else
    {
        Renderer_gl1 * gp = (Renderer_gl1 *)(vi->getRenderer());
        if (!gp)
            return 0;
        else
            return gp->getHandleAPOCellList();
    }    
}

QList <CellAPO>    * V3d_PluginLoader::getHandleAPOCellList_3DGlobalViewer(v3dhandle image_window)
{
	V3dR_GLWidget * vi = (V3dR_GLWidget *)(getView3DControl(image_window));
    return getHandleAPOCellList_3DGLWidget(vi);
}

QList <CellAPO>    * V3d_PluginLoader::getHandleAPOCellList_Any3DViewer(V3dR_MainWindow *w) 
{
    if (!w) return 0;
	V3dR_GLWidget * vi = w->getGLWidget();
    return getHandleAPOCellList_3DGLWidget(vi);
}

//====

QList <LabelSurf> getListLabelSurf_3DGLWidget(V3dR_GLWidget *vi) //a utility function
{
    QList <LabelSurf> mylabelsurf;
    if (!vi) return mylabelsurf;
    else
    {
        Renderer_gl1 * gp = (Renderer_gl1 *)(vi->getRenderer());
        if (!gp)
            return mylabelsurf;
        else
            return gp->getListLabelSurf();
    }    
}

QList <LabelSurf> V3d_PluginLoader::getListLabelSurf_3DGlobalViewer(v3dhandle image_window)
{
    QList <LabelSurf> mylabelsurf;
    
	V3dR_GLWidget * vi = (V3dR_GLWidget *)(getView3DControl(image_window));
    return getListLabelSurf_3DGLWidget(vi);
}

QList <LabelSurf> V3d_PluginLoader::getListLabelSurf_Any3DViewer(V3dR_MainWindow *w)
{
    QList <LabelSurf> mylabelsurf;
    if (!w) return mylabelsurf;
	V3dR_GLWidget * vi = w->getGLWidget();
    return getListLabelSurf_3DGLWidget(vi);
}

DataLists_in_3dviewer V3d_PluginLoader::fetch_3dviewer_datafilelist(QString name3dviewer)  //20140122 a conveniece function to access the record of data in a 3D viewer
{
    DataLists_in_3dviewer s;

    V3dR_MainWindow * w = find3DViewerByName(name3dviewer);
    if (!w)
    {
        v3d_msg(QString("Fail to find valid V3dR_MainWindow for the name [%1].").arg(name3dviewer));
        return s;
    }
    if (!(w->_idep))
    {
        v3d_msg(QString("The found V3dR_MainWindow for the name [%1] has a NULL pointer to the 3D data.").arg(name3dviewer));
        return s;
    }

    s.swc_file_list = w->_idep->swc_file_list;
    s.pointcloud_file_list = w->_idep->pointcloud_file_list;
    s.surface_file = w->_idep->surface_file;
    s.labelfield_file = w->_idep->labelfield_file;

    if (w->_idep->image4d)
        s.imgfile = w->_idep->image4d->getFileName();

    return s;
}

//==========

bool setListLabelSurf_3DGLWidget(V3dR_GLWidget *vi, QList <LabelSurf> listLabelSurfinput) //a utility function
{
    if (!vi) return false;
    else
    {
        Renderer_gl1 * gp = (Renderer_gl1 *)(vi->getRenderer());
        if (!gp)
            return false;
        else
        {
            gp->setListLabelSurf(listLabelSurfinput);
            return true;
        }
    }            
}

bool V3d_PluginLoader::setListLabelSurf_3DGlobalViewer(v3dhandle image_window, QList <LabelSurf> listLabelSurfinput)
{
	V3dR_GLWidget * vi = (V3dR_GLWidget *)(getView3DControl(image_window));
    return setListLabelSurf_3DGLWidget(vi, listLabelSurfinput);
}

bool V3d_PluginLoader::setListLabelSurf_Any3DViewer(V3dR_MainWindow *w, QList <LabelSurf> listLabelSurfinput)
{
    if (!w) return false;
	V3dR_GLWidget * vi = w->getGLWidget();
    return setListLabelSurf_3DGLWidget(vi, listLabelSurfinput);
}

//added PHC 20130904 allow a plugin program to refresh and rescan all plugins //not working by PHC 20130904
//void V3d_PluginLoader::refreshMainMenuPluginList()
//{
//    rescanPlugins();
//}

//added TeraFly interface, functions are provided by Alessadnro Bria, the wrapper is provided by Zhi Zhou Aug. 23, 2017
NeuronTree V3d_PluginLoader::getSWCTeraFly()
{
    return terafly::PluginInterface::getSWC();
}

bool V3d_PluginLoader::setSWCTeraFly(NeuronTree & nt)
{
    return terafly::PluginInterface::setSWC(nt);
}

LandmarkList V3d_PluginLoader::getLandmarkTeraFly()
{
    return terafly::PluginInterface::getLandmark();
}

bool V3d_PluginLoader::setLandmarkTeraFly(LandmarkList & landmark_list)
{
    return terafly::PluginInterface::setLandmark(landmark_list);
}

QString V3d_PluginLoader::getPathTeraFly()
{
    return QString(terafly::PluginInterface::getPath().c_str());
}

const Image4DSimple * V3d_PluginLoader::getImageTeraFly()
{
    return terafly::PluginInterface::getImage();
}

QString V3d_PluginLoader::versionTeraFly()
{
    return QString(terafly::PluginInterface::version().c_str());
}

bool V3d_PluginLoader::getDimTeraFly(const std::string & path, V3DLONG * & sz)
{
    sz = new V3DLONG [5];
    if (!sz)
    {
        return false;
    }
    sz[0] = terafly::PluginInterface::getXDim(path);
    sz[1] = terafly::PluginInterface::getYDim(path);
    sz[2] = terafly::PluginInterface::getZDim(path);
    sz[3] = terafly::PluginInterface::getCDim(path);
    sz[4] = terafly::PluginInterface::getTDim(path);
    return true;
}

unsigned char* V3d_PluginLoader::getSubVolumeTeraFly(const std::string & path, size_t x0, size_t x1, size_t y0, size_t y1, size_t z0, size_t z1)
{
    return terafly::PluginInterface::getSubVolume(path,x0,x1,y0,y1,z0,z1);
}


void V3d_PluginLoader::releaseOpenedVolumesTeraFly()
{
    terafly::PluginInterface::releaseOpenedVolumes();
}

bool V3d_PluginLoader::setImageTeraFly(size_t x, size_t y, size_t z)
{
    return terafly::PluginInterface::setImage(x,y,z);
}

#ifdef __ALLOW_VR_FUNCS__
void V3d_PluginLoader::openVRWindow(V3dR_MainWindow *w, bool bOnlineMode)
{
    qDebug("V3d_PluginLoader::openVRWindow ");
    if (w)
    {
        qDebug("V3d_PluginLoader::openVRWindow ----if w");
        V3dR_GLWidget * v = w->getGLWidget();
        if (v)
        {
            qDebug("V3d_PluginLoader::openVRWindow ----if v");
            v->doimageVRView(bOnlineMode);
        }
    }
}

void V3d_PluginLoader::openVRWindowV2(v3dhandle image_window, bool bOnlineMode)
{
    qDebug("V3d_PluginLoader::openVRWindow version 2");
    V3dR_GLWidget * vi = (V3dR_GLWidget *)(getView3DControl(image_window));
    if(vi) vi->doimageVRView(bOnlineMode);
}
#endif
