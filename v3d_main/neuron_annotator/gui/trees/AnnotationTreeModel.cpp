#include "AnnotationTreeModel.h"
#include "EntityTreeItem.h"
#include "../../entity_model/Entity.h"
#include <QtGui>

AnnotationTreeModel::AnnotationTreeModel(Entity *entity, QObject *parent)
    : EntityTreeModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Name" << "Type";
    rootItem = new EntityTreeItem(rootData, 0);
    if (entity == NULL) return;

    QPersistentModelIndex rootIndex = QPersistentModelIndex();
    setupModelData(entity, rootItem, rootIndex);
}

void AnnotationTreeModel::setupModelData(Entity *entity, EntityTreeItem *parent, QPersistentModelIndex & parentIndex)
{

    QList<QVariant> columnData;
    columnData << *entity->name << *entity->entityType;

    EntityTreeItem *item = new EntityTreeItem(columnData, entity, parent);
    parent->appendChild(item);

    QPersistentModelIndex modelIndex = QPersistentModelIndex(index(item->row(), 0, parentIndex));
    termIndexMap.insert(*entity->id, modelIndex);

    if (!entity->entityDataSet.isEmpty())
    {
        QList<EntityData *> list = entity->getOrderedEntityData();
        QList<EntityData *>::const_iterator i;
        for (i = list.begin(); i != list.end(); ++i)
        {
            EntityData *data = *i;
            if (data->childEntity != NULL)
            {
                setupModelData(data->childEntity, item, modelIndex);
            }
        }
    }
}
