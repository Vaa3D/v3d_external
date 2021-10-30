

#include "MozakUI.h"
#include "Mozak3DView.h"
#include "v3d_application.h"
// 20170624 RZC: central control include files in Mozak3DView.h
//#include "../terafly/src/control/CViewer.h"

using namespace mozak;

void MozakUI::init(V3d_PluginLoader *pl)
{
	createInstance(pl, 0);
}

MozakUI::~MozakUI()
{
    V3dApplication::activateMainWindow();
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


//20170803 RZC
void MozakUI::onImageTraceHistoryChanged()
{
	Mozak3DView* mozak_view = (Mozak3DView*)( teramanager::CViewer::getCurrent() );
	if (! mozak_view) return;

	mozak_view->onNeuronEdit();
}

