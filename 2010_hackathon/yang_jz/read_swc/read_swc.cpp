/*
 *  read_swc .cpp
 *  read_swc 
 *
 *  Created by Yang, Jinzhu, on 11/27/10.
 *
 */

#include "read_swc.h"
#include "v3d_message.h" 
#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(read_swc, READ_SWClugin);

void Read_SWC(V3DPluginCallback &callback, QWidget *parent, int method_code);

//plugin funcs
const QString title = "read_swc";
QStringList READ_SWClugin::menulist() const
{
    return QStringList() 
	<< tr("read_swc")
	<< tr("Help");
}

void READ_SWClugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("read_swc"))
	{
    	Read_SWC(callback, parent,1 );
    }
	else if (menu_name == tr("Help"))
	{
		v3d_msg("Read SWC File");
		return;
	}

}

void Read_SWC(V3DPluginCallback &callback, QWidget *parent, int method_code)
{
	QString filename;
	filename = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
												"",
												QObject::tr("Supported file (*.swc)"
															";;Neuron structure	(*.swc)"
															));
	NeuronTree nt;
	if (filename.size()>0)
	{
		nt = readSWC_file(filename);
		
		V3DLONG N = 1000*1000;
		unsigned char* newdata1d = new unsigned char[N]();
		
		Image4DSimple tmp;
		
		tmp.setData(newdata1d, 1000,1000,1,1,V3D_UINT8);		
		
		v3dhandle newwin = callback.newImageWindow();
		
		callback.setImage(newwin, &tmp);
		
		callback.updateImageWindow(newwin);
		
		callback.open3DWindow(newwin);
	
      /////////////////////////////////
		
		v3dhandle curwin = callback.currentImageWindow();
	
		callback.setSWC(curwin,nt);		
			
		callback.pushObjectIn3DWindow(curwin);
		
		callback.updateImageWindow(curwin);	
	}
		else 
	{
		return;
	}
		
}
