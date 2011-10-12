// Fix windows compile problem with windows.h being included too late.
// I wish it wasn't included at all...
#ifdef _MSC_VER
#define NOMINMAX //added by PHC, 2010-05-20 to overcome VC min max macro
#include <windows.h>
#endif

#include <QDir>
#include <QFileInfo>
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

using namespace std;

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

NaMainWindow::NaMainWindow()
    : nutateThread(NULL)
    , statusProgressBar(NULL)
{
    ui.setupUi(this);

    dataFlowModel=0;
    // TODO - neuronSelector should probably be a member of Na3DViewer, not of NaMainWindow
    neuronSelector = new NeuronSelector(this);

    // Create stubs for recent file menu
    for (int i = 0; i < MaxRecentFiles; ++i) {
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

    // alpha blending
    connect(ui.action3D_alpha_blending, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setAlphaBlending(bool)));
    connect(ui.v3dr_glwidget, SIGNAL(alphaBlendingChanged(bool)),
            ui.action3D_alpha_blending, SLOT(setChecked(bool)));

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
            ui.naLargeMIPWidget, SLOT(showCrosshair(bool)));
    connect(ui.actionShow_Crosshair, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(showCrosshair(bool)));
    connect(ui.actionShow_Crosshair, SIGNAL(toggled(bool)),
            ui.naZStackWidget, SLOT(showCrosshair(bool)));

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
    ui.annotationFrame->consoleConnect();

    initializeContextMenus();
    initializeStereo3DOptions();
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
    }
}

/* slot */
void NaMainWindow::onColorModelChanged()
{
    {
        DataColorModel::Reader colorReader(dataFlowModel->getDataColorModel());
        if (dataFlowModel->getDataColorModel().readerIsStale(colorReader)) return;
        ui.redToggleButton->setChecked(colorReader.getChannelVisibility(0));
        ui.greenToggleButton->setChecked(colorReader.getChannelVisibility(1));
        ui.blueToggleButton->setChecked(colorReader.getChannelVisibility(2));
        ui.sharedGammaWidget->setGammaBrightness(colorReader.getChannelGamma(0));
    }
}

