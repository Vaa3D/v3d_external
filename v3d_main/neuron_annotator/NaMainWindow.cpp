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
#include "../basic_c_fun/v3d_message.h"
#include "../v3d/v3d_application.h"
#include "AnnotationSession.h"
#include "MultiColorImageStackNode.h"
#include "NeuronAnnotatorResultNode.h"
#include "TimebasedIdentifierGenerator.h"
#include "RendererNeuronAnnotator.h"
#include "GalleryButton.h"
#include "../cell_counter/CellCounter3D.h"
#include "NeuronSelector.h"

using namespace std;

NaMainWindow::NaMainWindow()
    : nutateThread(NULL)
{
    ui.setupUi(this);
    //QMetaObject::connectSlotsByName(this); This is apparently already called by setupUi, so calling it again creates repeat trigger events
    annotationSession=0;

    // Wire up MIP viewer
    // Status bar message
    connect(ui.naLargeMIPWidget, SIGNAL(statusMessage(const QString&)),
            statusBar(), SLOT(showMessage(const QString&)));
    connect(ui.gammaWidget_MIP, SIGNAL(gammaBrightnessChanged(double)),
            ui.naLargeMIPWidget, SLOT(setGammaBrightness(double)));

    // Wire up Z-stack / HDR viewer
    ui.HDR_checkBox->setChecked(true);
    ui.naZStackWidget->setHDRCheckState(true);
    ui.gammaWidget_Zstack->hide();
    connect(ui.HDR_checkBox, SIGNAL(stateChanged(int)),
            ui.naZStackWidget, SLOT(setHDRCheckState(int)));
    connect(ui.naZStackWidget, SIGNAL(changedHDRCheckState(bool)),
            ui.gammaWidget_Zstack, SLOT(setVisible(bool)));
    connect(ui.gammaWidget_Zstack, SIGNAL(gammaBrightnessChanged(double)),
            ui.naZStackWidget, SLOT(setGammaBrightness(double)));
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
    connect(ui.gammaWidget_3D, SIGNAL(gammaBrightnessChanged(double)),
            ui.v3dr_glwidget, SLOT(setGammaBrightness(double)));
    connect(ui.gammaWidget_3D, SIGNAL(gammaBrightnessChanged(double)),
            this, SLOT(updateThumbnailGamma(double)));
    connect(ui.colorResetButton, SIGNAL(clicked()),
            ui.gammaWidget_3D, SLOT(reset()));


    // Whether to use common zoom and focus in MIP, ZStack and 3D viewers
    connect(ui.actionLink_viewers, SIGNAL(toggled(bool)),
            this, SLOT(unifyCameras(bool)));
    unifyCameras(true); // Start with cameras linked
    // Crosshair
    connect(ui.actionShow_Crosshair, SIGNAL(toggled(bool)),
            ui.naLargeMIPWidget, SLOT(showCrosshair(bool)));
    connect(ui.actionShow_Crosshair, SIGNAL(toggled(bool)),
            ui.v3dr_glwidget, SLOT(showCrosshair(bool)));

    // Create "Undo" menu options
    // TODO - figure out which of these variables to expose once we have a QUndoCommand to work with.
    QUndoGroup * undoGroup = new QUndoGroup(this);
    QAction * undoAction = undoGroup->createUndoAction(this);
    QAction * redoAction = undoGroup->createRedoAction(this);
    ui.menuEdit->insertAction(ui.menuEdit->actions().at(0), redoAction);
    ui.menuEdit->insertAction(redoAction, undoAction);
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

void NaMainWindow::handleCoordinatedCloseEvent(QCloseEvent *event) {
    event->accept();
}

void NaMainWindow::closeEvent(QCloseEvent *event)
{
    V3dApplication::handleCloseEvent(event);
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
                                                        QFileDialog::ShowDirsOnly);

    QDir imageDir(dirName);

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
    QString maskLabelIndexFilePath = imageInputDirectory.absolutePath() + "/" + MultiColorImageStackNode::IMAGE_MASK_INDEX_FILENAME;
    QFile maskLabelIndexFile(maskLabelIndexFilePath);
    if (!maskLabelIndexFile.exists()) {
        QMessageBox::warning(this, tr("Could not find expected image stack mask label index file"),
                             "Error finding file="+maskLabelIndexFilePath);
        return false;
    }

    // Create input nodes
    MultiColorImageStackNode* multiColorImageStackNode = new MultiColorImageStackNode();
    multiColorImageStackNode->setPathToMulticolorLabelMaskFile(maskLabelFilePath);
    multiColorImageStackNode->setPathToMulticolorLabelMasIndexFile(maskLabelIndexFilePath);
    multiColorImageStackNode->setPathToOriginalImageStackFile(originalImageStackFilePath);
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
    if (!annotationSession->loadOriginalImageStack()) {
        return false;
    }
    if (!annotationSession->loadNeuronMaskStack()) {
        return false;
    }
    if (!annotationSession->loadMaskLabelIndexFile()) {
        return false;
    }
    if (!annotationSession->populateMaskMipList()) {
        return false;
    }
    createMaskGallery();

    // ui.v3dr_glwidget->loadMy4DImage(annotationSession->getOriginalImageStackAsMy4DImage());
    if (! loadMy4DImage(annotationSession->getOriginalImageStackAsMy4DImage(),
                  annotationSession->getNeuronMaskAsMy4DImage()) )
        return false;

    if (!ui.v3dr_glwidget->populateNeuronMask(annotationSession->getNeuronMaskAsMy4DImage())) {
        return false;
    }

    ui.v3dr_glwidget->update();

    // Annotation model update
    ui.v3dr_glwidget->setAnnotationSession(annotationSession);
    ui.naLargeMIPWidget->setAnnotationSession(annotationSession);
    ui.naZStackWidget->setAnnotationSession(annotationSession);

    // disable slow 3D update temporarily so I can test MIP update
    connect(annotationSession, SIGNAL(modelUpdated(QString)), ui.v3dr_glwidget, SLOT(annotationModelUpdate(QString)));
    connect(annotationSession, SIGNAL(modelUpdated(QString)), ui.naLargeMIPWidget, SLOT(annotationModelUpdate(QString)));
    connect(annotationSession, SIGNAL(modelUpdated(QString)), ui.naZStackWidget, SLOT(annotationModelUpdate(QString)));
	
	// Neuron Selector update
	neuronSelector = new NeuronSelector();
	
	connect(ui.v3dr_glwidget, SIGNAL(neuronSelected(double,double,double)), neuronSelector, SLOT(updateSelectedPosition(double,double,double)));
        connect(neuronSelector, SIGNAL(neuronHighlighted(bool)), ui.v3dr_glwidget, SLOT(updateHighlightNeurons(bool)));
	
        neuronSelector->setAnnotationSession(annotationSession);
	neuronSelector->init();

        // show selected neuron
        connect(ui.v3dr_glwidget, SIGNAL(neuronShown(bool)), annotationSession, SLOT(showSelectedNeuron(bool)));
        connect(ui.v3dr_glwidget, SIGNAL(neuronShownAll(bool)), annotationSession, SLOT(showAllNeurons(bool)));
        connect(annotationSession, SIGNAL(neuronMaskStatusSet()), ui.v3dr_glwidget, SLOT(updateAnnotationModels()));
        connect(annotationSession, SIGNAL(neuronMaskStatusSet()), this, SLOT(updateAnnotationModels()));
        connect(annotationSession, SIGNAL(scrollBarFocus(int)), ui.scrollArea->horizontalScrollBar(), SLOT(setValue(int)));

    return true;
}

