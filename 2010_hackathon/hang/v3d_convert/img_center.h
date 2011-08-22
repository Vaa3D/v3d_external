#ifndef _IMG_CENTER_H_
#define _IMG_CENTER_H_

#include "basic_memory.h"
#include <limits>
#include "img_segment.h"
#include <iostream>
using namespace std;

// this is used for segmentated image

template<class T> bool maximum_connected_component_center(double * &pos, T* &inimg1d, V3DLONG * &sz, double thresh)
{
	if(inimg1d == 0 || sz == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	if(pos == 0) pos = new double[3];

	return true;
}

template<class T> bool maximum_xyzplane_intensity_center(double * &pos, T* &inimg1d, V3DLONG * &sz, double thresh)
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
	double avgi = 0.0, avgj = 0.0, avgk = 0.0;
	V3DLONG i,j,k;
	for(k = 0; k < sz[2]; k++)
	{
		for(j = 0; j < sz[1]; j++)
		{
			for(i = 0; i < sz[0]; i++)
			{
				T intensity = inimg3d[k][j][i] >= thresh ? inimg3d[k][j][i] : 0;
				avgi += i * intensity;
				avgj += j * intensity;
				avgk += k * intensity;
				sum_int += intensity;
			}
		}
	}
	if(sum_int > 0.0)
	{
		avgi /= sum_int;
		avgj /= sum_int;
		avgk /= sum_int;
	}
	else avgi = avgj = avgk = 0.0;

	int r = 5;

	double max_density = 0.0;
	V3DLONG kk,jj,ii;
	V3DLONG maxi, maxj, maxk;
	// x-y plane
	k = (V3DLONG)(avgk + 0.5);
	for(j = 0; j < sz[1]; j++)
	{
		for(i = 0; i < sz[0]; i++)
		{
			double density = 0.0;
			for(kk = k - r; kk <= k+r; kk++)
			{
				if(kk < 0 || kk >= sz[2]) continue;
				for(jj = j -r; jj <= j+r; jj++)
				{
					if(jj < 0 || jj >= sz[1]) continue;
					for(ii = i - r; ii <= i+r; ii++)
					{
						if(ii < 0 || ii >= sz[0]) continue;
						if(inimg3d[kk][jj][ii] < thresh) continue;
						density += inimg3d[kk][jj][ii];
					}
				}
			}
			if(density > max_density)
			{
				max_density = density;
				maxi = i;
				maxj = j;
				maxk = k;
			}
		}
	}
	// x-z plane
	j = (V3DLONG)(avgj + 0.5);
	for(k = 0; k < sz[2]; k++)
	{
		for(i = 0; i < sz[0]; i++)
		{
			double density = 0.0;
			for(kk = k - r; kk <= k+r; kk++)
			{
				if(kk < 0 || kk >= sz[2]) continue;
				for(jj = j -r; jj <= j+r; jj++)
				{
					if(jj < 0 || jj >= sz[1]) continue;
					for(ii = i - r; ii <= i+r; ii++)
					{
						if(ii < 0 || ii >= sz[0]) continue;
						if(inimg3d[kk][jj][ii] < thresh) continue;
						density += inimg3d[kk][jj][ii];
					}
				}
			}
			if(density > max_density)
			{
				max_density = density;
				maxi = i;
				maxj = j;
				maxk = k;
			}
		}
	}
	// y-z plane
	i = (V3DLONG)(avgi + 0.5);
	for(k = 0; k < sz[2]; k++)
	{
		for(j = 0; j < sz[1]; j++)
		{
			double density = 0.0;
			for(kk = k - r; kk <= k+r; kk++)
			{
				if(kk < 0 || kk >= sz[2]) continue;
				for(jj = j -r; jj <= j+r; jj++)
				{
					if(jj < 0 || jj >= sz[1]) continue;
					for(ii = i - r; ii <= i+r; ii++)
					{
						if(ii < 0 || ii >= sz[0]) continue;
						if(inimg3d[kk][jj][ii] < thresh) continue;
						density += inimg3d[kk][jj][ii];
					}
				}
			}
			if(density > max_density)
			{
				max_density = density;
				maxi = i;
				maxj = j;
				maxk = k;
			}
		}
	}

	pos[2] = maxk; pos[1] = maxj; pos[0] = maxi;
	if(inimg3d)delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);

	return true;
}
#endif
