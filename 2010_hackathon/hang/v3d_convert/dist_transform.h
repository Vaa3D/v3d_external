#ifndef __DIST_TRANSFORM_H_H__
#define __DIST_TRANSFORM_H_H__

#include "fastmarching.h"
#include "v3d_basicdatatype.h"
#include "FL_bwdist.h"

template<class T> bool normal_distance_transform(double * &pDist, T* inimg1d, V3DLONG * &sz, double thresh)
{
	if(inimg1d == 0 || sz == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;

	V3DLONG tol_sz = sz[0] * sz[1] * sz[2]; 
	pDist = (pDist==0) ? new double[tol_sz] : pDist;

	V3DLONG       * pLabel = new V3DLONG[tol_sz]; 
	unsigned char * pData  = new unsigned char [tol_sz];

	for(V3DLONG i = 0; i < tol_sz; i++) pData[i] = (inimg1d[i] >= thresh) ? 1 : 0;

	dt3d_binary(pData, pDist, pLabel, sz, 0);

	if(pLabel){delete [] pLabel; pLabel = 0;}
	if(pData) {delete [] pData; pData = 0;}
	return true;
}

template<class T> bool fastmarching_distance(double * &pDist, T* inimg1d, V3DLONG * &sz, double thresh)
{
	return perform_fastmarching(pDist, inimg1d, sz, thresh);
}

#endif
