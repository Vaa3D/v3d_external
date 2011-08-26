#include "EntityAdapter.h"

#include <QtGui>;

Entity* convert(cds::fw__entity *fwEntity)
{
    Entity *entity = new Entity();

    if (fwEntity->id != NULL) entity->id = new qint64(*fwEntity->id);
    if (fwEntity->name != NULL) entity->name = new QString(fwEntity->name->c_str());
    if (fwEntity->entityType != NULL) entity->entityType = new QString(fwEntity->entityType->c_str());
    if (fwEntity->entityStatus != NULL) entity->entityStatus = new QString(fwEntity->entityStatus->c_str());

    if (fwEntity->entityDataSet != NULL)
    {
        entity->entityDataSet.clear();
        std::vector<class cds::fw__entityData *> children = fwEntity->entityDataSet->entityData;

        for (int c = 0; c < children.size(); ++c)
        {
            cds::fw__entityData *childED = children[c];
            EntityData *entityData = new EntityData();

            if (childED->id != NULL) entityData->id = new qint64(*childED->id);
            if (childED->orderIndex != NULL) entityData->orderIndex = new qint64(*childED->orderIndex);
            if (childED->entityAttribute != NULL) entityData->attributeName = new QString(childED->entityAttribute->c_str());
            if (childED->value != NULL) entityData->value = new QString(childED->value->c_str());
            if (childED->user != NULL) entityData->user = new QString(childED->user->c_str());

            entityData->parentEntity = entity;

            if (childED->childEntity != NULL)
            {
                // Recursively convert children
                entityData->childEntity = convert(childED->childEntity);
            }

            entity->entityDataSet.insert(entityData);
        }

    }
    return entity;
}
