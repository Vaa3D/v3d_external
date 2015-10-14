#ifndef ANNOTATEDBRANCHTREEMODEL_H
#define ANNOTATEDBRANCHTREEMODEL_H

#include "EntityTreeModel.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QHash>

class AnnotatedBranch;
class EntityTreeItem;
class Entity;

class AnnotatedBranchTreeModel : public EntityTreeModel
{
    Q_OBJECT

public:
    AnnotatedBranchTreeModel(AnnotatedBranch *annotatedBranch, QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role) const;

protected:
    void setupModelData(Entity *entity, EntityTreeItem *parent, QPersistentModelIndex & parentIndex);

private:
    AnnotatedBranch *annotatedBranch;

};

#endif // ANNOTATEDBRANCHTREEMODEL_H
