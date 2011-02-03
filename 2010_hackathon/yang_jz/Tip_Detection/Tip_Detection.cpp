/*
 *  TIP_DETECTION.cpp
 *
 *  Created by Yang, Jinzhu, on 12/15/10.
 *
 */

#include "Tip_Detection.h"
#include "v3d_message.h"
#include <deque>
#include <algorithm>
#include <functional>
#include<math.h>

#define BACKGROUND -1 
#define DISOFENDS 5 


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(TIP_DETECTION, TipPlugin);

//plugin funcs
const QString title = "Tip Detection";
QStringList TipPlugin::menulist() const
{
    return QStringList()
	<< tr("Tip Detection")
	<< tr("Dtectioon Surface")
	<< tr("Distance From Source")
	<< tr("Segmentation")
	<< tr("MarkList StatisticalPixel")
	<< tr("XOY Data")
	<< tr("XOZ Data")
	<< tr("ZOY Data")
	<< tr("MPR Image Statistical")
	<< tr("Tip two_dimension")
	<< tr("Gaussian")
	<< tr("Canny")
	<< tr("Help");
}

void TipPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("Tip Detection"))
	{
    	Tipdetection(callback, parent,1);
    }
	else if (menu_name == tr("Dtectioon Surface"))
	{
		Tipdetection(callback, parent,2);
	}else if (menu_name == tr("Distance From Source"))
	{
		Tipdetection(callback, parent,3);
	}else if (menu_name == tr("Segmentation"))
	{
		Tipdetection(callback, parent,4);
	}else if (menu_name == tr("MarkList StatisticalPixel"))
	{
		Tipdetection(callback, parent,5);
		
	}else if(menu_name == tr("XOY Data"))
	{
		Tipdetection(callback, parent,6);
	}else if(menu_name == tr("XOZ Data"))
	{
		Tipdetection(callback, parent,7);
		
	}else if(menu_name == tr("ZOY Data")) 
	{
		Tipdetection(callback, parent,8);
		
	}else if(menu_name == tr("MPR Image Statistical"))
	{
		Tipdetection(callback, parent,9);
	}else if(menu_name == tr("Tip two_dimension"))
	{
		Tipdetection(callback, parent,10);
	}else if(menu_name == tr("Gaussian"))
	{
	   Tipdetection(callback, parent,11);
		
	}else if (menu_name == tr("Canny"))
	{
		Canny(callback, parent,1);
	}
	else if (menu_name == tr("Help"))
	{
		//v3d_msg("Simple adaptive thresholding: for each voxel, compute a threshold which is the average intensity o");
	}
}
//
void TipPlugin::SetImageInfo1D(V3DLONG* data, V3DLONG count, V3DLONG width, V3DLONG height)
{
	m_ppsOriData1D = data;
	
	m_iImgCount = count;
	
	m_iImgWidth = width;
	
	m_iImgHeight = height;
	
	printf("count=%d w=%d h=%d\n",m_iImgCount,m_iImgWidth,m_iImgHeight);
}

void TipPlugin::Set_DFS_Seed(SpacePoint_t seed)
{
	m_sptSeed = seed;
	
}
void TipPlugin::SetEndPoints(vector<SpacePoint_t> points)
{
	m_vdfsptEndPoint.clear();
	V3DLONG size = points.size();
	for(V3DLONG i = 0; i < size; ++i)
	{
		DFSPoint_t pt;
		pt.m_x = points.at(i).m_x;
		pt.m_y = points.at(i).m_y;
		pt.m_z = points.at(i).m_z;
		m_vdfsptEndPoint.push_back(pt);
	}
}

void TipPlugin::SetMinLength(V3DLONG minlength)
{
	m_iMinLength = minlength;
}

void TipPlugin::Initialize1D()
{
//bug : should add margin for x and y directions as well
	
	
	m_iImgSize = m_iImgWidth * m_iImgHeight;
	
	
	m_ppsImgData = new V3DLONG*[m_iImgCount + 2];
	
	printf("count=%d\n",m_iImgCount);
	
	m_psTemp = new V3DLONG[m_iImgSize];
	memset(m_psTemp, BACKGROUND, m_iImgSize * sizeof(V3DLONG));
	m_ppsImgData[0] = m_psTemp;
	for(V3DLONG c = 0; c < m_iImgCount; ++c)
	{
		m_ppsImgData[c + 1] = &m_ppsOriData1D[c*m_iImgSize];
	}
	
	m_psTemp = new V3DLONG[m_iImgSize];
	
	memset(m_psTemp, BACKGROUND, m_iImgSize * sizeof(V3DLONG));
	
	m_ppsImgData[m_iImgCount + 1] = m_psTemp;
	
	m_iImgCount += 2;
	m_ulVolumeSize = m_iImgCount * m_iImgSize;
	
}
void TipPlugin::Initialize2D()
{
	
	m_ppsImgData2D = new V3DLONG[m_iImgWidth * m_iImgHeight];
	
	memset(m_ppsImgData2D, BACKGROUND, m_iImgWidth*m_iImgHeight*sizeof(V3DLONG));
	
	for (V3DLONG j=0; j <m_iImgHeight; j++)
	{
		for (V3DLONG i=0; i< m_iImgWidth; i++)
		{
			m_ppsImgData2D[j*m_iImgWidth + i] = m_ppsOriData1D[j*m_iImgWidth +i];		
		}
	}
		
	m_iImgSize = m_iImgWidth * m_iImgHeight;
	
	
}
void TipPlugin::SetDFB2D()
{
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};	
	V3DLONG x = 0;
	V3DLONG y = 0;
	V3DLONG z = 0;
	
	V3DLONG i = 0;
	V3DLONG j = 0;
	V3DLONG index= 0;
	V3DLONG count = 0;
	bool find;
    DFBPoint_t point;
	
	//printf("countnew=%d\n",m_iImgCount);
	
	for(j = 1; j < m_iImgHeight-1; j++)
	{
			for(i = 1; i < m_iImgWidth-1; i++)
			{
				if(m_ppsImgData2D[m_iImgWidth * j + i] != BACKGROUND)
				{
					index = m_iImgWidth * j + i;
					find = false;
					// 8
						for(int n = 0; n < 9; ++n)
						{
							y = j + nDy[n];
							x = i + nDx[n];
							if(m_ppsImgData2D[m_iImgWidth * y + x] == BACKGROUND)
							{
								find = true;
							}
						}
					if(find)
					{
						point.m_x = i;
						point.m_y = j;
						point.m_z = 1;
						//printf("m_x=%d m_y=%d surfacepixel=%d\n",i,j,m_ppsImgData2D[m_iImgWidth * j + i]);
						m_vdfbptSurface.push_back(point);
					}
				}
			}
	}
	
}
void TipPlugin::SetDFS2D()
{
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	
	V3DLONG x = 0;
	V3DLONG y = 0;
	V3DLONG z = 0;
	V3DLONG count = 0;
	
	V3DLONG i = 0;
	V3DLONG j = 0;
	V3DLONG k = 0;
	V3DLONG l = 0;
    DFSPoint_t point;
	deque<DFSPoint_t> dfs;
	
	V3DLONG temp_l = 0;
	V3DLONG index_nei = 0;
	
	point.m_x = m_sptSeed.m_x;
	point.m_y = m_sptSeed.m_y;
	point.m_z = m_sptSeed.m_z + 1;
	point.m_l = 0;
	dfs.push_back(point);
	try
	{
		m_piDFS = new V3DLONG[m_iImgSize]; 
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_piDFS) {delete []m_piDFS; m_piDFS=0;}
		return;
	}
	memset(m_piDFS, BACKGROUND, m_iImgSize * sizeof(V3DLONG));
	// dis(S) = 0;
	
	m_piDFS[m_iImgWidth * point.m_y + point.m_x] = 0;

	while(!dfs.empty())
	{	
        point = dfs.front();
		dfs.pop_front();
		
		i = point.m_z;
		j = point.m_y;
		k = point.m_x;
		l = point.m_l;
		count = 0;
		// 8
			for(int n = 0; n < 9; n++)
			{
				y = j + nDy[n];
				x = k + nDx[n];
				++count;
				index_nei = m_iImgWidth * y + x;
				if(m_ppsImgData2D[index_nei]!= BACKGROUND)
				{
					index_nei = m_iImgWidth * y + x;
					switch(count)
					{ //4 edge
						case 5:
						case 2:
						case 4:
						case 6:
						case 8:
							temp_l = l + 10;
							if(m_piDFS[index_nei] != BACKGROUND)
							{
								if(temp_l < m_piDFS[index_nei])
								{
									m_piDFS[index_nei] = temp_l;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = 1;
									point.m_l = temp_l;
									dfs.push_back(point);
								}
							}
							else
							{
								m_piDFS[index_nei] = temp_l;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = 1;
								point.m_l = temp_l;
								dfs.push_back(point);
							}
							break;
							// 4 vertexes
						case 1:
						case 3:
						case 7:
						case 9:
							temp_l = l + 14;
							if(m_piDFS[index_nei] != BACKGROUND)
							{
								if(temp_l < m_piDFS[index_nei])
								{
									m_piDFS[index_nei] = temp_l;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = 1;
									point.m_l = temp_l;
									dfs.push_back(point);
								}
							}
							else
							{
								m_piDFS[index_nei] = temp_l;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_l = temp_l;
								dfs.push_back(point);
							}
							break;
						default:
							break;
					}
				}
			}
	}
	for(j = 0; j < m_iImgHeight; j++)
	{
		for(k = 0; k < m_iImgWidth; k++)
		{
			//if (m_piDFS[j*m_iImgWidth+k] > -1) 
			{
				//printf("m_piDFS=%d\n",m_piDFS[j*m_iImgWidth+k]);
			}
		}
	}
	
}

void TipPlugin::GenSourceField()
{
	V3DLONG nS,nE,nCount,kk;
	int i,j,k;
	int x,y,z;
	int xx,yy,zz;
	bool bFind ;
	int nXW,nYW,nZW;
	DFSPoint_t nowP;
	try
	{
		m_piDFS = new V3DLONG[m_iImgSize]; 
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_piDFS) {delete []m_piDFS; m_piDFS=0;}
		return;
	}
	
	memset(m_piDFS, BACKGROUND, m_iImgSize * sizeof(V3DLONG));
	
	try
	{
		m_pAVMark = new V3DLONG[m_iImgSize]; 
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_pAVMark) {delete []m_pAVMark; m_pAVMark=0;}
		return;
	}
	memset(m_pAVMark, 0, m_iImgSize * sizeof(V3DLONG));

	DFSPoint_t m_vSourcePoint;
	
	V3DLONG temp_l = 0;
	V3DLONG index_nei = 0;
	
	m_vSourcePoint.m_x = m_sptSeed.m_x;
	m_vSourcePoint.m_y = m_sptSeed.m_y;
	m_vSourcePoint.m_z = 1;
	m_vSourcePoint.m_l = 0;	
	
	if(m_ppsImgData2D[m_vSourcePoint.m_y*m_iImgWidth + m_vSourcePoint.m_x]== BACKGROUND)
	{
		v3d_msg("The wrong start point");
		return;
	}

	DFSPoint_t* pPointList = new DFSPoint_t[m_iImgWidth*m_iImgHeight];
	nE = 0;
	nCount = 1;
	pPointList[0] = m_vSourcePoint;
	x = m_vSourcePoint.m_x ;
	y = m_vSourcePoint.m_y;
	z = m_vSourcePoint.m_z;
	m_piDFS[y*m_iImgWidth+x] = 0;
	m_pAVMark[y*m_iImgWidth+x] = 1;
loop:
	nS = nE;
	nE = nS + nCount;
	nCount = 0;
	bFind = false;
	for(kk=nS;kk<nE;kk++)
	{
		nowP = pPointList[kk];
		for(i=-1;i<2;i++)
			for(j=-1;j<2;j++)
				{
					x = nowP.m_x+i;
					y = nowP.m_y+j;
					if((x<0)||(y<0)||(x>m_iImgWidth)||(y>m_iImgHeight))
						continue;
					xx = x ;
					yy = y ;
					if(m_pAVMark[yy*m_iImgWidth+xx]==1)
						continue;
					if(m_ppsImgData2D[y*m_iImgWidth+x] == 255)
					{
						SDistCount(x,y,z);
						pPointList[nE+nCount].m_x = x;
						pPointList[nE+nCount].m_y = y;
						m_pAVMark[yy*m_iImgWidth+xx] = 1;
						nCount++;
						bFind = true;
					}
				}
	}
	if(bFind == true)
		goto loop;
	delete []pPointList;
}

void TipPlugin::SDistCount(int x,int y,int z)
{
	V3DLONG mdist,tdist;
	int xx,yy,zz;
	int i,j,k;
	int sum;
	mdist = 32767000;
	for(i=-1;i<2;i++)
		for(j=-1;j<2;j++)
			for(k=-1;k<2;k++)
			{
				xx = x+i;
				yy = y+j;
				zz = z+k;
				if((xx<0)||(yy<0)||(xx>m_iImgWidth)||(yy>m_iImgHeight))
					continue;
				if(m_pAVMark[yy*m_iImgWidth+xx]==1)
				{
					sum = abs(xx-x)+abs(yy-y);
					if(sum == 1)
						tdist = m_piDFS[yy*m_iImgWidth+xx] + 10;
					else if(sum == 2)
						tdist = m_piDFS[yy*m_iImgWidth+xx] + 14;;
					if(tdist<mdist)
						mdist = tdist;
				}
			}
	m_piDFS[y*m_iImgWidth+x] = mdist;
}


