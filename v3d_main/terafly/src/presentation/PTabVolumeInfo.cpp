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

    // virtual pyramid panel
    vp_panel = new QGroupBox("Info", this);
#ifdef Q_OS_LINUX
    virtualpyramid_panel->setStyle(new QWindowsStyle());
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


    // virtual pyramid RAM panel
    vp_ram_panel = new QGroupBox("RAM usage", this);
    #ifdef Q_OS_LINUX
        vp_ram_panel->setStyle(new QWindowsStyle());
    #endif
    for(size_t i=0; i<vp_ram_max_size; i++)
    {
        vp_ram_labels.push_back(new QLabel(this));
        vp_ram_used_labels.push_back(new QLabel(this));
        vp_ram_bars.push_back(new QGradientBar(this));
        vp_ram_clear_buttons.push_back(new QPushButton("Clear", this));

        vp_ram_bars[i]->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    }
    vp_max_ram_spinbox = new QDoubleSpinBox(this);
    vp_max_ram_spinbox->setSuffix(" GB");
    vp_max_ram_spinbox->setDecimals(1);
    vp_max_ram_spinbox->setMinimum(0.5);
    vp_max_ram_spinbox->setAlignment(Qt::AlignCenter);
    vp_max_ram_spinbox->setSingleStep(0.1);
    vp_ram_show_res_buttons = new QPushButton(this);
    vp_ram_show_res_buttons->setCheckable(true);

    /*** LAYOUT SECTION ***/
    /* ---------------- info panel ------------------- */
    QGridLayout* info_panel_layout = new QGridLayout();
    QLabel* size_label = new QLabel("Size:");
    size_label->setFixedWidth(80);
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
    info_panel->setLayout(info_panel_layout);
    /* ----------- virtual pyramid panel ------------- */
    QGridLayout* vp_layout = new QGridLayout();
    QLabel* path_label = new QLabel("Path:");
    path_label->setFixedWidth(70);
    vp_layout->addWidget(path_label,                            0,0,1,1);
    vp_layout->addWidget(vp_path,                               0,1,1,1);
    vp_layout->addWidget(vp_open,                               0,2,1,1);
    vp_layout->addWidget(new QLabel("Size on disk:"),           1,0,1,1);
    vp_layout->addWidget(vp_size,                               1,1,1,1);
    vp_layout->addWidget(vp_recheck,                            1,2,1,1);
    vp_layout->addWidget(new QLabel("Layers:"),                 2,0,1,1);
    vp_layout->addWidget(vp_subsampling,                        2,1,1,2);
    vp_layout->addWidget(new QLabel("Block dims:"),             3,0,1,1);
    vp_layout->addWidget(vp_tiledims,                           3,1,1,2);
    vp_panel->setLayout(vp_layout);
    /* ----------- allocated RAM panel --------------- */
    QGridLayout* vp_RAM_layout = new QGridLayout();
    vp_RAM_layout->addWidget(new QLabel("Set limit to:"),       0,0,1,1);
    vp_RAM_layout->addWidget(vp_max_ram_spinbox,                0,1,1,1);
    vp_ram_labels[0]->setFixedWidth(70);
    vp_RAM_layout->addWidget(vp_ram_labels[0],                  1,0,1,1);
    vp_RAM_layout->addWidget(vp_ram_bars[0],                    1,1,1,1);
    vp_RAM_layout->addWidget(vp_ram_used_labels[0],             1,2,1,1);
    vp_RAM_layout->addWidget(vp_ram_clear_buttons[0],           1,3,1,1);
    vp_RAM_layout->addWidget(new QWidget(this),                 2,0,1,4);
    vp_RAM_layout->addWidget(vp_ram_show_res_buttons,           3,0,1,4);
    for(size_t i=1; i<vp_ram_max_size; i++)
    {
        vp_RAM_layout->addWidget(vp_ram_labels[i],              i+3,0,1,1);
        vp_RAM_layout->addWidget(vp_ram_bars[i],                i+3,1,1,1);
        vp_RAM_layout->addWidget(vp_ram_used_labels[i],         i+3,2,1,1);
        vp_RAM_layout->addWidget(vp_ram_clear_buttons[i],       i+3,3,1,1);
    }
    vp_ram_panel->setLayout(vp_RAM_layout);
    /* ---------------- MAIN LAYOUT ------------------ */
    QVBoxLayout* pyramid_layout = new QVBoxLayout();
    pyramid_layout->addWidget(vp_panel, 0);
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
    connect(vp_ram_show_res_buttons, SIGNAL(toggled(bool)), this, SLOT(show_ram_layers_toggled(bool)));

    for(size_t i=0; i<vp_ram_clear_buttons.size(); i++)
        connect(vp_ram_clear_buttons[i], SIGNAL(clicked()), this, SLOT(clear_button_clicked()));
}

