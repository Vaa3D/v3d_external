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


#include "pull.h"
#include "privatePull.h"

pullTrace *
pullTraceNew(void) {
  pullTrace *ret;

  ret = AIR_CALLOC(1, pullTrace);
  if (ret) {
    ret->seedPos[0] = ret->seedPos[1] = AIR_NAN;
    ret->seedPos[2] = ret->seedPos[3] = AIR_NAN;
    ret->nvert = nrrdNew();
    ret->nstrn = nrrdNew();
    ret->nvelo = nrrdNew();
    ret->seedIdx = 0;
    ret->whyStop[0] = ret->whyStop[1] = pullTraceStopUnknown;
    ret->whyNowhere = pullTraceStopUnknown;
  }
  return ret;
}

pullTrace *
pullTraceNix(pullTrace *pts) {

  if (pts) {
    nrrdNuke(pts->nvert);
    nrrdNuke(pts->nstrn);
    nrrdNuke(pts->nvelo);
    free(pts);
  }
  return NULL;
}


int
pullTraceSet(pullContext *pctx, pullTrace *pts,
             int recordStrength,
             double scaleDelta, double halfScaleWin,
             double velocityMax, unsigned int arrIncr,
             const double seedPos[4]) {
  static const char me[]="pullTraceSet";
  pullPoint *point;
  airArray *mop, *trceArr[2], *hstrnArr[2];
  double *trce[2], ssrange[2], *vert, *hstrn[2], *strn, *velo, travmax;
  int constrFail;
  unsigned int dirIdx, lentmp, tidx, oidx, vertNum;

  if (!( pctx && pts && seedPos )) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( AIR_EXISTS(scaleDelta) && scaleDelta > 0.0 )) {
    biffAddf(PULL, "%s: need existing scaleDelta > 0 (not %g)",
             me, scaleDelta);
    return 1;
  }
  if (!( halfScaleWin > 0 )) {
    biffAddf(PULL, "%s: need halfScaleWin > 0", me);
    return 1;
  }
  if (!(pctx->constraint)) {
    biffAddf(PULL, "%s: given context doesn't have constraint set", me);
    return 1;
  }
  if (recordStrength && !pctx->ispec[pullInfoStrength]) {
    biffAddf(PULL, "%s: want to record strength but %s not set in context",
             me, airEnumStr(pullInfo, pullInfoStrength));
    return 1;
  }
  if (pullConstraintScaleRange(pctx, ssrange)) {
    biffAddf(PULL, "%s: trouble getting scale range", me);
    return 1;
  }

  /* re-initialize termination descriptions (in case of trace re-use) */
  pts->whyStop[0] = pts->whyStop[1] = pullTraceStopUnknown;
  pts->whyNowhere = pullTraceStopUnknown;

  /* save seedPos in any case */
  ELL_4V_COPY(pts->seedPos, seedPos);

  mop = airMopNew();
  point = pullPointNew(pctx); /* we'll want to decrement idtagNext later */
  airMopAdd(mop, point, (airMopper)pullPointNix, airMopAlways);

  ELL_4V_COPY(point->pos, seedPos);
  if (_pullConstraintSatisfy(pctx->task[0], point, 2, &constrFail)) {
    biffAddf(PULL, "%s: constraint sat on seed point", me);
    airMopError(mop);
    return 1;
  }
  /*
  fprintf(stderr, "!%s: seed=(%g,%g,%g,%g) -> %s (%g,%g,%g,%g)\n", me,
          seedPos[0], seedPos[1], seedPos[2], seedPos[3],
          constrFail ? "!NO!" : "(yes)",
          point->pos[0] - seedPos[0], point->pos[1] - seedPos[1],
          point->pos[2] - seedPos[2], point->pos[3] - seedPos[3]);
  */
  if (constrFail) {
    pts->whyNowhere = pullTraceStopConstrFail;
    airMopOkay(mop);
    pctx->idtagNext -= 1; /* HACK */
    return 0;
  }

  /* else constraint sat worked at seed point; we have work to do */
  /* travmax is passed to _pullConstraintSatisfy; the intention is
     that constraint satisfaction should not fail because the point
     traveled too far (so make travmax large); the limiting of the
     trace based on velocity is handled here, not by
     _pullConstraintSatisfy */
  travmax = 10.0*scaleDelta*velocityMax/pctx->voxelSizeSpace;

  for (dirIdx=0; dirIdx<2; dirIdx++) {
    trceArr[dirIdx] = airArrayNew((void**)(trce + dirIdx), NULL,
                                  4*sizeof(double), arrIncr);
    airMopAdd(mop, trceArr[dirIdx], (airMopper)airArrayNuke, airMopAlways);
    if (recordStrength) {
      hstrnArr[dirIdx] = airArrayNew((void**)(hstrn + dirIdx), NULL,
                                     sizeof(double), arrIncr);
      airMopAdd(mop, hstrnArr[dirIdx], (airMopper)airArrayNuke, airMopAlways);
    } else {
      hstrnArr[dirIdx] = NULL;
      hstrn[dirIdx] = NULL;
    }
  }
  for (dirIdx=0; dirIdx<2; dirIdx++) {
    unsigned int step;
    double dscl;
    dscl = (!dirIdx ? -1 : +1)*scaleDelta;
    step = 0;
    while (1) {
      if (!step) {
        /* first step in both directions requires special tricks */
        if (0 == dirIdx) {
          /* save constraint sat of seed point */
          tidx = airArrayLenIncr(trceArr[0], 1);
          ELL_4V_COPY(trce[0] + 4*tidx, point->pos);
          if (recordStrength) {
            tidx = airArrayLenIncr(hstrnArr[0], 1);
            hstrn[0][0] = pullPointScalar(pctx, point, pullInfoStrength,
                                          NULL, NULL);
          }
        } else {
          /* re-set position from constraint sat of seed pos */
          ELL_4V_COPY(point->pos, trce[0] + 4*0);
        }
      }
      /* nudge position along scale */
      point->pos[3] += dscl;
      if (!AIR_IN_OP(ssrange[0], point->pos[3], ssrange[1])) {
        /* if we've stepped outside the range of scale for the volume
           containing the constraint manifold, we're done */
        pts->whyStop[dirIdx] = pullTraceStopBounds;
        break;
      }
      if (AIR_ABS(point->pos[3] - seedPos[3]) > halfScaleWin) {
        /* we've moved along scale as far as allowed */
        pts->whyStop[dirIdx] = pullTraceStopLength;
        break;
      }
      /* re-assert constraint */
      /*
      fprintf(stderr, "%s(%u): pos = %g %g %g %g.... \n", me,
              point->idtag, point->pos[0], point->pos[1],
              point->pos[2], point->pos[3]);
      */
      if (_pullConstraintSatisfy(pctx->task[0], point,
                                 travmax, &constrFail)) {
        biffAddf(PULL, "%s: dir %u, step %u", me, dirIdx, step);
        airMopError(mop);
        return 1;
      }
      /*
      fprintf(stderr, "%s(%u): ... %s(%d); pos = %g %g %g %g\n", me,
              point->idtag,
              constrFail ? "FAIL" : "(ok)",
              constrFail, point->pos[0], point->pos[1],
              point->pos[2], point->pos[3]);
      */
      if (constrFail) {
        /* constraint sat failed; no error, we're just done
           with stepping for this direction */
        pts->whyStop[dirIdx] = pullTraceStopConstrFail;
        break;
      }
      if (trceArr[dirIdx]->len >= 2) {
        /* see if we're moving too fast, by comparing with previous point */
        double pos0[3], pos1[3], diff[3], vv;
        unsigned int ii;

        ii = trceArr[dirIdx]->len-2;
        ELL_3V_COPY(pos0, trce[dirIdx] + 4*(ii+0));
        ELL_3V_COPY(pos1, trce[dirIdx] + 4*(ii+1));
        ELL_3V_SUB(diff, pos1, pos0);
        vv = ELL_3V_LEN(diff)/scaleDelta;
        /*
        fprintf(stderr, "%s(%u): velo %g %s velocityMax %g => %s\n", me,
                point->idtag, vv,
                vv > velocityMax ? ">" : "<=",
                velocityMax,
                vv > velocityMax ? "FAIL" : "(ok)");
        */
        if (vv > velocityMax) {
          pts->whyStop[dirIdx] = pullTraceStopSpeeding;
          break;
        }
      }
      /* else save new point on trace */
      tidx = airArrayLenIncr(trceArr[dirIdx], 1);
      ELL_4V_COPY(trce[dirIdx] + 4*tidx, point->pos);
      if (recordStrength) {
        tidx = airArrayLenIncr(hstrnArr[dirIdx], 1);
        hstrn[dirIdx][tidx] = pullPointScalar(pctx, point, pullInfoStrength,
                                              NULL, NULL);
      }
      step++;
    }
  }

  /* transfer trace halves to pts->nvert */
  vertNum = trceArr[0]->len + trceArr[1]->len;
  if (0 == vertNum || 1 == vertNum || 2 == vertNum) {
    pts->whyNowhere = pullTraceStopStub;
    airMopOkay(mop);
    pctx->idtagNext -= 1; /* HACK */
    return 0;
  }

  if (nrrdMaybeAlloc_va(pts->nvert, nrrdTypeDouble, 2,
                        AIR_CAST(size_t, 4),
                        AIR_CAST(size_t, vertNum))
      || nrrdMaybeAlloc_va(pts->nvelo, nrrdTypeDouble, 1,
                           AIR_CAST(size_t, vertNum))) {
    biffMovef(PULL, NRRD, "%s: allocating output", me);
    airMopError(mop);
    return 1;
  }
  if (recordStrength) {
    if (nrrdSlice(pts->nstrn, pts->nvert, 0 /* axis */, 0 /* pos */)) {
      biffMovef(PULL, NRRD, "%s: allocating output", me);
      airMopError(mop);
      return 1;
    }
  }
  vert = AIR_CAST(double *, pts->nvert->data);
  if (recordStrength) {
    strn = AIR_CAST(double *, pts->nstrn->data);
  } else {
    strn = NULL;
  }
  velo = AIR_CAST(double *, pts->nvelo->data);
  lentmp = trceArr[0]->len;
  oidx = 0;
  for (tidx=0; tidx<lentmp; tidx++) {
    ELL_4V_COPY(vert + 4*oidx, trce[0] + 4*(lentmp - 1 - tidx));
    if (strn) {
      strn[oidx] = hstrn[0][lentmp - 1 - tidx];
    }
    oidx++;
  }
  /* the last index written to (before oidx++) was the seed index */
  pts->seedIdx = oidx-1;
  lentmp = trceArr[1]->len;
  for (tidx=0; tidx<lentmp; tidx++) {
    ELL_4V_COPY(vert + 4*oidx, trce[1] + 4*tidx);
    if (strn) {
      strn[oidx] = hstrn[1][tidx];
    }
    oidx++;
  }
  lentmp = pts->nvelo->axis[0].size;
  if (1 == lentmp) {
    velo[0] = 0.0;
  } else {
    for (tidx=0; tidx<lentmp; tidx++) {
      double *p0, *p1, *p2, diff[3];
      if (!tidx) {
        /* first */
        p1 = vert + 4*tidx;
        p2 = vert + 4*(tidx+1);
        ELL_3V_SUB(diff, p2, p1);
        velo[tidx] = ELL_3V_LEN(diff)/(p2[3]-p1[3]);
      } else if (tidx < lentmp-1) {
        /* middle */
        p0 = vert + 4*(tidx-1);
        p2 = vert + 4*(tidx+1);
        ELL_3V_SUB(diff, p2, p0);
        velo[tidx] = ELL_3V_LEN(diff)/(p2[3]-p0[3]);
      } else {
        /* last */
        p0 = vert + 4*(tidx-1);
        p1 = vert + 4*tidx;
        ELL_3V_SUB(diff, p1, p0);
        velo[tidx] = ELL_3V_LEN(diff)/(p1[3]-p0[3]);
      }
    }
  }

  airMopOkay(mop);
  pctx->idtagNext -= 1; /* HACK */
  return 0;
}

