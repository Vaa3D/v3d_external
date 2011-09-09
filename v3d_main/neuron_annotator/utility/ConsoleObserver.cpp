#include "ConsoleObserver.h"
#include "DataThread.h"
#include "../gui/NaMainWindow.h"
#include "../entity_model/Entity.h"
#include "../entity_model/Ontology.h"
#include "../../webservice/impl/EntityAdapter.h"
#include "../../webservice/console/cdsConsoleDataServiceProxy.h"

ConsoleObserver::ConsoleObserver(NaMainWindow *naMainWindow, QObject *parent) :
    QObject(parent),
    mainWindow(naMainWindow),
    loadOntologyThread(0),
    entitySelectedThread(0),
    entityViewRequestedThread(0)
{
}

//*******************************************************************************************
// ontologySelected event
//*******************************************************************************************

void ConsoleObserver::loadCurrentOntology()
{
    if (loadOntologyThread != NULL)
    {
        loadOntologyThread->disconnect();
    }

    loadOntologyThread = new GetCurrentOntologyThread;
    connect(loadOntologyThread, SIGNAL(gotResults(const void *)),
            this, SLOT(loadOntologyResults(const void *)));
    connect(loadOntologyThread, SIGNAL(gotError(const QString &)),
            this, SLOT(loadOntologyError(const QString &)));
    loadOntologyThread->start(QThread::NormalPriority);
}

void ConsoleObserver::loadOntologyResults(const void *results)
{
    emit openOntology((Ontology *)results);
    delete loadOntologyThread;
    loadOntologyThread = NULL;
}

void ConsoleObserver::loadOntologyError(const QString &error)
{
    emit openOntology(0);
    emit communicationError(error);
    delete loadOntologyThread;
    loadOntologyThread = NULL;
}

void ConsoleObserver::ontologySelected(long rootId)
{
    qDebug() << "Got signal ontologySelected:" << rootId;
    loadCurrentOntology();
}


//*******************************************************************************************
// ontologyChanged event
//*******************************************************************************************

void ConsoleObserver::ontologyChanged(long rootId)
{
    qDebug() << "Got signal ontologyChanged:" << rootId;
    loadCurrentOntology();
}

//*******************************************************************************************
// entitySelected event
//*******************************************************************************************

void ConsoleObserver::entitySelected(long entityId)
{
    qDebug() << "Got signal entitySelected:" << entityId;

    if (entitySelectedThread != NULL)
    {
        entitySelectedThread->disconnect();
    }

    entitySelectedThread = new GetEntityThread(entityId);
    connect(entitySelectedThread, SIGNAL(gotResults(const void *)),
            this, SLOT(entitySelectedResults(const void *)));
    connect(entitySelectedThread, SIGNAL(gotError(const QString &)),
            this, SLOT(entitySelectedError(const QString &)));
    entitySelectedThread->start(QThread::NormalPriority);
}

void ConsoleObserver::entitySelectedResults(const void *results)
{

    Entity *entity = (Entity *)results;
    if (entity == NULL) return;

    QString type = *entity->entityType;

    // TODO: reimplement this using entities and the annotatedBranch tree

//    qDebug() << "  Type:" <<type;
//    if (type == "Tif 2D Image") // TODO: change to Neuron Fragment in the future?
//    {
//        QString filepath = entity->getValueByAttributeName("File Path");
//        qDebug() << "  File Path:"<<filepath;

//        if (!filepath.isEmpty())
//        {
//            QString macPath = convertPathToMac(filepath);
//            QRegExp rx("neuronSeparatorPipeline.PR.neuron(\\d+)\\.tif");
//            if (rx.indexIn(macPath) != -1)
//            {
//                QString numStr = rx.cap(1);
//                bool ok;
//                int num = numStr.toInt(&ok);
//                if (ok)
//                {
//                    qDebug() << "  Select neuron:" <<num;
//                    //mainWindow->getAnnotationSession()->getNeuronSelectionModel().showExactlyOneNeuron(num);
//                }
//            }
//        }
//    }

    delete entitySelectedThread;
    entitySelectedThread = NULL;
}

void ConsoleObserver::entitySelectedError(const QString & error) {
    emit communicationError(error);
    delete entitySelectedThread;
    entitySelectedThread = NULL;
}

//*******************************************************************************************
// entityViewRequested event
//*******************************************************************************************

