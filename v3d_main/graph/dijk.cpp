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




/* dijk.cpp

 the Dijkstra algorithm

 2009-05-12: by Hanchuan Peng

 */

#include "graph.h"
#include "graph_basic.h"

#include "dijk.h"

//global variables

const int ColorWhite = 0;
const int ColorGrey = 1;
const int ColorBlack = 2;
const int ColorRed = 3;

V3DLONG extractWhiteMin(BYTE * colorQ, vector <connectionVal> *adjMatrix, V3DLONG len);

V3DLONG extractWhiteMin(BYTE * colorQ, vector <connectionVal> *adjMatrix, V3DLONG len)
{
	double min=_very_large_double;
	V3DLONG idxmin=-1;
	V3DLONG b_min0 = 0;
	V3DLONG i,j,jtmp;
	float wei;
	for (i=0;i<len;i++)
	{
		if (colorQ[i]==ColorBlack)
		{
			for (jtmp=0;jtmp<adjMatrix[i].size();jtmp++)
			{
				j = adjMatrix[i].at(jtmp).cNode;
				wei = adjMatrix[i].at(jtmp).aVal;
				if (colorQ[j]==ColorWhite && wei>0)
				{
					if (b_min0==0)
					{
						b_min0 = 1;
						min = wei;
						idxmin = j;
					}
					else
					{ //if (b_min0==1)
						if (min>wei)
						{
							min = wei;
							idxmin = j;
						}
					}
				}
			}
		}
	}
	if (b_min0==0)
	{
		idxmin = -1;
	}
	return idxmin;
}

float DijkstraClass::getAdjMatrixValue(V3DLONG parentNode, V3DLONG childNode)
{
	if (!adjMatrix) return 0;
	if (parentNode>=nnode || parentNode<0 || childNode>=nnode || childNode<0) return 0;

	bool b_exist=false;
	for (V3DLONG j=0;j<adjMatrix[parentNode].size();j++)
	{
		if (adjMatrix[parentNode].at(j).cNode==childNode) {b_exist=true; return adjMatrix[parentNode].at(j).aVal;}
	}
	if (!b_exist) return 0;
}


void DijkstraClass::dosearch(V3DLONG r) //r -- root node
{
	if (nnode<=0 || !adjMatrix)
	{
		printf("The input data has not been set yet!\n");
		return;
	}

	if (r<0 || r>=nnode)
	{
		printf("The root node is invalid. Must between 0 and nnode-1! Do nothing.\n");
		return;
	}

	float curEdgeVal;
	V3DLONG i,j,jtmp;
	V3DLONG * localQueue_node = 0;
	V3DLONG time;
	V3DLONG nleftnode;

	localQueue_node = new V3DLONG [nnode];
	if (!localQueue_node)
	{
		printf("Fail to do: V3DLONG * localQueue_node = new V3DLONG [nnode];\n");
		goto Label_FreeMemory_Return;
	}

	// initialization

	for (i=0;i<nnode;i++)
	{
		localQueue_node[i] = i;
		nodeColor[i] = ColorWhite;
		nodeDistEst[i] = _very_large_double;//revise a larger num later
		nodeParent[i] = -1;
		nodeDetectTime[i] = -1;
		nodeFinishTime[i] = -1;
	}
	time = 0;
	nodeDistEst[r] = 0;
	nodeParent[r] = -1;

	// begin loop

	nleftnode = nnode;
	while (nleftnode>0)
	{
		i = extractWhiteMin(nodeColor,adjMatrix,nnode);
		if (i==-1)
		{ //for the first node
			i = r;
		}
		nodeDetectTime[i] = ++time;

		if (b_disp)
		{
			printf("time=%i curnode=%i \n",time,i+1);
		}

		for(jtmp=0;jtmp<adjMatrix[i].size();jtmp++)
		{
			j = adjMatrix[i].at(jtmp).cNode;
			curEdgeVal = adjMatrix[i].at(jtmp).aVal; //getAdjMatrixValue(i,j); //this will be faster

			if (curEdgeVal>0 &&
				nodeColor[j]==ColorWhite &&
				curEdgeVal<nodeDistEst[j])
			{
				nodeParent[j] = i+1; //add 1 for the matlab convention
				nodeDistEst[j] = curEdgeVal;
			}
		}

		nodeColor[i] = ColorBlack;
		nodeFinishTime[i] = ++time;
		nleftnode--;
	}

Label_FreeMemory_Return:

	if (localQueue_node) delete []localQueue_node;

	return;
}


