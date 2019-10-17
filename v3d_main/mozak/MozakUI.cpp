#include "MozakUI.h"
#include "Mozak3DView.h"
#include "v3d_application.h"
// 20170624 RZC: central control include files in Mozak3DView.h
//#include "../terafly/src/control/CViewer.h"
#include <Shlobj.h>
#include <boost/bind.hpp>
#include  "math.h"
#include "Poco/Stopwatch.h"
#include "dsl/dslLogging.h"
#include "dsl/dslLogger.h"
#include "dsl/dslMathUtils.h"
#include "dsl/dslFileUtils.h"
#include "dsl/dslStringUtils.h"
#include "dsl/dslWin32Utils.h"

using namespace Poco;
using namespace mozak;
using namespace ai;
using namespace dsl;

MozakUI::MozakUI()
    :
    mLogLevel(lInfo),    
    mGameControllerZoomFactor(1),
    mZoomSpeed(10),
    mConfigEditorProcessID(NULL)
{}

void MozakUI::init(V3d_PluginLoader *pl)
{
	createInstance(pl, 0);
}

MozakUI::~MozakUI()
{
    mLogLevel = dsl::gLogger.getLogLevel();
    mGeneralProperties.write();
    mIniFile.save();

    //if config window is open, close it
    if (mConfigEditorProcessID != 0)
    {
    //    Poco::Process::kill(mConfigEditorProcessID);
    }

    V3dApplication::activateMainWindow();
}

MozakUI::MozakUI(V3DPluginCallback2 *callback, QWidget *parent)
	:
	teramanager::PMain(callback, parent),	
	mLastPOV(ai::povNotEngaged),
	mGC(unique_ptr<ai::GameControllerRaw>(new GameControllerRaw())),
    mLogLevel(lInfo),
    mGameControllerZoomFactor(1),
    mZoomSpeed(1),
    mAppDataFolder(joinPath(getKnownFolder(FOLDERID_LocalAppData), "Vaa3D-Mozak"))
{   
    gLogger.setLogLevel(dsl::lDebug5);
	mMozak3DView = NULL;

    if (!dsl::folderExists(mAppDataFolder) && !dsl::createFolder(mAppDataFolder))
    {
            Log(dsl::lError) << "Failed to create folder: " << mAppDataFolder;        
    }  

    mGeneralProperties.setSectionName("GENERAL");
    mGeneralProperties.setIniFile(&mIniFile);
    mGeneralProperties.add((dsl::BaseProperty*)&mLogLevel.setup("LOG_LEVEL", dsl::lAny));
    mGeneralProperties.add((dsl::BaseProperty*)&mGameControllerZoomFactor.setup("GAME_CONTROLLER_ZOOM_FACTOR", 10.0));
    mGeneralProperties.add((dsl::BaseProperty*)&mZoomSpeed.setup("ZOOM_SPEED", 10.0));
    
    dsl::gLogger.logToFile(dsl::joinPath(mAppDataFolder, "va3d-mozak.log"));

	if (mGC)
	{
        mWindowsHandle = this->winId();
		mGC->capture(mWindowsHandle);		
		mGC->mPOV.assignEvent(bind(&MozakUI::onPOV, this, _1));
		mGC->mJoyStick1.mXAxis.assignEvent(bind(&MozakUI::onAxis, this, _1 ));
		mGC->mJoyStick1.mYAxis.assignEvent(bind(&MozakUI::onAxis, this, _1 ));

		mGC->mJoyStick2.mXAxis.assignEvent(bind(&MozakUI::onAxis, this, _1));
		mGC->mJoyStick2.mYAxis.assignEvent(bind(&MozakUI::onAxis, this, _1));

		mGC->mFrontLeftAxis.assignEvent(bind(&MozakUI::onAxis, this, _1));
		mGC->mFrontRightAxis.assignEvent(bind(&MozakUI::onAxis, this, _1));
		
        mGC->mButton5.assignButtonEvents(bind(&MozakUI::onButtonDown, this, _1), NULL);
		mGC->mButton6.assignButtonEvents(bind(&MozakUI::onButtonDown, this, _1), NULL);
		mGC->mButton7.assignButtonEvents(bind(&MozakUI::onButtonDown, this, _1), NULL);
		mGC->mButton8.assignButtonEvents(bind(&MozakUI::onButtonDown, this, _1), NULL);
		mGC->mButton9.assignButtonEvents(bind(&MozakUI::onButtonDown, this, _1), NULL);        
	}

    //The space navigator
    mSpaceNavigator.init(mWindowsHandle);
    mSpaceNavigator.mTx.assignEvent(bind(&MozakUI::onSpaceMouseAxis, this, _1));
    mSpaceNavigator.mTy.assignEvent(bind(&MozakUI::onSpaceMouseAxis, this, _1));
    mSpaceNavigator.mTz.assignEvent(bind(&MozakUI::onSpaceMouseAxis, this, _1));
    mSpaceNavigator.mRx.assignEvent(bind(&MozakUI::onSpaceMouseAxis, this, _1));
    mSpaceNavigator.mRy.assignEvent(bind(&MozakUI::onSpaceMouseAxis, this, _1));
    mSpaceNavigator.mRz.assignEvent(bind(&MozakUI::onSpaceMouseAxis, this, _1));
       
	// This inherits from the PMain constructor ( teramanager::PMain(callback, parent) )
	// so that constructor will be called before the following code:

	// Adjust Terafly UI
    //Read version file
    string vrsFileName("MOZAK_VERSION.txt");
    stringstream caption;
    caption << "Mozak UI";
    try
    {        
        string version(dsl::getFileContent(vrsFileName));        
        
        if (version.size())
        {
            caption << " - version: " << version;
        }
        else
        {
            caption << "<undefined version>";
        }
    }
    catch (...)
    {
        Log(dsl::lError) << "Failed to read version file: " << vrsFileName;
    }

    setWindowTitle(QString::fromStdString(caption.str()));    

    //Read inifile    
    loadIniFile();
}

