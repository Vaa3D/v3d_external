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
** HEY: this has to be threadsafe, at least threadsafe when there
** are no errors, because this can now be called from multiple
** tasks during population control
*/
pullPoint *
pullPointNew(pullContext *pctx) {
  static const char me[]="pullPointNew";
  pullPoint *pnt;
  unsigned int ii;
  size_t pntSize;
  pullPtrPtrUnion pppu;

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return NULL;
  }
  if (!pctx->infoTotalLen) {
    biffAddf(PULL, "%s: can't allocate points w/out infoTotalLen set\n", me);
    return NULL;
  }
  /* Allocate the pullPoint so that it has pctx->infoTotalLen doubles.
     The pullPoint declaration has info[1], hence the "- 1" below */
  pntSize = sizeof(pullPoint) + sizeof(double)*(pctx->infoTotalLen - 1);
  pnt = AIR_CAST(pullPoint *, calloc(1, pntSize));
  if (!pnt) {
    biffAddf(PULL, "%s: couldn't allocate point (info len %u)\n", me,
             pctx->infoTotalLen - 1);
    return NULL;
  }

  pnt->idtag = pctx->idtagNext++;
  pnt->idCC = 0;
  pnt->neighPoint = NULL;
  pnt->neighPointNum = 0;
  pppu.points = &(pnt->neighPoint);
  pnt->neighPointArr = airArrayNew(pppu.v, &(pnt->neighPointNum),
                                   sizeof(pullPoint *),
                                   PULL_POINT_NEIGH_INCR);
  pnt->neighPointArr->noReallocWhenSmaller = AIR_TRUE;
  pnt->neighDistMean = 0;
  ELL_10V_ZERO_SET(pnt->neighCovar);
  pnt->stability = 0.0;
#if PULL_TANCOVAR
  ELL_6V_ZERO_SET(pnt->neighTanCovar);
#endif
  pnt->neighInterNum = 0;
  pnt->stuckIterNum = 0;
#if PULL_PHIST
  pnt->phist = NULL;
  pnt->phistNum = 0;
  pnt->phistArr = airArrayNew(AIR_CAST(void**, &(pnt->phist)),
                              &(pnt->phistNum),
                              5*sizeof(double), 32);
#endif
  pnt->status = 0;
  ELL_4V_SET(pnt->pos, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN);
  pnt->energy = AIR_NAN;
  ELL_4V_SET(pnt->force, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN);
  pnt->stepEnergy = pctx->sysParm.stepInitial;
  pnt->stepConstr = pctx->sysParm.stepInitial;
  for (ii=0; ii<pctx->infoTotalLen; ii++) {
    pnt->info[ii] = AIR_NAN;
  }
  return pnt;
}

pullPoint *
pullPointNix(pullPoint *pnt) {

  pnt->neighPointArr = airArrayNuke(pnt->neighPointArr);
#if PULL_PHIST
  pnt->phistArr = airArrayNuke(pnt->phistArr);
#endif
  airFree(pnt);
  return NULL;
}

#if PULL_PHIST
void
_pullPointHistInit(pullPoint *point) {

  airArrayLenSet(point->phistArr, 0);
  return;
}

void
_pullPointHistAdd(pullPoint *point, int cond) {
  unsigned int phistIdx;

  phistIdx = airArrayLenIncr(point->phistArr, 1);
  ELL_4V_COPY(point->phist + 5*phistIdx, point->pos);
  (point->phist + 5*phistIdx)[3] = 1.0;
  (point->phist + 5*phistIdx)[4] = cond;
  return;
}
#endif

/*
** HEY: there should be something like a "map" over all the points,
** which could implement all these redundant functions
*/

unsigned int
pullPointNumberFilter(const pullContext *pctx,
                      unsigned int idtagMin,
                      unsigned int idtagMax) {
  unsigned int binIdx, pointNum;
  const pullBin *bin;
  const pullPoint *point;

  pointNum = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    unsigned int pointIdx;
    bin = pctx->bin + binIdx;
    if (0 == idtagMin && 0 == idtagMax) {
      pointNum += bin->pointNum;
    } else {
      for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
        point = bin->point[pointIdx];
        pointNum += (idtagMin <= point->idtag
                     && (0 == idtagMax
                         || point->idtag <= idtagMax));
      }
    }
  }
  return pointNum;
}

unsigned int
pullPointNumber(const pullContext *pctx) {

  return pullPointNumberFilter(pctx, 0, 0);
}

double
_pullEnergyTotal(const pullContext *pctx) {
  unsigned int binIdx, pointIdx;
  const pullBin *bin;
  const pullPoint *point;
  double sum;

  sum = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      sum += point->energy;
    }
  }
  return sum;
}

void
_pullPointStepEnergyScale(pullContext *pctx, double scale) {
  unsigned int binIdx, pointIdx;
  const pullBin *bin;
  pullPoint *point;

  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      point->stepEnergy *= scale;
      point->stepEnergy = AIR_MIN(point->stepEnergy,
                                  _PULL_STEP_ENERGY_MAX);
    }
  }
  return;
}

double
_pullStepInterAverage(const pullContext *pctx) {
  unsigned int binIdx, pointIdx, pointNum;
  const pullBin *bin;
  const pullPoint *point;
  double sum, avg;

  sum = 0;
  pointNum = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    pointNum += bin->pointNum;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      sum += point->stepEnergy;
    }
  }
  avg = (!pointNum ? AIR_NAN : sum/pointNum);
  return avg;
}
/* ^^^  vvv HEY HEY HEY: COPY + PASTE COPY + PASTE COPY + PASTE */
double
_pullStepConstrAverage(const pullContext *pctx) {
  unsigned int binIdx, pointIdx, pointNum;
  const pullBin *bin;
  const pullPoint *point;
  double sum, avg;

  sum = 0;
  pointNum = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    pointNum += bin->pointNum;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      sum += point->stepConstr;
    }
  }
  avg = (!pointNum ? AIR_NAN : sum/pointNum);
  return avg;
}

