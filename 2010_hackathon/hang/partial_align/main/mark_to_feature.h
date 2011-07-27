#ifndef __MARK_TO_FEATURE_H_
#define __MARK_TO_FEATURE_H_
#include "mark_io.h"
#include "v3d_basicdatatype.h"
#include <vector>

using namespace std;

struct FeatureType
{
	V3DLONG x;
	V3DLONG y;
	V3DLONG z;
	vector<double> descriptor;
};

bool mark_to_feature(FeatureType* &features, list<MyImageMarker> listMarker, const unsigned char* inimg1d, V3DLONG sz[3]);

#endif
