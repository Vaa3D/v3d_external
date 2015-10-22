#pragma once
#include "../terafly/src/presentation/PMain.h"

namespace neurongame
{

	class NeuronGameUI : public teramanager::PMain
	{
		protected:
			NeuronGameUI(){}
			~NeuronGameUI(){}
			NeuronGameUI(V3DPluginCallback2 *callback, QWidget *parent);
			static void createInstance(V3DPluginCallback2 *callback, QWidget *parent);
		public:
			static void init(V3d_PluginLoader *pl);
			virtual void reset();
	};

}