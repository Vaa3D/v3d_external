#include "OntologyTreeModel.h"
#include "EntityTreeItem.h"
#include "../../entity_model/Entity.h"
#include "../../entity_model/Ontology.h"
#include "../../utility/Icons.h"
#include <QtGui>

OntologyTreeModel::OntologyTreeModel(Ontology *ontology, QObject *parent)
    : EntityTreeModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Term" << "Keybind";
    rootItem = new EntityTreeItem(rootData, 0);
    if (ontology == NULL) return;

    QPersistentModelIndex rootIndex = QPersistentModelIndex();

    QMap<QKeySequence, qint64>::const_iterator i = ontology->keyBindMap()->constBegin();
    while (i != ontology->keyBindMap()->constEnd())
    {
        if (termKeyMap.contains(i.value()))
        {
            qDebug() << "Ontology term "<<i.value()<< "is already mapped to a key";
        }
        else
        {
            QKeySequence keySeq(i.key());
            QString keySeqLabel(keySeq.toString(QKeySequence::NativeText));
            termKeyMap.insert(i.value(), keySeqLabel);
        }
        ++i;
    }

    setupModelData(ontology->root(), rootItem, rootIndex);
}

void OntologyTreeModel::setupModelData(Entity *entity, EntityTreeItem *parent, QPersistentModelIndex & parentIndex)
{

    QString keybind;
    if (termKeyMap.contains(*entity->id))
    {
        keybind = termKeyMap.value(*entity->id, "");
    }

    QList<QVariant> columnData;
    columnData << *entity->name << keybind;

    EntityTreeItem *item = new EntityTreeItem(columnData, entity, parent);
    parent->appendChild(item);

    QPersistentModelIndex modelIndex = QPersistentModelIndex(index(item->row(), 0, parentIndex));
    termIndexMap.insert(*entity->id, modelIndex);

    if (!entity->entityDataSet.isEmpty())
    {
        QList<EntityData *> list = entity->getOrderedEntityData();
        QList<EntityData *>::const_iterator i;
        for (i = list.begin(); i != list.end(); ++i)
        {
            EntityData *data = *i;
            if (data->childEntity != NULL)
            {
                setupModelData(data->childEntity, item, modelIndex);
            }
        }
    }
}

QVariant OntologyTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    EntityTreeItem *item = static_cast<EntityTreeItem*>(index.internalPointer());

    if (role == Qt::DecorationRole)
    {
        if (index.column()==1) return QVariant(); // No icons in the keybind column
        return Icons::getIcon(item->entity());
    }

    if (role == Qt::DisplayRole)
        return item->data(index.column());

    return QVariant();
}


