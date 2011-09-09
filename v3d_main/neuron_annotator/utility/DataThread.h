#ifndef DATATHREAD_H
#define DATATHREAD_H

#include "../../webservice/console/cdsConsoleDataServiceProxy.h"
#include "../entity_model/AnnotatedBranch.h"
#include <QObject>
#include <QThread>
#include <QHash>

class DataThread : public QThread
{
    Q_OBJECT

public:
    explicit DataThread(QObject *parent = 0);
    void run();
    ~DataThread();

signals:
    void gotResults(const void *results);
    void gotError(const QString & message);

protected:
    void *results;
    QString *errorMessage;
    cds::ConsoleDataServiceProxy proxy;
    virtual void fetchData() = 0;
};

// ===========================================================
// Get Current Ontology Tree
// ===========================================================

class GetCurrentOntologyThread : public DataThread
{
    Q_OBJECT

public:
    explicit GetCurrentOntologyThread(QObject *parent = 0);
    void fetchData();

};

// ===========================================================
// Get AnnotatedBranch
// ===========================================================

class GetAnnotatedBranchThread : public DataThread
{
    Q_OBJECT

public:
    explicit GetAnnotatedBranchThread(long entityId, QObject *parent = 0);
    void fetchData();
private:
    void fetchAnnotations(Entity *entity);
    QHash<qint64, AnnotationList*> *annotationMap;
    long entityId;
};

// ===========================================================
// Get Entity
// ===========================================================

class GetEntityThread : public DataThread
{
    Q_OBJECT

public:
    explicit GetEntityThread(long entityId, QObject *parent = 0);
    void fetchData();
private:
    long entityId;
};

// ===========================================================
// Get Entity Annotations
// ===========================================================

class GetEntityAnnotationsThread : public DataThread
{
    Q_OBJECT

public:
    explicit GetEntityAnnotationsThread(long entityId, QObject *parent = 0);
    void fetchData();
    inline long getEntityId() const { return entityId; }
private:
    long entityId;
};

#endif // DATATHREAD_H
