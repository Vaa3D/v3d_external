// Fix windows compile problem with windows.h being included too late.
// I wish it wasn't included at all...
#ifdef _MSC_VER
#define NOMINMAX //added by PHC, 2010-05-20 to overcome VC min max macro
#include <windows.h>
#endif

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <iostream>
#include <cmath>
#include <cassert>
#include "NaMainWindow.h"
#include "Na3DWidget.h"
#include "ui_NaMainWindow.h"
#include "../../basic_c_fun/v3d_message.h"
#include "../../v3d/v3d_application.h"
#include "../DataFlowModel.h"
#include "../MultiColorImageStackNode.h"
#include "../NeuronAnnotatorResultNode.h"
#include "../TimebasedIdentifierGenerator.h"
#include "RendererNeuronAnnotator.h"
#include "GalleryButton.h"
#include "../../cell_counter/CellCounter3D.h"
#include "../NeuronSelector.h"
#include "FragmentGalleryWidget.h"
#include "AnnotationWidget.h"
#include "../utility/loadV3dFFMpeg.h"

using namespace std;
using namespace jfrc;

//////////////////
// NutateThread //
//////////////////

NutateThread::NutateThread(qreal cyclesPerSecond, QObject * parentObj /* = NULL */)
    : QThread(parentObj)
    , speed(cyclesPerSecond)
    , interval(0.200) // update every 200 milliseconds
    , currentAngle(0.0)
{
    deltaAngle = 2.0 * 3.14159 * cyclesPerSecond * interval;
}

void NutateThread::run()
{
    while(true) {
        if (paused) {
            msleep(500);
            continue;
        }
        // qDebug() << "nutation angle = " << currentAngle;
        rot = deltaNutation(currentAngle, deltaAngle);
        emit nutate(rot);
        currentAngle += deltaAngle;
        while (currentAngle > 2.0 * 3.14159) currentAngle -= 2.0 * 3.14159;
        msleep( (1000.0 * deltaAngle) / (2.0 * 3.14159 * speed) );
    }
}
void NutateThread::pause() {paused = true;}
void NutateThread::unpause() {paused = false;}


//////////////////
// NaMainWindow //
//////////////////

