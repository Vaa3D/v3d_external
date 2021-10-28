#include "../../3drenderer/v3dr_common.h" // Pick up Glee first
#include "AnnotationWidget.h"
#include "ui_AnnotationWidget.h"
#include "trees/OntologyTreeModel.h"
#include "trees/AnnotatedBranchTreeModel.h"
#include "trees/EntityTreeItem.h"
#include "NaMainWindow.h"
#include "../data_model/NeuronSelectionModel.h"
#include "../entity_model/Entity.h"
#include "../entity_model/Ontology.h"
#include "../entity_model/OntologyAnnotation.h"
#include "../entity_model/AnnotationSession.h"
#include "../../webservice/impl/ConsoleObserverServiceImpl.h"
#include "../utility/ConsoleObserver.h"
#include "../utility/DataThread.h"
#include "../utility/JacsUtil.h"
#include <QModelIndex>
#ifdef USE_Qt5
  #include <QtWidgets>
#else
  #include <QtGui>
#endif

// Compiler bug workaround borrowed from 3drenderer/qtr_widget.h
#ifdef Q_WS_X11
#define QEVENT_KEY_PRESS 6 //for crazy RedHat error: expected unqualified-id before numeric constant
#else
#define QEVENT_KEY_PRESS QEvent::KeyPress
#endif

#define CONSOLE_CONNECT_RETRY_INTERVAL_MSECS 3000
#define LABEL_NONE "None"
#define LABEL_CONSOLE_DISCONNECTED "Console not connected"
#define LABEL_CONSOLE_CONNECTED "Console connected"
#define BUTTON_CONNECT "Connect"
#define BUTTON_CONNECTING "Connecting..."
#define BUTTON_DISCONNECT "Disconnect"
#define BUTTON_DISCONNECTING "Disconnecting..."
#define LABEL_CONSOLE_DESYCNED "Console not synced"
#define LABEL_CONSOLE_SYNCED "Console synced"
#define BUTTON_SYNC "Synchronize"
#define BUTTON_SYNCING "Synchronizing..."

AnnotationWidget::AnnotationWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::AnnotationWidget),
    naMainWindow(0),
    consoleObserverService(0),
    consoleObserver(0),
    ontology(0),
    ontologyTreeModel(0),
    annotatedBranch(0),
    annotatedBranchTreeModel(0),
    selectedEntity(0),
    createAnnotationThread(0),
    removeAnnotationThread(0),
    selectEntityThread(0),
    mutex(QMutex::Recursive),
    retries(0)
{
    ui->setupUi(this);
    connect(ui->ontologyTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(ontologyTreeDoubleClicked(QModelIndex)));
    connect(ui->annotatedBranchTreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(annotatedBranchTreeClicked(QModelIndex)));
    connect(ui->annotatedBranchTreeView, SIGNAL(removeAnnotation(const Entity*)), this, SLOT(removeAnnotation(const Entity*)));
    connect(this, SIGNAL(entitySelected(const Entity*)), this, SLOT(entityWasSelected(const Entity*)));
    showDisconnected();
}

AnnotationWidget::~AnnotationWidget()
{
    closeOntology();
    closeAnnotatedBranch();
    if (ui != 0) delete ui;
    if (consoleObserver != NULL) {
        delete consoleObserver;
        consoleObserver = NULL;
    }
    if (consoleObserverService != 0) consoleObserverService->stopServer(); // will delete itself later
    if (createAnnotationThread != 0) createAnnotationThread->disregard(); // will delete itself later
    if (removeAnnotationThread != 0) removeAnnotationThread->disregard(); // will delete itself later
    if (selectEntityThread != 0) selectEntityThread->disregard(); // will delete itself later
}

void AnnotationWidget::setMainWindow(NaMainWindow *mainWindow)
{
    naMainWindow = mainWindow;
}

//*************************************************************************************
// Actions
//*************************************************************************************

