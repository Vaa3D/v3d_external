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
	beta =1;
	//compute mask
	NeuronSWC *p_tmp=0;
	double xs,ys,zs,xe,ye,ze;
	double ballxl,ballxr,ballyf,ballyb,ballzt,ballzd;
	xs = ys = zs = xe = ye = ze = 0;
	V3DLONG aa = 0;
	V3DLONG count1 = sx*sy;
	printf("xmin=%d ymin=%d zmin=%d \n",x_min, y_min,z_min);	
	
	for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
	{
		p_tmp = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));

		if(x_min <= 0 )
		{	
			xs = abs((p_tmp->x ) - x_min);
		}
		if(y_min <= 0 )
		{	
			ys = abs((p_tmp->y ) - y_min);		
		}
		if(z_min <= 0 )
		{	
			zs = abs((p_tmp->z )-  z_min);	
		}
		double rs = p_tmp->r;
		ballxl = xs - rs;
		ballxr = xs + rs;
		ballyf = ys + rs;
		ballyb = ys - rs;
		ballzt = zs + rs;
		ballzd = zs - rs;
		ballxl = (ballxl<0)?0:ballxl;
		ballxr = (ballxr>sx)? sx:ballxr;
		ballyf = (ballyf>sy)?sy:ballyf;
		ballyb = (ballyb <0)?0:ballyb;
		ballzd = (ballzd <0)?0:ballzd;
		ballzt = (ballzt >sz)?sz:ballzt;
	
		V3DLONG indLoop = (zs)*count1 + (ys)*sx + xs;

	    for(V3DLONG k = ballzd; k < ballzt; k++ )
		{
			for(V3DLONG j = ballyb; j < ballyf; j++)
			{
				for(V3DLONG i = ballxl; i < ballxr; i++)
				{
					double norms10 = (xs-i)*(xs-i) + (ys-j)*(ys-j) + (zs-k)*(zs-k);
					double dt = sqrt(norms10);
					V3DLONG indLoop = (k)*count1 + (j)*sx + i;
					if(dt < rs)
					{  
		//				printf("dt=%lf xs=%lf ys=%lf zs=%lf rs=%lf\n",dt,xs,ys,zs,rs);
						int n=rand()%256+1;
						if(ImMark[indLoop] == 0)
						{
							//pImMask[indLoop] += (p_tmp->type + 1);
							pImMask[indLoop] += n;
							ImMark[indLoop] = 1;
						}
						else
						{
							pImMask[indLoop] = n;
							//pImMask[indLoop] = (p_tmp->type + 1);
						}
					}
				}
			}
		}
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
	
		if(x_min <= 0 )
		{	
			xe = abs((pp->x ) - x_min);
		}
		if(y_min <= 0 )
		{	
			ye = abs((pp->y ) - y_min);
			
		}
		if(z_min <= 0 )
		{	
			ze = abs((pp->z )-  z_min);
		}
		double re = pp->r;
		if (rs < 1) 
		{
			rs =alpha*rs+beta;
		}
		
//		if (re < 1) 
//		{
//			re =alpha*re+beta;
//		}
			//finding the envelope 
	//	double re = pp->r;
		double x_down = (xs>xe)? xe: xs;
		double x_top = (xs>xe)? xs: xe;
		double y_down = (ys>ye)? ye: ys;
		double y_top = (ys>ye)? ys: ye;
		double z_down = (zs>ze)? ze: zs;
		double z_top = (zs>ze)? zs: ze;
		x_top +=rs;x_down-=rs;z_top+=rs;z_down-=rs;z_top+=rs;z_down-=rs;
		//x_top +=2*rs;x_down-=2*rs;z_top+=2*rs;z_down-=2*rs;z_top+=2*rs;z_down-=2*rs;
		x_down = (x_down<0)?0:x_down;
		y_down = (y_down <0)?0:y_down;
		z_down = (z_down <0)?0:z_down;
		z_top = (z_top >sz)?sz:z_top;	
		x_top = (x_top>sx)?sx:x_top;
		y_top = (y_top>sy)?sy:y_top;		
