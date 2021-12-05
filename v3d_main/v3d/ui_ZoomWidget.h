/********************************************************************************
** Form generated from reading UI file 'ZoomWidget.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ZOOMWIDGET_H
#define UI_ZOOMWIDGET_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDial>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSpacerItem>
#include <QWidget>
#include "../neuron_annotator/gui/ZoomSpinBox.h"

QT_BEGIN_NAMESPACE

class Ui_ZoomWidget
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    ZoomSpinBox *zoomSpinBox;
    QDial *dial;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *ZoomWidget)
    {
        if (ZoomWidget->objectName().isEmpty())
            ZoomWidget->setObjectName(QString::fromUtf8("ZoomWidget"));
        ZoomWidget->resize(149, 35);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ZoomWidget->sizePolicy().hasHeightForWidth());
        ZoomWidget->setSizePolicy(sizePolicy);
        ZoomWidget->setMinimumSize(QSize(0, 0));
        ZoomWidget->setMaximumSize(QSize(16777215, 35));
        horizontalLayout = new QHBoxLayout(ZoomWidget);
        horizontalLayout->setSpacing(3);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(ZoomWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        zoomSpinBox = new ZoomSpinBox(ZoomWidget);
        zoomSpinBox->setObjectName(QString::fromUtf8("zoomSpinBox"));
        zoomSpinBox->setSingleStep(5);

        horizontalLayout->addWidget(zoomSpinBox);

        dial = new QDial(ZoomWidget);
        dial->setObjectName(QString::fromUtf8("dial"));
        dial->setMinimumSize(QSize(35, 35));
        dial->setSingleStep(5);
        dial->setPageStep(20);
        dial->setWrapping(true);

        horizontalLayout->addWidget(dial);

        horizontalSpacer = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        retranslateUi(ZoomWidget);

        QMetaObject::connectSlotsByName(ZoomWidget);
    } // setupUi

    void retranslateUi(QWidget *ZoomWidget)
    {
        ZoomWidget->setWindowTitle(QApplication::translate("ZoomWidget", "Form", 0));
        label->setText(QApplication::translate("ZoomWidget", "Zoom", 0));
    } // retranslateUi

};

namespace Ui {
    class ZoomWidget: public Ui_ZoomWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ZOOMWIDGET_H
