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




// downsample a 3D volume by a factor along each dimension
// F. Long
// 20080507
// 20080830 update

#ifndef __DOWN_SAMPLE3D__
#define __DOWN_SAMPLE3D__

#include "./local_basic_c_fun/volimg_proc.h"
#include "./local_basic_c_fun/img_definition.h"
#include "./local_basic_c_fun/stackutil.h"

#include "FL_defType.h"

// tag indicate whether pixels in the downsampled image should be the average of neighboring pixels in the original image (tag = 0)
// or sample pixlels in the original image directly (tag other than 0 )

template <class T> bool downsample3dvol(T ***outdata, T *** indata, V3DLONG *szin, double *dfactor, unsigned char tag)
{	
	
	V3DLONG szout[3];
	for (V3DLONG i=0; i<3; i++)
	{
		szout[i] = (V3DLONG)(floor(double(szin[i]) / double(dfactor[i])));
		if (szout[0] <= 0)
		{
			fprintf(stderr, "The dfactor is not properly set, -- the downsampled size is too small. Do nothing. [%s][%d].\n", __FILE__, __LINE__);
			return false;
		}
	}
	
	if (tag == 0)
	{
		for (V3DLONG k=0;k<szout[2];k++)
		{
			V3DLONG k2low=V3DLONG(floor(k*dfactor[2])), k2high=V3DLONG(floor((k+1)*dfactor[2]-1));
			if (k2high>szin[2]) k2high = szin[2];
			V3DLONG kw = k2high - k2low + 1;
			
			for (V3DLONG j=0;j<szout[1];j++)
			{
				V3DLONG j2low=V3DLONG(floor(j*dfactor[1])), j2high=V3DLONG(floor((j+1)*dfactor[1]-1));
				if (j2high>szin[1]) j2high = szin[1];
				V3DLONG jw = j2high - j2low + 1;
				
				for (V3DLONG i=0;i<szout[0];i++)
				{
					V3DLONG i2low=V3DLONG(floor(i*dfactor[0])), i2high=V3DLONG(floor((i+1)*dfactor[0]-1));
					if (i2high>szin[0]) i2high = szin[0];
					V3DLONG iw = i2high - i2low + 1;
					
					double cubevolume = double(kw) * jw * iw;
					//cout<<cubevolume <<" ";
					
					double s=0.0;
					for (V3DLONG k1=k2low;k1<=k2high;k1++)
					{
						for (V3DLONG j1=j2low;j1<=j2high;j1++)
						{
							for (V3DLONG i1=i2low;i1<=i2high;i1++)
							{
								s += indata[k1][j1][i1];
							}
						}
					}
					
					outdata[k][j][i] = (T)(s/cubevolume);
	//				if (outdata[k][j][i]>0)
	//					printf("%f\n", s/cubevolume);
				}
			}
		}
	}
	else
	{
		for (V3DLONG k=0;k<szout[2]; k++)
		{
			V3DLONG k2low=V3DLONG(floor(k*dfactor[2])), k2high=V3DLONG(floor((k+1)*dfactor[2]-1));
			if (k2high>szin[2]) k2high = szin[2];
			V3DLONG kw = k2high - k2low + 1;
			
			for (V3DLONG j=0;j<szout[1];j++)
			{
				V3DLONG j2low=V3DLONG(floor(j*dfactor[1])), j2high=V3DLONG(floor((j+1)*dfactor[1]-1));
				if (j2high>szin[1]) j2high = szin[1];
				V3DLONG jw = j2high - j2low + 1;
				
				for (V3DLONG i=0;i<szout[0];i++)
				{
					V3DLONG i2low=V3DLONG(floor(i*dfactor[0])), i2high=V3DLONG(floor((i+1)*dfactor[0]-1));
					if (i2high>szin[0]) i2high = szin[0];
					V3DLONG iw = i2high - i2low + 1;
					
					outdata[k][j][i] = indata[int((k2low+k2high)/2)][int((j2low+j2high)/2)][int((i2low+i2high)/2)]; 
					
				}
			}
		}
	}
	
	return true;
}

// tag indicate whether pixels in the downsampled image should be the average of neighboring pixels in the original image (tag = 0), or
// sample pixlels in the original image directly (tag other than 0)

template <class T> bool downsample3dvol(Vol3DSimple<T> * &outimg, Vol3DSimple<T> * inimg, double *dfactor, unsigned char tag)
{
	// check arguments
	if (outimg || !inimg || !inimg->valid())
	{
		fprintf(stderr, "The outimg pointer is not initialized as NULL or get NULL input image pointer [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
	
	for (V3DLONG i=0; i<3; i++)
	{
		if (int(dfactor[i])<1 || int(dfactor[i]) >32)
		{
			fprintf(stderr, "The downsampling factor must be >=1 and <= 32 [%s][%d] dfactor=%.2f.\n", __FILE__, __LINE__, dfactor);
			return false;
		}
	}
	
	V3DLONG sz[3];
	sz[0] = inimg->sz0();
	sz[1] = inimg->sz1();
	sz[2] = inimg->sz2();
	
	V3DLONG cur_sz[3];
	for (V3DLONG i=0; i<3; i++)
	{
		cur_sz[i] = (V3DLONG)(floor(double(sz[i]) / double(dfactor[i])));
		if (cur_sz[0] <= 0)
		{
			fprintf(stderr, "The dfactor is not properly set, -- the downsampled size is too small. Do nothing. [%s][%d].\n", __FILE__, __LINE__);
			return false;
		}
	}
	
	outimg = new Vol3DSimple<T> (cur_sz[0], cur_sz[1], cur_sz[2]);
	
	if (!outimg)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
	
	T *** out_tmp3d = outimg->getData3dHandle();
	T *** in_tmp3d = inimg->getData3dHandle();
	
	downsample3dvol(out_tmp3d, in_tmp3d, sz, dfactor, tag);
	
	return true;
}




#endif
