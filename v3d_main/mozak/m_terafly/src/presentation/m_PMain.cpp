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

#include "m_PMain.h"
#include "m_PDialogImport.h"
#include "m_PAbout.h"
#include "m_PLog.h"
#include "m_PAnoToolBar.h"
#include "m_PDialogProofreading.h"
#include "m_QPixmapToolTip.h"
#include "m_CImport.h"
#include "m_CVolume.h"
#include "m_CSettings.h"
#include "m_CViewer.h"
#include "m_CAnnotations.h"
#include "m_V3Dsubclasses.h"
#include "m_CAnnotations.h"
#include "m_CImageUtils.h"
#include "m_COperation.h"
#include "renderer_gl1.h"
#include "v3dr_mainwindow.h"
#include <typeinfo>
#include "TimeSeries.h"
#include "BDVVolume.h"
#include <QtGlobal>
#include <cmath>
#include "VolumeConverter.h"
#include "TiledMCVolume.h"

using namespace teramanager;
using namespace iim;

string PMain::HTwelcome = "Go to <i>File > Open volume</i> and select From the file dialog, select any of volume resolutions starting with \"RES\". To change volume import options, go to <i>Options > Import</i>.";
string PMain::HTbase =    "<b>What's this?</b><br><i>Move the mouse over an object and its description will be displayed here.</i>";
string PMain::HTvoiDim =  "Set the 3D viewer <b>dimensions</b> (voxels). The larger, the more graphic card memory is used and the slower is the visualization. Suggested range: [100,300] for x-y-z, [1-10] for t.";
string PMain::HTjumpToRes = "Choose from pull-down menu the <b>resolution</b> at which you want to visualize the current volume of interest (VOI). "
                            "You can change the VOI using the Vaa3D <i>Volume Cut</i> scrollbars or the <i>VOI's coordinates</i> spinboxes of TeraFly.";
string PMain::HTzoomOutThres = "Select the <b>zoom</b> factor threshold to change the image resolution when zooming-out with mouse scroll. The default is 0. "
                           "Set it to -100 to disable this feature.";
string PMain::HTzoomInThres = "Select the <b>zoom</b> factor threshold to change the image resolution when zooming-in with mouse scroll. The default is 50. "
                             "Set it to 100 to disable this feature.";
string PMain::HTzoomInMethod = "Choose from pull-down menu the method used to generate the volume of interest (VOI) when zooming-in with the mouse scroll";
string PMain::HTcacheSens = "Adjust data caching sensitivity when zooming-in. This controls the minimum amount of overlap between the requested VOI "
                            " and the <b>cached VOI</b> that is required to restore the cached VOI instead of loading a new VOI from the storage. If you always want to zoom-in to the cached VOI, please set this to 0\%.";
string PMain::HTtraslatePos = "Translate the view along this axis in its <i>natural</i> direction.";
string PMain::HTtraslateNeg = "Translate the view along this axis in its <i>opposite</i> direction.";
string PMain::HTvolcuts = "Define a volume of interest (<b>VOI</b>) using <b>absolute spatial coordinates</b> (i.e. referred to the highest resolution). "
                          "You may then choose the resolution you want to display the VOI from the <i>Resolution</i> pull-down menu.";
string PMain::HTrefsys = "Rotate the reference system";
string PMain::HTresolution = "A heat map like bar that indicates the currently displayed image resolution (the \"hotter\", the higher)";
string PMain::HTproofreading = "Start a stoppable/resumable block-by-block scan of the entire volume (or a VOI) to proofread automatic cell counts or neuron reconstructions.";
string PMain::HTquickscan = "<i>QuickScan</i>: a scrollable maximum-intensity-projection-based preview to roughly check hundreds of blocks per minute and load only the nonempty ones.";

PMain* PMain::uniqueInstance = 0;
PMain* PMain::instance(V3DPluginCallback2 *callback, QWidget *parent)
{
    printf("instance\n");
    if (uniqueInstance == 0)
        uniqueInstance = new PMain(callback, parent);
    else
    {
        uniqueInstance->setWindowState(Qt::WindowNoState);
        uniqueInstance->raise();
        uniqueInstance->activateWindow();
        uniqueInstance->show();
        if(CViewer::getCurrent())
        {
            CViewer::getCurrent()->window3D->setWindowState(Qt::WindowNoState);
            CViewer::getCurrent()->window3D->raise();
            CViewer::getCurrent()->window3D->activateWindow();
            CViewer::getCurrent()->window3D->show();
            CViewer::getCurrent()->alignToLeft(uniqueInstance, 0);
        }
    }
	return uniqueInstance;
}
PMain* PMain::getInstance()
{
    if(uniqueInstance)
        return uniqueInstance;
    else
    {
        itm::warning("TeraFly not yet instantiated", __itm__current__function__);
        QMessageBox::critical(0,QObject::tr("Error"), QObject::tr("TeraFly not yet instantiated"),QObject::tr("Ok"));
		return 0;
    }
}

void PMain::uninstance()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CImport::uninstance();
    PDialogImport::uninstance();
    PAbout::uninstance();
    CVolume::uninstance();
    CViewer::uninstance();
    CSettings::uninstance();
    CAnnotations::uninstance();
    PLog::uninstance();
    PAnoToolBar::uninstance();
    PDialogProofreading::uninstance();
    QPixmapToolTip::uninstance();
    if(uniqueInstance)
        delete uniqueInstance;
    uniqueInstance = 0;
}

PMain::~PMain()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);
}