/*
** convenience function for learning a scalar AND its gradient or hessian
**
** NOTE: this is where pullInfoSeedThresh and pullInfoLiveThresh are
** adjusted according to sysParm.theta (kind of a hack)
*/
double
pullPointScalar(const pullContext *pctx, const pullPoint *point, int sclInfo,
                /* output */
                double grad[3], double hess[9]) {
  static const char me[]="pullPointScalar";
  double scl;
  const pullInfoSpec *ispec;
  int gradInfo[1+PULL_INFO_MAX] = {
    0,                        /* pullInfoUnknown */
    0,                        /* pullInfoTensor */
    0,                        /* pullInfoTensorInverse */
    0,                        /* pullInfoHessian */
    pullInfoInsideGradient,   /* pullInfoInside */
    0,                        /* pullInfoInsideGradient */
    pullInfoHeightGradient,   /* pullInfoHeight */
    0,                        /* pullInfoHeightGradient */
    0,                        /* pullInfoHeightHessian */
    0,                        /* pullInfoHeightLaplacian */
    0,                        /* pullInfoSeedPreThresh */
    0,                        /* pullInfoSeedThresh */
    0,                        /* pullInfoLiveThresh */
    0,                        /* pullInfoLiveThresh2 */
    0,                        /* pullInfoLiveThresh3 */
    0,                        /* pullInfoTangent1 */
    0,                        /* pullInfoTangent2 */
    0,                        /* pullInfoNegativeTangent1 */
    0,                        /* pullInfoNegativeTangent2 */
    pullInfoIsovalueGradient, /* pullInfoIsovalue */
    0,                        /* pullInfoIsovalueGradient */
    0,                        /* pullInfoIsovalueHessian */
    0,                        /* pullInfoStrength */
  };
  int hessInfo[1+PULL_INFO_MAX] = {
    0,                        /* pullInfoUnknown */
    0,                        /* pullInfoTensor */
    0,                        /* pullInfoTensorInverse */
    0,                        /* pullInfoHessian */
    0,                        /* pullInfoInside */
    0,                        /* pullInfoInsideGradient */
    pullInfoHeightHessian,    /* pullInfoHeight */
    0,                        /* pullInfoHeightGradient */
    0,                        /* pullInfoHeightHessian */
    0,                        /* pullInfoHeightLaplacian */
    0,                        /* pullInfoSeedPreThresh */
    0,                        /* pullInfoSeedThresh */
    0,                        /* pullInfoLiveThresh */
    0,                        /* pullInfoLiveThresh2 */
    0,                        /* pullInfoLiveThresh3 */
    0,                        /* pullInfoTangent1 */
    0,                        /* pullInfoTangent2 */
    0,                        /* pullInfoNegativeTangent1 */
    0,                        /* pullInfoNegativeTangent2 */
    pullInfoIsovalueHessian,  /* pullInfoIsovalue */
    0,                        /* pullInfoIsovalueGradient */
    0,                        /* pullInfoIsovalueHessian */
    0,                        /* pullInfoStrength */
  };
  const unsigned int *infoIdx;

  infoIdx = pctx->infoIdx;
  ispec = pctx->ispec[sclInfo];
  /* NB: this "scl" is not scale-space scale; its the scaling
     of the scalar.  this is getting confusing ... */
  scl = point->info[infoIdx[sclInfo]];
  scl = (scl - ispec->zero)*ispec->scale;
  if (0 && _pullVerbose) {
    if (pullInfoSeedThresh == sclInfo) {
      printf("!%s: seed thresh (%g - %g)*%g == %g\n", me,
             point->info[infoIdx[sclInfo]], ispec->zero, ispec->scale, scl);
    }
  }
  /* HEY: this logic is confused and the implementation is confused;
     this should be removed before release */
  if (pullInfoLiveThresh == sclInfo
      || pullInfoSeedThresh == sclInfo) {
    scl -= (pctx->sysParm.theta)*(point->pos[3])*(point->pos[3]);
  }
  if (0 && _pullVerbose) {
    if (pullInfoSeedThresh == sclInfo) {
      printf("!%s:  ---> w/ theta %g -> %g\n", me,
             pctx->sysParm.theta, scl);
    }
  }
  /*
    learned: this wasn't thought through: the idea was that the height
    *laplacian* answer should be transformed by the *height* zero and
    scale.  scale might make sense, but not zero.  This cost a few
    hours of tracking down the fact that the first zero-crossing
    detection phase of the lapl constraint was failing because the
    laplacian was vacillating around hspec->zero, not 0.0 . . .
  if (pullInfoHeightLaplacian == sclInfo) {
    const pullInfoSpec *hspec;
    hspec = pctx->ispec[pullInfoHeight];
    scl = (scl - hspec->zero)*hspec->scale;
  } else {
    scl = (scl - ispec->zero)*ispec->scale;
  }
  */
  /*
  printf("%s = (%g - %g)*%g = %g*%g = %g = %g\n",
         airEnumStr(pullInfo, sclInfo),
         point->info[infoIdx[sclInfo]],
         ispec->zero, ispec->scale,
         point->info[infoIdx[sclInfo]] - ispec->zero, ispec->scale,
         (point->info[infoIdx[sclInfo]] - ispec->zero)*ispec->scale,
         scl);
  */
  if (grad && gradInfo[sclInfo]) {
    const double *ptr = point->info + infoIdx[gradInfo[sclInfo]];
    ELL_3V_SCALE(grad, ispec->scale, ptr);
  }
  if (hess && hessInfo[sclInfo]) {
    const double *ptr = point->info + infoIdx[hessInfo[sclInfo]];
    ELL_3M_SCALE(hess, ispec->scale, ptr);
  }
  return scl;
}

