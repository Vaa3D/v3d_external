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




//graph_basic.h
// copied and revised from graph_matlab_basic.h
//by Hanchuan Peng
//2009-05-12
//some basic functions that are used for the graph algorithms and interfacing with Matlab

#ifndef __GRAPH_BASIC_H__
#define __GRAPH_BASIC_H__

#ifdef _USE_MATLAB_MEX_
#include "mex.h"
#include "matrix.h"
#endif

#include <vector>
using namespace std;

#if defined(NAN_EQUALS_ZERO)
#define IsNonZero(d) ((d)!=0.0 || mxIsNaN(d))
#else
#define IsNonZero(d) ((d)!=0.0)
#endif

#include "../basic_c_fun/v3d_basicdatatype.h"

struct connectionVal //080613
{
	V3DLONG pNode; //parent node index
	V3DLONG cNode; //children node index
	float aVal; //adjacent matrix value (weight on an edge) //note that I changed the edge type from unsigned char to float
	connectionVal() {pNode=-1; cNode=-1; aVal=0;}
};

#ifdef _USE_MATLAB_MEX_
bool copyMatlabSparseData(const mxArray * srcdata, 
						  vector <connectionVal> * adjMatrix, 
						  V3DLONG nnodes,
						  float &minn, 
						  float &maxx);
bool copyMatlabFullData(const mxArray * srcdata, 
						vector <connectionVal> * adjMatrix, 
						V3DLONG nnodes,
						float &minn, 
						float &maxx);
template <class T> bool copyMatlabFullData_real(T * srcdata, 
												V3DLONG m,
												V3DLONG n, 
												vector <connectionVal> * adjMatrix, 
												V3DLONG nnodes,
												float &minn, 
												float &maxx);
#endif

template <class T> int new1dArrayMatlabProtocal(T * & img1d,V3DLONG nodenum);
template <class T> void delete1dArrayMatlabProtocal(T * & img1d);

template <class T> int new2dArrayMatlabProtocal(T ** & img2d,T * & img1d,V3DLONG imghei,V3DLONG imgwid);
template <class T> void delete2dArrayMatlabProtocal(T ** & img2d,T * & img1d);

//the definitions of several functions
#ifdef _USE_MATLAB_MEX_
//===========================================
bool copyMatlabSparseData(const mxArray * srcdata, 
									vector <connectionVal> * adjMatrix, 
									V3DLONG nnodes,
									float &minn, 
									float &maxx)
{
	if (!adjMatrix || nnodes<=0) {mexPrintf("the adjMatrix pointer or nnodes of copyMatlabSparseData() is invalid. Do nothing.\n");return false;}
	if (!srcdata) {mexPrintf("the input data pointer of copyMatlabSparseData() is NULL. Do nothing.\n");return false;}
	if (!mxIsSparse(srcdata)) {mexPrintf("the input data of copyMatlabSparseData() is not a sparse matrix. Do nothing. \n");return false;}
	if (mxIsComplex(srcdata)) {mexPrintf("the input data of copyMatlabSparseData() is complex which not supported. Do nothing.\n");return false;}
	if (!mxIsDouble(srcdata)) {mexPrintf("the input data of copyMatlabSparseData() is not DOUBLE type matrix. Do nothing. \n");return false;}

	mwSize m = mxGetM(srcdata);
	mwSize n = mxGetN(srcdata);
	if (m<=0 || n<=0) {mexPrintf("the input data of copyMatlabSparseData() is empty. Do nothing.\n");return false;}

	if (nnodes != ((m>n)?m:n)) {mexPrintf("the nnodes of copyMatlabSparseData() is invalid. Do nothing.\n");return false;}

	//now get the data pointers of the Matlab sparse matrix
	double *pr, *pi;
	mwIndex *ir, *jc;
	mwIndex starting_row_index, stopping_row_index, current_row_index;
	mwSize col, total=0;

	pr = mxGetPr(srcdata);
	pi = mxGetPi(srcdata);
	ir = mxGetIr(srcdata);
	jc = mxGetJc(srcdata);
	
	connectionVal tmpVal;
	for (col=0; col<n; col++)  
	{ 
		starting_row_index = jc[col]; 
		stopping_row_index = jc[col+1]; 
		if (starting_row_index == stopping_row_index)
			continue;
		else 
		{
			for (current_row_index = starting_row_index; current_row_index < stopping_row_index; current_row_index++)  
			{
				register V3DLONG curnode = ir[current_row_index];
				tmpVal.pNode = curnode;
				tmpVal.cNode = col;
				tmpVal.aVal = pr[total++];
				adjMatrix[curnode].push_back(tmpVal);
				
				if (total==1)
				{
					minn=maxx=pr[total];
				}
				else
				{
					if (minn<pr[total]) minn=pr[total];
					if (maxx>pr[total]) maxx=pr[total];
				}
			}
		}
	}
	
	return true;
}		


