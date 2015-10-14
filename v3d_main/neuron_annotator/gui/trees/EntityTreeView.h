#ifndef ENTITYTREEVIEW_H
#define ENTITYTREEVIEW_H

#include <QTreeView>

class Entity;

class EntityTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit EntityTreeView(QWidget *parent = 0);
    void selectEntity(const qint64 & entityId);
    void expandTo(const QModelIndex &index);

protected:
    void keyPressEvent(QKeyEvent *event);

};

#endif // ENTITYTREEVIEW_H
