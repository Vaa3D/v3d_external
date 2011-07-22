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
#include "../AnnotationSession.h"
#include "../MultiColorImageStackNode.h"
#include "../NeuronAnnotatorResultNode.h"
#include "../TimebasedIdentifierGenerator.h"
#include "RendererNeuronAnnotator.h"
#include "GalleryButton.h"
#include "../../cell_counter/CellCounter3D.h"
#include "../NeuronSelector.h"
#include "FragmentGalleryWidget.h"

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
            sleep(0.5);
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
    : nutateThread(NULL), statusProgressBar(NULL)
{
    ui.setupUi(this);
    //QMetaObject::connectSlotsByName(this); This is apparently already called by setupUi, so calling it again creates repeat trigger events
    annotationSession=0;

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

    // hide the File->Open 3D Image stack menu item
    ui.menuFile->removeAction(ui.actionLoad_Tiff);
    ui.menuFile->removeAction(ui.actionCell_Counter_3D_2ch_lsm);

    // visualize compartment map
    //QDockWidget *dock = new QDockWidget(tr("Compartment Map"), this);
    //dock->setWidget( ui.compartmentMapWidget);

    ui.compartmentMapWidget->setComboBox(ui.compartmentMapComboBox);
    connect(ui.compartmentMapComboBox, SIGNAL(currentIndexChanged(int)), ui.compartmentMapWidget, SLOT(switchCompartment(int)));
    
    // Wire up MIP viewer
    // Status bar message
    connect(ui.naLargeMIPWidget, SIGNAL(statusMessage(const QString&)),
            statusBar(), SLOT(showMessage(const QString&)));
    connect(ui.sharedGammaWidget, SIGNAL(gammaBrightnessChanged(qreal)),
            ui.naLargeMIPWidget, SLOT(setGammaBrightness(qreal)));
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

    // Wire up Z-stack / HDR viewer
    ui.HDR_checkBox->setChecked(true);
    ui.naZStackWidget->setHDRCheckState(false);
    ui.HDR_checkBox->setChecked(false);
    // ui.gammaWidget_Zstack->hide();
    ui.BoxSize_spinBox->setMinimum(MINSZBOX);
    
    connect(ui.HDR_checkBox, SIGNAL(stateChanged(int)),
            ui.naZStackWidget, SLOT(setHDRCheckState(int)));
    // connect(ui.naZStackWidget, SIGNAL(changedHDRCheckState(bool)),
    //         ui.gammaWidget_Zstack, SLOT(setVisible(bool)));
    connect(ui.sharedGammaWidget, SIGNAL(gammaBrightnessChanged(qreal)),
            ui.naZStackWidget, SLOT(setGammaBrightness(qreal)));
    connect(ui.HDRRed_pushButton, SIGNAL(clicked()),
            ui.naZStackWidget, SLOT(setRedChannel()));
    connect(ui.HDRGreen_pushButton, SIGNAL(clicked()),
            ui.naZStackWidget, SLOT(setGreenChannel()));
    connect(ui.HDRBlue_pushButton, SIGNAL(clicked()),
            ui.naZStackWidget, SLOT(setBlueChannel()));
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
    connect(ui.redToggleButton, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setChannelR(bool)));
    connect(ui.greenToggleButton, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setChannelG(bool)));
    connect(ui.blueToggleButton, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(setChannelB(bool)));
    // 3D rotation
    connect(&(ui.v3dr_glwidget->cameraModel), SIGNAL(rotationChanged(const Rotation3D&)),
            this, SLOT(on3DViewerRotationChanged(const Rotation3D&)));
    connect(ui.rotXWidget, SIGNAL(angleChanged(int)),
            this, SLOT(update3DViewerXYZBodyRotation()));
    connect(ui.rotYWidget, SIGNAL(angleChanged(int)),
            this, SLOT(update3DViewerXYZBodyRotation()));
    connect(ui.rotZWidget, SIGNAL(angleChanged(int)),
            this, SLOT(update3DViewerXYZBodyRotation()));
    // 3D gamma correction
    connect(ui.sharedGammaWidget, SIGNAL(gammaBrightnessChanged(qreal)),
            ui.v3dr_glwidget, SLOT(setGammaBrightness(qreal)));
    connect(ui.sharedGammaWidget, SIGNAL(gammaBrightnessChanged(qreal)),
            this, SLOT(updateThumbnailGamma(qreal)));
    connect(ui.v3dr_glwidget, SIGNAL(progressAchieved(int)),
            this, SLOT(set3DProgress(int)));
    connect(ui.v3dr_glwidget, SIGNAL(progressComplete()),
            this, SLOT(complete3DProgress()));
    connect(ui.v3dr_glwidget, SIGNAL(progressMessage(QString)),
            this, SLOT(set3DProgressMessage(QString)));

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
    connect(ui.resetColorsButton, SIGNAL(clicked()),
            this, SLOT(resetColors()));

    // Crosshair
    connect(ui.actionShow_Crosshair, SIGNAL(toggled(bool)),
            ui.naLargeMIPWidget, SLOT(showCrosshair(bool)));
    connect(ui.actionShow_Crosshair, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(showCrosshair(bool)));
    connect(ui.actionShow_Crosshair, SIGNAL(toggled(bool)),
            ui.naZStackWidget, SLOT(showCrosshair(bool)));

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

