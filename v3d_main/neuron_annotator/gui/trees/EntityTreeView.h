#ifndef ENTITYTREEVIEW_H
#define ENTITYTREEVIEW_H

#include <QTreeView>

/*
 * Overriden to disable QTreeView's usual keyboard shortcuts
 */
class EntityTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit EntityTreeView(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *event);

};

#endif // ENTITYTREEVIEW_H