NaMainWindow::NaMainWindow(QWidget * parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , nutateThread(NULL)
    , statusProgressBar(NULL)
    , neuronSelector(this)
    , undoStack(NULL)
    , showAllNeuronsInEmptySpaceAction(NULL)
    , hideAllAction(NULL)
    , selectNoneAction(NULL)
    , neuronContextMenu(NULL)
    , viewerContextMenu(NULL)
    , recentViewer(VIEWER_3D)
    , dynamicRangeTool(NULL)
    , isInCustomCutMode(false)
    , bShowCrosshair(true) // default to on
    , viewMode(VIEW_SINGLE_STACK)
    , recentFileActions(NaMainWindow::maxRecentFiles, NULL)
{
    // Set up potential 3D stereo modes before creating QGLWidget.
#ifdef ENABLE_STEREO
    QGLFormat glFormat = QGLFormat::defaultFormat();
    glFormat.setStereo(true);
    glFormat.setDoubleBuffer(true);
    if (glFormat.stereo())
        qDebug() << "Attempting to set 3D stereo format";
    else
        qDebug() << "Failed to make stereo 3D default QGLFormat";
    QGLFormat::setDefaultFormat(glFormat);
#endif

    ui.setupUi(this);
    setAcceptDrops(true);

    // Z value comes from camera model
    qRegisterMetaType<Vector3D>("Vector3D");

    // hide neuron gallery until there are neurons to show
    setViewMode(VIEW_SINGLE_STACK);
    // ui.mipsFrame->setVisible(false);

    // hide compartment map until it works correctly and is not so slow on Mac
    ui.compartmentSelectGroupBox->hide();

    dataFlowModel=0;
    // TODO - neuronSelector should probably be a member of Na3DViewer, not of NaMainWindow
    // neuronSelector = new NeuronSelector(this);

    // Create stubs for recent file menu
    for (int i = 0; i < maxRecentFiles; ++i) {
        recentFileActions[i] = new OpenFileAction(this);
        ui.menuOpen_Recent->addAction(recentFileActions[i]);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(openFileRequested(QString)),
                this, SLOT(openMulticolorImageStack(QString)));
    }
    updateRecentFileActions();

    // Create an area in the status bar for progress messages.
    statusProgressMessage = new QLabel(NULL);
    statusBar()->addWidget(statusProgressMessage);
    statusProgressBar = new QProgressBar(NULL);
    statusProgressBar->setValue(0);
    statusProgressBar->setMinimum(0);
    statusProgressBar->setMaximum(100);
    statusBar()->addWidget(statusProgressBar);
    statusProgressBar->hide();
    statusProgressMessage->hide();

    // hide progress bar for 3d viewer until it is needed
    ui.widget_progress3d->hide();

    // hide the File->Open 3D Image stack menu item
    ui.menuFile->removeAction(ui.actionLoad_Tiff);
    ui.menuFile->removeAction(ui.actionCell_Counter_3D_2ch_lsm);

#ifdef USE_FFMPEG
    ui.actionLoad_movie_as_texture->setVisible(true);
    ui.actionLoad_fast_separation_result->setVisible(true);
#else
    ui.actionLoad_movie_as_texture->setVisible(false);
    ui.actionLoad_fast_separation_result->setVisible(false);
#endif

    // visualize compartment map
    //QDockWidget *dock = new QDockWidget(tr("Compartment Map"), this);
    //dock->setWidget( ui.compartmentMapWidget);
    qRegisterMetaType<QList<LabelSurf> >("QList<LabelSurf>");

    ui.compartmentMapWidget->setComboBox(ui.compartmentMapComboBox);
    connect(ui.compartmentMapComboBox, SIGNAL(currentIndexChanged(int)), ui.compartmentMapWidget, SLOT(switchCompartment(int)));
    //connect(ui.compartmentMapWidget, SIGNAL(viscomp3dview(QList<LabelSurf>)), (Renderer_gl1*)(ui.v3dr_glwidget->getRenderer()), SLOT(setListLabelSurf(QList<LabelSurf>))); // vis compartments in Na3Dviewer
    
    // Wire up MIP viewer
    // Status bar message
    connect(ui.naLargeMIPWidget, SIGNAL(statusMessage(const QString&)),
            statusBar(), SLOT(showMessage(const QString&)));
    connect(ui.naZStackWidget, SIGNAL(statusMessage(const QString&)),
            statusBar(), SLOT(showMessage(const QString&)));
    ui.progressWidgetMip->hide();
    connect(ui.naLargeMIPWidget, SIGNAL(showProgress()),
            ui.progressWidgetMip, SLOT(show()));
    connect(ui.naLargeMIPWidget, SIGNAL(hideProgress()),
            ui.progressWidgetMip, SLOT(hide()));
    connect(ui.naLargeMIPWidget, SIGNAL(setProgressMax(int)),
            ui.progressBarMip, SLOT(setMaximum(int)));
    connect(ui.naLargeMIPWidget, SIGNAL(setProgress(int)),
            ui.progressBarMip, SLOT(setValue(int)));
    ui.progressWidgetZ->hide();

    // ui.gammaWidget_Zstack->hide();
    // Distinguish the two gamma sliders
    ui.sharedGammaWidget->gamma_label->setText("N "); // "neurons"
    ui.sharedGammaWidget->setToolTip(tr("Brightness/gamma of data"));
    ui.referenceGammaWidget->gamma_label->setText("R "); // "reference"
    ui.referenceGammaWidget->setToolTip(tr("Brightness/gamma of reference channel"));

    ui.BoxSize_spinBox->setMinimum(MINSZBOX);
    
    // Wire up Z-stack / HDR viewer
    connect(ui.HDR_checkBox, SIGNAL(toggled(bool)),
            ui.naZStackWidget, SLOT(setHDRCheckState(bool)));
    connect(ui.naZStackWidget, SIGNAL(changedHDRCheckState(bool)),
            ui.HDR_checkBox, SLOT(setChecked(bool)));
    connect(ui.HDRRed_pushButton, SIGNAL(clicked()),
            ui.naZStackWidget, SLOT(setRedChannel()));
    connect(ui.HDRGreen_pushButton, SIGNAL(clicked()),
            ui.naZStackWidget, SLOT(setGreenChannel()));
    connect(ui.HDRBlue_pushButton, SIGNAL(clicked()),
            ui.naZStackWidget, SLOT(setBlueChannel()));
    connect(ui.HDRNc82_pushButton, SIGNAL(clicked()),
            ui.naZStackWidget, SLOT(setNc82Channel()));
    connect(ui.naZStackWidget, SIGNAL(curColorChannelChanged(NaZStackWidget::Color)),
            this, SLOT(onHdrChannelChanged(NaZStackWidget::Color)));
    ui.naZStackWidget->setHDRCheckState(false);

    connect(ui.ZSlice_horizontalScrollBar, SIGNAL(valueChanged(int)),
            ui.naZStackWidget, SLOT(setCurrentZSlice(int)));
    connect(ui.naZStackWidget, SIGNAL(curZsliceChanged(int)),
            ui.ZSlice_horizontalScrollBar, SLOT(setValue(int)));
    connect(ui.BoxSize_spinBox, SIGNAL(valueChanged(int)),
            ui.naZStackWidget, SLOT(updateROIsize(int)));
    connect(ui.naZStackWidget, SIGNAL(boxSizeChanged(int)),
            ui.BoxSize_spinBox, SLOT(setValue(int)));

    // 3D viewer
    connect(ui.rotationResetButton, SIGNAL(clicked()),
            ui.v3dr_glwidget, SLOT(resetRotation()));
    connect(ui.nutateButton, SIGNAL(toggled(bool)),
            this, SLOT(setNutate(bool)));
    connect(this, SIGNAL(nutatingChanged(bool)),
            ui.nutateButton, SLOT(setChecked(bool)));
    connect(ui.actionAnimate_3D_nutation, SIGNAL(toggled(bool)),
            this, SLOT(setNutate(bool)));
    connect(this, SIGNAL(nutatingChanged(bool)),
            ui.actionAnimate_3D_nutation, SLOT(setChecked(bool)));
    /* obsolete.  now we toggle channels.
    connect(ui.redToggleButton, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setChannelR(bool)));
    connect(ui.greenToggleButton, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setChannelG(bool)));
    connect(ui.blueToggleButton, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setChannelB(bool)));
            */
    // 3D rotation
    // synchronize compartment map
    connect(&sharedCameraModel, SIGNAL(rotationChanged(const Rotation3D&)),
            ui.compartmentMapWidget, SLOT(setRotation(const Rotation3D&)));
    // connect(&sharedCameraModel, SIGNAL(focusChanged(const Vector3D&)),
    //        ui.compartmentMapWidget, SLOT(setFocus(const Vector3D&)));

    connect(&(ui.v3dr_glwidget->cameraModel), SIGNAL(rotationChanged(const Rotation3D&)),
            this, SLOT(on3DViewerRotationChanged(const Rotation3D&)));
    connect(ui.rotXWidget, SIGNAL(angleChanged(int)),
            this, SLOT(update3DViewerXYZBodyRotation()));
    connect(ui.rotYWidget, SIGNAL(angleChanged(int)),
            this, SLOT(update3DViewerXYZBodyRotation()));
    connect(ui.rotZWidget, SIGNAL(angleChanged(int)),
            this, SLOT(update3DViewerXYZBodyRotation()));
    connect(ui.v3dr_glwidget, SIGNAL(progressValueChanged(int)),
            this, SLOT(set3DProgress(int)));
    connect(ui.v3dr_glwidget, SIGNAL(progressComplete()),
            this, SLOT(complete3DProgress()));
    connect(ui.v3dr_glwidget, SIGNAL(progressMessageChanged(QString)),
            this, SLOT(set3DProgressMessage(QString)));
    connect(ui.v3dr_glwidget, SIGNAL(progressAborted(QString)),
            this, SLOT(complete3DProgress()));

    // 3D volume cut
    connect(ui.v3dr_glwidget, SIGNAL(changeXCut0(int)), ui.XcminSlider, SLOT(setValue(int))); // x-cut
    connect(ui.XcminSlider, SIGNAL(valueChanged(int)), ui.v3dr_glwidget, SLOT(setXCut0(int)));
    connect(ui.v3dr_glwidget, SIGNAL(changeXCut1(int)), ui.XcmaxSlider, SLOT(setValue(int)));
    connect(ui.XcmaxSlider, SIGNAL(valueChanged(int)), ui.v3dr_glwidget, SLOT(setXCut1(int)));

    connect(ui.v3dr_glwidget, SIGNAL(changeYCut0(int)), ui.YcminSlider, SLOT(setValue(int))); // y-cut
    connect(ui.YcminSlider, SIGNAL(valueChanged(int)), ui.v3dr_glwidget, SLOT(setYCut0(int)));
    connect(ui.v3dr_glwidget, SIGNAL(changeYCut1(int)), ui.YcmaxSlider, SLOT(setValue(int)));
    connect(ui.YcmaxSlider, SIGNAL(valueChanged(int)), ui.v3dr_glwidget, SLOT(setYCut1(int)));

    connect(ui.v3dr_glwidget, SIGNAL(changeZCut0(int)), ui.ZcminSlider, SLOT(setValue(int))); // z-cut
    connect(ui.ZcminSlider, SIGNAL(valueChanged(int)), ui.v3dr_glwidget, SLOT(setZCut0(int)));
    connect(ui.v3dr_glwidget, SIGNAL(changeZCut1(int)), ui.ZcmaxSlider, SLOT(setValue(int)));
    connect(ui.ZcmaxSlider, SIGNAL(valueChanged(int)), ui.v3dr_glwidget, SLOT(setZCut1(int)));

    connect(ui.XCutCB, SIGNAL(stateChanged(int)), ui.v3dr_glwidget, SLOT(setXCutLock(int)));
    connect(ui.YCutCB, SIGNAL(stateChanged(int)), ui.v3dr_glwidget, SLOT(setYCutLock(int)));
    connect(ui.ZCutCB, SIGNAL(stateChanged(int)), ui.v3dr_glwidget, SLOT(setZCutLock(int)));

    connect(ui.slabThicknessSlider, SIGNAL(valueChanged(int)),
            ui.v3dr_glwidget, SLOT(setSlabThickness(int)));
    connect(ui.slabPositionSlider, SIGNAL(valueChanged(int)),
            ui.v3dr_glwidget, SLOT(setSlabPosition(int)));
    connect(ui.v3dr_glwidget, SIGNAL(slabThicknessChanged(int)),
            ui.slabThicknessSlider, SLOT(setValue(int)));
    connect(ui.v3dr_glwidget, SIGNAL(slabPositionChanged(int)),
            ui.slabPositionSlider, SLOT(setValue(int)));
    connect(ui.freezeFrontBackButton, SIGNAL(clicked()),
            ui.v3dr_glwidget, SLOT(clipSlab()));

    // alpha blending
    connect(ui.action3D_alpha_blending, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setAlphaBlending(bool)));
    connect(ui.v3dr_glwidget, SIGNAL(alphaBlendingChanged(bool)),
            ui.action3D_alpha_blending, SLOT(setChecked(bool)));

    // show axes
    connect(ui.actionShow_Axes, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setShowCornerAxes(bool)));

    // Whether to use common zoom and focus in MIP, ZStack and 3D viewers
    connect(ui.actionLink_viewers, SIGNAL(toggled(bool)),
            this, SLOT(unifyCameras(bool)));
    unifyCameras(true); // Start with cameras linked
    connect(ui.resetViewButton, SIGNAL(clicked()),
            this, SLOT(resetView()));
    connect(ui.zoomWidget, SIGNAL(zoomValueChanged(qreal)),
            &sharedCameraModel, SLOT(setScale(qreal)));
    connect(&sharedCameraModel, SIGNAL(scaleChanged(qreal)),
            ui.zoomWidget, SLOT(setZoomValue(qreal)));
    connect(&sharedCameraModel, SIGNAL(scaleChanged(qreal)),
            this, SLOT(updateViewers()));
    // Colors
    connect(ui.redToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(setChannelZeroVisibility(bool)));
    connect(ui.greenToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(setChannelOneVisibility(bool)));
    connect(ui.blueToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(setChannelTwoVisibility(bool)));

    // Crosshair
    connect(ui.actionShow_Crosshair, SIGNAL(toggled(bool)),
            this, SLOT(setCrosshairVisibility(bool)));
    connect(this, SIGNAL(crosshairVisibilityChanged(bool)),
            ui.actionShow_Crosshair, SLOT(setChecked(bool)));
    connect(this, SIGNAL(crosshairVisibilityChanged(bool)),
            ui.naLargeMIPWidget, SLOT(showCrosshair(bool)));
    connect(this, SIGNAL(crosshairVisibilityChanged(bool)),
            ui.v3dr_glwidget, SLOT(showCrosshair(bool)));
    connect(this, SIGNAL(crosshairVisibilityChanged(bool)),
            ui.naZStackWidget, SLOT(showCrosshair(bool)));
    retrieveCrosshairVisibilitySetting();

    // Axes
    // TODO I want a small set of axes that sits in the lower left corner.  The gigantic axes are less useful.
    // connect(ui.actionShow_Axes, SIGNAL(toggled(bool)),
    //         ui.v3dr_glwidget, SLOT(enableShowAxes(bool)));

    // Clear status message when viewer changes
    connect(ui.viewerStackedWidget, SIGNAL(currentChanged(int)),
            this, SLOT(onViewerChanged(int)));

    // Create "Undo" menu options
    // TODO - figure out which of these variables to expose once we have a QUndoCommand to work with.
    QUndoGroup * undoGroup = new QUndoGroup(this);
    QAction * undoAction = undoGroup->createUndoAction(this);
    QAction * redoAction = undoGroup->createRedoAction(this);
    ui.menuEdit->insertAction(ui.menuEdit->actions().at(0), redoAction);
    ui.menuEdit->insertAction(redoAction, undoAction);
    // expose undoStack
    undoStack = new QUndoStack(undoGroup);
    undoGroup->setActiveStack(undoStack);
    ui.v3dr_glwidget->setUndoStack(*undoStack);

    // Connect sort buttons to gallery widget
    connect(ui.gallerySortBySizeButton, SIGNAL(clicked()),
            ui.fragmentGalleryWidget, SLOT(sortBySize()));
    connect(ui.gallerySortByColorButton, SIGNAL(clicked()),
            ui.fragmentGalleryWidget, SLOT(sortByColor()));
    connect(ui.gallerySortByIndexButton, SIGNAL(clicked()),
            ui.fragmentGalleryWidget, SLOT(sortByIndex()));

    // Allow cross-thread signals/slots that pass QList<int>
    qRegisterMetaType< QList<int> >("QList<int>");

    // Set up the annotation widget
    ui.annotationFrame->setMainWindow(this);
    ui.centralwidget->installEventFilter(ui.annotationFrame);
    ui.annotationFrame->consoleConnect(3);

    // NeuronSelector helper class for selecting neurons
    connect(&neuronSelector, SIGNAL(neuronSelected(int)),
            ui.annotationFrame, SLOT(selectNeuron(int)));
    connect(ui.v3dr_glwidget, SIGNAL(neuronSelected(double,double,double)),
            &neuronSelector, SLOT(updateSelectedPosition(double,double,double)));
    connect(ui.actionDynamic_range, SIGNAL(triggered(bool)),
            this, SLOT(showDynamicRangeTool()));

    connect(ui.actionFull_Screen, SIGNAL(toggled(bool)),
            this, SLOT(setFullScreen(bool)));

    connect(this, SIGNAL(benchmarkTimerResetRequested()),
            this, SLOT(resetBenchmarkTimer()));
    connect(this, SIGNAL(benchmarkTimerPrintRequested(QString)),
            this, SLOT(printBenchmarkTimer(QString)));
    connect(ui.v3dr_glwidget, SIGNAL(benchmarkTimerPrintRequested(QString)),
            this, SIGNAL(benchmarkTimerPrintRequested(QString)));
    connect(ui.v3dr_glwidget, SIGNAL(benchmarkTimerResetRequested()),
            this, SIGNAL(benchmarkTimerResetRequested()));

    initializeContextMenus();
    initializeStereo3DOptions();
    connectCustomCut();
}

/* slot */
void NaMainWindow::resetBenchmarkTimer()
{
    mainWindowStopWatch.restart();
}

/* slot */
void NaMainWindow::printBenchmarkTimer(QString message)
{
    qDebug() << "BENCHMARK" << message << "at" << mainWindowStopWatch.elapsed()/1000.0 << "seconds";
}

/* virtual */
void NaMainWindow::keyPressEvent(QKeyEvent *event)
{
    // Press <ESC> to exit full screen mode
    if (event->key() == Qt::Key_Escape)
    {
        // qDebug() << "escape pressed in main window";
        exitFullScreen();
    }
    QMainWindow::keyPressEvent(event);
}

