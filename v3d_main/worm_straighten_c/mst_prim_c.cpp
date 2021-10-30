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




/* minimum spanning tree .cpp

   ver: 0.3

   This version is a revised version used as a standalone to be called by other C programs.
   
   The pure algorithms/codes are the same as the mst_prim.cpp file in the elementfunc/graph directory.

   by Hanchuan Peng, March 5, 2008.
   by Hanchuan Peng, March 17, 2008: remove "parents index +1" which was designed for Matlab compatability
   
   Note: I don't think the BFS application can find the longest MST diameter for the general case. Need to use Dijkstr algorithm in the future. Noted by Hanchuan, 080317.

  */

#include "graphsupport.h"

#include "bdb_minus.h"
#include "bfs.h"

//global variables

const int ColorWhite = 0;
const int ColorGrey = 1;
const int ColorBlack = 2;
const int ColorRed = 3;

V3DLONG extractWhiteMin(_ELEMENT_GRAPH_BYTE * colorQ, double ** wei, V3DLONG len);
class PrimMSTClass //the Prim's MST algorithm
{
public:
  V3DLONG nnode;

  double * adjMatrix1d,  ** adjMatrix2d;

  _ELEMENT_GRAPH_BYTE * nodeColor; //decide if a node has been visited or not
  double * nodeKey; 
  V3DLONG * nodeParent; 
  V3DLONG * nodeDetectTime;
  V3DLONG * nodeFinishTime;

  void dosearch(V3DLONG r);//r -- root node
  int allocatememory(V3DLONG nodenum);
  void delocatememory();

  int b_disp;

  PrimMSTClass() {
    nnode = 0;
    adjMatrix1d = 0;
    adjMatrix2d = 0;
    nodeColor = 0;
    nodeKey = 0;
    nodeParent = 0;
    nodeDetectTime = 0;
    nodeFinishTime = 0;
    b_disp = 0;
  }
  ~PrimMSTClass() {
    delocatememory();
    nnode = 0;
  }
};

V3DLONG extractWhiteMin(_ELEMENT_GRAPH_BYTE * colorQ, double ** wei, V3DLONG len)
{
  double min=100000000;
  V3DLONG idxmin=-1;
  V3DLONG b_min0 = 0;
  V3DLONG i,j;
  for (i=0;i<len;i++) {
    if (colorQ[i]==ColorBlack) {
      for (j=0;j<len;j++) {
        if (colorQ[j]==ColorWhite && wei[i][j]>0) {
          if (b_min0==0) {
            b_min0 = 1;
            min = wei[i][j];
            idxmin = j;
          }
          else { //if (b_min0==1)
            if (min>wei[i][j]) {
              min = wei[i][j];
              idxmin = j;
            }
          }
        }
      }
    }
  }
  if (b_min0==0) {
    idxmin = -1;
  }
  return idxmin;
}
/*
V3DLONG extractWhiteMin(V3DLONG * priorityQ, _ELEMENT_GRAPH_BYTE * colorQ, V3DLONG len)
{
  V3DLONG min=100000000,imin=0;
  V3DLONG b_min0 = 0;
  for (V3DLONG i=0;i<len;i++) {
    if (colorQ[i]==ColorWhite) {
      if (b_min0==0) {
        b_min0 = 1;
        min = priorityQ[i];
        imin = i;
      }
      else { //if (b_min0==1)
        if (min>priorityQ[i]) {
          min = priorityQ[i];
          imin = i;
        }
      }
    }
  }
  if (b_min0==1) {
    //printf("%d ",min);
    colorQ[imin] = ColorBlack; //so will not visit this node any more
    return min;
  }
  else {
    min = 100000000;
    return min;
  }
}
*/
void PrimMSTClass::dosearch(V3DLONG r) //r -- root node
{
  if (nnode<=0 || !adjMatrix1d || !adjMatrix2d) {
    printf("The input data has not been set yet!\n");
    return;
  }

  //make r a reasonable index
  
  r = (r<0)?0:r;
  r = (r>nnode)?nnode-1:r;

  V3DLONG i,j;
  V3DLONG * localQueue_node = 0;
  V3DLONG time;
  V3DLONG nleftnode;

  localQueue_node = new V3DLONG [nnode];
  if (!localQueue_node) {
    printf("Fail to do: V3DLONG * localQueue_node = new V3DLONG [nnode];\n");
    goto Label_FreeMemory_Return;
  }

  // initialization

  for (i=0;i<nnode;i++) {
    localQueue_node[i] = i;
    nodeColor[i] = ColorWhite;
    nodeKey[i] = 1000000;//revise a larger num later
    nodeParent[i] = -1;
    nodeDetectTime[i] = -1;
    nodeFinishTime[i] = -1;
  }
  time = 0;

  nodeKey[r] = 0;
  nodeParent[r] = -1;
  
  // begin BFS loop

  nleftnode = nnode;
  while (nleftnode>0) {
    i = extractWhiteMin(nodeColor,adjMatrix2d,nnode);
    if (i==-1) { //for the first node
      i = r;
    }
    nodeDetectTime[i] = ++time;

    if (b_disp) {
      printf("time=%i curnode=%i \n",time,i+1);
    }
    
    for (j=0;j<nnode;j++) {
      if (adjMatrix2d[i][j]>0 &&
          nodeColor[j]==ColorWhite && 
          adjMatrix2d[i][j]<nodeKey[j]) {
            nodeParent[j] = i; //add 1 for the matlab convention. Remove this on 080317 for C codes
            nodeKey[j] = adjMatrix2d[i][j];
          }
    }
    
    nodeColor[i] = ColorBlack;
    nodeFinishTime[i] = ++time;
    nleftnode--;
  }

 Label_FreeMemory_Return:
  
  if (localQueue_node) delete []localQueue_node;

  return;
}//%================ end of MST_dosearch()=================

