//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#ifndef PMAIN_GUI_H
#define PMAIN_GUI_H


#include "renderer_gl2.h"

#include <QtGui>
#include <v3d_interface.h>
#include "../control/CPlugin.h"
#include "../control/CVolume.h"
#include "../control/CViewer.h"
#include "PDialogImport.h"
#include "v3dr_glwidget.h"
#include "QArrowButton.h"
#include "QHelpBox.h"
#include "QGradientBar.h"
#include "QGLRefSys.h"
#include "PDialogVirtualPyramid.h"
#include "PTabVolumeInfo.h"
#ifdef __ALLOW_VR_FUNCS__
#include "fileserver.h"



/*----------------collaborate mdoe-------------------*/
class ManageSocket;
class V3dR_Communicator;
/*---------------------------------------------------*/
#endif
class terafly::PMain : public QWidget
{
    Q_OBJECT

	protected: //20170623 RZC: change from private for merging mozak
	//private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PMain* uniqueInstance;
        PMain(){    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);   }
        PMain(V3DPluginCallback2 *callback, QWidget *parent);

        //members
        V3DPluginCallback2* V3D_env;    //handle of V3D environment
        QWidget *parentWidget;          //handle of parent widget
        string annotationsPathLRU;      //last recently used (LRU) annotations filepath

        // menu bar
        QMenuBar* menuBar;              //Menu bar

        // "File" menu widgets
        QMenu* fileMenu;                        //"File" menu
        QAction* openTeraFlyVolumeAction;       //"Open TeraFly image" menu action
        QAction* openHDF5VolumeAction;          //"Open HDF5 image" menu action
        QMenu*   openUnconvertedVolumeMenu;     //"Open unconverted image" menu
        QAction* openUnconvertedVolumeFileAction;       //"Browse for file..." action
        QAction* openUnconvertedVolumeFolderAction;     //"Browse for dir..." action
        QAction* openUnstitchedImageAction;             //"Open unstitched image" menu
        QAction* closeVolumeAction;     //"Close volume" menu action
        //QAction* saveandchangetype;      //"Change current block types" mean action
        //QAction* returntochangedtype;
        QAction* loadAnnotationsAction; //"Load annotations" menu action
        QAction* saveAnnotationsAction; //"Save annotations" menu action
        QAction* saveAnnotationsAfterRemoveDupNodesAction;//"save annotations after removing duplicated nodes
        QAction* saveAnnotationsAsAction; //"Save annotations as" menu action
        QAction* clearAnnotationsAction;//"Clear annotations" menu action
        QAction* exitAction;            //"Exit" menu action

        // "Options" menu widgets
        QMenu* optionsMenu;             //"Options" menu
        // ---- import menu level ------------------- 2
        QMenu* importOptionsMenu;       //"Import" menu level 2
        QAction *regenMData_cAction;    // if active, metadata will be regenerated
        QAction *regenVMap_cAction;     // if active, volume map is regenerated
        // ---- annotation menu level --------------- 2
        QMenu* annotationMenu;          //"Annotation" menu level 2
        // ---- curves menu level ------------------- 3
        QMenu* curvesMenu;              //"Curves" menu level 3
        QMenu* curveDimsMenu;           //"Curve dims" menu level 4
        QWidgetAction* curveDimsWidget; //"Curve dims" menu action widget
        QSpinBox* curveDimsSpinBox;     //"Curve dims" spinbox
        QMenu* curveAspectMenu;         //"Curve aspect" menu level 4
        QAction* curveAspectTube;       //"Tube" action
        QAction* curveAspectSkeleton;   //"Skeleton" action
        // ---- markers menu level ------------------ 3
        QMenu* markersMenu;                             //"Markers" menu level 3
        // ---- markers size menu level ------------- 4
        QMenu* markersSizeMenu;                         //"Size" menu level 4
        QWidgetAction* markersSizeWidget;               //"Size" menu action widget
        QSpinBox* markersSizeSpinBox;                   //"Size" spinbox
        // ---- markers show menu level ------------- 4
        QMenu* markersShowROIMarginMenu;                //"Virtual margin size" menu level 5
        QWidgetAction* markersShowROIMarginWidget;      //"Virtual margin size" menu action widget
        QSpinBox* markersShowROIMarginSpinBox;          //"Virtual margin size" spinbox
        // ---- virtual space menu level ------------ 3
        QMenu* virtualSpaceSizeMenu;    //"Virtual space size" menu level 3
        QAction* spaceSizeAuto;         //"Auto" action
        QAction* spaceSizeUnlimited;    //"Unlimited" action
        // ---- navigation menu level --------------- 2
        QMenu* navigationMenu;          //"Navigation" menu level 2
        // ---- directional shifts menu level ------- 3
        QMenu* DirectionalShiftsMenu;   //"Directional shifts" menu level 3
        QMenu* xShiftMenu;              // x-shift entry
        QWidgetAction* xShiftWidget;    // x-shift action
        QSpinBox *xShiftSBox;           // x-shift widget (a spinbox)
        QMenu* yShiftMenu;              // y-shift entry
        QWidgetAction* yShiftWidget;    // y-shift action
        QSpinBox *yShiftSBox;           // y-shift widget (a spinbox)
        QMenu* zShiftMenu;              // z-shift entry
        QWidgetAction* zShiftWidget;    // z-shift action
        QSpinBox *zShiftSBox;           // z-shift widget (a spinbox)
        QMenu* tShiftMenu;              // t-shift entry
        QWidgetAction* tShiftWidget;    // t-shift action
        QSpinBox *tShiftSBox;           // t-shift widget (a spinbox)
        // ---- directional shifts menu level ------- 3
        QMenu* fetchDisplayMenu;        //"Fetch-and-display" menu level 3
        QAction* fdPreviewAction;       //"Preview/streaming" checkbox
        QAction* fdDirectAction;        //"Direct" action
        // ---- conversions menu level -------------- 2
        QMenu* conversionsMenu;         //"Conversions" menu level 2
        QMenu* from8bitsdataMenu;       //"from 8 bits data" menu level 3
        QMenu* from16bitsdataMenu;      //"from 16 bits data" menu level 3
        QWidgetAction* from8bitsdataActionWidget;   // "from 8 bits data" action
        QComboBox *from8bitsdataCBox;               // "from 8 bits data" combobox
        QWidgetAction* from16bitsdataActionWidget;  // "from 16 bits data" action
        QComboBox *from16bitsdataCBox;              // "from 16 bits data" combobox

        // "Utility" menu widgets
        QMenu* utilityMenu;
        QAction* convertVtk2APO;
        QAction* convertMaMuT2APO;
        QAction* diffAPO;
        QAction* displayAnoOctree;      // display annotation Octree
        QAction* trimAPO;
        QAction* diffnAPO;
        QAction* typeIandTypeIIerrorsAPO;
        QAction* mergeImageJCellCounterXMLs;
        QAction* countMarkersDuplicates;
        QAction* labelDuplicateAPO;
        QMenu* generateTimeSeries;
        QAction* generateTimeSeriesInterpolation;
        QAction* generateTimeSeriesDataReplication;

        // "Help" menu widgets
        QMenu* helpMenu;                //"Help" menu
        QAction* aboutAction;           //"About" menu action
        QMenu *recentVolumesMenu;
        QAction* clearRecentVolumesAction;

        // "Debug" menu widgets
        QMenu* debugMenu;               //"Debug" menu for debugging purposes
        QAction* debugAction1;          //debug menu action #1
        QAction* debugShowLogAction;    //debug menu action "Show log"
        QMenu* debugStreamingStepsMenu;                    // streaming steps entry
        QWidgetAction* debugStreamingStepsActionWidget;    // streaming steps action
        QSpinBox *debugStreamingStepsSBox;                 // streaming steps widget (a spinbox)
        QMenu* debugVerbosityMenu;                         // verbosity entry
        QWidgetAction* debugVerbosityActionWidget;         // verbosity action
        QComboBox *debugVerbosityCBox;                     // verbosity widget (a combobox)
        QMenu* debugRedirectSTDoutMenu;                    // redirect stdout entry
        QWidgetAction* debugRedirectSTDoutActionWidget;    // redirect stdout action
        QLineEdit *debugRedirectSTDoutPath;                // redirect stdout widget (a line edit)
        QAction* addGaussianNoiseToTimeSeries;             // add gaussian noise to time series action

        //toolbar widgets
        QToolBar* toolBar;                                  //tool bar with buttons
        QToolButton *openVolumeToolButton;                  //tool button for volume opening
        QToolButton *showToolbarButton;                     //show/hide toolbar


        QTabWidget *tabs;               //tab widget
        //Page "Volume's info": contains informations of the loaded volume
        PTabVolumeInfo* info_page;

        //Page "Controls": contains navigation controls
        QWidget* controls_page;

        // mini-map page
        QWidget* minimap_page;

        /* ------- local viewer panel widgets ------- */
        QGroupBox* localViewer_panel;
        QGradientBar* gradientBar;
        QSpinBox* Vdim_sbox;
        QSpinBox* Hdim_sbox;
        QSpinBox* Ddim_sbox;
        QSpinBox* Tdim_sbox;
        QComboBox* resolution_cbox;
