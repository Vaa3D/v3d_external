#ifndef _IMG_SEGMENT_H_
#define _IMG_SEGMENT_H_

#include "basic_memory.h"
#include <limits>

// refer to http://en.wikipedia.org/wiki/Disjoint-set_data_structure
class DisjointSetSimple
{
public:
	DisjointSetSimple* parent;
	DisjointSetSimple(){parent = this;}
};

class DisjointSetWithRank   // public DisjointSetSimple
{
public:
	DisjointSetWithRank * parent;
	int rank;
	DisjointSetWithRank(){parent = this;rank = 0;}
};

class DisjointSetWithRankAndLink
{
public:
	DisjointSetWithRankAndLink * parent;
	int rank;
	DisjointSetWithRankAndLink * next_node;
	int sz;
	DisjointSetWithRankAndLink(){parent = this;rank = 0; next_node = this; sz = 1;}
};

V3DLONG                      find_set(V3DLONG x, V3DLONG * &djs_parent);
DisjointSetSimple*           find_set(DisjointSetSimple * x);
DisjointSetWithRank *        find_set(DisjointSetWithRank * x);
DisjointSetWithRankAndLink * find_set(DisjointSetWithRankAndLink * x);

void union_set(V3DLONG x, V3DLONG y, V3DLONG * & djs_parent);
void union_set(DisjointSetSimple* x, DisjointSetSimple * y);
void union_set(DisjointSetWithRank* x, DisjointSetWithRank * y);
void union_set(DisjointSetWithRankAndLink* x, DisjointSetWithRankAndLink *y);

template<class T>            bool construct_disjoint_set(V3DLONG * & djs, T* &inimg1d, V3DLONG * &sz, double thresh);
template<class T1, class T2> bool construct_disjoint_set(T2 * & djs, T1* &inimg1d, V3DLONG * &sz, double thresh);

template<class T>            bool maximum_connected_component(T* &outimg1d, T* &inimg1d, V3DLONG * &sz, double thresh);

#endif