PMain::PMain(V3DPluginCallback2 *callback, QWidget *parent) : QWidget(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    //initializing members
    V3D_env = callback;
    parentWidget = parent;
    annotationsPathLRU = "";
    marginLeft = 65;

    //creating fonts
    QFont tinyFont = QApplication::font();
    #ifdef Q_OS_LINUX
    tinyFont.setPointSize(9);
    #endif

    //initializing menu
    /**/itm::debug(itm::LEV3, "initializing menu", __itm__current__function__);
    menuBar = new QMenuBar(0);
    /* --------------------------- "File" menu --------------------------- */
    fileMenu = menuBar->addMenu("File");
    openTeraFlyVolumeAction = new QAction("Open TeraFly volume", this);
    openTeraFlyVolumeAction->setIcon(QIcon(":/icons/open_volume.png"));
    openHDF5VolumeAction = new QAction("Open HDF5 volume", this);
    openHDF5VolumeAction->setIcon(QIcon(":/icons/import.png"));
    closeVolumeAction = new QAction("Close volume", this);
    closeVolumeAction->setIcon(QIcon(":/icons/close.png"));
    loadAnnotationsAction = new QAction("Load annotations", this);
    loadAnnotationsAction->setIcon(QIcon(":/icons/open_ano.png"));
    saveAnnotationsAction = new QAction("Save annotations", this);
    saveAnnotationsAction->setIcon(QIcon(":/icons/save.png"));
    saveAnnotationsAsAction = new QAction("Save annotations as", this);
    saveAnnotationsAsAction->setIcon(QIcon(":/icons/saveas.png"));
    clearAnnotationsAction = new QAction("Clear annotations", this);
    clearAnnotationsAction->setIcon(QIcon(":/icons/clear.png"));
    exitAction = new QAction("Quit", this);
    openTeraFlyVolumeAction->setShortcut(QKeySequence("Ctrl+O"));
    openHDF5VolumeAction->setShortcut(QKeySequence("Ctrl+H"));
    closeVolumeAction->setShortcut(QKeySequence("Ctrl+C"));
    loadAnnotationsAction->setShortcut(QKeySequence("Ctrl+L"));
    saveAnnotationsAction->setShortcut(QKeySequence("Ctrl+S"));
    saveAnnotationsAsAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    clearAnnotationsAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(openTeraFlyVolumeAction, SIGNAL(triggered()), this, SLOT(openTeraFlyVolume()));
    connect(openHDF5VolumeAction, SIGNAL(triggered()), this, SLOT(openHDF5Volume()));
    connect(closeVolumeAction, SIGNAL(triggered()), this, SLOT(closeVolume()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(exit()));
    connect(loadAnnotationsAction, SIGNAL(triggered()), this, SLOT(loadAnnotations()));
    connect(saveAnnotationsAction, SIGNAL(triggered()), this, SLOT(saveAnnotations()));
    connect(saveAnnotationsAsAction, SIGNAL(triggered()), this, SLOT(saveAnnotationsAs()));
    connect(clearAnnotationsAction, SIGNAL(triggered()), this, SLOT(clearAnnotations()));
    fileMenu->addAction(openTeraFlyVolumeAction);
    fileMenu->addAction(openHDF5VolumeAction);
    recentVolumesMenu = new QMenu("Recent volumes");
    recentVolumesMenu->setIcon(QIcon(":/icons/open_volume_recent.png"));
    fileMenu->addMenu(recentVolumesMenu);
    fileMenu->addAction(closeVolumeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(loadAnnotationsAction);
    fileMenu->addAction(saveAnnotationsAction);
    fileMenu->addAction(saveAnnotationsAsAction);
    fileMenu->addAction(clearAnnotationsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    /* ------------------------- "Options" menu -------------------------- */
    optionsMenu = menuBar->addMenu("Options");
    /* ------------------------- "Options" menu: Import ------------------ */
    importOptionsMenu = optionsMenu->addMenu("Import");
    regenMData_cAction = new QAction("Regenerate metadata", this);
    regenMData_cAction->setCheckable(true);
    importOptionsMenu-> addAction(regenMData_cAction);
    regenVMap_cAction = new QAction("Regenerate volume map", this);
    regenVMap_cAction->setCheckable(true);
    importOptionsMenu-> addAction(regenVMap_cAction);
    /* ------------------------- "Options" menu: Annotation ---------------------- */
    annotationMenu = optionsMenu->addMenu("Annotations");
    markersMenu = annotationMenu->addMenu("Markers");
    curvesMenu = annotationMenu->addMenu("Curves");
    curveAspectMenu = curvesMenu->addMenu("Aspect");
    curveDimsMenu = curvesMenu->addMenu("Skeleton width");
    curveAspectTube = new QAction("Tube", this);
    curveAspectSkeleton = new QAction("Skeleton", this);
    curveAspectTube->setCheckable(true);
    curveAspectSkeleton->setCheckable(true);
    QActionGroup* curveAspectMutex = new QActionGroup(this);
    curveAspectMutex->addAction(curveAspectTube);
    curveAspectMutex->addAction(curveAspectSkeleton);
    curveAspectMutex->setExclusive(true);
    curveAspectMenu->addAction(curveAspectTube);
    curveAspectMenu->addAction(curveAspectSkeleton);
    curveDimsWidget = new QWidgetAction(this);
    curveDimsSpinBox = new QSpinBox();
    curveDimsSpinBox->setMinimum(1);
    curveDimsSpinBox->setMaximum(10);
    curveDimsSpinBox->setSuffix(" (pixels)");
    curveDimsWidget->setDefaultWidget(curveDimsSpinBox);
    curveDimsMenu->addAction(curveDimsWidget);
    curveAspectTube->setChecked(CSettings::instance()->getAnnotationCurvesAspectTube());
    curveAspectSkeleton->setChecked(!curveAspectTube->isChecked());
    curveDimsSpinBox->setValue(CSettings::instance()->getAnnotationCurvesDims());
    connect(curveAspectTube, SIGNAL(changed()), this, SLOT(curveAspectChanged()));
    //connect(curveAspectSkeleton, SIGNAL(changed()), this, SLOT(curveAspectChanged()));
    connect(curveDimsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(curveDimsChanged(int)));
    virtualSpaceSizeMenu = annotationMenu->addMenu("Virtual space size");
    spaceSizeAuto = new QAction("Auto", this);
    spaceSizeUnlimited = new QAction("Unlimited", this);
    spaceSizeAuto->setCheckable(true);
    spaceSizeUnlimited->setCheckable(true);
    QActionGroup* virtualSpaceSizeMutex = new QActionGroup(this);
    virtualSpaceSizeMutex->addAction(spaceSizeAuto);
    virtualSpaceSizeMutex->addAction(spaceSizeUnlimited);
    virtualSpaceSizeMutex->setExclusive(true);
    virtualSpaceSizeMenu->addAction(spaceSizeAuto);
    virtualSpaceSizeMenu->addAction(spaceSizeUnlimited);
    spaceSizeUnlimited->setChecked(CSettings::instance()->getAnnotationSpaceUnlimited());
    spaceSizeAuto->setChecked(!spaceSizeUnlimited->isChecked());
    connect(spaceSizeAuto, SIGNAL(changed()), this, SLOT(virtualSpaceSizeChanged()));
    //connect(spaceSizeUnlimited, SIGNAL(changed()), this, SLOT(virtualSpaceSizeChanged()));
    /**/
    markersSizeMenu = markersMenu->addMenu("Size");
    markersSizeWidget = new QWidgetAction(this);
    markersSizeSpinBox = new QSpinBox();
    markersSizeSpinBox->setMinimum(1);
    markersSizeSpinBox->setMaximum(100);
    markersSizeSpinBox->setSuffix(" (highrez pixels)");
    markersSizeSpinBox->setValue(CSettings::instance()->getAnnotationMarkerSize());
    markersSizeWidget->setDefaultWidget(markersSizeSpinBox);
    markersSizeMenu->addAction(markersSizeWidget);
    connect(markersSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(markersSizeSpinBoxChanged(int)));
    /**/
    markersShowROIMarginMenu = markersMenu->addMenu("VOI extra margin");
    markersShowROIMarginWidget = new QWidgetAction(this);
    markersShowROIMarginSpinBox = new QSpinBox();
    markersShowROIMarginSpinBox->setSuffix(" %");
    markersShowROIMarginSpinBox->setMaximum(100);
    markersShowROIMarginSpinBox->setMinimum(1);
    markersShowROIMarginWidget->setDefaultWidget(markersShowROIMarginSpinBox);
    markersShowROIMarginMenu->addAction(markersShowROIMarginWidget);
    markersShowROIMarginSpinBox->setValue(CSettings::instance()->getAnnotationVirtualMargin());
    connect(markersShowROIMarginSpinBox, SIGNAL(valueChanged(int)), this, SLOT(markersShowROIMarginSpinBoxChanged(int)));
    /**/
    /* ------------------------- "Options" menu: Navigation ---------------- */
    navigationMenu = optionsMenu->addMenu("Navigation");
    /* ------------------------- "Options->Navigation" menu: Directional shift ---- */
    DirectionalShiftsMenu = navigationMenu->addMenu("Directional shift");
    /* ------------------------------ x-shift ------------------------------------- */
    xShiftMenu = new QMenu("X-shift overlap");
    xShiftWidget = new QWidgetAction(this);
    xShiftSBox = new QSpinBox();
    xShiftSBox->setSuffix("%%");
    xShiftSBox->setMinimum(1);
    xShiftSBox->setMaximum(99);
    xShiftSBox->setValue(CSettings::instance()->getTraslX());
    xShiftWidget->setDefaultWidget(xShiftSBox);
    xShiftMenu->addAction(xShiftWidget);
    DirectionalShiftsMenu->addMenu(xShiftMenu);
    /* ------------------------------ y-shift ------------------------------------- */
    yShiftMenu = new QMenu("Y-shift overlap");
    yShiftWidget = new QWidgetAction(this);
    yShiftSBox = new QSpinBox();
    yShiftSBox->setSuffix("%%");
    yShiftSBox->setMinimum(1);
    yShiftSBox->setMaximum(99);
    yShiftSBox->setValue(CSettings::instance()->getTraslY());
    yShiftWidget->setDefaultWidget(yShiftSBox);
    yShiftMenu->addAction(yShiftWidget);
    DirectionalShiftsMenu->addMenu(yShiftMenu);
    /* ------------------------------ z-shift ------------------------------------- */
    zShiftMenu = new QMenu("Z-shift overlap");
    zShiftWidget = new QWidgetAction(this);
    zShiftSBox = new QSpinBox();
    zShiftSBox->setSuffix("%%");
    zShiftSBox->setMinimum(1);
    zShiftSBox->setMaximum(99);
    zShiftSBox->setValue(CSettings::instance()->getTraslZ());
    zShiftWidget->setDefaultWidget(zShiftSBox);
    zShiftMenu->addAction(zShiftWidget);
    DirectionalShiftsMenu->addMenu(zShiftMenu);
    /* ------------------------------ t-shift ------------------------------------- */
    tShiftMenu = new QMenu("T-shift overlap");
    tShiftWidget = new QWidgetAction(this);
    tShiftSBox = new QSpinBox();
    tShiftSBox->setSuffix("%%");
    tShiftSBox->setMinimum(0);
    tShiftSBox->setMaximum(99);
    tShiftSBox->setValue(CSettings::instance()->getTraslT());
    tShiftWidget->setDefaultWidget(tShiftSBox);
    tShiftMenu->addAction(tShiftWidget);
    DirectionalShiftsMenu->addMenu(tShiftMenu);
    /* ------------------------- "Options->Navigation" menu: Fetch-and-Display ---- */
    fetchDisplayMenu = navigationMenu->addMenu("Fetch-and-Display");
    fdPreviewAction = new QAction("Preview/streaming", this);
    fdDirectAction = new QAction("Direct", this);
    fdPreviewAction->setCheckable(true);
    fdDirectAction->setCheckable(true);
    QActionGroup* fdMutex = new QActionGroup(this);
    fdMutex->addAction(fdPreviewAction);
    fdMutex->addAction(fdDirectAction);
    fdMutex->setExclusive(true);
    fetchDisplayMenu->addAction(fdPreviewAction);
    fetchDisplayMenu->addAction(fdDirectAction);
    fdPreviewAction->setChecked(CSettings::instance()->getPreviewMode());
    fdDirectAction->setChecked(!fdPreviewAction->isChecked());
    connect(fdPreviewAction, SIGNAL(changed()), this, SLOT(fetchAndDisplayChanged()));



    // "Utility" Menu
    utilityMenu = menuBar->addMenu("Utilities");
    QMenu* utilsAnno = utilityMenu->addMenu("Annotations");
    QMenu* utilsMarkers = utilsAnno->addMenu("Markers");
    QMenu* utilsConvert = utilsMarkers->addMenu("Convert");
    QMenu* utilsAnalyze = utilsMarkers->addMenu("Analyze");
    QMenu* timeseriesMenu = utilityMenu->addMenu("Time-series");


    countMarkersDuplicates = new QAction("count duplicates in the whole image", this);
    connect(countMarkersDuplicates, SIGNAL(triggered()), this, SLOT(showDialogCountDuplicateMarkers()));
    utilsAnalyze->addAction(countMarkersDuplicates);
    typeIandTypeIIerrorsAPO = new QAction("count type I/II errors from two .apo files", this);
    connect(typeIandTypeIIerrorsAPO, SIGNAL(triggered()), this, SLOT(showDialogTypeIandTypeIIerrors()));
    utilsAnalyze->addAction(typeIandTypeIIerrorsAPO);
    diffAPO = new QAction("diff of two .apo files", this);
    connect(diffAPO, SIGNAL(triggered()), this, SLOT(showDialogDiffAPO()));
    utilsAnalyze->addAction(diffAPO);
    diffnAPO = new QAction("diff of multiple .apo files", this);
    connect(diffnAPO, SIGNAL(triggered()), this, SLOT(showDialogDiffnAPO()));
    utilsAnalyze->addAction(diffnAPO);
    labelDuplicateAPO = new QAction("label duplicates in .apo file", this);
    connect(labelDuplicateAPO, SIGNAL(triggered()), this, SLOT(showDialogLabelDuplicateAPO()));
    utilsAnalyze->addAction(labelDuplicateAPO);
    trimAPO = new QAction("trim .apo with VOI selection", this);
    connect(trimAPO, SIGNAL(triggered()), this, SLOT(showDialogTrimAPO()));
    utilsAnalyze->addAction(trimAPO);
    convertVtk2APO = new QAction(".vtk  (VTK) -> .apo", this);
    connect(convertVtk2APO, SIGNAL(triggered()), this, SLOT(showDialogVtk2APO()));
    utilsConvert->addAction(convertVtk2APO);
    convertMaMuT2APO = new QAction(".xml (MaMuT/BigDataViewer) -> .apo", this);
    connect(convertMaMuT2APO, SIGNAL(triggered()), this, SLOT(showDialogMaMut2APO()));
    utilsConvert->addAction(convertMaMuT2APO);
    mergeImageJCellCounterXMLs = new QAction(".xml (ImageJ Cell Counter) -> .apo", this);
    connect(mergeImageJCellCounterXMLs, SIGNAL(triggered()), this, SLOT(showDialogMergeImageJCellCounterXMLs()));
    utilsConvert->addAction(mergeImageJCellCounterXMLs);

    generateTimeSeries = new QMenu("Generate time series by");
    generateTimeSeriesDataReplication = new QAction("single-frame replication with increasing gaussian noise", this);
    connect(generateTimeSeriesDataReplication, SIGNAL(triggered()), this, SLOT(showDialogGenerateTimeSeriesReplication()));
    generateTimeSeriesInterpolation = new QAction("interpolation from frames A to B", this);
    connect(generateTimeSeriesInterpolation, SIGNAL(triggered()), this, SLOT(showDialogGenerateTimeSeriesInterpolation()));
    generateTimeSeries->addAction(generateTimeSeriesDataReplication);
    generateTimeSeries->addAction(generateTimeSeriesInterpolation);
    timeseriesMenu->addMenu(generateTimeSeries);
    displayAnoOctree = new QAction("Display annotations octree", this);
    connect(displayAnoOctree, SIGNAL(triggered()), this, SLOT(showAnoOctree()));
    utilsAnno->addAction(displayAnoOctree);


    // "Debug" menu
    debugMenu = menuBar->addMenu("Debug");
    /* --------------------------------- show log --------------------------------- */
    debugShowLogAction = new QAction("Show log", debugMenu);
    connect(debugShowLogAction, SIGNAL(triggered()), this, SLOT(showLogTriggered()));
    debugMenu->addAction(debugShowLogAction);
    /* ------------------------------ streaming steps ----------------------------- */
    debugStreamingStepsMenu = new QMenu("Streaming steps");
    debugStreamingStepsActionWidget = new QWidgetAction(this);
    debugStreamingStepsSBox = new QSpinBox();
    debugStreamingStepsSBox->setMinimum(1);
    debugStreamingStepsSBox->setMaximum(10);
    debugStreamingStepsActionWidget->setDefaultWidget(debugStreamingStepsSBox);
    debugStreamingStepsMenu->addAction(debugStreamingStepsActionWidget);
    debugMenu->addMenu(debugStreamingStepsMenu);
    /* --------------------------------- verbosity -------------------------------- */
    debugVerbosityMenu = new QMenu("Verbosity");
    debugVerbosityActionWidget = new QWidgetAction(this);
    debugVerbosityCBox = new QComboBox();
    debugVerbosityCBox->addItem("Silent mode");
    debugVerbosityCBox->addItem("Level 1");
    debugVerbosityCBox->addItem("Level 2");
    debugVerbosityCBox->addItem("Level 3");
    debugVerbosityCBox->addItem("Verbose");
    CSettings::instance()->readSettings();
    debugVerbosityActionWidget->setDefaultWidget(debugVerbosityCBox);
    debugVerbosityMenu->addAction(debugVerbosityActionWidget);
    connect(debugVerbosityCBox, SIGNAL(currentIndexChanged(int)), this, SLOT(verbosityChanged(int)));
    debugMenu->addMenu(debugVerbosityMenu);
    /* ---------------------------- redirect to stdout ---------------------------- */
    debugRedirectSTDoutMenu = new QMenu("Redirect stdout to file at");
    debugRedirectSTDoutActionWidget = new QWidgetAction(this);
    debugRedirectSTDoutPath = new QLineEdit();
    debugRedirectSTDoutActionWidget->setDefaultWidget(debugRedirectSTDoutPath);
    debugRedirectSTDoutMenu->addAction(debugRedirectSTDoutActionWidget);
    connect(debugRedirectSTDoutPath, SIGNAL(textEdited(QString)), this, SLOT(debugRedirectSTDoutPathEdited(QString)));
    debugMenu->addMenu(debugRedirectSTDoutMenu);
    /* ------------------------------ debug action 1 ------------------------------ */
    debugAction1 = new QAction("Debug action", debugMenu);
    connect(debugAction1, SIGNAL(triggered()), this, SLOT(debugAction1Triggered()));
    debugMenu->addAction(debugAction1);
    /* --------------------- add gaussian noise to time series -------------------- */
    addGaussianNoiseToTimeSeries = new QAction("Add gaussian noise to time series", debugMenu);
    addGaussianNoiseToTimeSeries->setCheckable(true);
    connect(addGaussianNoiseToTimeSeries, SIGNAL(triggered()), this, SLOT(addGaussianNoiseTriggered()));
    debugMenu->addAction(addGaussianNoiseToTimeSeries);


    // "Help" menu
    helpMenu = menuBar->addMenu("Help");
    aboutAction = new QAction("Info about TeraFly", helpMenu);
    aboutAction->setIcon(QIcon(":/icons/about.png"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    helpMenu->addAction(aboutAction);

    //toolbar
    toolBar = new QToolBar("ToolBar", this);
    toolBar->setOrientation(Qt::Horizontal);
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setIconSize(QSize(25,25));
    toolBar->setStyleSheet("QToolBar{background:qlineargradient(x1: 1, y1: 0, x2: 1, y2: 1,"
                           "stop: 0 rgb(180,180,180), stop: 1 rgb(220,220,220)); border-left: none; border-right: none; border-bottom: 1px solid rgb(150,150,150);}");

    QMenu *openMenu = new QMenu();
    std::list<string> recentVolumes = CSettings::instance()->getVolumePathHistory();
    for(std::list<string>::reverse_iterator it = recentVolumes.rbegin(); it != recentVolumes.rend(); it++)
    {
        QAction *action = new QAction(it->c_str(), this);
        connect(action, SIGNAL(triggered()), this, SLOT(openVolumeActionTriggered()));
        recentVolumesMenu->addAction(action);
    }
    clearRecentVolumesAction = new QAction("Clear menu",recentVolumesMenu);
    connect(clearRecentVolumesAction, SIGNAL(triggered()), this, SLOT(clearRecentVolumesTriggered()));
    recentVolumesMenu->addSeparator();
    recentVolumesMenu->addAction(clearRecentVolumesAction);

    openMenu->addAction(openTeraFlyVolumeAction);
    openMenu->addAction(openHDF5VolumeAction);
    openMenu->addMenu(recentVolumesMenu);
    openVolumeToolButton = new QToolButton();
    openVolumeToolButton->setMenu(openMenu);
    openVolumeToolButton->setPopupMode(QToolButton::InstantPopup);
    openVolumeToolButton->setIcon(QIcon(":/icons/open_volume.png"));
    toolBar->insertWidget(0, openVolumeToolButton);
    toolBar->insertAction(0, closeVolumeAction);
    showToolbarButton = new QToolButton();
    showToolbarButton->setCheckable(true);
    showToolbarButton->setIcon(QIcon(":/icons/toolbar.png"));
    showToolbarButton->setToolTip("Display/hide TeraFly's toolbar within the Vaa3D 3D viewer");
    connect(showToolbarButton, SIGNAL(toggled(bool)), this, SLOT(showToolbarButtonChanged(bool)));
    toolBar->insertWidget(0, showToolbarButton);
    toolBar->addAction(aboutAction);
    toolBar->layout()->setSpacing(5);

    // TAB widget: where to store pages
    tabs = new QTabWidget(this);

    //Page "Volume's info": contains informations of the loaded volume
    /**/itm::debug(itm::LEV3, "Page \"Volume's info\"", __itm__current__function__);
    info_page = new QWidget();
    vol_size_field = new QLineEdit();
    vol_size_field->setAlignment(Qt::AlignLeft);
    vol_size_field->setReadOnly(true);
    vol_size_field->setFont(tinyFont);

    vol_dims_vxl_field = new QLineEdit();
    vol_dims_vxl_field->setAlignment(Qt::AlignLeft);
    vol_dims_vxl_field->setReadOnly(true);
    vol_dims_vxl_field->setFont(tinyFont);

    vol_dims_mm_field = new QLineEdit();
    vol_dims_mm_field->setAlignment(Qt::AlignLeft);
    vol_dims_mm_field->setReadOnly(true);
    vol_dims_mm_field->setFont(tinyFont);


    tiles_grid_field = new QLineEdit();
    tiles_grid_field->setAlignment(Qt::AlignLeft);
    tiles_grid_field->setReadOnly(true);
    tiles_grid_field->setFont(tinyFont);


    tile_dim_field = new QLineEdit();
    tile_dim_field->setAlignment(Qt::AlignLeft);
    tile_dim_field->setReadOnly(true);
    tile_dim_field->setFont(tinyFont);

    voxel_dims_label = new QLabel(QString("Voxel's dims (").append(QChar(0x03BC)).append("m):"));
    vxl_field = new QLineEdit();
    vxl_field->setAlignment(Qt::AlignLeft);
    vxl_field->setReadOnly(true);
    vxl_field->setFont(tinyFont);

    org_field = new QLineEdit();
    org_field->setAlignment(Qt::AlignLeft);
    org_field->setReadOnly(true);
    org_field->setFont(tinyFont);

    //Page "Controls": contains navigation controls
    /**/itm::debug(itm::LEV3, "Page \"Controls\"", __itm__current__function__);
    /* ------- local viewer panel widgets ------- */
    controls_page = new QWidget();
    localViewer_panel = new QGroupBox("Viewer");
    gradientBar = new QGradientBar(this);
    gradientBar->installEventFilter(this);
    Vdim_sbox = new QSpinBox();
    Vdim_sbox->setAlignment(Qt::AlignCenter);
    Vdim_sbox->setMaximum(1000);
    Vdim_sbox->setValue(CSettings::instance()->getVOIdimV());
    Vdim_sbox->setSuffix("(y)");
    Vdim_sbox->setFont(tinyFont);
    Vdim_sbox->installEventFilter(this);
    Hdim_sbox = new QSpinBox();
    Hdim_sbox->setAlignment(Qt::AlignCenter);
    Hdim_sbox->setMaximum(1000);
    Hdim_sbox->setValue(CSettings::instance()->getVOIdimH());
    Hdim_sbox->setSuffix("(x)");
    Hdim_sbox->installEventFilter(this);
    Ddim_sbox = new QSpinBox();
    Ddim_sbox->setAlignment(Qt::AlignCenter);
    Ddim_sbox->setMaximum(1000);
    Ddim_sbox->setValue(CSettings::instance()->getVOIdimD());
    Ddim_sbox->setSuffix("(z)");
    Ddim_sbox->installEventFilter(this);
    Tdim_sbox = new QSpinBox();
    Tdim_sbox->setAlignment(Qt::AlignCenter);
    Tdim_sbox->setMaximum(1000);
    Tdim_sbox->setMinimum(1);
    Tdim_sbox->setValue(CSettings::instance()->getVOIdimT());
    Tdim_sbox->setSuffix("(t)");
    Tdim_sbox->installEventFilter(this);
    resolution_cbox = new QComboBox();
    resolution_cbox->installEventFilter(this);

    /* ------- zoom options panel widgets ------- */
    /**/itm::debug(itm::LEV3, "zoom options panel", __itm__current__function__);
    zoom_panel = new QGroupBox("Zoom-in/out");
    zoomOutSens = new QSlider(Qt::Horizontal, this);
    zoomOutSens->setTickPosition(QSlider::TicksBelow);
    zoomOutSens->setMinimum(-100);
    zoomOutSens->setMaximum(0);
    zoomOutSens->setSingleStep(10);
    zoomOutSens->setPageStep(20);
    zoomOutSens->installEventFilter(this);
    zoomInSens = new QSlider(Qt::Horizontal);
    zoomInSens->setTickPosition(QSlider::TicksBelow);
    zoomInSens->setMinimum(0);
    zoomInSens->setMaximum(100);
    zoomInSens->setSingleStep(10);
    zoomInSens->setPageStep(20);
    zoomInSens->installEventFilter(this);
    cacheSens = new QSlider(Qt::Horizontal, this);
    cacheSens->setTickPosition(QSlider::TicksBelow);
    cacheSens->setMinimum(0);
    cacheSens->setMaximum(100);
    cacheSens->setSingleStep(5);
    cacheSens->setPageStep(5);
    cacheSens->installEventFilter(this);
    controlsResetButton = new QPushButton(this);
    controlsResetButton->setIcon(QIcon(":/icons/reset.png"));
    zoomInMethod = new QComboBox();
    zoomInMethod->addItem("Virtual Finger (bVF)");
    zoomInMethod->addItem("Mean-shift of mean-shift (MSMS)");
    zoomInMethod->installEventFilter(this);
    zoomInMethod->setCurrentIndex(1);

    //"Global coordinates" widgets
    /**/itm::debug(itm::LEV3, "\"Volume Of Interest (VOI)'s coordinates\" panel", __itm__current__function__);
    globalCoord_panel = new QGroupBox("Volume Of Interest (VOI)'s coordinates");
    traslXpos = new QArrowButton(this, QColor(255,0,0), 15, 6, 0, Qt::LeftToRight, true);
    traslXneg = new QArrowButton(this, QColor(255,0,0), 15, 6, 0, Qt::RightToLeft, true);
    traslXlabel = new QLabel("");
    traslYpos = new QArrowButton(this, QColor(0,200,0), 15, 6, 0, Qt::LeftToRight, true);
    traslYneg = new QArrowButton(this, QColor(0,200,0), 15, 6, 0, Qt::RightToLeft, true);
    traslYlabel = new QLabel("");
    traslZpos = new QArrowButton(this, QColor(0,0,255), 15, 6, 0, Qt::LeftToRight, true);
    traslZneg = new QArrowButton(this, QColor(0,0,255), 15, 6, 0, Qt::RightToLeft, true);
    traslZlabel = new QLabel("");
    traslTpos = new QArrowButton(this, QColor(150,150,150), 15, 6, 0, Qt::LeftToRight, true);
    traslTneg = new QArrowButton(this, QColor(150,150,150), 15, 6, 0, Qt::RightToLeft, true);
    traslTlabel = new QLabel("");
    traslXpos->installEventFilter(this);
    traslXneg->installEventFilter(this);
    traslYpos->installEventFilter(this);
    traslYneg->installEventFilter(this);
    traslZpos->installEventFilter(this);
    traslZneg->installEventFilter(this);
    traslTpos->installEventFilter(this);
    traslTneg->installEventFilter(this);
    V0_sbox = new QSpinBox();
    V0_sbox->setAlignment(Qt::AlignCenter);
    V1_sbox = new QSpinBox();
    V1_sbox->setAlignment(Qt::AlignCenter);
    H0_sbox = new QSpinBox();
    H0_sbox->setAlignment(Qt::AlignCenter);
    H1_sbox = new QSpinBox();
    H1_sbox->setAlignment(Qt::AlignCenter);
    D0_sbox = new QSpinBox();
    D0_sbox->setAlignment(Qt::AlignCenter);
    D1_sbox = new QSpinBox();
    D1_sbox->setAlignment(Qt::AlignCenter);
    T0_sbox = new QLineEdit();
    T0_sbox->setAlignment(Qt::AlignCenter);
    T0_sbox->setReadOnly(true);
    T1_sbox = new QLineEdit();
    T1_sbox->setAlignment(Qt::AlignCenter);
    T1_sbox->setReadOnly(true);
    V0_sbox->installEventFilter(this);
    V1_sbox->installEventFilter(this);
    H0_sbox->installEventFilter(this);
    H1_sbox->installEventFilter(this);
    D0_sbox->installEventFilter(this);
    D1_sbox->installEventFilter(this);
    T0_sbox->installEventFilter(this);
    T1_sbox->installEventFilter(this);
    to_label_1 = new QLabel("to");
    to_label_1->setAlignment(Qt::AlignCenter);
    to_label_2 = new QLabel("to");
    to_label_2->setAlignment(Qt::AlignCenter);
    to_label_3 = new QLabel("to");
    to_label_3->setAlignment(Qt::AlignCenter);
    to_label_4 = new QLabel("to");
    to_label_4->setAlignment(Qt::AlignCenter);
    refSys = new QGLRefSys(tabs);
    refSys->installEventFilter(this);
    frameCoord = new QLineEdit();
    frameCoord->setReadOnly(true);
    frameCoord->setAlignment(Qt::AlignCenter);
    QFont veryTinyFont = QApplication::font();
    veryTinyFont.setPointSize(8);
    frameCoord->setFont(veryTinyFont);

    /* ------- global coord panel widgets ------- */
    PR_panel = new QGroupBox("Proofreading");
    PR_button = new QPushButton("Start");
    PR_button->installEventFilter(this);
    PR_spbox = new QSpinBox();
    PR_spbox->setAlignment(Qt::AlignCenter);
    PR_spbox->installEventFilter(this);
    PR_spbox->setPrefix("Block ");

    //other widgets
    helpBox = new QHelpBox(this);
    progressBar = new QProgressBar(this);
    statusBar = new QStatusBar();

    //****LAYOUT SECTIONS****
    /**/itm::debug(itm::LEV3, "Layouting", __itm__current__function__);

    //Page "Volume's info": contains informations of the loaded volume
    QGridLayout* info_panel_layout = new QGridLayout();

    QLabel* size_label = new QLabel("Size (in files):");
    size_label->setFixedWidth(100);
    info_panel_layout->addWidget(size_label,                    0,0,1,1);
    info_panel_layout->addWidget(vol_size_field,                0,2,1,1);
    info_panel_layout->addWidget(new QLabel("Dims (mm):"),      1,0,1,1);
    info_panel_layout->addWidget(vol_dims_mm_field,             1,2,1,1);
    info_panel_layout->addWidget(new QLabel("Dims (vxl)"),      2,0,1,1);
    info_panel_layout->addWidget(vol_dims_vxl_field,            2,2,1,1);
    info_panel_layout->addWidget(new QLabel("Tiles grid"),      3,0,1,1);
    info_panel_layout->addWidget(tiles_grid_field,              3,2,1,1);
    info_panel_layout->addWidget(new QLabel("Tiles dims"),      4,0,1,1);
    info_panel_layout->addWidget(tile_dim_field,                4,2,1,1);
    info_panel_layout->addWidget(voxel_dims_label,              5,0,1,1);
    info_panel_layout->addWidget(vxl_field,                     5,2,1,1);
    info_panel_layout->addWidget(new QLabel("Origin (mm)"),     6,0,1,1);
    info_panel_layout->addWidget(org_field,                     6,2,1,1);

    QVBoxLayout* info_page_layout = new QVBoxLayout(info_page);
    info_page_layout->addLayout(info_panel_layout, 0);
    info_page_layout->addStretch(1);
    info_page->setLayout(info_page_layout);
    #ifdef Q_OS_LINUX
    info_page->setStyle(new QWindowsStyle());
    #endif

    // "Global coordinates" panel layout
    QGridLayout* global_coordinates_layout = new QGridLayout();
    global_coordinates_layout->setVerticalSpacing(2);
    /* ------------- fix left block elements size ---------------- */
    QWidget* refSysContainer = new QWidget();
    refSysContainer->setFixedWidth(marginLeft);
    refSysContainer->setStyleSheet(" border-style: solid; border-width: 1px; border-color: rgb(150,150,150);");
    QHBoxLayout* refSysContainerLayout = new QHBoxLayout();
    refSysContainerLayout->setContentsMargins(1,1,1,1);
    refSysContainerLayout->addWidget(refSys, 1);
    refSysContainer->setLayout(refSysContainerLayout);
    frameCoord->setFixedWidth(marginLeft);
    /* ------------ fix central block elements size -------------- */
    QHBoxLayout *xShiftLayout = new QHBoxLayout();
    xShiftLayout->setContentsMargins(0,0,0,0);
    int fixedArrowWidth = 20;
    traslXneg->setFixedWidth(fixedArrowWidth);
    traslXpos->setFixedWidth(fixedArrowWidth);
    xShiftLayout->addStretch();
    xShiftLayout->addWidget(traslXneg, 0);
    traslXlabel->setFixedWidth(20);
    xShiftLayout->addWidget(traslXlabel, 0);
    xShiftLayout->addWidget(traslXpos, 0);
    xShiftLayout->addStretch();
    xShiftLayout->setSpacing(5);
    QWidget* xShiftWidget = new QWidget();
    xShiftWidget->setFixedWidth(marginLeft);
    xShiftWidget->setLayout(xShiftLayout);
    QHBoxLayout *yShiftLayout = new QHBoxLayout();
    yShiftLayout->setContentsMargins(0,0,0,0);
    traslYneg->setFixedWidth(fixedArrowWidth);
    traslYpos->setFixedWidth(fixedArrowWidth);
    yShiftLayout->addStretch();
    yShiftLayout->addWidget(traslYneg, 0);
    traslYlabel->setFixedWidth(20);
    yShiftLayout->addWidget(traslYlabel, 0);
    yShiftLayout->addWidget(traslYpos, 0);
    yShiftLayout->addStretch();
    yShiftLayout->setSpacing(5);
    QWidget* yShiftWidget = new QWidget();
    yShiftWidget->setFixedWidth(marginLeft);
    yShiftWidget->setLayout(yShiftLayout);
    QHBoxLayout *zShiftLayout = new QHBoxLayout();
    zShiftLayout->setContentsMargins(0,0,0,0);
    traslZneg->setFixedWidth(fixedArrowWidth);
    traslZpos->setFixedWidth(fixedArrowWidth);
    zShiftLayout->addStretch();
    traslZlabel->setFixedWidth(20);
    zShiftLayout->addWidget(traslZneg, 0);
    zShiftLayout->addWidget(traslZlabel, 0);
    zShiftLayout->addWidget(traslZpos, 0);
    zShiftLayout->addStretch();
    zShiftLayout->setSpacing(5);
    QWidget* zShiftWidget = new QWidget();
    zShiftWidget->setFixedWidth(marginLeft);
    zShiftWidget->setLayout(zShiftLayout);
    QHBoxLayout *tShiftLayout = new QHBoxLayout();
    tShiftLayout->setContentsMargins(0,0,0,0);
    traslTneg->setFixedWidth(fixedArrowWidth);
    traslTpos->setFixedWidth(fixedArrowWidth);
    tShiftLayout->addStretch();
    tShiftLayout->addWidget(traslTneg, 0);
    traslTlabel->setFixedWidth(20);
    tShiftLayout->addWidget(traslTlabel, 0);
    tShiftLayout->addWidget(traslTpos, 0);
    tShiftLayout->addStretch();
    tShiftLayout->setSpacing(5);
    QWidget* tShiftWidget = new QWidget();
    tShiftWidget->setFixedWidth(marginLeft);
    tShiftWidget->setLayout(tShiftLayout);
    /* ------------- fix right block elements size --------------- */
    QHBoxLayout *xGlobalCoordLayout = new QHBoxLayout();
    xGlobalCoordLayout->setSpacing(5);
    xGlobalCoordLayout->setContentsMargins(0,0,0,0);
    xGlobalCoordLayout->addWidget(H0_sbox, 1);
    xGlobalCoordLayout->addWidget(to_label_1, 0);
    xGlobalCoordLayout->addWidget(H1_sbox, 1);
    QHBoxLayout *yGlobalCoordLayout = new QHBoxLayout();
    yGlobalCoordLayout->setSpacing(5);
    yGlobalCoordLayout->setContentsMargins(0,0,0,0);
    yGlobalCoordLayout->addWidget(V0_sbox, 1);
    yGlobalCoordLayout->addWidget(to_label_2, 0);
    yGlobalCoordLayout->addWidget(V1_sbox, 1);
    QHBoxLayout *zGlobalCoordLayout = new QHBoxLayout();
    zGlobalCoordLayout->setSpacing(5);
    zGlobalCoordLayout->setContentsMargins(0,0,0,0);
    zGlobalCoordLayout->addWidget(D0_sbox, 1);
    zGlobalCoordLayout->addWidget(to_label_3, 0);
    zGlobalCoordLayout->addWidget(D1_sbox, 1);
    QHBoxLayout *tGlobalCoordLayout = new QHBoxLayout();
    tGlobalCoordLayout->setSpacing(5);
    tGlobalCoordLayout->setContentsMargins(0,0,0,0);
    tGlobalCoordLayout->addWidget(T0_sbox, 1);
    tGlobalCoordLayout->addWidget(to_label_4, 0);
	tGlobalCoordLayout->addWidget(T1_sbox, 1); 
#if QT_VERSION < 0x040806 // MK, 12082017: Qt4.8+ seemed to disallow visible property of QLayout class. Need to comment out this part for Qt4.8.
    QVBoxLayout *rightBlockLayout = new QVBoxLayout();
    rightBlockLayout->setContentsMargins(0,0,0,0);
    rightBlockLayout->addLayout(xGlobalCoordLayout, 0);
    rightBlockLayout->addLayout(yGlobalCoordLayout, 0);
    rightBlockLayout->addLayout(zGlobalCoordLayout, 0);
    rightBlockLayout->addLayout(tGlobalCoordLayout, 0);
#endif
    /* -------------- put elements into 4x4 grid ----------------- */
    global_coordinates_layout->addWidget(refSysContainer,   0, 0, 3, 1);
    global_coordinates_layout->addWidget(frameCoord,        3, 0, 1, 1);
    global_coordinates_layout->addWidget(xShiftWidget,      0, 1, 1, 1);
    global_coordinates_layout->addWidget(yShiftWidget,      1, 1, 1, 1);
    global_coordinates_layout->addWidget(zShiftWidget,      2, 1, 1, 1);
    global_coordinates_layout->addWidget(tShiftWidget,      3, 1, 1, 1);
    global_coordinates_layout->addLayout(xGlobalCoordLayout,0, 2, 1, 2);
    global_coordinates_layout->addLayout(yGlobalCoordLayout,1, 2, 1, 2);
    global_coordinates_layout->addLayout(zGlobalCoordLayout,2, 2, 1, 2);
    global_coordinates_layout->addLayout(tGlobalCoordLayout,3, 2, 1, 2);
    /* ------------- FINALIZATION -------------- */
    global_coordinates_layout->setContentsMargins(10,5,10,5);
    globalCoord_panel->setLayout(global_coordinates_layout);
    #ifdef Q_OS_LINUX
    globalCoord_panel->setStyle(new QWindowsStyle());
    #endif

    // "Proofreading" panel layout
    QHBoxLayout* esPanelLayout = new QHBoxLayout();
    PR_button->setFixedWidth(marginLeft);
    esPanelLayout->addWidget(PR_button, 0);
    esPanelLayout->addWidget(PR_spbox, 1);
    esPanelLayout->setContentsMargins(10,5,10,5);
    PR_panel->setLayout(esPanelLayout);
    #ifdef Q_OS_LINUX
    PR_panel->setStyle(new QWindowsStyle());
    #endif

    //local viewer panel
    QVBoxLayout* localviewer_panel_layout= new QVBoxLayout();
    /* --------------------- first row ---------------------- */
    QHBoxLayout *resolutionSelection_layout = new QHBoxLayout();
    resolutionSelection_layout->setContentsMargins(0,0,0,0);
    QLabel *resolutionLabel = new QLabel("Resolution:");
    //resolutionLabel->setStyleSheet("background-color:blue");
    resolutionLabel->setFixedWidth(marginLeft);
    resolutionSelection_layout->addWidget(resolutionLabel, 0);
    resolutionSelection_layout->addWidget(resolution_cbox, 1);
    /* -------------------- second row ---------------------- */
    QHBoxLayout *resolutionBar_layout = new QHBoxLayout();
    resolutionBar_layout->setContentsMargins(0,0,0,0);
    QLabel *emptyLabel = new QLabel("");
    emptyLabel->setFixedWidth(marginLeft);
    resolutionBar_layout->addWidget(emptyLabel, 0);
    resolutionBar_layout->addWidget(gradientBar, 1);
    /* --------------------- third row ---------------------- */
    QHBoxLayout *VOImaxsize_layout = new QHBoxLayout();
    VOImaxsize_layout->setContentsMargins(0,0,0,0);
    QLabel *viewMaxDimsLabel = new QLabel("Max dims:");
    viewMaxDimsLabel->setFixedWidth(marginLeft);
    VOImaxsize_layout->addWidget(viewMaxDimsLabel, 0, Qt::AlignLeft);
    QWidget* fourSpinboxes = new QWidget();
    QHBoxLayout *fourSpinboxes_layout = new QHBoxLayout();
    fourSpinboxes_layout->setContentsMargins(0,0,0,0);
    fourSpinboxes_layout->addWidget(Hdim_sbox, 1);
    fourSpinboxes_layout->addWidget(Vdim_sbox, 1);
    fourSpinboxes_layout->addWidget(Ddim_sbox, 1);
    fourSpinboxes_layout->addWidget(Tdim_sbox, 1);
    fourSpinboxes_layout->setSpacing(2);
    fourSpinboxes->setLayout(fourSpinboxes_layout);
    VOImaxsize_layout->addWidget(fourSpinboxes, 1, Qt::AlignLeft);
    /* ------------- FINALIZATION -------------- */
    localviewer_panel_layout->addLayout(resolutionSelection_layout, 0);
    localviewer_panel_layout->addLayout(resolutionBar_layout, 0);
    localviewer_panel_layout->addLayout(VOImaxsize_layout, 0);
    localviewer_panel_layout->setContentsMargins(10,5,10,5);
    localViewer_panel->setLayout(localviewer_panel_layout);
    #ifdef Q_OS_LINUX
    localViewer_panel->setStyle(new QWindowsStyle());
    #endif

    //zoom options panel
    QVBoxLayout* zoomOptions_panel_layout= new QVBoxLayout();
    /* --------------------- first row ---------------------- */
    QHBoxLayout *zoomInMethod_layout = new QHBoxLayout();
    zoomInMethod_layout->setContentsMargins(0,0,0,0);
    QLabel *zoomInMethodLabel = new QLabel("Z/i method:");
    //zoomInMethodLabel->setStyleSheet("background-color:blue");
    zoomInMethodLabel->setFixedWidth(marginLeft);
    zoomInMethod_layout->addWidget(zoomInMethodLabel, 0);
    zoomInMethod_layout->addWidget(zoomInMethod, 1);
    /* -------------------- second row ---------------------- */
    QHBoxLayout *zoomControls_layout = new QHBoxLayout();
    zoomControls_layout->setContentsMargins(0,0,0,0);
    /* -------------------- second row: first column -------- */
    QVBoxLayout *zoomControls_col1_layout = new QVBoxLayout();
    zoomControls_col1_layout->setContentsMargins(0,0,0,0);
    QLabel *zoomInThresholdLabel = new QLabel("Z/i thres:");
    zoomInThresholdLabel->setFixedWidth(marginLeft);
    QLabel *cachingSensLabel = new QLabel("Z/i cache:");
    cachingSensLabel->setFixedWidth(marginLeft);
    QLabel* zoomOutThres = new QLabel("Z/o thres:");
    zoomOutThres->setFixedWidth(marginLeft);
    zoomControls_col1_layout->addWidget(zoomInThresholdLabel, 0);
    zoomControls_col1_layout->addWidget(cachingSensLabel, 0);
    zoomControls_col1_layout->addWidget(zoomOutThres, 0);
    /* -------------------- second row: second column ------- */
    QVBoxLayout *zoomControls_col2_layout = new QVBoxLayout();
    zoomControls_col2_layout->setContentsMargins(0,0,0,0);
    zoomControls_col2_layout->addWidget(zoomInSens, 0);
    zoomControls_col2_layout->addWidget(cacheSens, 0);
    zoomControls_col2_layout->addWidget(zoomOutSens, 0);
    /* -------------------- second row: third column -------- */
    /* -------------------- second row: fourth column ------- */
    /* -------------------- second row: FINALIZATION -------- */
    zoomControls_layout->addLayout(zoomControls_col1_layout, 0);
    zoomControls_layout->addLayout(zoomControls_col2_layout, 1);
    zoomControls_layout->addWidget(controlsResetButton, 0);
    /* -------------------- FINALIZATION -------------------- */
    zoomOptions_panel_layout->addLayout(zoomInMethod_layout, 0);
    zoomOptions_panel_layout->addLayout(zoomControls_layout, 0);
    zoomOptions_panel_layout->setContentsMargins(10,5,10,5);
    zoom_panel->setLayout(zoomOptions_panel_layout);
    #ifdef Q_OS_LINUX
    zoom_panel->setStyle(new QWindowsStyle());
    #endif

    // Page "Controls" layout
    QVBoxLayout* controlsLayout = new QVBoxLayout(controls_page);
    controlsLayout->addWidget(localViewer_panel, 0);
    controlsLayout->addWidget(zoom_panel, 0);
    controlsLayout->addWidget(globalCoord_panel, 0);
    controlsLayout->addWidget(PR_panel, 0);
    controlsLayout->addStretch(1);
    #ifdef Q_OS_MAC
    controlsLayout->setContentsMargins(10,0,10,10);
    #endif
    controls_page->setLayout(controlsLayout);

    //pages
    tabs->addTab(controls_page, "TeraFly controls");
    tabs->addTab(info_page, "Volume's info");

    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    QHBoxLayout* centralLayout = new QHBoxLayout();
    QVBoxLayout* innerLayout = new QVBoxLayout();
    QVBoxLayout* bottomLayout = new QVBoxLayout();
    innerLayout->addWidget(toolBar, 0);
    innerLayout->addWidget(tabs, 0);
    innerLayout->addStretch(1);
    QHBoxLayout* helpBoxLayout = new QHBoxLayout();
    helpBoxLayout->setContentsMargins(0,0,0,0);
    helpBoxLayout->addWidget(helpBox, 1);
    innerLayout->addLayout(helpBoxLayout, 1);
    innerLayout->setSpacing(5);
    centralLayout->addLayout(innerLayout, 1);
    bottomLayout->addWidget(statusBar);
    bottomLayout->addWidget(progressBar);
    bottomLayout->setContentsMargins(10,10,10,10);
    layout->addWidget(menuBar, 0);
    centralLayout->setContentsMargins(0,0,0,0);
    layout->addLayout(centralLayout, 1);
    layout->addLayout(bottomLayout, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    setWindowTitle(QString("TeraFly v").append(teramanager::version.c_str()));
    this->setFont(tinyFont);


    // signals and slots
    /**/itm::debug(itm::LEV3, "Signals and slots", __itm__current__function__);
    connect(CImport::instance(), SIGNAL(sendOperationOutcome(itm::RuntimeException*, qint64)), this, SLOT(importDone(itm::RuntimeException*, qint64)), Qt::QueuedConnection);
    connect(xShiftSBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(yShiftSBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(zShiftSBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(tShiftSBox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(Vdim_sbox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(Hdim_sbox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(Ddim_sbox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(Tdim_sbox, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged(int)));
    connect(resolution_cbox, SIGNAL(currentIndexChanged(int)), this, SLOT(resolutionIndexChanged(int)));
    connect(traslXpos, SIGNAL(clicked()), this, SLOT(traslXposClicked()));
    connect(traslXneg, SIGNAL(clicked()), this, SLOT(traslXnegClicked()));
    connect(traslYpos, SIGNAL(clicked()), this, SLOT(traslYposClicked()));
    connect(traslYneg, SIGNAL(clicked()), this, SLOT(traslYnegClicked()));
    connect(traslZpos, SIGNAL(clicked()), this, SLOT(traslZposClicked()));
    connect(traslZneg, SIGNAL(clicked()), this, SLOT(traslZnegClicked()));
    connect(traslTpos, SIGNAL(clicked()), this, SLOT(traslTposClicked()));
    connect(traslTneg, SIGNAL(clicked()), this, SLOT(traslTnegClicked()));
    connect(controlsResetButton, SIGNAL(clicked()), this, SLOT(resetMultiresControls()));
    connect(PR_button, SIGNAL(clicked()), this, SLOT(PRbuttonClicked()));
    connect(PR_spbox, SIGNAL(valueChanged(int)), this, SLOT(PRblockSpinboxChanged(int)));
    connect(this, SIGNAL(sendProgressBarChanged(int, int, int, const char*)), this, SLOT(progressBarChanged(int, int, int, const char*)), Qt::QueuedConnection);
    connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(tabIndexChanged(int)));

    // first resize to the desired size
    resize(380, qApp->desktop()->availableGeometry().height());


    //set always on top and show
    //setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
    show();


    // fix current window size
    //setFixedSize(width(), height());

    // move to center(vertical)-right(horizontal)
    move(qApp->desktop()->availableGeometry().width() - width(), 0);

	PMain::virtualSpaceSizeChanged();


    // register this as event filter
    installEventFilter(this);

    // instance PLog
    PLog::instance(this);


    /**/itm::debug(itm::LEV1, "object successfully constructed", __itm__current__function__);
}

//reset everything
void PMain::reset()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    //resetting menu options and widgets
    openTeraFlyVolumeAction->setEnabled(true);
    openHDF5VolumeAction->setEnabled(true);
    openVolumeToolButton->setEnabled(true);
    recentVolumesMenu->setEnabled(true);
    closeVolumeAction->setEnabled(false);
    importOptionsMenu->setEnabled(true);
    loadAnnotationsAction->setEnabled(false);
    saveAnnotationsAction->setEnabled(false);
    saveAnnotationsAsAction->setEnabled(false);
    clearAnnotationsAction->setEnabled(false);
    virtualSpaceSizeMenu->setEnabled(true);
//    PAnoToolBar::instance()->setEnabled(false);
    showToolbarButton->setChecked(false);
    showToolbarButton->setEnabled(false);

    //reseting info panel widgets
    info_page->setEnabled(false);
    vol_size_field->setText("");
    vol_dims_mm_field->setText("");
    vol_dims_vxl_field->setText("");
    tiles_grid_field->setText("");
    tile_dim_field->setText("");
    vxl_field->setText("");
    org_field->setText("");

    //resetting multiresolution mode widgets
    gradientBar->setEnabled(false);
    resolution_cbox->setEnabled(false);
    zoom_panel->setEnabled(false);
    Vdim_sbox->setValue(CSettings::instance()->getVOIdimV());
    Hdim_sbox->setValue(CSettings::instance()->getVOIdimH());
    Ddim_sbox->setValue(CSettings::instance()->getVOIdimD());
    Tdim_sbox->setValue(CSettings::instance()->getVOIdimT());
    resolution_cbox->setEnabled(false);
    while(resolution_cbox->count())
        resolution_cbox->removeItem(0);
    traslXlabel->setAlignment(Qt::AlignCenter);
//    traslXlabel->setTextFormat(Qt::RichText);
    traslXlabel->setText("<font size=\"3\">x</font>");
    traslYlabel->setAlignment(Qt::AlignCenter);
//    traslYlabel->setTextFormat(Qt::RichText);
    traslYlabel->setText("<font size=\"3\">y</font>");
    traslZlabel->setAlignment(Qt::AlignCenter);
//    traslZlabel->setTextFormat(Qt::RichText);
    traslZlabel->setText("<font size=\"3\">z</font>");
    traslTlabel->setAlignment(Qt::AlignCenter);
//    traslTlabel->setTextFormat(Qt::RichText);
    traslTlabel->setText("<font size=\"3\">t</font>");
    traslXpos->setEnabled(false);
    traslXneg->setEnabled(false);
    traslYpos->setEnabled(false);
    traslYneg->setEnabled(false);
    traslZpos->setEnabled(false);
    traslZneg->setEnabled(false);
    traslTpos->setEnabled(false);
    traslTneg->setEnabled(false);
    gradientBar->setEnabled(false);
    gradientBar->setNSteps(-1);
    gradientBar->setStep(0);
    resetMultiresControls();

    //resetting subvol panel widgets
    globalCoord_panel->setEnabled(false);
    PR_panel->setEnabled(false);
    V0_sbox->setValue(0);
    V1_sbox->setValue(0);
    H0_sbox->setValue(0);
    H1_sbox->setValue(0);
    D0_sbox->setValue(0);
    D1_sbox->setValue(0);
    T0_sbox->setText("");
    T1_sbox->setText("");
//    T0_sbox->setValue(0);
//    T1_sbox->setValue(0);
    frameCoord->setText("");
    refSys->setXRotation(200);
    refSys->setYRotation(50);
    refSys->setZRotation(0);
    refSys->setDims(1,1,1);
    refSys->setFilled(true);
    refSys->resetZoom();
    frameCoord->setPalette(globalCoord_panel->palette());

    //reset PR panel widgets
    PR_button->setText("Start");
    PR_spbox->setSuffix(" of 0");
    PR_spbox->setMaximum(0);
    PR_spbox->setMinimum(0);
    PR_spbox->setValue(0);
    PR_spbox->setEnabled(false);

    //resetting progress bar and text
    progressBar->setEnabled(false);
    progressBar->setMaximum(1);         //needed to stop animation on some operating systems
    statusBar->clearMessage();
    statusBar->showMessage("Ready.");
    helpBox->setText(HTwelcome);

    // @ADDED Vaa3D-controls-within-TeraFly feature.
    if(tabs->count() == 3)
    {
        int tab_selected = tabs->currentIndex();
        tabs->removeTab(1);
        tabs->setCurrentIndex(tab_selected);
    }
}


//reset GUI method
void PMain::resetGUI()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    progressBar->setEnabled(false);
    progressBar->setMaximum(1);         //needed to stop animation on some operating systems
    statusBar->clearMessage();
    statusBar->showMessage("Ready.");
    this->setCursor(QCursor(Qt::ArrowCursor));
}


/**********************************************************************************
* Called when a path in the "Recent volumes" menu is selected.
***********************************************************************************/
void PMain::openVolumeActionTriggered()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    std::string recentpath = qobject_cast<QAction*>(sender())->text().toStdString();
    QFileInfo pathinfo(recentpath.c_str());
    if(pathinfo.isDir())
        openTeraFlyVolume(qobject_cast<QAction*>(sender())->text().toStdString());
    else if(pathinfo.isFile())
        openHDF5Volume(qobject_cast<QAction*>(sender())->text().toStdString());
    else
        QMessageBox::critical(this,QObject::tr("Error"), "Cannot find the file",QObject::tr("Ok"));
}

/**********************************************************************************
* Called when "Clear menu" action in "Recent volumes" menu is triggered.
***********************************************************************************/
void PMain::clearRecentVolumesTriggered()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CSettings::instance()->clearVolumePathHistory();
    QList<QAction*> actions = recentVolumesMenu->actions();
    qDeleteAll(actions.begin(), actions.end());
    clearRecentVolumesAction = new QAction("Clear menu",recentVolumesMenu);
    connect(clearRecentVolumesAction, SIGNAL(triggered()), this, SLOT(clearRecentVolumesTriggered()));
    recentVolumesMenu->addSeparator();
    recentVolumesMenu->addAction(clearRecentVolumesAction);
}

/**********************************************************************************
* Called when "Open HDF5 volume" menu action is triggered.
* If path is not provided, opens a QFileDialog to select volume's path.
***********************************************************************************/
void PMain::openHDF5Volume(string path)
{
    try
    {
        if(path.empty())
        {
            /**/itm::debug(itm::LEV2, "import_path is empty, launch file dialog", __itm__current__function__);
            #ifdef _USE_QT_DIALOGS
            QFileDialog dialog(0);
            dialog.setFileMode(QFileDialog::ExistingFile);
            dialog.setNameFilter(tr("BigDataViewer HDF5 files (*.h5)"));
            dialog.setViewMode(QFileDialog::Detail);
            dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
            dialog.setWindowTitle("Select volume's file");
            dialog.setDirectory(CSettings::instance()->getVolumePathLRU().c_str());
            if(dialog.exec())
                path = dialog.directory().absolutePath().toStdString();

            #else
            path = QFileDialog::getOpenFileName(this, "Open HDF5 volume's file", QString(), tr("HDF5 files (*.h5)")).toStdString();
            #endif
            /**/itm::debug(itm::LEV3, strprintf("path = %s", path.c_str()).c_str(), __itm__current__function__);


            if (path.empty())
                return;
        }
        else
        {
            /**/itm::debug(itm::LEV2, strprintf("path is not empty (= \"%s\")", path.c_str()).c_str(), __itm__current__function__);

            if(!QFile::exists(path.c_str()))
                throw RuntimeException(strprintf("Path \"%s\" does not exist", path.c_str()).c_str());
        }


        //then checking that no volume has imported yet
        if(!CImport::instance()->isEmpty())
            throw RuntimeException("A volume has been already imported! Please close the current volume first.");


        //storing the path into CSettings
        CSettings::instance()->setVolumePathLRU(path);
        CSettings::instance()->addVolumePathToHistory(path);
        CSettings::instance()->writeSettings();

        //updating recent volumes menu
        QList<QAction*> actions = recentVolumesMenu->actions();
        qDeleteAll(actions.begin(), actions.end());
        std::list<string> recentVolumes = CSettings::instance()->getVolumePathHistory();
        for(std::list<string>::reverse_iterator it = recentVolumes.rbegin(); it != recentVolumes.rend(); it++)
        {
            QAction *action = new QAction(it->c_str(), this);
            connect(action, SIGNAL(triggered()), this, SLOT(openVolumeActionTriggered()));
            recentVolumesMenu->addAction(action);
        }
        clearRecentVolumesAction = new QAction("Clear menu",recentVolumesMenu);
        connect(clearRecentVolumesAction, SIGNAL(triggered()), this, SLOT(clearRecentVolumesTriggered()));
        recentVolumesMenu->addSeparator();
        recentVolumesMenu->addAction(clearRecentVolumesAction);


        //disabling import form and enabling progress bar animation
        progressBar->setEnabled(true);
        progressBar->setMinimum(0);
        progressBar->setMaximum(0);
        statusBar->showMessage("Importing volume...");

        //starting import
        //throw itm::RuntimeException("HDF5 not yet implemented");
        CImport::instance()->setPath(path);
        CImport::instance()->updateMaxDims();
        CImport::instance()->start();
    }
    catch(iim::IOException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::getInstance()->resetGUI();
        CImport::instance()->reset();
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::getInstance()->resetGUI();
        CImport::instance()->reset();
    }
}

/**********************************************************************************
* Called when "Open TeraFly volume" menu action is triggered.
* If path is not provided, opens a QFileDialog to select volume's path.
***********************************************************************************/
void PMain::openTeraFlyVolume(string path /* = "" */)
{
    /**/itm::debug(itm::LEV1, strprintf("path = \"%s\"", path.c_str()).c_str(), __itm__current__function__);

    try
    {
        QString import_path = path.c_str();

        if(import_path.isEmpty())
        {
            /**/itm::debug(itm::LEV2, "import_path is empty, launching file dialog", __itm__current__function__);

            #ifdef _USE_QT_DIALOGS
            QFileDialog dialog(0);
            dialog.setFileMode(QFileDialog::Directory);
            dialog.setViewMode(QFileDialog::Detail);
            dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
            dialog.setWindowTitle("Select volume's directory");
            dialog.setDirectory(CSettings::instance()->getVolumePathLRU().c_str());
            if(dialog.exec())
                import_path = dialog.directory().absolutePath().toStdString().c_str();

            #else
            //added by PHC 20130823
            //itm::setWidgetOnTop(this, false);
            import_path = QFileDialog::getExistingDirectory(this, tr("Select a folder for a resolution of the volume image you want to visualize"),
                                                            CSettings::instance()->getVolumePathLRU().c_str(),
                                                             QFileDialog::ShowDirsOnly
                                                        //     | QFileDialog::DontResolveSymlinks   //maybe I should allow symbolic links as well, by PHC, 20130823
                                                                    );
            //itm::setWidgetOnTop(this, true);

#endif

            /**/itm::debug(itm::LEV3, strprintf("import_path = %s", qPrintable(import_path)).c_str(), __itm__current__function__);

            if (import_path.isEmpty())
                return;
        }
        else
        {
            /**/itm::debug(itm::LEV2, strprintf("import_path is not empty (= \"%s\")", import_path.toStdString().c_str()).c_str(), __itm__current__function__);

            if(!QFile::exists(import_path))
                throw RuntimeException(strprintf("Path \"%s\" does not exist", import_path.toStdString().c_str()).c_str());
        }


        //then checking that no volume has imported yet
        if(!CImport::instance()->isEmpty())
            throw RuntimeException("A volume has been already imported! Please close the current volume first.");


        // check that folder name matches with the used convention
        QDir dir(import_path);
        if( dir.dirName().toStdString().substr(0,3).compare(itm::RESOLUTION_PREFIX) != 0)
            throw RuntimeException(strprintf("\"%s\" is not a valid resolution: the name of the folder does not start with \"%s\"",
                                             qPrintable(import_path), itm::RESOLUTION_PREFIX.c_str() ).c_str());


        //storing the path into CSettings
        CSettings::instance()->setVolumePathLRU(qPrintable(import_path));
        CSettings::instance()->addVolumePathToHistory(qPrintable(import_path));
        CSettings::instance()->writeSettings();

        //updating recent volumes menu
        QList<QAction*> actions = recentVolumesMenu->actions();
        qDeleteAll(actions.begin(), actions.end());
        std::list<string> recentVolumes = CSettings::instance()->getVolumePathHistory();
        for(std::list<string>::reverse_iterator it = recentVolumes.rbegin(); it != recentVolumes.rend(); it++)
        {
            QAction *action = new QAction(it->c_str(), this);
            connect(action, SIGNAL(triggered()), this, SLOT(openVolumeActionTriggered()));
            recentVolumesMenu->addAction(action);
        }
        clearRecentVolumesAction = new QAction("Clear menu",recentVolumesMenu);
        connect(clearRecentVolumesAction, SIGNAL(triggered()), this, SLOT(clearRecentVolumesTriggered()));
        recentVolumesMenu->addSeparator();
        recentVolumesMenu->addAction(clearRecentVolumesAction);

        //check if additional informations are required
        if(!VirtualVolume::isDirectlyImportable(qPrintable(import_path))  || regenMData_cAction->isChecked())
        {
           if(PDialogImport::instance(this)->exec() == QDialog::Rejected)
                return;
           CImport::instance()->setReimport(true);
           CImport::instance()->setRegenerateVolumeMap(true);
        }
        else
            CImport::instance()->setRegenerateVolumeMap(regenVMap_cAction->isChecked());
        CImport::instance()->setPath(qPrintable(import_path));

        //disabling import form and enabling progress bar animation
        progressBar->setEnabled(true);
        progressBar->setMinimum(0);
        progressBar->setMaximum(0);
        statusBar->showMessage("Importing volume...");

        //starting import
        CImport::instance()->updateMaxDims();
        CImport::instance()->start();
    }
    catch(iim::IOException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::getInstance()->resetGUI();
        CImport::instance()->reset();
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PMain::getInstance()->resetGUI();
        CImport::instance()->reset();
    }
}

/**********************************************************************************
* Called when "Close volume" menu action is triggered.
* All the memory allocated is released and GUI is reset".
***********************************************************************************/
void PMain::closeVolume()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);
    qDebug()<< "PMain::closeVolume()";

    if(PAnoToolBar::isInstantiated())
        PAnoToolBar::instance()->releaseTools();

    CImport::instance()->reset();
    CVolume::instance()->reset();

    PDialogImport::uninstance();
    CViewer::uninstance();
    CAnnotations::uninstance();
    PDialogProofreading::uninstance();
    reset();
}

/**********************************************************************************
* Called when "Open annotations" menu action is triggered.
* Opens QFileDialog to select annotation file path.
***********************************************************************************/
void PMain::loadAnnotations()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CViewer *cur_win = CViewer::getCurrent();

    try
    {
        if(cur_win)
        {
            //obtaining current volume's parent folder path
            QDir dir(CImport::instance()->getPath().c_str());
            dir.cdUp();

            #ifdef _USE_QT_DIALOGS
            QString path = "";
            QFileDialog dialog(cur_win->window3D);
            dialog.setFileMode(QFileDialog::ExistingFile);
            dialog.setViewMode(QFileDialog::Detail);
            dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
            dialog.setWindowTitle("Open annotation file");
            dialog.setNameFilter(tr("annotation files (*.ano)"));
            dialog.setDirectory(dir.absolutePath().toStdString().c_str());
            if(dialog.exec())
               if(!dialog.selectedFiles().empty())
                   path = dialog.selectedFiles().front();

            #else
            //itm::setWidgetOnTop(cur_win->window3D, false);
            QString path = QFileDialog::getOpenFileName(cur_win->window3D, "Open annotation file", dir.absolutePath(), tr("annotation files (*.ano)"));
            //itm::setWidgetOnTop(cur_win->window3D, true);
            #endif

            if(!path.isEmpty())
            {
                annotationsPathLRU = path.toStdString();
                CAnnotations::getInstance()->load(annotationsPathLRU.c_str());

                // save current cursor and set wait cursor
                QCursor cursor = cur_win->view3DWidget->cursor();
                if(PAnoToolBar::isInstantiated())
                    PAnoToolBar::instance()->setCursor(Qt::WaitCursor);
                CViewer::setCursor(Qt::WaitCursor);

                // load
                cur_win->loadAnnotations();
                saveAnnotationsAction->setEnabled(true);                
                virtualSpaceSizeMenu->setEnabled(false);

                // reset saved cursor
                CViewer::setCursor(cursor);
                if(PAnoToolBar::isInstantiated())
                    PAnoToolBar::instance()->setCursor(cursor);
            }
            else
                return;
        }
    }
    catch(RuntimeException &ex)
    {
        CAnnotations::getInstance()->clear();
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Called when "Save annotations" or "Save annotations as" menu actions are triggered.
* If required, opens QFileDialog to select annotation file path.
***********************************************************************************/
void PMain::saveAnnotations()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CViewer *cur_win = CViewer::getCurrent();

    try
    {
        if(cur_win)
        {
            if(annotationsPathLRU.compare("")==0)
            {
                saveAnnotationsAs();
                return;
            }

            // save current cursor and set wait cursor
            QCursor cursor = cur_win->view3DWidget->cursor();
            if(PAnoToolBar::isInstantiated())
                PAnoToolBar::instance()->setCursor(Qt::WaitCursor);
            CViewer::setCursor(Qt::WaitCursor);

            // save
            cur_win->storeAnnotations();
            CAnnotations::getInstance()->save(annotationsPathLRU.c_str());

            // reset saved cursor
            CViewer::setCursor(cursor);
            if(PAnoToolBar::isInstantiated())
                PAnoToolBar::instance()->setCursor(cursor);
        }
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}
void PMain::saveAnnotationsAs()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CViewer *cur_win = CViewer::getCurrent();

    try
    {
        if(cur_win)
        {
            //obtaining current volume's parent folder path
            QDir dir(CImport::instance()->getPath().c_str());
            dir.cdUp();

            #ifdef _USE_QT_DIALOGS
            QString path = "";
            QFileDialog dialog(0);
            dialog.setFileMode(QFileDialog::AnyFile);
            dialog.setAcceptMode(QFileDialog::AcceptSave);
            dialog.setViewMode(QFileDialog::Detail);
            dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
            dialog.setWindowTitle("Save annotation file as");
            dialog.setNameFilter(tr("annotation files (*.ano)"));
            dialog.setDirectory(dir.absolutePath().toStdString().c_str());
            if(dialog.exec())
               if(!dialog.selectedFiles().empty())
                   path = dialog.selectedFiles().front();

            #else
            //itm::setWidgetOnTop(cur_win->window3D, false);
            QString path = QFileDialog::getSaveFileName(this, "Save annotation file as", dir.absolutePath(), tr("annotation files (*.ano)"));
            //itm::setWidgetOnTop(cur_win->window3D, true);
            #endif

            if(!path.isEmpty())
            {
                annotationsPathLRU = path.toStdString();
                if(annotationsPathLRU.find(".ano") == string::npos)
                    annotationsPathLRU.append(".ano");

                // save current cursor and set wait cursor
                QCursor cursor = cur_win->view3DWidget->cursor();
                if(PAnoToolBar::isInstantiated())
                    PAnoToolBar::instance()->setCursor(Qt::WaitCursor);
                CViewer::setCursor(Qt::WaitCursor);

                // save
                cur_win->storeAnnotations();
                CAnnotations::getInstance()->save(annotationsPathLRU.c_str());
                saveAnnotationsAction->setEnabled(true);

                // reset saved cursor
                CViewer::setCursor(cursor);
                if(PAnoToolBar::isInstantiated())
                    PAnoToolBar::instance()->setCursor(cursor);
            }
            else
                return;
        }
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Called when "Clear annotations" menu action is triggered.
***********************************************************************************/
void PMain::clearAnnotations()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    try
    {
        CViewer *cur_win = CViewer::getCurrent();
        if(cur_win)
        {
            CAnnotations::getInstance()->clear();
            cur_win->undoStack.clear();
            cur_win->loadAnnotations();
            if(PAnoToolBar::isInstantiated())
            {
                if (PAnoToolBar::instance()->buttonUndo != 0)
                    PAnoToolBar::instance()->buttonUndo->setEnabled(false);
                if (PAnoToolBar::instance()->buttonRedo != 0)
                    PAnoToolBar::instance()->buttonRedo->setEnabled(false);
            }
        }
        virtualSpaceSizeMenu->setEnabled(true);
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Called when "Exit" menu action is triggered or TeraFly window is closed.
***********************************************************************************/
void PMain::exit()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    this->close();
}

/**********************************************************************************
* Called when "Help->About" menu action is triggered
***********************************************************************************/
void PMain::about()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    PAbout::instance(this)->exec();
}


/*********************************************************************************
* Called by <CImport> when the associated operation has been performed.
* If an exception has occurred in the <CImport> thread,  it is propagated and man-
* aged in the current thread (ex != 0). Otherwise, volume information are imported
* in the GUI by the <StackedVolume> handle of <CImport>.
**********************************************************************************/
void PMain::importDone(RuntimeException *ex, qint64 elapsed_time)
{
    /**/itm::debug(itm::LEV1, strprintf("ex = %s", (ex? "error" : "0")).c_str(), __itm__current__function__);

    //if an exception has occurred, showing a message error and re-enabling import form
    if(ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
    }
    else
    {
        // create new macro-group for ImportOperation
        itm::ImportOperation::newGroup();

        //first updating IO time
        /**/itm::debug(itm::LEV3, "updating IO time", __itm__current__function__);
        PLog::instance()->appendOperation(new ImportOperation("Volume imported and map loaded", itm::IO, elapsed_time));

        //otherwise inserting volume's informations
        /**/itm::debug(itm::LEV3, "inserting volume's informations", __itm__current__function__);
        QElapsedTimer timerGUI;
        timerGUI.start();
        VirtualVolume* volume = CImport::instance()->getHighestResVolume();
        info_page->setEnabled(true);

        double GVoxels = (volume->getDIM_V()/1000.0f)*(volume->getDIM_H()/1000.0f)*(volume->getDIM_D()/1000.0f);
        double TVoxels = GVoxels/1000.0;
        if(TVoxels < 0.1)
        {
            double GBytes = GVoxels*CImport::instance()->getVMapCDim();
            vol_size_field->setText(itm::strprintf("  %.1f Gigavoxels (%.1f Gigabytes)", GVoxels, GBytes).c_str());
        }
        else
        {
            double TBytes = TVoxels*CImport::instance()->getVMapCDim();
            vol_size_field->setText(itm::strprintf("  %.1f Teravoxels (%.1f Terabytes)", TVoxels, TBytes).c_str());
        }

        vol_dims_mm_field->setText(
                    itm::strprintf("  %.3f(x) x %.3f(y) x %.3f(z)",
                                   fabs(volume->getDIM_H()*volume->getVXL_H()/1000.0f),
                                   fabs(volume->getDIM_V()*volume->getVXL_V()/1000.0f),
                                   fabs(volume->getDIM_D()*volume->getVXL_D()/1000.0f)).c_str());


        vol_dims_vxl_field->setText(
                    itm::strprintf("  %d(x) x %d(y) x %d(z) x %d(c) x %d(t)",
                                   volume->getDIM_H(),
                                   volume->getDIM_V(),
                                   volume->getDIM_D(), volume->getDIM_C(), volume->getDIM_T()).c_str());
        VirtualVolume* volume_ith = dynamic_cast<TimeSeries*>(volume) ? dynamic_cast<TimeSeries*>(volume)->getFrameAt(0) : volume;
        if(dynamic_cast<StackedVolume*>(volume_ith))
        {
            StackedVolume* vol = dynamic_cast<StackedVolume*>(volume_ith);
            tiles_grid_field->setText(itm::strprintf("  %d(x) x %d(y) x 1(z)", vol->getN_COLS(), vol->getN_ROWS()).c_str());
            tile_dim_field->setText(itm::strprintf("  %d(x) x %d(y) x %d(z)", vol->getStacksWidth(), vol->getStacksWidth(), vol->getDIM_D()).c_str());
        }
        else if(dynamic_cast<TiledVolume*>(volume_ith) || dynamic_cast<TiledMCVolume*>(volume_ith))
        {
            TiledVolume* vol = dynamic_cast<TiledVolume*>(volume_ith) ? dynamic_cast<TiledVolume*>(volume_ith) : dynamic_cast<TiledMCVolume*>(volume_ith)->getVolumes()[0];
            tiles_grid_field->setText(itm::strprintf("  %d(x) x %d(y) x %d(z)", vol->getN_COLS(), vol->getN_ROWS(), vol->getBLOCKS()[0][0]->getN_BLOCKS()).c_str());
            tile_dim_field->setText(itm::strprintf("  %d(x) x %d(y) x %d(z)", vol->getStacksWidth(), vol->getStacksWidth(), vol->getBLOCKS()[0][0]->getBLOCK_SIZE()[0]).c_str());
        }
        else if(dynamic_cast<BDVVolume*>(volume_ith))
        {
            tiles_grid_field->setText("BDV-HDF5 custom grid");
            tile_dim_field->setText("BDV-HDF5 custom tile dim");
        }
        else
        {
            tiles_grid_field->setText(itm::strprintf("unrecognized volume \"%s\"", typeid(volume).name()).c_str());
            tile_dim_field->setText(itm::strprintf("unrecognized volume \"%s\"", typeid(volume).name()).c_str());
        }
        vxl_field->setText(itm::strprintf("  %.3f(x) x %.3f(y) x %.3f(z)", volume->getVXL_H(), volume->getVXL_V(), volume->getVXL_D()).c_str());
        org_field->setText(itm::strprintf("  {%.3f(x), %.3f(y), %.3f(z)}", volume->getORG_H(), volume->getORG_V(), volume->getORG_D()).c_str());

        //and setting subvol widgets limits
        /**/itm::debug(itm::LEV3, "setting subvol widgets limits", __itm__current__function__);
        V0_sbox->setMinimum(1);
        V0_sbox->setMaximum(volume->getDIM_V());
        V0_sbox->setValue(1);
        V1_sbox->setMinimum(1);
        V1_sbox->setMaximum(volume->getDIM_V());
        V1_sbox->setValue(volume->getDIM_V());
        H0_sbox->setMinimum(1);
        H0_sbox->setMaximum(volume->getDIM_H());
        H0_sbox->setValue(1);
        H1_sbox->setMinimum(1);
        H1_sbox->setMaximum(volume->getDIM_H());
        H1_sbox->setValue(volume->getDIM_H());
        D0_sbox->setMinimum(1);
        D0_sbox->setMaximum(volume->getDIM_D());
        D0_sbox->setValue(1);
        D1_sbox->setMinimum(1);
        D1_sbox->setMaximum(volume->getDIM_D());
        D1_sbox->setValue(volume->getDIM_D());        
//        T0_sbox->setMinimum(0);
//        T0_sbox->setMaximum(CImport::instance()->getVMapTDim()-1);
        if(volume->getDIM_T() > 1)
        {
            T0_sbox->setText("1");
            T1_sbox->setText(QString::number(CImport::instance()->getVMapTDim()));
            frameCoord->setText(strprintf("t = %d/%d", 1, volume->getDIM_T()).c_str());
        }
//        T1_sbox->setMinimum(0);
//        T1_sbox->setMaximum(CImport::instance()->getVMapTDim()-1);
        globalCoord_panel->setEnabled(true);
        PR_panel->setEnabled(true);

        //updating menu items
        /**/itm::debug(itm::LEV3, "updating menu items", __itm__current__function__);
        openTeraFlyVolumeAction->setEnabled(false);
        openHDF5VolumeAction->setEnabled(false);
        recentVolumesMenu->setEnabled(false);
        openVolumeToolButton->setEnabled(false);
        importOptionsMenu->setEnabled(false);
        closeVolumeAction->setEnabled(true);
        clearAnnotationsAction->setEnabled(true);
        showToolbarButton->setEnabled(true);

        //enabling multiresolution panel and hiding volume map options        
        gradientBar->setEnabled(true);
        resolution_cbox->setEnabled(true);
        this->zoom_panel->setEnabled(true);

        //enabling menu actions
        loadAnnotationsAction->setEnabled(true);
        saveAnnotationsAsAction->setEnabled(true);

        //updating gradient bar widget
        gradientBar->setEnabled(true);
        gradientBar->setNSteps(CImport::instance()->getResolutions());

        //inserting available resolutions
        /**/itm::debug(itm::LEV3, "inserting available resolutions", __itm__current__function__);
        resolution_cbox->setEnabled(false);
        for(int i=0; i<CImport::instance()->getResolutions(); i++)
        {
            QString option = "";
            VirtualVolume* vol = CImport::instance()->getVolume(i);
            float vxl_v = vol->getVXL_V() < 0 ? vol->getVXL_V()*-1 : vol->getVXL_V();
            float vxl_h = vol->getVXL_H() < 0 ? vol->getVXL_H()*-1 : vol->getVXL_H();
            float vxl_d = vol->getVXL_D() < 0 ? vol->getVXL_D()*-1 : vol->getVXL_D();
            option = option + QString::number(vol->getDIM_H()) + QChar(0x00D7) + QString::number(vol->getDIM_V()) + QChar(0x00D7) + QString::number(vol->getDIM_D()) +
                    " (voxel: " + QString::number(vxl_h, 'f', 1) + QChar(0x00D7) + QString::number(vxl_v, 'f', 1) + QChar(0x00D7) +
                    QString::number(vxl_d, 'f', 1) + " " + QChar(0x03BC)+"m)";
            resolution_cbox->insertItem(i, option);
        }
        resolution_cbox->setEnabled(true);

        //updating traslation widgets
        traslXlabel->setText("<font size=\"3\" color=\"red\"><b>x</b></font>");
        traslYlabel->setText("<font size=\"3\" color=\"green\"><b>y</b></font>");
        traslZlabel->setText("<font size=\"3\" color=\"blue\"><b>z</b></font>");
        traslTlabel->setText("<font size=\"3\" color=\"gray\"><b>t</b></font>");

        //disabling useless time-related widgets if data is < 5D
        to_label_4->setEnabled(volume->getDIM_T() > 1);
        frameCoord->setEnabled(volume->getDIM_T() > 1);
        traslTpos->setEnabled(volume->getDIM_T() > 1);
        traslTlabel->setEnabled(volume->getDIM_T() > 1);
        traslTneg->setEnabled(volume->getDIM_T() > 1);
        T0_sbox->setEnabled(volume->getDIM_T() > 1);
        T1_sbox->setEnabled(volume->getDIM_T() > 1);


        //instantiating CAnnotations
        /**/itm::debug(itm::LEV3, "instantiating CAnnotations", __itm__current__function__);
        CAnnotations::instance(volume->getDIM_V(), volume->getDIM_H(), volume->getDIM_D());

        //updating GUI time
        /**/itm::debug(itm::LEV3, "updating GUI time", __itm__current__function__);

        PLog::instance()->appendOperation(new ImportOperation( "TeraFly's GUI initialized", itm::GPU, timerGUI.elapsed()));

        //starting 3D exploration
        /**/itm::debug(itm::LEV3, "instantiating CViewer", __itm__current__function__);
        CViewer *new_win = initViewer(V3D_env, CImport::instance()->getVMapResIndex(), CImport::instance()->getVMapRawData(),
                            0, CImport::instance()->getVMapYDim(), 0, CImport::instance()->getVMapXDim(),
                            0, CImport::instance()->getVMapZDim(), 0, CImport::instance()->getVMapTDim()-1, CImport::instance()->getVMapCDim(), 0);
        /**/itm::debug(itm::LEV3, "showing CViewer", __itm__current__function__);
        new_win->show();
        new_win->isReady = true;

        helpBox->setText(HTbase);

        //finally storing in application settings the path of the opened volume
        CSettings::instance()->setVolumePathLRU(CImport::instance()->getPath());

        //and installing event filter on pixmap tooltip
        QPixmapToolTip::instance(this)->installEventFilter(this);

        //updating actual time
        PLog::instance()->appendOperation(new ImportOperation( "TeraFly 3D exploration started", itm::ALL_COMPS, CImport::instance()->timerIO.elapsed()));

        //activate annotation toolbar
        showToolbarButton->setChecked(true);
    }

    //resetting some widgets
    resetGUI();

}

// Separate initialization to allow inherited classes to define their own viewer
CViewer * PMain::initViewer(V3DPluginCallback2* _V3D_env, int _resIndex, itm::uint8* _imgData, int _volV0, int _volV1,
							int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer* _prev)
{
	CViewer *new_win = new CViewer(_V3D_env, _resIndex, _imgData, _volV0, _volV1,
                        _volH0, _volH1, _volD0, _volD1, _volT0, _volT1, _nchannels, _prev);
	return new_win;
}

//overrides closeEvent method of QWidget
void PMain::closeEvent(QCloseEvent *evt)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    if(evt)
    {
        if(progressBar->isEnabled() && QMessageBox::information(this, "Warning", "An operation is still in progress. Terminating it can be unsafe and cause Vaa3D to crash. \n"
                                                                        "\nPlease save your data first.", "Close TeraFly plugin", "Cancel"))
        {
            evt->ignore();
        }
        else
        {
            evt->accept();
            PMain::uninstance();
        }
    }
}


/**********************************************************************************
* Called when the GUI widgets that control application settings change.
* This is used to manage persistent platform-independent application settings.
***********************************************************************************/
void PMain::settingsChanged(int)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CSettings::instance()->setVOIdimV(Vdim_sbox->value());
    CSettings::instance()->setVOIdimH(Hdim_sbox->value());
    CSettings::instance()->setVOIdimD(Ddim_sbox->value());
    CSettings::instance()->setVOIdimT(Tdim_sbox->value());
    CSettings::instance()->setTraslX(xShiftSBox->value());
    CSettings::instance()->setTraslY(yShiftSBox->value());
    CSettings::instance()->setTraslZ(zShiftSBox->value());
    CSettings::instance()->setTraslT(tShiftSBox->value());
    CSettings::instance()->writeSettings();
    if(PDialogProofreading::isActive())
        PDialogProofreading::instance()->updateBlocks(0);
}

/**********************************************************************************
* Linked to resolution combobox
* This switches to the given resolution index.
***********************************************************************************/
void PMain::resolutionIndexChanged(int i)
{
    /**/itm::debug(itm::LEV1, strprintf("resolution = %d", i).c_str(), __itm__current__function__);

    try
    {
        if(resolution_cbox->isEnabled() && CViewer::getCurrent() && i > CViewer::getCurrent()->getResIndex())
        {
//            int voiV0 = CVolume::scaleVCoord(V0_sbox->value()-1, CImport::instance()->getResolutions()-1, i);
//            int voiV1 = CVolume::scaleVCoord(V1_sbox->value()-1, CImport::instance()->getResolutions()-1, i);
//            int voiH0 = CVolume::scaleHCoord(H0_sbox->value()-1, CImport::instance()->getResolutions()-1, i);
//            int voiH1 = CVolume::scaleHCoord(H1_sbox->value()-1, CImport::instance()->getResolutions()-1, i);
//            int voiD0 = CVolume::scaleDCoord(D0_sbox->value()-1, CImport::instance()->getResolutions()-1, i);
//            int voiD1 = CVolume::scaleDCoord(D1_sbox->value()-1, CImport::instance()->getResolutions()-1, i);

//            int voiTDim = std::min(static_cast<int>(CImport::instance()->getVMapTDim()), Tdim_sbox->value());
//            float MVoxels = ((voiV1-voiV0+1)/1024.0f)*((voiH1-voiH0+1)/1024.0f)*(voiD1-voiD0+1)*voiTDim;
//            if(QMessageBox::Yes == QMessageBox::question(this, "Confirm", QString("The volume to be loaded is ").append(QString::number(MVoxels, 'f', 1)).append(" MVoxels big.\n\nDo you confirm?"), QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes))
            {
                int currentRes = CViewer::getCurrent()->getResIndex();
                int x0 = CVolume::scaleHCoord(H0_sbox->value()-1, CImport::instance()->getResolutions()-1, currentRes);
                int x1 = CVolume::scaleHCoord(H1_sbox->value()-1, CImport::instance()->getResolutions()-1, currentRes);
                int y0 = CVolume::scaleVCoord(V0_sbox->value()-1, CImport::instance()->getResolutions()-1, currentRes);
                int y1 = CVolume::scaleVCoord(V1_sbox->value()-1, CImport::instance()->getResolutions()-1, currentRes);
                int z0 = CVolume::scaleDCoord(D0_sbox->value()-1, CImport::instance()->getResolutions()-1, currentRes);
                int z1 = CVolume::scaleDCoord(D1_sbox->value()-1, CImport::instance()->getResolutions()-1, currentRes);
                int t0 = CViewer::getCurrent()->volT0;
                int t1 = CViewer::getCurrent()->volT1;
                /**/itm::debug(itm::LEV3, strprintf("global VOI [%d,%d) [%d,%d) [%d,%d) rescaled to [%d,%d) [%d,%d) [%d,%d) at currentRes = %d",
                                                       H0_sbox->value()-1, H1_sbox->value()-1,
                                                       V0_sbox->value()-1, V1_sbox->value()-1,
                                                       D0_sbox->value()-1, D1_sbox->value()-1,
                                                       x0, x1, y0, y1, z0, z1, currentRes).c_str(), __itm__current__function__);
                CViewer::getCurrent()->newViewer(x1, y1, z1, i, t0, t1, false, -1, -1, -1, x0, y0, z0, true);
            }
//            else
//                resolution_cbox->setCurrentIndex(CViewer::getCurrent()->getResIndex());
        }
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        resetGUI();
        globalCoord_panel->setEnabled(true);
        resolution_cbox->setCurrentIndex(CViewer::getCurrent()->getResIndex());
    }
}

/**********************************************************************************
* Enables / Disables directional shift controls
***********************************************************************************/
void PMain::setEnabledDirectionalShifts(bool enabled)
{
    traslXneg->setEnabled(enabled);
    traslXpos->setEnabled(enabled);
    traslYneg->setEnabled(enabled);
    traslYpos->setEnabled(enabled);
    traslZneg->setEnabled(enabled);
    traslZpos->setEnabled(enabled);
    traslTneg->setEnabled(enabled);
    traslTpos->setEnabled(enabled);
}

/**********************************************************************************
* Called when the correspondent buttons are clicked
***********************************************************************************/
void PMain::traslXposClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CViewer* expl = CViewer::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        expl->newViewer((expl->volH1-expl->volH0)/2 + (expl->volH1-expl->volH0)*(100-CSettings::instance()->getTraslX())/100.0f,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}
void PMain::traslXnegClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CViewer* expl = CViewer::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        expl->newViewer((expl->volH1-expl->volH0)/2 - (expl->volH1-expl->volH0)*(100-CSettings::instance()->getTraslX())/100.0f,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}
void PMain::traslYposClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CViewer* expl = CViewer::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        expl->newViewer((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2 + (expl->volV1-expl->volV0)*(100-CSettings::instance()->getTraslY())/100.0f,
                      (expl->volD1-expl->volD0)/2, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}
void PMain::traslYnegClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CViewer* expl = CViewer::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        expl->newViewer((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2 - (expl->volV1-expl->volV0)*(100-CSettings::instance()->getTraslY())/100.0f,
                      (expl->volD1-expl->volD0)/2, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}
void PMain::traslZposClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CViewer* expl = CViewer::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        expl->newViewer((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2 + (expl->volD1-expl->volD0)*(100-CSettings::instance()->getTraslZ())/100.0f, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}
void PMain::traslZnegClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CViewer* expl = CViewer::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        expl->newViewer((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2 - (expl->volD1-expl->volD0)*(100-CSettings::instance()->getTraslZ())/100.0f, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}
void PMain::traslTposClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CViewer* expl = CViewer::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        int newT0 = Tdim_sbox->value() == 1 ? expl->volT0+1 : expl->volT0 + (expl->volT1-expl->volT0)*(100-CSettings::instance()->getTraslT())/100.0f;
        int newT1 = newT0 + (Tdim_sbox->value()-1);
        if(newT1 >= CImport::instance()->getTDim())
        {
            newT1 = CImport::instance()->getTDim() - 1;
            newT0 = newT1 - (Tdim_sbox->value()-1);
        }
        expl->newViewer((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2,
                      expl->volResIndex,
                      newT0,
                      newT1, false);
//                      expl->volT0 + (expl->volT1-expl->volT0)*(100-CSettings::instance()->getTraslT())/100.0f,
//                      expl->volT1 + (expl->volT1-expl->volT0)*(100-CSettings::instance()->getTraslT())/100.0f, false);
    }
}
void PMain::traslTnegClicked()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CViewer* expl = CViewer::getCurrent();
    if(expl && expl->isActive && !expl->toBeClosed)
    {
        int newT1 = Tdim_sbox->value() == 1 ? expl->volT1-1 : expl->volT1 - (expl->volT1-expl->volT0)*(100-CSettings::instance()->getTraslT())/100.0f;
        int newT0 = newT1 - (Tdim_sbox->value()-1);
        if(newT0 < 0)
        {
            newT0 = 0;
            newT1 = newT0 + (Tdim_sbox->value()-1);
        }
        expl->newViewer((expl->volH1-expl->volH0)/2,
                      (expl->volV1-expl->volV0)/2,
                      (expl->volD1-expl->volD0)/2,
                      expl->volResIndex,
                      newT0,
                      newT1, false);
//                      expl->volT0 - (expl->volT1-expl->volT0)*(100-CSettings::instance()->getTraslT())/100.0f,
//                      expl->volT1 - (expl->volT1-expl->volT0)*(100-CSettings::instance()->getTraslT())/100.0f, false);
    }
}

/**********************************************************************************
* Filters events generated by the widgets to which a help message must be associated
***********************************************************************************/
bool PMain::eventFilter(QObject *object, QEvent *event)
{
    if ((object == Vdim_sbox || object == Hdim_sbox || object == Ddim_sbox || object == Tdim_sbox) && localViewer_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTvoiDim);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == resolution_cbox) && localViewer_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTjumpToRes);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == gradientBar) && localViewer_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTresolution);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == zoomOutSens) && zoom_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTzoomOutThres);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);

        displayToolTip(zoomOutSens, event, QString::number(zoomOutSens->value()).toStdString());
    }
    else if((object == zoomInSens) && zoom_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTzoomInThres);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);

        displayToolTip(zoomInSens, event, QString::number(zoomInSens->value()).toStdString());
    }
    else if((object == zoomInMethod) && zoom_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTzoomInMethod);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == cacheSens) && zoom_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTcacheSens);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);

        displayToolTip(cacheSens, event, QString::number(cacheSens->value()).append("%").toStdString());

    }
    else if ((object == traslXpos || object == traslYpos || object == traslZpos || object == traslTpos) && globalCoord_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTtraslatePos);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if ((object == traslXneg || object == traslYneg || object == traslZneg || object == traslTneg) && globalCoord_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTtraslateNeg);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if ((object == V0_sbox || object == V1_sbox ||
              object == H0_sbox || object == H1_sbox ||
              object == D0_sbox || object == D1_sbox ||
              object == T0_sbox || object == T1_sbox ) && globalCoord_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTvolcuts);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == refSys) && globalCoord_panel->isEnabled())
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTrefsys);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if(object == PR_button && PR_button->isEnabled() && PR_button->text().compare("Start") == 0)
    {
        if(event->type() == QEvent::Enter)
            helpBox->setText(HTproofreading);
        else if(event->type() == QEvent::Leave)
            helpBox->setText(HTbase);
    }
    else if((object == PR_spbox || object == QPixmapToolTip::getInstance()) && PR_spbox->isEnabled())
    {
        if(event->type() == QEvent::Enter)
        {
            QPixmapToolTip* pixmapToolTip = QPixmapToolTip::instance();
            int width = PR_spbox->width();
            pixmapToolTip->setFixedWidth(width);
            pixmapToolTip->setFixedHeight(width);
            pixmapToolTip->move(PR_spbox->mapToGlobal(QPoint(0,0))-QPoint(0,width+10));
            PRblockSpinboxChanged(PR_spbox->value());
            pixmapToolTip->show();
            pixmapToolTip->raise();
            helpBox->setText(HTquickscan);

            //PR_spbox->setAttribute(Qt::WA_Hover); // this control the QEvent::ToolTip and QEvent::HoverMove
            QPoint gpos = PR_spbox->mapToGlobal(QPoint(0,10));
            QToolTip::showText(gpos, "Click and press Enter to load", PR_spbox);
            //PR_spbox->setFocus(Qt::MouseFocusReason);

        }
        else if(event->type() == QEvent::Leave)
        {
            QPixmapToolTip::instance()->hide();        
            helpBox->setText(HTbase);
        }
        else if(event->type() == 6)
        {
            QKeyEvent* key_evt = (QKeyEvent*)event;
            if(key_evt->key() == Qt::Key_Return)
            {
                PRblockSpinboxEditingFinished();
                return true;
            }
        }
    }
    return false;
}