/* slot */
void NaMainWindow::setViewMode(ViewMode mode)
{
    // if (mode == viewMode)
    //     return; // no change
    viewMode = mode;
    if (mode == VIEW_SINGLE_STACK) {
        ui.mipsFrame->setVisible(false);
        ui.annotationFrame->setVisible(true);
        ui.referenceGammaWidget->setVisible(false);
        // qDebug() << "Changing to single stack mode" << __FILE__ << __LINE__;
    }
    if (mode == VIEW_NEURON_SEPARATION) {
        ui.mipsFrame->setVisible(true);
        ui.annotationFrame->setVisible(true);
        ui.referenceGammaWidget->setVisible(true);
        // qDebug() << "Changing to separation result mode" << __FILE__ << __LINE__;
    }
    update();
}

/* slot */
void NaMainWindow::exitFullScreen()
{
    if (! isFullScreen())
        return;
    if (viewMode == VIEW_NEURON_SEPARATION)
    {
        ui.mipsFrame->show();
    }
    ui.annotationFrame->show();
    ui.viewerSelectorAndControlFrame->show();
    statusBar()->show();
    showNormal();
}

/* slot */
void NaMainWindow::setFullScreen(bool b)
{
    if (isFullScreen() == b)
        return;
    if (b)
    {
        ui.annotationFrame->hide();
        ui.mipsFrame->hide();
        ui.viewerSelectorAndControlFrame->hide();
        statusBar()->hide();
        showFullScreen();
    }
    else
    {
        exitFullScreen();
    }
}

QString NaMainWindow::getDataDirectoryPathWithDialog()
{
    QString initialDialogPath = QDir::currentPath();
    // Use previous annotation path as initial file browser location
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    QString previousAnnotationDirString = settings.value("NeuronAnnotatorPreviousAnnotationPath").toString();
    if (! previousAnnotationDirString.isEmpty()) {
        QDir previousAnnotationDir(previousAnnotationDirString);
        if (previousAnnotationDir.exists() && previousAnnotationDir.isReadable())
        {
            initialDialogPath = previousAnnotationDir.path();
            // qDebug() << "Annotation directory path = " << initialDialogPath;
        }
    }

    QString dirName = QFileDialog::getExistingDirectory(
            this,
            "Select neuron separation directory",
            initialDialogPath,
            QFileDialog::ShowDirsOnly);
    if (dirName.isEmpty())
        return "";

    QDir dir(dirName);
    if (! dir.exists() )
    {
        QMessageBox::warning(this, tr("No such directory"),
                             QString("'%1'\n No such directory.\nIs the file share mounted?\nHas the directory moved?").arg(dirName));
        return "";
    }

    // Remember parent directory to ease browsing next time
    if (dir.cdUp()) {
        if (dir.exists()) {
            // qDebug() << "Saving annotation dir parent path " << parentDir.path();
            settings.setValue("NeuronAnnotatorPreviousAnnotationPath", dir.path());
        }
        else {
            qDebug() << "Problem saving parent directory of " << dirName;
        }
    }

    return dirName;
}

