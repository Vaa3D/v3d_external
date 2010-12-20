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

#define BACKGROUND -1 //背景
#define DISOFENDS 1 //相邻两端点距离

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
	}
	else if (menu_name == tr("Help"))
	{
		//v3d_msg("Simple adaptive thresholding: for each voxel, compute a threshold which is the average intensity o");
	}
//
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
	
//	for (V3DLONG k=0; k< m_iImgCount; k++)
//	{
//		for (V3DLONG j=0; j< m_iImgHeight; j++)
//		{
//			for (V3DLONG i=0; i< m_iImgWidth; i++)
//			{
//				if (m_ppsImgData[k][j*m_iImgWidth + i] == 255)
//				{
//					//printf("m_ppsImgData=%d\n",m_ppsImgData[k][j*m_iImgWidth + i]);
//					
//				}
//				//printf("m_ppsImgData=%d\n",m_ppsImgData[k][j*m_iImgWidth + i]);
//			}
//		}
//	}	
	
	m_ulVolumeSize = m_iImgCount * m_iImgSize;
	
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
	
}//
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
	vector<V3DLONG> len;// 
	vector<DFSPoint_t> temp;// 
	
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
		
		// ’“26
		for(int m = 0; m < 3; ++m)
			for(int n = 0; n < 9; ++n)
			{
				z = i + nDz[m];
				y = j + nDy[n];
				x = k + nDx[n];
				if(m_ppsImgData[z][m_iImgWidth * y + x] != BACKGROUND)
				{
					index_nei = m_iImgSize * z + m_iImgWidth * y + x;
					if(index != index_nei)
					{
						// 
						++count1;
						//
						if(l <=m_piDFS[index_nei])
						{
							endp = false;
						}
						// 
						if(l >= m_piDFS[index_nei])
						{
							++count2;
						}
					}
				}
			}
		// 
		if(endp)
		{
			point_l.m_x = k;
			point_l.m_y = j;
			point_l.m_z = i;	
			point_l.m_l = l;
			m_vdfsptEndPoint.push_back(point_l);	
			len.push_back(l);
		}
		//
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
					// 
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
	
	unsigned char * pData = pSub.begin();
	
	V3DLONG *apsInput = new V3DLONG[sx*sy*sz];
	memset(apsInput, 0, sx*sy*sz * sizeof(V3DLONG));
	
	if(method_code == 1)
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
		for (V3DLONG aa = 0; aa < m_vdfsptEndPoint.size(); aa++) 
		{
			DFSPoint_t p;
			p = m_vdfsptEndPoint.at(aa);
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
		callback.setImageName(newwin, QString("Tip Detection"));
		callback.updateImageWindow(newwin);
		
	}else if( method_code == 2)
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
		printf("size=%d\n",nn);
		
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
	}
	int end_t = clock();
//	printf("time eclapse %d s for dist computing!\n", (end_t-start_t)/1000000);
}

void TipDialog::update()
{
	//get current data
	Dn = Dnumber->text().toLong()-1;
	Dh = Ddistance->text().toLong()-1;
		//printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[ind], nx, ny, nz, ind);
}
