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


#include "ten.h"
#include "privateTen.h"

int
tenEvecRGB(Nrrd *nout, const Nrrd *nin,
           const tenEvecRGBParm *rgbp) {
  static const char me[]="tenEvecRGB";
  size_t size[NRRD_DIM_MAX];
  float (*lup)(const void *, size_t), (*ins)(void *, size_t, float);
  float ten[7], eval[3], evec[9], RGB[3];
  size_t II, NN;
  unsigned char *odataUC;
  unsigned short *odataUS;

  if (!(nout && nin)) {
    biffAddf(TEN, "%s: got NULL pointer (%p,%p)",
             me, AIR_CAST(void *, nout), AIR_CVOIDP(nin));
    return 1;
  }
  if (tenEvecRGBParmCheck(rgbp)) {
    biffAddf(TEN, "%s: RGB parm trouble", me);
    return 1;
  }
  if (!(2 <= nin->dim && 7 == nin->axis[0].size)) {
    char stmp[AIR_STRLEN_SMALL];
    biffAddf(TEN, "%s: need nin->dim >= 2 (not %u), axis[0].size == 7 "
             "(not %s)", me, nin->dim,
             airSprintSize_t(stmp, nin->axis[0].size));
    return 1;
  }

  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size);
  size[0] = rgbp->genAlpha ? 4 : 3;
  if (nrrdMaybeAlloc_nva(nout, (nrrdTypeDefault == rgbp->typeOut
                                ? nin->type
                                : rgbp->typeOut), nin->dim, size)) {
    biffMovef(TEN, NRRD, "%s: couldn't alloc output", me);
    return 1;
  }
  odataUC = AIR_CAST(unsigned char *, nout->data);
  odataUS = AIR_CAST(unsigned short *, nout->data);

  NN = nrrdElementNumber(nin)/7;
  lup = nrrdFLookup[nin->type];
  ins = nrrdFInsert[nout->type];
  for (II=0; II<NN; II++) {
    TEN_T_SET(ten, lup(nin->data, 0 + 7*II),
              lup(nin->data, 1 + 7*II), lup(nin->data, 2 + 7*II),
              lup(nin->data, 3 + 7*II), lup(nin->data, 4 + 7*II),
              lup(nin->data, 5 + 7*II), lup(nin->data, 6 + 7*II));
    tenEigensolve_f(eval, evec, ten);
    tenEvecRGBSingle_f(RGB, ten[0], eval, evec + 3*(rgbp->which), rgbp);
    switch (nout->type) {
    case nrrdTypeUChar:
      odataUC[0 + size[0]*II] = airIndexClamp(0.0, RGB[0], 1.0, 256);
      odataUC[1 + size[0]*II] = airIndexClamp(0.0, RGB[1], 1.0, 256);
      odataUC[2 + size[0]*II] = airIndexClamp(0.0, RGB[2], 1.0, 256);
      if (rgbp->genAlpha) {
        odataUC[3 + size[0]*II] = 255;
      }
      break;
    case nrrdTypeUShort:
      odataUS[0 + size[0]*II] = airIndexClamp(0.0, RGB[0], 1.0, 65536);
      odataUS[1 + size[0]*II] = airIndexClamp(0.0, RGB[1], 1.0, 65536);
      odataUS[2 + size[0]*II] = airIndexClamp(0.0, RGB[2], 1.0, 65536);
      if (rgbp->genAlpha) {
        odataUS[3 + size[0]*II] = 65535;
      }
      break;
    default:
      ins(nout->data, 0 + size[0]*II, RGB[0]);
      ins(nout->data, 1 + size[0]*II, RGB[1]);
      ins(nout->data, 2 + size[0]*II, RGB[2]);
      if (rgbp->genAlpha) {
        ins(nout->data, 3 + size[0]*II, 1.0);
      }
      break;
    }
  }
  if (nrrdAxisInfoCopy(nout, nin, NULL, (NRRD_AXIS_INFO_SIZE_BIT))) {
    biffMovef(TEN, NRRD, "%s: couldn't copy axis info", me);
    return 1;
  }
  nout->axis[0].kind = nrrdKind3Color;
  if (nrrdBasicInfoCopy(nout, nin,
                        NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)) {
    biffAddf(TEN, "%s:", me);
    return 1;
  }

  return 0;
}