QString NaMainWindow::getStackPathWithDialog()
{
    QString initialDialogPath = QDir::currentPath();
    // Use previous annotation path as initial file browser location
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    QString previousAnnotationDirString = settings.value("NeuronAnnotatorPreviousAnnotationPath").toString();
    if (! previousAnnotationDirString.isEmpty()) {
        QDir previousAnnotationDir(previousAnnotationDirString);
        if (previousAnnotationDir.exists() && previousAnnotationDir.isReadable())
        {
            initialDialogPath = previousAnnotationDir.path();
            // qDebug() << "Annotation directory path = " << initialDialogPath;
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Select volume file",
                                                    initialDialogPath,
                                                    tr("Stacks (*.lsm *.tif *.tiff *.mp4 *.v3draw *.v3dpbd)"));

    if (fileName.isEmpty()) // Silently do nothing when user presses Cancel.  No error dialogs please!
        return "";

    QFile movieFile(fileName);

    if (! movieFile.exists() )
    {
        QMessageBox::warning(this, tr("No such file"),
                             QString("'%1'\n No such file.\nIs the file share mounted?\nHas the directory moved?").arg(fileName));
        return "";
    }

    // Remember parent directory to ease browsing next time
    QDir parentDir = QFileInfo(movieFile).dir();
    if (parentDir.exists()) {
        // qDebug() << "Saving annotation dir parent path " << parentDir.path();
        settings.setValue("NeuronAnnotatorPreviousAnnotationPath", parentDir.path());
    }
    else {
        qDebug() << "Problem saving parent directory of " << fileName;
    }

    return fileName;
}

/* slot */
void NaMainWindow::on_actionLoad_movie_as_texture_triggered()
{
#ifdef USE_FFMPEG
    QString fileName = getStackPathWithDialog();
    if (fileName.isEmpty()) return;
    if (! dataFlowModel) return;
    dataFlowModel->getFast3DTexture().loadFile(fileName);
#endif
}

/* slot */
void NaMainWindow::on_actionLoad_fast_separation_result_triggered()
{
#ifdef USE_FFMPEG
    // qDebug() << "NaMainWindow::on_actionLoad_fast_separation_result_triggered()" << __FILE__ << __LINE__;
    // Ask user for a directory containing results.
    QString dirName = getDataDirectoryPathWithDialog();
    if (dirName.isEmpty()) return;
    QDir dir(dirName);

    emit benchmarkTimerResetRequested();
    emit benchmarkTimerPrintRequested("Load fast directory triggered");

    createNewDataFlowModel();

    Fast3DTexture& mpegTexture = dataFlowModel->getFast3DTexture();
    {
        Fast3DTexture::Writer textureWriter(mpegTexture);

        // First series of lossy downsampled images in mpeg4 format for fast loading
        // TODO - only load the files that exist
        // First load lowest resolution mp4 to put something on the screen immediately ~300ms elapsed
        mpegTexture.queueVolume(dir.filePath("ConsolidatedSignal2_25.mp4"),
                                 BlockScaler::CHANNEL_RGB);
        // First refinement: load largest subsample that can fit on the video card. ~1500ms elapsed
        mpegTexture.queueVolume(dir.filePath("ConsolidatedSignal2_100.mp4"),
                                 BlockScaler::CHANNEL_RGB);
        // Next add the reference channel
        mpegTexture.queueVolume(dir.filePath("Reference2_100.mp4"),
                                 BlockScaler::CHANNEL_ALPHA);
        // Individual color channels to sharpen the colors
        mpegTexture.queueVolume(dir.filePath("ConsolidatedSignal2Red_100.mp4"),
                                 BlockScaler::CHANNEL_RED);
        mpegTexture.queueVolume(dir.filePath("ConsolidatedSignal2Green_100.mp4"),
                                 BlockScaler::CHANNEL_GREEN);
        mpegTexture.queueVolume(dir.filePath("ConsolidatedSignal2Blue_100.mp4"), // ~4000ms elapsed
                                 BlockScaler::CHANNEL_BLUE);

        mpegTexture.loadNextVolume(); // starts loading process in another thread
    }

    // dataFlowModel->getDataColorModel().initializeRgba32();
    // dataFlowModel->getSlow3DColorModel().initializeRgba32();
    emit initializeColorModelRequested();

    // Apply gamma bias already applied to input images
    // dataFlowModel->getSlow3DColorModel().setSharedGamma(0.46);
    // dataFlowModel->getSlow3DColorModel().setReferenceGamma(0.46);
    emit offset3dGammaChanged(0.46);
    emit benchmarkTimerPrintRequested("Initialized color models");

    // keep reference channel off
    // dataFlowModel->getNeuronSelectionModel().initializeSelectionModel();
    emit initializeSelectionModelRequested();
    setViewMode(VIEW_SINGLE_STACK);

    // TODO - load lossless image into VolumeTexture
    // connect(mpegTexture, SIGNAL(volumeLoadSequenceCompleted()),
    //         &dataFlowModel->getVolumeTexture(), SLOT(loadVolumeDataFromFiles()),
    //         Qt::UniqueConnection);

    // Prepare to load lossless files after mp4 files have loaded
    connect(&mpegTexture, SIGNAL(volumeLoadSequenceCompleted()),
            &dataFlowModel->getVolumeData(), SLOT(loadVolumeDataFromFiles()),
            Qt::UniqueConnection);
    {
        NaVolumeData::Writer volumeWriter(dataFlowModel->getVolumeData());
        volumeWriter.setOriginalImageStackFilePath(dir.filePath("ConsolidatedSignal3.v3dpbd"));
        volumeWriter.setReferenceStackFilePath(dir.filePath("Reference3.v3dpbd"));
        volumeWriter.setMaskLabelFilePath(dir.filePath("ConsolidatedLabel3.v3dpbd"));
    }
    // TODO - emit, don't risk copying by direct dataFlowModel->getFoo()
    dataFlowModel->getVolumeData().doFlipY = false;
    dataFlowModel->getVolumeData().bDoUpdateSignalTexture = false;

#endif
}

/* slot */
void NaMainWindow::on_actionOpen_Single_Movie_Stack_triggered()
{
    qDebug() << "NaMainWindow::on_actionOpen_Single_Movie_Stack_triggered";
    QString initialDialogPath = QDir::currentPath();
    // Use previous annotation path as initial file browser location
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    QString previousAnnotationDirString = settings.value("NeuronAnnotatorPreviousAnnotationPath").toString();
    if (! previousAnnotationDirString.isEmpty()) {
        QDir previousAnnotationDir(previousAnnotationDirString);
        if (previousAnnotationDir.exists() && previousAnnotationDir.isReadable())
        {
            initialDialogPath = previousAnnotationDir.path();
            // qDebug() << "Annotation directory path = " << initialDialogPath;
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Select MPEG4 format volume data",
                                                    initialDialogPath);

    // qDebug() << dirName;

    // If user presses cancel, QFileDialog::getOpenFileName returns a null string
    if (fileName.isEmpty()) // Silently do nothing when user presses Cancel.  No error dialogs please!
        return;

    QFile movieFile(fileName);

    if (! movieFile.exists() )
    {
        QMessageBox::warning(this, tr("No such file"),
                             QString("'%1'\n No such file.\nIs the file share mounted?\nHas the directory moved?").arg(fileName));
        return;
    }

    // Remember parent directory to ease browsing next time
    QDir parentDir = QFileInfo(movieFile).dir();
    if (parentDir.exists()) {
        // qDebug() << "Saving annotation dir parent path " << parentDir.path();
        settings.setValue("NeuronAnnotatorPreviousAnnotationPath", parentDir.path());
    }
    else {
        qDebug() << "Problem saving parent directory of " << fileName;
    }

    loadSingleStack(fileName, false);
}

/* slot */
void NaMainWindow::setCrosshairVisibility(bool b)
{
    if (bShowCrosshair == b) return; // no change
    bShowCrosshair = b;
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    settings.setValue("NaCrosshairVisibility", bShowCrosshair);
    emit crosshairVisibilityChanged(bShowCrosshair);
}

void NaMainWindow::retrieveCrosshairVisibilitySetting()
{
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    bool bVisible = true; // default to "on"
    QVariant val = settings.value("NaCrosshairVisibility");
    if (! val.isNull())
        bVisible = val.toBool();
    setCrosshairVisibility(bVisible);
}

/////////////////////////////////////////////////////
// Drop volume files onto main window to view them //
/////////////////////////////////////////////////////

// Return file name if the dragged item can be usefully dropped into the NeuronAnnotator main window
QString checkDragEvent(QDropEvent* event)
{
    if (! event->mimeData()->hasFormat("text/uri-list"))
        return ""; // only files are accepted

    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return "";

    QString fileName = urls.first().toLocalFile();
    if (fileName.isEmpty())
        return "";

    // check for recognized file extensions
    QString fileExtension = QFileInfo(fileName).suffix().toLower();
    if (fileExtension == "lsm")
        return fileName;
    if (fileExtension.startsWith("tif")) // tif or tiff
        return fileName;
    if (fileExtension.startsWith("v3d")) // v3draw or v3dpdb
        return fileName;
#ifdef USE_FFMPEG
    if (fileExtension.startsWith("mp4")) // v3draw or v3dpdb
        return fileName;
#endif

    return "";
}

void NaMainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    QString fileName = checkDragEvent(event);
    if (! fileName.isEmpty())
        event->acceptProposedAction();
    qDebug() << "NaMainWindow::dragEnterEvent" << fileName << __FILE__ << __LINE__;
}

void NaMainWindow::dropEvent(QDropEvent * event)
{
    QString fileName = checkDragEvent(event);
    if (fileName.isEmpty()) return;

    // If this looks like a neuron separation directory, load it in NeuronAnnotator
    QDir directory = QFileInfo(fileName).dir();
    if (   QFile(directory.filePath("ConsolidatedLabel.v3dpbd")).exists()
        || QFile(directory.filePath("ConsolidatedLabel.tif")).exists() )
    {
        qDebug() << "Found separated neurons directory";
        openMulticolorImageStack(directory.absolutePath());
    }
    else
    {
        qDebug() << "Switching to Vaa3D default mode to view single image stack";
        loadSingleStack(fileName, false);
        // ui.actionV3DDefault->trigger(); // switch mode
        // emit defaultVaa3dFileLoadRequested(fileName);
    }

    // qDebug() << "NaMainWindow::dropEvent" << fileName << __FILE__ << __LINE__;
}

void NaMainWindow::moveEvent ( QMoveEvent * event )
{
    // qDebug() << "NaMainWindow::moveEvent()" << __FILE__ << __LINE__;
    ui.v3dr_glwidget->updateScreenPosition();
    QMainWindow::moveEvent(event);
}

/* slot */
void NaMainWindow::loadSingleStack(QString fileName)
{
    loadSingleStack(fileName, true); // default to classic mode
}

/* slot */
void NaMainWindow::loadSingleStack(QString fileName, bool useVaa3dClassic)
{
    mainWindowStopWatch.start();
    if (useVaa3dClassic) {
        // Open in Vaa3D classic mode
        ui.actionV3DDefault->trigger(); // switch mode
        emit defaultVaa3dFileLoadRequested(fileName);
    }
    else
    {
        setViewMode(VIEW_SINGLE_STACK);
        onDataLoadStarted();
        createNewDataFlowModel();
        emit singleStackLoadRequested(fileName);
    }
}


///////////////////////////////////
// User clip planes in 3D viewer //
///////////////////////////////////

void NaMainWindow::connectCustomCut()
{
    connect(ui.customCutButton, SIGNAL(pressed()),
            this, SLOT(applyCustomCut()));
    connect(ui.defineClipPlaneButton, SIGNAL(pressed()),
            this, SLOT(toggleCustomCutMode()));
}

/* slot */
void NaMainWindow::applyCustomCut()
{
    assert(isInCustomCutMode);
    ui.v3dr_glwidget->applyCustomCut();
    if (isInCustomCutMode)
        toggleCustomCutMode();
}

/* slot */
void NaMainWindow::toggleCustomCutMode()
{
    if (isInCustomCutMode)
    {
        // Turn off custom cut mode
        ui.defineClipPlaneButton->setText(tr("Custom..."));
        ui.customCutButton->setEnabled(false);
        ui.v3dr_glwidget->cancelCustomCutMode();
    }
    else
    {
        // Activate custom cut mode
        ui.defineClipPlaneButton->setText(tr("Cancel"));
        ui.customCutButton->setEnabled(true);
        ui.v3dr_glwidget->setCustomCutMode();
    }
    // Switch mode flag
    isInCustomCutMode = ! isInCustomCutMode;
}

/* slot */
void NaMainWindow::showDynamicRangeTool()
{
    // qDebug() << "NaMainWindow::showDynamicRangeTool";
    if (! dynamicRangeTool) {
        dynamicRangeTool = new DynamicRangeTool(this);
        if (dataFlowModel)
            dynamicRangeTool->setColorModel(&dataFlowModel->getDataColorModel());
        else
            dynamicRangeTool->setColorModel(NULL);
    }
    dynamicRangeTool->show();
}

void NaMainWindow::onDataLoadStarted()
{
    // Give strong indication to user that load is in progress
    ui.viewerControlTabWidget->setEnabled(false);
    ViewerIndex currentIndex = (ViewerIndex)ui.viewerStackedWidget->currentIndex();
    if (currentIndex != VIEWER_WAIT_LOADING_SCREEN)
        recentViewer = currentIndex;
    ui.viewerStackedWidget->setCurrentIndex(VIEWER_WAIT_LOADING_SCREEN);
    update();
}

void NaMainWindow::onDataLoadFinished()
{
    if (undoStack)
        undoStack->clear();
    ui.viewerStackedWidget->setCurrentIndex(recentViewer);
    ui.viewerControlTabWidget->setEnabled(true);
    qDebug() << "Data load took" << mainWindowStopWatch.elapsed()/1000.0 << "seconds";
    update();
}

void NaMainWindow::initializeStereo3DOptions()
{
    // Only check one stereo format at a time
    QActionGroup* stereoModeGroup = new QActionGroup(this);
    stereoModeGroup->setExclusive(true);
    stereoModeGroup->addAction(ui.actionMono_Off);
    stereoModeGroup->addAction(ui.actionLeft_eye_view);
    stereoModeGroup->addAction(ui.actionRight_eye_view);
    stereoModeGroup->addAction(ui.actionQuadro_120_Hz);
    stereoModeGroup->addAction(ui.actionAnaglyph_Red_Cyan);
    stereoModeGroup->addAction(ui.actionAnaglyph_Green_Magenta);
    stereoModeGroup->addAction(ui.actionRow_Interleaved_Zalman);
    stereoModeGroup->addAction(ui.actionChecker_Interleaved_3DTV);
    stereoModeGroup->addAction(ui.actionColumn_Interleaved);

    connect(ui.actionMono_Off, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setStereoOff(bool)));
    connect(ui.actionLeft_eye_view, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setStereoLeftEye(bool)));
    connect(ui.actionRight_eye_view, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setStereoRightEye(bool)));
    connect(ui.actionQuadro_120_Hz, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setStereoQuadBuffered(bool)));
    connect(ui.actionAnaglyph_Red_Cyan, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setStereoAnaglyphRedCyan(bool)));
    connect(ui.actionAnaglyph_Green_Magenta, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setStereoAnaglyphGreenMagenta(bool)));
    connect(ui.actionRow_Interleaved_Zalman, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setStereoRowInterleaved(bool)));
    connect(ui.actionChecker_Interleaved_3DTV, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setStereoCheckerInterleaved(bool)));
    connect(ui.actionColumn_Interleaved, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setStereoColumnInterleaved(bool)));

    connect(ui.v3dr_glwidget, SIGNAL(quadStereoSupported(bool)),
            this, SLOT(supportQuadStereo(bool)));
}

/* slot */
void NaMainWindow::supportQuadStereo(bool b)
{
    ui.actionQuadro_120_Hz->setEnabled(b);
    if ( (!b) && ui.actionQuadro_120_Hz->isChecked() )
        ui.actionMono_Off->setChecked(true);
}

void NaMainWindow::connectContextMenus(const NeuronSelectionModel& neuronSelectionModel)
{
    connect(showAllNeuronsInEmptySpaceAction, SIGNAL(triggered()),
            &neuronSelectionModel, SLOT(showAllNeuronsInEmptySpace()));
    connect(selectNoneAction, SIGNAL(triggered()),
            &neuronSelectionModel, SLOT(clearSelection()));
    connect(hideAllAction, SIGNAL(triggered()),
            &neuronSelectionModel, SLOT(showNothing()));
    neuronContextMenu->connectActions(neuronSelectionModel);
}

void NaMainWindow::initializeContextMenus()
{
    viewerContextMenu = new QMenu(this);
    neuronContextMenu = new NeuronContextMenu(this);
    // Some QActions were already made in Qt Designer
    showAllNeuronsInEmptySpaceAction = ui.actionShow_all_neurons_in_empty_space;
    hideAllAction = ui.actionClear_Hide_All;
    selectNoneAction = ui.actionSelect_None;
    //
    viewerContextMenu->addAction(showAllNeuronsInEmptySpaceAction);
    viewerContextMenu->addAction(hideAllAction);
    viewerContextMenu->addAction(selectNoneAction);
    //
    neuronContextMenu->addSeparator();
    neuronContextMenu->addAction(showAllNeuronsInEmptySpaceAction);
    neuronContextMenu->addAction(hideAllAction);
    neuronContextMenu->addAction(selectNoneAction);
    //
    ui.naLargeMIPWidget->setContextMenus(viewerContextMenu, neuronContextMenu);
    ui.naZStackWidget->setContextMenus(viewerContextMenu, neuronContextMenu);
    ui.v3dr_glwidget->setContextMenus(viewerContextMenu, neuronContextMenu);
}

/* slot */
void NaMainWindow::onHdrChannelChanged(NaZStackWidget::Color channel)
{
    switch(channel)
    {
    // Due to exclusive group, checking one button unchecks the others.
    case NaZStackWidget::COLOR_RED:
        ui.HDRRed_pushButton->setChecked(true);
        break;
    case NaZStackWidget::COLOR_GREEN:
        ui.HDRGreen_pushButton->setChecked(true);
        break;
    case NaZStackWidget::COLOR_BLUE:
        ui.HDRBlue_pushButton->setChecked(true);
        break;
    case NaZStackWidget::COLOR_NC82:
        ui.HDRNc82_pushButton->setChecked(true);
        break;
    }
}

