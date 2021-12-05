//wkernel.h: the program to define kernels
// by Hanchuan Peng
// 2006-2011

#ifndef __WEIGHT_KERNEL__
#define __WEIGHT_KERNEL__

#include "v3d_basicdatatype.h"

enum KernelType {
    KT_CUBE_ALL1, 
    KT_SPHERE_ALL1};

class WeightKernel3D
{
public:
  double * data1d;
  V3DLONG totalLen;
  double *** data3d;
  V3DLONG xwid, ywid, zwid, rwid; 
  V3DLONG x0, y0, z0; 
  WeightKernel3D(V3DLONG xw, V3DLONG yw, V3DLONG zw);
  WeightKernel3D(V3DLONG r);
  ~WeightKernel3D();

  void cleanData();
  void initData();
  int createInternalData(V3DLONG xw, V3DLONG yw, V3DLONG zw, V3DLONG xw0, V3DLONG yw0, V3DLONG zw0);
  
  void generateKernel(KernelType t, int pp, int qq, int rr);
  bool valid() {return (!data1d || !data3d || totalLen<=0 || xwid<=0 || ywid<=0 || zwid<=0 || totalLen!=xwid*ywid*zwid) ? false : true; }
};


#endif

