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
#include "renderer_gl1.h"

#include "PConverter.h"
#include "PMain.h"
#include "ProgressBar.h"
#include "PLog.h"
#include "../control/CConverter.h"
#include "../control/CSettings.h"
#include "QProgressSender.h"

using namespace terafly;

PConverter* PConverter::uniqueInstance = NULL;
PConverter* PConverter::instance(V3DPluginCallback *callback, QWidget *parent)
{
    if (uniqueInstance == 0)
        uniqueInstance = new PConverter(callback, parent);
    return uniqueInstance;
}
void PConverter::uninstance()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    CConverter::uninstance();
    PLog::uninstance();
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = 0;
    }
}

PConverter::PConverter(V3DPluginCallback *callback, QWidget *parent) : QWidget(parent)
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    //initializing members
    V3D_env = callback;
    parentWidget = parent;
    operationInProgress = false;

    //main widgets
    helpBox = new QHelpBox(this);
    helpBox->setFixedHeight(60);
    helpBox->setIconSize(35, 35);
    import_panel = new QGroupBox("Step 1: Import volume from:");
    conversion_panel = new QGroupBox("Step 2: Convert volume to:");

#if defined(USE_Qt5) 
    
    import_panel->setStyle(new QCommonStyle());
    conversion_panel->setStyle(new QCommonStyle());
    
#else
    
    import_panel->setStyle(new QWindowsStyle());
    conversion_panel->setStyle(new QWindowsStyle());
    
