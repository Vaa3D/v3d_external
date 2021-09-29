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

/*
** HEY: this messes with the points' idtag, and pctx->idtagNext,
** even though it really shouldn't have to
*/
int
pullCCFind(pullContext *pctx) {
  static const char me[]="pullCCFind";
  airArray *mop, *eqvArr;
  unsigned int passIdx, binIdx, pointIdx, neighIdx, eqvNum,
    pointNum, *idmap;
  pullBin *bin;
  pullPoint *point, *her;

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (_pullIterate(pctx, pullProcessModeNeighLearn)) {
    biffAddf(PULL, "%s: trouble with %s for CC", me,
             airEnumStr(pullProcessMode, pullProcessModeNeighLearn));
    return 1;
  }

  mop = airMopNew();
  pointNum = pullPointNumber(pctx);
  eqvArr = airArrayNew(NULL, NULL, 2*sizeof(unsigned int), pointNum);
  airMopAdd(mop, eqvArr, (airMopper)airArrayNuke, airMopAlways);
  idmap = AIR_CAST(unsigned int *, calloc(pointNum, sizeof(unsigned int)));
  airMopAdd(mop, idmap, airFree, airMopAlways);

  /* to be safe, renumber all points, so that we know that the
     idtags are contiguous, starting at 0. HEY: this should handled
     by having a map from real point->idtag to a point number assigned
     just for the sake of doing CCs */
  pctx->idtagNext = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      point->idtag = pctx->idtagNext++;
    }
  }

  /* same stupidity copied from limn/polymod.c:limnPolyDataCCFind */
  eqvNum = 0;
  for (passIdx=0; passIdx<2; passIdx++) {
    if (passIdx) {
      airArrayLenPreSet(eqvArr, eqvNum);
    }
    for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
      bin = pctx->bin + binIdx;
      for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
        point = bin->point[pointIdx];
        for (neighIdx=0; neighIdx<point->neighPointNum; neighIdx++) {
          if (0 == passIdx) {
            ++eqvNum;
          } else {
            her = point->neighPoint[neighIdx];
            airEqvAdd(eqvArr, point->idtag, her->idtag);
          }
        }
      }
    }
  }

  /* do the CC analysis */
  pctx->CCNum = airEqvMap(eqvArr, idmap, pointNum);

  /* assign idcc's */
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      point->idCC = idmap[point->idtag];
    }
  }

  airMopOkay(mop);
  return 0;
}

/*
** measure something about connected componts already found
**
** measrInfo can be 0 to say "measure # particles in CC", or
** it can be a scalar pullInfo
**
** rho == 0: only size, rho == 1: only measrInfo
*/
int
pullCCMeasure(pullContext *pctx, Nrrd *nmeasr, int measrInfo, double rho) {
  static const char me[]="pullCCMeasure";
  airArray *mop;
  unsigned int binIdx, pointIdx, ii;
  double *meas, *size;
  pullBin *bin;
  pullPoint *point;

  if (!( pctx && nmeasr )) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!pctx->CCNum) {
    biffAddf(PULL, "%s: CCNum == 0: haven't yet learned CCs?", me);
    return 1;
  }
  if (measrInfo) {
    if (airEnumValCheck(pullInfo, measrInfo)) {
      biffAddf(PULL, "%s: measrInfo %d not a valid %s", me,
               measrInfo, pullInfo->name);
      return 1;
    }
    if (1 != pullInfoLen(measrInfo)) {
      biffAddf(PULL, "%s: measrInfo %s (%d) isn't a scalar (len %u)", me,
               airEnumStr(pullInfo, measrInfo), measrInfo,
               pullInfoLen(measrInfo));
      return 1;
    }
    if (!pctx->ispec[measrInfo]) {
      biffAddf(PULL, "%s: no ispec set for measrInfo %s (%d)", me,
               airEnumStr(pullInfo, measrInfo), measrInfo);
      return 1;
    }
  } /* else measrInfo is zero, they want to know # points */
  /* in any case nmeasr is allocated for doubles */
  if (nrrdMaybeAlloc_va(nmeasr, nrrdTypeDouble, 1,
                        AIR_CAST(size_t, pctx->CCNum))) {
    biffMovef(PULL, NRRD, "%s: couldn't alloc nmeasr", me);
    return 1;
  }
  meas = AIR_CAST(double *, nmeasr->data);

  mop = airMopNew();
  /* HEY: don't actually need to allocate and set size[],
     if measrInfo == 0 */
  if (!(size = AIR_CAST(double *,
                        calloc(pctx->CCNum, sizeof(double))))) {
    biffAddf(PULL, "%s: couldn't alloc size", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, size, airFree, airMopAlways);

  /* finally, do measurement */
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      size[point->idCC]++;
      meas[point->idCC] += (measrInfo
                            ? pullPointScalar(pctx, point, measrInfo,
                                              NULL, NULL)
                            : 1);
    }
  }
  if (measrInfo) {
    for (ii=0; ii<pctx->CCNum; ii++) {
      meas[ii] /= size[ii];
      meas[ii] = AIR_LERP(rho, size[ii], meas[ii]);
    }
  }

  airMopOkay(mop);
  return 0;
}

