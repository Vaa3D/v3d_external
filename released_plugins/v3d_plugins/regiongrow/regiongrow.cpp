/* regiongrow.cpp
 * adapted from Hanchuan's region growing code
 * 2010-04-02: create this program by Yang Yu
 */

#include <QtGui>

#include <cmath>
#include <stdlib.h>
#include <ctime>

#include <vector>

#include <sstream>
#include <iostream>

#include "regiongrow.h"

#include "../basic_c_fun/basic_surf_objs.h"
//#include "../basic_c_fun/stackutil.h"
//#include "../basic_c_fun/volimg_proc.h"
//#include "../basic_c_fun/img_definition.h"
#include "../basic_c_fun/basic_landmark.h"

//
//statistics of count of labeling
class STCL
{
public:
	STCL(){}
	~STCL(){}
	
public:
	int count;
	int label;
};

//
#define NO_OBJECT 0

// this is an extension to 3D region growing from octave 2D bwlabel code
static int find( int set[], int x )
{
    int r = x;
    while ( set[r] != r )
        r = set[r];
    return r;
}


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(regiongrow, RegionGrowPlugin);

void regiongrowing(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "Region Growing";
QStringList RegionGrowPlugin::menulist() const
{
    return QStringList() << tr("Region Growing");
}

void RegionGrowPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == tr("Region Growing"))
    {
    	regiongrowing(callback, parent);
    }
}

