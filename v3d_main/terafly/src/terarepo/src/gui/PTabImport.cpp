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

/******************
*    CHANGELOG    *
*******************
* 2018-01-25. Giulio.     @ADDED support for MultiVolume format
* 2016-05-03. Giulio.     @CHANGED ComboBox for selecting the reference system: eliminated configurations that are not supported
* 2017-04-30. Giulio.     @ADDED propagation to PTabDisplComp of all operation over the channel_selection combo box
* 2017-04-27. Giulio.     @ADDED code to get and initialize the input plugin form the xml if specified
*/

#include "PTabImport.h"
#include "iomanager.config.h"
#include "vmStackedVolume.h"
#include "vmBlockVolume.h"
#include "vmMCVolume.h"
#include "PTeraStitcher.h"
#include "CImportUnstitched.h"
#include "CPreview.h"
#include "PTabDisplComp.h"
#include "PTabDisplProj.h"
#include "PTabDisplThresh.h"
#include "PTabMergeTiles.h"
#include "PTabPlaceTiles.h"
#include "IOPluginAPI.h"
#include <QGridLayout>
#include <QMessageBox>

using namespace terastitcher;

/*********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
**********************************************************************************/
PTabImport* PTabImport::uniqueInstance = NULL;
void PTabImport::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

PTabImport::PTabImport(QMyTabWidget* _container, int _tab_index) : QWidget(), container(_container), tab_index(_tab_index)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport created\n", this->thread()->currentThreadId());
    #endif

    //import form widgets
#ifdef Q_OS_MAC
	QFont smallFont;
#else
	QFont smallFont;//("", 8);
#endif
    import_form = new QGroupBox("Import form");
    path_field    = new QPrefixSuffixLineEdit("Volume/XML path: ");
    path_field->setFont(smallFont);
    path_field->setTextMargins(5,0,0,0);
    voldir_button       = new QPushButton("Select folder...");
    projfile_button     = new QPushButton("Select XML...");
    regex_field = new QPrefixSuffixLineEdit(  "Filenames regular expression: ");
    regex_field->setFont(smallFont);
    regex_field->setTextMargins(5,0,0,0);
    connect(regex_field, SIGNAL(textChanged(QString)), this, SLOT(regexFieldChanged()));
    regex_field->installEventFilter(this);
    rescan_checkbox = new QCheckBox("Rescan files");
	rescan_checkbox->setToolTip("Rescan all files in the folder");
    rescan_checkbox->setFont(smallFont);
    imin_plugin_cbox = new QComboBox();
    imin_plugin_cbox->setFont(smallFont);
    std::vector<std::string> ioplugins = iom::IOPluginFactory::registeredPluginsList();
    imin_plugin_cbox->addItem("--- I/O plugin: ---");
    for(int i=0; i<ioplugins.size(); i++)
        imin_plugin_cbox->addItem(ioplugins[i].c_str());
    imin_plugin_cbox->addItem("MultiVolume");
    imin_plugin_cbox->setEditable(true);
    imin_plugin_cbox->lineEdit()->setReadOnly(true);
    imin_plugin_cbox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < imin_plugin_cbox->count(); i++)
        imin_plugin_cbox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    axs1_field = new QComboBox();
    axs1_field->addItem("--- first axis ---");
    axs1_field->addItem("Y");
    axs1_field->addItem("-Y");
    axs1_field->addItem("X");
    axs1_field->addItem("-X");
    //axs1_field->addItem("Z");
    //axs1_field->addItem("-Z");
    axs1_field->setEditable(true);
    axs1_field->lineEdit()->setReadOnly(true);
    axs1_field->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < axs1_field->count(); i++)
        axs1_field->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    axs1_field->setFont(smallFont);
    axs2_field = new QComboBox();
    axs2_field->addItem("--- second axis ---");
    axs2_field->addItem("X");
    axs2_field->addItem("-X");
    axs2_field->addItem("Y");
    axs2_field->addItem("-Y");
    //axs2_field->addItem("Z");
    //axs2_field->addItem("-Z");
    axs2_field->setEditable(true);
    axs2_field->lineEdit()->setReadOnly(true);
    axs2_field->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < axs2_field->count(); i++)
        axs2_field->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    axs2_field->setFont(smallFont);
    axs3_field = new QComboBox();
    axs3_field->setFont(smallFont);
    //axs3_field->addItem("--- third axis ---");
    //axs3_field->addItem("X");
    //axs3_field->addItem("-X");
    //axs3_field->addItem("Y");
    //axs3_field->addItem("-Y");
    axs3_field->addItem("Z");
    //axs3_field->addItem("-Z");
    axs3_field->setEditable(true);
    axs3_field->lineEdit()->setReadOnly(true);
    axs3_field->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < axs3_field->count(); i++)
        axs3_field->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    vxl1_field = new QDoubleSpinBox();
    vxl1_field->setAlignment(Qt::AlignCenter);
    vxl1_field->setPrefix(QString("voxel (").append(QChar(0x03BC)).append("m): "));
    vxl1_field->setFont(smallFont);
    vxl2_field = new QDoubleSpinBox();
    vxl2_field->setPrefix(QString("voxel (").append(QChar(0x03BC)).append("m): "));
    vxl2_field->setAlignment(Qt::AlignCenter);
    vxl2_field->setFont(QFont(smallFont));
    vxl3_field = new QDoubleSpinBox();
    vxl3_field->setPrefix(QString("voxel (").append(QChar(0x03BC)).append("m): "));
    vxl3_field->setAlignment(Qt::AlignCenter);
    vxl3_field->setFont(smallFont);
    axs1_field->setVisible(false);
    axs2_field->setVisible(false);
    axs3_field->setVisible(false);
    vxl1_field->setVisible(false);
    vxl2_field->setVisible(false);
    vxl3_field->setVisible(false);
	/*   vol_format_cbox = new QComboBox();
	vol_format_cbox->setFont(smallFont);
	vol_format_cbox->setEditable(true);
	vol_format_cbox->lineEdit()->setReadOnly(true);
	vol_format_cbox->lineEdit()->setAlignment(Qt::AlignCenter);
	vol_format_cbox->addItem("--- Volume format ---");
	std::vector <std::string> volformats = vm::VirtualVolumeFactory::registeredPluginsList();
	for(int i=0; i<volformats.size(); i++)
	vol_format_cbox->addItem(volformats[i].c_str());
	for(int i = 0; i < vol_format_cbox->count(); i++)
	vol_format_cbox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);*/
