#include "../../v3d/v3d_core.h"
#include "AnnotationWidget.h"
#include "ui_AnnotationWidget.h"
#include "ontology_tree/TreeModel.h"
#include <QApplication>

AnnotationWidget::AnnotationWidget(QWidget *parent) : QFrame(parent), ui(new Ui::AnnotationWidget)
{
    ui->setupUi(this);
    ui->title->setText(QString("Loading..."));
}

void AnnotationWidget::setOntology(Entity *root) {
    if (root->name != NULL) ui->title->setText(*root->name);
    TreeModel *treeModel = new TreeModel(root);
    ui->treeView->setModel(treeModel);
    ui->treeView->setVisible(false);
    ui->treeView->expandAll();
    ui->treeView->resizeColumnToContents(0);
    ui->treeView->setVisible(true);
}

AnnotationWidget::~AnnotationWidget()
{
    delete ui;
}
