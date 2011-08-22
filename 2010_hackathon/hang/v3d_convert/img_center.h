#ifndef _IMG_CENTER_H_
#define _IMG_CENTER_H_

#include "basic_memory.h"
#include <limits>

#define __DEBUG__
#ifdef __DEBUG__
#include <iostream>
using namespace std;
#endif 

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

// find the pixel nearest to the mean position
template<class T> bool center_marker_max_intensity(double * &pos, T* &inimg1d, V3DLONG * &sz, double thresh)
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

	//find the pixel above thresh and nearest to mean position
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
#ifdef __DEBUG__
	cout<<"Nearest pixel : "<<mini<<","<<minj<<","<<mink<<endl;
#endif
	//calc mean position around pos
	int r = 100;
	T max_int = (T)0;
	pos[0] = pos[1] = pos[2] = 0.0;
	for(V3DLONG k = mink - r; k <= mink + r; k++)
	{
		if(k < 0 || k >= sz[2]) continue;
		for(V3DLONG j = minj -r; j <= minj + r; j++)
		{
			if(j < 0 || j >= sz[1]) continue;
			for(V3DLONG i = mini -r; i <= mini + r; i++)
			{
				if(i < 0 || i >= sz[0]) continue;
				if(inimg3d[k][j][i] < thresh) continue;
				if(inimg3d[k][j][i] > max_int)
				{
					pos[0] = i;
					pos[1] = j;
					pos[2] = k;
					max_int = inimg3d[k][j][i];
				}
			}
		}
	}

	if(inimg3d)delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);

	return true;
}

// this is used for segmentated image
template<class T> bool maximum_connecting_component_center(double * &pos, T* &inimg1d, V3DLONG * &sz)
{
	if(inimg1d == 0 || sz == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	T*** inimg3d = 0;
	V3DLONG * djs1d = 0, *djs3d = 0;
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
		for(int k = 0; k < sz[2]; k++)
	{
		for(int j = 0; j < sz[1]; j++)
		{
			for(int i = 0; i < sz[0]; i++)
			{
				DisjointSet* djs = djs3d[k][j][i];
				djs->i = i; djs->j = j; djs->k = k;
				if
			}
		}
	}
	if(inimg3d)delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);

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
