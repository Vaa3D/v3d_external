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

const char *
tenModelPrefixStr = "DWMRI_model:";

static const tenModel *
str2model(const char *str) {
  const tenModel *ret;

  if (!strcmp(str, TEN_MODEL_STR_ZERO)) {
    ret = tenModelZero;
  } else if (!strcmp(str, TEN_MODEL_STR_B0)) {
    ret = tenModelB0;
  } else if (!strcmp(str, TEN_MODEL_STR_BALL)) {
    ret = tenModelBall;
  } else if (!strcmp(str, TEN_MODEL_STR_1STICK)) {
    ret = tenModel1Stick;
  } else if (!strcmp(str, TEN_MODEL_STR_1VECTOR2D)) {
    ret = tenModel1Vector2D;
  } else if (!strcmp(str, TEN_MODEL_STR_1UNIT2D)) {
    ret = tenModel1Unit2D;
  } else if (!strcmp(str, TEN_MODEL_STR_2UNIT2D)) {
    ret = tenModel2Unit2D;
  } else if (!strcmp(str, TEN_MODEL_STR_BALL1STICKEMD)) {
    ret = tenModelBall1StickEMD;
  } else if (!strcmp(str, TEN_MODEL_STR_BALL1STICK)) {
    ret = tenModelBall1Stick;
  } else if (!strcmp(str, TEN_MODEL_STR_BALL1CYLINDER)) {
    ret = tenModelBall1Cylinder;
  } else if (!strcmp(str, TEN_MODEL_STR_1CYLINDER)) {
    ret = tenModel1Cylinder;
  } else if (!strcmp(str, TEN_MODEL_STR_1TENSOR2)) {
    ret = tenModel1Tensor2;
  } else {
    /* we don't currently have a tenModelUnknown */
    ret = NULL;
  }
  return ret;
}

