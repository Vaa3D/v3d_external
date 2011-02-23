/* mapviewer.cpp
 * 2011-01-26 the program is created by Yang jinzhu
 */

#include "mapviewer.h"
#include "stackutil-11.h"
#include "v3d_message.h" 
#include <QPainter>

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(mapviewer, MAPiewerPlugin);

template <class T> QPixmap copyRaw2QPixmap_xPlanes(const T * pdata,
												   V3DLONG sz0,
												   V3DLONG sz1,
												   V3DLONG sz2,
												   V3DLONG sz3,
												   ImageDisplayColorType Ctype,
												   V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,
												   double *p_vmax,
												   double *p_vmin)
{
	QImage tmpimg = QImage(sz2, sz1, QImage::Format_RGB32);//zy
	
	int tr,tg,tb;
	
	V3DLONG i,j;
	
	double tmpr,tmpg,tmpb;
	
	double tmpr_min, tmpg_min, tmpb_min;
	
	if (sz3>=3)
	{
		tmpb = p_vmax[2]-p_vmin[2]; tmpb = (tmpb==0)?1:tmpb;
		tmpb_min = p_vmin[2];
	}
	
	if (sz3>=2)
	{
		tmpg = p_vmax[1]-p_vmin[1]; tmpg = (tmpg==0)?1:tmpg;
		tmpg_min = p_vmin[1];
	}
	
	if (sz3>=1)
	{
		tmpr = p_vmax[0]-p_vmin[0]; tmpr = (tmpr==0)?1:tmpr;
		tmpr_min = p_vmin[0];
	}
	
	int channel_compressed_sz = sz0*sz1*sz2;
	
	switch (Ctype) 
	{
		case colorRed2Gray:
			for (long j = 0; j < sz1; j ++) 
			{
				long offset = j*sz0 + cz0;
				for (long k =0; k < sz2; k++) 
				{
					long idx = offset + k*sz0*sz1;
					
					tb = tg = tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
					
					tmpimg.setPixel(k, j, qRgb(tr, tg, tb));
				}
			}
			break;
		case colorRGB:
			for (long j = 0; j < sz1; j ++) 
			{
				long offset = j*sz0 + cz0;
				for (long k =0; k < sz2; k++) 
				{
					long idx = offset + k*sz0*sz1;
					
					tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
					
					tg = floor((pdata[idx+channel_compressed_sz]-tmpg_min)/tmpg*255.0);
					
					tb = floor((pdata[idx+2*channel_compressed_sz]-tmpb_min)/tmpb*255.0);
					
					tmpimg.setPixel(k, j, qRgb(tr, tg, tb));
				}
			}
			
			break;

		case colorRG:
				tb = 0;
				for (long j = 0; j < sz1; j ++) 
				{
					long offset = j*sz0 + cz0;
					for (long k =0; k < sz2; k++) 
					{
						long idx = offset + k*sz0*sz1;
						
						tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
						
						tg = floor((pdata[idx+channel_compressed_sz]-tmpg_min)/tmpg*255.0);
						tmpimg.setPixel(k, j, qRgb(tr, tg, tb));
					}
				}
			
			break;

		default:
			break;
	}
	
	//painter.setCompositionMode(QPainter::CompositionMode_Source);
	//painter.drawImage(0,0, xy_image);
	
	return QPixmap::fromImage(tmpimg);	
	
}


template <class T> QPixmap copyRaw2QPixmap_yPlanes(const T * pdata,
												   V3DLONG sz0,
												   V3DLONG sz1,
												   V3DLONG sz2,
												   V3DLONG sz3,
												   ImageDisplayColorType Ctype,
												   V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,
												   double *p_vmax,
												   double *p_vmin)//xz
{
	
	QImage tmpimg = QImage(sz0, sz2, QImage::Format_RGB32);
	
	int tr,tg,tb;
	
	V3DLONG i,j;
	
	double tmpr,tmpg,tmpb;
	
	double tmpr_min, tmpg_min, tmpb_min;
	
	if (sz3>=3)
	{
		tmpb = p_vmax[2]-p_vmin[2]; tmpb = (tmpb==0)?1:tmpb;
		tmpb_min = p_vmin[2];
	}
	
	if (sz3>=2)
	{
		tmpg = p_vmax[1]-p_vmin[1]; tmpg = (tmpg==0)?1:tmpg;
		tmpg_min = p_vmin[1];
	}
	
	if (sz3>=1)
	{
		tmpr = p_vmax[0]-p_vmin[0]; tmpr = (tmpr==0)?1:tmpr;
		tmpr_min = p_vmin[0];
	}

	int channel_compressed_sz = sz0*sz1*sz2;
	
//	for (long k = 0; k < sz2; k ++) 
//	{
//		long offset = k*sz0*sz1 + cz1*sz0;
//		for (long i =0; i < sz0; i++) 
//		{
//			long idx = offset + i;
//			
//			tmpimg.setPixel(i,k,qRgb(pdata[idx], pdata[idx+channel_compressed_sz], pdata[idx+2*channel_compressed_sz]));
//		}
//	}
	
	switch (Ctype) 
	{
		case colorRed2Gray:
			for (long k = 0; k < sz2; k ++) 
			{
				long offset = k*sz0*sz1 + cz1*sz0;
				
				for (long i =0; i < sz0; i++) 
				{
					long idx = offset + i;
					
					tb = tg = tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
					tmpimg.setPixel(i, k, qRgb(tr, tg, tb));
					
				}
			}
			
			break;
			
		case colorRGB:
			for (long k = 0; k < sz2; k ++) 
			{
				long offset = k*sz0*sz1 + cz1*sz0;
				
				for (long i =0; i < sz0; i++) 
				{
					long idx = offset + i;
					
					tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
					tg = floor((pdata[idx+channel_compressed_sz]-tmpg_min)/tmpg*255.0);
					tb = floor((pdata[idx+2*channel_compressed_sz]-tmpb_min)/tmpb*255.0);
					tmpimg.setPixel(i, k, qRgb(tr, tg, tb));
					
				}
			}
			
			break;
			
		case colorRG:
			tb = 0;
			for (long k = 0; k < sz2; k ++) 
			{
				long offset = k*sz0*sz1 + cz1*sz0;
				
				for (long i =0; i < sz0; i++) 
				{
					long idx = offset + i;
					
					tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
					tg = floor((pdata[idx+channel_compressed_sz]-tmpg_min)/tmpg*255.0);
					tmpimg.setPixel(i, k, qRgb(tr, tg, tb));
					
				}
			}
			
			break;
			
		default:
			break;
	}
	return QPixmap::fromImage(tmpimg);
}

