/********************************************************************************
** Form generated from reading UI file 'DynamicRangeTool.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DYNAMICRANGETOOL_H
#define UI_DYNAMICRANGETOOL_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include "../neuron_annotator/gui/ColorRangeSliderWidget.h"

QT_BEGIN_NAMESPACE

class Ui_DynamicRangeTool
{
public:
    QVBoxLayout *verticalLayout;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *comboBox;
    QSpacerItem *horizontalSpacer;
    QFrame *colorFrame;
    QPushButton *changeColorButton;
    QFrame *histogramFrame;
    QVBoxLayout *verticalLayout_2;
    QFrame *frame_4;
    ColorRangeSliderWidget *colorRangeSlider;
    QFrame *frame_3;
    QHBoxLayout *horizontalLayout_2;
    QToolButton *toolButton;
    QSpinBox *channelMin_spinBox;
    QSpacerItem *horizontalSpacer_2;
    QDoubleSpinBox *channelGamma_doubleSpinBox;
    QSpacerItem *horizontalSpacer_3;
    QToolButton *toolButton_2;
    QSpinBox *channelMax_spinBox;
    QFrame *frame_2;
    QHBoxLayout *horizontalLayout_3;
    QToolButton *toolButton_5;
    QToolButton *toolButton_4;
    QToolButton *toolButton_3;
    QSpacerItem *horizontalSpacer_4;
    QLabel *label_2;
    QSpinBox *spinBox_2;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DynamicRangeTool)
    {
        if (DynamicRangeTool->objectName().isEmpty())
            DynamicRangeTool->setObjectName(QString::fromUtf8("DynamicRangeTool"));
        DynamicRangeTool->resize(723, 472);
        verticalLayout = new QVBoxLayout(DynamicRangeTool);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        frame = new QFrame(DynamicRangeTool);
        frame->setObjectName(QString::fromUtf8("frame"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy);
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(frame);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        comboBox = new QComboBox(frame);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));

        horizontalLayout->addWidget(comboBox);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        colorFrame = new QFrame(frame);
        colorFrame->setObjectName(QString::fromUtf8("colorFrame"));
        colorFrame->setMinimumSize(QSize(40, 40));
        colorFrame->setAutoFillBackground(true);
        colorFrame->setStyleSheet(QString::fromUtf8(""));
        colorFrame->setFrameShape(QFrame::StyledPanel);
        colorFrame->setFrameShadow(QFrame::Plain);
        colorFrame->setLineWidth(4);

        horizontalLayout->addWidget(colorFrame);

        changeColorButton = new QPushButton(frame);
        changeColorButton->setObjectName(QString::fromUtf8("changeColorButton"));

        horizontalLayout->addWidget(changeColorButton);


        verticalLayout->addWidget(frame);

        histogramFrame = new QFrame(DynamicRangeTool);
        histogramFrame->setObjectName(QString::fromUtf8("histogramFrame"));
        histogramFrame->setMinimumSize(QSize(0, 50));
        histogramFrame->setFrameShape(QFrame::StyledPanel);
        histogramFrame->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(histogramFrame);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        frame_4 = new QFrame(histogramFrame);
        frame_4->setObjectName(QString::fromUtf8("frame_4"));
        frame_4->setFrameShape(QFrame::StyledPanel);
        frame_4->setFrameShadow(QFrame::Raised);

        verticalLayout_2->addWidget(frame_4);

        colorRangeSlider = new ColorRangeSliderWidget(histogramFrame);
        colorRangeSlider->setObjectName(QString::fromUtf8("colorRangeSlider"));
        colorRangeSlider->setMaximumSize(QSize(16777215, 15));

        verticalLayout_2->addWidget(colorRangeSlider);


        verticalLayout->addWidget(histogramFrame);

        frame_3 = new QFrame(DynamicRangeTool);
        frame_3->setObjectName(QString::fromUtf8("frame_3"));
        sizePolicy.setHeightForWidth(frame_3->sizePolicy().hasHeightForWidth());
        frame_3->setSizePolicy(sizePolicy);
        frame_3->setFrameShape(QFrame::StyledPanel);
        frame_3->setFrameShadow(QFrame::Raised);
        horizontalLayout_2 = new QHBoxLayout(frame_3);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        toolButton = new QToolButton(frame_3);
        toolButton->setObjectName(QString::fromUtf8("toolButton"));
        toolButton->setEnabled(false);

        horizontalLayout_2->addWidget(toolButton);

        channelMin_spinBox = new QSpinBox(frame_3);
        channelMin_spinBox->setObjectName(QString::fromUtf8("channelMin_spinBox"));

        horizontalLayout_2->addWidget(channelMin_spinBox);

        horizontalSpacer_2 = new QSpacerItem(187, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        channelGamma_doubleSpinBox = new QDoubleSpinBox(frame_3);
        channelGamma_doubleSpinBox->setObjectName(QString::fromUtf8("channelGamma_doubleSpinBox"));
        channelGamma_doubleSpinBox->setMinimum(0.12);
        channelGamma_doubleSpinBox->setMaximum(8);
        channelGamma_doubleSpinBox->setSingleStep(0.05);
        channelGamma_doubleSpinBox->setValue(1);

        horizontalLayout_2->addWidget(channelGamma_doubleSpinBox);

        horizontalSpacer_3 = new QSpacerItem(187, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);

        toolButton_2 = new QToolButton(frame_3);
        toolButton_2->setObjectName(QString::fromUtf8("toolButton_2"));
        toolButton_2->setEnabled(false);

        horizontalLayout_2->addWidget(toolButton_2);

        channelMax_spinBox = new QSpinBox(frame_3);
        channelMax_spinBox->setObjectName(QString::fromUtf8("channelMax_spinBox"));

        horizontalLayout_2->addWidget(channelMax_spinBox);


        verticalLayout->addWidget(frame_3);

        frame_2 = new QFrame(DynamicRangeTool);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        sizePolicy.setHeightForWidth(frame_2->sizePolicy().hasHeightForWidth());
        frame_2->setSizePolicy(sizePolicy);
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        horizontalLayout_3 = new QHBoxLayout(frame_2);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        toolButton_5 = new QToolButton(frame_2);
        toolButton_5->setObjectName(QString::fromUtf8("toolButton_5"));
        toolButton_5->setEnabled(false);

        horizontalLayout_3->addWidget(toolButton_5);

        toolButton_4 = new QToolButton(frame_2);
        toolButton_4->setObjectName(QString::fromUtf8("toolButton_4"));
        toolButton_4->setEnabled(false);

        horizontalLayout_3->addWidget(toolButton_4);

        toolButton_3 = new QToolButton(frame_2);
        toolButton_3->setObjectName(QString::fromUtf8("toolButton_3"));
        toolButton_3->setEnabled(false);

        horizontalLayout_3->addWidget(toolButton_3);

        horizontalSpacer_4 = new QSpacerItem(360, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_4);

        label_2 = new QLabel(frame_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy);

        horizontalLayout_3->addWidget(label_2);

        spinBox_2 = new QSpinBox(frame_2);
        spinBox_2->setObjectName(QString::fromUtf8("spinBox_2"));
        spinBox_2->setMinimum(1);
        spinBox_2->setValue(5);

        horizontalLayout_3->addWidget(spinBox_2);


        verticalLayout->addWidget(frame_2);

        buttonBox = new QDialogButtonBox(DynamicRangeTool);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::Reset);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(DynamicRangeTool);
        QObject::connect(buttonBox, SIGNAL(accepted()), DynamicRangeTool, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DynamicRangeTool, SLOT(reject()));

        QMetaObject::connectSlotsByName(DynamicRangeTool);
    } // setupUi

    void retranslateUi(QDialog *DynamicRangeTool)
    {
        DynamicRangeTool->setWindowTitle(QApplication::translate("DynamicRangeTool", "Data color tool", 0));
        label->setText(QApplication::translate("DynamicRangeTool", "Channel:", 0));
        comboBox->clear();
        comboBox->insertItems(0, QStringList()
         << QApplication::translate("DynamicRangeTool", "All Channels", 0)
         << QApplication::translate("DynamicRangeTool", "Channel 1", 0)
        );
        changeColorButton->setText(QApplication::translate("DynamicRangeTool", "Change Color", 0));
#ifndef QT_NO_TOOLTIP
        colorRangeSlider->setToolTip(QApplication::translate("DynamicRangeTool", "drag handles to adjust color range", 0));
#endif // QT_NO_TOOLTIP
        toolButton->setText(QApplication::translate("DynamicRangeTool", "...", 0));
#ifndef QT_NO_TOOLTIP
        channelMin_spinBox->setToolTip(QApplication::translate("DynamicRangeTool", "HDR minimum data value", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        channelGamma_doubleSpinBox->setToolTip(QApplication::translate("DynamicRangeTool", "gamma brightness", 0));
#endif // QT_NO_TOOLTIP
        toolButton_2->setText(QApplication::translate("DynamicRangeTool", "...", 0));
#ifndef QT_NO_TOOLTIP
        channelMax_spinBox->setToolTip(QApplication::translate("DynamicRangeTool", "HDR maximum data value", 0));
#endif // QT_NO_TOOLTIP
        toolButton_5->setText(QApplication::translate("DynamicRangeTool", "...", 0));
        toolButton_4->setText(QApplication::translate("DynamicRangeTool", "...", 0));
        toolButton_3->setText(QApplication::translate("DynamicRangeTool", "...", 0));
        label_2->setText(QApplication::translate("DynamicRangeTool", "Max Transparency", 0));
#ifndef QT_NO_TOOLTIP
        spinBox_2->setToolTip(QApplication::translate("DynamicRangeTool", "Thickness in micrometers required to obscure half the background", 0));
#endif // QT_NO_TOOLTIP
        spinBox_2->setSuffix(QApplication::translate("DynamicRangeTool", " um", 0));
    } // retranslateUi

};

namespace Ui {
    class DynamicRangeTool: public Ui_DynamicRangeTool {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DYNAMICRANGETOOL_H
