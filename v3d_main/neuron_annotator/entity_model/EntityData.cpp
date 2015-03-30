#include "Entity.h"
#include <QtGui>

EntityData::EntityData() : id(0), orderIndex(0), attributeName(0), user(0), value(0), childEntity(0), parentEntity(0)
{
}

EntityData::~EntityData()
{
//    qDebug() << "delete entity data "<<id<<attributeName;
    if (id != 0) delete id;
    if (orderIndex != 0) delete orderIndex;
    if (attributeName != 0) delete attributeName;
    if (user != 0) delete user;
    if (value != 0) delete value;
    // this will recursively delete all the ancestor entities
    if (childEntity != 0) delete childEntity;
    // parentEntity is just a back pointer that we are not responsible for here
}

void EntityData::dumpEntityData(int level)
{
    std::string ind = std::string(level*4, ' ');
    const char * indent = ind.c_str();

    if (id != 0) qDebug("%sEntityData.id: %lld",indent,*id);
    if (orderIndex != 0) qDebug("%sOrder Index: %lld",indent,*orderIndex);
    if (attributeName != 0) qDebug("%sAttribute: %s",indent,attributeName->toUtf8().constData());
    if (user != 0) qDebug("%sUser: %s",indent,user->toUtf8().constData());
    if (value != 0) qDebug("%sValue: %s",indent,value->toUtf8().constData());
    if (childEntity != 0)
    {
        childEntity->dumpEntity(level+1);
    }

}
