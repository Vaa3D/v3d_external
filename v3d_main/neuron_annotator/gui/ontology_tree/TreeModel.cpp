#include "TreeModel.h"
#include "TreeItem.h"
#include "../../entity_model/Entity.h"
#include "../../entity_model/Ontology.h"
#include <QtGui>

TreeModel::TreeModel(Ontology *ontology, QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Term" << "Keybind";
    rootItem = new TreeItem(rootData);
    if (ontology == NULL) return;

    QMap<QKeySequence, qint64>::const_iterator i = ontology->keyBindMap()->constBegin();
    while (i != ontology->keyBindMap()->constEnd()) {
        if (termKeyMap.contains(i.value())) {
            qDebug() << "Ontology term "<<i.value()<< "is already mapped to a key";
        }
        else {
            QKeySequence keySeq(i.key());
            QString keySeqLabel(keySeq.toString(QKeySequence::NativeText));
            termKeyMap.insert(i.value(), keySeqLabel);
        }
        ++i;
    }

    setupModelData(ontology->root(), rootItem);
}

TreeModel::~TreeModel()
{
    qDebug() << "Delete existing tree";
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void TreeModel::setupModelData(Entity *entity, TreeItem *parent)
{

    QString keybind;
    if (termKeyMap.contains(*entity->id)) {
        keybind = termKeyMap.value(*entity->id, "");
    }

    QList<QVariant> columnData;
    columnData << *entity->name << keybind;

    TreeItem *item = new TreeItem(columnData, parent);
    parent->appendChild(item);

    if (!entity->entityDataSet.isEmpty()) {
        QList<EntityData *> list = entity->getOrderedEntityData();
        QList<EntityData *>::const_iterator i;
        for (i = list.begin(); i != list.end(); ++i)
        {
            EntityData *data = *i;
            if (data->childEntity != NULL) {
                setupModelData(data->childEntity, item);
            }
        }
    }
}