int
pullProbe(pullTask *task, pullPoint *point) {
  static const char me[]="pullProbe";
  unsigned int ii, gret=0;
  int edge;
  /*
  fprintf(stderr, "!%s: task->probeSeedPreThreshOnly = %d\n", me,
          task->probeSeedPreThreshOnly);
  */
#if 0
  static int opened=AIR_FALSE;
  static FILE *flog;
#endif

#if 0
  static int logIdx=0, logDone=AIR_FALSE, logStarted=AIR_FALSE;
  static Nrrd *nlog;
  static double *log=NULL;
  if (!logStarted) {
    if (81 == point->idtag) {
      printf("\n\n%s: ###### HELLO begin logging . . .\n\n\n", me);
      /* knowing the logIdx at the end of logging . . . */
      nlog = nrrdNew();
      nrrdMaybeAlloc_va(nlog, nrrdTypeDouble, 2,
                        AIR_CAST(size_t, 25),
                        AIR_CAST(size_t, 2754));
      log = AIR_CAST(double*, nlog->data);
      logStarted = AIR_TRUE;
    }
  }
#endif

  if (!ELL_4V_EXISTS(point->pos)) {
    fprintf(stderr, "%s: pnt %u non-exist pos (%g,%g,%g,%g)\n\n!!!\n\n\n",
            me, point->idtag, point->pos[0], point->pos[1],
            point->pos[2], point->pos[3]);
    biffAddf(PULL, "%s: pnt %u non-exist pos (%g,%g,%g,%g)\n\n!!!\n\n\n",
             me, point->idtag, point->pos[0], point->pos[1],
             point->pos[2], point->pos[3]);
    return 1;
    /* can't probe, but make it go away as quickly as possible */
    /*
    ELL_4V_SET(point->pos, 0, 0, 0, 0);
    point->status |= PULL_STATUS_NIXME_BIT;
    return 0;
    */
  }
  if (task->pctx->verbose > 3) {
    printf("%s: hello; probing %u volumes\n", me, task->pctx->volNum);
  }
  edge = AIR_FALSE;
  task->pctx->count[pullCountProbe] += 1;
  for (ii=0; ii<task->pctx->volNum; ii++) {
    pullVolume *vol;
    vol = task->vol[ii];
    if (task->probeSeedPreThreshOnly
        && !(vol->forSeedPreThresh)) {
      /* we're here *only* to probe SeedPreThresh,
         and this volume isn't used for that */
      continue;
    }
    if (task->pctx->iter && vol->seedOnly) {
      /* its after the 1st iteration (#0), and this vol is only for seeding */
      continue;
    }
    /* HEY should task->vol[ii]->scaleNum be the using-scale-space test? */
    if (!task->vol[ii]->ninScale) {
      /*
        if (81 == point->idtag) {
        printf("%s: probing vol[%u] @ %g %g %g\n", me, ii,
        point->pos[0], point->pos[1], point->pos[2]);
        }
      */
      gret = gageProbeSpace(task->vol[ii]->gctx,
                            point->pos[0], point->pos[1], point->pos[2],
                            AIR_FALSE /* index-space */,
                            AIR_TRUE /* clamp */);
    } else {
      if (task->pctx->verbose > 3) {
        printf("%s: vol[%u] has scale (%u)-> "
               "gageStackProbeSpace(%p) (v %d)\n",
               me, ii, task->vol[ii]->scaleNum,
               AIR_VOIDP(task->vol[ii]->gctx),
               task->vol[ii]->gctx->verbose);
      }
      /*
        if (81 == point->idtag) {
        printf("%s: probing vol[%u] @ %g %g %g %g\n", me, ii,
        point->pos[0], point->pos[1], point->pos[2], point->pos[3]);
        }
      */
      gret = gageStackProbeSpace(task->vol[ii]->gctx,
                                 point->pos[0], point->pos[1], point->pos[2],
                                 (task->pctx->flag.scaleIsTau
                                  ? gageSigOfTau(point->pos[3])
                                  : point->pos[3]),
                                 AIR_FALSE /* index-space */,
                                 AIR_TRUE /* clamp */);
    }
    if (gret) {
      biffAddf(PULL, "%s: probe failed on vol %u/%u: (%d) %s", me,
               ii, task->pctx->volNum,
               task->vol[ii]->gctx->errNum, task->vol[ii]->gctx->errStr);
      return 1;
    }
    /*
    if (!edge
        && AIR_ABS(point->pos[1] - 67) < 1
        && AIR_ABS(point->pos[2] - 67) < 1
        && point->pos[3] > 3.13
        && !!task->vol[ii]->gctx->edgeFrac) {
      fprintf(stderr, "!%s(%u @ %g,%g,%g,%g): "
              "vol[%u]->gctx->edgeFrac %g => edge bit on\n",
              me, point->idtag,
              point->pos[0], point->pos[1],
              point->pos[2], point->pos[3],
              ii, task->vol[ii]->gctx->edgeFrac);
    }
    */
    edge |= !!task->vol[ii]->gctx->edgeFrac;
  }
  if (edge) {
    point->status |= PULL_STATUS_EDGE_BIT;
  } else {
    point->status &= ~PULL_STATUS_EDGE_BIT;
  }

  /* maybe is a little stupid to have the infos indexed this way,
     since it means that we always have to loop through all indices,
     but at least the compiler can unroll it . . . */
  for (ii=0; ii<=PULL_INFO_MAX; ii++) {
    unsigned int alen, aidx;
    const pullInfoSpec *ispec;
    ispec = task->pctx->ispec[ii];
    if (ispec) {
      alen = _pullInfoLen[ii];
      aidx = task->pctx->infoIdx[ii];
      if (pullSourceGage == ispec->source) {
        _pullInfoCopy[alen](point->info + aidx, task->ans[ii]);
        /*
        if (81 == point->idtag) {
          pullVolume *vol;
          pullInfoSpec *isp;
          isp = task->pctx->ispec[ii];
          vol = task->pctx->vol[isp->volIdx];
          if (1 == alen) {
            printf("!%s: info[%u] %s: %s(\"%s\") = %g\n", me,
                   ii, airEnumStr(pullInfo, ii),
                   airEnumStr(vol->kind->enm, isp->item),
                   vol->name, task->ans[ii][0]);
          } else {
            unsigned int vali;
            printf("!%s: info[%u] %s: %s(\"%s\") =\n", me,
                   ii, airEnumStr(pullInfo, ii),
                   airEnumStr(vol->kind->enm, isp->item), vol->name);
            for (vali=0; vali<alen; vali++) {
              printf("!%s:    [%u]  %g\n", me, vali,
                     task->ans[ii][vali]);
            }
          }
        }
        */
      } else if (pullSourceProp == ispec->source) {
        switch (ispec->prop) {
        case pullPropIdtag:
          point->info[aidx] = point->idtag;
          break;
        case pullPropIdCC:
          point->info[aidx] = point->idCC;
          break;
        case pullPropEnergy:
          point->info[aidx] = point->energy;
          break;
        case pullPropStepEnergy:
          point->info[aidx] = point->stepEnergy;
          break;
        case pullPropStepConstr:
          point->info[aidx] = point->stepConstr;
          break;
        case pullPropStuck:
          point->info[aidx] = ((point->status & PULL_STATUS_STUCK_BIT)
                               ? point->stuckIterNum
                               : 0);
          break;
        case pullPropPosition:
          ELL_4V_COPY(point->info + aidx, point->pos);
          break;
        case pullPropForce:
          ELL_4V_COPY(point->info + aidx, point->force);
          break;
        case pullPropNeighDistMean:
          point->info[aidx] = point->neighDistMean;
          break;
        case pullPropScale:
          point->info[aidx] = point->pos[3];
          break;
        case pullPropNeighCovar:
          ELL_10V_COPY(point->info + aidx, point->neighCovar);
          break;
        case pullPropNeighCovar7Ten:
          TEN_T_SET(point->info + aidx, 1.0f,
                    point->neighCovar[0],
                    point->neighCovar[1],
                    point->neighCovar[2],
                    point->neighCovar[4],
                    point->neighCovar[5],
                    point->neighCovar[7]);
          break;
#if PULL_TANCOVAR
        case pullPropNeighTanCovar:
          TEN_T_SET(point->info + aidx, 1.0f,
                    point->neighTanCovar[0],
                    point->neighTanCovar[1],
                    point->neighTanCovar[2],
                    point->neighTanCovar[3],
                    point->neighTanCovar[4],
                    point->neighTanCovar[5]);
          break;
#endif
        case pullPropStability:
          point->info[aidx] = point->stability;
          break;
        default:
          break;
        }
      }
    }
  }

#if 0
  if (logStarted && !logDone) {
    unsigned int ai;
    /* the actual logging */
    log[0] = point->idtag;
    ELL_4V_COPY(log + 1, point->pos);
    for (ai=0; ai<20; ai++) {
      log[5 + ai] = point->info[ai];
    }
    log += nlog->axis[0].size;
    logIdx++;
    if (1 == task->pctx->iter && 81 == point->idtag) {
      printf("\n\n%s: ###### OKAY done logging (%u). . .\n\n\n", me, logIdx);
      nrrdSave("probelog.nrrd", nlog, NULL);
      nlog = nrrdNuke(nlog);
      logDone = AIR_TRUE;
    }
  }
#endif


#if 0
  if (!opened) {
    flog = fopen("flog.txt", "w");
    opened = AIR_TRUE;
  }
  if (opened) {
    fprintf(flog, "%s(%u): spthr(%g,%g,%g,%g) = %g\n", me, point->idtag,
            point->pos[0], point->pos[1], point->pos[2], point->pos[3],
            point->info[task->pctx->infoIdx[pullInfoSeedPreThresh]]);
  }
#endif

  return 0;
}