#endif
    
    progressBar = new QProgressBar(this);
    startButton = new QPushButton(this);
    startButton->setIcon(QIcon(":/icons/start.png"));
    startButton->setText("Start");
    stopButton = new QPushButton(this);
    stopButton->setIcon(QIcon(":/icons/stop.png"));
    stopButton->setText("Stop");
    statusBar = new QStatusBar();
    statusBar->setFont(QFont("",8));

    //import form widget
    inFormatCBox = new QComboBox();
    inFormatCBox->insertItem(0, iim::TIF3D_FORMAT.c_str());
    inFormatCBox->insertItem(1, iim::SIMPLE_FORMAT.c_str());
    inFormatCBox->insertItem(2, iim::STACKED_FORMAT.c_str());
    inFormatCBox->insertItem(3, iim::TILED_TIF3D_FORMAT.c_str());
    inFormatCBox->insertItem(4, iim::TILED_MC_TIF3D_FORMAT.c_str());
    inFormatCBox->insertItem(5, iim::RAW_FORMAT.c_str());
    inFormatCBox->insertItem(6, iim::SIMPLE_RAW_FORMAT.c_str());
    inFormatCBox->insertItem(7, iim::TILED_FORMAT.c_str());
    inFormatCBox->insertItem(8, iim::TILED_MC_FORMAT.c_str());
   // PMain::setEnabledComboBoxItem(inFormatCBox, 0, false);
    inFormatCBox->setEditable(true);
    inFormatCBox->lineEdit()->setReadOnly(true);
    inFormatCBox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < inFormatCBox->count(); i++)
    {
      inFormatCBox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
      if(inFormatCBox->itemText(i).compare(CSettings::instance()->getVCInputFormat().c_str()) == 0)
          inFormatCBox->setCurrentIndex(i);
    }
    inPathField    = new QLineEdit(QString(CSettings::instance()->getVCInputPath().c_str()));
    inDirButton = new QPushButton("Browse for dir...");
    inFileButton = new QPushButton("Browse for file...");
    timeSeriesCheckBox = new QCheckBox("Time series of");
    timeSeriesCheckBox->setChecked(CSettings::instance()->getVCTimeSeries());

    //import form layout
    inButtonLayout = new QStackedLayout();
    inButtonLayout->addWidget(inDirButton);
    inButtonLayout->addWidget(inFileButton);
    QHBoxLayout* importFormRow1Layout = new QHBoxLayout();
    importFormRow1Layout->addWidget(inFormatCBox);
    inFormatCBox->setFixedWidth(220);
    importFormRow1Layout->addWidget(inPathField, 1);
    importFormRow1Layout->addLayout(inButtonLayout);
    QHBoxLayout* importFormRow2Layout = new QHBoxLayout();
    timeSeriesCheckBox->setFixedWidth(220);
    importFormRow2Layout->addWidget(timeSeriesCheckBox, 0);
    importFormRow2Layout->addStretch(1);
    QVBoxLayout* importFormLayout = new QVBoxLayout();
    importFormLayout->addLayout(importFormRow2Layout);
    importFormLayout->addLayout(importFormRow1Layout);
    import_panel->setLayout(importFormLayout);

    //conversion form widget
    outFormatCBox = new QComboBox();
    outFormatCBox->insertItem(0, iim::STACKED_FORMAT.c_str());
    outFormatCBox->insertItem(1, iim::SIMPLE_FORMAT.c_str());
    //outFormatCBox->insertItem(2, iim::TIF3D_FORMAT.c_str());
    outFormatCBox->insertItem(2, iim::TILED_TIF3D_FORMAT.c_str());
    outFormatCBox->insertItem(3, iim::TILED_MC_TIF3D_FORMAT.c_str());
    //outFormatCBox->insertItem(5, iim::RAW_FORMAT.c_str());
    outFormatCBox->insertItem(4, iim::TILED_FORMAT.c_str());
    outFormatCBox->insertItem(5, iim::TILED_MC_FORMAT.c_str());
    outFormatCBox->insertItem(6, iim::BDV_HDF5_FORMAT.c_str());
    outFormatCBox->insertItem(7, iim::SIMPLE_RAW_FORMAT.c_str());
    /*PMain::setEnabledComboBoxItem(outFormatCBox, 1, false);
    PMain::setEnabledComboBoxItem(outFormatCBox, 2, false);
    PMain::setEnabledComboBoxItem(outFormatCBox, 5, false);
    PMain::setEnabledComboBoxItem(outFormatCBox, 8, false);*/
    outFormatCBox->setEditable(true);
    outFormatCBox->lineEdit()->setReadOnly(true);
    outFormatCBox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < outFormatCBox->count(); i++)
    {
      outFormatCBox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
      if(outFormatCBox->itemText(i).compare(CSettings::instance()->getVCOutputFormat().c_str()) == 0)
          outFormatCBox->setCurrentIndex(i);
    }
    outPathField    = new QLineEdit(QString(CSettings::instance()->getVCOutputPath().c_str()));
    outDirButton = new QPushButton("Browse for dir...");
    outFileButton = new QPushButton("Browse for file...");
    resolutionsLayout = new QVBoxLayout();
    addResolutionButton = new QPushButton("+");
    blockWidthField = new QSpinBox();
    blockWidthField->setAlignment(Qt::AlignCenter);
    blockWidthField->setMinimum(100);
    blockWidthField->setMaximum(100000);
    blockWidthField->setValue(CSettings::instance()->getVCStacksWidth());
    blockWidthField->setSuffix(" (X)");
    blockHeightField = new QSpinBox();
    blockHeightField->setAlignment(Qt::AlignCenter);
    blockHeightField->setMinimum(100);
    blockHeightField->setMaximum(100000);
    blockHeightField->setSuffix(" (Y)");
    blockHeightField->setValue(CSettings::instance()->getVCStacksHeight());
    blockDepthField = new QSpinBox();
    blockDepthField->setAlignment(Qt::AlignCenter);
    blockDepthField->setMinimum(1);
    blockDepthField->setMaximum(100000);
    blockDepthField->setSuffix(" (Z)");
    blockDepthField->setValue(CSettings::instance()->getVCStacksDepth());
    memoryField = new QLabel();
    memoryField->setAlignment(Qt::AlignLeft);
    downsamplingCbox = new QComboBox(this);
    downsamplingCbox->addItem(QString("Mean (2").append(QChar(0x00D7)).append("2").append(QChar(0x00D7)).append("2)"));
    downsamplingCbox->addItem(QString("Max  (2").append(QChar(0x00D7)).append("2").append(QChar(0x00D7)).append("2)"));
    downsamplingCbox->setCurrentIndex(1);

    //conversion form layout
    QVBoxLayout* conversionFormLayout = new QVBoxLayout();
    outButtonLayout = new QStackedLayout();
    outButtonLayout->addWidget(outDirButton);
    outButtonLayout->addWidget(outFileButton);
    QHBoxLayout* outputFormLayout = new QHBoxLayout();
    outputFormLayout->addWidget(outFormatCBox);
    outFormatCBox->setFixedWidth(220);
    outputFormLayout->addWidget(outPathField, 1);
    outputFormLayout->addLayout(outButtonLayout);
    conversionFormLayout->addLayout(outputFormLayout);
    conversionFormLayout->addSpacing(20);
    QLabel* outputLabel = new QLabel("Resolutions:");
    outputLabel->setFixedWidth(220);
    QLabel* sizeLabel = new QLabel("Size (GB)");
    sizeLabel->setAlignment(Qt::AlignCenter);


    QHBoxLayout* resolutionsHeaderLayout = new QHBoxLayout();
    resolutionsHeaderLayout->addWidget(outputLabel, 0);
    resolutionsHeaderLayout->addSpacing(15);
    QLabel* labelX = new QLabel("X");
    labelX->setAlignment(Qt::AlignCenter);
    labelX->setFixedWidth(80);
    resolutionsHeaderLayout->addWidget(labelX, 0);
    QLabel* labelY = new QLabel("Y");
    labelY->setAlignment(Qt::AlignCenter);
    labelY->setFixedWidth(80);
    resolutionsHeaderLayout->addWidget(labelY, 0);
    QLabel* labelZ = new QLabel("Z");
    labelZ->setAlignment(Qt::AlignCenter);
    labelZ->setFixedWidth(80);
    resolutionsHeaderLayout->addWidget(labelZ, 0);
    QLabel* labelC = new QLabel("CH");
    labelC->setAlignment(Qt::AlignCenter);
    labelC->setFixedWidth(65);
    resolutionsHeaderLayout->addWidget(labelC, 0);
    QLabel* labelT = new QLabel("t");
    labelT->setAlignment(Qt::AlignCenter);
    labelT->setFixedWidth(65);
    resolutionsHeaderLayout->addWidget(labelT, 0);
    resolutionsHeaderLayout->addWidget(sizeLabel, 1);

    resolutionsLayout->addLayout(resolutionsHeaderLayout);
    conversionFormLayout->addLayout(resolutionsLayout, 0);

    QHBoxLayout *addResolutionLayout = new QHBoxLayout();
    QLabel* arlLabel = new QLabel();
    arlLabel->setFixedWidth(220);
    addResolutionLayout->addWidget(arlLabel);
    addResolutionLayout->addWidget(addResolutionButton, 0);
    conversionFormLayout->addLayout(addResolutionLayout, 0);
    conversionFormLayout->addSpacing(20);

    QHBoxLayout* stacksDimLayout = new QHBoxLayout();
    QLabel* stacksDimLabel = new QLabel("Tile dims:");
    stacksDimLabel->setFixedWidth(220);
    stacksDimLayout->addWidget(stacksDimLabel);
    stacksDimLayout->addWidget(blockWidthField, 1);
    stacksDimLayout->addSpacing(15);
    stacksDimLayout->addWidget(blockHeightField, 1);
    stacksDimLayout->addSpacing(15);
    stacksDimLayout->addWidget(blockDepthField, 1);
    conversionFormLayout->addLayout(stacksDimLayout);

    QHBoxLayout* downSampleMethLayout = new QHBoxLayout();
    QLabel* downSampleMethLabel = new QLabel("Downsampling method:");
    downSampleMethLabel->setFixedWidth(220);
    downSampleMethLayout->addWidget(downSampleMethLabel);
    downSampleMethLayout->addWidget(downsamplingCbox, 0, Qt::AlignLeft);
    downsamplingCbox->setFixedWidth(160);
    downSampleMethLayout->addStretch(1);

    QHBoxLayout* ramLayout = new QHBoxLayout();
    QLabel* memoryLabel = new QLabel("Estimated RAM usage:");
    memoryLabel->setFixedWidth(220);
    ramLayout->addWidget(memoryLabel);
    ramLayout->addWidget(memoryField, 0, Qt::AlignLeft);
    ramLayout->addStretch(1);
    conversionFormLayout->addSpacing(10);
    conversionFormLayout->addLayout(downSampleMethLayout);
    conversionFormLayout->addSpacing(10);
    conversionFormLayout->addLayout(ramLayout);
    conversion_panel->setLayout(conversionFormLayout);
    conversion_panel->setEnabled(false);


    //overall layout
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(import_panel);
    layout->addSpacing(10);
    layout->addWidget(conversion_panel);
    layout->addSpacing(5);
    layout->addWidget(helpBox);
    layout->addStretch(1);
    layout->addSpacing(10);
    layout->addWidget(statusBar);
    QWidget* container = new QWidget();
    QGridLayout* bottomBar = new QGridLayout();
    bottomBar->addWidget(progressBar, 0,0,1,18);
    bottomBar->addWidget(startButton, 0,18,1,1);
    bottomBar->addWidget(stopButton, 0,19,1,1);
    bottomBar->setContentsMargins(0,0,0,5);
    container->setLayout(bottomBar);
    layout->addWidget(container);    
    layout->setSpacing(0);
    layout->setContentsMargins(10,5,10,0);
    setLayout(layout);
    setWindowTitle(QString("TeraConverter v").append(terafly::version.c_str()));
    this->setFixedWidth(800);

    //signals and slots
    connect(startButton, SIGNAL(clicked()), this, SLOT(startButtonClicked()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
    qRegisterMetaType<std::string>("std::string");
    connect(ts::QProgressSender::instance(), SIGNAL(sendProgressBarChanged(int, int, int, std::string)), this, SLOT(progressBarChanged(int, int, int, std::string)), Qt::QueuedConnection);
    connect(inFormatCBox, SIGNAL(currentIndexChanged(int)), this, SLOT(volformatChanged(int)));
    connect(outFormatCBox, SIGNAL(currentIndexChanged(int)), this, SLOT(volformatChanged(int)));
    connect(timeSeriesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(volformatChanged(int)));
    connect(CConverter::instance(), SIGNAL(sendOperationOutcome(tf::RuntimeException*)), this, SLOT(operationDone(tf::RuntimeException*)), Qt::QueuedConnection);
    connect(inDirButton, SIGNAL(clicked()), this, SLOT(inDirButtonClicked()));
    connect(inFileButton, SIGNAL(clicked()), this, SLOT(inFileButtonClicked()));
    connect(outFileButton, SIGNAL(clicked()), this, SLOT(outFileButtonClicked()));
    connect(outDirButton, SIGNAL(clicked()), this, SLOT(outDirButtonClicked()));
    connect(inPathField, SIGNAL(textChanged(QString)), this, SLOT(settingsChanged()));
    connect(inPathField, SIGNAL(editingFinished()), this, SLOT(startButtonClicked()));
    connect(outPathField, SIGNAL(textChanged(QString)), this, SLOT(settingsChanged()));
    connect(inFormatCBox, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()));
    connect(outFormatCBox, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsChanged()));
    connect(blockWidthField, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    connect(blockHeightField, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    connect(blockDepthField, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    connect(addResolutionButton, SIGNAL(clicked()), this, SLOT(addResolution()));

    terastitcher::ProgressBar::instance()->setToGUI(true);

    resetGUI();

    //set always on top
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->setFocusPolicy(Qt::StrongFocus);

    // instance PLog
    PLog::instance(this);

    /**/tf::debug(tf::LEV1, "object successfully created", __itm__current__function__);
}

PConverter::~PConverter()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);
}

