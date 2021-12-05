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




//bfs.h
//by Hanchuan Peng
//revised from previous bfs code in the elementfunc/graph path,
// last update: 080310

#ifndef __BFS_PHC__
#define __BFS_PHC__

#include "graph.h"
#include "graphsupport.h"

class BFSClass //BreadthFirstSearchClass
{
public:
  V3DLONG nnode;

  _ELEMENT_GRAPH_UBYTE * adjMatrix1d,  ** adjMatrix2d;

  _ELEMENT_GRAPH_BYTE * nodeColor; //decide if a node has been visited or not
  V3DLONG * nodeLabel; //this will decide in which subgraph a node is in
  V3DLONG * nodeParent; //for the DFS tree
  V3DLONG * nodeDetectTime;
  V3DLONG * nodeFinishTime;

  void dosearch();
  int allocatememory(V3DLONG nodenum);
  void delocatememory();

  V3DLONG rootnode;
  V3DLONG setrootnode(V3DLONG i) {
    rootnode=(i<0)?0:i; 
    rootnode=(rootnode>=nnode)?nnode:rootnode;
    return rootnode;
  }

  int b_disp;

  BFSClass() {
    nnode = 0;
    adjMatrix1d = 0;
    adjMatrix2d = 0;
    nodeColor = 0;
    nodeLabel = 0;
    nodeParent = 0;
    nodeDetectTime = 0;
    nodeFinishTime = 0;
    b_disp = 0;
    rootnode = 0;
  }
  ~BFSClass() {
    delocatememory();
    nnode = 0;
    rootnode = 0;
  }
};



#endif