int
tenModelParse(const tenModel **model, int *plusB0,
              int requirePrefix, const char *_str) {
  static const char me[]="tenModelParse";
  char *str, *modstr, *pre;
  airArray *mop;

  if (!( model && plusB0 && _str)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  str = airStrdup(_str);
  if (!str) {
    biffAddf(TEN, "%s: couldn't strdup", me);
    return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, str, airFree, airMopAlways);
  pre = strstr(str, tenModelPrefixStr);
  if (pre) {
    str += strlen(tenModelPrefixStr);
  } else {
    if (requirePrefix) {
      biffAddf(TEN, "%s: didn't see prefix \"%s\" in \"%s\"", me,
               tenModelPrefixStr, _str);
      airMopError(mop); return 1;
    }
  }
  airToLower(str); /* for sake of "b0" and str2model below */

  if ((modstr = strchr(str, '+'))) {
    *modstr = '\0';
    ++modstr;
    if (!strcmp(str, "b0")) {
      *plusB0 = AIR_TRUE;
    } else {
      biffAddf(TEN, "%s: string (\"%s\") prior to \"+\" not \"b0\"", me, str);
      airMopError(mop); return 1;
    }
  } else {
    *plusB0 = AIR_FALSE;
    modstr = str;
  }
  if (!(*model = str2model(modstr))) {
    biffAddf(TEN, "%s: didn't recognize \"%s\" as model", me, modstr);
    airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

int
tenModelFromAxisLearnPossible(const NrrdAxisInfo *axinfo) {

  /* HEY keep in synch with nrrdKind* code below */
  return (nrrdKind3DSymMatrix == axinfo->kind
          || nrrdKind3DMaskedSymMatrix == axinfo->kind
          || airStrlen(axinfo->label));
}

int
tenModelFromAxisLearn(const tenModel **modelP,
                      int *plusB0,
                      const NrrdAxisInfo *axinfo) {
  static const char me[]="tenModelFromAxisLearn";

  if (!(modelP && plusB0 && axinfo)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  *plusB0 = AIR_FALSE;
  /* first try to learn model from axis "kind" */
  /* HEY should probably also support 3 vector for stick? */
  /* HEY keep in synch with nrrdKind* code above */
  if (nrrdKind3DSymMatrix == axinfo->kind
      || nrrdKind3DMaskedSymMatrix == axinfo->kind) {
    *modelP = tenModel1Tensor2;
  } else if (airStrlen(axinfo->label)) {
    /* try to parse from label */
    if (tenModelParse(modelP, plusB0, AIR_TRUE, axinfo->label)) {
      biffAddf(TEN, "%s: couldn't parse label \"%s\"", me, axinfo->label);
      *modelP = NULL;
      return 1;
    }
  } else {
    biffAddf(TEN, "%s: don't have kind or label info to learn model", me);
    *modelP = NULL;
    return 1;
  }

  return 0;
}

/*
** If nB0 is given, then those B0 image values will be used.
** In this case, either the parm vector can be short by one (seems to be
** missing B0), or the parm vector includes B0, but these will be ignored
** and over-written with the B0 values from nB0.
**
** basic and axis info is derived from _nparm
*/
int
tenModelSimulate(Nrrd *ndwi, int typeOut,
                 tenExperSpec *espec,
                 const tenModel *model,
                 const Nrrd *_nB0,
                 const Nrrd *_nparm,
                 int keyValueSet) {
  static const char me[]="tenModelSimulate";
  double *ddwi, *parm, (*ins)(void *v, size_t I, double d);
  char *dwi;
  size_t szOut[NRRD_DIM_MAX], II, numSamp;
  const Nrrd *nB0,    /* B0 as doubles */
    *ndparm,          /* parm as doubles */
    *ndpparm,         /* parm as doubles, padded */
    *nparm;           /* final parm as doubles, padded, w/ correct B0 values */
  Nrrd *ntmp;         /* non-const pointer for working */
  airArray *mop;
  unsigned int gpsze, /* given parm size */
    ii;
  int useB0Img, needPad, axmap[NRRD_DIM_MAX];

  if (!(ndwi && espec && model /* _nB0 can be NULL */ && _nparm)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (!espec->imgNum) {
    biffAddf(TEN, "%s: given espec wants 0 images, unset?", me);
    return 1;
  }

  gpsze = _nparm->axis[0].size;
  if (model->parmNum - 1 == gpsze) {
    /* got one less than needed parm #, see if we got B0 */
    if (!_nB0) {
      biffAddf(TEN, "%s: got %u parms, need %u (for %s), "
               "but didn't get B0 vol",
               me, gpsze, model->parmNum, model->name);
      return 1;
    }
    useB0Img = AIR_TRUE;
    needPad = AIR_TRUE;
  } else if (model->parmNum != gpsze) {
    biffAddf(TEN, "%s: mismatch between getting %u parms, "
             "needing %u (for %s)\n",
             me, gpsze, model->parmNum, model->name);
    return 1;
  } else {
    /* model->parmNum == gpsze */
    needPad = AIR_FALSE;
    useB0Img = !!_nB0;
  }

  mop = airMopNew();
  /* get parms as doubles */
  if (nrrdTypeDouble == _nparm->type) {
    ndparm = _nparm;
  } else {
    ntmp = nrrdNew();
    airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdConvert(ntmp, _nparm, nrrdTypeDouble)) {
      biffMovef(TEN, NRRD, "%s: couldn't convert parm to %s", me,
                airEnumStr(nrrdType, nrrdTypeDouble));
      airMopError(mop); return 1;
    }
    ndparm = ntmp;
  }
  /* get parms the right length */
  if (!needPad) {
    ndpparm = ndparm;
  } else {
    ptrdiff_t min[NRRD_DIM_MAX], max[NRRD_DIM_MAX];
    unsigned int ax;

    ntmp = nrrdNew();
    airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
    for (ax=0; ax<ndparm->dim; ax++) {
      min[ax] = (!ax ? -1 : 0);
      max[ax] = ndparm->axis[ax].size-1;
    }
    if (nrrdPad_nva(ntmp, ndparm, min, max, nrrdBoundaryBleed, 0.0)) {
      biffMovef(TEN, NRRD, "%s: couldn't pad", me);
      airMopError(mop); return 1;
    }
    ndpparm = ntmp;
  }
  /* put in B0 values if needed */
  if (!useB0Img) {
    nparm = ndpparm;
  } else {
    if (nrrdTypeDouble == _nB0->type) {
      nB0 = _nB0;
    } else {
      ntmp = nrrdNew();
      airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
      if (nrrdConvert(ntmp, _nB0, nrrdTypeDouble)) {
        biffMovef(TEN, NRRD, "%s: couldn't convert B0 to %s", me,
                  airEnumStr(nrrdType, nrrdTypeDouble));
        airMopError(mop); return 1;
      }
      nB0 = ntmp;
    }
    /* HEY: this is mostly likely a waste of memory,
       but its all complicated by const-correctness */
    ntmp = nrrdNew();
    airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdSplice(ntmp, ndpparm, nB0, 0, 0)) {
      biffMovef(TEN, NRRD, "%s: couldn't splice in B0", me);
      airMopError(mop); return 1;
    }
    nparm = ntmp;
  }

  /* allocate output (and set axmap) */
  for (ii=0; ii<nparm->dim; ii++) {
    szOut[ii] = (!ii
                 ? espec->imgNum
                 : nparm->axis[ii].size);
    axmap[ii] = (!ii
                 ? -1
                 : AIR_CAST(int, ii));
  }
  if (nrrdMaybeAlloc_nva(ndwi, typeOut, nparm->dim, szOut)) {
    biffMovef(TEN, NRRD, "%s: couldn't allocate output", me);
    airMopError(mop); return 1;
  }
  if (!( ddwi = AIR_CALLOC(espec->imgNum, double))) {
    biffAddf(TEN, "%s: couldn't allocate dwi buffer", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, ddwi, airFree, airMopAlways);
  numSamp = nrrdElementNumber(nparm)/nparm->axis[0].size;

  /* set output */
  ins = nrrdDInsert[typeOut];
  parm = AIR_CAST(double *, nparm->data);
  dwi = AIR_CAST(char *, ndwi->data);
  for (II=0; II<numSamp; II++) {
    model->simulate(ddwi, parm, espec);
    for (ii=0; ii<espec->imgNum; ii++) {
      ins(dwi, ii, ddwi[ii]);
    }
    parm += model->parmNum;
    dwi += espec->imgNum*nrrdTypeSize[typeOut];
  }

  if (keyValueSet) {
    if (tenDWMRIKeyValueFromExperSpecSet(ndwi, espec)) {
      biffAddf(TEN, "%s: trouble", me);
      airMopError(mop); return 1;
    }
  }

  if (nrrdAxisInfoCopy(ndwi, _nparm, axmap, NRRD_AXIS_INFO_SIZE_BIT)
      || nrrdBasicInfoCopy(ndwi, _nparm,
                           NRRD_BASIC_INFO_DATA_BIT
                           | NRRD_BASIC_INFO_TYPE_BIT
                           | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                           | NRRD_BASIC_INFO_DIMENSION_BIT
                           | NRRD_BASIC_INFO_CONTENT_BIT
                           | NRRD_BASIC_INFO_COMMENTS_BIT
                           | (nrrdStateKeyValuePairsPropagate
                              ? 0
                              : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    biffMovef(TEN, NRRD, "%s: couldn't copy axis or basic info", me);
    airMopError(mop); return 1;
  }
  ndwi->axis[0].kind = nrrdKindList;

  airMopOkay(mop);
  return 0;
}

/*
** _tenModelSqeFitSingle
**
** callable function (as opposed to tenModel method) for doing
** sqe fitting.  Returns the sqe at the converged fit location
** Requires PARM_NUM length buffers testParm and grad
*/
double
_tenModelSqeFitSingle(const tenModel *model,
                      double *testParm, double *grad,
                      double *parm, double *convFrac, unsigned int *itersTaken,
                      const tenExperSpec *espec,
                      double *dwiBuff, const double *dwiMeas,
                      const double *parmInit, int knownB0,
                      unsigned int minIter, unsigned int maxIter,
                      double convEps, int verbose) {
  static const char me[]="_tenModelSqeFitSingle";
  unsigned int iter, subIter;
  double step, bak, opp, val, testval, dist, td;
  int done;
  char pstr[AIR_STRLEN_MED];

  step = 1;
  model->copy(parm, parmInit);
  val = model->sqe(parm, espec, dwiBuff, dwiMeas, knownB0);
  model->sqeGrad(grad, parm, espec, dwiBuff, dwiMeas, knownB0);
  if (verbose > 1) {
    model->sprint(pstr, parm);
    fprintf(stderr, "\n");
    fprintf(stderr, "%s(%s): minIter = %u, maxIter = %u\n", me, model->name,
            minIter, maxIter);
    fprintf(stderr, "%s(%s): starting at %s -> %g (step %g)\n", me,
            model->name, pstr, val, step);
  }

  opp = 1.2;  /* opportunistic step size increase */
  bak = 0.5;  /* scaling back because of bad step */
  iter = 0;
  dist = convEps*8;
  do {
    subIter = 0;
    do {
      model->step(testParm, -step, grad, parm);
      testval = model->sqe(testParm, espec, dwiBuff, dwiMeas, knownB0);
      if (verbose > 1) {
        model->sprint(pstr, testParm);
        fprintf(stderr, "%s(%s): (iter %u/%u) tried %s -> %g (step %g)\n",
                me, model->name, iter, subIter, pstr, testval, step);
      }
      if (testval > val) {
        step *= bak;
      }
      subIter++;
    } while (testval > val && subIter <= maxIter);
    if (subIter > maxIter) {
      /* something went wrong with merely trying to find a downhill step;
         this has occurred previously when (because of a bug) the
         per-parameter bounds put the test location inside the bounding
         box while the initial location was outside => could never converge.
         Not using biff, so this is one way of trying to signal the problem */
      model->copy(parm, parmInit);
      *convFrac = AIR_POS_INF;
      *itersTaken = maxIter;
      return AIR_POS_INF;
    }
    td = model->dist(testParm, parm);
    dist = (td + dist)/2;
    val = testval;
    model->copy(parm, testParm);
    model->sqeGrad(grad, parm, espec, dwiBuff, dwiMeas, knownB0);
    step *= opp;
    iter++;
    done = (iter < minIter
            ? AIR_FALSE
            : (iter > maxIter) || dist < convEps);
  } while (!done);
  *convFrac = dist/convEps;
  *itersTaken = iter;
  return val;
}

int
tenModelSqeFit(Nrrd *nparm,
               Nrrd **nsqeP, Nrrd **nconvP, Nrrd **niterP,
               const tenModel *model,
               const tenExperSpec *espec, const Nrrd *ndwi,
               int knownB0, int saveB0, int typeOut,
               unsigned int minIter, unsigned int maxIter,
               unsigned int starts, double convEps,
               airRandMTState *_rng, int verbose) {
  static const char me[]="tenModelSqeFit";
  char doneStr[13];
  double *ddwi, *dwibuff, sqe, sqeBest,
    *dparm, *dparmBest,
    (*ins)(void *v, size_t I, double d),
    (*lup)(const void *v, size_t I);
  airArray *mop;
  unsigned int saveParmNum, dwiNum, ii, lablen, itersTaken;
  size_t szOut[NRRD_DIM_MAX], II, numSamp;
  int axmap[NRRD_DIM_MAX], erraxmap[NRRD_DIM_MAX], fitVerbose;
  const char *dwi;
  char *parm;
  airRandMTState *rng;
  Nrrd *nsqe, *nconv, *niter;

  /* nsqeP, nconvP, niterP can be NULL */
  if (!( nparm && model && espec && ndwi )) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( starts > 0 )) {
    biffAddf(TEN, "%s: need non-zero starts", me);
    return 1;
  }
  if (!( nrrdTypeFloat == typeOut || nrrdTypeDouble == typeOut )) {
    biffAddf(TEN, "%s: typeOut must be %s or %s, not %s", me,
             airEnumStr(nrrdType, nrrdTypeFloat),
             airEnumStr(nrrdType, nrrdTypeDouble),
             airEnumStr(nrrdType, typeOut));
    return 1;
  }
  dwiNum = ndwi->axis[0].size;
  if (espec->imgNum != dwiNum) {
    biffAddf(TEN, "%s: espec expects %u images but dwi has %u on axis 0",
             me, espec->imgNum, AIR_CAST(unsigned int, dwiNum));
    return 1;
  }

  /* allocate output (and set axmap) */
  dparm = model->alloc();
  dparmBest = model->alloc();
  if (!( dparm && dparmBest )) {
    biffAddf(TEN, "%s: couldn't allocate parm vecs", me);
    return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, dparm, airFree, airMopAlways);
  airMopAdd(mop, dparmBest, airFree, airMopAlways);
  saveParmNum = saveB0 ? model->parmNum : model->parmNum-1;
  for (ii=0; ii<ndwi->dim; ii++) {
    szOut[ii] = (!ii
                 ? saveParmNum
                 : ndwi->axis[ii].size);
    axmap[ii] = (!ii
                 ? -1
                 : AIR_CAST(int, ii));
    if (ii) {
      erraxmap[ii-1] = AIR_CAST(int, ii);
    }
  }
  if (nrrdMaybeAlloc_nva(nparm, typeOut, ndwi->dim, szOut)) {
    biffMovef(TEN, NRRD, "%s: couldn't allocate output "
              "(saveB0 %d, knownB0 %d)", me, saveB0, knownB0);
    airMopError(mop); return 1;
  }
  if (nsqeP) {
    nsqe = *nsqeP;
    if (!nsqe) {
      nsqe = nrrdNew();
      *nsqeP = nsqe;
    }
    if (nrrdMaybeAlloc_nva(nsqe, typeOut, ndwi->dim-1, szOut+1)) {
      biffMovef(TEN, NRRD, "%s: couldn't allocate error output", me);
      airMopError(mop); return 1;
    }
  } else {
    nsqe = NULL;
  }
  if (nconvP) {
    nconv = *nconvP;
    if (!nconv) {
      nconv = nrrdNew();
      *nconvP = nconv;
    }
    if (nrrdMaybeAlloc_nva(nconv, nrrdTypeDouble, ndwi->dim-1, szOut+1)) {
      biffMovef(TEN, NRRD, "%s: couldn't allocate conv output", me);
      airMopError(mop); return 1;
    }
  } else {
    nconv = NULL;
  }
  if (niterP) {
    niter = *niterP;
    if (!niter) {
      niter = nrrdNew();
      *niterP = niter;
    }
    if (nrrdMaybeAlloc_nva(niter, nrrdTypeUInt, ndwi->dim-1, szOut+1)) {
      biffMovef(TEN, NRRD, "%s: couldn't allocate iter output", me);
      airMopError(mop); return 1;
    }
  } else {
    niter = NULL;
  }
  ddwi = AIR_CALLOC(espec->imgNum, double);
  dwibuff = AIR_CALLOC(espec->imgNum, double);
  if (!(ddwi && dwibuff)) {
    biffAddf(TEN, "%s: couldn't allocate dwi buffers", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, ddwi, airFree, airMopAlways);
  airMopAdd(mop, dwibuff, airFree, airMopAlways);

  /* set output */
  if (_rng) {
    rng = _rng;
  } else {
    airRandMTStateGlobalInit();
    rng = airRandMTStateGlobal;
  }
  numSamp = nrrdElementNumber(ndwi)/ndwi->axis[0].size;
  lup = nrrdDLookup[ndwi->type];
  ins = nrrdDInsert[typeOut];
  parm = AIR_CAST(char *, nparm->data);
  dwi = AIR_CAST(char *, ndwi->data);
  itersTaken = 0;
  if (verbose) {
    fprintf(stderr, "%s: fitting ...       ", me);
    fflush(stderr);
  }
  for (II=0; II<numSamp; II++) {
    double cvf, convFrac=0;
    unsigned int ss, itak;
    if (verbose) {
      fprintf(stderr, "%s", airDoneStr(0, II, numSamp, doneStr));
      fflush(stderr);
    }
    for (ii=0; ii<dwiNum; ii++) {
      ddwi[ii] = lup(dwi, ii);
    }
    sqeBest = DBL_MAX; /* forces at least one improvement */
    for (ss=0; ss<starts; ss++) {
      /* can add other debugging conditions here */
      fitVerbose = verbose;
      if (knownB0) {
        dparm[0] = tenExperSpecKnownB0Get(espec, ddwi);
      }
      model->rand(dparm, rng, knownB0);
      sqe = model->sqeFit(dparm, &cvf, &itak,
                          espec, dwibuff, ddwi,
                          dparm, knownB0, minIter, maxIter,
                          convEps, fitVerbose);
      if (sqe <= sqeBest) {
        sqeBest = sqe;
        model->copy(dparmBest, dparm);
        itersTaken = itak;
        convFrac = cvf;
      }
    }
    for (ii=0; ii<saveParmNum; ii++) {
      ins(parm, ii, saveB0 ? dparmBest[ii] : dparmBest[ii+1]);
    }
    /* save things about fitting into nrrds */
    if (nsqeP) {
      ins(nsqe->data, II, sqeBest);
    }
    if (nconvP) {
      nrrdDInsert[nrrdTypeDouble](nconv->data, II, convFrac);
    }
    if (niterP) {
      nrrdDInsert[nrrdTypeUInt](niter->data, II, itersTaken);
    }
    parm += saveParmNum*nrrdTypeSize[typeOut];
    dwi += espec->imgNum*nrrdTypeSize[ndwi->type];
  }
  if (verbose) {
    fprintf(stderr, "%s\n", airDoneStr(0, II, numSamp, doneStr));
  }

  if (nrrdAxisInfoCopy(nparm, ndwi, axmap, NRRD_AXIS_INFO_SIZE_BIT)
      || nrrdBasicInfoCopy(nparm, ndwi,
                           NRRD_BASIC_INFO_DATA_BIT
                           | NRRD_BASIC_INFO_TYPE_BIT
                           | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                           | NRRD_BASIC_INFO_DIMENSION_BIT
                           | NRRD_BASIC_INFO_CONTENT_BIT
                           | NRRD_BASIC_INFO_COMMENTS_BIT
                           | (nrrdStateKeyValuePairsPropagate
                              ? 0
                              : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    biffMovef(TEN, NRRD, "%s: couldn't copy axis or basic info", me);
    airMopError(mop); return 1;
  }
  if (nsqeP) {
    if (nrrdAxisInfoCopy(nsqe, ndwi, erraxmap, NRRD_AXIS_INFO_SIZE_BIT)
        || nrrdBasicInfoCopy(nsqe, ndwi,
                             NRRD_BASIC_INFO_DATA_BIT
                             | NRRD_BASIC_INFO_TYPE_BIT
                             | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                             | NRRD_BASIC_INFO_DIMENSION_BIT
                             | NRRD_BASIC_INFO_CONTENT_BIT
                             | NRRD_BASIC_INFO_COMMENTS_BIT
                             | (nrrdStateKeyValuePairsPropagate
                                ? 0
                                : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
      biffMovef(TEN, NRRD,
                "%s: couldn't copy axis or basic info to error out", me);
      airMopError(mop); return 1;
    }
  }
  if (nconvP) {
    if (nrrdAxisInfoCopy(nconv, ndwi, erraxmap, NRRD_AXIS_INFO_SIZE_BIT)
        || nrrdBasicInfoCopy(nconv, ndwi,
                             NRRD_BASIC_INFO_DATA_BIT
                             | NRRD_BASIC_INFO_TYPE_BIT
                             | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                             | NRRD_BASIC_INFO_DIMENSION_BIT
                             | NRRD_BASIC_INFO_CONTENT_BIT
                             | NRRD_BASIC_INFO_COMMENTS_BIT
                             | (nrrdStateKeyValuePairsPropagate
                                ? 0
                                : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
      biffMovef(TEN, NRRD,
                "%s: couldn't copy axis or basic info to conv out", me);
      airMopError(mop); return 1;
    }
  }
  if (niterP) {
    if (nrrdAxisInfoCopy(niter, ndwi, erraxmap, NRRD_AXIS_INFO_SIZE_BIT)
        || nrrdBasicInfoCopy(niter, ndwi,
                             NRRD_BASIC_INFO_DATA_BIT
                             | NRRD_BASIC_INFO_TYPE_BIT
                             | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                             | NRRD_BASIC_INFO_DIMENSION_BIT
                             | NRRD_BASIC_INFO_CONTENT_BIT
                             | NRRD_BASIC_INFO_COMMENTS_BIT
                             | (nrrdStateKeyValuePairsPropagate
                                ? 0
                                : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
      biffMovef(TEN, NRRD,
                "%s: couldn't copy axis or basic info to iter out", me);
      airMopError(mop); return 1;
    }
  }
  lablen = (strlen(tenModelPrefixStr)
            + (saveB0 ? strlen("B0+") : 0)
            + strlen(model->name)
            + 1);
  nparm->axis[0].label = AIR_CALLOC(lablen, char);
  sprintf(nparm->axis[0].label, "%s%s%s",
          tenModelPrefixStr,
          saveB0 ? "B0+" : "",
          model->name);

  airMopOkay(mop);
  return 0;
}

int
tenModelNllFit(Nrrd *nparm, Nrrd **nnllP,
               const tenModel *model,
               const tenExperSpec *espec, const Nrrd *ndwi,
               int rician, double sigma, int knownB0) {

  AIR_UNUSED(nparm);
  AIR_UNUSED(nnllP);
  AIR_UNUSED(model);
  AIR_UNUSED(espec);
  AIR_UNUSED(ndwi);
  AIR_UNUSED(rician);
  AIR_UNUSED(sigma);
  AIR_UNUSED(knownB0);

  return 0;
}

/*
** copy the B0 info if we have it
** use the same type on the way out.
*/
int
tenModelConvert(Nrrd *nparmDst, int *convRetP, const tenModel *modelDst,
                const Nrrd *nparmSrc, const tenModel *_modelSrc) {
  static char me[]="tenModelConvert";
  const tenModel *modelSrc;
  double *dpdst, *dpsrc, (*lup)(const void *v, size_t I),
    (*ins)(void *v, size_t I, double d);
  size_t szOut[NRRD_DIM_MAX], II, NN, tsize;
  airArray *mop;
  int withB0, axmap[NRRD_DIM_MAX], convRet=0;
  unsigned int parmNumDst, parmNumSrc, ii, lablen;
  const char *parmSrc;
  char *parmDst;

  if (!( nparmDst && modelDst && nparmSrc )) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (!_modelSrc) {
    /* we have to try to learn the source model from the nrrd */
    if (tenModelFromAxisLearn(&modelSrc, &withB0, nparmSrc->axis + 0)) {
      biffAddf(TEN, "%s: couldn't learn model from src nparm", me);
      return 1;
    }
  } else {
    modelSrc = _modelSrc;
    if (modelSrc->parmNum == nparmSrc->axis[0].size) {
      withB0 = AIR_TRUE;
    } if (modelSrc->parmNum-1 == nparmSrc->axis[0].size) {
      withB0 = AIR_FALSE;
    } else {
      biffAddf(TEN, "%s: axis[0].size %u is not \"%s\" parmnum %u or 1 less",
               me, AIR_CAST(unsigned int, nparmSrc->axis[0].size),
               modelSrc->name, modelSrc->parmNum);
      return 1;
    }
  }

  mop = airMopNew();
  dpdst = modelDst->alloc();
  airMopAdd(mop, dpdst, airFree, airMopAlways);
  dpsrc = modelSrc->alloc();
  airMopAdd(mop, dpsrc, airFree, airMopAlways);
  lup = nrrdDLookup[nparmSrc->type];
  ins = nrrdDInsert[nparmSrc->type];
  parmNumDst = withB0 ? modelDst->parmNum : modelDst->parmNum-1;
  parmNumSrc = nparmSrc->axis[0].size;
  for (ii=0; ii<nparmSrc->dim; ii++) {
    szOut[ii] = (!ii
                 ? parmNumDst
                 : nparmSrc->axis[ii].size);
    axmap[ii] = (!ii
                 ? -1
                 : AIR_CAST(int, ii));
  }
  if (nrrdMaybeAlloc_nva(nparmDst, nparmSrc->type, nparmSrc->dim, szOut)) {
    biffMovef(TEN, NRRD, "%s: couldn't allocate output", me);
    airMopError(mop); return 1;
  }

  NN = nrrdElementNumber(nparmSrc)/nparmSrc->axis[0].size;
  tsize = nrrdTypeSize[nparmSrc->type];
  parmSrc = AIR_CAST(char *, nparmSrc->data);
  parmDst = AIR_CAST(char *, nparmDst->data);
  if (!withB0) {
    dpsrc[0] = 0;
  }
  for (II=0; II<NN; II++) {
    for (ii=0; ii<parmNumSrc; ii++) {
      dpsrc[withB0 ? ii : ii+1] = lup(parmSrc, ii);
    }
    convRet = modelDst->convert(dpdst, dpsrc, modelSrc);
    if (2 == convRet) {  /* HEY should be enum for this value */
      biffAddf(TEN, "%s: error converting from \"%s\" to \"%s\"", me,
               modelSrc->name, modelDst->name);
      airMopError(mop); return 1;
    }
    for (ii=0; ii<parmNumDst; ii++) {
      ins(parmDst, ii, dpdst[withB0 ? ii : ii+1]);
    }
    parmSrc += parmNumSrc*tsize;
    parmDst += parmNumDst*tsize;
  }
  if (convRetP) {
    *convRetP = convRet;
  }

  if (nrrdAxisInfoCopy(nparmDst, nparmSrc, axmap, NRRD_AXIS_INFO_SIZE_BIT)
      || nrrdBasicInfoCopy(nparmDst, nparmSrc,
                           NRRD_BASIC_INFO_DATA_BIT
                           | NRRD_BASIC_INFO_TYPE_BIT
                           | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                           | NRRD_BASIC_INFO_DIMENSION_BIT
                           | NRRD_BASIC_INFO_CONTENT_BIT
                           | NRRD_BASIC_INFO_COMMENTS_BIT
                           | (nrrdStateKeyValuePairsPropagate
                              ? 0
                              : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    biffMovef(TEN, NRRD, "%s: couldn't copy axis or basic info", me);
    airMopError(mop); return 1;
  }
  /* HEY: COPY AND PASTE! from above. perhaps make helper functions? */
  lablen = (strlen(tenModelPrefixStr)
            + (withB0 ? strlen("B0+") : 0)
            + strlen(modelDst->name)
            + 1);
  nparmDst->axis[0].label = AIR_CALLOC(lablen, char);
  sprintf(nparmDst->axis[0].label, "%s%s%s",
          tenModelPrefixStr,
          withB0 ? "B0+" : "",
          modelDst->name);

  airMopOkay(mop);
  return 0;
}
