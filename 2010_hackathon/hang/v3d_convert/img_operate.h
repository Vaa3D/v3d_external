#ifndef __IMG_OPERATE__
#define __IMG_OPERATE__
#include "v3d_basicdatatype.h"
#include <string>
using namespace std;

#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : (-x))
#endif

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

// larger than 255 will be set to 255 
template<class T> bool img_operation(string method, T* &outimg, T* &inimg1, V3DLONG *&sz, T* &inimg2)
{
	V3DLONG tol_sz = sz[0] * sz[1] * sz[2] * sz[3];
	if(method == "plus") return img_plus(outimg, inimg1, inimg2, tol_sz);
	if(method == "minus") return img_minus(outimg, inimg1, inimg2, tol_sz);
	if(method == "absminus") return img_absminus(outimg, inimg1, inimg2, tol_sz);
	if(method == "multiply") return img_multiply(outimg, inimg1, inimg2, tol_sz);
	if(method == "divide") return img_divide(outimg, inimg1, inimg2, tol_sz);
	if(method == "complement") return img_complement(outimg, inimg1, tol_sz);
	if(method == "and") return img_and(outimg, inimg1, inimg2, tol_sz);
	if(method == "or") return img_or(outimg, inimg1, inimg2, tol_sz);
	if(method == "xor") return img_xor(outimg, inimg1, inimg2, tol_sz);
	if(method == "not") return img_not(outimg, inimg1, tol_sz);
}

template<class T> bool img_plus(T* &outimg, T* &inimg1, T* &inimg2, V3DLONG tol_sz)
{
	if(inimg1 == 0 || inimg2 == 0 || tol_sz <= 0) return false;
	if(outimg == 0) outimg = new T[tol_sz];
	T max_value = (T)((1 << (sizeof(T) * 8)) - 1);
	for(V3DLONG i = 0; i < tol_sz; i++) 
		outimg[i] = MIN((inimg1[i] + inimg2[2]), max_value);
	
	return true;
}
// lower than 0 will be set to 0
template<class T> bool img_minus(T* &outimg, T* &inimg1, T* &inimg2, V3DLONG tol_sz)
{
	if(inimg1 == 0 || inimg2 == 0 || tol_sz <= 0) return false;
	if(outimg == 0) outimg = new T[tol_sz];
	for(V3DLONG i = 0; i < tol_sz; i++) outimg[i] = MAX((inimg1[i] - inimg2[i]), 0);
	
	return true;
}
// absolute difference 
template<class T> bool img_absminus(T* &outimg, T* &inimg1, T* &inimg2, V3DLONG tol_sz)
{
	if(inimg1 == 0 || inimg2 == 0 || tol_sz <= 0) return false;
	if(outimg == 0) outimg = new T[tol_sz];

	for(V3DLONG i = 0; i < tol_sz; i++) outimg[i] = ABS(inimg1[i] - inimg2[i]);
	
	return true;
}

// multiply two image, larger than 255 will be set to 255
template<class T> bool img_multiply(T* &outimg, T* &inimg1, T* &inimg2, V3DLONG tol_sz)
{
	if(inimg1 == 0 || inimg2 == 0 || tol_sz <= 0) return false;
	if(outimg == 0) outimg = new T[tol_sz];
	T max_value = (T)((1 << (sizeof(T) * 8)) - 1);
	for(V3DLONG i = 0; i < tol_sz; i++) outimg[i] = MIN((inimg1[i] * inimg2[i]), max_value);
	
	return true;
}

// inimg2[i] / inimg1[i] * 255
template<class T> bool img_divide(T* &outimg, T* &inimg1, T* &inimg2, V3DLONG tol_sz)
{
	if(inimg1 == 0 || inimg2 == 0 || tol_sz <= 0) return false;
	if(outimg == 0) outimg = new T[tol_sz];
	T max_value = (T)((1 << (sizeof(T) * 8)) - 1);
	for(V3DLONG i = 0; i < tol_sz; i++) outimg[i] = inimg2[i] > inimg1[1] ? max_value : 0;
	
	return true;
}

template<class T> bool img_complement(T* &outimg, T* &inimg, V3DLONG tol_sz)
{
	if(inimg == 0 || tol_sz <= 0) return false;
	if(outimg == 0) outimg = new T[tol_sz];
	T max_value = (T)((1 << (sizeof(T) * 8)) - 1);
	for(V3DLONG i = 0; i < tol_sz; i++) outimg[i] = (max_value - inimg[i]);
	
	return true;
}

// refer to http://www.cplusplus.com/doc/boolean/
template<class T> bool img_and(T* &outimg, T* &inimg1, T* &inimg2, V3DLONG tol_sz)
{
	if(inimg1 == 0 || inimg2 == 0 || tol_sz <= 0) return false;
	if(outimg == 0) outimg = new T[tol_sz];
	for(V3DLONG i = 0; i < tol_sz; i++) outimg[i] = (inimg2[i] & inimg1[1]);
	
	return true;
}

template<class T> bool img_or(T* &outimg, T* &inimg1, T* &inimg2, V3DLONG tol_sz)
{
	if(inimg1 == 0 || inimg2 == 0 || tol_sz <= 0) return false;
	if(outimg == 0) outimg = new T[tol_sz];
	for(V3DLONG i = 0; i < tol_sz; i++) outimg[i] = (inimg2[i] | inimg1[1]);
	
	return true;
}

template<class T> bool img_xor(T* &outimg, T* &inimg1, T* &inimg2, V3DLONG tol_sz)
{
	if(inimg1 == 0 || inimg2 == 0 || tol_sz <= 0) return false;
	if(outimg == 0) outimg = new T[tol_sz];
	for(V3DLONG i = 0; i < tol_sz; i++) outimg[i] = (inimg2[i] ^ inimg1[1]);
	
	return true;
}
template<class T> bool img_not(T* &outimg, T* &inimg, V3DLONG tol_sz)
{
	if(inimg == 0 || tol_sz <= 0) return false;
	if(outimg == 0) outimg = new T[tol_sz];
	for(V3DLONG i = 0; i < tol_sz; i++) outimg[i] = (~inimg[i]);
	
	return true;
}
#endif
