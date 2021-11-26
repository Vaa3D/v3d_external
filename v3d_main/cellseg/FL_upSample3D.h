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




// upsample a 3D volume by a factor along each dimension
// F. Long
// 20080507
// 20080830 updated

#ifndef __UP_SAMPLE3D__
#define __UP_SAMPLE3D__

#include "./local_basic_c_fun/volimg_proc.h"
#include "./local_basic_c_fun/img_definition.h"
#include "./local_basic_c_fun/stackutil.h"

#include "FL_interpolateCoordLinear3D.h"
#include "FL_coordDefinition.h"
#include "FL_defType.h"

template <class T> bool upsample3dvol(T ***outdata, T ***indata, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2, double *dfactor)
{ 

	V3DLONG tsz0o = (V3DLONG)(ceil(dfactor[0]*tsz0)), tsz1o = (V3DLONG)(ceil(dfactor[1]*tsz1)), tsz2o = (V3DLONG)(ceil(dfactor[2]*tsz2));
	
	V3DLONG totallen = tsz0o * tsz1o * tsz2o; 
	Coord3D * c = new Coord3D [totallen];
	T * v = new T [totallen];
	
	if (!c || !v)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d]\n", __FILE__, __LINE__);
		if (c) {delete []c; c=0;}
		if (v) {delete []v; v=0;}
		return false;
	}
	
    //computation
	V3DLONG i,j,k, ind;
    ind=0;
	
	for (k=0;k<tsz2o;k++)
	{
		for (j=0;j<tsz1o;j++)
		{
			for (i=0;i<tsz0o;i++)
			{
				c[ind].x = double(i)/dfactor[0];
#ifndef POSITIVE_Y_COORDINATE
				c[ind].y = double(tsz1o-1-j)/dfactor[1];
#else
				c[ind].y = double(j)/dfactor[1]; 
#endif
				c[ind].z = double(k)/dfactor[2];
				v[ind] = -1; //set as a special value 
				ind++;
			}
		}
	}
	
	interpolate_coord_linear(v, c, totallen, indata, tsz0, tsz1, tsz2, 0, tsz0-1, 0, tsz1-1, 0, tsz2-1);	
	
    ind=0;
	for (k=0;k<tsz2o;k++)
	{
		for (j=0;j<tsz1o;j++)
		{
			for (i=0;i<tsz0o;i++)
			{
				outdata[k][j][i] = (T)(v[ind++]);
			}
		}
	}

	
    // free temporary memory
	if (c) {delete []c; c=0;}
	if (v) {delete []v; v=0;}
	return true;
}


template <class T> bool upsample3dvol(Vol3DSimple<T> * & outimg, Vol3DSimple<T> * inimg, double *dfactor) 
{
    //verify the input parameters
	
    if (outimg || !inimg || !inimg->valid() || dfactor[0]<1 || dfactor[1]<1 || dfactor[2]<1) 
	   {fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__); return false;}
	

	T *** timg3din = inimg->getData3dHandle();
	V3DLONG tsz0 = inimg->sz0();
	V3DLONG tsz1 = inimg->sz1();
	V3DLONG tsz2 = inimg->sz2(); 

	V3DLONG tsz0o = V3DLONG(ceil(dfactor[0]*tsz0)), tsz1o = V3DLONG(ceil(dfactor[1]*tsz1)), tsz2o = V3DLONG(ceil(dfactor[2]*tsz2));
	
    outimg = new Vol3DSimple<T> (tsz0o, tsz1o, tsz2o);
	if (!outimg || !outimg->valid())
	{
		fprintf(stderr, "Fail to allocate memory in upsample3dvol(). [%d]\n", __LINE__);
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}
	
	T *** timg3dout = outimg->getData3dHandle();
	upsample3dvol(timg3dout, timg3din, tsz0, tsz1, tsz2, dfactor);
	
	return true;
}	

#endif
