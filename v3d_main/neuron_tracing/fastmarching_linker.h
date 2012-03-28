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

#define INF 3.4e+38             // float INF

//#define GI(ind) exp(li*pow((1-(inimg1d[ind]-min_int)/max_int),2))

#define GI(ind) givals[(int)((inimg1d[ind] - min_int)/max_int*255)]

static double givals[256] = {22026.5,   20368, 18840.3, 17432.5, 16134.8, 14938.4, 13834.9, 12816.8,
11877.4, 11010.2, 10209.4,  9469.8, 8786.47, 8154.96, 7571.17, 7031.33,
6531.99, 6069.98, 5642.39, 5246.52, 4879.94, 4540.36, 4225.71, 3934.08,
 3663.7, 3412.95, 3180.34,  2964.5, 2764.16, 2578.14, 2405.39,  2244.9,
2095.77, 1957.14, 1828.24, 1708.36, 1596.83, 1493.05, 1396.43, 1306.47,
1222.68, 1144.62, 1071.87, 1004.06, 940.819, 881.837, 826.806, 775.448,
727.504, 682.734, 640.916, 601.845, 565.329, 531.193, 499.271, 469.412,
441.474, 415.327, 390.848, 367.926, 346.454, 326.336, 307.481, 289.804,
273.227, 257.678, 243.089, 229.396, 216.541, 204.469, 193.129, 182.475,
172.461, 163.047, 154.195, 145.868, 138.033, 130.659, 123.717, 117.179,
111.022,  105.22, 99.7524, 94.5979, 89.7372, 85.1526,  80.827, 76.7447,
 72.891, 69.2522, 65.8152, 62.5681, 59.4994, 56.5987,  53.856, 51.2619,
48.8078, 46.4854, 44.2872, 42.2059, 40.2348, 38.3676, 36.5982, 34.9212,
33.3313, 31.8236, 30.3934, 29.0364, 27.7485,  26.526,  25.365, 24.2624,
23.2148, 22.2193,  21.273, 20.3733, 19.5176, 18.7037, 17.9292,  17.192,
16.4902,  15.822, 15.1855,  14.579, 14.0011, 13.4503, 12.9251, 12.4242,
11.9464, 11.4905, 11.0554, 10.6401, 10.2435, 9.86473, 9.50289, 9.15713,
8.82667, 8.51075, 8.20867, 7.91974, 7.64333, 7.37884, 7.12569, 6.88334,
6.65128, 6.42902,  6.2161, 6.01209, 5.81655, 5.62911, 5.44938, 5.27701,
5.11167, 4.95303, 4.80079, 4.65467, 4.51437, 4.37966, 4.25027, 4.12597,
4.00654, 3.89176, 3.78144, 3.67537, 3.57337, 3.47528, 3.38092, 3.29013,
3.20276, 3.11868, 3.03773,  2.9598, 2.88475, 2.81247, 2.74285, 2.67577,
2.61113, 2.54884, 2.48881, 2.43093, 2.37513, 2.32132, 2.26944, 2.21939,
2.17111, 2.12454, 2.07961, 2.03625, 1.99441, 1.95403, 1.91506, 1.87744,
1.84113, 1.80608, 1.77223, 1.73956, 1.70802, 1.67756, 1.64815, 1.61976,
1.59234, 1.56587, 1.54032, 1.51564, 1.49182, 1.46883, 1.44664, 1.42522,
1.40455,  1.3846, 1.36536,  1.3468,  1.3289, 1.31164, 1.29501, 1.27898,
1.26353, 1.24866, 1.23434, 1.22056,  1.2073, 1.19456, 1.18231, 1.17055,
1.15927, 1.14844, 1.13807, 1.12814, 1.11864, 1.10956, 1.10089, 1.09262,
1.08475, 1.07727, 1.07017, 1.06345, 1.05709, 1.05109, 1.04545, 1.04015,
1.03521,  1.0306, 1.02633, 1.02239, 1.01878,  1.0155, 1.01253, 1.00989,
1.00756, 1.00555, 1.00385, 1.00246, 1.00139, 1.00062, 1.00015,       1};


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
                assert(x >= 0 && x < sz0 && y >= 0 && y < sz1 && z >= 0 && z < sz2);
                tar_map[ind] = tar_markers[i];
        }

        for(long i = 0; i < sub_markers.size(); i++)
        {
                int x = sub_markers[i].x + 0.5;
                int y = sub_markers[i].y + 0.5;
                int z = sub_markers[i].z + 0.5;
                long ind = z*sz01 + y*sz0 + x;
                assert(x >= 0 && x < sz0 && y >= 0 && y < sz1 && z >= 0 && z < sz2);
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



