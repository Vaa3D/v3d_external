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




// watershed segmentation using revised Vincent and Soille method
//
// L. Vincent and P. Soille, "Watershed in digital
// spaces: An efficient algorithm based on immersion simulations," IEEE
// Transactions on Pattern Analysis and Machine Intelligence, vol. 13, n. 6,
// 1990, pp. 583-598.  
//
// F. Long
// 20080525


#ifndef __FL_WATERSHED_VS__
#define __FL_WATERSHED_VS__

#define INIT -1
#define MASK -2
#define WSHED 0
#define FICTITIOUS -1


#include "FL_neighborhood.h"
#include "FL_queue.h"
#include "FL_sort.h"
#include "FL_bwdist.h"
//#include "FL_distanceTransform3D.h"

    
	
void remove_watershed_lines(float *&label_data, const V3DLONG *sz, const V3DLONG ndims)
{

	V3DLONG num_elements = 1;
	V3DLONG i;
	
//	for (i=0; i<2; i++) //for test 2d purpose
	for (i=0; i<ndims; i++)
		num_elements = num_elements * sz[i];	
			
	V3DLONG *pix_index = new V3DLONG [num_elements];

	float *tmp_data = new float [num_elements];
	
	for (i=0; i<num_elements; i++)
	{	
		if (label_data[i]==1)
			tmp_data[i] = 0;
		else
			if (label_data[i]>1)
				tmp_data[i] = 1;
	}
	dt3d_binary(tmp_data, pix_index, sz, 0);


//	for (i=0; i<num_elements; i++)
//	{	
//		tmp_data[i] = label_data[pix_index[i]];
//	}
//
//	for (i=0; i<num_elements; i++)
//	{	
//		label_data[i] = tmp_data[i];
//	}


	for (i=0; i<num_elements; i++)
	{	
		tmp_data[i] = label_data[i];
	}


	for (i=0;i<num_elements; i++)
	{
		if (tmp_data[i]==0)
			label_data[i] = tmp_data[pix_index[i]]; // assign watershed lines to one of the regions
	}

	for (i=0;i<num_elements; i++)
	{
		label_data[i] = label_data[i]-1; // decrease the labels of all regions by 1, so that background is 0
	}


	delete [] tmp_data; tmp_data = 0;
	delete [] pix_index; pix_index = 0;

}


