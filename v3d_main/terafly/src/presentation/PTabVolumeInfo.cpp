
#include "renderer_gl1.h"

#include "PTabVolumeInfo.h"
#include "CImport.h"
#include "VirtualVolume.h"
#include "TiledVolume.h"
#include "TiledMCVolume.h"
#include "StackedVolume.h"
#include "SimpleVolume.h"
#include "SimpleVolumeRaw.h"
#include "BDVVolume.h"
#include "VirtualPyramid.h"
#include "TimeSeries.h"
#include "CViewer.h"
#include "PMain.h"

tf::PTabVolumeInfo::PTabVolumeInfo(QWidget *parent) : QWidget(parent)
{
    QFont tinyFont = QApplication::font();
    #ifdef Q_OS_LINUX
    tinyFont.setPointSize(9);
    #endif

    tabs = new QTabWidget(this);
    info_tab = new QWidget(tabs);
    pyramid_tab = new QWidget(tabs);

    // info panel
    info_panel = new QWidget(this);
    vol_format_field = new QLineEdit();
    vol_format_field->setAlignment(Qt::AlignLeft);
    vol_format_field->setReadOnly(true);
    vol_format_field->setFont(tinyFont);
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
    voxel_dims_label = new QLabel(QString("Voxel (").append(QChar(0x03BC)).append("m):"));
    vxl_field = new QLineEdit();
    vxl_field->setAlignment(Qt::AlignLeft);
    vxl_field->setReadOnly(true);
    vxl_field->setFont(tinyFont);
    org_field = new QLineEdit();
    org_field->setAlignment(Qt::AlignLeft);
    org_field->setReadOnly(true);
    org_field->setFont(tinyFont);
    vol_path = new QLineEdit();
    vol_path->setAlignment(Qt::AlignLeft);
    vol_path->setReadOnly(true);
    vol_path->setFont(tinyFont);

    // virtual pyramid panel
    vp_panel = new QGroupBox("Info", this);
#ifdef Q_OS_LINUX
    vp_panel->setStyle(new QWindowsStyle());
#endif
    vp_path = new QLineEdit(this);
    vp_path->setReadOnly(true);
    vp_path->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    vp_path->setTextMargins(5, 0, 0, 0);
    vp_open = new QPushButton("Open", this);
    vp_size = new QLineEdit(this);
    vp_size->setTextMargins(5, 0, 0, 0);
    vp_recheck = new QPushButton("Recheck", this);
    vp_size->setReadOnly(true);
    vp_size->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    vp_subsampling = new QLineEdit(this);
    vp_subsampling->setReadOnly(true);
    vp_subsampling->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    vp_subsampling->setTextMargins(5, 0, 0, 0);
    vp_tiledims = new QLineEdit(this);
    vp_tiledims->setReadOnly(true);
    vp_tiledims->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    vp_tiledims->setTextMargins(5, 0, 0, 0);
    vp_tileformat = new QLineEdit(this);
    vp_tileformat->setReadOnly(true);
    vp_tileformat->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    vp_tileformat->setTextMargins(5, 0, 0, 0);

    // virtual pyramid exploration panel
    vp_exploration_panel = new QGroupBox("Image exploration", this);
#ifdef Q_OS_LINUX
    vp_exploration_panel->setStyle(new QWindowsStyle());
#endif
    vp_exploration_bar_local = new QGradientBar(this);
    vp_exploration_bar_local->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    vp_exploration_bar_local->setMinimumHeight(25);
    vp_exploration_bar_global = new QGradientBar(this);
    vp_exploration_bar_global->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    vp_exploration_bar_global->setMinimumHeight(25);
    vp_refill_button = new QPushButton("Refill", this);
    vp_empty_viz_method_combobox = new QComboBox(this);
    vp_empty_viz_method_combobox->addItem("100% black");
    vp_empty_viz_method_combobox->addItem("100% intensity");
    vp_empty_viz_method_combobox->addItem("black + salt (10%)");
    vp_empty_viz_method_combobox->addItem("black + salt (1%)");
    vp_empty_viz_method_combobox->addItem("black + salt (0.1%)");
    vp_empty_viz_method_combobox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    vp_empty_viz_intensity = new QSpinBox(this);
    vp_empty_viz_intensity->setMinimum(0);
    vp_empty_viz_intensity->setMaximum(255);
    vp_empty_viz_intensity->setPrefix(" I = ");
    vp_refill_times_spinbox = new QSpinBox(this);
    vp_refill_times_spinbox->setPrefix("x");
    vp_refill_times_spinbox->setAlignment(Qt::AlignCenter);
    vp_refill_strategy_combobox = new QComboBox(this);
    vp_refill_strategy_combobox->addItem("randomly");
    vp_refill_strategy_combobox->addItem("sequentially");
    vp_refill_strategy_combobox->addItem("spirally");
    vp_refill_strategy_combobox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    vp_refill_auto_checkbox = new QCheckBox("auto", this);
    vp_refill_stop_combobox = new QComboBox(this);
    vp_refill_stop_combobox->addItem("by fetching exactly");
    vp_refill_stop_combobox->addItem("until %VOI explored is <");
    vp_refill_coverage_spinbox = new QSpinBox(this);
    vp_refill_coverage_spinbox->setSuffix("%");
    vp_refill_coverage_spinbox->setAlignment(Qt::AlignCenter);
    vp_block_dimX = new QSpinBox(this);
    vp_block_dimX->setAlignment(Qt::AlignCenter);
    vp_block_dimX->setSuffix("(x)");
    vp_block_dimX->setMinimum(1);
    vp_block_dimX->setMaximum(999);
    vp_block_dimY = new QSpinBox(this);
    vp_block_dimY->setAlignment(Qt::AlignCenter);
    vp_block_dimY->setSuffix("(y)");
    vp_block_dimY->setMinimum(1);
    vp_block_dimY->setMaximum(999);
    vp_block_dimZ = new QSpinBox(this);
    vp_block_dimZ->setAlignment(Qt::AlignCenter);
    vp_block_dimZ->setSuffix("(z)");
    vp_block_dimZ->setMinimum(1);
    vp_block_dimZ->setMaximum(999);
    vp_refill_time_spent = new QLineEdit(this);
    vp_refill_time_spent->setReadOnly(true);
    vp_highest_res_cache = new QCheckBox("Enable caching/saving", this);
    vp_highest_res_freeze = new QCheckBox("Freeze", this);

    // virtual pyramid RAM panel
    vp_ram_panel = new QGroupBox("RAM usage", this);
    #ifdef Q_OS_LINUX
        vp_ram_panel->setStyle(new QWindowsStyle());
    #endif
    for(size_t i=0; i<vp_ram_max_size; i++)
    {
        vp_ram_labels.push_back(new QLabel(this));
        vp_ram_bars.push_back(new QGradientBar(this));
        std::vector <QColor> bar_colors;
        bar_colors.push_back(QColor(167,229,145));
        bar_colors.push_back(QColor(255,230,153));
        bar_colors.push_back(QColor(231,158,158));
        vp_ram_bars.back()->setMultiColor(bar_colors);
        vp_ram_clear_buttons.push_back(new QPushButton("Clear", this));

        vp_ram_bars[i]->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    }
    vp_max_ram_spinbox = new QDoubleSpinBox(this);
    vp_max_ram_spinbox->setSuffix(" GB");
    vp_max_ram_spinbox->setDecimals(1);
    vp_max_ram_spinbox->setMinimum(0.5);
    vp_max_ram_spinbox->setAlignment(Qt::AlignCenter);
    vp_max_ram_spinbox->setSingleStep(0.1);
    //vp_ram_show_res_buttons = new QPushButton(this);
    //vp_ram_show_res_buttons->setCheckable(true);

    /*** LAYOUT SECTION ***/
    /* ---------------- info panel ------------------- */
    int firstColumnWidth = 80;
    int lastColumnWidth = 80;
    QGridLayout* info_panel_layout = new QGridLayout();
    QLabel* size_label = new QLabel("Size:");
    size_label->setFixedWidth(firstColumnWidth);
    info_panel_layout->addWidget(size_label,                    0,0,1,1);
    info_panel_layout->addWidget(vol_size_field,                0,2,1,1);
    info_panel_layout->addWidget(new QLabel("Dims (mm):"),      1,0,1,1);
    info_panel_layout->addWidget(vol_dims_mm_field,             1,2,1,1);
    info_panel_layout->addWidget(new QLabel("Dims (vxl):"),     2,0,1,1);
    info_panel_layout->addWidget(vol_dims_vxl_field,            2,2,1,1);
    info_panel_layout->addWidget(new QLabel("Format:"),         3,0,1,1);
    info_panel_layout->addWidget(vol_format_field,              3,2,1,1);
    info_panel_layout->addWidget(voxel_dims_label,              4,0,1,1);
    info_panel_layout->addWidget(vxl_field,                     4,2,1,1);
    info_panel_layout->addWidget(new QLabel("Origin (mm):"),    5,0,1,1);
    info_panel_layout->addWidget(org_field,                     5,2,1,1);
    info_panel_layout->addWidget(new QLabel("Volume path:"),    6,0,1,1);
    info_panel_layout->addWidget(vol_path,                      6,2,1,1);
    info_panel->setLayout(info_panel_layout);
    /* ----------- virtual pyramid panel ------------- */
    QGridLayout* vp_layout = new QGridLayout();
    vp_layout->setContentsMargins(5,5,5,5);
    vp_layout->setSpacing(5);
    QLabel* path_label = new QLabel("Path:");
    path_label->setFixedWidth(firstColumnWidth);
    vp_layout->addWidget(path_label,                            0,0,1,1);
    vp_layout->addWidget(vp_path,                               0,1,1,1);
    vp_open->setFixedWidth(lastColumnWidth);
    vp_recheck->setFixedWidth(lastColumnWidth);
    vp_layout->addWidget(vp_open,                               0,2,1,1);
    vp_layout->addWidget(new QLabel("Size on disk:"),           1,0,1,1);
    vp_layout->addWidget(vp_size,                               1,1,1,1);
    vp_layout->addWidget(vp_recheck,                            1,2,1,1);
    vp_layout->addWidget(new QLabel("Layers:"),                 2,0,1,1);
    vp_layout->addWidget(vp_subsampling,                        2,1,1,2);
    vp_layout->addWidget(new QLabel("Block dims:"),             3,0,1,1);
    vp_layout->addWidget(vp_tiledims,                           3,1,1,2);
    vp_layout->addWidget(new QLabel("Block format:"),           4,0,1,1);
    vp_layout->addWidget(vp_tileformat,                         4,1,1,2);
    vp_panel->setLayout(vp_layout);
    /* ----------- virtual pyramid exploration panel ------------- */
    QGridLayout* expl_panel_layout = new QGridLayout();
    expl_panel_layout->setContentsMargins(5,5,5,5);
    expl_panel_layout->setSpacing(6);
    expl_panel_layout->addWidget(new QLabel("Unexplored:"),     0,0,1,1);
    expl_panel_layout->addWidget(vp_empty_viz_method_combobox,  0,1,1,1);
    vp_empty_viz_intensity->setFixedWidth(lastColumnWidth);
    expl_panel_layout->addWidget(vp_empty_viz_intensity,        0,2,1,1);
    QLabel* coverage_label = new QLabel("Explored:");
    coverage_label->setFixedWidth(firstColumnWidth);
    expl_panel_layout->addWidget(coverage_label,                1,0,1,1);
    expl_panel_layout->addWidget(vp_exploration_bar_global,     1,1,1,2);
    expl_panel_layout->addWidget(vp_exploration_bar_local,      2,1,1,2);
    vp_refill_button->setFixedWidth(firstColumnWidth);
    vp_refill_times_spinbox->setFixedWidth(lastColumnWidth);
    vp_refill_coverage_spinbox->setFixedWidth(lastColumnWidth);
    expl_panel_layout->addWidget(vp_refill_button,              3,0,1,1);
    expl_panel_layout->addWidget(vp_refill_strategy_combobox,   3,1,1,1);
    expl_panel_layout->addWidget(vp_refill_auto_checkbox,       4,0,2,1, Qt::AlignCenter);
    expl_panel_layout->addWidget(vp_refill_stop_combobox,       4,1,1,1);
    expl_panel_layout->addWidget(vp_refill_times_spinbox,       4,2,1,1);
    expl_panel_layout->addWidget(vp_refill_coverage_spinbox,    4,2,1,1);
    expl_panel_layout->addWidget(new QLabel("(block dims)"),    5,2,1,1);
    QHBoxLayout* block_layout = new QHBoxLayout();
    block_layout->addWidget(vp_block_dimX);
    block_layout->addWidget(vp_block_dimY);
    block_layout->addWidget(vp_block_dimZ);
    expl_panel_layout->addLayout(block_layout,                  5,1,1,1);
    QLabel *vp_refill_time_spent_label = new QLabel("Avg refill time:");
    vp_refill_time_spent_label->setFixedWidth(firstColumnWidth);
    expl_panel_layout->addWidget(vp_refill_time_spent_label,    6,0,1,1);
    expl_panel_layout->addWidget(vp_refill_time_spent,          6,1,1,1);
    QLabel *vp_highest_res_label = new QLabel("Highest res:");
    vp_highest_res_label->setFixedWidth(firstColumnWidth);
    expl_panel_layout->addWidget(vp_highest_res_label,          7,0,1,1);
    expl_panel_layout->addWidget(vp_highest_res_cache,          7,1,1,1);
    expl_panel_layout->addWidget(vp_highest_res_freeze,         8,1,1,1);
    vp_exploration_panel->setLayout(expl_panel_layout);
    /* ----------- allocated RAM panel --------------- */
    QGridLayout* vp_RAM_layout = new QGridLayout();
    vp_RAM_layout->setContentsMargins(5,5,5,5);
    vp_RAM_layout->setSpacing(5);
    vp_RAM_layout->addWidget(new QLabel("Set limit to:"),       0,0,1,1);
    vp_RAM_layout->addWidget(vp_max_ram_spinbox,                0,1,1,1);
    vp_ram_labels[0]->setFixedWidth(firstColumnWidth);
    vp_RAM_layout->addWidget(vp_ram_labels[0],                  1,0,1,1);
    vp_RAM_layout->addWidget(vp_ram_bars[0],                    1,1,1,1);
    vp_RAM_layout->addWidget(vp_ram_clear_buttons[0],           1,2,1,1);
    vp_ram_clear_buttons[0]->setFixedWidth(lastColumnWidth);
    for(size_t i=1; i<vp_ram_max_size; i++)
    {
        vp_RAM_layout->addWidget(vp_ram_labels[i],              i+1,0,1,1);
        vp_RAM_layout->addWidget(vp_ram_bars[i],                i+1,1,1,1);
        vp_RAM_layout->addWidget(vp_ram_clear_buttons[i],       i+1,2,1,1);
        vp_ram_clear_buttons[i]->setFixedWidth(lastColumnWidth);
    }
    vp_ram_panel->setLayout(vp_RAM_layout);
    /* ---------------- MAIN LAYOUT ------------------ */
    QVBoxLayout* pyramid_layout = new QVBoxLayout();
    pyramid_layout->addWidget(vp_panel, 0);
    pyramid_layout->addWidget(vp_exploration_panel, 0);
    pyramid_layout->addWidget(vp_ram_panel, 0);
    pyramid_layout->addStretch(1);
    pyramid_layout->setContentsMargins(10,10,10,10);
    pyramid_tab->setLayout(pyramid_layout);
    QVBoxLayout* info_layout = new QVBoxLayout();
    info_layout->addWidget(info_panel, 0);
    info_layout->addStretch(1);
    info_layout->setContentsMargins(10,10,10,10);
    info_tab->setLayout(info_layout);
    QVBoxLayout* layout = new QVBoxLayout();
    tabs->addTab(pyramid_tab, "Virtual Pyramid");
    tabs->addTab(info_tab, "Volume's info");
    tabs->setContentsMargins(0,0,0,0);
    layout->addWidget(tabs);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);
    #ifdef Q_OS_LINUX
    setStyle(new QWindowsStyle());
    #endif

    updateTimer.setInterval(1000);
    updateTimer.start();

    connect(vp_open, SIGNAL(clicked()), this, SLOT(open_button_clicked()));
    connect(vp_recheck, SIGNAL(clicked()), this, SLOT(recheck_button_clicked()));
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(update()));
    connect(vp_max_ram_spinbox, SIGNAL(valueChanged(double)), this, SLOT(ram_limit_changed(double)));
    connect(vp_refill_button, SIGNAL(clicked()), this, SLOT(vp_refill_button_clicked()));
    connect(vp_empty_viz_method_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(empty_combobox_index_changed(int)));
    connect(vp_empty_viz_intensity, SIGNAL(valueChanged(int)), this, SLOT(empty_intensity_value_changed(int)));
    connect(vp_refill_strategy_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(vp_refill_strategy_combobox_changed(int)));
    connect(vp_refill_times_spinbox, SIGNAL(valueChanged(int)), this, SLOT(vp_refill_times_spinbox_changed(int)));
    connect(vp_refill_auto_checkbox, SIGNAL(toggled(bool)), this, SLOT(vp_refill_auto_checkbox_changed(bool)));
    connect(vp_refill_stop_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(vp_refill_stop_combobox_changed(int)));
    connect(vp_refill_coverage_spinbox, SIGNAL(valueChanged(int)), this, SLOT(vp_refill_coverage_spinbox_changed(int)));
    connect(vp_highest_res_cache, SIGNAL(toggled(bool)), this, SLOT(vp_highest_res_cache_checkbox_changed(bool)));
    connect(vp_highest_res_freeze, SIGNAL(toggled(bool)), this, SLOT(vp_highest_res_freeze_checkbox_changed(bool)));

    for(size_t i=0; i<vp_ram_clear_buttons.size(); i++)
        connect(vp_ram_clear_buttons[i], SIGNAL(clicked()), this, SLOT(clear_button_clicked()));

    QApplication::instance()->installEventFilter(&inactivityDetector);
}

