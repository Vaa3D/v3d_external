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

/*
** Tests:
** nrrdQuantize (to 8-bits), nrrdUnquantize
** nrrdArithBinaryOp (with nrrdBinaryOpSubtract)
*/

#define KERN_SIZE_MAX 10
#define PROBE_NUM 300

static void
errPrefix(char *dst,
          int typi, unsigned int supi, unsigned int prbi,
          unsigned int probePass, double dxi, double dyi, double dzi,
          unsigned int xi, unsigned int yi, unsigned int zi) {
  sprintf(dst, "%s[%s][%u] #%u pp %u: (%g,%g,%g)->(%u,%u,%u): ",
          "probeMulti",
          airEnumStr(nrrdType, typi), supi, prbi, probePass,
          dxi, dyi, dzi, xi, yi, zi);
  return;
}


int
main(int argc, const char **argv) {
  airArray *mop, *submop;
  char *err;

  int typi;
  unsigned int supi, probePass, cti /* context copy index */,
    pvlIdx[NRRD_TYPE_MAX+1], sx, sy, sz, subnum;
  size_t sizes[3] = {42,61,50} /* one of these must be even */,
    ii, nn;
  Nrrd *norigScl, *nucharScl, *nunquant, *nqdiff,
    *nconvScl[NRRD_TYPE_MAX+1];
  unsigned char *ucharScl;
  gageContext *gctx[2][KERN_SIZE_MAX+1];
  gagePerVolume *gpvl[2][NRRD_TYPE_MAX+1][KERN_SIZE_MAX+1];
  const double *vansScl[2][NRRD_TYPE_MAX+1][KERN_SIZE_MAX+1],
    *gansScl[2][NRRD_TYPE_MAX+1][KERN_SIZE_MAX+1],
    *hansScl[2][NRRD_TYPE_MAX+1][KERN_SIZE_MAX+1];
  double *origScl, omin, omax,  dsx, dsy, dsz,
    spcOrig[NRRD_SPACE_DIM_MAX] = {0.0, 0.0, 0.0},
    spcVec[3][NRRD_SPACE_DIM_MAX] = {
      {1.1, 0.0, 0.0},
      {0.0, 2.2, 0.0},
      {0.0, 0.0, 3.3}};

  mop = airMopNew();

#define NRRD_NEW(name, mop)                                     \
  (name) = nrrdNew();                                           \
  airMopAdd((mop), (name), (airMopper)nrrdNuke, airMopAlways)

  /* --------------------------------------------------------------- */
  /* Creating initial volume */
  NRRD_NEW(norigScl, mop);
  if (nrrdMaybeAlloc_nva(norigScl, nrrdTypeDouble, 3, sizes)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "trouble allocating:\n%s", err);
    airMopError(mop); return 1;
  }
  origScl = AIR_CAST(double *, norigScl->data);
  nn = nrrdElementNumber(norigScl);
  airSrandMT(42*42);
  for (ii=0; ii<nn/2; ii++) {
    airNormalRand(origScl + 2*ii + 0, origScl + 2*ii + 1);
  }
  /* learn real range */
  omin = omax = origScl[0];
  for (ii=1; ii<nn; ii++) {
    omin = AIR_MIN(omin, origScl[ii]);
    omax = AIR_MAX(omax, origScl[ii]);
  }
  ELL_3V_SET(spcOrig, 0.0, 0.0, 0.0);
  if (nrrdSpaceSet(norigScl, nrrdSpaceRightAnteriorSuperior)
      || nrrdSpaceOriginSet(norigScl, spcOrig)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "trouble setting space:\n%s", err);
    airMopError(mop); return 1;
  }
  nrrdAxisInfoSet_nva(norigScl, nrrdAxisInfoSpaceDirection, spcVec);
  dsx = AIR_CAST(double, sizes[0]);
  dsy = AIR_CAST(double, sizes[1]);
  dsz = AIR_CAST(double, sizes[2]);
  sx = AIR_CAST(unsigned int, sizes[0]);
  sy = AIR_CAST(unsigned int, sizes[1]);
  sz = AIR_CAST(unsigned int, sizes[2]);
  subnum = AIR_CAST(unsigned int, PROBE_NUM*0.9);


  /* --------------------------------------------------------------- */
  /* Quantizing to 8-bits and checking */
  submop = airMopNew();
  NRRD_NEW(nucharScl, mop);
  NRRD_NEW(nunquant, submop);
  NRRD_NEW(nqdiff, submop);
  if (nrrdQuantize(nucharScl, norigScl, NULL, 8)
      || nrrdUnquantize(nunquant, nucharScl, nrrdTypeDouble)
      || nrrdArithBinaryOp(nqdiff, nrrdBinaryOpSubtract,
                           norigScl, nunquant)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "trouble quantizing and back:\n%s", err);
    airMopError(submop); airMopError(mop); return 1;
  }
  if (!( nucharScl->oldMin == omin
         && nucharScl->oldMax == omax )) {
    fprintf(stderr, "quantization range [%g,%g] != real range [%g,%g]\n",
            nucharScl->oldMin, nucharScl->oldMax, omin, omax);
    airMopError(submop); airMopError(mop); return 1;
  }
  {
    double *qdiff, *unquant;
    /* empirically determined tolerance, which had to be increased in
       order to work under valgrind (!)- perhaps because of a
       difference in the use of 80-bit registers */
    double epsilon=0.50000000000004;
    qdiff = AIR_CAST(double *, nqdiff->data);
    unquant = AIR_CAST(double *, nunquant->data);
    for (ii=0; ii<nn; ii++) {
      double dd;
      /* with infinite precision, the max difference between original and
         quantized values should be exactly half the width (in value)
         of 1/256 of value range  ==> dd = 0.5 */
      dd = qdiff[ii]*256/(omax - omin);
      if (AIR_ABS(dd) > epsilon) {
        unsigned int ui;
        ui = AIR_CAST(unsigned int, ii);
        fprintf(stderr, "|orig[%u]=%.17g - unquant=%.17g|*256/%.17g "
                "= %.17g > %.17g!\n", ui, origScl[ii], unquant[ii],
                omax - omin, AIR_ABS(dd), epsilon);
        airMopError(submop); airMopError(mop); return 1;
      }
    }
  }
  airMopOkay(submop);
  ucharScl = AIR_CAST(unsigned char *, nucharScl->data);

  /* --------------------------------------------------------------- */
  /* Converting to all other types */
  for (typi=nrrdTypeUnknown+1; typi<nrrdTypeLast; typi++) {
    if (nrrdTypeBlock == typi) {
      nconvScl[typi] = NULL;
      continue;
    }
    NRRD_NEW(nconvScl[typi], mop);
    if (nrrdConvert(nconvScl[typi], nucharScl, typi)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "trouble converting:\n%s", err);
      airMopError(mop); return 1;
    }
  }
  for (supi=1; supi<=KERN_SIZE_MAX; supi++) {
    unsigned int pvii;
    int E;
    double kparm[1];
    gctx[0][supi] = gageContextNew();
    airMopAdd(mop, gctx[0][supi], (airMopper)gageContextNix, airMopAlways);
    gageParmSet(gctx[0][supi], gageParmRenormalize, AIR_FALSE);
    gageParmSet(gctx[0][supi], gageParmCheckIntegrals, AIR_TRUE);
    kparm[0] = supi;
    E = 0;
    if (!E) E |= gageKernelSet(gctx[0][supi], gageKernel00,
                               nrrdKernelBoxSupportDebug, kparm);
    pvii = 0;
    for (typi=nrrdTypeUnknown+1; typi<nrrdTypeLast; typi++) {
      if (nrrdTypeBlock == typi) {
        gpvl[0][typi][supi] = NULL;
        continue;
      }
      if (!E) E |= !(gpvl[0][typi][supi]
                     = gagePerVolumeNew(gctx[0][supi], nconvScl[typi],
                                        gageKindScl));
      if (!E) E |= gagePerVolumeAttach(gctx[0][supi], gpvl[0][typi][supi]);
      if (1 == supi) {
        /* first time through this typi loop; its the occasion to
           set the pvlIdx array, which records the index into the
           gageContext->pvl[] array for the per-type perVolume.
           Having to do this is a symptom of bad API design in gage */
        pvlIdx[typi] = pvii++;
      }
      if (!E) E |= gageQueryItemOn(gctx[0][supi], gpvl[0][typi][supi],
                                   gageSclValue);
      if (E) {
        break;
      }
    }
    if (!E) E |= gageUpdate(gctx[0][supi]);
    if (E) {
      airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
      fprintf(stderr, "trouble (supi=%u, %d/%s) set-up:\n%s\n",
              supi, typi, airEnumStr(nrrdType, typi), err);
      airMopError(mop); return 1;
    }
    if (gctx[0][supi]->radius != supi) {
      fprintf(stderr, "supi %u != gageContext->radius %u\n",
              supi, gctx[0][supi]->radius);
      airMopError(mop); return 1;
    }
    for (typi=nrrdTypeUnknown+1; typi<nrrdTypeLast; typi++) {
      if (nrrdTypeBlock == typi) {
        vansScl[0][typi][supi] = NULL;
        gansScl[0][typi][supi] = NULL;
        hansScl[0][typi][supi] = NULL;
        continue;
      }
      vansScl[0][typi][supi] =
        gageAnswerPointer(gctx[0][supi], gpvl[0][typi][supi], gageSclValue);
      gansScl[0][typi][supi] =
        gageAnswerPointer(gctx[0][supi], gpvl[0][typi][supi], gageSclGradVec);
      hansScl[0][typi][supi] =
        gageAnswerPointer(gctx[0][supi], gpvl[0][typi][supi], gageSclHessian);
    }
  }

  /* --------------------------------------------------------------- */
  /* Exercising gageContextCopy */
  for (supi=1; supi<=KERN_SIZE_MAX; supi++) {
    if (!(gctx[1][supi] = gageContextCopy(gctx[0][supi]))) {
      airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
      fprintf(stderr, "trouble copying gctx[%u]:\n%s\n",
              supi, err);
      airMopError(mop); return 1;
    }
    for (typi=nrrdTypeUnknown+1; typi<nrrdTypeLast; typi++) {
      if (nrrdTypeBlock == typi) {
        vansScl[1][typi][supi] = NULL;
        gansScl[1][typi][supi] = NULL;
        hansScl[1][typi][supi] = NULL;
        continue;
      }
      gpvl[1][typi][supi] = gctx[1][supi]->pvl[pvlIdx[typi]];
      vansScl[1][typi][supi] =
        gageAnswerPointer(gctx[1][supi], gpvl[1][typi][supi], gageSclValue);
      gansScl[1][typi][supi] =
        gageAnswerPointer(gctx[1][supi], gpvl[1][typi][supi], gageSclGradVec);
      hansScl[1][typi][supi] =
        gageAnswerPointer(gctx[1][supi], gpvl[1][typi][supi], gageSclHessian);
    }
  }

  /* --------------------------------------------------------------- */
  /* the two different probing passes are just to use two different
     sets of kernels (first nrrdKernelBoxSupportDebug on pass 0, then
     nrrdKernelCos4SupportDebug and its derivatives on pass 1).
     Because nrrdKernelBoxSupportDebug has already been set prior to
     the first pass, there is some some second-time only "if (1 ==
     probePass)" logic that confuses the clarity of this */
  for (probePass=0; probePass<=1; probePass++) {
    unsigned int prbi, lastjj=0, xi, yi, zi;
    double thet, xu, yu, zu, dxi, dyi, dzi,
      elapsed[2][KERN_SIZE_MAX+1], time0;
    char errpre[AIR_STRLEN_LARGE];

    if (1 == probePass) {
      /* switch to cos^4 kernel, turn on gradient and hessian */
      for (cti=0; cti<2; cti++) {
        for (supi=1; supi<=KERN_SIZE_MAX; supi++) {
          int E;
          double kparm[1];
          gageParmSet(gctx[cti][supi], gageParmRenormalize, AIR_FALSE);
          gageParmSet(gctx[cti][supi], gageParmCheckIntegrals, AIR_TRUE);
          kparm[0] = supi;
          E = 0;
          if (!E) E |= gageKernelSet(gctx[cti][supi], gageKernel00,
                                     nrrdKernelCos4SupportDebug, kparm);
          if (!E) E |= gageKernelSet(gctx[cti][supi], gageKernel11,
                                     nrrdKernelCos4SupportDebugD, kparm);
          if (!E) E |= gageKernelSet(gctx[cti][supi], gageKernel22,
                                     nrrdKernelCos4SupportDebugDD, kparm);
          for (typi=nrrdTypeUnknown+1; typi<nrrdTypeLast; typi++) {
            if (nrrdTypeBlock == typi) {
              continue;
            }
            if (!E) E |= gageQueryItemOn(gctx[cti][supi],
                                         gpvl[cti][typi][supi],
                                         gageSclGradVec);
            if (!E) E |= gageQueryItemOn(gctx[cti][supi],
                                         gpvl[cti][typi][supi],
                                         gageSclHessian);
            if (E) {
              break;
            }
          }
          if (!E) E |= gageUpdate(gctx[cti][supi]);
          if (E) {
            airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
            fprintf(stderr, "trouble (cti=%u, supi=%u, %d/%s) "
                    "set-up:\n%s\n", cti,
                    supi, typi, airEnumStr(nrrdType, typi), err);
            airMopError(mop); return 1;
          }
        }
      }
    }
    for (cti=0; cti<2; cti++) {
      for (supi=1; supi<=KERN_SIZE_MAX; supi++) {
        elapsed[cti][supi] = 0.0;
      }
    }
    /* do the probes along a curvy path */
    for (prbi=0; prbi<PROBE_NUM; prbi++) {
      unsigned int jj;
      jj = airIndex(0, prbi, PROBE_NUM-1, subnum);
      thet = AIR_AFFINE(0, jj, subnum-1, 0.0, AIR_PI);
      xu = -cos(5*thet);
      yu = -cos(3*thet);
      zu = -cos(thet);
      dxi = AIR_AFFINE(-1.0, xu, 1.0, -0.5, dsx-0.5);
      dyi = AIR_AFFINE(-1.0, yu, 1.0, -0.5, dsy-0.5);
      dzi = AIR_AFFINE(-1.0, zu, 1.0, -0.5, dsz-0.5);
      if (prbi && lastjj == jj) {
        /* this occasionally tests the logic in gage that seeks to
           re-compute convolution weights only when necessary */
        dxi += airSgn(xu);
        dyi += airSgn(yu);
        dzi += airSgn(zu);
      }
      xi = airIndexClamp(-0.5, dxi, dsx-0.5, sx);
      yi = airIndexClamp(-0.5, dyi, dsy-0.5, sy);
      zi = airIndexClamp(-0.5, dzi, dsz-0.5, sz);
      lastjj = jj;
      for (supi=1; supi<=KERN_SIZE_MAX; supi++) {
        double truevalOrig[NRRD_TYPE_MAX+1];
        for (cti=0; cti<2; cti++) {
          time0 = airTime();
          if (gageProbeSpace(gctx[cti][supi], dxi, dyi, dzi,
                             AIR_TRUE /* indexSpace */,
                             AIR_TRUE /* clamp */)) {
            fprintf(stderr, "probe (cti %u support %u) error (%d): %s\n",
                    cti, supi,
                    gctx[cti][supi]->errNum, gctx[cti][supi]->errStr);
            airMopError(mop); return 1;
          }
          elapsed[cti][supi] = airTime() - time0;
          for (typi=nrrdTypeUnknown+1; typi<nrrdTypeLast; typi++) {
            double arrayval, trueval, probeval;
            if (nrrdTypeBlock == typi
                || (1 == probePass && nrrdTypeChar == typi)) {
              /* can't easily correct interpolation on signed char
                 values to make it match interpolation on unsigned char
                 values, prior to wrap-around */
              continue;
            }
            /* probeval is what we learned by probing with gage */
            probeval = vansScl[cti][typi][supi][0];
            if (0 == probePass) {
              /* arrayval is the value directly from array of same type
                 (converted from original uchar) */
              arrayval = (nrrdDLookup[typi])(nconvScl[typi]->data,
                                            xi + sx*(yi + sy*zi));
              /* when using box, gage-reconstructed value should
                 match value from probing */
              if (arrayval != probeval) {
#define SPRINT_ERR_PREFIX                                           \
                errPrefix(errpre, typi, supi, prbi, probePass,      \
                          dxi, dyi, dzi, xi, yi, zi)
                SPRINT_ERR_PREFIX;
                fprintf(stderr, "%s: (cti %u) probed %g != conv %g\n", errpre,
                        cti, probeval, arrayval);
                airMopError(mop); return 1;
              }
              /* trueval on pass 0 is the original uchar value */
              trueval = AIR_CAST(double, ucharScl[xi + sx*(yi + sy*zi)]);
            } else {
              /* trueval on pass 1 is the value from probing uchar volume */
              trueval = vansScl[cti][nrrdTypeUChar][supi][0];
            }
            if (nrrdTypeChar == typi && trueval > 127) {
              /* recreate value wrapping of signed char */
              trueval -= 256;
            }
            if (0 == cti) {
              truevalOrig[typi] = trueval;
            } else {
              /* make sure that result from gageContextCopy'd context
                 (trueval) is same result as original (truevalOrig[typi]) */
              if (truevalOrig[typi] != trueval) {
                SPRINT_ERR_PREFIX;
                fprintf(stderr, "%s: original->%g, gageContextCopy->%g\n",
                        errpre, truevalOrig[typi], trueval);
                airMopError(mop); return 1;
              }
            }
            /* regardless of the volume (excepting where we've continue'd,
               above) the reconstructed value probeval should match trueval */
            if (trueval != probeval) {
              SPRINT_ERR_PREFIX;
              fprintf(stderr, "%s: (cti %u) probed %g != true %g\n", errpre,
                      cti, probeval, trueval);
              airMopError(mop); return 1;
            }
            if (1 == probePass) {
              /* and when we use a differentiable kernel, the gradient
                 and Hessian had better agree too */
              double diff3[3], diff9[9];
              ELL_3V_SUB(diff3,
                         gansScl[cti][nrrdTypeUChar][supi],
                         gansScl[cti][typi][supi]);
              if (ELL_3V_LEN(diff3) > 0.0) {
                SPRINT_ERR_PREFIX;
                fprintf(stderr, "%s: (cti %u) probed gradient error len %f\n",
                        errpre, cti, ELL_3V_LEN(diff3));
                airMopError(mop); return 1;
              }
              ELL_9V_SUB(diff9,
                         hansScl[cti][nrrdTypeUChar][supi],
                         hansScl[cti][typi][supi]);
              if (ELL_9V_LEN(diff9) > 0.0) {
                SPRINT_ERR_PREFIX;
                fprintf(stderr, "%s: (cti %u) probed hessian error len %f\n",
                        errpre, cti, ELL_9V_LEN(diff9));
                airMopError(mop); return 1;
              }
            }
          }
        }
      }
    }
    for (cti=0; cti<2; cti++) {
      for (supi=1; supi<=KERN_SIZE_MAX; supi++) {
        fprintf(stderr, "elapsed[%u][%u] = %g ms\n",
                cti, supi, 1000*elapsed[cti][supi]);
      }
    }
  }

#undef NRRD_NEW
#undef SPRINT_ERR_PREFIX
  airMopOkay(mop);
  exit(0);
}
