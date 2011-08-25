#ifndef __NEURON_TRACING_H__
#define __NEURON_TRACING_H__
#include "img_threshold.h"
#include "img_segment.h"
#include <cmath>
#include <set>
using namespace std;
//#include "basic_surf_objs.h"
struct ImageMarker
{
	double x;
	double y;
	double z;
	double radius;
	ImageMarker* parent;
	ImageMarker(){x=y=z=radius=0.0; parent=0;}
};
template<class T> double markerRadius(T*** &inimg3d, V3DLONG * sz, ImageMarker & marker);
#define PI 3.1415926
#ifndef MAX
#define MAX(x,y) (x > y ? (x) : (y))
#endif
inline double dist(ImageMarker &a, ImageMarker &b)
{
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}
#define BACKGROUND 0
// inimg1d is binarized outside, 0 for background
template<class T> double markerRadius(T* &inimg1d, V3DLONG * sz, ImageMarker & marker, double thresh)
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
	double r = markerRadius(inimg3d, sz, marker, thresh);
	if(inimg3d)delete3dpointer(inimg3d, sz[0], sz[1], sz[2]);
	return r;
}
template<class T> double markerRadius(T*** &inimg3d, V3DLONG * sz, ImageMarker & marker, double thresh)
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
							if(inimg3d[z[kk]][y[jj]][x[ii]] < thresh){cout<<"background"<<endl; bak_num++;}
							if((bak_num / tol_num) > 0.0001) return ir;
						}
					}
				}
			}
		}
	}
	return ir;
}
template<class T> void coordinate_transformation(T &x, T &y, T &z, double thetax, double thetay, ImageMarker &marker)
{
	T x0 = x;
	T y0 = y*cos(thetax) - z * sin(thetax);
	T z0 = y * sin(thetax) + z * cos(thetax);

	x = x0 * cos(thetay) + z0 * sin(thetay);
	y = y0;
	z = -x0 * sin(thetay) + z0 * cos(thetay);

	x += marker.x;
	y += marker.y;
	z += marker.z;
}

template<class T> void coordinate_transformation(T &x, T &y, T &z, double &sin_thetax, double &cos_thetax, double &sin_thetay, double &cos_thetay, ImageMarker &marker)
{
	T x0 = x;
	T y0 = y*cos_thetax - z * sin_thetax;
	T z0 = y * sin_thetax + z * cos_thetax;

	x = x0 * cos_thetay + z0 * sin_thetay;
	y = y0;
	z = -x0 * sin_thetay + z0 * cos_thetay;

	x += marker.x;
	y += marker.y;
	z += marker.z;
}

