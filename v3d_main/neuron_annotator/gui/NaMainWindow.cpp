// Fix windows compile problem with windows.h being included too late.
// I wish it wasn't included at all...
#ifdef _MSC_VER
#define NOMINMAX //added by PHC, 2010-05-20 to overcome VC min max macro
#include <windows.h>
#endif

#include "../3drenderer/GLee2glew.h" //2020-2-10 RZC

#include "RendererNeuronAnnotator.h"

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
#include "GalleryButton.h"
#include "../../cell_counter/CellCounter3D.h"
#include "../NeuronSelector.h"
#include "FragmentGalleryWidget.h"
#include "AnnotationWidget.h"
#include "../utility/loadV3dFFMpeg.h"
#include "PreferencesDialog.h"
#include "../utility/FooDebug.h"
#include "../utility/url_tools.h"
#include "../entity_model/Entity.h"

#include <cstdlib> // getenv

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

inline const char * getConsolePort()
{
    const char *port;
    port = getenv("WORKSTATION_SERVICE_PORT");
    if(NULL == port) port = "30001";
    return port;
}

NaMainWindow::NaMainWindow(QWidget * parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , ui(new Ui::NaMainWindow)
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
    , cutPlanner(NULL)
{
    const char* port = getConsolePort();
    qDebug() << "Using console port: " << port;
    QString qport(port);
    QString url = QString("http://localhost:%1/axis2/services/cds").arg(port);
    consoleUrl = new char[url.length() + 1];
    QByteArray ba = url.toUtf8();
    strcpy(consoleUrl, ba.data());
    qDebug() << "Using console URL: " << consoleUrl;

    // Set up potential 3D stereo modes before creating QGLWidget.
#ifdef ENABLE_STEREO
    QGLFormat glFormat = QGLFormat::defaultFormat();
    glFormat.setStereo(true);
    glFormat.setDoubleBuffer(true);
    if (glFormat.stereo()) {
       // qDebug() << "Attempting to set 3D stereo format";
    }
    else {
       // qDebug() << "Failed to make stereo 3D default QGLFormat";
    }
    QGLFormat::setDefaultFormat(glFormat);
#endif

    recentFileActions.fill(NULL, NaMainWindow::maxRecentFiles);

    ui->setupUi(this);
    setAcceptDrops(true);

    // Z value comes from camera model
    qRegisterMetaType<Vector3D>("Vector3D");

    // hide neuron gallery until there are neurons to show
    setViewMode(VIEW_SINGLE_STACK);
    // ui->mipsFrame->setVisible(false);

    // hide compartment map until it works correctly and is not so slow on Mac
    ui->compartmentSelectGroupBox->hide();

    dataFlowModel=0;
    // TODO - neuronSelector should probably be a member of Na3DViewer, not of NaMainWindow
    // neuronSelector = new NeuronSelector(this);

    // Create stubs for recent file menu
    for (int i = 0; i < maxRecentFiles; ++i) {
        recentFileActions[i] = new OpenFileAction(this);
        ui->menuOpen_Recent->addAction(recentFileActions[i]);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(openFileRequested(QString)),
                this, SLOT(openFileOrUrl(QString)));
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
    ui->widget_progress3d->hide();

    // hide the File->Open 3D Image stack menu item
    ui->menuFile->removeAction(ui->actionLoad_Tiff);
    ui->menuFile->removeAction(ui->actionCell_Counter_3D_2ch_lsm);

    // hide dev-version rotate-X movie maker, until it become more user-friendly
    ui->menuExport->removeAction(ui->actionX_Rotation_Movie);

    // hide fps option: it's for debugging
    ui->menuView->removeAction(ui->actionMeasure_Frame_Rate);

    // hide octree test item
    ui->menuFile->removeAction(ui->actionOpen_Octree_Volume);

#ifdef USE_FFMPEG
    ui->actionLoad_movie_as_texture->setVisible(true);
    ui->actionLoad_fast_separation_result->setVisible(true);
#else
    ui->actionLoad_movie_as_texture->setVisible(false);
    ui->actionLoad_fast_separation_result->setVisible(false);
#endif

    // visualize compartment map
    //QDockWidget *dock = new QDockWidget(tr("Compartment Map"), this);
    //dock->setWidget( ui->compartmentMapWidget);
    qRegisterMetaType<QList<LabelSurf> >("QList<LabelSurf>");

    ui->compartmentMapWidget->setComboBox(ui->compartmentMapComboBox);
    connect(ui->compartmentMapComboBox, SIGNAL(currentIndexChanged(int)), ui->compartmentMapWidget, SLOT(switchCompartment(int)));
    //connect(ui->compartmentMapWidget, SIGNAL(viscomp3dview(QList<LabelSurf>)), (Renderer_gl1*)(ui->v3dr_glwidget->getRenderer()), SLOT(setListLabelSurf(QList<LabelSurf>))); // vis compartments in Na3Dviewer

    // Wire up MIP viewer
    // Status bar message
    connect(ui->naLargeMIPWidget, SIGNAL(statusMessage(const QString&)),
            statusBar(), SLOT(showMessage(const QString&)));
    connect(ui->naZStackWidget, SIGNAL(statusMessage(const QString&)),
            statusBar(), SLOT(showMessage(const QString&)));
    ui->progressWidgetMip->hide();
    connect(ui->naLargeMIPWidget, SIGNAL(showProgress()),
            ui->progressWidgetMip, SLOT(show()));
    connect(ui->naLargeMIPWidget, SIGNAL(hideProgress()),
            ui->progressWidgetMip, SLOT(hide()));
    connect(ui->naLargeMIPWidget, SIGNAL(setProgressMax(int)),
            ui->progressBarMip, SLOT(setMaximum(int)));
    connect(ui->naLargeMIPWidget, SIGNAL(setProgress(int)),
            ui->progressBarMip, SLOT(setValue(int)));
    ui->progressWidgetZ->hide();

    // ui->gammaWidget_Zstack->hide();
    // Distinguish the two gamma sliders
    ui->sharedGammaWidget->gamma_label->setText("N "); // "neurons"
    ui->sharedGammaWidget->setToolTip(tr("Brightness/gamma of data"));
    ui->referenceGammaWidget->gamma_label->setText("R "); // "reference"
    ui->referenceGammaWidget->setToolTip(tr("Brightness/gamma of reference channel"));

    ui->BoxSize_spinBox->setMinimum(NaZStackWidget::minHdrBoxSize);

    // Wire up Z-stack / HDR viewer
    connect(ui->HDR_checkBox, SIGNAL(toggled(bool)),
            ui->naZStackWidget, SLOT(setHDRCheckState(bool)));
    connect(ui->naZStackWidget, SIGNAL(changedHDRCheckState(bool)),
            ui->HDR_checkBox, SLOT(setChecked(bool)));
    connect(ui->HDRRed_pushButton, SIGNAL(clicked()),
            ui->naZStackWidget, SLOT(setRedChannel()));
    connect(ui->HDRGreen_pushButton, SIGNAL(clicked()),
            ui->naZStackWidget, SLOT(setGreenChannel()));
    connect(ui->HDRBlue_pushButton, SIGNAL(clicked()),
            ui->naZStackWidget, SLOT(setBlueChannel()));
    connect(ui->HDRNc82_pushButton, SIGNAL(clicked()),
            ui->naZStackWidget, SLOT(setNc82Channel()));
    connect(ui->naZStackWidget, SIGNAL(curColorChannelChanged(NaZStackWidget::Color)),
            this, SLOT(onHdrChannelChanged(NaZStackWidget::Color)));
    ui->naZStackWidget->setHDRCheckState(false);

    connect(ui->ZSlice_horizontalScrollBar, SIGNAL(valueChanged(int)),
            ui->naZStackWidget, SLOT(setCurrentZSlice(int)));
    connect(ui->naZStackWidget, SIGNAL(curZsliceChanged(int)),
            ui->ZSlice_horizontalScrollBar, SLOT(setValue(int)));
    connect(ui->BoxSize_spinBox, SIGNAL(valueChanged(int)),
            ui->naZStackWidget, SLOT(setHdrBoxSize(int)));
    connect(ui->naZStackWidget, SIGNAL(hdrBoxSizeChanged(int)),
            ui->BoxSize_spinBox, SLOT(setValue(int)));

    // 3D viewer
    connect(ui->rotationResetButton, SIGNAL(clicked()),
            ui->v3dr_glwidget, SLOT(resetRotation()));
    connect(ui->nutateButton, SIGNAL(toggled(bool)),
            this, SLOT(setNutate(bool)));
    connect(this, SIGNAL(nutatingChanged(bool)),
            ui->nutateButton, SLOT(setChecked(bool)));
    connect(ui->actionAnimate_3D_nutation, SIGNAL(toggled(bool)),
            this, SLOT(setNutate(bool)));
    connect(this, SIGNAL(nutatingChanged(bool)),
            ui->actionAnimate_3D_nutation, SLOT(setChecked(bool)));
    connect(ui->v3dr_glwidget, SIGNAL(signalTextureLoaded()),
            this, SLOT(onDataLoadFinished()));
    /* obsolete.  now we toggle channels.
    connect(ui->redToggleButton, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setChannelR(bool)));
    connect(ui->greenToggleButton, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setChannelG(bool)));
    connect(ui->blueToggleButton, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setChannelB(bool)));
            */
    // 3D rotation
    // synchronize compartment map
    connect(&sharedCameraModel, SIGNAL(rotationChanged(const Rotation3D&)),
            ui->compartmentMapWidget, SLOT(setRotation(const Rotation3D&)));
    // connect(&sharedCameraModel, SIGNAL(focusChanged(const Vector3D&)),
    //        ui->compartmentMapWidget, SLOT(setFocus(const Vector3D&)));

    connect(&(ui->v3dr_glwidget->cameraModel), SIGNAL(rotationChanged(const Rotation3D&)),
            this, SLOT(on3DViewerRotationChanged(const Rotation3D&)));
    connect(ui->rotXWidget, SIGNAL(angleChanged(int)),
            this, SLOT(update3DViewerXYZBodyRotation()));
    connect(ui->rotYWidget, SIGNAL(angleChanged(int)),
            this, SLOT(update3DViewerXYZBodyRotation()));
    connect(ui->rotZWidget, SIGNAL(angleChanged(int)),
            this, SLOT(update3DViewerXYZBodyRotation()));
    connect(ui->v3dr_glwidget, SIGNAL(progressValueChanged(int)),
            this, SLOT(set3DProgress(int)));
    connect(ui->v3dr_glwidget, SIGNAL(progressComplete()),
            this, SLOT(complete3DProgress()));
    connect(ui->v3dr_glwidget, SIGNAL(progressMessageChanged(QString)),
            this, SLOT(set3DProgressMessage(QString)));
    connect(ui->v3dr_glwidget, SIGNAL(progressAborted(QString)),
            this, SLOT(complete3DProgress()));

    // 3D volume cut
    connect(ui->v3dr_glwidget, SIGNAL(changeXCut0(int)), ui->XcminSlider, SLOT(setValue(int))); // x-cut
    connect(ui->XcminSlider, SIGNAL(valueChanged(int)), ui->v3dr_glwidget, SLOT(setXCut0(int)));
    connect(ui->v3dr_glwidget, SIGNAL(changeXCut1(int)), ui->XcmaxSlider, SLOT(setValue(int)));
    connect(ui->XcmaxSlider, SIGNAL(valueChanged(int)), ui->v3dr_glwidget, SLOT(setXCut1(int)));

    connect(ui->v3dr_glwidget, SIGNAL(changeYCut0(int)), ui->YcminSlider, SLOT(setValue(int))); // y-cut
    connect(ui->YcminSlider, SIGNAL(valueChanged(int)), ui->v3dr_glwidget, SLOT(setYCut0(int)));
    connect(ui->v3dr_glwidget, SIGNAL(changeYCut1(int)), ui->YcmaxSlider, SLOT(setValue(int)));
    connect(ui->YcmaxSlider, SIGNAL(valueChanged(int)), ui->v3dr_glwidget, SLOT(setYCut1(int)));

    connect(ui->v3dr_glwidget, SIGNAL(changeZCut0(int)), ui->ZcminSlider, SLOT(setValue(int))); // z-cut
    connect(ui->ZcminSlider, SIGNAL(valueChanged(int)), ui->v3dr_glwidget, SLOT(setZCut0(int)));
    connect(ui->v3dr_glwidget, SIGNAL(changeZCut1(int)), ui->ZcmaxSlider, SLOT(setValue(int)));
    connect(ui->ZcmaxSlider, SIGNAL(valueChanged(int)), ui->v3dr_glwidget, SLOT(setZCut1(int)));

    connect(ui->XCutCB, SIGNAL(toggled(bool)), ui->v3dr_glwidget, SLOT(setXCutLock(bool)));
    connect(ui->YCutCB, SIGNAL(toggled(bool)), ui->v3dr_glwidget, SLOT(setYCutLock(bool)));
    connect(ui->ZCutCB, SIGNAL(toggled(bool)), ui->v3dr_glwidget, SLOT(setZCutLock(bool)));

    connect(ui->slabThicknessSlider, SIGNAL(valueChanged(int)),
            ui->v3dr_glwidget, SLOT(setSlabThickness(int)));
    connect(ui->slabPositionSlider, SIGNAL(valueChanged(int)),
            ui->v3dr_glwidget, SLOT(setSlabPosition(int)));
    connect(ui->v3dr_glwidget, SIGNAL(slabThicknessChanged(int)),
            this, SLOT(onSlabThicknessChanged(int)));
            // ui->slabThicknessSlider, SLOT(setValue(int)));
    connect(ui->v3dr_glwidget, SIGNAL(slabPositionChanged(int)),
            ui->slabPositionSlider, SLOT(setValue(int)));
    connect(ui->freezeFrontBackButton, SIGNAL(clicked()),
            ui->v3dr_glwidget, SLOT(clipSlab()));

    // alpha blending
    connect(ui->action3D_alpha_blending, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setAlphaBlending(bool)));
    connect(ui->v3dr_glwidget, SIGNAL(alphaBlendingChanged(bool)),
            ui->action3D_alpha_blending, SLOT(setChecked(bool)));

    // show axes
    connect(ui->actionShow_Axes, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setShowCornerAxes(bool)));
    // show scale bar
    connect(ui->actionShow_Scale_Bar, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(showScaleBar(bool)));

    // Whether to use common zoom and focus in MIP, ZStack and 3D viewers
    connect(ui->actionLink_viewers, SIGNAL(toggled(bool)),
            this, SLOT(unifyCameras(bool)));
    unifyCameras(true); // Start with cameras linked
    connect(ui->resetViewButton, SIGNAL(clicked()),
            this, SLOT(resetView()));
    connect(ui->zoomWidget, SIGNAL(zoomValueChanged(qreal)),
            &sharedCameraModel, SLOT(setScale(qreal)));
    connect(&sharedCameraModel, SIGNAL(scaleChanged(qreal)),
            ui->zoomWidget, SLOT(setZoomValue(qreal)));
    connect(&sharedCameraModel, SIGNAL(scaleChanged(qreal)),
            this, SLOT(updateViewers()));
    // Colors
    connect(ui->redToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(setChannelZeroVisibility(bool)));
    connect(ui->greenToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(setChannelOneVisibility(bool)));
    connect(ui->blueToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(setChannelTwoVisibility(bool)));

    // Crosshair
    connect(ui->actionShow_Crosshair, SIGNAL(toggled(bool)),
            this, SLOT(setCrosshairVisibility(bool)));
    connect(this, SIGNAL(crosshairVisibilityChanged(bool)),
            ui->actionShow_Crosshair, SLOT(setChecked(bool)));
    connect(this, SIGNAL(crosshairVisibilityChanged(bool)),
            ui->naLargeMIPWidget, SLOT(showCrosshair(bool)));
    connect(this, SIGNAL(crosshairVisibilityChanged(bool)),
            ui->v3dr_glwidget, SLOT(showCrosshair(bool)));
    connect(this, SIGNAL(crosshairVisibilityChanged(bool)),
            ui->naZStackWidget, SLOT(showCrosshair(bool)));
    retrieveCrosshairVisibilitySetting();

    // Axes
    // TODO I want a small set of axes that sits in the lower left corner.  The gigantic axes are less useful.
    // connect(ui->actionShow_Axes, SIGNAL(toggled(bool)),
    //         ui->v3dr_glwidget, SLOT(enableShowAxes(bool)));

    // Clear status message when viewer changes
    connect(ui->viewerStackedWidget, SIGNAL(currentChanged(int)),
            this, SLOT(onViewerChanged(int)));

    // Create "Undo" menu options
    // TODO - figure out which of these variables to expose once we have a QUndoCommand to work with.
    QUndoGroup * undoGroup = new QUndoGroup(this);
    QAction * undoAction = undoGroup->createUndoAction(this);
    undoAction->setShortcuts(QKeySequence::Undo);
    QAction * redoAction = undoGroup->createRedoAction(this);
    redoAction->setShortcuts(QKeySequence::Redo);
    ui->menuEdit->insertAction(ui->menuEdit->actions().at(0), redoAction);
    ui->menuEdit->insertAction(redoAction, undoAction);
    // expose undoStack
    undoStack = new QUndoStack(undoGroup);
    undoGroup->setActiveStack(undoStack);
    ui->v3dr_glwidget->setUndoStack(*undoStack);

    // Connect sort buttons to gallery widget
    connect(ui->gallerySortBySizeButton, SIGNAL(clicked()),
            ui->fragmentGalleryWidget, SLOT(sortBySize()));
    connect(ui->gallerySortByColorButton, SIGNAL(clicked()),
            ui->fragmentGalleryWidget, SLOT(sortByColor()));
    connect(ui->gallerySortByIndexButton, SIGNAL(clicked()),
            ui->fragmentGalleryWidget, SLOT(sortByIndex()));
    connect(ui->gallerySortByNameButton, SIGNAL(clicked()),
            ui->fragmentGalleryWidget, SLOT(sortByName()));

    // Allow cross-thread signals/slots that pass QList<int>
    qRegisterMetaType< QList<int> >("QList<int>");

    // Set up the annotation widget
    ui->annotationFrame->setMainWindow(this);
    ui->centralwidget->installEventFilter(ui->annotationFrame);
    ui->annotationFrame->consoleConnect(3);

    // NeuronSelector helper class for selecting neurons
    connect(&neuronSelector, SIGNAL(neuronSelected(int)),
            ui->annotationFrame, SLOT(selectNeuron(int)));
    connect(ui->v3dr_glwidget, SIGNAL(neuronSelected(double,double,double)),
            &neuronSelector, SLOT(updateSelectedPosition(double,double,double)));
    connect(ui->actionDynamic_range, SIGNAL(triggered(bool)),
            this, SLOT(showDynamicRangeTool()));

    connect(ui->actionFull_Screen, SIGNAL(toggled(bool)),
            this, SLOT(setFullScreen(bool)));

    connect(this, SIGNAL(benchmarkTimerResetRequested()),
            this, SLOT(resetBenchmarkTimer()));
    connect(this, SIGNAL(benchmarkTimerPrintRequested(QString)),
            this, SLOT(printBenchmarkTimer(QString)));
    connect(ui->v3dr_glwidget, SIGNAL(benchmarkTimerPrintRequested(QString)),
            this, SIGNAL(benchmarkTimerPrintRequested(QString)));
    connect(ui->v3dr_glwidget, SIGNAL(benchmarkTimerResetRequested()),
            this, SIGNAL(benchmarkTimerResetRequested()));

    initializeContextMenus();
    initializeStereo3DOptions();
    connectCustomCut();
}

