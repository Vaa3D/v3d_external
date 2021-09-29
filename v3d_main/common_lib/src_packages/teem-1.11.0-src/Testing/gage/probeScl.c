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

#include "teem/gage.h"
#include <testDataPath.h>

/*
** Tests:
**
*/

#define INTERP_KERN_NUM 4
#define BLUR_KERN_NUM 5

int
main(int argc, const char **argv) {
  const char *me;
  Nrrd *nscl;
  double *dscl;
  airArray *mop;
  char *fullname;
  gageContext *igctx[INTERP_KERN_NUM], *bgctx[BLUR_KERN_NUM];
  const NrrdKernel *ikern[INTERP_KERN_NUM] = {
    nrrdKernelBox,
    nrrdKernelTent,
    nrrdKernelBCCubic,
    nrrdKernelCatmullRom,
  };
  double ikparm[INTERP_KERN_NUM][NRRD_KERNEL_PARMS_NUM] = {
    {1.0},
    {1.0},
    {1.0, 0.0, 0.5},
    {AIR_NAN},
  };
  const NrrdKernel *bkern[BLUR_KERN_NUM] = {
    nrrdKernelTent,
    nrrdKernelBSpline3,
    nrrdKernelBSpline5,
    nrrdKernelBCCubic,
    nrrdKernelGaussian,
  };
  const NrrdKernel *bkernD[BLUR_KERN_NUM] = {
    nrrdKernelForwDiff,
    nrrdKernelBSpline3D,
    nrrdKernelBSpline5D,
    nrrdKernelBCCubicD,
    nrrdKernelGaussianD,
  };
  const NrrdKernel *bkernDD[BLUR_KERN_NUM] = {
    nrrdKernelZero,
    nrrdKernelBSpline3DD,
    nrrdKernelBSpline5DD,
    nrrdKernelBCCubicDD,
    nrrdKernelGaussianDD,
  };
  double bkparm[BLUR_KERN_NUM][NRRD_KERNEL_PARMS_NUM] = {
    {1.0},
    {AIR_NAN},
    {AIR_NAN},
    {2.0, 1.0, 0.0},
    {1.2, 5.0},
  };
  const double *ivalAns[INTERP_KERN_NUM], *bvalAns[BLUR_KERN_NUM],
    *bgrdAns[BLUR_KERN_NUM], *bhesAns[BLUR_KERN_NUM];
  int E;
  unsigned int sx, sy, sz, ki;

  AIR_UNUSED(argc);
  me = argv[0];
  mop = airMopNew();

  nscl = nrrdNew();
  airMopAdd(mop, nscl, (airMopper)nrrdNuke, airMopAlways);
  fullname = testDataPathPrefix("fmob-c4h.nrrd");
  airMopAdd(mop, fullname, airFree, airMopAlways);
  if (nrrdLoad(nscl, fullname, NULL)) {
    char *err;
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble reading data \"%s\":\n%s",
            me, fullname, err);
    airMopError(mop); return 1;
  }
  /* make sure its a double-type volume (assumed below) */
  if (nrrdTypeDouble != nscl->type) {
    fprintf(stderr, "%s: volume type %s != expected type %s\n", me,
            airEnumStr(nrrdType, nscl->type),
            airEnumStr(nrrdType, nrrdTypeDouble));
    airMopError(mop); return 1;
  }
  dscl = AIR_CAST(double *, nscl->data);
  sx = AIR_CAST(unsigned int, nscl->axis[0].size);
  sy = AIR_CAST(unsigned int, nscl->axis[1].size);
  sz = AIR_CAST(unsigned int, nscl->axis[2].size);

  for (ki=0; ki<INTERP_KERN_NUM; ki++) {
    gagePerVolume *gpvl;
    igctx[ki] = gageContextNew();
    airMopAdd(mop, igctx[ki], (airMopper)gageContextNix, airMopAlways);
    gageParmSet(igctx[ki], gageParmRenormalize, AIR_FALSE);
    gageParmSet(igctx[ki], gageParmCheckIntegrals, AIR_TRUE);
    gageParmSet(igctx[ki], gageParmOrientationFromSpacing, AIR_FALSE);
    E = 0;
    if (!E) E |= !(gpvl = gagePerVolumeNew(igctx[ki], nscl, gageKindScl));
    if (!E) E |= gageKernelSet(igctx[ki], gageKernel00,
                               ikern[ki], ikparm[ki]);
    if (!E) E |= gagePerVolumeAttach(igctx[ki], gpvl);
    if (!E) E |= gageQueryItemOn(igctx[ki], gpvl, gageSclValue);
    if (!E) E |= gageUpdate(igctx[ki]);
    if (E) {
      char *err;
      airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble %s set-up:\n%s\n", me,
              ikern[ki]->name, err);
      airMopError(mop); return 1;
    }
    ivalAns[ki] = gageAnswerPointer(igctx[ki], gpvl, gageSclValue);
  }

  /* traverse all samples of volume, probing with the interpolating
     kernels, make sure we recover the original values */
  {
    unsigned int xi, yi, zi;
    double pval[INTERP_KERN_NUM], err, rval;
    int pret;
    for (zi=0; zi<sz; zi++) {
      for (yi=0; yi<sy; yi++) {
        for (xi=0; xi<sx; xi++) {
          rval = dscl[xi + sx*(yi + sy*zi)];
          for (ki=0; ki<INTERP_KERN_NUM; ki++) {
            pret = gageProbeSpace(igctx[ki], xi, yi, zi,
                                  AIR_TRUE /* indexSpace */,
                                  AIR_FALSE /* clamp */);
            if (pret) {
              fprintf(stderr, "%s: %s probe error(%d): %s\n", me,
                      ikern[ki]->name, igctx[ki]->errNum, igctx[ki]->errStr);

              airMopError(mop); return 1;
            }
            pval[ki] = *ivalAns[ki];
            err = AIR_ABS(rval - pval[ki]);
            if (err) {
              fprintf(stderr, "%s: interp's [%u,%u,%u] %s probe %f "
                      "!= true %f (err %f)\n", me, xi, yi, zi,
                      ikern[ki]->name, pval[ki], rval, err);
              airMopError(mop); return 1;
            }
          }
        }
      }
    }
  }

  /* set up contexts for non-interpolating (blurring) kernels,
     and their first and second derivatives */
  for (ki=0; ki<BLUR_KERN_NUM; ki++) {
    gagePerVolume *gpvl;
    bgctx[ki] = gageContextNew();
    airMopAdd(mop, bgctx[ki], (airMopper)gageContextNix, airMopAlways);
    gageParmSet(bgctx[ki], gageParmRenormalize, AIR_TRUE);
    gageParmSet(bgctx[ki], gageParmCheckIntegrals, AIR_TRUE);
    gageParmSet(bgctx[ki], gageParmOrientationFromSpacing, AIR_FALSE);
    E = 0;
    if (!E) E |= !(gpvl = gagePerVolumeNew(bgctx[ki], nscl, gageKindScl));
    if (!E) E |= gageKernelSet(bgctx[ki], gageKernel00,
                               bkern[ki], bkparm[ki]);
    if (!E) E |= gageKernelSet(bgctx[ki], gageKernel11,
                               bkernD[ki], bkparm[ki]);
    if (!E) E |= gageKernelSet(bgctx[ki], gageKernel22,
                               bkernDD[ki], bkparm[ki]);
    if (!E) E |= gagePerVolumeAttach(bgctx[ki], gpvl);
    if (!E) E |= gageQueryItemOn(bgctx[ki], gpvl, gageSclValue);
    if (!E) E |= gageQueryItemOn(bgctx[ki], gpvl, gageSclGradVec);
    if (!E) E |= gageQueryItemOn(bgctx[ki], gpvl, gageSclHessian);
    if (!E) E |= gageUpdate(bgctx[ki]);
    if (E) {
      char *err;
      airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble %s set-up:\n%s\n", me,
              bkern[ki]->name, err);
      airMopError(mop); return 1;
    }
    fprintf(stderr, "%s radius = %u\n", bkern[ki]->name, bgctx[ki]->radius);
    bvalAns[ki] = gageAnswerPointer(bgctx[ki], gpvl, gageSclValue);
    bgrdAns[ki] = gageAnswerPointer(bgctx[ki], gpvl, gageSclGradVec);
    bhesAns[ki] = gageAnswerPointer(bgctx[ki], gpvl, gageSclHessian);
  }

  {
#define POS_NUM 12
    double xp[POS_NUM], yp[POS_NUM], zp[POS_NUM],
      pos[POS_NUM*POS_NUM*POS_NUM][3], *prbd,
      offs[POS_NUM/2] = {0, 1.22222, 2.444444, 3.777777, 5.88888, 7.55555};
    Nrrd *nprbd, *ncorr;
    unsigned int ii, jj, kk, qlen = 1 + 3 + 9;
    char *corrfn, explain[AIR_STRLEN_LARGE];
    int pret, differ;

    corrfn = testDataPathPrefix("test/probeSclAns.nrrd");
    airMopAdd(mop, corrfn, airFree, airMopAlways);
    ncorr = nrrdNew();
    airMopAdd(mop, ncorr, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdLoad(ncorr, corrfn, NULL)) {
      char *err;
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble reading data \"%s\":\n%s",
              me, corrfn, err);
      airMopError(mop); return 1;
    }
    for (ii=0; ii<POS_NUM/2; ii++) {
      xp[ii] = yp[ii] = zp[ii] = offs[ii];
      xp[POS_NUM-1-ii] = AIR_CAST(double, sx)-1.0-offs[ii];
      yp[POS_NUM-1-ii] = AIR_CAST(double, sy)-1.0-offs[ii];
      zp[POS_NUM-1-ii] = AIR_CAST(double, sz)-1.0-offs[ii];
    }
    for (kk=0; kk<POS_NUM; kk++) {
      for (jj=0; jj<POS_NUM; jj++) {
        for (ii=0; ii<POS_NUM; ii++) {
          ELL_3V_SET(pos[ii + POS_NUM*(jj + POS_NUM*kk)],
                     xp[ii], yp[jj], zp[kk]);
        }
      }
    }
    nprbd = nrrdNew();
    airMopAdd(mop, nprbd, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdMaybeAlloc_va(nprbd, nrrdTypeDouble, 3,
                          AIR_CAST(size_t, qlen),
                          AIR_CAST(size_t, BLUR_KERN_NUM),
                          AIR_CAST(size_t, POS_NUM*POS_NUM*POS_NUM))) {
      char *err;
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble setting up prbd:\n%s", me, err);
      airMopError(mop); return 1;
    }
    prbd = AIR_CAST(double *, nprbd->data);
    for (ii=0; ii<POS_NUM*POS_NUM*POS_NUM; ii++) {
      for (ki=0; ki<BLUR_KERN_NUM; ki++) {
        pret = gageProbeSpace(bgctx[ki], pos[ii][0], pos[ii][1], pos[ii][2],
                              AIR_TRUE /* indexSpace */,
                              AIR_FALSE /* clamp */);
        if (pret) {
          fprintf(stderr, "%s: %s probe error(%d): %s\n", me,
                  bkern[ki]->name, bgctx[ki]->errNum, bgctx[ki]->errStr);
          airMopError(mop); return 1;
        }
        prbd[0 + qlen*(ki + BLUR_KERN_NUM*(ii))] = bvalAns[ki][0];
        ELL_3V_COPY(prbd + 1 + qlen*(ki + BLUR_KERN_NUM*(ii)), bgrdAns[ki]);
        ELL_9V_COPY(prbd + 4 + qlen*(ki + BLUR_KERN_NUM*(ii)), bhesAns[ki]);
      }
    }
    /* HEY: weirdly, so far its only on Windows (and more than 10 times worse
       on Cygwin) this epsilon needs to be larger than zero, and only for the
       radius 6 Gaussian? */
    if (nrrdCompare(ncorr, nprbd, AIR_FALSE /* onlyData */,
                    8.0e-14 /* epsilon */, &differ, explain)) {
      char *err;
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble comparing:\n%s", me, err);
      airMopError(mop); return 1;
    }
    if (differ) {
      fprintf(stderr, "%s: probed values not correct: %s\n", me, explain);
      airMopError(mop); return 1;
    } else {
      fprintf(stderr, "all good\n");
    }
  }

  airMopOkay(mop);
  return 0;
}
