#include "EntityTreeView.h"
#include "EntityTreeModel.h"
#include "../../entity_model/Entity.h"
#include <QtGui>

EntityTreeView::EntityTreeView(QWidget *parent) :
    QTreeView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

void EntityTreeView::keyPressEvent(QKeyEvent *event) {
    // Ignore all key presses so that they can be captured by AnnotationWidget's event listener
    event->ignore();
}

void EntityTreeView::selectEntity(const qint64 & entityId)
{
    // Get the indexes
    EntityTreeModel *treeModel = static_cast<EntityTreeModel *>(model());
    QModelIndex termIndex = treeModel->indexForId(entityId);
    QModelIndex beginIndex = treeModel->index(termIndex.row(), 0, termIndex.parent());
    QModelIndex endIndex = treeModel->index(termIndex.row(), treeModel->columnCount() - 1, termIndex.parent());

    // Select the indexes
    QItemSelectionModel *selection = selectionModel();
    selection->clearSelection();
    selection->select(QItemSelection(beginIndex, endIndex), QItemSelectionModel::Select);

    // Expand to the node
    expandTo(termIndex);
    scrollTo(termIndex);
}

void EntityTreeView::expandTo(const QModelIndex &index)
{
    EntityTreeModel *treeModel = static_cast<EntityTreeModel *>(model());

    QModelIndex curr = index;
    while (curr.isValid()) {
        setExpanded(model()->index(curr.row(), 0, treeModel->parent(curr)), true);
        curr = treeModel->parent(curr);
    }
}
