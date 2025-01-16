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
* Vaa3D's plug-in loading support functions
*
* 2009-08-11, Zongcai Ruan
* Last change: 2010-10-08. Hanchuan Peng
* Last change: 2015-02-10. Hanchuan Peng
***************************************************************************
*/

#ifndef _V3D_PLUGIN_LOADER_H_
#define _V3D_PLUGIN_LOADER_H_
#include <qmenu.h>
#include <set>

// These two explicit includes make my IDE work better - CMB 08-Oct-2010
#include "../basic_c_fun/v3d_interface.h"
#include "./vrrenderer/V3dR_Communicator.h"


QString     v3d_getInterfaceName(QObject *plugin);
QStringList v3d_getInterfaceMenuList(QObject *plugin);
QStringList v3d_getInterfaceFuncList(QObject *plugin);

class MainWindow;
class V3d_PluginLoader;
class DataFlowPlus{
public:
    static DataFlowPlus& getInstance(){
        static DataFlowPlus instance;
        return instance;
    }

    // Avoid copy construct
    DataFlowPlus(DataFlowPlus const&) = delete;
    void operator=(DataFlowPlus const&)  = delete;
    void insert(QString name,Image4DSimple* img);
    void insert(QString name,NeuronTree* nt);
    void insert(QString name,void* sth);
    Image4DSimple* findimg(QString name);
    NeuronTree* findnt(QString name);
    void* findvoid(QString name);
private:
    DataFlowPlus();
    ~DataFlowPlus();
    static QHash<QString,Image4DSimple*> imgflow;
    static QHash<QString,NeuronTree*> ntflow;
    static QHash<QString,void*> voidflow;

};
class V3d_PluginLoader : public QObject, public V3DPluginCallback2
{
    Q_OBJECT;

public:
    V3d_PluginLoader(QMenu* menuPlugin, MainWindow* mainwindow);
    V3d_PluginLoader(MainWindow* mainwindow); //by PHC, 101008. a convenience function for access plugin interface w/o a menu
    virtual ~V3d_PluginLoader() {clear();}
    static QList<QDir> getPluginsDirList();
    V3dR_Communicator* TeraflyCommunicator;

public slots:
	void rescanPlugins();
	void populateMenus(); //hook menu to v3d, called by rescanPlugins, MainWindow::updateProcessingMenu
	void aboutPlugins();
    void runPlugin();
	void runPlugin(QPluginLoader *loader, const QString & menuString); //by PHC, 101008
    void runRecentPlugin(); //added by Zhi Z, 20140721
    void clear_recentPlugins();

protected:
    //ljs,dlc,csz
    void pushImageToTeraWin(v3dhandle);
    void putDataToCViewer(const unsigned char*,V3DPluginCallback2*);
    bool updateTerafly();

    //csz
    void OpenImageInTerafly(QString image_path,V3DPluginCallback2* callback);
    bool isCViewerVisable();

	bool runSingleImageInterface(QObject* plugin, const QString& command);
	bool runPluginInterface(QObject* plugin, const QString& command);
	bool runPluginInterface2(QObject* plugin, const QString& command);
	bool runPluginInterface2_1(QObject* plugin, const QString& command);
	void clear();
	void loadPlugins(); //load only once
	void addToMenu(QMenu *menu, QObject *plugin, const QStringList &texts, const char *member);
	void searchPluginDirs(QMenu* menu, const QDir& pluginsDir);
	void searchPluginFiles(QMenu* menu, const QDir& pluginsDir);
    void addrecentPlugins(QMenu* menu); //add by Zhi Z, 20140721
    void updated_recentPlugins();

    // QList<QDir> pluginsDirList;
    QStringList pluginFilenameList;
    QList<QPluginLoader*> pluginList;
    QMenu plugin_menu;

    QStringList recentpluginsList;
    QList<QVariant> recentpluginsIndex;
    QMenu *plugin_recent;//added by Zhi Z, 20140721
    QMenu *plugin_most;//added by Zhi Z, 20140721

    QMenu* v3d_menuPlugin;
    MainWindow* v3d_mainwindow;

public:
	QStringList getPluginNameList(); //by YuY 20110429

//V3DPluginCallback
public:
	virtual bool callPluginFunc(const QString &plugin_name,
			const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output);
    //virtual bool callPluginMenu(const QString &plugin_name, const QString &func_name); //a bug caused in v2.823?

	virtual v3dhandleList getImageWindowList() const;
	virtual v3dhandle currentImageWindow();
	virtual v3dhandle curHiddenSelectedWindow(); //by PHC 101009

	virtual v3dhandle newImageWindow(QString name);
	virtual void updateImageWindow(v3dhandle image_window, bool b_forceUpdateChannelMinMaxValues); //by PHC, 20120412

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
    virtual bool setSWC(v3dhandle image_window, NeuronTree & nt, bool collaborate=false);

    virtual Image4DSimple * loadImage(char *filename);  //2013-08-09. two more functions for simplied calls to use Vaa3D's image loading and saving functions without linking to additional libs
    virtual Image4DSimple * loadImage(char *filename, V3DLONG zsliceno);  //2013-11-02
    virtual bool saveImage(Image4DSimple * img, char *filename);

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

    //added PHC 20120406. add a main window handle, to allow access everything in Vaa3D

    virtual MainWindow * getVaa3DMainWindow();
    virtual QList <V3dR_MainWindow *> getListAll3DViewers();
    virtual V3dR_MainWindow * find3DViewerByName(QString fileName); //the name can be partially matched

