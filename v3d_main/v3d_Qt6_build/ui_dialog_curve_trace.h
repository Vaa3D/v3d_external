/********************************************************************************
** Form generated from reading UI file 'dialog_curve_trace.ui'
**
** Created by: Qt User Interface Compiler version 6.2.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_CURVE_TRACE_H
#define UI_DIALOG_CURVE_TRACE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_curve_trace_dialog
{
public:
    QWidget *widget;
    QVBoxLayout *verticalLayout_3;
    QGridLayout *gridLayout_3;
    QLabel *label_start_landmark;
    QSpinBox *spinBox_landmark_start;
    QLabel *label_end_landmark;
    QSpinBox *spinBox_landmark_end;
    QFrame *line_5;
    QLabel *label_12;
    QSpinBox *spinBox_datachannel;
    QFrame *line_4;
    QVBoxLayout *verticalLayout;
    QCheckBox *checkBox_b_curve;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_15;
    QSpinBox *spinBox_sp_nodestep;
    QCheckBox *checkBox_b_6connect;
    QCheckBox *checkBox_b_fullimage;
    QGridLayout *gridLayout_2;
    QLabel *label_20;
    QSpinBox *spinBox_sp_downsamplestep;
    QLabel *label_17;
    QSpinBox *spinBox_sp_smoothwinsz;
    QFrame *line_3;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_18;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QSpinBox *spinBox_nloops;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_2;
    QSpinBox *spinBox_n_controlpts;
    QCheckBox *checkBox_adaptiveCtrlPoints;
    QGridLayout *gridLayout;
    QLabel *label_4;
    QLabel *label_7;
    QLabel *label_5;
    QDoubleSpinBox *doubleSpinBox_1;
    QDoubleSpinBox *doubleSpinBox_2;
    QDoubleSpinBox *doubleSpinBox_3;
    QFrame *line_2;
    QHBoxLayout *horizontalLayout;
    QCheckBox *checkBox_estimateRadii;
    QCheckBox *checkBox_postMergeClosebyBranches;
    QFrame *line;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *cancelButton;
    QPushButton *okButton;

    void setupUi(QDialog *curve_trace_dialog)
    {
        if (curve_trace_dialog->objectName().isEmpty())
            curve_trace_dialog->setObjectName(QString::fromUtf8("curve_trace_dialog"));
        curve_trace_dialog->resize(407, 629);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(curve_trace_dialog->sizePolicy().hasHeightForWidth());
        curve_trace_dialog->setSizePolicy(sizePolicy);
        widget = new QWidget(curve_trace_dialog);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(19, 10, 371, 600));
        verticalLayout_3 = new QVBoxLayout(widget);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        label_start_landmark = new QLabel(widget);
        label_start_landmark->setObjectName(QString::fromUtf8("label_start_landmark"));

        gridLayout_3->addWidget(label_start_landmark, 0, 0, 1, 1);

        spinBox_landmark_start = new QSpinBox(widget);
        spinBox_landmark_start->setObjectName(QString::fromUtf8("spinBox_landmark_start"));
        spinBox_landmark_start->setMinimum(1);

        gridLayout_3->addWidget(spinBox_landmark_start, 0, 1, 1, 1);

        label_end_landmark = new QLabel(widget);
        label_end_landmark->setObjectName(QString::fromUtf8("label_end_landmark"));

        gridLayout_3->addWidget(label_end_landmark, 1, 0, 1, 1);

        spinBox_landmark_end = new QSpinBox(widget);
        spinBox_landmark_end->setObjectName(QString::fromUtf8("spinBox_landmark_end"));
        spinBox_landmark_end->setMinimum(1);

        gridLayout_3->addWidget(spinBox_landmark_end, 1, 1, 1, 1);

        line_5 = new QFrame(widget);
        line_5->setObjectName(QString::fromUtf8("line_5"));
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);

        gridLayout_3->addWidget(line_5, 2, 0, 1, 2);

        label_12 = new QLabel(widget);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout_3->addWidget(label_12, 3, 0, 1, 1);

        spinBox_datachannel = new QSpinBox(widget);
        spinBox_datachannel->setObjectName(QString::fromUtf8("spinBox_datachannel"));
        spinBox_datachannel->setMinimum(1);
        spinBox_datachannel->setMaximum(1);

        gridLayout_3->addWidget(spinBox_datachannel, 3, 1, 1, 1);

        line_4 = new QFrame(widget);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);

        gridLayout_3->addWidget(line_4, 4, 0, 1, 2);


        verticalLayout_3->addLayout(gridLayout_3);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        checkBox_b_curve = new QCheckBox(widget);
        checkBox_b_curve->setObjectName(QString::fromUtf8("checkBox_b_curve"));
        checkBox_b_curve->setChecked(true);

        verticalLayout->addWidget(checkBox_b_curve);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_15 = new QLabel(widget);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        horizontalLayout_5->addWidget(label_15);

        spinBox_sp_nodestep = new QSpinBox(widget);
        spinBox_sp_nodestep->setObjectName(QString::fromUtf8("spinBox_sp_nodestep"));
        spinBox_sp_nodestep->setMinimum(1);
        spinBox_sp_nodestep->setMaximum(1000);
        spinBox_sp_nodestep->setValue(1);

        horizontalLayout_5->addWidget(spinBox_sp_nodestep);

        checkBox_b_6connect = new QCheckBox(widget);
        checkBox_b_6connect->setObjectName(QString::fromUtf8("checkBox_b_6connect"));
        checkBox_b_6connect->setChecked(true);

        horizontalLayout_5->addWidget(checkBox_b_6connect);

        checkBox_b_fullimage = new QCheckBox(widget);
        checkBox_b_fullimage->setObjectName(QString::fromUtf8("checkBox_b_fullimage"));
        checkBox_b_fullimage->setChecked(true);

        horizontalLayout_5->addWidget(checkBox_b_fullimage);


        verticalLayout->addLayout(horizontalLayout_5);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_20 = new QLabel(widget);
        label_20->setObjectName(QString::fromUtf8("label_20"));

        gridLayout_2->addWidget(label_20, 0, 0, 1, 1);

        spinBox_sp_downsamplestep = new QSpinBox(widget);
        spinBox_sp_downsamplestep->setObjectName(QString::fromUtf8("spinBox_sp_downsamplestep"));
        spinBox_sp_downsamplestep->setMinimum(1);
        spinBox_sp_downsamplestep->setMaximum(1000);
        spinBox_sp_downsamplestep->setValue(5);

        gridLayout_2->addWidget(spinBox_sp_downsamplestep, 0, 1, 1, 1);

        label_17 = new QLabel(widget);
        label_17->setObjectName(QString::fromUtf8("label_17"));

        gridLayout_2->addWidget(label_17, 1, 0, 1, 1);

        spinBox_sp_smoothwinsz = new QSpinBox(widget);
        spinBox_sp_smoothwinsz->setObjectName(QString::fromUtf8("spinBox_sp_smoothwinsz"));
        spinBox_sp_smoothwinsz->setMinimum(1);
        spinBox_sp_smoothwinsz->setMaximum(1000);
        spinBox_sp_smoothwinsz->setValue(11);

        gridLayout_2->addWidget(spinBox_sp_smoothwinsz, 1, 1, 1, 1);


        verticalLayout->addLayout(gridLayout_2);


        verticalLayout_3->addLayout(verticalLayout);

        line_3 = new QFrame(widget);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        verticalLayout_3->addWidget(line_3);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        label_18 = new QLabel(widget);
        label_18->setObjectName(QString::fromUtf8("label_18"));

        verticalLayout_2->addWidget(label_18);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_3->addWidget(label);

        spinBox_nloops = new QSpinBox(widget);
        spinBox_nloops->setObjectName(QString::fromUtf8("spinBox_nloops"));
        spinBox_nloops->setEnabled(true);
        spinBox_nloops->setMinimum(1);
        spinBox_nloops->setMaximum(1000);
        spinBox_nloops->setValue(100);

        horizontalLayout_3->addWidget(spinBox_nloops);


        verticalLayout_2->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_2 = new QLabel(widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_4->addWidget(label_2);

        spinBox_n_controlpts = new QSpinBox(widget);
        spinBox_n_controlpts->setObjectName(QString::fromUtf8("spinBox_n_controlpts"));
        spinBox_n_controlpts->setValue(10);

        horizontalLayout_4->addWidget(spinBox_n_controlpts);

        checkBox_adaptiveCtrlPoints = new QCheckBox(widget);
        checkBox_adaptiveCtrlPoints->setObjectName(QString::fromUtf8("checkBox_adaptiveCtrlPoints"));
        checkBox_adaptiveCtrlPoints->setChecked(true);

        horizontalLayout_4->addWidget(checkBox_adaptiveCtrlPoints);


        verticalLayout_2->addLayout(horizontalLayout_4);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_4 = new QLabel(widget);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 0, 0, 1, 1);

        label_7 = new QLabel(widget);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 1, 0, 1, 1);

        label_5 = new QLabel(widget);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 2, 0, 1, 1);

        doubleSpinBox_1 = new QDoubleSpinBox(widget);
        doubleSpinBox_1->setObjectName(QString::fromUtf8("doubleSpinBox_1"));

        gridLayout->addWidget(doubleSpinBox_1, 0, 1, 1, 1);

        doubleSpinBox_2 = new QDoubleSpinBox(widget);
        doubleSpinBox_2->setObjectName(QString::fromUtf8("doubleSpinBox_2"));

        gridLayout->addWidget(doubleSpinBox_2, 1, 1, 1, 1);

        doubleSpinBox_3 = new QDoubleSpinBox(widget);
        doubleSpinBox_3->setObjectName(QString::fromUtf8("doubleSpinBox_3"));

        gridLayout->addWidget(doubleSpinBox_3, 2, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout);


        verticalLayout_3->addLayout(verticalLayout_2);

        line_2 = new QFrame(widget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        verticalLayout_3->addWidget(line_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        checkBox_estimateRadii = new QCheckBox(widget);
        checkBox_estimateRadii->setObjectName(QString::fromUtf8("checkBox_estimateRadii"));
        checkBox_estimateRadii->setChecked(true);

        horizontalLayout->addWidget(checkBox_estimateRadii);

        checkBox_postMergeClosebyBranches = new QCheckBox(widget);
        checkBox_postMergeClosebyBranches->setObjectName(QString::fromUtf8("checkBox_postMergeClosebyBranches"));
        checkBox_postMergeClosebyBranches->setChecked(true);

        horizontalLayout->addWidget(checkBox_postMergeClosebyBranches);


        verticalLayout_3->addLayout(horizontalLayout);

        line = new QFrame(widget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout_3->addWidget(line);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        cancelButton = new QPushButton(widget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout_2->addWidget(cancelButton);

        okButton = new QPushButton(widget);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        horizontalLayout_2->addWidget(okButton);


        verticalLayout_3->addLayout(horizontalLayout_2);

        QWidget::setTabOrder(spinBox_datachannel, spinBox_landmark_start);
        QWidget::setTabOrder(spinBox_landmark_start, spinBox_landmark_end);
        QWidget::setTabOrder(spinBox_landmark_end, checkBox_b_curve);
        QWidget::setTabOrder(checkBox_b_curve, spinBox_sp_nodestep);
        QWidget::setTabOrder(spinBox_sp_nodestep, checkBox_b_6connect);
        QWidget::setTabOrder(checkBox_b_6connect, spinBox_sp_downsamplestep);
        QWidget::setTabOrder(spinBox_sp_downsamplestep, spinBox_sp_smoothwinsz);
        QWidget::setTabOrder(spinBox_sp_smoothwinsz, spinBox_nloops);
        QWidget::setTabOrder(spinBox_nloops, spinBox_n_controlpts);
        QWidget::setTabOrder(spinBox_n_controlpts, checkBox_adaptiveCtrlPoints);
        QWidget::setTabOrder(checkBox_adaptiveCtrlPoints, cancelButton);
        QWidget::setTabOrder(cancelButton, okButton);

        retranslateUi(curve_trace_dialog);

        okButton->setDefault(true);


        QMetaObject::connectSlotsByName(curve_trace_dialog);
    } // setupUi

    void retranslateUi(QDialog *curve_trace_dialog)
    {
        curve_trace_dialog->setWindowTitle(QCoreApplication::translate("curve_trace_dialog", "Curve tracing parameters", nullptr));
        label_start_landmark->setText(QCoreApplication::translate("curve_trace_dialog", "start landmark ID", nullptr));
        label_end_landmark->setText(QCoreApplication::translate("curve_trace_dialog", "end landmark ID", nullptr));
        label_12->setText(QCoreApplication::translate("curve_trace_dialog", "data channel", nullptr));
        checkBox_b_curve->setText(QCoreApplication::translate("curve_trace_dialog", "Hybrid curve parameters ", nullptr));
        label_15->setText(QCoreApplication::translate("curve_trace_dialog", "resolution step", nullptr));
        checkBox_b_6connect->setText(QCoreApplication::translate("curve_trace_dialog", "6-connect", nullptr));
        checkBox_b_fullimage->setText(QCoreApplication::translate("curve_trace_dialog", "full image", nullptr));
        label_20->setText(QCoreApplication::translate("curve_trace_dialog", "final path sub-sampling step", nullptr));
        label_17->setText(QCoreApplication::translate("curve_trace_dialog", "final radius smoothing step", nullptr));
        label_18->setText(QCoreApplication::translate("curve_trace_dialog", "Deformable model parameters", nullptr));
        label->setText(QCoreApplication::translate("curve_trace_dialog", "# optimization loops", nullptr));
        label_2->setText(QCoreApplication::translate("curve_trace_dialog", "# control points", nullptr));
        checkBox_adaptiveCtrlPoints->setText(QCoreApplication::translate("curve_trace_dialog", "adaptive", nullptr));
        label_4->setText(QCoreApplication::translate("curve_trace_dialog", "length force weight", nullptr));
        label_7->setText(QCoreApplication::translate("curve_trace_dialog", "smoothness force weight", nullptr));
        label_5->setText(QCoreApplication::translate("curve_trace_dialog", "prior force weight", nullptr));
        checkBox_estimateRadii->setText(QCoreApplication::translate("curve_trace_dialog", "Estimate radii", nullptr));
        checkBox_postMergeClosebyBranches->setText(QCoreApplication::translate("curve_trace_dialog", "Post merge closeby branches", nullptr));
        cancelButton->setText(QCoreApplication::translate("curve_trace_dialog", "Cancel tracing", nullptr));
        okButton->setText(QCoreApplication::translate("curve_trace_dialog", "OK", nullptr));
    } // retranslateUi

};

namespace Ui {
    class curve_trace_dialog: public Ui_curve_trace_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_CURVE_TRACE_H
