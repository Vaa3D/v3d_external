#ifndef DATATHREAD_H
#define DATATHREAD_H

#include <QObject>
#include <QThread>
#include "../../webservice/console/cdsConsoleDataServiceProxy.h"

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

#endif // DATATHREAD_H
