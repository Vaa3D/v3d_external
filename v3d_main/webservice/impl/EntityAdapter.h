#ifndef ENTITYADAPTER_H
#define ENTITYADAPTER_H

#include "../neuron_annotator/entity_model/Entity.h"
#include "../../webservice/console/cdsStub.h"

Entity* convert(cds::fw__entity *entity);

#endif // ENTITYADAPTER_H
