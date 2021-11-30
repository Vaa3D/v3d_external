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

/*
** does not use biff
*/
gageStackBlurParm *
gageStackBlurParmNew() {
  gageStackBlurParm *parm;

  parm = AIR_CALLOC(1, gageStackBlurParm);
  if (parm) {
    parm->scale = NULL;
    parm->sigmaMax = gageDefStackBlurSigmaMax;
    parm->padValue = AIR_NAN;
    parm->kspec = NULL;
    parm->dataCheck = AIR_TRUE;
    parm->boundary = nrrdBoundaryUnknown;
    parm->renormalize = AIR_FALSE;
    parm->verbose = 0;
  }
  return parm;
}

gageStackBlurParm *
gageStackBlurParmNix(gageStackBlurParm *sbp) {

  if (sbp) {
    airFree(sbp->scale);
    nrrdKernelSpecNix(sbp->kspec);
    free(sbp);
  }
  return NULL;
}

int
gageStackBlurParmScaleSet(gageStackBlurParm *sbp, unsigned int num,
                          double scaleMin, double scaleMax,
                          int uniform, int optim) {
  static const char me[]="gageStackBlurParmScaleSet";
  unsigned int ii;

  if (!( sbp )) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  airFree(sbp->scale);
  sbp->scale = NULL;
  if (!( scaleMin < scaleMax )) {
    biffAddf(GAGE, "%s: scaleMin %g not < scaleMax %g", me,
             scaleMin, scaleMax);
    return 1;
  }
  sbp->scale = AIR_CALLOC(num, double);
  if (!sbp->scale) {
    biffAddf(GAGE, "%s: couldn't alloc scale for %u", me, num);
    return 1;
  }
  sbp->num = num;

  if (uniform) {
    for (ii=0; ii<num; ii++) {
      sbp->scale[ii] = AIR_AFFINE(0, ii, num-1, scaleMin, scaleMax);
    }
  } else if (!optim) {
    double tau0, tau1, tau;
    tau0 = gageTauOfSig(scaleMin);
    tau1 = gageTauOfSig(scaleMax);
    for (ii=0; ii<num; ii++) {
      tau = AIR_AFFINE(0, ii, num-1, tau0, tau1);
      sbp->scale[ii] = gageSigOfTau(tau);
    }
  } else {
    unsigned int sigmax;
    sigmax = AIR_CAST(unsigned int, scaleMax);
    if (!( 0 == scaleMin && sigmax == scaleMax )) {
      biffAddf(GAGE, "%s: range [%g,%g] not [0,N] w/ integral N", me,
               scaleMin, scaleMax);
      return 1;
    }
    if (gageOptimSigSet(sbp->scale, num, sigmax)) {
      biffAddf(GAGE, "%s: trouble w/ optimal sigmas", me);
      return 1;
    }
  }

  return 0;
}

int
gageStackBlurParmKernelSet(gageStackBlurParm *sbp,
                           const NrrdKernelSpec *kspec,
                           int renormalize) {
  static const char me[]="gageStackBlurParmKernelSet";

  if (!( sbp && kspec )) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  nrrdKernelSpecNix(sbp->kspec);
  sbp->kspec = nrrdKernelSpecCopy(kspec);
  sbp->renormalize = renormalize;
  return 0;
}

int
gageStackBlurParmBoundarySet(gageStackBlurParm *sbp,
                             int boundary, double padValue) {
  static const char me[]="gageStackBlurParmBoundarySet";

  if (!sbp) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(nrrdBoundary, boundary)) {
    biffAddf(GAGE, "%s: %d not a known %s", me,
             boundary, nrrdBoundary->name);
    return 1;
  }
  if (nrrdBoundaryPad == boundary && !AIR_EXISTS(padValue)) {
    biffAddf(GAGE, "%s: want boundary %s but padValue %g doesn't exist", me,
             airEnumStr(nrrdBoundary, boundary), padValue);
    return 1;
  }
  sbp->boundary = boundary;
  sbp->padValue = padValue;
  return 0;
}

