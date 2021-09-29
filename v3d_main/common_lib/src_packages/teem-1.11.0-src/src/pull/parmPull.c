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

/* the Init() functions are up here for easier reference */

void
_pullIterParmInit(pullIterParm *iterParm) {

  iterParm->min = 0;
  iterParm->max = 0;
  iterParm->stuckMax = 4;
  iterParm->constraintMax = 15;
  iterParm->popCntlPeriod = 10;
  iterParm->addDescent = 10;
  iterParm->callback = 1;
  iterParm->snap = 0;
  iterParm->energyIncreasePermitHalfLife = 0;
  return;
}

void
_pullSysParmInit(pullSysParm *sysParm) {

  sysParm->alpha = 0.5;
  sysParm->beta = 0.5;
  sysParm->gamma = 1;
  sysParm->separableGammaLearnRescale = 8;
  sysParm->theta = 0.0;
  sysParm->wall = 1;
  sysParm->radiusSpace = 1;
  sysParm->radiusScale = 1;
  sysParm->binWidthSpace = 1.001; /* supersititious */
  sysParm->neighborTrueProb = 1.0;
  sysParm->probeProb = 1.0;
  sysParm->stepInitial = 1;
  sysParm->opporStepScale = 1.0;
  sysParm->backStepScale = 0.5;
  sysParm->constraintStepMin = 0.0001;
  sysParm->energyDecreaseMin = 0.001;
  sysParm->energyDecreasePopCntlMin = 0.02;
  sysParm->energyIncreasePermit = 0.0;
  sysParm->fracNeighNixedMax = 0.25;
  return;
}

void
_pullFlagInit(pullFlag *flag) {

  flag->permuteOnRebin = AIR_FALSE;
  flag->noPopCntlWithZeroAlpha = AIR_FALSE;
  flag->useBetaForGammaLearn = AIR_FALSE;
  flag->restrictiveAddToBins = AIR_TRUE;
  flag->energyFromStrength = AIR_FALSE;
  flag->nixAtVolumeEdgeSpace = AIR_FALSE;
  flag->constraintBeforeSeedThresh = AIR_FALSE;
  flag->noAdd = AIR_FALSE;
  flag->popCntlEnoughTest = AIR_TRUE; /* really needs to be true by default */
  flag->binSingle = AIR_FALSE;
  flag->allowCodimension3Constraints = AIR_FALSE;
  flag->scaleIsTau = AIR_FALSE;
  flag->startSkipsPoints = AIR_FALSE; /* must be false by default */
  return;
}

int
_pullIterParmCheck(pullIterParm *iterParm) {
  static const char me[]="_pullIterParmCheck";

  if (!( 1 <= iterParm->constraintMax
         && iterParm->constraintMax <= 500 )) {
    biffAddf(PULL, "%s: iterParm->constraintMax %u not in range [%u,%u]",
             me, iterParm->constraintMax, 1, _PULL_CONSTRAINT_ITER_MAX);
    return 1;
  }
  return 0;
}

int
pullIterParmSet(pullContext *pctx, int which, unsigned int pval) {
  static const char me[]="pullIterParmSet";

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!AIR_IN_OP(pullIterParmUnknown, which, pullIterParmLast)) {
    biffAddf(PULL, "%s: iter parm %d not valid", me, which);
    return 1;
  }
  switch(which) {
  case pullIterParmMin:
    pctx->iterParm.min = pval;
    break;
  case pullIterParmMax:
    pctx->iterParm.max = pval;
    break;
  case pullIterParmStuckMax:
    pctx->iterParm.stuckMax = pval;
    break;
  case pullIterParmConstraintMax:
    pctx->iterParm.constraintMax = pval;
    break;
  case pullIterParmPopCntlPeriod:
    pctx->iterParm.popCntlPeriod = pval;
    break;
  case pullIterParmAddDescent:
    pctx->iterParm.addDescent = pval;
    break;
  case pullIterParmCallback:
    pctx->iterParm.callback = pval;
    break;
  case pullIterParmSnap:
    pctx->iterParm.snap = pval;
    break;
  case pullIterParmEnergyIncreasePermitHalfLife:
    pctx->iterParm.energyIncreasePermitHalfLife = pval;
    if (pval) {
      pctx->eipScale = pow(0.5, 1.0/pval);
    } else {
      pctx->eipScale = 1;
    }
    break;
  default:
    biffAddf(me, "%s: sorry, iter parm %d valid but not handled?", me, which);
    return 1;
  }
  return 0;
}

