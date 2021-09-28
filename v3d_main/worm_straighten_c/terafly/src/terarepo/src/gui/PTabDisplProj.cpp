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

#include "PTabDisplProj.h"
#include "PTabDisplThresh.h"
#include "PTabMergeTiles.h"
#include "PTabPlaceTiles.h"
#include "CImportUnstitched.h"
#include "iomanager.config.h"
#include "vmStackedVolume.h"
#include "PTeraStitcher.h"
#include "StackStitcher.h"

using namespace terastitcher;

/*********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
**********************************************************************************/
PTabDisplProj* PTabDisplProj::uniqueInstance = NULL;
void PTabDisplProj::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

PTabDisplProj::PTabDisplProj(QMyTabWidget* _container, int _tab_index) : QWidget(), container(_container), tab_index(_tab_index)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplProj created\n", this->thread()->currentThreadId());
    #endif

    //other widgets
    saveproj_label = new QLabel("Save project XML to:");
    saveproj_field = new QLineEdit();
    //saveproj_field->setFont(QFont("",8));
    browse_button = new QPushButton("...");
    total_displ_number_label = new QLabel("Displacements:");
    total_displ_number_field = new QLineEdit();
    total_displ_number_field->setReadOnly(true);
    total_displ_number_field->setAlignment(Qt::AlignCenter);
    per_stack_displ_number_label = new QLabel("Per-pair displacements:");
    per_stack_displ_number_field = new QLineEdit();
    per_stack_displ_number_field->setReadOnly(true);
    per_stack_displ_number_field->setAlignment(Qt::AlignCenter);

    /*** LAYOUT SECTIONS ***/

    //widgets
    QGridLayout* gridlayout = new QGridLayout();
    saveproj_label->setFixedWidth(200);
    browse_button->setFixedWidth(80);
    gridlayout->addWidget(saveproj_label, 0, 0, 1, 1);
    gridlayout->addWidget(saveproj_field, 0, 1, 1, 10);
    gridlayout->addWidget(browse_button, 0, 11, 1, 1);
    gridlayout->addWidget(total_displ_number_label, 1, 0, 1, 1);
    gridlayout->addWidget(total_displ_number_field, 1, 1, 1, 3);
    gridlayout->addWidget(per_stack_displ_number_label, 2, 0, 1, 1);
    gridlayout->addWidget(per_stack_displ_number_field, 2, 1, 1, 3);
    gridlayout->setVerticalSpacing(2);
    gridlayout->setContentsMargins(10,0,10,0);
    QWidget *container = new QWidget();
    container->setLayout(gridlayout);

    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);
    layout->addWidget(container);
    setLayout(layout);

    //wait animated GIF tab icon
    wait_movie = new QMovie(":/icons/wait.gif");
    wait_label = new QLabel(this);
    wait_label->setMovie(wait_movie);

    // signals and slots    
    connect(browse_button, SIGNAL(clicked()), this, SLOT(browse_button_clicked()));

    reset();
}


PTabDisplProj::~PTabDisplProj()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplProj destroyed\n", this->thread()->currentThreadId());
    #endif
}

//reset method
void PTabDisplProj::reset()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplProj::reset()\n", this->thread()->currentThreadId());
    #endif

    total_displ_number_field->setText("");
    per_stack_displ_number_field->setText("");
    saveproj_field->setText("");
    setEnabled(false);
}