typedef union {
  pullTrace ***trace;
  void **v;
} blahblahUnion;

pullTraceMulti *
pullTraceMultiNew(void) {
  /* static const char me[]="pullTraceMultiNew"; */
  pullTraceMulti *ret;
  blahblahUnion bbu;

  ret = AIR_CALLOC(1, pullTraceMulti);
  if (ret) {
    ret->trace = NULL;
    ret->traceNum = 0;
    ret->traceArr = airArrayNew((bbu.trace = &(ret->trace), bbu.v),
                                &(ret->traceNum), sizeof(pullTrace*),
                                _PULL_TRACE_MULTI_INCR);
    airArrayPointerCB(ret->traceArr,
                      NULL, /* because we get handed pullTrace structs
                               that have already been allocated
                               (and then we own them) */
                      (void *(*)(void *))pullTraceNix);
  }
  return ret;
}

int
pullTraceMultiAdd(pullTraceMulti *mtrc, pullTrace *trc, int *addedP) {
  static const char me[]="pullTraceMultiAdd";
  unsigned int indx;

  if (!(mtrc && trc && addedP)) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(trc->nvert->data && trc->nvert->axis[1].size >= 3)) {
    /*  for now getting a stub trace is not an error
    biffAddf(PULL, "%s: got stub trace", me);
    return 1; */
    *addedP = AIR_FALSE;
    return 0;
  }
  if (!(trc->nvelo->data
        && trc->nvelo->axis[0].size == trc->nvert->axis[1].size)) {
    biffAddf(PULL, "%s: velo data inconsistent", me);
    return 1;
  }
  *addedP = AIR_TRUE;
  indx = airArrayLenIncr(mtrc->traceArr, 1);
  if (!mtrc->trace) {
    biffAddf(PULL, "%s: alloc error", me);
    return 1;
  }
  mtrc->trace[indx] = trc;
  return 0;
}

