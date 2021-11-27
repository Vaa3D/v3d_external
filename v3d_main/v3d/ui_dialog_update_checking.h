/********************************************************************************
** Form generated from reading UI file 'dialog_update_checking.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_UPDATE_CHECKING_H
#define UI_DIALOG_UPDATE_CHECKING_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QProgressBar>
#include <QSpacerItem>
#include <QVBoxLayout>

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
        QObject::connect(buttonBox, SIGNAL(accepted()), checking_for_updates_dialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), checking_for_updates_dialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(checking_for_updates_dialog);
    } // setupUi

    void retranslateUi(QDialog *checking_for_updates_dialog)
    {
        checking_for_updates_dialog->setWindowTitle(QApplication::translate("checking_for_updates_dialog", "Dialog", 0));
        label->setText(QApplication::translate("checking_for_updates_dialog", "V3D is checking for updates", 0));
    } // retranslateUi

};

namespace Ui {
    class checking_for_updates_dialog: public Ui_checking_for_updates_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_UPDATE_CHECKING_H
