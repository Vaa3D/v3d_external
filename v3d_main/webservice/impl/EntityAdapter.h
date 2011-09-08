#ifndef ENTITYADAPTER_H
#define ENTITYADAPTER_H

#include "../../neuron_annotator/entity_model/Entity.h"
#include "../console/cdsStub.h"
#include <QHash>

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
    static Entity* convert(cds::fw__entity *entity);
    static QMap<QKeySequence, qint64>* convert(cds::fw__ontologyKeyBindings *keyBindings);

};

#endif // ENTITYADAPTER_H
