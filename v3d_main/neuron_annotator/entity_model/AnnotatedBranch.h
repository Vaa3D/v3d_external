#ifndef ANNOTATEDBRANCH_H
#define ANNOTATEDBRANCH_H

#include <QtCore>
#include <QHash>
#include <QColor>

class Entity;
typedef QList<Entity *> AnnotationList;
typedef QHash<QString, QColor> UserColorMap;

//
// Combines an Entity subtree (i.e. branch) with access to annotations for all the entities in the subtree.
//
class AnnotatedBranch
{
public:
    AnnotatedBranch(Entity *entity, QHash<qint64, AnnotationList*> *annotationMap, UserColorMap *userColorMap);
    ~AnnotatedBranch(); // Recursively deletes all AnnotatedBranch data including entities and annotations
    AnnotationList *getAnnotations(const Entity *entity) const;
    AnnotationList *getAnnotations(const qint64 & entityId) const;
    QColor getUserColor(const QString & username) const;
    void updateAnnotations(const qint64 & entityId, AnnotationList* annotations, UserColorMap *userColorMap);
    QString getFilePath() const;
    QString getVisuallyLosslessImage() const;
    QString getLosslessImage() const;
    QString getChannelSpecification() const;
    QString name() const;
    Entity* getEntityById(const qint64 & entityId) const;
    inline Entity *entity() const { return _entity; }

private:
    Entity *findEntityById(Entity *entity, const qint64 & entityId) const;

private:
    Entity *_entity;
    QHash<qint64, AnnotationList*> *_annotationMap;
    UserColorMap *_userColorMap;
};


#endif // ANNOTATEDBRANCH_H
