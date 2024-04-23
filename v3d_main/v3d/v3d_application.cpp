#include "v3d_application.h"

MainWindow* V3dApplication::mainWindow=0; //reset to be 0. by Hanchuan Peng, 20110705
bool V3dApplication::mainWindowIsActive;

#ifdef _ALLOW_WORKMODE_MENU_
NaMainWindow* V3dApplication::naMainWindow = 0; //reset to be 0. by Hanchuan Peng, 20110705
bool V3dApplication::naMainWindowIsActive;
#endif

V3dApplication::V3dApplication(int & argc, char ** argv) 
    : QApplication(argc, argv)
{
#if defined(USE_Qt5)
	QSurfaceFormat format;
  format.setDepthBufferSize(24);
  QSurfaceFormat::setDefaultFormat(format);
#endif
  
	if (!V3dApplication::mainWindow) //force checking. by Hanchuan Peng, 20110705
		V3dApplication::mainWindow = new MainWindow();

    V3dApplication::mainWindowIsActive = false;
    
#ifdef _ALLOW_WORKMODE_MENU_
    V3dApplication::naMainWindowIsActive = false;
#endif
}


V3dApplication::~V3dApplication() {
    // Assuming Qt will delete mainWindow, neuronAnnotatorMainWindow
}