//    vol_format_cbox->setVisible(false);
    sparsedata_checkbox = new QCheckBox("Sparse data");
    sparsedata_checkbox->setFont(smallFont);
//    sparsedata_checkbox->setVisible(false);

    //info panel widgets
    info_panel = new QGroupBox("Volume's informations");
    info_panel->setEnabled(false);
    volumedir_label = new QLabel("Absolute path:");
    volumedir_field = new QLineEdit();
    volumedir_field->setReadOnly(true);
    volumedir_field->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    volume_dims_label = new QLabel("Number of tiles:");
    by_label_1 = new QLabel(QChar(0x00D7));
    by_label_1->setAlignment(Qt::AlignCenter);
    nrows_field = new QLineEdit();
    nrows_field->setReadOnly(true);
    nrows_field->setAlignment(Qt::AlignCenter);
    ncols_field = new QLineEdit();
    ncols_field->setReadOnly(true);
    ncols_field->setAlignment(Qt::AlignCenter);
    stacks_dims_label = new QLabel("Tile dimensions (voxels):");
    by_label_2 = new QLabel(QChar(0x00D7));
    by_label_3 = new QLabel(QChar(0x00D7));
    by_label_2->setAlignment(Qt::AlignCenter);
    by_label_3->setAlignment(Qt::AlignCenter);
    stack_height_field = new QLineEdit();
    stack_height_field->setReadOnly(true);
    stack_height_field->setAlignment(Qt::AlignCenter);
    stack_width_field = new QLineEdit();
    stack_width_field->setReadOnly(true);
    stack_width_field->setAlignment(Qt::AlignCenter);
    stack_depth_field = new QLineEdit();
    stack_depth_field->setReadOnly(true);
    stack_depth_field->setAlignment(Qt::AlignCenter);
    voxel_dims_label = new QLabel(QString("Voxel's dims (").append(QChar(0x03BC)).append("m):"));
    by_label_4 = new QLabel(QChar(0x00D7));
    by_label_5 = new QLabel(QChar(0x00D7));
    by_label_4->setAlignment(Qt::AlignCenter);
    by_label_5->setAlignment(Qt::AlignCenter);
    vxl_Y_field = new QLineEdit();
    vxl_Y_field->setReadOnly(true);
    vxl_Y_field->setAlignment(Qt::AlignCenter);
    vxl_X_field = new QLineEdit();
    vxl_X_field->setReadOnly(true);
    vxl_X_field->setAlignment(Qt::AlignCenter);
    vxl_Z_field = new QLineEdit();
    vxl_Z_field->setReadOnly(true);
    vxl_Z_field->setAlignment(Qt::AlignCenter);
    origin_label = new QLabel("Origin (mm):");
    org_Y_field = new QLineEdit();
    org_Y_field->setReadOnly(true);
    org_Y_field->setAlignment(Qt::AlignCenter);
    org_X_field = new QLineEdit();
    org_X_field->setReadOnly(true);
    org_X_field->setAlignment(Qt::AlignCenter);
    org_Z_field = new QLineEdit();
    org_Z_field->setReadOnly(true);
    org_Z_field->setAlignment(Qt::AlignCenter);
    stacks_overlap_label = new QLabel("Tile overlap (voxels):");
    ovp_Y_field = new QLineEdit();
    ovp_Y_field->setReadOnly(true);
    ovp_Y_field->setAlignment(Qt::AlignCenter);
    ovp_X_field = new QLineEdit();
    ovp_X_field->setReadOnly(true);
    ovp_X_field->setAlignment(Qt::AlignCenter);


    //preview panel
    slice_spinbox = new QSpinBox();
    slice_spinbox->setPrefix("Slice ");
    slice_spinbox->setAlignment(Qt::AlignCenter);
    
    // 2017-04-28. Giulio. @CHANGED made adaptable to the input plugin
    channel_selection = new QComboBox();
    channel_selection->addItem("unselected");
