/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "gage.h"
#include "privateGage.h"

const int
gagePresent = 42;

/*
******** gageZeroNormal[]
**
** this is the vector to supply when someone wants the normalized
** version of a vector with zero length.  We could be nasty and
** set this to {AIR_NAN, AIR_NAN, AIR_NAN}, but simply passing
** NANs around can make things fantastically slow . . .
*/
double
gageZeroNormal[3] = {0,0,0};

const char *
_gageKernelStr[] = {
  "(unknown_kernel)",
  "00",
  "10",
  "11",
  "20",
  "21",
  "22",
  /* "33", */
  /* "44", */
  "stack"
};

const char *
_gageKernelDesc[] = {
  "unknown kernel",
  "kernel for reconstructing values",
  "kernel for reconstruction values when doing 1st derivatives",
  "kernel for measuring 1st derivatives when doing 1st derivatives",
  "kernel for reconstruction values when doing 2nd derivatives",
  "kernel for measuring 1st derivatives when doing 2nd derivatives",
  "kernel for measuring 2nd derivatives when doing 2nd derivatives",
  /* "kernel for measuring 3rd derivatives when doing 3rd derivatives", */
  /* "kernel for measuring 4th derivatives when doing 4th derivatives", */
  "kernel for reconstruction across a stack"
};

const char *
_gageKernelStrEqv[] = {
  "00", "k00",
  "10", "k10",
  "11", "k11",
  "20", "k20",
  "21", "k21",
  "22", "k22",
  /* "33", "k33", */
  /* "44", "k44", */
  "stack", "ss", "kss",
  ""
};

const int
_gageKernelValEqv[] = {
  gageKernel00, gageKernel00,
  gageKernel10, gageKernel10,
  gageKernel11, gageKernel11,
  gageKernel20, gageKernel20,
  gageKernel21, gageKernel21,
  gageKernel22, gageKernel22,
  /* gageKernel33, gageKernel33, */
  /* gageKernel44, gageKernel44, */
  gageKernelStack, gageKernelStack, gageKernelStack
};

const airEnum
_gageKernel_enum = {
  "kernel",
  GAGE_KERNEL_MAX,
  _gageKernelStr, NULL,
  _gageKernelDesc,
  _gageKernelStrEqv, _gageKernelValEqv,
  AIR_FALSE
};
const airEnum *const
gageKernel = &_gageKernel_enum;

void
gageParmReset(gageParm *parm) {

  if (parm) {
    parm->renormalize = gageDefRenormalize;
    parm->checkIntegrals = gageDefCheckIntegrals;
    parm->k3pack = gageDefK3Pack;
    parm->gradMagCurvMin = gageDefGradMagCurvMin;
    parm->curvNormalSide = gageDefCurvNormalSide;
    parm->kernelIntegralNearZero = gageDefKernelIntegralNearZero;
    parm->defaultCenter = gageDefDefaultCenter;
    parm->stackUse = gageDefStackUse;
    parm->stackNormalizeRecon = gageDefStackNormalizeRecon;
    parm->stackNormalizeDeriv = gageDefStackNormalizeDeriv;
    parm->stackNormalizeDerivBias = gageDefStackNormalizeDerivBias;
    parm->orientationFromSpacing = gageDefOrientationFromSpacing;
    parm->generateErrStr = gageDefGenerateErrStr;
  }
  return;
}

void
gagePointReset(gagePoint *point) {

  if (point) {
    unsigned int big;
    /* learned: can't initialize the floating point to AIR_NAN,
       non-dot-net windows compilers proclaim that QNAN == x
       for any existent x!!!  For some reason though, infinity
       is handled correctly */
    ELL_4V_SET(point->frac,
               AIR_POS_INF, AIR_POS_INF, AIR_POS_INF, AIR_POS_INF);
    big = AIR_CAST(unsigned int, -1);
    ELL_4V_SET(point->idx, big, big, big, big);
    point->stackFwNonZeroNum = 0;
  }
  return;
}

void
gageItemSpecInit(gageItemSpec *isp) {

  if (isp) {
    isp->item = -1;
    isp->kind = NULL;
  }
  return;
}

gageItemSpec *
gageItemSpecNew(void) {
  gageItemSpec *isp;

  isp = (gageItemSpec *)calloc(1, sizeof(gageItemSpec));
  gageItemSpecInit(isp);
  return isp;
}

gageItemSpec *
gageItemSpecNix(gageItemSpec *isp) {

  if (isp) {
    airFree(isp);
  }
  return NULL;
}

const char *
_gageErrStr[GAGE_ERR_MAX+1] = {
  "(unknown gageErr)",
  "none",
  "space bounds",
  "stack bounds",
  "stack integral",
  "stack search",
  "stack unused"
};

const airEnum
_gageErr = {
  "gageErr",
  GAGE_ERR_MAX,
  _gageErrStr, NULL,
  NULL,
  NULL, NULL,
  AIR_FALSE
};
const airEnum *const
gageErr = &_gageErr;

const char *
_gageItemPackPartStr[] = {
  "(unknown_pack_part)",
  "scalar",
  "gradvec",
  "gradmag",
  "normal",
  "hessian",
  "hesseval0",
  "hesseval1",
  "hesseval2",
  "hessevec0",
  "hessevec1",
  "hessevec2"
};

const char *
_gageItemPackPartDesc[] = {
  "unknown pack part",
  "the base scalar F",
  "gradient (vector) of F",
  "magnitude of the gradient of F",
  "normalized gradient (vector) of F",
  "Hessian of F",
  "1st eigenvalue of Hessian of F",
  "2nd eigenvalue of Hessian of F",
  "3rd eigenvalue of Hessian of F",
  "1st eigenvector of Hessian of F",
  "2nd eigenvector of Hessian of F",
  "3rd eigenvector of Hessian of F"
};

const char *
_gageItemPackPartStrEqv[] = {
  "scalar", "scl",
  "gradvec", "gvec",
  "gradmag", "gmag",
  "normal", "norm",
  "hessian", "hess",
  "hesseval0", "heval0",
  "hesseval1", "heval1",
  "hesseval2", "heval2",
  "hessevec0", "hevec0",
  "hessevec1", "hevec1",
  "hessevec2", "hevec2",
  ""
};

const int
_gageItemPackPartValEqv[] = {
  gageItemPackPartScalar,      gageItemPackPartScalar,
  gageItemPackPartGradVec,     gageItemPackPartGradVec,
  gageItemPackPartGradMag,     gageItemPackPartGradMag,
  gageItemPackPartNormal,      gageItemPackPartNormal,
  gageItemPackPartHessian,     gageItemPackPartHessian,
  gageItemPackPartHessEval0,   gageItemPackPartHessEval0,
  gageItemPackPartHessEval1,   gageItemPackPartHessEval1,
  gageItemPackPartHessEval2,   gageItemPackPartHessEval2,
  gageItemPackPartHessEvec0,   gageItemPackPartHessEvec0,
  gageItemPackPartHessEvec1,   gageItemPackPartHessEvec1,
  gageItemPackPartHessEvec2,   gageItemPackPartHessEvec2,
};

const airEnum
_gageItemPackPart_enum = {
  "pack part",
  GAGE_ITEM_PACK_PART_MAX,
  _gageItemPackPartStr, NULL,
  _gageItemPackPartDesc,
  _gageItemPackPartStrEqv, _gageItemPackPartValEqv,
  AIR_FALSE
};
const airEnum *const
gageItemPackPart = &_gageItemPackPart_enum;