typedef struct {
  unsigned int i;
  double d;
} ccpair;

/* we intend to sort in *descending* order */
static int
ccpairCompare(const void *_a, const void *_b) {
  const ccpair *a, *b;

  a = AIR_CAST(const ccpair *, _a);
  b = AIR_CAST(const ccpair *, _b);
  return (a->d < b->d
          ? +1
          : (a->d > b->d
             ? -1
             : 0));
}

/*
******** pullCCSort
**
** sort CCs in pull context
**
** measrInfo == 0: sort by size, else,
** sort by blend of size and measrInfo
** rho == 0: only size, rho == 1: only measrInfo
*/
int
pullCCSort(pullContext *pctx, int measrInfo, double rho) {
  static const char me[]="pullCCSort";
  ccpair *pair;
  Nrrd *nmeasr;
  airArray *mop;
  unsigned int ii, *revm, binIdx, pointIdx;
  double *measr;
  pullBin *bin;
  pullPoint *point;
  int E;

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!pctx->CCNum) {
    biffAddf(PULL, "%s: haven't yet learned CCs?", me);
    return 1;
  }

#define CALLOC(T) (AIR_CAST(T *, calloc(pctx->CCNum, sizeof(T))))
  mop = airMopNew();
  if (!(nmeasr = nrrdNew())
      || airMopAdd(mop, nmeasr, (airMopper)nrrdNuke, airMopAlways)
      || !(pair = CALLOC(ccpair))
      || airMopAdd(mop, pair, airFree, airMopAlways)
      || !(revm = CALLOC(unsigned int))
      || airMopAdd(mop, revm, airFree, airMopAlways)) {
    biffAddf(PULL, "%s: couldn't allocate everything", me);
    airMopError(mop); return 1;
  }
#undef CALLOC
  if (!measrInfo) {
    /* sorting by size */
    E = pullCCMeasure(pctx, nmeasr, 0, 0.0);
  } else {
    /* sorting by some blend of size and pullInfo */
    E = pullCCMeasure(pctx, nmeasr, measrInfo, rho);
  }
  if (E) {
    biffAddf(PULL, "%s: problem measuring CCs", me);
    airMopError(mop); return 1;
  }

  measr = AIR_CAST(double *, nmeasr->data);
  for (ii=0; ii<pctx->CCNum; ii++) {
    pair[ii].i = ii;
    pair[ii].d = measr[ii];
  }
  qsort(pair, pctx->CCNum, sizeof(ccpair), ccpairCompare);
  for (ii=0; ii<pctx->CCNum; ii++) {
    revm[pair[ii].i] = ii;
  }

  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      point->idCC = revm[point->idCC];
    }
  }

  airMopOkay(mop);
  return 0;
}
