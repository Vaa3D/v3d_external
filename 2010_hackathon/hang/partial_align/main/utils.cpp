#include <iostream>
#include <vector>

#include "img_definition.h"
#include "basic_memory.h"
#include "utils.h"
#include "basic_types.h"

#include "../../v3d_convert/gaussian_blur.cpp"    // smooth
#include "../edge_detection.h"
#include "../compute_moments.h"

using namespace std;

bool detect_marker(vector<MarkerType> & markervector, const unsigned char* inimg1d, V3DLONG sz[3])
{
#define SIGMA 1.0
#define THRESHOLD 20

	if(!inimg1d || sz[0] <=0 || sz[1] <=0 || sz[2]<=0) return false;
	double * outimg1d = new double [sz[0] * sz[1] * sz[2]];
	double * eimg1d = 0;
	//if(!smooth(outimg1d, inimg1d, sz, SIGMA)) return false;
	if(computeGaussian(outimg1d, inimg1d, sz, SIGMA, 3) == -1) return false;
	if(computeThreshold(outimg1d, sz, THRESHOLD) == -1) return false;
	if(computeGradience(eimg1d, outimg1d, sz) == -1) return false;

	V3DLONG gsz[3] = {10,10,10};
	V3DLONG * grids[3];
	V3DLONG gridnum;
	if(computeEdgeGrid(grids, gridnum, gsz, eimg1d, sz) == -1) return false;
	cout<<"gridnum = "<<gridnum<<" , gsz[0] = "<<gsz[0]<<" gsz[1] = "<<gsz[1]<<" gsz[2] = "<<gsz[2]<<endl;
	for(V3DLONG i = 0; i < gridnum; i++)
	{
		markervector.push_back(MarkerType(grids[0][i], grids[1][i], grids[2][i]));
	}

	if(outimg1d) {delete outimg1d; outimg1d = 0;}
	if(eimg1d) {delete eimg1d; eimg1d = 0;}
}


bool mark_to_feature(vector<FeatureType> &vecFeature, vector<FeatureType> vecMarker, const unsigned char* inimg1d, V3DLONG sz[3])
{
#define MOMENT_NUM  5
	if(!vecFeature.empty() || vecMarker.empty() || inimg1d ==0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;

	V3DLONG n = vecMarker.size();
	vecFeature.resize(n);

	Vector1DSimple<double> momentVec;
	momentVec.resize(MOMENT_NUM);
	V3DLONG r = 30;

	vector<FeatureType>::iterator it = vecMarker.begin();
	int i = 0;

	while(it != vecMarker.end())
	{
		FeatureType S = *it;
		vecFeature[i].x = S.x;
		vecFeature[i].y = S.y;
		vecFeature[i].z = S.z;
		
		computeGMI(momentVec, (unsigned char *)inimg1d, sz , S.x, S.y, S.z, r);
		double* data1d = momentVec.getData1dHandle();

		for(V3DLONG ii = 0; ii < MOMENT_NUM; ii++) vecFeature[i].descriptor.push_back(data1d[ii]);

		it++;
		i++;
	}
}
