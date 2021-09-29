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

void
_pullInitParmInit(pullInitParm *initParm) {

  initParm->method = pullInitMethodUnknown;
  initParm->liveThreshUse = AIR_FALSE;
  initParm->unequalShapesAllow = AIR_FALSE;
  initParm->jitter = 1.0;
  initParm->numInitial = 0;
  initParm->haltonStartIndex = 0;
  initParm->samplesAlongScaleNum = 0;
  initParm->ppvZRange[0] = 1;
  initParm->ppvZRange[1] = 0;
  initParm->pointPerVoxel = 0;
  initParm->npos = NULL;
  return;
}

#define CHECK(thing, min, max)                                         \
  if (!( AIR_EXISTS(iparm->thing)                                      \
         && min <= iparm->thing && iparm->thing <= max )) {            \
    biffAddf(PULL, "%s: initParm->" #thing " %g not in range [%g,%g]", \
             me, iparm->thing, min, max);                              \
    return 1;                                                          \
  }

int
_pullInitParmCheck(pullInitParm *iparm) {
  static const char me[]="_pullInitParmCheck";

  if (!AIR_IN_OP(pullInitMethodUnknown, iparm->method, pullInitMethodLast)) {
    biffAddf(PULL, "%s: init method %d not valid", me, iparm->method);
    return 1;
  }
  CHECK(jitter, 0.0, 2.0);
  switch (iparm->method) {
  case pullInitMethodGivenPos:
    if (nrrdCheck(iparm->npos)) {
      biffMovef(PULL, NRRD, "%s: got a broken npos", me);
      return 1;
    }
    if (!( 2 == iparm->npos->dim
           && 4 == iparm->npos->axis[0].size
           && (nrrdTypeDouble == iparm->npos->type
               || nrrdTypeFloat == iparm->npos->type) )) {
      biffAddf(PULL, "%s: npos not a 2-D 4-by-N array of %s or %s"
               "(got %u-D %u-by-X of %s)", me,
               airEnumStr(nrrdType, nrrdTypeFloat),
               airEnumStr(nrrdType, nrrdTypeDouble),
               iparm->npos->dim,
               AIR_CAST(unsigned int, iparm->npos->axis[0].size),
               airEnumStr(nrrdType, iparm->npos->type));
      return 1;
    }
    break;
  case pullInitMethodPointPerVoxel:
    if (iparm->pointPerVoxel < -3001 || iparm->pointPerVoxel > 10) {
      biffAddf(PULL, "%s: pointPerVoxel %d unreasonable", me,
               iparm->pointPerVoxel);
      return 1;
    }
    if (-1 == iparm->pointPerVoxel) {
      biffAddf(PULL, "%s: pointPerVoxel should be < -1 or >= 1", me);
      return 1;
    }
    if (0 == iparm->jitter && 1 < iparm->pointPerVoxel) {
      biffAddf(PULL, "%s: must have jitter > 0 if pointPerVoxel (%d) > 1", me,
               iparm->pointPerVoxel);
      return 1;
    }
    break;
  case pullInitMethodRandom:
  case pullInitMethodHalton:
    if (!( iparm->numInitial >= 1 )) {
      biffAddf(PULL, "%s: iparm->numInitial (%d) not >= 1\n", me,
               iparm->numInitial);
      return 1;
    }
    break;
  /* no check needed on haltonStartIndex */
  default:
    biffAddf(PULL, "%s: init method %d valid but not handled?", me,
             iparm->method);
    return 1;
  }

  return 0;
}

int
pullInitRandomSet(pullContext *pctx, unsigned int numInitial) {
  static const char me[]="pullInitRandomSet";

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!numInitial) {
    biffAddf(PULL, "%s: need non-zero numInitial", me);
    return 1;
  }

  pctx->initParm.method = pullInitMethodRandom;
  pctx->initParm.numInitial = numInitial;
  return 0;
}

int
pullInitHaltonSet(pullContext *pctx, unsigned int numInitial,
                  unsigned int startIndex) {
  static const char me[]="pullInitHaltonSet";

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!numInitial) {
    biffAddf(PULL, "%s: need non-zero numInitial", me);
    return 1;
  }

  pctx->initParm.method = pullInitMethodHalton;
  pctx->initParm.numInitial = numInitial;
  pctx->initParm.haltonStartIndex = startIndex;
  return 0;
}

int
pullInitPointPerVoxelSet(pullContext *pctx, int pointPerVoxel,
                         unsigned int zSlcMin, unsigned int zSlcMax,
                         unsigned int alongScaleNum,
                         double jitter) {
  static const char me[]="pullInitPointPerVoxelSet";

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!pointPerVoxel) {
    biffAddf(PULL, "%s: need non-zero pointPerVoxel", me);
    return 1;
  }
  if (!AIR_EXISTS(jitter)) {
    biffAddf(PULL, "%s: got non-existent jitter %g", me, jitter);
    return 1;
  }

  pctx->initParm.method = pullInitMethodPointPerVoxel;
  pctx->initParm.pointPerVoxel = pointPerVoxel;
  pctx->initParm.samplesAlongScaleNum = alongScaleNum;
  pctx->initParm.ppvZRange[0] = zSlcMin;
  pctx->initParm.ppvZRange[1] = zSlcMax;
  pctx->initParm.jitter = jitter;
  return 0;
}

int
pullInitGivenPosSet(pullContext *pctx, const Nrrd *npos) {
  static const char me[]="pullInitGivenPosSet";

  if (!(pctx && npos)) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }

  pctx->initParm.method = pullInitMethodGivenPos;
  pctx->initParm.npos = npos;
  return 0;
}

int
pullInitLiveThreshUseSet(pullContext *pctx, int liveThreshUse) {
  static const char me[]="pullInitLiveThreshUseSet";

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }

  pctx->initParm.liveThreshUse = liveThreshUse;
  return 0;
}

int
pullInitUnequalShapesAllowSet(pullContext *pctx, int allow) {
  static const char me[]="pullInitUnequalShapesAllowSet";

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }

  pctx->initParm.unequalShapesAllow = allow;
  return 0;
}

#undef CHECK

FILE *
_pullPointAddLog = NULL;