//     channel_selection->addItem("to gray");
//     channel_selection->addItem("R");
//     channel_selection->addItem("G");
//     channel_selection->addItem("B");
    channel_selection->setEditable(true);
    channel_selection->lineEdit()->setReadOnly(true);
    channel_selection->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < channel_selection->count(); i++)
        channel_selection->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    connect(channel_selection, SIGNAL(currentIndexChanged(int)),this, SLOT(channelSelectedChanged(int)));
    
    // 2017-04-28. Giulio. @CHANGED made not visible; used to set the depth of the preview directly from the imported volume
    imgdepth_cbox = new QComboBox();
    imgdepth_cbox->insertItem(0, "8 bits");
    imgdepth_cbox->insertItem(1, "16 bits");
    imgdepth_cbox->setEditable(true);
    imgdepth_cbox->lineEdit()->setReadOnly(true);
    imgdepth_cbox->lineEdit()->setAlignment(Qt::AlignCenter);
    for(int i = 0; i < imgdepth_cbox->count(); i++)
        imgdepth_cbox->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    imgdepth_cbox->setVisible(false);
    //imgdepth_cbox->setEditable(false);
    
    preview_button = new QPushButton(this);
    preview_button->setIcon(QIcon(":/icons/preview.png"));
    preview_button->setIconSize(QSize(20,20));
    preview_button->setText("Preview");

    /*** LAYOUT SECTIONS ***/
    // import form
    // import form: row 1
    QHBoxLayout* import_row_1 = new QHBoxLayout();
    path_field->setFixedWidth(420);
	import_row_1->addWidget(path_field);
	import_row_1->addSpacing(10);
	import_row_1->addWidget(imin_plugin_cbox);
	imin_plugin_cbox->setFixedWidth(140);
	//voldir_button->setFixedWidth(230);
	import_row_1->addSpacing(10);
    import_row_1->addWidget(voldir_button, 1);
    import_row_1->addWidget(projfile_button, 1);
	//import_row_1->setSpacing(10);
    // import form: row 2
    QHBoxLayout* import_row_2 = new QHBoxLayout();
    regex_field->setFixedWidth(420);
	import_row_2->addWidget(regex_field);
	import_row_2->setSpacing(0);
	import_row_2->addSpacing(10);
    //imin_plugin_cbox->setFixedWidth(230);
    import_row_2->addWidget(rescan_checkbox);
	import_row_2->addSpacing(10);
	import_row_2->addWidget(sparsedata_checkbox);
	import_row_2->addStretch(1);
    // import form - reimport panel
    QWidget* reimport_panel = new QWidget();
    QVBoxLayout* reimport_panel_layout = new QVBoxLayout();
    reimport_panel_layout->setContentsMargins(0,0,0,0);
    // import form - reimport panel - row 1
    QHBoxLayout* reimport_panel_layout_row1 = new QHBoxLayout();
    QWidget* axes_row = new QWidget();
    QHBoxLayout* axes_layout = new QHBoxLayout();
    axes_layout->setContentsMargins(0,0,0,0);
    axes_layout->addWidget(axs1_field, 1);
    axes_layout->addWidget(axs2_field, 1);
    axes_layout->addWidget(axs3_field, 1);
    axes_row->setLayout(axes_layout);
    axes_row->setFixedWidth(420);
    reimport_panel_layout_row1->addWidget(axes_row);
	reimport_panel_layout->addLayout(reimport_panel_layout_row1);
	reimport_panel_layout_row1->setAlignment(Qt::AlignLeft);
    // import form - reimport panel - row 2
    QHBoxLayout* reimport_panel_layout_row2 = new QHBoxLayout();
    QWidget* voxel_row = new QWidget();
    QHBoxLayout* voxel_layout = new QHBoxLayout();
    voxel_layout->setContentsMargins(0,0,0,0);
    voxel_layout->addWidget(vxl1_field, 1);
    voxel_layout->addWidget(vxl2_field, 1);
    voxel_layout->addWidget(vxl3_field, 1);
    voxel_row->setLayout(voxel_layout);
    voxel_row->setFixedWidth(420);
    reimport_panel_layout_row2->addWidget(voxel_row);
	reimport_panel_layout_row2->setAlignment(Qt::AlignLeft);
    //reimport_panel_layout_row2->addStretch(1);
    reimport_panel_layout->addLayout(reimport_panel_layout_row2);
    // import form - reimport panel - finalize
    reimport_panel->setLayout(reimport_panel_layout);
    // import form - finalize
    QVBoxLayout* import_form_layout = new QVBoxLayout();
	import_form_layout->setSpacing(5);
    import_form_layout->addLayout(import_row_1);
    import_form_layout->addLayout(import_row_2);
    import_form_layout->addWidget(reimport_panel,1);
    import_form->setLayout(import_form_layout);
    //import_form->setStyle(new QWindowsStyle());

    //info panel
    QGridLayout* info_panel_layout = new QGridLayout();
    info_panel_layout->addWidget(volumedir_label,       0,0,1,1);
    volumedir_label->setFixedWidth(230);
    info_panel_layout->addWidget(volumedir_field,       0,1,1,11);
    info_panel_layout->addWidget(volume_dims_label,     1,0,1,1);
    info_panel_layout->addWidget(nrows_field,           1,1,1,3);
    info_panel_layout->addWidget(by_label_1,            1,4,1,1);
    info_panel_layout->addWidget(ncols_field,           1,5,1,3);
    info_panel_layout->addWidget(stacks_dims_label,     2,0,1,1);
    info_panel_layout->addWidget(stack_width_field,    2,1,1,3);
    info_panel_layout->addWidget(by_label_2,            2,4,1,1);
    info_panel_layout->addWidget(stack_height_field,     2,5,1,3);
    info_panel_layout->addWidget(by_label_3,            2,8,1,1);
    info_panel_layout->addWidget(stack_depth_field,     2,9,1,3);
    info_panel_layout->addWidget(voxel_dims_label,      3,0,1,1);
    info_panel_layout->addWidget(vxl_X_field,           3,1,1,3);
    info_panel_layout->addWidget(by_label_4,            3,4,1,1);
    info_panel_layout->addWidget(vxl_Y_field,           3,5,1,3);
    info_panel_layout->addWidget(by_label_5,            3,8,1,1);
    info_panel_layout->addWidget(vxl_Z_field,           3,9,1,3);
    info_panel_layout->addWidget(origin_label,          4,0,1,1);
    info_panel_layout->addWidget(org_X_field,           4,1,1,3);
    info_panel_layout->addWidget(org_Y_field,           4,5,1,3);
    info_panel_layout->addWidget(org_Z_field,           4,9,1,3);
    info_panel_layout->addWidget(stacks_overlap_label,  5,0,1,1);
    info_panel_layout->addWidget(ovp_X_field,           5,1,1,3);
    QLabel* by_label_6 = new QLabel(QChar(0x00D7));
    by_label_6->setAlignment(Qt::AlignCenter);
    info_panel_layout->addWidget(by_label_6,            5,4,1,1);
    info_panel_layout->addWidget(ovp_Y_field,           5,5,1,3);
    info_panel_layout->addWidget(new QLabel("Stitch test:"),6,0,1,1);
    info_panel_layout->addWidget(slice_spinbox,         6,1,1,3);

    QHBoxLayout* slice_format_layout = new QHBoxLayout();
    //channel_selection->setFixedWidth(80);
   // imgdepth_cbox->setFixedWidth(80);
    slice_format_layout->addWidget(channel_selection, 1);
    slice_format_layout->addSpacing(10);
    slice_format_layout->addWidget(imgdepth_cbox, 1);

    info_panel_layout->addLayout(slice_format_layout,     6,5,1,3);
    info_panel_layout->addWidget(preview_button,        6,9,1,3);
    info_panel_layout->setVerticalSpacing(2);
    info_panel->setLayout(info_panel_layout);
    //info_panel->setStyle(new QWindowsStyle());


    //overall
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(import_form);
    layout->addWidget(info_panel);
    layout->addStretch(1);
