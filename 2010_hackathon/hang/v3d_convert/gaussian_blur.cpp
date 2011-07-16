#ifndef __GAUSSIAN_BLUR_SRC__
#define __GAUSSIAN_BLUR_SRC__

#include <iostream>
#include <vector>
#include <cmath>

#include "basic_memory.cpp"
#include "v3d_basicdatatype.h"

#include "gaussian_blur.h"

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