template <class T> QPixmap copyRaw2QPixmap_zPlanes(const T * pdata,
												   V3DLONG sz0,
												   V3DLONG sz1,
												   V3DLONG sz2,
												   V3DLONG sz3,
												   ImageDisplayColorType Ctype,
												   V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,
												   double *p_vmax,
												   double *p_vmin)//xy
{
	QImage tmpimg = QImage(sz0, sz1, QImage::Format_RGB32);
	
	int tr,tg,tb;
	
	V3DLONG i,j;
	double tmpr,tmpg,tmpb;
	double tmpr_min, tmpg_min, tmpb_min;
	
	if (sz3>=3)
	{
		tmpb = p_vmax[2]-p_vmin[2]; tmpb = (tmpb==0)?1:tmpb;
		tmpb_min = p_vmin[2];
	}
	
	if (sz3>=2)
	{
		tmpg = p_vmax[1]-p_vmin[1]; tmpg = (tmpg==0)?1:tmpg;
		tmpg_min = p_vmin[1];
	}
	
	if (sz3>=1)
	{
		tmpr = p_vmax[0]-p_vmin[0]; tmpr = (tmpr==0)?1:tmpr;
		tmpr_min = p_vmin[0];
	}
	int channel_compressed_sz = sz0*sz1*sz2;
	
	switch (Ctype) 
	{
		case colorRed2Gray:
			for (long j = 0; j < sz1; j ++) 
			{
				long offset = cz2*sz0*sz1 + j*sz0;
				for (long i=0; i<sz0; i++) 
				{
					long idx = offset + i;
					
					tb = tg = tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;
		case colorRGB:
			for (long j = 0; j < sz1; j ++) 
			{
				long offset = cz2*sz0*sz1 + j*sz0;
				for (long i=0; i<sz0; i++) 
				{
					long idx = offset + i;
					
					tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
					tg = floor((pdata[idx+channel_compressed_sz]-tmpg_min)/tmpg*255.0);
					tb = floor((pdata[idx+2*channel_compressed_sz]-tmpb_min)/tmpb*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;
        case colorRG:
			tb = 0;
			for (long j = 0; j < sz1; j ++) 
			{
				long offset = cz2*sz0*sz1 + j*sz0;
				for (long i=0; i<sz0; i++) 
				{
					long idx = offset + i;
					tr = floor((pdata[idx]-tmpr_min)/tmpr*255.0);
					tg = floor((pdata[idx+channel_compressed_sz]-tmpg_min)/tmpg*255.0);
					tmpimg.setPixel(i, j, qRgb(tr, tg, tb));
				}
			}
			break;

		default:
			break;
	}
		return QPixmap::fromImage(tmpimg);
	
}

template <class T1, class T2> 
int CopyData_resamp_raw(T1 *apsInput, T2 *aspOutput,V3DLONG channel_size, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG resampling_size)
{
	
	long vx, vy, vz, vc;
	long rx, ry, rz, rc;
	
	vx = iImageWidth/resampling_size ; 
	
	ry = vy = iImageHeight;
	
	rz = vz = iImageLayer;
	
	rc = vc = channel_size;
	
	rx = iImageWidth ; 
	
	V3DLONG tempc = vx*vy*vz, tempcz = vx*vy;
	
	V3DLONG temprc = rx*ry*rz, temprcz = rx*ry;
	
	V3DLONG t = resampling_size;
	
	for(long c=0; c<rc; c++)
	{
		long oc = c*tempc;
		long orc = c*temprc;
		
		for(long k= 0; k<rz; k++)
		{
			long omk = oc + (k)*tempcz;
			
			long ork = orc + (k)*temprcz;
			
			for(long j=0; j<ry; j++)
			{
				long oj = omk + (j)*vx;
				
				long orj = ork + (j)*rx;
				
				for(long i=0; i<rx; i = i+resampling_size)
				{
					long idx = oj + i/t;
					long idxr = orj + i;
					{
						aspOutput[idx] = apsInput[idxr];
					}
				}
			}
		}
	}
	
}
template <class T1, class T2> 
int CopyData_resamp_tc(T1 *apsInput, T2 *aspOutput,V3DLONG * sz,V3DLONG * szo,
					   V3DLONG start_x, V3DLONG start_y, V3DLONG start_z,
				       V3DLONG x_start,V3DLONG y_start,V3DLONG z_start, V3DLONG x_end,V3DLONG y_end,V3DLONG z_end, 
					   V3DLONG tile2vi_xs, V3DLONG tile2vi_ys,V3DLONG tile2vi_zs,V3DLONG resampling_size)
{
	
	
	
	long vx, vy, vz, vc;
	long rx, ry, rz, rc;
	
	vx = (sz[0])/resampling_size; // suppose the size same of all tiles
	
	vy = (sz[1])/resampling_size;
	
	vz = (sz[2])/resampling_size;
	
	vc = sz[3];
	
	rx=szo[0]; ry=szo[1]; rz=szo[2];rc=szo[3];	
	
	V3DLONG tempc = vx*vy*vz, tempcz = vx*vy;
	
	V3DLONG temprc = rx*ry*rz, temprcz = rx*ry;
	
	V3DLONG t = resampling_size;
	
	for(long c=0; c<rc; c++)
	{
		long oc = c*tempc;
		long orc = c*temprc;
		
		for(long k=(z_start/t); k<(z_end/t); k++)
		{
			long omk = oc + (k-start_z/t)*tempcz;
			
			long ork = orc + (k-tile2vi_zs/t)*temprcz;
			
			for(long j=(y_start/t); j<(y_end/t); j++)
			{
				long oj = omk + (j-start_y/t)*vx;
				
				long orj = ork + (j-tile2vi_ys/t)*rx;
				
				for(long i=x_start; i<x_end; i = i+resampling_size)
				{
					long idx = oj + i/t - start_x;
					long idxr = orj + (i - tile2vi_xs);
					//long idx = oj + i - start_x;
					//long idxr = orj + (i - tile2vi_xs);
					{
						aspOutput[idx] = apsInput[idxr];
					}
				}
			}
		}
	}
	
}

template <class T> 
int CopyData(T *apsInput, T *aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer,V3DLONG channel_size)
{
		
	V3DLONG tempc = iImageWidth*iImageHeight*iImageLayer, tempz = iImageWidth*iImageHeight;
	
	for(long c=0; c<channel_size; c++)
	{
		//long orc = c*temprc;
		
		for(long k= 0; k<iImageLayer; k++)
		{
			//long ork = orc + (k)*temprcz;
			
			for(long j=0; j<iImageHeight; j++)
			{
				//long orj = ork + (j)*rx;
				
				for(long i=0; i<iImageWidth; i++)
				{
					//long idx = orj + i;
					
					aspOutput[c*tempc+k*tempz+j*iImageWidth+i] = apsInput[c*tempc+k*tempz+j*iImageWidth+i];
				}
			}
		}
	}
	
}
template <class T>
int CopyData_tc(T *apsInput, T *aspOutput,V3DLONG * szo,
			V3DLONG start_x, V3DLONG start_y, V3DLONG start_z,
			V3DLONG x_start,V3DLONG y_start,V3DLONG z_start, V3DLONG x_end,V3DLONG y_end,V3DLONG z_end, 
			V3DLONG vx,V3DLONG vy,V3DLONG vz)
{
	long rx,ry,rz,rc;
	
	rx=szo[0], ry=szo[1], rz=szo[2], rc=szo[3];
	
	//	qDebug()<<"x_y_z="<<rx<<ry<<rz;
	
	V3DLONG tempc = vx*vy*vz, tempcz = vx*vy;
	V3DLONG temprc = rx*ry*rz, temprcz = rx*ry;
	V3DLONG total = tempc * rc;
	V3DLONG totalr = temprc * rc;
	for(long c=0; c<rc; c++)
	{
		long oc = c*tempc;
		long orc = c*temprc;
		for(long k=z_start; k<z_end; k++)
		{
			long omk = oc + (k-start_z)*tempcz;
			long ork = orc + (k-z_start)*temprcz;
			for(long j=y_start; j<y_end; j++)
			{
				long oj = omk + (j-start_y)*vx;
				long orj = ork + (j-y_start)*rx;
				for(long i=x_start; i<x_end; i++)
				{
					long idx = oj + (i-start_x);
					long idxr = orj + (i-x_start);
					if(idx > total ){idx = total;}
					if(idxr > totalr ){idxr = totalr;}
					//if(pVImg[idx]>0)
					{
						//	pVImg[idx] = (pVImg[idx]>relative1d[idxr])?pVImg[idx]:relative1d[idxr];
					}
					//else
					{
						aspOutput[idx] = apsInput[idxr];
					}
				}
			}
		}
	}
	
}


const QString title = "Map Viewer";
XMapView* XMapView::m_show = 0;
QStringList MAPiewerPlugin::menulist() const
{
    return QStringList() << tr("load Image")
						 << tr("generate thumbnail map from tc file")
	                     << tr("generate thumbnail map from raw data")
						 << tr("help");
}

void MAPiewerPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == tr("load Image"))
    {
    	iViewer(callback, parent);
    }
	else if(menu_name == tr("generate thumbnail map from tc file")) 
	{
		resampling_tc(callback, parent);
	}
	else if(menu_name == tr("generate thumbnail map from raw data")) 
	{
		resampling_rawdata(callback, parent);
	}
	else if (menu_name == tr("help"))
	{
	}
}

void MAPiewerPlugin::resampling_tc(V3DPluginCallback &callback, QWidget *parent)
{
	QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open profile"), "", QObject::tr("Supported file (*.tc)"));
	if(m_FileName.isEmpty())	
		return;
	// tiled images path
	QString curFilePath = QFileInfo(m_FileName).path();
	
	curFilePath.append("/");
	
	string filename = m_FileName.toStdString();
	
	if(vim.y_load(filename)!= true)
	{
		QMessageBox::information(0, "TC file reading", QObject::tr("tc file is illegal"));
		return ;
	}
	
	V3DLONG *sz1;
	
	sz1 = new V3DLONG [4];
	
	long sx=vim.sz[0], sy=vim.sz[1], sz=vim.sz[2];
	
	sz1[0]=sx;
	sz1[1]=sy;
	sz1[2]=sz;
	sz1[3] =vim.sz[3];

	long temsize = (sx > sy)? sx:sy; 
	
	int target_pixel_size;
	
	if (temsize <= 1000) 
	{
		target_pixel_size = 2;
	}else if(1000 < temsize <=2000)
	{
		target_pixel_size = 4;
		
	}else if(2000 < temsize <=3000)
	{
		target_pixel_size = 6;
		
	}else if(3000 < temsize <=4000)
	{
		target_pixel_size = 8;
		
	}else 
	{
		target_pixel_size = 10;
		
	}

	size_t start_t = clock();
	
	long vx, vy, vz, vc;
	long rx, ry, rz, rc;
	
	long start_x = 0,start_y = 0, start_z=0;
	long end_x = sx;
	long end_y = sy;
	long end_z = sz;
	
	vx = (sx)/target_pixel_size; // suppose the size same of all tiles
	
	vy = (sy)/target_pixel_size;
	
	vz = (sz)/target_pixel_size;
	
	vc = vim.sz[3];
	
	qDebug()<<"vx vy vz target_size"<<vx<<vy<<vz<<target_pixel_size;
	
	long pagesz_vim = vx*vy*vz*vc;
	
	void *pData = NULL;

	ImagePixelType datatype;
	bool rdaa = true;
	
	//for(long ii=0; ii<1; ii++)
	for(long ii=0; ii<vim.number_tiles; ii++)
	{	
		cout << "satisfied image: "<< vim.lut[ii].fn_img << endl;

		char * curFileSuffix = getSurfix(const_cast<char *>(vim.lut[ii].fn_img.c_str()));

		cout << "suffix ... " << curFileSuffix << endl; // 

		QString curPath = curFilePath;

		string fn = curPath.append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();

		qDebug()<<"testing..."<<curFilePath<< fn.c_str();

		char * imgSrcFile = const_cast<char *>(fn.c_str());

		V3DLONG *sz_relative = 0; 
		
		int datatype_relative = 0;
		
		unsigned char* resampling = 0;
		
		V3DLONG *szo=0;
		
		if(loadImage_resampling(imgSrcFile,resampling,sz_relative,szo,datatype_relative,target_pixel_size)!=true)
		{
			QMessageBox::information(0, "Load Image", QObject::tr("File load failure"));
			return;
		
		}
		long tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0]; 
		long tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0]; 
		long tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1]; 
		long tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1]; 
		long tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2]; 
		long tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2]; 
		
		long x_start = (start_x > tile2vi_xs) ? start_x : tile2vi_xs; 
		long x_end = (end_x < tile2vi_xe) ? end_x : tile2vi_xe;
		long y_start = (start_y > tile2vi_ys) ? start_y : tile2vi_ys;
		long y_end = (end_y < tile2vi_ye) ? end_y : tile2vi_ye;
		long z_start = (start_z > tile2vi_zs) ? start_z : tile2vi_zs;
		long z_end = (end_z < tile2vi_ze) ? end_z : tile2vi_ze;
        
		// loading relative imagg files
		
		if(datatype_relative ==1)
		{
			if(rdaa)
			{
					try
				{
					pData  = new unsigned char [pagesz_vim];
					
					memset(pData, 0, sizeof(unsigned char)*pagesz_vim);
				}
				catch (...) 
				{
					printf("Fail to allocate memory.\n");
					return ;
				}
				rdaa = false;
			}
			
			datatype = V3D_UINT8;
			
			CopyData_resamp_tc((unsigned char*)resampling,(unsigned char*)pData,sz1,szo,
							   start_x,start_y,start_z,
							   x_start,y_start,z_start,x_end,y_end,z_end,
							   tile2vi_xs,tile2vi_ys,tile2vi_zs,target_pixel_size);
			
		}
		else if(datatype_relative == 2)
		{
			if(rdaa)
			{
				try
			{
				pData = new unsigned short [pagesz_vim]; 
				memset(pData, 0, sizeof(unsigned short)*pagesz_vim);
			}
			catch (...)
			{
				printf("Fail to allocate memory in data combination.");
				if (pData) {delete []pData; pData=0;}
				return;
			}
				rdaa = false;
		}
			
			datatype = V3D_UINT16;
			CopyData_resamp_tc((unsigned short*)resampling,(unsigned short*)pData,sz1,szo,
								start_x,start_y,start_z,
								x_start,y_start,z_start,x_end,y_end,z_end,
								tile2vi_xs,tile2vi_ys,tile2vi_zs,target_pixel_size);
			
		}
		else if(datatype_relative==4)
		{
			//float*	pData = NULL;
			if(rdaa)
			{
			try
			{
				pData = new float [pagesz_vim];
				memset(pData, 0, sizeof(float)*pagesz_vim);
			}
			catch (...)
			{
				printf("Fail to allocate memory in data combination.");
				if (pData) {delete []pData; pData=0;}
				return;
			}
				rdaa = false;
			}
			datatype = V3D_FLOAT32;
			CopyData_resamp_tc((float*)resampling,(float*)pData,sz1,szo,
								start_x,start_y,start_z,
								x_start,y_start,z_start,x_end,y_end,z_end,
								tile2vi_xs,tile2vi_ys,tile2vi_zs,target_pixel_size);
		}
		
	
		if(sz_relative) {delete []sz_relative; sz_relative=0;}
		if(resampling) {delete []resampling; resampling=0;}
		if(szo) {delete []szo; szo=0;}
	}
	// time consumption
	size_t end_t = clock();
	
	cout<<"resampling time = "<<end_t-start_t<<endl;
	
	Image4DSimple p4DImage;
	//	
	p4DImage.setData((unsigned char *)pData, vx, vy, vz, vc, datatype);
	
	v3dhandle curwin;
	
	if(!callback.currentImageWindow())
		curwin = callback.newImageWindow();
	else
		curwin = callback.currentImageWindow();
	
	callback.setImage(curwin, &p4DImage);
	callback.setImageName(curwin, "Resampling Image");
	callback.updateImageWindow(curwin);
	
	callback.pushImageIn3DWindow(curwin);
	
	V3DLONG sz_tmp[4];

	QString tmp_filename = curFilePath + "/" + "stitched_image.raw";
	
	sz_tmp[0] = vx; sz_tmp[1] = vy; sz_tmp[2] = vz; sz_tmp[3] = vc; 
	
	if (saveImage(tmp_filename.toStdString().c_str(), (const unsigned char *)pData, sz_tmp, datatype)!=true)
	{
		fprintf(stderr, "Error happens in file writing. Exit. \n");
		return ;
	}
	////////////////////////////////////////// save tc file