//    layout->setContentsMargins(10,0,10,5);
    setLayout(layout);

    //wait animated GIF tab icon
    wait_movie = new QMovie(":/icons/wait.gif");
    wait_label = new QLabel(this);
    wait_label->setMovie(wait_movie);

    // signals and slots
    connect(voldir_button, SIGNAL(clicked()), this, SLOT(voldir_button_clicked()));
    connect(projfile_button, SIGNAL(clicked()), this, SLOT(projfile_button_clicked()));
    connect(preview_button, SIGNAL(clicked()), this, SLOT(preview_button_clicked()));
    connect(CImportUnstitched::instance(), SIGNAL(sendOperationOutcome(iom::exception*)), this, SLOT(import_done(iom::exception*)), Qt::QueuedConnection);
#ifdef VAA3D_TERASTITCHER
    connect(CPreview::instance(), SIGNAL(sendOperationOutcome(iom::exception*,Image4DSimple*)), this, SLOT(preview_done(iom::exception*,Image4DSimple*)), Qt::QueuedConnection);
#else
	connect(CPreview::instance(), SIGNAL(sendOperationOutcome(iom::exception*)), this, SLOT(preview_done(iom::exception*)), Qt::QueuedConnection);
#endif
	connect(path_field, SIGNAL(textChanged(QString)), this, SLOT(volumePathChanged(QString)));
    connect(rescan_checkbox, SIGNAL(stateChanged(int)), this, SLOT(rescanCheckboxChanged(int)));
    connect(imin_plugin_cbox, SIGNAL(currentIndexChanged(QString)), this, SLOT(iopluginChanged(QString)));
    //connect(vol_format_cbox, SIGNAL(currentIndexChanged(QString)), this, SLOT(volformatChanged(QString)));
    connect(sparsedata_checkbox, SIGNAL(stateChanged(int)), this, SLOT(sparsedataCheckboxChanged(int)));

    reset();
}


PTabImport::~PTabImport()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport destroyed\n", this->thread()->currentThreadId());
    #endif
}

//reset method
void PTabImport::reset()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport::reset()\n", this->thread()->currentThreadId());
    #endif

    //import form panel
    import_form->setEnabled(true);
    path_field->setText("");
    rescan_checkbox->setChecked(false);
    axs1_field->setCurrentIndex(0);
    PTeraStitcher::setEnabledComboBoxItem(axs1_field, 0, false);
    axs2_field->setCurrentIndex(0);
    PTeraStitcher::setEnabledComboBoxItem(axs2_field, 0, false);
    axs3_field->setCurrentIndex(0);
    PTeraStitcher::setEnabledComboBoxItem(axs3_field, 0, false);
    imin_plugin_cbox->setCurrentIndex(0);
    PTeraStitcher::setEnabledComboBoxItem(imin_plugin_cbox, 0, false);
    //vol_format_cbox->setCurrentIndex(0);
    //PTeraStitcher::setEnabledComboBoxItem(vol_format_cbox, 0, false);
    vxl1_field->setMinimum(0.1);
    vxl1_field->setMaximum(1000.0);
    vxl1_field->setSingleStep(0.1);
    vxl1_field->setValue(1.0);
    vxl2_field->setMinimum(0.1);
    vxl2_field->setMaximum(1000.0);
    vxl2_field->setSingleStep(0.1);
    vxl2_field->setValue(1.0);
    vxl3_field->setMinimum(0.1);
    vxl3_field->setMaximum(1000.0);
    vxl3_field->setSingleStep(0.1);
    vxl3_field->setValue(1.0);
    sparsedata_checkbox->setChecked(false);

    regex_field->setStyleSheet("QLineEdit{background: white;}");

    //info panel
    info_panel->setEnabled(false);
    volumedir_field->setText("");
    nrows_field->setText("");
    ncols_field->setText("");
    stack_height_field->setText("");
    stack_width_field->setText("");
    stack_depth_field->setText("");
    vxl_Y_field->setText("");
    vxl_X_field->setText("");
    vxl_Z_field->setText("");
    org_Y_field->setText("");
    org_X_field->setText("");
    org_Z_field->setText("");
    ovp_Y_field->setText("");
    ovp_X_field->setText("");

    slice_spinbox->setMinimum(0);
    slice_spinbox->setMaximum(0);
    slice_spinbox->setValue(0);
    slice_spinbox->setSuffix("/0");
    
//     delete channel_selection;
//     channel_selection = new QComboBox();
//     channel_selection->addItem("unselected");
}

