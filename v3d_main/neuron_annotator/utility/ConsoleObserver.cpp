#include "ConsoleObserver.h"
#include "DataThread.h"
#include "../gui/NaMainWindow.h"
#include "../entity_model/Entity.h"
#include "../entity_model/Ontology.h"
#include "../entity_model/AnnotationSession.h"
#include "../utility/JacsUtil.h"
#include "../../webservice/impl/EntityAdapter.h"
#include "../../webservice/console/cdsConsoleDataServiceProxy.h"

ConsoleObserver::ConsoleObserver(NaMainWindow *mainWindow, QObject *parent) :
    QObject(parent),
    mainWindow(mainWindow),
    loadOntologyThread(0),
    entityViewRequestedThread(0),
    annotatedBranchViewRequestedThread(0),
    annotatedBranchViewRequested2Thread(0),
    annotationsChangedThread(0),
    loadAnnotationSessionThread(0),
    loadOntologyMutex(QMutex::Recursive),
    entityViewRequestedMutex(QMutex::Recursive),
    annotatedBranchViewRequestedMutex(QMutex::Recursive),
    annotatedBranchViewRequested2Mutex(QMutex::Recursive),
    annotationsChangedMutex(QMutex::Recursive),
    loadAnnotationSessionMutex(QMutex::Recursive)
{
}

//*******************************************************************************************
// ontologySelected event
//*******************************************************************************************

void ConsoleObserver::loadOntology(qint64 rootId)
{
    QMutexLocker locker(&loadOntologyMutex);

    if (loadOntologyThread != NULL)
    {
        loadOntologyThread->disregard();
    }

    loadOntologyThread = new GetOntologyThread(mainWindow->getConsoleURL(), rootId);
    connect(loadOntologyThread, SIGNAL(gotResults(const void *)),
            this, SLOT(loadOntologyResults(const void *)));
    connect(loadOntologyThread, SIGNAL(gotError(const QString &)),
            this, SLOT(loadOntologyError(const QString &)));
    loadOntologyThread->start(QThread::NormalPriority);
}

void ConsoleObserver::loadOntologyResults(const void *results)
{
    QMutexLocker locker(&loadOntologyMutex);

    emit openOntology((Ontology *)results);
    loadOntologyThread = NULL;
}

void ConsoleObserver::loadOntologyError(const QString &error)
{
    QMutexLocker locker(&loadOntologyMutex);

    emit openOntology(0);
    emit communicationError(error);
    loadOntologyThread = NULL;
}

void ConsoleObserver::ontologySelected(qint64 rootId)
{
    QMutexLocker locker(&loadOntologyMutex);

    // qDebug() << "Got signal ontologySelected:" << rootId;
    loadOntology(rootId);
}


//*******************************************************************************************
// ontologyChanged event
//*******************************************************************************************

void ConsoleObserver::ontologyChanged(qint64 rootId)
{
    // qDebug() << "Got signal ontologyChanged:" << rootId;
    loadOntology(rootId);
}

//*******************************************************************************************
// entitySelected event
//*******************************************************************************************

void ConsoleObserver::entitySelected(const QString & category, const QString & uniqueId, bool clearAll)
{
    // qDebug() << "Got signal entitySelected:" << uniqueId << "category="<<category<< "clearAll?=" << clearAll;
    if (category == "mainViewer" || category == "secViewer") {
        QStringList list = uniqueId.split(QRegExp("/"));
        QString idStr = list.last().split("_").last();
        bool ok;
        qint64 entityId = (qint64)idStr.toLongLong(&ok);
        if (ok) {
            emit selectEntityById(entityId, true);
        }
    }
}

//*******************************************************************************************
// entityDeselected event
//*******************************************************************************************

void ConsoleObserver::entityDeselected(const QString & category, const QString & uniqueId)
{
    // qDebug() << "Got signal entityDeselected:" << uniqueId<< "category="<<category;
}

//*******************************************************************************************
// entityViewRequested event
//*******************************************************************************************