//	Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim1;
//	
//	V3DLONG count=0;
//	
//	V3DLONG offset[3];
//	
//	offset[0]=0; offset[1]=0; offset[2]=0;
//	
//	indexed_t<V3DLONG, REAL> idx_t(offset);
//	
//	idx_t.n = count;
//	idx_t.ref_n = 0; // init with default values
//	idx_t.fn_image = m_FileName.toStdString();
//	idx_t.score = 0;
//	vim1.tilesList.push_back(idx_t);
//	
//	int a  = vim1.tilesList.size();
//	
//	(&vim1.tilesList.at(0))->sz_image = new V3DLONG [4];
//	
//	(&vim1.tilesList.at(0))->sz_image[0] = sz_relative[0];
//	(&vim1.tilesList.at(0))->sz_image[1] = sz_relative[1];
//	(&vim1.tilesList.at(0))->sz_image[2] = sz_relative[2];
//	(&vim1.tilesList.at(0))->sz_image[3] = sz_relative[3];
//	
//	QString tmp_filename;
//	QString tmp_filename1;
//	
//	tmp_filename = curFilePath + "/" + "stitched_image.tc"; //.tc tile configuration
//	
//	tmp_filename1 = curFilePath + "/" + "stitched_image.raw";
//	
//	// construct lookup table
//	vim1.y_clut(vim1.tilesList.size());
//	
//	QString ff = "stitched_image.raw";
//	
//	char * thumnalilFile = const_cast<char *>(ff.toStdString().c_str());
//	
//	strcpy(vim1.fn_thumbnail, thumnalilFile);
//	
//	//vim1.fn_thumbnail = "stitched_image.raw";
//	
//	vim1.b_thumbnailcreated = true;
//	
//	vim1.y_save(tmp_filename.toStdString());
	////////////////////////////////////////////end tc file
	
	if (sz1) {delete []sz1; sz1=0;}
}
void MAPiewerPlugin::resampling_rawdata(V3DPluginCallback &callback, QWidget *parent)
{
	QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open profile"), "", QObject::tr("Supported file (*.raw)"));
	if(m_FileName.isEmpty())	
		return;
	
	QString curFilePath = QFileInfo(m_FileName).path();
	
	QString curPath = curFilePath;
	
	string fn = m_FileName.toStdString();
	
	char * imgSrcFile = const_cast<char *>(fn.c_str());
	
	V3DLONG *sz_relative = 0; 
	
	int datatype_relative = 0;
	
	ImagePixelType datatype;
	
	unsigned char* resampling = 0;
	
	int target_pixel_size;
	
	V3DLONG *szo=0;
	
	size_t start_t = clock();
	
	if(loadImage_raw_resampling(imgSrcFile,resampling,szo,sz_relative,datatype_relative,target_pixel_size)!= true)
	{
		QMessageBox::information(0, "Load Image", QObject::tr("File load failure"));
		return;
	}
	
	long vx, vy, vz, vc;
		
	vx = szo[0]/target_pixel_size ; 
	
	vy = szo[1];
	
	vz = szo[2];
	
	vc = szo[3];
	
	//----------------------------------------------------------------------------------------------------------------------------------
	V3DLONG channelsz = szo[0]*szo[1]*szo[2];
	
	V3DLONG pagesz_vim = szo[3]*channelsz;
	
	void *pData = NULL;
	
	if(datatype_relative ==1 )
	{
		try
		{
	    	pData  = new unsigned char [pagesz_vim];
			
			memset(pData, 0, sizeof(unsigned char)*pagesz_vim);
		}
		catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return ;
		}

			datatype = V3D_UINT8;
		
			CopyData_resamp_raw((unsigned char*)resampling,(unsigned char*)pData,szo[3], szo[0], szo[1], szo[2],target_pixel_size);
		
	}
	else if(datatype_relative == 2)
	{
		try
		{
			pData = new unsigned short [pagesz_vim]; 
			memset(pData, 0, sizeof(unsigned short)*pagesz_vim);
		}
		catch (...)
		{
			printf("Fail to allocate memory in data combination.");
			if (pData) {delete []pData; pData=0;}
			return;
		}
		
			datatype = V3D_UINT16;
			CopyData_resamp_raw((unsigned short*)resampling,(unsigned short*)pData,szo[3], szo[0], szo[1], szo[2],target_pixel_size);
		
	}
	else if(datatype_relative==4)
	{
		//float*	pData = NULL;
		try
		{
			pData = new float [pagesz_vim];
			memset(pData, 0, sizeof(float)*pagesz_vim);
		}
			catch (...)
		{
			printf("Fail to allocate memory in data combination.");
			if (pData) {delete []pData; pData=0;}
			return;
		}

			datatype = V3D_FLOAT32;
			CopyData_resamp_raw((float*)resampling,(float*)pData,szo[3], szo[0], szo[1], szo[2],target_pixel_size);
	}
	// time consumption
	size_t end_t = clock();

	cout<<"resampling time = "<<end_t-start_t<<endl;
	
	Image4DSimple p4DImage;
	//	
	p4DImage.setData((unsigned char *)pData, vx, vy, vz, vc, datatype);
	
	v3dhandle curwin;
	
	if(!callback.currentImageWindow())
		curwin = callback.newImageWindow();
	else
		curwin = callback.currentImageWindow();
	
	callback.setImage(curwin, &p4DImage);
	callback.setImageName(curwin, "Resampling Image");
	callback.updateImageWindow(curwin);
	
	callback.pushImageIn3DWindow(curwin);
	
	V3DLONG sz_tmp[4];
	//////////////////////save tc.file
	
	REAL *scale = new REAL [6];
	
	scale[0] = 1;
	scale[1] = 1;
	scale[2] = 1;
	scale[3] = 1;
	scale[4] = 1;
	scale[5] = 1;
	
	Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim1;
	
	V3DLONG count=0;
	
	V3DLONG offset[3];
	
	offset[0]=0; offset[1]=0; offset[2]=0;
	
	indexed_t<V3DLONG, REAL> idx_t(offset);
	
	idx_t.n = count;
	idx_t.ref_n = 0; // init with default values
	idx_t.fn_image = m_FileName.toStdString();
	idx_t.score = 0;
	vim1.tilesList.push_back(idx_t);
	
	int a  = vim1.tilesList.size();
	
	(&vim1.tilesList.at(0))->sz_image = new V3DLONG [4];
	
	(&vim1.tilesList.at(0))->sz_image[0] = sz_relative[0];
	(&vim1.tilesList.at(0))->sz_image[1] = sz_relative[1];
	(&vim1.tilesList.at(0))->sz_image[2] = sz_relative[2];
	(&vim1.tilesList.at(0))->sz_image[3] = sz_relative[3];
	
	QString tmp_filename;
	QString tmp_filename1;
	
	tmp_filename = curFilePath + "/" + "stitched_image.tc"; //.tc tile configuration
	
	tmp_filename1 = curFilePath + "/" + "stitched_image.raw";
	
	// construct lookup table
	vim1.y_clut(vim1.tilesList.size());
	
	QString ff = "stitched_image.raw";
	
	char * thumnalilFile = const_cast<char *>(ff.toStdString().c_str());
	
	strcpy(vim1.fn_thumbnail, thumnalilFile);
		
	//vim1.fn_thumbnail = "stitched_image.raw";
	
	vim1.b_thumbnailcreated = true;
	
	vim1.y_save(tmp_filename.toStdString());
	
	sz_tmp[0] = vx; sz_tmp[1] = vy; sz_tmp[2] = vz; sz_tmp[3] = vc; 
	
	if (saveImage(tmp_filename1.toStdString().c_str(), (const unsigned char *)pData, sz_tmp, datatype)!=true)
	{
		fprintf(stderr, "Error happens in file writing. Exit. \n");
		return ;
	}	
	
}
void MAPiewerPlugin::iViewer(V3DPluginCallback &callback, QWidget *parent)
{
	// load indexed data file
	QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open profile"), "", QObject::tr("Supported file (*.tc)"));
	
	if(m_FileName.isEmpty())	
		return;
	// tiled images path
	QString curFilePath = QFileInfo(m_FileName).path();
	curFilePath.append("/");
	
	if (XMapView::m_show)
	{
		XMapView::m_show->show();
		return;
	}
	ImageSetWidget *inw = new ImageSetWidget(callback, parent,m_FileName, curFilePath, 5);//5
	
	if (inw)
	{
		inw->show();
 
	}
}
void XMapView::setImgData(ImagePlaneDisplayType ptype,ImagePixelType dtype,ImageDisplayColorType Ctype,V3DLONG *sz_compressed,V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,unsigned char *pdata,double * p_vmax, double* p_vmin)
{
	cur_focus_pos = 1;
	
//	Ctype = ctype; //
	
	cx=sz_compressed[0], cy=sz_compressed[1], cz=sz_compressed[2], cc=sz_compressed[3];
	
	channel_compressed_sz = cx*cy*cz;
	
	Ptype = ptype;//plane type
	
	//V3DLONG pagesz_vim = cc * channel_compressed_sz;
	
	start_x = cur_x = cz0;
	
	cur_y = cz1;
	
	cur_z = cz2;
	
	Datatype = dtype;
	
	if(dtype==V3D_UINT8 )
	{		
		pixmap = copyRaw2QPixmap((const unsigned char *)pdata,cx,cy,cz,cc,Ctype,cur_x,cur_y,cur_z,Ptype,p_vmax,p_vmin);
		
	}
	else if(dtype==V3D_UINT16)
	{
		pixmap = copyRaw2QPixmap((const unsigned short int *)pdata,cx,cy,cz,cc,Ctype,cur_x,cur_y,cur_z,Ptype,p_vmax,p_vmin);
	}
	else if(dtype==V3D_FLOAT32)
	{
		pixmap = copyRaw2QPixmap((const float *)pdata,cx,cy,cz,cc,Ctype,cur_x,cur_y,cur_z,Ptype,p_vmax,p_vmin);

	}
	else
	{
		printf("Right now only support UINT8, UINT16, and FLOAT32.\n", 0);
		return; 
	}
	//qDebug()<<"setimagedata ..."<<cur_x<<cur_y<<cur_z;
	
}

