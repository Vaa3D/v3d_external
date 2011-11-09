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
* Last change: 2010-10-08. Hanchuan Peng 
*
***************************************************************************
*/

#ifndef _V3D_PLUGIN_LOADER_H_
#define _V3D_PLUGIN_LOADER_H_

#include <QtGui>
#include "../basic_c_fun/v3d_interface.h"


QString     v3d_getInterfaceName(QObject *plugin);
QStringList v3d_getInterfaceMenuList(QObject *plugin);
QStringList v3d_getInterfaceFuncList(QObject *plugin);

class MainWindow;

class V3d_PluginLoader : public QObject, public V3DPluginCallback2
{
    Q_OBJECT;

public:
    V3d_PluginLoader(QMenu* menuPlugin, MainWindow* mainwindow);
    V3d_PluginLoader(MainWindow* mainwindow); //by PHC, 101008. a convenience function for access plugin interface w/o a menu
    virtual ~V3d_PluginLoader() {clear();}
    static QList<QDir> getPluginsDirList();

public slots:
	void rescanPlugins();
	void populateMenus(); //hook menu to v3d, called by rescanPlugins, MainWindow::updateProcessingMenu
	void aboutPlugins();
    void runPlugin();
	void runPlugin(QPluginLoader *loader, const QString & menuString); //by PHC, 101008

protected:
	bool runSingleImageInterface(QObject* plugin, const QString& command);
	bool runPluginInterface(QObject* plugin, const QString& command);
	bool runPluginInterface2(QObject* plugin, const QString& command);
	bool runPluginInterface2_1(QObject* plugin, const QString& command);

	void clear();
	void loadPlugins(); //load only once
	void addToMenu(QMenu *menu, QObject *plugin, const QStringList &texts, const char *member);
	void searchPluginDirs(QMenu* menu, const QDir& pluginsDir);
	void searchPluginFiles(QMenu* menu, const QDir& pluginsDir);

    // QList<QDir> pluginsDirList;
    QStringList pluginFilenameList;
    QList<QPluginLoader*> pluginList;
    QMenu plugin_menu;

    QMenu* v3d_menuPlugin;
    MainWindow* v3d_mainwindow;
	
public:
	QStringList getPluginNameList(); //by YuY 20110429

//V3DPluginCallback
public:
	virtual bool callPluginFunc(const QString &plugin_name,
			const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output);

	virtual v3dhandleList getImageWindowList() const;
	virtual v3dhandle currentImageWindow();
	virtual v3dhandle curHiddenSelectedWindow(); //by PHC 101009

	virtual v3dhandle newImageWindow(QString name);
	virtual void updateImageWindow(v3dhandle image_window);

	virtual QString getImageName(v3dhandle image_window) const;
	virtual void setImageName(v3dhandle image_window, QString name);

	virtual Image4DSimple* getImage(v3dhandle image_window);
	virtual bool setImage(v3dhandle image_window, Image4DSimple* image);

	//a special debug function. don't use if you don't know how to use. by PHC 100731.
	virtual bool setImageTest(v3dhandle image_window, Image4DSimple* image, unsigned char *a);

	virtual LandmarkList  getLandmark(v3dhandle image_window);
	virtual bool setLandmark(v3dhandle image_window, LandmarkList& landmark_list);

	virtual ROIList getROI(v3dhandle image_window);
	virtual bool setROI(v3dhandle image_window, ROIList & roi_list);

	virtual NeuronTree getSWC(v3dhandle image_window);
	virtual bool setSWC(v3dhandle image_window, NeuronTree & nt);

	virtual V3D_GlobalSetting getGlobalSetting();
	virtual bool setGlobalSetting( V3D_GlobalSetting & gs );

	virtual void open3DWindow(v3dhandle image_window);
	virtual void close3DWindow(v3dhandle image_window);
	virtual void openROI3DWindow(v3dhandle image_window);
	virtual void closeROI3DWindow(v3dhandle image_window);

	virtual void pushObjectIn3DWindow(v3dhandle image_window);
	virtual void pushImageIn3DWindow(v3dhandle image_window);
	virtual int pushTimepointIn3DWindow(v3dhandle image_window, int timepoint);

	virtual bool screenShot3DWindow(v3dhandle image_window, QString filename);
	virtual bool screenShotROI3DWindow(v3dhandle image_window, QString filename);
	

//V3DPluginCallback2
public:
	virtual View3DControl * getView3DControl(v3dhandle image_window);
	virtual View3DControl * getLocalView3DControl(v3dhandle image_window);
	virtual TriviewControl * getTriviewControl(v3dhandle image_window); //aded by PHC, 2010-12-08

};

#endif
