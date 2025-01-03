#ifndef CONSOLEOBSERVER_H
#define CONSOLEOBSERVER_H

#include <QtCore>
#include <QMutex>
#include "../entity_model/AnnotatedBranch.h"

class DataThread;
class NaMainWindow;
class AnnotationSession;
class Ontology;
class Entity;

// This object is the bridge between the web service observing the Console, and the NeuronAnnotator GUI.
// It listens for signals from the web service, processes them, and potentially requests additional information
// from the console. Once it is satisfied in having all the information needed, it sends out a signal triggering
// an action, such as "open this ontology" or "load this sample." It is assumed that there will always be a
// receiver who listens to such signals and accepts responsibility for the data passed with them.
//
class ConsoleObserver : public QObject
{
    //Q_OBJECT
public:
    explicit ConsoleObserver(NaMainWindow *mainWindow = 0, QObject *parent = 0);
    void loadOntology(qint64 rootId);

//signals:
    // These signals are emitted when a processed console event triggers an action
    void openOntology(Ontology *ontology);
    void openAnnotatedBranch(AnnotatedBranch *annotatedBranch);
    void updateAnnotations(qint64 entityId, AnnotationList *annotations, UserColorMap *userColorMap);
    void openAnnotationSession(AnnotationSession *session);
    void closeAnnotationSession();
    void selectEntityById(const qint64 & entityId, const bool external);
    void communicationError(const QString & errorMessage);
    void updateCurrentSample(Entity *sample);
    void openStackWithVaa3d(Entity* entity);

//public slots:
    // These slots implement the console observer interface.
    // They are called by the console observer service whenever
    // a new event is received from the console.
    void ontologySelected(qint64 rootId);
    void ontologyChanged(qint64 rootId);
    void entitySelected(const QString & category, const QString & uniqueId, bool clearAll);
    void entityDeselected(const QString & category, const QString & uniqueId);
    void entityViewRequested(qint64 entityId);
    void annotationsChanged(qint64 entityId);
    void sessionSelected(qint64 sessionId);
    void sessionDeselected();

//private slots:
    // These slots are for listening to internal worker threads
    void annotatedBranchViewRequested(qint64 entityId);
    void loadOntologyResults(const void *results);
    void loadOntologyError(const QString & error);
    void entityViewRequestedResults(const void *results);
    void entityViewRequestedError(const QString & error);
    void annotatedBranchViewRequestedResults(const void *results);
    void annotatedBranchViewRequestedError(const QString & error);
    void annotatedBranchViewRequested2Results(const void *results);
    void annotatedBranchViewRequested2Error(const QString & error);
    void annotationsChangedResults(const void *results);
    void annotationsChangedError(const QString & error);
    void loadAnnotationSessionResults(const void *results);
    void loadAnnotationSessionError(const QString & error);

private:
    NaMainWindow *mainWindow;

    // Internal worker threads for loading data
    DataThread *loadOntologyThread;
    DataThread *entityViewRequestedThread;
    DataThread *annotatedBranchViewRequestedThread;
    DataThread *annotatedBranchViewRequested2Thread;
    DataThread *annotationsChangedThread;
    DataThread *loadAnnotationSessionThread;

    // Synchronize access to the worker threads
    QMutex loadOntologyMutex;
    QMutex entityViewRequestedMutex;
    QMutex annotatedBranchViewRequestedMutex;
    QMutex annotatedBranchViewRequested2Mutex;
    QMutex annotationsChangedMutex;
    QMutex loadAnnotationSessionMutex;
};

#endif // CONSOLEOBSERVER_H
