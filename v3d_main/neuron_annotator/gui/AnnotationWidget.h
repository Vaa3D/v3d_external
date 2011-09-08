#ifndef ANNOTATION_WIDGET_H_
#define ANNOTATION_WIDGET_H_

#include <QFrame>
#include <QEvent>

class Entity;
class Ontology;
class OntologyTreeModel;
class ConsoleObserver;
class NaMainWindow;
class QModelIndex;

namespace obs {
    class ConsoleObserverServiceImpl;
}

namespace Ui {
    class AnnotationWidget;
}

class AnnotationWidget : public QFrame
{
    Q_OBJECT

public:
    explicit AnnotationWidget(QWidget *parent = 0);
    ~AnnotationWidget();
    void setMainWindow(NaMainWindow *mainWindow);

public slots:
    void setOntology(Ontology *ontology);
    void consoleConnect();
    void consoleDisconnect();
    void ontologyTreeDoubleClicked(const QModelIndex & index);
    void annotateSelectedItemWithOntologyTerm(const Entity *ontologyTerm);

protected:
    bool eventFilter (QObject* watched_object, QEvent* e);
    void showOntologyError(const QString & text);
    void showErrorDialog(const QString & text);
    void showDisconnected();

private:
    Ontology *ontology;
    OntologyTreeModel *ontologyTreeModel;
    Ui::AnnotationWidget *ui;
    obs::ConsoleObserverServiceImpl *consoleObserverService;
    ConsoleObserver *consoleObserver;
    NaMainWindow *naMainWindow;
    bool firstLoad;
};

#endif // ANNOTATION_WIDGET_H_
