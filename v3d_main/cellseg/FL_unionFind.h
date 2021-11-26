/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




// define union find class
// F. Long
// 20080508

#ifndef __UNION_FIND__
#define __UNION_FIND__

#define LENGTH 100
#define EXTENDLENGTHFACTOR 2 

#include "FL_defType.h"

class UnionFind 
{
private:

    V3DLONG *its_idd;
    V3DLONG *its_sz;
    V3DLONG its_num_sets;
    V3DLONG its_allocated_length;
    V3DLONG its_num_nodes;	
    V3DLONG its_finalized;
	
public:

	// constructor
	UnionFind(V3DLONG len)
	{
		if (len <= 0)
		{
			len = LENGTH;
		}

		its_idd = new V3DLONG [len]; 
		its_sz = new V3DLONG [len];
		its_allocated_length = len;
		its_num_nodes = 0;
		its_num_sets = 0;
		its_finalized = false;
	}
		
	// new a node in the tree
	void unionFindNewNode()
	{
		if (its_num_nodes >= its_allocated_length)
		{
			V3DLONG allocated_length_new = its_allocated_length * EXTENDLENGTHFACTOR;
						
			V3DLONG *idd1 = new V3DLONG [allocated_length_new]; 
			V3DLONG *sz1 = new V3DLONG [allocated_length_new];
				
			V3DLONG i;
			
			for (i=0;i< its_allocated_length; i++)
			{
				idd1[i] = its_idd[i];
				sz1[i] = its_sz[i];
			}			
			
			V3DLONG *iddtmp, *sztmp;
			iddtmp = its_idd; 
			sztmp = its_sz;

			its_idd = idd1; // now both its_idd and idd1 point to the same address
			its_sz = sz1; // now both its_sz and sz1 point to the same address
			
			if (iddtmp) {delete []iddtmp; iddtmp = 0;}
			if (sztmp) {delete [] sztmp; sztmp = 0;}
						
			its_allocated_length = allocated_length_new;
			
		}

		its_num_nodes++;
		its_idd[its_num_nodes - 1] = its_num_nodes - 1;
		its_sz[its_num_nodes - 1] = 1;
	}
	
	V3DLONG unionFindSearch(V3DLONG p)
	{
		V3DLONG i;
		V3DLONG t;
		V3DLONG *idd1 = its_idd;
		
		for (i = p; i != idd1[i]; i = idd1[i])
		{
			t = i;
			i = idd1[idd1[t]];
			idd1[t] = i;
		}

		return i;
	}
	
	void unionFindUnion(V3DLONG p, V3DLONG q)
	{
		if (its_sz[p] < its_sz[q])
		{
			its_idd[p] = q;
			its_sz[q] += its_sz[p];
		}
		else
		{
			its_idd[q] = p;
			its_sz[p] += its_sz[q];
		}
	}

	
	void unionFindMergePair(V3DLONG p, V3DLONG q)
	{
		V3DLONG i;
		V3DLONG j;

		i = unionFindSearch(p);
		j = unionFindSearch(q);
		if (i != j)
		{
			unionFindUnion(i, j);
		}
	}
	
	// renumber 
	V3DLONG unionFindEnumerate(V3DLONG first)
	{
		V3DLONG k;
		V3DLONG counter = first;

		its_finalized = true;

		for (k = 0; k < its_num_nodes; k++)
		{
			if (its_idd[k] == k)
			{
				its_sz[k] = counter++;
			}
		}

		its_num_sets = counter - first;

		return its_num_sets;
	}
	
	V3DLONG unionFindGetSet(V3DLONG p)
	{
		V3DLONG k;
//		V3DLONG *idd = idd;

		k = unionFindSearch(p);
		return its_sz[k];
	}
	
	~UnionFind()
	{
		if (its_idd) { delete [] its_idd; its_idd = 0;}
		if (its_sz)  { delete [] its_sz; its_sz = 0;}
	}	
};

#endif 

