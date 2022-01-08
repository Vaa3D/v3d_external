//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

#include "renderer_gl1.h"

#include "PDialogVirtualPyramid.h"
#include "PMain.h"
#include "../control/CImport.h"
#include "../control/CSettings.h"

using namespace terafly;

PDialogVirtualPyramid::PDialogVirtualPyramid(const std::string & _volumepath, iim::VirtualVolume *_volumeHandle, QWidget* parent) : QDialog(parent), volumePath(_volumepath), volume(_volumeHandle)
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    // header
    header = new QLabel("<html><table><tr style=\"vertical-align: middle;\"><td><img src=\":/icons/help.png\"></td>"
                          "<td><p style=\"text-align:justify; margin-left:10px; margin-right:3px\"> "
                              "This is the import dialog for <b>Unconverted</b> and <b>Unstitched</b> volume formats.<br>"
                              "TeraFly will now setup a <b>Virtual Pyramid</b> on top of your volume to enable fast multiresolution-based 3D visualization.</td></tr></table> </html>");
    header->setStyleSheet("border: 1px solid; border-color: gray; background-color: rgb(255,255,255); margin-top:10px; margin-bottom:10px; padding-top:10px; padding-bottom:10px;");
    header->setWordWrap(true);
    header->setFixedHeight(100);

    // images and associated label
    pyramid_image_full = new QPixmap(":/icons/virtual.pyramid.full.png");
    pyramid_image_empty = new QPixmap(":/icons/virtual.pyramid.empty.png");
    pyramid_image_lowres = new QPixmap(":/icons/virtual.pyramid.lowres.png");
    pyramid_image_label = new QLabel(this);

    // subsampling panel
    subsampling_panel = new QGroupBox("Subsampling", this);
#ifdef Q_OS_LINUX
    subsampling_panel->setStyle(new QWindowsStyle());
#endif
    auto_radiobutton = new QRadioButton("auto", this);
    subsampling_spinbox = new QSpinBox(this);
    subsampling_spinbox->setMinimum(2);
    subsampling_spinbox->setPrefix(" downsample by ");
    subsampling_spinbox->setAlignment(Qt::AlignCenter);
    //subsampling_spinbox->setSuffix(" along x,y,z");
    lowerbound_spinbox = new QSpinBox(this);
    lowerbound_spinbox->setMinimum(1);
    lowerbound_spinbox->setPrefix(" until size is < ");
    lowerbound_spinbox->setSuffix(" MVoxels");
    lowerbound_spinbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    lowerbound_spinbox->setMinimumWidth(200);
    lowerbound_spinbox->setAlignment(Qt::AlignCenter);
    manual_radiobutton = new QRadioButton("manual", this);
    subsamplings_line = new QPrefixSuffixLineEdit(" subsampling xyz triplets: ", "", this);
    blockx_spinbox = new QSpinBox(this);
    blockx_spinbox->setMinimum(64);
    blockx_spinbox->setMaximum(2048);
    blockx_spinbox->setSuffix(" (x)");
    blockx_spinbox->setAlignment(Qt::AlignCenter);
    blocky_spinbox = new QSpinBox(this);
    blocky_spinbox->setMinimum(64);
    blocky_spinbox->setMaximum(2048);
    blocky_spinbox->setSuffix(" (y)");
    blocky_spinbox->setAlignment(Qt::AlignCenter);
    blockz_spinbox = new QSpinBox(this);
    blockz_spinbox->setMinimum(64);
    blockz_spinbox->setMaximum(2048);
    blockz_spinbox->setAlignment(Qt::AlignCenter);
    blockz_spinbox->setSuffix(" (z)");
    space_required_line = new QLineEdit(this);
    space_required_line->setReadOnly(true);
    QButtonGroup *subsampling_group = new QButtonGroup(this);
    subsampling_group->addButton(auto_radiobutton);
    subsampling_group->addButton(manual_radiobutton);

    saveto_panel = new QGroupBox("Save to", this);
#ifdef Q_OS_LINUX
    saveto_panel->setStyle(new QWindowsStyle());
