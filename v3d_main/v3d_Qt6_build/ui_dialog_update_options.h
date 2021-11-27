/********************************************************************************
** Form generated from reading UI file 'dialog_update_options.ui'
**
** Created by: Qt User Interface Compiler version 6.1.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_UPDATE_OPTIONS_H
#define UI_DIALOG_UPDATE_OPTIONS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_dialog_update_options
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label_3;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QLabel *label_2;
    QComboBox *comboBox;
    QSpacerItem *verticalSpacer_2;
    QLabel *label_4;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *useDefaultVersionButton;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *dialog_update_options)
    {
        if (dialog_update_options->objectName().isEmpty())
            dialog_update_options->setObjectName(QString::fromUtf8("dialog_update_options"));
        dialog_update_options->resize(666, 248);
        verticalLayout = new QVBoxLayout(dialog_update_options);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label_3 = new QLabel(dialog_update_options);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout->addWidget(label_3);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        label_2 = new QLabel(dialog_update_options);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        comboBox = new QComboBox(dialog_update_options);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName(QString::fromUtf8("comboBox"));

        horizontalLayout_2->addWidget(comboBox);


        verticalLayout->addLayout(horizontalLayout_2);

        verticalSpacer_2 = new QSpacerItem(20, 4, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        label_4 = new QLabel(dialog_update_options);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        verticalLayout->addWidget(label_4);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(dialog_update_options);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        lineEdit = new QLineEdit(dialog_update_options);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        horizontalLayout->addWidget(lineEdit);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);

        useDefaultVersionButton = new QPushButton(dialog_update_options);
        useDefaultVersionButton->setObjectName(QString::fromUtf8("useDefaultVersionButton"));

        horizontalLayout_3->addWidget(useDefaultVersionButton);


        verticalLayout->addLayout(horizontalLayout_3);

        verticalSpacer = new QSpacerItem(20, 4, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(dialog_update_options);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::Open|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        label_2->setBuddy(comboBox);
#endif // QT_CONFIG(shortcut)

        retranslateUi(dialog_update_options);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog_update_options, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog_update_options, qOverload<>(&QDialog::reject));

        comboBox->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(dialog_update_options);
    } // setupUi

    void retranslateUi(QDialog *dialog_update_options)
    {
        dialog_update_options->setWindowTitle(QCoreApplication::translate("dialog_update_options", "V3D updater options", nullptr));
        label_3->setText(QCoreApplication::translate("dialog_update_options", "How often do you want Vaa3D to automatically check for updates?", nullptr));
        label_2->setText(QCoreApplication::translate("dialog_update_options", "Check for &updates", nullptr));
        comboBox->setItemText(0, QCoreApplication::translate("dialog_update_options", "every time", nullptr));
        comboBox->setItemText(1, QCoreApplication::translate("dialog_update_options", "once a day", nullptr));
        comboBox->setItemText(2, QCoreApplication::translate("dialog_update_options", "once a week", nullptr));
        comboBox->setItemText(3, QCoreApplication::translate("dialog_update_options", "once a month", nullptr));
        comboBox->setItemText(4, QCoreApplication::translate("dialog_update_options", "never", nullptr));

        label_4->setText(QCoreApplication::translate("dialog_update_options", "Where do you want to get updates from?", nullptr));
        label->setText(QCoreApplication::translate("dialog_update_options", "Vaa3D version URL", nullptr));
        useDefaultVersionButton->setText(QCoreApplication::translate("dialog_update_options", "Use official Vaa3D version URL", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialog_update_options: public Ui_dialog_update_options {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_UPDATE_OPTIONS_H
