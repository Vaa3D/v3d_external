/********************************************************************************
** Form generated from reading UI file 'GammaWidget.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GAMMAWIDGET_H
#define UI_GAMMAWIDGET_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QWidget>

QT_BEGIN_NAMESPACE

class Ui_GammaWidget
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *gamma_label;
    QSlider *gamma_slider;
    QLineEdit *gamma_lineEdit;

    void setupUi(QWidget *GammaWidget)
    {
        if (GammaWidget->objectName().isEmpty())
            GammaWidget->setObjectName(QString::fromUtf8("GammaWidget"));
        GammaWidget->resize(181, 46);
        horizontalLayout = new QHBoxLayout(GammaWidget);
        horizontalLayout->setSpacing(2);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        gamma_label = new QLabel(GammaWidget);
        gamma_label->setObjectName(QString::fromUtf8("gamma_label"));

        horizontalLayout->addWidget(gamma_label);

        gamma_slider = new QSlider(GammaWidget);
        gamma_slider->setObjectName(QString::fromUtf8("gamma_slider"));
        gamma_slider->setFocusPolicy(Qt::StrongFocus);
        gamma_slider->setMinimum(-200);
        gamma_slider->setMaximum(200);
        gamma_slider->setSingleStep(5);
        gamma_slider->setPageStep(25);
        gamma_slider->setOrientation(Qt::Horizontal);
        gamma_slider->setTickPosition(QSlider::TicksBelow);
        gamma_slider->setTickInterval(100);

        horizontalLayout->addWidget(gamma_slider);

        gamma_lineEdit = new QLineEdit(GammaWidget);
        gamma_lineEdit->setObjectName(QString::fromUtf8("gamma_lineEdit"));
        gamma_lineEdit->setMaximumSize(QSize(35, 16777215));

        horizontalLayout->addWidget(gamma_lineEdit);


        retranslateUi(GammaWidget);

        QMetaObject::connectSlotsByName(GammaWidget);
    } // setupUi

    void retranslateUi(QWidget *GammaWidget)
    {
        GammaWidget->setWindowTitle(QApplication::translate("GammaWidget", "Form", 0));
#ifndef QT_NO_TOOLTIP
        gamma_label->setToolTip(QApplication::translate("GammaWidget", "Gamma (\316\223) brightness", 0));
#endif // QT_NO_TOOLTIP
        gamma_label->setText(QApplication::translate("GammaWidget", "\316\223 ", 0));
#ifndef QT_NO_TOOLTIP
        gamma_slider->setToolTip(QApplication::translate("GammaWidget", "Drag to adjust gamma brightness", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        gamma_lineEdit->setToolTip(QApplication::translate("GammaWidget", "Gamma brightness: Larger values are darker.", 0));
#endif // QT_NO_TOOLTIP
        gamma_lineEdit->setText(QApplication::translate("GammaWidget", "1.00", 0));
    } // retranslateUi

};

namespace Ui {
    class GammaWidget: public Ui_GammaWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GAMMAWIDGET_H