void NaMainWindow::resetColors() {
    ui.redToggleButton->setChecked(true);
    ui.greenToggleButton->setChecked(true);
    ui.blueToggleButton->setChecked(true);
    ui.sharedGammaWidget->reset();
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
        if (nutateThread->isRunning())
            nutateThread->unpause();
    }
    else {
        // qDebug() << "stop nutating";
        if (!nutateThread) return;
        if (nutateThread->isRunning())
            nutateThread->pause();
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

void NaMainWindow::handleCoordinatedCloseEvent(QCloseEvent *e) {
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

void NaMainWindow::on_actionOpen_triggered() {

    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        "Select Color Separation Image Directory",
                                                        QDir::currentPath(),
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

    // qDebug() << dirName;

    // If user presses cancel, QFileDialog::getExistingDirectory returns a null string
    if (dirName.isEmpty()) // Silently do nothing when user presses Cancel.  No error dialogs please!
        return;

    openMulticolorImageStack(dirName);
}


void NaMainWindow::openMulticolorImageStack(QString dirName)
{
    QDir imageDir(dirName);

    if (! imageDir.exists()) {
        QMessageBox::warning(this, tr("No such directory"),
                             QString("No directory '%1' exists!  Please try something else.").arg(dirName));
        return;
    }

    std::cout << "Selected directory=" << imageDir.absolutePath().toStdString() << endl;

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

QString NaMainWindow::suggestedExportFilenameFromCurrentState() {
    MultiColorImageStackNode* multiColorImageStackNode=this->annotationSession->getMultiColorImageStackNode();
    QStringList lsmFilePathsList=multiColorImageStackNode->getLsmFilePathList();
    if (lsmFilePathsList.size()>0) {
        // First get filename prefix
        QString firstFilePath=lsmFilePathsList.at(0);
        QStringList components=firstFilePath.split(QRegExp("/"));
        QString name=components.at(components.size()-1);
        QStringList extComponents=name.split(QRegExp("\\."));
        QString filenamePrefix=extComponents.at(0);
        // Next, add current state
        if(annotationSession->getOverlayStatusList().at(AnnotationSession::REFERENCE_MIP_INDEX)) {
            filenamePrefix.append("_R");
        }
        if (annotationSession->getOverlayStatusList().at(AnnotationSession::BACKGROUND_MIP_INDEX)) {
            filenamePrefix.append("_B");
        }
        QList<bool> neuronStatusList = annotationSession->getMaskStatusList();
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



void NaMainWindow::on_action3D_Volume_triggered() {
    QString suggestedFile=suggestedExportFilenameFromCurrentState();
    QString filename = QFileDialog::getSaveFileName(0, QObject::tr("Save 3D Volume to an .tif file"), suggestedFile, QObject::tr("3D Volume (*.tif)"));
    if (!(filename.isEmpty())){
        if(annotationSession){
            ExportFile *pExport = new ExportFile;
            if(pExport->init(annotationSession->getOriginalImageStackAsMy4DImage(), annotationSession->getNeuronMaskAsMy4DImage(), annotationSession->getReferenceStack(),
                             annotationSession->getMaskStatusList(), annotationSession->getOverlayStatusList(), filename)){
                connect(pExport, SIGNAL(finished()), pExport, SLOT(deleteLater()));
                pExport->start();
            }
        }
    }
}

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

bool NaMainWindow::loadAnnotationSessionFromDirectory(QDir imageInputDirectory) {
    annotationSession = new AnnotationSession();

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
    annotationSession->setMultiColorImageStackNode(multiColorImageStackNode);

    // Create result node
    long resultNodeId=TimebasedIdentifierGenerator::getSingleId();
    NeuronAnnotatorResultNode* resultNode = new NeuronAnnotatorResultNode(resultNodeId);
    if (!resultNode->ensureDirectoryExists()) {
        QMessageBox::warning(this, tr("Could not create NeuronAnnotationResultNode"),
                             "Error creating directory="+resultNode->getDirectoryPath());
        return false;
    }
    annotationSession->setNeuronAnnotatorResultNode(resultNode);

    // Load session
    if (!annotationSession->loadReferenceStack()) {
        return false;
    }

    if (!annotationSession->loadOriginalImageStack()) {
        return false;
    }

    if (!annotationSession->loadLsmMetadata()) {
        return false;
    }

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

    if (!annotationSession->loadNeuronMaskStack()) {
        return false;
    }
    if (!annotationSession->prepareLabelIndex()) {
        return false;
    }
    if (!annotationSession->populateMipLists()) {
        return false;
    }

    createOverlayGallery();

    createNeuronGallery();

    // ui.v3dr_glwidget->loadMy4DImage(annotationSession->getOriginalImageStackAsMy4DImage());
    if (! loadMy4DImage(annotationSession->getOriginalImageStackAsMy4DImage(),
                  annotationSession->getNeuronMaskAsMy4DImage()) )
        return false;

    if (!ui.v3dr_glwidget->populateNeuronMaskAndReference(annotationSession->getNeuronMaskAsMy4DImage(), annotationSession->getReferenceStack())) {
        return false;
    }

    ui.v3dr_glwidget->setThickness(annotationSession->getZRatio());

    ui.v3dr_glwidget->update();

    // Annotation model update
    ui.v3dr_glwidget->setAnnotationSession(annotationSession);
    ui.naLargeMIPWidget->setAnnotationSession(annotationSession);
    ui.naZStackWidget->setAnnotationSession(annotationSession);

    connect(annotationSession, SIGNAL(modelUpdated(QString)), ui.naLargeMIPWidget, SLOT(annotationModelUpdate(QString)));
    connect(annotationSession, SIGNAL(modelUpdated(QString)), ui.naZStackWidget, SLOT(annotationModelUpdate(QString)));
    connect(annotationSession, SIGNAL(modelUpdated(QString)), ui.v3dr_glwidget, SLOT(annotationModelUpdate(QString)));
	
    // Neuron Selector update
    neuronSelector = new NeuronSelector();

    connect(ui.v3dr_glwidget, SIGNAL(neuronSelected(double,double,double)), neuronSelector, SLOT(updateSelectedPosition(double,double,double)));
    connect(neuronSelector, SIGNAL(neuronHighlighted(bool)), ui.v3dr_glwidget, SLOT(updateHighlightNeurons(bool)));
    connect(annotationSession, SIGNAL(deselectNeuron()), neuronSelector, SLOT(deselectCurrentNeuron()));

    neuronSelector->setAnnotationSession(annotationSession);
    neuronSelector->init();

    // show selected neuron
    connect(ui.v3dr_glwidget, SIGNAL(neuronShown(QList<int>)), annotationSession, SLOT(showSelectedNeuron(QList<int>)));
    connect(ui.v3dr_glwidget, SIGNAL(neuronShownAll(QList<int>)), annotationSession, SLOT(showAllNeurons(QList<int>)));
    connect(annotationSession, SIGNAL(modelUpdated(QString)), this, SLOT(synchronizeGalleryButtonsToAnnotationSession(QString)));
    connect(annotationSession, SIGNAL(modelUpdated(QString)), neuronSelector, SLOT(updateSelectedNeurons()));

    // connect(annotationSession, SIGNAL(scrollBarFocus(int)), ui.scrollArea->horizontalScrollBar(), SLOT(setValue(int)));
    connect(annotationSession, SIGNAL(scrollBarFocus(FragmentSelectionModel::FragmentIndex)),
            ui.fragmentGalleryWidget, SLOT(scrollToFragment(FragmentSelectionModel::FragmentIndex)));

    return true;
}

bool NaMainWindow::closeAnnotationSession() {
    if (annotationSession!=0) {
        annotationSession->save();
        delete annotationSession;
    }
    return true;
}

bool NaMainWindow::loadMy4DImage(const My4DImage * img, const My4DImage * neuronMaskImg)
{
    ui.naLargeMIPWidget->loadMy4DImage(img, neuronMaskImg);
    ui.v3dr_glwidget->loadMy4DImage(img, neuronMaskImg);
    ui.naZStackWidget->loadMy4DImage(img, neuronMaskImg);
    setZRange(1, img->getZDim());
    // Start in middle of volume
    ui.naZStackWidget->setCurrentZSlice(img->getZDim() / 2 + 1);
    // Need at least two colors for use of the color buttons to make sense
    ui.HDRRed_pushButton->setEnabled(img->getCDim() > 1);
    ui.HDRGreen_pushButton->setEnabled(img->getCDim() > 1);
    ui.HDRBlue_pushButton->setEnabled(img->getCDim() > 2);
    return true;
}

// local setButtonGamma() method used to support multithreaded version of updateThumbnailGamma()
static BrightnessCalibrator<int> buttonCalibrator;
static GalleryButton* setButtonGamma(GalleryButton* & button) {
    button->setBrightness(buttonCalibrator);
    return button;
}

void NaMainWindow::updateThumbnailGamma(qreal gamma)
{
    buttonCalibrator.setHdrRange(0, 255);
    buttonCalibrator.setGamma(gamma);
    bool bUseConcurrent = true;
    if (bUseConcurrent) { // asynchronous update
        QtConcurrent::map(overlayGalleryButtonList, setButtonGamma);
        QtConcurrent::map(neuronGalleryButtonList, setButtonGamma);
    }
    else { // serial update
        foreach(GalleryButton* button, overlayGalleryButtonList) {
            button->setBrightness(buttonCalibrator);
        }
        foreach(GalleryButton* button, neuronGalleryButtonList) {
            button->setBrightness(buttonCalibrator);
        }
    }
}

void NaMainWindow::createOverlayGallery() {
    qDebug() << "createOverlayGallery() start";
    QList<QImage*> * overlayMipList = annotationSession->getOverlayMipList();

    QFrame* ui_maskFrame = qFindChild<QFrame*>(this, "maskFrame");
    if (! ui_maskFrame->layout()) {
        ui_maskFrame->setLayout(new QHBoxLayout());
        assert(ui_maskFrame->layout());
    }
    QLayout *managementLayout = ui_maskFrame->layout();

    // Delete any old contents from the layout, such as previous thumbnails
    QLayoutItem * item;
    while ( ( item = managementLayout->takeAt(0)) != NULL )
    {
        delete item->widget();
        delete item;
    }

    GalleryButton* referenceButton = new GalleryButton(*overlayMipList->at(AnnotationSession::REFERENCE_MIP_INDEX), "Reference", AnnotationSession::REFERENCE_MIP_INDEX);
    referenceButton->setChecked(false); // we do not want reference initially loaded
    annotationSession->setOverlayStatus(AnnotationSession::REFERENCE_MIP_INDEX, false);
    managementLayout->addWidget(referenceButton);
    overlayGalleryButtonList.append(referenceButton);
    connect(referenceButton, SIGNAL(declareChange(int,bool)), annotationSession, SLOT(overlayUpdate(int,bool)));

    GalleryButton* backgroundButton = new GalleryButton(*overlayMipList->at(AnnotationSession::BACKGROUND_MIP_INDEX), "Background", AnnotationSession::BACKGROUND_MIP_INDEX);
    backgroundButton->setChecked(true); // we do want background initially loaded
    annotationSession->setOverlayStatus(AnnotationSession::BACKGROUND_MIP_INDEX, true);
    managementLayout->addWidget(backgroundButton);
    overlayGalleryButtonList.append(backgroundButton);
    connect(backgroundButton, SIGNAL(declareChange(int,bool)), annotationSession, SLOT(overlayUpdate(int,bool)));

}

void NaMainWindow::createNeuronGallery() {
    qDebug() << "createNeuronGallery() start";
    QList<QImage*> * maskMipList = annotationSession->getNeuronMipList();

    // QFrame* ui_maskGallery = qFindChild<QFrame*>(this, "maskGallery");
    // Delete any old contents from the layout, such as previous thumbnails
    ui.fragmentGalleryWidget->clear();

    qDebug() << "Number of neuron masks = " << maskMipList->size();
    for (int i = 0; i < maskMipList->size(); ++i) {
        GalleryButton* button = new GalleryButton(*maskMipList->at(i), QString("Neuron %1").arg(i), i);
        neuronGalleryButtonList.append(button);
        ui.fragmentGalleryWidget->appendFragment(button);
        button->setChecked(true); // start as checked since full image loaded initially
        annotationSession->setNeuronMaskStatus(i, true);
        connect(button, SIGNAL(declareChange(int,bool)), annotationSession, SLOT(neuronMaskUpdate(int,bool)));
    }

    qDebug() << "createMaskGallery() end size=" << maskMipList->size();
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
void NaMainWindow::synchronizeGalleryButtonsToAnnotationSession(QString updateString) {
    // We are not using the update string in this case, which is from the modelUpdated() signal,
    // because we are doing a total update.
    int maskStatusListSize=annotationSession->getMaskStatusList().size();
    int neuronGalleryButtonSize=neuronGalleryButtonList.size();
        for (int i=0;i<annotationSession->getMaskStatusList().size();i++) {
            if (annotationSession->neuronMaskIsChecked(i)) {
                neuronGalleryButtonList.at(i)->setChecked(true);
            }
            else{
                neuronGalleryButtonList.at(i)->setChecked(false);
            }
        }

        // Reference toggle
        if (annotationSession->getOverlayStatusList().at(AnnotationSession::REFERENCE_MIP_INDEX)) {
            overlayGalleryButtonList.at(AnnotationSession::REFERENCE_MIP_INDEX)->setChecked(true);
        } else {
            overlayGalleryButtonList.at(AnnotationSession::REFERENCE_MIP_INDEX)->setChecked(false);
        }

        // Background toggle
        if (annotationSession->getOverlayStatusList().at(AnnotationSession::BACKGROUND_MIP_INDEX)) {
            overlayGalleryButtonList.at(AnnotationSession::BACKGROUND_MIP_INDEX)->setChecked(true);
        } else {
            overlayGalleryButtonList.at(AnnotationSession::BACKGROUND_MIP_INDEX)->setChecked(false);
        }

}

void NaMainWindow::set3DProgress(int prog) {
    if (prog >= 100) {
        complete3DProgress();
    }
    else {
        statusProgressBar->setValue(prog);
        statusProgressBar->show();
        // neither setEnabled() nor raise() help with initial grayness problem of progress bar
        // statusProgressBar->setEnabled(true);
        // statusProgressBar->raise();
        // update();
    }
}

void NaMainWindow::complete3DProgress() {
    statusProgressBar->hide();
    statusProgressMessage->hide();
    statusBar()->showMessage("Progress complete", 1000);
}

void NaMainWindow::set3DProgressMessage(QString msg) {
    statusBar()->showMessage("");
    statusProgressMessage->setText(msg);
    statusProgressMessage->show();
}

// NutateThread



