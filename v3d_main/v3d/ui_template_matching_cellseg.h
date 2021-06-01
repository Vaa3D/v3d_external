/********************************************************************************
** Form generated from reading UI file 'template_matching_cellseg.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEMPLATE_MATCHING_CELLSEG_H
#define UI_TEMPLATE_MATCHING_CELLSEG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Dialog_TemplateMatchingSegPara
{
public:
    QWidget *widget;
    QVBoxLayout *verticalLayout_4;
    QVBoxLayout *verticalLayout_3;
    QGridLayout *gridLayout;
    QLabel *label;
    QSpinBox *spinBox_Channel;
    QLabel *label_3;
    QComboBox *comboBox_Kernel;
    QVBoxLayout *verticalLayout;
    QLabel *label_5;
    QGridLayout *gridLayout1;
    QLabel *label_2;
    QSpinBox *spinBox_szx;
    QLabel *label_4;
    QSpinBox *spinBox_szy;
    QLabel *label_7;
    QSpinBox *spinBox_szz;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_10;
    QGridLayout *_2;
    QLabel *label_6;
    QLabel *label_8;
    QLabel *label_9;
    QDoubleSpinBox *doubleSpinBox_stdz;
    QDoubleSpinBox *doubleSpinBox_stdy;
    QDoubleSpinBox *doubleSpinBox_stdx;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QLabel *label_11;
    QDoubleSpinBox *doubleSpinBox_pixelval;
    QLabel *label_12;
    QDoubleSpinBox *doubleSpinBox_rgnval;
    QDoubleSpinBox *doubleSpinBox_corrcoef;
    QLabel *label_13;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_3;
    QLabel *label_14;
    QDoubleSpinBox *doubleSpinBox_rgn_merge_radius;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_cancel;
    QPushButton *pushButton_ok;

    void setupUi(QDialog *Dialog_TemplateMatchingSegPara)
    {
        if (Dialog_TemplateMatchingSegPara->objectName().isEmpty())
            Dialog_TemplateMatchingSegPara->setObjectName(QString::fromUtf8("Dialog_TemplateMatchingSegPara"));
        Dialog_TemplateMatchingSegPara->resize(298, 534);
        widget = new QWidget(Dialog_TemplateMatchingSegPara);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(20, 11, 260, 503));
        verticalLayout_4 = new QVBoxLayout(widget);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        spinBox_Channel = new QSpinBox(widget);
        spinBox_Channel->setObjectName(QString::fromUtf8("spinBox_Channel"));
        spinBox_Channel->setMinimum(1);

        gridLayout->addWidget(spinBox_Channel, 0, 1, 1, 1);

        label_3 = new QLabel(widget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        comboBox_Kernel = new QComboBox(widget);
        comboBox_Kernel->setObjectName(QString::fromUtf8("comboBox_Kernel"));

        gridLayout->addWidget(comboBox_Kernel, 1, 1, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label_5 = new QLabel(widget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_5);

        gridLayout1 = new QGridLayout();
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        label_2 = new QLabel(widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignCenter);

        gridLayout1->addWidget(label_2, 0, 0, 1, 1);

        spinBox_szx = new QSpinBox(widget);
        spinBox_szx->setObjectName(QString::fromUtf8("spinBox_szx"));
        spinBox_szx->setMinimum(1);
        spinBox_szx->setValue(5);

        gridLayout1->addWidget(spinBox_szx, 0, 1, 1, 1);

        label_4 = new QLabel(widget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout1->addWidget(label_4, 1, 0, 1, 1);

        spinBox_szy = new QSpinBox(widget);
        spinBox_szy->setObjectName(QString::fromUtf8("spinBox_szy"));
        spinBox_szy->setMinimum(1);
        spinBox_szy->setValue(5);

        gridLayout1->addWidget(spinBox_szy, 1, 1, 1, 1);

        label_7 = new QLabel(widget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setAlignment(Qt::AlignCenter);

        gridLayout1->addWidget(label_7, 2, 0, 1, 1);

        spinBox_szz = new QSpinBox(widget);
        spinBox_szz->setObjectName(QString::fromUtf8("spinBox_szz"));
        spinBox_szz->setMinimum(1);
        spinBox_szz->setValue(3);

        gridLayout1->addWidget(spinBox_szz, 2, 1, 1, 1);


        verticalLayout->addLayout(gridLayout1);


        gridLayout->addLayout(verticalLayout, 2, 0, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        label_10 = new QLabel(widget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(label_10);

        _2 = new QGridLayout();
        _2->setObjectName(QString::fromUtf8("_2"));
        label_6 = new QLabel(widget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setAlignment(Qt::AlignCenter);

        _2->addWidget(label_6, 0, 0, 1, 1);

        label_8 = new QLabel(widget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setAlignment(Qt::AlignCenter);

        _2->addWidget(label_8, 1, 0, 1, 1);

        label_9 = new QLabel(widget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setAlignment(Qt::AlignCenter);

        _2->addWidget(label_9, 2, 0, 1, 1);

        doubleSpinBox_stdz = new QDoubleSpinBox(widget);
        doubleSpinBox_stdz->setObjectName(QString::fromUtf8("doubleSpinBox_stdz"));
        doubleSpinBox_stdz->setMinimum(0.5);
        doubleSpinBox_stdz->setValue(1.5);

        _2->addWidget(doubleSpinBox_stdz, 2, 1, 1, 1);

        doubleSpinBox_stdy = new QDoubleSpinBox(widget);
        doubleSpinBox_stdy->setObjectName(QString::fromUtf8("doubleSpinBox_stdy"));
        doubleSpinBox_stdy->setMinimum(0.5);
        doubleSpinBox_stdy->setValue(3);

        _2->addWidget(doubleSpinBox_stdy, 1, 1, 1, 1);

        doubleSpinBox_stdx = new QDoubleSpinBox(widget);
        doubleSpinBox_stdx->setObjectName(QString::fromUtf8("doubleSpinBox_stdx"));
        doubleSpinBox_stdx->setMinimum(0.5);
        doubleSpinBox_stdx->setValue(3);

        _2->addWidget(doubleSpinBox_stdx, 0, 1, 1, 1);


        verticalLayout_2->addLayout(_2);


        gridLayout->addLayout(verticalLayout_2, 2, 1, 1, 1);


        verticalLayout_3->addLayout(gridLayout);

        groupBox = new QGroupBox(widget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_11 = new QLabel(groupBox);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout_2->addWidget(label_11, 0, 0, 1, 1);

        doubleSpinBox_pixelval = new QDoubleSpinBox(groupBox);
        doubleSpinBox_pixelval->setObjectName(QString::fromUtf8("doubleSpinBox_pixelval"));
        doubleSpinBox_pixelval->setMaximum(65535);
        doubleSpinBox_pixelval->setSingleStep(10);
        doubleSpinBox_pixelval->setValue(50);

        gridLayout_2->addWidget(doubleSpinBox_pixelval, 0, 1, 1, 1);

        label_12 = new QLabel(groupBox);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout_2->addWidget(label_12, 1, 0, 1, 1);

        doubleSpinBox_rgnval = new QDoubleSpinBox(groupBox);
        doubleSpinBox_rgnval->setObjectName(QString::fromUtf8("doubleSpinBox_rgnval"));
        doubleSpinBox_rgnval->setMaximum(65535);
        doubleSpinBox_rgnval->setSingleStep(10);
        doubleSpinBox_rgnval->setValue(40);

        gridLayout_2->addWidget(doubleSpinBox_rgnval, 1, 1, 1, 1);

        doubleSpinBox_corrcoef = new QDoubleSpinBox(groupBox);
        doubleSpinBox_corrcoef->setObjectName(QString::fromUtf8("doubleSpinBox_corrcoef"));
        doubleSpinBox_corrcoef->setMaximum(1);
        doubleSpinBox_corrcoef->setValue(0.5);

        gridLayout_2->addWidget(doubleSpinBox_corrcoef, 2, 1, 1, 1);

        label_13 = new QLabel(groupBox);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        gridLayout_2->addWidget(label_13, 2, 0, 1, 1);


        verticalLayout_3->addWidget(groupBox);


        verticalLayout_4->addLayout(verticalLayout_3);

        groupBox_2 = new QGroupBox(widget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_3 = new QGridLayout(groupBox_2);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        label_14 = new QLabel(groupBox_2);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        gridLayout_3->addWidget(label_14, 0, 0, 1, 1);

        doubleSpinBox_rgn_merge_radius = new QDoubleSpinBox(groupBox_2);
        doubleSpinBox_rgn_merge_radius->setObjectName(QString::fromUtf8("doubleSpinBox_rgn_merge_radius"));
        doubleSpinBox_rgn_merge_radius->setMaximum(65535);
        doubleSpinBox_rgn_merge_radius->setSingleStep(10);
        doubleSpinBox_rgn_merge_radius->setValue(20);

        gridLayout_3->addWidget(doubleSpinBox_rgn_merge_radius, 0, 1, 1, 1);


        verticalLayout_4->addWidget(groupBox_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton_cancel = new QPushButton(widget);
        pushButton_cancel->setObjectName(QString::fromUtf8("pushButton_cancel"));

        horizontalLayout->addWidget(pushButton_cancel);

        pushButton_ok = new QPushButton(widget);
        pushButton_ok->setObjectName(QString::fromUtf8("pushButton_ok"));
        pushButton_ok->setDefault(true);

        horizontalLayout->addWidget(pushButton_ok);


        verticalLayout_4->addLayout(horizontalLayout);


        retranslateUi(Dialog_TemplateMatchingSegPara);

        QMetaObject::connectSlotsByName(Dialog_TemplateMatchingSegPara);
    } // setupUi

    void retranslateUi(QDialog *Dialog_TemplateMatchingSegPara)
    {
        Dialog_TemplateMatchingSegPara->setWindowTitle(QApplication::translate("Dialog_TemplateMatchingSegPara", "Template matching segmentation parameters", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "Channel", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "Kernel (template)", 0, QApplication::UnicodeUTF8));
        comboBox_Kernel->clear();
        comboBox_Kernel->insertItems(0, QStringList()
         << QApplication::translate("Dialog_TemplateMatchingSegPara", "Gaussian", 0, QApplication::UnicodeUTF8)
        );
        label_5->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "Dimension \n"
"(radius, # pixels)", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "x", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "y", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "z", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "Standard Deviation", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "x", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "y", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "z", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("Dialog_TemplateMatchingSegPara", "Thresholds", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "Pixel value >=", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "Local region pixel \n"
"mean value >=", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "Correlation >=", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("Dialog_TemplateMatchingSegPara", "Other parameters", 0, QApplication::UnicodeUTF8));
        label_14->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "region merge radius", 0, QApplication::UnicodeUTF8));
        pushButton_cancel->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "Cancel", 0, QApplication::UnicodeUTF8));
        pushButton_ok->setText(QApplication::translate("Dialog_TemplateMatchingSegPara", "OK", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog_TemplateMatchingSegPara: public Ui_Dialog_TemplateMatchingSegPara {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEMPLATE_MATCHING_CELLSEG_H