#define SQR(i) ((i)*(i))

short
tenEvqSingle(float vec[3], float scl) {
  static const char me[]="tenEvqSingle";
  float tmp, L1;
  int mi, bins, base, vi, ui;
  short ret;

  ELL_3V_NORM_TT(vec, float, vec, tmp);
  L1 = AIR_ABS(vec[0]) + AIR_ABS(vec[1]) + AIR_ABS(vec[2]);
  ELL_3V_SCALE(vec, 1/L1, vec);
  scl = AIR_CLAMP(0.0f, scl, 1.0f);
  scl = AIR_CAST(float, pow(scl, 0.75));
  mi = airIndex(0.0, scl, 1.0, 6);
  if (mi) {
    switch (mi) {
    case 1: bins = 16; base = 1;                                 break;
    case 2: bins = 32; base = 1+SQR(16);                         break;
    case 3: bins = 48; base = 1+SQR(16)+SQR(32);                 break;
    case 4: bins = 64; base = 1+SQR(16)+SQR(32)+SQR(48);         break;
    case 5: bins = 80; base = 1+SQR(16)+SQR(32)+SQR(48)+SQR(64); break;
    default:
      fprintf(stderr, "%s: PANIC: mi = %d\n", me, mi);
      exit(0);
    }
    vi = airIndex(-1, vec[0]+vec[1], 1, bins);
    ui = airIndex(-1, vec[0]-vec[1], 1, bins);
    ret = vi*bins + ui + base;
  }
  else {
    ret = 0;
  }
  return ret;
}

int
tenEvqVolume(Nrrd *nout,
             const Nrrd *nin, int which, int aniso, int scaleByAniso) {
  static const char me[]="tenEvqVolume";
  int map[3];
  short *qdata;
  const float *tdata;
  float eval[3], evec[9], an;
  size_t N, I, sx, sy, sz;

  if (!(nout && nin)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(AIR_IN_CL(0, which, 2))) {
    biffAddf(TEN, "%s: eigenvector index %d not in range [0..2]", me, which);
    return 1;
  }
  if (scaleByAniso) {
    if (airEnumValCheck(tenAniso, aniso)) {
      biffAddf(TEN, "%s: anisotropy metric %d not valid", me, aniso);
      return 1;
    }
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
    biffAddf(TEN, "%s: didn't get a valid DT volume", me);
    return 1;
  }
  sx = nin->axis[1].size;
  sy = nin->axis[2].size;
  sz = nin->axis[3].size;
  if (nrrdMaybeAlloc_va(nout, nrrdTypeShort, 3,
                        sx, sy, sz)) {
    biffMovef(TEN, NRRD, "%s: can't allocate output", me);
    return 1;
  }
  N = sx*sy*sz;
  tdata = (float *)nin->data;
  qdata = (short *)nout->data;
  for (I=0; I<N; I++) {
    tenEigensolve_f(eval, evec, tdata);
    if (scaleByAniso) {
      an = tenAnisoEval_f(eval, aniso);
    } else {
      an = 1.0;
    }
    qdata[I] = tenEvqSingle(evec+ 3*which, an);
    tdata += 7;
  }
  ELL_3V_SET(map, 1, 2, 3);
  if (nrrdAxisInfoCopy(nout, nin, map, (NRRD_AXIS_INFO_SIZE_BIT
                                        | NRRD_AXIS_INFO_KIND_BIT) )) {
    biffMovef(TEN, NRRD, "%s: trouble", me);
    return 1;
  }
  if (nrrdBasicInfoCopy(nout, nin,
                        NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)) {
    biffAddf(TEN, "%s:", me);
    return 1;
  }

  return 0;
}

