// Feb 27, 2012      Hang Xiao

#ifndef __FASTMARCHING_LINKER_H__
#define __FASTMARCHING_LINKER_H__

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
#include <time.h>
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

void clean_fm_marker_vector(vector<MyMarker*> &outswc)
{
    for (V3DLONG i=outswc.size()-1;i>=0;i--)
    {
         if (outswc.at(i)) {delete outswc.at(i); outswc.at(i)=0; }
        outswc.pop_back();
    }
}

// if time==0, then do not consider consuming time, time is in seconds
template<class T> bool fastmarching_linker(vector<MyMarker> &sub_markers,vector<MyMarker> & tar_markers,
										   T * inimg1d, vector<MyMarker *> &outswc, int sz0, int sz1, int sz2, float time, int cnn_type = 2)
{
        int ALIVE = -1;
		int TRIAL = 0;
		int FARST = 1;

		clock_t t1=clock(); // start time
        long tol_sz = sz0 * sz1 * sz2;
        long sz01 = sz0 * sz1;
        //int cnn_type = 2;  // ?
        cout<<"cnn_type = "<<cnn_type<<endl;

        float * phi = new float[tol_sz]; for(long i = 0; i < tol_sz; i++){phi[i] = INF;}
        map<long, MyMarker> sub_map, tar_map;
        for(long i = 0; i < tar_markers.size(); i++)
        {
                int x = tar_markers[i].x + 0.5;
                int y = tar_markers[i].y + 0.5;
                int z = tar_markers[i].z + 0.5;
                long ind = z*sz01 + y*sz0 + x;
                tar_map[ind] = tar_markers[i];
        }

        for(long i = 0; i < sub_markers.size(); i++)
        {
                int x = sub_markers[i].x + 0.5;
                int y = sub_markers[i].y + 0.5;
                int z = sub_markers[i].z + 0.5;
                long ind = z*sz01 + y*sz0 + x;
                sub_map[ind] = sub_markers[i];
        }

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
        char * state = new char[tol_sz];
        for(long i = 0; i < tol_sz; i++) state[i] = FARST;

        vector<long> submarker_inds;
        for(long s = 0; s < sub_markers.size(); s++) {
                int i = sub_markers[s].x + 0.5;
                int j = sub_markers[s].y + 0.5;
                int k = sub_markers[s].z + 0.5;
                long ind = k*sz01 + j*sz0 + i;
                submarker_inds.push_back(ind);
                state[ind] = ALIVE;
                phi[ind] = 0.0;
        }
        int * parent = new int[tol_sz]; for(int ind = 0; ind < tol_sz; ind++) parent[ind] = ind;

        BasicHeap<HeapElemX> heap;
        map<long, HeapElemX*> elems;

        // init heap
        for(long s = 0; s < submarker_inds.size(); s++)
        {
                long index = submarker_inds[s];
                HeapElemX *elem = new HeapElemX(index, phi[index]);
                elem->prev_ind = index;
                heap.insert(elem);
                elems[index] = elem;
        }
        // loop
        int time_counter = sub_markers.size();
        double process1 = time_counter*1000.0/tol_sz;
        long stop_ind = -1;
        while(!heap.empty())
        {
                double process2 = (time_counter++)*1000.0/tol_sz;
                if(process2 - process1 >= 1){cout<<"\r"<<((int)process2)/10.0<<"%";cout.flush(); process1 = process2;}
                // time consuming until this pos
                if((time!=0)&&(((clock()-t1) > time*CLOCKS_PER_SEC) ))
                {
                     return false;
                }

                HeapElemX* min_elem = heap.delete_min();
                elems.erase(min_elem->img_ind);

                long min_ind = min_elem->img_ind;
                parent[min_ind] = min_elem->prev_ind;
                if(tar_map.find(min_ind) != tar_map.end()){stop_ind = min_ind; break;}

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

                                                if(state[index] == FARST)
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
                MyMarker tar_marker = tar_map[stop_ind];
                MyMarker * new_marker = new MyMarker(tar_marker.x, tar_marker.y, tar_marker.z);
                //new_marker->radius = markerRadius(inimg1d, in_sz, *new_marker, thresh);
                new_marker->parent = 0; //tar_marker;

                outswc.push_back(new_marker);

                MyMarker * par_marker = new_marker;
                ind = parent[ind];
                while(sub_map.find(ind) == sub_map.end())
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
                MyMarker sub_marker = sub_map[ind];
                new_marker = new MyMarker(sub_marker.x, sub_marker.y, sub_marker.z);
                new_marker->parent = par_marker;
                //new_marker->radius = markerRadius(inimg1d, in_sz, *new_marker, thresh);
                outswc.push_back(new_marker);
        }
        cout<<outswc.size()<<" markers linked"<<endl;
        //for(int i = 0; i < sub_markers.size(); i++) outswc.push_back(sub_markers[i]);
        //for(int i = 0; i < tar_markers.size(); i++) outswc.push_back(tar_markers[i]);

		if(!elems.empty()) for(map<long, HeapElemX*>::iterator it = elems.begin(); it != elems.end(); it++) delete it->second;
        if(phi) {delete [] phi; phi = 0;}
        if(parent) {delete [] parent; parent = 0;}
        if(state) {delete [] state; state = 0;}
        return true;
}


#endif



