#ifndef MYFEATURE_H_H
#define MYFEATURE_H_H
#include <iostream>
#include <vector>
#include "v3d_interface.h"
#include "v3d_basicdatatype.h"
#include "img_definition.h"

#include "compute_moments.h"

using namespace std;

typedef float REAL;
enum MyFeatureType{NONE_FEATURE, AVERAGE_FEATURE, STD_VAR_FEATURE, SIFT_FEATURE, INVARIANT_MOMENT_FEATURE};

class MyFeature
{
	public:
		MyFeature()
		{
			m_ndims = 1;
			m_size = 0;
			m_featureType = NONE_FEATURE;
		}
		~MyFeature(){};

		void printFeatures()
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

		template <class T> void setFeatures(LandmarkList & landmarks, Vol3DSimple<T> *vol3d, MyFeatureType type)
		{
			T *** data3d = vol3d->getData3dHandle();

			if(type == AVERAGE_FEATURE)
			{
				cout<<"set average intensity feature"<<endl;
				m_featureType = type;
				m_ndims = 1;
				m_size = landmarks.size();
				m_features.resize(m_size);
				float radius = 5.0;
				for(int i = 0; i < m_size; i++)
				{
					int posx = landmarks[i].x;
					int posy = landmarks[i].y;
					int posz = landmarks[i].z;

					int minx = posx - radius >= 0 ? posx - radius : 0;
					int maxx = posx + radius < vol3d->sz0() ? posx + radius : vol3d->sz0();

					int miny = posy - radius >= 0 ? posy - radius : 0;
					int maxy = posy + radius < vol3d->sz1() ? posy + radius : vol3d->sz1();

					int minz = posz - radius >= 0 ? posz - radius : 0;
					int maxz = posz + radius < vol3d->sz2() ? posz + radius : vol3d->sz2();

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
									sum += data3d[z][y][x]; 
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
				m_size = landmarks.size();
				m_features.resize(m_size);
				float radius = 5.0;
				for(int i = 0; i < m_size; i++)
				{
					int posx = landmarks[i].x;
					int posy = landmarks[i].y;
					int posz = landmarks[i].z;

					int minx = posx - radius >= 0 ? posx - radius : 0;
					int maxx = posx + radius < vol3d->sz0() ? posx + radius : vol3d->sz0();

					int miny = posy - radius >= 0 ? posy - radius : 0;
					int maxy = posy + radius < vol3d->sz1() ? posy + radius : vol3d->sz1();

					int minz = posz - radius >= 0 ? posz - radius : 0;
					int maxz = posz + radius < vol3d->sz2() ? posz + radius : vol3d->sz2();

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
									int intensity = data3d[z][y][x];
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
			else if(type == INVARIANT_MOMENT_FEATURE)
			{
				cout<<"set invariant methods feature"<<endl;
				m_featureType = type;
				m_ndims = 0;
				m_size = landmarks.size();
				m_features.resize(m_size);

				Vector1DSimple<double> momentVec;
				int r = 5;
				KernelSet* ks = new KernelSet(4, r, KT_CUBE_ALL1);
				for(int i = 0; i < m_size; i++)
				{
					V3DLONG x0 = landmarks[i].x;
					V3DLONG y0 = landmarks[i].y;
					V3DLONG z0 = landmarks[i].z;

					computeGMI(momentVec, vol3d, x0, y0, z0, r);
					m_ndims = momentVec.sz0();
					cout<<"momentVec.sz0() = "<<m_ndims<<endl;

					double* data1d = momentVec.getData1dHandle();
					for(int j = 0; j < m_ndims; j++) m_features[i].push_back(data1d[j]);
				}

				//if(ks) delete ks;
			}
			else 
			{
				m_featureType = NONE_FEATURE;
				return;
			}
		}

		MyFeatureType  featureType() {return m_featureType;}

	private:
		int m_ndims;
		int m_size;
		vector<vector<REAL> > m_features;
		MyFeatureType m_featureType;
};

#endif
