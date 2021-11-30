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


#include <stdio.h>

#include <teem/biff.h>
#include <teem/hest.h>
#include <teem/nrrd.h>
#include <teem/gage.h>
#include <teem/ten.h>
#include <teem/meet.h>

static const char *probeInfo =
  ("Shows off the functionality of the gage library. "
   "Uses gageProbe() to query various kinds of volumes "
   "to learn various measured or derived quantities. "
   "Can set environment variable TEEM_VPROBE_HACK_ZI "
   "to limit probing to a single z slice.");

int
main(int argc, const char *argv[]) {
  gageKind *kind;
  const char *me;
  char *whatS, *err, *outS, *stackReadFormat, *stackSaveFormat;
  hestParm *hparm;
  hestOpt *hopt = NULL;
  NrrdKernelSpec *k00, *k11, *k22, *kSS, *kSSblur;
  int what, E=0, renorm, SSuniform, SSoptim, verbose,
    orientationFromSpacing, SSnormd;
  unsigned int iBaseDim, oBaseDim, axi, numSS, ninSSIdx, seed;
  const double *answer;
  Nrrd *nin, *nout, **ninSS=NULL;
  Nrrd *ngrad=NULL, *nbmat=NULL;
  size_t ai, ansLen, idx, xi, yi, zi, six, siy, siz, sox, soy, soz;
  double bval=0, gmc, rangeSS[2], wrlSS, idxSS=AIR_NAN,
    dsix, dsiy, dsiz, dsox, dsoy, dsoz;
  gageContext *ctx;
  gagePerVolume *pvl=NULL;
  double t0, t1, x, y, z, scale[3], rscl[3], min[3], maxOut[3], maxIn[3];
  airArray *mop;
  unsigned int hackZi, *skip, skipNum;
  double (*ins)(void *v, size_t I, double d);
  gageStackBlurParm *sbp;

  char hackKeyStr[]="TEEM_VPROBE_HACK_ZI", *hackValStr;
  int otype, hackSet;
  char stmp[4][AIR_STRLEN_SMALL];

  me = argv[0];
  /* parse environment variables first, in case they break nrrdDefault*
     or nrrdState* variables in a way that nrrdSanity() should see */
  nrrdDefaultGetenv();
  nrrdStateGetenv();
  /* no harm done in making sure we're sane */
  if (!nrrdSanity()) {
    fprintf(stderr, "******************************************\n");
    fprintf(stderr, "******************************************\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  %s: nrrd sanity check FAILED.\n", me);
    fprintf(stderr, "\n");
    fprintf(stderr, "  This means that either nrrd can't work on this "
            "platform, or (more likely)\n");
    fprintf(stderr, "  there was an error in the compilation options "
            "and variable definitions\n");
    fprintf(stderr, "  for how Teem was built here.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  %s\n", err = biffGetDone(NRRD));
    fprintf(stderr, "\n");
    fprintf(stderr, "******************************************\n");
    fprintf(stderr, "******************************************\n");
    free(err);
    return 1;
  }

  mop = airMopNew();
  hparm = hestParmNew();
  airMopAdd(mop, hparm, AIR_CAST(airMopper, hestParmFree), airMopAlways);
  hparm->elideSingleOtherType = AIR_TRUE;
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "k", "kind", airTypeOther, 1, 1, &kind, NULL,
             "\"kind\" of volume (\"scalar\", \"vector\", "
             "\"tensor\", or \"dwi\")",
             NULL, NULL, meetHestGageKind);
  hestOptAdd(&hopt, "v", "verbosity", airTypeInt, 1, 1, &verbose, "1",
             "verbosity level");
  hestOptAdd(&hopt, "q", "query", airTypeString, 1, 1, &whatS, NULL,
             "the quantity (scalar, vector, or matrix) to learn by probing");
  hestOptAdd(&hopt, "s", "sclX sclY sxlZ", airTypeDouble, 3, 3, scale,
             "1.0 1.0 1.0",
             "scaling factor for resampling on each axis "
             "(>1.0 : supersampling)");
  hestOptAdd(&hopt, "k00", "kern00", airTypeOther, 1, 1, &k00,
             "tent", "kernel for gageKernel00",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k11", "kern11", airTypeOther, 1, 1, &k11,
             "cubicd:1,0", "kernel for gageKernel11",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k22", "kern22", airTypeOther, 1, 1, &k22,
             "cubicdd:1,0", "kernel for gageKernel22",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "seed", "N", airTypeUInt, 1, 1, &seed, "42",
             "RNG seed; mostly for debugging");

  hestOptAdd(&hopt, "ssn", "SS #", airTypeUInt, 1, 1, &numSS,
             "0", "how many scale-space samples to evaluate, or, "
             "0 to turn-off all scale-space behavior");
  hestOptAdd(&hopt, "ssr", "scale range", airTypeDouble, 2, 2, rangeSS,
             "nan nan", "range of scales in scale-space");
  hestOptAdd(&hopt, "ssrf", "SS read format", airTypeString, 1, 1,
             &stackReadFormat, "",
             "printf-style format (including a \"%u\") for the "
             "filenames from which to *read* "
             "pre-blurred volumes computed for the stack");
  hestOptAdd(&hopt, "sssf", "SS save format", airTypeString, 1, 1,
             &stackSaveFormat, "",
             "printf-style format (including a \"%u\") for the "
             "filenames in which to *save* "
             "pre-blurred volumes computed for the stack");
  hestOptAdd(&hopt, "ssw", "SS pos", airTypeDouble, 1, 1, &wrlSS, "0",
             "\"world\"-space position (true sigma) "
             "at which to sample in scale-space");
  hestOptAdd(&hopt, "kssb", "kernel", airTypeOther, 1, 1, &kSSblur,
             "dgauss:1,5", "blurring kernel, to sample scale space",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "kssr", "kernel", airTypeOther, 1, 1, &kSS,
             "hermite", "kernel for reconstructing from scale space samples",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "ssu", NULL, airTypeInt, 0, 0, &SSuniform, NULL,
             "do uniform samples along sigma, and not (by default) "
             "samples according to the effective diffusion scale");
  hestOptAdd(&hopt, "sso", NULL, airTypeInt, 0, 0, &SSoptim, NULL,
             "if not using \"-ssu\", use pre-computed optimal "
             "sigmas when possible");
  hestOptAdd(&hopt, "ssnd", NULL, airTypeInt, 0, 0, &SSnormd, NULL,
             "normalize derivatives by scale");

  hestOptAdd(&hopt, "rn", NULL, airTypeInt, 0, 0, &renorm, NULL,
             "renormalize kernel weights at each new sample location. "
             "\"Accurate\" kernels don't need this; doing it always "
             "makes things go slower");
  hestOptAdd(&hopt, "gmc", "min gradmag", airTypeDouble, 1, 1, &gmc,
             "0.0", "For curvature-based queries, use zero when gradient "
             "magnitude is below this");
  hestOptAdd(&hopt, "ofs", "ofs", airTypeInt, 0, 0, &orientationFromSpacing,
             NULL, "If only per-axis spacing is available, use that to "
             "contrive full orientation info");
  hestOptAdd(&hopt, "t", "type", airTypeEnum, 1, 1, &otype, "float",
             "type of output volume", NULL, nrrdType);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output volume");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, probeInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, AIR_CAST(airMopper, hestOptFree), airMopAlways);
  airMopAdd(mop, hopt, AIR_CAST(airMopper, hestParseFree), airMopAlways);

  what = airEnumVal(kind->enm, whatS);
  if (!what) {
    /* 0 indeed always means "unknown" for any gageKind */
    fprintf(stderr, "%s: couldn't parse \"%s\" as measure of \"%s\" volume\n",
            me, whatS, kind->name);
    hestUsage(stderr, hopt, me, hparm);
    hestGlossary(stderr, hopt, hparm);
    airMopError(mop);
    return 1;
  }

  /* special set-up required for DWI kind */
  if (!strcmp(TEN_DWI_GAGE_KIND_NAME, kind->name)) {
    if (tenDWMRIKeyValueParse(&ngrad, &nbmat, &bval, &skip, &skipNum, nin)) {
      airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble parsing DWI info:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
    if (skipNum) {
      fprintf(stderr, "%s: sorry, can't do DWI skipping in tenDwiGage", me);
      airMopError(mop); return 1;
    }
    /* this could stand to use some more command-line arguments */
    if (tenDwiGageKindSet(kind, 50, 1, bval, 0.001, ngrad, nbmat,
                          tenEstimate1MethodLLS,
                          tenEstimate2MethodQSegLLS, seed)) {
      airMopAdd(mop, err = biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble parsing DWI info:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  }

  /* for setting up pre-blurred scale-space samples */
  if (numSS) {
    unsigned int vi;
    sbp = gageStackBlurParmNew();
    airMopAdd(mop, sbp, (airMopper)gageStackBlurParmNix, airMopAlways);
    ninSS = AIR_CAST(Nrrd **, calloc(numSS, sizeof(Nrrd *)));
    if (!ninSS) {
      fprintf(stderr, "%s: couldn't allocate ninSS", me);
      airMopError(mop); return 1;
    }
    for (ninSSIdx=0; ninSSIdx<numSS; ninSSIdx++) {
      ninSS[ninSSIdx] = nrrdNew();
      airMopAdd(mop, ninSS[ninSSIdx], (airMopper)nrrdNuke, airMopAlways);
    }
    if (gageStackBlurParmScaleSet(sbp, numSS, rangeSS[0], rangeSS[1],
                                  SSuniform, SSoptim)
        || gageStackBlurParmKernelSet(sbp, kSSblur, AIR_TRUE)
        || gageStackBlurParmBoundarySet(sbp, nrrdBoundaryBleed, AIR_NAN)
        || gageStackBlurParmVerboseSet(sbp, verbose)) {
      airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble with stack blur info:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
    if (airStrlen(stackReadFormat)) {
      if (nrrdLoadMulti(ninSS, numSS,
                        stackReadFormat, 0, NULL)) {
        airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble loading blurrings:\n%s\n", me, err);
        airMopError(mop); return 1;
      }
      if (gageStackBlurCheck(AIR_CAST(const Nrrd *const*, ninSS),
                             sbp, nin, kind)) {
        airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble:\n%s\n", me, err);
        airMopError(mop); return 1;
      }
    } else {
      if (gageStackBlur(ninSS, sbp, nin, kind)) {
        airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
        fprintf(stderr, "%s: trouble pre-computing blurrings:\n%s\n", me, err);
        airMopError(mop); return 1;
      }
      if (airStrlen(stackSaveFormat) && !airStrlen(stackReadFormat)) {
        if (nrrdSaveMulti(stackSaveFormat,
                          AIR_CAST(const Nrrd *const *, ninSS),
                          numSS, 0, NULL)) {
          airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
          fprintf(stderr, "%s: trouble saving blurrings:\n%s\n", me, err);
          airMopError(mop); return 1;
        }
      }
    }
    /* there's actually work to do here, weirdly: gageProbe can either
       work in index space, or in world space, but vprobe has
       basically always been index-space-centric.  For doing any kind
       scale/stack space hacking for which vprobe is suited, its nicer
       to have the user-specified stack position be in the stack's
       "world" space, not the (potentially non-uniform) index space.
       So here, we have to actually replicate work that is done by
       _gageProbeSpace() in converting from world to index space */
    for (vi=0; vi<numSS-1; vi++) {
      if (AIR_IN_CL(sbp->scale[vi], wrlSS, sbp->scale[vi+1])) {
        idxSS = vi + AIR_AFFINE(sbp->scale[vi], wrlSS, sbp->scale[vi+1], 0, 1);
        if (verbose > 1) {
          fprintf(stderr, "%s: scale pos %g -> idx %g = %u + %g\n", me,
                  wrlSS, idxSS, vi,
                  AIR_AFFINE(sbp->scale[vi], wrlSS, sbp->scale[vi+1], 0, 1));
        }
        break;
      }
    }
    if (vi == numSS-1) {
      fprintf(stderr, "%s: scale pos %g outside range %g=%g, %g=%g\n", me,
              wrlSS, rangeSS[0], sbp->scale[0],
              rangeSS[1], sbp->scale[numSS-1]);
      airMopError(mop); return 1;
    }
  } else {
    ninSS = NULL;
    sbp = NULL;
  }

  /***
  **** Except for the gageProbe() call in the inner loop below,
  **** and the gageContextNix() call at the very end, all the gage
  **** calls which set up (and take down) the context and state are here.
  ***/
  ctx = gageContextNew();
  airMopAdd(mop, ctx, AIR_CAST(airMopper, gageContextNix), airMopAlways);
  gageParmSet(ctx, gageParmGradMagCurvMin, gmc);
  gageParmSet(ctx, gageParmVerbose, verbose);
  gageParmSet(ctx, gageParmRenormalize, renorm ? AIR_TRUE : AIR_FALSE);
  gageParmSet(ctx, gageParmCheckIntegrals, AIR_TRUE);
  gageParmSet(ctx, gageParmOrientationFromSpacing, orientationFromSpacing);
  E = 0;
  if (!E) E |= !(pvl = gagePerVolumeNew(ctx, nin, kind));
  if (!E) E |= gageKernelSet(ctx, gageKernel00, k00->kernel, k00->parm);
  if (!E) E |= gageKernelSet(ctx, gageKernel11, k11->kernel, k11->parm);
  if (!E) E |= gageKernelSet(ctx, gageKernel22, k22->kernel, k22->parm);
  if (numSS) {
    gagePerVolume **pvlSS;
    gageParmSet(ctx, gageParmStackUse, AIR_TRUE);
    gageParmSet(ctx, gageParmStackNormalizeDeriv, SSnormd);
    if (!E) E |= !(pvlSS = AIR_CAST(gagePerVolume **,
                                    calloc(numSS, sizeof(gagePerVolume *))));
    if (!E) airMopAdd(mop, pvlSS, (airMopper)airFree, airMopAlways);
    if (!E) E |= gageStackPerVolumeNew(ctx, pvlSS,
                                       AIR_CAST(const Nrrd*const*, ninSS),
                                       numSS, kind);
    if (!E) E |= gageStackPerVolumeAttach(ctx, pvl, pvlSS, sbp->scale, numSS);
    if (!E) E |= gageKernelSet(ctx, gageKernelStack, kSS->kernel, kSS->parm);
  } else {
    if (!E) E |= gagePerVolumeAttach(ctx, pvl);
  }
  if (!E) E |= gageQueryItemOn(ctx, pvl, what);
  if (!E) E |= gageUpdate(ctx);
  if (E) {
    airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }
  answer = gageAnswerPointer(ctx, pvl, what);
  /***
  **** end gage setup.
  ***/

  ansLen = kind->table[what].answerLength;
  iBaseDim = kind->baseDim;
  oBaseDim = 1 == ansLen ? 0 : 1;
  six = nin->axis[0+iBaseDim].size;
  siy = nin->axis[1+iBaseDim].size;
  siz = nin->axis[2+iBaseDim].size;
  dsix = AIR_CAST(double, six);
  dsiy = AIR_CAST(double, siy);
  dsiz = AIR_CAST(double, siz);
  sox = AIR_CAST(size_t, scale[0]*dsix);
  soy = AIR_CAST(size_t, scale[1]*dsiy);
  soz = AIR_CAST(size_t, scale[2]*dsiz);
  dsox = AIR_CAST(double, sox);
  dsoy = AIR_CAST(double, soy);
  dsoz = AIR_CAST(double, soz);
  if (verbose) {
    fprintf(stderr, "%s: six,y,z = %u %u %u\n", me,
            AIR_UINT(six),
            AIR_UINT(siy),
            AIR_UINT(siz));
    fprintf(stderr, "%s: sox,y,z = %u %u %u\n", me,
            AIR_UINT(sox),
            AIR_UINT(soy),
            AIR_UINT(soz));
  }
  rscl[0] = dsix/dsox;
  rscl[1] = dsiy/dsoy;
  rscl[2] = dsiz/dsoz;

  if (verbose) {
    fprintf(stderr, "%s: kernel support = %d^3 samples\n", me,
            2*ctx->radius);
    fprintf(stderr, "%s: effective scaling is %g %g %g\n", me,
            rscl[0], rscl[1], rscl[2]);
  }
  /* else, normal volume probing */
  if (ansLen > 1) {
    if (verbose) {
      fprintf(stderr, "%s: creating %s x %s x %s x %s output\n", me,
              airSprintSize_t(stmp[0], ansLen),
              airSprintSize_t(stmp[1], sox),
              airSprintSize_t(stmp[2], soy),
              airSprintSize_t(stmp[3], soz));
    }
    if (!E) E |= nrrdMaybeAlloc_va(nout=nrrdNew(), otype, 4,
                                   ansLen, sox, soy, soz);
  } else {
    if (verbose) {
      fprintf(stderr, "%s: creating %s x %s x %s output\n", me,
              airSprintSize_t(stmp[0], sox),
              airSprintSize_t(stmp[1], soy),
              airSprintSize_t(stmp[2], soz));
    }
    if (!E) E |= nrrdMaybeAlloc_va(nout=nrrdNew(), otype, 3,
                                   sox, soy, soz);
  }
  airMopAdd(mop, nout, AIR_CAST(airMopper, nrrdNuke), airMopAlways);
  if (E) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  hackSet = nrrdGetenvUInt(&hackZi, &hackValStr, hackKeyStr);
  if (AIR_FALSE == hackSet) {
    fprintf(stderr, "%s: couldn't parse value of \"%s\" (\"%s\") as uint\n",
            me, hackKeyStr, hackValStr);
    airMopError(mop);
    return 1;
  }
  if (AIR_TRUE == hackSet) {
    fprintf(stderr, "%s: %s hack on: will only measure Zi=%u\n",
            me, hackKeyStr, hackZi);
  }

  if (nrrdCenterCell == ctx->shape->center) {
    ELL_3V_SET(min, -0.5, -0.5, -0.5);
    ELL_3V_SET(maxOut, dsox-0.5, dsoy-0.5, dsoz-0.5);
    ELL_3V_SET(maxIn,  dsix-0.5, dsiy-0.5, dsiz-0.5);
  } else {
    ELL_3V_SET(min, 0, 0, 0);
    ELL_3V_SET(maxOut, dsox-1, dsoy-1, dsoz-1);
    ELL_3V_SET(maxIn, dsix-1, dsiy-1, dsiz-1);
  }
  t0 = airTime();
  ins = nrrdDInsert[nout->type];
  gageParmSet(ctx, gageParmVerbose, verbose/10);
  for (zi=0; zi<soz; zi++) {
    if (verbose) {
      if (verbose > 1) {
        fprintf(stderr, "z = ");
      }
      fprintf(stderr, " %s/%s",
              airSprintSize_t(stmp[0], zi),
              airSprintSize_t(stmp[1], soz-1));
      fflush(stderr);
      if (verbose > 1) {
        fprintf(stderr, "\n");
      }
    }
    if (AIR_TRUE == hackSet) {
      if (hackZi != zi) {
        continue;
      }
    }

    z = AIR_AFFINE(min[2], zi, maxOut[2], min[2], maxIn[2]);
    for (yi=0; yi<soy; yi++) {
      y = AIR_AFFINE(min[1], yi, maxOut[1], min[1], maxIn[1]);
      if (2 == verbose) {
        fprintf(stderr, " %u/%u", AIR_UINT(yi),
                AIR_UINT(soy));
        fflush(stderr);
      }
      for (xi=0; xi<sox; xi++) {
        if (verbose > 2) {
          fprintf(stderr, " (%u,%u)/(%u,%u)",
                  AIR_UINT(xi), AIR_UINT(yi),
                  AIR_UINT(sox), AIR_UINT(soy));
          fflush(stderr);
        }
        x = AIR_AFFINE(min[0], xi, maxOut[0], min[0], maxIn[0]);
        idx = xi + sox*(yi + soy*zi);
        E = (numSS
             ? gageStackProbe(ctx, x, y, z, idxSS)
             : gageProbe(ctx, x, y, z));
        if (E) {
          fprintf(stderr,
                  "%s: trouble at i=(%s,%s,%s) -> f=(%g,%g,%g):\n%s\n(%d)\n",
                  me, airSprintSize_t(stmp[0], xi),
                  airSprintSize_t(stmp[1], yi),
                  airSprintSize_t(stmp[2], zi), x, y, z,
                  ctx->errStr, ctx->errNum);
          airMopError(mop);
          return 1;
        }
        if (1 == ansLen) {
          ins(nout->data, idx, *answer);
        } else {
          for (ai=0; ai<=ansLen-1; ai++) {
            ins(nout->data, ai + ansLen*idx, answer[ai]);
          }
        }
      }
    }
  }

  /* HEY: this isn't actually correct in general, but is true
     for gageKindScl and gageKindVec */
  nrrdContentSet_va(nout, "probe", nin, "%s", airEnumStr(kind->enm, what));

  for (axi=0; axi<3; axi++) {
    /* HEY: why not using nrrdAxisInfoCopy? */
    nout->axis[axi+oBaseDim].label = airStrdup(nin->axis[axi+iBaseDim].label);
    nout->axis[axi+oBaseDim].center = ctx->shape->center;
    nout->axis[axi+oBaseDim].kind = nin->axis[axi+iBaseDim].kind;
  }

  nrrdBasicInfoCopy(nout, nin, (NRRD_BASIC_INFO_DATA_BIT
                                | NRRD_BASIC_INFO_TYPE_BIT
                                | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                                | NRRD_BASIC_INFO_DIMENSION_BIT
                                | NRRD_BASIC_INFO_CONTENT_BIT
                                | NRRD_BASIC_INFO_SPACEORIGIN_BIT
                                | NRRD_BASIC_INFO_OLDMIN_BIT
                                | NRRD_BASIC_INFO_OLDMAX_BIT
                                | NRRD_BASIC_INFO_COMMENTS_BIT
                                | NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT));
  if (ctx->shape->fromOrientation) {
    if (nin->space) {
      nrrdSpaceSet(nout, nin->space);
    } else {
      nrrdSpaceDimensionSet(nout, nin->spaceDim);
    }
    nrrdSpaceVecCopy(nout->spaceOrigin, nin->spaceOrigin);
    for (axi=0; axi<3; axi++) {
      nrrdSpaceVecScale(nout->axis[axi+oBaseDim].spaceDirection,
                        rscl[axi],
                        nin->axis[axi+iBaseDim].spaceDirection);
      z = AIR_AFFINE(min[axi], 0, maxOut[axi], min[axi], maxIn[axi]);
      nrrdSpaceVecScaleAdd2(nout->spaceOrigin,
                            1.0, nout->spaceOrigin,
                            z, nin->axis[axi+iBaseDim].spaceDirection);
    }
  } else {
    for (axi=0; axi<3; axi++) {
      nout->axis[axi+oBaseDim].spacing =
        rscl[axi]*nin->axis[axi+iBaseDim].spacing;
    }
  }

  fprintf(stderr, "\n");
  t1 = airTime();
  fprintf(stderr, "probe rate = %g KHz\n", dsox*dsoy*dsoz/(1000.0*(t1-t0)));
  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving output:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  airMopOkay(mop);
  return 0;
}
