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




/* define structural elements and morphology operations, dilate, erode, open, close, and fill
 * F. Long
 * 20080512
 */

#ifndef __STRUCTURAL_ELEMENT__
#define __STRUCTURAL_ELEMENT__

#include "FL_neighborhood.h"
#include "FL_neighborhoodWalker.h"


class Strel
{
	V3DLONG *its_offset;
	V3DLONG its_num_neighbors;
	V3DLONG its_num_dims;
	V3DLONG *its_array_sz;

public:

	Strel();
	void printStrelInfo();
	void createDiskStrel(const V3DLONG radius);
	void createBallStrel(const V3DLONG radius);	
	void creatCuboidStrel(const V3DLONG *cuboid_sz);
	void creatRectangleStrel(const V3DLONG *rectangle_sz);	
	Neighborhood * convertStrelToNeighbor();
	
	~Strel();
};



//void bwDilate(const bool *indata, const V3DLONG *indata_sz, const V3DLONG indata_dims, bool *outdata, Strel *se); 
//void bwErode(const bool *indata, const V3DLONG *indata_sz, const V3DLONG indata_dims, bool *outdata, Strel *se);
//void bwOpen(const bool *indata, const V3DLONG *indata_sz, const V3DLONG indata_dims, bool *outdata, Strel *se);
//void bwClose(const bool *indata, const V3DLONG *indata_sz, const V3DLONG indata_dims, bool *outdata, Strel *se);

template <class T> void bwDilate(Vol3DSimple <T> *img3d,  Strel *se);
template <class T> void bwDilate(Image2DSimple <T> *img2d,  Strel *se);

template <class T> void bwErode(Vol3DSimple <T> *img3d,  Strel *se);
template <class T> void bwErode(Image2DSimple <T> *img2d,  Strel *se);

template <class T> void bwClose(Vol3DSimple<T> *img3d, Strel *se);
template <class T> void bwClose(Image2DSimple<T> *img2d, Strel *se);

template <class T> void bwOpen(Vol3DSimple<T> *img3d, Strel *se);
template <class T> void bwOpen(Image2DSimple<T> *img2d, Strel *se);

template <class T> void bwErode(T *indata,  V3DLONG len0, V3DLONG len1, V3DLONG len2, Strel *se);


#endif
