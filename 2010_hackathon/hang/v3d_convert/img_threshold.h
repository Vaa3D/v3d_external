#ifndef _IMG_THRESHOLD_H_
#define _IMG_THRESHOLD_H_

#include <algorithm>
#include <map>
#include <vector>
#include <cmath>

#ifdef __THRESH_DEBUG__
	#include <iostream>
#endif

#include "sort_algorithms.h"

using namespace std;

template<class T> bool black_threshold(double thresh_value, T* &inimg1d, V3DLONG sz[3], T* &outimg1d)
{
	if(inimg1d == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	V3DLONG size = sz[0] * sz[1] * sz[2];
	if(outimg1d == 0) outimg1d = new T[size];

	for(V3DLONG i = 0; i < size; i++) outimg1d[i] = inimg1d[i] >= thresh_value ? inimg1d[i] : 0;
	return true;
}

template<class T> bool white_threshold(double thresh_value, T* &inimg1d, V3DLONG sz[3], T* &outimg1d)
{
	if(inimg1d == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	V3DLONG size = sz[0] * sz[1] * sz[2];
	if(outimg1d == 0) outimg1d = new T[size];

	T max_value;
	if(sizeof(T) == 1) max_value = (T)255;
	else if(sizeof(T) == 2) max_value = (T)65535;
	else if(sizeof(T) == 4) max_value = (T)4294967295;
	else max_value = (T)1.0;

	for(V3DLONG i = 0; i < size; i++) outimg1d[i] = inimg1d[i] >= thresh_value ? max_value : inimg1d[i];
	return true;
}

template<class T> bool binary_threshold(double thresh_value, T* &inimg1d, V3DLONG sz[3], T* &outimg1d)
{
	if(inimg1d == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	V3DLONG size = sz[0] * sz[1] * sz[2];
	if(outimg1d == 0) outimg1d = new T[size];

	T max_value, min_value = (T)0;
	if(sizeof(T) == 1) max_value = (T)255;
	else if(sizeof(T) == 2) max_value = (T)65535;
	else if(sizeof(T) == 4) max_value = (T)4294967295;
	else max_value = (T)1.0;

	for(V3DLONG i = 0; i < size; i++) outimg1d[i] = inimg1d[i] >= thresh_value ? max_value : min_value;
	return true;
}

// refer to http://www.labbookpages.co.uk/software/imgProc/otsuThreshold.html
template<class T> bool otsu_threshold(double & thresh_value, T* &inimg1d, V3DLONG sz[3])
{
	if(inimg1d == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;

	int nlevels = 1 << (sizeof(T) * 8);
	long tol_sz = sz[0] * sz[1] * sz[2];
	vector<double> hist(nlevels, 0.0);
	double sum_int1 = 0.0, sum_int2 = 0.0;
	double sum_num1 = 0.0, sum_num2 = tol_sz;
	for(long i = 0; i < tol_sz; i++)
	{
		hist[inimg1d[i]]++;
		sum_int2 += inimg1d[i];
	}

	double w1 = 0.0,  w2 = 1.0;
	double mu1 = 0.0, mu2 = 0.0;

	int max_t = 0;
	double max_b = 0.0;

	for(int t = 0; t < nlevels - 1; t++)
	{
		//cout<<hist[t]<<"\t";
		sum_num1 += hist[t];
		sum_num2 -= hist[t];
		if(sum_num2 <= 0) break;

		sum_int1 += hist[t] * t;
		sum_int2 -= hist[t] * t;
		w1 = sum_num1 / tol_sz;
		w2 = sum_num2 / tol_sz;
		mu1 = sum_int1 / sum_num1;
		mu2 = sum_int2 / sum_num2;

		double b = w1*w2*(mu1-mu2)*(mu1-mu2);
		
		if(b > max_b){
			max_b = b;
			max_t = t;
		}
	}
	thresh_value = max_t;
//	if(hist){delete [] hist; hist = 0;}
	return true;
}
template<class T>
void adaptive_threshold(T * &inimg1d, T * &outimg1d, )
{
}
#endif