void XMapView::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.save();
    
	painter.setRenderHint(QPainter::Antialiasing);
    
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
		
	int pwid = disp_width; //changed to disp_height/disp_width 
	int phei = disp_height;
    
	QPointF center(pwid/2.0, phei/2.0);
	if (m_scale>1)
		painter.translate(curDisplayCenter - center);
	else
		curDisplayCenter = center;
	
    painter.translate(center);
    painter.scale(m_scale, m_scale);
    painter.translate(-center);
	
    painter.scale(disp_scale, disp_scale);

    painter.drawPixmap(QPointF(0, 0), pixmap);
	
	painter.setPen(Qt::white );
	painter.setBrush(Qt::NoBrush);
	//if (QApplication::keyboardModifiers()==Qt::ControlModifier)
	{
		if (bMouseCurorIn && b_mousmove && b_mouseend)
		{
			QRect r ( dragStartPosition, curMousePos );
			painter.drawRect(r);
			//qDebug()<<"ee-xyz...."<<end_x<<end_y<<cz;
			//qDebug()<<"ss-xyz...."<<start_x<<start_y<<cur_z;
		}

	}
}

void ImageSetWidget::update_v3dviews(V3DPluginCallback *callback, long start_x, long start_y, long start_z, long end_x, long end_y, long end_z)
{
	// visualize in v3d tri-view
	
	size_t start_t = clock();
	
	//virtual image
	long vx, vy, vz, vc;
	
	vx = end_x - start_x + 1; // suppose the size same of all tiles
	vy = end_y - start_y + 1;
	vz = end_z - start_z + 1;
	vc = vim.sz[3];
	
	qDebug()<<"3dxyzc ..."<<start_x<<start_y<<start_z<<end_x<<end_y<<end_z;
	
	long pagesz_vim = vx*vy*vz*vc;
	
	unsigned char *pVImg = 0;
	
	try
	{
		pVImg = new unsigned char [pagesz_vim];
	}
	catch (...) 
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	
	// init
	for(long i=0; i<pagesz_vim; i++)
	{
		pVImg[i] = 0;
	}
	
	long x_s = start_x + vim.min_vim[0];
	long y_s = start_y + vim.min_vim[1];
	long z_s = start_z + vim.min_vim[2];
	
	long x_e = end_x + vim.min_vim[0];
	long y_e = end_y + vim.min_vim[1];
	long z_e = end_z + vim.min_vim[2];
	
	qDebug()<<"min ..."<<vim.min_vim[0]<<vim.min_vim[1]<<vim.min_vim[2];
	
	ImagePixelType datatype;
	
	cout << "satisfied image: "<< vim.lut[0].fn_img << endl;
	
	//
	//char * curFileSuffix = getSurfix(const_cast<char *>(vim.lut[0].fn_img.c_str()));
	
	//cout << "suffix ... " << curFileSuffix << endl; // tif lsm
	
	QString curPath = curFilePath;
	
	string fn = curPath.append( QString(vim.lut[0].fn_img.c_str()) ).toStdString();
	
	
	qDebug()<<"testing..."<<curFilePath<< fn.c_str();
	//
	char * imgSrcFile = const_cast<char *>(fn.c_str());
	
	size_t s1_t = clock();
	
	// loading relative imagg files
	V3DLONG *sz_relative = 0; 
	V3DLONG *szo = 0; 
	int datatype_relative = 0;
	unsigned char* relative1d = 0;
	
    //loadImage(imgSrcFile, relative1d, sz_relative, datatype_relative); //
  
	if(loadImage(imgSrcFile,relative1d,sz_relative,szo,start_x,start_y,start_z,end_x,end_y,end_z,datatype_relative)!=true)
	{
		QMessageBox::information(0, "Load Image", QObject::tr("File load failure"));
		return;
	}
	
	long rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];
	
	long sxx=szo[0], syy=szo[1], szz=szo[2], scc=szo[3];
	
	if(datatype_relative==1)
		datatype = V3D_UINT8;
	
	qDebug()<<"infomation..."<<rx<<ry<<rz;
	
	//qDebug()<<"infomationoooori..."<<sxx<<syy<<szz<<scc;
	
	size_t e1_t = clock();
	
	cout<<"time elapse for read tmpstack ... "<<e1_t-s1_t<<endl;
	int stt =2;
	if (stt == 1) 
	{
		for(long c=0; c<rc; c++)
		{
			long o_c = c*vx*vy*vz;
			long o_r_c = c*rx*ry*rz;
			for(long k=z_s; k<z_e; k++)
			{
				long o_k = o_c + (k-z_s)*vx*vy;
				long o_r_k = o_r_c + (k)*rx*ry;
				
				for(long j=y_s; j<y_e; j++)
				{
					long o_j = o_k + (j-y_s)*vx;
					long o_r_j = o_r_k + (j)*rx;
					for(long i=x_s; i<x_e; i++)
					{
						long idx = o_j + i-x_s;
						long idx_r = o_r_j + (i);
						pVImg[idx] = relative1d[idx_r];
					}
				}
			}
		}
		if(relative1d) {delete []relative1d; relative1d=0;}
		
	}
	
	size_t end1_t = clock();
	
	cout<<"time elapse ... "<<end1_t-start_t<<endl;
	
	//display
	Image4DSimple p4DImage;
	
	// p4DImage.setData((unsigned char*)relative1d, rx, ry, rz, rc, V3D_UINT16);
	 p4DImage.setData((unsigned char*)relative1d, szo[0], szo[1], szo[2], szo[3], V3D_UINT16);
	
	v3dhandle curwin;
	
	if(!callback->currentImageWindow())
		curwin = callback->newImageWindow();
	else
		curwin = callback->currentImageWindow();
	
	callback->setImage(curwin, &p4DImage);
	callback->setImageName(curwin, "Image");
	callback->updateImageWindow(curwin);
	
	callback->pushImageIn3DWindow(curwin);
	
	// time consumption
	size_t end_t = clock();
	
	cout<<"time elapse after loading configuration info ... "<<end_t-start_t<<endl;
	
	//loadImage(imgSrcFile, relative1d, sz_relative, datatype_relative); //
