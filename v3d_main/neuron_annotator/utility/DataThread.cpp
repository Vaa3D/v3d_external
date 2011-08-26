#include "DataThread.h"
#include "../../webservice/impl/EntityAdapter.h"

DataThread::DataThread(QObject *parent) :
    QThread(parent),
    results(0),
    errorMessage(0)
{
}

void DataThread::run()
{
    fetchData();
    if (errorMessage != 0) {
        qDebug() << "SOAP error: " << *errorMessage;
        emit gotError(*errorMessage);
    }
    else {
        emit gotResults(results);
    }
}

DataThread::~DataThread()
{
    proxy.destroy();
}

// ===========================================================
// Get Current Ontology Tree
// ===========================================================

GetCurrentOntologyThread::GetCurrentOntologyThread(QObject *parent) :
    DataThread(parent)
{
}

void GetCurrentOntologyThread::fetchData()
{
    cds::fw__getCurrentOntologyResponse response;
    qDebug() << "Getting current ontology...";
    if (proxy.getCurrentOntology(response) == SOAP_OK)
    {
        qDebug() << "Got results!";
        results = convert(response.return_);
        qDebug() << "Converted";
    }
    else
    {
        errorMessage = new QString(proxy.soap_fault_string());
    }
}

// ===========================================================
// Get Entity
// ===========================================================

GetEntityThread::GetEntityThread(long entityId, QObject *parent) :
    DataThread(parent),
    entityId(entityId)
{
}

void GetEntityThread::fetchData()
{
    cds::fw__getEntityByIdResponse response;
    if (proxy.getEntityById(entityId, response) == SOAP_OK)
    {
        results = convert(response.return_);
    }
    else
    {
        errorMessage = new QString(proxy.soap_fault_string());
    }
}