void AnnotationWidget::closeOntology()
{
    QMutexLocker locker(&mutex);

    if (this->ontology != 0)
    {
        delete this->ontology;
        this->ontology = 0;
    }

    if (ui->ontologyTreeView->model() != 0)
    {
        delete ui->ontologyTreeView->model();
        ui->ontologyTreeView->setModel(0);
    }
}

void AnnotationWidget::openOntology(Ontology *ontology)
{
    QMutexLocker locker(&mutex);

    closeOntology();
    this->ontology = ontology;

    if (ontology != NULL && ontology->root() != NULL && ontology->root()->name != NULL)
    {
        ui->ontologyTreeTitle->setText(*ontology->root()->name);

        ontologyTreeModel = new OntologyTreeModel(ontology);
        ui->ontologyTreeView->setModel(ontologyTreeModel);
        ui->ontologyTreeView->expandAll();

        if (ui->ontologyTreeView->header()->visualIndex(1) > 0)
        {
            ui->ontologyTreeView->header()->swapSections(1,0);
        }

        ui->ontologyTreeView->resizeColumnToContents(0);
        ui->ontologyTreeView->resizeColumnToContents(1);
    }
    else
    {
        ui->ontologyTreeTitle->setText("Error loading ontology");
    }
}

void AnnotationWidget::closeAnnotationSession()
{
    QMutexLocker locker(&mutex);

    if (this->annotationSession != 0)
    {
        delete this->annotationSession;
        this->annotationSession = 0;
    }

    // TODO: propagate this change to any relevant GUI components
}

void AnnotationWidget::openAnnotationSession(AnnotationSession *annotationSession)
{
    QMutexLocker locker(&mutex);

    closeAnnotationSession();
    this->annotationSession = annotationSession;

    if (annotationSession != NULL && annotationSession->sessionId != NULL)
    {
        // qDebug() << "Loaded session"<<*annotationSession->sessionId;
    }
    else
    {
        qDebug() << "Error loading session";
    }
}

void AnnotationWidget::closeAnnotatedBranch()
{
    QMutexLocker locker(&mutex);

    if (this->annotatedBranch != 0)
    {
        delete this->annotatedBranch;
        this->annotatedBranch = 0;
    }

    if (ui->annotatedBranchTreeView->model() != 0)
    {
        delete ui->annotatedBranchTreeView->model();
        ui->annotatedBranchTreeView->setModel(0);
    }

    this->selectedEntity = 0;
}

void AnnotationWidget::openAnnotatedBranch(AnnotatedBranch *annotatedBranch, bool openStack)
{
    QMutexLocker locker(&mutex);

    if (annotatedBranch == 0 || annotatedBranch->entity() == NULL)
    {
        ui->annotatedBranchTreeTitle->setText("Error loading annotations");
        closeAnnotatedBranch();
        return;
    }

    bool reload = (this->annotatedBranch != 0 && *annotatedBranch->entity()->id == *this->annotatedBranch->entity()->id);
    qint64 selectedEntityId = selectedEntity==0?-1:*selectedEntity->id;

    // Don't delete things if we're just reopening the same branch
    if (annotatedBranch != this->annotatedBranch)
    {
        closeAnnotatedBranch();
        this->annotatedBranch = annotatedBranch;
    }

    annotatedBranchTreeModel = new AnnotatedBranchTreeModel(annotatedBranch);
    ui->annotatedBranchTreeView->setModel(annotatedBranchTreeModel);

    // Expand the root
    ui->annotatedBranchTreeView->expand(annotatedBranchTreeModel->indexForId(*annotatedBranch->entity()->id));

    // Expand the root's children
    QSetIterator<EntityData *> i(annotatedBranch->entity()->entityDataSet);
    while (i.hasNext())
    {
        EntityData *ed = i.next();
        Entity *childEntity = ed->childEntity;
        if (childEntity==0) continue;
        if (*childEntity->entityType == "Supporting Data") continue;
        ui->annotatedBranchTreeView->expand(annotatedBranchTreeModel->indexForId(*childEntity->id));
    }

    ui->annotatedBranchTreeView->resizeColumnToContents(0);
    if (openStack) {
        QString path = annotatedBranch->getFilePath();
        QString li_path = annotatedBranch->getLosslessImage();
        QString vli_path = annotatedBranch->getVisuallyLosslessImage();
        QString ch_spec = annotatedBranch->getChannelSpecification();
        naMainWindow->setAuxillaryImagery(li_path, vli_path, ch_spec);
        if (naMainWindow->openMulticolorImageStack(path)) {
            // Populate physical voxel size from entity model
            QString ores = annotatedBranch->entity()->getValueByAttributeName("Optical Resolution");
            if (!ores.isEmpty()) {
                // e.g. "0.62x0.62x0.62"
                QStringList dims = ores.split("x");
                if (dims.size() == 3) {
                    bool ok = false;
                    double x = dims[0].toDouble(&ok);
                    double z = dims[2].toDouble(&ok);
                    if ((x > 0.0) && (z/x > 0.0)) {
                        naMainWindow->getDataFlowModel()->setZRatio(z/x);
                        naMainWindow->get3DWidget()->setThickness(z/x);
                        naMainWindow->get3DWidget()->xVoxelSizeInMicrons = x;
                    }
                }
            }
        }
    }

    // Reselect the entity that was previously selected
    if (reload && selectedEntityId >= 0)
        ui->annotatedBranchTreeView->selectEntity(selectedEntityId);
}

