//a header file that includes several missing function from Visual Studio but exist in other GNU c++ library
// by Yu Yang and Hanchuan Peng
// 2010-05-20

#ifndef __VC_DIFF_H__
#define __VC_DIFF_H__

#include <math.h>
#include <string.h>

#include "v3d_basicdatatype.h"

//strcasecmp strcmpi

#define isnan(x) ((x) != (x))

double round(double x);

V3DLONG floor(V3DLONG x);

int fabs(int x);

double log(int x);

V3DLONG lround(V3DLONG x);

//bool strcasecmp(const char * str1, const char * str2);

double log2(double x);

double pow(V3DLONG a, V3DLONG b);


//V3DLONG abs(V3DLONG a);



#endif
