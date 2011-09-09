#include "AnnotationWidget.h"
#include "ui_AnnotationWidget.h"
#include "trees/OntologyTreeModel.h"
#include "trees/AnnotatedBranchTreeModel.h"
#include "trees/EntityTreeItem.h"
#include "NaMainWindow.h"
#include "../entity_model/Entity.h"
#include "../entity_model/Ontology.h"
#include "../../webservice/impl/ConsoleObserverServiceImpl.h"
#include "../utility/ConsoleObserver.h"
#include <QModelIndex>
#include <QtGui>

// Compiler bug workaround borrowed from 3drenderer/qtr_widget.h
#ifdef Q_WS_X11
#define QEVENT_KEY_PRESS 6 //for crazy RedHat error: expected unqualified-id before numeric constant
#else
#define QEVENT_KEY_PRESS QEvent::KeyPress
#endif

#define LABEL_NONE "None"
#define LABEL_CONSOLE_DICONNECTED "Console not connected"
#define LABEL_CONSOLE_CONNECTED "Console connected"
#define BUTTON_CONNECT "Connect"
#define BUTTON_CONNECTING "Connecting..."
#define BUTTON_DISCONNECT "Disconnect"
#define BUTTON_DISCONNECTING "Disconnecting..."

AnnotationWidget::AnnotationWidget(QWidget *parent) : QFrame(parent), ui(new Ui::AnnotationWidget),
    ontology(0),
    ontologyTreeModel(0),
    annotatedBranch(0),
    annotatedBranchTreeModel(0),
    consoleObserverService(0),
    consoleObserver(0),
    naMainWindow(0),
    firstLoad(true)
{
    ui->setupUi(this);
    connect(ui->ontologyTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(ontologyTreeDoubleClicked(QModelIndex)));
    connect(ui->annotatedBranchTreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(annotatedBranchTreeClicked(QModelIndex)));
    showDisconnected();
}

AnnotationWidget::~AnnotationWidget()
{
    if (ui != 0) delete ui;
    if (consoleObserver != 0) delete consoleObserver;
    if (ontology != 0) delete this->ontology;
    consoleObserverService->stopServer(); // will delete itself later
}

void AnnotationWidget::setMainWindow(NaMainWindow *mainWindow)
{
    naMainWindow = mainWindow;
}

void AnnotationWidget::setOntology(Ontology *ontology)
{
    // Clean up memory from previous ontology if necessary
    if (this->ontology != 0) {
        delete this->ontology;
        delete ui->ontologyTreeView->model();
    }
    this->ontology = ontology;

    if (ontology != NULL && ontology->root() != NULL && ontology->root()->name != NULL)
    {
        ui->ontologyTreeTitle->setText(*ontology->root()->name);

        ontologyTreeModel = new OntologyTreeModel(ontology);
        ui->ontologyTreeView->setModel(ontologyTreeModel);
        ui->ontologyTreeView->expandAll();

        if (firstLoad)
        {
            ui->ontologyTreeView->header()->swapSections(1,0);
            firstLoad = false;
        }

        ui->ontologyTreeView->resizeColumnToContents(0);
        ui->ontologyTreeView->resizeColumnToContents(1);
    }
    else
    {
        ui->ontologyTreeTitle->setText("Error loading ontology");
        ui->ontologyTreeView->setModel(0);
    }
}

void AnnotationWidget::openAnnotatedBranch(AnnotatedBranch *annotatedBranch)
{
    if (this->annotatedBranch != 0)
    {
        delete this->annotatedBranch;
        delete ui->annotatedBranchTreeView->model();
    }

    this->annotatedBranch = annotatedBranch;
    this->selectedEntity = 0;

    if (annotatedBranch != NULL && annotatedBranch->entity() != NULL)
    {
        annotatedBranchTreeModel = new AnnotatedBranchTreeModel(annotatedBranch);
        ui->annotatedBranchTreeView->setModel(annotatedBranchTreeModel);
        ui->annotatedBranchTreeView->expandAll();
        ui->annotatedBranchTreeView->resizeColumnToContents(0);

        naMainWindow->openMulticolorImageStack(annotatedBranch->getFilePath());
    }
    else {
        ui->annotatedBranchTreeTitle->setText("Error loading annotatedBranch");
        ui->annotatedBranchTreeView->setModel(0);
    }
}

void AnnotationWidget::updateAnnotations(long entityId, AnnotationList *annotations)
{
    if (this->annotatedBranch == 0) return;

    annotatedBranch->updateAnnotations(entityId, annotations);

    if (annotatedBranchTreeModel != 0)
        delete annotatedBranchTreeModel;

    // TODO: this can be made more efficient in the future, for now let's just recreate it from scratch
    annotatedBranchTreeModel = new AnnotatedBranchTreeModel(annotatedBranch);
    ui->annotatedBranchTreeView->setModel(annotatedBranchTreeModel);
    ui->annotatedBranchTreeView->expandAll();
    ui->annotatedBranchTreeView->resizeColumnToContents(0);
}

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
    // Clear the ontology
    if (this->ontology != 0) delete this->ontology;
    this->ontology = 0;
    ui->ontologyTreeView->setModel(NULL);
    ui->ontologyTreeTitle->setText(LABEL_NONE);

    // Clear the annotations
    if (this->annotatedBranch != 0) delete this->annotatedBranch;
    this->annotatedBranch = 0;
    ui->annotatedBranchTreeView->setModel(NULL);

    // Reset the console link
    ui->consoleLinkLabel->setText(LABEL_CONSOLE_DICONNECTED);
    ui->consoleLinkButton->setText(BUTTON_CONNECT);
    ui->consoleLinkButton->setEnabled(true);
    ui->consoleLinkButton->disconnect();
    connect(ui->consoleLinkButton, SIGNAL(clicked()), this, SLOT(consoleConnect()));
}

