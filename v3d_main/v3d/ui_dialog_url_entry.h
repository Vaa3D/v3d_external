/********************************************************************************
** Form generated from reading UI file 'dialog_url_entry.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_URL_ENTRY_H
#define UI_DIALOG_URL_ENTRY_H

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
#include <QtGui/QLineEdit>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_V3dUrlDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QLineEdit *lineEdit;
    QCheckBox *checkBox;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *V3dUrlDialog)
    {
        if (V3dUrlDialog->objectName().isEmpty())
            V3dUrlDialog->setObjectName(QString::fromUtf8("V3dUrlDialog"));
        V3dUrlDialog->resize(443, 137);
        verticalLayout = new QVBoxLayout(V3dUrlDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(V3dUrlDialog);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(V3dUrlDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        lineEdit = new QLineEdit(V3dUrlDialog);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lineEdit->sizePolicy().hasHeightForWidth());
        lineEdit->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(lineEdit);


        verticalLayout->addLayout(horizontalLayout);

        checkBox = new QCheckBox(V3dUrlDialog);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setChecked(true);

        verticalLayout->addWidget(checkBox);

        buttonBox = new QDialogButtonBox(V3dUrlDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Open);

        verticalLayout->addWidget(buttonBox);

#ifndef QT_NO_SHORTCUT
        label->setBuddy(lineEdit);
        label_2->setBuddy(lineEdit);
#endif // QT_NO_SHORTCUT

        retranslateUi(V3dUrlDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), V3dUrlDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), V3dUrlDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(V3dUrlDialog);
    } // setupUi

    void retranslateUi(QDialog *V3dUrlDialog)
    {
        V3dUrlDialog->setWindowTitle(QApplication::translate("V3dUrlDialog", "Load image stack from web URL", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("V3dUrlDialog", "Please type or paste URL of image stack below", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("V3dUrlDialog", "&URL:", 0, QApplication::UnicodeUTF8));
        lineEdit->setText(QApplication::translate("V3dUrlDialog", "http://home.penglab.com/proj/vaa3d/data_v1.0/ex_Repo_hb9_eve.tif", 0, QApplication::UnicodeUTF8));
        checkBox->setText(QApplication::translate("V3dUrlDialog", "Keep local copy of file", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class V3dUrlDialog: public Ui_V3dUrlDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_URL_ENTRY_H