int
gageStackBlurParmVerboseSet(gageStackBlurParm *sbp, int verbose) {
  static const char me[]="gageStackBlurParmVerboseSet";

  if (!sbp) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  sbp->verbose = verbose;
  return 0;
}

int
gageStackBlurParmCheck(gageStackBlurParm *sbp) {
  static const char me[]="gageStackBlurParmCheck";
  unsigned int ii;

  if (!sbp) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( sbp->scale && sbp->kspec )) {
    biffAddf(GAGE, "%s: scale and kernel aren't set", me);
    return 1;
  }
  if (!( sbp->num >= 2)) {
    biffAddf(GAGE, "%s: need num >= 2, not %u", me, sbp->num);
    return 1;
  }
  for (ii=0; ii<sbp->num; ii++) {
    if (!AIR_EXISTS(sbp->scale[ii])) {
      biffAddf(GAGE, "%s: scale[%u] = %g doesn't exist", me, ii,
               sbp->scale[ii]);
      return 1;
    }
    if (ii) {
      if (!( sbp->scale[ii-1] < sbp->scale[ii] )) {
        biffAddf(GAGE, "%s: scale[%u] = %g not < scale[%u] = %g", me,
                 ii, sbp->scale[ii-1], ii+1, sbp->scale[ii]);
        return 1;
      }
    }
  }
  if (airEnumValCheck(nrrdBoundary, sbp->boundary)) {
    biffAddf(GAGE, "%s: %d not a known %s", me,
             sbp->boundary, nrrdBoundary->name);
    return 1;
  }
  return 0;
}

static int
_checkNrrd(Nrrd *const nblur[], const Nrrd *const ncheck[],
           unsigned int blNum, int checking,
           const Nrrd *nin, const gageKind *kind) {
  static const char me[]="_checkNrrd";
  unsigned int blIdx;

  for (blIdx=0; blIdx<blNum; blIdx++) {
    if (checking) {
      if (nrrdCheck(ncheck[blIdx])) {
        biffMovef(GAGE, NRRD, "%s: bad ncheck[%u]", me, blIdx);
        return 1;
      }
    } else {
      if (!nblur[blIdx]) {
        biffAddf(GAGE, "%s: NULL blur[%u]", me, blIdx);
        return 1;
      }
    }
  }
  if (3 + kind->baseDim != nin->dim) {
    biffAddf(GAGE, "%s: need nin->dim %u (not %u) with baseDim %u", me,
             3 + kind->baseDim, nin->dim, kind->baseDim);
    return 1;
  }
  return 0;
}

#define KVP_NUM 5

static const char                      /*    0             1         2          3            4     */
_blurKey[KVP_NUM][AIR_STRLEN_LARGE] = {"gageStackBlur", "scale", "kernel", "boundary", "renormalize"};

typedef struct {
  char val[KVP_NUM][AIR_STRLEN_LARGE];
} blurVal_t;

static blurVal_t *
_blurValAlloc(airArray *mop, gageStackBlurParm *sbp) {
  static const char me[]="_blurValAlloc";
  blurVal_t *blurVal;
  unsigned int blIdx;

  blurVal = AIR_CAST(blurVal_t *, calloc(sbp->num, sizeof(blurVal_t)));
  if (!blurVal) {
    biffAddf(GAGE, "%s: couldn't alloc blurVal for %u", me, sbp->num);
    return NULL;
  }
  for (blIdx=0; blIdx<sbp->num; blIdx++) {
    sbp->kspec->parm[0] = sbp->scale[blIdx];
    sprintf(blurVal[blIdx].val[0], "true");
    sprintf(blurVal[blIdx].val[1], "%g", sbp->scale[blIdx]);
    nrrdKernelSpecSprint(blurVal[blIdx].val[2], sbp->kspec);
    sprintf(blurVal[blIdx].val[3], "%s", sbp->renormalize ? "true" : "false");
    sprintf(blurVal[blIdx].val[4], "%s",
            airEnumStr(nrrdBoundary, sbp->boundary));
  }
  airMopAdd(mop, blurVal, airFree, airMopAlways);
  return blurVal;
}