// qt event filter
bool PTabImport::eventFilter(QObject *watched, QEvent *e)
{
//    if(watched == regex_field && e->type() == QEvent::FocusIn && regex_field->text().compare("(optional) Enter a regular expression to filter image filenames") == 0)
//    {
//        regex_field->setText("");
//    }
    return QWidget::eventFilter(watched, e);
}

/**********************************************************************************
* Called when "voldir_button" has been clicked.
* Opens QFileDialog to select volume's path, which is copied into "path_field".
***********************************************************************************/
void PTabImport::voldir_button_clicked()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport voldir_button_clicked() launched\n", this->thread()->currentThreadId());
    #endif

    //obtaining volume's directory
    //---- Alessandro 2013-05-20: obtaining volume's directory with QFileDialog instead of platform native file dialogs
    //                            since a strange behaviour has been shown by native file dialogs on MacOS X.
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    dialog.setViewMode(QFileDialog::Detail);
    //dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    dialog.setWindowTitle("Select volume's directory");
    dialog.setDirectory(QDir::currentPath());
    if(dialog.exec())
        path_field->setText(dialog.directory().absolutePath());
}

/**********************************************************************************
* Called when "projfile_button" has been clicked.
* Opens QFileDialog to select project's XML path, which is copied into "path_field".
***********************************************************************************/
void PTabImport::projfile_button_clicked()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport projfile_button_clicked() launched\n", this->thread()->currentThreadId());
    #endif

    //obtaining TeraStitcher XML project file
    //---- Alessandro 2013-05-20: obtaining volume's directory with QFileDialog instead of platform native file dialogs
    //                            since a strange behaviour has been shown by native file dialogs on MacOS X.
    QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
    //dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    dialog.setWindowTitle("Select TeraStitcher XML project file");
    dialog.setDirectory(QDir::currentPath());
    dialog.setNameFilter(tr("XML files (*.xml *.XML)"));
    if(dialog.exec())
        if(!dialog.selectedFiles().empty())
            path_field->setText(dialog.selectedFiles().front());
}
/**********************************************************************************
* Called when "preview_button" has been clicked.
* Launches stitching of the selected slice. The result is displayed in Vaa3D.
***********************************************************************************/
void PTabImport::preview_button_clicked()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport preview_button_clicked() launched\n", this->thread()->currentThreadId());
    #endif

    //disabling preview panel and enabling progress bar animation and tab wait animation
    PTeraStitcher::instance()->getProgressBar()->setEnabled(true);
    PTeraStitcher::instance()->getProgressBar()->setMinimum(0);
    PTeraStitcher::instance()->getProgressBar()->setMaximum(0);
    PTeraStitcher::instance()->getStatusBar()->showMessage("Stitching slice...");
    PTeraStitcher::instance()->setStartButtonEnabled(false);
    wait_movie->start();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, wait_label);

	// 2017-04-29. Giulio. @ADDED set of the input channel when the input plugin is non-interleaved
	if(iom::IMIN_PLUGIN == "tiff2D" || iom::IMIN_PLUGIN == "opencv2D" || iom::IMIN_PLUGIN == "tiff3D") 
		// interleaved inout plugins do not nothing
		;
	else
		// non-interleaved input plugins: active channel must be set before launching previewing
    	CImportUnstitched::instance()->getVolume()->setACTIVE_CHAN(iom::CHANS_no);

    //launching preview thread
    int bitdepth = 8;
    if(imgdepth_cbox->currentText().compare("16 bits") == 0)
        bitdepth = 16;
#ifdef VAA3D_TERASTITCHER
    CPreview::instance()->setMembers(CImportUnstitched::instance()->getVolume(), slice_spinbox->value()-1, bitdepth, PTeraStitcher::instance()->V3D_env);
#else
	CPreview::instance()->setMembers(CImportUnstitched::instance()->getVolume(), slice_spinbox->value()-1, bitdepth);
#endif
    CPreview::instance()->start();

}

/*********************************************************************************
* Start/Stop methods associated to the current step.
* They are called by the startButtonClicked/stopButtonClicked methods of <PTeraStitcher>
**********************************************************************************/
void PTabImport::start()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport start() launched\n", this->thread()->currentThreadId());
    #endif

	// check no volume has been imported yet
	if(CImportUnstitched::instance()->getVolume())
	{
		QMessageBox::warning(this,QObject::tr("Warning"), "Image has already been imported.");
		PTeraStitcher::instance()->setToReady();
		return;
	}

    try
    {
        // check the inserted path exists
        std::string import_path = path_field->text().toStdString();
        if(!vm::StackedVolume::fileExists(import_path.c_str()))
            throw iom::exception("The inserted path does not exist!");

        // check user input
        if(imin_plugin_cbox->currentIndex() == 0)
            throw iom::exception("Please select an image I/O plugin from the combolist");
        if(axs1_field->isVisible() && axs1_field->currentIndex() == 0)
            throw iom::exception("Please select the first axis of the reference system from the combolist");
        if(axs2_field->isVisible() && axs2_field->currentIndex() == 0)
            throw iom::exception("Please select the second axis of the reference system from the combolist");
        // 2017-05-03. Giulio. Currently the choice for the third axis is fixed
        //if(axs3_field->isVisible() && axs3_field->currentIndex() == 0)
		//	throw iom::exception("Please select the third axis of the reference system from the combolist");
        if(imin_plugin_cbox->isVisible() && imin_plugin_cbox->currentIndex() == 0)
            throw iom::exception("Please select the i/o plugin from the pull-down menu");

        // propagate user's input to CImport
        CImportUnstitched::instance()->setAxes(axs1_field->currentText().toStdString(),
                                     axs2_field->currentText().toStdString(),
                                     axs3_field->currentText().toStdString());
        CImportUnstitched::instance()->setVoxels(static_cast<float>(vxl1_field->value()),
                                       static_cast<float>(vxl2_field->value()),
                                       static_cast<float>(vxl3_field->value()));
        CImportUnstitched::instance()->setReimport(rescan_checkbox->isChecked());
        CImportUnstitched::instance()->setPath(import_path);

        // disable import form and enable progress bar animation and tab wait animation
        import_form->setEnabled(false);
        PTeraStitcher::instance()->getProgressBar()->setEnabled(true);
        PTeraStitcher::instance()->getProgressBar()->setMinimum(0);
        PTeraStitcher::instance()->getProgressBar()->setMaximum(0);
        PTeraStitcher::instance()->getStatusBar()->showMessage("Importing volume...");
        wait_movie->start();
        container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, wait_label);

        // start
        CImportUnstitched::instance()->start();
    }
    catch(iom::exception &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
        PTeraStitcher::instance()->setToReady();
        CImportUnstitched::instance()->reset();
    }
}
void PTabImport::stop()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport stop() launched\n", this->thread()->currentThreadId());
    #endif

    //terminating thread
    try
    {
        CImportUnstitched::instance()->terminate();
        CImportUnstitched::instance()->wait();
        CImportUnstitched::instance()->reset();
    }
    catch(iom::exception &ex) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));}
    catch(...) {QMessageBox::critical(this,QObject::tr("Error"), QObject::tr("Unable to determine error's type"),QObject::tr("Ok"));}

    //re-enabling import form and disabling progress bar and wait animations
    import_form->setEnabled(true);
    PTeraStitcher::instance()->setToReady();
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
}

