#ifndef ENTITY_H
#define ENTITY_H

#include "EntityData.h"
#include <QtCore>

//
// An entity in the JACS system. May be part of an entity tree.
//
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
    QList<EntityData *> getOrderedEntityData() const;
    EntityData* getEntityDataByAttributeName(const QString & attrName) const;
    const QString& getValueByAttributeName(const QString & attrName) const;
    void dumpEntity() const;
    void dumpEntity(int level) const;
};


#endif // ENTITY_H
