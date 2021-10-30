#include "AnnotatedBranch.h"
#include "Entity.h"
#include "../utility/JacsUtil.h"

AnnotatedBranch::AnnotatedBranch(Entity *entity, QHash<qint64, AnnotationList*> *annotationMap, UserColorMap *userColorMap) :
    _entity(entity), _annotationMap(annotationMap), _userColorMap(userColorMap)
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
        while (i.hasNext())
        {
            i.next();
            QList<Entity *>* alist = i.value();
            if (alist != 0)
            {
                qDeleteAll(*alist);
                delete alist;
            }
        }
    }

    // Delete color map
    if (_annotationMap != 0)
        delete _annotationMap;
}

AnnotationList* AnnotatedBranch::getAnnotations(const qint64 & entityId) const
{
    return _annotationMap->value(entityId);
}

QColor AnnotatedBranch::getUserColor(const QString & username) const
{
    return _userColorMap->value(username, QColor(Qt::white));
}

void AnnotatedBranch::updateAnnotations(const qint64 & entityId, AnnotationList* annotations, UserColorMap *userColorMap)
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

    // Insert all user colors, overriding any old ones
    QHashIterator<QString, QColor> i(*userColorMap);
    while (i.hasNext())
    {
        i.next();
        _userColorMap->insert(i.key(), i.value());
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
        return filepath;
    }
    return QString();
}

QString AnnotatedBranch::getVisuallyLosslessImage() const
{
    QString filepath = _entity->getValueByAttributeName("Visually Lossless Image");
    if (!filepath.isEmpty()) {
        return filepath;
    }
    return QString();
}

QString AnnotatedBranch::getLosslessImage() const
{
    QString filepath = _entity->getValueByAttributeName("Lossless Image");
    if (!filepath.isEmpty()) {
        return filepath;
    }
    return QString();
}

QString AnnotatedBranch::getChannelSpecification() const
{
    QString filepath = _entity->getValueByAttributeName("Channel Specification");
    if (!filepath.isEmpty()) {
        return filepath;
    }
    return QString();
}

Entity * AnnotatedBranch::findEntityById(Entity *entity, const qint64 & entityId) const
{
    if (entity==0) return 0;
    if (*entity->id == entityId) return entity;

    QSetIterator<EntityData *> i(entity->entityDataSet);
    while (i.hasNext())
    {
        EntityData *ed = i.next();
        Entity *child = ed->childEntity;
        Entity *entity = findEntityById(child, entityId);
        if (entity!=0) return entity;
    }
    return 0;
}

Entity * AnnotatedBranch::getEntityById(const qint64 & entityId) const
{
    return findEntityById(_entity, entityId);
}
