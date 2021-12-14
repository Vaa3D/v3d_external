/********************************************************************************
** Form generated from reading UI file 'dialog_update_downloading.ui'
**
** Created by: Qt User Interface Compiler version 6.2.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_UPDATE_DOWNLOADING_H
#define UI_DIALOG_UPDATE_DOWNLOADING_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_downloading_updates_dialog
{
public:
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_2;
    QLabel *iconLabel;
    QSpacerItem *verticalSpacer;
    QVBoxLayout *verticalLayout;
    QLabel *textLabel;
    QLabel *informativeTextLabel;
    QProgressBar *progressBar;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *downloading_updates_dialog)
    {
        if (downloading_updates_dialog->objectName().isEmpty())
            downloading_updates_dialog->setObjectName(QString::fromUtf8("downloading_updates_dialog"));
        downloading_updates_dialog->resize(450, 151);
        verticalLayout_3 = new QVBoxLayout(downloading_updates_dialog);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        iconLabel = new QLabel(downloading_updates_dialog);
        iconLabel->setObjectName(QString::fromUtf8("iconLabel"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(iconLabel->sizePolicy().hasHeightForWidth());
        iconLabel->setSizePolicy(sizePolicy);
        iconLabel->setMinimumSize(QSize(75, 75));
        iconLabel->setMaximumSize(QSize(75, 75));
        iconLabel->setBaseSize(QSize(75, 75));

        verticalLayout_2->addWidget(iconLabel);

        verticalSpacer = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);


        horizontalLayout->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        textLabel = new QLabel(downloading_updates_dialog);
        textLabel->setObjectName(QString::fromUtf8("textLabel"));

        verticalLayout->addWidget(textLabel);

        informativeTextLabel = new QLabel(downloading_updates_dialog);
        informativeTextLabel->setObjectName(QString::fromUtf8("informativeTextLabel"));

        verticalLayout->addWidget(informativeTextLabel);

        progressBar = new QProgressBar(downloading_updates_dialog);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setMaximum(0);
        progressBar->setValue(0);

        verticalLayout->addWidget(progressBar);


        horizontalLayout->addLayout(verticalLayout);


        verticalLayout_3->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(downloading_updates_dialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel);

        verticalLayout_3->addWidget(buttonBox);


        retranslateUi(downloading_updates_dialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, downloading_updates_dialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, downloading_updates_dialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(downloading_updates_dialog);
    } // setupUi

    void retranslateUi(QDialog *downloading_updates_dialog)
    {
        downloading_updates_dialog->setWindowTitle(QCoreApplication::translate("downloading_updates_dialog", "V3D software update", nullptr));
        iconLabel->setText(QString());
        textLabel->setText(QCoreApplication::translate("downloading_updates_dialog", "V3D is downloading updates", nullptr));
        informativeTextLabel->setText(QCoreApplication::translate("downloading_updates_dialog", "Examining files...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class downloading_updates_dialog: public Ui_downloading_updates_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_UPDATE_DOWNLOADING_H
