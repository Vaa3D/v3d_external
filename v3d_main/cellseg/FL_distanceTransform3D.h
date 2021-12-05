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




// 3D distance transform
// F. Long
// 20080822

#ifndef __DISTANCE_TRANSFORM3D__
#define __DISTANCE_TRANSFORM3D__

#include "./local_basic_c_fun/volimg_proc.h"
#include "./local_basic_c_fun/img_definition.h"

#include "FL_bwdist.h"

// inimg: input image
// distimg: distance transform of the input image
// indeximg: index of the region for each pixel, to which the distance is shortest
// tag: 1 if computing the distance transform of non-zero values, 0 if compute the distance transform of zero values

template <class T> bool distTrans3d(Vol3DSimple <T> *inimg, Vol3DSimple <float> *distimg, Vol3DSimple <V3DLONG> *indeximg, unsigned char tag)
{	
	if (!inimg || !inimg->valid() || !distimg || !distimg->valid() || !indeximg || !indeximg->valid())
	{	printf("invalid image in distance transform \n");
		return false;
	}
	
	V3DLONG *sz = new V3DLONG [3];
	sz[0] = inimg->sz0();
	sz[1] = inimg->sz1();
	sz[2] = inimg->sz2();
	
	V3DLONG len = sz[0]*sz[1]*sz[2];

	T *p = inimg->getData1dHandle();
	float *data1d = distimg->getData1dHandle();	
	V3DLONG *pix_index = indeximg->getData1dHandle();
	
	V3DLONG k;
		
	for (k=0;k<len;k++)
	{
		data1d[k] = (float) p[k];	
	}
	
	dt3d_binary(data1d, pix_index, sz, tag); // compute the distance transform for foreground (non-zero) pixels	if tag = 1
	
	return true;
}

template <class T1, class T2> bool distTrans3d(Vol3DSimple <T1> *inimg, Vol3DSimple <T2> *distimg, Vol3DSimple <V3DLONG> *indeximg, unsigned char tag)
{	
	if (!inimg || !inimg->valid() || !distimg || !distimg->valid() || !indeximg || !indeximg->valid())
	{	printf("invalid image in distance transform \n");
		return false;
	}
	
	V3DLONG *sz = new V3DLONG [3];
	sz[0] = inimg->sz0();
	sz[1] = inimg->sz1();
	sz[2] = inimg->sz2();
	
	T1 *indata1d = inimg->getData1dHandle();
	T2 *outdata1d = distimg->getData1dHandle();	
	V3DLONG *pix_index = indeximg->getData1dHandle();
	
	dt3d_binary(indata1d, outdata1d, pix_index, sz, tag); // compute the distance transform for foreground (non-zero) pixels	if tag = 1
	
	return true;
}


#endif
	
