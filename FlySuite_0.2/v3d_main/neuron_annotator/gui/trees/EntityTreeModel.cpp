#include "EntityTreeModel.h"
#include "EntityTreeItem.h"
#include "../../entity_model/Entity.h"

EntityTreeModel::EntityTreeModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

EntityTreeModel::~EntityTreeModel()
{
    delete rootItem;
}

int EntityTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<EntityTreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant EntityTreeModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    EntityTreeItem *item = static_cast<EntityTreeItem*>(index.internalPointer());

    if (role == Qt::DisplayRole)
    {
        return item->data(index.column());
    }

    return QVariant();
}

Qt::ItemFlags EntityTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant EntityTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex EntityTreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    EntityTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<EntityTreeItem*>(parent.internalPointer());

    EntityTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex EntityTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    EntityTreeItem *childItem = static_cast<EntityTreeItem*>(index.internalPointer());
    EntityTreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int EntityTreeModel::rowCount(const QModelIndex &parent) const
{
    EntityTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<EntityTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

QModelIndex EntityTreeModel::indexForId(const qint64 id) const
{
    return termIndexMap.value(id, QModelIndex());
}

EntityTreeItem* EntityTreeModel::node(const QModelIndex &index) const
{
    EntityTreeItem *item = static_cast<EntityTreeItem*>(index.internalPointer());
    return item;
}

EntityTreeItem* EntityTreeModel::node(const qint64 id) const
{
    QModelIndex index = indexForId(id);
    EntityTreeItem *item = static_cast<EntityTreeItem*>(index.internalPointer());
    return item;
}

