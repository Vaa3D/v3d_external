#ifndef ENTITY_H
#define ENTITY_H

#include "EntityData.h"
#include <QtCore>

class Entity
{
public:
    qint64 *id;
    QString *name;
    QString *user;
    QString *entityStatus;
    QString *entityType;
    QSet<EntityData *> entityDataSet;
public:
    Entity();
    ~Entity(); // Recursively deletes all ancestors in the entity tree
    QList<EntityData *> getOrderedEntityData();
    EntityData* getEntityDataByAttributeName(const QString & attrName) const;
    const QString& getValueByAttributeName(const QString & attrName) const;
    void dumpEntity();
    void dumpEntity(int level);
};


#endif // ENTITY_H
