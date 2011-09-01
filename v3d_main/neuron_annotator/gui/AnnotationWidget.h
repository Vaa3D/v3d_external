#ifndef ANNOTATION_WIDGET_H_
#define ANNOTATION_WIDGET_H_

#include <QFrame>
#include <QEvent>

class Ontology;
class ConsoleObserver;
class NaMainWindow;

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

public slots:
    void setOntology(Ontology *ontology);
    void consoleConnect();
    void consoleDisconnect();

protected:
    bool eventFilter (QObject* watched_object, QEvent* e);
    void showOntologyError(const QString & text);
    void showErrorDialog(const QString & text);
    void showDisconnected();

private:
    Ontology *ontology;
    Ui::AnnotationWidget *ui;
    obs::ConsoleObserverServiceImpl *consoleObserverService;
    ConsoleObserver *consoleObserver;
    NaMainWindow *naMainWindow;
};

#endif // ANNOTATION_WIDGET_H_
