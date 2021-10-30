#ifndef ENTITYDATA_H
#define ENTITYDATA_H

#include <QtCore>

class Entity;

//
// The link between two entities in a parent/child relationship.
//
class EntityData
{
public:
    qint64 *id;
    qint64 *orderIndex;
    QString *attributeName;
    QString *user;
    QString *value;
    Entity *childEntity;
    Entity *parentEntity;

public:
    EntityData();
    ~EntityData();
    void dumpEntityData(int level);

};

#endif // ENTITYDATA_H