bool NaMainWindow::closeAnnotationSession() {
    if (annotationSession!=0) {
        annotationSession->save();
        delete annotationSession;
    }
    return true;
}

void NaMainWindow::on_actionLoad_Tiff_triggered()
{
    static QString tiffDirectory;
    QString tiffFileName = QFileDialog::getOpenFileName(this,
            "Select 3D Image/Stack file",
            tiffDirectory,
            tr("TIFF/LSM/V3DRAW files (*.tif *.lsm *.raw)"));
    if (tiffFileName.isEmpty()) return;
    QFile tiffFile(tiffFileName);
    if (! tiffFile.exists()) return;
    tiffDirectory = tiffFileName;
    currentStackImage.loadImage(tiffFileName.toLocal8Bit().data());
    qDebug() << "currentStackImage x=" << currentStackImage.getXDim() << " y=" << currentStackImage.getYDim() << " z=" << currentStackImage.getZDim()
            << " c=" << currentStackImage.getCDim() << " d=" << currentStackImage.getDatatype();
    // renderer might not be created yet if 3D widget is not shown yet.
    if (! ui.v3dr_glwidget->getRenderer())
        ui.v3dr_glwidget->createRenderer();
    RendererNeuronAnnotator* renderer = (RendererNeuronAnnotator*)ui.v3dr_glwidget->getRenderer();
    assert(renderer);
    renderer->setMasklessSetupStackTexture(true);
    loadMy4DImage(&currentStackImage);
}