void ConsoleObserver::entityViewRequested(long entityId)
{
    qDebug() << "Got signal entityViewRequested:" << entityId;

    if (entityViewRequestedThread != NULL)
    {
        entityViewRequestedThread->disconnect();
    }

    entityViewRequestedThread = new GetEntityThread(entityId);
    connect(entityViewRequestedThread, SIGNAL(gotResults(const void *)),
            this, SLOT(entityViewRequestedResults(const void *)));
    connect(entityViewRequestedThread, SIGNAL(gotError(const QString &)),
            this, SLOT(entityViewRequestedError(const QString &)));
    entityViewRequestedThread->start(QThread::NormalPriority);
}

void ConsoleObserver::entityViewRequestedResults(const void *results)
{

    Entity *entity = (Entity *)results;
    if (entity == NULL) return;

    QString type = *entity->entityType;
    qDebug() << "  Type:" << type;
    if (type == "Neuron Separator Pipeline Result")
    {
        QApplication::alert((QWidget *)mainWindow, 10000);
        mainWindow->show();
        mainWindow->activateWindow();
        mainWindow->raise();
        // TODO: begin process indication
        annotatedBranchViewRequested(*entity->id);
    }

    delete entityViewRequestedThread;
    entityViewRequestedThread = NULL;
}

void ConsoleObserver::entityViewRequestedError(const QString & error)
{
    emit communicationError(error);
    delete entityViewRequestedThread;
    entityViewRequestedThread = NULL;
}

//*******************************************************************************************
// annotatedBranchViewRequested event
//*******************************************************************************************

void ConsoleObserver::annotatedBranchViewRequested(long entityId)
{
    qDebug() << "Got signal entityViewRequested:" << entityId;

    if (annotatedBranchViewRequestedThread != NULL)
    {
        annotatedBranchViewRequestedThread->disconnect();
    }

    annotatedBranchViewRequestedThread = new GetAnnotatedBranchThread(entityId);
    connect(annotatedBranchViewRequestedThread, SIGNAL(gotResults(const void *)),
            this, SLOT(annotatedBranchViewRequestedResults(const void *)));
    connect(annotatedBranchViewRequestedThread, SIGNAL(gotError(const QString &)),
            this, SLOT(annotatedBranchViewRequestedError(const QString &)));
    annotatedBranchViewRequestedThread->start(QThread::NormalPriority);
}

void ConsoleObserver::annotatedBranchViewRequestedResults(const void *results)
{

    AnnotatedBranch *annotatedBranch = (AnnotatedBranch *)results;
    if (annotatedBranch == NULL) return;

    QString filepath = annotatedBranch->getFilePath();
    qDebug() << "  Name:"<<annotatedBranch->name();
    qDebug() << "  File Path:"<<filepath;

    if (!filepath.isEmpty()) {
        emit openAnnotatedBranch(annotatedBranch);
    }

    delete annotatedBranchViewRequestedThread;
    annotatedBranchViewRequestedThread = NULL;
}

void ConsoleObserver::annotatedBranchViewRequestedError(const QString & error)
{
    emit communicationError(error);
    delete annotatedBranchViewRequestedThread;
    annotatedBranchViewRequestedThread = NULL;
}

//*******************************************************************************************
// annotationsChanged event
//*******************************************************************************************

void ConsoleObserver::annotationsChanged(long entityId)
{
    qDebug() << "Got signal annotationsChanged:" << entityId;

    if (annotationsChangedThread != NULL)
    {
        annotationsChangedThread->disconnect();
    }

    annotationsChangedThread = new GetEntityAnnotationsThread(entityId);
    connect(annotationsChangedThread, SIGNAL(gotResults(const void *)),
            this, SLOT(annotatedBranchViewRequestedResults(const void *)));
    connect(annotationsChangedThread, SIGNAL(gotError(const QString &)),
            this, SLOT(annotatedBranchViewRequestedError(const QString &)));
    annotationsChangedThread->start(QThread::NormalPriority);

}

void ConsoleObserver::annotationsChangedResults(const void *results)
{
    emit updateAnnotations(((GetEntityAnnotationsThread *)annotationsChangedThread)->getEntityId(), (AnnotationList *)results);
    delete annotationsChangedThread;
    annotationsChangedThread = NULL;
}

void ConsoleObserver::annotationsChangedError(const QString & error)
{
    emit communicationError(error);
    delete annotationsChangedThread;
    annotationsChangedThread = NULL;
}
