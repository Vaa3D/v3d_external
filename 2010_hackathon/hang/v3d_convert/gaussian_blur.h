#ifndef __GAUSSIAN_BLUR_H__
#define __GAUSSIAN_BLUR_H__

#include <iostream>
#include <vector>
#include <cmath>

#include "basic_memory.cpp"
#include "v3d_basicdatatype.h"

#include "gaussian_blur.h"

#define MAX(a,b) (a > b) ? (a) : (b)
#define MIN(a,b) (a < b) ? (a) : (b)
// W - window width or radius
template<class T1, class T2> void convolve_xyz(T1* dst, T2 const * src, V3DLONG sz[3], const double * filter, V3DLONG W)
{
	T1 * temp = 0;
	T2 *** src3d = 0;
	T1 *** dst3d = 0;
	T1 *** temp3d = 0;
	try
	{
		temp = new T1[sz[0] * sz[1] * sz[2]];
		new3dpointer(src3d, sz[0], sz[1], sz[2], src);
		new3dpointer(dst3d, sz[0], sz[1], sz[2], dst);
		new3dpointer(temp3d, sz[0], sz[1], sz[2], temp);
	}
	catch(...)
	{
		if(temp) {delete [] temp; temp = 0;}
		if(src3d) delete3dpointer(src3d, sz[0], sz[1], sz[2]);
		if(dst3d) delete3dpointer(dst3d, sz[0], sz[1], sz[2]);
		if(temp3d) delete3dpointer(temp3d, sz[0], sz[1], sz[2]);
	}

	// along x
	
	for(V3DLONG k = 0; k < sz[2]; k++){
		for(V3DLONG j = 0; j < sz[1]; j++){
			for(V3DLONG i = 0; i < sz[0]; i++){
				double acc = 0.0;
				if(i - W < 0){
					for(V3DLONG ii = 0; ii < W -i; ii++) acc += filter[ii] * src3d[k][j][0];
				}
				if(i + W > sz[0] - 1){
					for(V3DLONG ii = 0; ii < i+W - sz[0] +1; ii++) acc += filter[2*W - ii] * src3d[k][j][sz[0]-1];
				}
				V3DLONG start = MAX(0, i-W);
				V3DLONG stop  = MIN(i+W, sz[0] - 1);
				for(V3DLONG ii = start; ii <= stop; ii++) acc += filter[ii - i + W] * src3d[k][j][ii];
				dst3d[k][j][i] = (T1)acc;
			}
		}
	}

	// along y
	for(V3DLONG k = 0; k < sz[2]; k++){
		for(V3DLONG i = 0; i < sz[0]; i++){
			for(V3DLONG j = 0; j < sz[1]; j++){
				double acc = 0.0;
				if(j - W < 0){
					for(V3DLONG jj = 0; jj < W - j; jj++) acc += filter[jj] * dst3d[k][0][i];
				}
				if(j + W > sz[1] - 1){
					for(V3DLONG jj = 0; jj < j + W - sz[1] +1; jj++) acc += filter[2*W - jj] * dst3d[k][sz[1]-1][i];
				}
				V3DLONG start = MAX(0, j-W);
				V3DLONG stop  = MIN(j+W, sz[1] - 1);
				for(V3DLONG jj = start; jj <= stop; jj++) acc += filter[jj - j + W] * dst3d[k][jj][i];
				temp3d[k][j][i] = acc;
			}
		}
	}

	// along z
	for(V3DLONG j = 0; j < sz[1]; j++){
		for(V3DLONG i = 0; i < sz[0]; i++){
			for(V3DLONG k = 0; k < sz[2]; k++){
				double acc = 0.0;
				if(k - W < 0){
					for(V3DLONG kk = 0; kk < W - k; kk++) acc += filter[kk] * temp3d[0][j][i];
				}
				if(k + W > sz[2] - 1){
					for(V3DLONG kk = 0; kk < k + W - sz[2] +1; kk++) acc += filter[2*W - kk] * temp3d[sz[2]-1][j][i];
				}
				V3DLONG start = MAX(0, k-W);
				V3DLONG stop  = MIN(k+W, sz[2] - 1);
				for(V3DLONG kk = start; kk <= stop; kk++) acc += filter[kk - k + W] * temp3d[kk][j][i];
				dst3d[k][j][i] = acc;
			}
		}
	}

	if(temp) {delete [] temp; temp = 0;}
	if(src3d) delete3dpointer(src3d, sz[0], sz[1], sz[2]);
	if(dst3d) delete3dpointer(dst3d, sz[0], sz[1], sz[2]);
	if(temp3d) delete3dpointer(temp3d, sz[0], sz[1], sz[2]);
}