#ifdef __ALLOW_VR_FUNCS__
		QPushButton* teraflyVRView;
		QPushButton* collaborationVRView;
#endif
        /* ------- zoom options panel widgets ------- */
        QGroupBox* zoom_panel;
        QSlider* cacheSens;
        QSlider* zoomInSens;
        QSlider* zoomOutSens;
        QPushButton* controlsResetButton;
        QComboBox* zoomInMethod;
        QComboBox* zoomOutMethod;
        /* ------- global coord panel widgets ------- */
        QGroupBox* VOI_panel;
        QArrowButton* traslXpos;
        QLabel* traslXlabel;
        QArrowButton* traslXneg;
        QArrowButton* traslYpos;
        QLabel* traslYlabel;
        QArrowButton* traslYneg;
        QArrowButton* traslZpos;
        QLabel* traslZlabel;
        QArrowButton* traslZneg;
        QArrowButton* traslTpos;
        QLabel* traslTlabel;
        QArrowButton* traslTneg;
        QSpinBox* V0_sbox;
        QSpinBox* V1_sbox;
        QSpinBox* H0_sbox;
        QSpinBox* H1_sbox;
        QSpinBox* D0_sbox;
        QSpinBox* D1_sbox;
        QLineEdit* T0_sbox;
        QLineEdit* T1_sbox;
        QLabel* to_label_1;
        QLabel* to_label_2;
        QLabel* to_label_3;
        QLabel* to_label_4;
        QGLRefSys* refSys;              //interactive 3D-based reference system
        QLineEdit* frameCoord;
        /* ------- Proofreading (PR) panel widgets ------- */
        QGroupBox* PR_panel;
        QPushButton* PR_button;
        QSpinBox* PR_spbox;

        // voxel size
        QGroupBox* VoxelSize;
        QDoubleSpinBox* x_dsb;
        QDoubleSpinBox* y_dsb;
        QDoubleSpinBox* z_dsb;

        /* ------- Overview panel widgets ------- */
        QGroupBox* Overview_panel;

        QHBoxLayout* refSysContainerLayout;
        QVBoxLayout* refSysContainerLayout2;


        //other widgets
        QHelpBox* helpBox;              //help box
        QProgressBar* progressBar;      //progress bar
        QStatusBar* statusBar;          //status bar

        QCheckBox *checkBox_overview;  //added an overview checkbox by ZZ 04262018
        QCheckBox *lockMagnification;

        //layout
        int marginLeft;                 //width of first column containing labels only

        QString recentlyUsedPath;       // load and save annotations in most recently used folder

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PMain* instance(V3DPluginCallback2 *callback, QWidget *parent);
        static PMain* getInstance();
        static void uninstance();
        static bool isInstantiated(){return uniqueInstance != 0;}
        ~PMain();

        //GET and SET methods
        V3DPluginCallback2* getV3D_env(){return V3D_env;}
        QProgressBar* getProgressBar(){return progressBar;}
        QStatusBar* getStatusBar(){return statusBar;}
        std::string getAnnotationPathRLU(){return annotationsPathLRU;}

        //resets progress bar, start/stop buttons and tab bar
        void resetGUI();

        //reset everything
        void reset();

        //overrides closeEvent method of QWidget
        void closeEvent(QCloseEvent *evt);

        //enables / disables directional shift controls
        void setEnabledDirectionalShifts(bool enabled);

        //gets proofreading active flag
        bool isPRactive(){return PR_button->text().compare("Stop") == 0;}
        void PRsetActive(bool active);

        /**********************************************************************************
        * Filters events generated by the widgets to which a help message must be associated
        ***********************************************************************************/
        bool eventFilter(QObject *object, QEvent *event);

        /**********************************************************************************
        * Displays tooltip when ToolTip, MouseMove or KeyPress events occur on the widget.
        ***********************************************************************************/
        static void displayToolTip(QWidget* widget, QEvent* event, string msg);

        /**********************************************************************************
        * Called by algorithms running from different threads.
        * Emits <sendProgressBarChanged> signal
        ***********************************************************************************/
        void emitProgressBarChanged(int val, int minutes, int seconds, const char* message = 0)
        {emit sendProgressBarChanged(val, minutes, seconds, message);}

        //<CViewer> instances need to access to all members of the current class
        friend class CViewer;
        friend class PAnoToolBar;
        friend class PDialogProofreading;
        friend class PDialogVirtualPyramid;
        friend class PTabVolumeInfo;
        friend class myV3dR_GLWidget;

        //help texts
        static string HTwelcome;
        static string HTbase;
        static string HTvoiDim;
        static string HTjumpToRes;
        static string HTcacheSens;
        static string HTzoomInThres;
        static string HTzoomOutThres;
        static string HTtraslatePos;
        static string HTtraslateNeg;
        static string HTvolcuts;
        static string HTzoomInMethod;
        static string HTrefsys;
        static string HTresolution;
        static string HTproofreading;
        static string HTquickscan;

		bool resumeVR;
        bool isOverviewActive;
        bool annotationChanged;
        bool isMagnificationLocked;

        bool cleanOldAutosavedFiles; // e.g. longer than 24 hours

