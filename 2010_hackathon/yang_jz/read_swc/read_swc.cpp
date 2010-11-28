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
	float x_down,x_top,y_down,y_top,z_down,z_top;
	x_top = 0;
	x_down = 0;
	y_top = 0;
	y_down = 0;
	z_top = 0;
	z_down = 0;
	if (filename.size()>0)
	{
		nt = readSWC_file(filename);

		NeuronSWC *p_tmp=0;
		for (int ii=0; ii<nt.listNeuron.size(); ii++)
		{
			p_tmp = (NeuronSWC *)(&(nt.listNeuron.at(ii)));
			
			float xs = p_tmp->x;
			float ys = p_tmp->y;
			float zs = p_tmp->z;
			float rs = p_tmp->r;
			x_down = (xs<x_down)? xs:x_down;
			x_top = (xs>x_top)? xs:x_top;
			y_down = (ys<y_down)? ys:y_down;
			y_top = (ys>y_top)? ys:y_top;
			z_down = (zs<z_down)? zs: z_down;
			z_top = (zs>z_top)? zs:z_top;			
		}
			
		printf("%lf %lf %lf %lf %lf %lf\n", x_down, y_down, z_down, x_top, y_top, z_top);		
		
		V3DLONG iImageW = abs(x_down - x_top);
		V3DLONG iImageH = abs(y_down - y_top);
		V3DLONG iimageL = abs(z_down - z_top);
		
		V3DLONG size = iImageH*iImageW*iimageL;

		unsigned char* newdata1d = new unsigned char[size]();
		
		for (int ii=0; ii<nt.listNeuron.size(); ii++)
		{
			p_tmp = (NeuronSWC *)(&(nt.listNeuron.at(ii)));
			int xs = p_tmp->x;
			int ys = p_tmp->y;
			int zs = p_tmp->z;
			xs = xs + abs(x_down);
			ys = ys + abs(y_down);
			zs = zs + abs(z_down);
			if(xs <0 || ys <0|| zs<0)
			{
				v3d_msg("error");
			}
			xs = (xs>iImageW)?iImageW:xs;
			ys = (ys>iImageH)?iImageH:ys;
			zs = (zs>iimageL)?iimageL:zs;
			
			if( xs >iImageW || ys > iImageH || zs>iimageL)
			{
				v3d_msg("error2");
			}
			newdata1d[zs * iImageW * iImageH + ys* iImageW + xs] = 255;
		}			
		
		Image4DSimple tmp;
		
		tmp.setData(newdata1d, iImageW,iImageH,iimageL,1,V3D_UINT8);		
		
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