void TipPlugin::SearchEndPoints2D()
{
	V3DLONG i, j, k, l,ii,jj;
	V3DLONG x, y, z;
	V3DLONG index, index_nei;
	bool endp;
	DFBPoint_t point_d;
	DFSPoint_t point_l;
	DFSPoint_t last;
	V3DLONG count1;
	V3DLONG count2;
	vector<V3DLONG> len; 
	vector<DFSPoint_t> temp; 
//	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
//	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};	
	
	V3DLONG size = m_vdfbptSurface.size();
	printf("size=%d\n",size);
	
	for(V3DLONG num = 0; num < size; ++num)
	{
		point_d = m_vdfbptSurface.at(num);
		j = point_d.m_y;
		k = point_d.m_x;
		index =  m_iImgWidth * j + k;
		l = m_piDFS[index];
		if (l == BACKGROUND)
		{
			continue;
		}

	//	printf("surfacedfs=%d\n",l);
		endp = true;
		count1 = 0;
		count2 = 0;
		//8 
		for(ii=-1;ii<2;ii++)
			for(jj=-1;jj<2;jj++)
			{ 
				y = j + jj;
				x = k + ii;
			//for(int n = 0; n < 9; ++n)
		//	{
				//y = j + nDy[n];
				//x = k + nDx[n];
				if(m_ppsImgData2D[m_iImgWidth * y + x] != BACKGROUND)
				{
					index_nei = m_iImgWidth * y + x;
					if(index != index_nei)
					{
						++count1;
						if(l < m_piDFS[index_nei])
						{
							endp = false;
						}
						if(l >= m_piDFS[index_nei])
						{
							++count2;
						}
					}
				}
			//}
		}
		if(endp)
		{
			point_l.m_x = k;
			point_l.m_y = j;
			point_l.m_z = 1;	
			point_l.m_l = l;
			m_vdfsptEndPoint.push_back(point_l);	
			len.push_back(l);
		}
		else
		{
//			if(count1 == count2)
//			{
//				if(m_vdfsptEndPoint.empty())
//				{
//					point_l.m_x = k;
//					point_l.m_y = j;
//					point_l.m_z = 1;	
//					point_l.m_l = l;
//					m_vdfsptEndPoint.push_back(point_l);
//					len.push_back(l);
//				}
//				else
//				{
//					last = m_vdfsptEndPoint.at(m_vdfsptEndPoint.size() - 1);
//					if(sqrt((last.m_x - k) * (last.m_x - k) + (last.m_y - j) * (last.m_y - j)) > DISOFENDS)
//					{										
//						point_l.m_x = k;
//						point_l.m_y = j;
//						point_l.m_z = 1;	
//						point_l.m_l = l;
//						m_vdfsptEndPoint.push_back(point_l);	
//						len.push_back(l);
//					}
//				}
//			}
		}
	}
	size = len.size();
	stable_sort(len.begin(), len.end(), greater<V3DLONG>());
	for(i = 0; i < size; ++i)
	{
		for(V3DLONG j = 0; j < size; ++j)
		{
			if(len.at(i) == m_vdfsptEndPoint.at(j).m_l)
			{
				temp.push_back(m_vdfsptEndPoint.at(j));
				break;
			}
		}
	}
	m_vdfsptEndPoint.clear();
	m_vdfsptEndPoint = temp;

}
void TipPlugin::SetDFB()
{
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};	
	V3DLONG x = 0;
	V3DLONG y = 0;
	V3DLONG z = 0;
	V3DLONG count = 0;
	V3DLONG index = 0;
	
	V3DLONG i = 0;
	V3DLONG j = 0;
	V3DLONG k = 0;
	
	bool find;
    DFBPoint_t point;
	
	deque<DFBPoint_t> surface;
	
	try
	{
		m_piDFB = new V3DLONG[m_ulVolumeSize]; 
	}
		catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_piDFB) {delete []m_piDFB; m_piDFB=0;}
		return;
	}
	memset(m_piDFB, BACKGROUND, m_ulVolumeSize * sizeof(V3DLONG));
	
	//printf("countnew=%d\n",m_iImgCount);
	
	for(i = 0; i < m_iImgCount; i++)
	{
		for(j = 0; j < m_iImgHeight; j++)
		{
			for(k = 0; k < m_iImgWidth; k++)
			{
				if(m_ppsImgData[i][m_iImgWidth * j + k] != BACKGROUND)
				{
					count = 0;
					index = m_iImgSize * i + m_iImgWidth * j + k;
					find = false;
					// ’“26
					for(int m = 0; m < 3; ++m)
					{
						for(int n = 0; n < 9; ++n)
						{
							z = i + nDz[m];
							y = j + nDy[n];
							x = k + nDx[n];
							++count;
							if(m_ppsImgData[z][m_iImgWidth * y + x] == BACKGROUND)
							{
								find = true;
								switch(count)
								{
										// 6 faces
									case 5:
									case 11:
									case 13:
									case 15:
									case 17:
									case 23:                          
										m_piDFB[index] = 3;									
										break;
										// 12 edges
									case 2:
									case 4:
									case 6:
									case 8:
									case 10:
									case 12:
									case 16:
									case 18:
									case 20:
									case 22:
									case 24:
									case 26:
										if(m_piDFB[index] == BACKGROUND || m_piDFB[index] == 5)
											m_piDFB[index] = 4;									
										break;
										// 8 vertexes
									case 1:
									case 3:
									case 7:
									case 9:
									case 19:
									case 21:
									case 25:
									case 27:
										if(m_piDFB[index] == BACKGROUND)
											m_piDFB[index] = 5;											
										break;
									default:
										break;
								}
							}
						}
					}
					if(find)
					{
						point.m_x = k;
						point.m_y = j;
						point.m_z = i;
						point.m_d = m_piDFB[index];
						surface.push_back(point);
						m_vdfbptSurface.push_back(point);
					}
				}
			}
		}
	}
			
	// 
	int d = 0;
	int temp_d = 0;
	int index_nei = 0;
	
	while(!surface.empty())
	{	
        point = surface.front();
		surface.pop_front();
		i = point.m_z;
		j = point.m_y;
		k = point.m_x;
		d = point.m_d;
		count = 0;
		// ’“26
		for(int m = 0; m < 3; ++m)
			for(V3DLONG n = 0; n < 9; ++n)
			{
				z = i + nDz[m];
				y = j + nDy[n];
				x = k + nDx[n];
				
				count++;
				
				if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
				{
					index_nei = m_iImgSize * z + m_iImgWidth * y + x;
					switch(count)
					{
							// 6 faces
						case 5:
						case 11:
						case 13:
						case 15:
						case 17:
						case 23:
							temp_d = d + 3;
							if(m_piDFB[index_nei] != BACKGROUND)
							{
								if(temp_d < m_piDFB[index_nei])
								{
									m_piDFB[index_nei] = temp_d;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_d = temp_d;
									surface.push_back(point);
								}
							}
							else
							{
								m_piDFB[index_nei] = temp_d;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_d = temp_d;
								surface.push_back(point);
							}
							break;
							// 12 edges
						case 2:
						case 4:
						case 6:
						case 8:
						case 10:
						case 12:
						case 16:
						case 18:
						case 20:
						case 22:
						case 24:
						case 26:				
							temp_d = d + 4;
							if(m_piDFB[index_nei] != BACKGROUND)
							{
								if(temp_d < m_piDFB[index_nei])
								{
									m_piDFB[index_nei] = temp_d;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_d = temp_d;
									surface.push_back(point);
								}
							}
							else
							{
								m_piDFB[index_nei] = temp_d;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_d = temp_d;
								surface.push_back(point);
							}
							break;
							// 8 vertexes
						case 1:
						case 3:
						case 7:
						case 9:
						case 19:
						case 21:
						case 25:
						case 27:
							temp_d = d + 5;
							if(m_piDFB[index_nei] != BACKGROUND)
							{
								if(temp_d < m_piDFB[index_nei])
								{
									m_piDFB[index_nei] = temp_d;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_d = temp_d;
									surface.push_back(point);
								}
							}
							else
							{
								m_piDFB[index_nei] = temp_d;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_d = temp_d;
								surface.push_back(point);
							}
							break;
						default:
							break;
					}
				}
			}
	}
	
	///////////////////////////
//	for(i = 0; i < m_iImgCount; i++)
//	{
//		for(j = 0; j < m_iImgHeight; j++)
//		{	for(k = 0; k < m_iImgWidth; k++)
//			{
//				if (m_piDFB[i*m_iImgSize+j*m_iImgWidth+k] > -1) 
//				{
//					// printf("m_piDFB=%d\n",m_piDFB[i*m_iImgSize+j*m_iImgWidth+k]);
//					
//				}
//			}
//		}
//	}
//	
}

//
void TipPlugin::CheckDFB()
{
	int i, j, k;
	int index;
	SpacePoint_t point;
	
	V3DLONG x, y, z;
	V3DLONG index_nei;
	V3DLONG count;
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	for(i = 1; i < m_iImgCount - 1; ++i)
	{	
		for(j = 0; j < m_iImgHeight; ++j)
		{
			for(k = 0; k < m_iImgWidth; ++k)
			{
				index = i * m_iImgSize + m_iImgWidth * j + k;
				if(m_piDFB[index] != BACKGROUND)
				{
					count = 0;
					for(int m = 0; m < 3; ++m)
						for(int n = 0; n < 9; ++n)
						{
							x = k + nDx[n];
							y = j + nDy[n];
							z = i + nDz[m];
							index_nei = z * m_iImgSize + m_iImgWidth * y + x;
							if(m_piDFB[index_nei] != BACKGROUND)
							{
								++count;
							}
						}
					if(count != 27)
					{
						continue;
					}
					
					count = 0;
					for(int m = 0; m < 3; ++m)
						for(int n = 0; n < 9; ++n)
						{
							x = k + nDx[n];
							y = j + nDy[n];
							z = i + nDz[m];
							index_nei = z * m_iImgSize + m_iImgWidth * y + x;
							if(m_piDFB[index] >= m_piDFB[index_nei])
							{
								++count;					
							}
						}	
					if(count >= 25)
					{
						point.m_x = k;
						point.m_y = j;
						point.m_z = i;	
						m_vsptCenterpath.push_back(point);
					}
				}
			}
		}
	}
}

//
void TipPlugin::SetDFS()
{
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	
	V3DLONG x = 0;
	V3DLONG y = 0;
	V3DLONG z = 0;
	V3DLONG count = 0;
	
	V3DLONG i = 0;
	V3DLONG j = 0;
	V3DLONG k = 0;
	V3DLONG l = 0;
    DFSPoint_t point;
	deque<DFSPoint_t> dfs;
	
	V3DLONG temp_l = 0;
	V3DLONG index_nei = 0;
	
	point.m_x = m_sptSeed.m_x;
	point.m_y = m_sptSeed.m_y;
	point.m_z = m_sptSeed.m_z + 1;
	point.m_l = 0;
	dfs.push_back(point);
	try
	{
		m_piDFS = new V3DLONG[m_ulVolumeSize]; 
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (m_piDFS) {delete []m_piDFS; m_piDFS=0;}
		return;
	}
	memset(m_piDFS, BACKGROUND, m_ulVolumeSize * sizeof(V3DLONG));
	// dis(S) = 0;
	m_piDFS[m_iImgSize * point.m_z + m_iImgWidth * point.m_y + point.m_x] = 0;
	while(!dfs.empty())
	{	
        point = dfs.front();
		dfs.pop_front();
		i = point.m_z;
		j = point.m_y;
		k = point.m_x;
		l = point.m_l;
		count = 0;
		// ’“26
		for(int m = 0; m < 3; m++)
			for(int n = 0; n < 9; n++)
			{
				z = i + nDz[m];
				y = j + nDy[n];
				x = k + nDx[n];
				++count;
				if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
				{
					index_nei = m_iImgSize * z + m_iImgWidth * y + x;
					switch(count)
					{
							// 6 faces
						case 5:
						case 11:
						case 13:
						case 15:
						case 17:
						case 23:
							temp_l = l + 1;
							if(m_piDFS[index_nei] != BACKGROUND)
							{
								if(temp_l < m_piDFS[index_nei])
								{
									//printf("temp=%d m_pidfs=%d\n",temp_l,m_piDFS[index_nei]);
									m_piDFS[index_nei] = temp_l;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_l = temp_l;
									dfs.push_back(point);
								}
							}
							else
							{
							//	printf("tem=%d",temp_l);
								m_piDFS[index_nei] = temp_l;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_l = temp_l;
								dfs.push_back(point);
							}
							break;
							// 12 edges
						case 2:
						case 4:
						case 6:
						case 8:
						case 10:
						case 12:
						case 16:
						case 18:
						case 20:
						case 22:
						case 24:
						case 26:				
							temp_l = l + 2;
							if(m_piDFS[index_nei] != BACKGROUND)
							{
								if(temp_l < m_piDFS[index_nei])
								{
									m_piDFS[index_nei] = temp_l;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_l = temp_l;
									dfs.push_back(point);
								}
							}
							else
							{
								m_piDFS[index_nei] = temp_l;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_l = temp_l;
								dfs.push_back(point);
							}
							break;
							// 8 vertexes
						case 1:
						case 3:
						case 7:
						case 9:
						case 19:
						case 21:
						case 25:
						case 27:
							temp_l = l + 3;
							if(m_piDFS[index_nei] != BACKGROUND)
							{
								if(temp_l < m_piDFS[index_nei])
								{
									m_piDFS[index_nei] = temp_l;	
									point.m_x = x;
									point.m_y = y;
									point.m_z = z;
									point.m_l = temp_l;
									dfs.push_back(point);
								}
							}
							else
							{
								m_piDFS[index_nei] = temp_l;	
								point.m_x = x;
								point.m_y = y;
								point.m_z = z;
								point.m_l = temp_l;
								dfs.push_back(point);
							}
							break;
						default:
							break;
					}
				}
			}
	}
	//	for(i = 0; i < m_iImgCount; i++)
	//	{
	//		for(j = 0; j < m_iImgHeight; j++)
	//		{
	//			for(k = 0; k < m_iImgWidth; k++)
	//			{
	//				if (m_piDFS[i*m_iImgSize+j*m_iImgWidth+k] > -1) 
	//				{
	//					printf("m_piDFS=%d\n",m_piDFS[i*m_iImgSize+j*m_iImgWidth+k]);
	//				}
	//			}
	//		}
	//	}	
}
void TipPlugin::CheckDFS()
{
	V3DLONG i, j, k;
	V3DLONG index;
	SpacePoint_t point;
	
	for(i = 1; i < m_iImgCount - 1; ++i)
	{	
		for(j = 0; j < m_iImgHeight; ++j)
		{
			for(k = 0; k < m_iImgWidth; ++k)
			{
				index = i * m_iImgSize + m_iImgWidth * j + k;
				if(m_piDFS[index] == 80 || m_piDFS[index] == 81 || m_piDFS[index] == 82)
				{
					point.m_x = k;
					point.m_y = j;
					point.m_z = i;	
					m_vsptCenterpath.push_back(point);
				}
			}
		}
	}
}

// 

