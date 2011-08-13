#ifndef _IMG_CENTER_H_
#define _IMG_CENTER_H_

#include "basic_memory.h"
#include <limits>

template<class T> bool intensity_center( double * &pos, T* &inimg1d, V3DLONG * &sz)
{
	if(inimg1d == 0 || sz == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	T*** inimg3d = 0;
	try{
		if(pos == 0) pos = new double[3];
		pos[0] = pos[1] = pos[2] = 0.0;
		new3dpointer(inimg3d, sz[0], sz[1], sz[2], inimg1d);
	}
	catch(...)
	{
		if(inimg3d)delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);
		return false;
	}
	double sum_int = 0.0;
	for(V3DLONG k = 0; k < sz[2]; k++)
	{
		for(V3DLONG j = 0; j < sz[1]; j++)
		{
			for(V3DLONG i = 0; i < sz[0]; i++)
			{
				pos[0] += i * inimg3d[k][j][i];
				pos[1] += j * inimg3d[k][j][i];
				pos[2] += k * inimg3d[k][j][i];
				sum_int += inimg3d[k][j][i];
			}
		}
	}
	if(sum_int > 0.0)
	{
		pos[0] /= sum_int;
		pos[1] /= sum_int;
		pos[2] /= sum_int;
	}
	else pos[0] = pos[1] = pos[2] = 0.0;

	if(inimg3d)delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);

	return true;
}

template<class T> bool center_marker(double * &pos, T* &inimg1d, V3DLONG * &sz, double thresh)
{
	if(inimg1d == 0 || sz == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	T*** inimg3d = 0;
	try{
		if(pos == 0) pos = new double[3];
		pos[0] = pos[1] = pos[2] = 0.0;
		new3dpointer(inimg3d, sz[0], sz[1], sz[2], inimg1d);
	}
	catch(...)
	{
		if(inimg3d)delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);
		return false;
	}
	double sum_int = 0.0;
	for(V3DLONG k = 0; k < sz[2]; k++)
	{
		for(V3DLONG j = 0; j < sz[1]; j++)
		{
			for(V3DLONG i = 0; i < sz[0]; i++)
			{
				T intensity = inimg3d[k][j][i] >= thresh ? inimg3d[k][j][i] : 0;
				pos[0] += i * intensity;
				pos[1] += j * intensity;
				pos[2] += k * intensity;
				sum_int += intensity;
			}
		}
	}
	if(sum_int > 0.0)
	{
		pos[0] /= sum_int;
		pos[1] /= sum_int;
		pos[2] /= sum_int;
	}
	else pos[0] = pos[1] = pos[2] = 0.0;

	V3DLONG mini, minj, mink;
	double min_dist = std::numeric_limits<double>::max();
	for(V3DLONG k = 0; k < sz[2]; k++)
	{
		for(V3DLONG j = 0; j < sz[1]; j++)
		{
			for(V3DLONG i = 0; i < sz[0]; i++)
			{
				double dist = (k - pos[2]) * (k - pos[2]) + (j - pos[1])*(j - pos[1]) + (i - pos[0])*(i - pos[0]);
				if(dist < min_dist)
				{
					min_dist = dist;
					mini = i; minj = j; mink = k;
				}
			}
		}
	}
	pos[0] = mini; pos[1] = minj; pos[2] = mink;

	if(inimg3d)delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);

	return true;
}
#endif
