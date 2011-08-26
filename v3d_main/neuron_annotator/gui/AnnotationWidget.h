#ifndef ANNOTATION_WIDGET_H_
#define ANNOTATION_WIDGET_H_

#include <QFrame>
#include <QThread>
#include "../entity_model/Entity.h"

namespace Ui {
    class AnnotationWidget;
}

class AnnotationWidget : public QFrame
{
    Q_OBJECT

public:
    explicit AnnotationWidget(QWidget *parent = 0);
    ~AnnotationWidget();

public slots:
    void setOntology(Entity *root);

private:
    Ui::AnnotationWidget *ui;
};

#endif // ANNOTATION_WIDGET_H_