//	loadImage(imgSrcFile,relative1d,sz_relative,start_x,start_y,start_z,end_x,end_y,end_z,datatype_relative);	
	
	
}
XMapView::XMapView(QWidget *parent)
:QWidget(parent)
{		
	Gtype = PixmapType;
	m_scale = 1.0;
	//  m_shear = 0.0;
	//  m_rotation = 0.0;
	
	m_show = this;
	myCursor = QCursor(Qt::OpenHandCursor);
	
	b_displayFocusCrossLine = true;
		
	Ptype = imgPlaneUndefined; 
	cur_focus_pos = 1;
	compressed = 0; 
		
	// set a default map
	pixmap = QPixmap(256, 256);
	pixmap.fill(Qt::red);
	
	focusPosInWidth = pixmap.width()/2.0;
	focusPosInHeight = pixmap.height()/2.0;
	
	bMouseCurorIn = false;
	b_mouseend = false;
	b_mousmove = false;
	b_creadWindow = false;
	
	curDisplayCenter = QPoint(pixmap.width()/2.0, pixmap.height()/2.0);
	
	disp_scale = 1;
	disp_width = disp_scale * pixmap.width();
	disp_height = disp_scale * pixmap.height();
	
	start_x = start_y = start_z = end_x = end_y = end_z = 0;	
	in_startx = in_starty = in_endx = in_endy = 0;
	mousenumber = 0;
	
		
}
XMapView::~XMapView()
{
	m_show = 0;
	mousenumber = 0;
}


void ImageSetWidget::drawdata()
{
	 Bcopy = true;
	//qDebug()<<"draw";
	long s_x = xy_view->start_x;
	long s_y = xy_view->start_y;
	long s_z = xy_view->start_z;
	
    long e_x = xy_view->end_x;
	long e_y = xy_view->end_y;
	long e_z = xy_view->end_z;	
	long tem;
	if (s_x > e_x)
	{
		tem =e_x;
		e_x = s_x;
		s_x = tem ;
	}
	if (s_y > e_y)
	{
		tem =e_y;
		e_y = s_y;
		s_y = tem;
	}
	
	qDebug()<<"mapview_ImageN"<<s_x<<s_y<<s_z<<e_x<<e_y<<e_z;
	
	update_v3dviews(callback1, s_x*scaleFactor, s_y*scaleFactor, s_z*scaleFactor,e_x*scaleFactor, e_y*scaleFactor, e_z*scaleFactor);
	
	Bcopy = true;

}
void ImageSetWidget::updateGUI()
{
	
    cur_x = xValueSpinBox->text().toInt(); // / scaleFactor;
	cur_y = yValueSpinBox->text().toInt(); // / scaleFactor;
	cur_z = zValueSpinBox->text().toInt(); // / scaleFactor;
	
	CreadViewCheckBox->setEnabled(true);
	
	update_triview();
	
	xy_view->b_creadWindow = bcreadViews;
	
	yz_view->b_creadWindow = bcreadViews;
	
	zx_view->b_creadWindow = bcreadViews;

	xy_view->update();
	xy_view->repaint();
	
	yz_view->update();
	yz_view->repaint();
	
	zx_view->update();
	zx_view->repaint();
	
	
}
bool ImageSetWidget::updateminmaxvalues()
{
	//always delete the two pointers and recreate because if the image is altered in a plugin, the # of color channels may change	
	try
	{
		p_vmax = new double [cc];
		p_vmin = new double [cc];
	}
	catch (...)
	{
		printf("Error happened in allocating memory in updateminmaxvalues().\n");

		if (p_vmax) {delete []p_vmax; p_vmax=0;}
		if (p_vmin) {delete []p_vmin; p_vmin=0;}
		return false;
	}
	
	V3DLONG i, tmppos;
	
	V3DLONG channelPageSize = cx*cy*cz;
	
	if(p4DImage.setNewRawDataPointer(compressed1d))
	{
		p4DImage.setXDim(cx);
		p4DImage.setYDim(cy);
		p4DImage.setZDim(cz);
		p4DImage.setCDim(cc);
		
		p4DImage.setDatatype(dtype);
		
		switch (dtype)
		{
			case V3D_UINT8:
				for(i=0;i<cc;i++)
				{
					unsigned char minvv,maxvv;
					V3DLONG tmppos_min, tmppos_max;
					unsigned char *datahead = (unsigned char *)p4DImage.getRawDataAtChannel(i);
					minMaxInVector(datahead, channelPageSize, tmppos_min, minvv, tmppos_max, maxvv);
					p_vmax[i] = maxvv; p_vmin[i] = minvv;
					printf("channel= %ld min=%lf max=%lf\n",i,p_vmin[i],p_vmax[i]);
				}
				break;
				
			case V3D_UINT16:
				for(i=0;i<cc;i++)
				{
					unsigned short minvv,maxvv;
					V3DLONG tmppos_min, tmppos_max;
					unsigned short *datahead = (unsigned short *)p4DImage.getRawDataAtChannel(i);
					minMaxInVector(datahead, channelPageSize, tmppos_min, minvv, tmppos_max, maxvv);
					p_vmax[i] = maxvv; p_vmin[i] = minvv;
					printf("channel= %ld min=%lf max=%lf\n",i,p_vmin[i],p_vmax[i]);
				}
				break;
				
			case V3D_FLOAT32:
				for(i=0;i<cc;i++)
				{
					float minvv,maxvv;
					V3DLONG tmppos_min, tmppos_max;
					float *datahead = (float *)p4DImage.getRawDataAtChannel(i);
					minMaxInVector(datahead, channelPageSize, tmppos_min, minvv, tmppos_max, maxvv);
					p_vmax[i] = maxvv; p_vmin[i] = minvv;
					printf("channel= %ld min=%lf max=%lf\n",i,p_vmin[i],p_vmax[i]);
				}
				break;
				
			default:
				printf("Invalid data type found in updateminmaxvalues(). Should never happen, - check with V3D developers.");
				return false;
		}
	}
	

	return true;
}