//		/*********************************************************************/// compute cylinder and flag mask 
		for(V3DLONG k=V3DLONG(z_down); k<V3DLONG(z_top); k++)
		{
			for(V3DLONG j=V3DLONG(y_down); j<V3DLONG(y_top); j++)
			{
				for(V3DLONG i=V3DLONG(x_down); i<V3DLONG(x_top); i++)
				{
					k = (k>=sz)?sz:k;
					j = (j>=sy)?sy:j;
					i = (i>=sx)?sx:i;
					V3DLONG indLoop = (k)*sx*sy + (j)*sx + i;
			//		if (indLoop > sz*sx*sy) 
//					{
//						indLoop = sz*sx*sy;
//					}
					double countxsi = (xs-i);
					double countysj = (ys-j);
					double countzsk = (zs-k);
					double countxes = (xe-xs);
					double countyes = (ye-ys);
					double countzes = (ze-zs);
					double norms10 = countxsi * countxsi + countysj * countysj + countzsk * countzsk;
					double norms21 = countxes * countxes + countyes * countyes + countzes * countzes;
					double dots1021 = countxsi * countxes + countysj * countyes + countzsk * countzes; 
					double dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
					double t1 = -dots1021/norms21;
                    if(t1<0)
                        dist = sqrt(norms10);
                    else if(t1>1)
                        dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k)*scalar2);
				    int n=rand()%256+1;
					///////////////// compute piont of intersection
					double v1 = xe - xs;
					double v2 = ye - ys;
					double v3 = ze - zs;
					double vpt = v1*v1 + v2*v2 +v3*v3;
					double t = ((i-xs)*v1 +(j-ys)*v2 +(k-zs)*v3)/vpt;
					double temR,temr;
					if (rs > re)
					{
						temR = rs;
						temr = re;
				
					}else
					{ 
						temR = re;
						temr = rs;
					}
					double xc = xs + v1*t;
					double yc = ys + v2*t;
					double zc = zs + v3*t;
 					//////////////////////compute rr
					double normssc = sqrt((xs-xc)*(xs-xc)+(ys-yc)*(ys-yc)+(zs-zc)*(zs-zc));
					double normsgc = sqrt(normssc*normssc-(rs-re)*(rs-re));
				//	double rr = dist*normssc/normsgc;
					double rr = temR - (temR - temr)/sqrt(norms21)*normssc;
					/////////////////////
					if(dist < rr)
					{   
						printf("rs=%lf rr=%lf dist=%lf re%lf \n",rs,rr,dist,re);
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
	V3DLONG alpha,beta,namesize,temp;
	V3DLONG sx,sy,sz;
	namesize = 0;
	unsigned char* pImMask;
	unsigned char* ImMark;
	
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
			for (V3DLONG ii=0; ii< neurons.listNeuron.size(); ii++)
			{
				p_t = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));

				double xs = p_t->x ;
				double ys = p_t->y ;
				double zs = p_t->z ;
				x_min = (xs<x_min)? xs:x_min;
				x_max = (xs>x_max)? xs:x_max;
				y_min = (ys<y_min)? ys:y_min;
				y_max = (ys>y_max)? ys:y_max;
				z_min = (zs<z_min)? zs:z_min;
				z_max = (zs>z_max)? zs:z_max;				 
			}		
			sx = abs((x_max  - x_min));
			sy = abs((y_max  - y_min));
			sz = abs((z_max  - z_min));
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
			ImMark = new unsigned char [pagesz];
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
		namesize = filenames.size();
		NeuronSWC *p_t=0;
		for (V3DLONG i = 0; i < filenames.size();i++)
		{
			filename = filenames[i];
			if (filename.size()>0)
			{
				neurons = readSWC_file(filename);
				
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
					z_min = (zs<z_min)? zs:z_min;
					z_max = (zs>z_max)? zs:z_max;				 
				}
			}else 
			{
				v3d_msg("You don't have any image open in the main window.");
				return;
			}
		}
		sx = abs((x_max  - x_min));
		sy = abs((y_max  - y_min));
		sz = abs((z_max  - z_min));
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
		 ImMark = new unsigned char [pagesz];
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
		for (V3DLONG i = 0; i < filenames.size();i++)
		{
			filename = filenames[i];
			if (filename.size()>0)
			{
				neurons = readSWC_file(filename);
			
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
	callback.updateImageWindow(newwin);
		
}
