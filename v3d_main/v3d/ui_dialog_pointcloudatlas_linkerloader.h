/********************************************************************************
** Form generated from reading UI file 'dialog_pointcloudatlas_linkerloader.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_POINTCLOUDATLAS_LINKERLOADER_H
#define UI_DIALOG_POINTCLOUDATLAS_LINKERLOADER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Dialog_pointcloudatlas_linkerloader
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QFrame *line_2;
    QLabel *label_classname;
    QLineEdit *lineEdit_category_name;
    QLabel *label_info_num_apo_files;
    QLabel *label_sigclass;
    QLineEdit *lineEdit_anofolder_sig;
    QPushButton *pushButton_sig_folder;
    QLineEdit *lineEdit_anofolder_sig_nfiles;
    QLabel *label_refclass;
    QLineEdit *lineEdit_anofolder_ref;
    QPushButton *pushButton_ref_folder;
    QLineEdit *lineEdit_anofolder_ref_nfiles;
    QLabel *label_output_file;
    QLineEdit *lineEdit_output_file;
    QPushButton *pushButton_output_file;
    QPushButton *pushButton_common_ref_file;
    QLineEdit *lineEdit_targetref_file;
    QLabel *label_common_targetref;
    QLabel *label_common_referencemarker;
    QLineEdit *lineEdit_common_referencemarker;
    QFrame *line_3;
    QLabel *label_info_existing_classes;
    QFrame *line;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QPushButton *pushButton_cancel;
    QPushButton *pushButton_finish;
    QPushButton *pushButton_addanother;

    void setupUi(QDialog *Dialog_pointcloudatlas_linkerloader)
    {
        if (Dialog_pointcloudatlas_linkerloader->objectName().isEmpty())
            Dialog_pointcloudatlas_linkerloader->setObjectName(QString::fromUtf8("Dialog_pointcloudatlas_linkerloader"));
        Dialog_pointcloudatlas_linkerloader->resize(905, 389);
        Dialog_pointcloudatlas_linkerloader->setMinimumSize(QSize(772, 190));
        Dialog_pointcloudatlas_linkerloader->setMaximumSize(QSize(1200, 400));
        layoutWidget = new QWidget(Dialog_pointcloudatlas_linkerloader);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 10, 878, 366));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        line_2 = new QFrame(layoutWidget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_2, 3, 0, 1, 4);

        label_classname = new QLabel(layoutWidget);
        label_classname->setObjectName(QString::fromUtf8("label_classname"));

        gridLayout->addWidget(label_classname, 4, 0, 1, 1);

        lineEdit_category_name = new QLineEdit(layoutWidget);
        lineEdit_category_name->setObjectName(QString::fromUtf8("lineEdit_category_name"));

        gridLayout->addWidget(lineEdit_category_name, 4, 1, 1, 1);

        label_info_num_apo_files = new QLabel(layoutWidget);
        label_info_num_apo_files->setObjectName(QString::fromUtf8("label_info_num_apo_files"));

        gridLayout->addWidget(label_info_num_apo_files, 4, 3, 1, 1);

        label_sigclass = new QLabel(layoutWidget);
        label_sigclass->setObjectName(QString::fromUtf8("label_sigclass"));

        gridLayout->addWidget(label_sigclass, 5, 0, 1, 1);

        lineEdit_anofolder_sig = new QLineEdit(layoutWidget);
        lineEdit_anofolder_sig->setObjectName(QString::fromUtf8("lineEdit_anofolder_sig"));

        gridLayout->addWidget(lineEdit_anofolder_sig, 5, 1, 1, 1);

        pushButton_sig_folder = new QPushButton(layoutWidget);
        pushButton_sig_folder->setObjectName(QString::fromUtf8("pushButton_sig_folder"));

        gridLayout->addWidget(pushButton_sig_folder, 5, 2, 1, 1);

        lineEdit_anofolder_sig_nfiles = new QLineEdit(layoutWidget);
        lineEdit_anofolder_sig_nfiles->setObjectName(QString::fromUtf8("lineEdit_anofolder_sig_nfiles"));
        lineEdit_anofolder_sig_nfiles->setLayoutDirection(Qt::LeftToRight);

        gridLayout->addWidget(lineEdit_anofolder_sig_nfiles, 5, 3, 1, 1);

        label_refclass = new QLabel(layoutWidget);
        label_refclass->setObjectName(QString::fromUtf8("label_refclass"));

        gridLayout->addWidget(label_refclass, 6, 0, 1, 1);

        lineEdit_anofolder_ref = new QLineEdit(layoutWidget);
        lineEdit_anofolder_ref->setObjectName(QString::fromUtf8("lineEdit_anofolder_ref"));

        gridLayout->addWidget(lineEdit_anofolder_ref, 6, 1, 1, 1);

        pushButton_ref_folder = new QPushButton(layoutWidget);
        pushButton_ref_folder->setObjectName(QString::fromUtf8("pushButton_ref_folder"));

        gridLayout->addWidget(pushButton_ref_folder, 6, 2, 1, 1);

        lineEdit_anofolder_ref_nfiles = new QLineEdit(layoutWidget);
        lineEdit_anofolder_ref_nfiles->setObjectName(QString::fromUtf8("lineEdit_anofolder_ref_nfiles"));
        lineEdit_anofolder_ref_nfiles->setLayoutDirection(Qt::LeftToRight);

        gridLayout->addWidget(lineEdit_anofolder_ref_nfiles, 6, 3, 1, 1);

        label_output_file = new QLabel(layoutWidget);
        label_output_file->setObjectName(QString::fromUtf8("label_output_file"));

        gridLayout->addWidget(label_output_file, 7, 0, 1, 1);

        lineEdit_output_file = new QLineEdit(layoutWidget);
        lineEdit_output_file->setObjectName(QString::fromUtf8("lineEdit_output_file"));

        gridLayout->addWidget(lineEdit_output_file, 7, 1, 1, 1);

        pushButton_output_file = new QPushButton(layoutWidget);
        pushButton_output_file->setObjectName(QString::fromUtf8("pushButton_output_file"));

        gridLayout->addWidget(pushButton_output_file, 7, 2, 1, 1);

        pushButton_common_ref_file = new QPushButton(layoutWidget);
        pushButton_common_ref_file->setObjectName(QString::fromUtf8("pushButton_common_ref_file"));

        gridLayout->addWidget(pushButton_common_ref_file, 2, 2, 1, 1);

        lineEdit_targetref_file = new QLineEdit(layoutWidget);
        lineEdit_targetref_file->setObjectName(QString::fromUtf8("lineEdit_targetref_file"));

        gridLayout->addWidget(lineEdit_targetref_file, 2, 1, 1, 1);

        label_common_targetref = new QLabel(layoutWidget);
        label_common_targetref->setObjectName(QString::fromUtf8("label_common_targetref"));

        gridLayout->addWidget(label_common_targetref, 2, 0, 1, 1);

        label_common_referencemarker = new QLabel(layoutWidget);
        label_common_referencemarker->setObjectName(QString::fromUtf8("label_common_referencemarker"));

        gridLayout->addWidget(label_common_referencemarker, 1, 0, 1, 1);

        lineEdit_common_referencemarker = new QLineEdit(layoutWidget);
        lineEdit_common_referencemarker->setObjectName(QString::fromUtf8("lineEdit_common_referencemarker"));

        gridLayout->addWidget(lineEdit_common_referencemarker, 1, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        line_3 = new QFrame(layoutWidget);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_3);

        label_info_existing_classes = new QLabel(layoutWidget);
        label_info_existing_classes->setObjectName(QString::fromUtf8("label_info_existing_classes"));

        verticalLayout->addWidget(label_info_existing_classes);

        line = new QFrame(layoutWidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        pushButton_cancel = new QPushButton(layoutWidget);
        pushButton_cancel->setObjectName(QString::fromUtf8("pushButton_cancel"));

        horizontalLayout->addWidget(pushButton_cancel);

        pushButton_finish = new QPushButton(layoutWidget);
        pushButton_finish->setObjectName(QString::fromUtf8("pushButton_finish"));
        pushButton_finish->setDefault(true);

        horizontalLayout->addWidget(pushButton_finish);

        pushButton_addanother = new QPushButton(layoutWidget);
        pushButton_addanother->setObjectName(QString::fromUtf8("pushButton_addanother"));

        horizontalLayout->addWidget(pushButton_addanother);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(Dialog_pointcloudatlas_linkerloader);

        QMetaObject::connectSlotsByName(Dialog_pointcloudatlas_linkerloader);
    } // setupUi

    void retranslateUi(QDialog *Dialog_pointcloudatlas_linkerloader)
    {
        Dialog_pointcloudatlas_linkerloader->setWindowTitle(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Pointcloud atlas linker file editor", 0, QApplication::UnicodeUTF8));
        label_classname->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Signal name (e.g. gene name PS1)", 0, QApplication::UnicodeUTF8));
        label_info_num_apo_files->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "# of valid pointcloud files", 0, QApplication::UnicodeUTF8));
        label_sigclass->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Signal channel folder", 0, QApplication::UnicodeUTF8));
        pushButton_sig_folder->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Folder ...", 0, QApplication::UnicodeUTF8));
        label_refclass->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Reference channel folder", 0, QApplication::UnicodeUTF8));
        pushButton_ref_folder->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Folder ...", 0, QApplication::UnicodeUTF8));
        label_output_file->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Output pointcloud file of this signal", 0, QApplication::UnicodeUTF8));
        pushButton_output_file->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "File...", 0, QApplication::UnicodeUTF8));
        pushButton_common_ref_file->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "File...", 0, QApplication::UnicodeUTF8));
        label_common_targetref->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Common target pointcloud reference file", 0, QApplication::UnicodeUTF8));
        label_common_referencemarker->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Common reference (e.g. gene name EVE)", 0, QApplication::UnicodeUTF8));
        label_info_existing_classes->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Existing signals are: ", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Tip: If any texts above become red color, you need to enter valid file/folder info. \n"
" If the common target pointcloud ref file is left empty, then define target automatically.", 0, QApplication::UnicodeUTF8));
        pushButton_cancel->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Cancel", 0, QApplication::UnicodeUTF8));
        pushButton_finish->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Finish", 0, QApplication::UnicodeUTF8));
        pushButton_addanother->setText(QApplication::translate("Dialog_pointcloudatlas_linkerloader", "Add a new signal", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog_pointcloudatlas_linkerloader: public Ui_Dialog_pointcloudatlas_linkerloader {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_POINTCLOUDATLAS_LINKERLOADER_H