/**********************************************************************************
* Displays tooltip when ToolTip, MouseMove or KeyPress events occur on the widget.
***********************************************************************************/
void PMain::displayToolTip(QWidget* widget, QEvent* event, string msg)
{
    widget->setAttribute(Qt::WA_Hover); // this control the QEvent::ToolTip and QEvent::HoverMove
    widget->setFocusPolicy(Qt::WheelFocus); // accept KeyPressEvent when mouse wheel move

    bool event_tip = false;
    QPoint pos(0,0);
    switch (event->type())
    {
        case QEvent::ToolTip: // must turned on by setAttribute(Qt::WA_Hover) under Mac 64bit
                pos = ((QHelpEvent*)event)->pos();
                event_tip = true;
                break;
        case QEvent::MouseMove: // for mouse dragging
                pos = ((QMouseEvent*)event)->pos();
                event_tip = true;
                break;
        case 6: //QEvent::KeyPress: // for arrow key dragging
                pos = widget->mapFromGlobal(widget->cursor().pos());
                event_tip = true;
                break;
    }
    if (event_tip)
    {
        QPoint gpos = widget->mapToGlobal(pos);
        QToolTip::showText(gpos, msg.c_str(), widget);
    }
}

/**********************************************************************************
* Called when controlsResetButton is clicked
***********************************************************************************/
void PMain::resetMultiresControls()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    cacheSens->setValue(70);
    zoomInSens->setValue(40);
    zoomOutSens->setValue(0);
}

