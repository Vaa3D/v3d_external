#ifndef MYFEATURE_H_H
#define MYFEATURE_H_H
#include <iostream>
#include <vector>
#include "v3d_basicdatatype.h"

using namespace std;

typedef float REAL;
enum MyFeatureType{NONE_FEATURE, AVERAGE_FEATURE, STD_VAR_FEATURE, SIFT_FEATURE, INVARIANT_METHODS_FEATURE};
class MyFeature
{
	public:
		MyFeature();
		~MyFeature(){};
		void printFeatures();
		void setFeatures(vector<vector<int> >& mark_list, unsigned char *inimg1d, const V3DLONG sz[3], MyFeatureType type);
		MyFeatureType  featureType();
	private:
		int m_ndims;
		int m_size;
		vector<vector<REAL> > m_features;
		MyFeatureType m_featureType;
};

#endif