/* slot */
void NaMainWindow::onColorModelChanged()
{
    // For historical reasons, reference channel is denormalized into both NeuronSelectionModel and DataColorModel
    bool bReferenceColorIsVisible;
    bool bReferenceOverlayIsVisible;
    {
        DataColorModel::Reader colorReader(dataFlowModel->getDataColorModel());
        if (dataFlowModel->getDataColorModel().readerIsStale(colorReader)) return;
        ui.redToggleButton->setChecked(colorReader.getChannelVisibility(0));
        ui.greenToggleButton->setChecked(colorReader.getChannelVisibility(1));
        ui.blueToggleButton->setChecked(colorReader.getChannelVisibility(2));

        // Gamma
        ui.sharedGammaWidget->setGammaBrightness(colorReader.getSharedGamma());
        const int refIndex = 3;
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (volumeReader.hasReadLock() && volumeReader.hasReferenceImage())
            ui.referenceGammaWidget->setGammaBrightness(colorReader.getChannelGamma(refIndex));

        // Communicate reference channel changes between NeuronSelectionModel and DataColorModel
        bReferenceColorIsVisible = colorReader.getChannelVisibility(refIndex);
        NeuronSelectionModel::Reader selectionReader(dataFlowModel->getNeuronSelectionModel());
        if (! selectionReader.hasReadLock()) return;
        if (selectionReader.getMaskStatusList().size() < 1)
            return; // selection model is not active, single stack being viewed?
        bReferenceOverlayIsVisible = selectionReader.getOverlayStatusList()[DataFlowModel::REFERENCE_MIP_INDEX];
    } // release read locks
    // Communicate reference visibility change, if any, to NeuronSelectionModel
    if (bReferenceColorIsVisible != bReferenceOverlayIsVisible)
        dataFlowModel->getNeuronSelectionModel().updateOverlay(DataFlowModel::REFERENCE_MIP_INDEX, bReferenceColorIsVisible);
}

/* slot */
void NaMainWindow::onSelectionModelVisibilityChanged()
{
    // For historical reasons, reference channel is denormalized into both NeuronSelectionModel and DataColorModel
    bool bReferenceColorIsVisible;
    bool bReferenceOverlayIsVisible;
    {
        DataColorModel::Reader colorReader(dataFlowModel->getDataColorModel());
        if (dataFlowModel->getDataColorModel().readerIsStale(colorReader)) return;
        bReferenceColorIsVisible = colorReader.getChannelVisibility(3);
        NeuronSelectionModel::Reader selectionReader(dataFlowModel->getNeuronSelectionModel());
        if (! selectionReader.hasReadLock()) return;
        bReferenceOverlayIsVisible = selectionReader.getOverlayStatusList()[DataFlowModel::REFERENCE_MIP_INDEX];
    }
    if (bReferenceColorIsVisible != bReferenceOverlayIsVisible)
        // TODO - this causes a fork of data color model
        // dataFlowModel->getDataColorModel().setChannelVisibility(3, bReferenceOverlayIsVisible);
        emit channelVisibilityChanged(3, bReferenceOverlayIsVisible);
}

/* slot */
void NaMainWindow::setChannelZeroVisibility(bool v)
{
    // qDebug() << "NaMainWindow::setChannelZeroVisibility" << v;
    emit channelVisibilityChanged(0, v);
}

/* slot */
void NaMainWindow::setChannelOneVisibility(bool v)
{
    emit channelVisibilityChanged(1, v);
}

/* slot */
void NaMainWindow::setChannelTwoVisibility(bool v)
{
    emit channelVisibilityChanged(2, v);
}

/* slot */
void NaMainWindow::setChannelThreeVisibility(bool v) // reference channel
{
    // For reference channel, update both DataColorModel AND "overlay" of NeuronSelectionModel
    if (dataFlowModel)
        dataFlowModel->getNeuronSelectionModel().updateOverlay(DataFlowModel::REFERENCE_MIP_INDEX, v);
    emit channelVisibilityChanged(3, v);
}

void NaMainWindow::onViewerChanged(int viewerIndex)
{
    QString msg(" viewer selected.");
    switch(viewerIndex) {
    case VIEWER_MIP:
        msg = "Maximum intensity projection" + msg;
        break;
    case VIEWER_ZSTACK:
        msg = "Z-stack" + msg;
        break;
    case VIEWER_3D:
        msg = "3D" + msg;
        break;
    default:
        return; // wait window gets no message
        break;
    }
    ui.statusbar->showMessage(msg);
}

void NaMainWindow::setNutate(bool bDoNutate)
{
    if (bDoNutate) {
        // qDebug() << "nutate";
        if (! nutateThread) {
            nutateThread = new NutateThread(0.2, this);
            qRegisterMetaType<Rotation3D>("Rotation3D");
            connect(nutateThread, SIGNAL(nutate(const Rotation3D&)),
                    this, SLOT(nutate(const Rotation3D&)));
        }

        if (! nutateThread->isRunning())
            nutateThread->start(QThread::IdlePriority);
        if (nutateThread->isRunning() && nutateThread->isPaused()) {
            nutateThread->unpause();
            emit nutatingChanged(bDoNutate);
        }
    }
    else {
        // qDebug() << "stop nutating";
        if (!nutateThread) return;
        if (nutateThread->isRunning() && (!nutateThread->isPaused())) {
            nutateThread->pause();
            emit nutatingChanged(bDoNutate);
        }
    }
}

void NaMainWindow::nutate(const Rotation3D& R) {
    // qDebug() << "nutate!";
    // std::cout << R << std::endl;
    CameraModel& cam = ui.v3dr_glwidget->cameraModel;
    if (!ui.v3dr_glwidget->mouseIsDragging()) {
        cam.setRotation(R * cam.rotation());
        // TODO - use a signal here instead of processEvents
        QCoreApplication::processEvents(); // keep responsive during nutation
        ui.v3dr_glwidget->update();
    }
}

void NaMainWindow::resetView()
{
    // TODO - might not work if cameras are not linked
    Vector3D newFocus = ui.v3dr_glwidget->getDefaultFocus();
    // cerr << newFocus << __LINE__ << __FILE__;
    sharedCameraModel.setFocus(newFocus);
    sharedCameraModel.setRotation(Rotation3D()); // identity rotation
    sharedCameraModel.setScale(1.0); // fit to window
    ui.viewerStackedWidget->update(); // whichever viewer is shown
}

void NaMainWindow::updateViewers()
{
    ui.naLargeMIPWidget->update();
    ui.naZStackWidget->update();
    ui.v3dr_glwidget->update();
}

void NaMainWindow::unifyCameras(bool bDoUnify)
{
    // TODO - explicitly copy parameters from active displayed viewer
    if (bDoUnify) {
        ui.naLargeMIPWidget->synchronizeWithCameraModel(&sharedCameraModel);
        ui.naZStackWidget->synchronizeWithCameraModel(&sharedCameraModel);
        ui.v3dr_glwidget->synchronizeWithCameraModel(&sharedCameraModel);
        // qDebug() << "unify cameras";
    }
    else {
        ui.naLargeMIPWidget->decoupleCameraModel(&sharedCameraModel);
        ui.naZStackWidget->decoupleCameraModel(&sharedCameraModel);
        ui.v3dr_glwidget->decoupleCameraModel(&sharedCameraModel);
        // qDebug() << "disband cameras";
    }
}

void NaMainWindow::setZRange(int minZ, int maxZ) {
    // qDebug() << "minZ = " << minZ << "; maxZ = " << maxZ;
    QString text = QString("of %1").arg(maxZ);
    // qDebug() << text;
    ui.ZSliceTotal_label->setText(text);
    ui.ZSlice_horizontalScrollBar->setMaximum(maxZ);
    ui.ZSlice_spinBox->setMaximum(maxZ);
    ui.ZSlice_horizontalScrollBar->setMinimum(minZ);
    ui.ZSlice_spinBox->setMinimum(minZ);
}

void NaMainWindow::handleCoordinatedCloseEvent(QCloseEvent *e)
{
    if (isVisible())
    {
        // Remember window size for next time.
        // These settings affect both NaMainWindow and classic V3D MainWindows.  So only use
        // NaMainWindow settings if the NaMainWindow is visible.
        qDebug() << "Saving NaMainWindow size and position";
        QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
        settings.setValue("pos", pos());
        settings.setValue("size", size());
    }
    e->accept();
}

void NaMainWindow::closeEvent(QCloseEvent *e)
{
    V3dApplication::handleCloseEvent(e);
}

void NaMainWindow::on_actionQuit_triggered() {
    close();
}

void NaMainWindow::on_actionV3DDefault_triggered() {
    V3dApplication::deactivateNaMainWindow();
    V3dApplication::activateMainWindow();
}

void NaMainWindow::on_actionNeuronAnnotator_triggered() {
    V3dApplication::activateNaMainWindow();
    V3dApplication::deactivateMainWindow();
}

void NaMainWindow::setV3DDefaultModeCheck(bool checkState) {
    QAction* ui_actionV3DDefault = qFindChild<QAction*>(this, "actionV3DDefault");
    ui_actionV3DDefault->setChecked(checkState);
}

void NaMainWindow::setNeuronAnnotatorModeCheck(bool checkState) {
    QAction* ui_actionNeuronAnnotator = qFindChild<QAction*>(this, "actionNeuronAnnotator");
    ui_actionNeuronAnnotator->setChecked(checkState);
}

void NaMainWindow::on_actionOpen_triggered()
{
    QString initialDialogPath = QDir::currentPath();
    // Use previous annotation path as initial file browser location
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    QString previousAnnotationDirString = settings.value("NeuronAnnotatorPreviousAnnotationPath").toString();
    if (! previousAnnotationDirString.isEmpty()) {
        QDir previousAnnotationDir(previousAnnotationDirString);
        if (previousAnnotationDir.exists() && previousAnnotationDir.isReadable())
        {
            initialDialogPath = previousAnnotationDir.path();
            // qDebug() << "Annotation directory path = " << initialDialogPath;
        }
    }

    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        "Select Color Separation Image Directory",
                                                        initialDialogPath,
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

    // qDebug() << dirName;

    // If user presses cancel, QFileDialog::getExistingDirectory returns a null string
    if (dirName.isEmpty()) // Silently do nothing when user presses Cancel.  No error dialogs please!
        return;

    QDir imageDir(dirName);

    if ( imageDir.exists() )
    {
        // Remember parent directory to ease browsing next time
        QDir parentDir(dirName);
        bool bParentOk = parentDir.cdUp();
        if (bParentOk) {
            // qDebug() << "Saving annotation dir parent path " << parentDir.path();
            settings.setValue("NeuronAnnotatorPreviousAnnotationPath", parentDir.path());
        }
        else {
            // qDebug() << "Problem saving parent directory of " << dirName;
        }

        openMulticolorImageStack(dirName);
    }
    else
    {
        QMessageBox::warning(this, tr("No such directory"),
                             QString("'%1'\n No such directory.\nIs the file share mounted?\nHas the directory moved?").arg(dirName));
    }
}


