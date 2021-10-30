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




// neighborhood class   
// F. Long
// 20080508
// revised from matlab neighborhood.cpp and neighborhood.h


#ifndef __NEIGHBORHOOD__
#define __NEIGHBORHOOD__

#include <stdio.h>

#include "FL_accessory.h"
#include "FL_defType.h"

#define NH_CENTER_MIDDLE_ROUNDUP   0
#define NH_CENTER_MIDDLE_ROUNDDOWN 2
#define NH_CENTER_UL               4
#define NH_CENTER_LR               8


class Neighborhood
//template <class T> class Neighborhood
{
public:

    V3DLONG *its_array_coords;  // its_num_neighbors-by-its_num_dims array containing relative offsets.
	V3DLONG *its_array_sz; // size of the neighbor domain along each dimension
    V3DLONG its_num_neighbors; // number of non-zero elements in the neighbor domain
    V3DLONG its_num_dims; // number of dimensions
	
public:

	Neighborhood()
	{
		its_array_coords = 0;
		its_array_sz = 0;
		its_num_neighbors = 0;
		its_num_dims = 0;
	}
	
	Neighborhood(const V3DLONG code)
	{
	   
	   switch (code)
		{
		case 2:
		case 3:
			createNeighborhoodDefault(code); //generate default neighborhood, note here code means 2d or 3d
			break;
		case 4:
		case 8:
			createNeighborhoodSpecial2d(code);
			break;
		case 6:
		case 18:
		case 26:
			createNeighborhoodSpecial3d(code);
			break;
		default:
			fprintf(stderr, "Only support values 2,3,4,8,6,18,26.\n"); return;     
			break;
		}			
	}
	
	template <class T> Neighborhood(const T *D, const V3DLONG *sz, const V3DLONG in_num_dims, V3DLONG center_location_flag) //generate user defined neighborhood
	{
			
		V3DLONG num_elements;
		V3DLONG *cumprod;
		V3DLONG p;
		V3DLONG q;
		V3DLONG *coords;
		V3DLONG count = 0;
		

	   switch (in_num_dims)
		{
		case 2:
			{
				num_elements = (V3DLONG)(sz[0]*sz[1]);
				its_array_sz = new V3DLONG [2];
				its_array_sz[0] = sz[0];
				its_array_sz[1] = sz[1];
			}
			break;
		case 3:
			{
				num_elements = (V3DLONG)(sz[0]*sz[1]*sz[2]);
				its_array_sz = new V3DLONG [3];
				its_array_sz[0] = sz[0];
				its_array_sz[1] = sz[1];
				its_array_sz[1] = sz[2];			
			}
			break;
		default:
			{fprintf(stderr, "Only support 2D and 3D data.\n"); return;}    
			break;
		}			
						
		its_num_neighbors = num_nonzeros(D, num_elements); //its_num_neighbors is a member variable
		
		its_num_dims = in_num_dims; //its_num_dims is a member variable
		
		its_array_coords = new V3DLONG [its_num_neighbors * its_num_dims];
		
		cumprod = new V3DLONG [its_num_dims]; 
		cumprod[0] = 1;
		
		for (p = 1; p < its_num_dims; p++)
		{
			cumprod[p] = cumprod[p-1] * sz[p-1];
		}
		
		for (p = 0; p < num_elements; p++)
		{
			if (D[p])
			{
				coords = its_array_coords + count*its_num_dims; //its_array_coords is the member variable
				ind_to_sub(p, its_num_dims, cumprod, coords);
				if(center_location_flag == NH_CENTER_MIDDLE_ROUNDDOWN)
				{
					/*
					 * Subtract the location of the center pixel from
					 * the neighbor coordinates.
					 */
					for (q = 0; q < its_num_dims; q++)
					{
						coords[q] -= (sz[q] - 1) / 2;
					}
				}
				else if(center_location_flag == NH_CENTER_UL)
				{
					/*
					 * No change required for center in Upper Left
					 * assuming that ind_to_sub returns a zero based
					 * subscript with the top in the upper left
					 */
				}
				else if(center_location_flag == NH_CENTER_LR)
				{
					for (q = 0; q < its_num_dims; q++)
					{
						coords[q] -= (sz[q] - 1);
					}
				}
				if(center_location_flag == NH_CENTER_MIDDLE_ROUNDUP)
				{
					/*
					 * Subtract the location of the center pixel from
					 * the neighbor coordinates.
					 */
					for (q = 0; q < its_num_dims; q++)
					{
						coords[q] -= (sz[q] - 1) / 2 +
							((sz[q] - 1) % 2 ? 1:0);
					}
				}
				count++;
			}
		}
		
		if (cumprod) {delete [] cumprod; cumprod = 0;}
	}
	
   // Create 2-D neighborhood with 4 or 8 neighbors. 
	void createNeighborhoodSpecial2d(V3DLONG code)
	{
		V3DLONG r;
		V3DLONG c;
		V3DLONG max;
		V3DLONG sum;
		V3DLONG p;
		V3DLONG count;

		switch (code)
		{
		case 4:
			/*
			 * Use only edge-connected neighbors
			 */
			max = 1;
			break;
			
		case 8:
			/*
			 * Use edge- or vertex-connected neighbors
			 */
			max = 2;
			break;
			
		default:
				fprintf(stderr, "only values of 4 or 8 are allowed.\n"); return;     
				break;
		}

		// set member variables
		its_num_dims = 2;
		its_num_neighbors = code + 1; /* + 1 because of center pixel */
		
		its_array_sz = new V3DLONG [2];
		its_array_sz[0] = its_array_sz[1] = 3;
		
		its_array_coords = new V3DLONG [its_num_neighbors * its_num_dims];
		
		
		count = 0;
		for (c = -1; c <= 1; c++)
		{
			for (r = -1; r <= 1; r++)
			{
				/*
				 * sum == 1 implies edge-connected
				 * sum == 2 implies edge- or vertex-connected
				 */
				sum = (r != 0) + (c != 0);
				if (sum <= max)
				{
					p = count*its_num_dims;
					its_array_coords[p] = r;
					its_array_coords[p + 1] = c;
					count++;
				}
			}
		}

	}


	// Create 3-D neighborhood with 6, 18, or 26 neighbors.
	void createNeighborhoodSpecial3d(V3DLONG code)
	{
		V3DLONG r;
		V3DLONG c;
		V3DLONG z;
		V3DLONG max;
		V3DLONG sum;
		V3DLONG p;
		V3DLONG count;

		switch (code)
		{
		case 6:
			/* Face-connected neighbors only */
			max = 1;
			break;
			
		case 18:
			/* Face- or edge-connected neighbors */
			max = 2;
			break;
			
		case 26:
			/* Face-, edge-, or vertex-connected neighbors */
			max = 3;
			break;
		default:
			fprintf(stderr, "Invalid connectivity parameter, must be 6, 18, or 26 for 3D data.\n"); return;
			break;     
		}

		// set member variables
		its_num_dims = 3;
		its_array_sz = new V3DLONG [3];
		its_array_sz[0] = its_array_sz[1] = its_array_sz[2] = 3;
		its_num_neighbors = code + 1; /* its_num_neighbors is a member variable,  +1 because of the center pixel */
		its_array_coords = new V3DLONG [its_num_neighbors * its_num_dims],

		count = 0;
		for (z = -1; z <= 1; z++)
		{
			for (c = -1; c <= 1; c++)
			{
				for (r = -1; r <= 1; r++)
				{
					/*
					 * sum == 1 implies face-connected
					 * sum == 2 implies edge-connected
					 * sum == 3 implies vertex-connected
					 */
					sum = (r != 0) + (c != 0) + (z != 0);
					if (sum <= max)
					{
						p = count*its_num_dims;
						its_array_coords[p] = r;
						its_array_coords[p + 1] = c;
						its_array_coords[p + 2] = z;
						count++;
					}
				}
			}
		}
		
		if (count != its_num_neighbors)
		{
				fprintf(stderr, "error in createNeighborhoodSpecial3d"); return;     
		}
	}

	
	//generate 2d or 3d default neighborhood 
	void createNeighborhoodDefault(const V3DLONG in_num_dims)
	{
		V3DLONG *size = new V3DLONG [in_num_dims];
		V3DLONG *cumprod = new V3DLONG [in_num_dims];
		V3DLONG my_num_neighbors = 1;
		V3DLONG k;
		V3DLONG q;
		V3DLONG *coords;

		for (k = 0; k < in_num_dims; k++)
		{
			size[k] = 3;
			cumprod[k] = my_num_neighbors;
			my_num_neighbors *= 3;
		}

		//set member variables
		its_num_dims = in_num_dims;
		its_array_coords = new V3DLONG [my_num_neighbors * its_num_dims]; //its_array_coords is the member variable
		its_num_neighbors = my_num_neighbors; //its_num_neighbors is the member variable
		
		switch (in_num_dims)
		{
			case 2:
				{
					its_array_sz = new V3DLONG [2];
					its_array_sz[0] = its_array_sz[1] = 3;
				}
				break;
			case 3:
				{
					its_array_sz = new V3DLONG [3];
					its_array_sz[0] = its_array_sz[1] = its_array_sz[2] = 3;
				}
				break;
			default:
				fprintf(stderr, "Only support 2D and 3D data.\n"); return;     

		}
				
		for (k = 0; k < its_num_neighbors; k++)
		{
			coords = its_array_coords + k*its_num_dims;
			ind_to_sub(k, its_num_dims, cumprod, coords);
			for (q = 0; q < its_num_dims; q++)
			{
				coords[q] -= 1;
			}
		}
		
		if (size) { delete [] size; size = 0;}
		if (cumprod) { delete [] cumprod; cumprod = 0;}
		
	}

		
	void reflectNeighborhood() //Negate all neighborhood offset values
	{
		V3DLONG *coords = its_array_coords;
		V3DLONG num_elements = its_num_neighbors * its_num_dims;
		V3DLONG k;
		
		for (k = 0; k < num_elements; k++)
		{
			coords[k] = -coords[k];
		}
	}

	
	~Neighborhood()
	{ 
		if (its_array_coords) { delete [] its_array_coords; its_array_coords=0;}
		if (its_array_sz) { delete [] its_array_sz; its_array_sz=0;}		
	}
	
};

#endif
