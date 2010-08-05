/* miviewer.cpp
 * 2010-07-28: the program is created by Yang Yu
 */

#include "miviewer.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(miviewer, MIViewerPlugin);

// pairwise stitching function
//int focusControl(unsigned char *pData, IndexedData idata);
int iViewer(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "Microscopic Image Viewer";
QStringList MIViewerPlugin::menulist() const
{
    return QStringList() << tr("Microscopic Image Viewer")
						 << tr("about this plugin");
}

void MIViewerPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == tr("Microscopic Image Viewer"))
    {
		//initGUI(parent);
    	iViewer(callback, parent);
    }
	else if (menu_name == tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "MIViewer Plugin 1.0 (July 28, 2010) developed by Yang Yu. (Peng Lab, Janelia Research Farm Campus, HHMI)");
		return;
	}
}

//
int iViewer(V3DPluginCallback &callback, QWidget *parent)
{
	// load indexed data file
	// get filename
	QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open A Virtual Image"),
													  "/Documents",
													  QObject::tr("Image Configuration (*)"));
	if(m_FileName.isEmpty())
		return -1;
	
	size_t start_t = clock();
	
	QString curFilePath = QFileInfo(m_FileName).path();
	curFilePath.append("/");
	
//	QString suffix = QFileInfo(m_FileName).suffix();
//	
//	cout<<"suffix is ..."<< suffix.toStdString().c_str()<<endl;
	
	string filename = m_FileName.toStdString();
	
	// virtual image
	Y_VIM<float, long, indexed_t<long, float>, LUT<long> > vim;
	
	vim.y_load(filename);
	
	size_t end_t = clock();
	
	cout<<"time elapse after loading configuration info ... "<<end_t-start_t<<endl;
	
	
	// compress image to make it fit in control window	
	// showing initial tri-view of control window
	float scaleFactor = 10;
	
	ImageNavigatingWidget *inw = new ImageNavigatingWidget(vim.sz[0], vim.sz[1], vim.sz[2], m_FileName, scaleFactor);
	inw->show();
	
	
	//qDebug()<<"test inw ..."<< inw->roi_start_x << inw->roi_start_y << inw-> roi_end_x << inw->roi_end_y;
	