/*
** little helper function to do pre-blurring of a given nrrd
** of the sort that might be useful for scale-space gage use
**
** nblur has to already be allocated for "blNum" Nrrd*s, AND, they all
** have to point to valid (possibly empty) Nrrds, so they can hold the
** results of blurring
*/
int
gageStackBlur(Nrrd *const nblur[], gageStackBlurParm *sbp,
              const Nrrd *nin, const gageKind *kind) {
  static const char me[]="gageStackBlur";
  unsigned int blIdx, axi;
  NrrdResampleContext *rsmc;
  blurVal_t *blurVal;
  airArray *mop;
  int E;
  Nrrd *ntmp;

  if (!(nblur && sbp && nin && kind)) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  mop = airMopNew();
  if (gageStackBlurParmCheck(sbp)
      || _checkNrrd(nblur, NULL, sbp->num, AIR_FALSE, nin, kind)
      || (!( blurVal = _blurValAlloc(mop, sbp) )) ) {
    biffAddf(GAGE, "%s: problem", me);
    airMopError(mop); return 1;
  }
  rsmc = nrrdResampleContextNew();
  airMopAdd(mop, rsmc, (airMopper)nrrdResampleContextNix, airMopAlways);
  /* may or may not be needed for iterative diffusion */
  ntmp = nrrdNew();
  airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);

  E = 0;
  if (!E) E |= nrrdResampleDefaultCenterSet(rsmc, nrrdDefaultCenter);
  if (!E) E |= nrrdResampleInputSet(rsmc, nin);
  if (kind->baseDim) {
    unsigned int bai;
    for (bai=0; bai<kind->baseDim; bai++) {
      if (!E) E |= nrrdResampleKernelSet(rsmc, bai, NULL, NULL);
    }
  }
  for (axi=0; axi<3; axi++) {
    if (!E) E |= nrrdResampleSamplesSet(rsmc, kind->baseDim + axi,
                                        nin->axis[kind->baseDim + axi].size);
    if (!E) E |= nrrdResampleRangeFullSet(rsmc, kind->baseDim + axi);
  }
  if (!E) E |= nrrdResampleBoundarySet(rsmc, sbp->boundary);
  if (!E) E |= nrrdResampleTypeOutSet(rsmc, nrrdTypeDefault);
  if (!E) E |= nrrdResampleRenormalizeSet(rsmc, sbp->renormalize);
  if (E) {
    biffAddf(GAGE, "%s: trouble setting up resampling", me);
    airMopError(mop); return 1;
  }

  for (blIdx=0; blIdx<sbp->num; blIdx++) {
    unsigned int kvpIdx;
    if (sbp->verbose) {
      fprintf(stderr, "%s: blurring %u / %u (scale %g) ... ", me, blIdx,
              sbp->num, sbp->scale[blIdx]);
      fflush(stderr);
    }
    if (nrrdKernelDiscreteGaussian == sbp->kspec->kernel
        && sbp->scale[blIdx] > sbp->sigmaMax) {
      double timeLeft, /* amount of diffusion time left to do */
        timeStep,      /* length of diffusion time per blur */
        timeDo;        /* amount of diffusion time just applied */
      unsigned int passIdx = 0;
      timeLeft = sbp->scale[blIdx]*sbp->scale[blIdx];
      timeStep = (sbp->sigmaMax)*(sbp->sigmaMax);
      if (sbp->verbose) {
        fprintf(stderr, "\n");
        fprintf(stderr, "%s: scale %g > sigmaMax %g\n", me,
                sbp->scale[blIdx], sbp->sigmaMax);
        fprintf(stderr, "%s: diffusing for time %g in steps of %g\n", me,
                timeLeft, timeStep);
        fflush(stderr);
      }
      do {
        if (!passIdx) {
          if (!E) E |= nrrdResampleInputSet(rsmc, nin);
        } else {
          if (!E) E |= nrrdResampleInputSet(rsmc, ntmp);
        }
        timeDo = (timeLeft > timeStep
                  ? timeStep
                  : timeLeft);
        sbp->kspec->parm[0] = sqrt(timeDo);
        for (axi=0; axi<3; axi++) {
          if (!E) E |= nrrdResampleKernelSet(rsmc, kind->baseDim + axi,
                                             sbp->kspec->kernel,
                                             sbp->kspec->parm);
        }
        if (sbp->verbose) {
          fprintf(stderr, "  pass %u (timeLeft=%g => time=%g, sigma=%g) ...\n",
                  passIdx, timeLeft, timeDo, sbp->kspec->parm[0]);
        }
        if (!E) E |= nrrdResampleExecute(rsmc, ntmp);
        timeLeft -= timeDo;
        passIdx++;
      } while (!E && timeLeft > 0.0);
      if (!E) E |= nrrdCopy(nblur[blIdx], ntmp);
    } else {
      /* do blurring in one shot as usual */
      sbp->kspec->parm[0] = sbp->scale[blIdx];
      for (axi=0; axi<3; axi++) {
        if (!E) E |= nrrdResampleKernelSet(rsmc, kind->baseDim + axi,
                                           sbp->kspec->kernel,
                                           sbp->kspec->parm);
      }
      if (!E) E |= nrrdResampleExecute(rsmc, nblur[blIdx]);
    }
    if (E) {
      if (sbp->verbose) {
        fprintf(stderr, "problem!\n");
      }
      biffAddf(GAGE, "%s: trouble w/ %u of %u (scale %g)",
               me, blIdx, sbp->num, sbp->scale[blIdx]);
      airMopError(mop); return 1;
    }
    if (sbp->verbose) {
      fprintf(stderr, "done.\n");
    }
    /* add the KVPs to document how these were blurred */
    for (kvpIdx=0; kvpIdx<KVP_NUM; kvpIdx++) {
      if (!E) E |= nrrdKeyValueAdd(nblur[blIdx], _blurKey[kvpIdx],
                                   blurVal[blIdx].val[kvpIdx]);
    }
  }

  airMopOkay(mop);
  return 0;
}