#define CHECK(thing, min, max)                                         \
  if (!( AIR_EXISTS(sysParm->thing)                                    \
         && min <= sysParm->thing && sysParm->thing <= max )) {        \
    biffAddf(PULL, "%s: sysParm->" #thing " %g not in range [%g,%g]",  \
             me, sysParm->thing, min, max);                            \
    return 1;                                                          \
  }

int
_pullSysParmCheck(pullSysParm *sysParm) {
  static const char me[]="_pullSysParmCheck";

  /* these reality-check bounds are somewhat arbitrary */
  CHECK(alpha, 0.0, 1.0);
  CHECK(beta, 0.0, 1.0);
  /* HEY: no check on gamma? */
  /* no check on theta */
  CHECK(wall, 0.0, 100.0);
  CHECK(radiusSpace, 0.000001, 80.0);
  CHECK(radiusScale, 0.000001, 80.0);
  CHECK(binWidthSpace, 1.0, 15.0);
  CHECK(neighborTrueProb, 0.02, 1.0);
  CHECK(probeProb, 0.02, 1.0);
  if (!( AIR_EXISTS(sysParm->stepInitial)
         && sysParm->stepInitial > 0 )) {
    biffAddf(PULL, "%s: sysParm->stepInitial %g not > 0", me,
             sysParm->stepInitial);
    return 1;
  }
  CHECK(opporStepScale, 1.0, 5.0);
  CHECK(backStepScale, 0.01, 0.99);
  CHECK(constraintStepMin, 0.00000000000000001, 0.1);
  CHECK(energyDecreaseMin, -0.2, 1.0);
  CHECK(energyDecreasePopCntlMin, -1.0, 1.0);
  CHECK(energyIncreasePermit, 0.0, 1.0);
  CHECK(fracNeighNixedMax, 0.01, 0.99);
  return 0;
}
#undef CHECK

int
pullSysParmSet(pullContext *pctx, int which, double pval) {
  static const char me[]="pullSysParmSet";

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!AIR_IN_OP(pullSysParmUnknown, which, pullSysParmLast)) {
    biffAddf(PULL, "%s: sys parm %d not valid", me, which);
    return 1;
  }
  switch(which) {
  case pullSysParmAlpha:
    pctx->sysParm.alpha = pval;
    break;
  case pullSysParmBeta:
    pctx->sysParm.beta = pval;
    break;
  case pullSysParmGamma:
    pctx->sysParm.gamma = pval;
    break;
  case pullSysParmSeparableGammaLearnRescale:
    pctx->sysParm.separableGammaLearnRescale = pval;
    break;
  case pullSysParmTheta:
    pctx->sysParm.theta = pval;
    break;
  case pullSysParmStepInitial:
    pctx->sysParm.stepInitial = pval;
    break;
  case pullSysParmRadiusSpace:
    pctx->sysParm.radiusSpace = pval;
    break;
  case pullSysParmRadiusScale:
    pctx->sysParm.radiusScale = pval;
    break;
  case pullSysParmBinWidthSpace:
    pctx->sysParm.binWidthSpace = pval;
    break;
  case pullSysParmNeighborTrueProb:
    pctx->sysParm.neighborTrueProb = pval;
    break;
  case pullSysParmProbeProb:
    pctx->sysParm.probeProb = pval;
    break;
  case pullSysParmOpporStepScale:
    pctx->sysParm.opporStepScale = pval;
    break;
  case pullSysParmBackStepScale:
    pctx->sysParm.backStepScale = pval;
    break;
  case pullSysParmEnergyDecreasePopCntlMin:
    pctx->sysParm.energyDecreasePopCntlMin = pval;
    break;
  case pullSysParmEnergyDecreaseMin:
    pctx->sysParm.energyDecreaseMin = pval;
    break;
  case pullSysParmConstraintStepMin:
    pctx->sysParm.constraintStepMin = pval;
    break;
  case pullSysParmEnergyIncreasePermit:
    pctx->sysParm.energyIncreasePermit = pval;
    break;
  case pullSysParmFracNeighNixedMax:
    pctx->sysParm.fracNeighNixedMax = pval;
    break;
  case pullSysParmWall:
    pctx->sysParm.wall = pval;
    break;
  default:
    biffAddf(me, "%s: sorry, sys parm %d valid but not handled?", me, which);
    return 1;
  }
  return 0;
}

