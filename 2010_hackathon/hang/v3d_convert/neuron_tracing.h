#ifndef __NEURON_TRACING_H__
#define __NEURON_TRACING_H__
#include <cmath>
//#include "basic_surf_objs.h"
struct ImageMarker
{
	double x;
	double y;
	double z;
	double radius;
};
template<class T> double markerRadius(T*** &inimg3d, V3DLONG * sz, ImageMarker & marker);

#ifndef MAX
#define MAX(x,y) (x > y ? (x) : (y))
#endif
inline double dist(ImageMarker &a, ImageMarker &b)
{
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}
#define BACKGROUND 0
// inimg1d is binarized outside, 0 for background
template<class T> double markerRadius(T* &inimg1d, V3DLONG * sz, ImageMarker & marker)
{
	T*** inimg3d = 0;
	try
	{
		new3dpointer(inimg3d, sz[0], sz[1], sz[2], inimg1d);
	}
	catch(...)
	{
		if(inimg3d)delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);
	}
	double r = markerRadius(inimg3d, sz, marker);
	if(inimg3d)delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);
	return r;
}
template<class T> double markerRadius(T*** &inimg3d, V3DLONG * sz, ImageMarker & marker)
{
	double max_r = MAX(MAX(sz[0]/2.0, sz[1]/2.0), sz[2]/2.0);
	double ir;
	double tol_num, bak_num;
	double mx = marker.x;
	double my = marker.y;
	double mz = marker.z;
	cout<<"mx = "<<mx<<" my = "<<my<<" mz = "<<mz<<endl;
	V3DLONG x[2], y[2], z[2];

	double factor = 1.0/sqrt(3.0);
	tol_num = bak_num = 0.0;
	for(ir = 1; ir <= max_r; ir++)
	{
		double r1 = (ir -1)*factor;
		double r2 = ir;
		for(V3DLONG k = r1 ; k < r2; k++)
		{
			for(V3DLONG j = r1; j < r2; j++)
			{
				for(V3DLONG i = r1; i < r2; i++)
				{
					double dist = i * i + j * j + k * k;
					if(dist >= ir * ir || dist < (ir -1 )*(ir -1)) continue;
					x[0] = mx - i, x[1] = mx + i;
					y[0] = my - j, y[1] = my + i;
					z[0] = mz - k, z[1] = mz + k;
					for(char b = 0; b < 8; b++)
					{
						char ii = b & 0x01, jj = (b >> 1) & 0x01, kk = (b >> 2) & 0x01;
						if(x[ii]<0 || x[ii] >= sz[0] || y[jj]<0 || y[jj] >= sz[1] || z[kk]<0 || z[kk] >= sz[2]) return ir;
						else
						{
							tol_num++;
							cout<<"foreground"<<endl;
							if(inimg3d[z[kk]][y[jj]][x[ii]] == BACKGROUND){cout<<"background"<<endl; bak_num++;}
							else cout<<"foreground"<<endl;
							if((bak_num / tol_num) > 0.0001) return ir;
						}
					}
				}
			}
		}
	}
	return ir;
}

template<class T> bool get_initial_two_directions(T*** inimg3d, V3DLONG * &sz, ImageMarker &marker, ImageMarker &marker1, ImageMarker &marker2)
{
}

// inimg1d is binarized to 0 and 1, pDist is 0 outsize, plus inside
// return the number of condidate positions
/*template<class T> int neuron_tracing(double * pDist, T*** inimg3d, NP &x1, NP &x2, NP * &ox)
{
	// check validation

	if(!inimg3d[iy.k][iy.j][iy.i]) return 0;
	V3DLONG i, j, k;
	double d = dst;
	i = 0;
	return 0;
}
*/
#endif