void AnnotationWidget::updateAnnotations(qint64 entityId, AnnotationList *annotations, UserColorMap *userColorMap)
{
    QMutexLocker locker(&mutex);

    if (this->annotatedBranch == 0) return;

    AnnotationList *alist = annotatedBranch->getAnnotations(entityId);
    if (alist==0) return;

    // The next step could cause our currently selected annotation to be deleted, so lets make sure to deselect it.
    QListIterator<Entity *> i(*alist);
    while (i.hasNext())
    {
        Entity *annot = i.next();
        if (selectedEntity == annot) selectedEntity = 0;
    }

    annotatedBranch->updateAnnotations(entityId, annotations, userColorMap);

    // TODO: this can be made more efficient in the future, for now let's just recreate it from scratch
    openAnnotatedBranch(annotatedBranch, false);
}

void AnnotationWidget::updateCurrentSample(Entity *sample)
{
    naMainWindow->setTitle(QString("%1, %2").arg(*sample->name).arg(this->annotatedBranch->name()));
}


//*************************************************************************************
// Console link
//*************************************************************************************

void AnnotationWidget::communicationError(const QString & errorMessage)
{
    QString msg = QString("Error communicating with the Console: %1").arg(errorMessage);
    showErrorDialog(msg);
}

void AnnotationWidget::showErrorDialog(const QString & text)
{
    QMessageBox msgBox(QMessageBox::Critical, "Error", text, QMessageBox::Ok, this);
    msgBox.exec();
}

void AnnotationWidget::showDisconnected()
{
    QMutexLocker locker(&mutex);

    // Clear the ontology
    ui->ontologyTreeView->setModel(NULL);
    if (this->ontology != 0) delete this->ontology;
    this->ontology = 0;
    ui->ontologyTreeTitle->setText(LABEL_NONE);

    // Clear the annotations
    ui->annotatedBranchTreeView->setModel(NULL);
    if (this->annotatedBranch != 0) delete this->annotatedBranch;
    this->annotatedBranch = 0;

    // Reset the console link
    ui->consoleLinkLabel->setText(LABEL_CONSOLE_DISCONNECTED);
    ui->consoleLinkButton->setText(BUTTON_CONNECT);
    ui->consoleLinkButton->setEnabled(true);
    ui->consoleLinkButton->disconnect();
    connect(ui->consoleLinkButton, SIGNAL(clicked()), this, SLOT(consoleConnect()));

    // Reset console sy
    showDesynced();
    ui->consoleSyncButton->setEnabled(false);
}

