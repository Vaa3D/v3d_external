#ifndef ANNOTATIONTREEMODEL_H
#define ANNOTATIONTREEMODEL_H

#include "EntityTreeModel.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QHash>

class Entity;
class EntityTreeItem;

class AnnotationTreeModel : public EntityTreeModel
{
    Q_OBJECT

public:
    AnnotationTreeModel(Entity *entity, QObject *parent = 0);

protected:
    void setupModelData(Entity *entity, EntityTreeItem *parent, QPersistentModelIndex & parentIndex);

};

#endif // ANNOTATIONTREEMODEL_H