static int
_threshFail(const pullContext *pctx, const pullPoint *point, int info) {
  /* static const char me[]="_threshFail"; */
  double val;
  int ret;

  if (pctx->ispec[info]) {
    val = pullPointScalar(pctx, point, info, NULL, NULL);
    ret = (val < 0);
    /*
    fprintf(stderr, "%s(%s): val=%g -> ret=%d\n", me,
            airEnumStr(pullInfo, info), val, ret);
    */
  } else {
    ret = AIR_FALSE;
  }
  return ret;
}

int
pullPointInitializePerVoxel(const pullContext *pctx,
                            const unsigned int pointIdx,
                            pullPoint *point, pullVolume *scaleVol,
                            /* output */
                            int *createFailP) {
  static const char me[]="pullPointInitializePerVoxel";
  unsigned int vidx[3], pix;
  double iPos[3];
  airRandMTState *rng;
  pullVolume *seedVol;
  gageShape *seedShape;
  int reject, rejectEdge, constrFail;
  unsigned int k;

  seedVol = pctx->vol[pctx->ispec[pullInfoSeedThresh]->volIdx];
  seedShape = seedVol->gctx->shape;
  rng = pctx->task[0]->rng;

  /* Obtain voxel and indices from pointIdx */
  /* axis ordering for this is x, y, z, scale */
  pix = pointIdx;
  if (pctx->initParm.pointPerVoxel > 0) {
    pix /= pctx->initParm.pointPerVoxel;
  } else {
    pix *= -pctx->initParm.pointPerVoxel;
  }
  vidx[0] = pix % seedShape->size[0];
  pix = (pix - vidx[0])/seedShape->size[0];
  vidx[1] = pix % seedShape->size[1];
  pix = (pix - vidx[1])/seedShape->size[1];
  if (pctx->initParm.ppvZRange[0] <= pctx->initParm.ppvZRange[1]) {
    unsigned int zrn;
    zrn = pctx->initParm.ppvZRange[1] - pctx->initParm.ppvZRange[0] + 1;
    vidx[2] = (pix % zrn) + pctx->initParm.ppvZRange[0];
    pix = (pix - (pix % zrn))/zrn;
  } else {
    vidx[2] = pix % seedShape->size[2];
    pix = (pix - vidx[2])/seedShape->size[2];
  }
  for (k=0; k<=2; k++) {
    iPos[k] = vidx[k] + pctx->initParm.jitter*(airDrandMT_r(rng)-0.5);
  }
  gageShapeItoW(seedShape, point->pos, iPos);

  if (0 && _pullVerbose) {
    printf("!%s: pointIdx %u -> vidx %u %u %u (%u)\n"
           "       -> iPos %g %g %g -> wPos %g %g %g\n",
           me, pointIdx, vidx[0], vidx[1], vidx[2], pix,
           iPos[0], iPos[1], iPos[2],
           point->pos[0], point->pos[1], point->pos[2]);
  }

  /* Compute sigma coordinate from pix */
  if (pctx->haveScale) {
    int outside;
    double aidx, bidx;
    /* pix should already be integer in [0, pctx->samplesAlongScaleNum-1)]. */
    aidx = pix + pctx->initParm.jitter*(airDrandMT_r(rng)-0.5);
    bidx = AIR_AFFINE(-0.5, aidx, pctx->initParm.samplesAlongScaleNum-0.5,
                      0.0, scaleVol->scaleNum-1);
    point->pos[3] = gageStackItoW(scaleVol->gctx, bidx, &outside);
    if (pctx->flag.scaleIsTau) {
      point->pos[3] = gageTauOfSig(point->pos[3]);
    }
    if (0 && _pullVerbose) {
      printf("!%s(%u): pix %u -> a %g b %g -> wpos %g\n", me, point->idtag,
             pix, aidx, bidx, point->pos[3]);
    }
  } else {
    point->pos[3] = 0;
  }

  if (pctx->ispec[pullInfoSeedPreThresh]) {
    /* we first do a special-purpose probe just for SeedPreThresh */
    pctx->task[0]->probeSeedPreThreshOnly = AIR_TRUE;
    if (pullProbe(pctx->task[0], point)) {
      biffAddf(PULL, "%s: pre-probing pointIdx %u of world", me, pointIdx);
      return 1;
    }
    pctx->task[0]->probeSeedPreThreshOnly = AIR_FALSE;
    if (_threshFail(pctx, point, pullInfoSeedPreThresh)) {
      reject = AIR_TRUE;
      /* HEY! this obviously need to be re-written */
      goto finish;
    }
  }
  /* else, we didn't have a SeedPreThresh, or we did, and we passed
     it.  Now we REDO the probe, including possibly re-learning
     SeedPreThresh, which is silly, but the idea is that this is a
     small price compared to what has been saved by avoiding all the
     gageProbe()s on volumes unrelated to SeedPreThresh */
  if (pullProbe(pctx->task[0], point)) {
    biffAddf(PULL, "%s: probing pointIdx %u of world", me, pointIdx);
    return 1;
  }

  constrFail = AIR_FALSE;
  reject = AIR_FALSE;

  /* Check we pass pre-threshold */
  if (!reject) reject |= _threshFail(pctx, point, pullInfoSeedPreThresh);

  if (!pctx->flag.constraintBeforeSeedThresh) {
    /* we should be guaranteed to have a seed thresh info */
    if (!reject) reject |= _threshFail(pctx, point, pullInfoSeedThresh);
    if (pctx->initParm.liveThreshUse) {
      if (!reject) reject |= _threshFail(pctx, point, pullInfoLiveThresh);
      if (!reject) reject |= _threshFail(pctx, point, pullInfoLiveThresh2);
      if (!reject) reject |= _threshFail(pctx, point, pullInfoLiveThresh3);
    }
  }

  if (!reject && pctx->constraint) {
    if (_pullConstraintSatisfy(pctx->task[0], point,
                               10*_PULL_CONSTRAINT_TRAVEL_MAX,
                               &constrFail)) {
      biffAddf(PULL, "%s: on pnt %u",
               me, pointIdx);
      return 1;
    }
    reject |= constrFail;
    /* post constraint-satisfaction, we certainly have to assert thresholds */
    if (!reject) reject |= _threshFail(pctx, point, pullInfoSeedThresh);
    if (pctx->initParm.liveThreshUse) {
      if (!reject) reject |= _threshFail(pctx, point, pullInfoLiveThresh);
      if (!reject) reject |= _threshFail(pctx, point, pullInfoLiveThresh2);
      if (!reject) reject |= _threshFail(pctx, point, pullInfoLiveThresh3);
    }
    if (pctx->flag.nixAtVolumeEdgeSpace
        && (point->status & PULL_STATUS_EDGE_BIT)) {
      rejectEdge = AIR_TRUE;
    } else {
      rejectEdge = AIR_FALSE;
    }
    reject |= rejectEdge;
    if (pctx->verbose > 1) {
      fprintf(stderr, "%s(%u): constr %d, seed %d, thresh %d %d %d, edge %d\n",
              me, point->idtag, constrFail,
              _threshFail(pctx, point, pullInfoSeedThresh),
              _threshFail(pctx, point, pullInfoLiveThresh),
              _threshFail(pctx, point, pullInfoLiveThresh2),
              _threshFail(pctx, point, pullInfoLiveThresh3), rejectEdge);
    }
  } else {
    constrFail = AIR_FALSE;
  }

 finish:
  /* Gather consensus */
  if (reject) {
    *createFailP = AIR_TRUE;
  } else {
    *createFailP = AIR_FALSE;
  }

  return 0;
}