//reset GUI method
void PConverter::resetGUI()
{
    progressBar->setEnabled(false);
    progressBar->setMaximum(1);         //needed to stop animation on some operating systems
    startButton->setEnabled(true);
    stopButton->setEnabled(false);    
    statusBar->clearMessage();
    statusBar->showMessage("Ready to import volume.");
    volformatChanged(0);
    import_panel->setEnabled(true);
    operationInProgress = false;
}

//called when startButton has been clicked
void PConverter::startButtonClicked()
{
    if(operationInProgress)
        return;
    operationInProgress = true;

    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    try
    {
        CConverter::instance()->setMembers(this);
        import_panel->setEnabled(false);
        conversion_panel->setEnabled(false);
        progressBar->setEnabled(true);
        startButton->setEnabled(false);
        stopButton->setEnabled(true);
        if(!CConverter::instance()->isConversionModeEnabled())
        {
            statusBar->clearMessage();
            statusBar->showMessage("Importing volume...");
            progressBar->setMinimum(0);
            progressBar->setMaximum(0);
        }
        else
        {
            bool filemode = outButtonLayout->currentWidget() == outFileButton;

            //if destination dir does not exist, asking for creation
            if(!filemode && !QFile::exists(outPathField->text()))
            {
                if(!QMessageBox::information(this, "Create dir?", "The directory you selected does not exist. Create it?", "Yes", "Cancel"))
                    QDir().mkdir(outPathField->text());
                else
                {
                    statusBar->clearMessage();
                    statusBar->showMessage("Ready to convert volume.");
                    progressBar->setEnabled(false);
                    progressBar->setMaximum(1);         //needed to stop animation on some operating systems
                    startButton->setEnabled(true);
                    stopButton->setEnabled(false);
                    import_panel->setEnabled(false);
                    conversion_panel->setEnabled(true);
                    return;
                }
            }


            //checking destination dir is empty
            if(!filemode)
            {
                QDir directory(outPathField->text());
                QStringList dir_entries = directory.entryList();
                if(dir_entries.size() > 2 && QMessageBox::information(this, "Warning", "The directory you selected is NOT empty. \n\nIf you continue, the conversion "
                                                       "process could fail if the directories to be created already exist in the given path.", "Continue", "Cancel"))
                {
                    statusBar->clearMessage();
                    statusBar->showMessage("Ready to convert volume.");
                    progressBar->setEnabled(false);
                    progressBar->setMaximum(1);         //needed to stop animation on some operating systems
                    startButton->setEnabled(true);
                    stopButton->setEnabled(false);
                    import_panel->setEnabled(false);
                    conversion_panel->setEnabled(true);
                    return;
                }
            }
            progressBar->setMinimum(0);
            progressBar->setMaximum(100);
        }
        //PLog::instance()->show();
        timer.restart();
        CConverter::instance()->start();
    }
    catch(RuntimeException &ex) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));}
}

