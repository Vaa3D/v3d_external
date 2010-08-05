//the implementation of several missing function from Visual Studio but exist in other GNU c++ library
// by Yu Yang and Hanchuan Peng
// 2010-05-20

#include "vcdiff.h"

double round(double x)
{
	return x < 0 ? -floor(fabs(x) + .5) : floor(x + .5);
}
V3DLONG floor(V3DLONG x)
{
	return (V3DLONG)floorl((double)x);
}

int fabs(int x)
{
	return (int)(fabs((double)x));
}


double log(int x)
{
	return (double)(log((double)x));
}

V3DLONG lround(V3DLONG x)
{
	return (x < 0) ? -floor(fabs(double(x)) + .5) : floor(x + .5);
}

//bool strcasecmp(const char * str1, const char * str2)
//{
//	return strcmpi(str1, str2);
//}

double log2(double x)
{
	return log(x)/log(2.0);
}

double pow(V3DLONG a, V3DLONG b)
{
	return pow(double(a), int(b));
}

/*
V3DLONG abs(V3DLONG a)
{
	return (V3DLONG)abs(double(a));
}
*/