static void
_pullUnitToWorld(const pullContext *pctx, const pullVolume *scaleVol,
                 double wrld[4], const double unit[4]) {
  /* static const char me[]="_pullUnitToWorld"; */

  wrld[0] = AIR_AFFINE(0.0, unit[0], 1.0, pctx->bboxMin[0], pctx->bboxMax[0]);
  wrld[1] = AIR_AFFINE(0.0, unit[1], 1.0, pctx->bboxMin[1], pctx->bboxMax[1]);
  wrld[2] = AIR_AFFINE(0.0, unit[2], 1.0, pctx->bboxMin[2], pctx->bboxMax[2]);
  if (pctx->haveScale) {
    double sridx;
    int outside;
    sridx = AIR_AFFINE(0.0, unit[3], 1.0, 0, scaleVol->scaleNum-1);
    wrld[3] = gageStackItoW(scaleVol->gctx, sridx, &outside);
    if (pctx->flag.scaleIsTau) {
      wrld[3] = gageTauOfSig(wrld[3]);
    }
  } else {
    wrld[3] = 0.0;
  }
  /*
  fprintf(stderr, "!%s: (%g,%g,%g,%g) --> (%g,%g,%g,%g)\n", me,
          unit[0], unit[1], unit[2], unit[3],
          wrld[0], wrld[1], wrld[2], wrld[3]);
  */
  return;
}