/*********************************************************************************
* Called by <CImport> when the associated operation has been performed.
* If an exception has occurred in the <CImport> thread,  it is propagated and man-
* aged in the current thread (ex != 0). Otherwise, volume information are imported
* in the GUI by the <StackedVolume> handle of <CImport>.
**********************************************************************************/
void PTabImport::import_done(iom::exception *ex)
{
    /**/ts::debug(ts::LEV1, strprintf("ex = \"%s\"", ex ? ex->what() : "none").c_str(), __tsp__current__function__);

    //if an exception has occurred, showing a message error and re-enabling import form
    if(ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex->what()),QObject::tr("Ok"));
        import_form->setEnabled(true);
        PTeraStitcher::instance()->setToReady();
        container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
    }
    else
    {
        //otherwise inserting volume's informations...
        /**/ts::debug(ts::LEV_MAX, strprintf("insert volume's info").c_str(), __tsp__current__function__);
        info_panel->setEnabled(true);
        volumedir_field->setText(CImportUnstitched::instance()->getVolume()->getSTACKS_DIR());
        nrows_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getN_ROWS()).append(" (rows)"));
        ncols_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getN_COLS()).append(" (columns)"));
        stack_height_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getStacksHeight()).append(" (Y)"));
        stack_width_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getStacksWidth()).append(" (X)"));
        stack_depth_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getN_SLICES()).append(" (Z)"));
        vxl_Y_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getVXL_V()).append(" (Y)"));
        vxl_X_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getVXL_H()).append(" (X)"));
        vxl_Z_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getVXL_D()).append(" (Z)"));
        org_Y_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getORG_V()).append(" (Y)"));
        org_X_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getORG_H()).append(" (X)"));
        org_Z_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getORG_D()).append(" (Z)"));
        ovp_Y_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getOVERLAP_V()).append(" (Y)"));
        ovp_X_field->setText(QString::number(CImportUnstitched::instance()->getVolume()->getOVERLAP_H()).append(" (X)"));
        slice_spinbox->setMinimum(1);
        slice_spinbox->setMaximum(CImportUnstitched::instance()->getVolume()->getN_SLICES());
        slice_spinbox->setValue(slice_spinbox->maximum()/2);
        slice_spinbox->setSuffix(QString("/").append(QString::number(CImportUnstitched::instance()->getVolume()->getN_SLICES())));

		imgdepth_cbox->setCurrentIndex(CImportUnstitched::instance()->getVolume()->getBYTESxCHAN()-1);
    	imgdepth_cbox->setEditable(false);

		// 2017-04-29. Giulio. @ADDED setup of the channel selection combo box
		// the channel combo has already been emptied
		if ( CImportUnstitched::instance()->getVolume()->getDIM_C() > 1 ) {
			if( (iom::IMIN_PLUGIN == "tiff2D" || iom::IMIN_PLUGIN == "opencv2D" || iom::IMIN_PLUGIN == "tiff3D") && 
														vm::VOLUME_INPUT_FORMAT_PLUGIN.compare(vm::MCVolume::id) != 0) {
				channel_selection->addItem("Channel R");
				channel_selection->addItem("Channel G");
				channel_selection->addItem("Channel B");
				// propagate to PTabDisplComp
				PTabDisplComp::getInstance()->channel_selection->addItem("Channel R");
				PTabDisplComp::getInstance()->channel_selection->addItem("Channel G");
				PTabDisplComp::getInstance()->channel_selection->addItem("Channel B");
			}
			else {
				char num_str[20];
				for(int i = 0; i < CImportUnstitched::instance()->getVolume()->getDIM_C(); i++) {
					sprintf(num_str,"Channel %d",i);
					channel_selection->addItem(num_str);
					// propagate to PTabDisplComp
					PTabDisplComp::getInstance()->channel_selection->addItem(num_str);
				}
			}
 		}
		for(int i = 0; i < channel_selection->count(); i++) {
			channel_selection->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
			// propagate to PTabDisplComp
			PTabDisplComp::getInstance()->channel_selection->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
		}
		channel_selection->setCurrentIndex(0);
		channel_selection->setVisible(true);
		// propagate to PTabDisplComp
		PTabDisplComp::getInstance()->channel_selection->setCurrentIndex(0);
		PTabDisplComp::getInstance()->channel_selection->setVisible(true);
		
        PTeraStitcher::instance()->setToReady();

        //...and enabling (ed updating) all other tabs
        /**/ts::debug(ts::LEV_MAX, strprintf("enable all other tabs").c_str(), __tsp__current__function__);
        PTabDisplComp::getInstance()->setEnabled(true);
        PTabDisplProj::getInstance()->setEnabled(true);
        PTabDisplThresh::getInstance()->setEnabled(true);
        PTabPlaceTiles::getInstance()->setEnabled(true);
        PTabMergeTiles::getInstance()->setEnabled(true);

        /**/ts::debug(ts::LEV_MAX, strprintf("enable closeVolume button").c_str(), __tsp__current__function__);
        PTeraStitcher::instance()->closeVolumeAction->setEnabled(true);
    }

    //resetting some widgets
    /**/ts::debug(ts::LEV_MAX, strprintf("reset widgets").c_str(), __tsp__current__function__);
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);
}

