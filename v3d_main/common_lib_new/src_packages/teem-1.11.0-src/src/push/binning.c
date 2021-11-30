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

#include "push.h"
#include "privatePush.h"

/*
** because the pushContext keeps an array of bins (not pointers to them)
** we have Init and Done functions (not New and Nix)
*/
void
pushBinInit(pushBin *bin, unsigned int incr) {
  pushPtrPtrUnion pppu;

  bin->pointNum = 0;
  bin->point = NULL;
  bin->pointArr = airArrayNew((pppu.point = &(bin->point), pppu.v),
                              &(bin->pointNum),
                              sizeof(pushPoint *), incr);
  bin->neighbor = NULL;
  return;
}

/*
** bins own the points they contain- so this frees them
*/
void
pushBinDone(pushBin *bin) {
  unsigned int idx;

  for (idx=0; idx<bin->pointNum; idx++) {
    bin->point[idx] = pushPointNix(bin->point[idx]);
  }
  bin->pointArr = airArrayNuke(bin->pointArr);
  bin->neighbor = (pushBin **)airFree(bin->neighbor);
  return;
}


/*
** bins on boundary now extend to infinity; so the only time this
** returns NULL (indicating error) is for non-existent positions
*/
pushBin *
_pushBinLocate(pushContext *pctx, double *_posWorld) {
  static const char me[]="_pushBinLocate";
  double posWorld[4], posIdx[4];
  unsigned int axi, eidx[3], binIdx;

  if (!ELL_3V_EXISTS(_posWorld)) {
    biffAddf(PUSH, "%s: non-existent position (%g,%g,%g)", me,
             _posWorld[0], _posWorld[1], _posWorld[2]);
    return NULL;
  }

  if (pctx->binSingle) {
    binIdx = 0;
  } else {
    ELL_3V_COPY(posWorld, _posWorld);
    posWorld[3] = 1.0;
    ELL_4MV_MUL(posIdx, pctx->gctx->shape->WtoI, posWorld);
    ELL_34V_HOMOG(posIdx, posIdx);
    for (axi=0; axi<3; axi++) {
      eidx[axi] = airIndexClamp(-0.5,
                                posIdx[axi],
                                pctx->gctx->shape->size[axi]-0.5,
                                pctx->binsEdge[axi]);
    }
    binIdx = (eidx[0]
              + pctx->binsEdge[0]*(eidx[1]
                                   + pctx->binsEdge[1]*eidx[2]));
  }
  /*
  fprintf(stderr, "!%s: bin(%g,%g,%g) = %u\n", me,
          _posWorld[0], _posWorld[1], _posWorld[2], binIdx);
  */

  return pctx->bin + binIdx;
}

/*
** this makes the bin the owner of the point
*/
void
_pushBinPointAdd(pushContext *pctx, pushBin *bin, pushPoint *point) {
  int pntI;

  AIR_UNUSED(pctx);
  pntI = airArrayLenIncr(bin->pointArr, 1);
  bin->point[pntI] = point;

  return;
}

/*
** the bin loses track of the point, caller responsible for ownership
*/
void
_pushBinPointRemove(pushContext *pctx, pushBin *bin, int loseIdx) {

  AIR_UNUSED(pctx);
  bin->point[loseIdx] = bin->point[bin->pointNum-1];
  airArrayLenIncr(bin->pointArr, -1);

  return;
}

void
_pushBinNeighborSet(pushBin *bin, pushBin **nei, unsigned int num) {
  unsigned int neiI;

  bin->neighbor = (pushBin **)airFree(bin->neighbor);
  bin->neighbor = (pushBin **)calloc(1+num, sizeof(pushBin *));
  for (neiI=0; neiI<num; neiI++) {
    bin->neighbor[neiI] = nei[neiI];
  }
  bin->neighbor[neiI] = NULL;
  return;
}

void
pushBinAllNeighborSet(pushContext *pctx) {
  /* static const char me[]="pushBinAllNeighborSet"; */
  pushBin *nei[3*3*3];
  unsigned int neiNum, xi, yi, zi, xx, yy, zz, xmax, ymax, zmax, binIdx;
  int xmin, ymin, zmin;

  if (pctx->binSingle) {
    neiNum = 0;
    nei[neiNum++] = pctx->bin + 0;
    _pushBinNeighborSet(pctx->bin + 0, nei, neiNum);
  } else {
    for (zi=0; zi<pctx->binsEdge[2]; zi++) {
      zmin = AIR_MAX(0, (int)zi-1);
      zmax = AIR_MIN(zi+1, pctx->binsEdge[2]-1);
      for (yi=0; yi<pctx->binsEdge[1]; yi++) {
        ymin = AIR_MAX(0, (int)yi-1);
        ymax = AIR_MIN(yi+1, pctx->binsEdge[1]-1);
        for (xi=0; xi<pctx->binsEdge[0]; xi++) {
          xmin = AIR_MAX(0, (int)xi-1);
          xmax = AIR_MIN(xi+1, pctx->binsEdge[0]-1);
          neiNum = 0;
          for (zz=zmin; zz<=zmax; zz++) {
            for (yy=ymin; yy<=ymax; yy++) {
              for (xx=xmin; xx<=xmax; xx++) {
                binIdx = xx + pctx->binsEdge[0]*(yy + pctx->binsEdge[1]*zz);
                /*
                fprintf(stderr, "!%s: nei[%u](%u,%u,%u) = %u\n", me,
                        neiNum, xi, yi, zi, binIdx);
                */
                nei[neiNum++] = pctx->bin + binIdx;
              }
            }
          }
          _pushBinNeighborSet(pctx->bin + xi + pctx->binsEdge[0]
                              *(yi + pctx->binsEdge[1]*zi), nei, neiNum);
        }
      }
    }
  }
  return;
}

int
pushBinPointAdd(pushContext *pctx, pushPoint *point) {
  static const char me[]="pushBinPointAdd";
  pushBin *bin;

  if (!( bin = _pushBinLocate(pctx, point->pos) )) {
    biffAddf(PUSH, "%s: can't locate point %p %u",
             me, AIR_CAST(void*, point), point->ttaagg);
    return 1;
  }
  _pushBinPointAdd(pctx, bin, point);
  return 0;
}

/*
** This function is only called by the master thread, this
** does *not* have to be thread-safe in any way
*/
int
pushRebin(pushContext *pctx) {
  static const char me[]="pushRebin";
  unsigned int oldBinIdx, pointIdx;
  pushBin *oldBin, *newBin;
  pushPoint *point;

  if (!pctx->binSingle) {
    for (oldBinIdx=0; oldBinIdx<pctx->binNum; oldBinIdx++) {
      oldBin = pctx->bin + oldBinIdx;

      for (pointIdx=0; pointIdx<oldBin->pointNum; /* nope! */) {
        point = oldBin->point[pointIdx];
        newBin = _pushBinLocate(pctx, point->pos);
        if (!newBin) {
          biffAddf(PUSH, "%s: can't locate point %p %u",
                   me, AIR_CAST(void*, point), point->ttaagg);
          return 1;
        }
        if (oldBin != newBin) {
          _pushBinPointRemove(pctx, oldBin, pointIdx);
          _pushBinPointAdd(pctx, newBin, point);
        } else {
          /* its in the right bin, move on */
          pointIdx++;
        }
      } /* for pointIdx */

    } /* for oldBinIdx */
  }

  return 0;
}
