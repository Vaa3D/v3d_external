/********************************************************************************
** Form generated from reading UI file 'dialog_imagecrop_bbox.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_IMAGECROP_BBOX_H
#define UI_DIALOG_IMAGECROP_BBOX_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Dialog_ImageCrop_BBox
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_2;
    QSpinBox *xminBox;
    QSpinBox *xmaxBox;
    QLabel *label_3;
    QSpinBox *yminBox;
    QSpinBox *ymaxBox;
    QLabel *label_4;
    QSpinBox *zminBox;
    QSpinBox *zmaxBox;
    QLabel *label_7;
    QSpinBox *colorminBox;
    QSpinBox *colormaxBox;
    QLabel *label;
    QComboBox *landmarksBox;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancelButton;
    QPushButton *okButton;
    QLabel *infolabel;

    void setupUi(QDialog *Dialog_ImageCrop_BBox)
    {
        if (Dialog_ImageCrop_BBox->objectName().isEmpty())
            Dialog_ImageCrop_BBox->setObjectName(QString::fromUtf8("Dialog_ImageCrop_BBox"));
        Dialog_ImageCrop_BBox->resize(469, 332);
        layoutWidget = new QWidget(Dialog_ImageCrop_BBox);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 10, 443, 301));
        verticalLayout_2 = new QVBoxLayout(layoutWidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_5 = new QLabel(layoutWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy);

        gridLayout->addWidget(label_5, 0, 1, 1, 1);

        label_6 = new QLabel(layoutWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        sizePolicy.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy);

        gridLayout->addWidget(label_6, 0, 2, 1, 1);

        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy);
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        xminBox = new QSpinBox(layoutWidget);
        xminBox->setObjectName(QString::fromUtf8("xminBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(xminBox->sizePolicy().hasHeightForWidth());
        xminBox->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(xminBox, 1, 1, 1, 1);

        xmaxBox = new QSpinBox(layoutWidget);
        xmaxBox->setObjectName(QString::fromUtf8("xmaxBox"));
        sizePolicy1.setHeightForWidth(xmaxBox->sizePolicy().hasHeightForWidth());
        xmaxBox->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(xmaxBox, 1, 2, 1, 1);

        label_3 = new QLabel(layoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        yminBox = new QSpinBox(layoutWidget);
        yminBox->setObjectName(QString::fromUtf8("yminBox"));
        sizePolicy1.setHeightForWidth(yminBox->sizePolicy().hasHeightForWidth());
        yminBox->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(yminBox, 2, 1, 1, 1);

        ymaxBox = new QSpinBox(layoutWidget);
        ymaxBox->setObjectName(QString::fromUtf8("ymaxBox"));
        sizePolicy1.setHeightForWidth(ymaxBox->sizePolicy().hasHeightForWidth());
        ymaxBox->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(ymaxBox, 2, 2, 1, 1);

        label_4 = new QLabel(layoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        sizePolicy.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy);
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        zminBox = new QSpinBox(layoutWidget);
        zminBox->setObjectName(QString::fromUtf8("zminBox"));
        sizePolicy1.setHeightForWidth(zminBox->sizePolicy().hasHeightForWidth());
        zminBox->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(zminBox, 3, 1, 1, 1);

        zmaxBox = new QSpinBox(layoutWidget);
        zmaxBox->setObjectName(QString::fromUtf8("zmaxBox"));
        sizePolicy1.setHeightForWidth(zmaxBox->sizePolicy().hasHeightForWidth());
        zmaxBox->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(zmaxBox, 3, 2, 1, 1);

        label_7 = new QLabel(layoutWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        sizePolicy.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy);
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_7, 4, 0, 1, 1);

        colorminBox = new QSpinBox(layoutWidget);
        colorminBox->setObjectName(QString::fromUtf8("colorminBox"));
        sizePolicy1.setHeightForWidth(colorminBox->sizePolicy().hasHeightForWidth());
        colorminBox->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(colorminBox, 4, 1, 1, 1);

        colormaxBox = new QSpinBox(layoutWidget);
        colormaxBox->setObjectName(QString::fromUtf8("colormaxBox"));
        sizePolicy1.setHeightForWidth(colormaxBox->sizePolicy().hasHeightForWidth());
        colormaxBox->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(colormaxBox, 4, 2, 1, 1);


        verticalLayout->addLayout(gridLayout);

        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        landmarksBox = new QComboBox(layoutWidget);
        landmarksBox->setObjectName(QString::fromUtf8("landmarksBox"));

        verticalLayout->addWidget(landmarksBox);


        verticalLayout_2->addLayout(verticalLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(208, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        cancelButton = new QPushButton(layoutWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout->addWidget(cancelButton);

        okButton = new QPushButton(layoutWidget);
        okButton->setObjectName(QString::fromUtf8("okButton"));
        okButton->setDefault(true);

        horizontalLayout->addWidget(okButton);


        verticalLayout_2->addLayout(horizontalLayout);

        infolabel = new QLabel(layoutWidget);
        infolabel->setObjectName(QString::fromUtf8("infolabel"));
        sizePolicy.setHeightForWidth(infolabel->sizePolicy().hasHeightForWidth());
        infolabel->setSizePolicy(sizePolicy);

        verticalLayout_2->addWidget(infolabel);

        infolabel->raise();

        retranslateUi(Dialog_ImageCrop_BBox);
        QObject::connect(okButton, SIGNAL(clicked()), Dialog_ImageCrop_BBox, SLOT(accept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), Dialog_ImageCrop_BBox, SLOT(reject()));

        QMetaObject::connectSlotsByName(Dialog_ImageCrop_BBox);
    } // setupUi

    void retranslateUi(QDialog *Dialog_ImageCrop_BBox)
    {
        Dialog_ImageCrop_BBox->setWindowTitle(QApplication::translate("Dialog_ImageCrop_BBox", "Image Crop Options", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("Dialog_ImageCrop_BBox", "Min", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("Dialog_ImageCrop_BBox", "Max", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Dialog_ImageCrop_BBox", "X :", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Dialog_ImageCrop_BBox", "Y :", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("Dialog_ImageCrop_BBox", "Z :", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("Dialog_ImageCrop_BBox", "Color channel:", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Dialog_ImageCrop_BBox", "Landmarks options : ", 0, QApplication::UnicodeUTF8));
        landmarksBox->clear();
        landmarksBox->insertItems(0, QStringList()
         << QApplication::translate("Dialog_ImageCrop_BBox", "Subtract min and delete landmarks outside the bounding box", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_ImageCrop_BBox", "Subtract min and keep landmarks outside the bounding box", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Dialog_ImageCrop_BBox", "Do not alter landmark coordinates", 0, QApplication::UnicodeUTF8)
        );
        cancelButton->setText(QApplication::translate("Dialog_ImageCrop_BBox", "Cancel", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("Dialog_ImageCrop_BBox", "OK", 0, QApplication::UnicodeUTF8));
        infolabel->setText(QApplication::translate("Dialog_ImageCrop_BBox", "info: Crop Image Options", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog_ImageCrop_BBox: public Ui_Dialog_ImageCrop_BBox {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_IMAGECROP_BBOX_H
