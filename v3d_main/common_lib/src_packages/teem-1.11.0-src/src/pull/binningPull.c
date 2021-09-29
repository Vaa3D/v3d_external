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
** because the pullContext keeps an array of bins (not pointers to them)
** we have Init and Done functions (not New and Nix)
*/
void
_pullBinInit(pullBin *bin) {

  bin->point = NULL;
  bin->pointNum = 0;
  bin->pointArr = NULL;
  bin->neighBin = NULL;
  return;
}

/*
** bins own the points they contain- so this frees them
*/
void
_pullBinDone(pullBin *bin) {
  unsigned int idx;

  for (idx=0; idx<bin->pointNum; idx++) {
    bin->point[idx] = pullPointNix(bin->point[idx]);
  }
  bin->pointArr = airArrayNuke(bin->pointArr);
  bin->neighBin = (pullBin **)airFree(bin->neighBin);
  return;
}

int
_pullBinNeighborSet(pullContext *pctx, pullBin *bin) {
  static const char me[]="_pullBinNeighborSet";
  unsigned int neiIdx, neiNum, be[4], binIdx;
  unsigned int xi, yi, zi, si, xx, yy, zz, ss,
    xmax, ymax, zmax, smax;
  pullBin *nei[3*3*3*3];
  int xmin, ymin, zmin, smin;

  binIdx = AIR_UINT(bin - pctx->bin);
  /* annoyingly, have to recover the bin coordinates */
  ELL_4V_COPY(be, pctx->binsEdge);
  xi = binIdx % be[0];
  binIdx = (binIdx - xi)/be[0];
  yi = binIdx % be[1];
  binIdx = (binIdx - yi)/be[1];
  zi = binIdx % be[2];
  si = (binIdx - zi)/be[2];
  neiNum = 0;
  bin->neighBin = (pullBin **)airFree(bin->neighBin);
  smin = AIR_MAX(0, (int)si-1);
  smax = AIR_MIN(si+1, be[3]-1);
  zmin = AIR_MAX(0, (int)zi-1);
  zmax = AIR_MIN(zi+1, be[2]-1);
  ymin = AIR_MAX(0, (int)yi-1);
  ymax = AIR_MIN(yi+1, be[1]-1);
  xmin = AIR_MAX(0, (int)xi-1);
  xmax = AIR_MIN(xi+1, be[0]-1);
  for (ss=smin; ss<=smax; ss++) {
    for (zz=zmin; zz<=zmax; zz++) {
      for (yy=ymin; yy<=ymax; yy++) {
        for (xx=xmin; xx<=xmax; xx++) {
          nei[neiNum++] = pctx->bin + xx + be[0]*(yy + be[1]*(zz + be[2]*ss));
        }
      }
    }
  }
  if (!( bin->neighBin = AIR_CALLOC(1+neiNum, pullBin*) )) {
    biffAddf(PULL, "%s: couldn't calloc array of %u neighbor pointers", me, 1+neiNum);
    return 1;
  }
  for (neiIdx=0; neiIdx<neiNum; neiIdx++) {
    bin->neighBin[neiIdx] = nei[neiIdx];
  }
  /* NULL-terminate the bin array */
  bin->neighBin[neiIdx] = NULL;
  return 0;
}

/*
** bins on boundary now extend to infinity; so the only time this
** returns NULL (indicating error) is for non-existent positions
*/
pullBin *
_pullBinLocate(pullContext *pctx, double *posWorld) {
  static const char me[]="_pullBinLocate";
  unsigned int axi, eidx[4], binIdx;

  if (!ELL_4V_EXISTS(posWorld)) {
    biffAddf(PULL, "%s: non-existent position (%g,%g,%g,%g)", me,
             posWorld[0], posWorld[1], posWorld[2], posWorld[3]);
    return NULL;
  }

  if (pctx->flag.binSingle) {
    binIdx = 0;
  } else {
    for (axi=0; axi<4; axi++) {
      eidx[axi] = airIndexClamp(pctx->bboxMin[axi],
                                posWorld[axi],
                                pctx->bboxMax[axi],
                                pctx->binsEdge[axi]);
    }
    binIdx = (eidx[0]
              + pctx->binsEdge[0]*(
                    eidx[1] + pctx->binsEdge[1]*(
                         eidx[2] + pctx->binsEdge[2] * eidx[3])));
  }

  return pctx->bin + binIdx;
}