/*********************************************************************************
* Start/Stop methods associated to the current step.
* They are called by the startButtonClicked/stopButtonClicked methods of <PTeraStitcher>
**********************************************************************************/
void PTabDisplProj::start()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplProj start() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        //first checking that a volume has been properly imported
        if(!CImportUnstitched::instance()->getVolume())
            throw iom::exception("A volume must be properly imported first. Please perform the Import step.");

        //asking confirmation to continue if no displacements were found
        if(total_displ_number_field->text().toInt() == 0 &&
           QMessageBox::information(this, "Warning", "No computed displacements found. \n\nDisplacements will be generated using nominal stage coordinates.", "Continue", "Cancel"))
        {
            PTeraStitcher::instance()->setToReady();
            return;
        }

        //asking confirmation to continue when overwriting existing XML file
        if( vm::StackedVolume::fileExists(saveproj_field->text().toStdString().c_str()) &&
            PTeraStitcher::instance()->modeAdvancedAction->isChecked() &&
              QMessageBox::information(this, "Warning", "An XML file with the same name was found and it will be overwritten.", "Continue", "Cancel"))
        {
            PTeraStitcher::instance()->setToReady();
            return;
        }

        //disabling import form and enabling progress bar animation and tab wait animation
        PTeraStitcher::instance()->getProgressBar()->setEnabled(true);
        PTeraStitcher::instance()->getProgressBar()->setMinimum(0);
        PTeraStitcher::instance()->getProgressBar()->setMaximum(0);
        wait_movie->start();
        container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, wait_label);

        //performing operation
        vm::VirtualVolume* volume = CImportUnstitched::instance()->getVolume();
        if(!volume)
            throw iom::exception("Unable to start this step. A volume must be properly imported first.");
        StackStitcher stitcher(volume);
        stitcher.projectDisplacements();
        volume->saveXML(0, saveproj_field->text().toStdString().c_str());

        //showing operation successful message
        if(PTeraStitcher::instance()->modeAdvancedAction->isChecked())
            QMessageBox::information(this, "Operation successful", "Project step successfully performed!", QMessageBox::Ok);

        //enabling (and updating) other tabs
        this->setEnabled(true);
        PTabDisplThresh::getInstance()->setEnabled(true);
        PTabPlaceTiles::getInstance()->setEnabled(true);
        //PTabMergeTiles::getInstance()->setEnabled(true);

        stop();
    }
    catch(iom::exception &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PTeraStitcher::instance()->setToReady();
    }
}

void PTabDisplProj::stop()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplProj stop() launched\n", this->thread()->currentThreadId());
    #endif

    //re-enabling import form and disabling progress bar and wait animations
    PTeraStitcher::instance()->setToReady();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
}

/**********************************************************************************
* Opens file dialog to select the XML file to be saved.
* Called when user clicks on "browse_button".
***********************************************************************************/
void PTabDisplProj::browse_button_clicked()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabDisplProj browse_button_clicked() launched\n", this->thread()->currentThreadId());
    #endif

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter( tr("XML files (*.xml *.XML)") );
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDirectory(CImportUnstitched::instance()->getVolume()->getSTACKS_DIR());
    if (dialog.exec())
    {
        QStringList fileNames = dialog.selectedFiles();
        QString xmlpath = fileNames.first();
        if(!xmlpath.endsWith(".xml", Qt::CaseInsensitive))
            xmlpath.append(".xml");
        saveproj_field->setText(xmlpath);
    }
}

/**********************************************************************************
* Overrides QWidget's setEnabled(bool).
* If the widget is enabled, its fields are filled with the informations provided by
* the <StackedVolume> object of <CImport> instance.
***********************************************************************************/
void PTabDisplProj::setEnabled(bool enabled)
{
    /**/ts::debug(ts::LEV_MAX, strprintf("enabled = %s", enabled ? "true" : "false").c_str(), __tsp__current__function__);

    //then filling widget fields
    if(enabled && CImportUnstitched::instance()->getVolume())
    {
        int total;
        float per_stack;
        CImportUnstitched::instance()->getVolume()->countDisplacements(total, per_stack);
        total_displ_number_field->setText(QString::number(total));
        per_stack_displ_number_field->setText(QString::number(per_stack));

        if(!this->isEnabled())
        {
            QString saveproj_path = CImportUnstitched::instance()->getVolume()->getSTACKS_DIR();
            saveproj_path.append("/xml_displproj.xml");
            saveproj_field->setText(saveproj_path);
        }
    }

    //finally calling super-class implementation
    QWidget::setEnabled(enabled);
}
