#include <QStringList>
#include "EntityTreeItem.h"
#include <QtGui>

EntityTreeItem::EntityTreeItem(const QList<QVariant> &data, Entity *entity, EntityTreeItem *parent) :
    parentItem(parent),
    itemData(data),
    _entity(entity)
{
}

EntityTreeItem::~EntityTreeItem()
{
    qDeleteAll(childItems);
}

void EntityTreeItem::appendChild(EntityTreeItem *item)
{
    childItems.append(item);
}

EntityTreeItem *EntityTreeItem::child(int row)
{
    return childItems.value(row);
}

int EntityTreeItem::childCount() const
{
    return childItems.count();
}

int EntityTreeItem::columnCount() const
{
    return itemData.count();
}

QVariant EntityTreeItem::data(int column) const
{
    return itemData.value(column);
}

EntityTreeItem *EntityTreeItem::parent()
{
    return parentItem;
}

int EntityTreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<EntityTreeItem*>(this));

    return 0;
}
