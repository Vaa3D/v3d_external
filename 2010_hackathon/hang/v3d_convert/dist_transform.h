#ifndef __DIST_TRANSFORM_H_H__
#define __DIST_TRANSFORM_H_H__

#include "fastmarching.h"
#include "v3d_basicdatatype.h"

template<class T> bool normal_distance_transform(double * &phi, T* inimg1d, V3DLONG * &sz, double thresh)
{
	return true;
}

template<class T> bool fastmarching_distance(double * &phi, T* inimg1d, V3DLONG * &sz, double thresh)
{
	return perform_fastmarching(phi, inimg1d, sz, thresh);
}


#endif