void tf::PTabVolumeInfo::reset()
{
    refill_time_total = 0;
    refill_blocks_total = 0;

    vol_format_field->setText("");
    vol_size_field->setText("");
    vol_dims_mm_field->setText("");
    vol_dims_vxl_field->setText("");
    vxl_field->setText("");
    org_field->setText("");
    vol_path->setText("");
    vp_path->setText("");
    vp_size->setText("");
    vp_subsampling->setText("");
    vp_tiledims->setText("");
    vp_tileformat->setText("");
    vp_panel->setVisible(false);
    vp_ram_panel->setVisible(false);
    vp_exploration_panel->setVisible(false);

    for(int i=0; i<vp_ram_max_size; i++)
    {
        if(i==0)
            vp_ram_labels[i]->setText("Total:");
        else
            vp_ram_labels[i]->setText("");
        vp_ram_bars[i]->setNSteps(100);
    }

    vp_max_ram_spinbox->setValue(CSettings::instance()->getRamLimitGB());


    // get empty visualization flags
    tf::VirtualPyramid::empty_filling empty_viz_method = tf::VirtualPyramid::empty_filling(CSettings::instance()->getVpEmptyVizMethod());
    float empty_viz_salt_pepper_perc = CSettings::instance()->getVpEmptyVizSaltPepperPercentage();
    vp_empty_viz_intensity->setValue(CSettings::instance()->getVpEmptyVizIntensity());
    if(empty_viz_method == tf::VirtualPyramid::RAW)
        vp_empty_viz_method_combobox->setCurrentIndex(0);
    else if(empty_viz_method == tf::VirtualPyramid::SOLID)
        vp_empty_viz_method_combobox->setCurrentIndex(1);
    else if(empty_viz_method == tf::VirtualPyramid::SALT_AND_PEPPER)
    {
        if(empty_viz_salt_pepper_perc == 0.1f)
            vp_empty_viz_method_combobox->setCurrentIndex(2);
        else if(empty_viz_salt_pepper_perc == 0.01f)
            vp_empty_viz_method_combobox->setCurrentIndex(3);
        else if(empty_viz_salt_pepper_perc == 0.001f)
            vp_empty_viz_method_combobox->setCurrentIndex(4);
    }
    tf::VirtualPyramid::_unexploredFillingMethod = empty_viz_method;
    tf::VirtualPyramid::_unexploredIntensityVal = CSettings::instance()->getVpEmptyVizIntensity();
    tf::VirtualPyramid::_unexploredSaltAndPepperPerc = empty_viz_salt_pepper_perc;


    vp_exploration_bar_local->setNSteps(10000);
    vp_exploration_bar_global->setNSteps(10000);

    vp_refill_times_spinbox->setMinimum(1);
    vp_refill_times_spinbox->setMaximum(1000);
    vp_refill_times_spinbox->setValue(CSettings::instance()->getVpFetchNBlocks());

    vp_refill_strategy_combobox->setCurrentIndex(CSettings::instance()->getVpFetchMethod());

    vp_refill_auto_checkbox->setChecked(CSettings::instance()->getVpRefillAuto());
    vp_refill_auto_checkbox_changed(vp_refill_auto_checkbox->isChecked());

    int vp_refill_value = CSettings::instance()->getVpRefillCoverage();
    vp_refill_coverage_spinbox->setMinimum(1);
    vp_refill_coverage_spinbox->setMaximum(100);
    vp_refill_coverage_spinbox->setValue(vp_refill_value);
    vp_refill_stop_combobox->setCurrentIndex(CSettings::instance()->getVpRefillStopCondition());
    vp_refill_stop_combobox_changed(vp_refill_stop_combobox->currentIndex());

    terafly::VirtualPyramid::_cacheHighestRes = CSettings::instance()->getVpCacheHighestRes();
    vp_highest_res_cache->setChecked(terafly::VirtualPyramid::_cacheHighestRes);
    terafly::VirtualPyramid::_freezeHighestRes = CSettings::instance()->getVpFreezeHighestRes();
    vp_highest_res_freeze->setChecked(terafly::VirtualPyramid::_cacheHighestRes);
}

