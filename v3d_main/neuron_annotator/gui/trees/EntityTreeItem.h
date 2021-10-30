#ifndef ENTITYTREEITEM_H
#define ENTITYTREEITEM_H

#include <QList>
#include <QVariant>

class Entity;

class EntityTreeItem
{
public:
    EntityTreeItem(const QList<QVariant> &data, Entity *entity, EntityTreeItem *parent = 0);
    ~EntityTreeItem();

    void appendChild(EntityTreeItem *child);

    EntityTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    EntityTreeItem *parent();
    inline Entity *entity() const { return _entity; }

private:
    QList<EntityTreeItem*> childItems;
    QList<QVariant> itemData;
    EntityTreeItem *parentItem;
    Entity *_entity;
};

#endif // ENTITYTREEITEM_H
