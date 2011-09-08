#ifndef ENTITYTREEMODEL_H
#define ENTITYTREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QHash>

class EntityTreeItem;

class EntityTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    EntityTreeModel(QObject *parent = 0);
    virtual ~EntityTreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    EntityTreeItem* node(const QModelIndex &index) const;
    QModelIndex indexForId(const qint64) const;

protected:
    EntityTreeItem *rootItem;
    QHash<qint64, QPersistentModelIndex> termIndexMap;

};

#endif // ENTITYTREEMODEL_H