int
pullPointInitializeRandomOrHalton(pullContext *pctx,
                                  const unsigned int pointIdx,
                                  pullPoint *point, pullVolume *scaleVol) {
  static const char me[]="pullPointInitializeRandomOrHalton";
  int reject, verbo;
  airRandMTState *rng;
  unsigned int threshFailCount = 0, spthreshFailCount = 0,
    constrFailCount = 0;
  rng = pctx->task[0]->rng;

  do {
    double rpos[4];

    reject = AIR_FALSE;
    _pullPointHistInit(point);
    /* Populate tentative random point */
    if (pullInitMethodHalton == pctx->initParm.method) {
      airHalton(rpos, (pointIdx + threshFailCount + constrFailCount
                       + pctx->haltonOffset + pctx->initParm.haltonStartIndex),
                airPrimeList, 4);
      /*
      fprintf(stderr, "!%s(%u/%u): halton(%u=%u+%u+%u+%u+%u) => %g %g %g %g\n",
              me, pointIdx, pctx->idtagNext,
              (pointIdx + threshFailCount + constrFailCount +
               pctx->haltonOffset + pctx->initParm.haltonStartIndex),
              pointIdx, threshFailCount, constrFailCount,
              pctx->haltonOffset, pctx->initParm.haltonStartIndex,
              rpos[0], rpos[1], rpos[2], rpos[3]);
      */
                    /*
      fprintf(stderr, "%g %g %g %g ",
              rpos[0], rpos[1], rpos[2], rpos[3]);
              */
    } else {
      ELL_3V_SET(rpos, airDrandMT_r(rng), airDrandMT_r(rng), airDrandMT_r(rng));
      if (pctx->haveScale) {
        rpos[3] = airDrandMT_r(rng);
      }
    }
    _pullUnitToWorld(pctx, scaleVol, point->pos, rpos);
    /*
    verbo = (AIR_ABS(-0.246015 - point->pos[0]) < 0.1 &&
             AIR_ABS(-144.78 - point->pos[0]) < 0.1 &&
             AIR_ABS(-85.3813 - point->pos[0]) < 0.1);
    */
    verbo = AIR_FALSE;
    if (verbo) {
      fprintf(stderr, "%s: verbo on for point %u at %g %g %g %g\n", me,
              point->idtag, point->pos[0], point->pos[1],
              point->pos[2], point->pos[3]);
    }
    _pullPointHistAdd(point, pullCondOld);

    /* HEY copy and paste */
    if (pctx->ispec[pullInfoSeedPreThresh]) {
      /* we first do a special-purpose probe just for SeedPreThresh */
      pctx->task[0]->probeSeedPreThreshOnly = AIR_TRUE;
      if (pullProbe(pctx->task[0], point)) {
        biffAddf(PULL, "%s: pre-probing pointIdx %u of world", me, pointIdx);
        return 1;
      }
      pctx->task[0]->probeSeedPreThreshOnly = AIR_FALSE;
      if (_threshFail(pctx, point, pullInfoSeedPreThresh)) {
        threshFailCount++;
        spthreshFailCount++;
        reject = AIR_TRUE;
        goto reckoning;
      }
    }
    if (pullProbe(pctx->task[0], point)) {
      biffAddf(PULL, "%s: probing pointIdx %u of world", me, pointIdx);
      return 1;
    }
    /* Check we pass pre-threshold */
#define THRESH_TEST(INFO) \
    if (pctx->ispec[INFO] && _threshFail(pctx, point, INFO)) { \
      threshFailCount++; \
      reject = AIR_TRUE; \
      goto reckoning; \
    }
    if (!pctx->flag.constraintBeforeSeedThresh) {
      THRESH_TEST(pullInfoSeedThresh);
      if (pctx->initParm.liveThreshUse) {
        THRESH_TEST(pullInfoLiveThresh);
        THRESH_TEST(pullInfoLiveThresh2);
        THRESH_TEST(pullInfoLiveThresh3);
      }
    }

    if (pctx->constraint) {
      int constrFail;
      if (_pullConstraintSatisfy(pctx->task[0], point,
                                 _PULL_CONSTRAINT_TRAVEL_MAX,
                                 &constrFail)) {
        biffAddf(PULL, "%s: trying constraint on point %u", me, pointIdx);
        return 1;
      }
      if (constrFail) {
        constrFailCount++;
        reject = AIR_TRUE;
        goto reckoning;
      }
      /* post constraint-satisfaction, we certainly have to assert thresholds */
      THRESH_TEST(pullInfoSeedThresh);
      if (pctx->initParm.liveThreshUse) {
        THRESH_TEST(pullInfoLiveThresh);
        THRESH_TEST(pullInfoLiveThresh2);
        THRESH_TEST(pullInfoLiveThresh3);
      }
    }

  reckoning:
    if (reject) {
      if (threshFailCount + constrFailCount >= _PULL_RANDOM_SEED_TRY_MAX) {
        /* Very bad luck; we've too many times */
        biffAddf(PULL, "%s: failed too often (%u times) placing point %u: "
                 "%u fails on thresh (%u on pre-thresh), %u on constr",
                 me, _PULL_RANDOM_SEED_TRY_MAX, pointIdx,
                 threshFailCount, spthreshFailCount, constrFailCount);
        return 1;
      }
    }
  } while (reject);

  if (pullInitMethodHalton == pctx->initParm.method) {
    pctx->haltonOffset += threshFailCount + constrFailCount;
  }

  return 0;
}