/*
** this makes the bin the owner of the point
*/
int
_pullBinPointAdd(pullContext *pctx, pullBin *bin, pullPoint *point) {
  static const char me[]="_pullBinPointAdd";
  int pntI;
  pullPtrPtrUnion pppu;

  AIR_UNUSED(pctx);
  if (!(bin->pointArr)) {
    pppu.points = &(bin->point);
    bin->pointArr = airArrayNew(pppu.v, &(bin->pointNum),
                                sizeof(pullPoint *), _PULL_BIN_INCR);
    if (!( bin->pointArr )) {
      biffAddf(PULL, "%s: couldn't create point array", me);
      return 1;
    }
  }
  if (!( bin->neighBin )) {
    /* set up neighbor bin vector if not done so already */
    if (_pullBinNeighborSet(pctx, bin)) {
      biffAddf(PULL, "%s: couldn't initialize neighbor bins", me);
      return 1;
    }
  }
  pntI = airArrayLenIncr(bin->pointArr, 1);
  bin->point[pntI] = point;
  return 0;
}

/*
** the bin loses track of the point, caller responsible for ownership,
** even though caller never identifies it by pointer, which is weird
*/
void
_pullBinPointRemove(pullContext *pctx, pullBin *bin, int loseIdx) {

  AIR_UNUSED(pctx);
  bin->point[loseIdx] = bin->point[bin->pointNum-1];
  airArrayLenIncr(bin->pointArr, -1);
  return;
}

/*
** adds point to context
*/
int
pullBinsPointAdd(pullContext *pctx, pullPoint *point, pullBin **binP) {
  static const char me[]="pullBinsPointAdd";
  pullBin *bin;

  if (binP) {
    *binP = NULL;
  }
  if (!( bin = _pullBinLocate(pctx, point->pos) )) {
    biffAddf(PULL, "%s: can't locate point %p %u",
             me, AIR_CAST(void*, point), point->idtag);
    return 1;
  }
  if (binP) {
    *binP = bin;
  }
  if (_pullBinPointAdd(pctx, bin, point)) {
    biffAddf(PULL, "%s: trouble adding point %p %u",
             me, AIR_CAST(void*, point), point->idtag);
    return 1;
  }
  return 0;
}

int
pullBinsPointMaybeAdd(pullContext *pctx, pullPoint *point,
                      /* output */
                      pullBin **binP, int *added) {
  static const char me[]="pullBinsPointMaybeAdd";
  pullBin *bin;
  unsigned int idx;
  int okay;

  if (binP) {
    *binP = NULL;
  }
  if (!(pctx && point && added)) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( bin = _pullBinLocate(pctx, point->pos) )) {
    biffAddf(PULL, "%s: can't locate point %p %u",
             me, AIR_CAST(void*, point), point->idtag);
    return 1;
  }
  if (binP) {
    *binP = bin;
  }
  if (pctx->flag.restrictiveAddToBins) {
    okay = AIR_TRUE;
    for (idx=0; idx<bin->pointNum; idx++) {
      double diff[4], len;
      ELL_4V_SUB(diff, point->pos, bin->point[idx]->pos);
      ELL_3V_SCALE(diff, 1/pctx->sysParm.radiusSpace, diff);
      diff[3] /= pctx->sysParm.radiusScale;
      len = ELL_4V_LEN(diff);
      if (len < _PULL_BINNING_MAYBE_ADD_THRESH) {
        okay = AIR_FALSE;
        break;
      }
    }
    if (okay) {
      if (_pullBinPointAdd(pctx, bin, point)) {
        biffAddf(PULL, "%s: trouble adding point %p %u",
                 me, AIR_CAST(void*, point), point->idtag);
        return 1;
      }
      *added = AIR_TRUE;
    } else {
      *added = AIR_FALSE;
    }
  } else {
    if (_pullBinPointAdd(pctx, bin, point)) {
      biffAddf(PULL, "%s: trouble adding point %p %u",
               me, AIR_CAST(void*, point), point->idtag);
      return 1;
    }
    *added = AIR_TRUE;
  }
  return 0;
}