void ImageSetWidget::createGUI()
{
	bcreadViews = false;
	/* Set up the data related GUI */
	dataGroup = new QGroupBox(this);
	dataGroup->setTitle("Compressed data");
	
	viewGroup = new QGroupBox(dataGroup);
	viewGroup->setTitle("Map Views ");
	
	xy_view = new XMapView(viewGroup);
	long x = cx/2;
	long y = cy/2;
	long z = cz/2;
 	
	//qDebug()<<"xyviewcurx ..."<<x/2<<y/2<<z/2;	
	
	xy_view->setImgData(imgPlaneZ,dtype,Ctype, sz_compressed,x,y,z,compressed1d,p_vmax,p_vmin); //because the second parameter is 0 (NULL pointer), then just load the default maps for this view
	
	xy_view->set_disp_width(cx);
	xy_view->set_disp_height(cy);
	xy_view->set_disp_scale(1);
	xy_view->setFixedWidth(xy_view->get_disp_width());
	xy_view->setFixedHeight(xy_view->get_disp_height());
	xy_view->setFocusPolicy(Qt::ClickFocus);
	
	yz_view = new XMapView(viewGroup);

	yz_view->setImgData(imgPlaneX,dtype,Ctype,sz_compressed,x,y,z,compressed1d,p_vmax,p_vmin); //because the second parameter is 0 (NULL pointer), then just load the default maps for this view
	
	yz_view->set_disp_width(cz);
	yz_view->set_disp_height(cy);
	yz_view->set_disp_scale(1);
	yz_view->setFixedWidth(yz_view->get_disp_width());
	yz_view->setFixedHeight(yz_view->get_disp_height());
	yz_view->setFocusPolicy(Qt::ClickFocus);
	
	zx_view = new XMapView(viewGroup);
	zx_view->setImgData(imgPlaneY,dtype,Ctype,sz_compressed,x,y,z,compressed1d,p_vmax,p_vmin); //because the second parameter is 0 (NULL pointer), then just load the default maps for this view
	
	zx_view->set_disp_width(cx);
	zx_view->set_disp_height(cz);
	zx_view->set_disp_scale(1);
	zx_view->setFixedWidth(zx_view->get_disp_width());
	zx_view->setFixedHeight(zx_view->get_disp_height());
	zx_view->setFocusPolicy(Qt::ClickFocus);
	
	//qDebug()<<"disp_x_y ..."<<xy_view->get_disp_width()<<xy_view->get_disp_height();		// setup the control panel
	
	mainGroup = new QGroupBox(this);
	mainGroup->setFixedWidth(200);
	mainGroup->setTitle("Options");
	
	//qDebug()<<"options ...";
	
	// focus planes group
	
	coordGroup = new QGroupBox(mainGroup);
	coordGroup->setAttribute(Qt::WA_ContentsPropagated);
	coordGroup->setTitle("Focus Coordinates");
	
	xSliderLabel = new QLabel("X", coordGroup);
	
	xValueSpinBox = new QSpinBox;
	
	xValueSpinBox->setMaximum(cx-1); xValueSpinBox->setMinimum(0); xValueSpinBox->setValue(cx/2); xValueSpinBox->setSingleStep(int(scaleFactor));
	
	ySliderLabel = new QLabel("Y", coordGroup);
	
	yValueSpinBox = new QSpinBox;
	
	yValueSpinBox->setMaximum(cy-1); yValueSpinBox->setMinimum(0); yValueSpinBox->setValue(cy/2); yValueSpinBox->setSingleStep(int(scaleFactor));
	
	zSliderLabel = new QLabel("Z", coordGroup);
	
	zValueSpinBox = new QSpinBox;
	
	zValueSpinBox->setMaximum(cz-1); zValueSpinBox->setMinimum(0); zValueSpinBox->setValue(cz/2); zValueSpinBox->setSingleStep(int(scaleFactor));

	//CreadViewCheckBox = new QCheckBox("cread window");
//	
//	CreadViewCheckBox->setCheckState((bcreadViews) ? Qt::Checked : Qt::Unchecked);

	// focus draw group
	
	QGroupBox * landmarkGroup = new QGroupBox(mainGroup);
	landmarkGroup->setTitle("draw data");
	
	dataCopyButton = new QPushButton(landmarkGroup);
	dataCopyButton->setText("setting");
	
	CreadViewCheckBox = new QCheckBox("cread window");
	
	CreadViewCheckBox->setCheckState((bcreadViews) ? Qt::Checked : Qt::Unchecked);
	
	//qDebug()<<"Coordinates ...";
	
	// All layouts
	allLayout = new QHBoxLayout(this);
	allLayout->addWidget(dataGroup);
	allLayout->addWidget(mainGroup);
	
	xyzViewLayout = new QGridLayout(viewGroup);
	xyzViewLayout->addWidget(xy_view, 0, 0, 1, 1, Qt::AlignRight | Qt::AlignBottom);
	xyzViewLayout->addWidget(yz_view, 0, 1, 1, 1, Qt::AlignLeft | Qt::AlignBottom);
	xyzViewLayout->addWidget(zx_view, 1, 0, 1, 1, Qt::AlignRight | Qt::AlignTop);
	xyzViewLayout->update();
	
	dataGroupLayout = new QVBoxLayout(dataGroup);
	dataGroupLayout->addWidget(viewGroup);
	dataGroupLayout->addStretch(0);
	
	// layout for focus planes
	coordGroupLayout = new QGridLayout(coordGroup);
	coordGroupLayout->addWidget(zSliderLabel, 0, 0, 1, 1);
	coordGroupLayout->addWidget(zValueSpinBox, 0, 2, 1, 4);
	
	coordGroupLayout->addWidget(xSliderLabel, 1, 0, 1, 1);
	coordGroupLayout->addWidget(xValueSpinBox, 1, 2, 1, 4);
	
	coordGroupLayout->addWidget(ySliderLabel, 2, 0, 1, 1);
	coordGroupLayout->addWidget(yValueSpinBox, 2, 2, 1, 4);

	//coordGroupLayout->addWidget(CreadViewCheckBox, 3, 0, 1, 1);

	// layout for draw data
	datacopyGroupLayout = new QGridLayout(landmarkGroup);
	//datacopyGroupLayout->addWidget(dataCopyButton, 0, 0, 1, 4);
	datacopyGroupLayout->addWidget(CreadViewCheckBox, 0, 0, 1, 1);
 	
	
	// main control panel layout
	mainGroupLayout = new QVBoxLayout(mainGroup);
	mainGroupLayout->addWidget(coordGroup);
	mainGroupLayout->addWidget(landmarkGroup); 

	setLayout(allLayout);
	updateGeometry();
	allLayout->update();
	
	Bcopy = false;
	
	update_triview();
	
	//update_v3dviews(callback, start_x, start_y, start_z,end_x,end_y,end_z);
	
	connect(xValueSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateGUI()));
	connect(yValueSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateGUI()));
	connect(zValueSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateGUI()));	
	connect(dataCopyButton, SIGNAL(clicked()), this, SLOT(drawdata()));
	connect(CreadViewCheckBox, SIGNAL(clicked()), this, SLOT(toggCreadViewCheckBox()));
	
}	//
void ImageSetWidget::toggCreadViewCheckBox()
{
    bcreadViews = (CreadViewCheckBox->checkState()==Qt::Checked) ? true : false;
	xy_view->b_creadWindow = bcreadViews;
	yz_view->b_creadWindow = bcreadViews;
	zx_view->b_creadWindow = bcreadViews;
	update();
}

bool ImageSetWidget::setCTypeBasedOnImageData()
{
    if (!compressed1d)
	{
		printf("Invalid data in setCTypeBasedOnImageData()");
		return false;
	}
	
    if (cc<1)
	{
		printf("Error in data reading. The number of color channels cannot be smaller than 1!!\n");
		if (compressed1d) {delete compressed1d; compressed1d = 0;}
		return false;
	}
	
	if (dtype==V3D_UINT8 ||
		dtype==V3D_UINT16 ||
		dtype==V3D_FLOAT32)
	{
		if (cc>=3)
			Ctype = colorRGB;
		else if (cc==2)
			Ctype = colorRG;
		else //==1
			Ctype = colorRed2Gray;
	}
	else
	{
		printf("Seems you load an unknown data which is not supported for display at this moment. -- setCTypeBasedOnImageData()", 0);
		Ctype = colorRed2Gray;
	}
	
	return true;
}


ImageSetWidget::ImageSetWidget(V3DPluginCallback &callback, QWidget *parent, QString m_FileName, QString curFilePathInput, float scaleFactorInput)
{
	callback1 = &callback;
	
	curFilePath = curFilePathInput;
	
	string filename = m_FileName.toStdString();
	
	qDebug()<<"filename ..."<<filename.c_str();

	if(vim.y_load(filename) == true)
	{
		scaleFactor = scaleFactorInput;
		
		long sx=vim.sz[0], sy=vim.sz[1], sz=vim.sz[2];
		
		qDebug()<<"sxyx ..."<<sx<<sy<<sz;
		
		//****************************************************************
		// suppose compressed image saved as .tif
		
		QString m_FileName_compressed = m_FileName;
		m_FileName_compressed.chop(3); // ".tc"
		m_FileName_compressed.append(".raw");
		
		//m_FileName_compressed.append(".raw");
		// loading compressed image files
		sz_compressed = 0; 
		
		int datatype_compressed = 0;
		
		compressed1d = 0;
		
		//imgData->loadImage(openFileNameLabel.toAscii().data());
		
		if(loadImage(const_cast<char *>(m_FileName_compressed.toStdString().c_str()), compressed1d, sz_compressed, datatype_compressed)!= true)
		{
			QMessageBox::information(0, "Load Image", QObject::tr("File load failure"));
			return;
		}//careful
		
		cx=sz_compressed[0], cy=sz_compressed[1], cz=sz_compressed[2], cc=sz_compressed[3];
		
		channel_compressed_sz = cx*cy*cz;
		
		init_x = cx/2, init_y = cy/2, init_z = cz/2; 
		
		if(datatype_compressed ==1 )
		{
			dtype = V3D_UINT8;
		}
		else if(datatype_compressed == 2)
		{
			dtype = V3D_UINT16;
		}
		else if(datatype_compressed==4)
		{
			dtype = V3D_FLOAT32;
		}
		
		qDebug()<<"compressedsxyx ..."<<cx<<cy<<cz;			
		
		setCTypeBasedOnImageData();
		
		updateminmaxvalues();
		
		createGUI();
		
		scaleFactorInput = int(sy/cy);
		
		qDebug()<<"scaleFactorInput ..."<<scaleFactorInput;	
		
		xy_view->Setwidget(callback, m_FileName, curFilePath, scaleFactorInput);
		yz_view->Setwidget(callback, m_FileName, curFilePath, scaleFactorInput);
		zx_view->Setwidget(callback, m_FileName, curFilePath, scaleFactorInput);
	}
}
void ImageSetWidget::update_triview()
{
	cur_x = xValueSpinBox->text().toInt(); // / scaleFactor;
	cur_y = yValueSpinBox->text().toInt(); // / scaleFactor;
	cur_z = zValueSpinBox->text().toInt(); // / scaleFactor;
	
	xy_view->b_creadWindow = bcreadViews;
	
	yz_view->b_creadWindow = bcreadViews;
	
	zx_view->b_creadWindow = bcreadViews;
	
	xy_view->setImgData(imgPlaneZ,dtype,Ctype,sz_compressed,cur_x,cur_y,cur_z, compressed1d,p_vmax,p_vmin);
	
	yz_view->setImgData(imgPlaneX,dtype,Ctype,sz_compressed,cur_x,cur_y,cur_z,compressed1d,p_vmax,p_vmin);
	
	zx_view->setImgData(imgPlaneY,dtype,Ctype,sz_compressed,cur_x,cur_y,cur_z,compressed1d,p_vmax,p_vmin);
		
}

