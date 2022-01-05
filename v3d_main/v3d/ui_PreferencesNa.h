/********************************************************************************
** Form generated from reading UI file 'PreferencesNa.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PREFERENCESNA_H
#define UI_PREFERENCESNA_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PreferencesDialog
{
public:
    QVBoxLayout *verticalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QSpinBox *maxVideoMemBox;
    QWidget *fastLoadWidget;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *fastLoadCheckBox;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *srgbCheckBox;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *PreferencesDialog)
    {
        if (PreferencesDialog->objectName().isEmpty())
            PreferencesDialog->setObjectName(QString::fromUtf8("PreferencesDialog"));
        PreferencesDialog->resize(291, 218);
        verticalLayout_3 = new QVBoxLayout(PreferencesDialog);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        widget = new QWidget(PreferencesDialog);
        widget->setObjectName(QString::fromUtf8("widget"));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        maxVideoMemBox = new QSpinBox(widget);
        maxVideoMemBox->setObjectName(QString::fromUtf8("maxVideoMemBox"));
        maxVideoMemBox->setMinimum(1);
        maxVideoMemBox->setMaximum(100000);
        maxVideoMemBox->setSingleStep(100);
        maxVideoMemBox->setValue(350);

        horizontalLayout->addWidget(maxVideoMemBox);


        verticalLayout->addLayout(horizontalLayout);

        fastLoadWidget = new QWidget(widget);
        fastLoadWidget->setObjectName(QString::fromUtf8("fastLoadWidget"));
        horizontalLayout_2 = new QHBoxLayout(fastLoadWidget);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        fastLoadCheckBox = new QCheckBox(fastLoadWidget);
        fastLoadCheckBox->setObjectName(QString::fromUtf8("fastLoadCheckBox"));
        fastLoadCheckBox->setChecked(true);

        horizontalLayout_2->addWidget(fastLoadCheckBox);


        verticalLayout->addWidget(fastLoadWidget);

        widget_2 = new QWidget(widget);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        horizontalLayout_3 = new QHBoxLayout(widget_2);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        srgbCheckBox = new QCheckBox(widget_2);
        srgbCheckBox->setObjectName(QString::fromUtf8("srgbCheckBox"));

        horizontalLayout_3->addWidget(srgbCheckBox);


        verticalLayout->addWidget(widget_2);


        verticalLayout_2->addWidget(widget);

        buttonBox = new QDialogButtonBox(PreferencesDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_2->addWidget(buttonBox);


        verticalLayout_3->addLayout(verticalLayout_2);

#ifndef QT_NO_SHORTCUT
        label->setBuddy(maxVideoMemBox);
#endif // QT_NO_SHORTCUT

        retranslateUi(PreferencesDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), PreferencesDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), PreferencesDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(PreferencesDialog);
    } // setupUi

    void retranslateUi(QDialog *PreferencesDialog)
    {
        PreferencesDialog->setWindowTitle(QApplication::translate("PreferencesDialog", "NeuronAnnotator Preferences", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("PreferencesDialog", "Max video memory", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        maxVideoMemBox->setToolTip(QApplication::translate("PreferencesDialog", "How much of your video card memory to use", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        maxVideoMemBox->setSuffix(QApplication::translate("PreferencesDialog", " MB", 0, QApplication::UnicodeUTF8));
        maxVideoMemBox->setPrefix(QString());
        fastLoadCheckBox->setText(QApplication::translate("PreferencesDialog", "Use fast 3D loading", 0, QApplication::UnicodeUTF8));
        srgbCheckBox->setText(QApplication::translate("PreferencesDialog", "Use sRGB gamma correction", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class PreferencesDialog: public Ui_PreferencesDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PREFERENCESNA_H
