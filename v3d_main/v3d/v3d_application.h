//last change: by Hanchuan Peng, 110828. add conditional compilation of mode menu
//last change: by Hanchuan Peng, 120217. add a new constructor V3dApplication()

#ifndef V3D_APPLICATION_H
#define V3D_APPLICATION_H

#include "../3drenderer/v3dr_common.h"

#include <QApplication>
#include <QMainWindow>
#include "mainwindow.h"

#ifdef _ALLOW_WORKMODE_MENU_
#include "../neuron_annotator/gui/NaMainWindow.h"
#endif

#include "v3d_compile_constraints.h"

class V3dApplication : public QApplication
{
    Q_OBJECT

private:

    static V3dApplication* theApp;
    static MainWindow* mainWindow;
    static bool mainWindowIsActive;

#ifdef _ALLOW_WORKMODE_MENU_
    static NaMainWindow* naMainWindow;
    static bool naMainWindowIsActive;
#endif

    explicit V3dApplication(int & argc, char ** argv);

    static void deactivateMainWindowHelper(QMainWindow* qMainWindow) {
        if (qMainWindow!=0) {
            theApp->removeEventFilter(qMainWindow);
            // Remember the window size and position before deactivating
            QPoint windowPosition = qMainWindow->pos();
            QSize windowSize = qMainWindow->size();
            QSettings settings("HHMI", "Vaa3D");
            settings.setValue("pos", windowPosition);
            settings.setValue("size", windowSize);
            qMainWindow->hide();
        }
    }

    static void activateMainWindowHelper(QMainWindow* qMainWindow) {
        if (qMainWindow!=0) {
#ifdef CGS_AUTOLAUNCH
			qMainWindow->resize(QSize(0, 0));
			qMainWindow->hide();
#endif
			theApp->installEventFilter(qMainWindow);
			QSettings settings("HHMI", "Vaa3D");
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

#ifdef _ALLOW_WORKMODE_MENU_
        if (naMainWindow!=0) {
            naMainWindow->handleCoordinatedCloseEvent(event);
        }
#endif
//        mainWindow->close();
//        theApp->exit();
//        closeAllWindows();
//        if(mainWindow){
//            delete mainWindow;
//            mainWindow = 0;
//        }
//        if(naMainWindow){
//            delete naMainWindow;
//            naMainWindow = 0;
//        }
        QCoreApplication::postEvent(theApp, new QEvent(QEvent::Quit));
//        qApp->quit();
    }

    static MainWindow* getMainWindow() {
        return mainWindow;
    }

#ifdef _ALLOW_WORKMODE_MENU_
    static NaMainWindow* getNaMainWindow() {
        return naMainWindow;
    }
#endif

    static void activateMainWindow() {
        if (mainWindowIsActive==false) {
            activateMainWindowHelper(mainWindow);
//            if (mainWindow!=0) {
//#ifdef CGS_AUTOLAUNCH
//                mainWindow->resize(QSize(0, 0));
//                mainWindow->hide();
//#endif
//                theApp->installEventFilter(mainWindow);
//                QSettings settings("HHMI", "Vaa3D");
//                QPoint windowPosition = settings.value("pos", QPoint(10, 10)).toPoint();
//                QSize windowSize = settings.value("size", QSize(1000, 700)).toSize();
//                mainWindow->move(windowPosition);
//                mainWindow->resize(windowSize);
//                mainWindow->hide();
//            }
//            mainWindow->hide();
            mainWindowIsActive=true;
        }

#ifdef _ALLOW_WORKMODE_MENU_
        if (naMainWindow!=0) {
            naMainWindow->setV3DDefaultModeCheck(true);
        }
        mainWindow->setV3DDefaultModeCheck(true);
#endif
    }

    static void deactivateMainWindow() {
        if (mainWindowIsActive==true) {
            deactivateMainWindowHelper(mainWindow);
            mainWindowIsActive=false;
        }
#ifdef _ALLOW_WORKMODE_MENU_
        mainWindow->setV3DDefaultModeCheck(false);
        if (naMainWindow!=0) {
            naMainWindow->setV3DDefaultModeCheck(false);
        }
#endif
    }

#ifdef _ALLOW_WORKMODE_MENU_
    static void activateNaMainWindow() {
        if (naMainWindowIsActive==false) {
            if (naMainWindow==0) {
                naMainWindow = new NaMainWindow();
                // Allow NeuronAnnotator to activate file load in default window
                connect(naMainWindow, SIGNAL(defaultVaa3dFileLoadRequested(QString)),
                        mainWindow, SLOT(loadV3DFile(QString)));
                connect(naMainWindow, SIGNAL(defaultVaa3dUrlLoadRequested(QUrl)),
                        mainWindow, SLOT(loadV3DUrl(QUrl)));
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
#endif

    static V3dApplication* getInstance(int & argc, char ** argv) {
        if (theApp==0) {
            theApp = new V3dApplication(argc, argv);
        }
        return theApp;
    }
    static V3dApplication* getInstance() {
        int a=0;
        char ** p=0;
        if (theApp==0) {
            theApp = new V3dApplication(a, p);
        }
        return theApp;
    }

signals:

public slots:


};


#endif // V3D_APPLICATION_H