void tf::PTabVolumeInfo::init()
{
    // do nothing if volume has not been imported yet
    iim::VirtualVolume* volume = CImport::instance()->getHighestResVolume();
    if(!volume)
        return;

    // general info
    double GVoxels = (volume->getDIM_V()/1000.0f)*(volume->getDIM_H()/1000.0f)*(volume->getDIM_D()/1000.0f);
    double TVoxels = GVoxels/1000.0;
    if(TVoxels < 0.1)
    {
        double GBytes = GVoxels*CImport::instance()->getVMapCDim()*volume->getBYTESxCHAN();
        vol_size_field->setText(tf::strprintf("  %.1f Gigavoxels (%.1f Gigabytes)", GVoxels, GBytes).c_str());
    }
    else
    {
        double TBytes = TVoxels*CImport::instance()->getVMapCDim();
        vol_size_field->setText(tf::strprintf("  %.1f Teravoxels (%.1f Terabytes)", TVoxels, TBytes).c_str());
    }

    vol_dims_mm_field->setText(
                tf::strprintf("  %.3f(x) x %.3f(y) x %.3f(z)",
                               fabs(volume->getDIM_H()*volume->getVXL_H()/1000.0f),
                               fabs(volume->getDIM_V()*volume->getVXL_V()/1000.0f),
                               fabs(volume->getDIM_D()*volume->getVXL_D()/1000.0f)).c_str());


    vol_dims_vxl_field->setText(
                tf::strprintf("  %d(x) x %d(y) x %d(z) x %d(c) x %d(t)",
                               volume->getDIM_H(),
                               volume->getDIM_V(),
                               volume->getDIM_D(), volume->getDIM_C(), volume->getDIM_T()).c_str());

    vol_format_field->setText(QString("  ") + volume->getPrintableFormat().c_str());
    vxl_field->setText(tf::strprintf("  %.3f(x) x %.3f(y) x %.3f(z)", volume->getVXL_H(), volume->getVXL_V(), volume->getVXL_D()).c_str());
    org_field->setText(tf::strprintf("  {%.3f(x), %.3f(y), %.3f(z)}", volume->getORG_H(), volume->getORG_V(), volume->getORG_D()).c_str());
    vol_path->setText(QString("  ") + volume->getROOT_DIR());
    // Virtual Pyramid info (do nothing if no virtual pyramid is found)
    tf::VirtualPyramid *virtualPyramid = CImport::instance()->getVirtualPyramid();
    if(!virtualPyramid)
        return;
    {
        std::vector <tf::HyperGridCache*> pyramid = virtualPyramid->cachePyramid();
        std::vector<iim::VirtualVolume*>  layers = virtualPyramid->virtualPyramid();

        if(vp_panel->isVisible() == false)
            vp_panel->setVisible(true);
        if(vp_ram_panel->isVisible() == false)
            vp_ram_panel->setVisible(true);
        if(vp_exploration_panel->isVisible() == false)
            vp_exploration_panel->setVisible(true);

        std::reverse(layers.begin(), layers.end());

        vp_path->setText(virtualPyramid->path().c_str());

        std::string resampling_factors;
        for(size_t i=0; i<layers.size(); i++)
            resampling_factors += dynamic_cast<tf::VirtualPyramidLayer*>(layers[i])->resamplingFactor().toString() + " ";
        vp_subsampling->setText(resampling_factors.c_str());

        vp_tiledims->setText(pyramid[0]->blockDim().toString().c_str());
        vp_tileformat->setText(pyramid[0]->blockFormat().c_str());

        vp_block_dimX->setValue(pyramid[0]->blockDim().x);
        vp_block_dimY->setValue(pyramid[0]->blockDim().y);
        vp_block_dimZ->setValue(pyramid[0]->blockDim().z);

        recheck_button_clicked();

        if(vp_ram_max_size < pyramid.size() + 1)
        {
            QMessageBox::critical(this, "Error", "Cannot initialize allocated RAM panel for Virtual Pyramid: #layers exceeds max_layers");
            return;
        }
        for(size_t i = 0; i<vp_ram_max_size; i++)
        {
            vp_ram_labels[i]->setVisible(i < pyramid.size() + 1);
            vp_ram_clear_buttons[i]->setVisible(i < pyramid.size() + 1);
            vp_ram_clear_buttons[i]->setEnabled(true);
            vp_ram_bars[i]->setVisible(i < pyramid.size() + 1);

            if(i < pyramid.size() + 1 && i>0)
                vp_ram_labels[i]->setText(dynamic_cast<tf::VirtualPyramidLayer*>(layers[i-1])->resamplingFactor().toString().c_str());
        }

        update();
    }
}

