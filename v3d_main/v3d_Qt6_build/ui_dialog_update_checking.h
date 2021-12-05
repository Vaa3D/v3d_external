/********************************************************************************
** Form generated from reading UI file 'dialog_update_checking.ui'
**
** Created by: Qt User Interface Compiler version 6.1.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_UPDATE_CHECKING_H
#define UI_DIALOG_UPDATE_CHECKING_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_checking_for_updates_dialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QProgressBar *progressBar;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *checking_for_updates_dialog)
    {
        if (checking_for_updates_dialog->objectName().isEmpty())
            checking_for_updates_dialog->setObjectName(QString::fromUtf8("checking_for_updates_dialog"));
        checking_for_updates_dialog->resize(202, 114);
        verticalLayout = new QVBoxLayout(checking_for_updates_dialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(checking_for_updates_dialog);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        progressBar = new QProgressBar(checking_for_updates_dialog);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setMaximum(0);
        progressBar->setValue(0);

        verticalLayout->addWidget(progressBar);

        verticalSpacer = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(checking_for_updates_dialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(checking_for_updates_dialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, checking_for_updates_dialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, checking_for_updates_dialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(checking_for_updates_dialog);
    } // setupUi

    void retranslateUi(QDialog *checking_for_updates_dialog)
    {
        checking_for_updates_dialog->setWindowTitle(QCoreApplication::translate("checking_for_updates_dialog", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("checking_for_updates_dialog", "V3D is checking for updates", nullptr));
    } // retranslateUi

};

namespace Ui {
    class checking_for_updates_dialog: public Ui_checking_for_updates_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_UPDATE_CHECKING_H