int DijkstraClass::allocatememory(V3DLONG nodenum)
{
	if (nodenum>0)	nnode = nodenum;

	int b_memory = 1;
	if (nnode>0)
	{
		delocatememory();
		b_memory = b_memory && (new1dArrayMatlabProtocal(nodeColor,nnode));
		b_memory = b_memory && (new1dArrayMatlabProtocal(nodeDetectTime,nnode));
		b_memory = b_memory && (new1dArrayMatlabProtocal(nodeFinishTime,nnode));
		b_memory = b_memory && (new1dArrayMatlabProtocal(nodeDistEst,nnode));
		b_memory = b_memory && (new1dArrayMatlabProtocal(nodeParent,nnode));
		adjMatrix = new vector <connectionVal> [nnode];
		b_memory = b_memory && adjMatrix;
	}
	if (!b_memory)
	{
		delocatememory();
		return 0; //fail
	}
	else
		return 1; //success
}
void DijkstraClass::delocatememory()
{
	delete1dArrayMatlabProtocal(nodeColor);
	delete1dArrayMatlabProtocal(nodeDetectTime);
	delete1dArrayMatlabProtocal(nodeFinishTime);
	delete1dArrayMatlabProtocal(nodeDistEst);
	delete1dArrayMatlabProtocal(nodeParent);
	if (adjMatrix){delete []adjMatrix; adjMatrix=0;} //do I need to literally clear the contents in each of adjMatrix[i]? 080615
}

void DijkstraClass::printAdjMatrix()
{
	for (V3DLONG i=0;i<nnode;i++)
	{
		for (V3DLONG jtmp=0;jtmp<adjMatrix[i].size();jtmp++)
		{
			printf("row [%d] (%d -> %d) = %5.3f\n", i+1, adjMatrix[i].at(jtmp).pNode+1, adjMatrix[i].at(jtmp).cNode+1, adjMatrix[i].at(jtmp).aVal);
		}
	}
}

//main program

#ifdef _USE_MATLAB_MEX_

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	//check data

	if(nrhs < 1 || nrhs>2)
	{
		mexPrintf("Usage [node_attribute] = dijk(adjArray2d, rootnode).\n");
		mexPrintf("node_attr = [color,key,parent,dtime,ftime].\n");
		mexPrintf("If unspecified (between 1 to N), the root node is the first node.\n");
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

	DijkstraClass * p = new DijkstraClass;
	if (!p)
    {mexErrMsgTxt("Fail to allocate memory for MSTClass().");}
	p->nnode = chei;
	p->allocatememory(p->nnode);
	if (!(p->adjMatrix))
	{
		mexErrMsgTxt("Fail to assign value to the internal edge matrix.");
	}

	float minlevel,maxlevel;

	bool b_res;
	if (mxIsSparse(prhs[0]))
		b_res = copyMatlabSparseData(prhs[0], p->adjMatrix, p->nnode, minlevel, maxlevel);
	else
		b_res = copyMatlabFullData(prhs[0], p->adjMatrix, p->nnode, minlevel, maxlevel);
	if (!b_res)
	{
		mexErrMsgTxt("Fail to assign value to the internal edge matrix.");
	}

	//  double diffmaxmin;
	//  double minlevel,maxlevel;
	//  switch(inimgtype) {
	//  case mxINT8_CLASS:
	//    copyvecdata_T2D((BYTE *)inimg,totalpxlnum,p->adjMatrix1d,
	//		    diffmaxmin,minlevel,maxlevel);
	//    break;
	//  case mxUINT8_CLASS:
	//    copyvecdata_T2D((UBYTE *)inimg,totalpxlnum,p->adjMatrix1d,
	//		    diffmaxmin,minlevel,maxlevel);
	//    break;
	//  case mxDOUBLE_CLASS:
	//    copyvecdata_T2D((double *)inimg,totalpxlnum,p->adjMatrix1d,
	//		    diffmaxmin,minlevel,maxlevel);
	//    break;
	//  default:
	//    mexErrMsgTxt("Unsupported data type.");
	//    break;
	//  }

	//printf("min=%i max=%i\n",minlevel,maxlevel);

	//begin computation
	//p->printAdjMatrix();

	p->dosearch(rootnode); //set root as the first node

	//create the Matlab structure array

	plhs[0] = mxCreateDoubleMatrix(p->nnode,5,mxREAL);
	double * out_nodeColor = mxGetPr(plhs[0]);
	double * out_nodeDistEst = out_nodeColor + p->nnode;
	double * out_nodeParent = out_nodeDistEst + p->nnode;
	double * out_nodeDetectTime = out_nodeParent + p->nnode;
	double * out_nodeFinishTime = out_nodeDetectTime + p->nnode;
	for (V3DLONG i=0;i<p->nnode;i++)
	{
		out_nodeColor[i] = p->nodeColor[i];
		out_nodeDistEst[i] = p->nodeDistEst[i];
		out_nodeParent[i] = p->nodeParent[i];
		out_nodeDetectTime[i] = p->nodeDetectTime[i];
		out_nodeFinishTime[i] = p->nodeFinishTime[i];
	}

	//free memory and return

	if (p) {delete p;}
	return;
}

#endif