template<class T1, class T2> bool smooth(T1 * &dst, T2 const * src, V3DLONG sz[3], double s)
{
	if(dst == 0 || src == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0 || s <= 0.0) return false;
	V3DLONG W = V3DLONG(ceil(4.0f * s));
	double * filter = 0;
	V3DLONG filterReserved = 0;

	if(filterReserved < W){
		filterReserved = W;
		if(filter) delete [] filter;
		filter = new double[2*filterReserved + 1];
	}

	double acc = 0.0f;
	for(V3DLONG j = 0; j < 2*W+1; ++j)
	{
		filter[j] = std::exp(-0.5 * (j-W)*(j-W)/(s*s));
		acc += filter[j];
	}
	for(V3DLONG j = 0; j < 2*W+1; ++j)
	{
		filter[j] /= acc ;
	}
	convolve_xyz(dst, src, sz, filter, W); 
	return true;
}

template <class T1, class T2> bool compute_gaussian_blur(T2 * &outimg1d, T1 * inimg1d,V3DLONG sz[3], double sigma, int r)
{
	if(!inimg1d || sigma < 0.0 || r%2 != 1) return false;
	V3DLONG i, j, k;
	V3DLONG ii, jj, kk;
	V3DLONG size = r*r*r;

	T1 *** indata3d = 0 ;
	T2 *** outdata3d = 0;

	double * gauss1d = 0;
	double *** gauss3d = 0; 

	try
	{
		outimg1d = new T2[sz[0]*sz[1]*sz[2]];
		gauss1d = new double[size];

		new3dpointer(indata3d, sz[0], sz[1], sz[2], inimg1d);
		new3dpointer(outdata3d, sz[0], sz[1], sz[2], outimg1d);
		new3dpointer(gauss3d, r, r, r, gauss1d);
	}
	catch(...)
	{
		std::cerr<<"Not enough memory!"<<std::endl;
		if(outimg1d) {delete[] outimg1d; outimg1d = 0;}
		if(gauss1d)  {delete[] gauss1d; gauss1d = 0;}
		if(indata3d) delete3dpointer(indata3d, sz[0],sz[1],sz[2]);
		if(outdata3d) delete3dpointer(outdata3d, sz[0],sz[1],sz[2]);
		if(gauss3d) delete3dpointer(gauss3d, r, r, r);
		return false;
	}

	int rr = r/2;
	double PI = 3.1415926;
	double A = 1.0; //1.0/(2.0*PI*sigma*sigma);
	double B = -1.0/(2.0*sigma*sigma);

	double C = 0.0;
	for(k = 0; k < r; k++)
	{
		for(j = 0; j < r; j++)
		{
			for(i = 0; i < r; i++)
			{
				if(k <= rr && j <= rr && i <= rr){
					double D = (k - rr) * (k - rr) + (j - rr)*(j - rr) + (i - rr)*(i - rr); 
					gauss3d[k][j][i] = A * exp(B * D); 
				}
				else
				{
					V3DLONG k2 = k , j2 = j, i2 = i;
					if(k > rr) k2 = r - k - 1;
					if(j > rr) j2 = r - j - 1;
					if(i > rr) i2 = r - i - 1;
					gauss3d[k][j][i] = gauss3d[k2][j2][i2];
				}
				C += gauss3d[k][j][i];
			}
		}
	}

	// uniform 
	try
	{
		for(i = 0; i < size; i++) 
		{
			gauss1d[i] /= C;
		}
	}
	catch(...)
	{
		std::cout<<"divided by zero in gausian matrix"<<std::endl;
	}

	// main loop
	for(k = 0; k < sz[2]; k++)
	{
		for(j = 0; j < sz[1]; j++)
		{
			for(i = 0; i < sz[0]; i++)
			{
				double sumi = 0.0;
				double sumg = 1.0;
				for(kk = k - rr; kk <= k + rr; kk++)
				{
					for(jj = j - rr; jj <= j + rr; jj++)
					{
						for(ii = i - rr; ii <= i + rr; ii++)
						{
							if( ii >= 0 && jj >= 0 && kk >= 0 &&
									ii < sz[0] && jj < sz[1] && kk < sz[2])
								sumi += indata3d[kk][jj][ii] * gauss3d[kk - k + rr][jj - j + rr][ii - i + rr];
							else
							{
								sumg -= gauss3d[kk - k + rr][jj - j + rr][ii - i + rr];
							}
						}
					}
				}
				outdata3d[k][j][i] =(T2)(sumi /sumg);
			}
		}
	}

	if(gauss1d)  {delete[] gauss1d; gauss1d = 0;}
	if(indata3d) delete3dpointer(indata3d, sz[0],sz[1],sz[2]);
	if(outdata3d) delete3dpointer(outdata3d, sz[0],sz[1],sz[2]);
	if(gauss3d) delete3dpointer(gauss3d, r, r, r);
	return true;
}

#endif
