#include "AnnotatedBranchTreeView.h"
#include "AnnotatedBranchTreeModel.h"
#include "EntityTreeItem.h"
#include "../../entity_model/Entity.h"

#include <QAction>
#include <QMenu>

AnnotatedBranchTreeView::AnnotatedBranchTreeView(QWidget *parent) :
    EntityTreeView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    removeAnnotationAction = new QAction(tr("Delete"), this);
    connect(removeAnnotationAction, SIGNAL(triggered()), this, SLOT(removeAnnotation()));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint &)));
}

void AnnotatedBranchTreeView::showContextMenu(const QPoint& point)
{
    QList<QAction *> actions;
    QModelIndex idx = indexAt(point);
    if (idx.isValid()) {
        AnnotatedBranchTreeModel *treeModel = static_cast<AnnotatedBranchTreeModel *>(model());
        EntityTreeItem *item = treeModel->node(idx);
        if (item==0) return;
        Entity *entity = item->entity();
        if (entity==0) return;
        if (*entity->entityType != "Annotation") return;
        // TODO: only allow this action if the annotation is owned by the current user
        actions.append(removeAnnotationAction);
        QMenu::exec(actions, mapToGlobal(point));
    }
}

void AnnotatedBranchTreeView::removeAnnotation()
{
    AnnotatedBranchTreeModel *treeModel = static_cast<AnnotatedBranchTreeModel *>(model());
    EntityTreeItem *item = treeModel->node(currentIndex());
    if (item==0) return;
    Entity *entity = item->entity();
    if (entity==0) return;
    if (*entity->entityType != "Annotation") return;

    clearSelection(); // The entity will most likely get removed, so we can't be selecting it when that happens
    emit removeAnnotation(entity);
}