/* slot */
void NaMainWindow::onSlabThicknessChanged(int t)
{
    // qDebug() << "NaMainWindow::onSlabThicknessChanged()" << t << __FILE__ << __LINE__;
    if (t > ui->slabThicknessSlider->maximum()) {
        ui->slabThicknessSlider->setMaximum(t);
        ui->slabPositionSlider->setMaximum(t/2);
        ui->slabPositionSlider->setMinimum(-t/2);
    }
    ui->slabThicknessSlider->setValue(t);
}

/* slot */
void NaMainWindow::resetBenchmarkTimer()
{
    mainWindowStopWatch.restart();
}

/* slot */
void NaMainWindow::printBenchmarkTimer(QString message)
{
    // qDebug() << "BENCHMARK" << message << "at" << mainWindowStopWatch.elapsed()/1000.0 << "seconds";
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
        ui->mipsFrame->setVisible(false);
        ui->annotationFrame->setVisible(true);
        ui->referenceGammaWidget->setVisible(false);
        // qDebug() << "Changing to single stack mode" << __FILE__ << __LINE__;
    }
    if (mode == VIEW_NEURON_SEPARATION) {
        ui->mipsFrame->setVisible(true);
        ui->annotationFrame->setVisible(true);
        ui->referenceGammaWidget->setVisible(true);
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
        ui->mipsFrame->show();
    }
    ui->annotationFrame->show();
    ui->viewerSelectorAndControlFrame->show();
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
        ui->annotationFrame->hide();
        ui->mipsFrame->hide();
        ui->viewerSelectorAndControlFrame->hide();
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
           // qDebug() << "Problem saving parent directory of " << dirName;
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
       // qDebug() << "Problem saving parent directory of " << fileName;
    }

    return fileName;
}

