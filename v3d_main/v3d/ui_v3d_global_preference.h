/********************************************************************************
** Form generated from reading UI file 'v3d_global_preference.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_V3D_GLOBAL_PREFERENCE_H
#define UI_V3D_GLOBAL_PREFERENCE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Dialog_v3d_preference
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *vboxLayout;
    QTabWidget *V3D_MainWindows;
    QWidget *V3D_setting_mainwindow;
    QWidget *layoutWidget1;
    QVBoxLayout *verticalLayout_3;
    QCheckBox *checkBox_yaxis_up;
    QCheckBox *checkBox_autoConvert2_8bit;
    QHBoxLayout *hboxLayout;
    QLabel *label_4;
    QSpinBox *spinBox_defaultRightShiftBits;
    QCheckBox *checkBox_autoRescale16bitDisplay;
    QHBoxLayout *hboxLayout1;
    QLabel *label_2;
    QSpinBox *spinBox_lookGlassSize;
    QHBoxLayout *hboxLayout2;
    QLabel *label;
    QSpinBox *spinBox_markerSize;
    QCheckBox *checkBox_libTiff_Mylib;
    QCheckBox *checkBox_blendColor;
    QWidget *V3D_setting_3dviewer;
    QGroupBox *groupBox_2;
    QCheckBox *checkBox_autoVideoCardCompress;
    QCheckBox *checkBox_autoVideoCard3DTex;
    QCheckBox *checkBox_autoVideoCardNPTTex;
    QWidget *layoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_7;
    QSpinBox *spinBox_autoVideoCardStreamMode;
    QWidget *layoutWidget2;
    QVBoxLayout *verticalLayout;
    QCheckBox *checkBox_scrollupZoomin;
    QCheckBox *checkBox_autoOpenImg3DViewer;
    QCheckBox *checkBox_autoDispXYZAxes;
    QCheckBox *checkBox_autoDispBoundingBox;
    QHBoxLayout *horizontalLayout;
    QLabel *label_3;
    QSpinBox *spinBox_markerAmplifyFactor;
    QCheckBox *checkBox_autoSWCLineMode;
    QWidget *V3D_setting_imganalysis;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label_5;
    QComboBox *comboBox_reg_markermatch_method;
    QLabel *label_6;
    QComboBox *comboBox_warpingfield_method;
    QGroupBox *groupBox_3;
    QWidget *layoutWidget3;
    QGridLayout *gridLayout1;
    QCheckBox *checkBox_3dcurve_inertia;
    QCheckBox *checkBox_3dcurve_autoconnecttips;
    QCheckBox *checkBox_3dcurve_autodeform;
    QCheckBox *checkBox_3dcurve_autowidth;
    QCheckBox *checkBox_3dcurve_width_from_xyonly;
    QWidget *tab;
    QWidget *layoutWidget4;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_8;
    QSpinBox *spinBox_iChannel_for_plugin;
    QCheckBox *checkBox_b_plugin_dispResInNewWindow;
    QCheckBox *checkBox_b_plugin_dispParameterDialog;
    QCheckBox *checkBox_b_plugin_outputImgRescale;
    QCheckBox *checkBox_b_plugin_outputImgConvert2UINT8;
    QHBoxLayout *hboxLayout3;
    QPushButton *pushButton_cancel;
    QPushButton *pushButton_ok;

    void setupUi(QDialog *Dialog_v3d_preference)
    {
        if (Dialog_v3d_preference->objectName().isEmpty())
            Dialog_v3d_preference->setObjectName(QString::fromUtf8("Dialog_v3d_preference"));
        Dialog_v3d_preference->resize(575, 416);
        layoutWidget = new QWidget(Dialog_v3d_preference);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(21, 20, 531, 371));
        vboxLayout = new QVBoxLayout(layoutWidget);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        V3D_MainWindows = new QTabWidget(layoutWidget);
        V3D_MainWindows->setObjectName(QString::fromUtf8("V3D_MainWindows"));
        V3D_setting_mainwindow = new QWidget();
        V3D_setting_mainwindow->setObjectName(QString::fromUtf8("V3D_setting_mainwindow"));
        layoutWidget1 = new QWidget(V3D_setting_mainwindow);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(12, 30, 501, 231));
        verticalLayout_3 = new QVBoxLayout(layoutWidget1);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        checkBox_yaxis_up = new QCheckBox(layoutWidget1);
        checkBox_yaxis_up->setObjectName(QString::fromUtf8("checkBox_yaxis_up"));

        verticalLayout_3->addWidget(checkBox_yaxis_up);

        checkBox_autoConvert2_8bit = new QCheckBox(layoutWidget1);
        checkBox_autoConvert2_8bit->setObjectName(QString::fromUtf8("checkBox_autoConvert2_8bit"));

        verticalLayout_3->addWidget(checkBox_autoConvert2_8bit);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        label_4 = new QLabel(layoutWidget1);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        hboxLayout->addWidget(label_4);

        spinBox_defaultRightShiftBits = new QSpinBox(layoutWidget1);
        spinBox_defaultRightShiftBits->setObjectName(QString::fromUtf8("spinBox_defaultRightShiftBits"));
        spinBox_defaultRightShiftBits->setMinimum(-1);
        spinBox_defaultRightShiftBits->setMaximum(32);
        spinBox_defaultRightShiftBits->setValue(-1);

        hboxLayout->addWidget(spinBox_defaultRightShiftBits);


        verticalLayout_3->addLayout(hboxLayout);

        checkBox_autoRescale16bitDisplay = new QCheckBox(layoutWidget1);
        checkBox_autoRescale16bitDisplay->setObjectName(QString::fromUtf8("checkBox_autoRescale16bitDisplay"));

        verticalLayout_3->addWidget(checkBox_autoRescale16bitDisplay);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        label_2 = new QLabel(layoutWidget1);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        hboxLayout1->addWidget(label_2);

        spinBox_lookGlassSize = new QSpinBox(layoutWidget1);
        spinBox_lookGlassSize->setObjectName(QString::fromUtf8("spinBox_lookGlassSize"));
        spinBox_lookGlassSize->setMinimum(5);
        spinBox_lookGlassSize->setValue(7);

        hboxLayout1->addWidget(spinBox_lookGlassSize);


        verticalLayout_3->addLayout(hboxLayout1);

        hboxLayout2 = new QHBoxLayout();
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        label = new QLabel(layoutWidget1);
        label->setObjectName(QString::fromUtf8("label"));

        hboxLayout2->addWidget(label);

        spinBox_markerSize = new QSpinBox(layoutWidget1);
        spinBox_markerSize->setObjectName(QString::fromUtf8("spinBox_markerSize"));
        spinBox_markerSize->setValue(5);

        hboxLayout2->addWidget(spinBox_markerSize);


        verticalLayout_3->addLayout(hboxLayout2);

        checkBox_libTiff_Mylib = new QCheckBox(layoutWidget1);
        checkBox_libTiff_Mylib->setObjectName(QString::fromUtf8("checkBox_libTiff_Mylib"));

        verticalLayout_3->addWidget(checkBox_libTiff_Mylib);

        checkBox_blendColor = new QCheckBox(layoutWidget1);
        checkBox_blendColor->setObjectName(QString::fromUtf8("checkBox_blendColor"));
        checkBox_blendColor->setChecked(true);

        verticalLayout_3->addWidget(checkBox_blendColor);

        V3D_MainWindows->addTab(V3D_setting_mainwindow, QString());
        V3D_setting_3dviewer = new QWidget();
        V3D_setting_3dviewer->setObjectName(QString::fromUtf8("V3D_setting_3dviewer"));
        groupBox_2 = new QGroupBox(V3D_setting_3dviewer);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(20, 150, 471, 131));
        checkBox_autoVideoCardCompress = new QCheckBox(groupBox_2);
        checkBox_autoVideoCardCompress->setObjectName(QString::fromUtf8("checkBox_autoVideoCardCompress"));
        checkBox_autoVideoCardCompress->setGeometry(QRect(10, 30, 341, 21));
        checkBox_autoVideoCard3DTex = new QCheckBox(groupBox_2);
        checkBox_autoVideoCard3DTex->setObjectName(QString::fromUtf8("checkBox_autoVideoCard3DTex"));
        checkBox_autoVideoCard3DTex->setGeometry(QRect(10, 50, 451, 21));
        checkBox_autoVideoCardNPTTex = new QCheckBox(groupBox_2);
        checkBox_autoVideoCardNPTTex->setObjectName(QString::fromUtf8("checkBox_autoVideoCardNPTTex"));
        checkBox_autoVideoCardNPTTex->setGeometry(QRect(10, 70, 451, 21));
        layoutWidget_2 = new QWidget(groupBox_2);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(10, 90, 404, 27));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget_2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        label_7 = new QLabel(layoutWidget_2);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_2->addWidget(label_7);

        spinBox_autoVideoCardStreamMode = new QSpinBox(layoutWidget_2);
        spinBox_autoVideoCardStreamMode->setObjectName(QString::fromUtf8("spinBox_autoVideoCardStreamMode"));
        spinBox_autoVideoCardStreamMode->setMinimum(1);
        spinBox_autoVideoCardStreamMode->setMaximum(100);
        spinBox_autoVideoCardStreamMode->setSingleStep(1);
        spinBox_autoVideoCardStreamMode->setValue(1);

        horizontalLayout_2->addWidget(spinBox_autoVideoCardStreamMode);

        layoutWidget2 = new QWidget(V3D_setting_3dviewer);
        layoutWidget2->setObjectName(QString::fromUtf8("layoutWidget2"));
        layoutWidget2->setGeometry(QRect(30, 10, 359, 142));
        verticalLayout = new QVBoxLayout(layoutWidget2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        checkBox_scrollupZoomin = new QCheckBox(layoutWidget2);
        checkBox_scrollupZoomin->setObjectName(QString::fromUtf8("checkBox_scrollupZoomin"));

        verticalLayout->addWidget(checkBox_scrollupZoomin);

        checkBox_autoOpenImg3DViewer = new QCheckBox(layoutWidget2);
        checkBox_autoOpenImg3DViewer->setObjectName(QString::fromUtf8("checkBox_autoOpenImg3DViewer"));

        verticalLayout->addWidget(checkBox_autoOpenImg3DViewer);

        checkBox_autoDispXYZAxes = new QCheckBox(layoutWidget2);
        checkBox_autoDispXYZAxes->setObjectName(QString::fromUtf8("checkBox_autoDispXYZAxes"));

        verticalLayout->addWidget(checkBox_autoDispXYZAxes);

        checkBox_autoDispBoundingBox = new QCheckBox(layoutWidget2);
        checkBox_autoDispBoundingBox->setObjectName(QString::fromUtf8("checkBox_autoDispBoundingBox"));

        verticalLayout->addWidget(checkBox_autoDispBoundingBox);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_3 = new QLabel(layoutWidget2);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout->addWidget(label_3);

        spinBox_markerAmplifyFactor = new QSpinBox(layoutWidget2);
        spinBox_markerAmplifyFactor->setObjectName(QString::fromUtf8("spinBox_markerAmplifyFactor"));
        spinBox_markerAmplifyFactor->setMinimum(1);
        spinBox_markerAmplifyFactor->setMaximum(100);
        spinBox_markerAmplifyFactor->setSingleStep(5);
        spinBox_markerAmplifyFactor->setValue(25);

        horizontalLayout->addWidget(spinBox_markerAmplifyFactor);


        verticalLayout->addLayout(horizontalLayout);

        checkBox_autoSWCLineMode = new QCheckBox(layoutWidget2);
        checkBox_autoSWCLineMode->setObjectName(QString::fromUtf8("checkBox_autoSWCLineMode"));

        verticalLayout->addWidget(checkBox_autoSWCLineMode);

        V3D_MainWindows->addTab(V3D_setting_3dviewer, QString());
        V3D_setting_imganalysis = new QWidget();
        V3D_setting_imganalysis->setObjectName(QString::fromUtf8("V3D_setting_imganalysis"));
        groupBox = new QGroupBox(V3D_setting_imganalysis);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(30, 10, 471, 114));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setEnabled(false);

        gridLayout->addWidget(label_5, 0, 0, 1, 1);

        comboBox_reg_markermatch_method = new QComboBox(groupBox);
        comboBox_reg_markermatch_method->setObjectName(QString::fromUtf8("comboBox_reg_markermatch_method"));
        comboBox_reg_markermatch_method->setEnabled(false);

        gridLayout->addWidget(comboBox_reg_markermatch_method, 0, 1, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setEnabled(false);

        gridLayout->addWidget(label_6, 1, 0, 1, 1);

        comboBox_warpingfield_method = new QComboBox(groupBox);
        comboBox_warpingfield_method->setObjectName(QString::fromUtf8("comboBox_warpingfield_method"));
        comboBox_warpingfield_method->setEnabled(false);

        gridLayout->addWidget(comboBox_warpingfield_method, 1, 1, 1, 1);

        groupBox_3 = new QGroupBox(V3D_setting_imganalysis);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(30, 130, 471, 131));
        layoutWidget3 = new QWidget(groupBox_3);
        layoutWidget3->setObjectName(QString::fromUtf8("layoutWidget3"));
        layoutWidget3->setGeometry(QRect(10, 30, 431, 87));
        gridLayout1 = new QGridLayout(layoutWidget3);
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        gridLayout1->setContentsMargins(0, 0, 0, 0);
        checkBox_3dcurve_inertia = new QCheckBox(layoutWidget3);
        checkBox_3dcurve_inertia->setObjectName(QString::fromUtf8("checkBox_3dcurve_inertia"));

        gridLayout1->addWidget(checkBox_3dcurve_inertia, 0, 0, 1, 2);

        checkBox_3dcurve_autoconnecttips = new QCheckBox(layoutWidget3);
        checkBox_3dcurve_autoconnecttips->setObjectName(QString::fromUtf8("checkBox_3dcurve_autoconnecttips"));

        gridLayout1->addWidget(checkBox_3dcurve_autoconnecttips, 1, 0, 1, 2);

        checkBox_3dcurve_autodeform = new QCheckBox(layoutWidget3);
        checkBox_3dcurve_autodeform->setObjectName(QString::fromUtf8("checkBox_3dcurve_autodeform"));

        gridLayout1->addWidget(checkBox_3dcurve_autodeform, 2, 0, 1, 1);

        checkBox_3dcurve_autowidth = new QCheckBox(layoutWidget3);
        checkBox_3dcurve_autowidth->setObjectName(QString::fromUtf8("checkBox_3dcurve_autowidth"));

        gridLayout1->addWidget(checkBox_3dcurve_autowidth, 2, 1, 1, 1);

        checkBox_3dcurve_width_from_xyonly = new QCheckBox(layoutWidget3);
        checkBox_3dcurve_width_from_xyonly->setObjectName(QString::fromUtf8("checkBox_3dcurve_width_from_xyonly"));

        gridLayout1->addWidget(checkBox_3dcurve_width_from_xyonly, 3, 0, 1, 2);

        V3D_MainWindows->addTab(V3D_setting_imganalysis, QString());
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        layoutWidget4 = new QWidget(tab);
        layoutWidget4->setObjectName(QString::fromUtf8("layoutWidget4"));
        layoutWidget4->setGeometry(QRect(20, 20, 487, 122));
        verticalLayout_2 = new QVBoxLayout(layoutWidget4);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_8 = new QLabel(layoutWidget4);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        horizontalLayout_3->addWidget(label_8);

        spinBox_iChannel_for_plugin = new QSpinBox(layoutWidget4);
        spinBox_iChannel_for_plugin->setObjectName(QString::fromUtf8("spinBox_iChannel_for_plugin"));

        horizontalLayout_3->addWidget(spinBox_iChannel_for_plugin);


        verticalLayout_2->addLayout(horizontalLayout_3);

        checkBox_b_plugin_dispResInNewWindow = new QCheckBox(layoutWidget4);
        checkBox_b_plugin_dispResInNewWindow->setObjectName(QString::fromUtf8("checkBox_b_plugin_dispResInNewWindow"));

        verticalLayout_2->addWidget(checkBox_b_plugin_dispResInNewWindow);

        checkBox_b_plugin_dispParameterDialog = new QCheckBox(layoutWidget4);
        checkBox_b_plugin_dispParameterDialog->setObjectName(QString::fromUtf8("checkBox_b_plugin_dispParameterDialog"));

        verticalLayout_2->addWidget(checkBox_b_plugin_dispParameterDialog);

        checkBox_b_plugin_outputImgRescale = new QCheckBox(layoutWidget4);
        checkBox_b_plugin_outputImgRescale->setObjectName(QString::fromUtf8("checkBox_b_plugin_outputImgRescale"));

        verticalLayout_2->addWidget(checkBox_b_plugin_outputImgRescale);

        checkBox_b_plugin_outputImgConvert2UINT8 = new QCheckBox(layoutWidget4);
        checkBox_b_plugin_outputImgConvert2UINT8->setObjectName(QString::fromUtf8("checkBox_b_plugin_outputImgConvert2UINT8"));

        verticalLayout_2->addWidget(checkBox_b_plugin_outputImgConvert2UINT8);

        V3D_MainWindows->addTab(tab, QString());

        vboxLayout->addWidget(V3D_MainWindows);

        hboxLayout3 = new QHBoxLayout();
        hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
        pushButton_cancel = new QPushButton(layoutWidget);
        pushButton_cancel->setObjectName(QString::fromUtf8("pushButton_cancel"));

        hboxLayout3->addWidget(pushButton_cancel);

        pushButton_ok = new QPushButton(layoutWidget);
        pushButton_ok->setObjectName(QString::fromUtf8("pushButton_ok"));
        pushButton_ok->setDefault(true);

        hboxLayout3->addWidget(pushButton_ok);


        vboxLayout->addLayout(hboxLayout3);


        retranslateUi(Dialog_v3d_preference);

        V3D_MainWindows->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(Dialog_v3d_preference);
    } // setupUi

    void retranslateUi(QDialog *Dialog_v3d_preference)
    {
        Dialog_v3d_preference->setWindowTitle(QApplication::translate("Dialog_v3d_preference", "Preferences", 0, QApplication::UnicodeUTF8));
        checkBox_yaxis_up->setText(QApplication::translate("Dialog_v3d_preference", "Y axis direction (from bottom to top)", 0, QApplication::UnicodeUTF8));
        checkBox_autoConvert2_8bit->setText(QApplication::translate("Dialog_v3d_preference", "Automatic conversion 16(32)bit to 8bit when open an image", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("Dialog_v3d_preference", "# bits to shift for 16(32)bit->8bit conversion \n"
"(-1) for manual input", 0, QApplication::UnicodeUTF8));
        checkBox_autoRescale16bitDisplay->setText(QApplication::translate("Dialog_v3d_preference", "Automatic rescale 16bit data in display", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Dialog_v3d_preference", "Looking glass size  (radius, # pixels)", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Dialog_v3d_preference", "Default marker size (radius, # pixels)", 0, QApplication::UnicodeUTF8));
        checkBox_libTiff_Mylib->setText(QApplication::translate("Dialog_v3d_preference", "Use Libtiff (ON) (for <2G file) or MYLib (OFF) (for >2G file) for TIFF/LSM file", 0, QApplication::UnicodeUTF8));
        checkBox_blendColor->setText(QApplication::translate("Dialog_v3d_preference", "Blend multi-channel's color", 0, QApplication::UnicodeUTF8));
        V3D_MainWindows->setTabText(V3D_MainWindows->indexOf(V3D_setting_mainwindow), QApplication::translate("Dialog_v3d_preference", "Main TriView Window", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("Dialog_v3d_preference", "Automatic video-card mode", 0, QApplication::UnicodeUTF8));
        checkBox_autoVideoCardCompress->setText(QApplication::translate("Dialog_v3d_preference", "compression mode", 0, QApplication::UnicodeUTF8));
        checkBox_autoVideoCard3DTex->setText(QApplication::translate("Dialog_v3d_preference", "3D texture mode (Not preferred for large volume)", 0, QApplication::UnicodeUTF8));
        checkBox_autoVideoCardNPTTex->setText(QApplication::translate("Dialog_v3d_preference", "non-power-of-two size texture mode", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("Dialog_v3d_preference", "Stream texture mode (detail in Alt-V in 3D Viewer)", 0, QApplication::UnicodeUTF8));
        checkBox_scrollupZoomin->setText(QApplication::translate("Dialog_v3d_preference", "Mouse wheel scroll-up/down for zoom-in/out", 0, QApplication::UnicodeUTF8));
        checkBox_autoOpenImg3DViewer->setText(QApplication::translate("Dialog_v3d_preference", "Automatic launch 3D viewer when open an image", 0, QApplication::UnicodeUTF8));
        checkBox_autoDispXYZAxes->setText(QApplication::translate("Dialog_v3d_preference", "Automatic display XYZ axes of the 3D scene", 0, QApplication::UnicodeUTF8));
        checkBox_autoDispBoundingBox->setText(QApplication::translate("Dialog_v3d_preference", "Automatic display bounding box of scene", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Dialog_v3d_preference", "Default marker display zoom factor ", 0, QApplication::UnicodeUTF8));
        checkBox_autoSWCLineMode->setText(QApplication::translate("Dialog_v3d_preference", "Default display SWC file in LINE mode (skeleton only)", 0, QApplication::UnicodeUTF8));
        V3D_MainWindows->setTabText(V3D_MainWindows->indexOf(V3D_setting_3dviewer), QApplication::translate("Dialog_v3d_preference", "3D Viewer", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("Dialog_v3d_preference", "3D image registration (disabled as of now; will be avaiable as a plugin)", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("Dialog_v3d_preference", "Landmark matching method", 0, QApplication::UnicodeUTF8));
        comboBox_reg_markermatch_method->clear();
        comboBox_reg_markermatch_method->insertItems(0, QStringList()
         << QApplication::translate("Dialog_v3d_preference", "MATCH_MI", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_v3d_preference", "MATCH_MULTIPLE_MI_INT_CORR", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_v3d_preference", "MATCH_INTENSITY", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_v3d_preference", "MATCH_CORRCOEF", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_v3d_preference", "MATCH_IMOMENT", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_v3d_preference", "MATCH_MEANOFCIRCLES", 0, QApplication::UnicodeUTF8)
        );
        label_6->setText(QApplication::translate("Dialog_v3d_preference", "Displacement filed \n"
"generating method", 0, QApplication::UnicodeUTF8));
        comboBox_warpingfield_method->clear();
        comboBox_warpingfield_method->insertItems(0, QStringList()
         << QApplication::translate("Dialog_v3d_preference", "TPS", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_v3d_preference", "Hier-B-Spline", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_v3d_preference", "TPS-B-Spline-interpolation", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_v3d_preference", "TPS-linear-interpolation", 0, QApplication::UnicodeUTF8)
        );
        groupBox_3->setTitle(QApplication::translate("Dialog_v3d_preference", "3D curve", 0, QApplication::UnicodeUTF8));
        checkBox_3dcurve_inertia->setText(QApplication::translate("Dialog_v3d_preference", "3D curve-node fitting using inertia-rule (close to the last node)", 0, QApplication::UnicodeUTF8));
        checkBox_3dcurve_autoconnecttips->setText(QApplication::translate("Dialog_v3d_preference", "auto-connect start/end tips to existing neuron/curve nodes", 0, QApplication::UnicodeUTF8));
        checkBox_3dcurve_autodeform->setText(QApplication::translate("Dialog_v3d_preference", "auto-deform based on img content", 0, QApplication::UnicodeUTF8));
        checkBox_3dcurve_autowidth->setText(QApplication::translate("Dialog_v3d_preference", "auto-estimate width", 0, QApplication::UnicodeUTF8));
        checkBox_3dcurve_width_from_xyonly->setText(QApplication::translate("Dialog_v3d_preference", "estimate curve width based on XY plane only (but not XYZ)", 0, QApplication::UnicodeUTF8));
        V3D_MainWindows->setTabText(V3D_MainWindows->indexOf(V3D_setting_imganalysis), QApplication::translate("Dialog_v3d_preference", "Image Analysis", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("Dialog_v3d_preference", "Default channel id for plugin image analysis (-1 for all channels)", 0, QApplication::UnicodeUTF8));
        checkBox_b_plugin_dispResInNewWindow->setText(QApplication::translate("Dialog_v3d_preference", "Display plugin results in a new window", 0, QApplication::UnicodeUTF8));
        checkBox_b_plugin_dispParameterDialog->setText(QApplication::translate("Dialog_v3d_preference", "Ask for plugin parameters every time", 0, QApplication::UnicodeUTF8));
        checkBox_b_plugin_outputImgRescale->setText(QApplication::translate("Dialog_v3d_preference", "Rescale plugin output image intensity to [0, 255]", 0, QApplication::UnicodeUTF8));
        checkBox_b_plugin_outputImgConvert2UINT8->setText(QApplication::translate("Dialog_v3d_preference", "Convert plugin output as UINT8 (if turn off, then will be 32bit float)", 0, QApplication::UnicodeUTF8));
        V3D_MainWindows->setTabText(V3D_MainWindows->indexOf(tab), QApplication::translate("Dialog_v3d_preference", "Plugins", 0, QApplication::UnicodeUTF8));
        pushButton_cancel->setText(QApplication::translate("Dialog_v3d_preference", "Cancel", 0, QApplication::UnicodeUTF8));
        pushButton_ok->setText(QApplication::translate("Dialog_v3d_preference", "OK", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog_v3d_preference: public Ui_Dialog_v3d_preference {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_V3D_GLOBAL_PREFERENCE_H