//called when stopButton has been clicked
void PConverter::stopButtonClicked()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    if(QMessageBox::information(this, "Warning", "Terminating this step can be unsafe and cause Vaa3D to crash. \n"
                                              "\nPlease save your data first or click on \"Cancel\" and close the "
                                              "plugin to terminate safely this process.", "Continue", "Cancel"))
        return;
    else
    {
        if(!CConverter::instance()->isConversionModeEnabled())
            this->resetGUI();
        else
        {
            statusBar->clearMessage();
            statusBar->showMessage("Ready to convert volume.");
            progressBar->setEnabled(false);
            progressBar->setMaximum(1);         //needed to stop animation on some operating systems
            startButton->setEnabled(true);
            stopButton->setEnabled(false);
            import_panel->setEnabled(false);
            conversion_panel->setEnabled(true);
            operationInProgress = false;
        }
    }
}

void PConverter::inDirButtonClicked()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    #ifdef _USE_QT_DIALOGS
    QString path;
    QFileDialog dialog(0);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    dialog.setWindowTitle("Select volume's folder");
    dialog.setDirectory(CSettings::instance()->getVCInputPath().c_str());
    if(dialog.exec())
        path = dialog.directory().absolutePath();

    #else
    QString path = QFileDialog::getExistingDirectory(this, "Select volume's folder", CSettings::instance()->getVCInputPath().c_str(), QFileDialog::ShowDirsOnly);
    #endif

    if(!path.isEmpty())
    {
        inPathField->setText(path);

        //launching import
        startButtonClicked();
    }
}

