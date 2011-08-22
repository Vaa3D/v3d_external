#ifndef _IMG_SEGMENT_SRC_
#define _IMG_SEGMENT_SRC_

#include <iostream>
#include "img_segment.h"
using namespace std;
V3DLONG find_set(V3DLONG x, V3DLONG * &djs_parent)
{
	if(djs_parent[x] == x) return x;
	else return find_set(djs_parent[x], djs_parent);
}

DisjointSetSimple* find_set(DisjointSetSimple * x)
{
	if(x->parent == x) return x;
	else return find_set(x->parent);
}

DisjointSetWithRank * find_set(DisjointSetWithRank * x)
{
	if(x->parent == x) return x;
	else 
	{
		x->parent = find_set(x->parent);
		return x->parent;
	}
}

DisjointSetWithRankAndLink * find_set(DisjointSetWithRankAndLink * x)
{
	if(x->parent == x) return x;
	else 
	{
		x->parent = find_set(x->parent);
		return x->parent;
	}
}

void union_set(V3DLONG x, V3DLONG y, V3DLONG * & djs_parent)
{
	V3DLONG xroot = find_set(x, djs_parent);
	V3DLONG yroot = find_set(y, djs_parent);
	djs_parent[xroot] = yroot;
}

void union_set(DisjointSetSimple* x, DisjointSetSimple * y)
{
	DisjointSetSimple* xroot = find_set(x);
	DisjointSetSimple* yroot = find_set(y);
	xroot->parent = yroot;
}

void union_set(DisjointSetWithRank* x, DisjointSetWithRank * y)
{
	DisjointSetWithRank* xroot = find_set(x);
	DisjointSetWithRank* yroot = find_set(y);
	if(xroot == yroot) return;
	if(xroot->rank < yroot->rank) xroot->parent = yroot;
	else if(xroot->rank > yroot->rank)
		yroot->parent = xroot;
	else {
		yroot->parent = xroot;
		xroot->rank++;
	}
}

void union_set(DisjointSetWithRankAndLink* x, DisjointSetWithRankAndLink * y)
{
	DisjointSetWithRankAndLink* xroot = find_set(x);
	DisjointSetWithRankAndLink* yroot = find_set(y);
	if(xroot == yroot) return;
	else 
	{
		DisjointSetWithRankAndLink * temp = xroot->next_node;
		xroot->next_node = yroot->next_node;
		yroot->next_node = temp;
	}
	if(xroot->rank < yroot->rank) 
	{
		xroot->parent = yroot;
		yroot->sz += xroot->sz;
	}
	else if(xroot->rank > yroot->rank)
	{
		yroot->parent = xroot;
		xroot->sz += yroot->sz;
	}
	else {
		yroot->parent = xroot;
		xroot->sz += yroot->sz;
		xroot->rank++;
	}
}

#define CONNECTION_TYPE 1    // 6 neighbors
template<class T> bool construct_disjoint_set(V3DLONG * & djs, T* &inimg1d, V3DLONG * &sz, double thresh)
{
	if(inimg1d == 0 || sz == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	V3DLONG tol_sz = sz[0] * sz[1] * sz[2];
	V3DLONG xy_sz = sz[0] * sz[1];
	if(djs == 0) djs = new V3DLONG[tol_sz];
	for(V3DLONG i = 0; i < tol_sz; i++) djs[i] = i;
	for(V3DLONG k = 0; k < sz[2]; k++)
	{
		for(V3DLONG j = 0; j < sz[1]; j++)
		{
			for(V3DLONG i = 0; i < sz[0]; i++)
			{
				V3DLONG x = k * xy_sz + j * sz[0] + i;
				if(inimg1d[x] < thresh) continue;
				for(V3DLONG kk = k-1; kk < k+1; kk++)
				{
					if(kk < 0 || kk >= sz[2]) continue;
					int dk = (kk - k) * (kk - k);
					for(V3DLONG jj = j-1; jj < j+1; jj++)
					{
						if(jj < 0 || jj >= sz[1]) continue;
						int dj = (jj - j) * (jj - j);
						for(V3DLONG ii = i-1; ii <= i+1;ii++)
						{
							if(ii < 0 || ii >= sz[0]) continue;
							int di = (ii - i) * (ii - i);
							if(di + dj + dk > 1) continue;
							V3DLONG y = kk * xy_sz + jj * sz[0] + ii;
							if(inimg1d[y] < thresh) continue;
							union_set(x,y,djs);
						}
					}
				}
			}
		}
	}
	return true;
}

template<class T1, class T2> bool construct_disjoint_set(T2 * & djs, T1* &inimg1d, V3DLONG * &sz, double thresh)
{
	if(inimg1d == 0 || sz == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	V3DLONG tol_sz = sz[0] * sz[1] * sz[2];
	V3DLONG xy_sz = sz[0] * sz[1];
	if(djs == 0) djs = new T2[tol_sz];
	for(V3DLONG i = 0; i < tol_sz; i++) djs[i].parent = djs + i;
	for(V3DLONG k = 0; k < sz[2]; k++)
	{
		for(V3DLONG j = 0; j < sz[1]; j++)
		{
			for(V3DLONG i = 0; i < sz[0]; i++)
			{
				V3DLONG x = k * xy_sz + j * sz[0] + i;
				if(inimg1d[x] < thresh) continue;
				for(V3DLONG kk = k-1; kk < k+1; kk++)
				{
					if(kk < 0 || kk >= sz[2]) continue;
					int dk = (kk - k) * (kk - k);
					for(V3DLONG jj = j-1; jj < j+1; jj++)
					{
						if(jj < 0 || jj >= sz[1]) continue;
						int dj = (jj - j) * (jj - j);
						for(V3DLONG ii = i-1; ii <= i+1;ii++)
						{
							if(ii < 0 || ii >= sz[0]) continue;
							int di = (ii - i) * (ii - i);
							if(di + dj + dk > 1) continue;
							V3DLONG y = kk * xy_sz + jj * sz[0] + ii;
							if(inimg1d[y] < thresh) continue;
							union_set(djs + x, djs + y);
						}
					}
				}
			}
		}
	}
	return true;
}
// find maximum_connected_component 
template<class T> bool maximum_connected_component(T* &outimg1d, T* &inimg1d, V3DLONG * &sz, double thresh)
{
	if(inimg1d == 0 || sz == 0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;
	V3DLONG tol_sz = sz[0] * sz[1] * sz[2];
	if(outimg1d == 0) outimg1d = new T[tol_sz];
	for(V3DLONG i = 0; i < tol_sz; i++) outimg1d[i] = 0;

	DisjointSetWithRankAndLink * djs = 0; if(!construct_disjoint_set(djs, inimg1d, sz, thresh)) return false;
	
	int max_sz = 0; 
	V3DLONG max_x = 0;
	for(V3DLONG x = 0; x < tol_sz; x++)
	{
		if(djs[x].sz > max_sz)
		{
			max_x = x;
			max_sz = djs[x].sz;
		}
	}
	cout<<"max_sz : "<<max_sz<<endl;
	DisjointSetWithRankAndLink * root = djs + max_x;
	T max_value = (T)((1<< (sizeof(T) * 8)) - 1);
	outimg1d[max_x] = max_value;
	DisjointSetWithRankAndLink * p = root->next_node;
	
	while(p != root)
	{
		int pos = (int)(p - djs) ;
		outimg1d[pos] = max_value;
		p = p->next_node;
	}
	if(djs){delete [] djs; djs = 0;}
	return true;
}

#endif