void tf::PTabVolumeInfo::reset()
{
    vol_format_field->setText("");
    vol_size_field->setText("");
    vol_dims_mm_field->setText("");
    vol_dims_vxl_field->setText("");
    vxl_field->setText("");
    org_field->setText("");
    vp_path->setText("");
    vp_size->setText("");
    vp_subsampling->setText("");
    vp_tiledims->setText("");
    vp_panel->setVisible(false);
    vp_ram_panel->setVisible(false);

    for(int i=0; i<vp_ram_max_size; i++)
    {
        if(i==0)
            vp_ram_labels[i]->setText("Total:");
        else
            vp_ram_labels[i]->setText("");
        vp_ram_bars[i]->setNSteps(100);
        vp_ram_used_labels[i]->setText("0/0 MB");
    }

    vp_max_ram_spinbox->setValue(CSettings::instance()->getRamLimitGB());

    vp_ram_show_res_buttons->setText("Show all layers");
}

void tf::PTabVolumeInfo::init()
{
    iim::VirtualVolume* volume = CImport::instance()->getHighestResVolume();

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
    iim::VirtualVolume* volume_ith = dynamic_cast<TimeSeries*>(volume) ? dynamic_cast<TimeSeries*>(volume)->getFrameAt(0) : volume;
    vol_format_field->setText(QString("  ") + volume_ith->getPrintableFormat().c_str());
    vxl_field->setText(tf::strprintf("  %.3f(x) x %.3f(y) x %.3f(z)", volume->getVXL_H(), volume->getVXL_V(), volume->getVXL_D()).c_str());
    org_field->setText(tf::strprintf("  {%.3f(x), %.3f(y), %.3f(z)}", volume->getORG_H(), volume->getORG_V(), volume->getORG_D()).c_str());

    // Virtual Pyramid
    if(volume_ith->getPrintableFormat().find(tf::VirtualPyramidLayer::name()) != std::string::npos)
    {
        if(vp_panel->isVisible() == false)
            vp_panel->setVisible(true);
        if(vp_ram_panel->isVisible() == false)
            vp_ram_panel->setVisible(true);

        tf::VirtualPyramid *virtualPyramid = dynamic_cast<tf::VirtualPyramidLayer*>(volume_ith)->pyramid();
        std::vector <tf::HyperGridCache*> pyramid = virtualPyramid->cachePyramid();
        std::vector<iim::VirtualVolume*>  layers = virtualPyramid->virtualPyramid();
        std::reverse(layers.begin(), layers.end());

        vp_path->setText(virtualPyramid->path().c_str());

        std::string resampling_factors;
        for(size_t i=0; i<layers.size(); i++)
            resampling_factors += dynamic_cast<tf::VirtualPyramidLayer*>(layers[i])->resamplingFactor().toString() + " ";
        vp_subsampling->setText(resampling_factors.c_str());

        vp_tiledims->setText(pyramid[0]->blockDim().toString().c_str());

        recheck_button_clicked();

        if(vp_ram_max_size < pyramid.size() + 1)
        {
            QMessageBox::critical(this, "Error", "Cannot initialize allocated RAM panel for Virtual Pyramid: #layers exceeds max_layers");
            return;
        }
        for(size_t i = 0; i<vp_ram_max_size; i++)
        {
            vp_ram_labels[i]->setVisible(i < pyramid.size() + 1);
            vp_ram_used_labels[i]->setVisible(i < pyramid.size() + 1);
            vp_ram_clear_buttons[i]->setVisible(i < pyramid.size() + 1);
            vp_ram_clear_buttons[i]->setEnabled(true);
            vp_ram_bars[i]->setVisible(i < pyramid.size() + 1);

            if(i < pyramid.size() + 1 && i>0)
                vp_ram_labels[i]->setText(dynamic_cast<tf::VirtualPyramidLayer*>(layers[i-1])->resamplingFactor().toString().c_str());
        }
    }

    update();
    vp_ram_show_res_buttons->setChecked(true);
    show_ram_layers_toggled(true);
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
    if(vp_ram_panel->isVisible())
    {
        iim::VirtualVolume* volume = CImport::instance()->getHighestResVolume();
        if(!volume)
            return;
        iim::VirtualVolume* volume_ith = dynamic_cast<TimeSeries*>(volume) ? dynamic_cast<TimeSeries*>(volume)->getFrameAt(0) : volume;
        if(!volume_ith)
            return;
        tf::VirtualPyramid *virtualPyramid = dynamic_cast<tf::VirtualPyramidLayer*>(volume_ith)->pyramid();
        if(!virtualPyramid)
            return;

        int layer = -1;
        for(size_t i=1; i<vp_ram_clear_buttons.size() && layer == -1; i++)
            if(QObject::sender() == vp_ram_clear_buttons[i])
                layer = i-1;


        virtualPyramid->clear(true, layer);

        recheck_button_clicked();
    }
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
    if(vp_ram_panel->isVisible())
    {
        iim::VirtualVolume* volume = CImport::instance()->getHighestResVolume();
        if(!volume)
            return;
        iim::VirtualVolume* volume_ith = dynamic_cast<TimeSeries*>(volume) ? dynamic_cast<TimeSeries*>(volume)->getFrameAt(0) : volume;
        if(!volume_ith)
            return;
        tf::VirtualPyramid *virtualPyramid = dynamic_cast<tf::VirtualPyramidLayer*>(volume_ith)->pyramid();
        if(!virtualPyramid)
            return;
        std::vector <tf::HyperGridCache*> cache = virtualPyramid->cachePyramid();

        float sum = 0;
        for(int i=0; i<cache.size(); i++)
        {
            float allocableMB = cache[i]->maximumRamUsageGB() * 1000;
            float allocatedMB = cache[i]->currentRamUsageGB() * 1000;
            sum += allocatedMB;
            if(allocableMB < 100)
                vp_ram_used_labels[i+1]->setText(tf::strprintf("%.1f/%.1f MB", allocatedMB, allocableMB).c_str());
            else if(allocableMB < 100000)
                vp_ram_used_labels[i+1]->setText(tf::strprintf("%.1f/%.1f GB", allocatedMB/1000.0f, allocableMB/1000.0f).c_str());
            else
                vp_ram_used_labels[i+1]->setText(tf::strprintf("%.1f/%.1f TB", allocatedMB/1000000.0f, allocableMB/1000000.0f).c_str());
            vp_ram_bars[i+1]->setStep(tf::round(100*allocatedMB/allocableMB));

            if(CViewer::getCurrent()->getResIndex() == (cache.size()-i-1))
            {
                vp_ram_labels[i+1]->setStyleSheet("QLabel { color : red; }");
                vp_ram_used_labels[i+1]->setStyleSheet("QLabel { color : red; }");
            }
            else
            {
                vp_ram_labels[i+1]->setStyleSheet("");
                vp_ram_used_labels[i+1]->setStyleSheet("");
            }
        }

        vp_ram_used_labels[0]->setText(tf::strprintf("%.1f/%.1f GB", sum / 1000.0f, vp_max_ram_spinbox->value()).c_str());
        vp_ram_bars[0]->setStep(tf::round(100*(sum/1000.0f)/vp_max_ram_spinbox->value()));


        // automatically release RAM resources
        if(sum/1000 > vp_max_ram_spinbox->value() && CViewer::getCurrent()->isWaitingForData() == false)
        {
            virtualPyramid->clear(false);
            recheck_button_clicked();
        }
    }
}

