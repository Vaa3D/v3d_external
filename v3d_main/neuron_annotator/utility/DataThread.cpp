#include "DataThread.h"
#include "../entity_model/Entity.h"
#include "../entity_model/Ontology.h"
#include "../entity_model/OntologyAnnotation.h"
#include "../../webservice/impl/EntityAdapter.h"
#include <QMap>


// ===========================================================
// Utility Functions
// ===========================================================

QString fetchUserColor(cds::ConsoleDataServiceProxy* proxy, const QString & username)
{
    cds::fw__getUserAnnotationColorResponse response;
    if (proxy->getUserAnnotationColor(username.toStdString(), response) == SOAP_OK)
    {
         return QString(response.return_.c_str());
    }
    return QString();
}

void fetchUserColors(cds::ConsoleDataServiceProxy* proxy, AnnotationList *alist, QHash<QString, QColor> *userColorMap)
{
    QListIterator<Entity *> i(*alist);
    while (i.hasNext())
    {
        Entity *annotation = i.next();
        if (annotation->user == 0) continue;
        QString username(*annotation->user);
        if (!userColorMap->contains(username))
        {
            QString colorHex("#");
            colorHex.append(fetchUserColor(proxy, username));
            QColor color;
            color.setNamedColor(colorHex);
            userColorMap->insert(username, color);
        }
    }
}


// ===========================================================
// Data Thread Base Class
// ===========================================================