/**********************************************************************************
* Called by <CPreview> when the associated operation has been performed.
* If an exception has occurred in the <CPreview> thread,  it is propagated and man-
* aged in the current thread (ex != 0). Otherwise, the preview which was saved back
* onto the disk is loaded and shown in Vaa3D.
***********************************************************************************/
#ifdef VAA3D_TERASTITCHER
void PTabImport::preview_done(iom::exception *ex, Image4DSimple* img)
#else
void PTabImport::preview_done(iom::exception *ex)
#endif
{
    /**/ts::debug(ts::LEV1, strprintf("ex = \"%s\"", ex ? ex->what() : "none").c_str(), __tsp__current__function__);

    //resetting some widgets
    PTeraStitcher::instance()->setToReady();
    PTeraStitcher::instance()->setStartButtonEnabled(true);
    wait_movie->stop();
    container->getTabBar()->setTabButton(tab_index, QTabBar::LeftSide, 0);

    //if an exception has occurred, showing a message error. Otherwise showing the computed preview
    if(ex)
    {
        /**/ts::debug(ts::LEV_MAX, "display message error", __tsp__current__function__);
        QMessageBox::critical(this, "Error", ex->what());
        //QMessageBox::critical(this, "Error", "An error occurred while generating the selected slice");
    }
    else
    {
        /**/ts::debug(ts::LEV_MAX, "display image", __tsp__current__function__);
#ifdef VAA3D_TERASTITCHER
        v3dhandle new_win = PTeraStitcher::instance()->getV3D_env()->newImageWindow(QString("Slice ").append(slice_spinbox->text()));
        PTeraStitcher::instance()->getV3D_env()->setImage(new_win, img);
#else
		QMessageBox::information(this, "Success", "Slice generated successfully!");
#endif
    }
}

/**********************************************************************************
* Called when "path_field" value has changed.
***********************************************************************************/
void PTabImport::volumePathChanged(QString path)
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> PTabImport volumePathChanged(path)\n", this->thread()->currentThreadId());
    #endif

    // try to automatically locate the xml_import.xml from the directory
    if(!path_field->text().endsWith("xml", Qt::CaseInsensitive))
    {
        if(QDir(path_field->text()).exists())
        {
            if(QFile(path_field->text() + "/xml_import.xml").exists())
            {
                path_field->setText(path_field->text() + "/xml_import.xml");
                return;
            }
        }
    }

    // determine if current path is an xml file
    bool isXML = path_field->text().endsWith("xml", Qt::CaseInsensitive);

    // if path is an xml
    if (isXML)
    {
        // try to read and automatically select the volume format...
        try
        {
        	// 2017-04-27. Giulio. @ADDED set the input plugin if found in the xml file
        	std::string inplugin = vm::VirtualVolume::getInputPlugin(path_field->text().toStdString());
            int index = imin_plugin_cbox->findText(inplugin.c_str());
            if ( index != -1 ) { // -1 for not found
				imin_plugin_cbox->setCurrentIndex(index);
            	imin_plugin_cbox->setVisible(true);
            }
			else
                imin_plugin_cbox->setVisible(true);
                
//            std::string vformat = vm::VirtualVolume::getVolumeFormat(path_field->text().toStdString());
////            int index = vol_format_cbox->findText(vformat.c_str());
////            if ( index != -1 ) { // -1 for not found
////               vol_format_cbox->setCurrentIndex(index);
//////               vol_format_cbox->setVisible(false);
////            }
////            else
////                vol_format_cbox->setVisible(true);
//
//            // ...and suggest an i/o plugin
//            if(vformat.compare(vm::StackedVolume::id) == 0)
//            {
//                int index = imin_plugin_cbox->findText("tiff2D");
//                if ( index != -1 ) { // -1 for not found
//                   imin_plugin_cbox->setCurrentIndex(index);
//                }
//            }
//            else if(vformat.compare(vm::BlockVolume::id) == 0)
//            {
//                int index = imin_plugin_cbox->findText("tiff3D");
//                if ( index != -1 ) { // -1 for not found
//                   imin_plugin_cbox->setCurrentIndex(index);
//                }
//            }

            rescan_checkbox->setChecked(false);
            axs1_field->setVisible(false);
            axs2_field->setVisible(false);
            axs3_field->setVisible(false);
            vxl1_field->setVisible(false);
            vxl2_field->setVisible(false);
            vxl3_field->setVisible(false);
        }
        catch(...)
        {
            // if fails, volume format has to be inputted for XML files too
//            vol_format_cbox->setVisible(true);
        }
    }
    else
    {
        rescan_checkbox->setChecked(true);
//        vol_format_cbox->setVisible(true);
        axs1_field->setVisible(true);
        axs2_field->setVisible(true);
        axs3_field->setVisible(true);
        vxl1_field->setVisible(true);
        vxl2_field->setVisible(true);
        vxl3_field->setVisible(true);
    }
}

