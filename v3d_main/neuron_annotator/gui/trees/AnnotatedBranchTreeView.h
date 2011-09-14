#ifndef ANNOTATEDBRANCHTREEVIEW_H
#define ANNOTATEDBRANCHTREEVIEW_H

#include <QtGui>
#include "EntityTreeView.h"

class AnnotatedBranchTreeView : public EntityTreeView
{
    Q_OBJECT
public:
    explicit AnnotatedBranchTreeView(QWidget *parent = 0);

signals:
    void removeAnnotation(const Entity *annotation);

private slots:
    void showContextMenu(const QPoint& pnt);
    void removeAnnotation();

private:
    QAction *removeAnnotationAction;

};

#endif // ANNOTATEDBRANCHTREEVIEW_H
