/********************************************************************************
** Form generated from reading UI file 'import_images_tool.ui'
**
** Created by: Qt User Interface Compiler version 6.1.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMPORT_IMAGES_TOOL_H
#define UI_IMPORT_IMAGES_TOOL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_import_images_tool
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QSpinBox *startimgBox;
    QSpinBox *numimgBox;
    QPushButton *startimgButton;
    QPushButton *numimgButton;
    QPushButton *incButton;
    QSpinBox *incBox;
    QPushButton *filterButton;
    QLineEdit *filterEdit;
    QComboBox *comboPack;
    QPushButton *startimgButton_2;
    QSpinBox *endimgBox;
    QLabel *label;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancelButton;
    QPushButton *okButton;

    void setupUi(QDialog *import_images_tool)
    {
        if (import_images_tool->objectName().isEmpty())
            import_images_tool->setObjectName(QString::fromUtf8("import_images_tool"));
        import_images_tool->resize(317, 362);
        layoutWidget = new QWidget(import_images_tool);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(21, 21, 276, 321));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        startimgBox = new QSpinBox(layoutWidget);
        startimgBox->setObjectName(QString::fromUtf8("startimgBox"));

        gridLayout->addWidget(startimgBox, 1, 1, 1, 1);

        numimgBox = new QSpinBox(layoutWidget);
        numimgBox->setObjectName(QString::fromUtf8("numimgBox"));

        gridLayout->addWidget(numimgBox, 0, 1, 1, 1);

        startimgButton = new QPushButton(layoutWidget);
        startimgButton->setObjectName(QString::fromUtf8("startimgButton"));

        gridLayout->addWidget(startimgButton, 1, 0, 1, 1);

        numimgButton = new QPushButton(layoutWidget);
        numimgButton->setObjectName(QString::fromUtf8("numimgButton"));

        gridLayout->addWidget(numimgButton, 0, 0, 1, 1);

        incButton = new QPushButton(layoutWidget);
        incButton->setObjectName(QString::fromUtf8("incButton"));

        gridLayout->addWidget(incButton, 3, 0, 1, 1);

        incBox = new QSpinBox(layoutWidget);
        incBox->setObjectName(QString::fromUtf8("incBox"));

        gridLayout->addWidget(incBox, 3, 1, 1, 1);

        filterButton = new QPushButton(layoutWidget);
        filterButton->setObjectName(QString::fromUtf8("filterButton"));

        gridLayout->addWidget(filterButton, 4, 0, 1, 1);

        filterEdit = new QLineEdit(layoutWidget);
        filterEdit->setObjectName(QString::fromUtf8("filterEdit"));

        gridLayout->addWidget(filterEdit, 4, 1, 1, 1);

        comboPack = new QComboBox(layoutWidget);
        comboPack->setObjectName(QString::fromUtf8("comboPack"));

        gridLayout->addWidget(comboPack, 5, 0, 1, 2);

        startimgButton_2 = new QPushButton(layoutWidget);
        startimgButton_2->setObjectName(QString::fromUtf8("startimgButton_2"));

        gridLayout->addWidget(startimgButton_2, 2, 0, 1, 1);

        endimgBox = new QSpinBox(layoutWidget);
        endimgBox->setObjectName(QString::fromUtf8("endimgBox"));

        gridLayout->addWidget(endimgBox, 2, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        cancelButton = new QPushButton(layoutWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout->addWidget(cancelButton);

        okButton = new QPushButton(layoutWidget);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        horizontalLayout->addWidget(okButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(import_images_tool);
        QObject::connect(okButton, &QPushButton::clicked, import_images_tool, qOverload<>(&QDialog::accept));
        QObject::connect(cancelButton, &QPushButton::clicked, import_images_tool, qOverload<>(&QDialog::reject));

        okButton->setDefault(true);


        QMetaObject::connectSlotsByName(import_images_tool);
    } // setupUi

    void retranslateUi(QDialog *import_images_tool)
    {
        import_images_tool->setWindowTitle(QCoreApplication::translate("import_images_tool", "Image Sequence", nullptr));
        startimgButton->setText(QCoreApplication::translate("import_images_tool", "Start Image", nullptr));
        numimgButton->setText(QCoreApplication::translate("import_images_tool", "# files in the same folder", nullptr));
        incButton->setText(QCoreApplication::translate("import_images_tool", "Increment", nullptr));
        filterButton->setText(QCoreApplication::translate("import_images_tool", "File Name Contains", nullptr));
        startimgButton_2->setText(QCoreApplication::translate("import_images_tool", "End Image", nullptr));
        label->setText(QCoreApplication::translate("import_images_tool", "info: the Names of the filtered images \n"
"are sorted numerically.", nullptr));
        cancelButton->setText(QCoreApplication::translate("import_images_tool", "Cancel", nullptr));
        okButton->setText(QCoreApplication::translate("import_images_tool", "OK", nullptr));
    } // retranslateUi

};

namespace Ui {
    class import_images_tool: public Ui_import_images_tool {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMPORT_IMAGES_TOOL_H