void MozakUI::openConfigEditor()
{
    //Launch external config program
    Log(lInfo) << "Opening ini file: " << mIniFile.getFullFileName();
    std::vector<std::string> args;
    args.push_back(mIniFile.getFullFileName());

    try
    {
        Poco::ProcessHandle h = Poco::Process::launch("ConfigFileUI.exe", args);

        if (h.id() == 0)
        {
            Log(lError) << "Failed opening the inifile";
        }

        mConfigEditorProcessID = h.id();
    }
    catch (...)
    {
        Log(lError) << "Problem..";
    }
}

void MozakUI::toggleSpaceMouseOnOff()
{
    mSpaceNavigator.enable(!mSpaceNavigator.isEnabled());
    Log(lInfo) << "The spacemouse is: " << (mSpaceNavigator.isEnabled() ? "enabled" : "disabled");
}

void MozakUI::toggleGameControllerOnOff()
{
    if (mGC)
    {
        mGC->isEnabled() ? mGC->disable() : mGC->enable();
        Log(lInfo) << "The gamecontroller is: " << (mGC->isEnabled() ? "enabled" : "disabled");
    }
}

void MozakUI::loadIniFile()
{
    mIniFile.load(joinPath(mAppDataFolder, "vaa3d-mozak.ini"));
    mGeneralProperties.read();
    mSpaceNavigator.readProperties(mIniFile);
    dsl::gLogger.setLogLevel(mLogLevel);

}

void MozakUI::createInstance(V3DPluginCallback2 *callback, QWidget *parent)
{
    if (uniqueInstance == 0)
        uniqueInstance = new MozakUI(callback, parent);
    uniqueInstance->reset();
    uniqueInstance->show();

#ifdef MOZAK_AUTOLOAD_VOLUME_PATH
	string path = MOZAK_AUTOLOAD_VOLUME_PATH;
    QFileInfo pathinfo(path.c_str());
    if(path.c_str().isEmpty() || !QFile::exists(path.c_str()))
	    uniqueInstance->openTeraFlyVolume(""); // this will prompt for user to find path
    else if(pathinfo.isDir())
        uniqueInstance->openTeraFlyVolume(path);
    else if(pathinfo.isFile())
        uniqueInstance->openHDF5Volume(path);
    }
