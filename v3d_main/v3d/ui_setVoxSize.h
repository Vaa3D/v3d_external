/********************************************************************************
** Form generated from reading UI file 'setVoxSize.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETVOXSIZE_H
#define UI_SETVOXSIZE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_setVoxSizeDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QDoubleSpinBox *doubleSpinBox;
    QSpacerItem *horizontalSpacer;
    QLabel *label_2;
    QDoubleSpinBox *doubleSpinBox_2;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label_3;
    QDoubleSpinBox *doubleSpinBox_3;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *setVoxSizeDialog)
    {
        if (setVoxSizeDialog->objectName().isEmpty())
            setVoxSizeDialog->setObjectName(QString::fromUtf8("setVoxSizeDialog"));
        setVoxSizeDialog->resize(389, 114);
        verticalLayout = new QVBoxLayout(setVoxSizeDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        frame = new QFrame(setVoxSizeDialog);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Plain);
        frame->setLineWidth(2);
        frame->setMidLineWidth(0);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(frame);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        doubleSpinBox = new QDoubleSpinBox(frame);
        doubleSpinBox->setObjectName(QString::fromUtf8("doubleSpinBox"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(doubleSpinBox->sizePolicy().hasHeightForWidth());
        doubleSpinBox->setSizePolicy(sizePolicy);
        doubleSpinBox->setMinimumSize(QSize(70, 0));
        doubleSpinBox->setDecimals(2);
        doubleSpinBox->setSingleStep(0.01);
        doubleSpinBox->setValue(0.2);

        horizontalLayout->addWidget(doubleSpinBox);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label_2 = new QLabel(frame);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        doubleSpinBox_2 = new QDoubleSpinBox(frame);
        doubleSpinBox_2->setObjectName(QString::fromUtf8("doubleSpinBox_2"));
        doubleSpinBox_2->setMinimumSize(QSize(70, 0));
        doubleSpinBox_2->setSingleStep(0.01);
        doubleSpinBox_2->setValue(0.2);

        horizontalLayout->addWidget(doubleSpinBox_2);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        label_3 = new QLabel(frame);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout->addWidget(label_3);

        doubleSpinBox_3 = new QDoubleSpinBox(frame);
        doubleSpinBox_3->setObjectName(QString::fromUtf8("doubleSpinBox_3"));
        doubleSpinBox_3->setMinimumSize(QSize(70, 0));
        doubleSpinBox_3->setSingleStep(0.01);
        doubleSpinBox_3->setValue(1);

        horizontalLayout->addWidget(doubleSpinBox_3);


        verticalLayout->addWidget(frame);

        buttonBox = new QDialogButtonBox(setVoxSizeDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(setVoxSizeDialog);
        QObject::connect(buttonBox, SIGNAL(rejected()), setVoxSizeDialog, SLOT(reject()));
        QObject::connect(buttonBox, SIGNAL(accepted()), setVoxSizeDialog, SLOT(accept()));

        QMetaObject::connectSlotsByName(setVoxSizeDialog);
    } // setupUi

    void retranslateUi(QDialog *setVoxSizeDialog)
    {
        setVoxSizeDialog->setWindowTitle(QApplication::translate("setVoxSizeDialog", "Set corresponding image voxelsize", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("setVoxSizeDialog", "X:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("setVoxSizeDialog", "Y:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("setVoxSizeDialog", "Z:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class setVoxSizeDialog: public Ui_setVoxSizeDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETVOXSIZE_H
