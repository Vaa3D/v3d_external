#include "TreeView.h"
#include <QtGui>

TreeView::TreeView(QWidget *parent) :
    QTreeView(parent)
{
}

void TreeView::keyPressEvent(QKeyEvent *event) {
    // Ignore all key presses so that they can be captured by AnnotationWidget's event listener
    event->ignore();
}
