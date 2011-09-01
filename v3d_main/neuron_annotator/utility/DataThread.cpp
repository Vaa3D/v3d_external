#include "DataThread.h"
#include "../entity_model/Entity.h"
#include "../entity_model/Ontology.cpp"
#include "../../webservice/impl/EntityAdapter.h"
#include <QMap>

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

    Entity *root = 0;
    QMap<QKeySequence, qint64> *keyBindMap = 0;

    qDebug() << "Getting current ontology...";
    cds::fw__getCurrentOntologyResponse response;
    if (proxy.getCurrentOntology(response) == SOAP_OK)
    {
        root = EntityAdapter::convert(response.return_);

        qDebug() << "Getting current ontology keybindings...";
        cds::fw__getKeybindingsResponse keybindingsResponse;
        if (proxy.getKeybindings(*root->id, keybindingsResponse) == SOAP_OK)
        {
            cds::fw__ontologyKeyBindings *keyBindings = keybindingsResponse.return_;
            keyBindMap = EntityAdapter::convert(keyBindings);
        }
        else
        {
            errorMessage = new QString(proxy.soap_fault_string());
            delete root;
            return;
        }
    }
    else
    {
        errorMessage = new QString(proxy.soap_fault_string());
        return;
    }

    results = new Ontology(root, keyBindMap);
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
        results = EntityAdapter::convert(response.return_);
    }
    else
    {
        errorMessage = new QString(proxy.soap_fault_string());
    }
}