//very useful (not included in Qt): disables the given item of the given combobox
void PMain::setEnabledComboBoxItem(QComboBox* cbox, int _index, bool enabled)
{
    // Get the index of the value to disable
    QModelIndex index = cbox->model()->index(_index,0);

    // These are the effective 'disable/enable' flags
    QVariant v1(Qt::NoItemFlags);
    QVariant v2(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    //the magic
    cbox->model()->setData( index, enabled ? v2 : v1, Qt::UserRole -1);
}


/**********************************************************************************
* Called when the correspondent debug actions are triggered
***********************************************************************************/
void PMain::addGaussianNoiseTriggered()
{
    iim::ADD_NOISE_TO_TIME_SERIES = addGaussianNoiseToTimeSeries->isChecked();
}

void PMain::debugAction1Triggered()
{
    /**/itm::debug(itm::NO_DEBUG, 0, __itm__current__function__);

//    try
//    {
//        VolumeConverter *vc = new VolumeConverter();
//        iim::DEBUG = iim::LEV_MAX;
//        vc->setSrcVolume("/Volumes/Volumes/test.purkinje.big.tiff.3D.oneblock/RES(2122x1951x608)/000000/000000_000000/000000_000000_000000.tif", iim::TIF3D_FORMAT.c_str());
//    }
//    catch(...){QMessageBox::warning(this, "error", "error");}
//    QString apo1FilePath = QFileDialog::getOpenFileName(this, tr("Select first APO file (assumed as TRUTH)"), 0,tr("APO files (*.apo)"));
//    if(!apo1FilePath.isEmpty())
//    {

//        QList<CellAPO> cells = readAPO_file(apo1FilePath);
//        for(int k=0; k<cells.size(); k++)
//            v3d_msg(itm::strprintf("n = %d\nname = \"%s\"\ncomment = \"%s\"\nz = %f\nx = %f\ny = %d",
//                                   cells[k].n, cells[k].name.toStdString().c_str(),
//                                   cells[k].comment.toStdString().c_str(), cells[k].z, cells[k].x, cells[k].y).c_str());
//    }
    //CViewer *viewer = CViewer::getCurrent();
    //v3d_msg(itm::strprintf("X=[%d,%d], Y=[%d,%d], Z[%d,%d]", viewer->anoH0, viewer->anoH1, viewer->anoV0, viewer->anoV1, viewer->anoD0, viewer->anoD1).c_str());
//    QPalette Pal(palette());
//    Pal.setColor(QPalette::Background, Qt::red);
//    QWidget *emptyPanel = new QWidget();
//    emptyPanel->setAutoFillBackground(true);
//    emptyPanel->setPalette(Pal);
//    emptyPanel->setFixedWidth(50);
//    CViewer::getCurrent()->window3D->centralLayout->insertWidget(0, emptyPanel);
    //CViewer::getCurrent()->window3D->centralLayout->takeAt(0);
    //QMessageBox::information(this, "position", itm::strprintf("x = %d, y = %d", this->mapToGlobal(pos()).x(), this->mapToGlobal(pos()).y()).c_str());

//    int n_points = QInputDialog::getInt(this, "",  "Number of points:");
//    int n_samples = QInputDialog::getInt(this, "", "Number of samples:");

//    int dimX = 10000;
//    int dimY = 10000;
//    int dimZ = 10000;
//    CAnnotations::instance(dimY, dimX, dimZ);
//    CAnnotations* cano = CAnnotations::getInstance();
//    LandmarkList input_markers;
//    for(int k=0; k<n_points; k++)
//        input_markers.push_back(LocationSimple(rand()%dimX, rand()%dimY, rand()%dimZ));
//    cano->addLandmarks(itm::interval_t(0, dimX), itm::interval_t(0, dimY), itm::interval_t(0, dimZ),input_markers);

//    float t_lin_sum = 0, t_lin_sum_sq = 0, t_oct_sum = 0, t_oct_sum_sq = 0;
//    for(int n=0; n<n_samples; n++)
//    {
//        int x0 = rand() % (dimX-256);
//        int y0 = rand() % (dimY-256);
//        int z0 = rand() % (dimZ-256);
//        int x1 = x0 + 256;
//        int y1 = y0 + 256;
//        int z1 = z0 + 256;
//        QList<LocationSimple> markers_VOI_lin, markers_VOI;

//        QElapsedTimer timer_ano;
//        timer_ano.start();
//        for(int k=0; k<input_markers.size(); k++)
//            if( input_markers[k].x >= x0 && input_markers[k].x <= x1 &&
//                input_markers[k].y >= y0 && input_markers[k].y <= y1 &&
//                input_markers[k].z >= z0 && input_markers[k].z <= z1)
//                markers_VOI_lin.push_back(input_markers[k]);
//        t_lin_sum += timer_ano.elapsed();
//        t_lin_sum_sq += std::pow(timer_ano.elapsed(), 2.0f);

//        timer_ano.restart();
//        cano->findLandmarks(itm::interval_t(x0, x1),
//                            itm::interval_t(y0, y1),
//                            itm::interval_t(z0, z1), markers_VOI);
//        t_oct_sum += timer_ano.elapsed();
//        t_oct_sum_sq += std::pow(timer_ano.elapsed(), 2.0f);
//    }

//    QMessageBox::information(this, "Results", itm::strprintf("SCAN_LIN -> %.5f stdev %.5f\nSCAN_OCT -> %.5f stdev %.5f",
//                                                             t_lin_sum/n_samples, sqrt(t_lin_sum_sq/n_samples - pow(t_lin_sum/n_samples, 2.0f)),
//                                                             t_oct_sum/n_samples, sqrt(t_oct_sum_sq/n_samples - pow(t_oct_sum/n_samples, 2.0f))).c_str());
//    CAnnotations::uninstance();



    //    printf("PMain is: %s\n", PMain::getInstance() ? "not null" : "NULL");

//    QMessageBox::information(0, "The number of annotations is...", QString::number(CAnnotations::getInstance()->count()));

/*QList<CellAPO> cellsT0 = readAPO_file(QFileDialog::getOpenFileName(this, tr("Open APO file (proofreading I)"), 0,tr("APO files (*.apo)")));
QList<CellAPO> cellsT1 = readAPO_file(QFileDialog::getOpenFileName(this, tr("Open APO file (proofreading II)"), 0,tr("APO files (*.apo)")));
QList<CellAPO> cells_output;

// FNb = blue cells in T1 --> display as BLUE
int FNb = 0;
for(int i=0; i<cellsT1.size(); i++)
    if(cellsT1[i].color.b == 255)
    {
        cells_output.push_back(cellsT1[i]);
        FNb++;
    }

// FNa = red cells in T1 ---> display as PURPLE
int FNa = 0;
for(int i=0; i<cellsT1.size(); i++)
    if(cellsT1[i].color.r == 255)
    {
        CellAPO cell = cellsT1[i];
        cell.color.r = cell.color.g = 151;
        cell.color.b = 255;
        cells_output.push_back(cell);
        FNa++;
    }

// FPb = red cells in T0 but not in T1 --> display as RED
int FPb = 0;
for(int i=0; i<cellsT0.size(); i++)
{
    if(cellsT0[i].color.r == 255)
    {
        bool found = false;
        for(int j=0; j<cellsT1.size() && !found; j++)
            if(cellsT1[j].color.r == 255 &&
               cellsT0[i].x == cellsT1[j].x &&  cellsT0[i].y == cellsT1[j].y && cellsT0[i].z == cellsT1[j].z     )
                found = true;
        if(!found)
        {
            cells_output.push_back(cellsT0[i]);
            FPb++;
        }
    }
}


// FPa = blue cells in T0 but not in T1 --> display as ORANGE
int FPa = 0;
for(int i=0; i<cellsT0.size(); i++)
{
    if(cellsT0[i].color.b == 255)
    {
        bool found = false;
        for(int j=0; j<cellsT1.size() && !found; j++)
            if(cellsT1[j].color.b == 255 &&
               cellsT0[i].x == cellsT1[j].x &&  cellsT0[i].y == cellsT1[j].y && cellsT0[i].z == cellsT1[j].z     )
                found = true;
        if(!found)
        {
            CellAPO cell = cellsT0[i];
            cell.color.r = 255;
            cell.color.g = 128;
            cell.color.b = 0;
            cells_output.push_back(cell);
            FPa++;
        }
    }
}

QMessageBox::information(this, "result", itm::strprintf("FPa = %d\nFNa = %d\nFPb = %d\nFNb = %d", FPa, FNa, FPb, FNb).c_str());

writeAPO_file(QFileDialog::getSaveFileName(this, "save result to", 0, tr("APO files (*.apo)")), cells_output);*/

//    CAnnotations::getInstance()->prune();

//    QMessageBox::information(0, "after pruning, the number of annotations is...", QString::number(CAnnotations::getInstance()->count()));

}

void PMain::showLogTriggered()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    PLog::instance()->show();
}