void TipPlugin::SearchEndPoints()
{
	V3DLONG i, j, k, l;
	V3DLONG x, y, z;
	V3DLONG index, index_nei;
	bool endp;
	DFBPoint_t point_d;
	DFSPoint_t point_l;
	DFSPoint_t last;
	V3DLONG count1;
	V3DLONG count2;
	vector<V3DLONG> len; 
	vector<DFSPoint_t> temp; 
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	static int nDz[] = {-1,0,1};
	V3DLONG size = m_vdfbptSurface.size();
	//printf("size=%d\n",size);
	for(V3DLONG num = 0; num < size; ++num)
	{
		point_d = m_vdfbptSurface.at(num);
		i = point_d.m_z;
		j = point_d.m_y;
		k = point_d.m_x;
		index = m_iImgSize * i + m_iImgWidth * j + k;
		l = m_piDFS[index];		
		endp = true;
		count1 = 0;
		count2 = 0;
		//26
		for(int kk=-3;kk<4;kk++)
			for(int m=-3;m<4;m++)
				for(int n=-3; n<4; ++n)
			//for(int n = 0; n < 9; ++n)
			{
//				z = i + nDz[m];
//				y = j + nDy[n];
//				x = k + nDx[n];
				z = i + kk;
				y = j + m;
				x = k + n;
				if((x<0)||(y<0)||(x>m_iImgWidth)||(y>m_iImgHeight)|| (z > m_iImgCount))
					continue;
				if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
				{
					index_nei = m_iImgSize * z + m_iImgWidth * y + x;
					if(index != index_nei)
					{
						++count1;
						if(l < m_piDFS[index_nei])
						{
							endp = false;
						}
						if(l >= m_piDFS[index_nei])
						{
							++count2;
						}
					}
				}
			}
		if(endp)
		{
			point_l.m_x = k;
			point_l.m_y = j;
			point_l.m_z = i;	
			point_l.m_l = l;
			m_vdfsptEndPoint.push_back(point_l);	
			len.push_back(l);
		}
		else
		{
			if(count1 == count2)
			{
				if(m_vdfsptEndPoint.empty())
				{
					point_l.m_x = k;
					point_l.m_y = j;
					point_l.m_z = i;	
					point_l.m_l = l;
					m_vdfsptEndPoint.push_back(point_l);
					len.push_back(l);
				}
				else
				{
					last = m_vdfsptEndPoint.at(m_vdfsptEndPoint.size() - 1);
					if(sqrt(1.0*(last.m_x - k) * (last.m_x - k) + (last.m_y - j) * (last.m_y - j)
							+ (last.m_z - i) * (last.m_z - i)) > DISOFENDS)
					{										
						point_l.m_x = k;
						point_l.m_y = j;
						point_l.m_z = i;	
						point_l.m_l = l;
						m_vdfsptEndPoint.push_back(point_l);	
						len.push_back(l);
					}
				}
			}
		}
	}
	size = len.size();
	stable_sort(len.begin(), len.end(), greater<V3DLONG>());
	for(i = 0; i < size; ++i)
	{
		for(V3DLONG j = 0; j < size; ++j)
		{
			if(len.at(i) == m_vdfsptEndPoint.at(j).m_l)
			{
				temp.push_back(m_vdfsptEndPoint.at(j));
				break;
			}
		}
	}
	m_vdfsptEndPoint.clear();
	m_vdfsptEndPoint = temp;
}
void TipPlugin::Clear()
{
	if(m_piDFB)
	{
		delete []m_piDFB;
		m_piDFB = NULL;
	}
	
	if(m_piDFS)
	{
		delete []m_piDFS;
		m_piDFS = NULL;
	}
	
//	
//	if(m_ppsImgData[0])
//	{
//		delete []m_ppsImgData[0];
//		m_ppsImgData[0] = NULL;
//	}
//	if(m_ppsImgData[m_iImgCount - 1])
//	{
//		delete []m_ppsImgData[m_iImgCount - 1];
//		m_ppsImgData[m_iImgCount - 1] = NULL;
//	}
//	if(m_ppsImgData)
//	{
//		delete []m_ppsImgData;
//		m_ppsImgData = NULL;
//	}	
	m_psTemp = NULL;	
	//m_ppsOriData = NULL;
	m_ppsOriData1D = NULL;
	m_iMinLength = 0;
	m_iImgWidth = 0;
	m_iImgHeight = 0;
	m_iImgSize = 0;
	m_iImgCount = 0;
	m_ulVolumeSize = 0;
	m_vdfsptEndPoint.clear();
	m_vdfbptSurface.clear();
	
}
void TipPlugin::IterateSeg(unsigned char *apsInput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, unsigned char *apsOutput)
{
	V3DLONG pMax, pMin;
	V3DLONG i,j,k;
	double T1 ;
	double T2;
	V3DLONG S0 , n0;
	V3DLONG S1, n1 ;
	double allow; 
	double d ;	
	V3DLONG mCount = iImageHeight * iImageWidth;
	pMax = pMin = 0;
	for(V3DLONG k=0; k<iImageLayer; k++)
	{		
				
		for(j = 0; j<iImageHeight; j++)
		{
			for(i=0; i<iImageWidth; i++)
			{
				pMax = (apsInput[k*mCount+j *iImageWidth + i] > pMax)? apsInput[k*mCount+j *iImageWidth + i]:pMax;
				pMin = (apsInput[k*mCount+j *iImageWidth + i] < pMin)? apsInput[k*mCount+j *iImageWidth + i]:pMin;
			}
		}
	}
	for(V3DLONG m=0; m<iImageLayer; m++)
	{		
		pMax = pMin =apsInput[m*mCount];		
		for(i=0; i<mCount; i++)
		{
			for(k=0; k<mCount; k++)
			{
				pMax = (apsInput[m*mCount+k] > pMax)? apsInput[m*mCount+k]:pMax;
				pMin = (apsInput[m*mCount+k] < pMin)? apsInput[m*mCount+k]:pMin;
				
			}
			T1 = (pMax + pMin) / 2.0;
			T2 = 0;
			S0 = 0;
			n0 = 0;
			S1 = 0;
			n1 = 0;
			allow = 1.0; 
			if ( T1 > T2) 
			{
				d = T1 - T2;
			}else
			{
				d = T2 - T1;
			}
			
			//	d = abs(T - TT);			
			while(d > allow) 
			{	
				for(j=0; j<mCount; j++)
				{
					if(apsInput[m*mCount+j] > T1) 
					{
						S0 += apsInput[m*mCount+j];
						n0++;
					}
					else
					{
						S1 += apsInput[m*mCount+j];
						n1++;
					}
				}				
				if(n0 ==0 || n1 == 0)
					return  ;
				else
				{   
					T2 = (S0 / n0 + S1 / n1) / 2;
				}
				
				//	d = abs (T - TT);
				if ( T1 > T2) 
				{
					d = T1 -T2;
				}else
				{
					d = T2-T1;
				}
				T1 = T2;
			}
			for(i=0; i<mCount; i++)
			{
				if(apsInput[m*mCount+i] > T1)
				{
					apsOutput[m*mCount+i] = 255;				
				}
				else
				{
					apsOutput[m*mCount+i] = 0;				
				}
			}				
		}			
	}
//	for(V3DLONG m=0; m<iImageLayer; m++)
//	{		
//		pMax = pMin =apsInput[m*mCount];		
//		for(i=0; i<mCount; i++)
//		{
//			for(k=0; k<mCount; k++)
//			{
//				pMax = (apsInput[m*mCount+k] > pMax)? apsInput[m*mCount+k]:pMax;
//				pMin = (apsInput[m*mCount+k] < pMin)? apsInput[m*mCount+k]:pMin;
//				
//			}
//			T1 = (pMax + pMin) / 2.0;
//			T2 = 0;
//			S0 = 0;
//			n0 = 0;
//			S1 = 0;
//			n1 = 0;
//			allow = 1.0; 
//			if ( T1 > T2) 
//			{
//				d = T1 - T2;
//			}else
//			{
//				d = T2 - T1;
//			}
//
//		//	d = abs(T - TT);			
//			while(d > allow) 
//			{	
//				for(j=0; j<mCount; j++)
//				{
//					if(apsInput[m*mCount+j] > T1) 
//					{
//						S0 += apsInput[m*mCount+j];
//						n0++;
//					}
//					else
//					{
//						S1 += apsInput[m*mCount+j];
//						n1++;
//					}
//				}				
//				if(n0 ==0 || n1 == 0)
//					return  ;
//				else
//				{   
//					T2 = (S0 / n0 + S1 / n1) / 2;
//				}
//				
//			//	d = abs (T - TT);
//				if ( T1 > T2) 
//				{
//					d = T1 -T2;
//				}else
//				{
//					d = T2-T1;
//				}
//				T1 = T2;
//			}
//			for(i=0; i<mCount; i++)
//			{
//				if(apsInput[m*mCount+i] > T1)
//				{
//					apsOutput[m*mCount+i] = 255;				
//				}
//				else
//				{
//					apsOutput[m*mCount+i] = 0;				
//				}
//			}				
//		}			
//	}
}
void TipPlugin::BinaryProcess(unsigned char*apsInput, unsigned char * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d)
{
	V3DLONG i, j,k,n,count;
	double t, temp;
	V3DLONG mCount = iImageHeight * iImageWidth;
	for (i=0; i<iImageLayer; i++)
	{
		for (j=0; j<iImageHeight; j++)
		{
			for (k=0; k<iImageWidth; k++)
			{
				V3DLONG curpos = i * mCount + j*iImageWidth + k;
				V3DLONG curpos1 = i* mCount + j*iImageWidth;
				V3DLONG curpos2 = j* iImageWidth + k;
				temp = 0;					
				count = 0;
				for(n =1 ; n <= d  ;n++)
				{
					if (k>h*n) {temp += apsInput[curpos1 + k-(h*n)]; count++;}  
					if (k+(h*n)< iImageWidth) { temp += apsInput[curpos1 + k+(h*n)]; count++;}
                    if (j>h*n) {temp += apsInput[i* mCount + (j-(h*n))*iImageWidth + k]; count++;}//	
					if (j+(h*n)<iImageHeight) {temp += apsInput[i* mCount + (j+(h*n))*iImageWidth + k]; count++;}//
					if (i>(h*n)) {temp += apsInput[(i-(h*n))* mCount + curpos2]; count++;}//	
					if (i+(h*n)< iImageLayer) {temp += apsInput[(i+(h*n))* mCount + j* iImageWidth + k ]; count++;}
				}
				t =  apsInput[curpos]-temp/(count);
				aspOutput[curpos]= (t > 0)? t : 0;
			}
		}
	}						
}
void TipPlugin::Tipdetection(V3DPluginCallback &callback, QWidget *parent, int method_code)
{
	v3dhandle curwin = callback.currentImageWindow();
	
	int start_t = clock(); // record time point
	
	Image4DSimple* subject = callback.getImage(curwin);
	QString m_InputFileName = callback.getImageName(curwin);
	
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}	
	Image4DProxy<Image4DSimple> pSub(subject);	
	V3DLONG sx = subject->getXDim();
	V3DLONG sy = subject->getYDim();
	V3DLONG sz = subject->getZDim();
	V3DLONG pagesz_sub = sx*sy*sz;
	m_OiImgWidth = sx;
	m_OiImgHeight =sy;
	unsigned char * pData = pSub.begin();
	
	static int nDx[] = {-1,0,1,-1,0,1,-1,0,1};
	static int nDy[] = {-1,-1,-1,0,0,0,1,1,1};
	
	V3DLONG *apsInput = new V3DLONG[sx*sy*sz];
	memset(apsInput, 0, sx*sy*sz * sizeof(V3DLONG));	
	
	if(method_code == 1)
	{
		SpacePoint_t orgPoint;
		orgPoint.m_x = -1;
		orgPoint.m_y = -1;
		orgPoint.m_z = -1;
		
		unsigned char *apsInput1 = new unsigned char[sx*sy*sz];
		memset(apsInput1, 0, sx*sy*sz * sizeof(unsigned char));
		unsigned char *apsInput2 = new unsigned char[sx*sy*sz];
		memset(apsInput2, 0, sx*sy*sz * sizeof(unsigned char));
		
		int iSize = 5;		
		
		float *apfGaussTemplate3D = new float[iSize*iSize*iSize];
		memset(apfGaussTemplate3D, 0, iSize*iSize*iSize* sizeof(float));
	    
		CreateGaussFilterTemplet3D(apfGaussTemplate3D,iSize,0.8);
		
		GaussFilter3D(pData, apsInput1,sx,sy,sz,apfGaussTemplate3D,iSize);
		
		IterateSeg(apsInput1,sx,sy,sz,apsInput2);
		
//		for (V3DLONG k=0; k<sz; k++)
//		{
//			for (V3DLONG j=0; j<sy; j++)
//			{
//				for (V3DLONG i=0; i<sx; i++)
//				{
//					if (pData[k*sx*sy+j*sx+i]!=0)
//					{
//						apsInput[k*sx*sy+j*sx+i] = 255;
//						orgPoint.m_x = i;
//						orgPoint.m_y = j;
//						orgPoint.m_z = k;
//					}
//					else 
//					{
//						apsInput[k*sx*sy+j*sx+i] = BACKGROUND;
//					}
//				}
//			}
//		}
		
		
		for (V3DLONG k=0; k<sz; k++)
		{
			for (V3DLONG j=0; j<sy; j++)
			{
				for (V3DLONG i=0; i<sx; i++)
				{
					if (apsInput2[k*sx*sy+j*sx+i] != 0)
					{
						apsInput[k*sx*sy+j*sx+i] = 255;
						orgPoint.m_x = i;
						orgPoint.m_y = j;
						orgPoint.m_z = k;
					}
					else 
					{
						apsInput[k*sx*sy+j*sx+i] = BACKGROUND;
					}
				}
			}
		}
		//
		SetImageInfo1D(apsInput,sz,sx,sy);	
		Set_DFS_Seed(orgPoint);
		Initialize1D();
		SetDFB();
		SetDFS();
		if(!m_vdfsptEndPoint.size())
		{
			SearchEndPoints();
		}
		for (V3DLONG aa = 0; aa < m_vdfsptEndPoint.size(); aa++) 
		{
			DFSPoint_t p;
			p = m_vdfsptEndPoint.at(aa);
			apsInput2[p.m_z * sx*sy + p.m_y*sx + p.m_x] = 200;
			apsInput2[p.m_z *sx*sy + (p.m_y-1)*sx + p.m_x] = 200;
			apsInput2[p.m_z *sx*sy + (p.m_y)*sx + p.m_x+1] = 200;
			apsInput2[p.m_z * sx*sy + (p.m_y)*sx + p.m_x-1] = 200;
			apsInput2[p.m_z * sx*sy + (p.m_y+1)*sx + p.m_x] = 200;
		}		
		Image4DSimple p4DImage;
		p4DImage.setData(apsInput2, sx, sy, sz, 1, V3D_UINT8);
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, QString("Tip Detection"));
		callback.updateImageWindow(newwin);
		
	}else if(method_code == 2)
	{
		SpacePoint_t orgPoint;
		orgPoint.m_x = -1;
		orgPoint.m_y = -1;
		orgPoint.m_z = -1;
		for (V3DLONG k=0; k<sz; k++)
		{
			for (V3DLONG j=0; j<sy; j++)
			{
				for (V3DLONG i=0; i<sx; i++)
				{
					if (pData[k*sx*sy+j*sx+i] == 255)
					{
						apsInput[k*sx*sy+j*sx+i] = 255;
						
						orgPoint.m_x = i;
						orgPoint.m_y = j;
						orgPoint.m_z = k;
					}
					else 
					{
						apsInput[k*sx*sy+j*sx+i] = BACKGROUND;
					}
				}
			}
		}
		SetImageInfo1D(apsInput,sz,sx,sy);	
		Set_DFS_Seed(orgPoint);
		Initialize1D();
		SetDFB();
		SetDFS();
		if(!m_vdfsptEndPoint.size())
		{
			SearchEndPoints();
		}
		V3DLONG nn = m_vdfbptSurface.size();
	//	printf("size=%d\n",nn);		
		for (V3DLONG aa = 0; aa < m_vdfbptSurface.size(); aa++) 
		{
			DFBPoint_t p;
			p = m_vdfbptSurface.at(aa);
			pData[p.m_z * sx*sy + p.m_y*sx + p.m_x] = 200;
			pData[p.m_z *sx*sy + (p.m_y-1)*sx + p.m_x] = 200;
			pData[p.m_z *sx*sy + (p.m_y)*sx + p.m_x+1] = 200;
			pData[p.m_z * sx*sy + (p.m_y)*sx + p.m_x-1] = 200;
			pData[p.m_z * sx*sy + (p.m_y+1)*sx + p.m_x] = 200;
		}
		Image4DSimple p4DImage;
		p4DImage.setData(pData, sx, sy, sz, 1, V3D_UINT8);
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();
		
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, QString("Detection Souface"));
		callback.updateImageWindow(newwin);
		
	}else if(method_code == 3)
	{
		SpacePoint_t orgPoint;
		orgPoint.m_x = -1;
		orgPoint.m_y = -1;
		orgPoint.m_z = -1;
		for (V3DLONG k=0; k<sz; k++)
		{
			for (V3DLONG j=0; j<sy; j++)
			{
				for (V3DLONG i=0; i<sx; i++)
				{
					if (pData[k*sx*sy+j*sx+i] == 255)
					{
						apsInput[k*sx*sy+j*sx+i] = 255;
						
						orgPoint.m_x = i;
						orgPoint.m_y = j;
						orgPoint.m_z = k;
					}
					else 
					{
						apsInput[k*sx*sy+j*sx+i] = BACKGROUND;
					}
				}
			}
		}
		SetImageInfo1D(apsInput,sz,sx,sy);	
		Set_DFS_Seed(orgPoint);
		Initialize1D();
		//SetDFB();
		SetDFS();
		unsigned char * m_OutImgData = new  unsigned char[m_iImgCount * m_iImgWidth * m_iImgHeight];
		memset(m_OutImgData, 0, m_ulVolumeSize * sizeof(unsigned char));
		
		for (V3DLONG k=0; k< m_iImgCount; k++)
		{
			for (V3DLONG j=0; j< m_iImgHeight; j++)
			{
				for (V3DLONG i=0; i< m_iImgWidth; i++)
				{
					m_OutImgData[k* m_iImgSize + j * m_iImgWidth + i] = m_piDFS[k * m_iImgSize + j * m_iImgWidth + i];	
					//printf("DFB=%d\n",m_piDFB[k*m_iImgWidth * m_iImgHeight + j*m_iImgWidth + i]);
				}
			}
		}
		Image4DSimple p4DImage;
		p4DImage.setData(m_OutImgData, m_iImgWidth, m_iImgHeight, m_iImgCount, 1, V3D_UINT8);
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, QString(" Distance From Source"));
		callback.updateImageWindow(newwin);
	}else if(method_code == 4)
	{
		unsigned char *apsInput1 = new unsigned char[sx*sy*sz];
		memset(apsInput1, 0, sx*sy*sz * sizeof(unsigned char));
		unsigned char *apsInput2 = new unsigned char[sx*sy*sz];
		memset(apsInput2, 0, sx*sy*sz * sizeof(unsigned char));
		V3DLONG countall = 0;
		V3DLONG nn =0;
		//BinaryProcess(pData, apsInput1, sx, sy, sz, 5, 3);
//		for (V3DLONG k=0; k<sz; k++)
//			{
//				for (V3DLONG j=0; j<sy; j++)
//				{
//					for (V3DLONG i=0; i<sx; i++)
//					{
//						if (pData[k*sx*sy+j*sx+i] > 0) 
//						{
//							countall += pData[k*sx*sy+j*sx+i];
//							nn++;
//						}
//					}
//				}
//			}
//			double temp  = countall/nn;
//		    double tempd = 0;
//			for (V3DLONG k=0; k<sz; k++)
//			{
//				for (V3DLONG j=0; j<sy; j++)
//				{
//					for (V3DLONG i=0; i<sx; i++)
//					{
//						if (pData[k*sx*sy+j*sx+i] > 0) 
//						{
//							tempd += (pData[k*sx*sy+j*sx+i]-temp)*(pData[k*sx*sy+j*sx+i]-temp);
//						}
//					}
//				}
//			}
//		    double D = sqrt(tempd/nn);
//	    	printf("temp=%lf tempd=%lf countall=%d nn=%d D=%lf\n",temp,tempd,countall,nn,D);
			for (V3DLONG k=0; k<sz; k++)
			{
				for (V3DLONG j=0; j<sy; j++)
				{
					for (V3DLONG i=0; i<sx; i++)
					{
						if(pData[k*sx*sy+j*sx+i]>= 35) //temp + D)
						{
							apsInput2[k*sx*sy+j*sx+i] = 255;//pData[k*sx*sy+j*sx+i];
						}else
						{
							apsInput2[k*sx*sy+j*sx+i] = 0;
							
						}
					}
				}
			}
		
			//for (V3DLONG k=0; k<sz; k++)
//			{
//				for (V3DLONG j=0; j<(sy -1); j++)
//				{
//					for (V3DLONG i=0; i<(sx -1); i++)
//					{
//						if(apsInput2[k*sx*sy+j*sx+ i+1]+ apsInput2[k*sx*sy+j*sx+ i-1] + apsInput2[k*sx*sy+(j+1)*sx+i] + apsInput2[k*sx*sy+(j-1)*sx+i] >= 700)
//						{
//							apsInput2[k*sx*sy+j*sx+i] = 255; 
//						}else
//						{
//							apsInput2[k*sx*sy+j*sx+i] = 0;
//						}
//					}
//				}
//			}
		//IterateSeg(pData,sx,sy,sz,apsInput2);
		
		Image4DSimple p4DImage;
		p4DImage.setData((unsigned char*)apsInput2, sx, sy, sz, 1, V3D_UINT8);
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, QString(" seg image"));
		callback.updateImageWindow(newwin);	
		
	}else if(method_code == 5)
	{
		int Scount = 15;
		int Sstep = 1;
		int Sangle = 15;
		int num = 1;
		V3DLONG size;
		
		unsigned char *apsInput2 = new unsigned char[sx*sy*sz];
		memset(apsInput2, 0, sx*sy*sz * sizeof(unsigned char));
		
		for(V3DLONG j = 0 ; j < sy; j++)
		{
			for(V3DLONG i = 0 ; i < sx ; i++)
			{
				apsInput2[j*sx+i] = pData[j*sx+i];
			}
		}
		LandmarkList list = callback.getLandmark(curwin);
		if (list.size() > 0)
		{
			for(int aa = 0; aa <list.size(); aa++)
			{
				int xx= list.at(aa).x - 1;
				int yy =list.at(aa).y - 1;
				int zz =list.at(aa).z;
				printf("x=%d y=%d z=%d\n",xx,yy,zz);
				//printf("asp1=%d\n",pData[0*m_OiImgWidth*m_OiImgHeight + yy *m_OiImgWidth +xx]);
				StatisticalPixel(pData,apsInput2,xx,yy,list.at(aa).z,Scount,Sstep,Sangle,num,V3DLONG(aa));
			}
		}else 
		{
			return;
		}
	//	int w = 101;
	//	int h = 355;
	//	StatisticalPixel(pData,apsInput2,w,h,1,Scount,Sstep,Sangle,h*sx+w);		
		int iSize = 5;
		
		size = m_StatisticalPiont.size();
		
		float *apfGaussTemplate	 = new float[iSize];
		
		memset(apfGaussTemplate, 0, iSize * sizeof(float));
		
		CreateGaussFilterTemplet(apfGaussTemplate,iSize,0.8);	//0.8
		
		Borderexpand(m_StatisticalPiont.size(),m_StatisticalPiont[0].size());
	
		GaussFilter(apfGaussTemplate,iSize);
		
		Tip_Filter1(Scount);

		Tip_Filter(iSize);
	
		
		if(m_GaussFilterPiont.size()>0)
		{
			for(V3DLONG i = 0 ; i < m_GaussFilterPiont.size(); i++)
			{
				for(V3DLONG j = 0 ; j < m_GaussFilterPiont[i].size(); j++)
				{
					//printf("i=%d j=%d gusspixel=%d \n",i,j,m_GaussFilterPiont[i][j].pixel);
				}
			}
		}
		
		for(V3DLONG i = 0 ; i < m_DetectionPiont.size(); i++)
		{
			for(V3DLONG j = 0 ; j < m_DetectionPiont[i].size(); j++)
			{
				//printf("i=%d j=%d detection=%d \n",i,j,m_DetectionPiont[i][j].pixel);
			}
		}		
	//	V3DLONG px = m_GaussFilterPiont[0].size()+10;
//		V3DLONG py = m_GaussFilterPiont.size()*(Scount)+50;
		V3DLONG px = m_DetectionPiont[0].size()+10;
		V3DLONG py = m_DetectionPiont.size()*(Scount)+50;
		//printf("px=%d py=%d \n",px,py);
		
		unsigned char *apsInput3 = new unsigned char[px*py*sz];
		memset(apsInput3, 0, px*py*sz * sizeof(unsigned char));
		
		for(int pi = 0; pi<m_StatisticalPiont.size(); pi++)
		{
			for(int pj = 0 ; pj < m_StatisticalPiont[pi].size(); pj++)
			{
				int k = m_StatisticalPiont[pi][pj].pixel;
			//	printf("pj=%d k=%d\n",pj,k);
			}
		}
		for(int pi = 0; pi<m_DetectionPiont.size(); pi++)
		{
			for(int pj = 0 ; pj < m_DetectionPiont[pi].size(); pj++)
			{
				int k = int(m_DetectionPiont[pi][pj].pixel/255 +0.5);
			//	printf("k=%d\n",k);
				for(int n  = 0 ; n < k ; n++)
				{
					apsInput3[(Scount*(pi+1)-n)*px +pj]= 255;
				}
			}
		}
		
		Image4DSimple p4DImage;
		p4DImage.setData(apsInput2, sx, sy, 1, 1, V3D_UINT8);//ray
		//p4DImage.setData(apsInput3, px, py, 1, 1, V3D_UINT8);
		
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, QString("ray image"));
		callback.updateImageWindow(newwin);	
		
	}else if(method_code == 6)
	{
		unsigned char *apsInput1 = new unsigned char[sx*sy*sz];
		memset(apsInput1, 0, sx*sy*sz * sizeof(unsigned char));
		
		unsigned char *Pxoy = new unsigned char[sy*sx];
		memset(Pxoy, 0 , sx*sy*sizeof(unsigned char));
		
		unsigned char* mark = new unsigned char[sy*sx];
		memset(mark, 0 , sx*sy*sizeof(unsigned char));
		V3DLONG i,j,k;
		
		for ( k=0; k<sz; k++)
		{
			for ( j=0; j<sy; j++)
			{
				for ( i=0; i<sx; i++)
				{
					if(pData[k*sx*sy+j*sx+i]> 35) //temp + D)//30.ok
					{
						apsInput1[k*sx*sy+j*sx+i] = 255;//pData[k*sx*sy+j*sx+i];
					}else
					{
						apsInput1[k*sx*sy+j*sx+i] = 0;
						
					}
				}
			}
		}
		for ( k=0; k<sz; k++)
		{
			for ( j=0; j<sy; j++)
			{
				for ( i=0; i<sx; i++)
				{
					if(apsInput1[k*sx*sy+j*sx+i]==255)
					{
						Pxoy[j*sx+i] = 255;
					}
				}
			}
		}
		
		for (V3DLONG j=0; j<sy; j++)
		{
			for (V3DLONG i=0; i<sx; i++)
			{
				if (Pxoy[j*sx+i] == 255 )
				{
					//if(Pxoy[j*sx+i+1] == 0 && Pxoy[j*sx+i-1] == 0 && Pxoy[(j+1)*sx+i]==0 && Pxoy[(j-1)*sx+i]==0)
					if(Pxoy[j*sx+i+1] == 0 && Pxoy[j*sx+i-1] == 0 || Pxoy[(j+1)*sx+i] == 0 && Pxoy[(j-1)*sx+i-1] == 0 )
					{
						Pxoy[j*sx+i] = 0;
					}else 
					{
						Pxoy[j*sx+i]=255;
					}
				}
			}
		}
		int iSize = 3;
		unsigned char *apsOutput = new unsigned char[sx*sy*sz];
		memset(apsOutput, 0, sx*sy*sz * sizeof(unsigned char));
		
		float *apfGaussTemplate2D = new float[iSize*iSize];
		
		memset(apfGaussTemplate2D, 0, iSize*iSize* sizeof(float));
		
		CreateGaussFilterTemplet2D(apfGaussTemplate2D,iSize,0.8);	//0.8, 0.5*r+0.25
		
		GaussFilter2D(Pxoy, apsOutput,sx,sy,sz,apfGaussTemplate2D,iSize);
	
		for ( j=0; j<sy; j++)
		{
			for ( i=0; i<sx; i++)
			{
				if(apsOutput[j*sx+i]>= 35) //temp + D)
				{
					apsInput1[j*sx+i] = 255;//pData[k*sx*sy+j*sx+i];
				}else
				{
					apsInput1[j*sx+i] = 0;
					
				}
			}
		}
		Image4DSimple p4DImage;
	    p4DImage.setData(apsInput1, sx, sy, 1, 1, V3D_UINT8);	
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, QString("xoy image"));
		callback.updateImageWindow(newwin);	
		if (apsInput1) 
		{
			//delete []apsInput1;
			//apsInput1 =NULL;
		}
		if (mark) 
		{
			delete []mark;
			mark =NULL;
		}
	}else if(method_code == 7) 
	{
		unsigned char *apsInput1 = new unsigned char[sx*sy*sz];
		memset(apsInput1, 0, sx*sy*sz * sizeof(unsigned char));
		
		unsigned char *Pxoz = new unsigned char[1*sz*sx];
		memset(Pxoz, 0 , sx*sz*sizeof(unsigned char));
		
		unsigned char *mark = new unsigned char[sz*sy];
		memset(mark, 0 , sz*sy*sizeof(unsigned char));
		
		for (V3DLONG k=0; k<sz; k++)
		{
			for (V3DLONG j=0; j<sy; j++)
			{
				for (V3DLONG i=0; i<sx; i++)
				{
					if(pData[k*sx*sy+j*sx+i]>= 35) //temp + D)
					{
						apsInput1[k*sx*sy+j*sx+i] = 255;//pData[k*sx*sy+j*sx+i];
					}else
					{
						apsInput1[k*sx*sy+j*sx+i] = 0;
						
					}
				}
			}
		}
		for (V3DLONG k=0; k<sy; k++)
		{
			for (V3DLONG j=0; j<sz; j++)
			{
				for (V3DLONG i=0; i<sx; i++)
				{
					if(apsInput1[j*sx*sy+k*sx+i]== 255)
					{
						Pxoz[j*sx+i] = 255;
					}
				}
			}
		}
		for (V3DLONG j=0; j<sz; j++)
		{
			for (V3DLONG i=0; i<sx; i++)
			{
				if (Pxoz[j*sx+i] == 255 )
				{
					//if(Pxoy[j*sx+i+1] == 0 && Pxoy[j*sx+i-1] == 0 && Pxoy[(j+1)*sx+i]==0 && Pxoy[(j-1)*sx+i]==0)
					if(Pxoz[j*sx+i+1] == 0 && Pxoz[j*sx+i-1] == 0 || Pxoz[(j+1)*sx+i] == 0 && Pxoz[(j-1)*sx+i-1] == 0 )
					{
						Pxoz[j*sx+i] = 0;
					}else 
					{
						Pxoz[j*sx+i]=255;
					}
				}
			}
		}
		
        for (V3DLONG j=1; j<sz-1; j++)
		{
			for (V3DLONG i=1; i<sx-1; i++)
			{
				if( Pxoz[j*sx+i+1] != 0 
				   ||Pxoz[j*sx+i-1] != 0 
				   ||Pxoz[(j+1)*sx+i] != 0 
				   ||Pxoz[(j-1)*sx+i] != 0 
				   ||Pxoz[(j-1)*sx+i-1] != 0 
				   ||Pxoz[(j-1)*sx+i+1] != 0 
				   ||Pxoz[(j+1)*sx+i+1] != 0 
				   ||Pxoz[(j+1)*sx+i-1] != 0 
				   )
				{
					mark[j*sx+i]=1;
				}
			}
		}
		for (V3DLONG j=0; j<sz; j++)
		{
			for (V3DLONG i=0; i<sx; i++)
			{
				if(mark[j*sx+i]==1)
				{
					Pxoz[j*sx+i] == 0;
				}
			}
		}
		for (V3DLONG j=1; j<sz-1; j++)
		{
			for (V3DLONG i=1; i<sx-1; i++)
			{
				if (Pxoz[j*sx+i]==255 && Pxoz[(j+1)*sx+i]==0 && Pxoz[(j-1)*sx+i]==0) 
				{
					mark[j*sx+i] == 2;
				}
			}
		}
		
		for (V3DLONG j=0; j<sz; j++)
		{
			for (V3DLONG i=0; i<sx; i++)
			{
				if (Pxoz[j*sx+i]==255 && Pxoz[(j)*sx+i+1]==0 && Pxoz[j*sx+i-1]==0) 
				{
					mark[j*sx+i] == 3;
				}
			}
		}
		for (V3DLONG j=0; j<sz; j++)
		{
			for (V3DLONG i=0; i<sx; i++)
			{
				if(mark[j*sx+i]==2 || mark[j*sx+i]==3)
				{
					Pxoz[j*sx+i] == 0;
				}
			}
		}
		Image4DSimple p4DImage;
		p4DImage.setData(Pxoz, sx, sz, 1, 1, V3D_UINT8);
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, QString("xoz image"));
		callback.updateImageWindow(newwin);	
		if (apsInput1) 
		{
			delete []apsInput1;
			apsInput1 =NULL;
		}
		if (mark) 
		{
			delete []mark;
			mark =NULL;
		}
		
	}else if(method_code == 8) 
	{
		unsigned char *apsInput1 = new unsigned char[sx*sy*sz];
		memset(apsInput1, 0, sx*sy*sz * sizeof(unsigned char));
		
		unsigned char *Pzoy = new unsigned char[sz*sy];
		memset(Pzoy, 0 , sz*sy*sizeof(unsigned char));
		
		unsigned char *mark = new unsigned char[sz*sy];
		memset(mark, 0 , sz*sy*sizeof(unsigned char));
		
		for (V3DLONG k=0; k<sz; k++)
		{
			for (V3DLONG j=0; j<sy; j++)
			{
				for (V3DLONG i=0; i<sx; i++)
				{
					if(pData[k*sx*sy+j*sx+i]>= 35) //temp + D)
					{
						apsInput1[k*sx*sy+j*sx+i] = 255;//pData[k*sx*sy+j*sx+i];
					}else
					{
						apsInput1[k*sx*sy+j*sx+i] = 0;
						
					}
				}
			}
		}
			
		for (V3DLONG k=0; k<sx; k++)
		{
			for (V3DLONG j=0; j<sz; j++)
			{
				for (V3DLONG i=0; i<sy; i++)
				{
					if(apsInput1[j*sy*sx+i*sx+k]==255)
					{
						Pzoy[j*sy+i] =255;
					}
				}
			}
		}
		for (V3DLONG j=0; j<sz; j++)
		{
			for (V3DLONG i=0; i<sy; i++)
			{
				if (Pzoy[j*sy+i] == 255 )
				{
					//if(Pxoy[j*sx+i+1] == 0 && Pxoy[j*sx+i-1] == 0 && Pxoy[(j+1)*sx+i]==0 && Pxoy[(j-1)*sx+i]==0)
					if(Pzoy[j*sy+i+1] == 0 && Pzoy[j*sy+i-1] == 0 )//|| Pzoy[(j-1)*sy+i] == 0 && Pzoy[(j+1)*sy+i])
					{
						Pzoy[j*sy+i] = 0;
					}else 
					{
						Pzoy[j*sy+i]=255;
					}
				}
			}
		}
		
		
		for (V3DLONG j=1; j<sz-1; j++)
		{
			for (V3DLONG i=1; i<sy-1; i++)
			{
				if( Pzoy[j*sy+i+1] != 0 
				   ||Pzoy[j*sy+i-1] != 0 
				   ||Pzoy[(j+1)*sy+i] != 0 
				   ||Pzoy[(j-1)*sy+i] != 0 
				   ||Pzoy[(j-1)*sy+i-1] != 0 
				   ||Pzoy[(j-1)*sy+i+1] != 0 
				   ||Pzoy[(j+1)*sy+i+1] != 0 
				   ||Pzoy[(j+1)*sy+i-1] != 0 
				   )
				{
					mark[j*sy+i]=1;
				}
			}
		}
		for (V3DLONG j=0; j<sz; j++)
		{
			for (V3DLONG i=0; i<sy; i++)
			{
				if(mark[j*sy+i]==1)
				{
					Pzoy[j*sy+i] == 0;
				}
			}
		}
		for (V3DLONG j=1; j<sz-1; j++)
		{
			for (V3DLONG i=1; i<sy-1; i++)
			{
				if (Pzoy[j*sy+i]==255 && Pzoy[(j+1)*sy+i]==0 && Pzoy[(j-1)*sy+i]==0) 
				{
					mark[j*sy+i] == 2;
				}
			}
		}
		
		for (V3DLONG j=0; j<sz; j++)
		{
			for (V3DLONG i=0; i<sy; i++)
			{
				if (Pzoy[j*sy+i]==255 && Pzoy[(j)*sy+i+1]==0 && Pzoy[j*sy+i-1]==0) 
				{
					mark[j*sy+i] == 3;
				}
			}
		}
		for (V3DLONG j=0; j<sz; j++)
		{
			for (V3DLONG i=0; i<sy; i++)
			{
				if(mark[j*sy+i]==2 || mark[j*sy+i]==3)
				{
					Pzoy[j*sy+i] == 0;
				}
			}
		}
		Image4DSimple p4DImage;
		p4DImage.setData(Pzoy, sy, sz, 1, 1, V3D_UINT8);
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, QString("zoy image"));
		callback.updateImageWindow(newwin);
		if (apsInput1) 
		{
			delete []apsInput1;
			apsInput1 =NULL;
		}
		if (mark) 
		{
			delete []mark;
			mark =NULL;
		}
		
	}else if(method_code == 9)
	{
		unsigned char *apsInput1 = new unsigned char[sx*sy*sz];
		memset(apsInput1, 0, sx*sy*sz * sizeof(unsigned char));	
		unsigned char *apsInput2 = new unsigned char[sx*sy*sz];
		memset(apsInput2, 0, sx*sy*sz * sizeof(unsigned char));
		unsigned char *apsInput3 = new unsigned char[sx*sy*sz];
		memset(apsInput3, 0, sx*sy*sz * sizeof(unsigned char));

		V3DLONG x,y;
		int Scount = 15;
		int Sstep = 1;
		int Sangle = 15;
		V3DLONG index = 0;
		int num = 3;
		V3DLONG i,j,size;
		SpacePoint_t point;
		vector<SpacePoint_t> temp;
		
		bool find;
		int iSize = 5;
		for(j = 1; j < sy-1; j++)
		{
			for(i = 1; i < sx-1; i++)
			{
				if(pData[j*sx+i]==255)
				{
					apsInput1[j*sx+i] = 100;
					index = sx * j + i;
					find = false;
					for(int ii=-1;ii<2;ii++)
						for(int jj=-1;jj<2;jj++)
					{
						y = j+ii;
						x = i+jj;
						if(pData[sx * y + x] == 0)
						{
							find = true;
						}
					}
					if(find)
					{
						StatisticalPixel(pData,apsInput2,i,j,1,Scount,Sstep,Sangle,num,j*sx+i);
					}
				}
			}
		}
		size = m_StatisticalPiont.size();
		
		float *apfGaussTemplate	 = new float[iSize];
		
		memset(apfGaussTemplate, 0, iSize * sizeof(float));
		
		CreateGaussFilterTemplet(apfGaussTemplate,iSize,0.8);	//0.8
		
		Borderexpand(m_StatisticalPiont.size(),m_StatisticalPiont[0].size());
		
		GaussFilter(apfGaussTemplate,iSize);
		
		Tip_Filter1(Scount);
		
		Tip_Filter(iSize);
		
		LandmarkList markList;
		LocationSimple pp;
		
		if (m_TipP1.size()>0)
		{
			for(V3DLONG ii = 0; ii <m_TipP1.size(); ii++)
			{
				V3DLONG xx = m_TipP1[ii].m_x;
				V3DLONG yy = m_TipP1[ii].m_y;
				V3DLONG zz = m_TipP1[ii].m_z;
				
				V3DLONG weight = m_TipP[ii].weight;
				V3DLONG len = m_TipP[ii].cycle;
				V3DLONG area = m_TipP[ii].area;
				double var = m_TipP[ii].var;
				
				V3DLONG area1 = m_TipP1[ii].area;
				V3DLONG weight1 = m_TipP1[ii].weight;
				V3DLONG len1 = m_TipP1[ii].cycle;
				if (weight1==1  && area1< 6 && area1>0 && var < 2000)//4 for xoy,6
				{
					pp.x = xx;
					pp.y = yy;
					pp.z = zz;
					point.m_x = xx;
					point.m_y = yy;
					point.m_z = zz;
					temp.push_back(point);
					markList << pp;
					apsInput1[yy*sx+xx]= 20;
					printf("xx=%d yy=%d zz=%d area=%ld weight1=%d\n",xx,yy,zz,area1,weight1);
				}
			}
		}
		Image4DSimple p4DImage;
		//p4DImage.setData(apsInput2, sx, sy, 1, 1, V3D_UINT8);//ray
		p4DImage.setData(apsInput1, sx, sy, 1, 1, V3D_UINT8);
		//p4DImage.setData(apsInput3, sx, sy, 1, 1, V3D_UINT8);
		//p4DImage.setData(apsOutput, sx, sy, 1, 1, V3D_UINT8);
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
	//	callback.setLandmark(newwin, markList);
		callback.setImageName(newwin, QString("Weight image"));
		callback.updateImageWindow(newwin);	
		//callback.pushObjectIn3DWindow(newwin);
		
		if (apsInput1) 
		{
			//delete []apsInput1;
			//apsInput1 =NULL;
		}
		if (apsInput2) 
		{
			delete []apsInput2;
			apsInput1 =NULL;
		}
		if (apsInput3) 
		{
			delete []apsInput3;
			apsInput1 =NULL;
		}

	}else if (method_code == 10)
	{
		m_iImgCount = sz;
		
		m_iImgWidth = sx;
		
		m_iImgHeight = sy;		
		
		m_iImgSize = m_iImgWidth * m_iImgHeight;
		
		unsigned char *apsInput1 = new unsigned char[sx*sy*sz];
		memset(apsInput1, 0, sx*sy*sz * sizeof(unsigned char));			
		
		V3DLONG *apsInput2 = new V3DLONG[sx*sy*sz];
		memset(apsInput2, 0, sx*sy*sz * sizeof(V3DLONG));		
		
		unsigned char *apsInput3 = new unsigned char[sx*sy*sz];
		memset(apsInput3, 0, sx*sy*sz * sizeof(unsigned char));
		
		m_ppsImgData2D = new V3DLONG[sx * sy];
//		
		memset(m_ppsImgData2D, BACKGROUND, sx*sy*sizeof(V3DLONG));
		
		SpacePoint_t orgPoint;
		orgPoint.m_x = -1;
		orgPoint.m_y = -1;
		orgPoint.m_z = -1;
		for (V3DLONG j=0; j<sy; j++)
		{
			for (V3DLONG i=0; i<sx; i++)
			{
				apsInput1[j*sx+i] = pData[j*sx+i];
				
				if (pData[j*sx+i]==255)
				{
					m_ppsImgData2D[j*sx+i] = 255;
					//apsInput2[j*sx+i] = 255;
					orgPoint.m_x = i;
					orgPoint.m_y = j;
					orgPoint.m_z = 1;
				}
				else 
				{
					m_ppsImgData2D[j*sx+i] = BACKGROUND;
				//	apsInput2[j*sx+i] = BACKGROUND;
				}
			}
		}
		//printf("orgx=%d orgy=%d\n",orgPoint.m_x,orgPoint.m_y);
	//	SetImageInfo1D(apsInput2,sz,sx,sy);	
		Set_DFS_Seed(orgPoint);
	//	Initialize2D();
		SetDFB2D();
	//	SetDFS2D();
	    GenSourceField();
		
		if(!m_vdfsptEndPoint.size())
		{
			SearchEndPoints2D();
		}
		
		/////////////////////////////;
		int Scount = 15;
		int Sstep = 1;
		int Sangle = 15;
		V3DLONG index = 0;
		int num = 3;
		int iSize = 5;
		for (V3DLONG aa = 0; aa < m_vdfsptEndPoint.size(); aa++) 
		{
			DFSPoint_t p;
			
			p = m_vdfsptEndPoint.at(aa);
			
	        StatisticalPixel(pData,apsInput3,p.m_x,p.m_y,1,Scount,Sstep,Sangle,num,p.m_y*sx+p.m_x);	
			
		}
		
	    V3DLONG	size = m_StatisticalPiont.size();
		
		float *apfGaussTemplate	 = new float[iSize];
		
		memset(apfGaussTemplate, 0, iSize * sizeof(float));
		
		CreateGaussFilterTemplet(apfGaussTemplate,iSize,0.8);	//0.8
		
		Borderexpand(m_StatisticalPiont.size(),m_StatisticalPiont[0].size());
		
		GaussFilter(apfGaussTemplate,iSize);
		
		Tip_Filter1(Scount);
		
		Tip_Filter(iSize);
		
		LandmarkList markList;
		LocationSimple pp;
		
		if (m_TipP1.size()>0)
		{
			for(V3DLONG ii = 0; ii <m_TipP1.size(); ii++)
			{
				V3DLONG xx = m_TipP1[ii].m_x;
				V3DLONG yy = m_TipP1[ii].m_y;
				V3DLONG zz = m_TipP1[ii].m_z;
				
				V3DLONG weight = m_TipP[ii].weight;
				V3DLONG len = m_TipP[ii].cycle;
				V3DLONG area = m_TipP[ii].area;
				double var = m_TipP[ii].var;
				
				V3DLONG area1 = m_TipP1[ii].area;
				V3DLONG weight1 = m_TipP1[ii].weight;
				V3DLONG len1 = m_TipP1[ii].cycle;
				if (weight==1  && area1 > 0 && area1 < 6) //4 for xoy,6
				{
					pp.x = xx;
					pp.y = yy;
					pp.z = zz;
					markList << pp;
					apsInput1[yy*sx+xx]= 20;
					printf("xx=%d yy=%d zz=%d area=%ld weight1=%d\n",xx,yy,zz,area1,weight1);
				}
			}
		}
		
		
		apsInput1[orgPoint.m_y*sx + orgPoint.m_x] = 20;
		for (V3DLONG aa = 0; aa < m_vdfsptEndPoint.size(); aa++) 
		{
			DFSPoint_t p;
			p = m_vdfsptEndPoint.at(aa);
			 apsInput1[orgPoint.m_y*sx + orgPoint.m_x] = 200;
			 apsInput1[(p.m_y)*sx + p.m_x] = 200;
		//	printf("xx=%d yy=%d enpoint=%d\n",p.m_x,p.m_y,p.m_l);
		}
		unsigned char * m_OutImgData = new  unsigned char[m_iImgWidth * m_iImgHeight];
		memset(m_OutImgData, 0, m_iImgWidth * m_iImgHeight * sizeof(unsigned char));

		for (V3DLONG j=0; j< m_iImgHeight; j++)
		{
			for (V3DLONG i=0; i< m_iImgWidth; i++)
			{
				m_OutImgData[ j * m_iImgWidth + i] = m_piDFS[ j * m_iImgWidth + i];	
				//printf("x=%ld y=%ld DFs=%ld\n",i,j,m_piDFS[j * m_iImgWidth + i]);
			}
		}
		for (V3DLONG aa = 0; aa < m_vdfsptEndPoint.size(); aa++) 
		{
			DFSPoint_t p;
			p = m_vdfsptEndPoint.at(aa);
			m_OutImgData[(p.m_y)*sx + p.m_x] = 200;
		
		}		
		Image4DSimple p4DImage;
		//p4DImage.setData(m_OutImgData, m_iImgWidth, m_iImgHeight, sz, 1, V3D_UINT8);
		p4DImage.setData(apsInput1, sx, sy, sz, 1, V3D_UINT8);
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, QString("Tip Detection"));
		callback.updateImageWindow(newwin);
	}else if(method_code == 11)
	{
		int iSize = 3;
		unsigned char *apsOutput = new unsigned char[sx*sy*sz];
		memset(apsOutput, 0, sx*sy*sz * sizeof(unsigned char));
		unsigned char *apsInput1 = new unsigned char[sx*sy*sz];
		memset(apsInput1, 0, sx*sy*sz * sizeof(unsigned char));
		
		float *apfGaussTemplate2D = new float[iSize*iSize];
		
		memset(apfGaussTemplate2D, 0, iSize*iSize* sizeof(float));
		
		CreateGaussFilterTemplet2D(apfGaussTemplate2D,iSize,0.8);	//0.8, 0.5*r+0.25
		
		GaussFilter2D(pData, apsOutput,sx,sy,sz,apfGaussTemplate2D,iSize);
		
		for (V3DLONG k=0; k<sz; k++)
		{
			for (V3DLONG j=0; j<sy; j++)
			{
				for (V3DLONG i=0; i<sx; i++)
				{
					if(apsOutput[k*sx*sy+j*sx+i]> 35) //temp + D)
					{
						apsInput1[k*sx*sy+j*sx+i] = 255;//pData[k*sx*sy+j*sx+i];
					}else
					{
						apsInput1[k*sx*sy+j*sx+i] = 0;
						
					}
				}
			}
		}
		Image4DSimple p4DImage;
	  //  p4DImage.setData(apsOutput, sx, sy, 1, 1, V3D_UINT8);	
		p4DImage.setData(apsInput1, sx, sy, 1, 1, V3D_UINT8);
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();
		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, QString("gaussian image"));
		callback.updateImageWindow(newwin);	
		
		if (apsInput1) 
		{
		//	delete []apsInput1;
		//	apsInput1 =NULL;
		}
		if (apsOutput) 
		{
		//	delete []apsOutput;
		//	apsInput1 =NULL;
		}		
		
	}

}
void TipPlugin::Doublelinear_inserting (unsigned char*apsInput,V3DLONG &polValue,float x0, float y0, V3DLONG z0, V3DLONG currX, V3DLONG currY)
{
	//V3DLONG mCount = m_OiImgWidth * m_OiImgHeight;
  //  printf("m_OiImgWidth=%d m_OiImgHeight=%d x0=%lf y0=%lf z0=%d\n",m_OiImgWidth,m_OiImgHeight,x0,y0,z0);
	//printf("x00=%d currx=%d curry=%d\n",apsInput[currY * m_OiImgWidth + currX],currX,currY);	
	float deltaX, deltaY;
	deltaX = x0 - currX;
	deltaY = y0 - currY;
//	printf("deltax=%lf deltay=%lf\n",deltaX,deltaY);
	float x0y0 = (1-deltaX) * (1-deltaY);
	float x1y0 = deltaX * (1-deltaY);
	float x1y1 = deltaX * deltaY;
	float x0y1 = (1-deltaX) * deltaY;
	V3DLONG x = currX;
	V3DLONG y = currY;
	//printf("x=%d y=%d\n",x,y);
    polValue = (x0y0 * apsInput[y * m_OiImgWidth + x] +  
				x1y0 * apsInput[y * m_OiImgWidth + x +1]+
				x1y1 * apsInput[(y +1)* m_OiImgWidth + x +1] +
				x0y1 * apsInput[(y +1) * m_OiImgWidth + x ]
				);
	//printf("x0y0=%lf x1y0=%lf x1y1=%lf x0y1=%lf pixel=%d polvalue=%d\n",x0y0,x1y0,x1y1,x0y1,apsInput[currY * m_OiImgWidth + currX],polValue);	
}
void TipPlugin::Canny(V3DPluginCallback &callback, QWidget *parent, int method_code)
{
	
	v3dhandle curwin = callback.currentImageWindow();
	
	int start_t = clock(); // record time point
	V3DLONG i,j,k;
	
	Image4DSimple* subject = callback.getImage(curwin);

	QString m_InputFileName = callback.getImageName(curwin);
	
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}	
	Image4DProxy<Image4DSimple> pSub(subject);	
	V3DLONG sx = subject->getXDim();
	V3DLONG sy = subject->getYDim();
	V3DLONG sz = subject->getZDim();
	V3DLONG pagesz_sub = sx*sy*sz;
	unsigned char * pData = pSub.begin();
	
	unsigned char *apsInput = new unsigned char[sx*sy*sz];
	memset(apsInput, 0, sx*sy*sz * sizeof(unsigned char));
	
	int iSize = 3;
	
	unsigned char *apsOutput = new unsigned char[sx*sy*sz];
	memset(apsOutput, 0, sx*sy*sz * sizeof(unsigned char));
	
	unsigned char *apsInput1 = new unsigned char[sx*sy*sz];
	memset(apsInput1, 0, sx*sy*sz * sizeof(unsigned char));
	
	float *apfGaussTemplate3D = new float[iSize*iSize*iSize];
	
	memset(apfGaussTemplate3D, 0, iSize*iSize*iSize* sizeof(float));
	CreateGaussFilterTemplet3D(apfGaussTemplate3D,iSize,0.8);

	GaussFilter3D(pData, apsInput,sx,sy,sz,apfGaussTemplate3D,iSize);
	
	V3DLONG *pGradX = new V3DLONG[sx*sy*sz];
	memset(pGradX, 0, sx*sy*sz * sizeof(V3DLONG));
	
	V3DLONG *pGradY = new V3DLONG[sx*sy*sz];
	memset(pGradY, 0, sx*sy*sz * sizeof(V3DLONG));
	
	unsigned char *Gradient = new unsigned char[sx*sy*sz];
	memset(Gradient, 0, sx*sy*sz*sizeof(unsigned char));
	
	unsigned char *NonMax = new unsigned char[sx*sy*sz];
	memset(NonMax, 0, sx*sy*sz*sizeof(unsigned char));
	
	Getgrad(sx,sy,sz,apsInput,pGradX,pGradY,Gradient,NonMax);		

	DetectCannyEdges(sx,sy,sz,apsInput1,pGradX,pGradY,Gradient,NonMax);    	

		
	Image4DSimple p4DImage;
	p4DImage.setData(apsInput1, sx, sy, sz, 1, V3D_UINT8);
	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
		newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();
	callback.setImage(newwin, &p4DImage);
	callback.setImageName(newwin, QString("gaussian3D image"));
	callback.updateImageWindow(newwin);	
	
	if (pGradX) 
	{
		delete []pGradX;
		pGradX =NULL;
	}
	if (pGradY) 
	{
		delete []pGradY;
		pGradY =NULL;
	}
	
	if (Gradient) 
	{
		delete []Gradient;
		Gradient =NULL;
	}
	if (NonMax) 
	{
		delete []NonMax;
		NonMax =NULL;
	}
	if (apsInput) 
	{
		//delete []apsInput;
		//apsInput =NULL;
	}
	
}