void AnnotationWidget::showDesynced() {

    QMutexLocker locker(&mutex);

    // Reset the console sync
    ui->consoleSyncLabel->setText(LABEL_CONSOLE_DESYCNED);
    ui->consoleSyncButton->setText(BUTTON_SYNC);
    ui->consoleSyncButton->setEnabled(true);
    ui->consoleSyncButton->disconnect();
    connect(ui->consoleLinkButton, SIGNAL(clicked()), this, SLOT(consoleSync()));
}

void AnnotationWidget::showSynced() {

    QMutexLocker locker(&mutex);

    // Reset the console sync
    ui->consoleSyncLabel->setText(LABEL_CONSOLE_SYNCED);
    ui->consoleSyncButton->setText(BUTTON_SYNC);
    ui->consoleSyncButton->setEnabled(false);
    ui->consoleSyncButton->disconnect();
    connect(ui->consoleLinkButton, SIGNAL(clicked()), this, SLOT(consoleSync()));
}

void AnnotationWidget::consoleConnect() {

    QMutexLocker locker(&mutex);

    qDebug() << "Connecting to the console, retries=" << retries;

    ui->consoleLinkButton->setText(BUTTON_CONNECTING);
    ui->consoleLinkButton->setEnabled(false);

    if (consoleObserver != NULL)
        delete consoleObserver;
    consoleObserver = new ConsoleObserver(naMainWindow);
    connect(consoleObserver, SIGNAL(openOntology(Ontology*)), this, SLOT(openOntology(Ontology*)));
    connect(consoleObserver, SIGNAL(openAnnotatedBranch(AnnotatedBranch*)), this, SLOT(openAnnotatedBranch(AnnotatedBranch*)));
    connect(consoleObserver, SIGNAL(openAnnotationSession(AnnotationSession*)), this, SLOT(openAnnotationSession(AnnotationSession*)));
    connect(consoleObserver, SIGNAL(closeAnnotationSession()), this, SLOT(closeAnnotationSession()));
    connect(consoleObserver, SIGNAL(updateAnnotations(qint64,AnnotationList*,UserColorMap*)), this, SLOT(updateAnnotations(qint64,AnnotationList*,UserColorMap*)));
    connect(consoleObserver, SIGNAL(selectEntityById(qint64,bool)), this, SLOT(selectEntityById(qint64,bool)));
    connect(consoleObserver, SIGNAL(communicationError(const QString&)), this, SLOT(communicationError(const QString&)));
    connect(consoleObserver, SIGNAL(updateCurrentSample(Entity*)), this, SLOT(updateCurrentSample(Entity*)));
    connect(consoleObserver, SIGNAL(openStackWithVaa3d(Entity*)), naMainWindow, SLOT(loadSingleStack(Entity*)));

    consoleObserverService = new obs::ConsoleObserverServiceImpl(naMainWindow->getConsoleURL());

    if (consoleObserverService->errorMessage()!=0)
    {
        showDisconnected();
        if (retries>0)
        {
            retries--;
            QTimer::singleShot(CONSOLE_CONNECT_RETRY_INTERVAL_MSECS, this, SLOT(consoleConnect()));
        }
        else if (ui->ontologyTreeView->isVisible()) {
            QString msg = QString("Could not connect to the Console: %1").arg(*consoleObserverService->errorMessage());
            // showErrorDialog(msg);
        }
        return;
    }

    qDebug() << "Received console approval to run on port:"<<consoleObserverService->port();
    connect(consoleObserverService, SIGNAL(ontologySelected(qint64)), consoleObserver, SLOT(ontologySelected(qint64)));
    connect(consoleObserverService, SIGNAL(ontologyChanged(qint64)), consoleObserver, SLOT(ontologyChanged(qint64)));
    connect(consoleObserverService, SIGNAL(entitySelected(const QString &, const QString &,bool)), consoleObserver, SLOT(entitySelected(const QString &,const QString &,bool)));
    connect(consoleObserverService, SIGNAL(entityDeselected(const QString &, const QString &)), consoleObserver, SLOT(entityDeselected(const QString &,const QString &)));
    connect(consoleObserverService, SIGNAL(entityViewRequested(qint64)), consoleObserver, SLOT(entityViewRequested(qint64)));
    connect(consoleObserverService, SIGNAL(annotationsChanged(qint64)), consoleObserver, SLOT(annotationsChanged(qint64)));
    connect(consoleObserverService, SIGNAL(sessionSelected(qint64)), consoleObserver, SLOT(sessionSelected(qint64)));
    connect(consoleObserverService, SIGNAL(sessionDeselected()), consoleObserver, SLOT(sessionDeselected()));
    consoleObserverService->startServer();

    if (consoleObserverService->error!=0)
    {
        consoleObserverService->stopServer(); // will delete itself later
        qDebug() << "Could not start Console observer, error code:" << consoleObserverService->error;
        QString msg = QString("Could not bind to port %1").arg(consoleObserverService->port());
        showErrorDialog(msg);
        showDisconnected();
        return;
    }

    consoleObserverService->registerWithConsole();

    if (consoleObserverService->errorMessage()!=0)
    {
        consoleObserverService->stopServer(); // will delete itself later
        QString msg = QString("Could not register with the Console: %1").arg(*consoleObserverService->errorMessage());
        showErrorDialog(msg);
        ui->consoleLinkButton->setText(BUTTON_CONNECT);
        ui->consoleLinkButton->setEnabled(true);
        return;
    }

    // qDebug() << "Registered with console and listening for events";

    ui->consoleLinkButton->disconnect();
    connect(ui->consoleLinkButton, SIGNAL(clicked()), this, SLOT(consoleDisconnect()));

    ui->consoleLinkLabel->setText(LABEL_CONSOLE_CONNECTED);
    ui->consoleLinkButton->setText(BUTTON_DISCONNECT);
    ui->consoleLinkButton->setEnabled(true);

    ui->consoleSyncLabel->setText(LABEL_CONSOLE_SYNCED);
    ui->consoleSyncButton->setText(BUTTON_SYNC);
    ui->consoleSyncButton->setEnabled(false);
}

