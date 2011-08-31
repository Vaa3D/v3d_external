#ifndef ANNOTATION_WIDGET_H_
#define ANNOTATION_WIDGET_H_

#include <QFrame>
#include <QThread>
#include <QEvent>

class Ontology;

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
    void setOntology(Ontology *ontology);

protected:
    bool eventFilter (QObject* watched_object, QEvent* e);

private:
    Ontology *ontology;
    Ui::AnnotationWidget *ui;
};

#endif // ANNOTATION_WIDGET_H_