#endif
    local_radiobutton = new QRadioButton("local folder (Vaa3D)", this);
    storage_radiobutton = new QRadioButton("volume's folder", this);
    QButtonGroup *storage_group = new QButtonGroup(this);
    storage_group->addButton(local_radiobutton);
    storage_group->addButton(storage_radiobutton);
    volumepath_line = new QLineEdit(this);
    volumepath_line->setReadOnly(true);
    block_format_combobox = new QComboBox(this);
    block_format_combobox->addItem(".tif");
    block_format_combobox->addItem(".v3draw");

    lowres_panel = new QGroupBox("Preconversion", this);
#ifdef Q_OS_LINUX
    lowres_panel->setStyle(new QWindowsStyle());
#endif
    QButtonGroup *lowres_group = new QButtonGroup(this);
    imagefile_radiobutton = new QRadioButton("load lowest-res layer", this);
    imagefile_line = new QLineEdit(this);
    imagefile_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    browse_button = new QPushButton("Browse", this);
    generate_radiobutton = new QRadioButton("generate lowest-res layer now (slow)", this);
    generate_all_radiobutton = new QRadioButton("generate all layers now (very slow)", this);
    generate_spinbox = new QSpinBox(this);
    generate_spinbox->setMinimum(1);
    generate_spinbox->setPrefix(" sample 1 every ");
    generate_spinbox->setSuffix(" pixels");
    generate_spinbox->setAlignment(Qt::AlignCenter);
    noimage_radiobutton = new QRadioButton("none (all layers initially empty)", this);
    lowres_group->addButton(imagefile_radiobutton);
    lowres_group->addButton(generate_radiobutton);
    lowres_group->addButton(generate_all_radiobutton);
    lowres_group->addButton(noimage_radiobutton);


    qbuttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);



    /*** LAYOUT SECTION ***/
    /* ---------------- subsampling ------------------- */
    QVBoxLayout *subsampling_layout = new QVBoxLayout();
    QHBoxLayout *auto_layout = new QHBoxLayout();
    auto_layout->addWidget(auto_radiobutton);
    auto_layout->addWidget(subsampling_spinbox);
    auto_layout->addWidget(lowerbound_spinbox);
    subsampling_layout->addLayout(auto_layout);
    QHBoxLayout *manual_layout = new QHBoxLayout();
    manual_layout->addWidget(manual_radiobutton);
    manual_layout->addWidget(subsamplings_line);
    subsampling_layout->addLayout(manual_layout);
    subsampling_panel->setLayout(subsampling_layout);
    auto_radiobutton->setFixedWidth(100);
    manual_radiobutton->setFixedWidth(100);
    QLabel* block_label = new QLabel("block size:", this);
    block_label->setFixedWidth(100);
    QHBoxLayout* block_layout = new QHBoxLayout();
    block_layout->addWidget(block_label);
    block_layout->addWidget(blockx_spinbox);
    block_layout->addWidget(blocky_spinbox);
    block_layout->addWidget(blockz_spinbox);
    subsampling_layout->addLayout(block_layout);
    QHBoxLayout* space_required_layout = new QHBoxLayout();
    QLabel* space_required_label = new QLabel("space required:");
    space_required_label->setFixedWidth(100);
    space_required_layout->addWidget(space_required_label);
    space_required_layout->addWidget(space_required_line);
    subsampling_layout->addLayout(space_required_layout);
    /* ------------------ save to --------------------- */
    QVBoxLayout *storage_layout = new QVBoxLayout();
    QHBoxLayout *storage_buttons_layout = new QHBoxLayout();
    storage_buttons_layout->addWidget(local_radiobutton);
    storage_buttons_layout->addWidget(storage_radiobutton);
    QLabel *block_format_label = new QLabel("format:");
    block_format_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    storage_buttons_layout->addWidget(block_format_label);
    storage_buttons_layout->addWidget(block_format_combobox);
    storage_layout->addLayout(storage_buttons_layout);
    storage_layout->addWidget(volumepath_line);
    saveto_panel->setLayout(storage_layout);
    /* --------------- preprocessing ------------------ */
    QVBoxLayout *lowres_layout = new QVBoxLayout();
    QHBoxLayout *imagefile_layout = new QHBoxLayout();
    imagefile_layout->addWidget(imagefile_radiobutton);
    imagefile_layout->addWidget(imagefile_line);
    imagefile_layout->addWidget(browse_button);
    QHBoxLayout *generate_layout = new QHBoxLayout();
    generate_layout->addWidget(generate_radiobutton);
    generate_layout->addWidget(generate_spinbox);
    QHBoxLayout *generate_all_layout = new QHBoxLayout();
    generate_all_layout->addWidget(generate_all_radiobutton);
    QHBoxLayout *noimage_layout = new QHBoxLayout();
    noimage_layout->addWidget(noimage_radiobutton);
    lowres_layout->addLayout(noimage_layout);
    lowres_layout->addLayout(imagefile_layout);
    lowres_layout->addLayout(generate_layout);
    lowres_layout->addLayout(generate_all_layout);
    lowres_panel->setLayout(lowres_layout);
    //generate_spinbox->setFixedWidth(300);
    /* ------------------ OVERALL --------------------- */
    QVBoxLayout *inputLayout = new QVBoxLayout();
    inputLayout->setContentsMargins(0,0,0,0);
    inputLayout->setSpacing(5);
    inputLayout->addWidget(subsampling_panel);
    inputLayout->addWidget(lowres_panel);
    inputLayout->addWidget(saveto_panel);
    inputLayout->addWidget(qbuttons);

    QHBoxLayout *innerLayout = new QHBoxLayout();
    innerLayout->setSpacing(0);
    innerLayout->setContentsMargins(0,0,0,0);
    innerLayout->addWidget(pyramid_image_label);
    innerLayout->addSpacing(10);
    innerLayout->addLayout(inputLayout);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(20,20,20,20);
    layout->addWidget(header);
    layout->addLayout(innerLayout);
    setLayout(layout);

    //windows flags and title
    this->setWindowTitle("Virtual pyramid Setup");
    this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);

    // signals and slots
    connect(qbuttons, SIGNAL(accepted()), this, SLOT(ok_button_clicked()));
    connect(qbuttons, SIGNAL(rejected()), this, SLOT(close()));
    connect(local_radiobutton, SIGNAL(clicked()), this, SLOT(storage_radiobutton_changed()));
    connect(storage_radiobutton, SIGNAL(clicked()), this, SLOT(storage_radiobutton_changed()));
    connect(auto_radiobutton, SIGNAL(clicked()), this, SLOT(subsampling_radiobutton_changed()));
    connect(manual_radiobutton, SIGNAL(clicked()), this, SLOT(subsampling_radiobutton_changed()));
    connect(imagefile_radiobutton, SIGNAL(clicked()), this, SLOT(lowres_radiobutton_changed()));
    connect(generate_radiobutton, SIGNAL(clicked()), this, SLOT(lowres_radiobutton_changed()));
    connect(generate_all_radiobutton, SIGNAL(clicked()), this, SLOT(lowres_radiobutton_changed()));
    connect(noimage_radiobutton, SIGNAL(clicked()), this, SLOT(lowres_radiobutton_changed()));
    connect(browse_button, SIGNAL(clicked()), this, SLOT(browse_button_clicked()));
    connect(subsampling_spinbox, SIGNAL(valueChanged(int)), this, SLOT(subsampling_spinbox_changed(int)));
    connect(subsamplings_line, SIGNAL(textChanged(QString)), this, SLOT(subsamplings_line_changed(QString)));
    connect(block_format_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(block_format_combobox_changed(int)));

    reset();

    this->adjustSize();
    //setFixedWidth(900);
    //setFixedHeight(600);

    this->move(QApplication::desktop()->screen()->rect().center() - this->rect().center());

    /**/tf::debug(tf::LEV1, "object successfully constructed", __itm__current__function__);
}