void tf::PTabVolumeInfo::open_button_clicked()
{
    QDesktopServices::openUrl(QUrl(QString("file:////") + vp_path->text()));
}

void tf::PTabVolumeInfo::recheck_button_clicked()
{
    float size = dir_size(vp_path->text());
    if(size < 1000)
        vp_size->setText((iim::strprintf("%.1f", size ) + " Bytes").c_str());
    else
    {
        size /= 1000;
        if(size < 1000)
            vp_size->setText((iim::strprintf("%.1f", size ) + " KB").c_str());
        else
        {
            size /= 1000;
            if(size < 1000)
                vp_size->setText((iim::strprintf("%.1f", size ) + " MB").c_str());
            else
            {
                size /= 1000;
                if(size < 1000)
                    vp_size->setText((iim::strprintf("%.1f", size ) + " GB").c_str());
            }
        }
    }
}

void tf::PTabVolumeInfo::clear_button_clicked()
{
    // checks
    if(vp_ram_panel->isVisible() == false)
        return;
    CViewer* viewer = CViewer::getCurrent();
    if(!viewer)
        return;
    if(!viewer->isInSafeState())
        return;
    tf::VirtualPyramid *virtualPyramid = CImport::instance()->getVirtualPyramid();
    if(!virtualPyramid)
        return;

    // clear selected layer
    int layer = -1;
    for(size_t i=1; i<vp_ram_clear_buttons.size() && layer == -1; i++)
        if(QObject::sender() == vp_ram_clear_buttons[i])
            layer = i-1;
    virtualPyramid->clear(true, layer);

    // refresh
    viewer->refresh();
}

