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
	<<tr("multiple SWC_to_maskimage")
	<<tr("Help");
}

void SWC_TO_MASKIMAGElugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("swc_to_maskimage"))
	{
    	swc_to_maskimage(callback, parent,1 );
    }else if (menu_name == tr("multiple SWC_to_maskimage"))
	{
		swc_to_maskimage(callback, parent,2);
		
	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("Read SWC File to a mask image");
		return;
	}

}
void ComputemaskImage(NeuronTree neurons,unsigned char* pImMask,unsigned char* ImMark,V3DLONG sx,V3DLONG sy,V3DLONG sz, V3DLONG scale, V3DLONG x_min,V3DLONG y_min, V3DLONG z_min, QString Filename)
{
	double scalar = 1;
	double scalar2 = scalar*scalar;	
	double alpha,beta;
	alpha = 1;
	beta =0;
	//compute mask
	NeuronSWC *p_tmp=0;
	double xs,ys,zs,xe,ye,ze;
	xs = ys = zs = xe = ye = ze = 0;
	for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
	{
		p_tmp = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));

		if(x_min < 0 || y_min < 0 || z_min <0)
		{	

			xs = (p_tmp->x - 1) - x_min;
			ys = (p_tmp->y - 1) - y_min;
			zs = (p_tmp->z - 1)- z_min;	
			
		}else 
		{
			xs = p_tmp->x ;
			ys = p_tmp->y ;
			zs = p_tmp->z ;
		}
		double rs = p_tmp->r;
		
		//find previous node
		NeuronSWC *pp=0;
		for(V3DLONG j=0; j<neurons.listNeuron.size(); j++)
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
			xe = (pp->x -1) -x_min;
			ye = (pp->y -1)- y_min;
			ze = (pp->z -1)- z_min;	
			
			
		}else 
		{				
			xe = pp->x;
			ye = pp->y;
			ze = pp->z;
		}
		//double re = pp->r;

		rs =alpha*rs+beta;		

		//finding the envelope 
		
		double x_down = (xs>xe)? xe: xs;
		double x_top = (xs>xe)? xs: xe;
		double y_down = (ys>ye)? ye: ys;
		double y_top = (ys>ye)? ys: ye;
		double z_down = (zs>ze)? ze: zs;
		double z_top = (zs>ze)? zs: ze;
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
		V3DLONG count1 = sx*sy;
		for(V3DLONG k=V3DLONG(z_down); k<V3DLONG(z_top); k++)
		{
			for(V3DLONG j=V3DLONG(y_down); j<V3DLONG(y_top); j++)
			{
				for(V3DLONG i=V3DLONG(x_down); i<V3DLONG(x_top); i++)
				{
					V3DLONG indLoop = (k)*count1 + (j)*sx + i;
					V3DLONG  countxsi = (xs-i);
					V3DLONG  countysj = (ys-j);
					V3DLONG  countzsk = (zs-k);
					V3DLONG countxes = (xe-xs);
					V3DLONG countyes = (ye-ys);
					V3DLONG countzes = (ze-zs);
					//norm(cross(x0-x1,x1-x2))/norm(x1-x2)
					double norms10 = countxsi * countxsi + countysj * countysj + countzsk * countzsk;
					double norms21 = countxes * countxes + countyes * countyes + countzes * countzes;
					double dots1021 = countxsi * countxes + countysj * countyes + countzsk * countzes; 
					double dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
					double t = -dots1021/norms21;
					if(t<0)
						dist = sqrt(norms10);
					else if(t>1)
						dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k));
					
					printf("%lf %lf\n", dist, rs);
					int n=rand()%256;
					if(dist <  rs)
					{    
						if(ImMark[indLoop] == 0)
						{
							//pImMask[indLoop] += (p_tmp->type + 1);
							pImMask[indLoop] += n;
							ImMark[indLoop] = 1;
						}else 
						{
							pImMask[indLoop] = n;
							//pImMask[indLoop] = (p_tmp->type + 1);
						}

					}
				}
			}
		}
		
	}
	
}
void swc_to_maskimage(V3DPluginCallback &callback, QWidget *parent, int method_code)
{
	NeuronTree neurons;
	double x_min,x_max,y_min,y_max,z_min,z_max,x_total,y_total,z_total;
	x_min = 0;
	x_max = 0;
	y_min = 0;
	y_max = 0;
	z_min = 0;
	z_max = 0;
	x_total = 0;
	y_total = 0;
	z_total = 0;
	V3DLONG sx,sy,sz,alpha,beta,namesize,temp;
	alpha = 1;
	beta =0;
	namesize = 0;
	unsigned char* pImMask;
	
	if (method_code == 1)
	{
		QString filename;
		filename = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
												"",
												QObject::tr("Supported file (*.swc)"
															";;Neuron structure	(*.swc)"
															));
	   if(filename.isEmpty()) 
	   {
		   v3d_msg("You don't have any image open in the main window.");
		   return;
	   }
		NeuronSWC *p_t=0;
		if (filename.size()>0)
		{
			neurons = readSWC_file(filename);
			
			//printf("xmin=%lf xmax=%lf ymin=%lf ymax=%lf zmin=%lf zmax=%lf\n", x_min, x_max, y_min, y_max, z_min, z_max);
			
			for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
			{
				p_t = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));

				double xs = p_t->x ;
				double ys = p_t->y ;
				double zs = p_t->z ;
				
				x_min = (xs<x_min)? xs:x_min;
				x_max = (xs>x_max)? xs:x_max;
				y_min = (ys<y_min)? ys:y_min;
				y_max = (ys>y_max)? ys:y_max;
				z_min = (ys<z_min)? ys:z_min;
				z_max = (zs>z_max)? ys:z_max;				 
			}
			temp = x_min;
			temp = (temp > y_min)? y_min:temp;
			temp = (temp > z_min)? z_min:temp;

         //   v3d_msg("12");
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
			V3DLONG pagesz = sx*sy*sz;
			pImMask = new unsigned char[pagesz];
			if (!pImMask) 
			{
				printf("Fail to allocate memory.\n");
				return ;
			}
			else
			{
				for(V3DLONG i=0; i<pagesz; i++)
					pImMask[i] = 0; 
			}
			unsigned char* ImMark = new unsigned char [pagesz];
			if (!ImMark) 
			{
				printf("Fail to allocate memory.\n");
				return;
			}
			else
			{
				for(V3DLONG i=0; i<pagesz; i++)
					ImMark[i] = 0; 
			}					
			ComputemaskImage(neurons,pImMask,ImMark,sx,sy,sz,temp,x_min,y_min,z_min,filename);
		}
		else 
		{
			v3d_msg("You don't have any image open in the main window.");
			return;
		}

	}
	else if (method_code ==2)
	{
		QString filename;
		QStringList filenames;
		filenames = QFileDialog::getOpenFileNames(0, 0,"","Supported file (*.swc)" ";;Neuron structure(*.swc)",0,0);
		if(filenames.isEmpty()) 
		{
			v3d_msg("You don't have any image open in the main window.");
			return;
		}
	//	v3d_msg("1");
		namesize = filenames.size();
		NeuronSWC *p_t=0;
		for (V3DLONG i = 0; i < filenames.size();i++)
		{
			filename = filenames[i];
			if (filename.size()>0)
			{
				neurons = readSWC_file(filename);
				
				//printf("xmin=%lf xmax=%lf ymin=%lf ymax=%lf zmin=%lf zmax=%lf\n", x_min, x_max, y_min, y_max, z_min, z_max);
				for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
				{
					p_t = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));

					double xs = p_t->x - 1;
					double ys = p_t->y - 1;
					double zs = p_t->z - 1;
					x_min = (xs<x_min)? xs:x_min;
					x_max = (xs>x_max)? xs:x_max;
					y_min = (ys<y_min)? ys:y_min;
					y_max = (ys>y_max)? ys:y_max;
					z_min = (ys<z_min)? ys:z_min;
					z_max = (zs>z_max)? ys:z_max;				 
				}
				printf("xmin=%lf xmax=%lf ymin=%lf ymax=%lf zmin=%lf zmax=%lf\n", x_min, x_max, y_min, y_max, z_min, z_max);
			}else 
			{
				v3d_msg("You don't have any image open in the main window.");
				return;
			}
		}
		temp = x_min;
		temp = (temp > y_min)? y_min:temp;
		temp = (temp > z_min)? z_min:temp;
		//v3d_msg("2");
		if(x_min < 0 || y_min < 0 || z_min <0)
		{
			sx = V3DLONG(x_max - temp);
			sy = V3DLONG(y_max - temp);
			sz = V3DLONG(z_max - temp);
		}
		else
		{
			sx = V3DLONG(x_max);
			sy = V3DLONG(y_max);
			sz = V3DLONG(z_max);
		}
		//prV3DLONGf("sx=%d sy=%d sz=%d\n", sx, sy, sz);
		V3DLONG pagesz = sx*sy*sz;
		pImMask = new unsigned char [pagesz];
		if (!pImMask) 
		{
			printf("Fail to allocate memory.\n");
			return;
		}
		else
		{
			for(V3DLONG i=0; i<pagesz; i++)
				pImMask[i] = 0; 
		}		
		for (V3DLONG i = 0; i < filenames.size();i++)
		{
			filename = filenames[i];
			if (filename.size()>0)
			{
				neurons = readSWC_file(filename);
				unsigned char* ImMark = new unsigned char [pagesz];
				if (!ImMark) 
				{
					printf("Fail to allocate memory.\n");
					return;
				}
				else
				{
					for(V3DLONG i=0; i<pagesz; i++)
						ImMark[i] = 0; 
				}		
				
				ComputemaskImage(neurons,pImMask,ImMark,sx,sy,sz,temp,x_min,y_min,z_min,filename);
			}
			else 
			{
				return;
			}
		}
	
	}
	/*********************************************************************/// coupute coordinate region 		
	Image4DSimple tmp;
	tmp.setData(pImMask,sx,sy,sz,1,V3D_UINT8);		
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &tmp);
	callback.setImageName(newwin, QString("composition image"));
	//v3d_msg("dfss");
	callback.updateImageWindow(newwin);
		
}