//reset method
void PDialogVirtualPyramid::reset()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    auto_radiobutton->setChecked(true);
    subsampling_spinbox->setValue(CSettings::instance()->getPyramidResamplingFactor());
    subsamplings_line->setText("{1,1,1} {a,b,c} {d,e,f} ...");
    subsampling_radiobutton_changed();
    lowerbound_spinbox->setValue( tf::round(PMain::getInstance()->Vdim_sbox->value() * PMain::getInstance()->Hdim_sbox->value() * PMain::getInstance()->Ddim_sbox->value() / 1000000.0f));

    local_radiobutton->setChecked(true);
    storage_radiobutton_changed();

    blockx_spinbox->setValue(256);
    blocky_spinbox->setValue(256);
    blockz_spinbox->setValue(256);

    block_format_combobox->setCurrentIndex(CSettings::instance()->getVpBlockFormatIndex());

    if(QFile(tf::VirtualPyramid::pathLowRes(volumePath).c_str()).exists())
        imagefile_line->setText(tf::VirtualPyramid::pathLowRes(volumePath).c_str());
    else
        imagefile_line->setText(" lowres.tif not found");
    generate_spinbox->setValue(16);
    noimage_radiobutton->setChecked(true);
    lowres_radiobutton_changed();
}


void PDialogVirtualPyramid::subsampling_spinbox_changed(int v)
{
    tf::CSettings::instance()->setPyramidResamplingFactor(v);
    update_space_required();
}


