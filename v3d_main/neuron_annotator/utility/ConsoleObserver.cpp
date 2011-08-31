#include "ConsoleObserver.h"
#include "../gui/NaMainWindow.h"
#include "../entity_model/Ontology.h"

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

void ConsoleObserver::loadOntologyResults(const void *results) {

    emit openOntology((Ontology *)results);
    delete loadOntologyThread;
    loadOntologyThread = NULL;
}

void ConsoleObserver::loadOntologyError(const QString &error) {
    // TODO: this should notify the user in some way
    delete loadOntologyThread;
    loadOntologyThread = NULL;
}

void ConsoleObserver::ontologySelected(long rootId)
{
    qDebug() << "Got signal ontologySelected:" << rootId;
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

void ConsoleObserver::entitySelectedResults(const void *results) {

    Entity *entity = (Entity *)results;
    if (entity == NULL) return;

    QString type = *entity->entityType;
    qDebug() << "  Type:" <<type;
    if (type == "Tif 2D Image") // TODO: change to Neuron Fragment in the future?
    {
        QString filepath = entity->getValueByAttributeName("File Path");
        qDebug() << "  File Path:"<<filepath;

        if (!filepath.isEmpty())
        {
            QString macPath = convertPathToMac(filepath);
            QRegExp rx("neuronSeparatorPipeline.PR.neuron(\\d+)\\.tif");
            if (rx.indexIn(macPath) != -1)
            {
                QString numStr = rx.cap(1);
                bool ok;
                int num = numStr.toInt(&ok);
                if (ok)
                {
                    qDebug() << "  Select neuron:" <<num;
                    //mainWindow->getAnnotationSession()->getNeuronSelectionModel().showExactlyOneNeuron(num);
                }
            }
        }
    }

    delete entitySelectedThread;
    entitySelectedThread = NULL;
}

void ConsoleObserver::entitySelectedError(const QString & error) {
    // TODO: this should notify the user in some way
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

void ConsoleObserver::entityViewRequestedResults(const void *results) {

    Entity *entity = (Entity *)results;
    if (entity == NULL) return;

    QString type = *entity->entityType;
    qDebug() << "  Type:" << type;
    if (type == "Neuron Separator Pipeline Result")
    {
        QString filepath = entity->getValueByAttributeName("File Path");
        qDebug() << "  File Path:"<<filepath;

        if (!filepath.isEmpty()) {
            QString macPath = convertPathToMac(filepath);
            qDebug() << "  Opening image stack:" << macPath;
            QApplication::alert((QWidget *)mainWindow, 10000);
            mainWindow->show();
            mainWindow->activateWindow();
            mainWindow->raise();
            emit openMulticolorImageStack(macPath);
        }
    }

    delete entityViewRequestedThread;
    entityViewRequestedThread = NULL;
}

void ConsoleObserver::entityViewRequestedError(const QString & error) {
    // TODO: this should notify the user in some way
    delete entityViewRequestedThread;
    entityViewRequestedThread = NULL;
}

//*******************************************************************************************
// annotationsChanged event
//*******************************************************************************************

void ConsoleObserver::annotationsChanged(long entityId)
{
    qDebug() << "Got signal annotationsChanged:" << entityId;
}
