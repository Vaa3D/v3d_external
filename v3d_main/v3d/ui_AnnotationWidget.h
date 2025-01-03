/********************************************************************************
** Form generated from reading UI file 'AnnotationWidget.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ANNOTATIONWIDGET_H
#define UI_ANNOTATIONWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSplitter>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "../neuron_annotator/gui/trees/AnnotatedBranchTreeView.h"
#include "../neuron_annotator/gui/trees/OntologyTreeView.h"

QT_BEGIN_NAMESPACE

class Ui_AnnotationWidget
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *consoleGroupBox;
    QVBoxLayout *verticalLayout_3;
    QWidget *consoleLinkWidget;
    QHBoxLayout *h2Layout;
    QLabel *consoleLinkLabel;
    QPushButton *consoleLinkButton;
    QWidget *consoleSyncWidget;
    QHBoxLayout *h3Layout;
    QLabel *consoleSyncLabel;
    QPushButton *consoleSyncButton;
    QSplitter *splitter;
    QWidget *ontologyTreeWidget;
    QVBoxLayout *verticalLayout_1;
    QWidget *horizontalWidget_4;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_1;
    QLabel *ontologyTreeTitle;
    OntologyTreeView *ontologyTreeView;
    QWidget *annotationPanelWidget;
    QVBoxLayout *verticalLayout_2;
    QWidget *horizontalWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLabel *annotatedBranchTreeTitle;
    AnnotatedBranchTreeView *annotatedBranchTreeView;

    void setupUi(QWidget *AnnotationWidget)
    {
        if (AnnotationWidget->objectName().isEmpty())
            AnnotationWidget->setObjectName(QString::fromUtf8("AnnotationWidget"));
        AnnotationWidget->resize(380, 866);
        AnnotationWidget->setAutoFillBackground(false);
        verticalLayout = new QVBoxLayout(AnnotationWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        consoleGroupBox = new QGroupBox(AnnotationWidget);
        consoleGroupBox->setObjectName(QString::fromUtf8("consoleGroupBox"));
        verticalLayout_3 = new QVBoxLayout(consoleGroupBox);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        consoleLinkWidget = new QWidget(consoleGroupBox);
        consoleLinkWidget->setObjectName(QString::fromUtf8("consoleLinkWidget"));
        h2Layout = new QHBoxLayout(consoleLinkWidget);
        h2Layout->setObjectName(QString::fromUtf8("h2Layout"));
        consoleLinkLabel = new QLabel(consoleLinkWidget);
        consoleLinkLabel->setObjectName(QString::fromUtf8("consoleLinkLabel"));

        h2Layout->addWidget(consoleLinkLabel);

        consoleLinkButton = new QPushButton(consoleLinkWidget);
        consoleLinkButton->setObjectName(QString::fromUtf8("consoleLinkButton"));

        h2Layout->addWidget(consoleLinkButton);


        verticalLayout_3->addWidget(consoleLinkWidget);

        consoleSyncWidget = new QWidget(consoleGroupBox);
        consoleSyncWidget->setObjectName(QString::fromUtf8("consoleSyncWidget"));
        h3Layout = new QHBoxLayout(consoleSyncWidget);
        h3Layout->setObjectName(QString::fromUtf8("h3Layout"));
        consoleSyncLabel = new QLabel(consoleSyncWidget);
        consoleSyncLabel->setObjectName(QString::fromUtf8("consoleSyncLabel"));

        h3Layout->addWidget(consoleSyncLabel);

        consoleSyncButton = new QPushButton(consoleSyncWidget);
        consoleSyncButton->setObjectName(QString::fromUtf8("consoleSyncButton"));

        h3Layout->addWidget(consoleSyncButton);


        verticalLayout_3->addWidget(consoleSyncWidget);


        verticalLayout->addWidget(consoleGroupBox);

        splitter = new QSplitter(AnnotationWidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setAutoFillBackground(false);
        splitter->setOrientation(Qt::Vertical);
        splitter->setChildrenCollapsible(true);
        ontologyTreeWidget = new QWidget(splitter);
        ontologyTreeWidget->setObjectName(QString::fromUtf8("ontologyTreeWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(3);
        sizePolicy.setHeightForWidth(ontologyTreeWidget->sizePolicy().hasHeightForWidth());
        ontologyTreeWidget->setSizePolicy(sizePolicy);
        ontologyTreeWidget->setMinimumSize(QSize(300, 0));
        ontologyTreeWidget->setMaximumSize(QSize(16777215, 16777215));
        ontologyTreeWidget->setAutoFillBackground(false);
        verticalLayout_1 = new QVBoxLayout(ontologyTreeWidget);
        verticalLayout_1->setSpacing(0);
        verticalLayout_1->setContentsMargins(0, 0, 0, 0);
        verticalLayout_1->setObjectName(QString::fromUtf8("verticalLayout_1"));
        horizontalWidget_4 = new QWidget(ontologyTreeWidget);
        horizontalWidget_4->setObjectName(QString::fromUtf8("horizontalWidget_4"));
        horizontalLayout_4 = new QHBoxLayout(horizontalWidget_4);
        horizontalLayout_4->setSpacing(0);
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_1 = new QLabel(horizontalWidget_4);
        label_1->setObjectName(QString::fromUtf8("label_1"));
        label_1->setMinimumSize(QSize(0, 26));
        QFont font;
        font.setPointSize(12);
        font.setBold(true);
        font.setWeight(75);
        label_1->setFont(font);
        label_1->setMargin(5);

        horizontalLayout_4->addWidget(label_1);

        ontologyTreeTitle = new QLabel(horizontalWidget_4);
        ontologyTreeTitle->setObjectName(QString::fromUtf8("ontologyTreeTitle"));
        ontologyTreeTitle->setMinimumSize(QSize(0, 26));
        ontologyTreeTitle->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        ontologyTreeTitle->setMargin(5);

        horizontalLayout_4->addWidget(ontologyTreeTitle);


        verticalLayout_1->addWidget(horizontalWidget_4);

        ontologyTreeView = new OntologyTreeView(ontologyTreeWidget);
        ontologyTreeView->setObjectName(QString::fromUtf8("ontologyTreeView"));

        verticalLayout_1->addWidget(ontologyTreeView);

        splitter->addWidget(ontologyTreeWidget);
        annotationPanelWidget = new QWidget(splitter);
        annotationPanelWidget->setObjectName(QString::fromUtf8("annotationPanelWidget"));
        sizePolicy.setHeightForWidth(annotationPanelWidget->sizePolicy().hasHeightForWidth());
        annotationPanelWidget->setSizePolicy(sizePolicy);
        annotationPanelWidget->setMinimumSize(QSize(300, 0));
        annotationPanelWidget->setAutoFillBackground(false);
        verticalLayout_2 = new QVBoxLayout(annotationPanelWidget);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalWidget_2 = new QWidget(annotationPanelWidget);
        horizontalWidget_2->setObjectName(QString::fromUtf8("horizontalWidget_2"));
        horizontalLayout_2 = new QHBoxLayout(horizontalWidget_2);
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(horizontalWidget_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font);
        label_2->setMargin(5);

        horizontalLayout_2->addWidget(label_2);

        annotatedBranchTreeTitle = new QLabel(horizontalWidget_2);
        annotatedBranchTreeTitle->setObjectName(QString::fromUtf8("annotatedBranchTreeTitle"));
        annotatedBranchTreeTitle->setMinimumSize(QSize(0, 26));
        annotatedBranchTreeTitle->setMargin(5);

        horizontalLayout_2->addWidget(annotatedBranchTreeTitle);


        verticalLayout_2->addWidget(horizontalWidget_2);

        annotatedBranchTreeView = new AnnotatedBranchTreeView(annotationPanelWidget);
        annotatedBranchTreeView->setObjectName(QString::fromUtf8("annotatedBranchTreeView"));

        verticalLayout_2->addWidget(annotatedBranchTreeView);

        splitter->addWidget(annotationPanelWidget);

        verticalLayout->addWidget(splitter);


        retranslateUi(AnnotationWidget);

        QMetaObject::connectSlotsByName(AnnotationWidget);
    } // setupUi

    void retranslateUi(QWidget *AnnotationWidget)
    {
        AnnotationWidget->setWindowTitle(QApplication::translate("AnnotationWidget", "Form", 0, QApplication::UnicodeUTF8));
        consoleGroupBox->setTitle(QApplication::translate("AnnotationWidget", "Console Link", 0, QApplication::UnicodeUTF8));
        consoleLinkLabel->setText(QApplication::translate("AnnotationWidget", "Console not connected", 0, QApplication::UnicodeUTF8));
        consoleLinkButton->setText(QApplication::translate("AnnotationWidget", "Connect", 0, QApplication::UnicodeUTF8));
        consoleSyncLabel->setText(QApplication::translate("AnnotationWidget", "Console not synced", 0, QApplication::UnicodeUTF8));
        consoleSyncButton->setText(QApplication::translate("AnnotationWidget", "Synchronize", 0, QApplication::UnicodeUTF8));
        label_1->setText(QApplication::translate("AnnotationWidget", "Current Ontology:", 0, QApplication::UnicodeUTF8));
        ontologyTreeTitle->setText(QApplication::translate("AnnotationWidget", "None", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("AnnotationWidget", "Annotations:", 0, QApplication::UnicodeUTF8));
        annotatedBranchTreeTitle->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class AnnotationWidget: public Ui_AnnotationWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ANNOTATIONWIDGET_H
