/********************************************************************************
** Form generated from reading UI file 'single_cut.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SINGLE_CUT_H
#define UI_SINGLE_CUT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SingleCut
{
public:
    QHBoxLayout *horizontalLayout;
    QPushButton *orientButton;
    QComboBox *axisBox;
    QLineEdit *nameField;
    QPushButton *edgeButton;
    QPushButton *cutButton;
    QLineEdit *cutDistanceLineEdit;
    QLabel *label;

    void setupUi(QWidget *SingleCut)
    {
        if (SingleCut->objectName().isEmpty())
            SingleCut->setObjectName(QString::fromUtf8("SingleCut"));
        SingleCut->resize(608, 46);
        horizontalLayout = new QHBoxLayout(SingleCut);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        orientButton = new QPushButton(SingleCut);
        orientButton->setObjectName(QString::fromUtf8("orientButton"));

        horizontalLayout->addWidget(orientButton);

        axisBox = new QComboBox(SingleCut);
        axisBox->setObjectName(QString::fromUtf8("axisBox"));

        horizontalLayout->addWidget(axisBox);

        nameField = new QLineEdit(SingleCut);
        nameField->setObjectName(QString::fromUtf8("nameField"));

        horizontalLayout->addWidget(nameField);

        edgeButton = new QPushButton(SingleCut);
        edgeButton->setObjectName(QString::fromUtf8("edgeButton"));

        horizontalLayout->addWidget(edgeButton);

        cutButton = new QPushButton(SingleCut);
        cutButton->setObjectName(QString::fromUtf8("cutButton"));
        cutButton->setEnabled(false);

        horizontalLayout->addWidget(cutButton);

        cutDistanceLineEdit = new QLineEdit(SingleCut);
        cutDistanceLineEdit->setObjectName(QString::fromUtf8("cutDistanceLineEdit"));
        cutDistanceLineEdit->setReadOnly(true);

        horizontalLayout->addWidget(cutDistanceLineEdit);

        label = new QLabel(SingleCut);
        label->setObjectName(QString::fromUtf8("label"));
        label->setTextFormat(Qt::RichText);

        horizontalLayout->addWidget(label);


        retranslateUi(SingleCut);

        QMetaObject::connectSlotsByName(SingleCut);
    } // setupUi

    void retranslateUi(QWidget *SingleCut)
    {
        SingleCut->setWindowTitle(QApplication::translate("SingleCut", "Form", 0, QApplication::UnicodeUTF8));
        orientButton->setText(QApplication::translate("SingleCut", "Orient", 0, QApplication::UnicodeUTF8));
        axisBox->clear();
        axisBox->insertItems(0, QStringList()
         << QApplication::translate("SingleCut", "Free", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SingleCut", "+X", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SingleCut", "-X", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SingleCut", "+Y", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SingleCut", "-Y", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SingleCut", "+Z", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SingleCut", "-Z", 0, QApplication::UnicodeUTF8)
        );
        nameField->setText(QApplication::translate("SingleCut", "Top", 0, QApplication::UnicodeUTF8));
        edgeButton->setText(QApplication::translate("SingleCut", "Set Edge", 0, QApplication::UnicodeUTF8));
        cutButton->setText(QApplication::translate("SingleCut", "Cut", 0, QApplication::UnicodeUTF8));
        cutDistanceLineEdit->setText(QApplication::translate("SingleCut", "?", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("SingleCut", "&mu;m", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SingleCut: public Ui_SingleCut {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SINGLE_CUT_H