quint64 tf::dir_size(const QString & str)
{
    quint64 sizex = 0;
    QFileInfo str_info(str);
    if (str_info.isDir())
    {
        QDir dir(str);
        QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs |  QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        for (int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            if(fileInfo.isDir())
                sizex += dir_size(fileInfo.absoluteFilePath());
            else
                sizex += fileInfo.size();
        }
    }
    return sizex;
}

void tf::PTabVolumeInfo::update()
{
    // checks
    CViewer* viewer = CViewer::getCurrent();
    if(!viewer)
        return;
    if(!viewer->isInSafeState())
        return;
    if(!vp_ram_panel->isVisible() )
        return;
    tf::VirtualPyramid *virtualPyramid = CImport::instance()->getVirtualPyramid();
    if(!virtualPyramid)
        return;

    // update RAM usage for each Virtual Pyramid cache layer
    std::vector <tf::HyperGridCache*> cache = virtualPyramid->cachePyramid();
    float sum = 0;
    for(int i=0; i<cache.size(); i++)
    {
        float allocableMB = cache[i]->memoryMax() * 1000;
        float allocatedMB = cache[i]->memoryUsed() * 1000;
        sum += allocatedMB;
        if(allocableMB < 100)
            vp_ram_bars[i+1]->setText(tf::strprintf("%.1f/%.1f MB", allocatedMB, allocableMB));
        else if(allocableMB < 100000)
            vp_ram_bars[i+1]->setText(tf::strprintf("%.1f/%.1f GB", allocatedMB/1000.0f, allocableMB/1000.0f));
        else
            vp_ram_bars[i+1]->setText(tf::strprintf("%.1f/%.1f TB", allocatedMB/1000000.0f, allocableMB/1000000.0f));
        vp_ram_bars[i+1]->setStep(tf::round(100*allocatedMB/allocableMB));

        if(viewer->getResIndex() == (cache.size()-i-1))
            vp_ram_labels[i+1]->setStyleSheet("QLabel { color : blue; }");
        else

            vp_ram_labels[i+1]->setStyleSheet("");
    }

    // update total RAM usage
    vp_ram_bars[0]->setText(tf::strprintf("%.1f/%.1f GB", sum / 1000.0f, vp_max_ram_spinbox->value()).c_str());
    vp_ram_bars[0]->setStep(tf::round(100*(sum/1000.0f)/vp_max_ram_spinbox->value()));


    // automatically release RAM resources if needed
    if(sum/1000 > vp_max_ram_spinbox->value())
    {
        virtualPyramid->clear(false);
        recheck_button_clicked();
    }

    // update local and global exploration bars
    iim::voi3D<> voi( iim::xyz<size_t>(viewer->volH0, viewer->volV0, viewer->volD0), iim::xyz<size_t>(viewer->volH1, viewer->volV1, viewer->volD1) );
    float completeness_local  = viewer->volResIndex == cache.size() - 1 ? 1.0f : cache[cache.size()-1-viewer->volResIndex]->completeness(voi);
    float completeness_global = cache[cache.size()-1]->completeness();

    vp_exploration_bar_local->setStep(tf::round(completeness_local*10000));
    vp_exploration_bar_global->setStep(tf::round(completeness_global*10000));
    vp_exploration_bar_local->setText(tf::strprintf( "Current VOI: %.2f %%", completeness_local*100));
    vp_exploration_bar_global->setText(tf::strprintf("Whole image: %.2f %%", completeness_global*100));

    // update refill time elapsed
    vp_refill_time_spent->setText(tf::strprintf(" %.2f seconds / %d blocks", refill_time_total ? refill_time_total/refill_blocks_total : refill_time_total, refill_blocks_total).c_str());

    // refill in background after 3 seconds inactivity
    if(vp_refill_auto_checkbox->isChecked() && inactivityDetector.timer.elapsed() >= 3000)
        vp_refill_button_clicked(true);
}

