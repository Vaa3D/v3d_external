#include "OntologyAnnotation.h"
#include "Entity.h"

OntologyAnnotation::OntologyAnnotation() : annotationEntity(0),
    sessionId(0), targetEntityId(0), keyEntityId(0), keyString(0), valueEntityId(0), valueString(0)
{
}

OntologyAnnotation::OntologyAnnotation(Entity *entity) : annotationEntity(entity),
    sessionId(0), targetEntityId(0), keyEntityId(0), keyString(0), valueEntityId(0), valueString(0)
{
}

OntologyAnnotation::~OntologyAnnotation()
{
    if (annotationEntity != 0) delete annotationEntity;
    if (sessionId != 0) delete sessionId;
    if (targetEntityId != 0) delete targetEntityId;
    if (keyEntityId != 0) delete keyEntityId;
    if (keyString != 0) delete keyString;
    if (valueEntityId != 0) delete valueEntityId;
    if (valueString != 0) delete valueString;
}