void TipPlugin::Getgrad(V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount, unsigned char*apsInput,V3DLONG *pGradX, V3DLONG *pGradY, unsigned char *Gradient,unsigned char*NonMax)
{
	V3DLONG j,i,k;
	double dSqt1;
	double dSqt2;
	for(k = 0; k<m_iCount; k++)
	{
		for(j=1;j<m_iHei-1;j++)
		{
			for(i=1;i<m_iWid-1;i++)
			{
				V3DLONG cur = k*m_iWid*m_iHei+j*m_iWid+i;
			//	V3DLONG px = apsInput[cur+1]-apsInput[cur-1];
			//	V3DLONG py = apsInput[k*m_iWid*m_iHei+(j+1)*m_iWid +i] - apsInput[k*m_iWid*m_iHei+(j-1)*m_iWid +i];
				//V3DLONG p45 = apsInput[k*m_iWid*m_iHei+(j+1)*m_iWid +i-1] - apsInput[k*m_iWid*m_iHei+(j-1)*m_iWid +i+1];
				//V3DLONG p135 = apsInput[k*m_iWid*m_iHei+(j+1)*m_iWid +i+1] - apsInput[k*m_iWid*m_iHei+(j-1)*m_iWid +i-1];
				//pGradX[cur] = px+(p45+p135)/2;
				//pGradY[cur] = py+(p45-p135)/2;
				
				pGradX[cur] = apsInput[cur+1]-apsInput[cur-1];
				pGradY[cur] = apsInput[k*m_iWid*m_iHei+(j+1)*m_iWid +i] - apsInput[k*m_iWid*m_iHei+(j-1)*m_iWid +i];
				dSqt1 = pGradX[cur]*pGradX[cur];
				dSqt2 = pGradY[cur]*pGradY[cur];
				Gradient[cur] = (sqrt(dSqt1+dSqt2)+0.5);
				//NonMax[cur]=Gradient[cur];
			}
		}		
		
	}
	
}
void TipPlugin::DetectCannyEdges(V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount, unsigned char*apsInput,V3DLONG *pGradX, V3DLONG *pGradY, unsigned char *Gradient,unsigned char *NonMax)
{
	
	float Tangent;
	
	float PI=3.1416;
	
	V3DLONG k , j ,i,cur;
	
	V3DLONG nThrHigh,nThrLow;
	
	V3DLONG gx,gy;
	
	V3DLONG g1,g2,g3,g4;
	
	double weight;
	
	double Tmp,Tmp1,Tmp2;
	
	nThrLow = 15;
	nThrHigh = 25;
	bool type = false;
	if(type) 
	{
		for(k = 0; k<m_iCount; k++)
		{
			for(j=1;j<m_iHei-1;j++)
			{
				for(i=1;i<m_iWid-1;i++)
				{
					cur = k*m_iWid*m_iHei+j*m_iWid+i;
					if (pGradX[cur] == 0)
						Tangent = 90;
					else
						Tangent = (float)(atan(pGradY[cur]/pGradX[cur]) * 180 / PI); //rad to degree
				    
					//Horizontal Edge
					if (((-22.5 < Tangent) && (Tangent <= 22.5)) || ((157.5 < Tangent) && (Tangent <= -157.5)))
					{
						if ((Gradient[cur] < Gradient[k*m_iWid*m_iHei+(j+1)*m_iWid+i]) || (Gradient[cur] < Gradient[k*m_iWid*m_iHei+(j-1)*m_iWid+i]))
							NonMax[cur] = 0;
						else 
						{
							NonMax[cur] = 128;
							
						}
						
					}
					//Vertical Edge
					if (((-112.5 < Tangent) && (Tangent <= -67.5)) || ((67.5 < Tangent) && (Tangent <= 112.5)))
					{
						if ((Gradient[cur] < Gradient[k*m_iWid*m_iHei+j*m_iWid+i+1]) || (Gradient[cur] < Gradient[k*m_iWid*m_iHei+j*m_iWid+i]-1))
							NonMax[cur] = 0;
						else 
						{
							NonMax[cur] = 128;
							
						}
					}
					//-45 Degree Edge
					if (((-67.5 < Tangent) && (Tangent <= -22.5)) || ((112.5 < Tangent) && (Tangent <= 157.5)))
					{
						if ((Gradient[cur] < Gradient[k*m_iWid*m_iHei+(j-1)*m_iWid+i-1]) || (Gradient[cur]< Gradient[k*m_iWid*m_iHei+(j+1)*m_iWid+i-1]))
							NonMax[cur] = 0;
						else 
						{
							NonMax[cur] = 128;
							
						}
					}
					
					//+45 Degree Edge
					if (((-157.5 < Tangent) && (Tangent <= -112.5)) || ((67.5 < Tangent) && (Tangent <= 22.5)))
					{
						if ((Gradient[cur]< Gradient[k*m_iWid*m_iHei+(j+1)*m_iWid+i+1]) || (Gradient[cur] < Gradient[k*m_iWid*m_iHei+(j-1)*m_iWid+i-1]))
							NonMax[cur] = 0;
						else 
						{
							NonMax[cur] = 128;
							
						}
					}
					
				}
			}		
			
		}
	}else 	
	{
		for(k = 0; k<m_iCount; k++)
		{
			for(j=1;j<m_iHei-1;j++)
			{
				for(i=1;i<m_iWid-1;i++)
				{
					cur = k*m_iWid*m_iHei+j*m_iWid+i;
					if(Gradient[cur] == 0)
					{
						NonMax[cur] = 0;
					}
					else
					{
						Tmp = Gradient[cur];
						gx = pGradX[cur];
						gy = pGradY[cur];
						if(abs(gy) > abs(gx))
						{
							weight = fabs(gx)/fabs(gy);
							
							g2 = Gradient[cur-m_iWid];
							g4 = Gradient[cur+m_iWid];
							//g1 g2
							//      C
							//       g4 g3
							if(gx*gy>0)
							{
								g1 = Gradient[cur-m_iWid-1];
								g3 = Gradient[cur+m_iWid+1];
							}
							//       g2 g1
							//        C
							//    g3 g4
							else
							{
								g1 = Gradient[cur-m_iWid+1];
								g3 = Gradient[cur+m_iWid-1];
							}
						}
						else
						{
							weight = fabs(gy)/fabs(gx);
							
							g2 = Gradient[cur+1];
							g4 = Gradient[cur-1];
							
							//  g3
							//  g4 C g2
							//       g1
							if(gx * gy > 0)
							{
								g1 = Gradient[cur+m_iWid+1];
								g3 = Gradient[cur-m_iWid-1];
							}
							
							//         g1
							//    g4 C g2
							//    g3
							else
							{
								g1 = Gradient[cur-m_iWid+1];
								g3 = Gradient[cur+m_iWid-1];
							}
						}
						{
							Tmp1 = weight*g1 + (1-weight)*g2;
							Tmp2 = weight*g3 + (1-weight)*g4;
						
							if(Tmp>=Tmp1 && Tmp>=Tmp2)
							{
								NonMax[cur] = 128;
							}
							else
							{
								NonMax[cur] = 0;
							}
						}
					}
					
					
				}
			}		
			
		}
	}

	for(k = 0; k<m_iCount; k++)
	{
		for(j=1;j<m_iHei-1;j++)
		{
			for(i=1;i<m_iWid-1;i++)
			{
				cur = k*m_iWid*m_iHei+j*m_iWid+i;
				
				if((NonMax[cur]==128) && (Gradient[cur] >= nThrHigh))
				{
					apsInput[cur] = 255;
					TraceEdge(k,j,i,nThrLow,apsInput,Gradient,m_iWid,m_iHei,m_iCount);
				}
			}
		}
	}
	for(k = 0; k<m_iCount; k++)
	{
		for(j=1;j<m_iHei-1;j++)
		{
			for(i=1;i<m_iWid-1;i++)
			{
				cur = k*m_iWid*m_iHei+j*m_iWid+i;
				
				if(apsInput[cur]!=255)
				{
					apsInput[cur] = 0;
				}
			}
		}
	}
	
}