int
pullPointInitializeGivenPos(pullContext *pctx,
                            const double *posData,
                            const unsigned int pointIdx,
                            pullPoint *point,
                            /* output */
                            int *createFailP) {
  static const char me[]="pullPointInitializeGivenPos";
  int reject, rejectEdge;

  /* Copy nrrd point into pullPoint */
  ELL_4V_COPY(point->pos, posData + 4*pointIdx);

  /*
  if (AIR_ABS(247.828 - point->pos[0]) < 0.1 &&
      AIR_ABS(66.8817 - point->pos[1]) < 0.1 &&
      AIR_ABS(67.0031 - point->pos[2]) < 0.1) {
    fprintf(stderr, "%s: --------- point %u at %g %g %g %g\n", me,
            point->idtag,
            point->pos[0], point->pos[1],
            point->pos[2], point->pos[3]);
  }
  */

  /* we're dictating positions, but still have to do initial probe,
     and possibly liveThresholding */
  if (pullProbe(pctx->task[0], point)) {
    biffAddf(PULL, "%s: probing pointIdx %u of npos", me, pointIdx);
    return 1;
  }
  reject = AIR_FALSE;
  if (pctx->flag.nixAtVolumeEdgeSpace
      && (point->status & PULL_STATUS_EDGE_BIT)) {
    rejectEdge = AIR_TRUE;
  } else {
    rejectEdge = AIR_FALSE;
  }
  reject |= rejectEdge;
  if (pctx->initParm.liveThreshUse) {
    if (!reject) reject |= _threshFail(pctx, point, pullInfoLiveThresh);
    if (!reject) reject |= _threshFail(pctx, point, pullInfoLiveThresh2);
    if (!reject) reject |= _threshFail(pctx, point, pullInfoLiveThresh3);
  }
  /*
  if (reject) {
    fprintf(stderr, "!%s(%u): edge %d thresh %d %d %d\n",
            me, point->idtag, rejectEdge,
            _threshFail(pctx, point, pullInfoLiveThresh),
            _threshFail(pctx, point, pullInfoLiveThresh2),
            _threshFail(pctx, point, pullInfoLiveThresh3));
  }
  */
  if (reject) {
    *createFailP = AIR_TRUE;
  } else {
    *createFailP = AIR_FALSE;
  }

  return 0;
}