/**********************************************************************************
* Called when the correspondent Options->3D->Curve actions are triggered
***********************************************************************************/
void PMain::curveDimsChanged(int dim)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CSettings::instance()->setAnnotationCurvesDims(dim);
    CSettings::instance()->writeSettings();

    CViewer *cur_win = CViewer::getCurrent();
    if(cur_win)
    {
        /**/itm::debug(itm::LEV3, strprintf("set Vaa3D's renderer <lineWidth> = %d", curveDimsSpinBox->value()).c_str(), __itm__current__function__);
        cur_win->view3DWidget->getRenderer()->lineWidth = dim;
        cur_win->view3DWidget->updateTool();
        cur_win->view3DWidget->update();
    }
}

void PMain::curveAspectChanged()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CSettings::instance()->setAnnotationCurvesAspectTube(curveAspectTube->isChecked());
    CSettings::instance()->writeSettings();

    CViewer *cur_win = CViewer::getCurrent();
    if(cur_win)
    {
        /**/itm::debug(itm::LEV3, strprintf("set Vaa3D's renderer <lineWidth> = %d and <lineType> = %s", curveDimsSpinBox->value(), curveAspectTube->isChecked() ? "tube" : "skeleton").c_str(), __itm__current__function__);
        cur_win->view3DWidget->getRenderer()->lineWidth = curveDimsSpinBox->value();
        cur_win->view3DWidget->getRenderer()->lineType = !curveAspectTube->isChecked();
        cur_win->view3DWidget->updateTool();
        cur_win->view3DWidget->update();
    }
}