void NaMainWindow::on_actionCell_Counter_3D_2ch_lsm_triggered() {
    static QString lsmDirectory;
    QString lsmFileName = QFileDialog::getOpenFileName(this,
                                                       "Select 3D 2-channel lsm file",
                                                       lsmDirectory,
                                                       tr("LSM files (*.lsm)"));
    if (lsmFileName.isEmpty()) return;
    QFile lsmFile(lsmFileName);
    lsmDirectory = lsmFileName;
    My4DImage tmpImage;
    tmpImage.loadImage(lsmFileName.toLocal8Bit().data());
    CellCounter3D::convertMy4DImage2channelToRGB(tmpImage, currentStackImage);
    RendererNeuronAnnotator* renderer = (RendererNeuronAnnotator*)ui.v3dr_glwidget->getRenderer();
    renderer->setMasklessSetupStackTexture(true);
    CellCounter3D cellCounter;
    cellCounter.setInputFilePath(lsmFileName);
    cellCounter.loadMy4DImage(&currentStackImage);
    cellCounter.findCells();
    cellCounter.markImage();
    ui.v3dr_glwidget->loadMy4DImage(&currentStackImage);
    cellCounter.writeOutputImageFile();
    cellCounter.writeOutputReportFile();
}

bool NaMainWindow::loadMy4DImage(const My4DImage * img, const My4DImage * neuronMaskImg)
{
    ui.naLargeMIPWidget->loadMy4DImage(img, neuronMaskImg);
    ui.v3dr_glwidget->loadMy4DImage(img, neuronMaskImg);
    ui.naZStackWidget->loadMy4DImage(img, neuronMaskImg);
    setZRange(1, img->getZDim());
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

void NaMainWindow::updateThumbnailGamma(double gamma)
{
    buttonCalibrator.setHdrRange(0, 255);
    buttonCalibrator.setGamma(gamma);
    bool bUseConcurrent = true;
    if (bUseConcurrent) { // asynchronous update
        QtConcurrent::map(mipGalleryButtonList, setButtonGamma);
    }
    else { // serial update
        foreach(GalleryButton* button, mipGalleryButtonList) {
            button->setBrightness(buttonCalibrator);
        }
    }
}

void NaMainWindow::createMaskGallery() {
    qDebug() << "createMaskGallery() start";
    QList<QPixmap> * maskMipList = annotationSession->getMaskMipList();

    // Step 1: Add background MIP
    QFrame* ui_maskFrame = qFindChild<QFrame*>(this, "maskFrame");
    QHBoxLayout *managementLayout = new QHBoxLayout();
    ui_maskFrame->setLayout(managementLayout);

    GalleryButton* backgroundButton = new GalleryButton(maskMipList->at(0), "Background", 0);
    backgroundButton->setChecked(true); // since full image loaded initially
    annotationSession->setNeuronMaskStatus(0, true);
    managementLayout->addWidget(backgroundButton);
    mipGalleryButtonList.append(backgroundButton);
    connect(backgroundButton, SIGNAL(declareChange(int,bool)), annotationSession, SLOT(neuronMaskUpdate(int,bool)));

    // Step 2: Add Neuron-Mask Gallery
    QFrame* ui_maskGallery = qFindChild<QFrame*>(this, "maskGallery");
    QHBoxLayout *galleryLayout = new QHBoxLayout();
    ui_maskGallery->setLayout(galleryLayout);
    for (int i = 1; i < maskMipList->size(); ++i) {
        GalleryButton* button = new GalleryButton(maskMipList->at(i), QString("Neuron %1").arg(i), i);
        mipGalleryButtonList.append(button);
        galleryLayout->addWidget(button);
        button->setChecked(true); // start as checked since full image loaded initially
        annotationSession->setNeuronMaskStatus(i, true);
        connect(button, SIGNAL(declareChange(int,bool)), annotationSession, SLOT(neuronMaskUpdate(int,bool)));
    }

    qDebug() << "createMaskGallery() end size=" << maskMipList->size();
}

// update neuron selected status
void NaMainWindow::updateAnnotationModels() {
        for (int i=1;i<annotationSession->getMaskStatusList().size();i++) {
            if (annotationSession->neuronMaskIsChecked(i)) {
                mipGalleryButtonList.at(i)->setChecked(true);
            }
            else{
                mipGalleryButtonList.at(i)->setChecked(false);
            }
        }

        // Background toggle
        if (annotationSession->getMaskStatusList().at(0)) {
            mipGalleryButtonList.at(0)->setChecked(true);
        } else {
            mipGalleryButtonList.at(0)->setChecked(false);
        }

}