void PConverter::inFileButtonClicked()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    std::string namefilter;
    if(inFormatCBox->currentText().compare(iim::TIF3D_FORMAT.c_str()) == 0)
        namefilter = "TIFF files (*.tif *.TIF *.tiff *.TIFF)";
    else if(inFormatCBox->currentText().compare(iim::RAW_FORMAT.c_str()) == 0)
        namefilter = "V3D raw files (*.raw *.RAW *.v3draw *.V3DRAW)";
    else if(inFormatCBox->currentText().compare(iim::BDV_HDF5_FORMAT.c_str()) == 0)
        namefilter = "HDF5 files (*.h5)";

    #ifdef _USE_QT_DIALOGS
    QString path = "";
    QFileDialog dialog(0);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    dialog.setWindowTitle("Select volume's file");
    dialog.setNameFilter(namefilter.c_str());
    dialog.setDirectory(CSettings::instance()->getVCInputPath().c_str());
    if(dialog.exec())
       if(!dialog.selectedFiles().empty())
           path = dialog.selectedFiles().front();

    #else
    QString path = QFileDialog::getOpenFileName(this, "Select volume's file", CSettings::instance()->getVCInputPath().c_str(), namefilter.c_str());
    #endif

    if(!path.isEmpty())
    {
        inPathField->setText(path);

        //launching import
        startButtonClicked();
    }
}

void PConverter::outFileButtonClicked()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    #ifdef _USE_QT_DIALOGS
    QString path;
    QFileDialog dialog(0);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    dialog.setNameFilter(tr("HDF5 file (*.h5)"));
    dialog.setWindowTitle("Select the output file");
    dialog.setDirectory(CSettings::instance()->getVCOutputPath().c_str());
    if(dialog.exec())
        path = dialog.directory().absolutePath();

    #else
    QString path = QFileDialog::getSaveFileName(this, "Select output file", tf::cdUp(CSettings::instance()->getVCOutputPath()).c_str(), tr("HDF5 file (*.h5)"));
    //QString path = QFileDialog::getExistingDirectory(this, "Select the directory where the converted volume has to be stored", CSettings::instance()->getVCOutputPath().c_str(), QFileDialog::ShowDirsOnly);
    #endif

    if(!path.isEmpty())
        outPathField->setText(path);
}

void PConverter::outDirButtonClicked()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    #ifdef _USE_QT_DIALOGS
    QString path;
    QFileDialog dialog(0);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    dialog.setWindowTitle("Select the directory where the converted volume has to be stored");
    dialog.setDirectory(CSettings::instance()->getVCOutputPath().c_str());
    if(dialog.exec())
        path = dialog.directory().absolutePath();

    #else
    QString path = QFileDialog::getExistingDirectory(this, "Select the directory where the converted volume has to be stored", CSettings::instance()->getVCOutputPath().c_str(), QFileDialog::ShowDirsOnly);
    #endif

    if(!path.isEmpty())
        outPathField->setText(path);
}

/**********************************************************************************
* Called when the GUI widgets that control application settings change.
* This is used to manage persistent platform-independent application settings.
***********************************************************************************/
void PConverter::settingsChanged()
{
    CSettings::instance()->setVCInputPath(inPathField->text().toStdString());
    CSettings::instance()->setVCOutputPath(outPathField->text().toStdString());
    CSettings::instance()->setVCInputFormat(inFormatCBox->currentText().toStdString());
    CSettings::instance()->setVCOutputFormat(outFormatCBox->currentText().toStdString());
    CSettings::instance()->setVCStacksWidth(blockWidthField->value());
    CSettings::instance()->setVCStacksHeight(blockHeightField->value());
    CSettings::instance()->setVCStacksDepth(blockDepthField->value());
    CSettings::instance()->setVCTimeSeries(timeSeriesCheckBox->isChecked());
}

