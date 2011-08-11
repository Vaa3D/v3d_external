#ifndef _IMG_THRESHOLD_H_
#define _IMG_THRESHOLD_H_

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

#endif