void tf::PTabVolumeInfo::ram_limit_changed(double v)
{
    CSettings::instance()->setRamLimitGB(v);
    update();
}

void tf::PTabVolumeInfo::vp_refill_button_clicked(bool in_background)
{
    // checks
    if(vp_refill_button->isVisible() == false)
        return;
    CViewer* viewer = CViewer::getCurrent();
    if(!viewer)
        return;
    if(!viewer->isInSafeState())
        return;
    tf::VirtualPyramid *virtualPyramid = CImport::instance()->getVirtualPyramid();
    if(!virtualPyramid)
        return;
    std::vector <tf::HyperGridCache*> cache = virtualPyramid->cachePyramid();
    if(vp_exploration_bar_local->finished())
    {
        if(!in_background) // need to be "quiet" if refill is done in background
            QMessageBox::information(this, "Warning", "No refill needed!\n%VOI explored already is 100%");
        return;
    }
    if(vp_refill_stop_combobox->currentIndex() == 1 &&    // until %VOI explored is <
       in_background == false                       &&    // operation is not in background
       vp_refill_coverage_spinbox->value() <= vp_exploration_bar_local->step()/100 )
    {
        QMessageBox::information(this, "Warning", tf::strprintf("No refill needed!\n%VOI explored already is >= %d%%", vp_refill_coverage_spinbox->value()).c_str());
        return;
    }

    // try to lock mutex for refill, if fails return immediately
    if(!refill_mutex.tryLock())
        return;

    // set busy state
    PMain& pMain = *(PMain::getInstance());
    pMain.progressBar->setEnabled(true);
    pMain.progressBar->setMinimum(0);
    pMain.progressBar->setMaximum(0);
    pMain.statusBar->showMessage("Refill...");
    viewer->view3DWidget->setCursor(Qt::BusyCursor);
    viewer->window3D->setCursor(Qt::BusyCursor);
    pMain.setCursor(Qt::BusyCursor);

    // determine the number of refills
    int n_refills = 0;
    // background refill mode: 1 refill at the time
    if(in_background)
        QMessageBox::information(this, "title", "do one refill");
    // refill stop condition: do exactly 'n' refills with 'n' set by user
    else if (vp_refill_stop_combobox->currentIndex() == 0)
        n_refills = vp_refill_times_spinbox->value();
    // refill stop condition: we must reach a user-defined local exploration coverage
    // we can't calculate in advance exactly how many refills are needed
    else
        n_refills = -1; // <-- means 'we don't know'
    int refill_coverage = vp_refill_coverage_spinbox->value();

    // determine block dim
    // - automatic refill --> will be optimized by Virtual Pyramid code
    // - manual refill    --> user defined
    tf::xyz<size_t> block_dim = tf::xyz<size_t>::biggest(); // will tell Virtual Pyramdid to do on its own
    if( ! vp_refill_auto_checkbox->isChecked())
    {
        block_dim.x = vp_block_dimX->value();
        block_dim.y = vp_block_dimY->value();
        block_dim.z = vp_block_dimZ->value();
    }

    // display endless progress bar
    QProgressDialog progress("Refill...", "Cancel", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.show();

    // refill
    try
    {
        bool stop = false;
        for(int i=1; stop == false; i++)
        {
            // refill here
            // @TODO: use a separate thread
            QElapsedTimer timer;
            timer.start();
            QApplication::processEvents();
            iim::voi3D<> voi( iim::xyz<size_t>(viewer->volH0, viewer->volV0, viewer->volD0), iim::xyz<size_t>(viewer->volH1, viewer->volV1, viewer->volD1) );
            virtualPyramid->refill(cache.size()-1-viewer->volResIndex, voi, tf::VirtualPyramid::refill_strategy(vp_refill_strategy_combobox->currentIndex()), block_dim);
            QApplication::processEvents();
            refill_time_total += timer.elapsed() / 1000.0f;
            refill_blocks_total++;

            // update GUI (otherwise it will freeze: we are blocking the event-loop thread)
            update();
            QApplication::processEvents();

            // update stop condition
            if(n_refills > 0)
                stop = i == n_refills;
            else
                stop = (viewer->volResIndex == cache.size() - 1 ? 1.0f : cache[cache.size()-1-viewer->volResIndex]->completeness(voi)) * 100 >= refill_coverage;

            // refresh viewer image data
            viewer->refresh();

            // terminate immediately if requested by the user
            if (progress.wasCanceled())
                break;
        }
    }
    catch(tf::RuntimeException & ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
    catch (iim::IOException & ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
    catch (iom::exception & ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
    progress.close();

    // release mutex
    refill_mutex.unlock();

    // unset busy state
    viewer->view3DWidget->setCursor(Qt::ArrowCursor);
    viewer->window3D->setCursor(Qt::ArrowCursor);
    pMain.resetGUI();
}

void tf::PTabVolumeInfo::empty_combobox_index_changed(int v)
{
    // checks
    if(vp_empty_viz_method_combobox->isVisible() == false)
        return;
    CViewer* viewer = CViewer::getCurrent();
    if(!viewer)
        return;
    if(!viewer->isInSafeState())
        return;
    tf::VirtualPyramid *virtualPyramid = CImport::instance()->getVirtualPyramid();
    if(!virtualPyramid)
        return;

    vp_empty_viz_intensity->setEnabled(v);

    if(v == 0)          // 100% black
        tf::VirtualPyramid::_unexploredFillingMethod = tf::VirtualPyramid::RAW;
    else if(v == 1)     // 100% intensity
        tf::VirtualPyramid::_unexploredFillingMethod = tf::VirtualPyramid::SOLID;
    else if(v == 2)     // black + 10% salt
    {
        tf::VirtualPyramid::_unexploredFillingMethod = tf::VirtualPyramid::SALT_AND_PEPPER;
        tf::VirtualPyramid::_unexploredSaltAndPepperPerc = 0.1;
    }
    else if(v == 3)     // black + 1% salt
    {
        tf::VirtualPyramid::_unexploredFillingMethod = tf::VirtualPyramid::SALT_AND_PEPPER;
        tf::VirtualPyramid::_unexploredSaltAndPepperPerc = 0.01;
    }
    else if(v == 4)     // black + 0.1% salt
    {
        tf::VirtualPyramid::_unexploredFillingMethod = tf::VirtualPyramid::SALT_AND_PEPPER;
        tf::VirtualPyramid::_unexploredSaltAndPepperPerc = 0.001;
    }

    // store new settings
    CSettings::instance()->setVpEmptyVizMethod(tf::VirtualPyramid::_unexploredFillingMethod);
    CSettings::instance()->setVpEmptyVizSaltPepperPercentage(tf::VirtualPyramid::_unexploredSaltAndPepperPerc);

    // refresh 3D viewer
    viewer->refresh();
}

void tf::PTabVolumeInfo::empty_intensity_value_changed(int v)
{
    // checks
    if(vp_empty_viz_intensity->isVisible() == false)
        return;
    // checks
    CViewer* viewer = CViewer::getCurrent();
    if(!viewer)
        return;
    if(!viewer->isInSafeState())
        return;
    tf::VirtualPyramid *virtualPyramid = CImport::instance()->getVirtualPyramid();
    if(!virtualPyramid)
        return;

    // store new settings
    tf::VirtualPyramid::_unexploredIntensityVal = v;
    CSettings::instance()->setVpEmptyVizIntensity(v);

    // refresh 3D viewer
    viewer->refresh();
}

void tf::PTabVolumeInfo::vp_refill_strategy_combobox_changed(int v)
{
//    if(vp_refill_strategy_combobox->isVisible() == false)
//        return;

    CSettings::instance()->setVpFetchMethod(v);
}

void tf::PTabVolumeInfo::vp_refill_times_spinbox_changed(int v)
{
//    if(vp_refill_times_spinbox->isVisible() == false)
//        return;

    CSettings::instance()->setVpFetchNBlocks(v);
}

void tf::PTabVolumeInfo::vp_refill_coverage_spinbox_changed(int v)
{
//    if(vp_refill_coverage_spinbox->isVisible() == false)
//        return;

    CSettings::instance()->setVpRefillCoverage(v);
}

void tf::PTabVolumeInfo::vp_refill_auto_checkbox_changed(bool v)
{
//    if(vp_refill_auto_checkbox->isVisible() == false)
//        return;

    vp_refill_button->setEnabled(!v);
    vp_refill_times_spinbox->setEnabled(!v);
    vp_refill_coverage_spinbox->setEnabled(!v);
    vp_refill_stop_combobox->setEnabled(!v);
    vp_block_dimX->setEnabled(!v);
    vp_block_dimY->setEnabled(!v);
    vp_block_dimZ->setEnabled(!v);

    CSettings::instance()->setVpRefillAuto(v);
}

void tf::PTabVolumeInfo::vp_refill_stop_combobox_changed(int v)
{
//    if(vp_refill_stop_combobox->isVisible() == false)
//        return;

    vp_refill_coverage_spinbox->setVisible(v);
    vp_refill_times_spinbox->setVisible(!v);

    CSettings::instance()->setVpRefillStopCondition(v);
}


void tf::PTabVolumeInfo::vp_highest_res_cache_checkbox_changed(bool v)
{
    terafly::VirtualPyramid::_cacheHighestRes = v;
    CSettings::instance()->setVpCacheHighestRes(v);
}

void tf::PTabVolumeInfo::vp_highest_res_freeze_checkbox_changed(bool v)
{
    terafly::VirtualPyramid::_freezeHighestRes = v;
    CSettings::instance()->setVpFreezeHighestRes(v);

    vp_refill_auto_checkbox->setEnabled(!v);
    vp_refill_button->setEnabled(!v);
    vp_refill_strategy_combobox->setEnabled(!v);
    vp_refill_stop_combobox->setEnabled(!v);
    vp_refill_time_spent->setEnabled(!v);
    vp_highest_res_cache->setEnabled(!v);
    vp_refill_coverage_spinbox->setEnabled(!v);
    vp_block_dimX->setEnabled(!v);
    vp_block_dimY->setEnabled(!v);
    vp_block_dimZ->setEnabled(!v);
}
