/********************************************************************************
** Form generated from reading UI file 'import_filelist_dialog.ui'
**
** Created by: Qt User Interface Compiler version 6.1.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMPORT_FILELIST_DIALOG_H
#define UI_IMPORT_FILELIST_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_import_filelist_tool
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *vboxLayout;
    QGridLayout *gridLayout;
    QPushButton *filterButton;
    QLineEdit *filterEdit;
    QPushButton *numimgButton;
    QSpinBox *numimgBox;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *cancelButton;
    QPushButton *okButton;

    void setupUi(QDialog *import_filelist_tool)
    {
        if (import_filelist_tool->objectName().isEmpty())
            import_filelist_tool->setObjectName(QString::fromUtf8("import_filelist_tool"));
        import_filelist_tool->resize(371, 163);
        layoutWidget = new QWidget(import_filelist_tool);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(30, 21, 318, 117));
        vboxLayout = new QVBoxLayout(layoutWidget);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        filterButton = new QPushButton(layoutWidget);
        filterButton->setObjectName(QString::fromUtf8("filterButton"));

        gridLayout->addWidget(filterButton, 0, 0, 1, 1);

        filterEdit = new QLineEdit(layoutWidget);
        filterEdit->setObjectName(QString::fromUtf8("filterEdit"));

        gridLayout->addWidget(filterEdit, 0, 1, 1, 1);

        numimgButton = new QPushButton(layoutWidget);
        numimgButton->setObjectName(QString::fromUtf8("numimgButton"));

        gridLayout->addWidget(numimgButton, 1, 0, 1, 1);

        numimgBox = new QSpinBox(layoutWidget);
        numimgBox->setObjectName(QString::fromUtf8("numimgBox"));

        gridLayout->addWidget(numimgBox, 1, 1, 1, 1);


        vboxLayout->addLayout(gridLayout);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        cancelButton = new QPushButton(layoutWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        hboxLayout->addWidget(cancelButton);

        okButton = new QPushButton(layoutWidget);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        hboxLayout->addWidget(okButton);


        vboxLayout->addLayout(hboxLayout);


        retranslateUi(import_filelist_tool);
        QObject::connect(okButton, &QPushButton::clicked, import_filelist_tool, qOverload<>(&QDialog::accept));
        QObject::connect(cancelButton, &QPushButton::clicked, import_filelist_tool, qOverload<>(&QDialog::reject));

        okButton->setDefault(true);


        QMetaObject::connectSlotsByName(import_filelist_tool);
    } // setupUi

    void retranslateUi(QDialog *import_filelist_tool)
    {
        import_filelist_tool->setWindowTitle(QCoreApplication::translate("import_filelist_tool", "Image/file sequence importer", nullptr));
        filterButton->setText(QCoreApplication::translate("import_filelist_tool", "File name filter", nullptr));
        numimgButton->setText(QCoreApplication::translate("import_filelist_tool", "# files-filtered in this folder", nullptr));
        cancelButton->setText(QCoreApplication::translate("import_filelist_tool", "Cancel", nullptr));
        okButton->setText(QCoreApplication::translate("import_filelist_tool", "OK", nullptr));
    } // retranslateUi

};

namespace Ui {
    class import_filelist_tool: public Ui_import_filelist_tool {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMPORT_FILELIST_DIALOG_H