void AnnotationWidget::consoleConnect(int retries) {
    this->retries = retries;
    consoleConnect();
}

void AnnotationWidget::consoleDisconnect()
{
    QMutexLocker locker(&mutex);

    if (consoleObserverService == NULL) return;

    ui->consoleLinkButton->setText(BUTTON_DISCONNECTING);
    ui->consoleLinkButton->setEnabled(false);
    consoleObserverService->stopServer(); // will delete itself later

    showDisconnected();
}

void AnnotationWidget::consoleSync() {

    QMutexLocker locker(&mutex);

    ui->consoleSyncButton->setText(BUTTON_SYNCING);
    ui->consoleSyncButton->setEnabled(false);


}


//*************************************************************************************
// Annotation
//*************************************************************************************

bool AnnotationWidget::eventFilter(QObject* watched_object, QEvent* event)
{
    bool filtered = false;
    if (event->type() == QEVENT_KEY_PRESS)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        int keyInt = keyEvent->key();
        Qt::Key qtKey = static_cast<Qt::Key>(keyInt);

        // Qt doesn't understand this key code
        if (qtKey == Qt::Key_unknown)
        {
            qDebug() << "Unknown key:"<<qtKey;
            return false;
        }

        // Ignore this event if only a modifier was pressed by itself
        if (qtKey == Qt::Key_Control || qtKey == Qt::Key_Shift || qtKey == Qt::Key_Alt || qtKey == Qt::Key_Meta)
        {
            return false;
        }

        // Create key sequence
        Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
        if (modifiers & Qt::ShiftModifier) keyInt += Qt::SHIFT;
        if (modifiers & Qt::ControlModifier) keyInt += Qt::CTRL;
        if (modifiers & Qt::AltModifier) keyInt += Qt::ALT;
        if (modifiers & Qt::MetaModifier) keyInt += Qt::META;
        QKeySequence keySeq(keyInt);
        // qDebug() << "Key sequence:"<<keySeq.toString(QKeySequence::NativeText) << "Portable:"<< keySeq.toString(QKeySequence::PortableText);

        if (ontology != NULL) {
            QMap<QKeySequence, qint64>::const_iterator i = ontology->keyBindMap()->constBegin();
            while (i != ontology->keyBindMap()->constEnd())
            {
                QKeySequence keyBind = i.key();
                if (keyBind.matches(keySeq) == QKeySequence::ExactMatch)
                {
                    Entity *termEntity = ontology->getTermById(i.value());
                    Entity *parentEntity = ontology->getParentById(i.value());
                    ui->ontologyTreeView->selectEntity(*termEntity->id);
                    annotateSelectedEntityWithOntologyTerm(termEntity, parentEntity);
                    filtered = true;
                }
                ++i;
            }
        }
    }
    return filtered;
}

