/********************************************************************************
** Form generated from reading UI file 'surfaceobj_geometry_dialog.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SURFACEOBJ_GEOMETRY_DIALOG_H
#define UI_SURFACEOBJ_GEOMETRY_DIALOG_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QDial>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>

QT_BEGIN_NAMESPACE

class Ui_SurfaceObjGeometryDialog
{
public:
    QLabel *label_11;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label_7;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label;
    QDoubleSpinBox *doubleSpinBox_shift_x;
    QDoubleSpinBox *doubleSpinBox_shift_y;
    QDoubleSpinBox *doubleSpinBox_shift_z;
    QLabel *label_5;
    QDoubleSpinBox *doubleSpinBox_scale_x;
    QDoubleSpinBox *doubleSpinBox_scale_y;
    QDoubleSpinBox *doubleSpinBox_scale_z;
    QLabel *label_8;
    QCheckBox *checkBox_flip_x;
    QCheckBox *checkBox_flip_y;
    QCheckBox *checkBox_flip_z;
    QLabel *label_6;
    QDial *dial_x;
    QDial *dial_y;
    QDial *dial_z;
    QCheckBox *checkBox_enable_rotation;
    QLabel *label_10;
    QDoubleSpinBox *doubleSpinBox_gscale_x;
    QDoubleSpinBox *doubleSpinBox_gscale_y;
    QDoubleSpinBox *doubleSpinBox_gscale_z;
    QHBoxLayout *hboxLayout;
    QLabel *label_9;
    QDoubleSpinBox *doubleSpinBox_scale_r;
    QFrame *line_2;
    QHBoxLayout *hboxLayout1;
    QPushButton *pushButton_undo_last;
    QPushButton *pushButton_reset;
    QSpacerItem *spacerItem;
    QPushButton *pushButton_cancel;
    QPushButton *pushButton_ok;

    void setupUi(QDialog *SurfaceObjGeometryDialog)
    {
        if (SurfaceObjGeometryDialog->objectName().isEmpty())
            SurfaceObjGeometryDialog->setObjectName(QString::fromUtf8("SurfaceObjGeometryDialog"));
        SurfaceObjGeometryDialog->resize(514, 450);
        SurfaceObjGeometryDialog->setMinimumSize(QSize(514, 450));
        SurfaceObjGeometryDialog->setMaximumSize(QSize(600, 450));
        label_11 = new QLabel(SurfaceObjGeometryDialog);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(24, 388, 469, 53));
        label_11->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        widget = new QWidget(SurfaceObjGeometryDialog);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(19, 14, 478, 365));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_7 = new QLabel(widget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_7, 0, 0, 1, 1);

        label_2 = new QLabel(widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_2, 0, 1, 1, 1);

        label_3 = new QLabel(widget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_3, 0, 2, 1, 1);

        label_4 = new QLabel(widget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_4, 0, 3, 1, 1);

        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label, 1, 0, 1, 1);

        doubleSpinBox_shift_x = new QDoubleSpinBox(widget);
        doubleSpinBox_shift_x->setObjectName(QString::fromUtf8("doubleSpinBox_shift_x"));
        doubleSpinBox_shift_x->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_shift_x, 1, 1, 1, 1);

        doubleSpinBox_shift_y = new QDoubleSpinBox(widget);
        doubleSpinBox_shift_y->setObjectName(QString::fromUtf8("doubleSpinBox_shift_y"));
        doubleSpinBox_shift_y->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_shift_y, 1, 2, 1, 1);

        doubleSpinBox_shift_z = new QDoubleSpinBox(widget);
        doubleSpinBox_shift_z->setObjectName(QString::fromUtf8("doubleSpinBox_shift_z"));
        doubleSpinBox_shift_z->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_shift_z, 1, 3, 1, 1);

        label_5 = new QLabel(widget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_5, 2, 0, 1, 1);

        doubleSpinBox_scale_x = new QDoubleSpinBox(widget);
        doubleSpinBox_scale_x->setObjectName(QString::fromUtf8("doubleSpinBox_scale_x"));
        doubleSpinBox_scale_x->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_scale_x, 2, 1, 1, 1);

        doubleSpinBox_scale_y = new QDoubleSpinBox(widget);
        doubleSpinBox_scale_y->setObjectName(QString::fromUtf8("doubleSpinBox_scale_y"));
        doubleSpinBox_scale_y->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_scale_y, 2, 2, 1, 1);

        doubleSpinBox_scale_z = new QDoubleSpinBox(widget);
        doubleSpinBox_scale_z->setObjectName(QString::fromUtf8("doubleSpinBox_scale_z"));
        doubleSpinBox_scale_z->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_scale_z, 2, 3, 1, 1);

        label_8 = new QLabel(widget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_8, 3, 0, 1, 1);

        checkBox_flip_x = new QCheckBox(widget);
        checkBox_flip_x->setObjectName(QString::fromUtf8("checkBox_flip_x"));

        gridLayout->addWidget(checkBox_flip_x, 3, 1, 1, 1);

        checkBox_flip_y = new QCheckBox(widget);
        checkBox_flip_y->setObjectName(QString::fromUtf8("checkBox_flip_y"));

        gridLayout->addWidget(checkBox_flip_y, 3, 2, 1, 1);

        checkBox_flip_z = new QCheckBox(widget);
        checkBox_flip_z->setObjectName(QString::fromUtf8("checkBox_flip_z"));

        gridLayout->addWidget(checkBox_flip_z, 3, 3, 1, 1);

        label_6 = new QLabel(widget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_6, 4, 0, 1, 1);

        dial_x = new QDial(widget);
        dial_x->setObjectName(QString::fromUtf8("dial_x"));
        dial_x->setCursor(QCursor(Qt::PointingHandCursor));
        dial_x->setAutoFillBackground(false);

        gridLayout->addWidget(dial_x, 4, 1, 2, 1);

        dial_y = new QDial(widget);
        dial_y->setObjectName(QString::fromUtf8("dial_y"));
        dial_y->setCursor(QCursor(Qt::PointingHandCursor));

        gridLayout->addWidget(dial_y, 4, 2, 2, 1);

        dial_z = new QDial(widget);
        dial_z->setObjectName(QString::fromUtf8("dial_z"));
        dial_z->setCursor(QCursor(Qt::PointingHandCursor));

        gridLayout->addWidget(dial_z, 4, 3, 2, 1);

        checkBox_enable_rotation = new QCheckBox(widget);
        checkBox_enable_rotation->setObjectName(QString::fromUtf8("checkBox_enable_rotation"));

        gridLayout->addWidget(checkBox_enable_rotation, 5, 0, 1, 1);

        label_10 = new QLabel(widget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_10, 6, 0, 1, 1);

        doubleSpinBox_gscale_x = new QDoubleSpinBox(widget);
        doubleSpinBox_gscale_x->setObjectName(QString::fromUtf8("doubleSpinBox_gscale_x"));
        doubleSpinBox_gscale_x->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_gscale_x, 6, 1, 1, 1);

        doubleSpinBox_gscale_y = new QDoubleSpinBox(widget);
        doubleSpinBox_gscale_y->setObjectName(QString::fromUtf8("doubleSpinBox_gscale_y"));
        doubleSpinBox_gscale_y->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_gscale_y, 6, 2, 1, 1);

        doubleSpinBox_gscale_z = new QDoubleSpinBox(widget);
        doubleSpinBox_gscale_z->setObjectName(QString::fromUtf8("doubleSpinBox_gscale_z"));
        doubleSpinBox_gscale_z->setDecimals(4);

        gridLayout->addWidget(doubleSpinBox_gscale_z, 6, 3, 1, 1);


        verticalLayout->addLayout(gridLayout);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        label_9 = new QLabel(widget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setAlignment(Qt::AlignCenter);

        hboxLayout->addWidget(label_9);

        doubleSpinBox_scale_r = new QDoubleSpinBox(widget);
        doubleSpinBox_scale_r->setObjectName(QString::fromUtf8("doubleSpinBox_scale_r"));
        doubleSpinBox_scale_r->setDecimals(4);

        hboxLayout->addWidget(doubleSpinBox_scale_r);


        verticalLayout->addLayout(hboxLayout);

        line_2 = new QFrame(widget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_2);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        pushButton_undo_last = new QPushButton(widget);
        pushButton_undo_last->setObjectName(QString::fromUtf8("pushButton_undo_last"));
        pushButton_undo_last->setAutoDefault(false);

        hboxLayout1->addWidget(pushButton_undo_last);

        pushButton_reset = new QPushButton(widget);
        pushButton_reset->setObjectName(QString::fromUtf8("pushButton_reset"));
        pushButton_reset->setAutoDefault(false);

        hboxLayout1->addWidget(pushButton_reset);

        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout1->addItem(spacerItem);

        pushButton_cancel = new QPushButton(widget);
        pushButton_cancel->setObjectName(QString::fromUtf8("pushButton_cancel"));
        pushButton_cancel->setAutoDefault(false);

        hboxLayout1->addWidget(pushButton_cancel);

        pushButton_ok = new QPushButton(widget);
        pushButton_ok->setObjectName(QString::fromUtf8("pushButton_ok"));
        pushButton_ok->setAutoDefault(false);

        hboxLayout1->addWidget(pushButton_ok);


        verticalLayout->addLayout(hboxLayout1);


        retranslateUi(SurfaceObjGeometryDialog);

        QMetaObject::connectSlotsByName(SurfaceObjGeometryDialog);
    } // setupUi

    void retranslateUi(QDialog *SurfaceObjGeometryDialog)
    {
        SurfaceObjGeometryDialog->setWindowTitle(QApplication::translate("SurfaceObjGeometryDialog", "Basic Geometry of Surface Object", 0));
        label_11->setText(QApplication::translate("SurfaceObjGeometryDialog", "Tip: \"scaling\" means scaling an object w/o changing the 3D location \n"
"of the center; \"G-scaling\" means just multiplying a scaling factor to the \n"
"current absolute coordinates.", 0));
        label_7->setText(QApplication::translate("SurfaceObjGeometryDialog", "axis", 0));
        label_2->setText(QApplication::translate("SurfaceObjGeometryDialog", "X", 0));
        label_3->setText(QApplication::translate("SurfaceObjGeometryDialog", "Y", 0));
        label_4->setText(QApplication::translate("SurfaceObjGeometryDialog", "Z", 0));
        label->setText(QApplication::translate("SurfaceObjGeometryDialog", "shift", 0));
        label_5->setText(QApplication::translate("SurfaceObjGeometryDialog", "scaling * 1000", 0));
        label_8->setText(QApplication::translate("SurfaceObjGeometryDialog", "mirror", 0));
        checkBox_flip_x->setText(QApplication::translate("SurfaceObjGeometryDialog", "flip X", 0));
        checkBox_flip_y->setText(QApplication::translate("SurfaceObjGeometryDialog", "flip Y", 0));
        checkBox_flip_z->setText(QApplication::translate("SurfaceObjGeometryDialog", "flip Z", 0));
        label_6->setText(QApplication::translate("SurfaceObjGeometryDialog", "rotation \n"
"(around the axis)", 0));
        checkBox_enable_rotation->setText(QApplication::translate("SurfaceObjGeometryDialog", "use rotation", 0));
        label_10->setText(QApplication::translate("SurfaceObjGeometryDialog", "G-scaling * 1000", 0));
        label_9->setText(QApplication::translate("SurfaceObjGeometryDialog", "radius scaling * 1000", 0));
        pushButton_undo_last->setText(QApplication::translate("SurfaceObjGeometryDialog", "Undo/redo last change", 0));
        pushButton_reset->setText(QApplication::translate("SurfaceObjGeometryDialog", "Reset", 0));
        pushButton_cancel->setText(QApplication::translate("SurfaceObjGeometryDialog", "Cancel", 0));
        pushButton_ok->setText(QApplication::translate("SurfaceObjGeometryDialog", "OK", 0));
    } // retranslateUi

};

namespace Ui {
    class SurfaceObjGeometryDialog: public Ui_SurfaceObjGeometryDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SURFACEOBJ_GEOMETRY_DIALOG_H
