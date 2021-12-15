/********************************************************************************
** Form generated from reading UI file 'surface_obj_annotation.ui'
**
** Created by: Qt User Interface Compiler version 6.2.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SURFACE_OBJ_ANNOTATION_H
#define UI_SURFACE_OBJ_ANNOTATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SurfaceObjAnnotationDialog
{
public:
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *order;
    QLabel *label_2;
    QLineEdit *name;
    QLabel *label_3;
    QLineEdit *comment;
    QLabel *label_4;
    QTextEdit *textEdit_otherinfo;
    QHBoxLayout *horizontalLayout;
    QPushButton *cancelButton;
    QPushButton *okButton;

    void setupUi(QDialog *SurfaceObjAnnotationDialog)
    {
        if (SurfaceObjAnnotationDialog->objectName().isEmpty())
            SurfaceObjAnnotationDialog->setObjectName(QString::fromUtf8("SurfaceObjAnnotationDialog"));
        SurfaceObjAnnotationDialog->resize(704, 366);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(SurfaceObjAnnotationDialog->sizePolicy().hasHeightForWidth());
        SurfaceObjAnnotationDialog->setSizePolicy(sizePolicy);
        SurfaceObjAnnotationDialog->setMinimumSize(QSize(460, 150));
        SurfaceObjAnnotationDialog->setMaximumSize(QSize(1000, 600));
        QFont font;
        font.setFamilies({QString::fromUtf8("Courier")});
        font.setPointSize(13);
        font.setBold(false);
        font.setItalic(false);
        font.setUnderline(false);
        font.setStrikeOut(false);
        SurfaceObjAnnotationDialog->setFont(font);
        SurfaceObjAnnotationDialog->setCursor(QCursor(Qt::PointingHandCursor));
        SurfaceObjAnnotationDialog->setSizeGripEnabled(false);
        widget = new QWidget(SurfaceObjAnnotationDialog);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(11, 13, 681, 334));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        order = new QLineEdit(widget);
        order->setObjectName(QString::fromUtf8("order"));
        order->setMaxLength(200);

        gridLayout->addWidget(order, 0, 1, 1, 1);

        label_2 = new QLabel(widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        name = new QLineEdit(widget);
        name->setObjectName(QString::fromUtf8("name"));
        name->setMaxLength(200);

        gridLayout->addWidget(name, 1, 1, 1, 1);

        label_3 = new QLabel(widget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        comment = new QLineEdit(widget);
        comment->setObjectName(QString::fromUtf8("comment"));
        comment->setMaxLength(500);

        gridLayout->addWidget(comment, 2, 1, 1, 1);

        label_4 = new QLabel(widget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        textEdit_otherinfo = new QTextEdit(widget);
        textEdit_otherinfo->setObjectName(QString::fromUtf8("textEdit_otherinfo"));

        gridLayout->addWidget(textEdit_otherinfo, 3, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        cancelButton = new QPushButton(widget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout->addWidget(cancelButton);

        okButton = new QPushButton(widget);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        horizontalLayout->addWidget(okButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(SurfaceObjAnnotationDialog);

        okButton->setDefault(true);


        QMetaObject::connectSlotsByName(SurfaceObjAnnotationDialog);
    } // setupUi

    void retranslateUi(QDialog *SurfaceObjAnnotationDialog)
    {
        SurfaceObjAnnotationDialog->setWindowTitle(QCoreApplication::translate("SurfaceObjAnnotationDialog", "Surface/Object Annotation", nullptr));
        label->setText(QCoreApplication::translate("SurfaceObjAnnotationDialog", "Object no", nullptr));
        order->setInputMask(QString());
        order->setText(QString());
        label_2->setText(QCoreApplication::translate("SurfaceObjAnnotationDialog", "Name", nullptr));
        label_3->setText(QCoreApplication::translate("SurfaceObjAnnotationDialog", "Comments", nullptr));
        label_4->setText(QCoreApplication::translate("SurfaceObjAnnotationDialog", "Other \n"
"properties", nullptr));
        cancelButton->setText(QCoreApplication::translate("SurfaceObjAnnotationDialog", "Cancel", nullptr));
        okButton->setText(QCoreApplication::translate("SurfaceObjAnnotationDialog", "OK", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SurfaceObjAnnotationDialog: public Ui_SurfaceObjAnnotationDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SURFACE_OBJ_ANNOTATION_H