void TipPlugin::TraceEdge(V3DLONG k, V3DLONG y, V3DLONG x, V3DLONG nThrLow,unsigned char*apsInput,unsigned char*Gradient,V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount)
{
	
    static int nDx[] = {1,1,0,-1,-1,-1,0,1};
	static int nDy[] = {0,1,1,1,0,-1,-1,-1};
	
	V3DLONG yy,xx;
	
	for(k=0;k<8;k++)
	{
		yy = y+nDy[k];
		xx = x+nDx[k];
		if(apsInput[k*m_iWid*m_iHei+yy*m_iWid+xx]==128 && Gradient[k*m_iWid*m_iHei+yy*m_iWid+xx]>=nThrLow )
		{
			apsInput[k*m_iWid*m_iHei+yy*m_iWid+xx] = 255;
			
			TraceEdge(k,yy,xx,nThrLow,apsInput,Gradient,m_iWid,m_iHei,m_iCount);			
		}
	}
}


void TipPlugin::StatisticalPixel(unsigned char*apsInput,unsigned char*apsOutput,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG Pcount,float Step,V3DLONG Angle,V3DLONG num, V3DLONG marknum) //Pcount: Point number
{
	//printf("x=%d y=%d z=%d pcount=%d step=%lf angle=%d\n", x,y,z,Pcount, Step,Angle);
	if(apsInput[y*m_OiImgWidth +x]==0)
	{
		v3d_msg("00");
	}
	float x1,y1,x2,y2,x3,y3;	
	SpacePoint_t point;
	vector<SpacePoint_t> temp;
	//temp.clear();
	V3DLONG polValue,polValue2;
	V3DLONG pixelcount;
	V3DLONG currX,currY,currX2,currY2,currX3,currY3;;
	V3DLONG r = Pcount*Step;
	float pi = 3.1416;
	for(int j = 0; j < 360; j=j+Angle)
	{
		pixelcount = 0;
		float a;	
		int cal=0;
		int n=0;
		for(V3DLONG i = 1; i < Pcount; i ++)
		{ 
			cal++;
			if(j>=0 && j < 90)
			{
				a = j*pi/180;
				
				x1 = x + Step*cos(a)*i;
				y1 = y - Step*sin(a)*i;
				
				x2 = x + Step*cos(a)*(i+1);
				y2 = y - Step*sin(a)*(i+1);
				
				x3 = x + Step*cos(a)*(i+2);
				y3 = y - Step*sin(a)*(i+2);
				
			}else if (j>=90 && j<=180)
			{
				a = pi-j*pi/180;
				
				x1 = x - Step*cos(a)*i;
				y1 = y - Step*sin(a)*i;
				
				x2 = x - Step*cos(a)*(i+1);
				y2 = y - Step*sin(a)*(i+1);
				
				x3 = x - Step*cos(a)*(i+2);
				y3 = y - Step*sin(a)*(i+2);
				
			}else if (j> 180 && j<=270) 
			{
				a = j*pi/180-pi;
				
				x1 = x - Step*cos(a)*i;
				y1 = y + Step*sin(a)*i;
				
				x2 = x - Step*cos(a)*(i+1);
				y2 = y + Step*sin(a)*(i+1);
				
				x3 = x - Step*cos(a)*(i+2);
				y3 = y + Step*sin(a)*(i+2);

			}else if (j>270 && j<360)
			{
				a = 2*pi - j*pi/180;
				
				x1 = x + Step*cos(a)*i;
				y1 = y + Step*sin(a)*i;
			
				x2 = x + Step*cos(a)*(i+1);
				y2 = y + Step*sin(a)*(i+1);
				
				x3 = x + Step*cos(a)*(i+2);
				y3 = y + Step*sin(a)*(i+2);

				//printf("k=%lf x=%d y=%d xstep=%lf x1=%lf y1=%lf\n",k,x,y,xstep,x1,y1);
			}
			currX = V3DLONG(x1+0.5);
			currY = V3DLONG(y1+0.5);
			currX2 = V3DLONG(x2+0.5);
			currY2 = V3DLONG(x2+0.5);
			currX3 = V3DLONG(x3+0.5);
			currY3 = V3DLONG(x3+0.5);
			
//			if (x1 > m_OiImgWidth  || y1 > m_OiImgWidth)
//				continue;
//			if (x2 > m_OiImgHeight || y2 > m_OiImgHeight)
//				continue;
//			
//			currX = floor(x1);
//			currY = floor(y1);
//			currX2 = floor(x2);
//			currY2 = floor(x2);
//			Doublelinear_inserting (apsInput,polValue,x1, y1, 1, currX, currY);
//			Doublelinear_inserting (apsInput,polValue2,x2,y2, 1, currX, currY);
		  //  printf("j=%d cos=%lf sina=%lf k=%lf xstep=%lf x1=%lf y1=%lf cx=%d cy=%d\n",j,cos(a),sin(a),xstep,x1,y1,currX,currX);
			if (currX > m_OiImgWidth  || currY > m_OiImgWidth)
				continue;
			if (currX2 > m_OiImgHeight || currY2 > m_OiImgHeight)
				continue;
			polValue = apsInput[currY*m_OiImgWidth +currX];
			if (cal > num) //4,10 
			{
			  if (polValue == 0 && apsInput[currY2*m_OiImgWidth +currX2] == 0 && apsInput[currY3*m_OiImgWidth +currX3] == 0 )
				//if (polValue == 0 && polValue2 == 0)
				break;
			}
			pixelcount += polValue;
			//if (polValue) 
			{
				apsOutput[currY*m_OiImgWidth +currX] = 200;
			}
		}
		point.m_x = x;
		point.m_y = y;
		point.m_z = z;
		point.pixel =pixelcount;
		temp.push_back(point);
			
		// printf("markmum=%d angle=%d pixelcount=%d\n",marknum,j,pixelcount);
	}
	m_StatisticalPiont.push_back(temp);

}

