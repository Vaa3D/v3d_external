#include "EntityTreeView.h"
#include <QtGui>

EntityTreeView::EntityTreeView(QWidget *parent) :
    QTreeView(parent)
{
}

void EntityTreeView::keyPressEvent(QKeyEvent *event) {
    // Ignore all key presses so that they can be captured by AnnotationWidget's event listener
    event->ignore();
}
