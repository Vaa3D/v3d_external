#include "NeuronGameUI.h"

void neurongame::NeuronGameUI::init(V3d_PluginLoader *pl)
{
	NeuronGameUI::createInstance(pl, 0);
}

void neurongame::NeuronGameUI::createInstance(V3DPluginCallback2 *callback, QWidget *parent)
{
    if (uniqueInstance == 0)
        uniqueInstance = new NeuronGameUI(callback, parent);
    uniqueInstance->reset();
}

neurongame::NeuronGameUI::NeuronGameUI(V3DPluginCallback2 *callback, QWidget *parent) : teramanager::PMain(callback, parent)
{
    // This inherits from the PMain constructor ( teramanager::PMain(callback, parent) )
	// so that constructor will be called before the following code:
    
	// Adjust Terafly UI
	setWindowTitle(QString("Neuron Game"));
}

void neurongame::NeuronGameUI::reset()
{
	// Hide menus
	menuBar->hide();
	// Hide tabs
	controls_page->hide();
	info_page->hide();
}