template <class T> QPixmap copyRaw2QPixmap(const T * pdata, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, V3DLONG sz3,ImageDisplayColorType Ctype, 
										   V3DLONG cz0, V3DLONG cz1, V3DLONG cz2,ImagePlaneDisplayType disType, 
										   double *p_vmax, double *p_vmin)
{
	switch (disType)
	{
		case imgPlaneX:
			return copyRaw2QPixmap_xPlanes(pdata, sz0, sz1, sz2, sz3, Ctype,cz0,cz1,cz2,p_vmax, p_vmin);
			break;
			
		case imgPlaneY:
			return copyRaw2QPixmap_yPlanes(pdata, sz0, sz1, sz2, sz3,Ctype,cz0,cz1,cz2, p_vmax, p_vmin);
			break;
			
		case imgPlaneZ:
			return copyRaw2QPixmap_zPlanes(pdata, sz0, sz1, sz2, sz3,Ctype,cz0,cz1,cz2, p_vmax, p_vmin);
			break;
			
		default:
			printf("Undefined ImagePlaneDisplayType. Check your code.\n");
			return QPixmap(0,0); //return an empty image for this prohibited case
			break;
	}
}

void XMapView::mousePressEvent(QMouseEvent *e)
{
	switch (e->button())
	{
		case Qt::LeftButton:
			mouseLeftButtonPressEvent(e);
			break;
		case Qt::RightButton:
			mouseRightButtonPressEvent(e);
		default:
			break;
	}
}
void XMapView::mouseReleaseEvent(QMouseEvent * e)
{
	//flag_syn = false;
	
//	if (QApplication::keyboardModifiers()==Qt::ControlModifier)
	if (bMouseCurorIn && b_mousmove)
	{
		dragEndPosition = e->pos();
		b_mouseend = false;
		b_mousmove = false;
		flag_syn = true;
		setCursor(Qt::ArrowCursor);
		end_x = dragEndPosition.x();
		end_y = dragEndPosition.y();
		end_z = cz ;
		long in_startx = (start_x < end_x)? start_x:end_x;
		long in_starty = (start_y < end_y)? start_y:end_y;
		
		long in_endx = (end_x > start_x)? end_x:start_x;
		long in_endy = (end_y > start_y)? end_y:start_y;
		
		mousenumber++;
		switch(Ptype)
		{
			case imgPlaneZ://xoy
				
				update_v3dviews(callback1, in_startx*scaleFactor, in_starty*scaleFactor, start_z*scaleFactor,in_endx*scaleFactor, in_endy*scaleFactor, end_z*scaleFactor);				
								
				break;
				
			case imgPlaneX://zoy
				
				update_v3dviews(callback1, cur_x*scaleFactor, in_starty*scaleFactor,in_startx*scaleFactor,cx*scaleFactor, in_endy*scaleFactor,in_endx*scaleFactor);
				break;
				
			case imgPlaneY://xoz
				
				update_v3dviews(callback1, in_startx*scaleFactor, cur_y*scaleFactor, in_starty*scaleFactor,in_endx*scaleFactor, cy*scaleFactor, in_endy*scaleFactor);		
				
				break;
				
			default:
				return;
				break;
		}
	}	
//	 if (QApplication::keyboardModifiers()==Qt::ControlModifier)
//	 {
//		dragEndPosition = e->pos();
//		b_mouseend = false;
//		setCursor(Qt::ArrowCursor);
//	 }
}
void XMapView::mouseRightButtonPressEvent(QMouseEvent *e)
{
	//flag_syn = true;
//	if (QApplication::keyboardModifiers()==Qt::ControlModifier)
//	{
//		dragEndPosition = e->pos();
//		b_mouseend = false;
//		setCursor(Qt::ArrowCursor);
//		flag_syn = true;
//		end_x = dragEndPosition.x();
//		end_y = dragEndPosition.y();
//		end_z = cz ;
//		long tem;
//		if (start_x > end_x)
//		{
//			tem =end_x;
//			end_x = start_x;
//			end_x = tem ;
//		}
//		if (start_y > end_y)
//		{
//			tem =end_y;
//			end_y = start_y;
//			start_y = tem;
//		}
//		
//		update_v3dviews(callback1, start_x*scaleFactor, start_y*scaleFactor, start_z*scaleFactor,end_x*scaleFactor, end_y*scaleFactor, end_z*scaleFactor);
//		
		//qDebug()<<"rrrexyz...."<<end_x<<end_y<<end_z;
		//update_v3dviews(mapCallback, dragStartPosition.x(),dragStartPosition.y(),cur_z, dragEndPosition.x(), dragEndPosition.y(),cz);
		
	//}
}
void XMapView::mouseLeftButtonPressEvent(QMouseEvent *e) //080101
{
	//flag_syn = false;
	
	if (bMouseCurorIn )//&& QApplication::keyboardModifiers()==Qt::ControlModifier)
	{
		b_mouseend = true;
		dragStartPosition = e->pos();
		
		start_x = dragStartPosition.x();
		start_y = dragStartPosition.y();
		start_z = cur_z;
		setCursor(Qt::CrossCursor);
		//setCursor(myCursor);		
		update();
		
		//qDebug()<<"LLLLsxyz...."<<start_x<<start_y<<start_z;
	}
}

void XMapView::mouseMoveEvent (QMouseEvent * e)
{
	//curMousePos = e->pos()/disp_scale;
	
//	if (bMouseCurorIn && QApplication::keyboardModifiers()==Qt::ControlModifier && b_mousmove) 
//	{
//		curMousePos = e->pos();
//		start_x = curMousePos.x() - (in_endx - in_startx)/2;
//		end_x = curMousePos.x() + (in_endx - in_startx)/2;
//		start_y = curMousePos.y() - (in_endy - in_starty)/2;
//		end_y = curMousePos.y() + (in_endy - in_starty)/2;
//		
//	}else if (bMouseCurorIn)
	{
		b_mousmove = true;
		curMousePos = e->pos();
		//dragEndPosition= e->pos();
		end_x = curMousePos.x();
		end_y = curMousePos.y();
		end_z = cz ;
		
	}
	update();
}

void XMapView::enterEvent (QEvent * e)
{
	bMouseCurorIn = true;
}

void XMapView::leaveEvent (QEvent * e)
{
	//bMouseCurorIn = false;
	//update();
}