int
tenBMatrixCheck(const Nrrd *nbmat, int type, unsigned int minnum) {
  static const char me[]="tenBMatrixCheck";

  if (nrrdCheck(nbmat)) {
    biffMovef(TEN, NRRD, "%s: basic validity check failed", me);
    return 1;
  }
  if (!( 6 == nbmat->axis[0].size && 2 == nbmat->dim )) {
    char stmp[AIR_STRLEN_SMALL];
    biffAddf(TEN, "%s: need a 6xN 2-D array (not a %s x? %d-D array)", me,
             airSprintSize_t(stmp, nbmat->axis[0].size), nbmat->dim);
    return 1;
  }
  if (nrrdTypeDefault != type && type != nbmat->type) {
    biffAddf(TEN, "%s: requested type %s but got type %s", me,
             airEnumStr(nrrdType, type), airEnumStr(nrrdType, nbmat->type));
    return 1;
  }
  if (nrrdTypeBlock == nbmat->type) {
    biffAddf(TEN, "%s: sorry, can't use %s type", me,
             airEnumStr(nrrdType, nrrdTypeBlock));
    return 1;
  }
  if (!( minnum <= nbmat->axis[1].size )) {
    char stmp[AIR_STRLEN_SMALL];
    biffAddf(TEN, "%s: have only %s B-matrices, need at least %d", me,
             airSprintSize_t(stmp, nbmat->axis[1].size), minnum);
    return 1;
  }

  return 0;
}

/*
******** _tenFindValley
**
** This is not a general purpose function, and it will take some
** work to make it that way.
**
** the tweak argument implements a cheesy heuristic: threshold should be
** on low side of histogram valley, since stdev for background is much
** narrower then stdev for brain
*/
int
_tenFindValley(double *valP, const Nrrd *nhist, double tweak, int save) {
  static const char me[]="_tenFindValley";
  double gparm[NRRD_KERNEL_PARMS_NUM], dparm[NRRD_KERNEL_PARMS_NUM];
  Nrrd *ntmpA, *ntmpB, *nhistD, *nhistDD;
  float *hist, *histD, *histDD;
  airArray *mop;
  size_t bins, maxbb, bb;
  NrrdRange *range;

  /*
  tenEMBimodalParm *biparm;
  biparm = tenEMBimodalParmNew();
  tenEMBimodal(biparm, nhist);
  biparm = tenEMBimodalParmNix(biparm);
  */

  mop = airMopNew();
  airMopAdd(mop, ntmpA=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntmpB=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nhistD=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nhistDD=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);

  bins = nhist->axis[0].size;
  gparm[0] = bins/128;  /* wacky heuristic for gaussian stdev */
  gparm[1] = 3;        /* how many stdevs to cut-off at */
  dparm[0] = 1.0;      /* unit spacing */
  dparm[1] = 1.0;      /* B-Spline kernel */
  dparm[2] = 0.0;
  if (nrrdCheapMedian(ntmpA, nhist, AIR_TRUE, AIR_FALSE, 2, 1.0, 1024)
      || nrrdSimpleResample(ntmpB, ntmpA,
                            nrrdKernelGaussian, gparm, &bins, NULL)
      || nrrdSimpleResample(nhistD, ntmpB,
                            nrrdKernelBCCubicD, dparm, &bins, NULL)
      || nrrdSimpleResample(nhistDD, ntmpB,
                            nrrdKernelBCCubicDD, dparm, &bins, NULL)) {
    biffMovef(TEN, NRRD, "%s: trouble processing histogram", me);
    airMopError(mop); return 1;
  }
  if (save) {
    nrrdSave("tmp-histA.nrrd", ntmpA, NULL);
    nrrdSave("tmp-histB.nrrd", ntmpB, NULL);
  }
  hist = (float*)(ntmpB->data);
  histD = (float*)(nhistD->data);
  histDD = (float*)(nhistDD->data);
  range = nrrdRangeNewSet(ntmpB, nrrdBlind8BitRangeState);
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  for (bb=0; bb<bins-1; bb++) {
    if (hist[bb] == range->max) {
      /* first seek to max in histogram */
      break;
    }
  }
  maxbb = bb;
  for (; bb<bins-1; bb++) {
    if (histD[bb]*histD[bb+1] < 0 && histDD[bb] > 0) {
      /* zero-crossing in 1st deriv, positive 2nd deriv */
      break;
    }
  }
  if (bb == bins-1) {
    biffAddf(TEN, "%s: never saw a satisfactory zero crossing", me);
    airMopError(mop); return 1;
  }

  *valP = nrrdAxisInfoPos(nhist, 0, AIR_AFFINE(0, tweak, 1, maxbb, bb));

  airMopOkay(mop);
  return 0;
}

