/********************************************************************************
** Form generated from reading UI file 'dialog_vano_linkerloader.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_VANO_LINKERLOADER_H
#define UI_DIALOG_VANO_LINKERLOADER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Dialog_vano_linkerloader
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *vboxLayout;
    QGridLayout *gridLayout;
    QLabel *label_rawimg;
    QLineEdit *lineEdit_rawimg;
    QPushButton *pushButton_rawimg;
    QLabel *label_maskimg;
    QLineEdit *lineEdit_maskimg;
    QPushButton *pushButton_maskimg;
    QLabel *label_apofile;
    QLineEdit *lineEdit_apofile;
    QPushButton *pushButton_apofile;
    QHBoxLayout *hboxLayout;
    QLabel *label;
    QSpacerItem *spacerItem;
    QPushButton *pushButton_cancel;
    QPushButton *pushButton_loaddata;

    void setupUi(QDialog *Dialog_vano_linkerloader)
    {
        if (Dialog_vano_linkerloader->objectName().isEmpty())
            Dialog_vano_linkerloader->setObjectName(QString::fromUtf8("Dialog_vano_linkerloader"));
        Dialog_vano_linkerloader->resize(772, 190);
        Dialog_vano_linkerloader->setMinimumSize(QSize(772, 190));
        Dialog_vano_linkerloader->setMaximumSize(QSize(772, 190));
        layoutWidget = new QWidget(Dialog_vano_linkerloader);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(22, 20, 731, 146));
        vboxLayout = new QVBoxLayout(layoutWidget);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_rawimg = new QLabel(layoutWidget);
        label_rawimg->setObjectName(QString::fromUtf8("label_rawimg"));

        gridLayout->addWidget(label_rawimg, 0, 0, 1, 1);

        lineEdit_rawimg = new QLineEdit(layoutWidget);
        lineEdit_rawimg->setObjectName(QString::fromUtf8("lineEdit_rawimg"));

        gridLayout->addWidget(lineEdit_rawimg, 0, 1, 1, 1);

        pushButton_rawimg = new QPushButton(layoutWidget);
        pushButton_rawimg->setObjectName(QString::fromUtf8("pushButton_rawimg"));

        gridLayout->addWidget(pushButton_rawimg, 0, 2, 1, 1);

        label_maskimg = new QLabel(layoutWidget);
        label_maskimg->setObjectName(QString::fromUtf8("label_maskimg"));

        gridLayout->addWidget(label_maskimg, 1, 0, 1, 1);

        lineEdit_maskimg = new QLineEdit(layoutWidget);
        lineEdit_maskimg->setObjectName(QString::fromUtf8("lineEdit_maskimg"));

        gridLayout->addWidget(lineEdit_maskimg, 1, 1, 1, 1);

        pushButton_maskimg = new QPushButton(layoutWidget);
        pushButton_maskimg->setObjectName(QString::fromUtf8("pushButton_maskimg"));

        gridLayout->addWidget(pushButton_maskimg, 1, 2, 1, 1);

        label_apofile = new QLabel(layoutWidget);
        label_apofile->setObjectName(QString::fromUtf8("label_apofile"));

        gridLayout->addWidget(label_apofile, 2, 0, 1, 1);

        lineEdit_apofile = new QLineEdit(layoutWidget);
        lineEdit_apofile->setObjectName(QString::fromUtf8("lineEdit_apofile"));

        gridLayout->addWidget(lineEdit_apofile, 2, 1, 1, 1);

        pushButton_apofile = new QPushButton(layoutWidget);
        pushButton_apofile->setObjectName(QString::fromUtf8("pushButton_apofile"));

        gridLayout->addWidget(pushButton_apofile, 2, 2, 1, 1);


        vboxLayout->addLayout(gridLayout);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        hboxLayout->addWidget(label);

        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        pushButton_cancel = new QPushButton(layoutWidget);
        pushButton_cancel->setObjectName(QString::fromUtf8("pushButton_cancel"));

        hboxLayout->addWidget(pushButton_cancel);

        pushButton_loaddata = new QPushButton(layoutWidget);
        pushButton_loaddata->setObjectName(QString::fromUtf8("pushButton_loaddata"));

        hboxLayout->addWidget(pushButton_loaddata);


        vboxLayout->addLayout(hboxLayout);


        retranslateUi(Dialog_vano_linkerloader);

        QMetaObject::connectSlotsByName(Dialog_vano_linkerloader);
    } // setupUi

    void retranslateUi(QDialog *Dialog_vano_linkerloader)
    {
        Dialog_vano_linkerloader->setWindowTitle(QApplication::translate("Dialog_vano_linkerloader", "Linker file editor", 0, QApplication::UnicodeUTF8));
        label_rawimg->setText(QApplication::translate("Dialog_vano_linkerloader", "Raw Image", 0, QApplication::UnicodeUTF8));
        pushButton_rawimg->setText(QApplication::translate("Dialog_vano_linkerloader", "File ...", 0, QApplication::UnicodeUTF8));
        label_maskimg->setText(QApplication::translate("Dialog_vano_linkerloader", "Mask Image", 0, QApplication::UnicodeUTF8));
        pushButton_maskimg->setText(QApplication::translate("Dialog_vano_linkerloader", "File ...", 0, QApplication::UnicodeUTF8));
        label_apofile->setText(QApplication::translate("Dialog_vano_linkerloader", "Annotation file", 0, QApplication::UnicodeUTF8));
        pushButton_apofile->setText(QApplication::translate("Dialog_vano_linkerloader", "File ...", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Dialog_vano_linkerloader", "Tip: if any texts above become red color, the files do NOT exist!", 0, QApplication::UnicodeUTF8));
        pushButton_cancel->setText(QApplication::translate("Dialog_vano_linkerloader", "Cancel", 0, QApplication::UnicodeUTF8));
        pushButton_loaddata->setText(QApplication::translate("Dialog_vano_linkerloader", "Load data", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog_vano_linkerloader: public Ui_Dialog_vano_linkerloader {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_VANO_LINKERLOADER_H
