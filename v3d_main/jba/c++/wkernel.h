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




//wkernel.h: the program to define kernels
// date: 060509
// by Hanchuan Peng
// last change: 060704: add the support for KT_CUBE_ALL1, KT_SPHERE_ALL1 kernels
// 070514: add a valid function in the class

#ifndef __WEIGHT_KERNEL__
#define __WEIGHT_KERNEL__

#include "../../basic_c_fun/v3d_basicdatatype.h"

enum KernelType {KT_GMOMENT, KT_CUBE_ALL1, KT_SPHERE_ALL1};

class WeightKernel3D
{
public:
  double * data1d;
  V3DLONG totalLen;
  double *** data3d;
  V3DLONG xwid, ywid, zwid, rwid; //dimensions of x,y,z, r
  V3DLONG x0, y0, z0; //center location
  WeightKernel3D(V3DLONG xw, V3DLONG yw, V3DLONG zw);
  WeightKernel3D(V3DLONG r);
  ~WeightKernel3D();

  void cleanData();
  void initData();
  int createInternalData(V3DLONG xw, V3DLONG yw, V3DLONG zw, V3DLONG xw0, V3DLONG yw0, V3DLONG zw0);
  
  void generateKernel(KernelType t, int pp, int qq, int rr);
  bool valid() {return (!data1d || !data3d || totalLen<=0 || xwid<=0 || ywid<=0 || zwid<=0 || totalLen!=xwid*ywid*zwid) ? false : true; } //070514
};


#endif

