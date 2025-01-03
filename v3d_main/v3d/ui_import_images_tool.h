/********************************************************************************
** Form generated from reading UI file 'import_images_tool.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMPORT_IMAGES_TOOL_H
#define UI_IMPORT_IMAGES_TOOL_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

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
        okButton->setDefault(true);

        horizontalLayout->addWidget(okButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(import_images_tool);
        QObject::connect(okButton, SIGNAL(clicked()), import_images_tool, SLOT(accept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), import_images_tool, SLOT(reject()));

        QMetaObject::connectSlotsByName(import_images_tool);
    } // setupUi

    void retranslateUi(QDialog *import_images_tool)
    {
        import_images_tool->setWindowTitle(QApplication::translate("import_images_tool", "Image Sequence", 0));
        startimgButton->setText(QApplication::translate("import_images_tool", "Start Image", 0));
        numimgButton->setText(QApplication::translate("import_images_tool", "# files in the same folder", 0));
        incButton->setText(QApplication::translate("import_images_tool", "Increment", 0));
        filterButton->setText(QApplication::translate("import_images_tool", "File Name Contains", 0));
        startimgButton_2->setText(QApplication::translate("import_images_tool", "End Image", 0));
        label->setText(QApplication::translate("import_images_tool", "info: the Names of the filtered images \n"
"are sorted numerically.", 0));
        cancelButton->setText(QApplication::translate("import_images_tool", "Cancel", 0));
        okButton->setText(QApplication::translate("import_images_tool", "OK", 0));
    } // retranslateUi

};

namespace Ui {
    class import_images_tool: public Ui_import_images_tool {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMPORT_IMAGES_TOOL_H
