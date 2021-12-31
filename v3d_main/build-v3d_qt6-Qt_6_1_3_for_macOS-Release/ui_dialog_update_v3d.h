/********************************************************************************
** Form generated from reading UI file 'dialog_update_v3d.ui'
**
** Created by: Qt User Interface Compiler version 6.1.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_UPDATE_V3D_H
#define UI_DIALOG_UPDATE_V3D_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_dialog_update_v3d
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label_2;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *dialog_update_v3d)
    {
        if (dialog_update_v3d->objectName().isEmpty())
            dialog_update_v3d->setObjectName(QString::fromUtf8("dialog_update_v3d"));
        dialog_update_v3d->resize(373, 80);
        dialog_update_v3d->setModal(false);
        verticalLayout = new QVBoxLayout(dialog_update_v3d);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label_2 = new QLabel(dialog_update_v3d);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout->addWidget(label_2);

        buttonBox = new QDialogButtonBox(dialog_update_v3d);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(dialog_update_v3d);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog_update_v3d, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog_update_v3d, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(dialog_update_v3d);
    } // setupUi

    void retranslateUi(QDialog *dialog_update_v3d)
    {
        dialog_update_v3d->setWindowTitle(QCoreApplication::translate("dialog_update_v3d", "Check for new V3D software", nullptr));
        label_2->setText(QCoreApplication::translate("dialog_update_v3d", "Do you want to check for Vaa3D software updates?", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dialog_update_v3d: public Ui_dialog_update_v3d {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_UPDATE_V3D_H
