#ifndef ANNOTATION_WIDGET_H_
#define ANNOTATION_WIDGET_H_

#include "../entity_model/AnnotatedBranch.h"
#include <QFrame>
#include <QEvent>
#include <QMutex>
#include <QHash>

class Entity;
class Ontology;
class OntologyTreeModel;
class AnnotatedBranch;
class AnnotatedBranchTreeModel;
class ConsoleObserver;
class AnnotationSession;
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
    void neuronSelected(int index);
    void neuronsDeselected();

public slots:
    void closeOntology();
    void openOntology(Ontology *ontology);
    void closeAnnotationSession();
    void openAnnotationSession(AnnotationSession *annotationSession);
    void closeAnnotatedBranch();
    void openAnnotatedBranch(AnnotatedBranch *annotatedBranch, bool openStack = true);
    void updateAnnotations(qint64 entityId, AnnotationList *annotations, UserColorMap *userColorMap);
    void communicationError(const QString & errorMessage);
    void updateCurrentSample(Entity *sample);
    void consoleConnect();
    void consoleConnect(int retries);
    void consoleDisconnect();
    void consoleSync();
    void ontologyTreeDoubleClicked(const QModelIndex & index);
    void annotatedBranchTreeClicked(const QModelIndex & index);
    void annotateSelectedEntityWithOntologyTerm(const Entity *term, const Entity *parentTerm);
    void removeAnnotation(const Entity *annotation);
    void selectFragment(const Entity *entity);
    void selectEntity(const Entity *entity);
    void selectEntity(const Entity *entity, const bool external);
    void selectEntityById(const qint64 & entityId);
    void selectEntityById(const qint64 & entityId, const bool external);
    void selectNeuron(int index);
    void deselectNeurons();

protected:
    bool eventFilter (QObject* watched_object, QEvent* e);
    void showErrorDialog(const QString & text);
    void showDisconnected();
    void showSynced();
    void showDesynced();

private slots:
    void createAnnotationResults(const void *results);
    void createAnnotationError(const QString & error);
    void removeAnnotationResults(const void *results);
    void removeAnnotationError(const QString & error);
    void selectEntityResults(const void *results);
    void selectEntityError(const QString &error);
    void entityWasSelected(const Entity *entity);

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
    AnnotationSession *annotationSession;
    AnnotatedBranch *annotatedBranch;
    AnnotatedBranchTreeModel *annotatedBranchTreeModel;
    const Entity *selectedEntity;

    // Internal variables
    DataThread *createAnnotationThread;
    DataThread *removeAnnotationThread;
    DataThread *selectEntityThread;
    QMutex mutex;
    int retries;
};

#endif // ANNOTATION_WIDGET_H_