//copyMatlabFullData is similar to the original copyvecdata() function, but using different destination data structure
bool copyMatlabFullData(const mxArray * srcdata, 
									vector <connectionVal> * adjMatrix, 
									V3DLONG nnodes,
									float &minn, 
									float &maxx)
{
	if (!adjMatrix || nnodes<=0) {mexPrintf("the adjMatrix pointer or nnodes of copyMatlabFullData() is invalid. Do nothing.\n");return false;}
	if (!srcdata) {mexPrintf("the input data pointer of copyMatlabFullData() is NULL. Do nothing.\n");return false;}
	if (mxIsSparse(srcdata)) {mexPrintf("the input data of copyMatlabFullData() is not a full matrix. Do nothing. \n");return false;}
	if (mxIsComplex(srcdata)) {mexPrintf("the input data of copyMatlabFullData() is complex which not supported. Do nothing.\n");return false;}

	mwSize m = mxGetM(srcdata);
	mwSize n = mxGetN(srcdata);
	if (m<=0 || n<=0) {mexPrintf("the input data of copyMatlabFullData() is empty. Do nothing.\n");return false;}

	if (nnodes != ((m>n)?m:n)) {mexPrintf("the nnodes of copyMatlabFullData() is invalid. Do nothing.\n");return false;}

	//now get the data pointers of the Matlab full matrix
	void *pr = 	mxGetData(srcdata);
	V3DLONG i,j;

	mxClassID inimgtype = mxGetClassID(srcdata);
	switch(inimgtype) 
	{
		case mxINT8_CLASS: 
			copyMatlabFullData_real((char *)pr, m, n, adjMatrix, nnodes, minn, maxx);
			break;
		case mxUINT8_CLASS: 
			copyMatlabFullData_real((unsigned char *)pr, m, n, adjMatrix, nnodes, minn, maxx);
			break;
		case mxINT16_CLASS: 
			copyMatlabFullData_real((short int *)pr, m, n, adjMatrix, nnodes, minn, maxx);
			break;
		case mxUINT16_CLASS: 
			copyMatlabFullData_real((unsigned short int *)pr, m, n, adjMatrix, nnodes, minn, maxx);
			break;
		case mxSINGLE_CLASS: 
			copyMatlabFullData_real((float *)pr, m, n, adjMatrix, nnodes, minn, maxx);
			break;
		case mxDOUBLE_CLASS: 
			copyMatlabFullData_real((double *)pr, m, n, adjMatrix, nnodes, minn, maxx);
			break;
		default:
			mexErrMsgTxt("Unsupported data type.");
			break;
	} 

	return true;
}

template <class T> bool copyMatlabFullData_real(T * srcdata, 
									V3DLONG m,
									V3DLONG n, 
									vector <connectionVal> * adjMatrix, 
									V3DLONG nnodes,
									float &minn, 
									float &maxx)
{
	if (!adjMatrix || nnodes<=0 || !srcdata || m<=0 || n<=0)
		{ mexPrintf("the parameters to copyMatlabFullData_real() are invalid. \n"); return false; }

	//now get the data pointers of the Matlab full matrix
	V3DLONG i,j, total=0;

	connectionVal tmpVal;
	bool b_firstone=true;
	for (j=0; j<n; j++)  
	{ 
		for (i=0; i<m; i++)
		{
			if (IsNonZero(srcdata[total]))
			{
				tmpVal.pNode = i;
				tmpVal.cNode = j;
				tmpVal.aVal = float(srcdata[total]);
				adjMatrix[i].push_back(tmpVal);
				
				//mexPrintf("copy full: %d %d %e\n", i,j,float(srcdata[total]));

				if (b_firstone)
				{
					minn=maxx=float(srcdata[total]);
					b_firstone = false;
				}
				else
				{
					if (minn<srcdata[total]) minn=(float)(srcdata[total]);
					if (maxx>srcdata[total]) maxx=(float)(srcdata[total]);
				}
			}
			total++;
		}
	}		
	return true;
}
#endif

template <class T> int new1dArrayMatlabProtocal(T * & img1d, V3DLONG nnode)
{
	img1d = new T [nnode];
	if (!img1d) {
		printf("Fail to allocate mem in newIntImage2dPairMatlabProtocal()!");
		return 0; //fail
	}
	V3DLONG i;
	for (i=0;i<nnode;i++) {img1d[i] = (T)0;}
	return 1; //succeed
}

template <class T> void delete1dArrayMatlabProtocal(T * & img1d)
{
	if (img1d) {delete img1d;img1d=0;}
}

template <class T> int new2dArrayMatlabProtocal(T ** & img2d,T * & img1d,V3DLONG imghei,V3DLONG imgwid)
{
	V3DLONG totalpxlnum = (V3DLONG)imghei*imgwid;
	img1d = new T [totalpxlnum];
	img2d = new T * [(V3DLONG)imgwid];
	
	if (!img1d || !img2d)
	{
		if (img1d) {delete img1d;img1d=0;}
		if (img2d) {delete img2d;img2d=0;}
		printf("Fail to allocate mem in newIntImage2dPairMatlabProtocal()!");
		return 0; //fail
	}
	
	V3DLONG i;
	
	for (i=0;i<imgwid;i++) 
    {img2d[i] = img1d + i*imghei;}
	
	for (i=0;i<totalpxlnum;i++) 
    {img1d[i] = (T)0;}
	
	return 1; //succeed
}

template <class T> void delete2dArrayMatlabProtocal(T ** & img2d,T * & img1d)
{
	if (img1d) {delete img1d;img1d=0;}
	if (img2d) {delete img2d;img2d=0;}
}



#endif