/**********************************************************************************
* Called when "reimport_chheckbox" state has changed.
***********************************************************************************/
void PTabImport::rescanCheckboxChanged(int checked)
{
    // force rescan when image regex is provided
    if(rescan_checkbox->isChecked() == false && !regex_field->text().isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Re-scan of files is mandatory when an image regex is provided. If you uncheck this your image regex will be ignored");
        regex_field->setStyleSheet("QLineEdit{background: rgb(250,188,191);}");
        return;
    }
    else if(rescan_checkbox->isChecked() && !regex_field->text().isEmpty())
    {
        regex_field->setStyleSheet("QLineEdit{background: rgb(255,251,179);}");
    }

    // determine if current path is an xml file
    bool isXML = path_field->text().endsWith("xml", Qt::CaseInsensitive);

    // show axes and voxel fields if rescan is checked and input path is not an XML file (such info is stored in the XML file)
    axs1_field->setVisible(checked && !isXML);
    axs2_field->setVisible(checked && !isXML);
    axs3_field->setVisible(checked && !isXML);
    vxl1_field->setVisible(checked && !isXML);
    vxl2_field->setVisible(checked && !isXML);
    vxl3_field->setVisible(checked && !isXML);

    // volume format is a special case: it should be visibile also if re-scan is not checked, when the XML does not contain such info
//    vol_format_cbox->setVisible(checked || (isXML && vol_format_cbox->currentIndex() == 0));
}

/**********************************************************************************
* Called when "channel_selection" state has changed.
***********************************************************************************/
void PTabImport::channelSelectedChanged(int c)
{
	// 2017-04-29. Giulio. @ADDED distinguish between interleaved and non-interleaved plugins
	if( (iom::IMIN_PLUGIN == "tiff2D" || iom::IMIN_PLUGIN == "opencv2D" || iom::IMIN_PLUGIN == "tiff3D") && 
												vm::VOLUME_INPUT_FORMAT_PLUGIN.compare(vm::MCVolume::id) != 0) {
    	iom::CHANS = iom::channel(c);
    }
    else { // non-interleaved input plugin: the channel number has to be set
    	int index = channel_selection->currentIndex();
    	if ( index > 0 )
    		iom::CHANS_no = index - 1;
    	else
    		iom::CHANS_no = 0;
    }
}

/**********************************************************************************
* Called when "regex_field" state has changed.
***********************************************************************************/
void PTabImport::regexFieldChanged()
{
    if(regex_field->text().isEmpty())
    {
        regex_field->setStyleSheet("QLineEdit{background: white;}");
    }
    else
    {
        regex_field->setStyleSheet("QLineEdit{background: rgb(255,251,179);}");
        volumemanager::IMG_FILTER_REGEX = regex_field->text().toStdString();
        rescan_checkbox->setChecked(true);
    }
}

/**********************************************************************************
* Called when "inputImageIOPlugin" state has changed.
***********************************************************************************/
void PTabImport::iopluginChanged(QString str)
{
    iom::IMIN_PLUGIN = str.toStdString();

	// the combo list of channels is emptied every time the plugin is changed
	int n = channel_selection->count() - 1;
	for ( int i=0; i<n; i++ ) {
		channel_selection->removeItem(1);
		// propagate to PTabDisplComp
		PTabDisplComp::getInstance()->channel_selection->removeItem(1);
	}
 	channel_selection->setCurrentIndex(0);
	channel_selection->setVisible(true);
	// propagate to PTabDisplComp
	PTabDisplComp::getInstance()->channel_selection->setCurrentIndex(0);
	PTabDisplComp::getInstance()->channel_selection->setVisible(true);
		
	// 2017-04-25 by Alessandro: @ADDED automatic selection of volume format plugin
	// 2018-01-25 by Giulio: @ADDED selection of MultiVolume format 
	if(str == "MultiVolume") {
		vm::VOLUME_INPUT_FORMAT_PLUGIN = vm::MCVolume::id;
	}
	else if(str == "tiff2D" || str == "opencv2D" || "bioformats2D" ) {
		vm::VOLUME_INPUT_FORMAT_PLUGIN = vm::StackedVolume::id;
	}
	else {
		vm::VOLUME_INPUT_FORMAT_PLUGIN = vm::BlockVolume::id;
	}
}

/**********************************************************************************
* Called when "volformat_cbox" state has changed.
***********************************************************************************/
/*void PTabImport::volformatChanged(QString str)
{
    vm::VOLUME_INPUT_FORMAT_PLUGIN = str.toStdString();

    // suggest image plugin
    int index = 0;
    if(vm::VOLUME_INPUT_FORMAT_PLUGIN.compare(vm::StackedVolume::id) == 0)
    {
        index = imin_plugin_cbox->findText("tiff2D");
        if ( index != -1 ) { // -1 for not found
           imin_plugin_cbox->setCurrentIndex(index);
        }
    }
    else if(vm::VOLUME_INPUT_FORMAT_PLUGIN.compare(vm::BlockVolume::id) == 0)
    {
        index = imin_plugin_cbox->findText("tiff3D");
        if ( index != -1 ) { // -1 for not found
           imin_plugin_cbox->setCurrentIndex(index);
        }
    }
}*/

/**********************************************************************************
* Called when "sparse_data_checkbox" state has changed.
***********************************************************************************/
void PTabImport::sparsedataCheckboxChanged(int)
{
    vm::SPARSE_DATA = true;
}
