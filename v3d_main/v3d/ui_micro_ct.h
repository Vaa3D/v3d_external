/********************************************************************************
** Form generated from reading UI file 'micro_ct.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MICRO_CT_H
#define UI_MICRO_CT_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>
#include "../neuron_annotator/microCT/SingleCut.h"

QT_BEGIN_NAMESPACE

class Ui_CutPlannerDialog
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QDoubleSpinBox *micrometersBox;
    QSpacerItem *horizontalSpacer;
    QPushButton *labelBrainCompartmentsButton;
    SingleCut *basePlaneWidget;
    SingleCut *topCutWidget;
    SingleCut *frontCutWidget;
    SingleCut *rightCutWidget;
    SingleCut *rearCutWidget;
    SingleCut *leftCutWidget;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *savePlanButton;

    void setupUi(QDialog *CutPlannerDialog)
    {
        if (CutPlannerDialog->objectName().isEmpty())
            CutPlannerDialog->setObjectName(QString::fromUtf8("CutPlannerDialog"));
        CutPlannerDialog->resize(618, 418);
        verticalLayout = new QVBoxLayout(CutPlannerDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        widget = new QWidget(CutPlannerDialog);
        widget->setObjectName(QString::fromUtf8("widget"));
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        micrometersBox = new QDoubleSpinBox(widget);
        micrometersBox->setObjectName(QString::fromUtf8("micrometersBox"));
        micrometersBox->setMinimum(0.1);
        micrometersBox->setMaximum(10);
        micrometersBox->setSingleStep(0.05);
        micrometersBox->setValue(0.7);

        horizontalLayout->addWidget(micrometersBox);

        horizontalSpacer = new QSpacerItem(468, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        labelBrainCompartmentsButton = new QPushButton(widget);
        labelBrainCompartmentsButton->setObjectName(QString::fromUtf8("labelBrainCompartmentsButton"));

        horizontalLayout->addWidget(labelBrainCompartmentsButton);


        verticalLayout->addWidget(widget);

        basePlaneWidget = new SingleCut(CutPlannerDialog);
        basePlaneWidget->setObjectName(QString::fromUtf8("basePlaneWidget"));
        basePlaneWidget->setMinimumSize(QSize(600, 46));

        verticalLayout->addWidget(basePlaneWidget);

        topCutWidget = new SingleCut(CutPlannerDialog);
        topCutWidget->setObjectName(QString::fromUtf8("topCutWidget"));
        topCutWidget->setMinimumSize(QSize(600, 46));

        verticalLayout->addWidget(topCutWidget);

        frontCutWidget = new SingleCut(CutPlannerDialog);
        frontCutWidget->setObjectName(QString::fromUtf8("frontCutWidget"));
        frontCutWidget->setMinimumSize(QSize(600, 46));

        verticalLayout->addWidget(frontCutWidget);

        rightCutWidget = new SingleCut(CutPlannerDialog);
        rightCutWidget->setObjectName(QString::fromUtf8("rightCutWidget"));
        rightCutWidget->setMinimumSize(QSize(600, 46));

        verticalLayout->addWidget(rightCutWidget);

        rearCutWidget = new SingleCut(CutPlannerDialog);
        rearCutWidget->setObjectName(QString::fromUtf8("rearCutWidget"));
        rearCutWidget->setMinimumSize(QSize(600, 46));

        verticalLayout->addWidget(rearCutWidget);

        leftCutWidget = new SingleCut(CutPlannerDialog);
        leftCutWidget->setObjectName(QString::fromUtf8("leftCutWidget"));
        leftCutWidget->setMinimumSize(QSize(600, 46));

        verticalLayout->addWidget(leftCutWidget);

        widget_2 = new QWidget(CutPlannerDialog);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        horizontalLayout_2 = new QHBoxLayout(widget_2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer_2 = new QSpacerItem(498, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        savePlanButton = new QPushButton(widget_2);
        savePlanButton->setObjectName(QString::fromUtf8("savePlanButton"));
        savePlanButton->setAutoDefault(false);

        horizontalLayout_2->addWidget(savePlanButton);


        verticalLayout->addWidget(widget_2);


        retranslateUi(CutPlannerDialog);

        QMetaObject::connectSlotsByName(CutPlannerDialog);
    } // setupUi

    void retranslateUi(QDialog *CutPlannerDialog)
    {
        CutPlannerDialog->setWindowTitle(QApplication::translate("CutPlannerDialog", "microCT Trim Planner", 0));
        label->setText(QApplication::translate("CutPlannerDialog", "Voxel size:", 0));
        micrometersBox->setSuffix(QApplication::translate("CutPlannerDialog", "\302\265m", 0));
        labelBrainCompartmentsButton->setText(QApplication::translate("CutPlannerDialog", "Label Neuropil Compartments", 0));
        savePlanButton->setText(QApplication::translate("CutPlannerDialog", "Save File...", 0));
    } // retranslateUi

};

namespace Ui {
    class CutPlannerDialog: public Ui_CutPlannerDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MICRO_CT_H
