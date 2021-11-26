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




/* bfs_1root.cpp
   search the graph from a single root node;
   if some nodes cannot be arrived, then set the 
   detection and finish time of these node as infinity (i.e. -1)

   ver: 0.2

   by PHC, July, 2002

Last update: 080317. Remove "+1 for parent index" which was once designed for Matlab compatability 
  */

#include "bfs.h"

//========================

//global variables

const int ColorWhite = 0;
const int ColorGrey = 1;
const int ColorBlack = 2;
const int ColorRed = 3;

#include "bdb_minus.h"

void BFSClass::dosearch()
{
  if (nnode<=0 || !adjMatrix1d || !adjMatrix2d) {
    printf("The input data has not been set yet!\n");
    return;
  }
  
  V3DLONG i,j,iu,iv;
  V3DLONG * localQueue_node = 0;
  V3DLONG queueHead = 0;
  V3DLONG queueTail = 0;
  const V3DLONG queueProtectLen = 128;
  V3DLONG time;
  V3DLONG curDCGLabel = 1;

  localQueue_node = new V3DLONG [nnode+2*queueProtectLen];
  if (!localQueue_node) {
    printf("Fail to do: V3DLONG * localQueue_node = new V3DLONG [nnode];\n");
    goto Label_FreeMemory_Return;
  }

  // initialization

  for (i=0;i<nnode;i++) {
    nodeColor[i] = ColorWhite;
    nodeLabel[i] = -1;
    nodeParent[i] = -1;
    nodeDetectTime[i] = -1;
    nodeFinishTime[i] = -1;
  }
  time = 0;
  
  // begin BFS loop

  curDCGLabel = 1; //if the input graph is a symmetric undirected graph
                   //then this will return how many DCG in G
  for (i=rootnode;i<=rootnode;i++) {
    
    if (nodeColor[i]==ColorWhite) {

      queueHead = queueProtectLen; //protect both head and tail of queue
      queueTail = queueHead-1;

      nodeColor[i] = ColorGrey;
      nodeLabel[i] = curDCGLabel;
      nodeParent[i] = -1;
      nodeDetectTime[i] = ++time;

      //Enqueue
      queueTail++;
      localQueue_node[queueTail] = i; 
      if (queueTail>=nnode+queueProtectLen) {
	printf("The local queue is overflow!!!\n");
	goto Label_FreeMemory_Return;
      }
      if (b_disp) {
	printf("push %li\t queueHead = %li \tqueueTail = %li ===\n",i, queueHead, queueTail);
      }

      while (queueHead<=queueTail) {
	
	iu = localQueue_node[queueHead];

	for (j=0;j<nnode;j++) {

	  if (nodeColor[j]==ColorWhite && adjMatrix2d[j][iu]!=0) {
	    nodeColor[j] = ColorGrey;
	    nodeLabel[j] = curDCGLabel;
	    nodeParent[j] = iu; //add 1 for the matlab convention. 080317: remove this for C codes
	    nodeDetectTime[j] = nodeDetectTime[iu]+1;

	    //enqueue
	    queueTail++;
	    localQueue_node[queueTail] = j; 

	    if (queueTail>=nnode+queueProtectLen) {
	      printf("The local queue is overflow!!!\n");
	      goto Label_FreeMemory_Return;
	    }
	    if (b_disp) {
	      printf("push %li\t queueHead = %li \tqueueTail = %li\n",j,queueHead, queueTail);
	    }
	  }
	}

	queueHead++; //dequeue
	if (b_disp) {
	  printf("pop %li\t queueHead = %li \tqueueTail = %li\n",i, queueHead, queueTail);
	}
      
	nodeColor[iu] = ColorBlack;
	nodeFinishTime[iu] = ++time;
      }
      curDCGLabel++;
    }
  }

 Label_FreeMemory_Return:
  
  if (localQueue_node) delete []localQueue_node;

  return;
}//%================ end of DFS_dosearch()=================

int BFSClass::allocatememory(V3DLONG nodenum) 
{
  if (nodenum>0) {
    nnode = nodenum;
  }

  int b_memory = 1;
  if (nnode>0) {
    delocatememory();
    b_memory = b_memory && (new1dArrayMatlabProtocal(nodeColor,nnode));
    b_memory = b_memory && (new1dArrayMatlabProtocal(nodeDetectTime,nnode));
    b_memory = b_memory && (new1dArrayMatlabProtocal(nodeFinishTime,nnode));
    b_memory = b_memory && (new1dArrayMatlabProtocal(nodeLabel,nnode));
    b_memory = b_memory && (new2dArrayMatlabProtocal(adjMatrix2d,adjMatrix1d,nnode,nnode));
    b_memory = b_memory && (new1dArrayMatlabProtocal(nodeParent,nnode));
  }
  if (!b_memory) {
    delocatememory();
    return 0; //fail
  }
  else
    return 1; //success
}
void BFSClass::delocatememory() 
{
  delete1dArrayMatlabProtocal(nodeColor);
  delete1dArrayMatlabProtocal(nodeDetectTime);
  delete1dArrayMatlabProtocal(nodeFinishTime);
  delete1dArrayMatlabProtocal(nodeLabel);
  delete1dArrayMatlabProtocal(nodeParent);
  delete2dArrayMatlabProtocal(adjMatrix2d,adjMatrix1d);
}

//main program

BFSClass * bfs_1root(Image2DSimple <MYFLOAT> * fullConnectMatrix, V3DLONG rootnode)
{
  //check data 
  
  if(!fullConnectMatrix || !fullConnectMatrix->valid()) 
  {
    printf("Invalid graph pointers.\n");
	return 0;
  }

  //copy data

  V3DLONG cwid = (V3DLONG)fullConnectMatrix->sz0();
  V3DLONG chei = (V3DLONG)fullConnectMatrix->sz1();
  if(cwid!=chei) 
  {
    printf("the connectivity matrix has to be square!\n");
	return 0;
  }
  
  if (rootnode<0 || rootnode>=cwid)
  {
    printf("Invalid rootnode in bfs_1root.\n");
	return 0;
  }
  
  BFSClass * pBFS = new BFSClass;
  if (!pBFS)
  {
    printf("Fail to allocate memory for BFSClass().");
	return 0;
  }
  pBFS->nnode = chei;
  pBFS->allocatememory(pBFS->nnode);

  int nstate;
  _ELEMENT_GRAPH_UBYTE minlevel,maxlevel;
  copyvecdata_T2UB((MYFLOAT *)fullConnectMatrix->getData1dHandle(), fullConnectMatrix->getTotalElementNumber(),pBFS->adjMatrix1d,nstate,minlevel,maxlevel); 

  //printf("min=%i max=%i\n",minlevel,maxlevel);

  pBFS->setrootnode(rootnode);
  
  //begin computation
  pBFS->dosearch();

  //printf("then --> phcDebugRgnNum=%i phcDebugPosNum=%i\n",phcDebugRgnNum,phcDebugPosNum);
  
  //if (pBFS) {delete pBFS;}
  return pBFS;
}

