#ifndef __GAUSSIAN_BLUR_H_H__
#define __GAUSSIAN_BLUR_H_H__

#include "v3d_basicdatatype.h"

// outimg1d = 0
template <class T1, class T2> bool compute_gaussian_blur(T2 * &outimg1d, T1 * inimg1d,V3DLONG sz[3], double sigma, int r);

template<class T1, class T2> bool smooth(T1 * &dst, T2 const * src, V3DLONG sz[3], double s);

//#include "gaussian_blur.cpp"
#endif