int
pullTraceMultiFilterConcaveDown(Nrrd *nfilt, const pullTraceMulti *mtrc,
                                double winLenFrac) {
  static const char me[]="pullTraceMultiFilterConcaveDown";
  unsigned int ti;
  int *filt;

  if (!(nfilt && mtrc)) {
    biffAddf(PULL, "%s: got NULL pointer (%p %p)", me,
             AIR_VOIDP(nfilt), AIR_CVOIDP(mtrc));
    return 1;
  }
  if (!(AIR_EXISTS(winLenFrac) && AIR_IN_OP(0.0, winLenFrac, 1.0))) {
    biffAddf(PULL, "%s: winLenFrac %g doesn't exist or not in [0,1]",
             me, winLenFrac);
    return 1;
  }
  if (nrrdMaybeAlloc_va(nfilt, nrrdTypeInt, 1, mtrc->traceNum)) {
    biffMovef(PULL, NRRD, "%s: trouble creating output", me);
    return 1;
  }
  filt = AIR_CAST(int *, nfilt->data);
  for (ti=0; ti<mtrc->traceNum; ti++) {
    unsigned winLen;
    const pullTrace *trc;
    const double *velo;
    unsigned int schange, pidx, lentmp;
    double dv, dv0=0.0, rdv, dv1;

    trc = mtrc->trace[ti];
    lentmp = trc->nvert->axis[1].size;
    velo = AIR_CAST(const double *, trc->nvelo->data);
    winLen = AIR_CAST(unsigned int, winLenFrac*lentmp);
    if (winLen < 3) {
      continue;
    }
    schange = 0;
    rdv = 0.0;
    for (pidx=0; pidx<lentmp-1; pidx++) {
      /* normalizing by scaleDelta isn't needed for detecting sign changes */
      dv = velo[pidx+1] - velo[pidx];
      if (pidx < winLen) {
        rdv += dv;
      } else {
        double tmp;
        if (pidx == winLen) {
          dv0 = rdv;
        }
        tmp = rdv;
        rdv += dv;
        rdv -= velo[pidx-winLen+1] - velo[pidx-winLen];
        schange += (rdv*tmp < 0);
      }
    }
    dv1 = rdv;
    filt[ti] = (1 == schange) && dv0 < 0.0 && dv1 > 0.0;
  }
  return 0;
}