template <class T> void compute_watershed(T * indata, float *sortidx, const V3DLONG num_elements, NeighborhoodWalker * nh_walker, float * &label_data)
{

    V3DLONG       current_label = 0;
    V3DLONG       current_distance;
    T              current_level;
    V3DLONG       closest_dist;
    V3DLONG       closest_label_val;
    bool           closest_label_val_unique;
    Queue<V3DLONG> PixelQueue;
    V3DLONG       *dist = new V3DLONG [num_elements]; // work image of distances
	V3DLONG       count; // count the number of pixels already processed
    V3DLONG       k, k1, k2, i;
    V3DLONG       p, q, r;
    
     
    // do nothing if indata is empty 
    
    if (num_elements == 0)
    {
        return;
    }
    
    // assign INIT to each elements in label_data
    
    for (k = 0; k < num_elements; k++)
    {
        label_data[k] = INIT;
    }
    
    // Initialize the pixel queue
    
    PixelQueue.initialize(32);

	for(i=0;i<num_elements; i++)
		dist[i] = 0;
		
    count = 0;
    while (count < num_elements)
    {
        // Find the next set of pixels that all have the same value 
		   
        k1 = count;
        current_level = indata[(V3DLONG) sortidx[k1]];
        k2 = k1;
		
//		while ((k2 < num_elements) && (indata[(V3DLONG) sortidx[k2]] == current_level))
//			k2++;

        do
        {
            k2++;
        } 
        while ((k2 < num_elements) && (indata[(V3DLONG) sortidx[k2]] == current_level));
        k2--;
				
        // Mask all image pixels whose value equals current_level
        
        for (k = k1; k <= k2; k++)
        {
            p = (V3DLONG) sortidx[k];
            label_data[p] = MASK;
			
            nh_walker->setWalkerLocation(p);
			
            while (nh_walker->getNextInboundsNeighbor(&q, NULL))
            {
                if ((label_data[q] > 0) || (label_data[q] == WSHED))
                {
                    // Initialize queue with neighbors at level 'current_level'
                    // of current basins or watersheds
					
					dist[p] = 1;
                    PixelQueue.put(p);
//					printf("%d ---\n", p);
					
                    break;
                }
            }
            count++;
        }

        current_distance = 1;
        PixelQueue.put(FICTITIOUS);
        
        // Extend the basins
        
        while (true)
        {
            p = PixelQueue.get();
            if (p == FICTITIOUS)
            {
                if (PixelQueue.getSequenceLength() == 0)
                {
                    break;
                }
                else
                {
                    PixelQueue.put(FICTITIOUS);
                    current_distance++;
                    p = PixelQueue.get();
                }
            }

            // differnt from Vincent and Soille's paper  

            // Find the labeled or watershed neighbors with the closest
            // distance.  At the same time, put any masked neighbors whose
            // distance is 0 onto the queue and reset their distance to 1.
            
            closest_dist = 999999;
            closest_label_val = 0;
            closest_label_val_unique = true;
			
            nh_walker->setWalkerLocation(p);
            
			while (nh_walker->getNextInboundsNeighbor(&q, NULL))
            {
                if ((label_data[q] > 0) || (label_data[q] == WSHED))
                {
                    if (dist[q] < closest_dist)
                    {
                        closest_dist = dist[q];
                        if (label_data[q] > 0)
                        {
                            closest_label_val = (V3DLONG)label_data[q];
                        }
                    }
                    else if (dist[q] == closest_dist)
                    {
                        if (label_data[q] > 0)
                        {
                            if ((closest_label_val > 0) &&
                                (label_data[q] != closest_label_val))
                            {
                                closest_label_val_unique = false;
                            }
                            closest_label_val = (V3DLONG)label_data[q];
                        }
                    }
                }

                else if ((label_data[q] == MASK) && (dist[q] == 0)) // q is a plateau pixel.
                {
                    
                    dist[q] = current_distance + 1;
                    PixelQueue.put(q);
					
//					printf("%d ---\n", q);
					
                }
            }

            // Label p

            if ((closest_dist < current_distance) && (closest_label_val > 0)) //q belongs an existing basin
            {
                if (closest_label_val_unique && 
                    ((label_data[p] == MASK) || (label_data[p] == WSHED)))
                {
                    label_data[p] = closest_label_val;

                }
                else if (! closest_label_val_unique ||
                         (label_data[p] != closest_label_val))
                {
                    label_data[p] = WSHED;
                }
            }
            else if (label_data[p] == MASK) // q belongs to a watershed
            {
                label_data[p] = WSHED;
            }
        }

        // Detect and process new minima at current_level
        for (k = k1; k <= k2; k++)
        {
            p = (V3DLONG) sortidx[k];
            dist[p] = 0;
            if (label_data[p] == MASK)
            {
                // p is inside a new minimum

                current_label++;  // create a new label
				
                PixelQueue.put(p);
//				printf("%d---\n", p);
									
                label_data[p] = current_label;
				
                while (PixelQueue.getSequenceLength() > 0)
                {
					q = PixelQueue.get();

//					printf("%d, %d ***\n", q, PixelQueue.getSequenceLength());
											
				   if (label_data[q] == MASK) //20080813 add
					{
						label_data[q] = current_label;
					}

                     // Inspect neighbors of q

                    nh_walker -> setWalkerLocation(q);
                    while (nh_walker -> getNextInboundsNeighbor(&r, NULL))
                    {
							
                        if (label_data[r] == MASK)
                        {
                            PixelQueue.put(r);
//							printf("%d ---\n", r);	
													
                            label_data[r] = current_label;
							
                        }
                    }
                }
            }
        }
		
    }

    PixelQueue.freeSequence();

	if (dist) { delete [] dist; dist = 0;}

}


template <class T> void watershed_vs(T *indata, float * &label_data, const V3DLONG *sz, const V3DLONG ndims, const V3DLONG conn_code)
{	
    Neighborhood *nh = new Neighborhood(conn_code);
    NeighborhoodWalker *nh_walker = new NeighborhoodWalker(nh, sz, ndims, NH_SKIP_CENTER);
	float *sortidx=0, *sortidx2=0;
	
	V3DLONG num_elements = 1;
	V3DLONG i;

	for (i=0; i<ndims; i++)
		num_elements = num_elements * sz[i];
	
	if (label_data)
	{
		fprintf(stderr,"label_data must be NULL\n");
		return;
	};

	
	label_data = new float [num_elements];
	
	sortidx = new float [num_elements+1]; // sort2 does not sort the first element, add an element so that everyone is sorted	
	float *indata2 = new float [num_elements+1];
	
	indata2[0] = -999; // the 0th element is not sorted in sort2
	sortidx[0] = -999;
	
	for (i=0; i<num_elements; i++)
	{	indata2[i+1] = float(indata[i]);
		sortidx[i+1] = i;
	}
		
	sort2(num_elements, indata2, sortidx); // the first element is not sorted

	delete [] indata2; indata2 = 0;

	sortidx2 = new float [num_elements];
	for (i=0; i<num_elements; i++)
	{
		sortidx2[i] = sortidx[i+1];		
	}
	delete [] sortidx; sortidx = 0;
	
	// compute watershed
	compute_watershed(indata, sortidx2, num_elements, nh_walker, label_data);
	
	// remove watershed lines
	remove_watershed_lines(label_data, sz, ndims);
	
	if (nh) { delete nh; nh = 0;}
	if (nh_walker) {delete nh_walker; nh_walker = 0;}	
}


#endif //__FL_WATERSHED_VS__