void PDialogVirtualPyramid::block_format_combobox_changed(int v)
{
    CSettings::instance()->setVpBlockFormatIndex(v);
}

void PDialogVirtualPyramid::subsamplings_line_changed(QString)
{
    update_space_required();
}

void PDialogVirtualPyramid::ok_button_clicked()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    try
    {
        if(generate_radiobutton->isChecked())
            throw RuntimeException("Generation of low-res image on the fly not yet supported. Please choose another option.");
        if(generate_all_radiobutton->isChecked())
            throw RuntimeException("Generation of all resolution layers on the fly not yet supported. Please choose another option.");

        // set initialization mode and associated parameters
        if(noimage_radiobutton->isChecked())
            CImport::instance()->vpMode = tf::VirtualPyramid::DEFAULT;
        else if(imagefile_radiobutton->isChecked())
        {
            CImport::instance()->vpMode = tf::VirtualPyramid::GENERATE_LOW_RES_FROM_FILE;
            CImport::instance()->vpLowResImagePath = imagefile_line->text().toStdString();
        }
        else if(generate_radiobutton->isChecked())
        {
            CImport::instance()->vpMode = tf::VirtualPyramid::GENERATE_LOW_RES;
            CImport::instance()->vpSampling = generate_spinbox->value();
        }

        // set resampling factors
        if(auto_radiobutton->isChecked())
        {
            CImport::instance()->vpResamplingFactor = subsampling_spinbox->value();
            CImport::instance()->vpLowerBound = lowerbound_spinbox->value();
        }
        else if(manual_radiobutton->isChecked())
        {
            std::vector< tf::xyz<int> > resampling_factors;
            std::string line = subsamplings_line->text().toStdString();
            std::vector <std::string> tokens;
            tf::split(line, "{", tokens);
            for(size_t i=1; i<tokens.size(); i++)
            {
                std::string triplet = tf::cls(tokens[i]);
                std::vector <std::string> items;
                tf::parse(triplet, ",", 3, "<no file>", items);
                resampling_factors.push_back(tf::xyz<int>(tf::str2num<int>(items[0]), tf::str2num<int>(items[1]), tf::str2num<int>(items[2]) ));
            }
            CImport::instance()->vpResamplingFactors = resampling_factors;
        }

        CImport::instance()->vpLocal = local_radiobutton->isChecked();
        CImport::instance()->vpBlockDims.x = blockx_spinbox->value();
        CImport::instance()->vpBlockDims.y = blocky_spinbox->value();
        CImport::instance()->vpBlockDims.z = blockz_spinbox->value();
        CImport::instance()->vpBlockFormat = block_format_combobox->currentText().toStdString();

        accept();
        close();
    }
    catch(RuntimeException &ex)
    {
        QMessageBox::critical(this,QObject::tr("Error"), QObject::tr(ex.what()),QObject::tr("Ok"));
    }
}