void tf::PTabVolumeInfo::ram_limit_changed(double v)
{
    CSettings::instance()->setRamLimitGB(v);
    update();
}

void tf::PTabVolumeInfo::show_ram_layers_toggled(bool checked)
{
    if(checked)
        vp_ram_show_res_buttons->setText("Hide layers");
    else
        vp_ram_show_res_buttons->setText("Show layers");

    //if(vp_ram_panel->isVisible())
    {
        iim::VirtualVolume* volume = CImport::instance()->getHighestResVolume();
        if(!volume)
            return;
        iim::VirtualVolume* volume_ith = dynamic_cast<TimeSeries*>(volume) ? dynamic_cast<TimeSeries*>(volume)->getFrameAt(0) : volume;
        if(!volume_ith)
            return;
        tf::VirtualPyramid *virtualPyramid = dynamic_cast<tf::VirtualPyramidLayer*>(volume_ith)->pyramid();
        if(!virtualPyramid)
            return;
        std::vector <tf::HyperGridCache*> cache = virtualPyramid->cachePyramid();

        for(int i=0; i<cache.size(); i++)
        {
            vp_ram_labels[i+1]->setVisible(checked);
            vp_ram_used_labels[i+1]->setVisible(checked);
            vp_ram_bars[i+1]->setVisible(checked);
            vp_ram_clear_buttons[i+1]->setVisible(checked);
        }
    }
}