template<class T1, class T2> bool get_tangent_plane(T1* &outimg1d, V3DLONG * &out_sz, T2* &inimg1d, V3DLONG * &in_sz, ImageMarker & marker1, ImageMarker &marker2, double factor)
{
	if(inimg1d == 0 || in_sz[0] <= 0 || in_sz[1] <= 0 || in_sz[2] <= 0) return false;
	if(out_sz == 0) out_sz = new V3DLONG[4];
	V3DLONG r = factor * marker2.radius > 1.0 ? factor * marker2.radius + 0.5 : 1.0;
	out_sz[0] = 2 * r + 1;
	out_sz[1] = 2 * r + 1;
	out_sz[2] = 1;
	out_sz[3] = 1;
	if(outimg1d == 0) outimg1d = new T1[out_sz[0] * out_sz[1] * out_sz[2] * out_sz[3]];

	double A = marker2.x - marker1.x;
	double B = marker2.y - marker1.y;
	double C = marker2.z - marker1.z;

	double stx = B / sqrt(B*B + C*C);
	double ctx = C / sqrt(B*B + C*C);
	double sty = A / sqrt(A*A + C*C);
	double cty = C / sqrt(A*A + C*C);

	V3DLONG ix, iy, iz = 0;
	double ox, oy, oz;
	for(iy = -r; iy <= r; iy++)
	{
		for(ix = -r; ix <= r; ix++)
		{
			ox=ix, oy=iy, oz=iz;
			coordinate_transformation(ox,oy,oz,stx,ctx,sty,cty,marker2);
			V3DLONG sx = ix + r, sy = iy + r;
			V3DLONG tx = ox + 0.5, ty = oy + 0.5, tz = oz + 0.5;
			V3DLONG s_ind = sy * out_sz[0] + sx;
			V3DLONG t_ind = tz * in_sz[1] * in_sz[0] + ty * in_sz[0] + tx;
			if((ix * ix + iy * iy) > r * r || tx < 0 || tx >= in_sz[0] || ty < 0 || ty >= in_sz[1] || tz < 0 || tz >= in_sz[2])
			{
				outimg1d[s_ind] = 0;
			}
			else 
			{
				outimg1d[s_ind] = inimg1d[t_ind];
			}
		}
	}
	return true;
}
template<class T> vector<ImageMarker> mean_shift_centers(T* &inimg1d, V3DLONG *&sz)
{

}
// assume sz is two dimension
template<class T> vector<ImageMarker> threshold_weighted_centers(T* &inimg1d, V3DLONG * &sz, double thresh)
{
	vector<ImageMarker> center_marker;
	V3DLONG tol_sz = sz[0] * sz[1] * sz[2];
	DisjointSetWithRankAndLink * djs = 0; construct_disjoint_set(djs, inimg1d, sz, thresh);
	set<DisjointSetWithRankAndLink*> forest;
	for(V3DLONG i = 0; i < tol_sz; i++)
	{
		DisjointSetWithRankAndLink * root = find_set(djs + i);
		if(root->sz >=2) forest.insert(root);
	}
	set<DisjointSetWithRankAndLink*>::iterator it = forest.begin();
	while(it != forest.end())
	{
		DisjointSetWithRankAndLink * root = *it;

		ImageMarker marker;
		marker.x = 0; marker.y = 0; marker.z = 0;
		marker.radius = 0.0;
		int pos = root - djs;
		int x = pos % sz[0];
		int y = pos / sz[0];
		marker.x += inimg1d[pos] * x;
		marker.y += inimg1d[pos] * y;
		
		DisjointSetWithRankAndLink * p = root->next_node;
		while(p != root)
		{
			pos = p - djs;
			x = pos % sz[0];
			y = pos / sz[0];

			marker.x += inimg1d[pos] * x;
			marker.y += inimg1d[pos] * y;
			p = p->next_node;
		}
		marker.x /= root->sz;
		marker.y /= root->sz;
		center_marker.push_back(marker);
		it++;
	}
}
// inimg1d could be image intensity, image distance transformation, or else
template<class T> vector<ImageMarker> get_next_markers(T* &inimg1d, V3DLONG * &in_sz, ImageMarker &marker1, ImageMarker &marker2, double min_thresh)
{
	vector<ImageMarker*> marker_centers;
	T* outimg1d = 0; 
	V3DLONG * out_sz = 0;
	double factor = 1.0;
	get_tangent_plane(outimg1d, out_sz, inimg1d, in_sz, marker1, marker2, factor);
	double avg_thresh = 0.0; average_threshold(avg_thresh, outimg1d, out_sz);
	avg_thresh = avg_thresh * 4.0 / PI; // the ratio of circle area and square area
	if(avg_thresh < min_thresh) return marker_centers;
	marker_centers = threshold_weighted_centers(outimg1d, out_sz, avg_thresh);
	if(outimg1d){delete [] outimg1d; outimg1d = 0;}
	return marker_centers;
}
//template<class T> bool get_initial_two_directions(T*** inimg3d, V3DLONG * &sz, ImageMarker &marker, ImageMarker &marker1, ImageMarker &marker2)
//{
//}

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
