/********************************************************************************
** Form generated from reading UI file 'dialog_maskroi.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_MASKROI_H
#define UI_DIALOG_MASKROI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Dialog_MaskROI
{
public:
    QFrame *frame;
    QWidget *layoutWidget;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *maskregionBox;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_2;
    QLabel *label_7;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_3;
    QSpinBox *firstchannelBox;
    QLabel *label_4;
    QSpinBox *lastchannelBox;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_5;
    QSpinBox *fillingvalueBox;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_6;
    QComboBox *fillingroiBox;
    QLabel *infolabel;
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancelButton;
    QPushButton *okButton;

    void setupUi(QDialog *Dialog_MaskROI)
    {
        if (Dialog_MaskROI->objectName().isEmpty())
            Dialog_MaskROI->setObjectName(QString::fromUtf8("Dialog_MaskROI"));
        Dialog_MaskROI->resize(397, 331);
        frame = new QFrame(Dialog_MaskROI);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setGeometry(QRect(20, 10, 351, 201));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        layoutWidget = new QWidget(Dialog_MaskROI);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(30, 20, 332, 172));
        gridLayout = new QGridLayout(layoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        maskregionBox = new QComboBox(layoutWidget);
        maskregionBox->setObjectName(QString::fromUtf8("maskregionBox"));

        horizontalLayout->addWidget(maskregionBox);


        gridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_6->addWidget(label_2);

        label_7 = new QLabel(layoutWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_6->addWidget(label_7);


        gridLayout->addLayout(horizontalLayout_6, 1, 0, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_3 = new QLabel(layoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_5->addWidget(label_3);

        firstchannelBox = new QSpinBox(layoutWidget);
        firstchannelBox->setObjectName(QString::fromUtf8("firstchannelBox"));

        horizontalLayout_5->addWidget(firstchannelBox);

        label_4 = new QLabel(layoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_5->addWidget(label_4);

        lastchannelBox = new QSpinBox(layoutWidget);
        lastchannelBox->setObjectName(QString::fromUtf8("lastchannelBox"));

        horizontalLayout_5->addWidget(lastchannelBox);


        gridLayout->addLayout(horizontalLayout_5, 2, 0, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_5 = new QLabel(layoutWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_4->addWidget(label_5);

        fillingvalueBox = new QSpinBox(layoutWidget);
        fillingvalueBox->setObjectName(QString::fromUtf8("fillingvalueBox"));

        horizontalLayout_4->addWidget(fillingvalueBox);


        gridLayout->addLayout(horizontalLayout_4, 3, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_6 = new QLabel(layoutWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_3->addWidget(label_6);

        fillingroiBox = new QComboBox(layoutWidget);
        fillingroiBox->setObjectName(QString::fromUtf8("fillingroiBox"));

        horizontalLayout_3->addWidget(fillingroiBox);


        gridLayout->addLayout(horizontalLayout_3, 4, 0, 1, 1);

        infolabel = new QLabel(Dialog_MaskROI);
        infolabel->setObjectName(QString::fromUtf8("infolabel"));
        infolabel->setGeometry(QRect(20, 280, 332, 18));
        layoutWidget1 = new QWidget(Dialog_MaskROI);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(30, 224, 332, 32));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget1);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        cancelButton = new QPushButton(layoutWidget1);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout_2->addWidget(cancelButton);

        okButton = new QPushButton(layoutWidget1);
        okButton->setObjectName(QString::fromUtf8("okButton"));
        okButton->setDefault(true);

        horizontalLayout_2->addWidget(okButton);


        retranslateUi(Dialog_MaskROI);

        QMetaObject::connectSlotsByName(Dialog_MaskROI);
    } // setupUi

    void retranslateUi(QDialog *Dialog_MaskROI)
    {
        Dialog_MaskROI->setWindowTitle(QApplication::translate("Dialog_MaskROI", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Dialog_MaskROI", "Mask Region :", 0, QApplication::UnicodeUTF8));
        maskregionBox->clear();
        maskregionBox->insertItems(0, QStringList()
         << QApplication::translate("Dialog_MaskROI", "XYZ cube by ROI intersection", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_MaskROI", "XYZ cube by ROI union", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_MaskROI", "XY Plane only", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_MaskROI", "YZ Plane only", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_MaskROI", "ZX Plane only", 0, QApplication::UnicodeUTF8)
        );
        label_2->setText(QApplication::translate("Dialog_MaskROI", "Mask Channels :", 0, QApplication::UnicodeUTF8));
        label_7->setText(QString());
        label_3->setText(QApplication::translate("Dialog_MaskROI", "first channel :", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("Dialog_MaskROI", "last channel :", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("Dialog_MaskROI", "filling value :", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("Dialog_MaskROI", "filling in or outside ROI :", 0, QApplication::UnicodeUTF8));
        fillingroiBox->clear();
        fillingroiBox->insertItems(0, QStringList()
         << QApplication::translate("Dialog_MaskROI", "Outside ROI", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_MaskROI", "Inside ROI", 0, QApplication::UnicodeUTF8)
        );
        infolabel->setText(QApplication::translate("Dialog_MaskROI", "info: Mask ROI Options", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("Dialog_MaskROI", "Cancel", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("Dialog_MaskROI", "OK", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog_MaskROI: public Ui_Dialog_MaskROI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_MASKROI_H
