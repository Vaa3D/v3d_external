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

  */

#include "graphsupport.h"

//global variables

const int ColorWhite = 0;
const int ColorGrey = 1;
const int ColorBlack = 2;
const int ColorRed = 3;

V3DLONG extractWhiteMin(BYTE * colorQ, double ** wei, V3DLONG len);
class PrimMSTClass //the Prim's MST algorithm
{
public:
  V3DLONG nnode;

  double * adjMatrix1d,  ** adjMatrix2d;

  BYTE * nodeColor; //decide if a node has been visited or not
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

V3DLONG extractWhiteMin(BYTE * colorQ, double ** wei, V3DLONG len)
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
    nodeKey[i] = 10000;//revise a larger num later
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
            nodeParent[j] = i+1; //add 1 for the matlab convention
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

template <class T> bool mst(T **G, V3DLONG sz0, V3DLONG sz1)
{
  //check data 
  
  if(nrhs < 1 || nrhs>2) {
    printf("Usage [node_attribute] = mst_prim(adjArray2d, rootnode).\n");
    printf("node_attr = [color,key,parent,dtime,ftime].\n");
    printf("If unspecified (between 1 to N), the root node is the first node.\n");
    mexErrMsgTxt("No input para specified. Exit.");
  }
  if(nlhs > 1)
    mexErrMsgTxt("Too many output arguments <labelled_rgn>.");

  //copy data

  void *inimg = (void *)mxGetData(prhs[0]);
  const V3DLONG totalpxlnum = mxGetNumberOfElements(prhs[0]);
  mxClassID inimgtype = mxGetClassID(prhs[0]);
  V3DLONG cwid = (V3DLONG)mxGetN(prhs[0]);
  V3DLONG chei = (V3DLONG)mxGetM(prhs[0]);
  if(cwid!=chei) {
    mexErrMsgTxt("the connectivity matrix has to be square!");
  }
  
  //====================================by PHC, 041119=========
  V3DLONG rootnode=0; //default
  if (nrhs==2)
  {
    rootnode = (V3DLONG)*mxGetPr(prhs[1]) - 1;
    rootnode = (rootnode<0)?0:rootnode;
    rootnode = (rootnode>cwid-1)?(cwid-1):rootnode;
    printf("The root node is set as %dth node.\n", rootnode+1);
  }
  //===========================================================

  PrimMSTClass * pMST = new PrimMSTClass;
  if (!pMST)
    {mexErrMsgTxt("Fail to allocate memory for MSTClass().");}
  pMST->nnode = chei;
  pMST->allocatememory(pMST->nnode);

  double diffmaxmin;
  double minlevel,maxlevel;
  switch(inimgtype) {
  case mxINT8_CLASS: 
    copyvecdata_T2D((BYTE *)inimg,totalpxlnum,pMST->adjMatrix1d,
		    diffmaxmin,minlevel,maxlevel); 
    break;
  case mxUINT8_CLASS: 
    copyvecdata_T2D((UBYTE *)inimg,totalpxlnum,pMST->adjMatrix1d,
		    diffmaxmin,minlevel,maxlevel); 
    break;
  case mxDOUBLE_CLASS: 
    copyvecdata_T2D((double *)inimg,totalpxlnum,pMST->adjMatrix1d,
		    diffmaxmin,minlevel,maxlevel); 
    break;
  default:
    mexErrMsgTxt("Unsupported data type.");
    break;
  } 

  //printf("min=%i max=%i\n",minlevel,maxlevel);

  //begin computation
  pMST->dosearch(rootnode); //set root as the first node

  //create the Matlab structure array

  plhs[0] = mxCreateDoubleMatrix(pMST->nnode,5,mxREAL);
  double * out_nodeColor = mxGetPr(plhs[0]);
  double * out_nodeKey = out_nodeColor + pMST->nnode;
  double * out_nodeParent = out_nodeKey + pMST->nnode;
  double * out_nodeDetectTime = out_nodeParent + pMST->nnode;
  double * out_nodeFinishTime = out_nodeDetectTime + pMST->nnode;
  for (V3DLONG i=0;i<pMST->nnode;i++) {
    out_nodeColor[i] = pMST->nodeColor[i];
    out_nodeKey[i] = pMST->nodeKey[i];
    out_nodeParent[i] = pMST->nodeParent[i];
    out_nodeDetectTime[i] = pMST->nodeDetectTime[i];
    out_nodeFinishTime[i] = pMST->nodeFinishTime[i];
  }

  //free memory and return

  if (pMST) {delete pMST;}
  return;
}

