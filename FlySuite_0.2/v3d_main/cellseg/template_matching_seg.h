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




//template_match_seg.h
//by Hanchuan Peng
//090426

#ifndef __TEMPLATE_MATCHING_SEG_H__
#define __TEMPLATE_MATCHING_SEG_H__

#include "../basic_c_fun/img_definition.h"

#include "../v3d/template_matching_cellseg_dialog.h"

bool template_matching_seg(Vol3DSimple <unsigned char> *img3d, Vol3DSimple <unsigned short int> *outimg3d, const para_template_matching_cellseg & mypara);

Vol3DSimple<double> * genGaussianKernal3D(V3DLONG szx, V3DLONG szy, V3DLONG szz, double sigmax, double sigmay, double sigmaz);

template <class T1, class T2> double compute_corrcoef_two_vectors(T1 *v1, T2 *v2, V3DLONG len);
template <class T1, class T2> double compute_normalized_L1dist_two_vectors(T1 *v1, T2 *v2, V3DLONG len, bool b_remove_mean);
template <class T1, class T2> double compute_normalized_L2dist_two_vectors(T1 *v1, T2 *v2, V3DLONG len, bool b_remove_mean);


#endif