/**********************************************************************************
* Called when the corresponding Options->Navigation->Fetch-and-Display actions are triggered
***********************************************************************************/
void PMain::fetchAndDisplayChanged()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CSettings::instance()->setPreviewMode(fdPreviewAction->isChecked());
    CSettings::instance()->writeSettings();
}

/**********************************************************************************
* Called when the corresponding Options->3D annotation->Virtual space size actions are triggered
***********************************************************************************/
void PMain::virtualSpaceSizeChanged()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CSettings::instance()->setAnnotationSpaceUnlimited(spaceSizeUnlimited->isChecked());
}


/**********************************************************************************
* Linked to verbosity combobox
***********************************************************************************/
void PMain::verbosityChanged(int i)
{
    /**/itm::debug(itm::LEV1, strprintf("i = %d", i).c_str(), __itm__current__function__);

    itm::DEBUG = i;
    iim::DEBUG = i;
//    CSettings::instance()->writeSettings();
}

void PMain::PRbuttonClicked()
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    if(isPRactive())
        PRsetActive(false);
    else
        PDialogProofreading::instance()->show();
}

void PMain::PRsetActive(bool active)
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    // check precondition: 3D visualization is active
    CViewer* curWin = CViewer::getCurrent();
    if(!curWin)
        return;

    std::vector<itm::block_t> & blocks = PDialogProofreading::blocks;
    /* ------------------- "Local viewer" panel ---------------------- */
    resolution_cbox->setEnabled(!active);
    Vdim_sbox->setEnabled(!active);
    Hdim_sbox->setEnabled(!active);
    Ddim_sbox->setEnabled(!active);
    Tdim_sbox->setEnabled(!active);
    /* -------------------- "Zoom in/out" panel ---------------------- */
    zoom_panel->setEnabled(!active);
    if(active)
    {
        zoomInSens->setValue(zoomInSens->maximum());
        zoomOutSens->setValue(zoomOutSens->minimum());
    }
    else
        resetMultiresControls();
    /* ---------------- "Global coordinates" panel ------------------- */
    V0_sbox->setEnabled(!active);
    V1_sbox->setEnabled(!active);
    H0_sbox->setEnabled(!active);
    H1_sbox->setEnabled(!active);
    D0_sbox->setEnabled(!active);
    D1_sbox->setEnabled(!active);
    to_label_1->setEnabled(!active);
    to_label_2->setEnabled(!active);
    to_label_3->setEnabled(!active);
    if(frameCoord->isEnabled())
    {
        T0_sbox->setEnabled(!active);
        T1_sbox->setEnabled(!active);
        to_label_4->setEnabled(!active);
    }
    traslXlabel->setText(active ? "<font size=\"3\" color=\"gray\">x</font>" : "<font size=\"3\" color=\"red\"><b>x</b></font>");
    traslYlabel->setText(active ? "<font size=\"3\" color=\"gray\">y</font>" : "<font size=\"3\" color=\"green\"><b>y</b></font>");
    traslZlabel->setText(active ? "<font size=\"3\" color=\"gray\">z</font>" : "<font size=\"3\" color=\"blue\"><b>z</b></font>");
    traslTlabel->setText(active ? "<font size=\"3\" color=\"gray\">t</font>" : "<font size=\"3\" color=\"gray\"><b>t</b></font>");
    traslYneg->setEnabled(!active && curWin->volV0 > 0);
    traslYpos->setEnabled(!active && curWin->volV1 < CImport::instance()->getVolume(curWin->volResIndex)->getDIM_V());
    traslXneg->setEnabled(!active && curWin->volH0 > 0);
    traslXpos->setEnabled(!active && curWin->volH1 < CImport::instance()->getVolume(curWin->volResIndex)->getDIM_H());
    traslZneg->setEnabled(!active && curWin->volD0 > 0);
    traslZpos->setEnabled(!active && curWin->volD1 < CImport::instance()->getVolume(curWin->volResIndex)->getDIM_D());
    traslTneg->setEnabled(!active && curWin->volT0 > 0);
    traslTpos->setEnabled(!active && curWin->volT1 < CImport::instance()->getVolume(curWin->volResIndex)->getDIM_T()-1);
    refSys->setFilled(!active);
    if(active)
        refSys->setZoom(-8.0);
    else
    {
        refSys->setDims(curWin->volH1-curWin->volH0+1, curWin->volV1-curWin->volV0+1, curWin->volD1-curWin->volD0+1);
        refSys->resetZoom();
    }
    /* ------------------- "Proofreading" panel ---------------------- */
    PR_button->setText(active ? "Stop" : "Start");
    //PR_button->setIcon(active ? QIcon(":/icons/stop.png") : QIcon(":/icons/start.png"));
    PR_spbox->setEnabled(active);
    PR_spbox->setSuffix(active ? " of " + QString::number(blocks.size()) : " of 0");
    PR_spbox->setMaximum(active ? static_cast<int>(blocks.size()): 0);
    PR_spbox->setMinimum(active ? 1: 0);
    PR_spbox->setValue(active ? 1: 0);
}