/*
******** gageStackBlurCheck
**
** (docs)
**
** on why sbp->dataCheck should be non-zero: really need to check that
** the data values themselves are correct; its too dangerous to have
** this unchecked, because it means that experimental changes in
** volumes could mysteriously have no effect, because the cached
** pre-blurred volumes from the old data are being re-used
*/
int
gageStackBlurCheck(const Nrrd *const nblur[],
                   gageStackBlurParm *sbp,
                   const Nrrd *nin, const gageKind *kind) {
  static const char me[]="gageStackBlurCheck";
  gageShape *shapeOld, *shapeNew;
  blurVal_t *blurVal;
  airArray *mop;
  unsigned int blIdx, kvpIdx;

  if (!(nblur && sbp && nin && kind)) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  mop = airMopNew();
  if (gageStackBlurParmCheck(sbp)
      || _checkNrrd(NULL, nblur, sbp->num, AIR_TRUE, nin, kind)
      || (!( blurVal = _blurValAlloc(mop, sbp) )) ) {
    biffAddf(GAGE, "%s: problem", me);
    airMopError(mop); return 1;
  }

  shapeNew = gageShapeNew();
  airMopAdd(mop, shapeNew, (airMopper)gageShapeNix, airMopAlways);
  if (gageShapeSet(shapeNew, nin, kind->baseDim)) {
    biffAddf(GAGE, "%s: trouble setting up reference shape", me);
    airMopError(mop); return 1;
  }
  shapeOld = gageShapeNew();
  airMopAdd(mop, shapeOld, (airMopper)gageShapeNix, airMopAlways);

  for (blIdx=0; blIdx<sbp->num; blIdx++) {
    if (nin->type != nblur[blIdx]->type) {
      biffAddf(GAGE, "%s: nblur[%u]->type %s != nin type %s\n", me,
               blIdx, airEnumStr(nrrdType, nblur[blIdx]->type),
               airEnumStr(nrrdType, nin->type));
      airMopError(mop); return 1;
    }
    /* check to see if nblur[blIdx] is as expected */
    if (gageShapeSet(shapeOld, nblur[blIdx], kind->baseDim)
        || !gageShapeEqual(shapeOld, "nblur", shapeNew, "nin")) {
      biffAddf(GAGE, "%s: trouble, or nblur[%u] shape != nin shape",
               me, blIdx);
      airMopError(mop); return 1;
    }
    /* see if the KVPs are already there */
    for (kvpIdx=0; kvpIdx<KVP_NUM; kvpIdx++) {
      char *tmpval;
      tmpval = nrrdKeyValueGet(nblur[blIdx], _blurKey[kvpIdx]);
      if (!tmpval) {
        biffAddf(GAGE, "%s: didn't see key \"%s\" in nblur[%u]", me,
                 _blurKey[kvpIdx], blIdx);
        airMopError(mop); return 1;
      }
      airMopAdd(mop, tmpval, airFree, airMopAlways);
      if (strcmp(tmpval, blurVal[blIdx].val[kvpIdx])) {
        biffAddf(GAGE, "%s: found key[%s] \"%s\" != wanted \"%s\"", me,
                 _blurKey[kvpIdx], tmpval, blurVal[blIdx].val[kvpIdx]);
        airMopError(mop); return 1;
      }
    }
  }
  if (sbp->dataCheck) {
    double (*lup)(const void *, size_t), vin, vbl;
    size_t II, NN;
    if (!(0.0 == sbp->scale[0])) {
      biffAddf(GAGE, "%s: sorry, dataCheck w/ scale[0] %g "
               "!= 0.0 not implemented", me, sbp->scale[0]);
      airMopError(mop); return 1;
      /* so the non-zero return here will be acted upon as though there
         was a difference between the desired and the current stack,
         so things will be recomputed, which is conservative but costly */
    }
    lup = nrrdDLookup[nin->type];
    NN = nrrdElementNumber(nin);
    for (II=0; II<NN; II++) {
      vin = lup(nin->data, II);
      vbl = lup(nblur[0]->data, II);
      if (vin != vbl) {
        biffAddf(GAGE, "%s: value[%u] in nin %g != in nblur[0] %g\n", me,
                 AIR_CAST(unsigned int, II), vin, vbl);
        airMopError(mop); return 1;
      }
    }
  }

  airMopOkay(mop);
  return 0;
}

