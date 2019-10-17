#ifndef __MOZAK_UI_H__
#define __MOZAK_UI_H__

// 20170624 RZC: central control include files in Mozak3DView.h
//#include "../terafly/src/presentation/PMain.h"
//#include "../terafly/src/control/CViewer.h"
//#include "../terafly/src/control/CPlugin.h"
#include "Mozak3DView.h"
#include "GameControllerAPI/aiGameControllerRaw.h"
#include "3DXLib\aiSpaceNavigatorDevice.h"
#include "dsl/dslIniFile.h"
#include "dsl/dslProperty.h"
#include "dsl/dslIniFileProperties.h"
#include "dsl/dslLogLevel.h"
#include "Poco/Process.h"

using ai::SpaceNavigatorDevice;
using dsl::IniFile;
using dsl::Property;
using dsl::LogLevel;


class mozak::MozakUI : public teramanager::PMain
{
	public:
        MozakUI();
		~MozakUI();
		MozakUI(V3DPluginCallback2 *callback, QWidget *parent);
		static void createInstance(V3DPluginCallback2 *callback, QWidget *parent);
		static MozakUI* getMozakInstance();
	//public:
		friend class Mozak3DView;
		static void init(V3d_PluginLoader *pl);
		virtual void reset(); // override
        virtual teramanager::CViewer* initViewer(V3DPluginCallback2* _V3D_env, int _resIndex, itm::uint8* _imgData, int _volV0, int _volV1,
			int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer* _prev);

	public:
		static void onImageTraceHistoryChanged(); //20170803 RZC

        //!Game controller and Spacenavigator integration code by T. Karlsson
        
        //!The inifile contain parameters for changing some parameters related to the gamecontroller and the SpaceMouse        
        string                                  mAppDataFolder;
        dsl::IniFile                            mIniFile;
        dsl::IniFileProperties                  mGeneralProperties;
        dsl::Property<LogLevel>     		    mLogLevel;
        dsl::Property<double>     	     	    mGameControllerZoomFactor;
        dsl::Property<double>     	     	    mZoomSpeed;
        
        unique_ptr<ai::GameControllerRaw>		mGC;
        void									onPOV(ai::GameControllerPOV* p);		
        void									onAxis(ai::JoyStickAxis* axis);	
        void									onButtonDown(ai::GameControllerButton* btn);
        void									onButtonUp(ai::GameControllerButton* btn);
        void									zoom(bool zoomIn);       
        
        //!Spacenavigator integration code
        void									onSpaceMouseAxis(ai::SpaceNavigatorAxis* axis);
        void                                    openConfigEditor();
        void                                    loadIniFile();
        int                                     mConfigEditorProcessID;

        //!When things don't work
        void                                    toggleSpaceMouseOnOff();
        void                                    toggleGameControllerOnOff();
    protected:
        virtual bool							winEvent(MSG * message, long * result);
        HWND									mWindowsHandle;
        Mozak3DView*							mMozak3DView;				  
        ai::GameControllerPOVState				mLastPOV;
        SpaceNavigatorDevice                    mSpaceNavigator;
};

#endif


#pragma comment(lib, "aiGameControllerAPI-vs2013")
#pragma comment(lib, "ai3DXLib-vs2013")
#pragma comment(lib, "dslFoundation-vs2013")
#pragma comment(lib, "poco_foundation-vs2013")
