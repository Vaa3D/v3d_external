/********************************************************************************
** Form generated from reading UI file 'landmark_property.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LANDMARK_PROPERTY_H
#define UI_LANDMARK_PROPERTY_H

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialog>
#include <QFrame>
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

class Ui_LandmarkPropertyDialog
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *order;
    QLabel *label_2;
    QLineEdit *name;
    QLabel *label_3;
    QLineEdit *comment;
    QFrame *line;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QLabel *label_6;
    QSpacerItem *spacerItem1;
    QGridLayout *gridLayout_4;
    QLabel *label_4;
    QLabel *label_7;
    QLabel *label_8;
    QSpinBox *statistics_channel;
    QGridLayout *gridLayout1;
    QLabel *label_13;
    QLineEdit *coord_z;
    QLabel *label_16;
    QLineEdit *coord_x;
    QLabel *label_15;
    QLineEdit *coord_y;
    QLabel *label_19;
    QLineEdit *radius;
    QLabel *label_21;
    QComboBox *landmark_shape;
    QGridLayout *gridLayout_2;
    QLabel *label_14;
    QLineEdit *pix_val_red;
    QLabel *label_18;
    QLineEdit *pix_val_green;
    QLabel *label_20;
    QLineEdit *pix_val_blue;
    QLabel *label_22;
    QLabel *label_23;
    QLineEdit *pix_val_ch4;
    QLineEdit *pix_val_ch5;
    QGridLayout *gridLayout_3;
    QLabel *label_9;
    QLineEdit *val_peak;
    QLabel *label_11;
    QLineEdit *val_mean;
    QLabel *label_10;
    QLineEdit *val_stddev;
    QLabel *label_17;
    QLineEdit *val_size;
    QLabel *label_12;
    QLineEdit *val_mass;
    QLineEdit *val_pc3_d;
    QLabel *label_26;
    QLineEdit *val_pc2_d;
    QLabel *label_25;
    QLineEdit *val_pc1_d;
    QLabel *label_24;
    QLabel *label_27;
    QFrame *line_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label_5;
    QGridLayout *gridLayout2;
    QLabel *QualityAno1_text;
    QComboBox *QualityAno1;
    QLabel *QualityAno2_text;
    QComboBox *QualityAno2;
    QLabel *QualityAno3_text;
    QComboBox *QualityAno3;
    QSpacerItem *spacerItem2;
    QVBoxLayout *vboxLayout;
    QPushButton *cancelButton;
    QPushButton *okButton;

    void setupUi(QDialog *LandmarkPropertyDialog)
    {
        if (LandmarkPropertyDialog->objectName().isEmpty())
            LandmarkPropertyDialog->setObjectName(QString::fromUtf8("LandmarkPropertyDialog"));
        LandmarkPropertyDialog->resize(950, 480);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(LandmarkPropertyDialog->sizePolicy().hasHeightForWidth());
        LandmarkPropertyDialog->setSizePolicy(sizePolicy);
        LandmarkPropertyDialog->setMinimumSize(QSize(950, 480));
        LandmarkPropertyDialog->setMaximumSize(QSize(1000, 600));
        QFont font;
        font.setFamily(QString::fromUtf8("Courier"));
        font.setPointSize(13);
        font.setBold(false);
        font.setItalic(false);
        font.setUnderline(false);
       // font.setWeight(50);
        font.setStrikeOut(false);
        LandmarkPropertyDialog->setFont(font);
        LandmarkPropertyDialog->setCursor(QCursor(Qt::PointingHandCursor));
        LandmarkPropertyDialog->setSizeGripEnabled(false);
        layoutWidget = new QWidget(LandmarkPropertyDialog);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(17, 13, 913, 452));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(layoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        order = new QLineEdit(layoutWidget);
        order->setObjectName(QString::fromUtf8("order"));
        order->setMaxLength(200);

        gridLayout->addWidget(order, 0, 1, 1, 1);

        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        name = new QLineEdit(layoutWidget);
        name->setObjectName(QString::fromUtf8("name"));
        name->setMaxLength(200);

        gridLayout->addWidget(name, 1, 1, 1, 1);

        label_3 = new QLabel(layoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        comment = new QLineEdit(layoutWidget);
        comment->setObjectName(QString::fromUtf8("comment"));
        comment->setMaxLength(500);

        gridLayout->addWidget(comment, 2, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        line = new QFrame(layoutWidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        label_6 = new QLabel(layoutWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        hboxLayout->addWidget(label_6);

        spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem1);


        verticalLayout->addLayout(hboxLayout);

        gridLayout_4 = new QGridLayout();
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        label_4 = new QLabel(layoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_4, 0, 0, 1, 1);

        label_7 = new QLabel(layoutWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_7, 0, 1, 1, 1);

        label_8 = new QLabel(layoutWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_8, 0, 2, 1, 1);

        statistics_channel = new QSpinBox(layoutWidget);
        statistics_channel->setObjectName(QString::fromUtf8("statistics_channel"));

        gridLayout_4->addWidget(statistics_channel, 0, 3, 1, 1);

        gridLayout1 = new QGridLayout();
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        label_13 = new QLabel(layoutWidget);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        gridLayout1->addWidget(label_13, 0, 0, 1, 1);

        coord_z = new QLineEdit(layoutWidget);
        coord_z->setObjectName(QString::fromUtf8("coord_z"));
        QFont font1;
        font1.setPointSize(13);
        coord_z->setFont(font1);
        coord_z->setMaxLength(5);

        gridLayout1->addWidget(coord_z, 0, 1, 1, 1);

        label_16 = new QLabel(layoutWidget);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        gridLayout1->addWidget(label_16, 1, 0, 1, 1);

        coord_x = new QLineEdit(layoutWidget);
        coord_x->setObjectName(QString::fromUtf8("coord_x"));
        coord_x->setMaxLength(5);

        gridLayout1->addWidget(coord_x, 1, 1, 1, 1);

        label_15 = new QLabel(layoutWidget);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        gridLayout1->addWidget(label_15, 2, 0, 1, 1);

        coord_y = new QLineEdit(layoutWidget);
        coord_y->setObjectName(QString::fromUtf8("coord_y"));
        coord_y->setMaxLength(5);

        gridLayout1->addWidget(coord_y, 2, 1, 1, 1);

        label_19 = new QLabel(layoutWidget);
        label_19->setObjectName(QString::fromUtf8("label_19"));

        gridLayout1->addWidget(label_19, 3, 0, 1, 1);

        radius = new QLineEdit(layoutWidget);
        radius->setObjectName(QString::fromUtf8("radius"));
        radius->setMaxLength(5);

        gridLayout1->addWidget(radius, 3, 1, 1, 1);

        label_21 = new QLabel(layoutWidget);
        label_21->setObjectName(QString::fromUtf8("label_21"));

        gridLayout1->addWidget(label_21, 4, 0, 1, 1);

        landmark_shape = new QComboBox(layoutWidget);
        landmark_shape->setObjectName(QString::fromUtf8("landmark_shape"));

        gridLayout1->addWidget(landmark_shape, 4, 1, 1, 1);


        gridLayout_4->addLayout(gridLayout1, 1, 0, 1, 1);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_14 = new QLabel(layoutWidget);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        gridLayout_2->addWidget(label_14, 0, 0, 1, 1);

        pix_val_red = new QLineEdit(layoutWidget);
        pix_val_red->setObjectName(QString::fromUtf8("pix_val_red"));

        gridLayout_2->addWidget(pix_val_red, 0, 1, 1, 1);

        label_18 = new QLabel(layoutWidget);
        label_18->setObjectName(QString::fromUtf8("label_18"));

        gridLayout_2->addWidget(label_18, 1, 0, 1, 1);

        pix_val_green = new QLineEdit(layoutWidget);
        pix_val_green->setObjectName(QString::fromUtf8("pix_val_green"));

        gridLayout_2->addWidget(pix_val_green, 1, 1, 1, 1);

        label_20 = new QLabel(layoutWidget);
        label_20->setObjectName(QString::fromUtf8("label_20"));

        gridLayout_2->addWidget(label_20, 2, 0, 1, 1);

        pix_val_blue = new QLineEdit(layoutWidget);
        pix_val_blue->setObjectName(QString::fromUtf8("pix_val_blue"));

        gridLayout_2->addWidget(pix_val_blue, 2, 1, 1, 1);

        label_22 = new QLabel(layoutWidget);
        label_22->setObjectName(QString::fromUtf8("label_22"));

        gridLayout_2->addWidget(label_22, 3, 0, 1, 1);

        label_23 = new QLabel(layoutWidget);
        label_23->setObjectName(QString::fromUtf8("label_23"));

        gridLayout_2->addWidget(label_23, 4, 0, 1, 1);

        pix_val_ch4 = new QLineEdit(layoutWidget);
        pix_val_ch4->setObjectName(QString::fromUtf8("pix_val_ch4"));

        gridLayout_2->addWidget(pix_val_ch4, 3, 1, 1, 1);

        pix_val_ch5 = new QLineEdit(layoutWidget);
        pix_val_ch5->setObjectName(QString::fromUtf8("pix_val_ch5"));

        gridLayout_2->addWidget(pix_val_ch5, 4, 1, 1, 1);


        gridLayout_4->addLayout(gridLayout_2, 1, 1, 1, 1);

        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        label_9 = new QLabel(layoutWidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout_3->addWidget(label_9, 0, 0, 1, 1);

        val_peak = new QLineEdit(layoutWidget);
        val_peak->setObjectName(QString::fromUtf8("val_peak"));

        gridLayout_3->addWidget(val_peak, 0, 1, 1, 1);

        label_11 = new QLabel(layoutWidget);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout_3->addWidget(label_11, 1, 0, 1, 1);

        val_mean = new QLineEdit(layoutWidget);
        val_mean->setObjectName(QString::fromUtf8("val_mean"));

        gridLayout_3->addWidget(val_mean, 1, 1, 1, 1);

        label_10 = new QLabel(layoutWidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout_3->addWidget(label_10, 2, 0, 1, 1);

        val_stddev = new QLineEdit(layoutWidget);
        val_stddev->setObjectName(QString::fromUtf8("val_stddev"));

        gridLayout_3->addWidget(val_stddev, 2, 1, 1, 1);

        label_17 = new QLabel(layoutWidget);
        label_17->setObjectName(QString::fromUtf8("label_17"));

        gridLayout_3->addWidget(label_17, 3, 0, 1, 1);

        val_size = new QLineEdit(layoutWidget);
        val_size->setObjectName(QString::fromUtf8("val_size"));

        gridLayout_3->addWidget(val_size, 3, 1, 1, 1);

        label_12 = new QLabel(layoutWidget);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout_3->addWidget(label_12, 4, 0, 1, 1);

        val_mass = new QLineEdit(layoutWidget);
        val_mass->setObjectName(QString::fromUtf8("val_mass"));

        gridLayout_3->addWidget(val_mass, 4, 1, 1, 1);

        val_pc3_d = new QLineEdit(layoutWidget);
        val_pc3_d->setObjectName(QString::fromUtf8("val_pc3_d"));

        gridLayout_3->addWidget(val_pc3_d, 3, 3, 1, 1);

        label_26 = new QLabel(layoutWidget);
        label_26->setObjectName(QString::fromUtf8("label_26"));

        gridLayout_3->addWidget(label_26, 3, 2, 1, 1);

        val_pc2_d = new QLineEdit(layoutWidget);
        val_pc2_d->setObjectName(QString::fromUtf8("val_pc2_d"));

        gridLayout_3->addWidget(val_pc2_d, 2, 3, 1, 1);

        label_25 = new QLabel(layoutWidget);
        label_25->setObjectName(QString::fromUtf8("label_25"));

        gridLayout_3->addWidget(label_25, 2, 2, 1, 1);

        val_pc1_d = new QLineEdit(layoutWidget);
        val_pc1_d->setObjectName(QString::fromUtf8("val_pc1_d"));

        gridLayout_3->addWidget(val_pc1_d, 1, 3, 1, 1);

        label_24 = new QLabel(layoutWidget);
        label_24->setObjectName(QString::fromUtf8("label_24"));

        gridLayout_3->addWidget(label_24, 1, 2, 1, 1);

        label_27 = new QLabel(layoutWidget);
        label_27->setObjectName(QString::fromUtf8("label_27"));

        gridLayout_3->addWidget(label_27, 0, 3, 1, 1);


        gridLayout_4->addLayout(gridLayout_3, 1, 2, 1, 2);


        verticalLayout->addLayout(gridLayout_4);

        line_2 = new QFrame(layoutWidget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_5 = new QLabel(layoutWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout->addWidget(label_5);

        gridLayout2 = new QGridLayout();
        gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
        QualityAno1_text = new QLabel(layoutWidget);
        QualityAno1_text->setObjectName(QString::fromUtf8("QualityAno1_text"));

        gridLayout2->addWidget(QualityAno1_text, 0, 0, 1, 1);

        QualityAno1 = new QComboBox(layoutWidget);
        QualityAno1->setObjectName(QString::fromUtf8("QualityAno1"));
        QualityAno1->setMaxVisibleItems(2);

        gridLayout2->addWidget(QualityAno1, 0, 1, 1, 1);

        QualityAno2_text = new QLabel(layoutWidget);
        QualityAno2_text->setObjectName(QString::fromUtf8("QualityAno2_text"));

        gridLayout2->addWidget(QualityAno2_text, 1, 0, 1, 1);

        QualityAno2 = new QComboBox(layoutWidget);
        QualityAno2->setObjectName(QString::fromUtf8("QualityAno2"));
        QualityAno2->setMaxVisibleItems(2);

        gridLayout2->addWidget(QualityAno2, 1, 1, 1, 1);

        QualityAno3_text = new QLabel(layoutWidget);
        QualityAno3_text->setObjectName(QString::fromUtf8("QualityAno3_text"));

        gridLayout2->addWidget(QualityAno3_text, 2, 0, 1, 1);

        QualityAno3 = new QComboBox(layoutWidget);
        QualityAno3->setObjectName(QString::fromUtf8("QualityAno3"));
        QualityAno3->setMaxVisibleItems(2);

        gridLayout2->addWidget(QualityAno3, 2, 1, 1, 1);


        horizontalLayout->addLayout(gridLayout2);

        spacerItem2 = new QSpacerItem(148, 83, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(spacerItem2);

        vboxLayout = new QVBoxLayout();
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        cancelButton = new QPushButton(layoutWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        vboxLayout->addWidget(cancelButton);

        okButton = new QPushButton(layoutWidget);
        okButton->setObjectName(QString::fromUtf8("okButton"));
        okButton->setDefault(true);

        vboxLayout->addWidget(okButton);


        horizontalLayout->addLayout(vboxLayout);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(LandmarkPropertyDialog);

        QualityAno1->setCurrentIndex(-1);
        QualityAno2->setCurrentIndex(-1);
        QualityAno3->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(LandmarkPropertyDialog);
    } // setupUi

    void retranslateUi(QDialog *LandmarkPropertyDialog)
    {
        LandmarkPropertyDialog->setWindowTitle(QApplication::translate("LandmarkPropertyDialog", "Landmark Properties and Image Region Measures/Statistics", 0));
        label->setText(QApplication::translate("LandmarkPropertyDialog", "Landmark No./label", 0));
        order->setInputMask(QString());
        order->setText(QString());
        label_2->setText(QApplication::translate("LandmarkPropertyDialog", "Name", 0));
        label_3->setText(QApplication::translate("LandmarkPropertyDialog", "Comments", 0));
        label_6->setText(QApplication::translate("LandmarkPropertyDialog", "Other Information", 0));
        label_4->setText(QApplication::translate("LandmarkPropertyDialog", "Geometry: center & others", 0));
        label_7->setText(QApplication::translate("LandmarkPropertyDialog", "Voxel values", 0));
        label_8->setText(QApplication::translate("LandmarkPropertyDialog", "Statistics of Channel", 0));
        label_13->setText(QApplication::translate("LandmarkPropertyDialog", "Z (page)", 0));
        label_16->setText(QApplication::translate("LandmarkPropertyDialog", "X", 0));
        label_15->setText(QApplication::translate("LandmarkPropertyDialog", "Y", 0));
        label_19->setText(QApplication::translate("LandmarkPropertyDialog", "radius", 0));
        label_21->setText(QApplication::translate("LandmarkPropertyDialog", "shape", 0));
        landmark_shape->clear();
        landmark_shape->insertItems(0, QStringList()
         << QApplication::translate("LandmarkPropertyDialog", "Unset", 0)
         << QApplication::translate("LandmarkPropertyDialog", "sphere", 0)
         << QApplication::translate("LandmarkPropertyDialog", "cube", 0)
         << QApplication::translate("LandmarkPropertyDialog", "circle (XY plane)", 0)
         << QApplication::translate("LandmarkPropertyDialog", "circle (YZ plane)", 0)
         << QApplication::translate("LandmarkPropertyDialog", "circle (ZX plane)", 0)
         << QApplication::translate("LandmarkPropertyDialog", "square (XY plane)", 0)
         << QApplication::translate("LandmarkPropertyDialog", "square (YZ plane)", 0)
         << QApplication::translate("LandmarkPropertyDialog", "square (ZX plane)", 0)
         << QApplication::translate("LandmarkPropertyDialog", "line (hori, x-axis)", 0)
         << QApplication::translate("LandmarkPropertyDialog", "line (vert, y-axis)", 0)
         << QApplication::translate("LandmarkPropertyDialog", "line (in-out, z-axis)", 0)
        );
        label_14->setText(QApplication::translate("LandmarkPropertyDialog", "Red/Ch1", 0));
        label_18->setText(QApplication::translate("LandmarkPropertyDialog", "Green/Ch2", 0));
        label_20->setText(QApplication::translate("LandmarkPropertyDialog", "Blue/Ch3", 0));
        label_22->setText(QApplication::translate("LandmarkPropertyDialog", "Channel 4", 0));
        label_23->setText(QApplication::translate("LandmarkPropertyDialog", "Channel 5", 0));
        label_9->setText(QApplication::translate("LandmarkPropertyDialog", "Peak", 0));
        label_11->setText(QApplication::translate("LandmarkPropertyDialog", "Mean", 0));
        label_10->setText(QApplication::translate("LandmarkPropertyDialog", "Std Dev", 0));
        label_17->setText(QApplication::translate("LandmarkPropertyDialog", "Size", 0));
        label_12->setText(QApplication::translate("LandmarkPropertyDialog", "Mass", 0));
        label_26->setText(QApplication::translate("LandmarkPropertyDialog", "sigma3", 0));
        label_25->setText(QApplication::translate("LandmarkPropertyDialog", "sigma2", 0));
        label_24->setText(QApplication::translate("LandmarkPropertyDialog", "sigma1", 0));
        label_27->setText(QApplication::translate("LandmarkPropertyDialog", "Spatial anisotropy", 0));
        label_5->setText(QApplication::translate("LandmarkPropertyDialog", "Qualitative \n"
"description \n"
"of signal of\n"
"interest", 0));
        QualityAno1_text->setText(QApplication::translate("LandmarkPropertyDialog", "Intensity", 0));
        QualityAno2_text->setText(QApplication::translate("LandmarkPropertyDialog", "Distribution", 0));
        QualityAno3_text->setText(QApplication::translate("LandmarkPropertyDialog", "Shape", 0));
        cancelButton->setText(QApplication::translate("LandmarkPropertyDialog", "Cancel", 0));
        okButton->setText(QApplication::translate("LandmarkPropertyDialog", "OK", 0));
    } // retranslateUi

};

namespace Ui {
    class LandmarkPropertyDialog: public Ui_LandmarkPropertyDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LANDMARK_PROPERTY_H
