/*
 *  swc_to_maskimage .cpp
 *  swc_to_maskimage 
 *
 *  Created by Yang, Jinzhu, on 11/27/10.
 *
 */

#include "swc_to_maskimage.h"
#include "v3d_message.h" 
#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(swc_to_maskimage, SWC_TO_MASKIMAGElugin);

void swc_to_maskimage(V3DPluginCallback &callback, QWidget *parent, int method_code);

//plugin funcs
const QString title = "swc_to_maskimage";
QStringList SWC_TO_MASKIMAGElugin::menulist() const
{
    return QStringList() 
	<< tr("swc_to_maskimage")
	<< tr("Help");
}

void SWC_TO_MASKIMAGElugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("swc_to_maskimage"))
	{
    	swc_to_maskimage(callback, parent,1 );
    }
	else if (menu_name == tr("Help"))
	{
		v3d_msg("Read SWC File to a mask image");
		return;
	}

}

void swc_to_maskimage(V3DPluginCallback &callback, QWidget *parent, int method_code)
{
	QString filename;
	filename = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
												"",
												QObject::tr("Supported file (*.swc)"
															";;Neuron structure	(*.swc)"
															));
	NeuronTree neurons;
	float x_min,x_max,y_min,y_max,z_min,z_max,x_total,y_total,z_total;
	x_min = 0;
	x_max = 0;
	y_min = 0;
	y_max = 0;
	z_min = 0;
	z_max = 0;
	x_total = 0;
	y_total = 0;
	z_total = 0;
	V3DLONG sx,sy,sz,alpha,beta;
	alpha = 1;
	beta =2;
	NeuronSWC *p_t=0;
	
	if (filename.size()>0)
	{
		neurons = readSWC_file(filename);
		
		//printf("xmin=%lf xmax=%lf ymin=%lf ymax=%lf zmin=%lf zmax=%lf\n", x_min, x_max, y_min, y_max, z_min, z_max);
		
		for (int ii=0; ii<neurons.listNeuron.size(); ii++)
		{
			p_t = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
			
			float xs = p_t->x;
			float ys = p_t->y;
			float zs = p_t->z;
			
			x_min = (xs<x_min)? xs:x_min;
			x_max = (xs>x_max)? xs:x_max;
			y_min = (ys<y_min)? ys:y_min;
			y_max = (ys>y_max)? ys:y_max;
			z_min = (ys<z_min)? ys:z_min;
			z_max = (zs>z_max)? ys:z_max;				 
		}
			
		printf("xmin=%lf xmax=%lf ymin=%lf ymax=%lf zmin=%lf zmax=%lf\n", x_min, x_max, y_min, y_max, z_min, z_max);		
	
		if(x_min < 0 || y_min < 0 || z_min <0)
		{	
			 sx = V3DLONG(x_max - x_min);
			 sy = V3DLONG(y_max - y_min);
			 sz = V3DLONG(z_max - z_min);			
		}else 
		{
			 sx = V3DLONG(x_max);
			 sy = V3DLONG(y_max);
			 sz = V3DLONG(z_max);
		}
	//	printf("sx=%d sy=%d sz=%d\n", sx, sy, sz);
		
		V3DLONG pagesz = sx*sy*sz;
/*********************************************************************/// coupute coordinate region 
		float scalar = 1;
		float scalar2 = scalar*scalar;		
		unsigned char* pImMask = new unsigned char[pagesz];	
	
		if (!pImMask) 
		{
			printf("Fail to allocate memory.\n");
			return ;
		}
		else
		{
			for(long i=0; i<pagesz; i++)
				pImMask[i] = 0; 
		}
		
		//compute mask
		NeuronSWC *p_tmp=0;
		float xs,ys,zs,xe,ye,ze;
		for (int ii=0; ii<neurons.listNeuron.size(); ii++)
		{
			p_tmp = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
			if(x_min < 0 || y_min < 0 || z_min <0)
			{	
				 xs = p_tmp->x + abs(x_min);
				 ys = p_tmp->y + abs(y_min);
				 zs = p_tmp->z + abs(z_min);				
			}else 
			{
				 xs = p_tmp->x;
				 ys = p_tmp->y;
				 zs = p_tmp->z;
			}
			float rs = p_tmp->r;
			
			//find previous node
			NeuronSWC *pp=0;
			for(int j=0; j<neurons.listNeuron.size(); j++)
			{
				pp = (NeuronSWC *)(&(neurons.listNeuron.at(j)));
				
				if(pp->n == p_tmp->pn)
					break;
			}
			//no previous node
			if(pp->n != p_tmp->pn)
				continue; 
			
			if(x_min < 0 || y_min < 0 || z_min <0)
			{	
				 xe = pp->x + abs(x_min);
				 ye = pp->y + abs(y_min);
				 ze = pp->z + abs(z_min);			
			}else 
			{
				 xe = pp->x;
				 ye = pp->y;
				 ze = pp->z;
			}
			//float re = pp->r;
		
			rs = alpha*rs+beta;
			
			//finding the envelope 
			
			float x_down = (xs>xe)? xe: xs;
			float x_top = (xs>xe)? xs: xe;
			float y_down = (ys>ye)? ye: ys;
			float y_top = (ys>ye)? ys: ye;
			float z_down = (zs>ze)? ze: zs;
			float z_top = (zs>ze)? zs: ze;
			printf("x_down1=%lf xs=%lf xe%lf y_down1=%lf z_down1=%lf x_top1=%lf y_top1=%lf z_top1=%lf rs=%lf\n", x_down, xs,xe, y_down, z_down, x_top, y_top, z_top,rs);
			if(x_down == xs)
			{
				if(x_down-rs > 0)
					x_down -= rs;
				else
					x_down = 0;
			}
			else
			{
				if(x_down-rs > 0)
					x_down -= rs; //re
				else
					x_down = 0;
			}
			
			if(y_down == ys)
			{
				if(y_down-rs > 0)
					y_down -= rs;
				else
					y_down = 0;
			}
			else
			{
				if(y_down-rs > 0)
					y_down -= rs; //re
				else
					y_down = 0;
			}
			
			if(z_down == zs)
			{
				if(z_down-rs > 0)
					z_down -= rs;
				else
					z_down = 0;
			}
			else
			{
				if(z_down-rs > 0)
					z_down -= rs; //re
				else
					z_down = 0;
			}
			
			if(x_top == xs)
			{
				if(x_top+rs < sx)
					x_top += rs;
				else
					x_top = sx;
			}
			else
			{
				if(x_top+rs < sx)
					x_top += rs; //re
				else
					x_top = sx;
			}
			
			if(y_top == ys)
			{
				if(y_top+rs < sy)
					y_top += rs;
				else
					y_top = sy;
			}
			else
			{
				if(y_top+rs < sy)
					y_top += rs; //re
				else
					y_top = sy;
			}
			
			if(z_top == zs)
			{
				if(z_top+rs < sz)
					z_top += rs;
				else
					z_top = sz;
			}
			else
			{
				if(z_top+rs < sz)
					z_top += rs; //re
				else
					z_top = sz;
			}
		printf("x_down=%lf y_down=%lf z_down=%lf x_top=%lf y_top=%lf z_top=%lf rs=%lf\n", x_down, y_down, z_down, x_top, y_top, z_top,rs);
			
/*********************************************************************/// coupute cylinder and flag mask 
			for(long k=long(z_down); k<long(z_top); k++)
			{
				for(long j=long(y_down); j<long(y_top); j++)
				{
					for(long i=long(x_down); i<long(x_top); i++)
					{
						long indLoop = k*sx*sy + j*sx + i;
						//norm(cross(x0-x1,x1-x2))/norm(x1-x2)
						double norms10 = (xs-i)*(xs-i) + (ys-j)*(ys-j) + (zs-k)*(zs-k)*scalar2;
						double norms21 = (xe-xs)*(xe-xs) + (ye-ys)*(ye-ys) + (ze-zs)*(ze-zs)*scalar2; 
						double dots1021 = (xs-i)*(xe-xs) + (ys-j)*(ye-ys) + (zs-k)*(ze-zs)*scalar2; 
						double dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
						
						double t = -dots1021/norms21;
						
						if(t<0)
							dist = sqrt(norms10);
						else if(t>1)
							dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k)*scalar2);
						
						printf("%lf %lf\n", dist, rs);
						
						if(dist <= rs)
						{    
							 pImMask[indLoop] =(p_tmp->type + 200);
							// v3d_msg("'11");
						}
						
					}
				}
			}
			
		}
		
		Image4DSimple tmp;
		
		tmp.setData(pImMask,sx,sy,sz,1,V3D_UINT8);		
		
		v3dhandle newwin = callback.newImageWindow();
		
		callback.setImage(newwin, &tmp);
		
		callback.updateImageWindow(newwin);
	}
	else 
	{
		return;
	}
		
}