int
gageStackBlurGet(Nrrd *const nblur[], int *recomputedP,
                 gageStackBlurParm *sbp,
                 const char *format,
                 const Nrrd *nin, const gageKind *kind) {
  static const char me[]="gageStackBlurGet";
  airArray *mop;
  int recompute;
  unsigned int ii;

  if (!( nblur && sbp && nin && kind )) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  for (ii=0; ii<sbp->num; ii++) {
    if (!nblur[ii]) {
      biffAddf(GAGE, "%s: nblur[%u] NULL", me, ii);
      return 1;
    }
  }
  if (gageStackBlurParmCheck(sbp)) {
    biffAddf(GAGE, "%s: trouble with blur parms", me);
    return 1;
  }
  mop = airMopNew();

  /* set recompute flag */
  if (!airStrlen(format)) {
    /* no info about files to load, obviously have to recompute */
    if (sbp->verbose) {
      fprintf(stderr, "%s: no file info, must recompute blurrings\n", me);
    }
    recompute = AIR_TRUE;
  } else {
    char *fname, *suberr;
    int firstExists;
    FILE *file;
    /* do have info about files to load, but may fail in many ways */
    fname = AIR_CALLOC(strlen(format) + AIR_STRLEN_SMALL, char);
    if (!fname) {
      biffAddf(GAGE, "%s: couldn't allocate fname", me);
      airMopError(mop); return 1;
    }
    airMopAdd(mop, fname, airFree, airMopAlways);
    /* see if we can get the first file (number 0) */
    sprintf(fname, format, 0);
    firstExists = !!(file = fopen(fname, "r"));
    airFclose(file);
    if (!firstExists) {
      if (sbp->verbose) {
        fprintf(stderr, "%s: no file \"%s\"; will recompute blurrings\n",
                me, fname);
      }
      recompute = AIR_TRUE;
    } else if (nrrdLoadMulti(nblur, sbp->num, format, 0, NULL)) {
      airMopAdd(mop, suberr = biffGetDone(NRRD), airFree, airMopAlways);
      if (sbp->verbose) {
        fprintf(stderr, "%s: will recompute blurrings that couldn't be "
                "read:\n%s\n", me, suberr);
      }
      recompute = AIR_TRUE;
    } else if (gageStackBlurCheck(AIR_CAST(const Nrrd*const*, nblur),
                                  sbp, nin, kind)) {
      airMopAdd(mop, suberr = biffGetDone(GAGE), airFree, airMopAlways);
      if (sbp->verbose) {
        fprintf(stderr, "%s: will recompute blurrings (from \"%s\") "
                "that don't match:\n%s\n", me, format, suberr);
      }
      recompute = AIR_TRUE;
    } else {
      /* else precomputed blurrings could all be read, and did match */
      if (sbp->verbose) {
        fprintf(stderr, "%s: will reuse %u %s pre-blurrings.\n", me,
                sbp->num, format);
      }
      recompute = AIR_FALSE;
    }
  }
  if (recompute) {
    if (gageStackBlur(nblur, sbp, nin, kind)) {
      biffAddf(GAGE, "%s: trouble computing blurrings", me);
      airMopError(mop); return 1;
    }
  }
  if (recomputedP) {
    *recomputedP = recompute;
  }

  airMopOkay(mop);
  return 0;
}