void PMain::PRstart()
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    PRsetActive(true);
    PRblockSpinboxEditingFinished();
}

/**********************************************************************************
* Called when the correspondent spin box has changed
***********************************************************************************/
void PMain::PRblockSpinboxEditingFinished()
{
    std::vector<itm::block_t> & blocks = PDialogProofreading::blocks;
    int blocks_res = PDialogProofreading::blocks_res;
    int b = PR_spbox->value();

    if(b == 0 || !PR_spbox->isEnabled() || blocks.empty())
        return;

    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    CViewer* curWin = CViewer::getCurrent();
    if(curWin && curWin->isActive && !curWin->toBeClosed)
    {
        // if the selected block is the one being viewed, exit
        if(curWin->slidingViewerBlockID == b)
            return;

        // update reference system
        int ROIxS   = blocks[b-1].xInt.start;
        int ROIxDim = blocks[b-1].xInt.end   - ROIxS;
        int ROIyS   = blocks[b-1].yInt.start;
        int ROIyDim = blocks[b-1].yInt.end   - ROIyS;
        int ROIzS   = blocks[b-1].zInt.start;
        int ROIzDim = blocks[b-1].zInt.end   - ROIzS;
        int dimX    = CImport::instance()->getVolume(blocks_res)->getDIM_H();
        int dimY    = CImport::instance()->getVolume(blocks_res)->getDIM_V();
        int dimZ    = CImport::instance()->getVolume(blocks_res)->getDIM_D();
        refSys->setDims(dimX, dimY, dimZ, ROIxDim, ROIyDim, ROIzDim, ROIxS, ROIyS, ROIzS);

        // invoke new view
        curWin->newViewer(blocks[b-1].xInt.end, blocks[b-1].yInt.end, blocks[b-1].zInt.end, blocks_res,
                curWin->volT0, curWin->volT1, false, -1, -1, -1, blocks[b-1].xInt.start, blocks[b-1].yInt.start, blocks[b-1].zInt.start, true, false, b);

//        expl->newView((expl->volH1-expl->volH0)/2,
//                      (expl->volV1-expl->volV0)/2 - (expl->volV1-expl->volV0)*(100-CSettings::instance()->getTraslY())/100.0f,
//                      (expl->volD1-expl->volD0)/2, expl->volResIndex, expl->volT0, expl->volT1, false);
    }
}

/**********************************************************************************
* Called when the corresponding spin box has changed
***********************************************************************************/
void PMain::PRblockSpinboxChanged(int b)
{
    if(b == 0 || !PR_spbox->isEnabled())
        return;

    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    try
    {
        CViewer* curWin = CViewer::getCurrent();
        std::vector<itm::block_t> & blocks = PDialogProofreading::blocks;
        int blocks_res = PDialogProofreading::blocks_res;
        if(curWin && curWin->isActive && !curWin->toBeClosed)
        {
            // update 3D reference system
            int ROIxS   = blocks[b-1].xInt.start;
            int ROIxDim = blocks[b-1].xInt.end   - ROIxS;
            int ROIyS   = blocks[b-1].yInt.start;
            int ROIyDim = blocks[b-1].yInt.end   - ROIyS;
            int ROIzS   = blocks[b-1].zInt.start;
            int ROIzDim = blocks[b-1].zInt.end   - ROIzS;
            int dimX   = CImport::instance()->getVolume(blocks_res)->getDIM_H();
            int dimY   = CImport::instance()->getVolume(blocks_res)->getDIM_V();
            int dimZ   = CImport::instance()->getVolume(blocks_res)->getDIM_D();
            refSys->setDims(dimX, dimY, dimZ, ROIxDim, ROIyDim, ROIzDim, ROIxS, ROIyS, ROIzS);

            // compute block coordinates in the lowest resolution image space (to be used for quickly generating a low-res mip of the block)
            int ROIxs_lr = CVolume::scaleHCoord(ROIxS,          blocks_res, 0);
            int ROIxe_lr = CVolume::scaleHCoord(ROIxS+ROIxDim,  blocks_res, 0);
            int ROIys_lr = CVolume::scaleVCoord(ROIyS,          blocks_res, 0);
            int ROIye_lr = CVolume::scaleVCoord(ROIyS+ROIyDim,  blocks_res, 0);
            int ROIzs_lr = CVolume::scaleDCoord(ROIzS,          blocks_res, 0);
            int ROIze_lr = CVolume::scaleDCoord(ROIzS+ROIzDim,  blocks_res, 0);

            // compute block coordinates in the highest resolution image space (to be used for displaying info data)
            int ROIxs_hr = CVolume::scaleHCoord(ROIxS,          blocks_res, CImport::instance()->getResolutions()-1);
            int ROIxe_hr = CVolume::scaleHCoord(ROIxS+ROIxDim,  blocks_res, CImport::instance()->getResolutions()-1);
            int ROIys_hr = CVolume::scaleVCoord(ROIyS,          blocks_res, CImport::instance()->getResolutions()-1);
            int ROIye_hr = CVolume::scaleVCoord(ROIyS+ROIyDim,  blocks_res, CImport::instance()->getResolutions()-1);
            int ROIzs_hr = CVolume::scaleDCoord(ROIzS,          blocks_res, CImport::instance()->getResolutions()-1);
            int ROIze_hr = CVolume::scaleDCoord(ROIzS+ROIzDim,  blocks_res, CImport::instance()->getResolutions()-1);

            // count number of annotations
            int ROI_ano_count = 0;
            if(CAnnotations::getInstance()->getOctree())
                ROI_ano_count = CAnnotations::getInstance()->getOctree()->count(interval_t(ROIys_hr,ROIye_hr), interval_t(ROIxs_hr, ROIxe_hr), interval_t(ROIzs_hr,ROIze_hr));

            // count number of annotations in neighborhood
            int ROI_ano_perc_count = 0;
            interval_t x_range_perc(ROIxs_hr, ROIxe_hr);
            interval_t y_range_perc(ROIys_hr, ROIye_hr);
            interval_t z_range_perc(ROIzs_hr, ROIze_hr);
            int vmPerc = markersShowROIMarginSpinBox->value();
            int vmX = (x_range_perc.end - x_range_perc.start)*(vmPerc/100.0f)/2;
            int vmY = (y_range_perc.end - y_range_perc.start)*(vmPerc/100.0f)/2;
            int vmZ = (z_range_perc.end - z_range_perc.start)*(vmPerc/100.0f)/2;
            x_range_perc.start  = std::max(0, x_range_perc.start - vmX);
            x_range_perc.end   += vmX;
            y_range_perc.start  = std::max(0, y_range_perc.start - vmY);
            y_range_perc.end   += vmY;
            z_range_perc.start  = std::max(0, z_range_perc.start - vmZ);
            z_range_perc.end   += vmZ;
            if(CAnnotations::getInstance()->getOctree())
                ROI_ano_perc_count = CAnnotations::getInstance()->getOctree()->count(y_range_perc, x_range_perc, z_range_perc);


            // compute z-mip of the selected block on the lowest resolution image
            uint8 *mip = CViewer::first->getMIP(ROIxs_lr, ROIxe_lr, ROIys_lr, ROIye_lr, ROIzs_lr, ROIze_lr, -1, -1, itm::z, true, 240);
            QImage qmip(mip, ROIxe_lr-ROIxs_lr, ROIye_lr-ROIys_lr, QImage::Format_ARGB32);

            // apply current color map to the mip
            CImageUtils::applyVaa3DColorMap(qmip, ((Renderer_gl2*)(curWin->view3DWidget->getRenderer()))->colormap);

            // display mip and text info in the pixmap tooltip
            QPixmapToolTip *qPixmapToolTip = QPixmapToolTip::instance();
            if(qPixmapToolTip->raw())
                delete[] qPixmapToolTip->raw();     // delete previously displayed mip
            qPixmapToolTip->setImage(qmip, mip);
            qPixmapToolTip->setText(strprintf("X = [%d, %d]\nY = [%d,%d]\nZ = [%d, %d]\nano points = %d\nano points within %d%% margin = %d",
                                                          ROIxs_hr+1, ROIxe_hr, ROIys_hr+1, ROIye_hr, ROIzs_hr+1, ROIze_hr, ROI_ano_count,
                                              markersShowROIMarginSpinBox->value(), ROI_ano_perc_count ).c_str());

            // @FIXED by Alessandro on 2015-03-02. Causes crash on Mac. On the other hand, this is already done in the ::eventFilter.
            // display tooltip below spinbox
//            QPoint gpos = PR_spbox->mapToGlobal(QPoint(0,10));
//            QToolTip::showText(gpos, "Press Enter to load", PR_spbox);
        }
    }
    catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

/**********************************************************************************
* Called when the corresponding QLineEdit has been edited
***********************************************************************************/
void PMain::debugRedirectSTDoutPathEdited(QString s)
{
    if(s.isEmpty())
    {
        itm::DEBUG_TO_FILE = false;
        iim::DEBUG_TO_FILE = false;
    }
    else
    {
        itm::DEBUG_TO_FILE = true;
        itm::DEBUG_FILE_PATH = s.toStdString();
        iim::DEBUG_TO_FILE = true;
        iim::DEBUG_FILE_PATH = s.toStdString();
    }
}


/**********************************************************************************
* <sendProgressBarChanged> event handler
***********************************************************************************/
void PMain::progressBarChanged(int val, int minutes, int seconds, const char* message)
{
    /**/itm::debug(itm::LEV3, strprintf("val = %d, minutes = %d, seconds = %d, message = %s", val, minutes, seconds, message).c_str(), __itm__current__function__);

    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->setValue(val);
    QString remaining_time = QString::number(minutes);
    remaining_time.append(" minutes and ");
    remaining_time.append(QString::number(seconds));
    remaining_time.append(" seconds remaining");
    if(message && strlen(message) != 0)
    {
        statusBar->showMessage(message + QString(": ") + remaining_time);
    }
    else
        statusBar->showMessage(remaining_time);
}

/**********************************************************************************
* Called when showButton state has changed
***********************************************************************************/
void PMain::showToolbarButtonChanged(bool changed)
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);
#ifndef HIDE_ANO_TOOLBAR
    if(PAnoToolBar::isInstantiated())
        PAnoToolBar::instance()->setVisible(changed);
