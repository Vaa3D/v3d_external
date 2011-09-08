#include "OntologyTreeView.h"
#include <QtGui>

OntologyTreeView::OntologyTreeView(QWidget *parent) :
    QTreeView(parent)
{
}

void OntologyTreeView::keyPressEvent(QKeyEvent *event) {
    // Ignore all key presses so that they can be captured by AnnotationWidget's event listener
    event->ignore();
}
