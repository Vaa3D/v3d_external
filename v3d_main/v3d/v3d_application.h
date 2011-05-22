#ifndef V3D_APPLICATION_H
#define V3D_APPLICATION_H

#include <QApplication>
#include <QMainWindow>
#include "mainwindow.h"
#include "../neuron_annotator/NaMainWindow.h"

class V3dApplication : public QApplication
{
    Q_OBJECT

private:

    static V3dApplication* theApp;
    static MainWindow* mainWindow;
    static NaMainWindow* naMainWindow;
    static bool mainWindowIsActive;
    static bool naMainWindowIsActive;

    explicit V3dApplication(int & argc, char ** argv);

    static void deactivateMainWindowHelper(QMainWindow* qMainWindow) {
        if (qMainWindow!=0) {
            theApp->removeEventFilter(qMainWindow);
            // Remember the window size and position before deactivating
            QPoint windowPosition = qMainWindow->pos();
            QSize windowSize = qMainWindow->size();
            QSettings settings("HHMI", "V3D");
            settings.setValue("pos", windowPosition);
            settings.setValue("size", windowSize);
            qMainWindow->hide();
        }
    }

    static void activateMainWindowHelper(QMainWindow* qMainWindow) {
        if (qMainWindow!=0) {
            theApp->installEventFilter(qMainWindow);
            QSettings settings("HHMI", "V3D");
            QPoint windowPosition = settings.value("pos", QPoint(10, 10)).toPoint();
            QSize windowSize = settings.value("size", QSize(1000, 700)).toSize();
            qMainWindow->move(windowPosition);
            qMainWindow->resize(windowSize);
            qMainWindow->show();
        }
    }

public:
    ~V3dApplication();

    static bool hasMainWindow() {
        if (mainWindow==0) {
            return false;
        }
        return true;
    }

    static void handleCloseEvent(QCloseEvent* event) {
        mainWindow->handleCoordinatedCloseEvent(event);
        if (naMainWindow!=0) {
            naMainWindow->handleCoordinatedCloseEvent(event);
        }
        QCoreApplication::postEvent(theApp, new QEvent(QEvent::Quit)); // this more OK
    }

    static MainWindow* getMainWindow() {
        return mainWindow;
    }

    static NaMainWindow* getNaMainWindow() {
        return naMainWindow;
    }

    static void activateMainWindow() {
        if (mainWindowIsActive==false) {
            activateMainWindowHelper(mainWindow);
            mainWindowIsActive=true;
        }
        if (naMainWindow!=0) {
            naMainWindow->setV3DDefaultModeCheck(true);
        }
        mainWindow->setV3DDefaultModeCheck(true);
    }

    static void deactivateMainWindow() {
        if (mainWindowIsActive==true) {
            deactivateMainWindowHelper(mainWindow);
            mainWindowIsActive=false;
        }
        mainWindow->setV3DDefaultModeCheck(false);
        if (naMainWindow!=0) {
            naMainWindow->setV3DDefaultModeCheck(false);
        }
    }

    static void activateNaMainWindow() {
        if (naMainWindowIsActive==false) {
            if (naMainWindow==0) {
                naMainWindow = new NaMainWindow();
            }
            activateMainWindowHelper(naMainWindow);
            naMainWindowIsActive=true;
        }
        naMainWindow->setNeuronAnnotatorModeCheck(true);
        mainWindow->setNeuronAnnotatorModeCheck(true);
    }

    static void deactivateNaMainWindow() {
        if (naMainWindowIsActive==true) {
            deactivateMainWindowHelper(naMainWindow);
            naMainWindowIsActive=false;
        }
        if (naMainWindow!=0) {
            naMainWindow->setNeuronAnnotatorModeCheck(false);
        }
        mainWindow->setNeuronAnnotatorModeCheck(false);
    }

    static V3dApplication* getInstance(int & argc, char ** argv) {
        if (theApp==0) {
            theApp = new V3dApplication(argc, argv);
        }
        return theApp;
    }

signals:

public slots:


};


#endif // V3D_APPLICATION_H