/*
******** pullFlagSet
**
** uniform way of setting all the boolean-ish flags
*/
int
pullFlagSet(pullContext *pctx, int which, int flag) {
  static const char me[]="pullFlagSet";

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!AIR_IN_OP(pullFlagUnknown, which, pullFlagLast)) {
    biffAddf(PULL, "%s: flag %d not valid", me, which);
    return 1;
  }
  switch (which) {
  case pullFlagPermuteOnRebin:
    pctx->flag.permuteOnRebin = flag;
    break;
  case pullFlagNoPopCntlWithZeroAlpha:
    pctx->flag.noPopCntlWithZeroAlpha = flag;
    break;
  case pullFlagUseBetaForGammaLearn:
    pctx->flag.useBetaForGammaLearn = flag;
    break;
  case pullFlagRestrictiveAddToBins:
    pctx->flag.restrictiveAddToBins = flag;
    break;
  case pullFlagEnergyFromStrength:
    pctx->flag.energyFromStrength = flag;
    break;
  case pullFlagNixAtVolumeEdgeSpace:
    pctx->flag.nixAtVolumeEdgeSpace = flag;
    break;
  case pullFlagConstraintBeforeSeedThresh:
    pctx->flag.constraintBeforeSeedThresh = flag;
    break;
  case pullFlagNoAdd:
    pctx->flag.noAdd = flag;
    break;
  case pullFlagPopCntlEnoughTest:
    pctx->flag.popCntlEnoughTest = flag;
    break;
  case pullFlagBinSingle:
    pctx->flag.binSingle = flag;
    break;
  case pullFlagAllowCodimension3Constraints:
    pctx->flag.allowCodimension3Constraints = flag;
    break;
  case pullFlagScaleIsTau:
    pctx->flag.scaleIsTau = flag;
    break;
  case pullFlagStartSkipsPoints:
    pctx->flag.startSkipsPoints = flag;
    break;
  default:
    biffAddf(me, "%s: sorry, flag %d valid but not handled?", me, which);
    return 1;
  }
  return 0;
}

/*
** HEY: its really confusing to have the array of per-CONTEXT volumes.
** I know they're there to be copied upon task creation to create the
** per-TASK volumes, but its easy to think that one is supposed to be
** doing something with them, or that changes to them will have some
** effect . . .
*/
int
pullVerboseSet(pullContext *pctx, int verbose) {
  static const char me[]="pullVerboseSet";
  unsigned int volIdx, taskIdx;

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  pctx->verbose = verbose;
  for (volIdx=0; volIdx<pctx->volNum; volIdx++) {
    int v;
    v = verbose > 0 ? verbose - 1 : 0;
    gageParmSet(pctx->vol[volIdx]->gctx, gageParmVerbose, v);
  }
  for (taskIdx=0; taskIdx<pctx->threadNum; taskIdx++) {
    for (volIdx=0; volIdx<pctx->volNum; volIdx++) {
      int v;
      v = verbose > 0 ? verbose - 1 : 0;
      gageParmSet(pctx->task[taskIdx]->vol[volIdx]->gctx, gageParmVerbose, v);
    }
  }
  return 0;
}

