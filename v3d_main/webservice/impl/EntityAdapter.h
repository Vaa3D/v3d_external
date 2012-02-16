#ifndef ENTITYADAPTER_H
#define ENTITYADAPTER_H

#include "../../neuron_annotator/entity_model/Entity.h"
#include "../../neuron_annotator/entity_model/OntologyAnnotation.h"
#include "../../neuron_annotator/entity_model/AnnotationSession.h"
#include "../console/cdsStub.h"
#include <QHash>
#include <QMap>

//
// This class converts between GSOAP-generated classes (which are unstable) and
// the Entity model (which is stable and used throughout Neuron Annotator).
//
// It may also do Java to Qt conversions for some things, such as keybinds.
//
class EntityAdapter
{
private:
    QHash<QString,QString> keyNameMap_Java2Qt;
    QHash<QString,QString> shiftKeyNameMap_Java2Qt;

private:
    EntityAdapter(); // This class is a singleton because it caches some lookup tables
    QString convertKeyBind(QString & javaKeyBind);

public:
    static EntityAdapter &get(); // Get the singleton
    static EntityData* convert(cds::fw__entityData *fwEntityData, Entity *parent);
    static Entity* convert(cds::fw__entity *fwEntity);
    static QList<Entity *>* convert(cds::fw__entityArray *array);
    static QList<EntityData *>* convert(cds::fw__entityDataArray *array);
    static QMap<QKeySequence, qint64>* convert(cds::fw__ontologyKeyBindings *keyBindings);
    static OntologyAnnotation* convert(cds::fw__ontologyAnnotation *fwAnnotation);
    static cds::fw__ontologyAnnotation* convert(OntologyAnnotation* annotation);
    static AnnotationSession* convert(cds::fw__annotationSession *fwAnnotationSession);

};

#endif // ENTITYADAPTER_H