#ifdef _NEURON_ASSEMBLER_
		/****************** Fragment tracing related *****************/
		// MK, Sep, 2019
		bool fragTracePluginInstance;
		QPluginLoader* FragTracerQPluginPtr;
		V3d_PluginLoader* FragTracerPluginLoaderPtr;
		IPMain4NeuronAssembler* NeuronAssemblerPortal;
		bool xLockStatus, yLockStatus, zLockStatus;
		int getCViewerID();
		/*************************************************************/
#endif

    public slots:

        /**********************************************************************************
        * Unified method for all "Open ..." UI actions
        ***********************************************************************************/
        void openImage(std::string path = "");

        /**********************************************************************************
        * Called when a path in the "Recent volumes" menu is selected.
        ***********************************************************************************/
        void openRecentVolume();

        /**********************************************************************************
        * Called when "Clear menu" action in "Recent volumes" menu is triggered.
        ***********************************************************************************/
        void clearRecentVolumes();

        /**********************************************************************************
        * Called when "Close volume" menu action is triggered.
        * All the memory allocated is released and GUI is reset".
        ***********************************************************************************/
        void closeVolume();

        /**********************************************************************************
        * Called when "Open annotations" menu action is triggered.
        * Opens QFileDialog to select annotation file path.
        ***********************************************************************************/
        void loadAnnotations();

        /**********************************************************************************
        * Called when "Save annotations" or "Save annotations as" menu actions are triggered.
        * If required, opens QFileDialog to select annotation file path.
        ***********************************************************************************/
        void saveAnnotations();
        void saveAnnotationsAs();
        void autosaveAnnotations();
        void saveAnnotationsAfterRemoveDupNodes();
        //void changecurrentblocktype();
        //void return_to_the_changedtype();


        /**********************************************************************************
        * Called when "Clear annotations" menu action is triggered.
        ***********************************************************************************/
        void clearAnnotations();

        /**********************************************************************************
        * Called when "Exit" menu action is triggered or TeraFly window is closed.
        ***********************************************************************************/
        void exit();

        /********************************Overview_panel**************************************************
        * Called when "Help->About" menu action is triggered
        ***********************************************************************************/
        void about();

        /**********************************************************************************
        * Called when controlsResetButton is clicked
        ***********************************************************************************/
        void resetMultiresControls();

        /**********************************************************************************
        * Called by <CImport> when the associated operation has been performed.
        * If an exception has occurred in the <CImport> thread,  it is propagated and man-
        * aged in the current thread (ex != 0). Otherwise, volume information are imported
        * in the GUI by the <StackedVolume> handle of <CImport>.
        ***********************************************************************************/
        void importDone(tf::RuntimeException *ex, qint64 elapsed_time = 0);

        /**********************************************************************************
        * Called when the GUI widgets that control application settings change.
        * This is used to manage persistent platform-independent application settings.
        ***********************************************************************************/
        void settingsChanged(int);
        void voxelSizeChanged(double);

        /**********************************************************************************
        * Linked to resolution combobox
        * This switches to the given resolution index.
        ***********************************************************************************/
        void resolutionIndexChanged(int i);

        /**********************************************************************************
        * Called when the corresponding buttons are clicked
        ***********************************************************************************/
        void traslXposClicked();
        void traslXnegClicked();
        void traslYposClicked();
        void traslYnegClicked();
        void traslZposClicked();
        void traslZnegClicked();
        void traslTposClicked();
        void traslTnegClicked();

        /**********************************************************************************
        * Called when the corresponding debug actions are triggered
        ***********************************************************************************/
        void debugAction1Triggered();
        void addGaussianNoiseTriggered();
        void showLogTriggered();

        /**********************************************************************************
        * Called when the corresponding Options->3D annotation->Curve actions are triggered
        ***********************************************************************************/
        void curveDimsChanged(int dim);
        void curveAspectChanged();

        /**********************************************************************************
        * Called when the corresponding Options->3D annotation->Virtual space size actions are triggered
        ***********************************************************************************/
        void virtualSpaceSizeChanged();

        /**********************************************************************************
        * Called when the corresponding Options->Navigation->Fetch-and-Display actions are triggered
        ***********************************************************************************/
        void fetchAndDisplayChanged();

        /**********************************************************************************
        * Linked to verbosity combobox
        ***********************************************************************************/
        void verbosityChanged(int i);

        //very useful (not included in Qt): disables the given item of the given combobox
        static void setEnabledComboBoxItem(QComboBox* cbox, int _index, bool enabled);

        /**********************************************************************************
        * Called when the corresponding buttons are clicked
        ***********************************************************************************/
        void PRbuttonClicked();

        /**********************************************************************************
        * Called when the corresponding buttons are clicked
        ***********************************************************************************/
        void PRstart();

        /**********************************************************************************
        * Called when the corresponding spin box has changed
        ***********************************************************************************/
        void PRblockSpinboxChanged(int b);

        /**********************************************************************************
        * Called when the corresponding spin box has been edited
        ***********************************************************************************/
        void PRblockSpinboxEditingFinished();

        /**********************************************************************************
        * Called when the corresponding QLineEdit has been edited
        ***********************************************************************************/
        void debugRedirectSTDoutPathEdited(QString s);

        /**********************************************************************************
        * <sendProgressBarChanged> event handler
        ***********************************************************************************/
        void progressBarChanged(int val, int minutes, int seconds, const char* message);

        /**********************************************************************************
        * Called when showToolbarButton state has changed
        ***********************************************************************************/
        void showToolbarButtonChanged(bool changed);


        /**********************************************************************************
        * Called when markersShowROIMarginSpinBox state has changed
        ***********************************************************************************/
        void markersShowROIMarginSpinBoxChanged(int value);

        /**********************************************************************************
        * Called when markersSizeSpinBox state has changed
        ***********************************************************************************/
        void markersSizeSpinBoxChanged(int value);

        /**********************************************************************************
        * Called when annotations (markers, neuron trees, etc.) have changed
        ***********************************************************************************/
        void annotationsChanged();

        /**********************************************************************************
        * Called when bit conversion menu options have changed
        ***********************************************************************************/
        void from8bitsdataChanged(int);
        void from16bitsdataChanged(int);

        void tabIndexChanged(int value);

        void showDialogVtk2APO();
        void showDialogMaMut2APO();
        void showDialogDiffAPO();
        void showDialogTrimAPO();
        void showDialogDiffnAPO();
        void showDialogTypeIandTypeIIerrors();
        void showDialogMergeImageJCellCounterXMLs();
        void showDialogLabelDuplicateAPO();
        void showDialogCountDuplicateMarkers();
        void showDialogGenerateTimeSeriesInterpolation();
        void showDialogGenerateTimeSeriesReplication();
        void showAnoOctree();
#ifdef __ALLOW_VR_FUNCS__
		void doTeraflyVRView();
		void doCollaborationVRView();
        void teraflyShiftClickedinVR(int _direction);
#endif
        void setOverview(bool enable);
        void updateOverview();
        void updateAnnotationStatus();

        void setLockMagnification(bool locked);


    signals:

        /*********************************************************************************
        * Carries progress bar informations (progress percentage and remaining minutes).
        **********************************************************************************/
        void sendProgressBarChanged(int val, int minutes, int seconds, const char* message);
#ifdef __ALLOW_VR_FUNCS__
/*----------------collaborate mdoe-------------------*/
public:
        ManageSocket * managesocket;
protected:
        QMenu* collaborateMenu;
        QAction* loginAction;
        QAction* logoutAction;
        QAction* importAction;
        QAction* downAction;
        QAction* loadAction;
public slots:
        void login();
        void logout();
        void import();
        void download();
        void load();
        void deleteManageSocket();
public slots:
        void ColLoadANO(QString ANOfile);
        //V3dR_Communicator *TeraflyCommunicator;  move to v3dr_glwidget.h
/*---------------------------------------------------*/
#endif
};

#endif // PMAIN_GUI_H
