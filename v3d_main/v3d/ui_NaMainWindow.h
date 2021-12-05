/********************************************************************************
** Form generated from reading UI file 'NaMainWindow.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NAMAINWINDOW_H
#define UI_NAMAINWINDOW_H

#include <../neuron_annotator/gui/CompartmentMapWidget.h>
#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QSlider>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>
#include "../neuron_annotator/gui/AngleWidget.h"
#include "../neuron_annotator/gui/AnnotationWidget.h"
#include "../neuron_annotator/gui/CompartmentMapComboBox.h"
#include "../neuron_annotator/gui/FragmentGalleryWidget.h"
#include "../neuron_annotator/gui/GammaWidget.h"
#include "../neuron_annotator/gui/Na3DWidget.h"
#include "../neuron_annotator/gui/NaLargeMIPWidget.h"
#include "../neuron_annotator/gui/NaZStackWidget.h"
#include "../neuron_annotator/gui/ZoomWidget.h"

QT_BEGIN_NAMESPACE

class Ui_NaMainWindow
{
public:
    QAction *actionOpen;
    QAction *actionLog_off;
    QAction *actionAbout_NeuronAnnotator;
    QAction *actionQuit;
    QAction *actionV3DDefault_2;
    QAction *actionNeuronAnnotator_2;
    QAction *actionLoad_Tiff;
    QAction *actionLoad_2D_MIP_image;
    QAction *actionCell_Counter_3D_2ch_lsm;
    QAction *actionPreferences;
    QAction *actionV3D_default;
    QAction *actionNeuron_annotator;
    QAction *actionV3DDefault;
    QAction *actionNeuronAnnotator;
    QAction *actionLink_viewers;
    QAction *actionShow_Crosshair;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionSelect_All;
    QAction *actionSelect_None;
    QAction *actionInvert_Selection;
    QAction *action3D_Volume;
    QAction *action2D_MIP;
    QAction *actionScreenShot;
    QAction *actionFoo;
    QAction *actionClear_Hide_All;
    QAction *actionShow_all_neurons_in_empty_space;
    QAction *actionMono_Off;
    QAction *actionLeft_eye_view;
    QAction *actionRight_eye_view;
    QAction *actionAnaglyph_Red_Cyan;
    QAction *actionAnaglyph_Green_Magenta;
    QAction *actionRow_Interleaved_Zalman;
    QAction *actionQuadro_120_Hz;
    QAction *actionSwap_Eyes;
    QAction *actionShow_Axes;
    QAction *action3D_alpha_blending;
    QAction *actionAnimate_3D_nutation;
    QAction *actionDynamic_range;
    QAction *actionShow_Brain_Compartments;
    QAction *actionOpen_Single_Movie_Stack;
    QAction *actionCurrent_MIP;
    QAction *actionChecker_Interleaved_3DTV;
    QAction *actionFull_Screen;
    QAction *actionColumn_Interleaved;
    QAction *actionOpen_Reference_Channel;
    QAction *actionOpen_Neuron_Mask;
    QAction *actionLoad_movie_as_texture;
    QAction *actionLoad_fast_separation_result;
    QAction *actionX_Rotation_Movie;
    QAction *actionMeasure_Frame_Rate;
    QAction *actionOpen_Octree_Volume;
    QAction *actionOpen_microCT_Cut_Planner;
    QAction *actionAppend_key_frame_at_current_view;
    QAction *actionPlay_movie;
    QAction *actionClear_movie;
    QAction *actionSave_movie_frames;
    QAction *action1280x720;
    QAction *actionAdd_landmark_at_cursor;
    QAction *actionClear_landmarks;
    QAction *actionShow_Scale_Bar;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QFrame *viewerAndMIPsFrame;
    QVBoxLayout *verticalLayout;
    QFrame *viewerAndControlsFrame;
    QHBoxLayout *horizontalLayout_2;
    QStackedWidget *viewerStackedWidget;
    QWidget *viewerMIP_Page;
    QHBoxLayout *horizontalLayout_11;
    NaLargeMIPWidget *naLargeMIPWidget;
    QHBoxLayout *horizontalLayout_7;
    QSpacerItem *horizontalSpacer_5;
    QWidget *progressWidgetMip;
    QVBoxLayout *verticalLayout_15;
    QLabel *progressLabelMip;
    QProgressBar *progressBarMip;
    QSpacerItem *horizontalSpacer_6;
    QWidget *viewerZ_Page;
    QHBoxLayout *horizontalLayout_10;
    QFrame *frame_5;
    QVBoxLayout *verticalLayout_12;
    NaZStackWidget *naZStackWidget;
    QHBoxLayout *horizontalLayout_8;
    QSpacerItem *horizontalSpacer_7;
    QWidget *progressWidgetZ;
    QVBoxLayout *verticalLayout_16;
    QLabel *progressLabelZ;
    QProgressBar *progressBarZ;
    QSpacerItem *horizontalSpacer_8;
    QScrollBar *ZSlice_horizontalScrollBar;
    QWidget *viewer3D_Page;
    QHBoxLayout *horizontalLayout_3;
    QFrame *frame_3dwidget;
    QVBoxLayout *verticalLayout_5;
    QWidget *widget_progress3d;
    QHBoxLayout *horizontalLayout_5;
    QLabel *progressLabel3d;
    QProgressBar *progressBar3d;
    Na3DWidget *v3dr_glwidget;
    QWidget *viewer_busy_loading_Page;
    QVBoxLayout *verticalLayout_17;
    QSpacerItem *verticalSpacer_7;
    QLabel *label_2;
    QProgressBar *progressBar;
    QSpacerItem *verticalSpacer_8;
    QFrame *viewerSelectorAndControlFrame;
    QVBoxLayout *verticalLayout_7;
    QTabWidget *viewerControlTabWidget;
    QWidget *largeMipViewerControlPage;
    QVBoxLayout *verticalLayout_10;
    QSpacerItem *verticalSpacer_4;
    QWidget *zSliceViewerControlPage;
    QHBoxLayout *horizontalLayout_12;
    QFrame *frame;
    QVBoxLayout *verticalLayout_4;
    QGroupBox *HDR_groupBox;
    QVBoxLayout *verticalLayout_11;
    QCheckBox *HDR_checkBox;
    QFrame *frame_3;
    QHBoxLayout *horizontalLayout_13;
    QPushButton *HDRRed_pushButton;
    QPushButton *HDRGreen_pushButton;
    QPushButton *HDRBlue_pushButton;
    QPushButton *HDRNc82_pushButton;
    QFrame *frame_4;
    QHBoxLayout *horizontalLayout_14;
    QLabel *BoxSize_label;
    QSpinBox *BoxSize_spinBox;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *ZSlice_groupBox;
    QHBoxLayout *horizontalLayout_15;
    QSpinBox *ZSlice_spinBox;
    QLabel *ZSliceTotal_label;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *verticalSpacer_2;
    QWidget *viewer3DControlPage;
    QVBoxLayout *verticalLayout_3;
    QFrame *viewerControl3DFrame;
    QVBoxLayout *verticalLayout_18;
    QVBoxLayout *verticalLayout_14;
    QTabWidget *controlTabWidget;
    QWidget *tabRotation;
    AngleWidget *rotXWidget;
    AngleWidget *rotZWidget;
    AngleWidget *rotYWidget;
    QPushButton *rotationResetButton;
    QCheckBox *nutateButton;
    QWidget *tabVolumeCut;
    QVBoxLayout *verticalLayout_19;
    QCheckBox *XCutCB;
    QSlider *XcminSlider;
    QSlider *XcmaxSlider;
    QCheckBox *YCutCB;
    QSlider *YcminSlider;
    QSlider *YcmaxSlider;
    QCheckBox *ZCutCB;
    QSlider *ZcminSlider;
    QSlider *ZcmaxSlider;
    QFrame *frame_6;
    QHBoxLayout *horizontalLayout_16;
    QLabel *label_3;
    QPushButton *freezeFrontBackButton;
    QSlider *slabThicknessSlider;
    QSlider *slabPositionSlider;
    QFrame *frame_2;
    QHBoxLayout *horizontalLayout_9;
    QPushButton *defineClipPlaneButton;
    QPushButton *customCutButton;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_17;
    QLabel *label_4;
    QDoubleSpinBox *zThicknessDoubleSpinBox;
    QSpacerItem *verticalSpacer_5;
    QGroupBox *compartmentSelectGroupBox;
    QVBoxLayout *verticalLayout_8;
    QGridLayout *gridLayout;
    CompartmentMapComboBox *compartmentMapComboBox;
    CompartmentMapWidget *compartmentMapWidget;
    QSpacerItem *verticalSpacer;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_9;
    ZoomWidget *zoomWidget;
    QPushButton *resetViewButton;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_6;
    GammaWidget *sharedGammaWidget;
    GammaWidget *referenceGammaWidget;
    QFrame *sharedColorFrame;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *redToggleButton;
    QPushButton *greenToggleButton;
    QPushButton *blueToggleButton;
    QPushButton *resetColorsButton;
    QFrame *mipsFrame;
    QHBoxLayout *horizontalLayout_4;
    QFrame *maskFrame;
    QFrame *galleryFrame;
    QVBoxLayout *verticalLayout_2;
    FragmentGalleryWidget *fragmentGalleryWidget;
    QWidget *widget;
    QVBoxLayout *verticalLayout_13;
    QLabel *label;
    QPushButton *gallerySortBySizeButton;
    QPushButton *gallerySortByIndexButton;
    QPushButton *gallerySortByColorButton;
    QPushButton *gallerySortByNameButton;
    QSpacerItem *verticalSpacer_6;
    AnnotationWidget *annotationFrame;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuExport;
    QMenu *menuOpen_Recent;
    QMenu *menuHelp;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuStereo_3D;
    QMenu *menuAnimate;
    QMenu *menuSet_canvas_size;
    QMenu *menuWork_Mode;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *NaMainWindow)
    {
        if (NaMainWindow->objectName().isEmpty())
            NaMainWindow->setObjectName(QString::fromUtf8("NaMainWindow"));
        NaMainWindow->resize(1367, 1044);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(NaMainWindow->sizePolicy().hasHeightForWidth());
        NaMainWindow->setSizePolicy(sizePolicy);
        actionOpen = new QAction(NaMainWindow);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        actionOpen->setEnabled(true);
        actionOpen->setAutoRepeat(false);
        actionLog_off = new QAction(NaMainWindow);
        actionLog_off->setObjectName(QString::fromUtf8("actionLog_off"));
        actionAbout_NeuronAnnotator = new QAction(NaMainWindow);
        actionAbout_NeuronAnnotator->setObjectName(QString::fromUtf8("actionAbout_NeuronAnnotator"));
        actionAbout_NeuronAnnotator->setMenuRole(QAction::AboutRole);
        actionQuit = new QAction(NaMainWindow);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        actionQuit->setMenuRole(QAction::QuitRole);
        actionV3DDefault_2 = new QAction(NaMainWindow);
        actionV3DDefault_2->setObjectName(QString::fromUtf8("actionV3DDefault_2"));
        actionV3DDefault_2->setCheckable(true);
        actionNeuronAnnotator_2 = new QAction(NaMainWindow);
        actionNeuronAnnotator_2->setObjectName(QString::fromUtf8("actionNeuronAnnotator_2"));
        actionNeuronAnnotator_2->setCheckable(true);
        actionLoad_Tiff = new QAction(NaMainWindow);
        actionLoad_Tiff->setObjectName(QString::fromUtf8("actionLoad_Tiff"));
        actionLoad_2D_MIP_image = new QAction(NaMainWindow);
        actionLoad_2D_MIP_image->setObjectName(QString::fromUtf8("actionLoad_2D_MIP_image"));
        actionCell_Counter_3D_2ch_lsm = new QAction(NaMainWindow);
        actionCell_Counter_3D_2ch_lsm->setObjectName(QString::fromUtf8("actionCell_Counter_3D_2ch_lsm"));
        actionPreferences = new QAction(NaMainWindow);
        actionPreferences->setObjectName(QString::fromUtf8("actionPreferences"));
        actionPreferences->setMenuRole(QAction::PreferencesRole);
        actionV3D_default = new QAction(NaMainWindow);
        actionV3D_default->setObjectName(QString::fromUtf8("actionV3D_default"));
        actionNeuron_annotator = new QAction(NaMainWindow);
        actionNeuron_annotator->setObjectName(QString::fromUtf8("actionNeuron_annotator"));
        actionV3DDefault = new QAction(NaMainWindow);
        actionV3DDefault->setObjectName(QString::fromUtf8("actionV3DDefault"));
        actionV3DDefault->setCheckable(true);
        actionNeuronAnnotator = new QAction(NaMainWindow);
        actionNeuronAnnotator->setObjectName(QString::fromUtf8("actionNeuronAnnotator"));
        actionNeuronAnnotator->setCheckable(true);
        actionLink_viewers = new QAction(NaMainWindow);
        actionLink_viewers->setObjectName(QString::fromUtf8("actionLink_viewers"));
        actionLink_viewers->setCheckable(true);
        actionLink_viewers->setChecked(true);
        actionLink_viewers->setVisible(false);
        actionShow_Crosshair = new QAction(NaMainWindow);
        actionShow_Crosshair->setObjectName(QString::fromUtf8("actionShow_Crosshair"));
        actionShow_Crosshair->setCheckable(true);
        actionShow_Crosshair->setChecked(true);
        actionUndo = new QAction(NaMainWindow);
        actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
        actionUndo->setEnabled(false);
        actionRedo = new QAction(NaMainWindow);
        actionRedo->setObjectName(QString::fromUtf8("actionRedo"));
        actionRedo->setEnabled(false);
        actionSelect_All = new QAction(NaMainWindow);
        actionSelect_All->setObjectName(QString::fromUtf8("actionSelect_All"));
        actionSelect_All->setEnabled(false);
        actionSelect_None = new QAction(NaMainWindow);
        actionSelect_None->setObjectName(QString::fromUtf8("actionSelect_None"));
        actionSelect_None->setEnabled(true);
        actionInvert_Selection = new QAction(NaMainWindow);
        actionInvert_Selection->setObjectName(QString::fromUtf8("actionInvert_Selection"));
        actionInvert_Selection->setEnabled(false);
        action3D_Volume = new QAction(NaMainWindow);
        action3D_Volume->setObjectName(QString::fromUtf8("action3D_Volume"));
        action2D_MIP = new QAction(NaMainWindow);
        action2D_MIP->setObjectName(QString::fromUtf8("action2D_MIP"));
        actionScreenShot = new QAction(NaMainWindow);
        actionScreenShot->setObjectName(QString::fromUtf8("actionScreenShot"));
        actionFoo = new QAction(NaMainWindow);
        actionFoo->setObjectName(QString::fromUtf8("actionFoo"));
        actionClear_Hide_All = new QAction(NaMainWindow);
        actionClear_Hide_All->setObjectName(QString::fromUtf8("actionClear_Hide_All"));
        actionClear_Hide_All->setEnabled(true);
        actionShow_all_neurons_in_empty_space = new QAction(NaMainWindow);
        actionShow_all_neurons_in_empty_space->setObjectName(QString::fromUtf8("actionShow_all_neurons_in_empty_space"));
        actionShow_all_neurons_in_empty_space->setEnabled(true);
        actionMono_Off = new QAction(NaMainWindow);
        actionMono_Off->setObjectName(QString::fromUtf8("actionMono_Off"));
        actionMono_Off->setCheckable(true);
        actionMono_Off->setChecked(true);
        actionLeft_eye_view = new QAction(NaMainWindow);
        actionLeft_eye_view->setObjectName(QString::fromUtf8("actionLeft_eye_view"));
        actionLeft_eye_view->setCheckable(true);
        actionLeft_eye_view->setEnabled(true);
        actionRight_eye_view = new QAction(NaMainWindow);
        actionRight_eye_view->setObjectName(QString::fromUtf8("actionRight_eye_view"));
        actionRight_eye_view->setCheckable(true);
        actionRight_eye_view->setEnabled(true);
        actionAnaglyph_Red_Cyan = new QAction(NaMainWindow);
        actionAnaglyph_Red_Cyan->setObjectName(QString::fromUtf8("actionAnaglyph_Red_Cyan"));
        actionAnaglyph_Red_Cyan->setCheckable(true);
        actionAnaglyph_Red_Cyan->setEnabled(true);
        actionAnaglyph_Green_Magenta = new QAction(NaMainWindow);
        actionAnaglyph_Green_Magenta->setObjectName(QString::fromUtf8("actionAnaglyph_Green_Magenta"));
        actionAnaglyph_Green_Magenta->setCheckable(true);
        actionAnaglyph_Green_Magenta->setEnabled(true);
        actionRow_Interleaved_Zalman = new QAction(NaMainWindow);
        actionRow_Interleaved_Zalman->setObjectName(QString::fromUtf8("actionRow_Interleaved_Zalman"));
        actionRow_Interleaved_Zalman->setCheckable(true);
        actionRow_Interleaved_Zalman->setEnabled(true);
        actionQuadro_120_Hz = new QAction(NaMainWindow);
        actionQuadro_120_Hz->setObjectName(QString::fromUtf8("actionQuadro_120_Hz"));
        actionQuadro_120_Hz->setCheckable(true);
        actionQuadro_120_Hz->setEnabled(true);
        actionSwap_Eyes = new QAction(NaMainWindow);
        actionSwap_Eyes->setObjectName(QString::fromUtf8("actionSwap_Eyes"));
        actionSwap_Eyes->setCheckable(true);
        actionShow_Axes = new QAction(NaMainWindow);
        actionShow_Axes->setObjectName(QString::fromUtf8("actionShow_Axes"));
        actionShow_Axes->setCheckable(true);
        actionShow_Axes->setChecked(true);
        actionShow_Axes->setEnabled(true);
        action3D_alpha_blending = new QAction(NaMainWindow);
        action3D_alpha_blending->setObjectName(QString::fromUtf8("action3D_alpha_blending"));
        action3D_alpha_blending->setCheckable(true);
        action3D_alpha_blending->setChecked(false);
        actionAnimate_3D_nutation = new QAction(NaMainWindow);
        actionAnimate_3D_nutation->setObjectName(QString::fromUtf8("actionAnimate_3D_nutation"));
        actionAnimate_3D_nutation->setCheckable(true);
        actionDynamic_range = new QAction(NaMainWindow);
        actionDynamic_range->setObjectName(QString::fromUtf8("actionDynamic_range"));
        actionShow_Brain_Compartments = new QAction(NaMainWindow);
        actionShow_Brain_Compartments->setObjectName(QString::fromUtf8("actionShow_Brain_Compartments"));
        actionShow_Brain_Compartments->setCheckable(true);
        actionShow_Brain_Compartments->setEnabled(false);
        actionOpen_Single_Movie_Stack = new QAction(NaMainWindow);
        actionOpen_Single_Movie_Stack->setObjectName(QString::fromUtf8("actionOpen_Single_Movie_Stack"));
        actionOpen_Single_Movie_Stack->setEnabled(true);
        actionOpen_Single_Movie_Stack->setVisible(true);
        actionCurrent_MIP = new QAction(NaMainWindow);
        actionCurrent_MIP->setObjectName(QString::fromUtf8("actionCurrent_MIP"));
        actionChecker_Interleaved_3DTV = new QAction(NaMainWindow);
        actionChecker_Interleaved_3DTV->setObjectName(QString::fromUtf8("actionChecker_Interleaved_3DTV"));
        actionChecker_Interleaved_3DTV->setCheckable(true);
        actionFull_Screen = new QAction(NaMainWindow);
        actionFull_Screen->setObjectName(QString::fromUtf8("actionFull_Screen"));
        actionFull_Screen->setCheckable(true);
        actionColumn_Interleaved = new QAction(NaMainWindow);
        actionColumn_Interleaved->setObjectName(QString::fromUtf8("actionColumn_Interleaved"));
        actionColumn_Interleaved->setCheckable(true);
        actionOpen_Reference_Channel = new QAction(NaMainWindow);
        actionOpen_Reference_Channel->setObjectName(QString::fromUtf8("actionOpen_Reference_Channel"));
        actionOpen_Reference_Channel->setEnabled(false);
        actionOpen_Neuron_Mask = new QAction(NaMainWindow);
        actionOpen_Neuron_Mask->setObjectName(QString::fromUtf8("actionOpen_Neuron_Mask"));
        actionOpen_Neuron_Mask->setEnabled(false);
        actionLoad_movie_as_texture = new QAction(NaMainWindow);
        actionLoad_movie_as_texture->setObjectName(QString::fromUtf8("actionLoad_movie_as_texture"));
        actionLoad_movie_as_texture->setEnabled(false);
        actionLoad_fast_separation_result = new QAction(NaMainWindow);
        actionLoad_fast_separation_result->setObjectName(QString::fromUtf8("actionLoad_fast_separation_result"));
        actionX_Rotation_Movie = new QAction(NaMainWindow);
        actionX_Rotation_Movie->setObjectName(QString::fromUtf8("actionX_Rotation_Movie"));
        actionMeasure_Frame_Rate = new QAction(NaMainWindow);
        actionMeasure_Frame_Rate->setObjectName(QString::fromUtf8("actionMeasure_Frame_Rate"));
        actionOpen_Octree_Volume = new QAction(NaMainWindow);
        actionOpen_Octree_Volume->setObjectName(QString::fromUtf8("actionOpen_Octree_Volume"));
        actionOpen_microCT_Cut_Planner = new QAction(NaMainWindow);
        actionOpen_microCT_Cut_Planner->setObjectName(QString::fromUtf8("actionOpen_microCT_Cut_Planner"));
        actionAppend_key_frame_at_current_view = new QAction(NaMainWindow);
        actionAppend_key_frame_at_current_view->setObjectName(QString::fromUtf8("actionAppend_key_frame_at_current_view"));
        actionPlay_movie = new QAction(NaMainWindow);
        actionPlay_movie->setObjectName(QString::fromUtf8("actionPlay_movie"));
        actionClear_movie = new QAction(NaMainWindow);
        actionClear_movie->setObjectName(QString::fromUtf8("actionClear_movie"));
        actionSave_movie_frames = new QAction(NaMainWindow);
        actionSave_movie_frames->setObjectName(QString::fromUtf8("actionSave_movie_frames"));
        action1280x720 = new QAction(NaMainWindow);
        action1280x720->setObjectName(QString::fromUtf8("action1280x720"));
        actionAdd_landmark_at_cursor = new QAction(NaMainWindow);
        actionAdd_landmark_at_cursor->setObjectName(QString::fromUtf8("actionAdd_landmark_at_cursor"));
        actionClear_landmarks = new QAction(NaMainWindow);
        actionClear_landmarks->setObjectName(QString::fromUtf8("actionClear_landmarks"));
        actionShow_Scale_Bar = new QAction(NaMainWindow);
        actionShow_Scale_Bar->setObjectName(QString::fromUtf8("actionShow_Scale_Bar"));
        actionShow_Scale_Bar->setCheckable(true);
        actionShow_Scale_Bar->setChecked(true);
        centralwidget = new QWidget(NaMainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        viewerAndMIPsFrame = new QFrame(centralwidget);
        viewerAndMIPsFrame->setObjectName(QString::fromUtf8("viewerAndMIPsFrame"));
        viewerAndMIPsFrame->setFrameShape(QFrame::NoFrame);
        viewerAndMIPsFrame->setFrameShadow(QFrame::Plain);
        viewerAndMIPsFrame->setLineWidth(0);
        verticalLayout = new QVBoxLayout(viewerAndMIPsFrame);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        viewerAndControlsFrame = new QFrame(viewerAndMIPsFrame);
        viewerAndControlsFrame->setObjectName(QString::fromUtf8("viewerAndControlsFrame"));
        viewerAndControlsFrame->setFrameShape(QFrame::StyledPanel);
        viewerAndControlsFrame->setFrameShadow(QFrame::Raised);
        viewerAndControlsFrame->setLineWidth(1);
        horizontalLayout_2 = new QHBoxLayout(viewerAndControlsFrame);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        viewerStackedWidget = new QStackedWidget(viewerAndControlsFrame);
        viewerStackedWidget->setObjectName(QString::fromUtf8("viewerStackedWidget"));
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        QBrush brush1(QColor(0, 0, 0, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Window, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush1);
        viewerStackedWidget->setPalette(palette);
        viewerStackedWidget->setFrameShape(QFrame::NoFrame);
        viewerStackedWidget->setFrameShadow(QFrame::Plain);
        viewerStackedWidget->setLineWidth(0);
        viewerMIP_Page = new QWidget();
        viewerMIP_Page->setObjectName(QString::fromUtf8("viewerMIP_Page"));
        horizontalLayout_11 = new QHBoxLayout(viewerMIP_Page);
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        horizontalLayout_11->setContentsMargins(0, 0, 0, 0);
        naLargeMIPWidget = new NaLargeMIPWidget(viewerMIP_Page);
        naLargeMIPWidget->setObjectName(QString::fromUtf8("naLargeMIPWidget"));
        naLargeMIPWidget->setAutoFillBackground(true);
        horizontalLayout_7 = new QHBoxLayout(naLargeMIPWidget);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        horizontalSpacer_5 = new QSpacerItem(223, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_5);

        progressWidgetMip = new QWidget(naLargeMIPWidget);
        progressWidgetMip->setObjectName(QString::fromUtf8("progressWidgetMip"));
        progressWidgetMip->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(progressWidgetMip->sizePolicy().hasHeightForWidth());
        progressWidgetMip->setSizePolicy(sizePolicy1);
        progressWidgetMip->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);"));
        verticalLayout_15 = new QVBoxLayout(progressWidgetMip);
        verticalLayout_15->setObjectName(QString::fromUtf8("verticalLayout_15"));
        progressLabelMip = new QLabel(progressWidgetMip);
        progressLabelMip->setObjectName(QString::fromUtf8("progressLabelMip"));

        verticalLayout_15->addWidget(progressLabelMip);

        progressBarMip = new QProgressBar(progressWidgetMip);
        progressBarMip->setObjectName(QString::fromUtf8("progressBarMip"));
        progressBarMip->setValue(24);

        verticalLayout_15->addWidget(progressBarMip);


        horizontalLayout_7->addWidget(progressWidgetMip);

        horizontalSpacer_6 = new QSpacerItem(223, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_6);


        horizontalLayout_11->addWidget(naLargeMIPWidget);

        viewerStackedWidget->addWidget(viewerMIP_Page);
        viewerZ_Page = new QWidget();
        viewerZ_Page->setObjectName(QString::fromUtf8("viewerZ_Page"));
        horizontalLayout_10 = new QHBoxLayout(viewerZ_Page);
        horizontalLayout_10->setSpacing(0);
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        horizontalLayout_10->setContentsMargins(0, 0, 0, 0);
        frame_5 = new QFrame(viewerZ_Page);
        frame_5->setObjectName(QString::fromUtf8("frame_5"));
        frame_5->setFrameShape(QFrame::NoFrame);
        frame_5->setFrameShadow(QFrame::Raised);
        verticalLayout_12 = new QVBoxLayout(frame_5);
        verticalLayout_12->setSpacing(0);
        verticalLayout_12->setObjectName(QString::fromUtf8("verticalLayout_12"));
        verticalLayout_12->setContentsMargins(0, 0, 0, 0);
        naZStackWidget = new NaZStackWidget(frame_5);
        naZStackWidget->setObjectName(QString::fromUtf8("naZStackWidget"));
        naZStackWidget->setAutoFillBackground(true);
        horizontalLayout_8 = new QHBoxLayout(naZStackWidget);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        horizontalSpacer_7 = new QSpacerItem(223, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_7);

        progressWidgetZ = new QWidget(naZStackWidget);
        progressWidgetZ->setObjectName(QString::fromUtf8("progressWidgetZ"));
        sizePolicy1.setHeightForWidth(progressWidgetZ->sizePolicy().hasHeightForWidth());
        progressWidgetZ->setSizePolicy(sizePolicy1);
        progressWidgetZ->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);"));
        verticalLayout_16 = new QVBoxLayout(progressWidgetZ);
        verticalLayout_16->setObjectName(QString::fromUtf8("verticalLayout_16"));
        progressLabelZ = new QLabel(progressWidgetZ);
        progressLabelZ->setObjectName(QString::fromUtf8("progressLabelZ"));

        verticalLayout_16->addWidget(progressLabelZ);

        progressBarZ = new QProgressBar(progressWidgetZ);
        progressBarZ->setObjectName(QString::fromUtf8("progressBarZ"));
        progressBarZ->setValue(24);

        verticalLayout_16->addWidget(progressBarZ);


        horizontalLayout_8->addWidget(progressWidgetZ);

        horizontalSpacer_8 = new QSpacerItem(223, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_8);


        verticalLayout_12->addWidget(naZStackWidget);

        ZSlice_horizontalScrollBar = new QScrollBar(frame_5);
        ZSlice_horizontalScrollBar->setObjectName(QString::fromUtf8("ZSlice_horizontalScrollBar"));
        ZSlice_horizontalScrollBar->setMinimum(1);
        ZSlice_horizontalScrollBar->setMaximum(1);
        ZSlice_horizontalScrollBar->setOrientation(Qt::Horizontal);

        verticalLayout_12->addWidget(ZSlice_horizontalScrollBar);


        horizontalLayout_10->addWidget(frame_5);

        viewerStackedWidget->addWidget(viewerZ_Page);
        viewer3D_Page = new QWidget();
        viewer3D_Page->setObjectName(QString::fromUtf8("viewer3D_Page"));
        horizontalLayout_3 = new QHBoxLayout(viewer3D_Page);
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        frame_3dwidget = new QFrame(viewer3D_Page);
        frame_3dwidget->setObjectName(QString::fromUtf8("frame_3dwidget"));
        frame_3dwidget->setFrameShape(QFrame::NoFrame);
        frame_3dwidget->setFrameShadow(QFrame::Plain);
        frame_3dwidget->setLineWidth(0);
        verticalLayout_5 = new QVBoxLayout(frame_3dwidget);
        verticalLayout_5->setSpacing(0);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(0, 0, 0, 0);
        widget_progress3d = new QWidget(frame_3dwidget);
        widget_progress3d->setObjectName(QString::fromUtf8("widget_progress3d"));
        widget_progress3d->setEnabled(true);
        sizePolicy1.setHeightForWidth(widget_progress3d->sizePolicy().hasHeightForWidth());
        widget_progress3d->setSizePolicy(sizePolicy1);
        widget_progress3d->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);\n"
"alternate-background-color: rgb(0, 0, 0);\n"
"background-color: rgb(0, 0, 0);"));
        horizontalLayout_5 = new QHBoxLayout(widget_progress3d);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(-1, 0, -1, 0);
        progressLabel3d = new QLabel(widget_progress3d);
        progressLabel3d->setObjectName(QString::fromUtf8("progressLabel3d"));

        horizontalLayout_5->addWidget(progressLabel3d);

        progressBar3d = new QProgressBar(widget_progress3d);
        progressBar3d->setObjectName(QString::fromUtf8("progressBar3d"));
        progressBar3d->setValue(24);

        horizontalLayout_5->addWidget(progressBar3d);


        verticalLayout_5->addWidget(widget_progress3d);

//        v3dr_glwidget = new Na3DWidget(frame_3dwidget);
//        v3dr_glwidget->setObjectName(QString::fromUtf8("v3dr_glwidget"));
//        v3dr_glwidget->setMinimumSize(QSize(320, 240));

//        verticalLayout_5->addWidget(v3dr_glwidget);


        horizontalLayout_3->addWidget(frame_3dwidget);

        viewerStackedWidget->addWidget(viewer3D_Page);
        viewer_busy_loading_Page = new QWidget();
        viewer_busy_loading_Page->setObjectName(QString::fromUtf8("viewer_busy_loading_Page"));
        verticalLayout_17 = new QVBoxLayout(viewer_busy_loading_Page);
        verticalLayout_17->setObjectName(QString::fromUtf8("verticalLayout_17"));
        verticalSpacer_7 = new QSpacerItem(20, 290, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_17->addItem(verticalSpacer_7);

        label_2 = new QLabel(viewer_busy_loading_Page);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout_17->addWidget(label_2);

        progressBar = new QProgressBar(viewer_busy_loading_Page);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setMaximum(0);
        progressBar->setValue(-1);

        verticalLayout_17->addWidget(progressBar);

        verticalSpacer_8 = new QSpacerItem(20, 290, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_17->addItem(verticalSpacer_8);

        viewerStackedWidget->addWidget(viewer_busy_loading_Page);

        horizontalLayout_2->addWidget(viewerStackedWidget);

        viewerSelectorAndControlFrame = new QFrame(viewerAndControlsFrame);
        viewerSelectorAndControlFrame->setObjectName(QString::fromUtf8("viewerSelectorAndControlFrame"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(viewerSelectorAndControlFrame->sizePolicy().hasHeightForWidth());
        viewerSelectorAndControlFrame->setSizePolicy(sizePolicy2);
        viewerSelectorAndControlFrame->setMinimumSize(QSize(200, 0));
        viewerSelectorAndControlFrame->setMaximumSize(QSize(200, 16777215));
        viewerSelectorAndControlFrame->setFrameShape(QFrame::StyledPanel);
        verticalLayout_7 = new QVBoxLayout(viewerSelectorAndControlFrame);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        verticalLayout_7->setContentsMargins(0, 0, 0, 0);
        viewerControlTabWidget = new QTabWidget(viewerSelectorAndControlFrame);
        viewerControlTabWidget->setObjectName(QString::fromUtf8("viewerControlTabWidget"));
        largeMipViewerControlPage = new QWidget();
        largeMipViewerControlPage->setObjectName(QString::fromUtf8("largeMipViewerControlPage"));
        verticalLayout_10 = new QVBoxLayout(largeMipViewerControlPage);
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        verticalLayout_10->setContentsMargins(0, 0, 0, 0);
        verticalSpacer_4 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_10->addItem(verticalSpacer_4);

        viewerControlTabWidget->addTab(largeMipViewerControlPage, QString());
        zSliceViewerControlPage = new QWidget();
        zSliceViewerControlPage->setObjectName(QString::fromUtf8("zSliceViewerControlPage"));
        horizontalLayout_12 = new QHBoxLayout(zSliceViewerControlPage);
        horizontalLayout_12->setSpacing(0);
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        horizontalLayout_12->setContentsMargins(0, 0, 0, 0);
        frame = new QFrame(zSliceViewerControlPage);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::NoFrame);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout_4 = new QVBoxLayout(frame);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        HDR_groupBox = new QGroupBox(frame);
        HDR_groupBox->setObjectName(QString::fromUtf8("HDR_groupBox"));
        verticalLayout_11 = new QVBoxLayout(HDR_groupBox);
        verticalLayout_11->setSpacing(5);
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        verticalLayout_11->setContentsMargins(0, 0, 0, 0);
        HDR_checkBox = new QCheckBox(HDR_groupBox);
        HDR_checkBox->setObjectName(QString::fromUtf8("HDR_checkBox"));
        QFont font;
        font.setBold(true);
        //font.setWeight(75);
        HDR_checkBox->setFont(font);

        verticalLayout_11->addWidget(HDR_checkBox);

        frame_3 = new QFrame(HDR_groupBox);
        frame_3->setObjectName(QString::fromUtf8("frame_3"));
        frame_3->setFrameShape(QFrame::NoFrame);
        frame_3->setFrameShadow(QFrame::Raised);
        horizontalLayout_13 = new QHBoxLayout(frame_3);
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        horizontalLayout_13->setContentsMargins(0, 0, 0, 0);
        HDRRed_pushButton = new QPushButton(frame_3);
        HDRRed_pushButton->setObjectName(QString::fromUtf8("HDRRed_pushButton"));
        HDRRed_pushButton->setEnabled(false);
        HDRRed_pushButton->setStyleSheet(QString::fromUtf8("color: rgb(255, 0, 0);"));
        HDRRed_pushButton->setCheckable(true);
        HDRRed_pushButton->setChecked(true);
        HDRRed_pushButton->setAutoExclusive(true);
        HDRRed_pushButton->setAutoDefault(false);
        HDRRed_pushButton->setDefault(false);

        horizontalLayout_13->addWidget(HDRRed_pushButton);

        HDRGreen_pushButton = new QPushButton(frame_3);
        HDRGreen_pushButton->setObjectName(QString::fromUtf8("HDRGreen_pushButton"));
        HDRGreen_pushButton->setEnabled(false);
        HDRGreen_pushButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 128, 0);"));
        HDRGreen_pushButton->setCheckable(true);
        HDRGreen_pushButton->setAutoExclusive(true);

        horizontalLayout_13->addWidget(HDRGreen_pushButton);

        HDRBlue_pushButton = new QPushButton(frame_3);
        HDRBlue_pushButton->setObjectName(QString::fromUtf8("HDRBlue_pushButton"));
        HDRBlue_pushButton->setEnabled(false);
        HDRBlue_pushButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 255);"));
        HDRBlue_pushButton->setCheckable(true);
        HDRBlue_pushButton->setChecked(false);
        HDRBlue_pushButton->setAutoExclusive(true);

        horizontalLayout_13->addWidget(HDRBlue_pushButton);

        HDRNc82_pushButton = new QPushButton(frame_3);
        HDRNc82_pushButton->setObjectName(QString::fromUtf8("HDRNc82_pushButton"));
        HDRNc82_pushButton->setEnabled(false);
        HDRNc82_pushButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        HDRNc82_pushButton->setCheckable(true);
        HDRNc82_pushButton->setAutoExclusive(true);

        horizontalLayout_13->addWidget(HDRNc82_pushButton);


        verticalLayout_11->addWidget(frame_3);

        frame_4 = new QFrame(HDR_groupBox);
        frame_4->setObjectName(QString::fromUtf8("frame_4"));
        frame_4->setFrameShape(QFrame::NoFrame);
        frame_4->setFrameShadow(QFrame::Raised);
        horizontalLayout_14 = new QHBoxLayout(frame_4);
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        horizontalLayout_14->setContentsMargins(0, 0, 0, 0);
        BoxSize_label = new QLabel(frame_4);
        BoxSize_label->setObjectName(QString::fromUtf8("BoxSize_label"));

        horizontalLayout_14->addWidget(BoxSize_label);

        BoxSize_spinBox = new QSpinBox(frame_4);
        BoxSize_spinBox->setObjectName(QString::fromUtf8("BoxSize_spinBox"));
        BoxSize_spinBox->setMaximum(1000);
        BoxSize_spinBox->setValue(25);

        horizontalLayout_14->addWidget(BoxSize_spinBox);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_14->addItem(horizontalSpacer_2);


        verticalLayout_11->addWidget(frame_4);


        verticalLayout_4->addWidget(HDR_groupBox);

        ZSlice_groupBox = new QGroupBox(frame);
        ZSlice_groupBox->setObjectName(QString::fromUtf8("ZSlice_groupBox"));
        horizontalLayout_15 = new QHBoxLayout(ZSlice_groupBox);
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        horizontalLayout_15->setContentsMargins(0, 0, 0, 0);
        ZSlice_spinBox = new QSpinBox(ZSlice_groupBox);
        ZSlice_spinBox->setObjectName(QString::fromUtf8("ZSlice_spinBox"));
        ZSlice_spinBox->setMinimumSize(QSize(0, 0));
        ZSlice_spinBox->setMinimum(1);
        ZSlice_spinBox->setMaximum(1);

        horizontalLayout_15->addWidget(ZSlice_spinBox);

        ZSliceTotal_label = new QLabel(ZSlice_groupBox);
        ZSliceTotal_label->setObjectName(QString::fromUtf8("ZSliceTotal_label"));
        ZSliceTotal_label->setMinimumSize(QSize(60, 0));
        ZSliceTotal_label->setText(QString::fromUtf8("of 0"));
        ZSliceTotal_label->setTextFormat(Qt::PlainText);
        ZSliceTotal_label->setTextInteractionFlags(Qt::TextSelectableByMouse);

        horizontalLayout_15->addWidget(ZSliceTotal_label);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_15->addItem(horizontalSpacer);


        verticalLayout_4->addWidget(ZSlice_groupBox);

        verticalSpacer_2 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer_2);


        horizontalLayout_12->addWidget(frame);

        viewerControlTabWidget->addTab(zSliceViewerControlPage, QString());
        viewer3DControlPage = new QWidget();
        viewer3DControlPage->setObjectName(QString::fromUtf8("viewer3DControlPage"));
        verticalLayout_3 = new QVBoxLayout(viewer3DControlPage);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        viewerControl3DFrame = new QFrame(viewer3DControlPage);
        viewerControl3DFrame->setObjectName(QString::fromUtf8("viewerControl3DFrame"));
        viewerControl3DFrame->setFrameShape(QFrame::NoFrame);
        viewerControl3DFrame->setFrameShadow(QFrame::Plain);
        viewerControl3DFrame->setLineWidth(0);
        verticalLayout_18 = new QVBoxLayout(viewerControl3DFrame);
        verticalLayout_18->setObjectName(QString::fromUtf8("verticalLayout_18"));
        verticalLayout_18->setContentsMargins(0, 0, 0, 0);
        verticalLayout_14 = new QVBoxLayout();
        verticalLayout_14->setObjectName(QString::fromUtf8("verticalLayout_14"));
        controlTabWidget = new QTabWidget(viewerControl3DFrame);
        controlTabWidget->setObjectName(QString::fromUtf8("controlTabWidget"));
        controlTabWidget->setMinimumSize(QSize(128, 215));
        tabRotation = new QWidget();
        tabRotation->setObjectName(QString::fromUtf8("tabRotation"));
        rotXWidget = new AngleWidget(tabRotation);
        rotXWidget->setObjectName(QString::fromUtf8("rotXWidget"));
        rotXWidget->setGeometry(QRect(13, 3, 153, 41));
        sizePolicy.setHeightForWidth(rotXWidget->sizePolicy().hasHeightForWidth());
        rotXWidget->setSizePolicy(sizePolicy);
        rotXWidget->setMinimumSize(QSize(0, 35));
        rotXWidget->setBaseSize(QSize(0, 35));
        rotZWidget = new AngleWidget(tabRotation);
        rotZWidget->setObjectName(QString::fromUtf8("rotZWidget"));
        rotZWidget->setGeometry(QRect(13, 85, 153, 41));
        sizePolicy.setHeightForWidth(rotZWidget->sizePolicy().hasHeightForWidth());
        rotZWidget->setSizePolicy(sizePolicy);
        rotZWidget->setMinimumSize(QSize(0, 35));
        rotZWidget->setBaseSize(QSize(0, 35));
        rotYWidget = new AngleWidget(tabRotation);
        rotYWidget->setObjectName(QString::fromUtf8("rotYWidget"));
        rotYWidget->setGeometry(QRect(13, 44, 153, 41));
        sizePolicy.setHeightForWidth(rotYWidget->sizePolicy().hasHeightForWidth());
        rotYWidget->setSizePolicy(sizePolicy);
        rotYWidget->setMinimumSize(QSize(0, 35));
        rotYWidget->setBaseSize(QSize(0, 35));
        rotationResetButton = new QPushButton(tabRotation);
        rotationResetButton->setObjectName(QString::fromUtf8("rotationResetButton"));
        rotationResetButton->setGeometry(QRect(7, 130, 134, 32));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(rotationResetButton->sizePolicy().hasHeightForWidth());
        rotationResetButton->setSizePolicy(sizePolicy3);
        rotationResetButton->setAutoFillBackground(false);
        nutateButton = new QCheckBox(tabRotation);
        nutateButton->setObjectName(QString::fromUtf8("nutateButton"));
        nutateButton->setGeometry(QRect(16, 162, 151, 20));
        nutateButton->setCheckable(true);
        controlTabWidget->addTab(tabRotation, QString());
        tabVolumeCut = new QWidget();
        tabVolumeCut->setObjectName(QString::fromUtf8("tabVolumeCut"));
        verticalLayout_19 = new QVBoxLayout(tabVolumeCut);
        verticalLayout_19->setSpacing(0);
        verticalLayout_19->setObjectName(QString::fromUtf8("verticalLayout_19"));
        verticalLayout_19->setContentsMargins(0, 5, 0, 5);
        XCutCB = new QCheckBox(tabVolumeCut);
        XCutCB->setObjectName(QString::fromUtf8("XCutCB"));

        verticalLayout_19->addWidget(XCutCB);

        XcminSlider = new QSlider(tabVolumeCut);
        XcminSlider->setObjectName(QString::fromUtf8("XcminSlider"));
        XcminSlider->setFocusPolicy(Qt::WheelFocus);
        XcminSlider->setMaximum(1);
        XcminSlider->setOrientation(Qt::Horizontal);
        XcminSlider->setTickInterval(10);

        verticalLayout_19->addWidget(XcminSlider);

        XcmaxSlider = new QSlider(tabVolumeCut);
        XcmaxSlider->setObjectName(QString::fromUtf8("XcmaxSlider"));
        XcmaxSlider->setFocusPolicy(Qt::WheelFocus);
        XcmaxSlider->setMaximum(1);
        XcmaxSlider->setOrientation(Qt::Horizontal);
        XcmaxSlider->setTickInterval(10);

        verticalLayout_19->addWidget(XcmaxSlider);

        YCutCB = new QCheckBox(tabVolumeCut);
        YCutCB->setObjectName(QString::fromUtf8("YCutCB"));

        verticalLayout_19->addWidget(YCutCB);

        YcminSlider = new QSlider(tabVolumeCut);
        YcminSlider->setObjectName(QString::fromUtf8("YcminSlider"));
        YcminSlider->setFocusPolicy(Qt::WheelFocus);
        YcminSlider->setMaximum(1);
        YcminSlider->setOrientation(Qt::Horizontal);
        YcminSlider->setTickInterval(10);

        verticalLayout_19->addWidget(YcminSlider);

        YcmaxSlider = new QSlider(tabVolumeCut);
        YcmaxSlider->setObjectName(QString::fromUtf8("YcmaxSlider"));
        YcmaxSlider->setFocusPolicy(Qt::WheelFocus);
        YcmaxSlider->setMaximum(1);
        YcmaxSlider->setOrientation(Qt::Horizontal);
        YcmaxSlider->setTickInterval(10);

        verticalLayout_19->addWidget(YcmaxSlider);

        ZCutCB = new QCheckBox(tabVolumeCut);
        ZCutCB->setObjectName(QString::fromUtf8("ZCutCB"));

        verticalLayout_19->addWidget(ZCutCB);

        ZcminSlider = new QSlider(tabVolumeCut);
        ZcminSlider->setObjectName(QString::fromUtf8("ZcminSlider"));
        ZcminSlider->setFocusPolicy(Qt::WheelFocus);
        ZcminSlider->setMaximum(1);
        ZcminSlider->setOrientation(Qt::Horizontal);
        ZcminSlider->setTickInterval(10);

        verticalLayout_19->addWidget(ZcminSlider);

        ZcmaxSlider = new QSlider(tabVolumeCut);
        ZcmaxSlider->setObjectName(QString::fromUtf8("ZcmaxSlider"));
        ZcmaxSlider->setFocusPolicy(Qt::WheelFocus);
        ZcmaxSlider->setMaximum(1);
        ZcmaxSlider->setOrientation(Qt::Horizontal);
        ZcmaxSlider->setTickInterval(10);

        verticalLayout_19->addWidget(ZcmaxSlider);

        frame_6 = new QFrame(tabVolumeCut);
        frame_6->setObjectName(QString::fromUtf8("frame_6"));
        frame_6->setFrameShape(QFrame::StyledPanel);
        frame_6->setFrameShadow(QFrame::Raised);
        horizontalLayout_16 = new QHBoxLayout(frame_6);
        horizontalLayout_16->setSpacing(0);
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        horizontalLayout_16->setContentsMargins(0, 0, 0, 0);
        label_3 = new QLabel(frame_6);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_16->addWidget(label_3);

        freezeFrontBackButton = new QPushButton(frame_6);
        freezeFrontBackButton->setObjectName(QString::fromUtf8("freezeFrontBackButton"));
        QSizePolicy sizePolicy4(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(freezeFrontBackButton->sizePolicy().hasHeightForWidth());
        freezeFrontBackButton->setSizePolicy(sizePolicy4);

        horizontalLayout_16->addWidget(freezeFrontBackButton);


        verticalLayout_19->addWidget(frame_6);

        slabThicknessSlider = new QSlider(tabVolumeCut);
        slabThicknessSlider->setObjectName(QString::fromUtf8("slabThicknessSlider"));
        slabThicknessSlider->setFocusPolicy(Qt::NoFocus);
        slabThicknessSlider->setMinimum(2);
        slabThicknessSlider->setMaximum(1000);
        slabThicknessSlider->setValue(1000);
        slabThicknessSlider->setOrientation(Qt::Horizontal);

        verticalLayout_19->addWidget(slabThicknessSlider);

        slabPositionSlider = new QSlider(tabVolumeCut);
        slabPositionSlider->setObjectName(QString::fromUtf8("slabPositionSlider"));
        slabPositionSlider->setFocusPolicy(Qt::WheelFocus);
        slabPositionSlider->setMinimum(-500);
        slabPositionSlider->setMaximum(500);
        slabPositionSlider->setValue(0);
        slabPositionSlider->setSliderPosition(0);
        slabPositionSlider->setOrientation(Qt::Horizontal);

        verticalLayout_19->addWidget(slabPositionSlider);

        frame_2 = new QFrame(tabVolumeCut);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        horizontalLayout_9 = new QHBoxLayout(frame_2);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        horizontalLayout_9->setContentsMargins(-1, 0, -1, 0);
        defineClipPlaneButton = new QPushButton(frame_2);
        defineClipPlaneButton->setObjectName(QString::fromUtf8("defineClipPlaneButton"));

        horizontalLayout_9->addWidget(defineClipPlaneButton);

        customCutButton = new QPushButton(frame_2);
        customCutButton->setObjectName(QString::fromUtf8("customCutButton"));
        customCutButton->setEnabled(false);

        horizontalLayout_9->addWidget(customCutButton);


        verticalLayout_19->addWidget(frame_2);

        controlTabWidget->addTab(tabVolumeCut, QString());

        verticalLayout_14->addWidget(controlTabWidget);

        widget_2 = new QWidget(viewerControl3DFrame);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        horizontalLayout_17 = new QHBoxLayout(widget_2);
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));
        label_4 = new QLabel(widget_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_17->addWidget(label_4);

        zThicknessDoubleSpinBox = new QDoubleSpinBox(widget_2);
        zThicknessDoubleSpinBox->setObjectName(QString::fromUtf8("zThicknessDoubleSpinBox"));
        zThicknessDoubleSpinBox->setMinimum(0.01);
        zThicknessDoubleSpinBox->setMaximum(100);
        zThicknessDoubleSpinBox->setSingleStep(0.1);
        zThicknessDoubleSpinBox->setValue(1);

        horizontalLayout_17->addWidget(zThicknessDoubleSpinBox);


        verticalLayout_14->addWidget(widget_2);

        verticalSpacer_5 = new QSpacerItem(20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_14->addItem(verticalSpacer_5);

        compartmentSelectGroupBox = new QGroupBox(viewerControl3DFrame);
        compartmentSelectGroupBox->setObjectName(QString::fromUtf8("compartmentSelectGroupBox"));
        QSizePolicy sizePolicy5(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(compartmentSelectGroupBox->sizePolicy().hasHeightForWidth());
        compartmentSelectGroupBox->setSizePolicy(sizePolicy5);
        compartmentSelectGroupBox->setMinimumSize(QSize(128, 5));
        compartmentSelectGroupBox->setMaximumSize(QSize(16777215, 5));
        QFont font1;
        font1.setBold(false);
       // font1.setWeight(50);
        compartmentSelectGroupBox->setFont(font1);
        verticalLayout_8 = new QVBoxLayout(compartmentSelectGroupBox);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        verticalLayout_8->setContentsMargins(0, 0, 0, 0);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        compartmentMapComboBox = new CompartmentMapComboBox(compartmentSelectGroupBox);
        compartmentMapComboBox->setObjectName(QString::fromUtf8("compartmentMapComboBox"));

//        gridLayout->addWidget(compartmentMapComboBox, 1, 0, 1, 1);

//        compartmentMapWidget = new CompartmentMapWidget(compartmentSelectGroupBox);
//        compartmentMapWidget->setObjectName(QString::fromUtf8("compartmentMapWidget"));
//        compartmentMapWidget->setMinimumSize(QSize(128, 128));

//        gridLayout->addWidget(compartmentMapWidget, 0, 0, 1, 1);


        verticalLayout_8->addLayout(gridLayout);


        verticalLayout_14->addWidget(compartmentSelectGroupBox);

        verticalSpacer = new QSpacerItem(158, 13, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_14->addItem(verticalSpacer);


        verticalLayout_18->addLayout(verticalLayout_14);


        verticalLayout_3->addWidget(viewerControl3DFrame);

        viewerControlTabWidget->addTab(viewer3DControlPage, QString());

        verticalLayout_7->addWidget(viewerControlTabWidget);

        groupBox_3 = new QGroupBox(viewerSelectorAndControlFrame);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setEnabled(true);
        verticalLayout_9 = new QVBoxLayout(groupBox_3);
        verticalLayout_9->setSpacing(3);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        verticalLayout_9->setContentsMargins(-1, 0, -1, 0);
        zoomWidget = new ZoomWidget(groupBox_3);
        zoomWidget->setObjectName(QString::fromUtf8("zoomWidget"));

        verticalLayout_9->addWidget(zoomWidget);

        resetViewButton = new QPushButton(groupBox_3);
        resetViewButton->setObjectName(QString::fromUtf8("resetViewButton"));
        sizePolicy3.setHeightForWidth(resetViewButton->sizePolicy().hasHeightForWidth());
        resetViewButton->setSizePolicy(sizePolicy3);

        verticalLayout_9->addWidget(resetViewButton);


        verticalLayout_7->addWidget(groupBox_3);

        groupBox_2 = new QGroupBox(viewerSelectorAndControlFrame);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_6 = new QVBoxLayout(groupBox_2);
        verticalLayout_6->setSpacing(2);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(0, 0, 0, 0);
        sharedGammaWidget = new GammaWidget(groupBox_2);
        sharedGammaWidget->setObjectName(QString::fromUtf8("sharedGammaWidget"));

        verticalLayout_6->addWidget(sharedGammaWidget);

        referenceGammaWidget = new GammaWidget(groupBox_2);
        referenceGammaWidget->setObjectName(QString::fromUtf8("referenceGammaWidget"));

        verticalLayout_6->addWidget(referenceGammaWidget);

        sharedColorFrame = new QFrame(groupBox_2);
        sharedColorFrame->setObjectName(QString::fromUtf8("sharedColorFrame"));
        sharedColorFrame->setFrameShape(QFrame::NoFrame);
        sharedColorFrame->setFrameShadow(QFrame::Raised);
        horizontalLayout_6 = new QHBoxLayout(sharedColorFrame);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalLayout_6->setContentsMargins(0, 0, 0, 12);
        redToggleButton = new QPushButton(sharedColorFrame);
        redToggleButton->setObjectName(QString::fromUtf8("redToggleButton"));
        redToggleButton->setCheckable(true);
        redToggleButton->setChecked(true);

        horizontalLayout_6->addWidget(redToggleButton);

        greenToggleButton = new QPushButton(sharedColorFrame);
        greenToggleButton->setObjectName(QString::fromUtf8("greenToggleButton"));
        greenToggleButton->setCheckable(true);
        greenToggleButton->setChecked(true);

        horizontalLayout_6->addWidget(greenToggleButton);

        blueToggleButton = new QPushButton(sharedColorFrame);
        blueToggleButton->setObjectName(QString::fromUtf8("blueToggleButton"));
        blueToggleButton->setCheckable(true);
        blueToggleButton->setChecked(true);

        horizontalLayout_6->addWidget(blueToggleButton);


        verticalLayout_6->addWidget(sharedColorFrame);

        resetColorsButton = new QPushButton(groupBox_2);
        resetColorsButton->setObjectName(QString::fromUtf8("resetColorsButton"));
        sizePolicy3.setHeightForWidth(resetColorsButton->sizePolicy().hasHeightForWidth());
        resetColorsButton->setSizePolicy(sizePolicy3);

        verticalLayout_6->addWidget(resetColorsButton);


        verticalLayout_7->addWidget(groupBox_2);


        horizontalLayout_2->addWidget(viewerSelectorAndControlFrame);


        verticalLayout->addWidget(viewerAndControlsFrame);

        mipsFrame = new QFrame(viewerAndMIPsFrame);
        mipsFrame->setObjectName(QString::fromUtf8("mipsFrame"));
        QSizePolicy sizePolicy6(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(mipsFrame->sizePolicy().hasHeightForWidth());
        mipsFrame->setSizePolicy(sizePolicy6);
        mipsFrame->setMinimumSize(QSize(0, 200));
        mipsFrame->setFrameShape(QFrame::StyledPanel);
        mipsFrame->setFrameShadow(QFrame::Raised);
        mipsFrame->setLineWidth(1);
        horizontalLayout_4 = new QHBoxLayout(mipsFrame);
        horizontalLayout_4->setSpacing(0);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        maskFrame = new QFrame(mipsFrame);
        maskFrame->setObjectName(QString::fromUtf8("maskFrame"));
        sizePolicy.setHeightForWidth(maskFrame->sizePolicy().hasHeightForWidth());
        maskFrame->setSizePolicy(sizePolicy);
        maskFrame->setMinimumSize(QSize(200, 0));
        maskFrame->setFrameShape(QFrame::StyledPanel);
        maskFrame->setFrameShadow(QFrame::Raised);
        maskFrame->setLineWidth(1);

        horizontalLayout_4->addWidget(maskFrame);

        galleryFrame = new QFrame(mipsFrame);
        galleryFrame->setObjectName(QString::fromUtf8("galleryFrame"));
        galleryFrame->setFrameShape(QFrame::NoFrame);
        galleryFrame->setFrameShadow(QFrame::Plain);
        galleryFrame->setLineWidth(0);
        verticalLayout_2 = new QVBoxLayout(galleryFrame);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        fragmentGalleryWidget = new FragmentGalleryWidget(galleryFrame);
        fragmentGalleryWidget->setObjectName(QString::fromUtf8("fragmentGalleryWidget"));
        fragmentGalleryWidget->setFrameShape(QFrame::NoFrame);

        verticalLayout_2->addWidget(fragmentGalleryWidget);


        horizontalLayout_4->addWidget(galleryFrame);

        widget = new QWidget(mipsFrame);
        widget->setObjectName(QString::fromUtf8("widget"));
        verticalLayout_13 = new QVBoxLayout(widget);
        verticalLayout_13->setObjectName(QString::fromUtf8("verticalLayout_13"));
        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout_13->addWidget(label);

        gallerySortBySizeButton = new QPushButton(widget);
        gallerySortBySizeButton->setObjectName(QString::fromUtf8("gallerySortBySizeButton"));
        gallerySortBySizeButton->setCheckable(true);
        gallerySortBySizeButton->setAutoExclusive(true);

        verticalLayout_13->addWidget(gallerySortBySizeButton);

        gallerySortByIndexButton = new QPushButton(widget);
        gallerySortByIndexButton->setObjectName(QString::fromUtf8("gallerySortByIndexButton"));
        gallerySortByIndexButton->setCheckable(true);
        gallerySortByIndexButton->setChecked(true);
        gallerySortByIndexButton->setAutoExclusive(true);

        verticalLayout_13->addWidget(gallerySortByIndexButton);

        gallerySortByColorButton = new QPushButton(widget);
        gallerySortByColorButton->setObjectName(QString::fromUtf8("gallerySortByColorButton"));
        gallerySortByColorButton->setCheckable(true);
        gallerySortByColorButton->setAutoExclusive(true);

        verticalLayout_13->addWidget(gallerySortByColorButton);

        gallerySortByNameButton = new QPushButton(widget);
        gallerySortByNameButton->setObjectName(QString::fromUtf8("gallerySortByNameButton"));

        verticalLayout_13->addWidget(gallerySortByNameButton);

        verticalSpacer_6 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_13->addItem(verticalSpacer_6);


        horizontalLayout_4->addWidget(widget);


        verticalLayout->addWidget(mipsFrame);


        horizontalLayout->addWidget(viewerAndMIPsFrame);

        annotationFrame = new AnnotationWidget(centralwidget);
        annotationFrame->setObjectName(QString::fromUtf8("annotationFrame"));
        annotationFrame->setEnabled(true);
        QSizePolicy sizePolicy7(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(1);
        sizePolicy7.setHeightForWidth(annotationFrame->sizePolicy().hasHeightForWidth());
        annotationFrame->setSizePolicy(sizePolicy7);
        annotationFrame->setMinimumSize(QSize(350, 0));
        annotationFrame->setAutoFillBackground(false);
        annotationFrame->setFrameShape(QFrame::NoFrame);
        annotationFrame->setFrameShadow(QFrame::Raised);
        annotationFrame->setLineWidth(1);

        horizontalLayout->addWidget(annotationFrame);

        NaMainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(NaMainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1367, 22));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuExport = new QMenu(menuFile);
        menuExport->setObjectName(QString::fromUtf8("menuExport"));
        menuOpen_Recent = new QMenu(menuFile);
        menuOpen_Recent->setObjectName(QString::fromUtf8("menuOpen_Recent"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuView = new QMenu(menubar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuStereo_3D = new QMenu(menuView);
        menuStereo_3D->setObjectName(QString::fromUtf8("menuStereo_3D"));
        menuAnimate = new QMenu(menuView);
        menuAnimate->setObjectName(QString::fromUtf8("menuAnimate"));
        menuSet_canvas_size = new QMenu(menuView);
        menuSet_canvas_size->setObjectName(QString::fromUtf8("menuSet_canvas_size"));
        menuWork_Mode = new QMenu(menubar);
        menuWork_Mode->setObjectName(QString::fromUtf8("menuWork_Mode"));
        NaMainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(NaMainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        NaMainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuView->menuAction());
        menubar->addAction(menuWork_Mode->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(menuOpen_Recent->menuAction());
        menuFile->addAction(actionOpen_Single_Movie_Stack);
        menuFile->addAction(actionOpen_Reference_Channel);
        menuFile->addAction(actionOpen_Neuron_Mask);
        menuFile->addAction(actionOpen_Octree_Volume);
        menuFile->addSeparator();
        menuFile->addAction(menuExport->menuAction());
        menuFile->addSeparator();
        menuFile->addAction(actionQuit);
        menuExport->addAction(action3D_Volume);
        menuExport->addAction(action2D_MIP);
        menuExport->addAction(actionScreenShot);
        menuExport->addAction(actionX_Rotation_Movie);
        menuHelp->addAction(actionAbout_NeuronAnnotator);
        menuEdit->addSeparator();
        menuEdit->addAction(actionSelect_All);
        menuEdit->addAction(actionSelect_None);
        menuEdit->addAction(actionInvert_Selection);
        menuEdit->addSeparator();
        menuEdit->addAction(actionDynamic_range);
        menuEdit->addSeparator();
        menuEdit->addAction(actionPreferences);
        menuView->addAction(actionShow_all_neurons_in_empty_space);
        menuView->addAction(actionClear_Hide_All);
        menuView->addAction(actionFull_Screen);
        menuView->addSeparator();
        menuView->addAction(actionShow_Crosshair);
        menuView->addAction(actionLink_viewers);
        menuView->addAction(action3D_alpha_blending);
        menuView->addAction(actionAnimate_3D_nutation);
        menuView->addAction(actionShow_Axes);
        menuView->addAction(actionShow_Scale_Bar);
        menuView->addAction(actionShow_Brain_Compartments);
        menuView->addSeparator();
        menuView->addAction(menuStereo_3D->menuAction());
        menuView->addSeparator();
        menuView->addAction(actionMeasure_Frame_Rate);
        menuView->addAction(actionOpen_microCT_Cut_Planner);
        menuView->addSeparator();
        menuView->addAction(menuAnimate->menuAction());
        menuView->addAction(menuSet_canvas_size->menuAction());
        menuView->addSeparator();
        menuView->addAction(actionAdd_landmark_at_cursor);
        menuView->addAction(actionClear_landmarks);
        menuStereo_3D->addAction(actionMono_Off);
        menuStereo_3D->addAction(actionQuadro_120_Hz);
        menuStereo_3D->addAction(actionChecker_Interleaved_3DTV);
        menuStereo_3D->addAction(actionRow_Interleaved_Zalman);
        menuStereo_3D->addAction(actionColumn_Interleaved);
        menuStereo_3D->addAction(actionLeft_eye_view);
        menuStereo_3D->addAction(actionRight_eye_view);
        menuStereo_3D->addAction(actionAnaglyph_Red_Cyan);
        menuStereo_3D->addAction(actionAnaglyph_Green_Magenta);
        menuStereo_3D->addSeparator();
        menuStereo_3D->addAction(actionSwap_Eyes);
        menuAnimate->addAction(actionAppend_key_frame_at_current_view);
        menuAnimate->addAction(actionPlay_movie);
        menuAnimate->addAction(actionSave_movie_frames);
        menuAnimate->addSeparator();
        menuAnimate->addAction(actionClear_movie);
        menuAnimate->addSeparator();
        menuSet_canvas_size->addAction(action1280x720);
        menuWork_Mode->addAction(actionV3DDefault);
        menuWork_Mode->addAction(actionNeuronAnnotator);

        retranslateUi(NaMainWindow);
        QObject::connect(resetViewButton, SIGNAL(clicked()), zoomWidget, SLOT(reset()));
        QObject::connect(viewerControlTabWidget, SIGNAL(currentChanged(int)), viewerStackedWidget, SLOT(setCurrentIndex(int)));
        QObject::connect(ZSlice_horizontalScrollBar, SIGNAL(valueChanged(int)), ZSlice_spinBox, SLOT(setValue(int)));
        QObject::connect(ZSlice_spinBox, SIGNAL(valueChanged(int)), ZSlice_horizontalScrollBar, SLOT(setValue(int)));

        viewerStackedWidget->setCurrentIndex(2);
        viewerControlTabWidget->setCurrentIndex(2);
        controlTabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(NaMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *NaMainWindow)
    {
        NaMainWindow->setWindowTitle(QApplication::translate("NaMainWindow", "(No image loaded) - Vaa3D NeuronAnnotator", 0));
        actionOpen->setText(QApplication::translate("NaMainWindow", "Open MultiColor Stack Folder...", 0));
        actionLog_off->setText(QApplication::translate("NaMainWindow", "Log off", 0));
        actionAbout_NeuronAnnotator->setText(QApplication::translate("NaMainWindow", "About NeuronAnnotator...", 0));
        actionQuit->setText(QApplication::translate("NaMainWindow", "Quit", 0));
        actionV3DDefault_2->setText(QApplication::translate("NaMainWindow", "V3D default", 0));
        actionNeuronAnnotator_2->setText(QApplication::translate("NaMainWindow", "Neuron Annotator", 0));
        actionLoad_Tiff->setText(QApplication::translate("NaMainWindow", "Open 3D Image/Stack...", 0));
        actionLoad_2D_MIP_image->setText(QApplication::translate("NaMainWindow", "Load 2D MIP image...", 0));
        actionCell_Counter_3D_2ch_lsm->setText(QApplication::translate("NaMainWindow", "Cell Counter 3D 2ch lsm...", 0));
        actionPreferences->setText(QApplication::translate("NaMainWindow", "Preferences...", 0));
        actionV3D_default->setText(QApplication::translate("NaMainWindow", "V3D default", 0));
        actionNeuron_annotator->setText(QApplication::translate("NaMainWindow", "Neuron annotator", 0));
        actionV3DDefault->setText(QApplication::translate("NaMainWindow", "Vaa3D default", 0));
        actionNeuronAnnotator->setText(QApplication::translate("NaMainWindow", "Fly Workstation Annotator", 0));
        actionLink_viewers->setText(QApplication::translate("NaMainWindow", "Link viewers", 0));
        actionShow_Crosshair->setText(QApplication::translate("NaMainWindow", "Show Crosshair", 0));
        actionUndo->setText(QApplication::translate("NaMainWindow", "Undo", 0));
        actionRedo->setText(QApplication::translate("NaMainWindow", "Redo", 0));
        actionSelect_All->setText(QApplication::translate("NaMainWindow", "Select All", 0));
        actionSelect_None->setText(QApplication::translate("NaMainWindow", "Select None", 0));
        actionInvert_Selection->setText(QApplication::translate("NaMainWindow", "Invert Selection", 0));
        action3D_Volume->setText(QApplication::translate("NaMainWindow", "3D Volume...", 0));
        action2D_MIP->setText(QApplication::translate("NaMainWindow", "2D MIP...", 0));
        actionScreenShot->setText(QApplication::translate("NaMainWindow", "ScreenShot...", 0));
        actionFoo->setText(QApplication::translate("NaMainWindow", "Foo", 0));
        actionClear_Hide_All->setText(QApplication::translate("NaMainWindow", "Hide everything", 0));
        actionShow_all_neurons_in_empty_space->setText(QApplication::translate("NaMainWindow", "Show all neurons in empty space", 0));
        actionMono_Off->setText(QApplication::translate("NaMainWindow", "Mono/Off", 0));
        actionLeft_eye_view->setText(QApplication::translate("NaMainWindow", "Left Eye View", 0));
        actionRight_eye_view->setText(QApplication::translate("NaMainWindow", "Right Eye View", 0));
        actionAnaglyph_Red_Cyan->setText(QApplication::translate("NaMainWindow", "Anaglyph Red/Cyan", 0));
        actionAnaglyph_Green_Magenta->setText(QApplication::translate("NaMainWindow", "Anaglyph Green/Magenta", 0));
        actionRow_Interleaved_Zalman->setText(QApplication::translate("NaMainWindow", "Row Interleaved (Zalman)", 0));
        actionQuadro_120_Hz->setText(QApplication::translate("NaMainWindow", "Quadro (120 Hz)", 0));
        actionSwap_Eyes->setText(QApplication::translate("NaMainWindow", "Swap Eyes", 0));
        actionShow_Axes->setText(QApplication::translate("NaMainWindow", "Show Axes", 0));
        action3D_alpha_blending->setText(QApplication::translate("NaMainWindow", "Use 3D alpha blending", 0));
        actionAnimate_3D_nutation->setText(QApplication::translate("NaMainWindow", "Animate 3D nutation", 0));
        actionDynamic_range->setText(QApplication::translate("NaMainWindow", "Data colors...", 0));
        actionShow_Brain_Compartments->setText(QApplication::translate("NaMainWindow", "Show Brain Compartments", 0));
        actionOpen_Single_Movie_Stack->setText(QApplication::translate("NaMainWindow", "Open single stack...", 0));
        actionCurrent_MIP->setText(QApplication::translate("NaMainWindow", "Current MIP", 0));
        actionChecker_Interleaved_3DTV->setText(QApplication::translate("NaMainWindow", "Checker Interleaved (3DTV)", 0));
        actionFull_Screen->setText(QApplication::translate("NaMainWindow", "Full Screen", 0));
        actionFull_Screen->setShortcut(QApplication::translate("NaMainWindow", "F", 0));
        actionColumn_Interleaved->setText(QApplication::translate("NaMainWindow", "Column Interleaved", 0));
        actionOpen_Reference_Channel->setText(QApplication::translate("NaMainWindow", "Open reference channel...", 0));
        actionOpen_Neuron_Mask->setText(QApplication::translate("NaMainWindow", "Open neuron mask...", 0));
        actionLoad_movie_as_texture->setText(QApplication::translate("NaMainWindow", "Load movie as texture...", 0));
        actionLoad_fast_separation_result->setText(QApplication::translate("NaMainWindow", "Open fast loading folder...", 0));
        actionX_Rotation_Movie->setText(QApplication::translate("NaMainWindow", "X Rotation Movie...", 0));
        actionMeasure_Frame_Rate->setText(QApplication::translate("NaMainWindow", "Measure Frame Rate", 0));
        actionOpen_Octree_Volume->setText(QApplication::translate("NaMainWindow", "Open Octree Volume...", 0));
        actionOpen_microCT_Cut_Planner->setText(QApplication::translate("NaMainWindow", "Open microCT Trim Planner", 0));
#ifndef QT_NO_TOOLTIP
        actionOpen_microCT_Cut_Planner->setToolTip(QApplication::translate("NaMainWindow", "Open microCT cut planning tool", 0));
#endif // QT_NO_TOOLTIP
        actionAppend_key_frame_at_current_view->setText(QApplication::translate("NaMainWindow", "Add new key frame here", 0));
        actionPlay_movie->setText(QApplication::translate("NaMainWindow", "Play movie", 0));
        actionClear_movie->setText(QApplication::translate("NaMainWindow", "Clear movie", 0));
        actionSave_movie_frames->setText(QApplication::translate("NaMainWindow", "Save movie frames", 0));
        action1280x720->setText(QApplication::translate("NaMainWindow", "1280x720", 0));
        actionAdd_landmark_at_cursor->setText(QApplication::translate("NaMainWindow", "Add landmark at cursor", 0));
        actionClear_landmarks->setText(QApplication::translate("NaMainWindow", "Clear landmarks", 0));
        actionShow_Scale_Bar->setText(QApplication::translate("NaMainWindow", "Show Scale Bar", 0));
#ifndef QT_NO_TOOLTIP
        viewerStackedWidget->setToolTip(QString());
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        naLargeMIPWidget->setToolTip(QApplication::translate("NaMainWindow", "scroll to zoom\n"
"drag to pan\n"
"double-click to recenter", 0));
#endif // QT_NO_TOOLTIP
        progressLabelMip->setText(QApplication::translate("NaMainWindow", "Updating MIP viewer...", 0));
#ifndef QT_NO_TOOLTIP
        naZStackWidget->setToolTip(QApplication::translate("NaMainWindow", "drag to pan\n"
"shift-scroll to zoom\n"
"scroll to z-scan\n"
"double-click to recenter", 0));
#endif // QT_NO_TOOLTIP
        progressLabelZ->setText(QApplication::translate("NaMainWindow", "Updating Z stack view...", 0));
        progressLabel3d->setText(QApplication::translate("NaMainWindow", "Updating 3D view...", 0));
#ifndef QT_NO_TOOLTIP
      //  v3dr_glwidget->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        label_2->setText(QApplication::translate("NaMainWindow", "Loading data...", 0));
        viewerControlTabWidget->setTabText(viewerControlTabWidget->indexOf(largeMipViewerControlPage), QApplication::translate("NaMainWindow", "MIP", 0));
        viewerControlTabWidget->setTabToolTip(viewerControlTabWidget->indexOf(largeMipViewerControlPage), QApplication::translate("NaMainWindow", "View Maximum Intensity Projection", 0));
        HDR_groupBox->setTitle(QApplication::translate("NaMainWindow", "Dynamic Range (HDR) Tool", 0));
#ifndef QT_NO_TOOLTIP
        HDR_checkBox->setToolTip(QApplication::translate("NaMainWindow", "To explore High Dynamic Range images, \n"
"HDR tool adjusts the display range to local \n"
"intensity range in the yellow search box.", 0));
#endif // QT_NO_TOOLTIP
        HDR_checkBox->setText(QApplication::translate("NaMainWindow", "Adjust dynamic range", 0));
        HDRRed_pushButton->setText(QApplication::translate("NaMainWindow", "1", 0));
        HDRGreen_pushButton->setText(QApplication::translate("NaMainWindow", "2", 0));
        HDRBlue_pushButton->setText(QApplication::translate("NaMainWindow", "3", 0));
        HDRNc82_pushButton->setText(QApplication::translate("NaMainWindow", "nc82", 0));
#ifndef QT_NO_TOOLTIP
        BoxSize_label->setToolTip(QApplication::translate("NaMainWindow", "Change the size of the interactive HDR region box.\n"
"You can also resize the box by Ctrl + left mouse click.", 0));
#endif // QT_NO_TOOLTIP
        BoxSize_label->setText(QApplication::translate("NaMainWindow", "Box Size", 0));
#ifndef QT_NO_TOOLTIP
        BoxSize_spinBox->setToolTip(QApplication::translate("NaMainWindow", "Change the size of the interactive HDR region box.\n"
"You can also resize the box by Ctrl + left mouse click.", 0));
#endif // QT_NO_TOOLTIP
        ZSlice_groupBox->setTitle(QApplication::translate("NaMainWindow", "Z Slice", 0));
        viewerControlTabWidget->setTabText(viewerControlTabWidget->indexOf(zSliceViewerControlPage), QApplication::translate("NaMainWindow", "Z stack", 0));
        viewerControlTabWidget->setTabToolTip(viewerControlTabWidget->indexOf(zSliceViewerControlPage), QApplication::translate("NaMainWindow", "View 3D Z-stack", 0));
#ifndef QT_NO_TOOLTIP
        controlTabWidget->setToolTip(QApplication::translate("NaMainWindow", "Adjust 3D viewer object orientation", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        tabRotation->setToolTip(QApplication::translate("NaMainWindow", "Shift-drag in 3D viewer to rotate with the mouse.", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        rotXWidget->setToolTip(QApplication::translate("NaMainWindow", "Rotate about the object X axis", 0));
#endif // QT_NO_TOOLTIP
        rotXWidget->setTextLabel(QApplication::translate("NaMainWindow", "RotX", 0));
#ifndef QT_NO_TOOLTIP
        rotZWidget->setToolTip(QApplication::translate("NaMainWindow", "Rotate about the object Z axis", 0));
#endif // QT_NO_TOOLTIP
        rotZWidget->setTextLabel(QApplication::translate("NaMainWindow", "RotZ", 0));
#ifndef QT_NO_TOOLTIP
        rotYWidget->setToolTip(QApplication::translate("NaMainWindow", "Rotate about the object Y axis", 0));
#endif // QT_NO_TOOLTIP
        rotYWidget->setTextLabel(QApplication::translate("NaMainWindow", "RotY", 0));
#ifndef QT_NO_TOOLTIP
        rotationResetButton->setToolTip(QApplication::translate("NaMainWindow", "Restore original orientation to match MIP and Z-slice views", 0));
#endif // QT_NO_TOOLTIP
        rotationResetButton->setText(QApplication::translate("NaMainWindow", "Reset Rotation", 0));
#ifndef QT_NO_TOOLTIP
        nutateButton->setToolTip(QApplication::translate("NaMainWindow", "Gently rotate the view to enhance the 3D effect.", 0));
#endif // QT_NO_TOOLTIP
        nutateButton->setText(QApplication::translate("NaMainWindow", "Nutate", 0));
        controlTabWidget->setTabText(controlTabWidget->indexOf(tabRotation), QApplication::translate("NaMainWindow", "Rotation", 0));
#ifndef QT_NO_TOOLTIP
        tabVolumeCut->setToolTip(QString());
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        XCutCB->setToolTip(QApplication::translate("NaMainWindow", "checked to lock cut volume", 0));
#endif // QT_NO_TOOLTIP
        XCutCB->setText(QApplication::translate("NaMainWindow", "XCut", 0));
#ifndef QT_NO_TOOLTIP
        YCutCB->setToolTip(QApplication::translate("NaMainWindow", "checked to lock cut volume", 0));
#endif // QT_NO_TOOLTIP
        YCutCB->setText(QApplication::translate("NaMainWindow", "YCut", 0));
#ifndef QT_NO_TOOLTIP
        ZCutCB->setToolTip(QApplication::translate("NaMainWindow", "checked to lock cut volume", 0));
#endif // QT_NO_TOOLTIP
        ZCutCB->setText(QApplication::translate("NaMainWindow", "ZCut", 0));
        label_3->setText(QApplication::translate("NaMainWindow", "Front Slab", 0));
        freezeFrontBackButton->setText(QApplication::translate("NaMainWindow", "Clip", 0));
#ifndef QT_NO_TOOLTIP
        slabThicknessSlider->setToolTip(QApplication::translate("NaMainWindow", "Adjust slab thickness", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        slabPositionSlider->setToolTip(QApplication::translate("NaMainWindow", "Adjust slab position", 0));
#endif // QT_NO_TOOLTIP
        defineClipPlaneButton->setText(QApplication::translate("NaMainWindow", "Custom...", 0));
        customCutButton->setText(QApplication::translate("NaMainWindow", "Clip", 0));
        controlTabWidget->setTabText(controlTabWidget->indexOf(tabVolumeCut), QApplication::translate("NaMainWindow", "Volume Cut", 0));
        label_4->setText(QApplication::translate("NaMainWindow", "Z Thickness", 0));
        compartmentSelectGroupBox->setTitle(QApplication::translate("NaMainWindow", "Compartment Map", 0));
        viewerControlTabWidget->setTabText(viewerControlTabWidget->indexOf(viewer3DControlPage), QApplication::translate("NaMainWindow", "3D", 0));
        viewerControlTabWidget->setTabToolTip(viewerControlTabWidget->indexOf(viewer3DControlPage), QApplication::translate("NaMainWindow", "View 3D volume", 0));
        groupBox_3->setTitle(QApplication::translate("NaMainWindow", "View", 0));
        resetViewButton->setText(QApplication::translate("NaMainWindow", "Reset View", 0));
        groupBox_2->setTitle(QApplication::translate("NaMainWindow", "Color and Shading", 0));
        redToggleButton->setText(QApplication::translate("NaMainWindow", "1", 0));
        greenToggleButton->setText(QApplication::translate("NaMainWindow", "2", 0));
        blueToggleButton->setText(QApplication::translate("NaMainWindow", "3", 0));
#ifndef QT_NO_TOOLTIP
        resetColorsButton->setToolTip(QApplication::translate("NaMainWindow", "Restore default color intensities.", 0));
#endif // QT_NO_TOOLTIP
        resetColorsButton->setText(QApplication::translate("NaMainWindow", "Reset Colors", 0));
        label->setText(QApplication::translate("NaMainWindow", "Sort by:", 0));
#ifndef QT_NO_TOOLTIP
        gallerySortBySizeButton->setToolTip(QApplication::translate("NaMainWindow", "Show fragments with largest volume first", 0));
#endif // QT_NO_TOOLTIP
        gallerySortBySizeButton->setText(QApplication::translate("NaMainWindow", "Size", 0));
#ifndef QT_NO_TOOLTIP
        gallerySortByIndexButton->setToolTip(QApplication::translate("NaMainWindow", "Arrange fragments by numeric index", 0));
#endif // QT_NO_TOOLTIP
        gallerySortByIndexButton->setText(QApplication::translate("NaMainWindow", "Index", 0));
#ifndef QT_NO_TOOLTIP
        gallerySortByColorButton->setToolTip(QApplication::translate("NaMainWindow", "Arrange fragments by similar hue (first and last may be simiar too)", 0));
#endif // QT_NO_TOOLTIP
        gallerySortByColorButton->setText(QApplication::translate("NaMainWindow", "Color", 0));
        gallerySortByNameButton->setText(QApplication::translate("NaMainWindow", "Name", 0));
        menuFile->setTitle(QApplication::translate("NaMainWindow", "File", 0));
        menuExport->setTitle(QApplication::translate("NaMainWindow", "Export", 0));
        menuOpen_Recent->setTitle(QApplication::translate("NaMainWindow", "Open Recent", 0));
        menuHelp->setTitle(QApplication::translate("NaMainWindow", "Help", 0));
        menuEdit->setTitle(QApplication::translate("NaMainWindow", "Edit", 0));
        menuView->setTitle(QApplication::translate("NaMainWindow", "View", 0));
        menuStereo_3D->setTitle(QApplication::translate("NaMainWindow", "Stereo 3D", 0));
        menuAnimate->setTitle(QApplication::translate("NaMainWindow", "Animate", 0));
        menuSet_canvas_size->setTitle(QApplication::translate("NaMainWindow", "Set canvas size", 0));
        menuWork_Mode->setTitle(QApplication::translate("NaMainWindow", "Work-Mode", 0));
    } // retranslateUi

};

namespace Ui {
    class NaMainWindow: public Ui_NaMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NAMAINWINDOW_H
