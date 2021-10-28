#ifndef __MOZAK_UI_H__
#define __MOZAK_UI_H__

// 20170624 RZC: central control include files in Mozak3DView.h
//#include "../terafly/src/presentation/PMain.h"
//#include "../terafly/src/control/CViewer.h"
//#include "../terafly/src/control/CPlugin.h"
#include "Mozak3DView.h"

class mozak::MozakUI : public teramanager::PMain
{
	public:
		MozakUI(){}
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
};

#endif