void TipPlugin::Tip_Filter(int iTmpLen)
{
	SpacePoint_t point;
	V3DLONG peak;
	V3DLONG temp;
	V3DLONG size1;
	V3DLONG size = m_GaussFilterPiont.size();
	V3DLONG average[size];
	printf("size=%d\n",size);
	if(m_GaussFilterPiont.size()>0)
	{
		int start = m_StatisticalPiont[0].size() - (iTmpLen/2);		
		int end = start + m_StatisticalPiont[0].size()+1;
		V3DLONG len = (end - start);
		V3DLONG nn;
		V3DLONG countall;
		double variance=0 ;
		printf("start=%d end=%d\n",start,end);
		for(V3DLONG i = 0 ; i < m_GaussFilterPiont.size(); i++)
		{
			for(V3DLONG jj = start ; jj < end; jj++)
			{
				point.m_x = m_GaussFilterPiont[i][jj].m_x;
				point.m_y = m_GaussFilterPiont[i][jj].m_y;
				point.m_z = m_GaussFilterPiont[i][jj].m_z;
				printf("i=%d j=%d count=%d\n",i,jj,m_GaussFilterPiont[i][jj].pixel);
			}			
			peak = 0;
			for(V3DLONG j = start ; j < end; j++)
			{
				V3DLONG delta1 = m_GaussFilterPiont[i][j].pixel - m_GaussFilterPiont[i][j-1].pixel;
				V3DLONG delta2 = m_GaussFilterPiont[i][j+1].pixel - m_GaussFilterPiont[i][j].pixel;
				point.m_x = m_GaussFilterPiont[i][j].m_x;
				point.m_y = m_GaussFilterPiont[i][j].m_y;
				point.m_z = m_GaussFilterPiont[i][j].m_z;
			//	printf("i=%d j=%d fiterpoint=%d\n",i,j,m_GaussFilterPiont[i][j].pixel);
				if (delta1>0 && delta2<0 )
				{   
					peak++;
					//	printf("average=%d pixel=%d \n",average[i],m_StatisticalPiont[i][j].pixel);
					//point.pixel = m_StatisticalPiont[i][j].pixel;
				}else if(delta1 > 0 && delta2==0)
				{
					while ( m_GaussFilterPiont[i][j].pixel == m_GaussFilterPiont[i][j+1].pixel)
					{
						j++;
					}
			        delta2 = m_GaussFilterPiont[i][j+1].pixel-m_GaussFilterPiont[i][j].pixel;		
					if (delta2 < 0)
					{
						peak++;
					}
				}
			}
			countall = 0;
			nn=0;
			for(V3DLONG j = start ; j < end; j++)
			{
				if(m_GaussFilterPiont[i][j].pixel!=0)
				{
					countall += m_GaussFilterPiont[i][j].pixel;
					nn++;
				}
			}
			double temp  = countall/nn;
			double tempd = 0;
			for(V3DLONG j = start ; j < end; j++)
			{
				tempd += (m_GaussFilterPiont[i][j].pixel-temp)*(m_GaussFilterPiont[i][j].pixel-temp);
			}
		    variance = sqrt(tempd/nn);
			point.weight = peak;
		    point.var = variance;
			point.cycle = len;
			point.area = nn;
			m_TipP.push_back(point);
			//printf("temp=%lf tempd=%lf peak=%ld var=%lf area=%d cycle=%d\n",temp,tempd,peak,variance,nn,len);
		}
	}else 
	{
		return;
	}
	if (m_TipP.size()>0)
	{
		for(V3DLONG ii = 0; ii <m_TipP.size(); ii++)
		{
			V3DLONG xx = m_TipP[ii].m_x;
			V3DLONG yy = m_TipP[ii].m_y;
			V3DLONG zz = m_TipP[ii].m_z;
			V3DLONG weight = m_TipP[ii].weight;
			double var1 = m_TipP[ii].var;
			printf("xx=%d yy=%d zz=%d weight=%d var=%lf\n",xx,yy,zz,weight,var1);
		}
	}
	
	//StatisticalPixel(unsigned char*apsInput,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG Pcount,float Step,V3DLONG Angle);
	
}

