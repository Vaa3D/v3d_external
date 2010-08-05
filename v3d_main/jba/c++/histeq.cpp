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




//histeq.cpp
//by Hanchuan Peng
//080611
//080612

#include "histeq.h"

#include <stdio.h>

bool hist_eq_uint8(unsigned char * data1d, V3DLONG len)
{
	if (!data1d || len<=0)
	{
		printf("The input parameters are invalid in hist_eq_uint8().\n");
		return false;
	}
	
	//computing the histogram
	V3DLONG NBIN=256;
	V3DLONG *h = new V3DLONG [NBIN]; //histogram
	double *c = new double [NBIN]; //cumulative distribution
	if (!h)
	{
		printf("Fail to allocate memory in hist_eq_uint8().\n");
		return false;
	}
	
	V3DLONG i;
	
	for (i=0;i<NBIN;i++) h[i]=0;
	
	for (i=0;i<len; i++)
	{
		h[data1d[i]]++;
	}
	
	c[0]=h[0];
	for (i=1;i<NBIN;i++) c[i] = c[i-1]+h[i];
	for (i=0;i<NBIN;i++) {c[i] /= c[NBIN-1]; c[i] *= (NBIN-1);}
	
	for (i=0;i<len;i++)
	{
		data1d[i] = c[data1d[i]];
	}
	
	if (c) {delete []c; c=0;}
	if (h) {delete []h; h=0;}
	return true;
}

bool hist_eq_range_uint8(unsigned char * data1d, V3DLONG len, unsigned char lowerbound, unsigned char upperbound) //only eq the [lowerbound,upperbound]
{
	if (!data1d || len<=0)
	{
		printf("The input parameters are invalid in hist_eq_uint8().\n");
		return false;
	}
	
//	if (lowerbound<0 || lowerbound>255 || upperbound<0 || upperbound>255) //note that this should never happen as lowerbound and higherbound are declared as unsigned char
//	{
//		printf("The input lowerbound and upperbound parameter are wrong in hist_eq_uint8(). Do nothing.\n");
//		return false;
//	}
	
	if (lowerbound>upperbound) //swap low and high if necessary
	{
		unsigned char tmp=lowerbound; lowerbound=upperbound; upperbound=tmp;
	}
	
	//computing the histogram
	V3DLONG NBIN=256;
	V3DLONG *h = new V3DLONG [NBIN]; //histogram
	double *c = new double [NBIN]; //cumulative distribution
	if (!h)
	{
		printf("Fail to allocate memory in hist_eq_uint8().\n");
		return false;
	}
	
	V3DLONG i;
	
	for (i=0;i<NBIN;i++) h[i]=0;
	
	for (i=0;i<len; i++)
	{
		h[data1d[i]]++;
	}
	
	c[lowerbound]=h[lowerbound];
	for (i=lowerbound+1;i<=upperbound;i++) c[i] = c[i-1]+h[i];
	double range = upperbound-lowerbound;
	for (i=lowerbound;i<=upperbound;i++) {c[i] /= c[upperbound]; c[i] *= range; c[i] += lowerbound;}
	
	for (i=0;i<len;i++)
	{
		if (data1d[i]>=lowerbound && data1d[i]<=upperbound)
			data1d[i] = c[data1d[i]];
	}
	
	if (c) {delete []c; c=0;}
	if (h) {delete []h; h=0;}
	return true;
}