void NaMainWindow::openMulticolorImageStack(QString dirName)
{
    mainWindowStopWatch.start();
    QDir imageDir(dirName);

    if ( ! imageDir.exists() )
    {
        QMessageBox::warning(this, tr("No such directory"),
                             QString("'%1'\n No such directory.\nIs the file share mounted?\nHas the directory moved?").arg(dirName));
        return;
    }

    // std::cout << "Selected directory=" << imageDir.absolutePath().toStdString() << endl;

    if (! tearDownOldDataFlowModel()) {
        QMessageBox::warning(this, tr("Could not close previous Annotation Session"),
                     "Error saving previous session and/or clearing memory - please exit application");
                 return;
    }

    onDataLoadStarted();
    if (!loadAnnotationSessionFromDirectory(imageDir)) {
        QMessageBox::warning(this, tr("Could not load image directory"),
                                      "Error loading image directory - please check directory contents");

        onDataLoadFinished();
        return;
    }

    // qDebug() << "NaMainWindow::openMulticolorImageStack() calling addDirToRecentFilesList with dir=" << imageDir.absolutePath();
    addDirToRecentFilesList(imageDir);
}


// Recent files list
void NaMainWindow::addDirToRecentFilesList(QDir imageDir)
{
    QString fileName = imageDir.absolutePath();
    addFileNameToRecentFilesList(fileName);
}

void NaMainWindow::addFileNameToRecentFilesList(QString fileName)
{
    if (fileName.isEmpty()) return;
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    QStringList files = settings.value("NeuronAnnotatorRecentFileList").toStringList();
    if ( (files.size() > 0) && (files[0] == fileName) )
        return; // this dir is already the top entry as is
    files.removeAll(fileName);
    files.removeAll(QString());
    files.prepend(fileName);
    while (files.size() > maxRecentFiles)
            files.removeLast();
    settings.setValue("NeuronAnnotatorRecentFileList", files);
    updateRecentFileActions();
}


void NaMainWindow::updateRecentFileActions()
{
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    QStringList files = settings.value("NeuronAnnotatorRecentFileList").toStringList();
    ui.menuOpen_Recent->setEnabled(files.size() > 0);
    for (int i = 0; i < maxRecentFiles; ++i)
    {
        if ( (i < files.size() && (! files[i].isEmpty())) ) { // active
            recentFileActions[i]->setFileName(files[i]);
            recentFileActions[i]->setVisible(true);
        }
        else { // inactive
            recentFileActions[i]->setFileName(QString());
            recentFileActions[i]->setVisible(false);
        }
    }
}

QString NaMainWindow::suggestedExportFilenameFromCurrentState(const NeuronSelectionModel::Reader& selectionReader)
{
    MultiColorImageStackNode* multiColorImageStackNode=this->dataFlowModel->getMultiColorImageStackNode();
    QStringList lsmFilePathsList=multiColorImageStackNode->getLsmFilePathList();
    if (lsmFilePathsList.size()>0) {
        // First get filename prefix
        QString firstFilePath=lsmFilePathsList.at(0);
        QStringList components=firstFilePath.split(QRegExp("/"));
        QString name=components.at(components.size()-1);
        QStringList extComponents=name.split(QRegExp("\\."));
        QString filenamePrefix=extComponents.at(0);
        // Next, add current state
        if(selectionReader.getOverlayStatusList().at(DataFlowModel::REFERENCE_MIP_INDEX)) {
            filenamePrefix.append("_R");
        }
        if (selectionReader.getOverlayStatusList().at(DataFlowModel::BACKGROUND_MIP_INDEX)) {
            filenamePrefix.append("_B");
        }
        const QList<bool> neuronStatusList = selectionReader.getMaskStatusList();
        int activeCount=0;
        QString neuronStatusString;
        for (int i=0;i<neuronStatusList.size();i++) {
            if (neuronStatusList.at(i)) {
                neuronStatusString.append("_");
                QString number=QString("%1").arg(i);
                neuronStatusString.append(number);
                activeCount++;
            }
        }
        if (activeCount==neuronStatusList.size()) {
            filenamePrefix.append("_all");
        } else if (activeCount<6) {
            filenamePrefix.append(neuronStatusString);
        } else {
            filenamePrefix.append("_multiple");
        }
        return filenamePrefix;
    } else {
        return QString("");
    }
}

void expressRegretsAboutVolumeWriting() {
    // TODO
}

void NaMainWindow::on_action3D_Volume_triggered()
{
    if (! dataFlowModel) {
        expressRegretsAboutVolumeWriting();
        return;
    }
    NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
    if (! volumeReader.hasReadLock()) {
        expressRegretsAboutVolumeWriting();
        return;
    }
    NeuronSelectionModel::Reader selectionReader(dataFlowModel->getNeuronSelectionModel());
    if (! selectionReader.hasReadLock()) {
        expressRegretsAboutVolumeWriting();
        return;
    }

    QString suggestedFile=suggestedExportFilenameFromCurrentState(selectionReader);
    QString filename = QFileDialog::getSaveFileName(0, QObject::tr("Save 3D Volume to an .tif file"), suggestedFile, QObject::tr("3D Volume (*.tif)"));
    if (!(filename.isEmpty())){
        ExportFile *pExport = new ExportFile;
        // TODO - read lock is not held for long enough here
        if(pExport->init(volumeReader.getOriginalImageProxy().img0,
                         volumeReader.getNeuronMaskProxy().img0,
                         volumeReader.getReferenceImageProxy().img0,
                         selectionReader.getMaskStatusList(),
                         selectionReader.getOverlayStatusList(),
                         filename))
        {
            connect(pExport, SIGNAL(finished()), pExport, SLOT(deleteLater()));
            pExport->start();
        }
    }
} // release locks

void NaMainWindow::on_action2D_MIP_triggered() {
    QString filename = QFileDialog::getSaveFileName(0, QObject::tr("Save 2D MIP to an .tif file"), ".", QObject::tr("2D MIP (*.tif)"));
    if (!(filename.isEmpty())){
        bool saved = ui.naLargeMIPWidget->saveImage(filename);
    }
}

void NaMainWindow::on_actionScreenShot_triggered() {
    QString filename = QFileDialog::getSaveFileName(0, QObject::tr("Save 3D View to an .tif file"), ".", QObject::tr("screenshot (*.tif)"));
    if (!(filename.isEmpty())){
        bool saved = ui.v3dr_glwidget->screenShot(filename);
    }

}

// June 27, 2012 modify to accept "NULL" during data flow replacement
void NaMainWindow::setDataFlowModel(DataFlowModel* dataFlowModelParam)
{
    if (dataFlowModel == dataFlowModelParam)
        return; // no change

    if ( (dataFlowModelParam != NULL) // we are not currently tearing down
      && (dataFlowModel != NULL) ) // there is another different model in existence
    {
        qDebug() << "WARNING: setDataFlowModel() should not be tearing down old models" << __FILE__ << __LINE__;
        tearDownOldDataFlowModel();
    }

    dataFlowModel = dataFlowModelParam;
    ui.v3dr_glwidget->setDataFlowModel(dataFlowModel);
    ui.naLargeMIPWidget->setDataFlowModel(dataFlowModel);
    ui.naZStackWidget->setDataFlowModel(dataFlowModel);
    ui.fragmentGalleryWidget->setDataFlowModel(dataFlowModel);
    neuronSelector.setDataFlowModel(dataFlowModel);

    if (dynamicRangeTool) {
        if (NULL == dataFlowModel)
            dynamicRangeTool->setColorModel(NULL);
        else
            dynamicRangeTool->setColorModel(&dataFlowModel->getDataColorModel());
    }

    // No connecting if the model is NULL
    if (NULL == dataFlowModel)
    {
        ui.naLargeMIPWidget->setMipMergedData(NULL);
        return;
    }

    connect(dataFlowModel, SIGNAL(benchmarkTimerPrintRequested(QString)),
            this, SIGNAL(benchmarkTimerPrintRequested(QString)));
    connect(dataFlowModel, SIGNAL(benchmarkTimerResetRequested()),
            this, SIGNAL(benchmarkTimerResetRequested()));

    // was in loadAnnotationSessionFromDirectory June 27, 2012
    if (dynamicRangeTool)
        dynamicRangeTool->setColorModel(&dataFlowModel->getDataColorModel());

    // Connect mip viewer to data flow model
    ui.naLargeMIPWidget->setMipMergedData(&dataFlowModel->getMipMergedData());

    connectContextMenus(dataFlowModel->getNeuronSelectionModel());

    connect(dataFlowModel, SIGNAL(scrollBarFocus(NeuronSelectionModel::NeuronIndex)),
            ui.fragmentGalleryWidget, SLOT(scrollToFragment(NeuronSelectionModel::NeuronIndex)));

    connect(&dataFlowModel->getVolumeData(), SIGNAL(channelsLoaded(int)),
            this, SLOT(processUpdatedVolumeData()));

    // Both mip images and selection model need to be in place to update gallery
    connect(&dataFlowModel->getGalleryMipImages(), SIGNAL(dataChanged()),
            this, SLOT(updateGalleries()));
    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(initialized()),
            this, SLOT(initializeGalleries()));

    // Z value comes from camera model
    connect(&sharedCameraModel, SIGNAL(focusChanged(Vector3D)),
            &dataFlowModel->getZSliceColors(), SLOT(onCameraFocusChanged(Vector3D)));

    connect(ui.naZStackWidget, SIGNAL(hdrRangeChanged(int,qreal,qreal)),
            &dataFlowModel->getDataColorModel(), SLOT(setChannelHdrRange(int,qreal,qreal)));

    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(selectionCleared()),
            ui.annotationFrame, SLOT(deselectNeurons()));
    connect(ui.annotationFrame, SIGNAL(neuronSelected(int)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(selectExactlyOneNeuron(int)));
    connect(ui.annotationFrame, SIGNAL(neuronsDeselected()),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(clearSelection()));
    connect(this, SIGNAL(initializeSelectionModelRequested()),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(initializeSelectionModel()));

    // Progress if NaVolumeData file load
    // TODO - this is a lot of connection boilerplate code.  This should be abstracted into a single call or specialized ProgressManager class.
    connect(&dataFlowModel->getVolumeData(), SIGNAL(progressMessageChanged(QString)),
            this, SLOT(setProgressMessage(QString)));
    connect(&dataFlowModel->getVolumeData(), SIGNAL(progressValueChanged(int)),
            this, SLOT(setProgressValue(int)));
    connect(&dataFlowModel->getVolumeData(), SIGNAL(progressCompleted()),
            this, SLOT(completeProgress()));
    connect(&dataFlowModel->getVolumeData(), SIGNAL(progressAborted(QString)),
            this, SLOT(abortProgress(QString)));

    // Loading single stack
    connect(this, SIGNAL(singleStackLoadRequested(QString)),
            &dataFlowModel->getVolumeData(), SLOT(loadChannels(QString)));
    connect(&dataFlowModel->getVolumeData(), SIGNAL(channelsLoaded(int)),
            this, SLOT(onDataLoadFinished()));

    // Color toggling
    connect(this, SIGNAL(channelVisibilityChanged(int,bool)),
            &dataFlowModel->getDataColorModel(), SLOT(setChannelVisibility(int,bool)));
    connect(ui.resetColorsButton, SIGNAL(clicked()),
            &dataFlowModel->getDataColorModel(), SLOT(resetColors()));
    connect(ui.sharedGammaWidget, SIGNAL(gammaBrightnessChanged(qreal)),
            &dataFlowModel->getDataColorModel(), SLOT(setSharedGamma(qreal)));
    connect(ui.referenceGammaWidget, SIGNAL(gammaBrightnessChanged(qreal)),
            &dataFlowModel->getDataColorModel(), SLOT(setReferenceGamma(qreal)));
    connect(&dataFlowModel->getDataColorModel(), SIGNAL(dataChanged()),
            this, SLOT(onColorModelChanged()));
    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(visibilityChanged()),
            this, SLOT(onSelectionModelVisibilityChanged()));
    connect(this, SIGNAL(initializeColorModelRequested()),
            &dataFlowModel->getDataColorModel(), SLOT(initializeRgba32()));
    connect(this, SIGNAL(initializeColorModelRequested()),
            &dataFlowModel->getSlow3DColorModel(), SLOT(initializeRgba32()));
    connect(this, SIGNAL(offset3dGammaChanged(qreal)),
            &dataFlowModel->getSlow3DColorModel(), SLOT(setSharedGamma(qreal)));
    connect(this, SIGNAL(offset3dGammaChanged(qreal)),
            &dataFlowModel->getSlow3DColorModel(), SLOT(setReferenceGamma(qreal)));
}

