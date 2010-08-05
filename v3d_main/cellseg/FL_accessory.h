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




// accessory functions for get info from image matrix
// F. Long
// 20080510

#ifndef __ACCESSORY__
#define __ACCESSORY__

# include "FL_defType.h"

// Convert from linear index to array coordinates. 
void ind_to_sub(V3DLONG p, int in_num_dims, V3DLONG *cumprod, V3DLONG *coords)
{
    int j;
    
    for (j = in_num_dims-1; j >= 0; j--)
    {
        coords[j] = p / cumprod[j];
        p = p % cumprod[j];
    }
}

// Convert from array coordinates to zero-based linear index.  
V3DLONG sub_to_ind(V3DLONG *coords, V3DLONG *cumprod, int in_num_dims)
{
    V3DLONG index = 0;
    int k;

    for (k = 0; k < in_num_dims; k++)
    {
        index += coords[k] * cumprod[k];
    }

    return index;
}


// Compute linear relative offset when given relative array offsets.
V3DLONG sub_to_relative_ind(V3DLONG *coords, int in_num_dims)
{
    int N;
    int P;
    int abs_coord;
    V3DLONG cumprod = 1;
    int index = 0;
    int k;

     // Find the maximum absolute neighbor offset.
    N = 0;
    for (k = 0; k < in_num_dims; k++)
    {
        abs_coord = coords[k] > 0 ? coords[k] : -coords[k];
        if (abs_coord > N)
        {
            N = abs_coord;
        }
    }
    P = 2*N + 1;

    // Perform sub_to_ind computation.
    for (k = 0; k < in_num_dims; k++)
    {
        index += coords[k] * cumprod;
        cumprod *= P;
    }

    return index;
}


 // Count nonzero elements 

template <class T> V3DLONG num_nonzeros(const T *D, const V3DLONG num_elements)
{
    int p;
    V3DLONG count = 0;
		
	for (p = 0; p < num_elements; p++)
	{ 
	  if (D[p]) count++; 
	}
    
    return count;
}

#endif