#else
	uniqueInstance->openTeraFlyVolume(""); // this will prompt for user to find path
#endif
#ifdef MOZAK_HIDE_VAA3D_CONTROLS
	uniqueInstance->hide();
	//V3dApplication::deactivateMainWindow();
#endif	
}

bool MozakUI::winEvent(MSG * message, long * result)
{ 
	if (message->message == WM_INPUT)
	{
		if (mGC && mGC->isEnabled())
		{
            //This may trigger callbacks
			mGC->processRawInput(message->lParam);
		}
	}
    else
    { 
        mSpaceNavigator.handleEvent(*message);
    }  

	return false;
}

void MozakUI::onSpaceMouseAxis(ai::SpaceNavigatorAxis* axis)
{
    if (!axis)
    {
        return;
    }

    //The reported axis position
    double position = axis->getScaledPosition();

    //Log(lInfo) << "Space Navigator axes: " << axis->getLabel() << position;
    if (axis == &mSpaceNavigator.mTx)
    {
        double xShift = mMozak3DView->getGLWidget()->_xShift;
        mMozak3DView->getGLWidget()->setXShift((float) (xShift + position));
    }
    else if (axis == &mSpaceNavigator.mTy)
    {
        double yShift = mMozak3DView->getGLWidget()->_yShift;
        mMozak3DView->getGLWidget()->setYShift((float)(yShift + position));
    }
    else if (axis == &mSpaceNavigator.mTz)
    {                
        float pos = mMozak3DView->window3D->zoomSlider->sliderPosition();
        mMozak3DView->getGLWidget()->setZoom(pos + (float)(position));
        Log(lDebug3) << "Scaled zoom position: " << position;
    }

    //Rotations
    else if (axis == &mSpaceNavigator.mRx || axis == &mSpaceNavigator.mRy || axis == &mSpaceNavigator.mRz) 
    {        
        double rX = mSpaceNavigator.mRx.getScaledPosition();   
        double rY = mSpaceNavigator.mRy.getScaledPosition();               
        double rZ = mSpaceNavigator.mRz.getScaledPosition();
        mMozak3DView->view3DWidget->viewRotation(rX, rY, rZ);
    }
}

void MozakUI::onAxis(JoyStickAxis* axis)
{
    if (!mMozak3DView)
    {
        return;
    }

	if (axis == &mGC->mFrontLeftAxis || axis == &mGC->mFrontRightAxis)
	{       
        static float delta = 0;

        double pos = (mGC->mFrontLeftAxis.getPosition() / 65408.0) - 0.5; 

        static Stopwatch watch;        
        double elapsed = watch.elapsed() / 1000;
        
        if (elapsed < mZoomSpeed && watch.elapsed() != 0)
        {
            return;
        }      
        
        
        int sliderPosition = mMozak3DView->window3D->zoomSlider->sliderPosition();

        delta = (pos > 0) ? 1 : -1;

        
        
        float newPosition = sliderPosition + round(delta) ;
        //Log(lDebug3) << "Scaled zoom position: " << pos <<"\t Slider position: " << sliderPosition;
       
        mMozak3DView->getGLWidget()->setZoom(newPosition);

        //Control the speed.. could be more finegrained. faster with small steps?
        watch.restart();
	}

	if (axis == &mGC->mJoyStick2.mXAxis || axis == &mGC->mJoyStick2.mYAxis && mLastPOV)
	{
		int x = (mGC->mJoyStick2.mXAxis.getPosition() - 32768) ;
		int y = (mGC->mJoyStick2.mYAxis.getPosition() - 32768) * -1;

		double magnitude = sqrt(pow(x, 2) + pow(y, 2));

        //Skip if less than 85% of movement
        if ((double)(magnitude / 32768) < 0.85)
        {
            return;
        }

		double theta = dsl::toDegree( std::atan2(y, x)); 
		theta = fmod(theta + 270, 360);
		Log(lInfo) << "(x,y) -> (" << x << "," << y << ")\t" << "Theta: " << theta << "\tMagn" << magnitude << "POV: " << mGC->mPOV.getState();
						
        //This functionality is pretty useless
		if (mLastPOV == povWest )
		{				
			mMozak3DView->window3D->xRotBox->setValue(theta);											
		}
		else if (mLastPOV == povNorth)
		{
			mMozak3DView->window3D->yRotBox->setValue(theta);
		}

		else if (mLastPOV == povEast)
		{
			mMozak3DView->window3D->zRotBox->setValue(theta);
		}
		else
		{                
			QPoint p = mMozak3DView->view3DWidget->mapFromGlobal(QCursor::pos());
			QMouseEvent eve(QEvent::MouseButtonDblClick, p, Qt::RightButton, Qt::NoButton, Qt::NoModifier);
			mMozak3DView->eventFilter(mMozak3DView->view3DWidget, &eve);
		}				
	}
}