void AnnotationWidget::ontologyTreeDoubleClicked(const QModelIndex & index)
{
    EntityTreeItem *item = ontologyTreeModel->node(index);
    if (item->entity() != 0)
    {
        annotateSelectedEntityWithOntologyTerm(item->entity(), item->parent()==0?0:item->parent()->entity());
    }
}

// This reimplements the Console's AnnotateAction
void AnnotationWidget::annotateSelectedEntityWithOntologyTerm(const Entity *term, const Entity *parentTerm)
{
    QMutexLocker locker(&mutex);

    QString termType = term->getValueByAttributeName("Ontology Term Type");

    if (selectedEntity == NULL) return; // Nothing to annotate
    if (termType == "Category" || termType == "Enum") return; // Cannot use these types to annotate


    if (termType == "EnumText") {
        // TODO: support this in the future by reimplementing the Console's logic here
        QString msg = QString("Annotation with terms of type 'Enumeration Text' is not currently supported by this tool. Please use the FlyWorkstation Console to perform this annotation.");
        showErrorDialog(msg);
        return;
    }

    if (*selectedEntity->entityType == "Annotation") return; // Cannot annotate annotations

    // qDebug() << "Annotate"<<(selectedEntity==0?"":*selectedEntity->name)<<"with"<<(term == NULL ? "NULL" : *term->name) << "id="<< *term->id<< "type="<<termType;

    // Get input value, if required
    QString *value = 0;

    if (termType == "Interval")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Annotating with interval", "Value:", QLineEdit::Normal, "", &ok);

        if (ok && !text.isEmpty())
        {
            ok = false;
            double val = text.toDouble(&ok);
            double lowerBound = term->getValueByAttributeName("Ontology Term Type Interval Lower Bound").toDouble(&ok);
            double upperBound = term->getValueByAttributeName("Ontology Term Type Interval Upper Bound").toDouble(&ok);

            if (!ok || val < lowerBound || val > upperBound) {
                QString msg = QString("Input must be in range: [%1,%2]").arg(lowerBound).arg(upperBound);
                showErrorDialog(msg);
                return;
            }

            value = new QString(text);
        }
    }
    else if (termType == "Text")
    {
        bool ok;
        QString text = QInputDialog::getText(this, "Annotating with text", "Value:", QLineEdit::Normal, "", &ok);

        if (ok && !text.isEmpty())
        {
            value = new QString(text);
        }
    }

    OntologyAnnotation *annotation = new OntologyAnnotation();

    if (annotationSession != 0) {
        annotation->sessionId = new qint64(*annotationSession->sessionId);
    }

    annotation->targetEntityId = new qint64(*selectedEntity->id);
    annotation->keyEntityId = new qint64(*term->id);
    annotation->valueEntityId = 0;
    annotation->keyString = new QString(*term->name);
    annotation->valueString = value;

    if (termType == "EnumItem") {
        if (parentTerm == 0)
        {
            qDebug() << "WARNING: Enum element has no parent! Aborting annotation.";
            return;
        }
        annotation->keyEntityId = new qint64(*parentTerm->id);
        annotation->valueEntityId = new qint64(*term->id);
        annotation->keyString = new QString(*parentTerm->name);
        annotation->valueString = new QString(*term->name);
    }

    if (createAnnotationThread != NULL) createAnnotationThread->disregard();
    createAnnotationThread = new CreateAnnotationThread(naMainWindow->getConsoleURL(), annotation);

    connect(createAnnotationThread, SIGNAL(gotResults(const void *)),
            this, SLOT(createAnnotationResults(const void *)));
    connect(createAnnotationThread, SIGNAL(gotError(const QString &)),
            this, SLOT(createAnnotationError(const QString &)));
    createAnnotationThread->start(QThread::NormalPriority);
}