void ConsoleObserver::entityViewRequested(qint64 entityId)
{
    QMutexLocker locker(&entityViewRequestedMutex);

    // qDebug() << "Got signal entityViewRequested:" << entityId;

    if (entityViewRequestedThread != NULL)
    {
        entityViewRequestedThread->disregard();
    }

    entityViewRequestedThread = new GetEntityThread(mainWindow->getConsoleURL(), entityId);
    connect(entityViewRequestedThread, SIGNAL(gotResults(const void *)),
            this, SLOT(entityViewRequestedResults(const void *)));
    connect(entityViewRequestedThread, SIGNAL(gotError(const QString &)),
            this, SLOT(entityViewRequestedError(const QString &)));
    entityViewRequestedThread->start(QThread::NormalPriority);
}

void ConsoleObserver::entityViewRequestedResults(const void *results)
{
    QMutexLocker locker(&entityViewRequestedMutex);

    Entity *entity = (Entity *)results;
    if (entity == NULL) return;

    QString type = *entity->entityType;
    if (type == "Neuron Separator Pipeline Result")
    {
        QApplication::alert((QWidget *)mainWindow, 10000);
        mainWindow->show();
        mainWindow->activateWindow();
        mainWindow->raise();
        annotatedBranchViewRequested(*entity->id);
    }
    else if (type == "Image 3D" ||
             type == "Aligned Brain Stack" ||
             type == "LSM Stack" ||
             type == "Stitched V3D Raw Stack" ||
             type == "SWC File" ||
             type == "Vaa3d ANO File" ||
             type == "Tif 3D Image")
    {
        QApplication::alert((QWidget *)mainWindow, 10000);
        mainWindow->show();
        mainWindow->activateWindow();
        mainWindow->raise();

        QString filepath = entity->getValueByAttributeName("File Path");
        if (filepath != NULL)
        {
            // qDebug() << "openStackWithVaa3d " << filepath;
            emit openStackWithVaa3d(entity);
        }
    }

    entityViewRequestedThread = NULL;
}

void ConsoleObserver::entityViewRequestedError(const QString & error)
{
    QMutexLocker locker(&entityViewRequestedMutex);

    emit communicationError(error);
    entityViewRequestedThread = NULL;
}

//*******************************************************************************************
// annotatedBranchViewRequested event
//*******************************************************************************************

void ConsoleObserver::annotatedBranchViewRequested(qint64 entityId)
{
    QMutexLocker locker(&annotatedBranchViewRequestedMutex);

    // qDebug() << "Got signal annotatedBranchViewRequested:" << entityId;

    if (annotatedBranchViewRequestedThread != NULL)
    {
        annotatedBranchViewRequestedThread->disregard();
    }

    annotatedBranchViewRequestedThread = new GetAnnotatedBranchThread(mainWindow->getConsoleURL(), entityId);
    connect(annotatedBranchViewRequestedThread, SIGNAL(gotResults(const void *)),
            this, SLOT(annotatedBranchViewRequestedResults(const void *)));
    connect(annotatedBranchViewRequestedThread, SIGNAL(gotError(const QString &)),
            this, SLOT(annotatedBranchViewRequestedError(const QString &)));
    annotatedBranchViewRequestedThread->start(QThread::NormalPriority);
}

void ConsoleObserver::annotatedBranchViewRequestedResults(const void *results)
{
    QMutexLocker locker(&annotatedBranchViewRequestedMutex);

    AnnotatedBranch *annotatedBranch = (AnnotatedBranch *)results;
    if (annotatedBranch == NULL) return;

    QString filepath = annotatedBranch->getFilePath();
    // qDebug() << "Opening annotated branch, Name:"<<annotatedBranch->name()<<"FilePath:"<<filepath;

    if (!filepath.isEmpty()) {
        emit openAnnotatedBranch(annotatedBranch);

        annotatedBranchViewRequested2Thread = new GetAncestorThread(mainWindow->getConsoleURL(), *annotatedBranch->entity()->id, "Sample");
        connect(annotatedBranchViewRequested2Thread, SIGNAL(gotResults(const void *)),
                this, SLOT(annotatedBranchViewRequested2Results(const void *)));
        connect(annotatedBranchViewRequested2Thread, SIGNAL(gotError(const QString &)),
                this, SLOT(annotatedBranchViewRequested2Error(const QString &)));
        annotatedBranchViewRequested2Thread->start(QThread::NormalPriority);
    }

    annotatedBranchViewRequestedThread = NULL;
}

