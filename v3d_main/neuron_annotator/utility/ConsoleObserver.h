#ifndef CONSOLEOBSERVER_H
#define CONSOLEOBSERVER_H

#include <QtCore>
#include "../entity_model/AnnotatedBranch.h"

class DataThread;
class NaMainWindow;
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
    Q_OBJECT
public:
    explicit ConsoleObserver(NaMainWindow *mainWindow = 0, QObject *parent = 0);
    void loadCurrentOntology();

signals:
    // These signals are emitted when a processed console event triggers an action
    void openOntology(Ontology *ontology);
    void openAnnotatedBranch(AnnotatedBranch *annotatedBranch);
    void updateAnnotations(long entityId, AnnotationList *annotations);
    void communicationError(const QString & errorMessage);

public slots:
    // These slots implement the console observer interface.
    // They are called by the console observer service whenever
    // a new event is received from the console.
    void ontologySelected(long rootId);
    void ontologyChanged(long rootId);
    void entitySelected(long entityId);
    void entityViewRequested(long entityId);
    void annotationsChanged(long entityId);

private slots:
    // These slots are for listening to internal worker threads
    void annotatedBranchViewRequested(long entityId);
    void loadOntologyResults(const void *results);
    void loadOntologyError(const QString & error);
    void entitySelectedResults(const void *results);
    void entitySelectedError(const QString & error);
    void entityViewRequestedResults(const void *results);
    void entityViewRequestedError(const QString & error);
    void annotatedBranchViewRequestedResults(const void *results);
    void annotatedBranchViewRequestedError(const QString & error);
    void annotationsChangedResults(const void *results);
    void annotationsChangedError(const QString & error);

private:
    NaMainWindow *mainWindow;
    DataThread *loadOntologyThread;
    DataThread *entitySelectedThread;
    DataThread *entityViewRequestedThread;
    DataThread *annotatedBranchViewRequestedThread;
    DataThread *annotationsChangedThread;
};

#endif // CONSOLEOBSERVER_H
