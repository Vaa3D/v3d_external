#ifndef ONTOLOGYTREEMODEL_H
#define ONTOLOGYTREEMODEL_H

#include "EntityTreeModel.h"
#include <QHash>

class Entity;
class Ontology;
class EntityTreeItem;

class OntologyTreeModel : public EntityTreeModel
{
    Q_OBJECT

public:
    OntologyTreeModel(Ontology *ontology, QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role) const;

protected:
    void setupModelData(Entity *entity, EntityTreeItem *parent, QPersistentModelIndex &parentIndex);

private:
    QHash<qint64, QString> termKeyMap;
};

#endif // ONTOLOGYTREEMODEL_H
