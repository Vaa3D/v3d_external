/********************************************************************************
** Form generated from reading UI file 'dialog_imageresample.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_IMAGERESAMPLE_H
#define UI_DIALOG_IMAGERESAMPLE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Dialog_ImageResample
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label;
    QComboBox *axesBox;
    QLabel *label_3;
    QDoubleSpinBox *currentpixelsizeBox;
    QLabel *label_4;
    QDoubleSpinBox *targetpixelsizeBox;
    QLabel *label_2;
    QComboBox *interp_method_box;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancelButton;
    QPushButton *okButton;
    QLabel *infolabel;

    void setupUi(QDialog *Dialog_ImageResample)
    {
        if (Dialog_ImageResample->objectName().isEmpty())
            Dialog_ImageResample->setObjectName(QString::fromUtf8("Dialog_ImageResample"));
        Dialog_ImageResample->resize(569, 264);
        layoutWidget = new QWidget(Dialog_ImageResample);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(30, 20, 517, 202));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        axesBox = new QComboBox(layoutWidget);
        axesBox->setObjectName(QString::fromUtf8("axesBox"));

        gridLayout->addWidget(axesBox, 0, 1, 1, 1);

        label_3 = new QLabel(layoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        currentpixelsizeBox = new QDoubleSpinBox(layoutWidget);
        currentpixelsizeBox->setObjectName(QString::fromUtf8("currentpixelsizeBox"));
        currentpixelsizeBox->setDecimals(4);
        currentpixelsizeBox->setMaximum(1e+06);

        gridLayout->addWidget(currentpixelsizeBox, 1, 1, 1, 1);

        label_4 = new QLabel(layoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 2, 0, 1, 1);

        targetpixelsizeBox = new QDoubleSpinBox(layoutWidget);
        targetpixelsizeBox->setObjectName(QString::fromUtf8("targetpixelsizeBox"));
        targetpixelsizeBox->setDecimals(4);
        targetpixelsizeBox->setMaximum(1e+06);

        gridLayout->addWidget(targetpixelsizeBox, 2, 1, 1, 1);

        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 3, 0, 1, 1);

        interp_method_box = new QComboBox(layoutWidget);
        interp_method_box->setObjectName(QString::fromUtf8("interp_method_box"));

        gridLayout->addWidget(interp_method_box, 3, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        cancelButton = new QPushButton(layoutWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout_2->addWidget(cancelButton);

        okButton = new QPushButton(layoutWidget);
        okButton->setObjectName(QString::fromUtf8("okButton"));
        okButton->setDefault(true);

        horizontalLayout_2->addWidget(okButton);


        verticalLayout->addLayout(horizontalLayout_2);

        infolabel = new QLabel(layoutWidget);
        infolabel->setObjectName(QString::fromUtf8("infolabel"));

        verticalLayout->addWidget(infolabel);


        retranslateUi(Dialog_ImageResample);

        interp_method_box->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(Dialog_ImageResample);
    } // setupUi

    void retranslateUi(QDialog *Dialog_ImageResample)
    {
        Dialog_ImageResample->setWindowTitle(QApplication::translate("Dialog_ImageResample", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Dialog_ImageResample", "Axis/axes along which to resample image", 0, QApplication::UnicodeUTF8));
        axesBox->clear();
        axesBox->insertItems(0, QStringList()
         << QApplication::translate("Dialog_ImageResample", "Z axis", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_ImageResample", "XY axis (in proportion)", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_ImageResample", "XYZ axis (in proportion)", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_ImageResample", "X axis", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_ImageResample", "Y axis", 0, QApplication::UnicodeUTF8)
        );
        label_3->setText(QApplication::translate("Dialog_ImageResample", "current pixel size (um) :", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("Dialog_ImageResample", "target pixel size (um) :", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Dialog_ImageResample", "Voxel/pixel interpolation method:", 0, QApplication::UnicodeUTF8));
        interp_method_box->clear();
        interp_method_box->insertItems(0, QStringList()
         << QApplication::translate("Dialog_ImageResample", "nearest neighbor interpolation", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_ImageResample", "linear interpolation", 0, QApplication::UnicodeUTF8)
        );
        cancelButton->setText(QApplication::translate("Dialog_ImageResample", "Cancel", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("Dialog_ImageResample", "OK", 0, QApplication::UnicodeUTF8));
        infolabel->setText(QApplication::translate("Dialog_ImageResample", "info: Image Resampling Options", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog_ImageResample: public Ui_Dialog_ImageResample {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_IMAGERESAMPLE_H
