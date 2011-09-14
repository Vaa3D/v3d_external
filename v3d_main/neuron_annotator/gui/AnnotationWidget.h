#ifndef ANNOTATION_WIDGET_H_
#define ANNOTATION_WIDGET_H_

#include "../entity_model/AnnotatedBranch.h"
#include <QFrame>
#include <QEvent>
#include <QMutex>

class Entity;
class Ontology;
class OntologyTreeModel;
class AnnotatedBranch;
class AnnotatedBranchTreeModel;
class ConsoleObserver;
class DataThread;
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

signals:
    void entitySelected(const Entity *entity); // Parameter may be null if the selection is cleared

public slots:
    void closeOntology();
    void openOntology(Ontology *ontology);
    void closeAnnotatedBranch();
    void openAnnotatedBranch(AnnotatedBranch *annotatedBranch, bool openStack = true);
    void updateAnnotations(long entityId, AnnotationList *annotations);
    void communicationError(const QString & errorMessage);
    void consoleConnect();
    void consoleDisconnect();
    void ontologyTreeDoubleClicked(const QModelIndex & index);
    void annotatedBranchTreeClicked(const QModelIndex & index);
    void annotateSelectedEntityWithOntologyTerm(const Entity *term, const Entity *parentTerm);
    void removeAnnotation(const Entity *annotation);
    void selectEntity(const Entity *entity);

protected:
    bool eventFilter (QObject* watched_object, QEvent* e);
    void showErrorDialog(const QString & text);
    void showDisconnected();

private slots:
    void createAnnotationResults(const void *results);
    void createAnnotationError(const QString & error);
    void removeAnnotationResults(const void *results);
    void removeAnnotationError(const QString & error);

private:

    // User interface
    Ui::AnnotationWidget *ui;
    NaMainWindow *naMainWindow;

    // Console Observer
    obs::ConsoleObserverServiceImpl *consoleObserverService;
    ConsoleObserver *consoleObserver;

    // Ontology
    Ontology *ontology;
    OntologyTreeModel *ontologyTreeModel;

    // Annotations
    AnnotatedBranch *annotatedBranch;
    AnnotatedBranchTreeModel *annotatedBranchTreeModel;
    const Entity *selectedEntity;

    // Internal variables
    bool firstLoad;
    DataThread *createAnnotationThread;
    DataThread *removeAnnotationThread;
    QMutex mutex;
};

#endif // ANNOTATION_WIDGET_H_
