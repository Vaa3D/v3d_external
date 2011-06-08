#include <iostream>
#include <cmath>

#include "myfeature.h"
using namespace std;

MyFeature::MyFeature()
{
	m_ndims = 1;
	m_size = 0;
	m_featureType = NONE_FEATURE;
}

void MyFeature::printFeatures()
{
	for(int i = 0; i < m_size; i++)
	{
		for(int j = 0; j < m_ndims; j++)
		{
			cout<<m_features[i][j]<<" ";
		}
		cout<<endl;
	}
}

void MyFeature::setFeatures(vector<vector<int> >& mark_list, unsigned char *inimg1d, const V3DLONG sz[3], MyFeatureType type)
{
	if(type == AVERAGE_FEATURE)
	{
		cout<<"set average intensity feature"<<endl;
		m_featureType = type;
		m_ndims = 1;
		m_size = mark_list.size();
		m_features.resize(m_size);
		float radius = 5.0;
		for(int i = 0; i < m_size; i++)
		{
			int posx = mark_list[i][0];
			int posy = mark_list[i][1];
			int posz = mark_list[i][2];

			int minx = posx - radius >= 0 ? posx - radius : 0;
			int maxx = posx + radius < sz[0] ? posx + radius : sz[0];

			int miny = posy - radius >= 0 ? posy - radius : 0;
			int maxy = posy + radius < sz[1] ? posy + radius : sz[1];

			int minz = posz - radius >= 0 ? posz - radius : 0;
			int maxz = posz + radius < sz[2] ? posz + radius : sz[2];

			int count = 0;
			float sum = 0;
			for(int x = minx; x <= maxx; x++)
			{
				int dx = x - posx;
				for(int y = miny; y <= maxy; y++)
				{
					int dy = y - posy;
					for(int z = minz; z < maxz; z++)
					{
						int dz = z - posz;
						if(dx*dx + dy*dy + dz*dz <= radius*radius)
						{
							sum += inimg1d[z * sz[1] * sz[0] + y * sz[0] + x];
							count++;
						}
					}
				}
			}
			m_features[i].push_back(sum/count);		
		}
	}
	else if(type == STD_VAR_FEATURE)
	{
		cout<<"set stand variance feature"<<endl;
		m_featureType = type;
		m_ndims = 2;
		m_size = mark_list.size();
		m_features.resize(m_size);
		float radius = 5.0;
		for(int i = 0; i < m_size; i++)
		{
			int posx = mark_list[i][0];
			int posy = mark_list[i][1];
			int posz = mark_list[i][2];

			int minx = posx - radius >= 0 ? posx - radius : 0;
			int maxx = posx + radius < sz[0] ? posx + radius : sz[0];

			int miny = posy - radius >= 0 ? posy - radius : 0;
			int maxy = posy + radius < sz[1] ? posy + radius : sz[1];

			int minz = posz - radius >= 0 ? posz - radius : 0;
			int maxz = posz + radius < sz[2] ? posz + radius : sz[2];

			int count = 0;
			float sum_square = 0.0;
			float sum = 0.0;
			for(int x = minx; x <= maxx; x++)
			{
				int dx = x - posx;
				for(int y = miny; y <= maxy; y++)
				{
					int dy = y - posy;
					for(int z = minz; z < maxz; z++)
					{
						int dz = z - posz;
						if(dx*dx + dy*dy + dz*dz <= radius*radius)
						{
							int intensity = inimg1d[z * sz[1] * sz[0] + y * sz[0] + x];
							sum += intensity;
							sum_square += intensity*intensity;
							count++;
						}
					}
				}
			}
			float avg = sum / count;
			float std_var = sqrt((sum_square - count * avg * avg)/(count - 1));
			m_features[i].push_back(avg);
			m_features[i].push_back(std_var);
		}
	}
	else if(type == SIFT_FEATURE)
	{
		cout<<"set scale invariant feature transform (SIFT) feature"<<endl;
		m_featureType = type;
	}
	else if(type == INVARIANT_METHODS_FEATURE)
	{
		cout<<"set invariant methods feature"<<endl;
		m_featureType = type;
	}
	else 
	{
		m_featureType = NONE_FEATURE;
		return;
	}
}

MyFeatureType MyFeature::featureType()
{
	return m_featureType;
}