/*
** _pullPointSetup sets:
**
** This is only called by the master thread
**
** this should set stuff to be like after an update stage and
** just before the rebinning
*/
int
_pullPointSetup(pullContext *pctx) {
  static const char me[]="_pullPointSetup";
  char doneStr[AIR_STRLEN_SMALL];
  unsigned int pointIdx, binIdx, tick, pn;
  pullPoint *point;
  pullBin *bin;
  int createFail,added;
  airArray *mop;
  Nrrd *npos;
  pullVolume *seedVol, *scaleVol;
  gageShape *seedShape;
  double factor, *posData;
  unsigned int totalNumPoints, voxNum, ii;

  /* on using pullBinsPointMaybeAdd: This is used only in the context
     of constraints; it makes sense to be a little more selective
     about adding points, so that they aren't completely piled on top
     of each other. This relies on _PULL_BINNING_MAYBE_ADD_THRESH: its
     tempting to set this value high, to more aggressively limit the
     number of points added, but that's really the job of population
     control, and we can't always guarantee that constraint manifolds
     will be well-sampled (with respect to pctx->radiusSpace) to start
     with */

  if (pctx->verbose) {
    printf("%s: beginning . . . ", me);
    fflush(stdout);
  }
  mop = airMopNew();
  switch (pctx->initParm.method) {
  case pullInitMethodGivenPos:
    npos = nrrdNew();
    airMopAdd(mop, npos, (airMopper)nrrdNuke, airMopAlways);
    /* even if npos came in as double, we have to copy it */
    if (nrrdConvert(npos, pctx->initParm.npos, nrrdTypeDouble)) {
      biffMovef(PULL, NRRD, "%s: trouble converting npos", me);
      airMopError(mop); return 1;
    }
    posData = AIR_CAST(double *, npos->data);
    if (pctx->initParm.numInitial || pctx->initParm.pointPerVoxel) {
      printf("%s: with npos, overriding both numInitial (%u) "
             "and pointPerVoxel (%d)\n", me, pctx->initParm.numInitial,
             pctx->initParm.pointPerVoxel);
    }
    totalNumPoints = AIR_CAST(unsigned int, npos->axis[1].size);
    break;
  case pullInitMethodPointPerVoxel:
    npos = NULL;
    posData = NULL;
    if (pctx->initParm.numInitial && pctx->verbose) {
      printf("%s: pointPerVoxel %d overrides numInitial (%u)\n", me,
             pctx->initParm.pointPerVoxel, pctx->initParm.numInitial);
    }
    /* Obtain number of voxels */
    seedVol = pctx->vol[pctx->ispec[pullInfoSeedThresh]->volIdx];
    seedShape = seedVol->gctx->shape;
    if (pctx->initParm.ppvZRange[0] <= pctx->initParm.ppvZRange[1]) {
      unsigned int zrn;
      if (!( pctx->initParm.ppvZRange[0] < seedShape->size[2]
             && pctx->initParm.ppvZRange[1] < seedShape->size[2] )) {
        biffAddf(PULL, "%s: ppvZRange[%u,%u] outside volume [0,%u]", me,
                 pctx->initParm.ppvZRange[0], pctx->initParm.ppvZRange[1],
                 seedShape->size[2]-1);
        airMopError(mop); return 1;
      }
      zrn = pctx->initParm.ppvZRange[1] - pctx->initParm.ppvZRange[0] + 1;
      voxNum = seedShape->size[0]*seedShape->size[1]*zrn;
      if (pctx->verbose) {
        printf("%s: vol size %u %u [%u,%u] -> voxNum %u\n", me,
               seedShape->size[0], seedShape->size[1],
               pctx->initParm.ppvZRange[0], pctx->initParm.ppvZRange[1],
               voxNum);
      }
    } else {
      voxNum = seedShape->size[0]*seedShape->size[1]*seedShape->size[2];
      if (pctx->verbose) {
        printf("%s: vol size %u %u %u -> voxNum %u\n", me,
               seedShape->size[0], seedShape->size[1], seedShape->size[2],
               voxNum);
      }
    }

    /* Compute total number of points */
    if (pctx->initParm.pointPerVoxel > 0) {
      factor = pctx->initParm.pointPerVoxel;
    } else {
      factor = -1.0/pctx->initParm.pointPerVoxel;
    }
    if (pctx->haveScale) {
      unsigned int sasn;
      sasn = pctx->initParm.samplesAlongScaleNum;
      totalNumPoints = AIR_CAST(unsigned int, voxNum * factor * sasn);
    } else {
      totalNumPoints = AIR_CAST(unsigned int, voxNum * factor);
    }
    break;
  case pullInitMethodRandom:
  case pullInitMethodHalton:
    npos = NULL;
    posData = NULL;
    totalNumPoints = pctx->initParm.numInitial;
    break;
  default:
    biffAddf(PULL, "%s: pullInitMethod %d not handled!", me,
             pctx->initParm.method);
    airMopError(mop); return 1;
    break;
  }
  if (pctx->verbose) {
    printf("%s: initializing/seeding . . .       ", me);
    fflush(stdout);
  }

  /* find first scale volume, if there is one; this is used by some
     seeders to determine placement along the scale axis */
  scaleVol = NULL;
  for (ii=0; ii<pctx->volNum; ii++) {
    if (pctx->vol[ii]->ninScale) {
      scaleVol = pctx->vol[ii];
      break;
    }
  }

  /* Start adding points */
  tick = totalNumPoints/1000;
  point = NULL;
  for (pointIdx = 0; pointIdx < totalNumPoints; pointIdx++) {
    int E;
    if (pctx->verbose) {
      if (tick < 100 || 0 == pointIdx % tick) {
        printf("%s", airDoneStr(0, pointIdx, totalNumPoints, doneStr));
        fflush(stdout);
      }
    }
    if (pctx->verbose > 5) {
      printf("\n%s: setting up point = %u/%u\n", me,
             pointIdx, totalNumPoints);
    }
    /* Create point */
    if (!point) {
      point = pullPointNew(pctx);
    }
    /* Filling array according to initialization method */
    E = 0;
    switch(pctx->initParm.method) {
    case pullInitMethodRandom:
    case pullInitMethodHalton:
      E = pullPointInitializeRandomOrHalton(pctx, pointIdx, point, scaleVol);
      createFail = AIR_FALSE;
      break;
    case pullInitMethodPointPerVoxel:
      E = pullPointInitializePerVoxel(pctx, pointIdx, point, scaleVol,
                                      &createFail);
      break;
    case pullInitMethodGivenPos:
      E = pullPointInitializeGivenPos(pctx, posData, pointIdx, point,
                                      &createFail);
      break;
    }
    if (E) {
      biffAddf(PULL, "%s: trouble trying point %u (id %u)", me,
               pointIdx, point->idtag);
      airMopError(mop); return 1;
    }
    if (createFail) {
      /* We were not successful in creating a point; not an error */
      continue;
    }

    /* else, the point is ready for binning */
    if (pctx->constraint) {
      if (pullBinsPointMaybeAdd(pctx, point, NULL, &added)) {
        biffAddf(PULL, "%s: trouble binning point %u", me, point->idtag);
        airMopError(mop); return 1;
      }
      /*
      if (4523 == point->idtag) {
        fprintf(stderr, "!%s(%u): ----- added=%d at %g %g %g %g\n",
                me, point->idtag, added,
                point->pos[0], point->pos[1], point->pos[2], point->pos[3]);
      }
      */
      if (added) {
        point = NULL;
      }
    } else {
      if (pullBinsPointAdd(pctx, point, NULL)) {
          biffAddf(PULL, "%s: trouble binning point %u", me, point->idtag);
          airMopError(mop); return 1;
      }
      point = NULL;
    }
  } /* Done looping through total number of points */
  if (pctx->verbose) {
    printf("%s\n", airDoneStr(0, pointIdx, totalNumPoints,
                              doneStr));
  }
  if (point) {
    /* we created a new test point, but it was never placed in the volume */
    /* so, HACK: undo pullPointNew . . . */
    point = pullPointNix(point);
    pctx->idtagNext -= 1;
  }

  /* Final check: do we have any points? */
  pn = pullPointNumber(pctx);
  if (!pn) {
    if (pctx->ispec[pullInfoSeedThresh]) {
      biffAddf(PULL, "%s: zero points: seeding failed (bad seedthresh? %g)",
               me, pctx->ispec[pullInfoSeedThresh]->zero);
    } else {
      biffAddf(PULL, "%s: zero points: seeding failed", me);
    }
    airMopError(mop); return 1;
  }
  if (pctx->verbose) {
    fprintf(stderr, "%s: initialized to %u points (idtagNext = %u)\n",
            me, pn, pctx->idtagNext);
  }
  /*
  if (1) {
    Nrrd *ntmp;
    ntmp = nrrdNew();
    pullOutputGet(ntmp, NULL, NULL, NULL, 0.0, pctx);
    nrrdSave("pos-in.nrrd", ntmp, NULL);
    nrrdNuke(ntmp);
  }
  */
  pctx->tmpPointPtr = AIR_CAST(pullPoint **,
                               calloc(pn, sizeof(pullPoint*)));
  pctx->tmpPointPerm = AIR_CAST(unsigned int *,
                                calloc(pn, sizeof(unsigned int)));
  if (!( pctx->tmpPointPtr && pctx->tmpPointPerm )) {
    biffAddf(PULL, "%s: couldn't allocate tmp buffers %p %p", me,
             AIR_VOIDP(pctx->tmpPointPtr), AIR_VOIDP(pctx->tmpPointPerm));
    airMopError(mop); return 1;
  }
  pctx->tmpPointNum = pn;

  /* now that all points have been added, set their energy to help
     inspect initial state */
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      point->energy = _pullPointEnergyTotal(pctx->task[0], bin, point,
                                            /* ignoreImage */
                                            !pctx->haveScale,
                                            point->force);
    }
  }

  if (pctx->verbose) {
    printf("%s: all done. ", me);
    fflush(stdout);
  }
  airMopOkay(mop);
  return 0;
}

void
_pullPointFinish(pullContext *pctx) {

  airFree(pctx->tmpPointPtr);
  airFree(pctx->tmpPointPerm);
  return ;
}
