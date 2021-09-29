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

static void
printans(FILE *file, const double *ans, unsigned int len) {
  unsigned int ai;

  AIR_UNUSED(file);
  for (ai=0; ai<len; ai++) {
    if (ai) {
      printf(", ");
    }
    printf("%g", ans[ai]);
  }
}

static int
gridProbe(gageContext *ctx, gagePerVolume *pvl, int what,
          Nrrd *nout, int typeOut, Nrrd *_ngrid,
          int indexSpace, int verbose, int clamp) {
  char me[]="gridProbe";
  Nrrd *ngrid;
  airArray *mop;
  double *grid, pos[4];
  const double *answer;
  unsigned int ansLen, dim, aidx, baseDim, gridDim;
  size_t sizeOut[NRRD_DIM_MAX], coordOut[NRRD_DIM_MAX], II, NN;
  double (*ins)(void *v, size_t I, double d);
  char stmp[2][AIR_STRLEN_SMALL];

  if (!(ctx && pvl && nout && _ngrid)) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(nrrdType, typeOut)) {
    biffAddf(GAGE, "%s: type %d not valid", me, typeOut);
    return 1;
  }
  if (!gagePerVolumeIsAttached(ctx, pvl)) {
    biffAddf(GAGE, "%s: given pvl not attached to context", me);
    return 1;
  }
  if (!(2 == _ngrid->dim)) {
    biffAddf(GAGE, "%s: ngrid must be 2 (not %u)", me, _ngrid->dim);
    return 1;
  }
  if ((ctx->stackPos && _ngrid->axis[0].size != 5)
      || (!ctx->stackPos && _ngrid->axis[0].size != 4)) {
    biffAddf(GAGE, "%s: if %susing stack, need "
             "ngrid->axis[0].size = %u = 1 + %u (not %u)", me,
             (ctx->stackPos ? "" : "not "),
             (ctx->stackPos ? 4 : 3) + 1,
             (ctx->stackPos ? 4 : 3),
             AIR_UINT(_ngrid->axis[0].size));
    return 1;
  }

  mop = airMopNew();
  ngrid = nrrdNew();
  airMopAdd(mop, ngrid, (airMopper)nrrdNuke, airMopAlways);
  if (ctx->stackPos) {
    if (nrrdConvert(ngrid, _ngrid, nrrdTypeDouble)) {
      biffMovef(GAGE, NRRD, "%s: trouble converting ngrid", me);
      airMopError(mop); return 1;
    }
  } else {
    Nrrd *ntmp;
    ptrdiff_t minIdx[2], maxIdx[2];
    minIdx[0] = minIdx[1] = 0;
    maxIdx[0] = 4;                      /* pad by one sample */
    maxIdx[1] = AIR_CAST(ptrdiff_t, _ngrid->axis[1].size-1); /* no padding */
    ntmp = nrrdNew();
    airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdConvert(ntmp, _ngrid, nrrdTypeDouble)
        || nrrdPad_nva(ngrid, ntmp, minIdx, maxIdx, nrrdBoundaryPad, 0.0)) {
      biffMovef(GAGE, NRRD, "%s: trouble converting/padding ngrid", me);
      airMopError(mop); return 1;
    }
  }
  grid = AIR_CAST(double *, ngrid->data);
  gridDim = AIR_ROUNDUP_UI(grid[0]);
  if (gridDim + 1 != ngrid->axis[1].size) {
    biffAddf(GAGE, "%s: ngrid->axis[1].size = %u but expected %u = 1 + %u",
             me, AIR_UINT(ngrid->axis[1].size),
             1 + gridDim, gridDim);
    airMopError(mop); return 1;
  }
  answer = gageAnswerPointer(ctx, pvl, what);
  ansLen = pvl->kind->table[what].answerLength;
  baseDim = 1 == ansLen ? 0 : 1;
  dim = baseDim + gridDim;
  if (dim > NRRD_DIM_MAX) {
    biffAddf(GAGE, "%s: output dimension %u unreasonable", me, dim);
    airMopError(mop); return 1;
  }
  if (ansLen > 1) {
    sizeOut[0] = ansLen;
    coordOut[0] = 0;
  }
  NN = 1;
  for (aidx=0; aidx<gridDim; aidx++) {
    sizeOut[aidx + baseDim] = AIR_ROUNDUP_UI(grid[0 + 5*(aidx+1)]);
    NN *= sizeOut[aidx + baseDim];
    coordOut[aidx + baseDim] = 0;
  }
  if (nrrdMaybeAlloc_nva(nout, typeOut, dim, sizeOut)) {
    biffMovef(GAGE, NRRD, "%s: couldn't allocate output", me);
    airMopError(mop); return 1;
  }
  ins = nrrdDInsert[nout->type];
  for (II=0; II<NN; II++) {
    int E;
    if (verbose && 3 == gridDim && !coordOut[0] && !coordOut[1]) {
      if (verbose > 1) {
        fprintf(stderr, "z = ");
      }
      fprintf(stderr, " %s/%s",
              airSprintSize_t(stmp[0], coordOut[2]),
              airSprintSize_t(stmp[1], sizeOut[2]));
      fflush(stderr);
      if (verbose > 1) {
        fprintf(stderr, "\n");
      }
    }
    ELL_4V_COPY(pos, grid + 1 + 5*0);
    for (aidx=0; aidx<gridDim; aidx++) {
      ELL_4V_SCALE_ADD2(pos, 1, pos,
                        AIR_CAST(double, coordOut[aidx + baseDim]),
                        grid + 1 + 5*(1+aidx));
    }
    /*
    printf("%s: %u -> (%u %u) -> %g %g %g %g (%s)\n", me,
           AIR_UINT(II),
           AIR_UINT(coordOut[0+baseDim]),
           AIR_UINT(coordOut[1+baseDim]),
           pos[0], pos[1], pos[2], pos[3],
           indexSpace ? "index" : "world");
    */
    E = (ctx->stackPos
         ? gageStackProbeSpace(ctx, pos[0], pos[1], pos[2], pos[3],
                               indexSpace, clamp)
         : gageProbeSpace(ctx, pos[0], pos[1], pos[2],
                          indexSpace, clamp));
    if (E) {
      biffAddf(GAGE, "%s: trouble at II=%s =(%g,%g,%g,%g):\n%s\n(%d)\n", me,
               airSprintSize_t(stmp[0], II),
               pos[0], pos[1], pos[2], pos[3],
               ctx->errStr, ctx->errNum);
      airMopError(mop); return 1;
    }
    if (1 == ansLen) {
      ins(nout->data, II, *answer);
    } else {
      for (aidx=0; aidx<ansLen; aidx++) {
        ins(nout->data, aidx + ansLen*II, answer[aidx]);
      }
    }
    NRRD_COORD_INCR(coordOut, sizeOut, dim, baseDim);
  }
  if (verbose && verbose <= 1) {
    fprintf(stderr, "\n");
  }

  airMopOkay(mop);
  return 0;
}