void regiongrowing(V3DPluginCallback &callback, QWidget *parent)
{
    v3dhandleList win_list = callback.getImageWindowList();
	
	if(win_list.size()<1) 
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	
	int start_t = clock(); // record time point
	
	int i1=0;
	
	Image4DSimple* subject = callback.getImage(win_list[i1]);
	ROIList pRoiList=callback.getROI(win_list[i1]);
	
	QString m_InputFileName = callback.getImageName(win_list[i1]);
	
	qDebug() << "name of image ..." << m_InputFileName;
	
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	if (subject->getDatatype()!=V3D_UINT8)
	{
		QMessageBox::information(0, title, QObject::tr("This demo program only supports 8-bit data. Your current image data type is not supported."));
		return;
	}
	
    unsigned char* pSbject = subject->getRawData();
	
	long sz0 = subject->getXDim();
    long sz1 = subject->getYDim();
    long sz2 = subject->getZDim();
	long sz3 = subject->getCDim();
	
	long pagesz_sub = sz0*sz1*sz2;
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------
	//finding the bounding box of ROI
	bool vxy=true,vyz=true,vzx=true; // 3 2d-views
	
	QRect b_xy = pRoiList.at(0).boundingRect();
	QRect b_yz = pRoiList.at(1).boundingRect();
	QRect b_zx = pRoiList.at(2).boundingRect();
	
	if(b_xy.left()==-1 || b_xy.top()==-1 || b_xy.right()==-1 || b_xy.bottom()==-1)
		vxy=false;
	if(b_yz.left()==-1 || b_yz.top()==-1 || b_yz.right()==-1 || b_yz.bottom()==-1)
		vyz=false;
	if(b_zx.left()==-1 || b_zx.top()==-1 || b_zx.right()==-1 || b_zx.bottom()==-1)
		vzx=false;
		
	long bpos_x, bpos_y, bpos_z, bpos_c, epos_x, epos_y, epos_z, epos_c;
	
	// 8 cases
	if(vxy && vyz && vzx) // all 3 2d-views
	{
		bpos_x = qBound(long(0), long(qMax(b_xy.left(), b_zx.left())), sz0-1);
		bpos_y = qBound(long(0), long(qMax(b_xy.top(),  b_yz.top())), sz1-1);
		bpos_z = qBound(long(0), long(qMax(b_yz.left(), b_zx.top())), sz2-1);
		
		epos_x = qBound(long(0), long(qMin(b_xy.right(), b_zx.right())), sz0-1);
		epos_y = qBound(long(0), long(qMin(b_xy.bottom(), b_yz.bottom())), sz1-1);
		epos_z = qBound(long(0), long(qMin(b_yz.right(), b_zx.bottom())), sz2-1);
	}
	else if(!vxy && vyz && vzx) // 2 of 3
	{
		bpos_x = qBound(long(0), long(qMax(0, b_zx.left())), sz0-1);
		bpos_y = qBound(long(0), long(qMax(0,  b_yz.top())), sz1-1);
		bpos_z = qBound(long(0), long(qMax(b_yz.left(), b_zx.top())), sz2-1);
		
		epos_x = qBound(long(0), long(fmin(sz0-1, b_zx.right())), sz0-1);
		epos_y = qBound(long(0), long(fmin(sz1-1, b_yz.bottom())), sz1-1);
		epos_z = qBound(long(0), long(qMin(b_yz.right(), b_zx.bottom())), sz2-1);
	}
	else if(vxy && !vyz && vzx)
	{
		bpos_x = qBound(long(0), long(qMax(b_xy.left(), b_zx.left())), sz0-1);
		bpos_y = qBound(long(0), long(qMax(b_xy.top(),  0)), sz1-1);
		bpos_z = qBound(long(0), long(qMax(0, b_zx.top())), sz2-1);
		
		epos_x = qBound(long(0), long(qMin(b_xy.right(), b_zx.right())), sz0-1);
		epos_y = qBound(long(0), long(fmin(b_xy.bottom(), sz1-1)), sz1-1);
		epos_z = qBound(long(0), long(fmin(sz2-1, b_zx.bottom())), sz2-1);
	}
	else if(vxy && vyz && !vzx)
	{
		bpos_x = qBound(long(0), long(qMax(b_xy.left(), 0)), sz0-1);
		bpos_y = qBound(long(0), long(qMax(b_xy.top(),  b_yz.top())), sz1-1);
		bpos_z = qBound(long(0), long(qMax(b_yz.left(), 0)), sz2-1);
		
		epos_x = qBound(long(0), long(fmin(b_xy.right(), sz0-1)), sz0-1);
		epos_y = qBound(long(0), long(qMin(b_xy.bottom(), b_yz.bottom())), sz1-1);
		epos_z = qBound(long(0), long(fmin(b_yz.right(), sz2-1)), sz2-1);
	}
	else if(vxy && !vyz && !vzx) // only 1 of 3
	{
		bpos_x = qBound(long(0), long(qMax(b_xy.left(), 0)), sz0-1);
		bpos_y = qBound(long(0), long(qMax(b_xy.top(),  0)), sz1-1);
		bpos_z = 0;
		
		epos_x = qBound(long(0), long(fmin(b_xy.right(), sz0-1)), sz0-1);
		epos_y = qBound(long(0), long(fmin(b_xy.bottom(), sz1-1)), sz1-1);
		epos_z = sz2-1;
	}
	else if(!vxy && vyz && !vzx)
	{
		bpos_x = 0;
		bpos_y = qBound(long(0), long(qMax(0,  b_yz.top())), sz1-1);
		bpos_z = qBound(long(0), long(qMax(b_yz.left(), 0)), sz2-1);
		
		epos_x = sz0-1;
		epos_y = qBound(long(0), long(fmin(sz1-1, b_yz.bottom())), sz1-1);
		epos_z = qBound(long(0), long(fmin(b_yz.right(), sz2-1)), sz2-1);
	}
	else if(!vxy && !vyz && vzx)
	{
		bpos_x = qBound(long(0), long(qMax(0, b_zx.left())), sz0-1);
		bpos_y = 0;
		bpos_z = qBound(long(0), long(qMax(0, b_zx.top())), sz2-1);
		
		epos_x = qBound(long(0), long(fmin(sz0-1, b_zx.right())), sz0-1);
		epos_y = sz1-1;
		epos_z = qBound(long(0), long(fmin(sz2-1, b_zx.bottom())), sz2-1);
	}
	else // 0
	{
		bpos_x = 0;
		bpos_y = 0;
		bpos_z = 0;
		
		epos_x = sz0-1;
		epos_y = sz1-1;
		epos_z = sz2-1;
	}

	//qDebug("x %d y %d z %d x %d y %d z %d ",bpos_x,bpos_y,bpos_z,epos_x,epos_y,epos_z);

	//ROI extraction
	long sx = (epos_x-bpos_x)+1;
    long sy = (epos_y-bpos_y)+1;
    long sz = (epos_z-bpos_z)+1;
	long sc = sz3; // 0,1,2
	
	//choose the channel stack
	long pagesz = sx*sy*sz;
	
	double meanv=0, stdv=0;
	
	long offset_sub = 0;
	
	//------------------------------------------------------------------------------------------------------------------------------------
	
	unsigned char *data1d = new unsigned char [pagesz];
	if (!data1d) 
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	else
	{
		for(long k=bpos_z; k<=epos_z; k++)
		{
			long offset_z = k*sz0*sz1;
			long offset_crop_z = (k-bpos_z)*sx*sy;
			for(long j=bpos_y; j<=epos_y; j++)
			{
				long offset_y = j*sz0 + offset_z;
				long offset_crop_y = (j-bpos_y)*sx + offset_crop_z;
				for(long i=bpos_x; i<=epos_x; i++)
				{
					data1d[(i-bpos_x) + offset_crop_y] = pSbject[offset_sub + i+offset_y];
					
					meanv += data1d[(i-bpos_x) + offset_crop_y];
				}
			}
		}
	}
	meanv /= pagesz;
	
	for(long i=0; i<pagesz; i++)
		stdv += (data1d[i] - meanv)*(data1d[i] - meanv);
	
	stdv /= (pagesz-1);
	stdv = sqrt(stdv);
	
	qDebug() << "meanv ..." << meanv << "stdv ..." << stdv;
	
	//----------------------------------------------------------------------------------------------------------------------------------
	
	// de-alloc
	//if (pSbject) {delete []pSbject; pSbject=0;} // image visualized in v3d now
	
	// bw
	unsigned char *bw = new unsigned char [pagesz];
	unsigned int *L = new unsigned int [pagesz];
	
	for(long i=0; i<pagesz; i++)
	{
		bw[i] = (data1d[i]>meanv)?1:0;
		L[i] = 0;
	}
	
	//
	system("date");
	
	//
	long offset_y, offset_z;
	
	offset_y=sx;
	offset_z=sx*sy;
	
	long neighborhood_13[13] = {-1, -offset_y, -offset_z,
								-offset_y-1, -offset_y-offset_z, 
								offset_y-1, offset_y-offset_z,
								offset_z-1, -offset_z-1,
								-1-offset_y-offset_z, -1+offset_y-offset_z,
								1-offset_y-offset_z, 1+offset_y-offset_z}; 
	
	// other variables
    int *lset = new int [pagesz];   // label table/tree
    int ntable;                     // number of elements in the component table/tree
    
    ntable = 0;
    lset[0] = 0;
	
	for(long k = 0; k < sz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < sy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				
				if(i==0 || i==sx-1 || j==0 || j==sy-1 || k==0 || k==sz-1)
					continue;
				
				// find connected components
				if(bw[idx]) // if there is an object 
				{
					int n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13;
					
					n1 = find(lset, (int)L[idx + neighborhood_13[0] ]);
					n2 = find(lset, (int)L[idx + neighborhood_13[1] ]);
					n3 = find(lset, (int)L[idx + neighborhood_13[2] ]);
					n4 = find(lset, (int)L[idx + neighborhood_13[3] ]);
					n5 = find(lset, (int)L[idx + neighborhood_13[4] ]);
					n6 = find(lset, (int)L[idx + neighborhood_13[5] ]);
					n7 = find(lset, (int)L[idx + neighborhood_13[6] ]);
					n8 = find(lset, (int)L[idx + neighborhood_13[7] ]);
					n9 = find(lset, (int)L[idx + neighborhood_13[8] ]);
					n10 = find(lset, (int)L[idx + neighborhood_13[9] ]);
					n11 = find(lset, (int)L[idx + neighborhood_13[10] ]);
					n12 = find(lset, (int)L[idx + neighborhood_13[11] ]);
					n13 = find(lset, (int)L[idx + neighborhood_13[12] ]);
					
					
					
					if(n1 || n2 || n3 || n4 || n5 || n6 || n7 || n8 || n9 || n10 || n11 || n12 || n13)
					{
						int tlabel;
						
						if(n1) tlabel = n1;
						else if(n2) tlabel = n2;
						else if(n3) tlabel = n3;
						else if(n4) tlabel = n4;
						else if(n5) tlabel = n5;
						else if(n6) tlabel = n6;
						else if(n7) tlabel = n7;
						else if(n8) tlabel = n8;
						else if(n9) tlabel = n9;
						else if(n10) tlabel = n10;
						else if(n11) tlabel = n11;
						else if(n12) tlabel = n12;
						else if(n13) tlabel = n13;

						L[idx] = tlabel;
						
						if(n1 && n1 != tlabel) lset[n1] = tlabel;
						if(n2 && n2 != tlabel) lset[n2] = tlabel;
						if(n3 && n3 != tlabel) lset[n3] = tlabel;
						if(n4 && n4 != tlabel) lset[n4] = tlabel;
						if(n5 && n5 != tlabel) lset[n5] = tlabel;
						if(n6 && n6 != tlabel) lset[n6] = tlabel;
						if(n7 && n7 != tlabel) lset[n7] = tlabel;
						if(n8 && n8 != tlabel) lset[n8] = tlabel;
						if(n9 && n9 != tlabel) lset[n9] = tlabel;
						if(n10 && n10 != tlabel) lset[n10] = tlabel;
						if(n11 && n11 != tlabel) lset[n11] = tlabel;
						if(n12 && n12 != tlabel) lset[n12] = tlabel;
						if(n13 && n13 != tlabel) lset[n13] = tlabel;

					}
					else
					{
						ntable++;
						L[idx] = lset[ntable] = ntable;
					}
				
				}
				else
				{
					L[idx] = NO_OBJECT;
				
				}
					
			}
		}
	}
	
	// consolidate component table
    for( int i = 0; i <= ntable; i++ )
        lset[i] = find( lset, i );
	
    // run image through the look-up table
   	for(long k = 0; k < sz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < sy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				L[idx] = lset[ (int)L[idx] ];
			}
		}
	}
    
    // count up the objects in the image
    for( int i = 0; i <= ntable; i++ )
        lset[i] = 0;
	
   	for(long k = 0; k < sz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < sy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				lset[ (int)L[idx] ]++;
			}
		}
	}
	
    // number the objects from 1 through n objects
    int nobj = 0;
    lset[0] = 0;
    for( int i = 1; i <= ntable; i++ )
        if ( lset[i] > 0 )
            lset[i] = ++nobj;
	
	qDebug() << "how many objects found ..." << nobj;
	
    // run through the look-up table again
   	for(long k = 0; k < sz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < sy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				L[idx] = lset[ (int)L[idx] ];
			}
		}
	}
	
	//
	system("date");
	
	// visualize L 
	
	int max_L=0;
	
	for(long i=0; i<pagesz; i++)
	{
		if(max_L<L[i]) max_L = L[i];
	}
	
	if(max_L)
	{
		if(max_L<256)
		{
			for(long i=0; i<pagesz; i++)
			{
				bw[i] = L[i];
			}
		
		}
		else
		{
			for(long i=0; i<pagesz; i++)
			{
				bw[i] = 255*(L[i])/max_L;
			}
		}

	}
	
	
	Image4DSimple p4Dbw;
	p4Dbw.setData((unsigned char*)bw, sx, sy, sz, 1, V3D_UINT8);
	
	v3dhandle newwinbw = callback.newImageWindow();
	callback.setImage(newwinbw, &p4Dbw);
	callback.setImageName(newwinbw, "bw");
	callback.updateImageWindow(newwinbw);
	
	
	//
	int end_t = clock();
	
	qDebug() << "region growing time elapse ..." << end_t-start_t;
	
	
	// find the first N biggest regions 
	
	std::vector<STCL> labelList;
	
	// histogram of L
	int *a = new int [nobj+1];
	
	for(long i=0;  i<=nobj; i++)
	{
		a[i] = 0;
	}
	
	for(long i=0; i<pagesz; i++)
	{
		a[ L[i] ] ++;
	}
	
	//
	int np = fmin(5, nobj);
	
	for(int i=1;  i<=nobj; i++) // 0 is background
	{
		
		STCL s;
		
		s.count = a[i];
		s.label = i;

		//
		if(labelList.size()<1)
			labelList.push_back(s);
		else
		{
			for(unsigned int it=labelList.size(); it!=0; it--)
			{
				if(s.count<=labelList.at(it-1).count)
				{
					labelList.insert(labelList.begin() + it, 1, s);
					
					if(labelList.size()>np) // pick 5 points
						labelList.erase(labelList.end());
					
					break;
				}
				else
					continue;
				
			}
			
			//
			if(s.count>labelList.at(0).count && labelList.size()<np) // pick 5 points
				labelList.insert(labelList.begin(), s);
		}
		
		
	}
	
	
	LandmarkList cmList;
	
	bool flag_l = new bool [np];
	
	for(int i_n = 0; i_n<np; i_n++)
	{
		float scx=0,scy=0,scz=0,si=0;
	
		int label=labelList.at(i_n).label;
		
		for(long k = 0; k < sz; k++) 
		{				
			long idxk = k*offset_z;
			for(long j = 0;  j < sy; j++) 
			{
				long idxj = idxk + j*offset_y;
				
				for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
				{
					
					//
					if(L[idx]==label)
					{
						float cv = data1d[ idx ];
						
						scz += k*cv;
						scy += j*cv;
						scx += i*cv;
						si += cv;
					}
					
					
				}
			}
		}
		
		//
		if (si>0)
		{
			long ncx = scx/si + 0.5 +1; 
			long ncy = scy/si + 0.5 +1; 
			long ncz = scz/si + 0.5 +1;
		
			qDebug() << "position ..." << ncx << ncy << ncz;
			
			LocationSimple pp(ncx, ncy, ncz);
			cmList.push_back(pp);

		}
	
	}
	
	//
	system("date");
	
	int end_t_t = clock();
	
	qDebug() << "time elapse ..." << end_t_t - end_t;
	
	
	Image4DSimple p4DImage;
	p4DImage.setData((unsigned char*)data1d, sx, sy, sz, 1, subject->datatype); // data1d
	
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &p4DImage);
	callback.setImageName(newwin, "region_growing");
	callback.setLandmark(newwin, cmList); // center of mass
	callback.updateImageWindow(newwin);
	
}