int
pullThreadNumSet(pullContext *pctx, unsigned int threadNum) {
  static const char me[]="pullThreadNumSet";

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  pctx->threadNum = threadNum;
  return 0;
}

int
pullRngSeedSet(pullContext *pctx, unsigned int rngSeed) {
  static const char me[]="pullRngSeedSet";

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  pctx->rngSeed = rngSeed;
  return 0;
}

int
pullProgressBinModSet(pullContext *pctx, unsigned int bmod) {
  static const char me[]="pullProgressBinModSet";

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  pctx->progressBinMod = bmod;
  return 0;
}

int
pullCallbackSet(pullContext *pctx,
                void (*iter_cb)(void *data_cb),
                void *data_cb) {
  static const char me[]="pullCallbackSet";

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  pctx->iter_cb = iter_cb;
  pctx->data_cb = data_cb;
  return 0;
}

/*
******** pullInterEnergySet
**
** This is the single function for setting the inter-particle energy,
** which is clumsy because which pullEnergySpecs are necessary is
** different depending on interType.  Pass NULL for those not needed.
**
** Note that all the pullEnergySpecs inside the pctx are created
** by pullContextNew, so they should never be NULL.  When a pullEnergySpec
** is not needed for a given interType, we set it to pullEnergyZero
** and a vector of NaNs.
*/
int
pullInterEnergySet(pullContext *pctx, int interType,
                   const pullEnergySpec *enspR,
                   const pullEnergySpec *enspS,
                   const pullEnergySpec *enspWin) {
  static const char me[]="pullInterEnergySet";
  unsigned int zpi;
  double zeroParm[PULL_ENERGY_PARM_NUM];

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!AIR_IN_OP(pullInterTypeUnknown, interType, pullInterTypeLast)) {
    biffAddf(PULL, "%s: interType %d not valid", me, interType);
    return 1;
  }
  for (zpi=0; zpi<PULL_ENERGY_PARM_NUM; zpi++) {
    zeroParm[zpi] = AIR_NAN;
  }

#define CHECK_N_COPY(X)                                                 \
  if (!ensp##X) {                                                       \
    biffAddf(PULL, "%s: need non-NULL ensp" #X " for interType %s", me, \
             airEnumStr(pullInterType, interType));                     \
    return 1;                                                           \
  }                                                                     \
  pullEnergySpecCopy(pctx->energySpec##X, ensp##X)

  switch (interType) {
  case pullInterTypeJustR:
    /* 1: phi(r,s) = phi_r(r) */
  case pullInterTypeUnivariate:
    /* 2: phi(r,s) = phi_r(u); u = sqrt(r*r+s*s) */
    CHECK_N_COPY(R);
    pullEnergySpecSet(pctx->energySpecS, pullEnergyZero, zeroParm);
    pullEnergySpecSet(pctx->energySpecWin, pullEnergyZero, zeroParm);
    break;
  case pullInterTypeSeparable:
    /* 3: phi(r,s) = phi_r(r)*phi_s(s) */
    CHECK_N_COPY(R);
    CHECK_N_COPY(S);
    pullEnergySpecSet(pctx->energySpecWin, pullEnergyZero, zeroParm);
    break;
  case pullInterTypeAdditive:
    /* 4: phi(r,s) = beta*phi_r(r)*win(s) + (1-beta)*win(r)*phi_s(s) */
    CHECK_N_COPY(R);
    CHECK_N_COPY(S);
    CHECK_N_COPY(Win);
    break;
  default:
    biffAddf(PULL, "%s: interType %d valid but no handled?", me, interType);
    return 1;
  }
#undef CHECK_N_COPY

  pctx->interType = interType;
  return 0;
}

/*
** you can pass in a NULL FILE* if you want
*/
int
pullLogAddSet(pullContext *pctx, FILE *flog) {
  static const char me[]="pullLogAddSet";

  if (!(pctx)) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }

  pctx->logAdd = flog;
  return 0;
}
