#include "v3d_application.h"

MainWindow* V3dApplication::mainWindow=0; //reset to be 0. by Hanchuan Peng, 20110705
NaMainWindow* V3dApplication::naMainWindow=0; //reset to be 0. by Hanchuan Peng, 20110705
bool V3dApplication::mainWindowIsActive;
bool V3dApplication::naMainWindowIsActive;

V3dApplication::V3dApplication(int & argc, char ** argv) :
    QApplication(argc, argv)
{
	if (!V3dApplication::mainWindow) //force checking. by Hanchuan Peng, 20110705
		V3dApplication::mainWindow = new MainWindow();

    V3dApplication::mainWindowIsActive = false;
    V3dApplication::naMainWindowIsActive = false;
}

V3dApplication::~V3dApplication() {
    // Assuming Qt will delete mainWindow, neuronAnnotatorMainWindow
}