void AnnotationWidget::consoleConnect() {

    ui->consoleLinkButton->setText(BUTTON_CONNECTING);
    ui->consoleLinkButton->setEnabled(false);

    consoleObserver = new ConsoleObserver(naMainWindow);
    connect(consoleObserver, SIGNAL(openAnnotatedBranch(AnnotatedBranch*)), this, SLOT(openAnnotatedBranch(AnnotatedBranch*)));
    connect(consoleObserver, SIGNAL(openOntology(Ontology*)), this, SLOT(setOntology(Ontology*)));
    connect(consoleObserver, SIGNAL(communicationError(const QString&)), this, SLOT(communicationError(const QString&)));

    consoleObserverService = new obs::ConsoleObserverServiceImpl();

    if (consoleObserverService->errorMessage()!=0)
    {
        QString msg = QString("Could not connect to the Console: %1").arg(*consoleObserverService->errorMessage());
        showErrorDialog(msg);
        showDisconnected();
        return;
    }

    qDebug() << "Received console approval to run on port:"<<consoleObserverService->port();

    connect(consoleObserverService, SIGNAL(ontologySelected(long)), consoleObserver, SLOT(ontologySelected(long)));
    connect(consoleObserverService, SIGNAL(ontologyChanged(long)), consoleObserver, SLOT(ontologyChanged(long)));
    connect(consoleObserverService, SIGNAL(entitySelected(long)), consoleObserver, SLOT(entitySelected(long)));
    connect(consoleObserverService, SIGNAL(entityViewRequested(long)), consoleObserver, SLOT(entityViewRequested(long)));
    connect(consoleObserverService, SIGNAL(annotationsChanged(long)), consoleObserver, SLOT(annotationsChanged(long)));
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

    qDebug() << "Registered with console and listening for events";

    ui->consoleLinkButton->disconnect();
    connect(ui->consoleLinkButton, SIGNAL(clicked()), this, SLOT(consoleDisconnect()));

    ui->consoleLinkLabel->setText(LABEL_CONSOLE_CONNECTED);
    ui->consoleLinkButton->setText(BUTTON_DISCONNECT);
    ui->consoleLinkButton->setEnabled(true);

    // Load the current ontology
    ui->ontologyTreeTitle->setText(QString("Loading..."));
    consoleObserver->loadCurrentOntology();
}

void AnnotationWidget::consoleDisconnect()
{
    if (consoleObserverService == NULL) return;

    ui->consoleLinkButton->setText(BUTTON_DISCONNECTING);
    ui->consoleLinkButton->setEnabled(false);
    consoleObserverService->stopServer(); // will delete itself later

    qDebug() << "The consoleObserverService is now is defunct. It will free its own memory within the next"<<CONSOLE_OBSERVER_ACCEPT_TIMEOUT<<"seconds.";
    showDisconnected();
}

void AnnotationWidget::ontologyTreeDoubleClicked(const QModelIndex & index)
{
    EntityTreeItem *item = ontologyTreeModel->node(index);
    if (item->entity() != 0)
        annotateSelectedEntityWithOntologyTerm(item->entity());
}

void AnnotationWidget::annotatedBranchTreeClicked(const QModelIndex & index)
{
    EntityTreeItem *item = annotatedBranchTreeModel->node(index);
    if (item->entity() != 0)
    {
        selectedEntity = item->entity();
        emit entitySelected(selectedEntity);
    }
}

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
        qDebug() << "Key sequence:"<<keySeq.toString(QKeySequence::NativeText) << "Portable:"<< keySeq.toString(QKeySequence::PortableText);

        if (ontology != NULL) {
            QMap<QKeySequence, qint64>::const_iterator i = ontology->keyBindMap()->constBegin();
            while (i != ontology->keyBindMap()->constEnd())
            {
                QKeySequence keyBind = i.key();
                if (keyBind.matches(keySeq) == QKeySequence::ExactMatch)
                {
                    Entity *termEntity = ontology->getTermById(i.value());
                    ui->ontologyTreeView->selectEntity(termEntity);
                    annotateSelectedEntityWithOntologyTerm(termEntity);
                    filtered = true;
                }
                ++i;
            }
        }
    }
    return filtered;
}

void AnnotationWidget::annotateSelectedEntityWithOntologyTerm(const Entity *ontologyTerm)
{
    QString termType = ontologyTerm->getValueByAttributeName("Ontology Term Type");

    if (selectedEntity == NULL) return; // Nothing to annotate

    qDebug() << "Annotate"<<(selectedEntity==0?"":*selectedEntity->name)<<"with"<<(ontologyTerm == NULL ? "NULL" : *ontologyTerm->name) << "id="<< *ontologyTerm->id<< "type="<<termType;

    // TODO: get user input for complex term types
    if (termType == "Text") {

    }

    // TODO: create the annotation

}

void AnnotationWidget::selectEntity(const Entity *entity)
{
    ui->annotatedBranchTreeView->selectEntity(entity);
    selectedEntity = entity;
    emit entitySelected(entity); // This assumes that clients who call selectEntity are smart enough to ignore the subsequent entitySelected signal
}