static const char *probeInfo =
  ("Shows off the functionality of the gage library. "
   "Uses gageProbe() to query various kinds of volumes "
   "to learn various measured or derived quantities.");

int
main(int argc, const char *argv[]) {
  gageKind *kind;
  const char *me;
  char *whatS, *err, *outS, *stackFnameFormat;
  hestParm *hparm;
  hestOpt *hopt = NULL;
  NrrdKernelSpec *k00, *k11, *k22, *kSS, *kSSblur;
  int what, E=0, renorm, uniformSS, optimSS, verbose,
    orientationFromSpacing, probeSpaceIndex, normdSS;
  unsigned int iBaseDim, oBaseDim, axi, numSS, seed;
  const double *answer;
  Nrrd *nin, *_npos, *npos, *_ngrid, *ngrid, *nout, **ninSS=NULL;
  Nrrd *ngrad=NULL, *nbmat=NULL;
  size_t six, siy, siz, sox, soy, soz;
  double bval=0, eps, gmc, rangeSS[2], *pntPos, scale[3], posSS, biasSS,
    dsix, dsiy, dsiz, dsox, dsoy, dsoz;
  gageContext *ctx;
  gagePerVolume *pvl=NULL;
  double t0, t1, rscl[3], min[3], maxOut[3], maxIn[3];
  airArray *mop;
  unsigned int ansLen, *skip, skipNum, pntPosNum;
  gageStackBlurParm *sbp;
  int otype, clamp;
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
  hestOptAdd(&hopt, "gmc", "min gradmag", airTypeDouble, 1, 1, &gmc,
             "0.0", "For curvature-based queries, use zero when gradient "
             "magnitude is below this");
  hestOptAdd(&hopt, "ofs", "ofs", airTypeInt, 0, 0, &orientationFromSpacing,
             NULL, "If only per-axis spacing is available, use that to "
             "contrive full orientation info");
  hestOptAdd(&hopt, "seed", "N", airTypeUInt, 1, 1, &seed, "42",
             "RNG seed; mostly for debugging");
  hestOptAdd(&hopt, "c", "bool", airTypeBool, 1, 1, &clamp, "false",
             "clamp positions as part of probing");

  hestOptAdd(&hopt, "k00", "kern00", airTypeOther, 1, 1, &k00,
             "tent", "kernel for gageKernel00",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k11", "kern11", airTypeOther, 1, 1, &k11,
             "cubicd:1,0", "kernel for gageKernel11",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "k22", "kern22", airTypeOther, 1, 1, &k22,
             "cubicdd:1,0", "kernel for gageKernel22",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "rn", NULL, airTypeInt, 0, 0, &renorm, NULL,
             "renormalize kernel weights at each new sample location. "
             "\"Accurate\" kernels don't need this; doing it always "
             "makes things go slower");

  hestOptAdd(&hopt, "ssn", "SS #", airTypeUInt, 1, 1, &numSS,
             "0", "how many scale-space samples to evaluate, or, "
             "0 to turn-off all scale-space behavior");
  hestOptAdd(&hopt, "ssr", "scale range", airTypeDouble, 2, 2, rangeSS,
             "nan nan", "range of scales in scale-space");
  hestOptAdd(&hopt, "kssb", "kernel", airTypeOther, 1, 1, &kSSblur,
             "dgauss:1,5", "blurring kernel, to sample scale space",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "kssr", "kernel", airTypeOther, 1, 1, &kSS,
             "hermite", "kernel for reconstructing from scale space samples",
             NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&hopt, "ssu", NULL, airTypeInt, 0, 0, &uniformSS, NULL,
             "do uniform samples along sigma, and not (by default) "
             "samples according to the effective diffusion scale");
  hestOptAdd(&hopt, "sso", NULL, airTypeInt, 0, 0, &optimSS, NULL,
             "if not using \"-ssu\", use pre-computed optimal "
             "sigmas when possible");
  hestOptAdd(&hopt, "ssnd", NULL, airTypeInt, 0, 0, &normdSS, NULL,
             "normalize derivatives by scale");
  hestOptAdd(&hopt, "ssnb", "bias", airTypeDouble, 1, 1, &biasSS, "0.0",
             "bias on scale-based derivative normalization");
  hestOptAdd(&hopt, "ssf", "SS read/save format", airTypeString, 1, 1,
             &stackFnameFormat, "",
             "printf-style format (including a \"%u\") for the "
             "filenames from which to read "
             "pre-blurred volumes computed for the stack, if they "
             "exist and match the stack parameters, and where to save "
             "them if they had to be re-computed.  Leave this as empty "
             "string to disable this.");

  hestOptAdd(&hopt, "s", "sclX sclY sxlZ", airTypeDouble, 3, 3, scale,
             "1 1 1",
             "scaling factor for resampling on each axis "
             "(>1.0 : supersampling); use \"-ssp\" (and \"-psi\")"
             "to specify scale position of sampling");
  hestOptAdd(&hopt, "ssp", "pos", airTypeDouble, 1, 1, &posSS, "0",
             "when using scale-space, scale-position at which to probe");
  hestOptAdd(&hopt, "pg", "nrrd", airTypeOther, 1, 1, &_ngrid, "",
             "overrides \"-s\": "
             "2-D nrrd which specifies origin and direction vectors "
             "for sampling grid", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "pi", "nrrd", airTypeOther, 1, 1, &_npos, "",
             "overrides \"-pv\": probes at this list of 3-vec or "
             "4-vec positions", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "pp", "pos", airTypeDouble, 3, 4, &pntPos,
             "nan nan nan",
             "overrides \"-pi\": only sample at this specified point",
             &pntPosNum);
  hestOptAdd(&hopt, "eps", "epsilon", airTypeDouble, 1, 1, &eps, "0",
             "if non-zero, and if query is a scalar, and if using \"pp\" "
             "to query at a single point, then do epsilon offset probes "
             "to calculate discrete differences, to find the numerical "
             "gradient and hessian (for debugging)");
  hestOptAdd(&hopt, "psi", "p", airTypeBool, 1, 1, &probeSpaceIndex, "false",
             "whether the probe location specification (by any of "
             "the four previous flags) are in index space");

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
    airMopError(mop); return 1;
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
    int recompute;

    sbp = gageStackBlurParmNew();
    airMopAdd(mop, sbp, (airMopper)gageStackBlurParmNix, airMopAlways);
    if (gageStackBlurParmScaleSet(sbp, numSS, rangeSS[0], rangeSS[1],
                                  uniformSS, optimSS)
        || gageStackBlurParmKernelSet(sbp, kSSblur, AIR_TRUE)
        || gageStackBlurParmBoundarySet(sbp, nrrdBoundaryBleed, AIR_NAN)
        || gageStackBlurParmVerboseSet(sbp, verbose)
        || gageStackBlurManage(&ninSS, &recompute, sbp,
                               stackFnameFormat, AIR_TRUE, NULL,
                               nin, kind)) {
      airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble getting volume stack:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
    airMopAdd(mop, ninSS, airFree, airMopAlways);
    if (verbose > 2) {
      fprintf(stderr, "%s: sampling scale range %g--%g %suniformly:\n", me,
              rangeSS[0], rangeSS[1], uniformSS ? "" : "non-");
      for (vi=0; vi<numSS; vi++) {
        fprintf(stderr, "    scalePos[%u] = %g\n", vi, sbp->scale[vi]);
      }
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
    gageParmSet(ctx, gageParmStackNormalizeDerivBias, biasSS);
    gageParmSet(ctx, gageParmStackNormalizeDeriv, normdSS);
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
    airMopError(mop); return 1;
  }
  answer = gageAnswerPointer(ctx, pvl, what);
  ansLen = kind->table[what].answerLength;
  /***
  **** end gage setup.
  ***/
  if (verbose) {
    fprintf(stderr, "%s: kernel support = %d^3 samples\n", me,
            2*ctx->radius);
  }

  if (ELL_3V_EXISTS(pntPos)) {
    /* only interested in a single point, make sure we have the right
       info about the point WRT scale stuff */
    if (numSS) {
      if (!(4 == pntPosNum && ELL_4V_EXISTS(pntPos))) {
        fprintf(stderr, "%s: need a 4-vec position with scale-space", me);
        airMopError(mop); return 1;
      }
    } else {
      if (!(3 == pntPosNum)) {
        fprintf(stderr, "%s: need a 3-vec position (w/out scale-space)", me);
        airMopError(mop); return 1;
      }
    }
    if (numSS
        ? gageStackProbeSpace(ctx,
                              pntPos[0], pntPos[1], pntPos[2], pntPos[3],
                              probeSpaceIndex, clamp)
        : gageProbeSpace(ctx,
                         pntPos[0], pntPos[1], pntPos[2],
                         probeSpaceIndex, clamp)) {
      fprintf(stderr, "%s: trouble probing: (errNum %d) %s\n", me,
              ctx->errNum, ctx->errStr);
      airMopError(mop); return 1;
    }
    if (numSS) {
      printf("%s: %s(%s:%g,%g,%g,%g) = ", me, airEnumStr(kind->enm, what),
             probeSpaceIndex ? "index" : "world",
             pntPos[0], pntPos[1], pntPos[2], pntPos[3]);
    } else {
      printf("%s: %s(%s:%g,%g,%g) = ", me, airEnumStr(kind->enm, what),
             probeSpaceIndex ? "index" : "world",
             pntPos[0], pntPos[1], pntPos[2]);
    }
    printans(stdout, answer, ansLen);
    printf("\n");
    /* we're done, get out of here */
    /* except if we're supposed to debug derivatives */
    if (eps && 1 == ansLen) {
      double v[3][3][3], fes, ee;
      int xo, yo, zo;
      if (probeSpaceIndex) {
        fprintf(stderr, "\n%s: WARNING!!: not probing in world-space (via "
                "\"-wsp\") likely leads to errors in estimated "
                "derivatives\n\n", me);
      }
      gageParmSet(ctx, gageParmVerbose, 0);
#define PROBE(x, y, z)                                                  \
      ((numSS                                                           \
        ? gageStackProbeSpace(ctx, x, y, z, posSS,                      \
                              probeSpaceIndex, clamp)                   \
        : gageProbeSpace(ctx, x, y, z, probeSpaceIndex,                 \
                         clamp)),answer[0])
      for (xo=0; xo<=2; xo++) {
        for (yo=0; yo<=2; yo++) {
          for (zo=0; zo<=2; zo++) {
            v[xo][yo][zo] = PROBE(pntPos[0] + (xo-1)*eps,
                                  pntPos[1] + (yo-1)*eps,
                                  pntPos[2] + (zo-1)*eps);
          }
        }
      }
      printf("%s: approx gradient(%s) at (%g,%g,%g) = %f %f %f\n", me,
             airEnumStr(kind->enm, what), pntPos[0], pntPos[1], pntPos[2],
             (v[2][1][1] - v[0][1][1])/(2*eps),
             (v[1][2][1] - v[1][0][1])/(2*eps),
             (v[1][1][2] - v[1][1][0])/(2*eps));
      fes = 4*eps*eps;
      ee = eps*eps;
      printf("%s: approx hessian(%s) at (%g,%g,%g) = \n"
             "%f %f %f\n"
             "   %f %f\n"
             "      %f\n", me,
             airEnumStr(kind->enm, what), pntPos[0], pntPos[1], pntPos[2],
             (v[0][1][1] - 2*v[1][1][1] + v[2][1][1])/ee,
             (v[2][2][1] - v[0][2][1] - v[2][0][1] + v[0][0][1])/fes,
             (v[2][1][2] - v[0][1][2] - v[2][1][0] + v[0][1][0])/fes,
             (v[1][2][1] - 2*v[1][1][1] + v[1][0][1])/ee,
             (v[1][2][2] - v[1][0][2] - v[1][2][0] + v[1][0][0])/fes,
             (v[1][1][2] - 2*v[1][1][1] + v[1][1][0])/ee);
    }
    airMopOkay(mop); return 0;
  }

  if (_npos) {
    /* given a nrrd of probe locations */
    double *pos, (*ins)(void *v, size_t I, double d);
    size_t II, NN;
    unsigned int aidx;
    if (!(2 == _npos->dim
          && (3 == _npos->axis[0].size || 4 == _npos->axis[0].size))) {
      fprintf(stderr, "%s: need npos 2-D 3-by-N or 4-by-N "
              "(not %u-D %u-by-N)\n", me, _npos->dim,
              AIR_UINT(_npos->axis[0].size));
      airMopError(mop); return 1;
    }
    if ((numSS && 3 == _npos->axis[0].size)
        || (!numSS && 4 == _npos->axis[0].size)) {
      fprintf(stderr, "%s: have %u point coords but %s using scale-space\n",
              me, AIR_UINT(_npos->axis[0].size),
              numSS ? "are" : "are not");
      airMopError(mop); return 1;
    }
    NN = _npos->axis[1].size;
    npos = nrrdNew();
    airMopAdd(mop, npos, AIR_CAST(airMopper, nrrdNuke), airMopAlways);
    nout = nrrdNew();
    airMopAdd(mop, nout, AIR_CAST(airMopper, nrrdNuke), airMopAlways);
    if (nrrdConvert(npos, _npos, nrrdTypeDouble)
        || nrrdMaybeAlloc_va(nout, otype, 2,
                             AIR_CAST(size_t, ansLen),
                             AIR_CAST(size_t, NN))) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble with npos or nout:\n%s\n", me, err);
      airMopError(mop); return 1;
    }

    pos = AIR_CAST(double *, npos->data);
    ins = nrrdDInsert[nout->type];
    for (II=0; II<NN; II++) {
      if (numSS) {
        gageStackProbeSpace(ctx, pos[0], pos[1], pos[2], pos[3],
                            probeSpaceIndex, clamp);
      } else {
        gageProbeSpace(ctx, pos[0], pos[1], pos[2],
                       probeSpaceIndex, clamp);
      }
      if (1 == ansLen) {
        ins(nout->data, II, *answer);
      } else {
        for (aidx=0; aidx<ansLen; aidx++) {
          ins(nout->data, aidx + ansLen*II, answer[aidx]);
        }
      }
      /*
      if (numSS) {
        printf("%s: %s(%s:%g,%g,%g,%g) = ", me, airEnumStr(kind->enm, what),
               probeSpaceIndex ? "index" : "world",
               pos[0], pos[1], pos[2], pos[3]);
      } else {
        printf("%s: %s(%s:%g,%g,%g) = ", me, airEnumStr(kind->enm, what),
               probeSpaceIndex ? "index" : "world",
               pos[0], pos[1], pos[2]);
      }
      printans(stdout, answer, ansLen);
      printf("\n");
      */
      pos += _npos->axis[0].size;
    }
    if (nrrdSave(outS, nout, NULL)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble saving output:\n%s\n", me, err);
      airMopError(mop);
      return 1;
    }

    /* we're done, get out of here */
    airMopOkay(mop);
    exit(0);
  }

  /* else, we're sampling on some kind of grid */
  ngrid = nrrdNew();
  airMopAdd(mop, ngrid, (airMopper)nrrdNuke, airMopAlways);
  iBaseDim = kind->baseDim;
  oBaseDim = 1 == ansLen ? 0 : 1;
  if (!_ngrid) {
    /* did not get a grid, have to use "-s" args to define it */
    double *grid;
    size_t gridSize[NRRD_DIM_MAX];
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
    rscl[0] = dsix/dsox;
    rscl[1] = dsiy/dsoy;
    rscl[2] = dsiz/dsoz;
    if (verbose) {
      fprintf(stderr, "%s: creating %u x %s x %s x %s output\n", me,
              ansLen,
              airSprintSize_t(stmp[1], sox),
              airSprintSize_t(stmp[2], soy),
              airSprintSize_t(stmp[3], soz));
      fprintf(stderr, "%s: effective scaling is %g %g %g\n", me,
              rscl[0], rscl[1], rscl[2]);
    }
    gridSize[0] = numSS ? 5 : 4;
    gridSize[1] = 4;
    if (nrrdMaybeAlloc_nva(ngrid, nrrdTypeDouble, 2, gridSize)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble making ngrid:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
    grid = AIR_CAST(double *, ngrid->data);
    if (nrrdCenterCell == ctx->shape->center) {
      ELL_3V_SET(min, -0.5, -0.5, -0.5);
      ELL_3V_SET(maxOut, dsox-0.5, dsoy-0.5, dsoz-0.5);
      ELL_3V_SET(maxIn,  dsix-0.5, dsiy-0.5, dsiz-0.5);
    } else {
      ELL_3V_SET(min, 0, 0, 0);
      ELL_3V_SET(maxOut, dsox-1, dsoy-1, dsoz-1);
      ELL_3V_SET(maxIn,  dsix-1, dsiy-1, dsiz-1);
    }
    ELL_4V_SET(grid + gridSize[0]*0, 3,
               NRRD_POS(ctx->shape->center, min[0], maxIn[0], sox, 0),
               NRRD_POS(ctx->shape->center, min[1], maxIn[1], soy, 0),
               NRRD_POS(ctx->shape->center, min[2], maxIn[2], soz, 0));
    ELL_4V_SET(grid + gridSize[0]*1, dsox,
               AIR_DELTA(min[0], 1, maxOut[0], min[0], maxIn[0]),
               0,
               0);
    ELL_4V_SET(grid + gridSize[0]*2, dsoy,
               0,
               AIR_DELTA(min[1], 1, maxOut[1], min[1], maxIn[1]),
               0);
    ELL_4V_SET(grid + gridSize[0]*3, dsoz,
               0,
               0,
               AIR_DELTA(min[2], 1, maxOut[2], min[2], maxIn[2]));
    if (numSS) {
      if (!probeSpaceIndex) {
        double idxSS = AIR_NAN;
        unsigned int vi;
        /* there's actually work to do here, weirdly: gageProbe can
           either work in index space, or in world space, but the
           vprobe-style sampling is index-space-centric, so we have to
           convert the world-space stack position to index space, to be
           consistent with the way that the grid sampling will be
           defined. So, we have to actually replicate work that is done
           by _gageProbeSpace() in converting from world to index space */
        /* HEY: the way that idxSS is set is very strange */
        for (vi=0; vi<numSS-1; vi++) {
          if (AIR_IN_CL(sbp->scale[vi], posSS, sbp->scale[vi+1])) {
            idxSS = vi + AIR_AFFINE(sbp->scale[vi], posSS, sbp->scale[vi+1],
                                    0, 1);
            if (verbose > 1) {
              fprintf(stderr, "%s: scale pos %g -> idx %g = %u + %g\n", me,
                      posSS, idxSS, vi,
                      AIR_AFFINE(sbp->scale[vi], posSS, sbp->scale[vi+1],
                                 0, 1));
            }
            break;
          }
        }
        if (vi == numSS-1) {
          fprintf(stderr, "%s: scale pos %g outside range %g=%g, %g=%g\n", me,
                  posSS, rangeSS[0], sbp->scale[0],
                  rangeSS[1], sbp->scale[numSS-1]);
          airMopError(mop); return 1;
        }
        grid[4 + 5*0] = idxSS;
      } else {
        grid[4 + 5*0] = posSS;
      }
      grid[4 + 5*1] = 0;
      grid[4 + 5*2] = 0;
      grid[4 + 5*3] = 0;
    }
  } else {
    /* we did get a grid, here we only copy from _ngrid to ngrid,
       and let further massaging be done in gridProbe below */
    six = siy = siz = 0;
    sox = soy = soz = 0;
    if (nrrdCopy(ngrid, _ngrid)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble copying ngrid:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  }

  /* probe onto grid */
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  gageParmSet(ctx, gageParmVerbose, verbose);
  t0 = airTime();
  if (gridProbe(ctx, pvl, what, nout, otype, ngrid,
                (_ngrid
                 ? probeSpaceIndex  /* user specifies grid space */
                 : AIR_TRUE),       /* copying vprobe index-space behavior */
                verbose, clamp)) {
    /* note hijacking of GAGE key */
    airMopAdd(mop, err = biffGetDone(GAGE), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble probing on grid:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  t1 = airTime();
  if (verbose) {
    fprintf(stderr, "probe rate = %g KHz\n",
            AIR_CAST(double, nrrdElementNumber(nout)/ansLen)
            / (1000.0*(t1-t0)));
  }

  /* massage output some */
  nrrdContentSet_va(nout, "gprobe", nin, "%s", airEnumStr(kind->enm, what));
  if (!_ngrid) {
    /* did not get a grid, have to emulate vprobe per-axis behavior */
    for (axi=0; axi<3; axi++) {
      nout->axis[axi+oBaseDim].label =
        airStrdup(nin->axis[axi+iBaseDim].label);
      nout->axis[axi+oBaseDim].center = ctx->shape->center;
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
        double tmp;
        nrrdSpaceVecScale(nout->axis[axi+oBaseDim].spaceDirection,
                          rscl[axi],
                          nin->axis[axi+iBaseDim].spaceDirection);
        tmp = AIR_AFFINE(min[axi], 0, maxOut[axi], min[axi], maxIn[axi]);
        nrrdSpaceVecScaleAdd2(nout->spaceOrigin,
                              1.0, nout->spaceOrigin,
                              tmp, nin->axis[axi+iBaseDim].spaceDirection);
      }
    } else {
      for (axi=0; axi<3; axi++) {
        nout->axis[axi+oBaseDim].spacing =
          rscl[axi]*nin->axis[axi+iBaseDim].spacing;
      }
    }
  }

  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving output:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }

  airMopOkay(mop);
  return 0;
}
