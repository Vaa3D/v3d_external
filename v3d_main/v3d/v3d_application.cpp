#include "v3d_application.h"

MainWindow* V3dApplication::mainWindow;
NaMainWindow* V3dApplication::naMainWindow;
bool V3dApplication::mainWindowIsActive;
bool V3dApplication::naMainWindowIsActive;

V3dApplication::V3dApplication(int & argc, char ** argv) :
    QApplication(argc, argv)
{
    V3dApplication::mainWindow = new MainWindow();
    V3dApplication::mainWindowIsActive = false;
    V3dApplication::naMainWindowIsActive = false;
}

V3dApplication::~V3dApplication() {
    // Assuming Qt will delete mainWindow, neuronAnnotatorMainWindow
}



