
/******************************************************************************
 * Fast marching based region growing, will connect sub_marker and tar_marker
 * 
 * Input :  sub_marker      the subject marker
 *          tar_marker      the target marker
 *          inimg1d         original input image
 *
 * Output : outswc          the path from sub_markers to tar_markers, the first markr is in tar_markers, the last marker is in sub_markers
 *
 * very similar to voronoi diagram construction, consecutive diagram won't connect
 * *****************************************************************************/
#include <cstdlib>
#include <cmath>  // sqrt
#include <cassert>
#include <vector>
#include <map>
#include <iostream> // cerr
#include <set>
#include "heap.h"

using namespace std;

#define INF 1.0e300

#define GI(ind) exp(li*pow((1-(inimg1d[ind]-min_int)/max_int),2))


struct MyMarker
{
    MyMarker *parent;
    double x, y, z, radius;
    MyMarker(){x=y=z=radius=0;}
    MyMarker(double _x, double _y, double _z) {x=_x;y=_y;z=_z;radius=0;}
};

template<class T> bool fastmarching_linker(MyMarker sub_marker, MyMarker tar_marker, T * inimg1d, vector<MyMarker*> &outswc, int sz0, int sz1, int sz2, int cnn_type = 2)
{
	enum{ALIVE = -1, TRIAL = 0, FAR = 1};

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	//int cnn_type = 2;  // ?
	cout<<"cnn_type = "<<cnn_type<<endl;

	long sub_ind = (long)(sub_marker.z + 0.5) * sz01 + (long)(sub_marker.y + 0.5) * sz0 + (long)(sub_marker.x + 0.5);
	long tar_ind = (long)(tar_marker.z + 0.5) * sz01 + (long)(tar_marker.y + 0.5) * sz0 + (long)(tar_marker.x + 0.5);
	
	// GI parameter min_int, max_int, li
	double max_int = 0; // maximum intensity, used in GI
	double min_int = INF;
	for(long i = 0; i < tol_sz; i++) 
	{
		if(inimg1d[i] > max_int) max_int = inimg1d[i];
		if(inimg1d[i] < min_int) min_int = inimg1d[i];
	}
	max_int -= min_int;
	double li = 10;
	
	// initialization
	float * phi = new float[tol_sz]; for(long i = 0; i < tol_sz; i++){phi[i] = INF;}
	char * state = new char[tol_sz]; for(long i = 0; i < tol_sz; i++) state[i] = FAR;
	int * parent = new int[tol_sz]; for(int ind = 0; ind < tol_sz; ind++) parent[ind] = ind;

	state[sub_ind] = ALIVE;  phi[sub_ind] = 0.0;

	BasicHeap<HeapElemX> heap;
	map<long, HeapElemX*> elems;

	// init heap
	{
		long index = sub_ind;
		HeapElemX *elem = new HeapElemX(index, phi[index]);
		elem->prev_ind = index;
		heap.insert(elem);
		elems[index] = elem;
	}
	// loop
	int time_counter = 1;
	double process1 = time_counter*1000.0/tol_sz;
	long stop_ind = -1;
	while(!heap.empty())
	{
		double process2 = (time_counter++)*1000.0/tol_sz;
		if(process2 - process1 >= 1){cout<<"\r"<<((int)process2)/10.0<<"%";cout.flush(); process1 = process2;}

		HeapElemX* min_elem = heap.delete_min();
		elems.erase(min_elem->img_ind);

		long min_ind = min_elem->img_ind;
		parent[min_ind] = min_elem->prev_ind;
		if(min_ind == tar_ind){stop_ind = min_ind; break;}

		delete min_elem;

		state[min_ind] = ALIVE;
		int i = min_ind % sz0; 
		int j = (min_ind/sz0) % sz1; 
		int k = (min_ind/sz01) % sz2;
		int w, h, d;

		for(int kk = -1; kk <= 1; kk++)
		{
			d = k+kk;
			if(d < 0 || d >= sz2) continue;
			for(int jj = -1; jj <= 1; jj++)
			{
				h = j+jj;
				if(h < 0 || h >= sz1) continue;
				for(int ii = -1; ii <= 1; ii++)
				{
					w = i+ii;
					if(w < 0 || w >= sz0) continue;
					int offset = ABS(ii) + ABS(jj) + ABS(kk);
					if(offset == 0 || offset > cnn_type) continue;
					double factor = (offset == 1) ? 1.0 : ((offset == 2) ? 1.414214 : ((offset == 3) ? 1.732051 : 0.0));
					long index = d*sz01 + h*sz0 + w;

					if(state[index] != ALIVE)
					{
						double new_dist = phi[min_ind] + (GI(index) + GI(min_ind))*factor*0.5;
						long prev_ind = min_ind;

						if(state[index] == FAR)
						{
							phi[index] = new_dist;
							HeapElemX * elem = new HeapElemX(index, phi[index]);
							elem->prev_ind = prev_ind;
							heap.insert(elem);
							elems[index] = elem;
							state[index] = TRIAL;
						}
						else if(state[index] == TRIAL)
						{
							if(phi[index] > new_dist)
							{
								phi[index] = new_dist;
								HeapElemX * elem = elems[index];
								heap.adjust(elem->heap_id, phi[index]);
								elem->prev_ind = prev_ind;
							}
						}
					}
				}
			}
		}
		// assert(!mask_values.empty());
	}

	V3DLONG in_sz[4] = {sz0, sz1, sz2, 1};
	double thresh = 20;
	// connect markers according to disjoint set
	{
		// add tar_marker
		long ind = stop_ind;
		MyMarker * new_marker = new MyMarker(tar_marker.x, tar_marker.y, tar_marker.z);
                //new_marker->radius = markerRadius(inimg1d, in_sz, *new_marker, thresh);
		new_marker->parent = 0;//	tar_marker;

		outswc.push_back(new_marker);

		MyMarker * par_marker = new_marker;
		ind = parent[ind];
		while(ind != sub_ind)
		{
			int i = ind % sz0;
			int j = ind/sz0 % sz1;
			int k = ind/sz01 % sz2;
			new_marker = new MyMarker(i,j,k);
			new_marker->parent = par_marker;
                        //new_marker->radius = markerRadius(inimg1d, in_sz, *new_marker, thresh);
			outswc.push_back(new_marker);
			par_marker = new_marker;
			ind = parent[ind];
		}
		// add sub_marker
		new_marker = new MyMarker(sub_marker.x, sub_marker.y, sub_marker.z);
		new_marker->parent = par_marker;
                //new_marker->radius = markerRadius(inimg1d, in_sz, *new_marker, thresh);
		outswc.push_back(new_marker);
	}
	cout<<outswc.size()<<" markers linked"<<endl;
	//for(int i = 0; i < sub_markers.size(); i++) outswc.push_back(sub_markers[i]);
	//for(int i = 0; i < tar_markers.size(); i++) outswc.push_back(tar_markers[i]);
	
	if(state) {delete [] state; state = 0;}
	if(phi) {delete [] phi; phi = 0;}
	if(parent) {delete [] parent; parent = 0;}
	return true;
}