void PDialogVirtualPyramid::browse_button_clicked()
{
    /**/tf::debug(tf::LEV1, 0, __itm__current__function__);

    std::string filter = "TIFF files (*.tif *.TIF *.tiff *.TIFF)" ;
    std::string path;

    #ifdef _USE_QT_DIALOGS
    QFileDialog dialog(0);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr(filter.c_str()));
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    dialog.setWindowTitle("Select file");
    dialog.setDirectory(volumePath.c_str());
    if(dialog.exec())
        path = dialog.directory().absolutePath().toStdString();

    #else
    path = QFileDialog::getOpenFileName(this, "Select file", volumePath.c_str(), tr(filter.c_str())).toStdString();
    #endif

    if (!path.empty())
        imagefile_line->setText(path.c_str());
}


void PDialogVirtualPyramid::storage_radiobutton_changed()
{
    if(QObject::sender() == storage_radiobutton)
        volumepath_line->setText(tf::VirtualPyramid::pathRemote(volumePath).c_str());
    else
        volumepath_line->setText(tf::VirtualPyramid::pathLocal(volumePath).c_str());
}

void PDialogVirtualPyramid::subsampling_radiobutton_changed()
{
    if(QObject::sender() == manual_radiobutton)
    {
        subsampling_spinbox->setEnabled(false);
        lowerbound_spinbox->setEnabled(false);
        subsamplings_line->setEnabled(true);
    }
    else
    {
        subsampling_spinbox->setEnabled(true);
        lowerbound_spinbox->setEnabled(true);
        subsamplings_line->setEnabled(false);
    }

    update_space_required();
}

void PDialogVirtualPyramid::lowres_radiobutton_changed()
{
    if(QObject::sender() == imagefile_radiobutton)
    {
        imagefile_line->setEnabled(true);
        browse_button->setEnabled(true);

        generate_spinbox->setEnabled(false);


        pyramid_image_label->setPixmap(*pyramid_image_lowres);//->scaled(374, 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
    else if(QObject::sender() == generate_radiobutton)
    {
        imagefile_line->setEnabled(false);
        browse_button->setEnabled(false);

        generate_spinbox->setEnabled(true);


        pyramid_image_label->setPixmap(*pyramid_image_lowres);//->scaled(365, 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
    else if(QObject::sender() == generate_all_radiobutton)
    {
        imagefile_line->setEnabled(false);
        browse_button->setEnabled(false);

        generate_spinbox->setEnabled(true);


        pyramid_image_label->setPixmap(*pyramid_image_full);//->scaled(365, 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
    else
    {
        imagefile_line->setEnabled(false);
        browse_button->setEnabled(false);

        generate_spinbox->setEnabled(false);


        pyramid_image_label->setPixmap(*pyramid_image_empty);//->scaled(365, 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
}

void PDialogVirtualPyramid::update_space_required()
{
    try
    {
        float GB = 0;
        if(auto_radiobutton->isChecked())
            GB = tf::VirtualPyramid::predictGB(volume, subsampling_spinbox->value(), lowerbound_spinbox->value());
        else if(manual_radiobutton->isChecked())
        {
            std::vector< tf::xyz<int> > resampling_factors;
            std::string line = subsamplings_line->text().toStdString();
            std::vector <std::string> tokens;
            tf::split(line, "{", tokens);
            for(size_t i=1; i<tokens.size(); i++)
            {
                std::string triplet = tf::cls(tokens[i]);
                std::vector <std::string> items;
                tf::parse(triplet, ",", 3, "<no file>", items);
                resampling_factors.push_back(tf::xyz<int>(tf::str2num<int>(items[0]), tf::str2num<int>(items[1]), tf::str2num<int>(items[2]) ));
            }

            GB = tf::VirtualPyramid::predictGB(volume, resampling_factors);
        }
        if(GB)
            space_required_line->setText(tf::strprintf(" %.2f GB  at most ( 0%% compression and 100%% explored )", GB).c_str());
        else
            space_required_line->setText("n.a. (empty virtual pyramid)");
    }
    catch (tf::RuntimeException & ex)
    {
        space_required_line->setText(QString("n.a. (cannot parse subsampling line)"));
    }
}