//	// visualize in v3d tri-view
//	// input virtual image point position
//	long start[3], end[3];
//	
//	// passing from control window
//	// ****************************
//	//
//	
//	start[0] = 1;
//	start[1] = 1;
//	start[2] = 1;
//	
//	end[0] = 512;
//	end[1] = 512;
//	end[2] = 32;
//	
//	// ****************************
//	
//	
//	//virtual image
//	long vx, vy, vz, vc;
//	
//	vx = end[0] - start[0] + 1; // suppose the size same of all tiles
//	vy = end[1] - start[1] + 1;
//	vz = end[2] - start[2] + 1;
//	vc = vim.sz[3];
//	
//	long pagesz_vim = vx*vy*vz*vc;
//	
//	unsigned char *pVImg = 0;
//	
//	try
//	{
//		pVImg = new unsigned char [pagesz_vim];
//	}
//	catch (...) 
//	{
//		printf("Fail to allocate memory.\n");
//		return -1;
//	}
//	
//	// init
//	for(long i=0; i<pagesz_vim; i++)
//	{
//		pVImg[i] = 0;
//	}
//	
//	// flu bird algorithm
//	// 000 'f', 'l', 'u' ; 111 'b', 'r', 'd'; relative[2] relative[1] relative[0] 
//	bitset<3> lut_ss, lut_se, lut_es, lut_ee;
//	
//	// 
//	long x_s = start[0] + vim.min_vim[0];
//	long y_s = start[1] + vim.min_vim[1];
//	long z_s = start[2] + vim.min_vim[2];
//	
//	long x_e = end[0] + vim.min_vim[0];
//	long y_e = end[1] + vim.min_vim[1];
//	long z_e = end[2] + vim.min_vim[2];
//	
//	//
//	ImagePixelType datatype;
//	
//	// look up lut
//	for(long ii=0; ii<vim.number_tiles; ii++)
//	{	
//		// init
//		lut_ss.reset();
//		lut_se.reset();
//		lut_es.reset();
//		lut_ee.reset();
//		
//		//
//		if(x_s < vim.lut[ii].start_pos[0]) lut_ss[1] = 1; // r  0 l
//		if(y_s < vim.lut[ii].start_pos[1]) lut_ss[0] = 1; // d  0 u
//		if(z_s < vim.lut[ii].start_pos[2]) lut_ss[2] = 1; // b  0 f
//		
//		if(x_e < vim.lut[ii].start_pos[0]) lut_se[1] = 1; // r  0 l
//		if(y_e < vim.lut[ii].start_pos[1]) lut_se[0] = 1; // d  0 u
//		if(z_e < vim.lut[ii].start_pos[2]) lut_se[2] = 1; // b  0 f
//		
//		if(x_s < vim.lut[ii].end_pos[0]) lut_es[1] = 1; // r  0 l
//		if(y_s < vim.lut[ii].end_pos[1]) lut_es[0] = 1; // d  0 u
//		if(z_s < vim.lut[ii].end_pos[2]) lut_es[2] = 1; // b  0 f
//		
//		if(x_e < vim.lut[ii].end_pos[0]) lut_ee[1] = 1; // r  0 l
//		if(y_e < vim.lut[ii].end_pos[1]) lut_ee[0] = 1; // d  0 u
//		if(z_e < vim.lut[ii].end_pos[2]) lut_ee[2] = 1; // b  0 f
//		
//		// copy data
//		if( (!lut_ss.any() && lut_ee.any()) || (lut_es.any() && !lut_ee.any()) || (lut_ss.any() && !lut_se.any()) )
//		{
//			// 
//			cout << "satisfied image: "<< vim.lut[ii].fn_img << endl;
//			
//			//
//			char * curFileSuffix = getSurfix(const_cast<char *>(vim.lut[ii].fn_img.c_str()));
//			
//			cout << "suffix ... " << curFileSuffix << endl; // tif lsm
//			
//			QString curPath = curFilePath;
//			
//			string fn = curPath.append( QString(vim.lut[ii].fn_img.c_str()) ).toStdString();
//			
//			qDebug()<<"testing..."<<curFilePath<< fn.c_str();
//			
//			//
//			char * imgSrcFile = const_cast<char *>(fn.c_str());
//			
//			Stack *tmpstack = NULL;
//			
//			size_t s1_t = clock();
//			
//			if (strcasecmp(curFileSuffix, "tif")==0 || strcasecmp(curFileSuffix, "tiff")==0) //read tiff stacks
//			{
//				tmpstack = Read_Stack(imgSrcFile);
//			}
//			else if ( strcasecmp(curFileSuffix, "lsm")==0 ) //read lsm stacks
//			{
//				tmpstack = Read_LSM_Stack(imgSrcFile);
//			}
//			
//			size_t e1_t = clock();
//			cout<<"time elapse for read tmpstack ... "<<e1_t-s1_t<<endl;
//			
//			// loading relative imagg files
//			V3DLONG *sz_relative = 0; 
//			int datatype_relative = 0;
//			unsigned char* relative1d = 0;
//			
//			//
//			try
//			{
//				sz_relative = new V3DLONG [4];
//			
//				sz_relative[0] = tmpstack->width;
//				sz_relative[1] = tmpstack->height;
//				sz_relative[2] = tmpstack->depth;
//				switch (tmpstack->kind)
//				{
//					case GREY:
//						sz_relative[3] = 1;
//						datatype_relative = 1;
//						datatype = V3D_UINT8;
//						break;
//						
//					case GREY16:
//						sz_relative[3] = 1;
//						datatype_relative = 2;
//						datatype = V3D_UINT16;
//						break;
//						
//					case COLOR:
//						sz_relative[3] = 3;
//						datatype_relative = 1;
//						datatype = V3D_FLOAT32;
//						break;
//						
//					default:
//						printf("The type of tif file is not supported in this version.\n");
//						if (sz_relative) {delete sz_relative; sz_relative=0;}
//						Kill_Stack(tmpstack); tmpstack=0;
//						break;
//				}
//			}
//			catch(...)
//			{
//				cout<<"Error happens in assigning size and datatype."<<endl;
//				return false;
//			}
//			
//			long rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];
//			
//			
//			// ***********************************************
//			//
////			if (loadImage(const_cast<char *>(fn.c_str()), relative1d, sz_relative, datatype_relative)!=true)
////			{
////				fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
////				return -1;
////			}
////			long rx=sz_relative[0], ry=sz_relative[1], rz=sz_relative[2], rc=sz_relative[3];
////			
////			if(datatype_relative==1)
////				datatype = V3D_UINT8;
//			
//			// ************************************************
//			
//			//
//			long tile2vi_xs = vim.lut[ii].start_pos[0]-vim.min_vim[0]; 
//			long tile2vi_xe = vim.lut[ii].end_pos[0]-vim.min_vim[0]; 
//			long tile2vi_ys = vim.lut[ii].start_pos[1]-vim.min_vim[1]; 
//			long tile2vi_ye = vim.lut[ii].end_pos[1]-vim.min_vim[1]; 
//			long tile2vi_zs = vim.lut[ii].start_pos[2]-vim.min_vim[2]; 
//			long tile2vi_ze = vim.lut[ii].end_pos[2]-vim.min_vim[2]; 
//			
//			long x_start = (start[0] > tile2vi_xs) ? start[0] : tile2vi_xs; 
//			long x_end = (end[0] < tile2vi_xe) ? end[0] : tile2vi_xe;
//			long y_start = (start[1] > tile2vi_ys) ? start[1] : tile2vi_ys;
//			long y_end = (end[1] < tile2vi_ye) ? end[1] : tile2vi_ye;
//			long z_start = (start[2] > tile2vi_zs) ? start[2] : tile2vi_zs;
//			long z_end = (end[2] < tile2vi_ze) ? end[2] : tile2vi_ze;
//			
//			x_end++;
//			y_end++;
//			z_end++;
//			
//			//
//			cout << x_start << " " << x_end << " " << y_start << " " << y_end << " " << z_start << " " << z_end << endl;
//			
//			//
//			// loadTiledimage(filename, pointer, sz, start, end, datatype)
//			//
//		
//			//
//			for(long c=0; c<rc; c++)
//			{
//				long o_c = c*vx*vy*vz;
//				long o_r_c = c*rx*ry*rz;
//				for(long k=z_start; k<z_end; k++)
//				{
//					long o_k = o_c + (k-start[2])*vx*vy;
//					long o_r_k = o_r_c + (k-z_start)*rx*ry;
//					
//					for(long j=y_start; j<y_end; j++)
//					{
//						long o_j = o_k + (j-start[1])*vx;
//						long o_r_j = o_r_k + (j-y_start)*rx;
//						for(long i=x_start; i<x_end; i++)
//						{
//							long idx = o_j + i-start[0];
//							long idx_r = o_r_j + (i-x_start);
//							
//							if(pVImg[idx]>0)
//							{
//								//pVImg[idx] = (pVImg[idx]>relative1d[idx_r])?pVImg[idx]:relative1d[idx_r];
//								
//								unsigned char tmpv = Get_Stack_Pixel(tmpstack,i,j,k,c);
//								
//								pVImg[idx] = (pVImg[idx]>tmpv)?pVImg[idx]:tmpv;
//							}
//							else
//							{
//								//pVImg[idx] = relative1d[idx_r];
//								
//								pVImg[idx] = Get_Stack_Pixel(tmpstack,i,j,k,c);
//							}
//						}
//					}
//				}
//			}
//			
//			//de-alloc
//			if(relative1d) {delete []relative1d; relative1d=0;}
//			if(sz_relative) {delete []sz_relative; sz_relative=0;}
//			
//			if (tmpstack)
//			{
//				Kill_Stack(tmpstack);
//				tmpstack=0;
//			}
//		}
//		
//	}
//	
//	size_t end1_t = clock();
//	
//	cout<<"time elapse ... "<<end1_t-start_t<<endl;
//	
//	//display
//	Image4DSimple p4DImage;
//	p4DImage.setData(pVImg, vx, vy, vz, vc, datatype);
//	
//	v3dhandle newwin = callback.newImageWindow();
//	callback.setImage(newwin, &p4DImage);
//	callback.setImageName(newwin, "ROI of A Virtual Image");
//	callback.updateImageWindow(newwin);
}


//
//int focusControl(unsigned char *pData, IndexedData idata)
//{
//	// control
//	
//	
//	// return pointer of ROI of virtual image
//	
//	
//	
//}