    //added PHC 20120406 to allow uses to access the surface data objects in a 3D viewer but based on a tri-view window
	virtual QList <NeuronTree> * getHandleNeuronTrees_3DGlobalViewer(v3dhandle image_window);
	virtual QList <CellAPO>    * getHandleAPOCellList_3DGlobalViewer(v3dhandle image_window);
    virtual QList <LabelSurf> getListLabelSurf_3DGlobalViewer(v3dhandle image_window);
    virtual bool setListLabelSurf_3DGlobalViewer(v3dhandle image_window, QList <LabelSurf> listLabelSurfinput);

    //added PHC 20120406 to allow uses to access the surface data objects in a 3D viewer
	virtual QList <NeuronTree> * getHandleNeuronTrees_Any3DViewer(V3dR_MainWindow *w);
	virtual QList <CellAPO>    * getHandleAPOCellList_Any3DViewer(V3dR_MainWindow *w);
    virtual QList <LabelSurf> getListLabelSurf_Any3DViewer(V3dR_MainWindow *w);
    virtual bool setListLabelSurf_Any3DViewer(V3dR_MainWindow *w, QList <LabelSurf> listLabelSurfinput);

    //added PHC 20130904 allow a plugin program to refresh and rescan all plugins
    //virtual void refreshMainMenuPluginList(); //not working, by PHC 20130904

    virtual View3DControl * getView3DControl_Any3DViewer(V3dR_MainWindow *w);  //Dec. 02, 2013 by Zhi Zhou
    virtual void screenShot_Any3DViewer(V3dR_MainWindow *w,QString filename);//Dec. 02, 2013 by Zhi Zhou
    virtual void update_3DViewer(V3dR_MainWindow *w);//Dec, 9, 2013. PHC
    virtual void update_NeuronBoundingBox(V3dR_MainWindow *w); //Oct. 08, 2014 by Hanbo Chen

    virtual LandmarkList * getHandleLandmarkList_Any3DViewer(V3dR_MainWindow *w); //Oct. 16, 2014 by Hanbo Chen
    virtual bool setHandleLandmarkList_Any3DViewer(V3dR_MainWindow *w, LandmarkList & landmark_list); //Aug. 2, 2017 by Zhi Zhou

    virtual DataLists_in_3dviewer fetch_3dviewer_datafilelist(QString name3dviewer); //20140122 a conveniece function to access the record of data in a 3D viewer

    // extend the plugin interface by allowing open a 3D surface file , PHC Feb 10, 2015
    virtual V3dR_MainWindow * open3DViewerForSingleSurfaceFile(QString fileName);
    virtual V3dR_MainWindow * open3DViewerForLinkerFile(QString fileName);
    virtual V3dR_MainWindow * createEmpty3DViewer(); //create a 3D viewer with no content
    virtual void setWindowDataTitle(V3dR_MainWindow *w, QString title);
    virtual QString getWindowDataTitle(V3dR_MainWindow *w);
    virtual void moveWindow(V3dR_MainWindow *w, int x, int y);
    virtual void resizeWindow(V3dR_MainWindow *w, int x, int y);
    virtual void setHideDisplayControlButton(V3dR_MainWindow *w);
    //virtual void setResizeEvent(V3dR_MainWindow *w, int x, int y);

#ifdef _NEURON_ASSEMBLER_
	virtual int getSurfaceType(V3dR_MainWindow* w);
	virtual void set3DViewerMarkerDetectorStatus(bool on_off, V3dR_MainWindow* w);
	virtual QList<ImageMarker> send3DviewerMarkerList(V3dR_MainWindow* w);
	virtual QList<CellAPO> send3DviewerApoList(V3dR_MainWindow* w);
	virtual void refreshSelectedMarkers(V3dR_MainWindow* w);
#endif

#ifdef __ALLOW_VR_FUNCS__
    virtual void openVRWindow(V3dR_MainWindow *w, bool bOnlineMode = false);
    virtual void openVRWindowV2(v3dhandle image_window, bool bOnlineMode = false);
#endif

    //added TeraFly interface, functions are provided by Alessadnro Bria, the wrapper is provided by Zhi Zhou Aug. 23, 2017
    virtual NeuronTree getSWCTeraFly();
    virtual bool setSWCTeraFly(NeuronTree & nt);
    virtual LandmarkList  getLandmarkTeraFly();
    virtual bool setLandmarkTeraFly(LandmarkList & landmark_list);
    virtual QString getPathTeraFly();
    virtual const Image4DSimple * getImageTeraFly();
    virtual QString versionTeraFly();

    virtual bool getDimTeraFly(const std::string & path, V3DLONG * & sz);
    virtual unsigned char* getSubVolumeTeraFly(const std::string & path, size_t x0, size_t x1, size_t y0, size_t y1, size_t z0, size_t z1);
    virtual void releaseOpenedVolumesTeraFly();

    virtual bool setImageTeraFly(size_t x, size_t y, size_t z);

	virtual int setSWC_noDecompose(V3dR_MainWindow* window, const char* fileName);
	virtual bool hideSWC(V3dR_MainWindow* window, int treeIndex);
	virtual bool displaySWC(V3dR_MainWindow* window, int treeIndex);
	virtual QList<NeuronTree> loadedNeurons(V3dR_MainWindow* window, QList<string>& loadedSurfaces);

    virtual v3dhandle getTeraflyCommunicator();
    virtual void syncAddManySegs(std::vector<V_NeuronSWC> segs);
};

#endif
