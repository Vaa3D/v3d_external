#include "AnnotatedBranch.h"
#include "Entity.h"
#include "../utility/JacsUtil.h"

AnnotatedBranch::AnnotatedBranch(Entity *entity, QHash<qint64, AnnotationList*> *annotationMap) :
    _entity(entity), _annotationMap(annotationMap)
{
}

AnnotatedBranch::~AnnotatedBranch()
{
    // Delete the entity tree
    if (_entity != 0) delete _entity;

    // Delete all annotations
    if (_annotationMap != 0)
    {
        QHashIterator<qint64, AnnotationList*> i(*_annotationMap);
        while (i.hasNext()) {
            i.next();
            QList<Entity *>* alist = i.value();
            if (alist != 0)
            {
                qDeleteAll(*alist);
                delete alist;
            }
        }
    }
}

QString AnnotatedBranch::name() const
{
    return (_entity->name == 0) ? "" : *_entity->name;
}

QString AnnotatedBranch::getFilePath() const
{
    QString filepath = _entity->getValueByAttributeName("File Path");
    if (!filepath.isEmpty()) {
        QString macPath = convertPathToMac(filepath);
        return macPath;
    }
    return QString();
}

AnnotationList* AnnotatedBranch::getAnnotations(const Entity *entity) const
{
    return _annotationMap->value(*entity->id);
}

void AnnotatedBranch::updateAnnotations(const qint64 entityId, AnnotationList* annotations)
{
    if (_annotationMap->contains(entityId))
    {
        // Clean up memory from previous annotations
        QList<Entity *>* alist = _annotationMap->value(entityId);
        if (alist != 0)
        {
            qDeleteAll(*alist);
            delete alist;
        }
    }
    _annotationMap->insert(entityId, annotations);
}