int
pullTraceMultiPlotAdd(Nrrd *nplot, const pullTraceMulti *mtrc,
                      const Nrrd *nfilt,
                      unsigned int trcIdxMin,unsigned int trcNum) {
  static const char me[]="pullTraceMultiPlot";
  double ssRange[2], vRange[2], velHalf, *plot;
  unsigned int sizeS, sizeV, trcIdx, trcIdxMax;
  int *filt;

  if (!(nplot && mtrc)) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (nrrdCheck(nplot)) {
    biffMovef(PULL, NRRD, "%s: trouble with nplot", me);
    return 1;
  }
  if (nfilt) {
    if (nrrdCheck(nfilt)) {
      biffMovef(PULL, NRRD, "%s: trouble with nfilt", me);
      return 1;
    }
    if (!(1 == nfilt->dim && nrrdTypeInt == nfilt->type)) {
      biffAddf(PULL, "%s: didn't get 1-D array of %s (got %u-D of %s)", me,
               airEnumStr(nrrdType, nrrdTypeInt), nfilt->dim,
               airEnumStr(nrrdType, nfilt->type));
      return 1;
    }
  }
  if (!(2 == nplot->dim && nrrdTypeDouble == nplot->type)) {
    biffAddf(PULL, "%s: didn't get 2-D array of %s (got %u-D of %s)", me,
             airEnumStr(nrrdType, nrrdTypeDouble), nplot->dim,
             airEnumStr(nrrdType, nplot->type));
    return 1;
  }
  if (!(trcIdxMin < mtrc->traceNum)) {
    biffAddf(PULL, "%s: trcIdxMin %u not < traceNum %u", me,
             trcIdxMin, mtrc->traceNum);
    return 1;
  }
  if (trcNum) {
    trcIdxMax = trcIdxMin + trcNum-1;
    if (!(trcIdxMax < mtrc->traceNum)) {
      biffAddf(PULL, "%s: trcIdxMax %u = %u+%u-1 not < traceNum %u", me,
               trcIdxMax, trcIdxMin, trcNum, mtrc->traceNum);
      return 1;
    }
  } else {
    trcIdxMax = mtrc->traceNum-1;
  }
  ssRange[0] = nplot->axis[0].min;
  ssRange[1] = nplot->axis[0].max;
  vRange[0] = nplot->axis[1].min;
  vRange[1] = nplot->axis[1].max;
  if (!( AIR_EXISTS(ssRange[0]) && AIR_EXISTS(ssRange[1]) &&
         AIR_EXISTS(vRange[0]) && AIR_EXISTS(vRange[1]) )) {
    biffAddf(PULL, "%s: need both axis 0 (%g,%g) and 1 (%g,%g) min,max", me,
             ssRange[0], ssRange[1], vRange[0], vRange[1]);
    return 1;
  }
  if (0 != vRange[0]) {
    biffAddf(PULL, "%s: expected vRange[0] == 0 not %g", me, vRange[0]);
    return 1;
  }
  /* HEY: this is a sneaky hack; the non-linear encoding of velocity along
     this axis means that the max velocity is actually infinite, but this
     seems like the least wrong way of storing this information in the place
     where it belongs (in the output plot nrrd) instead of assuming it will
     always be passed the same in successive calls */
  velHalf = vRange[1]/2.0;
  plot = AIR_CAST(double *, nplot->data);
  filt = (nfilt
          ? AIR_CAST(int *, nfilt->data)
          : NULL);
  sizeS = AIR_CAST(unsigned int, nplot->axis[0].size);
  sizeV = AIR_CAST(unsigned int, nplot->axis[1].size);
  for (trcIdx=trcIdxMin; trcIdx<=trcIdxMax; trcIdx++) {
    unsigned int pntIdx, pntNum;
    const pullTrace *trc;
    const double *vert, *velo;
    if (filt && !filt[trcIdx]) {
      continue;
    }
    trc = mtrc->trace[trcIdx];
    if (pullTraceStopStub == trc->whyNowhere) {
      continue;
    }
    vert = AIR_CAST(double *, trc->nvert->data);
    velo = AIR_CAST(double *, trc->nvelo->data);
    pntNum = trc->nvert->axis[1].size;
    for (pntIdx=0; pntIdx<pntNum; pntIdx++) {
      const double *pp;
      unsigned int sidx, vidx;
      pp = vert + 4*pntIdx;
      if (!(AIR_IN_OP(ssRange[0], pp[3], ssRange[1]))) {
        continue;
      }
      if (velo[pntIdx] <= 0.0) {
        continue;
      }
      sidx = airIndex(ssRange[0], pp[3], ssRange[1], sizeS);
      /* HEY weird that Clamp is needed, but it is, as this atan()
         does sometime return a negative value (?) */
      vidx = airIndexClamp(0.0, atan(velo[pntIdx]/velHalf), AIR_PI/2, sizeV);
      plot[sidx + sizeS*vidx] += 1;
    }
  }
  return 0;
}