void AnnotationWidget::createAnnotationResults(const void *results)
{
    QMutexLocker locker(&mutex);

    // Succeeded but we can ignore this. The console will fire an annotationsChanged event so that we update the UI.
    createAnnotationThread = NULL;
}

void AnnotationWidget::createAnnotationError(const QString &error)
{
    QMutexLocker locker(&mutex);

    QString msg = QString("Annotation error: %1").arg(error);
    showErrorDialog(msg);
    createAnnotationThread = NULL;
}

// This reimplements the Console's EntityTagCloudPanel.deleteTag
void AnnotationWidget::removeAnnotation(const Entity *annotation)
{
    QMutexLocker locker(&mutex);

    if (annotation == NULL) return; // Nothing to annotate
    if (*annotation->entityType != "Annotation") return; // Cannot remove non-annotations

    // qDebug() << "Removing Annotation"<<*annotation->name;

    if (removeAnnotationThread != NULL) removeAnnotationThread->disregard();
    removeAnnotationThread = new RemoveAnnotationThread(naMainWindow->getConsoleURL(), *annotation->id);

    connect(removeAnnotationThread, SIGNAL(gotResults(const void *)),
            this, SLOT(removeAnnotationResults(const void *)));
    connect(removeAnnotationThread, SIGNAL(gotError(const QString &)),
            this, SLOT(removeAnnotationError(const QString &)));
    removeAnnotationThread->start(QThread::NormalPriority);
}

void AnnotationWidget::removeAnnotationResults(const void *results)
{
    QMutexLocker locker(&mutex);

    // Succeeded but we can ignore this. The console will fire an annotationsChanged event so that we update the UI.
    removeAnnotationThread = NULL;
}

void AnnotationWidget::removeAnnotationError(const QString &error)
{
    QMutexLocker locker(&mutex);

    QString msg = QString("Annotation error: %1").arg(error);
    showErrorDialog(msg);
    removeAnnotationThread = NULL;
}

//*************************************************************************************
// Entity and Neuron Selection
//*************************************************************************************

void AnnotationWidget::annotatedBranchTreeClicked(const QModelIndex & index)
{
    QMutexLocker locker(&mutex);

    EntityTreeItem *item = annotatedBranchTreeModel->node(index);
    if (item->entity() != 0) selectEntity(item->entity());
}

void AnnotationWidget::selectFragment(const Entity *entity) {
    int neuronNum = getNeuronNumber(entity);
    if (neuronNum >= 0) {
        // qDebug() << "AnnotationWidget::neuronSelected"<<neuronNum;
        naMainWindow->getDataFlowModel()->getNeuronSelectionModel().selectExactlyOneNeuron(neuronNum);
//            emit neuronSelected(neuronNum);
        // qDebug() << "AnnotationWidget::emitted neuronSelected"<<neuronNum;
    }
}

void AnnotationWidget::entityWasSelected(const Entity *entity)
{
    if (entity==0)
    {
        return;
    }
    else if (*entity->entityType == "Curated Neuron")
    {
        QListIterator<EntityData *> i(entity->getOrderedEntityData());
        while (i.hasNext())
        {
            EntityData *ed = i.next();
            Entity *entity = ed->childEntity;
            if (entity != NULL)
            {
                if (*entity->entityType == "Neuron Fragment")
                {
                    selectFragment(entity);
                }
            }
        }
    }
    else if (*entity->entityType == "Neuron Fragment")
    {
        selectFragment(entity);
    }
    else {
        emit neuronsDeselected();
    }
}