DataThread::DataThread(char* endpoint_url, QObject *parent) :
    QThread(parent),
    results(0),
    errorMessage(0),
    proxy(new cds::ConsoleDataServiceProxy(endpoint_url))
{
    // Deletes itself when done
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

DataThread::~DataThread()
{
    if (proxy!=0) {
        proxy->destroy();
        delete proxy;
    }
    if (errorMessage != 0) delete errorMessage;
}

void DataThread::run()
{
    fetchData();
    if (errorMessage != 0) {
        qDebug() << "SOAP error:" << *errorMessage;
        emit gotError(*errorMessage);
    }
    else {
        emit gotResults(results);
    }
}

void DataThread::disregard()
{
    disconnect(this, 0, 0, 0);
}


// ===========================================================
// Get Ontology Tree
// ===========================================================

GetOntologyThread::GetOntologyThread(char* endpoint_url, qint64 entityId, QObject *parent) :
    DataThread(endpoint_url, parent),
    entityId(entityId)
{
}

void GetOntologyThread::fetchData()
{
    Entity *root = 0;
    QMap<QKeySequence, qint64> *keyBindMap = 0;

    qDebug() << "Getting ontology..." << entityId;
    cds::fw__getOntologyResponse response;
    if (proxy->getOntology(entityId, response) == SOAP_OK)
    {
        root = EntityAdapter::convert(response.return_);

        qDebug() << "Getting current ontology keybindings...";
        cds::fw__getKeybindingsResponse keybindingsResponse;
        if (proxy->getKeybindings(*root->id, keybindingsResponse) == SOAP_OK)
        {
            cds::fw__ontologyKeyBindings *keyBindings = keybindingsResponse.return_;
            keyBindMap = EntityAdapter::convert(keyBindings);
        }
        else
        {
            errorMessage = new QString(proxy->soap_fault_string());
            delete root;
            return;
        }
    }
    else
    {
        errorMessage = new QString(proxy->soap_fault_string());
        return;
    }

    results = new Ontology(root, keyBindMap);
}

// ===========================================================
// Get AnnotatedBranch
// ===========================================================

GetAnnotatedBranchThread::GetAnnotatedBranchThread(char* endpoint_url, qint64 entityId, QObject *parent) :
    DataThread(endpoint_url, parent),
    entityId(entityId)
{
}

void GetAnnotatedBranchThread::fetchAnnotations(Entity *entity, QHash<QString, QColor> *userColorMap)
{
    if (entity == NULL) return;
    if (entity->id == NULL) {
        qDebug("ENTITY ID IS NULL");
        entity->dumpEntity();
        return; // CMB Nov 29 2012
    }

    cds::fw__getAnnotationsForEntityResponse response;
    if (proxy->getAnnotationsForEntity(*entity->id, response) == SOAP_OK)
    {
        AnnotationList *alist = EntityAdapter::convert(response.return_);
        annotationMap->insert(*entity->id, alist);
        fetchUserColors(proxy, alist, userColorMap);
    }
    else
    {
        // This overrides the last error message, but the user doesn't need all of them, and they've already been logged.
        errorMessage = new QString(proxy->soap_fault_string());
    }

    QSetIterator<EntityData *> i(entity->entityDataSet);
    while (i.hasNext())
    {
        EntityData *data = i.next();
        fetchAnnotations(data->childEntity, userColorMap);
    }
}

void GetAnnotatedBranchThread::fetchData()
{
    QHash<QString, QColor> *userColorMap = new QHash<QString, QColor>();
    Entity *entity;
    cds::fw__getEntityTreeResponse response;
    if (proxy->getEntityTree(entityId, response) == SOAP_OK)
    {
        // Convert the results into the Entity model
        entity = EntityAdapter::convert(response.return_);
        // Recursively retrieve annotations for every entity in the tree and populate the annotationMap
        annotationMap = new QHash<qint64, AnnotationList*>;
        fetchAnnotations(entity, userColorMap);
    }
    else
    {
        errorMessage = new QString(proxy->soap_fault_string());
        return;
    }

    results = new AnnotatedBranch(entity, annotationMap, userColorMap);
}

// ===========================================================
// Get Entity
// ===========================================================

GetEntityThread::GetEntityThread(char* endpoint_url, qint64 entityId, QObject *parent) :
    DataThread(endpoint_url, parent),
    entityId(entityId)
{
}

void GetEntityThread::fetchData()
{
    cds::fw__getEntityByIdResponse response;
    if (proxy->getEntityById(entityId, response) == SOAP_OK)
    {
        results = EntityAdapter::convert(response.return_);        
    }
    else
    {
        errorMessage = new QString(proxy->soap_fault_string());
    }
}

// ===========================================================
// Get Parents
// ===========================================================

GetParentsThread::GetParentsThread(char* endpoint_url, qint64 entityId, QObject *parent) :
    DataThread(endpoint_url, parent),
    entityId(entityId)
{
}

void GetParentsThread::fetchData()
{
    cds::fw__getParentEntityArrayResponse response;
    if (proxy->getParentEntityArray(entityId, response) == SOAP_OK)
    {
        results = EntityAdapter::convert(response.return_);
    }
    else
    {
        errorMessage = new QString(proxy->soap_fault_string());
    }
}

// ===========================================================
// Get Ancestor With Type
// ===========================================================

GetAncestorThread::GetAncestorThread(char* endpoint_url, qint64 entityId, const QString & type, QObject *parent) :
    DataThread(endpoint_url, parent),
    entityId(entityId),
    type(type)
{
}

void GetAncestorThread::fetchData()
{
    cds::fw__getAncestorWithTypeResponse response;
    if (proxy->getAncestorWithType(entityId, type.toStdString(), response) == SOAP_OK)
    {
        results = EntityAdapter::convert(response._return_);
    }
    else
    {
        errorMessage = new QString(proxy->soap_fault_string());
    }
}


// ===========================================================
// Get Entity Annotations
// ===========================================================

GetEntityAnnotationsThread::GetEntityAnnotationsThread(char* endpoint_url, qint64 entityId, QObject *parent) :
    DataThread(endpoint_url, parent),
    entityId(entityId)
{
}

void GetEntityAnnotationsThread::fetchData()
{
    userColorMap = new QHash<QString, QColor>();
    cds::fw__getAnnotationsForEntityResponse response;
    if (proxy->getAnnotationsForEntity(entityId, response) == SOAP_OK)
    {
        results = EntityAdapter::convert(response.return_);
        fetchUserColors(proxy, (AnnotationList *)results, userColorMap);
    }
    else
    {
        errorMessage = new QString(proxy->soap_fault_string());
    }
}

// ===========================================================
// Create Annotation
// ===========================================================

CreateAnnotationThread::CreateAnnotationThread(char* endpoint_url, OntologyAnnotation *annotation, QObject *parent) :
    DataThread(endpoint_url, parent),
    annotation(annotation)
{
}

CreateAnnotationThread::~CreateAnnotationThread()
{
    if (annotation!=0) delete annotation;
}

void CreateAnnotationThread::fetchData()
{
    cds::fw__ontologyAnnotation *fwAnnotation = EntityAdapter::convert(annotation);
    cds::fw__createAnnotationResponse response;
    if (proxy->createAnnotation(fwAnnotation, response) == SOAP_OK)
    {
        // Assume success
    }
    else
    {
        errorMessage = new QString(proxy->soap_fault_string());
    }

    delete fwAnnotation;
}

qint64* CreateAnnotationThread::getTargetEntityId() const
{
    return annotation->targetEntityId;
}

// ===========================================================
// Remove Annotation
// ===========================================================

RemoveAnnotationThread::RemoveAnnotationThread(char* endpoint_url, qint64 annotationId, QObject *parent) :
    DataThread(endpoint_url, parent), 
    annotationId(annotationId)
{
}

void RemoveAnnotationThread::fetchData()
{
    cds::fw__removeAnnotationResponse response;
    if (proxy->removeAnnotation(annotationId, response) == SOAP_OK)
    {
        // Assume success
    }
    else
    {
        errorMessage = new QString(proxy->soap_fault_string());
    }
}


// ===========================================================
// Get Annotation Session
// ===========================================================

GetAnnotationSessionThread::GetAnnotationSessionThread(char* endpoint_url, qint64 sessionId, QObject *parent) :
    DataThread(endpoint_url, parent), 
    sessionId(sessionId)
{
}

void GetAnnotationSessionThread::fetchData()
{
    cds::fw__getAnnotationSessionResponse response;
    if (proxy->getAnnotationSession(sessionId, response) == SOAP_OK)
    {
        results = EntityAdapter::convert(response.return_);
    }
    else
    {
        errorMessage = new QString(proxy->soap_fault_string());
    }
}


// ===========================================================
// Select Entity
// ===========================================================

SelectEntityThread::SelectEntityThread(char* endpoint_url, qint64 entityId, QObject *parent) :
    DataThread(endpoint_url, parent), 
    entityId(entityId)
{
}

void SelectEntityThread::fetchData()
{
    cds::fw__selectEntityResponse response;
    if (proxy->selectEntity(entityId, true, response) == SOAP_OK)
    {
        // Success
    }
    else
    {
        errorMessage = new QString(proxy->soap_fault_string());
    }
}