void TipPlugin::Tip_Filter1(int iTmpLen)
{
	SpacePoint_t point;
	V3DLONG peak;
	V3DLONG temp;
	V3DLONG size1;
	V3DLONG size = m_DetectionPiont.size();
	V3DLONG average[size];
	V3DLONG nn;
	V3DLONG ne;
	if(m_DetectionPiont.size()>0)
	{
		int start = m_StatisticalPiont[0].size();		
		int end = start + m_StatisticalPiont[0].size()+1;
		V3DLONG len = (end - start);
		printf("start=%d end=%d\n",start,end);
		
		for(V3DLONG i = 0 ; i < m_DetectionPiont.size(); i++)
		{
			nn=0;
			peak=0;
			for(V3DLONG jj = start ; jj < end; jj++)
			{
				point.m_x = m_DetectionPiont[i][jj].m_x;
				point.m_y = m_DetectionPiont[i][jj].m_y;
				point.m_z = m_DetectionPiont[i][jj].m_z;
				printf("i1=%d j1=%d count=%d\n",i, jj,m_DetectionPiont[i][jj].pixel);
				if(m_DetectionPiont[i][jj].pixel/255 > iTmpLen-5)//-5
				{
					nn++;
				}
				if(m_DetectionPiont[i][jj].pixel>0)
				{
					ne++;
				}
			}
			for(V3DLONG j = start ; j < end; j++)
			{
				point.m_x = m_DetectionPiont[i][j].m_x;
				point.m_y = m_DetectionPiont[i][j].m_y;
				point.m_z = m_DetectionPiont[i][j].m_z;
				printf("j1=%d count=%d\n",j,m_DetectionPiont[i][j].pixel/255);
				V3DLONG delta1 = m_DetectionPiont[i][j].pixel/255 - m_DetectionPiont[i][j-1].pixel/255;
				V3DLONG delta2 = m_DetectionPiont[i][j+1].pixel/255 - m_DetectionPiont[i][j].pixel/255;
			//	printf("i=%d j=%d fiterpoint=%d\n",i,j,m_DetectionPiont[i][j].pixel);
				if (delta1>0 && delta2<0 )
				{   
					peak++;
					printf("i1=%d j1=%d delta1=%d deta2=%d p1=%d\n",i,j,delta1,delta2, peak);
					
					//	printf("average=%d pixel=%d \n",average[i],m_StatisticalPiont[i][j].pixel);
					//point.pixel = m_StatisticalPiont[i][j].pixel;
				}else if(delta1 > 0 && delta2==0)
				{
					while ( m_DetectionPiont[i][j].pixel == m_DetectionPiont[i][j+1].pixel)
					{
						j++;
						printf("next=%d\n",j);
					}
			        delta2 = m_DetectionPiont[i][j+1].pixel-m_DetectionPiont[i][j].pixel;		
					if (delta2 < 0)
					{
						peak++;
						printf("i1=%d j1=%d p2=%d\n",i,j,peak);
					}
				}				
			}
			point.cycle = len;
			point.area = nn;
			point.weight = peak;
			m_TipP1.push_back(point);
			printf("i1=%d area1=%d cycle1=%d weight1=%d\n",i,nn,len,peak);
		}
	}else 
	{
		return;
	}
}

void TipPlugin::Thinning(unsigned char *apsInput,unsigned char *apsOutput,V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount)
{
	bool bCondition1;
	bool bCondition2;
	bool bCondition3;
	bool bCondition4;
	int neighbour[5][5];
    int nCount;
	int m;
	V3DLONG curPos;
	V3DLONG i,j ;
	V3DLONG* pTmpMark  = new V3DLONG[m_iWid * m_iHei];
	V3DLONG* pTmpMark1 = new V3DLONG[m_iWid * m_iHei];
	bool bModify;
	bModify = true;
	for(i =0; i< m_iHei; i++)
	{
		for(j = 0; j < m_iWid;j++)
		{
			pTmpMark1[i*m_iWid+j] = apsInput[i*m_iWid+j];
			printf("tmp=%d\n",pTmpMark1[i*m_iWid+j]);
		}
	}
	//memcpy(pTmpMark1, apsInput, m_iWid * m_iHei);
	while (bModify) 
	{
		bModify = false;
		
		memset(pTmpMark, 0, m_iWid * m_iHei * sizeof(V3DLONG));
		
		for(int i = 2 ; i< m_iHei - 2 ; i++ )
		{
			for(int j = 2; j<m_iWid - 2; j++)
			{
				bCondition1 = false;
				bCondition2 = false;
				
				bCondition3 = false;
				bCondition4 = false;
				curPos = i*m_iWid+j;
				if( pTmpMark1[curPos] == 255)
				{
					for (int a = 0; a < 5;a++ )
					{
						for (int n = 0; n < 5;n++)
						{
							neighbour[a][n] = pTmpMark1[curPos + (2 - a)*m_iWid +( n - 2)]/ 255;
						}
					}
//					for(int ii=-1;ii<2;ii++)
//						for(int jj=-1;jj<2;jj++)
//						{
//							V3DLONG y = j+ii;
//							V3DLONG x = i+jj;
//							neighbour[ii+2][jj+2] = (pTmpMark1[m_iWid * y + x]/255);
//						}
					
					//逐个判断条件。
					//判断2<=NZ(P1)<=6
					nCount =  neighbour[1][1] + neighbour[1][2] + neighbour[1][3] 
					+ neighbour[2][1] + neighbour[2][3] + 
					+ neighbour[3][1] + neighbour[3][2] + neighbour[3][3];
					if ( nCount >= 2 && nCount <=6)
						bCondition1 = true;
					//判断Z0(P1)=1
					nCount = 0;
					if (neighbour[1][2] == 0 && neighbour[1][1] == 1)
						nCount++;
					if (neighbour[1][1] == 0 && neighbour[2][1] == 1)
						nCount++;
					if (neighbour[2][1] == 0 && neighbour[3][1] == 1)
						nCount++;
					if (neighbour[3][1] == 0 && neighbour[3][2] == 1)
						nCount++;
					if (neighbour[3][2] == 0 && neighbour[3][3] == 1)
						nCount++;
					if (neighbour[3][3] == 0 && neighbour[2][3] == 1)
						nCount++;
					if (neighbour[2][3] == 0 && neighbour[1][3] == 1)
						nCount++;
					if (neighbour[1][3] == 0 && neighbour[1][2] == 1)
						nCount++;
					if (nCount == 1)
						bCondition2 = true;
					//判断P2*P4*P8=0 or Z0(p2)!=1
					if (neighbour[1][2]*neighbour[2][1]*neighbour[2][3] == 0)
						bCondition3 = true;
					else
					{
						nCount = 0;
						if (neighbour[0][2] == 0 && neighbour[0][1] == 1)
							nCount++;
						if (neighbour[0][1] == 0 && neighbour[1][1] == 1)
							nCount++;
						if (neighbour[1][1] == 0 && neighbour[2][1] == 1)
							nCount++;
						if (neighbour[2][1] == 0 && neighbour[2][2] == 1)
							nCount++;
						if (neighbour[2][2] == 0 && neighbour[2][3] == 1)
							nCount++;
						if (neighbour[2][3] == 0 && neighbour[1][3] == 1)
							nCount++;
						if (neighbour[1][3] == 0 && neighbour[0][3] == 1)
							nCount++;
						if (neighbour[0][3] == 0 && neighbour[0][2] == 1)
							nCount++;
						if (nCount != 1)
							bCondition3 = true;
					}
					//判断P2*P4*P6=0 or Z0(p4)!=1
					if (neighbour[1][2]*neighbour[2][1]*neighbour[3][2] == 0)
						bCondition4 = true;
					else
					{
						nCount = 0;
						if (neighbour[1][1] == 0 && neighbour[1][0] == 1)
							nCount++;
						if (neighbour[1][0] == 0 && neighbour[2][0] == 1)
							nCount++;
						if (neighbour[2][0] == 0 && neighbour[3][0] == 1)
							nCount++;
						if (neighbour[3][0] == 0 && neighbour[3][1] == 1)
							nCount++;
						if (neighbour[3][1] == 0 && neighbour[3][2] == 1)
							nCount++;
						if (neighbour[3][2] == 0 && neighbour[2][2] == 1)
							nCount++;
						if (neighbour[2][2] == 0 && neighbour[1][2] == 1)
							nCount++;
						if (neighbour[1][2] == 0 && neighbour[1][1] == 1)
							nCount++;
						if (nCount != 1)
							bCondition4 = true;
					}
					if(bCondition1 && bCondition2 && bCondition3 && bCondition4)
					{
						pTmpMark[curPos] = 0;
						bModify = true;
					}
					else
					{
						pTmpMark[curPos] = 255;
					}
				} 
			}
		}
		for(i =0; i< m_iHei; i++)
		{
			for(j = 0; j < m_iWid;j++)
			{
				pTmpMark1[i*m_iWid+j] = pTmpMark[i*m_iWid+j];
			}
		}		
		//memcpy(pTmpMark1, pTmpMark, m_iWid*m_iHei);
	} 

	for(int i = 1 ; i< m_iHei  ; i++ )
	{
		for(int j = 1; j<m_iWid ; j++)
		{
			curPos = i*m_iWid+j;
			apsOutput[curPos] |= pTmpMark1[curPos];
		}
	}
    delete []pTmpMark;
	delete []pTmpMark1;
}