/* slot */
void NaMainWindow::setChannelZeroVisibility(bool v)
{
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

void NaMainWindow::onViewerChanged(int viewerIndex) {
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
        QSettings settings("HHMI", "V3D");
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
    QSettings settings("HHMI", "V3D");
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
    QDir imageDir(dirName);

    if ( ! imageDir.exists() )
    {
        QMessageBox::warning(this, tr("No such directory"),
                             QString("'%1'\n No such directory.\nIs the file share mounted?\nHas the directory moved?").arg(dirName));
        return;
    }

    // std::cout << "Selected directory=" << imageDir.absolutePath().toStdString() << endl;

    if (!closeAnnotationSession()) {
        QMessageBox::warning(this, tr("Could not close previous Annotation Session"),
                     "Error saving previous session and/or clearing memory - please exit application");
                 return;
    }

    if (!loadAnnotationSessionFromDirectory(imageDir)) {
        QMessageBox::warning(this, tr("Could not load image directory"),
                                      "Error loading image directory - please check directory contents");
            return;
    }

    // qDebug() << "NaMainWindow::openMulticolorImageStack() calling addDirToRecentFilesList with dir=" << imageDir.absolutePath();
    addDirToRecentFilesList(imageDir);
}


// Recent files list
void NaMainWindow::addDirToRecentFilesList(QDir imageDir)
{
    QString fileName = imageDir.absolutePath();
    if (fileName.isEmpty()) return;
    QSettings settings("HHMI", "V3D");
    QStringList files = settings.value("NeuronAnnotatorRecentFileList").toStringList();
    if ( (files.size() > 0) && (files[0] == fileName) )
        return; // this dir is already the top entry as is
    files.removeAll(fileName);
    files.removeAll(QString());
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
            files.removeLast();
    settings.setValue("NeuronAnnotatorRecentFileList", files);
    updateRecentFileActions();
}


void NaMainWindow::updateRecentFileActions()
{
    QSettings settings("HHMI", "V3D");
    QStringList files = settings.value("NeuronAnnotatorRecentFileList").toStringList();
    ui.menuOpen_Recent->setEnabled(files.size() > 0);
    for (int i = 0; i < MaxRecentFiles; ++i)
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

bool NaMainWindow::loadAnnotationSessionFromDirectory(QDir imageInputDirectory)
{
    assert(! dataFlowModel);
    dataFlowModel = new DataFlowModel();

    // TODO - deprecate processUpdatedVolumeData() in favor of using downstream data flow components.
    ui.v3dr_glwidget->setDataFlowModel(*dataFlowModel);
    ui.naLargeMIPWidget->setDataFlowModel(*dataFlowModel);
    ui.naZStackWidget->setDataFlowModel(*dataFlowModel);
    ui.fragmentGalleryWidget->setDataFlowModel(*dataFlowModel);
    neuronSelector->setDataFlowModel(*dataFlowModel);
    connectContextMenus(dataFlowModel->getNeuronSelectionModel());

    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(initialized()),
            this, SLOT(processUpdatedVolumeData()));

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

    // Color toggling
    connect(this, SIGNAL(channelVisibilityChanged(int,bool)),
            &dataFlowModel->getDataColorModel(), SLOT(setChannelVisibility(int,bool)));
    connect(ui.resetColorsButton, SIGNAL(clicked()),
            &dataFlowModel->getDataColorModel(), SLOT(resetColors()));
    connect(ui.sharedGammaWidget, SIGNAL(gammaBrightnessChanged(qreal)),
            &dataFlowModel->getDataColorModel(), SLOT(setGamma(qreal)));
    connect(&dataFlowModel->getDataColorModel(), SIGNAL(dataChanged()),
            this, SLOT(onColorModelChanged()));

    // Annotation model update
    connect(dataFlowModel, SIGNAL(modelUpdated(QString)), ui.v3dr_glwidget, SLOT(annotationModelUpdate(QString)));

    // Both mip images and selection model need to be in place to update gallery
    connect(&dataFlowModel->getGalleryMipImages(), SIGNAL(dataChanged()),
            this, SLOT(updateGalleries()));
    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(initialized()),
            this, SLOT(updateGalleries()));

    // Z value comes from camera model
    qRegisterMetaType<Vector3D>("Vector3D");
    connect(&sharedCameraModel, SIGNAL(focusChanged(Vector3D)),
            &dataFlowModel->getZSliceColors(), SLOT(onCameraFocusChanged(Vector3D)));
    ui.naZStackWidget->setZSliceColors(&dataFlowModel->getZSliceColors());
    ui.naZStackWidget->setVolumeData(&dataFlowModel->getVolumeData());
    connect(ui.naZStackWidget, SIGNAL(hdrRangeChanged(int,qreal,qreal)),
            &dataFlowModel->getDataColorModel(), SLOT(setChannelHdrRange(int,qreal,qreal)));

    // Connect annotation widget to neuron selection
    connect(neuronSelector, SIGNAL(neuronSelected(int)),
            ui.annotationFrame, SLOT(selectNeuron(int)));
    connect(&dataFlowModel->getNeuronSelectionModel(), SIGNAL(selectionCleared()),
            ui.annotationFrame, SLOT(deselectNeurons()));
    connect(ui.annotationFrame, SIGNAL(neuronSelected(int)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(selectExactlyOneNeuron(int)));
    connect(ui.annotationFrame, SIGNAL(neuronsDeselected()),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(clearSelection()));

    // Connect mip viewer to data flow model
    ui.naLargeMIPWidget->setMipMergedData(dataFlowModel->getMipMergedData());

    // Need to construct (temporary until backend implemented) MultiColorImageStackNode from this directory
    // This code will be redone when the node/filestore is implemented.
    QString originalImageStackFilePath = imageInputDirectory.absolutePath() + "/" + MultiColorImageStackNode::IMAGE_STACK_FILENAME;
    QFile originalImageStackFile(originalImageStackFilePath);
    if (!originalImageStackFile.exists()) {
        QMessageBox::warning(this, tr("Could not find expected image stack tif file"),
                             "Error finding file="+originalImageStackFilePath);
        return false;
    }
    QString maskLabelFilePath = imageInputDirectory.absolutePath() + "/" + MultiColorImageStackNode::IMAGE_MASK_FILENAME;
    QFile maskLabelFile(maskLabelFilePath);
    if (!maskLabelFile.exists()) {
        QMessageBox::warning(this, tr("Could not find expected image stack mask file"),
                             "Error finding file="+maskLabelFilePath);
        return false;
    }
    QString referenceStackFilePath = imageInputDirectory.absolutePath() + "/" + MultiColorImageStackNode::IMAGE_REFERENCE_FILENAME;
    QFile referenceStackFile(referenceStackFilePath);
    if (!referenceStackFile.exists()) {
        QMessageBox::warning(this, tr("Could not find expected reference stack file"),
                             "Error finding file="+referenceStackFilePath);
        return false;
    }

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
    if (! dataFlowModel->loadVolumeData()) return false;
    // dataChanged() signal will be emitted if load succeeds
    return true;
}

void NaMainWindow::updateGalleries()
{
    updateOverlayGallery();
    updateNeuronGallery();
}

void NaMainWindow::processUpdatedVolumeData() // activated by volumeData::dataChanged() signal
{
    // TODO -- install separate listeners for dataChanged() in the various display widgets

    dataFlowModel->loadLsmMetadata();

    // good
    // ui.v3dr_glwidget->onVolumeDataChanged();

    QDir imageInputDirectory = dataFlowModel->getMultiColorImageStackNode()->getImageDir();

    // At this point it should be reasonable to set the window title
    QString lsmName("Unknown original image");
    QFile lsmFilePathsFile(QString("%1/lsmFilePaths.txt").arg(imageInputDirectory.absolutePath()));
    if (lsmFilePathsFile.exists()) {
        if (lsmFilePathsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            lsmName = lsmFilePathsFile.readLine();
            lsmFilePathsFile.close();
            lsmName = lsmName.trimmed(); // elide carriage return
        }
    }
    QFileInfo lsmFileInfo(lsmName);
    setWindowTitle(QString("%1 - V3D Neuron Annotator").arg(lsmFileInfo.fileName()));

    // good
    {
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (! volumeReader.hasReadLock()) return;
        const Image4DProxy<My4DImage>& imgProxy = volumeReader.getOriginalImageProxy();

        setZRange(1, imgProxy.sz);
        // Start in middle of volume
        ui.naZStackWidget->setCurrentZSlice(imgProxy.sz / 2 + 1);
        // Need at least two colors for use of the color buttons to make sense
        ui.HDRRed_pushButton->setEnabled(imgProxy.sc > 1);
        ui.HDRGreen_pushButton->setEnabled(imgProxy.sc > 1);
        ui.HDRBlue_pushButton->setEnabled(imgProxy.sc > 2);
    }

    // bad or good - depends run to run
    // TODO - why is 3D viewer blank if I move ui.v3dr_glwidget->onVolumeDataChanged() to end of the next block?
    // Looks like some race condition, with this spot near the cusp
    {
        NaVolumeData::Reader volumeReader(dataFlowModel->getVolumeData());
        if (! volumeReader.hasReadLock()) return;
        const Image4DProxy<My4DImage>& imgProxy = volumeReader.getOriginalImageProxy();

        // volume cut update
        ui.XcminSlider->setRange(0, imgProxy.sx-1);
        ui.XcminSlider->setValue(0);
        ui.XcmaxSlider->setRange(0, imgProxy.sx-1);
        ui.XcmaxSlider->setValue(imgProxy.sx-1);

        ui.YcminSlider->setRange(0, imgProxy.sy-1);
        ui.YcminSlider->setValue(0);
        ui.YcmaxSlider->setRange(0, imgProxy.sy-1);
        ui.YcmaxSlider->setValue(imgProxy.sy-1);

        ui.ZcminSlider->setRange(0, imgProxy.sz-1);
        ui.ZcminSlider->setValue(0);
        ui.ZcmaxSlider->setRange(0, imgProxy.sz-1);
        ui.ZcmaxSlider->setValue(imgProxy.sz-1);
    } // release lock

    connect(ui.v3dr_glwidget, SIGNAL(neuronSelected(double,double,double)), neuronSelector, SLOT(updateSelectedPosition(double,double,double)));
    connect(neuronSelector, SIGNAL(landmarksClearNeeded()),
            ui.v3dr_glwidget, SLOT(clearLandmarks()));
    connect(neuronSelector, SIGNAL(landmarksUpdateNeeded(QList<ImageMarker>)),
            ui.v3dr_glwidget, SLOT(setLandmarks(QList<ImageMarker>)));

    // show selected neuron
    connect(ui.v3dr_glwidget, SIGNAL(neuronShown(const QList<int>)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(showFirstSelectedNeuron()));
    connect(ui.v3dr_glwidget, SIGNAL(neuronShown(const QList<int>)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(showOverlays(const QList<int>)));
    connect(ui.v3dr_glwidget, SIGNAL(neuronShown(const QList<int>)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(clearSelection()));

    connect(ui.v3dr_glwidget, SIGNAL(neuronShownAll(const QList<int>)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(showAllNeurons()));
    connect(ui.v3dr_glwidget, SIGNAL(neuronShownAll(const QList<int>)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(showOverlays(const QList<int>)));
    connect(ui.v3dr_glwidget, SIGNAL(neuronShownAll(const QList<int>)),
            &dataFlowModel->getNeuronSelectionModel(), SLOT(clearSelection()));

    connect(dataFlowModel, SIGNAL(modelUpdated(QString)), this, SLOT(synchronizeGalleryButtonsToAnnotationSession(QString)));

    connect(dataFlowModel, SIGNAL(scrollBarFocus(NeuronSelectionModel::NeuronIndex)),
            ui.fragmentGalleryWidget, SLOT(scrollToFragment(NeuronSelectionModel::NeuronIndex)));

    return;
}

bool NaMainWindow::closeAnnotationSession() {
    if (dataFlowModel!=0) {
        dataFlowModel->save();
        delete dataFlowModel;
        dataFlowModel = NULL;
    }
    return true;
}

DataFlowModel* NaMainWindow::getDataFlowModel() const {
    return dataFlowModel;
}

void NaMainWindow::updateOverlayGallery()
{
    GalleryMipImages::Reader mipReader(dataFlowModel->getGalleryMipImages()); // acquire read lock
    if (! mipReader.hasReadLock()) return;
    if (mipReader.getNumberOfOverlays() < 1) return; // mip data are not initialized yet

    NeuronSelectionModel::Reader selectionReader(dataFlowModel->getNeuronSelectionModel());
    if (! selectionReader.hasReadLock()) return;

    // qDebug() << "updateOverlayGallery() start";
    // QList<QImage*> * overlayMipList = dataFlowModel->getOverlayMipList();

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

        GalleryButton* referenceButton = new GalleryButton(
                *mipReader.getOverlayMip(DataFlowModel::REFERENCE_MIP_INDEX),
                "Reference", DataFlowModel::REFERENCE_MIP_INDEX);
        referenceButton->setChecked(selectionReader.getOverlayStatusList().at(
                DataFlowModel::REFERENCE_MIP_INDEX)); // we do not want reference initially loaded
        managementLayout->addWidget(referenceButton);
        overlayGalleryButtonList.append(referenceButton);
        connect(referenceButton, SIGNAL(declareChange(int,bool)),
                &dataFlowModel->getNeuronSelectionModel(), SLOT(updateOverlay(int,bool)));

        GalleryButton* backgroundButton = new GalleryButton(
                *mipReader.getOverlayMip(DataFlowModel::BACKGROUND_MIP_INDEX),
                "Background", DataFlowModel::BACKGROUND_MIP_INDEX);
        backgroundButton->setChecked(selectionReader.getOverlayStatusList().at(
                DataFlowModel::BACKGROUND_MIP_INDEX)); // we do want background initially loaded
        managementLayout->addWidget(backgroundButton);
        overlayGalleryButtonList.append(backgroundButton);
        connect(backgroundButton, SIGNAL(declareChange(int,bool)),
                &dataFlowModel->getNeuronSelectionModel(), SLOT(updateOverlay(int,bool)));
    }
    // Just update icons on subsequent updates
    else {
        for (int i = 0; i < 2; ++i) {
            overlayGalleryButtonList[i]->setThumbnailIcon(*mipReader.getOverlayMip(i));
            overlayGalleryButtonList[i]->update();
        }
    }
}

void NaMainWindow::updateNeuronGallery()
{
    GalleryMipImages::Reader mipReader(dataFlowModel->getGalleryMipImages()); // acquire read lock
    if (! mipReader.hasReadLock()) return;

    NeuronSelectionModel::Reader selectionReader(dataFlowModel->getNeuronSelectionModel());
    if (! selectionReader.hasReadLock()) return;

    // qDebug() << "updateNeuronGallery() start";
    // QList<QImage*> * maskMipList = dataFlowModel->getNeuronMipList();

    // QFrame* ui_maskGallery = qFindChild<QFrame*>(this, "maskGallery");
    // Delete any old contents from the layout, such as previous thumbnails

    if (neuronGalleryButtonList.size() != mipReader.getNumberOfNeurons())
    {
        neuronGalleryButtonList.clear();
        ui.fragmentGalleryWidget->clear(); // deletes buttons

        // qDebug() << "Number of neuron masks = " << mipReader.getNumberOfNeurons();
        for (int i = 0; i < mipReader.getNumberOfNeurons(); ++i)
        {
            GalleryButton* button = new GalleryButton(
                    *mipReader.getNeuronMip(i),
                    QString("Neuron fragment %1").arg(i), i);
            button->setContextMenu(neuronContextMenu);
            neuronGalleryButtonList.append(button);
            ui.fragmentGalleryWidget->appendFragment(button);
            button->setChecked(selectionReader.getMaskStatusList().at(i)); // start as checked since full image loaded initially
            connect(button, SIGNAL(declareChange(int,bool)),
                    &dataFlowModel->getNeuronSelectionModel(), SLOT(updateNeuronMask(int,bool)));
        }

        // qDebug() << "createMaskGallery() end size=" << mipReader.getNumberOfNeurons();
    }
    else {
        for (int i = 0; i < mipReader.getNumberOfNeurons(); ++i)
        {
            neuronGalleryButtonList[i]->setThumbnailIcon(*mipReader.getNeuronMip(i));
            neuronGalleryButtonList[i]->update();
        }
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

void NaMainWindow::set3DProgress(int prog) {
    if (prog >= 100) {
        complete3DProgress();
        return;
    }
    ui.progressBar3d->setValue(prog);
    ui.v3dr_glwidget->setResizeEnabled(false); // don't show ugly brief resize behavior
    ui.widget_progress3d->show();
}

void NaMainWindow::complete3DProgress() {
    ui.widget_progress3d->hide();
    // avoid jerky resize to accomodated progress widget
    QCoreApplication::processEvents(); // flush pending resize events
    ui.v3dr_glwidget->resizeEvent(NULL);
    ui.v3dr_glwidget->setResizeEnabled(true);
    //
    ui.v3dr_glwidget->update();
}

void NaMainWindow::set3DProgressMessage(QString msg) {
    ui.progressLabel3d->setText(msg);
    ui.v3dr_glwidget->setResizeEnabled(false); // don't show ugly brief resize behavior
    ui.widget_progress3d->show();
}

// NutateThread