int
_pullBinSetup(pullContext *pctx) {
  static const char me[]="_pullBinSetup";
  unsigned ii;
  double volEdge[4], width;

  /* the maximum distance of interaction is when one particle is sitting
     on the edge of another particle's sphere of influence, *NOT*, when
     the spheres of influence of two particles are tangent: particles
     interact with potential fields of other particles, but there is
     no interaction between potential fields. */
  width = (pctx->sysParm.radiusSpace ? pctx->sysParm.radiusSpace : 0.1);
  pctx->maxDistSpace = pctx->sysParm.binWidthSpace*width;
  width = (pctx->sysParm.radiusScale ? pctx->sysParm.radiusScale : 0.1);
  pctx->maxDistScale = 1*width;

  if (pctx->verbose) {
    printf("%s: radiusSpace = %g -(%g)-> maxDistSpace = %g\n", me,
           pctx->sysParm.radiusSpace, pctx->sysParm.binWidthSpace,
           pctx->maxDistSpace);
    printf("%s: radiusScale = %g ----> maxDistScale = %g\n", me,
           pctx->sysParm.radiusScale, pctx->maxDistScale);
  }

  if (pctx->flag.binSingle) {
    pctx->binsEdge[0] = 1;
    pctx->binsEdge[1] = 1;
    pctx->binsEdge[2] = 1;
    pctx->binsEdge[3] = 1;
    pctx->binNum = 1;
  } else {
    volEdge[0] = pctx->bboxMax[0] - pctx->bboxMin[0];
    volEdge[1] = pctx->bboxMax[1] - pctx->bboxMin[1];
    volEdge[2] = pctx->bboxMax[2] - pctx->bboxMin[2];
    volEdge[3] = pctx->bboxMax[3] - pctx->bboxMin[3];
    if (pctx->verbose) {
      printf("%s: volEdge = %g %g %g %g\n", me,
             volEdge[0], volEdge[1], volEdge[2], volEdge[3]);
    }
    pctx->binsEdge[0] = AIR_CAST(unsigned int,
                                 floor(volEdge[0]/pctx->maxDistSpace));
    pctx->binsEdge[0] = pctx->binsEdge[0] ? pctx->binsEdge[0] : 1;
    pctx->binsEdge[1] = AIR_CAST(unsigned int,
                                 floor(volEdge[1]/pctx->maxDistSpace));
    pctx->binsEdge[1] = pctx->binsEdge[1] ? pctx->binsEdge[1] : 1;
    pctx->binsEdge[2] = AIR_CAST(unsigned int,
                                 floor(volEdge[2]/pctx->maxDistSpace));
    pctx->binsEdge[2] = pctx->binsEdge[2] ? pctx->binsEdge[2] : 1;
    pctx->binsEdge[3] = AIR_CAST(unsigned int,
                                 floor(volEdge[3]/pctx->maxDistScale));
    pctx->binsEdge[3] = pctx->binsEdge[3] ? pctx->binsEdge[3] : 1;
    /* hack to observe things at bin boundaries
    ELL_3V_SET(pctx->binsEdge, 3, 3, 3);
    */
    if (pctx->verbose) {
      printf("%s: binsEdge=(%u,%u,%u,%u)\n", me,
             pctx->binsEdge[0], pctx->binsEdge[1],
             pctx->binsEdge[2], pctx->binsEdge[3]);
    }
    pctx->binNum = (pctx->binsEdge[0]*pctx->binsEdge[1]
                    *pctx->binsEdge[2]*pctx->binsEdge[3]);
  }
  if (pctx->binNum > PULL_BIN_MAXNUM) {
    biffAddf(PULL, "%s: sorry, #bins %u > PULL_BIN_MAXNUM %u. Try "
             "increasing pctx->sysParm.binWidthSpace (%g)", me,
             pctx->binNum, PULL_BIN_MAXNUM, pctx->sysParm.binWidthSpace);
    return 1;
  }
  if (pctx->verbose) {
    printf("%s: trying to allocate %u bins . . . \n", me, pctx->binNum);
  }
  pctx->bin = AIR_CALLOC(pctx->binNum, pullBin);
  if (!( pctx->bin )) {
    biffAddf(PULL, "%s: couln't allocate %u bins", me, pctx->binNum);
    return 1;
  }
  if (pctx->verbose) {
    printf("%s: done allocating. Initializing . . . \n", me);
  }
  for (ii=0; ii<pctx->binNum; ii++) {
    _pullBinInit(pctx->bin + ii);
  }
  if (pctx->verbose) {
    printf("%s: done initializing.\n", me);
  }
  if (pctx->flag.binSingle) {
    if (!( pctx->bin[0].neighBin = AIR_CALLOC(2, pullBin*) )) {
      biffAddf(PULL, "%s: trouble allocating for single bin?", me);
      return 1;
    }
    pctx->bin[0].neighBin[0] = pctx->bin + 0;
    pctx->bin[0].neighBin[1] = NULL;
  }
  return 0;
}