/* slot */
void NaMainWindow::on_action1280x720_triggered() {
    int w = ui->v3dr_glwidget->width();
    int h = ui->v3dr_glwidget->height();
    int dw = 1280 - w;
    int dh = 720 - h;
    resize(width() + dw, height() + dh);
}

/* slot */
void NaMainWindow::on_actionAdd_landmark_at_cursor_triggered() {
    // qDebug() << "add landmark";
    RendererNeuronAnnotator * rna = ui->v3dr_glwidget->getRendererNa();
    if (rna != NULL) {
        float radius = 20.0;
        Vector3D focus = sharedCameraModel.focus();
        // Shape comes from type not shape argument. whatever
        //  0 - dodecahedron
        //  1 - cube
        ImageMarker landmark = ImageMarker(2, 0, focus.x(), rna->dim2 - focus.y(), rna->dim3 - focus.z(), radius);
        // cycle colors red->green->blue
        int c = viewerLandmarks3D.size() % 3;
        if (c == 0)
            landmark.color.i = 0xff5050ff;
        else if (c == 1)
            landmark.color.i = 0xff50ff50;
        else if (c == 2)
            landmark.color.i = 0xffff5050;
        viewerLandmarks3D << landmark;
        rna->sShowMarkers = true;
        rna->markerSize = (int) radius;
        rna->setLandmarks(viewerLandmarks3D);
        rna->b_showMarkerLabel = false;
        rna->b_showMarkerName = false;
        // rna->updateLandmark(); // deletes markerList!
    }
}

/* slot */
void NaMainWindow::on_actionAppend_key_frame_at_current_view_triggered() {
    // qDebug() << "append frame";
    KeyFrame newFrame(4.0);
    newFrame.storeCameraSettings(sharedCameraModel);
    newFrame.storeLandmarkVisibility(viewerLandmarks3D);
    if (dataFlowModel != NULL)
    {
        DataColorModel::Reader reader(dataFlowModel->getDataColorModel());
        newFrame.storeChannelColorModel(reader);
    }
    currentMovie.appendKeyFrame(newFrame);
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

void NaMainWindow::on_actionClear_landmarks_triggered() {
    viewerLandmarks3D.clear();
    RendererNeuronAnnotator * rna = ui->v3dr_glwidget->getRendererNa();
    if (rna != NULL) {
        rna->clearLandmarks();
    }
}

void NaMainWindow::on_actionClear_movie_triggered() {
    currentMovie.clear();
}

/* slot */
void NaMainWindow::on_actionMeasure_Frame_Rate_triggered()
{
    cout << "Measuring frame rate..." << endl;
    Rotation3D currentRotation = sharedCameraModel.rotation();
    Rotation3D dRot;
    const int numSteps = 30;
    dRot.setRotationFromAngleAboutUnitVector(
            2.0 * 3.14159 / numSteps,
            UnitVector3D(0, 1, 0));
    QElapsedTimer timer;
    timer.start();
    for (int deg = 0; deg < numSteps; ++deg)
    {
        // cout << "step " << deg << endl;
        currentRotation = dRot * currentRotation;
        sharedCameraModel.setRotation(currentRotation);
        QCoreApplication::processEvents();
#if defined(USE_Qt5)
        ui->v3dr_glwidget->update();
#else
        ui->v3dr_glwidget->updateGL();
#endif
        QCoreApplication::processEvents();
    }
    qint64 msTime = timer.elapsed();
    double meanFrameTime = msTime / (double)(numSteps);
    double frameRate = 1000.0 / meanFrameTime;
    cout << meanFrameTime << " ms per frame; " << frameRate << " frames per second" << endl;
    // cout << timer.elapsed() << endl;
}

/* slot */
void NaMainWindow::on_actionOpen_Octree_Volume_triggered()
{
    QString fileName = getStackPathWithDialog();
    if (fileName.isEmpty())
        return;
    loadSingleStack(fileName, false);
    // TODO - actually do something clever
}

/* slot */
void NaMainWindow::on_actionOpen_Single_Movie_Stack_triggered()
{
   // qDebug() << "NaMainWindow::on_actionOpen_Single_Movie_Stack_triggered";
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
       // qDebug() << "Problem saving parent directory of " << fileName;
    }

    loadSingleStack(fileName, false);
}

void NaMainWindow::on_actionPlay_movie_triggered() {
    // qDebug() << "Play movie";
    currentMovie.rewind();
    QTime movieTimer;
    movieTimer.start();
    double movieElapsedTime = 0.0;
    AnimationFrame frame;
    bool bPlayRealTime = true;
    while (currentMovie.hasMoreFrames()) {
        frame = currentMovie.getNextFrame();
        // Skip frames to catch up, if behind schedule
        movieElapsedTime += currentMovie.secondsPerFrame;
        if (bPlayRealTime && (movieElapsedTime < movieTimer.elapsed()/1000.0))
            continue; // race to next frame
        //
        animateToFrame(frame);
    }
    // Alway finish in final frame.
    if (bPlayRealTime)
        animateToFrame(frame);
}

void NaMainWindow::animateToFrame(const AnimationFrame& frame) {
    frame.retrieveCameraSettings(sharedCameraModel);
    frame.retrieveLandmarkVisibility(viewerLandmarks3D);
    RendererNeuronAnnotator * rna = ui->v3dr_glwidget->getRendererNa();
    if (rna != NULL) {
        rna->clearLandmarks();
        rna->setLandmarks(viewerLandmarks3D);
        rna->sShowMarkers = true;
    }

    // One channel visibility at a time
    // Test for positive comparisons, to avoid NaN values
    if (frame.channelZeroVisibility >= 0.5)
        emit setChannelZeroVisibility(true);
    else if (frame.channelZeroVisibility < 0.5)
        emit setChannelZeroVisibility(false);
    //
    if (frame.channelOneVisibility >= 0.5)
        emit setChannelOneVisibility(true);
    else if (frame.channelOneVisibility < 0.5)
        emit setChannelOneVisibility(false);
    //
    if (frame.channelTwoVisibility >= 0.5)
        emit setChannelTwoVisibility(true);
    else if (frame.channelTwoVisibility < 0.5)
        emit setChannelTwoVisibility(false);
    //
    if (frame.channelThreeVisibility >= 0.5)
        emit setChannelThreeVisibility(true);
    else if (frame.channelThreeVisibility < 0.5)
        emit setChannelThreeVisibility(false);

    ui->v3dr_glwidget->update();
    QApplication::processEvents();
}