void AnnotationWidget::selectEntity(const Entity *entity)
{
    selectEntity(entity, false);
}

void AnnotationWidget::selectEntity(const Entity *entity, const bool external)
{
    // qDebug() << "AnnotationWidget::selectEntity"<<(entity==0?"None":*entity->name)<<"external?"<<external;

    {
        QMutexLocker locker(&mutex);

        if ((selectedEntity==entity) || (selectedEntity!=0 && entity!=0 && *selectedEntity->id==*entity->id)) return;
        selectedEntity = entity;
        ui->annotatedBranchTreeView->clearSelection();
        if (entity!=0) ui->annotatedBranchTreeView->selectEntity(*entity->id);
    } // release lock before emit
    emit entitySelected(selectedEntity);

//    {
//        QMutexLocker locker(&mutex);
//        if (!external)
//        {
//            if (entity!=0)
//            {
//                qDebug() << "AnnotationWidget::selectEntity (notifying console)"<<*entity->id;
//                selectEntityThread = new SelectEntityThread(*entity->id);
//                selectEntityThread->start(QThread::NormalPriority);
//            }
//            else
//            {
//                // TODO: deselectEntity event?
//            }
//        }
//    }
}

void AnnotationWidget::selectEntityResults(const void *results)
{
    QMutexLocker locker(&mutex);

    // Succeeded but we can ignore this.
    selectEntityThread = NULL;
}

void AnnotationWidget::selectEntityError(const QString &error)
{
    QMutexLocker locker(&mutex);

    QString msg = QString("Entity selection error: %1").arg(error);
    showErrorDialog(msg);
    selectEntityThread = NULL;
}
void AnnotationWidget::selectEntityById(const qint64 & entityId)
{
    selectEntityById(entityId, false);
}

void AnnotationWidget::selectEntityById(const qint64 & entityId, const bool external)
{
    // qDebug() << "AnnotationWidget::selectEntityById"<<entityId<<"external?"<<external;
    {
        QMutexLocker locker(&mutex);

        if (annotatedBranch==0) return;
        Entity *entity = annotatedBranch->getEntityById(entityId);
        if (entity!=0) selectEntity(entity, external);
    }
}

void AnnotationWidget::selectNeuron(int index)
{
    // qDebug() << "AnnotationWidget::selectNeuron"<<index;
    QMutexLocker locker(&mutex);

    if (!annotatedBranch) return;

    Entity *fragmentParentEntity = annotatedBranch->entity();
    EntityData *nfed = annotatedBranch->entity()->getEntityDataByAttributeName("Neuron Fragments");
    if (nfed!=0 && nfed->childEntity!=0) {
        fragmentParentEntity = nfed->childEntity;
    }

    // Look for slightly different entity structure observed May 2013 CMB
    if (nfed == 0) {
        nfed = annotatedBranch->entity()->getEntityDataByAttributeName("Mask Entity Collection");
        if (nfed!=0 && nfed->childEntity!=0) {
            fragmentParentEntity = nfed->childEntity;
        }
    }

    QSetIterator<EntityData *> i(fragmentParentEntity->entityDataSet);
    while (i.hasNext())
    {
        EntityData *ed = i.next();
        Entity *entity = ed->childEntity;
        if (entity==0) continue;
        int testIndex = getNeuronNumber(entity);
        if (testIndex == index && selectedEntity!=entity)
        {
            if (entity->entityType->endsWith("2D Image")) // TODO: remove this case in the future
            {
                // This case is deprecated
                selectEntity(entity);
                return;
            }
            else if (*entity->entityType == "Neuron Fragment")
            {
                selectEntity(entity);
                return;
            }
        }
    }
}

void AnnotationWidget::deselectNeurons()
{
    QMutexLocker locker(&mutex);
    // qDebug() << "AnnotationWidget::deselectNeurons";

    if (selectedEntity==0||getNeuronNumber(selectedEntity)<0) return; // No neurons are selected
    selectEntity(0);
}