/*
******** gageStackBlurManage
**
** does the work of gageStackBlurGet and then some:
** allocates the array of Nrrds, allocates an array of doubles for scale,
** and saves output if recomputed
*/
int
gageStackBlurManage(Nrrd ***nblurP, int *recomputedP,
                    gageStackBlurParm *sbp,
                    const char *format,
                    int saveIfComputed, NrrdEncoding *enc,
                    const Nrrd *nin, const gageKind *kind) {
  static const char me[]="gageStackBlurManage";
  Nrrd **nblur;
  unsigned int ii;
  airArray *mop;
  int recomputed;

  if (!( nblurP && sbp && nin && kind )) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  nblur = *nblurP = AIR_CALLOC(sbp->num, Nrrd *);
  if (!nblur) {
    biffAddf(GAGE, "%s: couldn't alloc %u Nrrd*s", me, sbp->num);
    return 1;
  }

  mop = airMopNew();
  airMopAdd(mop, nblurP, (airMopper)airSetNull, airMopOnError);
  airMopAdd(mop, *nblurP, airFree, airMopOnError);
  for (ii=0; ii<sbp->num; ii++) {
    nblur[ii] = nrrdNew();
    airMopAdd(mop, nblur[ii], (airMopper)nrrdNuke, airMopOnError);
  }
  if (gageStackBlurGet(nblur, &recomputed, sbp, format, nin, kind)) {
    biffAddf(GAGE, "%s: trouble getting nblur", me);
    airMopError(mop); return 1;
  }
  if (recomputedP) {
    *recomputedP = recomputed;
  }
  if (recomputed && format && saveIfComputed) {
    NrrdIoState *nio;
    int E;
    E = 0;
    if (enc) {
      if (!enc->available()) {
        biffAddf(GAGE, "%s: requested %s encoding which is not "
                 "available in this build", me, enc->name);
        airMopError(mop); return 1;
      }
      nio = nrrdIoStateNew();
      airMopAdd(mop, nio, (airMopper)nrrdIoStateNix, airMopAlways);
      if (!E) E |= nrrdIoStateEncodingSet(nio, nrrdEncodingGzip);
    } else {
      nio = NULL;
    }
    if (!E) E |= nrrdSaveMulti(format, AIR_CAST(const Nrrd *const *,
                                                nblur),
                               sbp->num, 0, nio);
    if (E) {
      biffMovef(GAGE, NRRD, "%s: trouble saving blurrings", me);
      airMopError(mop); return 1;
    }
  }

  airMopOkay(mop);
  return 0;
}