void NaMainWindow::on_actionSave_movie_frames_triggered() {
    // Get output folder
    static QString startFolder;
    QString sf;
    if (! startFolder.isEmpty())
        sf = startFolder;
    QString folderName = QFileDialog::getExistingDirectory(this,
        tr("Choose folder to store movie frame images"),
        sf);
    if (folderName.isEmpty())
        return;
    QDir folder(folderName);
    if (! folder.exists()) {
        QMessageBox::warning(this, "No such folder", "Folder " +folderName+ " does not exist");
        return;
    }

    currentMovie.rewind();
    double secondsElapsed = 0.0;
    int frameIndex = 0;
    int savedCount = 0;
    while (currentMovie.hasMoreFrames()) {
        animateToFrame(currentMovie.getNextFrame());
#if defined(USE_Qt5)
        QImage grabbedFrame = ui->v3dr_glwidget->grabFramebuffer(); 
#else
        QImage grabbedFrame = ui->v3dr_glwidget->grabFrameBuffer(true); // true->with alpha
#endif
        frameIndex += 1;
        QString fileName;
        fileName.sprintf("frame_%05d.png", frameIndex);
        fileName = folder.absoluteFilePath(fileName);
        qDebug() << fileName;
        if (grabbedFrame.save(fileName)) {
            savedCount += 1;
        }
        else {
            // TODO - better error handling
            qDebug() << "Failed to save frame" << fileName;
        }
    }
    QMessageBox::information(this, "Finished saving frames", QString("Saved %1 frames").arg(savedCount));
    // Remember this folder next time
    startFolder = folderName;
}

void NaMainWindow::on_zThicknessDoubleSpinBox_valueChanged(double val) {
    ui->v3dr_glwidget->setThickness(val);
}

static bool isFolder(QString path) {
    if (path.isEmpty())
        return false;
    if (path.endsWith("/"))
        return true;
    if (QFileInfo(path).suffix().isEmpty())
        return true;
    return false;
}

void NaMainWindow::openFileOrUrl(QString name)
{
    // qDebug() << "NaMainWindow::openFileOrUrl" << name << __FILE__ << __LINE__;
    if (name.isEmpty())
        return;

    QUrl url(name);
    if (! url.isValid())
        url = QUrl::fromLocalFile(name);
    bool isDir = true;
    if (url.isValid() && (!url.isRelative()) && url.toLocalFile().isEmpty())
        isDir = isFolder(url.path()); // non-file URL
    else
        isDir = QFileInfo(url.toLocalFile()).isDir(); // local file
    if (isDir)
        openMulticolorImageStack(url);
    else
        loadSingleStack(url);
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
QUrl checkDragEvent(QDropEvent* event)
{
    QList<QUrl> urls;
    if (event->mimeData()->hasUrls())
        urls = event->mimeData()->urls();
    // Maybe the user dragged a string with a filename or url
    else if (event->mimeData()->hasText()) {
        QString text = event->mimeData()->text();
        QUrl url(text);
        if (url.isValid() && ! url.isEmpty())
                urls.append(url);
        else {
            if (QFileInfo(text).exists())
                urls.append(QUrl::fromLocalFile(text));
        }
    }

    if (urls.isEmpty())
        return QUrl();

    /* Switch to use URLs, not files Jan 2013
    QString fileName = urls.first().toLocalFile();
    if (fileName.isEmpty())
        return "";
        */
    QUrl url = urls.first();
    if (url.isEmpty())
        return QUrl();
    QString urlPath = url.path();
    if (url.host() == "")
        url.setHost("localhost");

    // check for recognized file extensions
    QString fileExtension = QFileInfo(urlPath).suffix().toLower();
    if (fileExtension == "lsm")
        return url;
    if (fileExtension.startsWith("tif")) // tif or tiff
        return url;
    if (fileExtension.startsWith("v3d")) // v3draw or v3dpdb
        return url;
#ifdef USE_FFMPEG
    if (fileExtension.startsWith("mp4")) // v3draw or v3dpdb
        return url;
#endif
    bool isDir = isFolder(url.path());
    if (isDir)
        return url; // neuron separation folder

    return QUrl();
}

void NaMainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    QUrl url = checkDragEvent(event);
    if (! url.isEmpty())
        event->acceptProposedAction();
   // qDebug() << "NaMainWindow::dragEnterEvent" << fileName << __FILE__ << __LINE__;
}

void NaMainWindow::dropEvent(QDropEvent * event)
{
    QUrl url = checkDragEvent(event);
    if (url.isEmpty()) return;

    openFileOrUrl(url.toString());
}

void NaMainWindow::moveEvent ( QMoveEvent * event )
{
    // qDebug() << "NaMainWindow::moveEvent()" << __FILE__ << __LINE__;
    ui->v3dr_glwidget->updateScreenPosition();
    QMainWindow::moveEvent(event);
}

void NaMainWindow::loadSingleStack(QUrl url)
{
    // Default to NeuronAnnotator, not Vaa3D classic, when URL is given
    loadSingleStack(url, false);
}

/* slot */
void NaMainWindow::loadSingleStack(Entity* entity)
{
    QString filepath = entity->getValueByAttributeName("File Path");
    channel_spec = entity->getValueByAttributeName("Channel Specification");
    if (channel_spec.isEmpty()) {
        channel_spec = QString();
    }

    loadSingleStack( filepath );
}

void NaMainWindow::loadSingleStack(QString fileName)
{
    QUrl url( fileName );
    bool local = url.scheme().isEmpty();

    if ( local )
    {
        QUrl local_url = QUrl::fromLocalFile( fileName );
        loadSingleStack( local_url, true ); // default to classic mode
    }
    else
        loadSingleStack( url, false ); // default to classic mode
}

/* slot */
void NaMainWindow::loadSingleStack(QUrl url, bool useVaa3dClassic)
{
    if (url.isEmpty())
        return;
    if (! url.isValid())
        return;

    mainWindowStopWatch.start();
    if (useVaa3dClassic) {
        // Open in Vaa3D classic mode
        ui->actionV3DDefault->trigger(); // switch mode
        QString fileName = url.toLocalFile();
        if (! fileName.isEmpty())
            emit defaultVaa3dFileLoadRequested(fileName);
        else
            emit defaultVaa3dUrlLoadRequested(url);
    }
    else
    {
        setViewMode(VIEW_SINGLE_STACK);
        onDataLoadStarted();
        createNewDataFlowModel();

        // All of these may be empty, but under certain cases (for Janelia)
        // the channel_spec will be valid
        dataFlowModel->getVolumeData().setAuxillaryImagery(losslessImage, visuallyLosslessImage, channel_spec);

        VolumeTexture& volumeTexture = dataFlowModel->getVolumeTexture();
        volumeTexture.queueVolumeData();

        QString baseName = QFileInfo(url.path()).fileName();
        setTitle(baseName);

        emit singleStackLoadRequested(url);
        addUrlToRecentFilesList(url);
    }
}


///////////////////////////////////
// User clip planes in 3D viewer //
///////////////////////////////////

void NaMainWindow::connectCustomCut()
{
    connect(ui->customCutButton, SIGNAL(pressed()),
            this, SLOT(applyCustomCut()));
    connect(ui->defineClipPlaneButton, SIGNAL(pressed()),
            this, SLOT(toggleCustomCutMode()));
}

/* slot */
void NaMainWindow::applyCustomCut()
{
    // assert(isInCustomCutMode);
    ui->v3dr_glwidget->applyCustomCut();
    if (isInCustomCutMode)
        toggleCustomCutMode();
}

/* slot */
void NaMainWindow::applyCustomKeepPlane()
{
    // qDebug() << "NaMainWindow::applyCustomKeepPlane()" << __FILE__ << __LINE__;
    // assert(isInCustomCutMode);
    ui->v3dr_glwidget->applyCustomKeepPlane();
    if (isInCustomCutMode)
        toggleCustomCutMode();
}

/* slot */
void NaMainWindow::setCustomCutMode(bool doCustom)
{
    if (doCustom)
    {
        // Activate custom cut mode
        ui->defineClipPlaneButton->setText(tr("Cancel"));
        ui->customCutButton->setEnabled(true);
        ui->v3dr_glwidget->setCustomCutMode();
    }
    else
    {
        // Turn off custom cut mode
        ui->defineClipPlaneButton->setText(tr("Custom..."));
        ui->customCutButton->setEnabled(false);
        ui->v3dr_glwidget->cancelCustomCutMode();
    }
    isInCustomCutMode = doCustom;
}

/* slot */
void NaMainWindow::toggleCustomCutMode()
{
    setCustomCutMode(!isInCustomCutMode);
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
    ui->viewerControlTabWidget->setEnabled(false);
    ViewerIndex currentIndex = (ViewerIndex)ui->viewerStackedWidget->currentIndex();
    if (currentIndex != VIEWER_WAIT_LOADING_SCREEN)
        recentViewer = currentIndex;
    ui->viewerStackedWidget->setCurrentIndex(VIEWER_WAIT_LOADING_SCREEN);
    update();
}

