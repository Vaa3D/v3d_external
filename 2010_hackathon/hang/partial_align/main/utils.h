#ifndef __UTILS_H__
#define __UTILS_H__

#include "basic_types.h"

bool detect_marker(vector<MarkerType> & vecMarker, const unsigned char* inimg1d, V3DLONG sz[3]);

bool mark_to_feature(vector<FeatureType> & vecFeature, vector<MarkerType> vecMarker, const unsigned char* inimg1d, V3DLONG sz[3]);

#endif
