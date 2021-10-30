#ifndef ONTOLOGYANNOTATION_H
#define ONTOLOGYANNOTATION_H

#include <QtCore>

class Entity;

class OntologyAnnotation
{
public:
    explicit OntologyAnnotation();
    explicit OntologyAnnotation(Entity *entity);
    ~OntologyAnnotation();

public:
    Entity *annotationEntity;
    qint64 *sessionId;
    qint64 *targetEntityId;
    qint64 *keyEntityId;
    QString *keyString;
    qint64 *valueEntityId;
    QString *valueString;
};

#endif // ONTOLOGYANNOTATION_H
