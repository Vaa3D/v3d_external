#ifndef ONTOLOGYTREEVIEW_H
#define ONTOLOGYTREEVIEW_H

#include <QTreeView>

class OntologyTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit OntologyTreeView(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *event);

};

#endif // ONTOLOGYTREEVIEW_H
