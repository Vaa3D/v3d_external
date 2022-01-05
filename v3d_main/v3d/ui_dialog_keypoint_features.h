/********************************************************************************
** Form generated from reading UI file 'dialog_keypoint_features.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_KEYPOINT_FEATURES_H
#define UI_DIALOG_KEYPOINT_FEATURES_H

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
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
#include <QWidget>

QT_BEGIN_NAMESPACE

class Ui_KeypointFeaturesDialog
{
public:
    QWidget *layoutWidget;
    QGridLayout *gridLayout_4;
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
    QLabel *label_4;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_x;
    QLineEdit *coord_x;
    QLabel *label_y;
    QLineEdit *coord_y;
    QLabel *label_z;
    QLineEdit *coord_z;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_channel;
    QSpinBox *statistics_channel;
    QLabel *label_r;
    QLineEdit *radius;
    QLabel *label_normalized;
    QSpinBox *normalized;
    QLabel *label_th;
    QLineEdit *threshold;
    QFrame *line_4;
    QGridLayout *gridLayout_3;
    QHBoxLayout *horizontalLayout_5;
    QLabel *orilabel;
    QSpacerItem *horizontalSpacer_5;
    QGridLayout *gridLayout1;
    QSpacerItem *horizontalSpacer_3;
    QLabel *label_lambda1_ori;
    QLineEdit *lambda1_ori_r1;
    QLineEdit *lambda1_ori_r2;
    QLineEdit *lambda1_ori_r3;
    QLineEdit *lambda1_ori_r4;
    QLabel *label_lambda2_ori;
    QLineEdit *lambda2_ori_r1;
    QLineEdit *lambda2_ori_r2;
    QLineEdit *lambda2_ori_r3;
    QLineEdit *lambda2_ori_r4;
    QLabel *label_lambda3_ori;
    QLineEdit *lambda3_ori_r1;
    QLineEdit *lambda3_ori_r2;
    QLineEdit *lambda3_ori_r3;
    QLineEdit *lambda3_ori_r4;
    QCheckBox *checkBox_ori_scale1;
    QCheckBox *checkBox_ori_scale2;
    QCheckBox *checkBox_ori_scale3;
    QCheckBox *checkBox_ori_scale4;
    QHBoxLayout *horizontalLayout_6;
    QLabel *difflabel;
    QSpacerItem *horizontalSpacer_6;
    QGridLayout *gridLayout_2;
    QSpacerItem *horizontalSpacer_4;
    QLabel *label_lambda1_diff;
    QLineEdit *lambda1_diff_r1;
    QLineEdit *lambda1_diff_r2;
    QLineEdit *lambda1_diff_r3;
    QLineEdit *lambda1_diff_r4;
    QLabel *label_lambda2_diff;
    QLineEdit *lambda2_diff_r1;
    QLineEdit *lambda2_diff_r2;
    QLineEdit *lambda2_diff_r3;
    QLineEdit *lambda2_diff_r4;
    QLabel *label_lambda3_diff;
    QLineEdit *lambda3_diff_r1;
    QLineEdit *lambda3_diff_r2;
    QLineEdit *lambda3_diff_r3;
    QLineEdit *lambda3_diff_r4;
    QCheckBox *checkBox_diff_scale1;
    QCheckBox *checkBox_diff_scale2;
    QCheckBox *checkBox_diff_scale3;
    QCheckBox *checkBox_diff_scale4;
    QFrame *line_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *GenSimMap;
    QFrame *line_3;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancelButton;
    QPushButton *okButton;

    void setupUi(QDialog *KeypointFeaturesDialog)
    {
        if (KeypointFeaturesDialog->objectName().isEmpty())
            KeypointFeaturesDialog->setObjectName(QString::fromUtf8("KeypointFeaturesDialog"));
        KeypointFeaturesDialog->resize(820, 740);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(KeypointFeaturesDialog->sizePolicy().hasHeightForWidth());
        KeypointFeaturesDialog->setSizePolicy(sizePolicy);
        KeypointFeaturesDialog->setMinimumSize(QSize(820, 740));
        KeypointFeaturesDialog->setMaximumSize(QSize(1000, 740));
        QFont font;
        font.setFamily(QString::fromUtf8("Courier"));
        font.setPointSize(13);
        font.setBold(false);
        font.setItalic(false);
        font.setUnderline(false);
     //  font.setWeight(50);
        font.setStrikeOut(false);
        KeypointFeaturesDialog->setFont(font);
        KeypointFeaturesDialog->setCursor(QCursor(Qt::PointingHandCursor));
        KeypointFeaturesDialog->setSizeGripEnabled(false);
        layoutWidget = new QWidget(KeypointFeaturesDialog);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(20, 20, 782, 688));
        gridLayout_4 = new QGridLayout(layoutWidget);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setContentsMargins(0, 0, 0, 0);
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


        gridLayout_4->addLayout(gridLayout, 0, 0, 1, 1);

        line = new QFrame(layoutWidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout_4->addWidget(line, 1, 0, 1, 1);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        label_6 = new QLabel(layoutWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        hboxLayout->addWidget(label_6);

        spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem1);


        gridLayout_4->addLayout(hboxLayout, 2, 0, 1, 1);

        label_4 = new QLabel(layoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_4, 3, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_x = new QLabel(layoutWidget);
        label_x->setObjectName(QString::fromUtf8("label_x"));

        horizontalLayout_2->addWidget(label_x);

        coord_x = new QLineEdit(layoutWidget);
        coord_x->setObjectName(QString::fromUtf8("coord_x"));
        coord_x->setMaxLength(5);

        horizontalLayout_2->addWidget(coord_x);

        label_y = new QLabel(layoutWidget);
        label_y->setObjectName(QString::fromUtf8("label_y"));

        horizontalLayout_2->addWidget(label_y);

        coord_y = new QLineEdit(layoutWidget);
        coord_y->setObjectName(QString::fromUtf8("coord_y"));
        coord_y->setMaxLength(5);

        horizontalLayout_2->addWidget(coord_y);

        label_z = new QLabel(layoutWidget);
        label_z->setObjectName(QString::fromUtf8("label_z"));

        horizontalLayout_2->addWidget(label_z);

        coord_z = new QLineEdit(layoutWidget);
        coord_z->setObjectName(QString::fromUtf8("coord_z"));
        QFont font1;
        font1.setPointSize(13);
        coord_z->setFont(font1);
        coord_z->setMaxLength(5);

        horizontalLayout_2->addWidget(coord_z);


        gridLayout_4->addLayout(horizontalLayout_2, 4, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_channel = new QLabel(layoutWidget);
        label_channel->setObjectName(QString::fromUtf8("label_channel"));
        label_channel->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(label_channel);

        statistics_channel = new QSpinBox(layoutWidget);
        statistics_channel->setObjectName(QString::fromUtf8("statistics_channel"));

        horizontalLayout_3->addWidget(statistics_channel);

        label_r = new QLabel(layoutWidget);
        label_r->setObjectName(QString::fromUtf8("label_r"));

        horizontalLayout_3->addWidget(label_r);

        radius = new QLineEdit(layoutWidget);
        radius->setObjectName(QString::fromUtf8("radius"));
        radius->setMaxLength(5);

        horizontalLayout_3->addWidget(radius);

        label_normalized = new QLabel(layoutWidget);
        label_normalized->setObjectName(QString::fromUtf8("label_normalized"));
        label_normalized->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(label_normalized);

        normalized = new QSpinBox(layoutWidget);
        normalized->setObjectName(QString::fromUtf8("normalized"));

        horizontalLayout_3->addWidget(normalized);

        label_th = new QLabel(layoutWidget);
        label_th->setObjectName(QString::fromUtf8("label_th"));

        horizontalLayout_3->addWidget(label_th);

        threshold = new QLineEdit(layoutWidget);
        threshold->setObjectName(QString::fromUtf8("threshold"));
        threshold->setMaxLength(5);

        horizontalLayout_3->addWidget(threshold);


        gridLayout_4->addLayout(horizontalLayout_3, 5, 0, 1, 1);

        line_4 = new QFrame(layoutWidget);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);

        gridLayout_4->addWidget(line_4, 6, 0, 1, 1);

        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        orilabel = new QLabel(layoutWidget);
        orilabel->setObjectName(QString::fromUtf8("orilabel"));
        orilabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_5->addWidget(orilabel);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_5);


        gridLayout_3->addLayout(horizontalLayout_5, 0, 0, 1, 1);

        gridLayout1 = new QGridLayout();
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        horizontalSpacer_3 = new QSpacerItem(58, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout1->addItem(horizontalSpacer_3, 0, 0, 1, 1);

        label_lambda1_ori = new QLabel(layoutWidget);
        label_lambda1_ori->setObjectName(QString::fromUtf8("label_lambda1_ori"));

        gridLayout1->addWidget(label_lambda1_ori, 1, 0, 1, 1);

        lambda1_ori_r1 = new QLineEdit(layoutWidget);
        lambda1_ori_r1->setObjectName(QString::fromUtf8("lambda1_ori_r1"));

        gridLayout1->addWidget(lambda1_ori_r1, 1, 1, 1, 1);

        lambda1_ori_r2 = new QLineEdit(layoutWidget);
        lambda1_ori_r2->setObjectName(QString::fromUtf8("lambda1_ori_r2"));

        gridLayout1->addWidget(lambda1_ori_r2, 1, 2, 1, 1);

        lambda1_ori_r3 = new QLineEdit(layoutWidget);
        lambda1_ori_r3->setObjectName(QString::fromUtf8("lambda1_ori_r3"));

        gridLayout1->addWidget(lambda1_ori_r3, 1, 3, 1, 1);

        lambda1_ori_r4 = new QLineEdit(layoutWidget);
        lambda1_ori_r4->setObjectName(QString::fromUtf8("lambda1_ori_r4"));

        gridLayout1->addWidget(lambda1_ori_r4, 1, 4, 1, 1);

        label_lambda2_ori = new QLabel(layoutWidget);
        label_lambda2_ori->setObjectName(QString::fromUtf8("label_lambda2_ori"));

        gridLayout1->addWidget(label_lambda2_ori, 2, 0, 1, 1);

        lambda2_ori_r1 = new QLineEdit(layoutWidget);
        lambda2_ori_r1->setObjectName(QString::fromUtf8("lambda2_ori_r1"));

        gridLayout1->addWidget(lambda2_ori_r1, 2, 1, 1, 1);

        lambda2_ori_r2 = new QLineEdit(layoutWidget);
        lambda2_ori_r2->setObjectName(QString::fromUtf8("lambda2_ori_r2"));

        gridLayout1->addWidget(lambda2_ori_r2, 2, 2, 1, 1);

        lambda2_ori_r3 = new QLineEdit(layoutWidget);
        lambda2_ori_r3->setObjectName(QString::fromUtf8("lambda2_ori_r3"));

        gridLayout1->addWidget(lambda2_ori_r3, 2, 3, 1, 1);

        lambda2_ori_r4 = new QLineEdit(layoutWidget);
        lambda2_ori_r4->setObjectName(QString::fromUtf8("lambda2_ori_r4"));

        gridLayout1->addWidget(lambda2_ori_r4, 2, 4, 1, 1);

        label_lambda3_ori = new QLabel(layoutWidget);
        label_lambda3_ori->setObjectName(QString::fromUtf8("label_lambda3_ori"));

        gridLayout1->addWidget(label_lambda3_ori, 3, 0, 1, 1);

        lambda3_ori_r1 = new QLineEdit(layoutWidget);
        lambda3_ori_r1->setObjectName(QString::fromUtf8("lambda3_ori_r1"));

        gridLayout1->addWidget(lambda3_ori_r1, 3, 1, 1, 1);

        lambda3_ori_r2 = new QLineEdit(layoutWidget);
        lambda3_ori_r2->setObjectName(QString::fromUtf8("lambda3_ori_r2"));

        gridLayout1->addWidget(lambda3_ori_r2, 3, 2, 1, 1);

        lambda3_ori_r3 = new QLineEdit(layoutWidget);
        lambda3_ori_r3->setObjectName(QString::fromUtf8("lambda3_ori_r3"));

        gridLayout1->addWidget(lambda3_ori_r3, 3, 3, 1, 1);

        lambda3_ori_r4 = new QLineEdit(layoutWidget);
        lambda3_ori_r4->setObjectName(QString::fromUtf8("lambda3_ori_r4"));

        gridLayout1->addWidget(lambda3_ori_r4, 3, 4, 1, 1);

        checkBox_ori_scale1 = new QCheckBox(layoutWidget);
        checkBox_ori_scale1->setObjectName(QString::fromUtf8("checkBox_ori_scale1"));
        checkBox_ori_scale1->setLayoutDirection(Qt::LeftToRight);

        gridLayout1->addWidget(checkBox_ori_scale1, 0, 1, 1, 1);

        checkBox_ori_scale2 = new QCheckBox(layoutWidget);
        checkBox_ori_scale2->setObjectName(QString::fromUtf8("checkBox_ori_scale2"));

        gridLayout1->addWidget(checkBox_ori_scale2, 0, 2, 1, 1);

        checkBox_ori_scale3 = new QCheckBox(layoutWidget);
        checkBox_ori_scale3->setObjectName(QString::fromUtf8("checkBox_ori_scale3"));

        gridLayout1->addWidget(checkBox_ori_scale3, 0, 3, 1, 1);

        checkBox_ori_scale4 = new QCheckBox(layoutWidget);
        checkBox_ori_scale4->setObjectName(QString::fromUtf8("checkBox_ori_scale4"));

        gridLayout1->addWidget(checkBox_ori_scale4, 0, 4, 1, 1);


        gridLayout_3->addLayout(gridLayout1, 1, 0, 1, 1);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        difflabel = new QLabel(layoutWidget);
        difflabel->setObjectName(QString::fromUtf8("difflabel"));
        difflabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_6->addWidget(difflabel);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_6);


        gridLayout_3->addLayout(horizontalLayout_6, 2, 0, 1, 1);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        horizontalSpacer_4 = new QSpacerItem(68, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_4, 0, 0, 1, 1);

        label_lambda1_diff = new QLabel(layoutWidget);
        label_lambda1_diff->setObjectName(QString::fromUtf8("label_lambda1_diff"));

        gridLayout_2->addWidget(label_lambda1_diff, 1, 0, 1, 1);

        lambda1_diff_r1 = new QLineEdit(layoutWidget);
        lambda1_diff_r1->setObjectName(QString::fromUtf8("lambda1_diff_r1"));

        gridLayout_2->addWidget(lambda1_diff_r1, 1, 1, 1, 1);

        lambda1_diff_r2 = new QLineEdit(layoutWidget);
        lambda1_diff_r2->setObjectName(QString::fromUtf8("lambda1_diff_r2"));

        gridLayout_2->addWidget(lambda1_diff_r2, 1, 2, 1, 1);

        lambda1_diff_r3 = new QLineEdit(layoutWidget);
        lambda1_diff_r3->setObjectName(QString::fromUtf8("lambda1_diff_r3"));

        gridLayout_2->addWidget(lambda1_diff_r3, 1, 3, 1, 1);

        lambda1_diff_r4 = new QLineEdit(layoutWidget);
        lambda1_diff_r4->setObjectName(QString::fromUtf8("lambda1_diff_r4"));

        gridLayout_2->addWidget(lambda1_diff_r4, 1, 4, 1, 1);

        label_lambda2_diff = new QLabel(layoutWidget);
        label_lambda2_diff->setObjectName(QString::fromUtf8("label_lambda2_diff"));

        gridLayout_2->addWidget(label_lambda2_diff, 2, 0, 1, 1);

        lambda2_diff_r1 = new QLineEdit(layoutWidget);
        lambda2_diff_r1->setObjectName(QString::fromUtf8("lambda2_diff_r1"));

        gridLayout_2->addWidget(lambda2_diff_r1, 2, 1, 1, 1);

        lambda2_diff_r2 = new QLineEdit(layoutWidget);
        lambda2_diff_r2->setObjectName(QString::fromUtf8("lambda2_diff_r2"));

        gridLayout_2->addWidget(lambda2_diff_r2, 2, 2, 1, 1);

        lambda2_diff_r3 = new QLineEdit(layoutWidget);
        lambda2_diff_r3->setObjectName(QString::fromUtf8("lambda2_diff_r3"));

        gridLayout_2->addWidget(lambda2_diff_r3, 2, 3, 1, 1);

        lambda2_diff_r4 = new QLineEdit(layoutWidget);
        lambda2_diff_r4->setObjectName(QString::fromUtf8("lambda2_diff_r4"));

        gridLayout_2->addWidget(lambda2_diff_r4, 2, 4, 1, 1);

        label_lambda3_diff = new QLabel(layoutWidget);
        label_lambda3_diff->setObjectName(QString::fromUtf8("label_lambda3_diff"));

        gridLayout_2->addWidget(label_lambda3_diff, 3, 0, 1, 1);

        lambda3_diff_r1 = new QLineEdit(layoutWidget);
        lambda3_diff_r1->setObjectName(QString::fromUtf8("lambda3_diff_r1"));

        gridLayout_2->addWidget(lambda3_diff_r1, 3, 1, 1, 1);

        lambda3_diff_r2 = new QLineEdit(layoutWidget);
        lambda3_diff_r2->setObjectName(QString::fromUtf8("lambda3_diff_r2"));

        gridLayout_2->addWidget(lambda3_diff_r2, 3, 2, 1, 1);

        lambda3_diff_r3 = new QLineEdit(layoutWidget);
        lambda3_diff_r3->setObjectName(QString::fromUtf8("lambda3_diff_r3"));

        gridLayout_2->addWidget(lambda3_diff_r3, 3, 3, 1, 1);

        lambda3_diff_r4 = new QLineEdit(layoutWidget);
        lambda3_diff_r4->setObjectName(QString::fromUtf8("lambda3_diff_r4"));

        gridLayout_2->addWidget(lambda3_diff_r4, 3, 4, 1, 1);

        checkBox_diff_scale1 = new QCheckBox(layoutWidget);
        checkBox_diff_scale1->setObjectName(QString::fromUtf8("checkBox_diff_scale1"));

        gridLayout_2->addWidget(checkBox_diff_scale1, 0, 1, 1, 1);

        checkBox_diff_scale2 = new QCheckBox(layoutWidget);
        checkBox_diff_scale2->setObjectName(QString::fromUtf8("checkBox_diff_scale2"));

        gridLayout_2->addWidget(checkBox_diff_scale2, 0, 2, 1, 1);

        checkBox_diff_scale3 = new QCheckBox(layoutWidget);
        checkBox_diff_scale3->setObjectName(QString::fromUtf8("checkBox_diff_scale3"));

        gridLayout_2->addWidget(checkBox_diff_scale3, 0, 3, 1, 1);

        checkBox_diff_scale4 = new QCheckBox(layoutWidget);
        checkBox_diff_scale4->setObjectName(QString::fromUtf8("checkBox_diff_scale4"));

        gridLayout_2->addWidget(checkBox_diff_scale4, 0, 4, 1, 1);


        gridLayout_3->addLayout(gridLayout_2, 3, 0, 1, 1);


        gridLayout_4->addLayout(gridLayout_3, 7, 0, 1, 1);

        line_2 = new QFrame(layoutWidget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        gridLayout_4->addWidget(line_2, 8, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer_2 = new QSpacerItem(148, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        GenSimMap = new QPushButton(layoutWidget);
        GenSimMap->setObjectName(QString::fromUtf8("GenSimMap"));
        GenSimMap->setAutoFillBackground(false);

        horizontalLayout->addWidget(GenSimMap);


        gridLayout_4->addLayout(horizontalLayout, 9, 0, 1, 1);

        line_3 = new QFrame(layoutWidget);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        gridLayout_4->addWidget(line_3, 10, 0, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer = new QSpacerItem(98, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        cancelButton = new QPushButton(layoutWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout_4->addWidget(cancelButton);

        okButton = new QPushButton(layoutWidget);
        okButton->setObjectName(QString::fromUtf8("okButton"));
        okButton->setDefault(true);

        horizontalLayout_4->addWidget(okButton);


        gridLayout_4->addLayout(horizontalLayout_4, 11, 0, 1, 1);


        retranslateUi(KeypointFeaturesDialog);

        QMetaObject::connectSlotsByName(KeypointFeaturesDialog);
    } // setupUi

    void retranslateUi(QDialog *KeypointFeaturesDialog)
    {
        KeypointFeaturesDialog->setWindowTitle(QApplication::translate("KeypointFeaturesDialog", "Keypoint Features Vector", 0));
        label->setText(QApplication::translate("KeypointFeaturesDialog", "Keypoint No./label", 0));
        order->setInputMask(QString());
        order->setText(QString());
        label_2->setText(QApplication::translate("KeypointFeaturesDialog", "Name", 0));
        label_3->setText(QApplication::translate("KeypointFeaturesDialog", "Comments", 0));
        label_6->setText(QApplication::translate("KeypointFeaturesDialog", "Keypoint Features", 0));
        label_4->setText(QApplication::translate("KeypointFeaturesDialog", "Geometry: center & others", 0));
        label_x->setText(QApplication::translate("KeypointFeaturesDialog", "X", 0));
        label_y->setText(QApplication::translate("KeypointFeaturesDialog", "Y", 0));
        label_z->setText(QApplication::translate("KeypointFeaturesDialog", "Z (page)", 0));
        label_channel->setText(QApplication::translate("KeypointFeaturesDialog", "Statistics of Channel", 0));
        label_r->setText(QApplication::translate("KeypointFeaturesDialog", "radius", 0));
        label_normalized->setText(QApplication::translate("KeypointFeaturesDialog", "Normalized", 0));
        label_th->setText(QApplication::translate("KeypointFeaturesDialog", "threshold", 0));
        orilabel->setText(QApplication::translate("KeypointFeaturesDialog", "PCA features of ori", 0));
        label_lambda1_ori->setText(QApplication::translate("KeypointFeaturesDialog", "Lambda 1", 0));
        label_lambda2_ori->setText(QApplication::translate("KeypointFeaturesDialog", "Lambda 2", 0));
        label_lambda3_ori->setText(QApplication::translate("KeypointFeaturesDialog", "Lambda 3", 0));
        checkBox_ori_scale1->setText(QApplication::translate("KeypointFeaturesDialog", "    scale 1", 0));
        checkBox_ori_scale2->setText(QApplication::translate("KeypointFeaturesDialog", "    scale 2", 0));
        checkBox_ori_scale3->setText(QApplication::translate("KeypointFeaturesDialog", "    scale 3", 0));
        checkBox_ori_scale4->setText(QApplication::translate("KeypointFeaturesDialog", "    scale 4", 0));
        difflabel->setText(QApplication::translate("KeypointFeaturesDialog", "PCA features of diff", 0));
        label_lambda1_diff->setText(QApplication::translate("KeypointFeaturesDialog", "Lambda 1", 0));
        label_lambda2_diff->setText(QApplication::translate("KeypointFeaturesDialog", "Lambda 2", 0));
        label_lambda3_diff->setText(QApplication::translate("KeypointFeaturesDialog", "Lambda 3", 0));
        checkBox_diff_scale1->setText(QApplication::translate("KeypointFeaturesDialog", "    scale 1", 0));
        checkBox_diff_scale2->setText(QApplication::translate("KeypointFeaturesDialog", "    scale 2", 0));
        checkBox_diff_scale3->setText(QApplication::translate("KeypointFeaturesDialog", "    scale 3", 0));
        checkBox_diff_scale4->setText(QApplication::translate("KeypointFeaturesDialog", "    scale 4", 0));
        GenSimMap->setText(QApplication::translate("KeypointFeaturesDialog", "Generating Similarity Map", 0));
        cancelButton->setText(QApplication::translate("KeypointFeaturesDialog", "Cancel", 0));
        okButton->setText(QApplication::translate("KeypointFeaturesDialog", "OK", 0));
    } // retranslateUi

};

namespace Ui {
    class KeypointFeaturesDialog: public Ui_KeypointFeaturesDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_KEYPOINT_FEATURES_H