void PConverter::volformatChanged (int )
{
    QComboBox* sender = static_cast<QComboBox*>(QObject::sender());
    QStackedLayout *buttonLayout = 0;
    QPushButton *dirButton = 0;
    QPushButton *fileButton = 0;
    if(sender == outFormatCBox)
    {
        buttonLayout = outButtonLayout;
        dirButton    = outDirButton;
        fileButton   = outFileButton;
    }
    else // both inFormatCBox and timeSeriesCheckBox
    {
        sender = inFormatCBox;
        buttonLayout = inButtonLayout;
        dirButton    = inDirButton;
        fileButton   = inFileButton;
    }

    if(sender->currentText().compare(iim::STACKED_FORMAT.c_str(), Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("Three-leveled (y,x,z) hierarchy of tiles with each tile composed by a series of 2D TIFF files.");
        buttonLayout->setCurrentWidget(dirButton);

        if(sender == outFormatCBox)
            blockDepthField->setVisible(false);
    }
    else if(sender->currentText().compare(iim::SIMPLE_FORMAT.c_str(), Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("A folder containing a series (1+) of 2D TIFF files");
        buttonLayout->setCurrentWidget(dirButton);

        if(sender == outFormatCBox)
            blockDepthField->setVisible(false);
    }
    else if(sender->currentText().compare(iim::TILED_FORMAT.c_str(), Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("Three-leveled (y,x,z) hierarchy of tiles with each tile composed by a series of Vaa3D 3D raw files.");
        buttonLayout->setCurrentWidget(dirButton);

        if(sender == outFormatCBox)
            blockDepthField->setVisible(true);
    }
    else if(sender->currentText().compare(iim::TILED_MC_FORMAT.c_str(), Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("Four-leveled (c,y,x,z) hierarchy of tiles with each tile composed by a series of Vaa3D 3D raw files.");
        buttonLayout->setCurrentWidget(dirButton);

        if(sender == outFormatCBox)
            blockDepthField->setVisible(true);
    }
    else if(sender->currentText().compare(iim::RAW_FORMAT.c_str(), Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("Single Vaa3D 3D raw file");
        if(timeSeriesCheckBox->isChecked())
            buttonLayout->setCurrentWidget(dirButton);
        else
            buttonLayout->setCurrentWidget(fileButton);

        if(sender == outFormatCBox)
            blockDepthField->setVisible(false);
    }
    else if(sender->currentText().compare(iim::SIMPLE_RAW_FORMAT.c_str(), Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("A folder containing a series (1+) of Vaa3D 2D raw files.");
        buttonLayout->setCurrentWidget(dirButton);

        if(sender == outFormatCBox)
            blockDepthField->setVisible(false);
    }
    else if(sender->currentText().compare(iim::TILED_TIF3D_FORMAT.c_str(), Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("Three-leveled (y,x,z) hierarchy of tiles with each tile composed by a series of multipage (3D) TIFF files.");
        buttonLayout->setCurrentWidget(dirButton);

        if(sender == outFormatCBox)
            blockDepthField->setVisible(true);
    }
    else if(sender->currentText().compare(iim::TILED_MC_TIF3D_FORMAT.c_str(), Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("Four-leveled (c,y,x,z) hierarchy of tiles with each tile composed by a series of multipage (3D) TIFF files.");
        buttonLayout->setCurrentWidget(dirButton);

        if(sender == outFormatCBox)
            blockDepthField->setVisible(true);
    }
    else if(sender->currentText().compare(iim::TIF3D_FORMAT.c_str(), Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("A single multipage (3D) TIFF file.");
        if(timeSeriesCheckBox->isChecked())
            buttonLayout->setCurrentWidget(dirButton);
        else
            buttonLayout->setCurrentWidget(fileButton);

        if(sender == outFormatCBox)
            blockDepthField->setVisible(false);
    }
    else if(sender->currentText().compare(iim::BDV_HDF5_FORMAT.c_str(), Qt::CaseInsensitive) == 0)
    {
        helpBox->setText("BigDataViewer HDF5 format (see <a href=\"http://fiji.sc/BigDataViewer#About_the_BigDataViewer_data_format\">this</a> link)");
        buttonLayout->setCurrentWidget(fileButton);

        if(sender == outFormatCBox)
            blockDepthField->setVisible(true);
    }
    else
        helpBox->setText("<html><p style=\"text-align:justify;\"> Format not yet supported. </p></html>");
}

//overrides closeEvent method of QWidget
void PConverter::closeEvent(QCloseEvent *evt)
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    if(progressBar->isEnabled() && QMessageBox::information(this, "Warning", "An operation is still in progress. Terminating it can be unsafe and cause Vaa3D to crash. \n"
                                                                    "\nPlease save your data first.", "Close TeraConverter plugin", "Cancel"))
    {
        evt->ignore();
    }
    else
    {
        evt->accept();
        PConverter::uninstance();
    }
}

/**********************************************************************************
* <sendProgressBarChanged> event handler
***********************************************************************************/
void PConverter::progressBarChanged(int val, int minutes, int seconds, std::string message)
{
    progressBar->setValue(val);
    QString remaining_time = QString::number(minutes);
    remaining_time.append(" minutes and ");
    remaining_time.append(QString::number(seconds));
    remaining_time.append(" seconds remaining");
    if(message.size())
        statusBar->showMessage(message.c_str() + QString(": ") + remaining_time);
    else
        statusBar->showMessage(remaining_time);
}

/**********************************************************************************
* Called by <CConverter> when the associated operation has been performed.
* If an exception has occurred in the <CConverter> thread, it is propagated and
* managed in the current thread (ex != 0).
***********************************************************************************/
void PConverter::operationDone(RuntimeException *ex)
{
    /**/tf::debug(tf::LEV1, strprintf("ex = %s", (ex? "error" : "0")).c_str(), __itm__current__function__);


    //if an exception has occurred, showing a message error
    if(ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
        if(!CConverter::instance()->isConversionModeEnabled())
            this->resetGUI();
        else
        {
            statusBar->clearMessage();
            statusBar->showMessage("Ready to convert volume.");
            progressBar->setEnabled(false);
            progressBar->setMaximum(1);         //needed to stop animation on some operating systems
            startButton->setEnabled(true);
            stopButton->setEnabled(false);
            import_panel->setEnabled(false);
            conversion_panel->setEnabled(true);
        }
    }
    else if(CConverter::instance()->isConversionModeEnabled())
    {
//        statusBar->clearMessage();
//        statusBar->showMessage("Conversion successfully performed!");
//        progressBar->setEnabled(false);
//        progressBar->setMaximum(1);         //needed to stop animation on some operating systems
//        startButton->setEnabled(false);
//        stopButton->setEnabled(false);
        double elapsed_mm = timer.elapsed()/(1000*60.0);
        QMessageBox::information(this, "Success!", tf::strprintf("Conversion successfully done in %.1f minutes!", elapsed_mm).c_str());
        //close();

        statusBar->clearMessage();
        statusBar->showMessage("Ready to convert volume.");
        progressBar->setEnabled(false);
        progressBar->setMaximum(1);         //needed to stop animation on some operating systems
        startButton->setEnabled(true);
        stopButton->setEnabled(false);
        import_panel->setEnabled(false);
        conversion_panel->setEnabled(true);
    }
    else
    {
        statusBar->clearMessage();
        statusBar->showMessage("Ready to convert volume.");
        progressBar->setEnabled(false);
        progressBar->setMaximum(1);         //needed to stop animation on some operating systems
        startButton->setEnabled(true);
        stopButton->setEnabled(false);
        conversion_panel->setEnabled(true);

        // refresh outFormatCBox
        int idx = outFormatCBox->currentIndex();
        outFormatCBox->setCurrentIndex(0);
        outFormatCBox->setCurrentIndex(idx);

        try
        {
            //generating at runtime the needed resolutions, which depend on the current volume size
            VolumeConverter *vc = CConverter::instance()->getVolumeConverter(); //throws an exception if volume has not been imported yet!
            int res = 1;
            bool add_a_resolution = true;
            while(add_a_resolution)
            {
                int height = (vc->getV1()-vc->getV0())/pow(2.0f, res);
                int width = (vc->getH1()-vc->getH0())/pow(2.0f, res);
                int depth = (vc->getD1()-vc->getD0())/pow(2.0f, res);

                add_a_resolution = height > CSettings::instance()->getVOIdimV() ||
                                   width  > CSettings::instance()->getVOIdimH() ||
                                   depth  > CSettings::instance()->getVOIdimD();

                add_a_resolution =  add_a_resolution && height > 10 && width > 10 && depth > 10;

                if(add_a_resolution)
                    res++;
            }
            for(int i=0; i<=res; i++)
                addResolution();
        }
        catch(RuntimeException &ex) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));}
    }
    operationInProgress = false;
}

/**********************************************************************************
* Updates widgets contents
***********************************************************************************/
void PConverter::updateContent()
{
    /**/tf::debug(tf::LEV3, 0, __itm__current__function__);

    try
    {
        //checking that at least one resolution has been selected
        bool selected = false;
        for(int i=0; i<resolutionsCboxs.size(); i++)
        {
            selected = selected || resolutionsCboxs[i]->isChecked();
            resolutionsDimsX[i]->setEnabled(resolutionsCboxs[i]->isChecked());
            resolutionsDimsY[i]->setEnabled(resolutionsCboxs[i]->isChecked());
            resolutionsDimsZ[i]->setEnabled(resolutionsCboxs[i]->isChecked());
            resolutionsDimsC[i]->setEnabled(resolutionsCboxs[i]->isChecked());
            resolutionsDimsT[i]->setEnabled(resolutionsCboxs[i]->isChecked() && timeSeriesCheckBox->isChecked());
            resolutionsSizes[i]->setEnabled(resolutionsCboxs[i]->isChecked());
        }
        if(!selected)
        {
            QMessageBox::warning(this,QObject::tr("Warning"), "At least one resolution must be selected",QObject::tr("Ok"));
            resolutionsCboxs[0]->setChecked(true);
            updateContent();
        }

        //determining the maximum resolution selected
        VolumeConverter *vc = CConverter::instance()->getVolumeConverter(); //throws an exception if volume has not been imported yet!
        int max_res = 0;
        for(int i=0; i<resolutionsCboxs.size(); i++)
        {
            if(resolutionsCboxs[i]->isChecked())
                max_res = std::max(max_res, i);
        }

        //updating RAM usage estimation
        int layer_height = vc->getV1()-vc->getV0();
        int layer_width = vc->getH1()-vc->getH0();
        int layer_depth = pow(2.0f, max_res);
        float GBytes = (layer_height/1000.0f)*(layer_width/1000.0f)*(layer_depth/1000.0f)*vc->getVolume()->getDIM_C()*vc->getVolume()->getBYTESxCHAN();
        memoryField->setText(QString::number(GBytes, 'f', 3).append(" GB"));
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}


void PConverter::addResolution()
{
    /**/tf::debug(tf::LEV3, 0, __itm__current__function__);

    try
    {
        VolumeConverter *vc = CConverter::instance()->getVolumeConverter(); //throws an exception if volume has not been imported yet!

        int i = static_cast<int>(resolutionsCboxs.size());
        int height = (vc->getV1()-vc->getV0())/pow(2.0f, i);
        int width = (vc->getH1()-vc->getH0())/pow(2.0f, i);
        int depth = (vc->getD1()-vc->getD0())/pow(2.0f, i);

        if(height < 2 ||
           width  < 2 ||
           depth  < 2 ||
           i >= iim::TMITREE_MAX_HEIGHT)
            return;

        float GBytes = (height/1000.0f)*(width/1000.0f)*(depth/1000.0f)*vc->getVolume()->getDIM_C()*vc->getVolume()->getDIM_T()*vc->getVolume()->getBYTESxCHAN();

        resolutionsDimsX.push_back(new QSpinBox());
        resolutionsDimsX[i]->setAlignment(Qt::AlignCenter);
        resolutionsDimsX[i]->setReadOnly(true);
        resolutionsDimsX[i]->setMinimum(width);
        resolutionsDimsX[i]->setMaximum(width);
        resolutionsDimsX[i]->setValue(width);
        resolutionsDimsX[i]->setFixedWidth(80);

        resolutionsDimsY.push_back(new QSpinBox());
        resolutionsDimsY[i]->setAlignment(Qt::AlignCenter);
        resolutionsDimsY[i]->setReadOnly(true);
        resolutionsDimsY[i]->setMinimum(height);
        resolutionsDimsY[i]->setMaximum(height);
        resolutionsDimsY[i]->setValue(height);
        resolutionsDimsY[i]->setFixedWidth(80);


        resolutionsDimsZ.push_back(new QSpinBox());
        resolutionsDimsZ[i]->setAlignment(Qt::AlignCenter);
        resolutionsDimsZ[i]->setReadOnly(true);
        resolutionsDimsZ[i]->setMinimum(depth);
        resolutionsDimsZ[i]->setMaximum(depth);
        resolutionsDimsZ[i]->setValue(depth);
        resolutionsDimsZ[i]->setFixedWidth(80);

        resolutionsDimsC.push_back(new QSpinBox());
        resolutionsDimsC[i]->setAlignment(Qt::AlignCenter);
        resolutionsDimsC[i]->setReadOnly(true);
        resolutionsDimsC[i]->setMinimum(vc->getVolume()->getDIM_C());
        resolutionsDimsC[i]->setMaximum(vc->getVolume()->getDIM_C());
        resolutionsDimsC[i]->setValue(vc->getVolume()->getDIM_C());
        resolutionsDimsC[i]->setFixedWidth(65);

        resolutionsDimsT.push_back(new QSpinBox());
        resolutionsDimsT[i]->setAlignment(Qt::AlignCenter);
        resolutionsDimsT[i]->setReadOnly(true);
        resolutionsDimsT[i]->setMinimum(vc->getVolume()->getDIM_T());
        resolutionsDimsT[i]->setMaximum(vc->getVolume()->getDIM_T());
        resolutionsDimsT[i]->setValue(vc->getVolume()->getDIM_T());
        resolutionsDimsT[i]->setFixedWidth(65);
        resolutionsDimsT[i]->setEnabled(timeSeriesCheckBox->isChecked());


        resolutionsSizes.push_back(new QLineEdit(QString::number(GBytes,'f',3)));
        resolutionsSizes[i]->setAlignment(Qt::AlignCenter);
        resolutionsSizes[i]->setReadOnly(true);

        resolutionsCboxs.push_back(new QCheckBox());
        resolutionsCboxs[i]->setStyleSheet("::indicator {subcontrol-position: center; subcontrol-origin: padding;}");
        resolutionsCboxs[i]->setChecked(true);
        resolutionsCboxs[i]->setFixedWidth(15);

        QHBoxLayout *resolutionsRowiLayout = new QHBoxLayout();

        QLabel* phantomLabel = new QLabel();
        phantomLabel->setFixedWidth(220);
        resolutionsRowiLayout->addWidget(phantomLabel);
        resolutionsRowiLayout->addWidget(resolutionsCboxs[i], 0);
        resolutionsRowiLayout->addWidget(resolutionsDimsX[i], 0);
        resolutionsRowiLayout->addWidget(resolutionsDimsY[i], 0);
        resolutionsRowiLayout->addWidget(resolutionsDimsZ[i], 0);
        resolutionsRowiLayout->addWidget(resolutionsDimsC[i], 0);
        resolutionsRowiLayout->addWidget(resolutionsDimsT[i], 0);
        resolutionsRowiLayout->addWidget(resolutionsSizes[i], 1);
        resolutionsLayout->addLayout(resolutionsRowiLayout, 0);
        resolutionsRowsLayouts.push_back(resolutionsRowiLayout);

        connect(resolutionsCboxs[i], SIGNAL(stateChanged(int)), this, SLOT(updateContent()));

        //updating content
        updateContent();
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}
