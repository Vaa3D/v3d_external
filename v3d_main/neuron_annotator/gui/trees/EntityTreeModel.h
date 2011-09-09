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

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual EntityTreeItem* node(const QModelIndex &index) const;
    virtual QModelIndex indexForId(const qint64) const;

protected:
    EntityTreeItem *rootItem;
    QHash<qint64, QPersistentModelIndex> termIndexMap;

};

#endif // ENTITYTREEMODEL_H