void ConsoleObserver::annotatedBranchViewRequestedError(const QString & error)
{
    QMutexLocker locker(&annotatedBranchViewRequestedMutex);

    emit communicationError(error);
    annotatedBranchViewRequestedThread = NULL;
}

void ConsoleObserver::annotatedBranchViewRequested2Results(const void *results)
{
    QMutexLocker locker(&annotatedBranchViewRequested2Mutex);

    Entity * ancestor = (Entity *)results;
    if (ancestor == NULL) return;

    emit updateCurrentSample(ancestor);

    annotatedBranchViewRequested2Thread = NULL;
}

void ConsoleObserver::annotatedBranchViewRequested2Error(const QString & error)
{
    QMutexLocker locker(&annotatedBranchViewRequested2Mutex);

    emit communicationError(error);
    annotatedBranchViewRequested2Thread = NULL;
}

//*******************************************************************************************
// annotationsChanged event
//*******************************************************************************************

void ConsoleObserver::annotationsChanged(qint64 entityId)
{
    QMutexLocker locker(&annotationsChangedMutex);

    // qDebug() << "Got signal annotationsChanged:" << entityId;

    if (annotationsChangedThread != NULL)
    {
        annotationsChangedThread->disregard();
    }

    annotationsChangedThread = new GetEntityAnnotationsThread(mainWindow->getConsoleURL(), entityId);
    connect(annotationsChangedThread, SIGNAL(gotResults(const void *)),
            this, SLOT(annotationsChangedResults(const void *)));
    connect(annotationsChangedThread, SIGNAL(gotError(const QString &)),
            this, SLOT(annotationsChangedError(const QString &)));
    annotationsChangedThread->start(QThread::NormalPriority);
}

void ConsoleObserver::annotationsChangedResults(const void *results)
{
    QMutexLocker locker(&annotationsChangedMutex);
    GetEntityAnnotationsThread *t = (GetEntityAnnotationsThread *)annotationsChangedThread;
    emit updateAnnotations(t->getEntityId(), (AnnotationList *)results, t->getUserColorMap());
    annotationsChangedThread = NULL;
}

void ConsoleObserver::annotationsChangedError(const QString & error)
{
    QMutexLocker locker(&annotationsChangedMutex);
    emit communicationError(error);
    annotationsChangedThread = NULL;
}

//*******************************************************************************************
// sessionSelected event
//*******************************************************************************************

void ConsoleObserver::sessionSelected(qint64 sessionId)
{
    QMutexLocker locker(&loadAnnotationSessionMutex);

    // qDebug() << "Got signal sessionSelected:" << sessionId;

    if (loadAnnotationSessionThread != NULL)
    {
        loadAnnotationSessionThread->disregard();
    }

    loadAnnotationSessionThread = new GetAnnotationSessionThread(mainWindow->getConsoleURL(), sessionId);
    connect(loadAnnotationSessionThread, SIGNAL(gotResults(const void *)),
            this, SLOT(loadAnnotationSessionResults(const void *)));
    connect(loadAnnotationSessionThread, SIGNAL(gotError(const QString &)),
            this, SLOT(loadAnnotationSessionError(const QString &)));
    loadAnnotationSessionThread->start(QThread::NormalPriority);
}

void ConsoleObserver::loadAnnotationSessionResults(const void *results)
{
    QMutexLocker locker(&loadAnnotationSessionMutex);

    emit openAnnotationSession((AnnotationSession *)results);
    loadAnnotationSessionThread = NULL;
}

void ConsoleObserver::loadAnnotationSessionError(const QString & error)
{
    QMutexLocker locker(&loadAnnotationSessionMutex);

    emit communicationError(error);
    loadAnnotationSessionThread = NULL;
}


//*******************************************************************************************
// sessionDeselected event
//*******************************************************************************************

void ConsoleObserver::sessionDeselected()
{
    // qDebug() << "Got signal sessionDeselected";

    emit closeAnnotationSession();
}

