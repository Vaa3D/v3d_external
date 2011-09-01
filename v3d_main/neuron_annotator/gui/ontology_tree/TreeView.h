#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

/*
 * Overriden to disable QTreeView's usual keyboard shortcuts
 */
class TreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit TreeView(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *event);

};

#endif // TREEVIEW_H