void
_pullBinFinish(pullContext *pctx) {
  unsigned int ii;

  for (ii=0; ii<pctx->binNum; ii++) {
    _pullBinDone(pctx->bin + ii);
  }
  pctx->bin = (pullBin *)airFree(pctx->bin);
  ELL_4V_SET(pctx->binsEdge, 0, 0, 0, 0);
  pctx->binNum = 0;
}

/*
** sets pctx->stuckNum
** resets all task[]->stuckNum
** reallocates pctx->tmpPointPerm and pctx->tmpPointPtr
** the point of this is to do rebinning
**
** This function is only called by the master thread, this
** does *not* have to be thread-safe in any way
*/
int
_pullIterFinishDescent(pullContext *pctx) {
  static const char me[]="_pullIterFinishDescent";
  unsigned int oldBinIdx, pointIdx, taskIdx, runIdx, pointNum;
  pullBin *oldBin, *newBin;
  pullPoint *point;

  _pullNixTheNixed(pctx);

  pctx->stuckNum = 0;
  for (taskIdx=0; taskIdx<pctx->threadNum; taskIdx++) {
    pctx->stuckNum += pctx->task[taskIdx]->stuckNum;
    pctx->task[taskIdx]->stuckNum = 0;
  }

  /* even w/ a single bin, we may still have to permute the points */
  pointNum = pullPointNumber(pctx);
  if (pointNum != pctx->tmpPointNum) {
    if (pctx->verbose) {
      printf("!%s: changing total point # %u --> %u\n", me,
             pctx->tmpPointNum, pointNum);
    }
    airFree(pctx->tmpPointPerm);
    airFree(pctx->tmpPointPtr);
    pctx->tmpPointPtr = AIR_CAST(pullPoint **,
                                 calloc(pointNum, sizeof(pullPoint*)));
    pctx->tmpPointPerm = AIR_CAST(unsigned int *,
                                  calloc(pointNum, sizeof(unsigned int)));
    if (!( pctx->tmpPointPtr && pctx->tmpPointPerm )) {
      biffAddf(PULL, "%s: couldn't allocate tmp buffers %p %p", me,
               AIR_VOIDP(pctx->tmpPointPtr), AIR_VOIDP(pctx->tmpPointPerm));
      return 1;
    }
    pctx->tmpPointNum = pointNum;
  }
  runIdx = 0;
  for (oldBinIdx=0; oldBinIdx<pctx->binNum; oldBinIdx++) {
    oldBin = pctx->bin + oldBinIdx;
    while (oldBin->pointNum) {
      /* tricky: we can't traverse bin->point[], because of how it is
         re-ordered on point removal, so we always grab point[0] */
      pctx->tmpPointPtr[runIdx++] = oldBin->point[0];
      _pullBinPointRemove(pctx, oldBin, 0);
    }
  }
  airShuffle_r(pctx->task[0]->rng,
               pctx->tmpPointPerm, pointNum, pctx->flag.permuteOnRebin);
  if (pctx->flag.permuteOnRebin && pctx->verbose) {
    printf("%s: permuting %u points\n", me, pointNum);
  }
  for (pointIdx=0; pointIdx<pointNum; pointIdx++) {
    point = pctx->tmpPointPtr[pctx->tmpPointPerm[pointIdx]];
    /*
    if (1 == pctx->iter && 102 == point->idtag) {
      _pullDebugSanity(pctx->task[0], point);
    }
    */
    newBin = _pullBinLocate(pctx, point->pos);
    if (!newBin) {
      biffAddf(PULL, "%s: can't locate point %p %u",
               me, AIR_CAST(void*, point), point->idtag);
      return 1;
    }
    if (_pullBinPointAdd(pctx, newBin, point)) {
      biffAddf(PULL, "%s: trouble adding point %p %u",
               me, AIR_CAST(void*, point), point->idtag);
      return 1;
    }
    pctx->tmpPointPtr[pctx->tmpPointPerm[pointIdx]] = NULL;
  }

  return 0;
}