bool NaMainWindow::tearDownOldDataFlowModel()
{
    if (NULL == dataFlowModel)
        return true;

    // TODO - orderly shut down of old data flow model
    DataFlowModel* dfm = dataFlowModel; // save pointer
    // TODO - make sure clients respect setting to null
    // TODO - make sure this does not yet delete dataFlowModel
    setDataFlowModel(NULL);
    // TODO - acquire write locks to make sure all clients are done reading
    { // especially NaVolumeData
        NaVolumeData::Writer volumeWriter(dfm->getVolumeData());
    }
    delete dfm;
    return true;
}

bool NaMainWindow::createNewDataFlowModel()
{
    if (NULL != dataFlowModel) {
        bool result = tearDownOldDataFlowModel();
        if (!result)
            return false;
    }
    DataFlowModel* dfm = new DataFlowModel();
    setDataFlowModel(dfm);
    return true;
}

bool NaMainWindow::loadAnnotationSessionFromDirectory(QDir imageInputDirectory)
{
    createNewDataFlowModel();

    // Need to construct (temporary until backend implemented) MultiColorImageStackNode from this directory
    // This code will be redone when the node/filestore is implemented.
    QString originalImageStackFilePath = imageInputDirectory.absolutePath() + "/" + MultiColorImageStackNode::IMAGE_STACK_BASE_FILENAME;
//    QFile originalImageStackFile(originalImageStackFilePath);
//    if (!originalImageStackFile.exists()) {
//        QMessageBox::warning(this, tr("Could not find expected image stack tif file"),
//                             "Error finding file="+originalImageStackFilePath);
//        return false;
//    }
    QString maskLabelFilePath = imageInputDirectory.absolutePath() + "/" + MultiColorImageStackNode::IMAGE_MASK_BASE_FILENAME;
//    QFile maskLabelFile(maskLabelFilePath);
//    if (!maskLabelFile.exists()) {
//        QMessageBox::warning(this, tr("Could not find expected image stack mask file"),
//                             "Error finding file="+maskLabelFilePath);
//        return false;
//    }
    QString referenceStackFilePath = imageInputDirectory.absolutePath() + "/" + MultiColorImageStackNode::IMAGE_REFERENCE_BASE_FILENAME;
//    QFile referenceStackFile(referenceStackFilePath);
//    if (!referenceStackFile.exists()) {
//        QMessageBox::warning(this, tr("Could not find expected reference stack file"),
//                             "Error finding file="+referenceStackFilePath);
//        return false;
//    }

    // Create input nodes
    MultiColorImageStackNode* multiColorImageStackNode = new MultiColorImageStackNode(imageInputDirectory);
    multiColorImageStackNode->setPathToMulticolorLabelMaskFile(maskLabelFilePath);
    multiColorImageStackNode->setPathToOriginalImageStackFile(originalImageStackFilePath);
    multiColorImageStackNode->setPathToReferenceStackFile(referenceStackFilePath);
    dataFlowModel->setMultiColorImageStackNode(multiColorImageStackNode);

    // Create result node
    long resultNodeId=TimebasedIdentifierGenerator::getSingleId();
    NeuronAnnotatorResultNode* resultNode = new NeuronAnnotatorResultNode(resultNodeId);
    if (!resultNode->ensureDirectoryExists()) {
        QMessageBox::warning(this, tr("Could not create NeuronAnnotationResultNode"),
                             "Error creating directory="+resultNode->getDirectoryPath());
        return false;
    }
    dataFlowModel->setNeuronAnnotatorResultNode(resultNode);

    // Load session
    setViewMode(VIEW_NEURON_SEPARATION);
    if (! dataFlowModel->loadVolumeData()) return false;
    // dataChanged() signal will be emitted if load succeeds

    // Show reference brightness slider in single neuron mode
    // ui.referenceGammaWidget->setVisible(true);

    return true;
}

void NaMainWindow::setTitle(QString title) {
    setWindowTitle(QString("%1 - V3D Neuron Annotator").arg(title));
}

/* slot */
void NaMainWindow::processUpdatedVolumeData() // activated by volumeData::dataChanged() signal
{
    onDataLoadFinished();
    // TODO -- install separate listeners for dataChanged() in the various display widgets

    dataFlowModel->loadLsmMetadata();

    {
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (! volumeReader.hasReadLock()) return;
        const Image4DProxy<My4DImage>& imgProxy = volumeReader.getOriginalImageProxy();
        const Image4DProxy<My4DImage>& refProxy = volumeReader.getReferenceImageProxy();

        setZRange(1, imgProxy.sz);
        // Start in middle of volume
        ui.naZStackWidget->setCurrentZSlice(imgProxy.sz / 2 + 1);
        // Need at least two colors for use of the color buttons to make sense
        ui.HDRRed_pushButton->setEnabled(imgProxy.sc > 1);
        ui.HDRGreen_pushButton->setEnabled(imgProxy.sc > 1);
        ui.HDRBlue_pushButton->setEnabled(imgProxy.sc > 2);
        ui.HDRNc82_pushButton->setEnabled(refProxy.sc > 0);
    }

    resetVolumeCutRange();
}

/* slot */
void NaMainWindow::resetVolumeCutRange()
{
    int mx, my, mz;
    mx = my = mz = 0;
    // first try VolumeTexture to get dimensions
    {
        VolumeTexture::Reader textureReader(dataFlowModel->getVolumeTexture());
        if (! dataFlowModel->getVolumeTexture().readerIsStale(textureReader))
        {
            Dimension size = textureReader.originalImageSize();
            mx = size.x() - 1;
            my = size.y() - 1;
            mz = size.z() - 1;
        }
    }
    // if that fails, try VolumeData
    if (mx <= 0)
    {
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (volumeReader.hasReadLock())
        {
            const Image4DProxy<My4DImage>& imgProxy = volumeReader.getOriginalImageProxy();
            mx = imgProxy.sx - 1;
            my = imgProxy.sy - 1;
            mz = imgProxy.sz - 1;
        }
    }
    if (mx <= 0)
        return;

    // volume cut update
    ui.XcminSlider->setRange(0, mx);
    ui.XcminSlider->setValue(0);
    ui.XcmaxSlider->setRange(0, mx);
    ui.XcmaxSlider->setValue(mx);

    ui.YcminSlider->setRange(0, my);
    ui.YcminSlider->setValue(0);
    ui.YcmaxSlider->setRange(0, my);
    ui.YcmaxSlider->setValue(my);

    ui.ZcminSlider->setRange(0, my);
    ui.ZcminSlider->setValue(0);
    ui.ZcmaxSlider->setRange(0, my);
    ui.ZcmaxSlider->setValue(my);

} // release lock

DataFlowModel* NaMainWindow::getDataFlowModel() const {
    return dataFlowModel;
}

void NaMainWindow::initializeGalleries()
{
    initializeOverlayGallery();
    initializeNeuronGallery();
}

void NaMainWindow::updateGalleries()
{
    updateOverlayGallery();
    updateNeuronGallery();
    // Show or hide galleries depending on data structures
    // In particular, hide galleries when there is no reference, nor any neurons.
    bool bShowGalleries = false;
    if (NULL == dataFlowModel)
        ; // bShowGalleries = false;
    else {
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (volumeReader.hasReadLock()) {
            if ( (volumeReader.getNumberOfNeurons() > 0)
                || (volumeReader.hasReferenceImage()) )
            {
                bShowGalleries = true;
            }
        }
    }
    // ui.mipsFrame->setVisible(bShowGalleries);
    if (bShowGalleries)
        setViewMode(VIEW_NEURON_SEPARATION);
}

