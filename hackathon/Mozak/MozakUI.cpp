#include "MozakUI.h"
#include "Mozak3DView.h"
#include "v3d_application.h"
#include "../terafly/src/control/CViewer.h"

using namespace mozak;

void MozakUI::init(V3d_PluginLoader *pl)
{
	createInstance(pl, 0);
}

void MozakUI::createInstance(V3DPluginCallback2 *callback, QWidget *parent)
{
    if (uniqueInstance == 0)
        uniqueInstance = new MozakUI(callback, parent);
    uniqueInstance->reset();
	string path = ""; // this will prompt for user to find path
	uniqueInstance->openVolume(path);
	//uniqueInstance->hide();
	V3dApplication::deactivateMainWindow();
}

MozakUI* MozakUI::getMozakInstance()
{
	return static_cast<MozakUI*>(uniqueInstance);
}

MozakUI::MozakUI(V3DPluginCallback2 *callback, QWidget *parent) : teramanager::PMain(callback, parent)
{
    // This inherits from the PMain constructor ( teramanager::PMain(callback, parent) )
	// so that constructor will be called before the following code:
    
	// Adjust Terafly UI
	//setWindowTitle(QString("Neuron Game UI"));
}

void MozakUI::reset()
{

}

teramanager::CViewer * MozakUI::initViewer(V3DPluginCallback2* _V3D_env, int _resIndex, itm::uint8* _imgData, int _volV0, int _volV1,
	int _volH0, int _volH1, int _volD0, int _volD1, int _volT0, int _volT1, int _nchannels, itm::CViewer* _prev)
{
	teramanager::CViewer* new_win = new Mozak3DView(_V3D_env, _resIndex, _imgData, _volV0, _volV1, _volH0, _volH1, _volD0, _volD1, _volT0, _volT1, _nchannels, _prev, -1);
	return new_win;
}