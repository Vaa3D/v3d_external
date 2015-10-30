#ifndef __NEURON_GAME_UI_H__
#define __NEURON_GAME_UI_H__

#include "../terafly/src/presentation/PMain.h"
#include "../terafly/src/control/CViewer.h"
#include "../terafly/src/control/CPlugin.h"
#include "NeuronGame3DView.h"



class neurongame::NeuronGameUI : public teramanager::PMain
{
	protected:
		NeuronGameUI(){}
		~NeuronGameUI(){}
		NeuronGameUI(V3DPluginCallback2 *callback, QWidget *parent);
		static void createInstance(V3DPluginCallback2 *callback, QWidget *parent);
	public:
		friend class NeuronGame3DView;
		static void init(V3d_PluginLoader *pl);
		virtual void reset(); // override
		virtual teramanager::CViewer* neurongame::NeuronGameUI::initViewer(V3DPluginCallback2* _V3D_env, int _resIndex, itm::uint8* _imgData, int _volV0, int _volV1,
			int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer* _prev);
};

#endif