static size_t
nsizeof(const Nrrd *nrrd) {
  return (nrrd
          ? nrrdElementSize(nrrd)*nrrdElementNumber(nrrd)
          : 0);
}

size_t
pullTraceMultiSizeof(const pullTraceMulti *mtrc) {
  size_t ret;
  unsigned int ti;

  if (!mtrc) {
    return 0;
  }
  ret = 0;
  for (ti=0; ti<mtrc->traceNum; ti++) {
    ret += sizeof(pullTrace);
    ret += nsizeof(mtrc->trace[ti]->nvert);
    ret += nsizeof(mtrc->trace[ti]->nstrn);
    ret += nsizeof(mtrc->trace[ti]->nvelo);
  }
  ret += sizeof(pullTrace*)*(mtrc->traceArr->size);
  return ret;
}

pullTraceMulti *
pullTraceMultiNix(pullTraceMulti *mtrc) {

  if (mtrc) {
    airArrayNuke(mtrc->traceArr);
    free(mtrc);
  }
  return NULL;
}


#define PULL_MTRC_MAGIC "PULLMTRC0001"
#define DEMARK_STR "======"

static int
tracewrite(FILE *file, const pullTrace *trc, unsigned int ti) {
  static const char me[]="tracewrite";

  fprintf(file, "%s %u\n", DEMARK_STR, ti);
  ell_4v_print_d(file, trc->seedPos);
#define WRITE(FF) \
  if (trc->FF && trc->FF->data) { \
    if (nrrdWrite(file, trc->FF, NULL)) { \
      biffMovef(PULL, NRRD, "%s: trouble with " #FF , me); \
      return 1; \
    } \
  } else { \
    fprintf(file, "NULL"); \
  } \
  fprintf(file, "\n")
  fprintf(file, "nrrds: vert strn velo = %d %d %d\n",
          trc->nvert && trc->nvert->data,
          trc->nstrn && trc->nstrn->data,
          trc->nvelo && trc->nvelo->data);
  WRITE(nvert);
  WRITE(nstrn);
  WRITE(nvelo);
  fprintf(file, "%u\n", trc->seedIdx);
  fprintf(file, "%s %s %s\n",
          airEnumStr(pullTraceStop, trc->whyStop[0]),
          airEnumStr(pullTraceStop, trc->whyStop[1]),
          airEnumStr(pullTraceStop, trc->whyNowhere));
#undef WRITE
  return 0;
}

int
pullTraceMultiWrite(FILE *file, const pullTraceMulti *mtrc) {
  static const char me[]="pullTraceMultiWrite";
  unsigned int ti;

  if (!(file && mtrc)) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  fprintf(file, "%s\n", PULL_MTRC_MAGIC);
  fprintf(file, "%u traces\n", mtrc->traceNum);

  for (ti=0; ti<mtrc->traceNum; ti++) {
    if (tracewrite(file, mtrc->trace[ti], ti)) {
      biffAddf(PULL, "%s: trace %u/%u", me, ti, mtrc->traceNum);
      return 1;
    }
  }
  return 0;
}

static int
traceread(pullTrace *trc, FILE *file, unsigned int _ti) {
  static const char me[]="traceread";
  char line[AIR_STRLEN_MED], name[AIR_STRLEN_MED];
  unsigned int ti, lineLen;
  int stops[3], hackhack, vertHN, strnHN, veloHN; /* HN == have nrrd */

  sprintf(name, "separator");
  lineLen = airOneLine(file, line, AIR_STRLEN_MED);
  if (!lineLen) {
    biffAddf(PULL, "%s: didn't get %s line", me, name);
    return 1;
  }
  if (1 != sscanf(line, DEMARK_STR " %u", &ti)) {
    biffAddf(PULL, "%s: \"%s\" doesn't look like %s line", me, line, name);
    return 1;
  }
  if (ti != _ti) {
    biffAddf(PULL, "%s: read trace index %u but wanted %u", me, ti, _ti);
    return 1;
  }
  sprintf(name, "seed pos");
  lineLen = airOneLine(file, line, AIR_STRLEN_MED);
  if (!lineLen) {
    biffAddf(PULL, "%s: didn't get %s line", me, name);
    return 1;
  }
  if (4 != sscanf(line, "%lg %lg %lg %lg", trc->seedPos + 0,
                  trc->seedPos + 1, trc->seedPos + 2, trc->seedPos + 3)) {
    biffAddf(PULL, "%s: couldn't parse %s line \"%s\" as 4 doubles",
             me, name, line);
    return 1;
  }
  sprintf(name, "have nrrds");
  lineLen = airOneLine(file, line, AIR_STRLEN_MED);
  if (!lineLen) {
    biffAddf(PULL, "%s: didn't get %s line", me, name);
    return 1;
  }
  if (3 != sscanf(line, "nrrds: vert strn velo = %d %d %d",
                  &vertHN, &strnHN, &veloHN)) {
    biffAddf(PULL, "%s: couldn't parse %s line", me, name);
    return 1;
  }
#define READ(FF) \
  if (FF##HN) {                         \
    if (nrrdRead(trc->n##FF, file, NULL)) {        \
      biffMovef(PULL, NRRD, "%s: trouble with " #FF , me); \
      return 1; \
    } \
    fgetc(file); \
  } else {                                      \
    airOneLine(file, line, AIR_STRLEN_MED); \
  }
  hackhack = nrrdStateVerboseIO;  /* should be fixed in Teem v2 */
  nrrdStateVerboseIO = 0;
  READ(vert);
  READ(strn);
  READ(velo);
  nrrdStateVerboseIO = hackhack;

  sprintf(name, "seed idx");
  lineLen = airOneLine(file, line, AIR_STRLEN_MED);
  if (!lineLen) {
    biffAddf(PULL, "%s: didn't get %s line", me, name);
    return 1;
  }
  if (1 != sscanf(line, "%u", &(trc->seedIdx))) {
    biffAddf(PULL, "%s: didn't parse uint from %s line \"%s\"",
             me, name, line);
    return 1;
  }
  sprintf(name, "stops");
  lineLen = airOneLine(file, line, AIR_STRLEN_MED);
  if (!lineLen) {
    biffAddf(PULL, "%s: didn't get %s line", me, name);
    return 1;
  }
  if (3 != airParseStrE(stops, line, " ", 3, pullTraceStop)) {
    biffAddf(PULL, "%s: didn't see 3 %s on %s line \"%s\"", me,
             pullTraceStop->name, name, line);
    return 1;
  }

  return 0;
}
int
pullTraceMultiRead(pullTraceMulti *mtrc, FILE *file) {
  static const char me[]="pullTraceMultiRead";
  char line[AIR_STRLEN_MED], name[AIR_STRLEN_MED];
  unsigned int lineLen, ti, tnum;
  pullTrace *trc;

  if (!(mtrc && file)) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  airArrayLenSet(mtrc->traceArr, 0);
  sprintf(name, "magic");
  lineLen = airOneLine(file, line, AIR_STRLEN_MED);
  if (!lineLen) {
    biffAddf(PULL, "%s: didn't get %s line", me, name);
    return 1;
  }
  if (strcmp(line, PULL_MTRC_MAGIC)) {
    biffAddf(PULL, "%s: %s line \"%s\" not expected \"%s\"",
             me, name, line, PULL_MTRC_MAGIC);
    return 1;
  }

  sprintf(name, "# of traces");
  lineLen = airOneLine(file, line, AIR_STRLEN_MED);
  if (!lineLen) {
    biffAddf(PULL, "%s: didn't get %s line", me, name);
    return 1;
  }
  if (1 != sscanf(line, "%u traces", &tnum)) {
    biffAddf(PULL, "%s: \"%s\" doesn't look like %s line", me, line, name);
    return 1;
  }
  for (ti=0; ti<tnum; ti++) {
    int added;
    trc = pullTraceNew();
    if (traceread(trc, file, ti)) {
      biffAddf(PULL, "%s: on trace %u/%u", me, ti, tnum);
      return 1;
    }
    if (pullTraceMultiAdd(mtrc, trc, &added)) {
      biffAddf(PULL, "%s: adding trace %u/%u", me, ti, tnum);
      return 1;
    }
    if (!added) {
      trc = pullTraceNix(trc);
    }
  }

  return 0;
}
