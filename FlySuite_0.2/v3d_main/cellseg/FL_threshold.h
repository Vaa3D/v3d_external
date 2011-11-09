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




// Thresholding methods
// 
// 1. otsu's optimum thresholding method
// 
// F. Long
// 20080824

#ifndef __THRESHOLD__
#define __THRESHOLD__

// calculate histogram
template <class T> float *histogram(T *data1d, V3DLONG data1dsz, V3DLONG maxval)
{
	V3DLONG i,k;
	float val = -999;
	
	for (i=0;i<data1dsz;i++)
	{
		if (data1d[i]> val)
			val = data1d[i];
	}
	
	if (val>maxval)
		maxval = val;

	float *hist = new float [maxval+1]; 
	for (i=0;i<maxval+1;i++) hist[i]=0;
		
	for (i=0;i<data1dsz;i++)
	{	
		hist[V3DLONG(data1d[i])] += 1;
	}
	
	return hist;
}

// otsu's thresholding method
//
// Reference:
// N. Otsu, "A Threshold Selection Method from Gray-Level Histograms,"
// IEEE Transactions on Systems, Man, and Cybernetics, vol. 9, no. 1,
// pp. 62-66, 1979.
// 
// The function here exhaustively search for the threshold that minimizes
// the within-class variance, defined as a weighted sum of variances of the two classes.
// Otsu shows that minimizing the intra-class variance is the same as maximizing 
// inter-class variance
//
// hist: histgorm, a 1d vector
// histsz: the size of the histogram, if hist takes values from 0 to 255, then histsz = 256

V3DLONG otsu_thre(float *hist, V3DLONG histsz)
{ 
	float  w = 0;                // first order cumulative
	float  u = 0;                // second order cumulative
	float  uT = 0;               // total mean level

	int    threshold = 0;        // optimal threshold value
	float  *histNormalized = new float [histsz];  // normalized histogram values //changed by PHC, 2010-05-20
	float  sumval=0, var1=0, var2=0, var=-999;		// working variables

	// Calculate the total sum
	for (int i=0; i<histsz; i++) 
		sumval = sumval + hist[i];
		
	// Normalize histogram values		
	for (int i=0; i<histsz; i++) 
		histNormalized[i] = hist[i]/sumval;

	// Calculate total mean level
	for (int i=0; i<histsz; i++) 
		uT+=(i*histNormalized[i]);

	// Find optimal threshold value
	for (int i=0; i<histsz; i++)
	{
		w+=histNormalized[i];
		u+=(i*histNormalized[i]);
		var1 = (uT * w - u);
		var2 = (var1 * var1) / ( w * (1.0f-w) );
		
//		printf("%f %f %d\n", var, var2, i);
		
		if (var2>var) 
		{ 
			var=var2;
			threshold = i;
		}
	}

//	// Convert the final value to an integer
//	threshold = (int)sqrt(var);
	
	if (histNormalized) {delete []histNormalized; histNormalized=0;} //added by PHC, 2010-05-20
	return threshold;
	
}

// global thresholding, allocate space of dataout outside globalThresh
template <class T1, class T2> void globalThresh(T1 *datain, T2 *dataout, V3DLONG datasz, V3DLONG thre)
{
	V3DLONG i;
	
	for (i=0;i<datasz; i++)
	{
//		printf("%d\n", datain[i]);
		
		if (datain[i]<thre)
		{
			dataout[i] = 0;
		}
		else
		{
			dataout[i] = 1;
		}
	}
			
}

#endif