void MozakUI::onPOV(ai::GameControllerPOV* p)
{	
	Log(dsl::lDebug) << "POV state: " << p->getStateAsString() << "\t" << p->getState() << "\t" << p->getHWState();
	if (p->getState() == povNotEngaged)
	{
		Log(dsl::lDebug) << "POV is not engaged anymore";
	}
	else
	{
		mLastPOV = p->getState();
	}	
}

void MozakUI::onButtonDown(ai::GameControllerButton* btn)
{
    if (!btn)
    {
        return;
    }
    else
    {
        Log(dsl::lDebug) << "Button: " << btn->getLabel() << " is down";
    }

	//Zoom in/out the scale (center on current cursor position)
	if (btn == &mGC->mButton5 || btn == &mGC->mButton6)
	{
		QPoint p = mMozak3DView->view3DWidget->mapFromGlobal(QCursor::pos());
		
        //send a Mouse double click on left or right button --> zoom Mozak resolution		
		QMouseEvent eve(QEvent::MouseButtonDblClick, p, (btn == &mGC->mButton6 ? Qt::LeftButton : Qt::RightButton), Qt::NoButton, Qt::NoModifier);
		mMozak3DView->eventFilter(mMozak3DView->view3DWidget, &eve);		
	}	
}

void MozakUI::zoom(bool zoomIn)
{	
    if (!mMozak3DView)
    {
        return;
    }

    float zoomStep(1.0);
	float zoom = (zoomIn ? -1  : 1 ) * zoomStep;

	Log(dsl::lInfo) << "Zoom by: " << zoom;
    float prevZoom = mMozak3DView->view3DWidget->zoom();
    float newZoom = zoom + prevZoom; 

	// Change zoom
	mMozak3DView->view3DWidget->setZoom(newZoom);

    Renderer_gl2* curr_renderer = (Renderer_gl2*)(mMozak3DView->view3DWidget->getRenderer());
	curr_renderer->paint(); // updates the projection matrix			
	QApplication::processEvents();	
}

MozakUI* MozakUI::getMozakInstance()
{
	return static_cast<MozakUI*>(uniqueInstance);
}

void MozakUI::reset()
{}

teramanager::CViewer * MozakUI::initViewer(V3DPluginCallback2* _V3D_env, int _resIndex, itm::uint8* _imgData, int _volV0, int _volV1,
	int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer* _prev)
{
	mMozak3DView = new Mozak3DView(_V3D_env, _resIndex, _imgData, _volV0, _volV1, _volH0, _volH1, _volD0, _volD1, _volT0, _volT1, _nchannels, _prev, -1);

	teramanager::CViewer* new_win = mMozak3DView;			
	return new_win;
}

//20170803 RZC
void MozakUI::onImageTraceHistoryChanged()
{
	Mozak3DView* mozak_view = (Mozak3DView*)( teramanager::CViewer::getCurrent() );
	if (! mozak_view) return;

	mozak_view->onNeuronEdit();
}