int PrimMSTClass::allocatememory(V3DLONG nodenum) 
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
    b_memory = b_memory && (new1dArrayMatlabProtocal(nodeKey,nnode));
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
void PrimMSTClass::delocatememory() 
{
  delete1dArrayMatlabProtocal(nodeColor);
  delete1dArrayMatlabProtocal(nodeDetectTime);
  delete1dArrayMatlabProtocal(nodeFinishTime);
  delete1dArrayMatlabProtocal(nodeKey);
  delete1dArrayMatlabProtocal(nodeParent);
  delete2dArrayMatlabProtocal(adjMatrix2d,adjMatrix1d);
}

//main program

V3DLONG * mst_parents_list(Image2DSimple <MYFLOAT> * fullConnectMatrix) //return a vector of V3DLONG type, each is the parent node index
{
  //check data 
  
  if(!fullConnectMatrix || !fullConnectMatrix->valid()) 
  {
    printf("The input pointer is empty\n");
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
  
  V3DLONG rootnode=0; //default
  
  //===========================================================

  PrimMSTClass * pMST = new PrimMSTClass;
  if (!pMST)
  {
    printf("Fail to allocate memory for MSTClass().");
	return 0;
  }
  pMST->nnode = chei;
  pMST->allocatememory(pMST->nnode);

  double diffmaxmin;
  double minlevel,maxlevel;
  copyvecdata_T2D((MYFLOAT *)fullConnectMatrix->getData1dHandle(),fullConnectMatrix->getTotalElementNumber(),pMST->adjMatrix1d, diffmaxmin, minlevel, maxlevel); 

  //printf("min=%i max=%i\n",minlevel,maxlevel);

  //begin computation

  pMST->dosearch(rootnode); 

  //create the Matlab structure array

  V3DLONG * parentNode = new V3DLONG [chei];
  if (!parentNode)
  {
    printf("Cannot allocate memory\n");
  }
  
  for (V3DLONG i=0;i<pMST->nnode;i++) 
  {
    parentNode[i] = pMST->nodeParent[i];
  }

  //free memory and return

  if (pMST) {delete pMST;}
  return parentNode;
}

Image2DSimple<MYFLOAT> * mst_fullgraph(V3DLONG * parentsList, V3DLONG len)
{
  if (!parentsList || len<=0)
  {
    fprintf(stderr, "Invalid mst pointer in mst_fullgraph().\n");
    return 0;
  }

  Image2DSimple<MYFLOAT> *m = new Image2DSimple<MYFLOAT>(len, len);
  MYFLOAT **m_2d = m->getData2dHandle();
  
  for (V3DLONG i=0;i<len; i++)
  {
    V3DLONG j=parentsList[i];
    if (j>=0 && j<len)
	  m_2d[i][j] = m_2d[j][i] = 1;
	else
	{
	  printf("current [i,j]=[%ld,%ld]\n", i,j); 
	}
  }
  
  return m;
}

V3DLONG * img_mst_diameter(Image2DSimple<MYFLOAT> * m, V3DLONG & length_diameter)
{
  if (!m)
  {
    fprintf(stderr, "Invalid mst pointer in img_mst_diameter().\n");
    return 0;
  }
	
  int N = m->sz0();
  if (N<=0 || N!=m->sz1())
  {
    fprintf(stderr, "Invalid size of the mst in img_mst_diameter().\n");
    return 0;
  }

  
  // As this MST is produced for an image object, the weight must be 1 for
  // adjacent node. Thus I don't judge the tree weight anymore. In another
  // word, I am not finding the path with the largest sum of weight, instead
  // just the path with the most nodes.
  //
  // Also note that I am not doing m=m+m' here as I assume the graph is undirectional already in mst_fullgraph()
  //
  V3DLONG i; V3DLONG maxt, nodeStart, nodeEnd;
  BFSClass * bfs_res=0;

  //the first bfs
  bfs_res = bfs_1root(m, 0);
  V3DLONG * bfs_nodeDetectTime = bfs_res->nodeDetectTime;
  nodeStart=0; maxt=bfs_nodeDetectTime[nodeStart];
  for (i=1;i<N; i++)
  {
    if (bfs_nodeDetectTime[i]>maxt)
	{
	  maxt = bfs_nodeDetectTime[i];
	  nodeStart=i;
	}
  }
  if (bfs_res) {delete bfs_res; bfs_res=0;} //delete the first bfs result
  
  //the second bfs
  bfs_res = bfs_1root(m, nodeStart);
  bfs_nodeDetectTime = bfs_res->nodeDetectTime;
  nodeEnd=0; maxt=bfs_nodeDetectTime[nodeEnd];
  for (i=1;i<N; i++)
  {
    if (bfs_nodeDetectTime[i]>maxt)
	{
	  maxt = bfs_nodeDetectTime[i];
	  nodeEnd=i;
	}
  }
  
  //now find the path
  V3DLONG *myord = new V3DLONG [maxt];
  V3DLONG *bfs_nodeParent = bfs_res->nodeParent;
  myord[0] = nodeEnd;
  for (i=1;i<maxt;i++)
  {
    myord[i] = bfs_nodeParent[myord[i-1]];
  }
  if (myord[maxt-1]!=nodeStart)
  {
    fprintf(stderr, "The path has error in img_mst_diameter()!!!; \n");
  }
  length_diameter = maxt;
  
  //now delete the result of the second bfs and return res
  if (bfs_res) {delete bfs_res; bfs_res=0;}

  return myord;
}

