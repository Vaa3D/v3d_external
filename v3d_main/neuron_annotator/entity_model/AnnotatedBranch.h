#ifndef ANNOTATEDBRANCH_H
#define ANNOTATEDBRANCH_H

#include <QtCore>
#include <QHash>

class Entity;
typedef QList<Entity *> AnnotationList;

//
// Combines an Entity subtree (i.e. branch) with access to annotations for all the entities in the subtree.
//
class AnnotatedBranch
{

public:
    AnnotatedBranch(Entity *entity, QHash<qint64, AnnotationList*> *annotationMap);
    ~AnnotatedBranch(); // Recursively deletes all AnnotatedBranch data including entities and annotations
    AnnotationList* getAnnotations(const Entity *entity) const;
    void updateAnnotations(const qint64 entityId, AnnotationList* annotations);
    QString getFilePath() const;
    QString name() const;
    inline Entity *entity() const { return _entity; }

private:
    Entity *_entity;
    QHash<qint64, AnnotationList*> *_annotationMap;
};

#endif // ANNOTATEDBRANCH_H
