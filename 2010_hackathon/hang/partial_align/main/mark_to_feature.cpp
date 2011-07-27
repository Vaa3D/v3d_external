#include "mark_to_feature.h"
#include "../compute_moments.h"

//features init to NULL
bool mark_to_feature(FeatureType* &features, list<MyImageMarker> listMarker, const unsigned char* inimg1d, V3DLONG sz[3])
{
	if(features != 0 || listMarker.empty() || inimg1d ==0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;

	V3DLONG n = listMarker.size();
	features = new FeatureType[n];

	Vector1DSimple<double> momentVec;
	momentVec.resize(5);
	V3DLONG r = 30;

	list<MyImageMarker>::iterator it = listMarker.begin();
	int i = 0;

	while(it != listMarker.end())
	{
		MyImageMarker S = *it;
		features[i].x = S.x;
		features[i].y = S.y;
		features[i].z = S.z;
		
		computeGMI(momentVec, inimg1d, sz, S.x, S.y, S.z, r);
		double* data1d = momentVec.getData1dHandle();

		for(V3DLONG ii = 0; ii < sz0; ii++) features[i].descriptor.push_back(data1d[ii]);

		it++;
		i++;
	}
}