void TipPlugin::CreateGaussFilterTemplet(float *apfTemplet, int iSize, float fLamb)
{
	if((iSize % 2) == 0)
	{
		return;
	}	
	int x;
	int i;
	int iHalfSize = iSize / 2;
	float fTemp;
	for (i = 0; i < iSize; i++)
	{
		//x = i - iHalfSize;
		x = 3*(i - iHalfSize)/iHalfSize;
		fTemp = (x * x)/(2 * fLamb * fLamb);
		apfTemplet[i] = exp(-fTemp);
		printf("guss=%lf\n",apfTemplet[i]);
	}
	return;		
}

void TipPlugin::CreateGaussFilterTemplet2D(float *apfTemplet, int iSize, float fLamb)
{
	if((iSize % 2) == 0)
	{
		return;
	}	
	int x;
	int y;
	int z;
	int i;
	int j;
	int k;
	
	int iHalfSize = iSize / 2;
	float fTemp;
	int iSizeS;
	
	int matrix_length;
	float std_dev;
	float sum;
	
	float radius = iHalfSize ;
	
	radius = (float)fabs(0.5*radius) + 0.25f;
	
	std_dev = radius;
	
	radius = std_dev * 2;
	
	matrix_length = int (2 * ceil(radius-0.5) + 1);
	
	if (matrix_length <= 0) matrix_length = 1;
	
	bool type = false;
	
	for (j = 0; j < iSize; j++)
	{
		for (i = 0; i < iSize; i++)
		{
			if(type) //not sampling
			{
				x = 3*(i - iHalfSize)/iHalfSize;
				y = 3*(j - iHalfSize)/iHalfSize;
				fTemp = (x * x + y * y ) / 2;
				apfTemplet[j*iSize+i] = (float)exp(-fTemp);
			//	printf("gaussian=%lf aa=%d\n",apfTemplet[j*iSize+i],aa);
				
			}else ///from -2*std_dev to 2*std_dev, sampling 50 points per pixel
			{
				x = i - iHalfSize;
				y = j - iHalfSize;				
				
				float base_x = abs(x) - (float)floor((float)(matrix_length/2)) - 0.5f;
				float base_y = abs(y) - (float)floor((float)(matrix_length/2)) - 0.5f;
				
				sum = 0;
				for (k = 1; k <= 50; k++)
				{
					if ( base_x+0.02*k <= radius ) 
					sum += (float)exp (-((base_x+0.02*k)*(base_x+0.02*k)+ (base_y+0.02*k)*(base_y+0.02*k))/ 
										   (2*std_dev*std_dev));
					
				}
				apfTemplet[j*iSize+i] = sum/50;
				//printf("gaussian=%lf\n",apfTemplet[j*iSize+i]);
			}
		}
	}	
	if (type==false) 
	{
		sum = 0;
		for (j=0; j<=50; j++)
		{
			sum += (float)exp (-(0.5+0.02*j)*(0.5+0.02*j)+(0.5+0.02*j)*(0.5+0.02*j)/
							   (2*std_dev*std_dev));
			
		}
		for(k = 0; k < iSize; k++)
			apfTemplet[iHalfSize*iSize+k] = sum/51;
		//   printf("gaussianhalf=%lf\n",apfTemplet[iHalfSize*iSize+k]);
	}
	
	return;
}

void TipPlugin::CreateGaussFilterTemplet3D(float *apfTemplet, int iSize, float fLamb)
{
	if((iSize % 2) == 0)
	{
		return;
	}	
	int x;
	int y;
	int z;
	int i;
	int j;
	int k;
	
	int iHalfSize = iSize / 2;
	float fTemp;
	int iSizeS;
	
	int matrix_length;
	float std_dev;
	float sum;

	float radius = iHalfSize ;
	
	radius = (float)fabs(0.5*radius) + 0.25f;
	
	std_dev = radius;
	
	radius = std_dev * 2;
	
	matrix_length = int (2 * ceil(radius-0.5) + 1);
	
	if (matrix_length <= 0) matrix_length = 1;
	
	for(k = 0; k < iSize; k++)
	{
		for (j = 0; j < iSize; j++)
		{
			for (i = 0; i < iSize; i++)
			{
				{
				//	x = 3*(i - iHalfSize)/iHalfSize;
//					y = 3*(j - iHalfSize)/iHalfSize;
//					z = 3*(k - iHalfSize)/iHalfSize;
//					fTemp = (z * z + x * x + y * y ) / 2;
//					apfTemplet[k*iSize*iSize+j*iSize+i] = (float)exp(-fTemp);
					
					x = i - iHalfSize;
					y = j - iHalfSize;				
					z = k - iHalfSize;
					float base_x = abs(x) - (float)floor((float)(matrix_length/2)) - 0.5f;
					float base_y = abs(y) - (float)floor((float)(matrix_length/2)) - 0.5f;
					float base_z = abs(z) - (float)floor((float)(matrix_length/2)) - 0.5f;
					sum = 0;
					for (int kk = 1; kk <= 50; kk++)
					{
						if ( base_x+0.02*kk <= radius ) 
							sum += (float)exp (-((base_x+0.02*kk)*(base_x+0.02*kk)+ (base_y+0.02*kk)*(base_y+0.02*kk)+(base_z+0.02*kk)*(base_z+0.02*kk))/ 
											   (2*std_dev*std_dev));
						
					}
					apfTemplet[j*iSize+i] = sum/50;
					
				}
			}
		}	
	}
	return;
}

void TipPlugin::GaussFilter2D(unsigned char *apsImgInput, unsigned char *apfImgOutput,V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount, float *apfTemplate,int iTmpLen)
{
	
	int i;
	int j;
	int k;
	int m;
	int n;
	int l;
	
	float fSum;
	float fTotal = 0.0;
	int iSize = iTmpLen / 2;
	
	for (n = -iSize; n < iSize + 1; n++)
	{
		for (m = -iSize; m < iSize + 1; m++)
		{
			fTotal += apfTemplate[(n + iSize)*iTmpLen+m + iSize];
		}
	}
	
	for (j = iSize; j < m_iHei - iSize; j++)
	{
		for (i = iSize; i < m_iWid - iSize ; i++)
		{
			fSum = 0;
			{
				for (n = -iSize; n < iSize + 1; n++)
				{
					for (m = -iSize; m < iSize + 1; m++)
					{
						fSum += apsImgInput[(j + n)*m_iWid + i + m] * 
						apfTemplate[(n + iSize)*iTmpLen+m + iSize];
					}
				}
			}
			apfImgOutput[j*m_iWid+i] = (char)(0.5f + fSum / fTotal);
		}
	}
		
}


void TipPlugin::GaussFilter3D(unsigned char *apsImgInput, unsigned char *apfImgOutput,V3DLONG m_iWid, V3DLONG m_iHei, V3DLONG m_iCount, float *apfTemplate,int iTmpLen)
{
	
	int i;
	int j;
	int k;
	int m;
	int n;
	int l;
	float fSum;
	float fTotal = 0.0;
	int iSize = iTmpLen / 2;
	
	for (l = -iSize; l < iSize + 1; l++)
	{
		for (n = -iSize; n < iSize + 1; n++)
		{
			for (m = -iSize; m < iSize + 1; m++)
			{
				
				fTotal += apfTemplate[(l + iSize)*iTmpLen*iTmpLen+(n + iSize)*iTmpLen+m + iSize];
			}
		}
	}
	
	for (k =  iSize; k < m_iCount - iSize; k++)
	{
		for (j = iSize; j < m_iHei - iSize; j++)
		{
			for (i = iSize; i < m_iWid - iSize ; i++)
			{
				fSum = 0;
				{
					for (l = -iSize; l < iSize + 1; l++)
					{
						for (n = -iSize; n < iSize + 1; n++)
						{
							for (m = -iSize; m < iSize + 1; m++)
							{
								fSum += apsImgInput[(k + l)*m_iHei*m_iWid+(j + n)*m_iWid+i + m] * 
								apfTemplate[(l + iSize)*iTmpLen*iTmpLen+(n + iSize)*iTmpLen+m + iSize];
							}
						}
					}
				}
				apfImgOutput[k*m_iHei*m_iWid+j*m_iWid+i] = fSum / fTotal;
			}
		}
	}
	
}



void TipPlugin::GaussFilter(float *apfTemplate,int iTmpLen)
{
	V3DLONG i;
	V3DLONG j;
	V3DLONG m,n;	
	SpacePoint_t point;
	vector<SpacePoint_t> temp;
	float fSum;
	float fTotal = 0.0;
	int iSize = iTmpLen/2;
	
	for (m = 0; m < iTmpLen ; m++)
	{
		fTotal += apfTemplate[m];
	}
	if( m_DetectionPiont.size()>0)
	{
		for(V3DLONG i = 0 ; i < m_DetectionPiont.size(); i++)
		{
			temp.clear();
			for(V3DLONG j = iSize ; j < m_DetectionPiont[i].size()-iSize; j++)
			{
				fSum = 0;
				for (n = -iSize; n < iSize + 1; n++)
				{
					fSum += m_DetectionPiont[i][j+n].pixel * 
					apfTemplate[(n + iSize)];
				}
				point.m_x = m_DetectionPiont[i][j+n].m_x;
				point.m_y = m_DetectionPiont[i][j+n].m_y;
				point.m_z = m_DetectionPiont[i][j+n].m_z;
				point.pixel =V3DLONG(fSum/fTotal);
			//	printf("i=%d j=%d fsum=%lf ftotal=%lf pixel=%d\n",i,j,fSum,fTotal,point.pixel);
				temp.push_back(point);
			}
			m_GaussFilterPiont.push_back(temp);
		}
		//printf("pixel=%d\n",m_GaussFilterPiont[0][1].pixel);
	}
	else{return;}
	
}
void TipPlugin::Borderexpand(V3DLONG size,V3DLONG Scount)
{
	vector<SpacePoint_t> temp;
	
	int kk = 400;
    SpacePoint_t *bpoint = new SpacePoint_t[size*Scount];
	
	//SpacePoint_t bpoint[size][Scount];
	//vector< vector<SpacePoint_t> > bpoint;	
	SpacePoint_t point;
	
	V3DLONG k,i,j;
	V3DLONG m,n;
//	printf("size=%ld count=%ld\n",size,Scount);
	for (m = 0; m < m_StatisticalPiont.size(); m++) 
	{
		for (n = 0; n< m_StatisticalPiont[m].size(); n++)
		{
//			bpoint[m][n].m_x=m_StatisticalPiont[m][n].m_x;
//			bpoint[m][n].m_y=m_StatisticalPiont[m][n].m_y;
//			bpoint[m][n].m_z=m_StatisticalPiont[m][n].m_z;
//			bpoint[m][n].pixel=m_StatisticalPiont[m][n].pixel;
			bpoint[m*Scount+n].m_x=m_StatisticalPiont[m][n].m_x;
			bpoint[m*Scount+n].m_y=m_StatisticalPiont[m][n].m_y;
			bpoint[m*Scount+n].m_z=m_StatisticalPiont[m][n].m_z;
			bpoint[m*Scount+n].pixel=m_StatisticalPiont[m][n].pixel;
			
			//printf("i=%d j=%d bpint=%d\n",i,j,bpoint[i][j].pixel);
		}
	}
//	for (m = 0; m < m_StatisticalPiont.size(); m++) 
//	{
//		for (n = 0; n< m_StatisticalPiont[m].size(); n++)
//		{
//			point.m_x=m_StatisticalPiont[m][n].m_x;
//			point.m_y=m_StatisticalPiont[m][n].m_y;
//			point.m_z=m_StatisticalPiont[m][n].m_z;
//			point.pixel=m_StatisticalPiont[m][n].pixel;
//			//printf("i=%d j=%d bpint=%d\n",i,j,bpoint[i][j].pixel);
//			temp.push_back(point);
//		}
//		bpoint.push_back(temp);
//	}
	for( i = 0 ; i< size; i++)
	{
		temp.clear();
		for(k = 0;k <3;k++)
		{
			for (j = 0; j< Scount; j++)
			{ 
				point.m_x=bpoint[i*Scount+j].m_x;
				point.m_y=bpoint[i*Scount+j].m_y;
				point.m_z=bpoint[i*Scount+j].m_z;
				point.pixel=bpoint[i*Scount+j].pixel;
				temp.push_back(point);
			//	printf("i=%d j=%d bpint=%d\n",i,j,bpoint[i][j].pixel);
			}
		}
		m_DetectionPiont.push_back(temp);
	}
	//qDebug()<<"detection" << m_DetectionPiont.size() << m_DetectionPiont[0].size();
	printf("detection=%d detection=%d\n",m_DetectionPiont.size(),m_DetectionPiont[0].size());
		
}

void TipDialog::update()
{
	//get current data
	Dn = Dnumber->text().toLong()-1;
	Dh = Ddistance->text().toLong()-1;
		//printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[ind], nx, ny, nz, ind);
}