void XMapView::drawROI(QPainter *painter)
{
	if (bMouseCurorIn && b_mouseend)
	{
		painter->setPen(Qt::white );
		painter->setBrush(Qt::NoBrush);
		
		QRect r( dragStartPosition, curMousePos );
		painter->drawRect( r );
		//painter->drawRect(QRect(dragStartPosition.x(),dragStartPosition.y(),curMousePos.x(),curMousePos.y());
	}

}
void XMapView::update_v3dviews(V3DPluginCallback *callback, long start_x, long start_y, long start_z, long end_x, long end_y, long end_z)
{
	long vx, vy, vz, vc;
	
	long tx = 0;
	long ty = 0;
	long tz = 0;
    
	qDebug()<<"start end x y z "<< start_x << start_y << start_z << end_x << end_y << end_z;
   
	qDebug()<<"vim x y z "<< vim.sz[0] <<vim.sz[1] << vim.sz[2] ;
	
	switch(Ptype)
	{
		case imgPlaneZ://xoy
			
			tz = start_z;
			
			start_z = tz - 10;
			
			end_z = tz + 10;
			
			if (start_z < 0)
			{
				start_z = 0;
			}
			if (end_z > vim.sz[2]) 
			{
				end_z=vim.sz[2];
				
			}
			break;
			
		case imgPlaneX://zoy
			
			tx = start_x;
			
			start_x = tx - 10;
			
			end_x = tx + 10;
			
			if (start_x < 0)
			{
				start_x = 0;
			}
			if (end_x > vim.sz[0]) 
			{
				end_x = vim.sz[0];
				
			}
			break;
			
		case imgPlaneY://xoz
			
			ty = start_y;
			
			start_y = ty - 10;
			
			end_y = ty + 10;
			
			if (start_y < 0)
			{
				start_y = 0;
			}
			if (end_y > vim.sz[1]) 
			{
				end_y = vim.sz[1];
				
			}			
			break;
			
		default:
			return;
			break;
	}
	
	
	size_t start_t = clock();

	
	vx = end_x - start_x ;//+ 1; // suppose the size same of all tiles
	vy = end_y - start_y ;//+ 1;
	vz = end_z - start_z ;//+ 1;
	vc = vim.sz[3];
	
	printf("vx=%ld vy=%ld vz=%ld vc=%ld\n",vx,vy,vz,vc);
	
	long pagesz_vim = vx*vy*vz*vc;
    
	void * pData = NULL;
	
	switch (Datatype)
	{
		case V3D_UINT8:
			try
		{
			pData  = new unsigned char [pagesz_vim];
			
			memset(pData, 0, sizeof(unsigned char)*pagesz_vim);
		}
			catch (...) 
		{
			printf("Fail to allocate memory.\n");
			return ;
		}
			break;
			
		case V3D_UINT16:
			try
		{
			pData = new unsigned short [pagesz_vim]; 
			memset(pData, 0, sizeof(unsigned short)*pagesz_vim);
		}
			catch (...)
		{
			printf("Fail to allocate memory in data combination.");
			if (pData) {delete []pData; pData=0;}
			return;
		}
			break;
			
		case V3D_FLOAT32:
			try
		{
			pData = new float [pagesz_vim];
			memset(pData, 0, sizeof(float)*pagesz_vim);
		}
			catch (...)
		{
			printf("Fail to allocate memory in data combination.");
			if (pData) {delete []pData; pData=0;}
			return;
		}
			break;
			
		default:
			printf("Right now only support UINT8, UINT16, and FLOAT32.\n", 0);
			break;
	}
	
		
	if(vim.number_tiles == 1)
	{
		cout << "satisfied image: "<< vim.lut[0].fn_img << endl;
		
		//char * curFileSuffix = getSurfix(const_cast<char *>(vim.lut[0].fn_img.c_str()));
		
		//cout << "suffix ... " << curFileSuffix << endl; // 
		
		QString curPath = curFilePath;
		
		string fn = curPath.append( QString(vim.lut[0].fn_img.c_str()) ).toStdString();
		
		qDebug()<<"testing..."<<curFilePath<< fn.c_str();
		
		char * imgSrcFile = const_cast<char *>(fn.c_str());
		
		// loading relative imagg files
		V3DLONG *sz_relative = 0; 
		V3DLONG *szo = 0; 
		int datatype_relative = 0;
		unsigned char* relative1d = 0;
		
		if(loadImage(imgSrcFile,relative1d,sz_relative,szo,start_x,start_y,start_z,end_x,end_y,end_z,datatype_relative) != true)
		{
			QMessageBox::information(0, "Load Image", QObject::tr("File load failure"));
			
			return;
			
		}
		//loadImage(imgSrcFile,relative1d,sz_relative,szo,(x_start-tile2vi_xs),(y_start-tile2vi_ys),(z_start-tile2vi_zs),(x_end-tile2vi_xs),(y_end-tile2vi_ys),(z_end-tile2vi_zs),datatype_relative);

		vx = szo[0];
		vy = szo[1];
		vz = szo[2];
		vc = szo[3];
		switch (Datatype)
		{
			case V3D_UINT8:
				(unsigned char*)pData = (unsigned char*)relative1d;
				//CopyData((unsigned char*)relative1d, (unsigned char*)pData, vx,vy,vz,vc);
				break;
				
			case V3D_UINT16:
				(unsigned short*)pData = (unsigned short*)relative1d;
				//CopyData((unsigned short*)relative1d, (unsigned short*)pData, vx,vy,vz,vc);
				 break;
				
			case V3D_FLOAT32:
				(float*)pData = (float*)relative1d;
				//CopyData((float*)relative1d, (float*)pData, vx,vy,vz,vc);
				break;
			default:
				printf("Right now only support UINT8, UINT16, and FLOAT32.\n", 0);
				break;
		}
	}else
	{
		bitset<3> lut_ss, lut_se, lut_es, lut_ee;
		
		long x_s = start_x + vim.min_vim[0];
		long y_s = start_y + vim.min_vim[1];
		long z_s = start_z + vim.min_vim[2];
		
		long x_e = end_x + vim.min_vim[0];
		long y_e = end_y + vim.min_vim[1];
		long z_e = end_z + vim.min_vim[2];
		
		for(long ii=0; ii<vim.number_tiles; ii++)
		{	
			// init
			lut_ss.reset();
			lut_se.reset();
			lut_es.reset();
			lut_ee.reset();
			//
			if(x_s < vim.lut[ii].start_pos[0]) lut_ss[1] = 1; // r  0 l
			if(y_s < vim.lut[ii].start_pos[1]) lut_ss[0] = 1; // d  0 u
			if(z_s < vim.lut[ii].start_pos[2]) lut_ss[2] = 1; // b  0 f
			
			if(x_e < vim.lut[ii].start_pos[0]) lut_se[1] = 1; // r  0 l
			if(y_e < vim.lut[ii].start_pos[1]) lut_se[0] = 1; // d  0 u
			if(z_e < vim.lut[ii].start_pos[2]) lut_se[2] = 1; // b  0 f
			
			if(x_s < vim.lut[ii].end_pos[0]) lut_es[1] = 1; // r  0 l
			if(y_s < vim.lut[ii].end_pos[1]) lut_es[0] = 1; // d  0 u
			if(z_s < vim.lut[ii].end_pos[2]) lut_es[2] = 1; // b  0 f
			
			if(x_e < vim.lut[ii].end_pos[0]) lut_ee[1] = 1; // r  0 l
			if(y_e < vim.lut[ii].end_pos[1]) lut_ee[0] = 1; // d  0 u
			if(z_e < vim.lut[ii].end_pos[2]) lut_ee[2] = 1; // b  0 f
			
			// copy data
			if( (!lut_ss.any() && lut_ee.any()) || (lut_es.any() && !lut_ee.any()) || (lut_ss.any() && !lut_se.any()) )
			{
				// 
				cout << "satisfied image: "<< vim.lut[ii].fn_img << endl;
				
				char * curFileSuffix = getSurfix(const_cast<char *>(vim.lut[ii].fn_img.c_str()));
				
				cout << "suffix ... " << curFileSuffix << endl; // 
				
				QString curPath = curFilePath;
				
				string fn = curPath.append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();
				
				qDebug()<<"testing..."<<curFilePath<< fn.c_str();
				
				//
				char * imgSrcFile = const_cast<char *>(fn.c_str());
				
				size_t s1_t = clock();
				
				long tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0]; 
				long tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0]; 
				long tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1]; 
				long tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1]; 
				long tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2]; 
				long tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2]; 
				
				long x_start = (start_x > tile2vi_xs) ? start_x : tile2vi_xs; 
				long x_end = (end_x < tile2vi_xe) ? end_x : tile2vi_xe;
				long y_start = (start_y > tile2vi_ys) ? start_y : tile2vi_ys;
				long y_end = (end_y < tile2vi_ye) ? end_y : tile2vi_ye;
				long z_start = (start_z > tile2vi_zs) ? start_z : tile2vi_zs;
				long z_end = (end_z < tile2vi_ze) ? end_z : tile2vi_ze;
				
				//x_end++;
				//	y_end++;
				//z_end++;
				
				// loading relative imagg files
				V3DLONG *sz_relative = 0; 
				int datatype_relative = 0;
				unsigned char* relative1d = 0;
				V3DLONG *szo=0;
				
				if (x_end > x_start && y_end > y_start && z_end > z_start) 
				{
					if(loadImage(imgSrcFile,relative1d,sz_relative,szo,(x_start-tile2vi_xs),(y_start-tile2vi_ys),(z_start-tile2vi_zs),(x_end-tile2vi_xs),(y_end-tile2vi_ys),(z_end-tile2vi_zs),datatype_relative)!=true)
					{
						QMessageBox::information(0, "Load Image", QObject::tr("File load failure"));
						return;
					}
					
					switch (Datatype)
					{
						case V3D_UINT8:
								
							CopyData_tc((unsigned char*)relative1d,(unsigned char*)pData,szo,start_x,start_y,start_z,
										  x_start,y_start,z_start,x_end,y_end,z_end,vx,vy,vz);	
							break;
							
						case V3D_UINT16:
							
							CopyData_tc((unsigned short*)relative1d,(unsigned short*)pData,szo,start_x,start_y, start_z,
										x_start,y_start,z_start,x_end,y_end,z_end,vx,vy,vz);
							break;
							
						case V3D_FLOAT32:
							
							CopyData_tc((float*)relative1d,(float*)pData,szo,start_x,start_y, start_z,
										x_start,y_start,z_start,x_end,y_end,z_end,vx,vy,vz);
							break;
						default:
							printf("Right now only support UINT8, UINT16, and FLOAT32.\n", 0);
							break;
					}
				}
				if(sz_relative) {delete []sz_relative; sz_relative=0;}
				if(relative1d) {delete []relative1d; relative1d=0;}
			}
		}
	}
	size_t end1_t = clock();
	
	cout<<"time elapse ... "<<end1_t-start_t<<endl;
	
	Image4DSimple p4DImage;
	
	//p4DImage.setData((unsigned char*)relative1d, rx, ry, rz, rc, V3D_UINT8);
	
	p4DImage.setData((unsigned char*)pData,vx,vy,vz,vc,Datatype);
	
	v3dhandle curwin;
	
	QString curImageName;
	
	QString file ;
	
	if (mousenumber ==1)
	{
		curImageName = curFilePath + "Map Image 1"; 
		curwin = callback->newImageWindow();
		callback->setImage(curwin, &p4DImage);
		callback->setImageName(curwin, curImageName);
		callback->updateImageWindow(curwin);
		callback->pushImageIn3DWindow(curwin);
		
	}else 
	{	
		if(!b_creadWindow)
		{
			curwin = callback->currentImageWindow();
			callback->setImage(curwin, &p4DImage);
            QString curImageName2 = callback->getImageName(curwin);
			callback->setImageName(curwin, curImageName2);
			callback->updateImageWindow(curwin);
			callback->pushImageIn3DWindow(curwin);
			mousenumber--;
			if (mousenumber < 0) 
			{
				mousenumber = 1;
			}
		}
		else
		{
			file.sprintf("%d",mousenumber);
			QString curImageName1 = curFilePath + "Map Image" + file;
			curwin = callback->newImageWindow();
			callback->setImage(curwin, &p4DImage);
			callback->setImageName(curwin, curImageName1);
			callback->updateImageWindow(curwin);
			callback->pushImageIn3DWindow(curwin);
		}
		
	}
	
	size_t end_t = clock();
	
	cout<<"time elapse after loading configuration info ... "<<end_t-start_t<<endl;
}

void XMapView::Setwidget(V3DPluginCallback &callback, QString m_FileName, QString curFilePathInput, float scaleFactorInput)
{
	callback1 = &callback;
	
	curFilePath = curFilePathInput;
	
	string filename = m_FileName.toStdString();
	
	//qDebug()<<"filename ..."<<filename.c_str();
	
	if(vim.y_load(filename)!= true)
	{
		QMessageBox::information(0, "TC file reading", QObject::tr("tc file is illegal"));
		return ;
	}
	
	scaleFactor = (int)scaleFactorInput;
	
}