void NaMainWindow::initializeOverlayGallery()
{
    // qDebug() << "NaMainWindow::initializeOverlayGallery()" << __FILE__ << __LINE__;

    // Create layout, only if needed.
    QFrame* ui_maskFrame = qFindChild<QFrame*>(this, "maskFrame");
    if (! ui_maskFrame->layout()) {
        ui_maskFrame->setLayout(new QHBoxLayout());
        assert(ui_maskFrame->layout());
    }
    QLayout *managementLayout = ui_maskFrame->layout();

    // Create new buttons, only if needed.
    if (overlayGalleryButtonList.size() != 2) {
        // Delete any old contents from the layout, such as previous thumbnails
        QLayoutItem * item;
        while ( ( item = managementLayout->takeAt(0)) != NULL )
        {
            delete item->widget();
            delete item;
        }

        QImage initialImage(100, 140, QImage::Format_ARGB32);
        initialImage.fill(Qt::gray);
        GalleryButton* referenceButton = new GalleryButton(
                initialImage,
                "Reference",
                DataFlowModel::REFERENCE_MIP_INDEX,
                GalleryButton::OVERLAY_BUTTON);
        managementLayout->addWidget(referenceButton);
        overlayGalleryButtonList.append(referenceButton);

        GalleryButton* backgroundButton = new GalleryButton(
                initialImage,
                "Background",
                DataFlowModel::BACKGROUND_MIP_INDEX,
                GalleryButton::OVERLAY_BUTTON);
        managementLayout->addWidget(backgroundButton);
        overlayGalleryButtonList.append(backgroundButton);
    }
    // Initialize signals whether the buttons were already there or not
    for (int i = 0; i < 2; ++i)
    {
        overlayGalleryButtonList[i]->setNeuronSelectionModel(dataFlowModel->getNeuronSelectionModel());
    }
    updateOverlayGallery();
}

void NaMainWindow::updateOverlayGallery()
{
    if (overlayGalleryButtonList.size() != 2) return; // not initialized
    if (NULL == dataFlowModel) return;

    {
        NeuronSelectionModel::Reader selectionReader(dataFlowModel->getNeuronSelectionModel());
        if (selectionReader.getOverlayStatusList().size() < 2) return;
        if (selectionReader.hasReadLock())
        {
            for (int i = 0; i < 2; ++i)
                overlayGalleryButtonList[i]->setChecked(selectionReader.getOverlayStatusList().at(i));
        }
    }

    {
        GalleryMipImages::Reader mipReader(dataFlowModel->getGalleryMipImages()); // acquire read lock
        if (mipReader.hasReadLock() && (mipReader.getNumberOfOverlays() == 2))
        {
            for (int i = 0; i < 2; ++i)
                overlayGalleryButtonList[i]->setThumbnailIcon(*mipReader.getOverlayMip(i));
        }
    }
    for (int i = 0; i < 2; ++i)
        overlayGalleryButtonList[i]->update();
}

void NaMainWindow::initializeNeuronGallery()
{
    GalleryMipImages::Reader mipReader(dataFlowModel->getGalleryMipImages()); // acquire read lock
    if (! mipReader.hasReadLock()) return;

    NeuronSelectionModel::Reader selectionReader(dataFlowModel->getNeuronSelectionModel());
    if (! selectionReader.hasReadLock()) return;

    if (neuronGalleryButtonList.size() != mipReader.getNumberOfNeurons())
    {
        neuronGalleryButtonList.clear();
        ui.fragmentGalleryWidget->clear(); // deletes buttons

        // qDebug() << "Number of neuron masks = " << mipReader.getNumberOfNeurons();
        for (int i = 0; i < mipReader.getNumberOfNeurons(); ++i)
        {
            GalleryButton* button = new GalleryButton(
                    *mipReader.getNeuronMip(i),
                    QString("Neuron fragment %1").arg(i),
                    i,
                    GalleryButton::NEURON_BUTTON);
            button->setContextMenu(neuronContextMenu);
            neuronGalleryButtonList.append(button);
            ui.fragmentGalleryWidget->appendFragment(button);
        }

        // qDebug() << "createMaskGallery() end size=" << mipReader.getNumberOfNeurons();
    }
    // Update signals whether the buttons were already there or not
    for (int i = 0; i < mipReader.getNumberOfNeurons(); ++i)
    {
        neuronGalleryButtonList[i]->setThumbnailIcon(*mipReader.getNeuronMip(i));
        neuronGalleryButtonList[i]->setChecked(selectionReader.getMaskStatusList().at(i));
        neuronGalleryButtonList[i]->update();
        neuronGalleryButtonList[i]->setNeuronSelectionModel(
                dataFlowModel->getNeuronSelectionModel());
    }
    ui.fragmentGalleryWidget->updateButtonsGeometry();
}

void NaMainWindow::updateNeuronGallery()
{
    GalleryMipImages::Reader mipReader(dataFlowModel->getGalleryMipImages()); // acquire read lock
    if (! mipReader.hasReadLock()) return;
    NeuronSelectionModel::Reader selectionReader(dataFlowModel->getNeuronSelectionModel());
    if (! selectionReader.hasReadLock()) return;

    if (neuronGalleryButtonList.size() != mipReader.getNumberOfNeurons())
        initializeNeuronGallery();
    else {
        for (int i = 0; i < mipReader.getNumberOfNeurons(); ++i)
        {
            neuronGalleryButtonList[i]->setThumbnailIcon(*mipReader.getNeuronMip(i));
            neuronGalleryButtonList[i]->setChecked(selectionReader.getMaskStatusList().at(i));
            neuronGalleryButtonList[i]->update();
        }
        ui.fragmentGalleryWidget->updateButtonsGeometry();
    }
}

void NaMainWindow::on3DViewerRotationChanged(const Rotation3D& rot)
{
    Vector3D angles = rot.convertBodyFixedXYZRotationToThreeAngles();
    int rotX = Na3DWidget::radToDeg(angles.x());
    int rotY = Na3DWidget::radToDeg(angles.y());
    int rotZ = Na3DWidget::radToDeg(angles.z());
    int oldRotX = ui.rotXWidget->spinBox->value();
    int oldRotY = ui.rotYWidget->spinBox->value();
    int oldRotZ = ui.rotZWidget->spinBox->value();
    if (Na3DWidget::eulerAnglesAreEquivalent(rotX, rotY, rotZ, oldRotX, oldRotY, oldRotZ))
        return;
    // Block signals from individual rot widgets until we update them all
    ui.rotXWidget->blockSignals(true);
    ui.rotYWidget->blockSignals(true);
    ui.rotZWidget->blockSignals(true);

    ui.rotXWidget->setAngle(rotX);
    ui.rotYWidget->setAngle(rotY);
    ui.rotZWidget->setAngle(rotZ);

    ui.rotXWidget->blockSignals(false);
    ui.rotYWidget->blockSignals(false);
    ui.rotZWidget->blockSignals(false);
}

void NaMainWindow::update3DViewerXYZBodyRotation()
{
    int rotX = ui.rotXWidget->spinBox->value();
    int rotY = ui.rotYWidget->spinBox->value();
    int rotZ = ui.rotZWidget->spinBox->value();
    // qDebug() << rotX << ", " << rotY << ", " << rotZ;
    ui.v3dr_glwidget->setXYZBodyRotationInt(rotX, rotY, rotZ);
}

// update neuron selected status
void NaMainWindow::synchronizeGalleryButtonsToAnnotationSession(QString updateString)
{
    NeuronSelectionModel::Reader selectionReader(
            dataFlowModel->getNeuronSelectionModel());
    if (! selectionReader.hasReadLock()) return;

    // We are not using the update string in this case, which is from the modelUpdated() signal,
    // because we are doing a total update.
    int maskStatusListSize=selectionReader.getMaskStatusList().size();
    int neuronGalleryButtonSize=neuronGalleryButtonList.size();
    assert(neuronGalleryButtonSize == maskStatusListSize);
    for (int i = 0; i < maskStatusListSize; i++) {
        neuronGalleryButtonList.at(i)->setChecked(
                selectionReader.neuronMaskIsChecked(i));
    }

    // Reference toggle
    if (selectionReader.getOverlayStatusList().at(DataFlowModel::REFERENCE_MIP_INDEX)) {
        overlayGalleryButtonList.at(DataFlowModel::REFERENCE_MIP_INDEX)->setChecked(true);
    } else {
        overlayGalleryButtonList.at(DataFlowModel::REFERENCE_MIP_INDEX)->setChecked(false);
    }

    // Background toggle
    if (selectionReader.getOverlayStatusList().at(DataFlowModel::BACKGROUND_MIP_INDEX)) {
        overlayGalleryButtonList.at(DataFlowModel::BACKGROUND_MIP_INDEX)->setChecked(true);
    } else {
        overlayGalleryButtonList.at(DataFlowModel::BACKGROUND_MIP_INDEX)->setChecked(false);
    }
}

/* slot */
void NaMainWindow::setProgressValue(int progressValueParam)
{
    if (progressValueParam >= 100) {
        completeProgress();
        return;
    }
    statusProgressBar->setValue(progressValueParam);
    statusProgressBar->show();
}

/* slot */
void NaMainWindow::setProgressMessage(QString msg)
{
    statusBar()->showMessage(""); // avoid overlap of temporary messages with progress message
    statusProgressMessage->setText(msg);
    statusProgressMessage->show();
}

/* slot */
void NaMainWindow::completeProgress()
{
    statusProgressBar->hide();
    statusProgressMessage->hide();
    statusBar()->showMessage("", 500);
}

/* slot */
void NaMainWindow::abortProgress(QString msg)
{
    statusProgressBar->hide();
    statusProgressMessage->hide();
    statusBar()->showMessage(msg, 1000);
}

static const bool use3DProgress = false;

void NaMainWindow::set3DProgress(int prog)
{
    if (prog >= 100) {
        complete3DProgress();
        return;
    }
    if (use3DProgress) {
        ui.progressBar3d->setValue(prog);
        // ui.v3dr_glwidget->setResizeEnabled(false); // don't show ugly brief resize behavior
        ui.widget_progress3d->show();
    }
    else
        setProgressValue(prog);
}

void NaMainWindow::complete3DProgress()
{
    if (use3DProgress) {
        ui.widget_progress3d->hide();
        // avoid jerky resize to accomodated progress widget
        QCoreApplication::processEvents(); // flush pending resize events
        ui.v3dr_glwidget->resizeEvent(NULL);
        ui.v3dr_glwidget->setResizeEnabled(true);
        //
        ui.v3dr_glwidget->update();
    }
    else completeProgress();
}

void NaMainWindow::set3DProgressMessage(QString msg)
{
    if (use3DProgress) {
        ui.progressLabel3d->setText(msg);
        ui.v3dr_glwidget->setResizeEnabled(false); // don't show ugly brief resize behavior
        ui.widget_progress3d->show();
    }
    else
        setProgressMessage(msg);
}

// NutateThread
