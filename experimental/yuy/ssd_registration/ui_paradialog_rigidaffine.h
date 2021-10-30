/********************************************************************************
** Form generated from reading UI file 'paradialog_rigidaffine.ui'
**
** Created: Tue Dec 6 11:56:30 2011
**      by: Qt User Interface Compiler version 4.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARADIALOG_RIGIDAFFINE_H
#define UI_PARADIALOG_RIGIDAFFINE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_Paradialog_rigidaffine
{
public:
    QDialogButtonBox *buttonBox;
    QGroupBox *groupBox_fileio;
    QLabel *label_img_tar;
    QLabel *label_img_sub;
    QLineEdit *lineEdit_img_tar;
    QPushButton *pushButton_img_tar;
    QPushButton *pushButton_img_sub;
    QLineEdit *lineEdit_img_sub;
    QLineEdit *lineEdit_img_sub2tar;
    QLabel *label_img_sub2tar;
    QPushButton *pushButton_img_sub2tar;
    QPushButton *pushButton_swc_grid;
    QLabel *label_swc_grid;
    QLineEdit *lineEdit_swc_grid;
    QGroupBox *groupBox_paras;
    QLineEdit *lineEdit_refchannel;
    QLabel *label_refchannel;
    QLineEdit *lineEdit_downsampleratio;
    QLabel *label_downsample;
    QGroupBox *groupBox_Gaussian;
    QLabel *label_Gau_sigma;
    QLabel *label_Gau_radius;
    QLineEdit *lineEdit_Gau_radius;
    QLineEdit *lineEdit_Gau_sigma;
    QCheckBox *checkBox_histogrammatching;
    QCheckBox *checkBox_alignedges;
    QFrame *line;
    QLineEdit *lineEdit_iter_max;
    QLabel *label_iter_max;
    QLabel *label_step_multiplyfactor;
    QLineEdit *lineEdit_step_multiplyfactor;
    QLabel *label_step_annealingrate;
    QLineEdit *lineEdit_step_annealingrate;
    QLabel *label_step_min;
    QLineEdit *lineEdit_step_min;

    void setupUi(QDialog *Paradialog_rigidaffine)
    {
        if (Paradialog_rigidaffine->objectName().isEmpty())
            Paradialog_rigidaffine->setObjectName(QString::fromUtf8("Paradialog_rigidaffine"));
        Paradialog_rigidaffine->resize(471, 543);
        buttonBox = new QDialogButtonBox(Paradialog_rigidaffine);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(280, 510, 181, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        groupBox_fileio = new QGroupBox(Paradialog_rigidaffine);
        groupBox_fileio->setObjectName(QString::fromUtf8("groupBox_fileio"));
        groupBox_fileio->setGeometry(QRect(10, 0, 451, 241));
        label_img_tar = new QLabel(groupBox_fileio);
        label_img_tar->setObjectName(QString::fromUtf8("label_img_tar"));
        label_img_tar->setGeometry(QRect(10, 30, 411, 21));
        label_img_sub = new QLabel(groupBox_fileio);
        label_img_sub->setObjectName(QString::fromUtf8("label_img_sub"));
        label_img_sub->setGeometry(QRect(10, 80, 411, 21));
        lineEdit_img_tar = new QLineEdit(groupBox_fileio);
        lineEdit_img_tar->setObjectName(QString::fromUtf8("lineEdit_img_tar"));
        lineEdit_img_tar->setGeometry(QRect(10, 50, 371, 22));
        pushButton_img_tar = new QPushButton(groupBox_fileio);
        pushButton_img_tar->setObjectName(QString::fromUtf8("pushButton_img_tar"));
        pushButton_img_tar->setGeometry(QRect(380, 46, 71, 32));
        pushButton_img_sub = new QPushButton(groupBox_fileio);
        pushButton_img_sub->setObjectName(QString::fromUtf8("pushButton_img_sub"));
        pushButton_img_sub->setGeometry(QRect(380, 97, 71, 32));
        lineEdit_img_sub = new QLineEdit(groupBox_fileio);
        lineEdit_img_sub->setObjectName(QString::fromUtf8("lineEdit_img_sub"));
        lineEdit_img_sub->setGeometry(QRect(10, 100, 371, 22));
        lineEdit_img_sub2tar = new QLineEdit(groupBox_fileio);
        lineEdit_img_sub2tar->setObjectName(QString::fromUtf8("lineEdit_img_sub2tar"));
        lineEdit_img_sub2tar->setGeometry(QRect(10, 150, 371, 22));
        label_img_sub2tar = new QLabel(groupBox_fileio);
        label_img_sub2tar->setObjectName(QString::fromUtf8("label_img_sub2tar"));
        label_img_sub2tar->setGeometry(QRect(10, 130, 411, 21));
        pushButton_img_sub2tar = new QPushButton(groupBox_fileio);
        pushButton_img_sub2tar->setObjectName(QString::fromUtf8("pushButton_img_sub2tar"));
        pushButton_img_sub2tar->setGeometry(QRect(380, 146, 71, 32));
        pushButton_swc_grid = new QPushButton(groupBox_fileio);
        pushButton_swc_grid->setObjectName(QString::fromUtf8("pushButton_swc_grid"));
        pushButton_swc_grid->setGeometry(QRect(380, 197, 71, 32));
        label_swc_grid = new QLabel(groupBox_fileio);
        label_swc_grid->setObjectName(QString::fromUtf8("label_swc_grid"));
        label_swc_grid->setGeometry(QRect(10, 180, 411, 21));
        lineEdit_swc_grid = new QLineEdit(groupBox_fileio);
        lineEdit_swc_grid->setObjectName(QString::fromUtf8("lineEdit_swc_grid"));
        lineEdit_swc_grid->setGeometry(QRect(10, 200, 371, 22));
        groupBox_paras = new QGroupBox(Paradialog_rigidaffine);
        groupBox_paras->setObjectName(QString::fromUtf8("groupBox_paras"));
        groupBox_paras->setGeometry(QRect(10, 250, 451, 251));
        lineEdit_refchannel = new QLineEdit(groupBox_paras);
        lineEdit_refchannel->setObjectName(QString::fromUtf8("lineEdit_refchannel"));
        lineEdit_refchannel->setGeometry(QRect(170, 36, 41, 22));
        label_refchannel = new QLabel(groupBox_paras);
        label_refchannel->setObjectName(QString::fromUtf8("label_refchannel"));
        label_refchannel->setGeometry(QRect(10, 30, 131, 31));
        lineEdit_downsampleratio = new QLineEdit(groupBox_paras);
        lineEdit_downsampleratio->setObjectName(QString::fromUtf8("lineEdit_downsampleratio"));
        lineEdit_downsampleratio->setGeometry(QRect(170, 70, 41, 22));
        label_downsample = new QLabel(groupBox_paras);
        label_downsample->setObjectName(QString::fromUtf8("label_downsample"));
        label_downsample->setGeometry(QRect(10, 70, 161, 21));
        groupBox_Gaussian = new QGroupBox(groupBox_paras);
        groupBox_Gaussian->setObjectName(QString::fromUtf8("groupBox_Gaussian"));
        groupBox_Gaussian->setGeometry(QRect(10, 160, 211, 80));
        groupBox_Gaussian->setCheckable(true);
        label_Gau_sigma = new QLabel(groupBox_Gaussian);
        label_Gau_sigma->setObjectName(QString::fromUtf8("label_Gau_sigma"));
        label_Gau_sigma->setGeometry(QRect(20, 52, 91, 21));
        label_Gau_radius = new QLabel(groupBox_Gaussian);
        label_Gau_radius->setObjectName(QString::fromUtf8("label_Gau_radius"));
        label_Gau_radius->setGeometry(QRect(20, 25, 101, 21));
        lineEdit_Gau_radius = new QLineEdit(groupBox_Gaussian);
        lineEdit_Gau_radius->setObjectName(QString::fromUtf8("lineEdit_Gau_radius"));
        lineEdit_Gau_radius->setGeometry(QRect(160, 25, 41, 22));
        lineEdit_Gau_sigma = new QLineEdit(groupBox_Gaussian);
        lineEdit_Gau_sigma->setObjectName(QString::fromUtf8("lineEdit_Gau_sigma"));
        lineEdit_Gau_sigma->setGeometry(QRect(160, 50, 41, 22));
        checkBox_histogrammatching = new QCheckBox(groupBox_paras);
        checkBox_histogrammatching->setObjectName(QString::fromUtf8("checkBox_histogrammatching"));
        checkBox_histogrammatching->setGeometry(QRect(10, 100, 161, 21));
        checkBox_alignedges = new QCheckBox(groupBox_paras);
        checkBox_alignedges->setObjectName(QString::fromUtf8("checkBox_alignedges"));
        checkBox_alignedges->setGeometry(QRect(10, 130, 161, 21));
        line = new QFrame(groupBox_paras);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(220, 30, 20, 211));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        lineEdit_iter_max = new QLineEdit(groupBox_paras);
        lineEdit_iter_max->setObjectName(QString::fromUtf8("lineEdit_iter_max"));
        lineEdit_iter_max->setGeometry(QRect(390, 40, 51, 22));
        label_iter_max = new QLabel(groupBox_paras);
        label_iter_max->setObjectName(QString::fromUtf8("label_iter_max"));
        label_iter_max->setGeometry(QRect(240, 40, 151, 21));
        label_step_multiplyfactor = new QLabel(groupBox_paras);
        label_step_multiplyfactor->setObjectName(QString::fromUtf8("label_step_multiplyfactor"));
        label_step_multiplyfactor->setGeometry(QRect(240, 70, 151, 21));
        lineEdit_step_multiplyfactor = new QLineEdit(groupBox_paras);
        lineEdit_step_multiplyfactor->setObjectName(QString::fromUtf8("lineEdit_step_multiplyfactor"));
        lineEdit_step_multiplyfactor->setGeometry(QRect(390, 70, 51, 22));
        label_step_annealingrate = new QLabel(groupBox_paras);
        label_step_annealingrate->setObjectName(QString::fromUtf8("label_step_annealingrate"));
        label_step_annealingrate->setGeometry(QRect(240, 100, 161, 21));
        lineEdit_step_annealingrate = new QLineEdit(groupBox_paras);
        lineEdit_step_annealingrate->setObjectName(QString::fromUtf8("lineEdit_step_annealingrate"));
        lineEdit_step_annealingrate->setGeometry(QRect(390, 100, 51, 22));
        label_step_min = new QLabel(groupBox_paras);
        label_step_min->setObjectName(QString::fromUtf8("label_step_min"));
        label_step_min->setGeometry(QRect(240, 130, 161, 21));
        lineEdit_step_min = new QLineEdit(groupBox_paras);
        lineEdit_step_min->setObjectName(QString::fromUtf8("lineEdit_step_min"));
        lineEdit_step_min->setGeometry(QRect(390, 130, 51, 22));

        retranslateUi(Paradialog_rigidaffine);
        QObject::connect(buttonBox, SIGNAL(accepted()), Paradialog_rigidaffine, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Paradialog_rigidaffine, SLOT(reject()));

        QMetaObject::connectSlotsByName(Paradialog_rigidaffine);
    } // setupUi

    void retranslateUi(QDialog *Paradialog_rigidaffine)
    {
        Paradialog_rigidaffine->setWindowTitle(QApplication::translate("Paradialog_rigidaffine", "Dialog", 0, QApplication::UnicodeUTF8));
        groupBox_fileio->setTitle(QApplication::translate("Paradialog_rigidaffine", "File IO: ", 0, QApplication::UnicodeUTF8));
        label_img_tar->setText(QApplication::translate("Paradialog_rigidaffine", "Input target/template image: ", 0, QApplication::UnicodeUTF8));
        label_img_sub->setText(QApplication::translate("Paradialog_rigidaffine", "Input subject image: (to be warpped)", 0, QApplication::UnicodeUTF8));
        pushButton_img_tar->setText(QApplication::translate("Paradialog_rigidaffine", "...", 0, QApplication::UnicodeUTF8));
        pushButton_img_sub->setText(QApplication::translate("Paradialog_rigidaffine", "...", 0, QApplication::UnicodeUTF8));
        label_img_sub2tar->setText(QApplication::translate("Paradialog_rigidaffine", "Output warpped subject image: ", 0, QApplication::UnicodeUTF8));
        pushButton_img_sub2tar->setText(QApplication::translate("Paradialog_rigidaffine", "...", 0, QApplication::UnicodeUTF8));
        pushButton_swc_grid->setText(QApplication::translate("Paradialog_rigidaffine", "...", 0, QApplication::UnicodeUTF8));
        label_swc_grid->setText(QApplication::translate("Paradialog_rigidaffine", "Output warpped grid to swc: ", 0, QApplication::UnicodeUTF8));
        lineEdit_swc_grid->setText(QString());
        groupBox_paras->setTitle(QApplication::translate("Paradialog_rigidaffine", "Paras:", 0, QApplication::UnicodeUTF8));
        label_refchannel->setText(QApplication::translate("Paradialog_rigidaffine", "Reference channel:\n"
"(R:1, G:2, B:3)", 0, QApplication::UnicodeUTF8));
        label_downsample->setText(QApplication::translate("Paradialog_rigidaffine", "Downsample ratio:", 0, QApplication::UnicodeUTF8));
        groupBox_Gaussian->setTitle(QApplication::translate("Paradialog_rigidaffine", "Gaussian smoothing", 0, QApplication::UnicodeUTF8));
        label_Gau_sigma->setText(QApplication::translate("Paradialog_rigidaffine", "kernel sigma: ", 0, QApplication::UnicodeUTF8));
        label_Gau_radius->setText(QApplication::translate("Paradialog_rigidaffine", "kernel radius:", 0, QApplication::UnicodeUTF8));
        checkBox_histogrammatching->setText(QApplication::translate("Paradialog_rigidaffine", "Histogram matching", 0, QApplication::UnicodeUTF8));
        checkBox_alignedges->setText(QApplication::translate("Paradialog_rigidaffine", "Align edges", 0, QApplication::UnicodeUTF8));
        label_iter_max->setText(QApplication::translate("Paradialog_rigidaffine", "Max iter num:", 0, QApplication::UnicodeUTF8));
        label_step_multiplyfactor->setText(QApplication::translate("Paradialog_rigidaffine", "Step inimultiplyfactor:", 0, QApplication::UnicodeUTF8));
        label_step_annealingrate->setText(QApplication::translate("Paradialog_rigidaffine", "Step annealing rate:", 0, QApplication::UnicodeUTF8));
        label_step_min->setText(QApplication::translate("Paradialog_rigidaffine", "Min step (stop iter):", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Paradialog_rigidaffine: public Ui_Paradialog_rigidaffine {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARADIALOG_RIGIDAFFINE_H
