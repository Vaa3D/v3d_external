#ifndef ANNOTATION_WIDGET_H_
#define ANNOTATION_WIDGET_H_

#include "../entity_model/AnnotatedBranch.h"
#include <QFrame>
#include <QEvent>

class Entity;
class Ontology;
class OntologyTreeModel;
class AnnotatedBranch;
class AnnotatedBranchTreeModel;
class ConsoleObserver;
class NaMainWindow;
class QModelIndex;

namespace obs {
    class ConsoleObserverServiceImpl;
}

namespace Ui {
    class AnnotationWidget;
}

class AnnotationWidget : public QFrame
{
    Q_OBJECT

public:
    explicit AnnotationWidget(QWidget *parent = 0);
    ~AnnotationWidget();
    void setMainWindow(NaMainWindow *mainWindow);

signals:
    void entitySelected(const Entity *entity);

public slots:
    void setOntology(Ontology *ontology);
    void openAnnotatedBranch(AnnotatedBranch *annotatedBranch);
    void updateAnnotations(long entityId, AnnotationList *annotations);
    void communicationError(const QString & errorMessage);
    void consoleConnect();
    void consoleDisconnect();
    void ontologyTreeDoubleClicked(const QModelIndex & index);
    void annotatedBranchTreeClicked(const QModelIndex & index);
    void annotateSelectedEntityWithOntologyTerm(const Entity *ontologyTerm);
    void selectEntity(const Entity *entity);

protected:
    bool eventFilter (QObject* watched_object, QEvent* e);
    void showErrorDialog(const QString & text);
    void showDisconnected();

private:

    // User interface
    Ui::AnnotationWidget *ui;
    NaMainWindow *naMainWindow;

    // Console Observer
    obs::ConsoleObserverServiceImpl *consoleObserverService;
    ConsoleObserver *consoleObserver;

    // Ontology
    Ontology *ontology;
    OntologyTreeModel *ontologyTreeModel;

    // Annotations
    AnnotatedBranch *annotatedBranch;
    AnnotatedBranchTreeModel *annotatedBranchTreeModel;

    // Currently selected entity
    const Entity *selectedEntity;

    // Internal variables
    bool firstLoad;
};

#endif // ANNOTATION_WIDGET_H_
