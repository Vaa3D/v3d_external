#include "AnnotatedBranchTreeModel.h"
#include "EntityTreeItem.h"
#include "../../entity_model/Entity.h"
#include "../../entity_model/AnnotatedBranch.h"
#include "../../utility/JacsUtil.h"
#include <QtGui>

AnnotatedBranchTreeModel::AnnotatedBranchTreeModel(AnnotatedBranch *annotatedBranch, QObject *parent)
    : EntityTreeModel(parent), annotatedBranch(annotatedBranch)
{
    QList<QVariant> rootData;
    rootData << "Name";
    rootItem = new EntityTreeItem(rootData, 0);
    if (annotatedBranch == NULL) return;

    QPersistentModelIndex rootIndex = QPersistentModelIndex();
    setupModelData(annotatedBranch->entity(), rootItem, rootIndex);
}

void AnnotatedBranchTreeModel::setupModelData(Entity *entity, EntityTreeItem *parent, QPersistentModelIndex & parentIndex)
{

    if (entity == NULL || entity->name == NULL|| entity->id == NULL) {
        return;
    }

    QList<QVariant> columnData;
    columnData << *entity->name;

    EntityTreeItem *item = new EntityTreeItem(columnData, entity, parent);
    parent->appendChild(item);

    QPersistentModelIndex modelIndex = QPersistentModelIndex(index(item->row(), 0, parentIndex));
    termIndexMap.insert(*entity->id, modelIndex);

    AnnotationList *annotations = annotatedBranch->getAnnotations(*entity->id);
    if (annotations != NULL && !annotations->isEmpty())
    {
        // TODO: Create annotation subnode?

        QListIterator<Entity *> i(*annotations);
        while (i.hasNext())
        {
            Entity *annot = i.next();
            setupModelData(annot, item, modelIndex);
        }
    }

    if (!entity->entityDataSet.isEmpty())
    {
        QListIterator<EntityData *> i(entity->getOrderedEntityData());
        while (i.hasNext())
        {
            EntityData *ed = i.next();
            setupModelData(ed->childEntity, item, modelIndex);
        }
    }
}

QVariant AnnotatedBranchTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    EntityTreeItem *item = static_cast<EntityTreeItem*>(index.internalPointer());

    if (role == Qt::DecorationRole)
    {
        return getIcon(item->entity());
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->data(index.column());
}