#endif
}


/**********************************************************************************
* Called when markersShowROIMarginSpinBox state has changed
***********************************************************************************/
void PMain::markersShowROIMarginSpinBoxChanged(int value)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    CSettings::instance()->setAnnotationVirtualMargin(value);
    CSettings::instance()->writeSettings();
    if(PAnoToolBar::isInstantiated())
        PAnoToolBar::instance()->buttonMarkerRoiViewChecked(PAnoToolBar::instance()->buttonMarkerRoiView->isChecked());
}

/**********************************************************************************
* Called when markersSizeSpinBox state has changed
***********************************************************************************/
void PMain::markersSizeSpinBoxChanged(int value)
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);
    CSettings::instance()->setAnnotationMarkerSize(value);
    CSettings::instance()->writeSettings();

    CViewer *cur_win = CViewer::getCurrent();
    if(cur_win)
    {
        cur_win->view3DWidget->getRenderer()->markerSize = std::max(5, static_cast<int>( value / CImport::instance()->getResRatio(cur_win->volResIndex) + 0.5f));
        cur_win->view3DWidget->updateTool();
        cur_win->view3DWidget->update();
    }
}

void PMain::showDialogVtk2APO()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    try
    {
        QString vtkFilePath = QFileDialog::getOpenFileName(this, tr("Select input file"), 0,tr("VTK ascii files (*.vtk)"));
        if(!vtkFilePath.isEmpty())
        {
            QString apoFilePath = QFileDialog::getSaveFileName(this, tr("Save to"), 0,tr("APO files (*.apo)"));
            if(!apoFilePath.isEmpty())
            {
                itm::CAnnotations::convertVtk2APO(vtkFilePath.toStdString(), apoFilePath.toStdString());
                v3d_msg("DONE!");
            }
        }
    }
    catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

void PMain::showDialogMaMut2APO()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    try
    {
        QString xmlFilePath = QFileDialog::getOpenFileName(this, tr("Select input file"), 0,tr("MaMuT input files (*.xml)"));
        if(!xmlFilePath.isEmpty())
        {
            QString apoFilePath = QFileDialog::getSaveFileName(this, tr("Save to"), 0,tr("APO files (*.apo)"));
            if(!apoFilePath.isEmpty())
            {
                itm::CAnnotations::convertMaMuT2APO(xmlFilePath.toStdString(), apoFilePath.toStdString());
                v3d_msg("DONE!");
            }
        }
    }
    catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

void PMain::showDialogTypeIandTypeIIerrors()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    try
    {
        QString apo1FilePath = QFileDialog::getOpenFileName(this, tr("Select first APO file (assumed as TRUTH)"), 0,tr("APO files (*.apo)"));
        if(!apo1FilePath.isEmpty())
        {
            QString apo2FilePath = QFileDialog::getOpenFileName(this, tr("Select second APO file"), 0,tr("APO files (*.apo)"));
            if(!apo2FilePath.isEmpty())
            {
                QString outputPath = QFileDialog::getSaveFileName(this, tr("Save output to"), 0,tr("APO files (*.apo)"));
                if(!outputPath.isEmpty())
                {
                    int d = QInputDialog::getInt(this, "Input 1/2", "Tolerance distance:", 5, 0);
                    std::string ID = QInputDialog::getText(this, "Input 2/2", "Filter cells by name (no ',' or '.' characters):", QLineEdit::Normal, "none").toStdString();
                    std::pair<int,int> errors = itm::CAnnotations::typeIandIIerrorAPO(apo1FilePath.toStdString(), apo2FilePath.toStdString(), d, ID, outputPath.toStdString());
                    v3d_msg(itm::strprintf("Type I  errors (false positives): %d\n"
                            "Type II errors (false negatives): %d", errors.first, errors.second).c_str());
                }
            }
        }
    }
    catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

void PMain::showDialogDiffAPO()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    try
    {
        QString apo1FilePath = QFileDialog::getOpenFileName(this, tr("Select first APO file (assumed as TRUTH)"), 0,tr("APO files (*.apo)"));
        if(!apo1FilePath.isEmpty())
        {
            QString apo2FilePath = QFileDialog::getOpenFileName(this, tr("Select second APO file"), 0,tr("APO files (*.apo)"));
            if(!apo2FilePath.isEmpty())
            {
                QString savePath = QFileDialog::getSaveFileName(this, tr("Output file"), 0,tr("APO files (*.apo)"));
                if(!savePath.isEmpty())
                {
                    itm::CAnnotations::diffAPO(apo1FilePath.toStdString(), apo2FilePath.toStdString(), H0_sbox->value(), H1_sbox->value(),
                                           V0_sbox->value(), V1_sbox->value(), D0_sbox->value(), D1_sbox->value(), savePath.toStdString());

                    v3d_msg("DONE!");
                }
            }
        }
    }
    catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

void PMain::showDialogTrimAPO()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    try
    {
        QString apo1FilePath = QFileDialog::getOpenFileName(this, tr("Open input APO file"), 0,tr("APO files (*.apo)"));
        if(!apo1FilePath.isEmpty())
        {
            QString apo2FilePath = QFileDialog::getSaveFileName(this, tr("Select output APO file"), 0,tr("APO files (*.apo)"));
            if(!apo2FilePath.isEmpty())
            {
                int x0 = QInputDialog::getInt(this, "Select VOI", "[x0");
                int x1 = QInputDialog::getInt(this, "Select VOI", "x1)");
                int y0 = QInputDialog::getInt(this, "Select VOI", "[y0");
                int y1 = QInputDialog::getInt(this, "Select VOI", "y1)");
                int z0 = QInputDialog::getInt(this, "Select VOI", "[z0");
                int z1 = QInputDialog::getInt(this, "Select VOI", "z1)");
                itm::CAnnotations::trimAPO(apo1FilePath.toStdString(), apo2FilePath.toStdString(), x0, x1, y0, y1, z0, z1);

                v3d_msg("DONE!");
            }
        }
    }
    catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

void itm::PMain::showDialogLabelDuplicateAPO()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    try
    {
        QString apo1FilePath = QFileDialog::getOpenFileName(this, tr("Open input APO file"), 0,tr("APO files (*.apo)"));
        if(!apo1FilePath.isEmpty())
        {
            QString apo2FilePath = QFileDialog::getSaveFileName(this, tr("Select output APO file"), 0,tr("APO files (*.apo)"));
            if(!apo2FilePath.isEmpty())
            {
                int d = QInputDialog::getInt(this, "Input 1/4", "Tolerance distance:", 5, 0);
                int r = QInputDialog::getInt(this, "Input 2/4", "Label color (R): ",255,0,255);
                int g = QInputDialog::getInt(this, "Input 3/4", "Label color (G): ",255,0,255);
                int b = QInputDialog::getInt(this, "Input 4/4", "Label color (B): ",255,0,255);
                RGBA8 color;
                color.r = r;
                color.g = g;
                color.b = b;
                itm::CAnnotations::labelDuplicates(apo1FilePath.toStdString(), apo2FilePath.toStdString(), d, color);

                v3d_msg("DONE!");
            }
        }
    }
    catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

void itm::PMain::showDialogDiffnAPO()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    try
    {
        QStringList apos = QFileDialog::getOpenFileNames(this, "Select two (or more) input files", 0, tr("APO files (*.apo)"));
        if(apos.size() < 2)
            throw itm::RuntimeException("At least two .apo files should be selected");

        QString apoPath = QFileDialog::getSaveFileName(this, tr("Select output APO file"), 0,tr("APO files (*.apo)"));
        if(apoPath.size())
        {
            itm::CAnnotations::diffnAPO(apos, apoPath.toStdString());
            v3d_msg("DONE!");
        }
    }
    catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

void PMain::showDialogMergeImageJCellCounterXMLs()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    try
    {
        QMessageBox::warning(this, ".xml (ImageJ Cell Counter markers) -> .apo", "<html>Before selecting the file(s), be aware that<ul>"
            "<li>filenames should end with xyz (e.g. example142.xml), where (x,y,z) are the block indices along the X,Y,Z axes</li>"
            "<li>(x,y,z) indices must be in the range [0,9]</li>"
            "<li>after file(s) selection, you will be asked to provide blocks size, overlap and top-left corner coordinate of block (0,0,0)</li></ul></html>");
        QStringList xmls = QFileDialog::getOpenFileNames(this, "Select one (or more) file(s)", 0, tr("ImageJ Cell Counter markers (*.xml)"));
        QString apoPath = QFileDialog::getSaveFileName(this, tr("Select output APO file"), 0,tr("APO files (*.apo)"));
        if(!xmls.isEmpty())
        {
            int blocks_overlap = QInputDialog::getInt(this, ".xml (ImageJ Cell Counter markers) -> .apo", "Insert blocks overlap:", 0, 0);
            int xS = QInputDialog::getInt(this, ".xml (ImageJ Cell Counter markers) -> .apo", "Insert blocks size along X axis:", 0, 0);
            int yS = QInputDialog::getInt(this, ".xml (ImageJ Cell Counter markers) -> .apo", "Insert blocks size along Y axis:", 0, 0);
            int zS = QInputDialog::getInt(this, ".xml (ImageJ Cell Counter markers) -> .apo", "Insert blocks size along Z axis:", 0, 0);
            int x0 = QInputDialog::getInt(this, ".xml (ImageJ Cell Counter markers) -> .apo", "Insert top-left corner X coordinate:", 0, 0);
            int y0 = QInputDialog::getInt(this, ".xml (ImageJ Cell Counter markers) -> .apo", "Insert top-left corner Y coordinate:", 0, 0);
            int z0 = QInputDialog::getInt(this, ".xml (ImageJ Cell Counter markers) -> .apo", "Insert top-left corner Z coordinate:", 0, 0);
            itm::CAnnotations::mergeImageJCellCounterXMLs(xmls, apoPath.toStdString(), xS, yS, zS, blocks_overlap, x0, y0, z0);

            v3d_msg("DONE!");
        }
    }
    catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

void PMain::showDialogCountDuplicateMarkers()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    if(!itm::CViewer::current)
    {
        v3d_msg("No image opened");
        return;
    }

    int d = QInputDialog::getInt(this, "Input required", "Tolerance distance:");
    int count = itm::CAnnotations::getInstance()->countDuplicateMarkers(d);
    QMessageBox::information(this, "Result", itm::strprintf("Found %d markers", count).c_str());
    itm::CViewer::current->clearAnnotations();
    itm::CViewer::current->loadAnnotations();
}

void PMain::showDialogGenerateTimeSeriesInterpolation()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    try
    {
        std::string im1Path = QFileDialog::getOpenFileName(this, tr("Select frame A image")).toStdString();
        if(!im1Path.empty())
        {
            std::string im2Path = QFileDialog::getOpenFileName(this, tr("Select frame B image"), itm::cdUp(im1Path).c_str()).toStdString();
            if(!im2Path.empty())
            {
                std::string dirPath = QFileDialog::getExistingDirectory(this, tr("Select output directory"), 0, QFileDialog::ShowDirsOnly).toStdString();
                if(!dirPath.empty())
                {
                    int nSteps = QInputDialog::getInt(this, "Insert", "Number of output frames:", 100, 2);
                    progressBar->setEnabled(true);
                    progressBar->setMinimum(0);
                    progressBar->setMaximum(nSteps);
                    progressBar->setValue(0);
                    statusBar->showMessage("Loading frame A...");
                    Image4DSimple* im1 =  V3D_env->loadImage(const_cast<char*>(im1Path.c_str()));
                    statusBar->showMessage("Loading frame B...");
                    Image4DSimple* im2 =  V3D_env->loadImage(const_cast<char*>(im2Path.c_str()));
                    for(int k=0; k<=nSteps; k++)
                    {
                        statusBar->showMessage(itm::strprintf("Processing frame %d/%d...", k+1, nSteps+1).c_str());
                        QTime dieTime= QTime::currentTime().addSecs(1);
                            while (QTime::currentTime() < dieTime)
                                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                        Image4DSimple* out = itm::CImageUtils::interpolateLinear(im1, im2, k, nSteps);
                        out->saveImage(itm::strprintf("%s/frame%07d.v3draw", dirPath.c_str(), k).c_str());
                        delete out;
                        progressBar->setValue(k);
                    }
                    this->resetGUI();
                }
            }
        }
    }
    catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        this->resetGUI();
    }
}

void PMain::showDialogGenerateTimeSeriesReplication()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    try
    {
        std::string im1Path = QFileDialog::getOpenFileName(this, tr("Select single-frame image")).toStdString();
        if(!im1Path.empty())
        {
            std::string dirPath = QFileDialog::getExistingDirectory(this, tr("Select output directory"), 0, QFileDialog::ShowDirsOnly).toStdString();
            if(!dirPath.empty())
            {
                int nSteps = QInputDialog::getInt(this, "Insert", "Number of output frames:", 100, 2);
                progressBar->setEnabled(true);
                progressBar->setMinimum(0);
                progressBar->setMaximum(nSteps);
                progressBar->setValue(0);
                statusBar->showMessage("Loading frame...");
                Image4DSimple* im1 =  V3D_env->loadImage(const_cast<char*>(im1Path.c_str()));
                for(int k=0; k<nSteps; k++)
                {
                    statusBar->showMessage(itm::strprintf("Processing frame %d/%d...", k+1, nSteps).c_str());
                    QTime dieTime= QTime::currentTime().addSecs(1);
                        while (QTime::currentTime() < dieTime)
                            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

                    Image4DSimple* out = itm::CImageUtils::addGaussianNoise(im1,static_cast<float>(k)/(nSteps-1));
                    out->saveImage(itm::strprintf("%s/frame%07d.v3draw", dirPath.c_str(), k).c_str());
                    delete out;
                    progressBar->setValue(k);
                }
                this->resetGUI();
            }
        }
    }
    catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        this->resetGUI();
    }
}

void PMain::showAnoOctree()
{
    /**/itm::debug(itm::LEV2, 0, __itm__current__function__);

    try
    {
        CViewer *cur_win = CViewer::getCurrent();
        if(cur_win)
        {
            // display warning: current annotations will be lost
            if(QMessageBox::Yes == QMessageBox::question(this, "Confirm", QString("Current annotations will be lost. \n\nProceed?"), QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes))
            {
                // update Octree with recent changes
                cur_win->storeAnnotations();

                // get NeuronTree from the Octree
                NeuronTree nt = CAnnotations::getInstance()->getOctree()->toNeuronTree();

                // clear annotations
                cur_win->clearAnnotations();

                // add NeuronTree
                interval_t x_range(cur_win->anoH0, cur_win->anoH1);
                interval_t y_range(cur_win->anoV0, cur_win->anoV1);
                interval_t z_range(cur_win->anoD0, cur_win->anoD1);
                CAnnotations::getInstance()->addCurves(x_range, y_range, z_range, nt);

                // update displayed annotations
                cur_win->loadAnnotations();
            }
        }
        else
            QMessageBox::warning(this, "Warning", "No 3D viewer found");
    }
    catch(itm::RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

void PMain::tabIndexChanged(int value)
{
    helpBox->setVisible(value == 0);
}