void NaMainWindow::onDataLoadFinished()
{
    if (undoStack)
        undoStack->clear();
    ui->viewerStackedWidget->setCurrentIndex(recentViewer);
    ui->viewerControlTabWidget->setEnabled(true);
    // qDebug() << "Data load took" << mainWindowStopWatch.elapsed()/1000.0 << "seconds";
    update();
}

void NaMainWindow::initializeStereo3DOptions()
{
    // Only check one stereo format at a time
    QActionGroup* stereoModeGroup = new QActionGroup(this);
    stereoModeGroup->setExclusive(true);
    stereoModeGroup->addAction(ui->actionMono_Off);
    stereoModeGroup->addAction(ui->actionLeft_eye_view);
    stereoModeGroup->addAction(ui->actionRight_eye_view);
    stereoModeGroup->addAction(ui->actionQuadro_120_Hz);
    stereoModeGroup->addAction(ui->actionAnaglyph_Red_Cyan);
    stereoModeGroup->addAction(ui->actionAnaglyph_Green_Magenta);
    stereoModeGroup->addAction(ui->actionRow_Interleaved_Zalman);
    stereoModeGroup->addAction(ui->actionChecker_Interleaved_3DTV);
    stereoModeGroup->addAction(ui->actionColumn_Interleaved);

    connect(ui->actionMono_Off, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setStereoOff(bool)));
    connect(ui->actionLeft_eye_view, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setStereoLeftEye(bool)));
    connect(ui->actionRight_eye_view, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setStereoRightEye(bool)));
    connect(ui->actionQuadro_120_Hz, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setStereoQuadBuffered(bool)));
    connect(ui->actionAnaglyph_Red_Cyan, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setStereoAnaglyphRedCyan(bool)));
    connect(ui->actionAnaglyph_Green_Magenta, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setStereoAnaglyphGreenMagenta(bool)));
    connect(ui->actionRow_Interleaved_Zalman, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setStereoRowInterleaved(bool)));
    connect(ui->actionChecker_Interleaved_3DTV, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setStereoCheckerInterleaved(bool)));
    connect(ui->actionColumn_Interleaved, SIGNAL(toggled(bool)),
            ui->v3dr_glwidget, SLOT(setStereoColumnInterleaved(bool)));

    connect(ui->v3dr_glwidget, SIGNAL(quadStereoSupported(bool)),
            this, SLOT(supportQuadStereo(bool)));
}

/* slot */
void NaMainWindow::supportQuadStereo(bool b)
{
    ui->actionQuadro_120_Hz->setEnabled(b);
    if ( (!b) && ui->actionQuadro_120_Hz->isChecked() )
        ui->actionMono_Off->setChecked(true);
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
    showAllNeuronsInEmptySpaceAction = ui->actionShow_all_neurons_in_empty_space;
    hideAllAction = ui->actionClear_Hide_All;
    selectNoneAction = ui->actionSelect_None;
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
    ui->naLargeMIPWidget->setContextMenus(viewerContextMenu, neuronContextMenu);
    ui->naZStackWidget->setContextMenus(viewerContextMenu, neuronContextMenu);
    ui->v3dr_glwidget->setContextMenus(viewerContextMenu, neuronContextMenu);
}

