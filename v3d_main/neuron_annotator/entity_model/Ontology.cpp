#include "Ontology.h"
#include "Entity.h"
#include <QtGui>

void Ontology::populateMaps(Entity *entity)
{
    if (entity == NULL) return;
    _termMap->insert(*entity->id, entity);

    QSet<EntityData *>::const_iterator i;
    for (i = entity->entityDataSet.begin(); i != entity->entityDataSet.end(); ++i)
    {
        EntityData *data = *i;
        if (data->childEntity != NULL)
        {
            _parentMap->insert(*data->childEntity->id, entity);
            populateMaps(data->childEntity);
        }
    }
}

Ontology::Ontology(Entity *root, QMap<QKeySequence, qint64> *keyBindMap) :
    _root(root),
    _keyBindMap(keyBindMap)
{
    _termMap = new QHash<qint64, Entity*>;
    _parentMap = new QHash<qint64, Entity*>;
    populateMaps(_root);
}

Ontology::~Ontology()
{
    qDebug() << "Destroying ontology"<<*_root->name;
    if (_root != NULL) delete _root;
    if (_keyBindMap != NULL) delete _keyBindMap;
    if (_termMap != NULL) delete _termMap;
    if (_parentMap != NULL) delete _parentMap;
}
