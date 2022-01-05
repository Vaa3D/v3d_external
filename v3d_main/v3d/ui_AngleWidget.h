/********************************************************************************
** Form generated from reading UI file 'AngleWidget.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ANGLEWIDGET_H
#define UI_ANGLEWIDGET_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDial>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSpinBox>
#include <QWidget>

QT_BEGIN_NAMESPACE

class Ui_AngleWidget
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QSpinBox *spinBox;
    QDial *dial;

    void setupUi(QWidget *AngleWidget)
    {
        if (AngleWidget->objectName().isEmpty())
            AngleWidget->setObjectName(QString::fromUtf8("AngleWidget"));
        AngleWidget->resize(142, 40);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(AngleWidget->sizePolicy().hasHeightForWidth());
        AngleWidget->setSizePolicy(sizePolicy);
        AngleWidget->setBaseSize(QSize(0, 40));
        AngleWidget->setFocusPolicy(Qt::NoFocus);
        horizontalLayout = new QHBoxLayout(AngleWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(AngleWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        spinBox = new QSpinBox(AngleWidget);
        spinBox->setObjectName(QString::fromUtf8("spinBox"));
        spinBox->setFocusPolicy(Qt::StrongFocus);
        spinBox->setWrapping(true);
        spinBox->setFrame(true);
        spinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        spinBox->setAccelerated(true);
        spinBox->setMaximum(359);
        spinBox->setSingleStep(1);

        horizontalLayout->addWidget(spinBox);

        dial = new QDial(AngleWidget);
        dial->setObjectName(QString::fromUtf8("dial"));
        dial->setMinimumSize(QSize(0, 0));
        dial->setMaximumSize(QSize(16777215, 40));
        dial->setBaseSize(QSize(0, 0));
        dial->setCursor(QCursor(Qt::ArrowCursor));
        dial->setFocusPolicy(Qt::StrongFocus);
        dial->setMaximum(359);
        dial->setSingleStep(15);
        dial->setPageStep(30);
        dial->setSliderPosition(0);
        dial->setOrientation(Qt::Horizontal);
        dial->setInvertedAppearance(true);
        dial->setInvertedControls(true);
        dial->setWrapping(true);
        dial->setNotchTarget(3.7);
        dial->setNotchesVisible(true);

        horizontalLayout->addWidget(dial);

#ifndef QT_NO_SHORTCUT
        label->setBuddy(spinBox);
#endif // QT_NO_SHORTCUT

        retranslateUi(AngleWidget);
        QObject::connect(dial, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
        QObject::connect(spinBox, SIGNAL(valueChanged(int)), dial, SLOT(setValue(int)));

        QMetaObject::connectSlotsByName(AngleWidget);
    } // setupUi

    void retranslateUi(QWidget *AngleWidget)
    {
        AngleWidget->setWindowTitle(QApplication::translate("AngleWidget", "Form", 0));
        label->setText(QApplication::translate("AngleWidget", "RotX ", 0));
#ifndef QT_NO_TOOLTIP
        spinBox->setToolTip(QApplication::translate("AngleWidget", "enter rotation angle in degrees", 0));
#endif // QT_NO_TOOLTIP
        spinBox->setSuffix(QApplication::translate("AngleWidget", "\302\260", 0));
        spinBox->setPrefix(QString());
#ifndef QT_NO_TOOLTIP
        dial->setToolTip(QApplication::translate("AngleWidget", "drag dial with mouse to rotate", 0));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class AngleWidget: public Ui_AngleWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ANGLEWIDGET_H