/* slot */
void NaMainWindow::onHdrChannelChanged(NaZStackWidget::Color channel)
{
    switch(channel)
    {
    // Due to exclusive group, checking one button unchecks the others.
    case NaZStackWidget::COLOR_RED:
        ui->HDRRed_pushButton->setChecked(true);
        break;
    case NaZStackWidget::COLOR_GREEN:
        ui->HDRGreen_pushButton->setChecked(true);
        break;
    case NaZStackWidget::COLOR_BLUE:
        ui->HDRBlue_pushButton->setChecked(true);
        break;
    case NaZStackWidget::COLOR_NC82:
        ui->HDRNc82_pushButton->setChecked(true);
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
        ui->redToggleButton->setChecked(colorReader.getChannelVisibility(0));
        ui->greenToggleButton->setChecked(colorReader.getChannelVisibility(1));
        ui->blueToggleButton->setChecked(colorReader.getChannelVisibility(2));

        // Gamma
        ui->sharedGammaWidget->setGammaBrightness(colorReader.getSharedGamma());
        const int refIndex = 3;
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (volumeReader.hasReadLock() && volumeReader.hasReferenceImage())
            ui->referenceGammaWidget->setGammaBrightness(colorReader.getChannelGamma(refIndex));

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
    ui->statusbar->showMessage(msg);
}

void NaMainWindow::setRotation(Rotation3D r) {
    sharedCameraModel.setRotation(r);
    ui->v3dr_glwidget->update();
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
    CameraModel& cam = ui->v3dr_glwidget->cameraModel;
    if (!ui->v3dr_glwidget->mouseIsDragging()) {
        cam.setRotation(R * cam.rotation());
        // TODO - use a signal here instead of processEvents
        QCoreApplication::processEvents(); // keep responsive during nutation
        ui->v3dr_glwidget->update();
    }
}

void NaMainWindow::resetView()
{
    // TODO - might not work if cameras are not linked
    Vector3D newFocus = ui->v3dr_glwidget->getDefaultFocus();
    // cerr << newFocus << __LINE__ << __FILE__;
    sharedCameraModel.setFocus(newFocus);
    sharedCameraModel.setRotation(Rotation3D()); // identity rotation
    sharedCameraModel.setScale(1.0); // fit to window
    ui->viewerStackedWidget->update(); // whichever viewer is shown
}

void NaMainWindow::updateViewers()
{
    ui->naLargeMIPWidget->update();
    ui->naZStackWidget->update();
    ui->v3dr_glwidget->update();
}

void NaMainWindow::unifyCameras(bool bDoUnify)
{
    // TODO - explicitly copy parameters from active displayed viewer
    if (bDoUnify) {
        ui->naLargeMIPWidget->synchronizeWithCameraModel(&sharedCameraModel);
        ui->naZStackWidget->synchronizeWithCameraModel(&sharedCameraModel);
        ui->v3dr_glwidget->synchronizeWithCameraModel(&sharedCameraModel);
        // qDebug() << "unify cameras";
    }
    else {
        ui->naLargeMIPWidget->decoupleCameraModel(&sharedCameraModel);
        ui->naZStackWidget->decoupleCameraModel(&sharedCameraModel);
        ui->v3dr_glwidget->decoupleCameraModel(&sharedCameraModel);
        // qDebug() << "disband cameras";
    }
}

void NaMainWindow::setZRange(int minZ, int maxZ) {
    // qDebug() << "minZ = " << minZ << "; maxZ = " << maxZ;
    QString text = QString("of %1").arg(maxZ);
    // qDebug() << text;
    ui->ZSliceTotal_label->setText(text);
    ui->ZSlice_horizontalScrollBar->setMaximum(maxZ);
    ui->ZSlice_spinBox->setMaximum(maxZ);
    ui->ZSlice_horizontalScrollBar->setMinimum(minZ);
    ui->ZSlice_spinBox->setMinimum(minZ);
}

void NaMainWindow::handleCoordinatedCloseEvent(QCloseEvent *e)
{
    if (isVisible())
    {
        // Remember window size for next time.
        // These settings affect both NaMainWindow and classic V3D MainWindows.  So only use
        // NaMainWindow settings if the NaMainWindow is visible.
       // qDebug() << "Saving NaMainWindow size and position";
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
    QAction* ui_actionV3DDefault = this->findChild<QAction*>("actionV3DDefault");
    ui_actionV3DDefault->setChecked(checkState);
}

void NaMainWindow::setNeuronAnnotatorModeCheck(bool checkState) {
    QAction* ui_actionNeuronAnnotator = this->findChild<QAction*>("actionNeuronAnnotator");
    ui_actionNeuronAnnotator->setChecked(checkState);
}

void NaMainWindow::on_actionOpen_microCT_Cut_Planner_triggered()
{
    if (cutPlanner == NULL) {
        cutPlanner = new CutPlanner(sharedCameraModel, *ui->v3dr_glwidget, this);
        connect(cutPlanner, SIGNAL(rotationAdjusted(Rotation3D)),
                this, SLOT(setRotation(Rotation3D)));
        connect(cutPlanner, SIGNAL(clipPlaneRequested()),
                this, SLOT(applyCustomCut()));
        connect(cutPlanner, SIGNAL(keepPlaneRequested()),
                this, SLOT(applyCustomKeepPlane()));
        connect(cutPlanner, SIGNAL(cutGuideRequested(bool)),
                this, SLOT(setCustomCutMode(bool)));
        connect(cutPlanner, SIGNAL(compartmentNamingRequested()),
                this, SLOT(labelNeuronsAsFlyBrainCompartments()));
    }
    setCustomCutMode(true);
    cutPlanner->show();
}

void NaMainWindow::on_actionOpen_triggered()
{
    QString dirName = getDataDirectoryPathWithDialog();
    openMulticolorImageStack(dirName);
}

bool NaMainWindow::openMulticolorImageStack(QString dirName)
{
    // qDebug() << "NaMainWindow::openMulticolorImageStack" << dirName << __FILE__ << __LINE__;
    // string could be a folder name or a URL string

    // Try for folder name
    QDir imageDir(dirName);
    if (imageDir.exists()) {
        QUrl url = QUrl::fromLocalFile(imageDir.absolutePath());
        return openMulticolorImageStack(url);
    }

    // Path is always a folder, so make it explicit
    if (! dirName.endsWith("/"))
        dirName = dirName + "/";

    // That didn't work: try for a URL
    QUrl url(dirName);
    if (! url.isValid()) {
        QMessageBox::warning(this, tr("No such directory or URL"),
                             QString("'%1'\n No such directory or URL.\nIs the file share mounted?\nHas the directory moved?").arg(dirName));
        return false;
    }

    // qDebug() << url;

    bool result = openMulticolorImageStack(url);
    if (! result) {
        QMessageBox::warning(this, tr("Error opening directory or URL"),
                             QString("'%1'\n Could not open directory or URL.\nIs the file share mounted?\nHas the directory moved?").arg(dirName));

    }

    return result;
}

bool NaMainWindow::openMulticolorImageStack(QUrl url)
{
    // qDebug() << "NaMainWindow::openMulticolorImageStack" << url << __FILE__ << __LINE__;
    mainWindowStopWatch.start();
    // std::cout << "Selected directory=" << imageDir.absolutePath().toStdString() << endl;
    emit benchmarkTimerResetRequested();
    emit benchmarkTimerPrintRequested("openMulticolorImageStack called");

    if (! tearDownOldDataFlowModel()) {
        QMessageBox::warning(this, tr("Could not close previous Annotation Session"),
                     "Error saving previous session and/or clearing memory - please exit application");
                 return false;
    }

    // Try to avoid Dec 2013 crash
    ui->v3dr_glwidget->clearImage();

    createNewDataFlowModel();
    // reset front/back clip slab
    ui->v3dr_glwidget->resetSlabThickness();
    emit initializeColorModelRequested();

    VolumeTexture& volumeTexture = dataFlowModel->getVolumeTexture();

    // Queue up the various volumes to load, using StageFileLoaders
    // delegated to VolumeTexture (3D viewer) and NaVolumeTexture (all viewers)
    onDataLoadStarted();
    if (! volumeTexture.queueSeparationFolder(url))
    {
        onDataLoadFinished();
        return false;
    }
    // Make sure 3D viewer is showing if fast loading is enabled
    if(volumeTexture.hasFastVolumesQueued()) {
        // Fast loading is only interesting if 3D viewer is selected.
        // So show the 3D viewer
        ui->viewerControlTabWidget->setCurrentIndex(2);
        setViewMode(VIEW_SINGLE_STACK); // no gallery yet.
    }

    // MulticolorImageStackNode setup is required for loadLsmMetadata call to succeed.
    MultiColorImageStackNode* multiColorImageStackNode =
            new MultiColorImageStackNode(url.toString());
    QUrl fileUrl = url;
    QString path = fileUrl.path();
    if (! path.endsWith("/"))
        path = path + "/";
    // These file names will be overridden by Staged loader
    fileUrl.setPath(path + "ConsolidatedSignal");
    multiColorImageStackNode->setPathToOriginalImageStackFile(fileUrl.toString());
    fileUrl.setPath(path + "Reference");
    multiColorImageStackNode->setPathToReferenceStackFile(fileUrl.toString());
    fileUrl.setPath(path + "ConsolidatedLabel");
    multiColorImageStackNode->setPathToMulticolorLabelMaskFile(fileUrl.toString());
    dataFlowModel->setMultiColorImageStackNode(multiColorImageStackNode);
    dataFlowModel->getVolumeData().setAuxillaryImagery(losslessImage, visuallyLosslessImage, channel_spec);
    if (! dataFlowModel->getVolumeData().queueSeparationFolder(url)) {
        onDataLoadFinished();
        return false;
    }

    // Correct Z-thickness
    dataFlowModel->loadLsmMetadata();

    // Kick off loading sequence
    emit stagedLoadRequested();
    // volumeTexture.loadNextVolume();

    addUrlToRecentFilesList(url);
    return true;
}

bool NaMainWindow::loadSeparationDirectoryV1Pbd(QUrl imageInputDirectory)
{
    onDataLoadStarted();
    // Need to construct (temporary until backend implemented) MultiColorImageStackNode from this directory
    // This code will be redone when the node/filestore is implemented.
    QUrl originalImageStackFilePath =
            appendPath(imageInputDirectory, MultiColorImageStackNode::IMAGE_STACK_BASE_FILENAME);
    QUrl maskLabelFilePath =
            appendPath(imageInputDirectory, MultiColorImageStackNode::IMAGE_MASK_BASE_FILENAME);
    QUrl referenceStackFilePath =
            appendPath(imageInputDirectory, MultiColorImageStackNode::IMAGE_REFERENCE_BASE_FILENAME);

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

    // Opposite of fast loading behavior
    dataFlowModel->getVolumeData().doFlipY_mask = true;
    dataFlowModel->getVolumeData().doFlipY_image = true;
    dataFlowModel->getVolumeData().bDoUpdateSignalTexture = true;
    // fooDebug() << __FILE__ << __LINE__;

    // Load session
    setViewMode(VIEW_NEURON_SEPARATION);
    if (! dataFlowModel->loadVolumeData()) return false;
    // dataChanged() signal will be emitted if load succeeds

    return true;
}

// Recent files list
void NaMainWindow::addDirToRecentFilesList(QDir imageDir)
{
    QString fileName = imageDir.absolutePath();
    addFileNameToRecentFilesList(QUrl::fromLocalFile(fileName).toString());
}

void NaMainWindow::addUrlToRecentFilesList(QUrl url)
{
    addFileNameToRecentFilesList(url.toString());
}

void NaMainWindow::addFileNameToRecentFilesList(QString fileName)
{
    // fooDebug() << fileName << __FILE__ << __LINE__;
    if (fileName.isEmpty()) return;
    QSettings settings(QSettings::UserScope, "HHMI", "Vaa3D");
    QVariant filesVariant = settings.value("NeuronAnnotatorRecentFileList");
    QStringList files = filesVariant.toStringList();
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
    ui->menuOpen_Recent->setEnabled(files.size() > 0);
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

void expressRegretsAboutVolumeWriting(QString message) {
	QMessageBox::warning(NULL, "Volume export failed",
			message);
}

void NaMainWindow::on_action3D_Volume_triggered()
{
    if (! dataFlowModel) {
        expressRegretsAboutVolumeWriting("No data available to save");
        return;
    }
    QString suggestedFile;
    {
    	    NeuronSelectionModel::Reader selectionReader(dataFlowModel->getNeuronSelectionModel());
    	    if (! selectionReader.hasReadLock()) {
    	    	    expressRegretsAboutVolumeWriting("Could not access selection data");
    	    	    return;
    	    }
        suggestedFile=suggestedExportFilenameFromCurrentState(selectionReader);
    }
    QString fileTypes = "*.v3dpbd *.v3draw *.tif";
#ifdef USE_FFMPEG
    fileTypes += " *.mp4";
#endif
    fileTypes = "3D Volumes ("+fileTypes+")";
    QString filename = QFileDialog::getSaveFileName(
    		0,
    		QObject::tr("Save 3D Volume to a file"),
    		suggestedFile,
    		QObject::tr(fileTypes.toStdString().c_str()));
    if (filename.isEmpty())
    	    return; // user pressed "Cancel"
    QFileInfo fi(filename);
    if (fi.suffix().isEmpty())
    	    filename = filename + ".v3dpbd";
    fooDebug() << filename;
	ExportFile *pExport = new ExportFile(
			filename,
			dataFlowModel->getVolumeData(),
			dataFlowModel->getNeuronSelectionModel(),
            dataFlowModel->getDataColorModel(),
            sharedCameraModel);
	connect(pExport, SIGNAL(finished()), pExport, SLOT(deleteLater()));
	connect(pExport, SIGNAL(exportFinished(QString)),
			this, SLOT(onExportFinished(QString)));
	connect(pExport, SIGNAL(exportFailed(QString, QString)),
			this, SLOT(onExportFinished(QString, QString)));
	pExport->start();
}

/* slot */
void NaMainWindow::onExportFinished(QString fileName) {
	QMessageBox::information(this, "Volume export succeeded",
			"Saved file " + fileName);
}

/* slot */
void NaMainWindow::onExportFailed(QString fileName, QString message) {
	QMessageBox::warning(this, "Volume export failed",
			message + ": " + fileName);
}

void NaMainWindow::on_action2D_MIP_triggered() {
    QString filename = QFileDialog::getSaveFileName(0, QObject::tr("Save 2D MIP to an .tif file"), ".", QObject::tr("2D MIP (*.tif)"));
    if (!(filename.isEmpty())){

        // bool saved = ui->naLargeMIPWidget->saveImage(filename); // REPLACING WITH 3D MIP USING ROTATION and CUT PLANES

        ExportFile *pExport = new ExportFile(
                filename,
                dataFlowModel->getVolumeData(),
                dataFlowModel->getNeuronSelectionModel(),
                dataFlowModel->getDataColorModel(),
                sharedCameraModel,
                true /* is2D */);
        connect(pExport, SIGNAL(finished()), pExport, SLOT(deleteLater()));
        connect(pExport, SIGNAL(exportFinished(QString)),
                this, SLOT(onExportFinished(QString)));
        connect(pExport, SIGNAL(exportFailed(QString, QString)),
                this, SLOT(onExportFinished(QString, QString)));
        pExport->start();
    }
}

void NaMainWindow::on_actionScreenShot_triggered() {
    static QString dirname = ".";
    QString filename = QFileDialog::getSaveFileName(
        ui->v3dr_glwidget,
        QObject::tr("Save 3D View to an image file"),
        dirname,
        QObject::tr("Images (*.tif *.png *.jpg *.ppm *.xpm)"));
    if (filename.isEmpty())
        return; // User cancelled
    bool saved = ui->v3dr_glwidget->screenShot(filename);
    if (saved) {
        QMessageBox::information(ui->v3dr_glwidget,
            "Successfully saved screen shot",
            "Successfully saved screen shot to file " + filename);
        // Remember this directory next time
        // TODO - use QSettings for more persistent memory
        dirname = filename;
    }
    else {
        QMessageBox::critical(this,
            "Failed to save screen shot",
            "Failed to save screen shot to file " + filename
            + " \nDo you have write permission in that folder?"
            + " \nMaybe a different image format would work better?");
    }
}

void NaMainWindow::on_actionPreferences_triggered()
{
    PreferencesDialog dlg(this);
    dlg.loadPreferences();
    int result = dlg.exec();
    if (result == QDialog::Accepted) {
        dlg.savePreferences();
    }
}

void NaMainWindow::on_actionX_Rotation_Movie_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(
            this, tr("Save movie frame images"),
            "",
            tr("Images (*.png *.jpg *.ppm)"));
    if (fileName.isEmpty())
        return;
    QFileInfo fi(fileName);
    QDir dir = fi.absoluteDir();
    QString base = fi.completeBaseName();
    QString suffix = fi.suffix();
    int frameCount = 540;
    Rotation3D dRot;
    dRot.setRotationFromAngleAboutUnitVector(
            2.0 * 3.14159 / frameCount,
            UnitVector3D(1, 0, 0));
    Rotation3D currentRotation = sharedCameraModel.rotation();
    ui->v3dr_glwidget->resize(1280, 720);
    for (int f = 0; f < frameCount; ++f)
    {
        QString fnum = QString("_%1.").arg(f, 5, 10, QChar('0'));
        QString fName = dir.absoluteFilePath(base + fnum + suffix);
        fooDebug() << fName;
        currentRotation = dRot * currentRotation;
        sharedCameraModel.setRotation(currentRotation);
        ui->v3dr_glwidget->repaint();
        QCoreApplication::processEvents();
#if defined(USE_Qt5)
        QImage frameImage = ui->v3dr_glwidget->grabFramebuffer();
#else
        QImage frameImage = ui->v3dr_glwidget->grabFrameBuffer();
#endif
        frameImage.save(fName, NULL, 95);
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
    ui->v3dr_glwidget->setDataFlowModel(dataFlowModel);
    ui->naLargeMIPWidget->setDataFlowModel(dataFlowModel);
    ui->naZStackWidget->setDataFlowModel(dataFlowModel);
    ui->fragmentGalleryWidget->setDataFlowModel(dataFlowModel);
    neuronSelector.setDataFlowModel(dataFlowModel);

    // was in loadAnnotationSessionFromDirectory June 27, 2012
    if (dynamicRangeTool) {
        if (NULL == dataFlowModel)
            dynamicRangeTool->setColorModel(NULL);
        else
            dynamicRangeTool->setColorModel(&dataFlowModel->getDataColorModel());
    }

    // No connecting if the model is NULL
    if (NULL == dataFlowModel)
    {
        ui->naLargeMIPWidget->setMipMergedData(NULL);
        return;
    }

    connect(dataFlowModel, SIGNAL(zRatioChanged(double)),
            ui->zThicknessDoubleSpinBox, SLOT(setValue(double)));

    connect(this, SIGNAL(subsampleLabelPbdFileNamed(QUrl)),
            &dataFlowModel->getVolumeTexture(), SLOT(setLabelPbdFileUrl(QUrl)));

    connect(dataFlowModel, SIGNAL(benchmarkTimerPrintRequested(QString)),
            this, SIGNAL(benchmarkTimerPrintRequested(QString)));
    connect(dataFlowModel, SIGNAL(benchmarkTimerResetRequested()),
            this, SIGNAL(benchmarkTimerResetRequested()));

    // Connect mip viewer to data flow model
    ui->naLargeMIPWidget->setMipMergedData(&dataFlowModel->getMipMergedData());

    connectContextMenus(dataFlowModel->getNeuronSelectionModel());

    connect(dataFlowModel, SIGNAL(scrollBarFocus(NeuronSelectionModel::NeuronIndex)),
            ui->fragmentGalleryWidget, SLOT(scrollToFragment(NeuronSelectionModel::NeuronIndex)));

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

    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(selectionCleared()),
            ui->annotationFrame, SLOT(deselectNeurons()));
    connect(ui->annotationFrame, SIGNAL(neuronSelected(int)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(selectExactlyOneNeuron(int)));
    connect(ui->annotationFrame, SIGNAL(neuronsDeselected()),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(clearSelection()));
    connect(this, SIGNAL(initializeSelectionModelRequested()),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(initializeSelectionModel()));
    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(visibilityChanged()),
            this, SLOT(onSelectionModelVisibilityChanged()));

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
    connect(this, SIGNAL(singleStackLoadRequested(QUrl)),
            &dataFlowModel->getVolumeData(), SLOT(loadChannels(QUrl)));
    connect(&dataFlowModel->getVolumeData(), SIGNAL(channelsLoaded(int)),
            this, SLOT(onDataLoadFinished()));

    // Loading a series of separation result stacks
    connect(this, SIGNAL(stagedLoadRequested()),
            &dataFlowModel->getVolumeTexture(), SLOT(loadStagedVolumes()));

    // Color toggling
    connect(this, SIGNAL(channelVisibilityChanged(int,bool)),
            &dataFlowModel->getDataColorModel(), SLOT(setChannelVisibility(int,bool)));
    connect(ui->resetColorsButton, SIGNAL(clicked()),
            &dataFlowModel->getDataColorModel(), SLOT(resetColors()));
    connect(ui->sharedGammaWidget, SIGNAL(gammaBrightnessChanged(qreal)),
            &dataFlowModel->getDataColorModel(), SLOT(setSharedGamma(qreal)));
    connect(ui->referenceGammaWidget, SIGNAL(gammaBrightnessChanged(qreal)),
            &dataFlowModel->getDataColorModel(), SLOT(setReferenceGamma(qreal)));
    connect(&dataFlowModel->getDataColorModel(), SIGNAL(dataChanged()),
            this, SLOT(onColorModelChanged()));
    connect(ui->naZStackWidget, SIGNAL(hdrRangeChanged(int,qreal,qreal)),
            &dataFlowModel->getDataColorModel(), SLOT(setChannelHdrRange(int,qreal,qreal)));
    connect(this, SIGNAL(initializeColorModelRequested()),
            &dataFlowModel->getDataColorModel(), SLOT(resetColors()));
}

bool NaMainWindow::tearDownOldDataFlowModel()
{
    ui->v3dr_glwidget->clearImage();
    if (NULL == dataFlowModel)
        return true;

    // TODO - orderly shut down of old data flow model
    DataFlowModel* dfm = dataFlowModel; // save pointer
    // TODO - make sure clients respect setting to null
    // TODO - make sure this does not yet delete dataFlowModel
    setDataFlowModel(NULL);
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
    ui->v3dr_glwidget->invalidate();
    ui->naZStackWidget->invalidate();
    ui->naLargeMIPWidget->invalidate();
    ui->v3dr_glwidget->clearImage();
    ui->v3dr_glwidget->initializeDefaultTextures(); // <- this is how to reset the label texture
    return true;
}


void NaMainWindow::setTitle(QString title) {
    setWindowTitle(QString("%1 - Vaa3D Neuron Annotator").arg(title));
}

/* slot */
void NaMainWindow::processUpdatedVolumeData() // activated by volumeData::dataChanged() signal
{
    onDataLoadFinished();
    // TODO -- install separate listeners for dataChanged() in the various display widgets

    dataFlowModel->loadLsmMetadata();

    int img_sc, img_sz, ref_sc;
    {
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (! volumeReader.hasReadLock()) return;
        const Image4DProxy<My4DImage>& imgProxy = volumeReader.getOriginalImageProxy();
        const Image4DProxy<My4DImage>& refProxy = volumeReader.getReferenceImageProxy();
        img_sc = imgProxy.sc;
        img_sz = imgProxy.sz;
        ref_sc = refProxy.sc;
    } // release read locks

    setZRange(0, img_sz - 1);

    // Ensure z-stack viewer gets enabled
    dataFlowModel->getZSliceColors().onCameraFocusChanged(sharedCameraModel.focus());

    // Start in middle of volume
    // No, initial position should be set in 3D viewer
    // ui->naZStackWidget->setCurrentZSlice(img_sz / 2 + 1);

    // Need at least two colors for use of the color buttons to make sense
    ui->HDRRed_pushButton->setEnabled(img_sc > 1);
    ui->HDRGreen_pushButton->setEnabled(img_sc > 1);
    ui->HDRBlue_pushButton->setEnabled(img_sc > 2);
    ui->HDRNc82_pushButton->setEnabled(ref_sc > 0);

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
            mx = (int)size.x() - 1;
            my = (int)size.y() - 1;
            mz = (int)size.z() - 1;
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
    ui->XcminSlider->setRange(0, mx);
    ui->XcminSlider->setValue(0);
    ui->XcmaxSlider->setRange(0, mx);
    ui->XcmaxSlider->setValue(mx);

    ui->YcminSlider->setRange(0, my);
    ui->YcminSlider->setValue(0);
    ui->YcmaxSlider->setRange(0, my);
    ui->YcmaxSlider->setValue(my);

    ui->ZcminSlider->setRange(0, my);
    ui->ZcminSlider->setValue(0);
    ui->ZcmaxSlider->setRange(0, my);
    ui->ZcmaxSlider->setValue(my);

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
    // ui->mipsFrame->setVisible(bShowGalleries);
    if (bShowGalleries)
        setViewMode(VIEW_NEURON_SEPARATION);
}

void NaMainWindow::initializeOverlayGallery()
{
    // qDebug() << "NaMainWindow::initializeOverlayGallery()" << __FILE__ << __LINE__;

    // Create layout, only if needed.
    QFrame* ui_maskFrame = this->findChild<QFrame*>("maskFrame");
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
        ui->fragmentGalleryWidget->clear(); // deletes buttons

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
            ui->fragmentGalleryWidget->appendFragment(button);
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
    ui->fragmentGalleryWidget->updateButtonsGeometry();
}

// For microCT mode
/* slot */
void NaMainWindow::labelNeuronsAsFlyBrainCompartments() {
    // If a second renaming scheme is ever needed, refactor this to load names from an
    // external data source.
    QList<QString> compartmentNames;
    compartmentNames << "FB (Fan-shaped Body)";
    compartmentNames << "EB (Ellipsoid Body)";
    compartmentNames << "SAD (Saddle)";
    compartmentNames << "NO (Noduli)";
    compartmentNames << "SOG (Suboesophageal Ganglion)";
    compartmentNames << "PB (Protocerebral Bridge)";
    compartmentNames << "CRE_R (Crepine)";
    compartmentNames << "EPA_R (Epaulette)";
    compartmentNames << "VES_R (Vesta)";
    compartmentNames << "ATL_R (Antler)";
    compartmentNames << "PLP_R (Posterior Lateral Protocerebrum)";
    compartmentNames << "AVLP_R (Anterior Ventro-lateral protocerebrum)";
    compartmentNames << "AL_R (Antennal Lobe)";
    compartmentNames << "GOR_R (Gorget)";
    compartmentNames << "SCL_R (Superior Clamp)";
    compartmentNames << "FLA (Flange)";
    compartmentNames << "ICL_R (Inferior Clamp)";
    compartmentNames << "ME_R (Medulla)";
    compartmentNames << "LOP_R (Lobula Plate)";
    compartmentNames << "LO_R (Lobula)";
    compartmentNames << "MB_R (Mushroom Body)";
    compartmentNames << "PVLP_R (Posterior Ventro-lateral Protocerebrum)";
    compartmentNames << "OTU_R (Optic Tubercle)";
    compartmentNames << "WED_R (Wedge)";
    compartmentNames << "SMP_R (Superior Medial Protocerebrum)";
    compartmentNames << "LH_R (Lateral Horn)";
    compartmentNames << "SLP_R (Superior Lateral Protocerebrum)";
    compartmentNames << "LB_R (Lateral Bulb)";
    compartmentNames << "SIP_R (Superior Intermediate Protocerebrum)";
    compartmentNames << "IB_R (Inferior Bridge)";
    compartmentNames << "IVLP_R (Inferior Ventro-lateral Protocerebrum)";
    compartmentNames << "IPS_R (Inferior Posterior Slope)";
    compartmentNames << "SPS_R (Superior Posterior Slope)";
    compartmentNames << "LAL_R (Lateral Accessory Lobe)";
    compartmentNames << "PRW (Prow)";
    compartmentNames << "AME_R (Accessory Medulla)";
    compartmentNames << "GA_R (Gall)";
    compartmentNames << "CRE_L (Crepine)";
    compartmentNames << "EPA_L (Epaulette)";
    compartmentNames << "VES_L (Vesta)";
    compartmentNames << "ATL_L (Antler)";
    compartmentNames << "PLP_L (Posterior Lateral Protocerebrum)";
    compartmentNames << "AVLP_L (Anterior Ventro-lateral protocerebrum)";
    compartmentNames << "AL_L (Antennal Lobe)";
    compartmentNames << "GOR_L (Gorget)";
    compartmentNames << "SCL_L (Superior Clamp)";
    compartmentNames << "ICL_L (Inferior Clamp)";
    compartmentNames << "ME_L (Medulla)";
    compartmentNames << "LOP_L (Lobula Plate)";
    compartmentNames << "LO_L (Lobula)";
    compartmentNames << "MB_L (Mushroom Body)";
    compartmentNames << "PVLP_L (Posterior Ventro-lateral Protocerebrum)";
    compartmentNames << "OTU_L (Optic Tubercle)";
    compartmentNames << "WED_L (Wedge)";
    compartmentNames << "SMP_L (Superior Medial Protocerebrum)";
    compartmentNames << "LH_L (Lateral Horn)";
    compartmentNames << "SLP_L (Superior Lateral Protocerebrum)";
    compartmentNames << "LB_L (Lateral Bulb)";
    compartmentNames << "SIP_L (Superior Intermediate Protocerebrum)";
    compartmentNames << "IB_L (Inferior Bridge)";
    compartmentNames << "IVLP_L (Inferior Ventro-lateral Protocerebrum)";
    compartmentNames << "IPS_L (Inferior Posterior Slope)";
    compartmentNames << "SPS_L (Superior Posterior Slope)";
    compartmentNames << "LAL_L (Lateral Accessory Lobe)";
    compartmentNames << "AME_L (Accessory Medulla)";
    compartmentNames << "GA_L (Gall)";
    compartmentNames << "AMMC_L (Antennal Mechanosensory and Motor Centre)";
    compartmentNames << "AMMC_R (Antennal Mechanosensory and Motor Centre)";

    for (int i = 0; i < neuronGalleryButtonList.size(); ++ i) {
        if (i >= compartmentNames.size())
            break;
        neuronGalleryButtonList[i]->setLabelText(compartmentNames[i]);
    }

    ui->fragmentGalleryWidget->updateNameSortTable();
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
        ui->fragmentGalleryWidget->updateButtonsGeometry();
    }
}

void NaMainWindow::on3DViewerRotationChanged(const Rotation3D& rot)
{
    Vector3D angles = rot.convertBodyFixedXYZRotationToThreeAngles();
    int rotX = Na3DWidget::radToDeg(angles.x());
    int rotY = Na3DWidget::radToDeg(angles.y());
    int rotZ = Na3DWidget::radToDeg(angles.z());
    int oldRotX = ui->rotXWidget->spinBox->value();
    int oldRotY = ui->rotYWidget->spinBox->value();
    int oldRotZ = ui->rotZWidget->spinBox->value();
    if (Na3DWidget::eulerAnglesAreEquivalent(rotX, rotY, rotZ, oldRotX, oldRotY, oldRotZ))
        return;
    // Block signals from individual rot widgets until we update them all
    ui->rotXWidget->blockSignals(true);
    ui->rotYWidget->blockSignals(true);
    ui->rotZWidget->blockSignals(true);

    ui->rotXWidget->setAngle(rotX);
    ui->rotYWidget->setAngle(rotY);
    ui->rotZWidget->setAngle(rotZ);

    ui->rotXWidget->blockSignals(false);
    ui->rotYWidget->blockSignals(false);
    ui->rotZWidget->blockSignals(false);
}

void NaMainWindow::update3DViewerXYZBodyRotation()
{
    int rotX = ui->rotXWidget->spinBox->value();
    int rotY = ui->rotYWidget->spinBox->value();
    int rotZ = ui->rotZWidget->spinBox->value();
    // qDebug() << rotX << ", " << rotY << ", " << rotZ;
    ui->v3dr_glwidget->setXYZBodyRotationInt(rotX, rotY, rotZ);
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
        ui->progressBar3d->setValue(prog);
        // ui->v3dr_glwidget->setResizeEnabled(false); // don't show ugly brief resize behavior
        ui->widget_progress3d->show();
    }
    else
        setProgressValue(prog);
}

void NaMainWindow::complete3DProgress()
{
    if (use3DProgress) {
        ui->widget_progress3d->hide();
        // avoid jerky resize to accomodated progress widget
        QCoreApplication::processEvents(); // flush pending resize events
        ui->v3dr_glwidget->resizeEvent(NULL);
        ui->v3dr_glwidget->setResizeEnabled(true);
        //
        ui->v3dr_glwidget->update();
    }
    else completeProgress();
}

void NaMainWindow::set3DProgressMessage(QString msg)
{
    if (use3DProgress) {
        ui->progressLabel3d->setText(msg);
        ui->v3dr_glwidget->setResizeEnabled(false); // don't show ugly brief resize behavior
        ui->widget_progress3d->show();
    }
    else
        setProgressMessage(msg);
}

char* NaMainWindow::getConsoleURL()
{
    return consoleUrl;
}

// NutateThread
