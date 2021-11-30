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

pullTask *
_pullTaskNew(pullContext *pctx, int threadIdx) {
  static const char me[]="_pullTaskNew";
  pullTask *task;
  unsigned int ii;
  pullPtrPtrUnion pppu;

  task = AIR_CALLOC(1, pullTask);
  if (!task) {
    biffAddf(PULL, "%s: couldn't allocate task", me);
    return NULL;
  }

  task->pctx = pctx;
  for (ii=0; ii<pctx->volNum; ii++) {
    if (!(task->vol[ii] = _pullVolumeCopy(pctx->vol[ii]))) {
      biffAddf(PULL, "%s: trouble copying vol %u/%u", me, ii, pctx->volNum);
      return NULL;
    }
  }
  if (0) {
    gagePerVolume *pvl;
    const double *ans;
    double pos[3];
    int gret;
    for (ii=0; ii<pctx->volNum; ii++) {
      pvl = task->vol[ii]->gctx->pvl[0];
      printf("!%s: vol[%u] query:\n", me, ii);
      gageQueryPrint(stdout, pvl->kind, pvl->query);
      ans = gageAnswerPointer(task->vol[ii]->gctx, pvl, gageSclValue);
      ELL_3V_SET(pos, 0.6, 0.6, 0.3);
      gret = gageProbeSpace(task->vol[ii]->gctx, pos[0], pos[1], pos[2],
                            AIR_FALSE, AIR_TRUE);
      printf("!%s: (%d) val(%g,%g,%g) = %g\n", me, gret,
             pos[0], pos[1], pos[2], *ans);
      ELL_3V_SET(pos, 0.5, 0.0, 0.0);
      gret = gageProbeSpace(task->vol[ii]->gctx, pos[0], pos[1], pos[2],
                            AIR_FALSE, AIR_TRUE);
      printf("!%s: (%d) val(%g,%g,%g) = %g\n", me, gret,
             pos[0], pos[1], pos[2], *ans);
    }
  }
  /* now set up all pointers for per-task pullInfos */
  for (ii=0; ii<=PULL_INFO_MAX; ii++) {
    const pullVolume *vol;
    if (pctx->ispec[ii]) {
      if (pullSourceGage == pctx->ispec[ii]->source) {
        vol = task->vol[pctx->ispec[ii]->volIdx];
        task->ans[ii] = gageAnswerPointer(vol->gctx, vol->gpvl,
                                          pctx->ispec[ii]->item);
        if (pctx->verbose) {
          printf("%s: task->ans[%u] = (%s) %p\n", me, ii,
                 vol->kind->name, AIR_CVOIDP(task->ans[ii]));
        }
      } else {
        task->ans[ii] = NULL;
      }
    } else {
      task->ans[ii] = NULL;
    }
  }
  /* HEY: any idea why there is so little error checking in the below? */
  /* initialize to descent because that's what's needed for the end of point
     initialization, when initial energy must be learned */
  task->processMode = pullProcessModeDescent;
  task->probeSeedPreThreshOnly = AIR_FALSE;
  if (pctx->threadNum > 1) {
    task->thread = airThreadNew();
  }
  task->threadIdx = threadIdx;
  task->rng = airRandMTStateNew(pctx->rngSeed + threadIdx);
  task->pointBuffer = pullPointNew(pctx);
  pctx->idtagNext = 0; /* because pullPointNew incremented it */
  task->neighPoint = AIR_CAST(pullPoint **, calloc(_PULL_NEIGH_MAXNUM,
                                                   sizeof(pullPoint*)));
  task->addPoint = NULL;
  task->addPointNum = 0;
  pppu.points = &(task->addPoint);
  task->addPointArr = airArrayNew(pppu.v, &(task->addPointNum),
                                  sizeof(pullPoint*),
                                  /* not exactly the right semantics . . . */
                                  PULL_POINT_NEIGH_INCR);
  task->nixPoint = NULL;
  task->nixPointNum = 0;
  pppu.points = &(task->nixPoint);
  task->nixPointArr = airArrayNew(pppu.v, &(task->nixPointNum),
                                  sizeof(pullPoint*),
                                  /* not exactly the right semantics . . . */
                                  PULL_POINT_NEIGH_INCR);
  task->returnPtr = NULL;
  task->stuckNum = 0;
  return task;
}

pullTask *
_pullTaskNix(pullTask *task) {
  unsigned int ii;

  if (task) {
    for (ii=0; ii<task->pctx->volNum; ii++) {
      task->vol[ii] = pullVolumeNix(task->vol[ii]);
    }
    if (task->pctx->threadNum > 1) {
      task->thread = airThreadNix(task->thread);
    }
    task->rng = airRandMTStateNix(task->rng);
    task->pointBuffer = pullPointNix(task->pointBuffer);
    airFree(task->neighPoint);
    task->addPointArr = airArrayNuke(task->addPointArr);
    task->nixPointArr = airArrayNuke(task->nixPointArr);
    airFree(task);
  }
  return NULL;
}

/*
** _pullTaskSetup sets:
**** pctx->task
**** pctx->task[]
*/
int
_pullTaskSetup(pullContext *pctx) {
  static const char me[]="_pullTaskSetup";
  unsigned int tidx;

  pctx->task = (pullTask **)calloc(pctx->threadNum, sizeof(pullTask *));
  if (!(pctx->task)) {
    biffAddf(PULL, "%s: couldn't allocate array of tasks", me);
    return 1;
  }
  for (tidx=0; tidx<pctx->threadNum; tidx++) {
    if (pctx->verbose) {
      printf("%s: creating task %u/%u\n", me, tidx, pctx->threadNum);
    }
    pctx->task[tidx] = _pullTaskNew(pctx, tidx);
    if (!(pctx->task[tidx])) {
      biffAddf(PULL, "%s: couldn't allocate task %d", me, tidx);
      return 1;
    }
  }
  return 0;
}

void
_pullTaskFinish(pullContext *pctx) {
  unsigned int tidx;

  for (tidx=0; tidx<pctx->threadNum; tidx++) {
    pctx->task[tidx] = _pullTaskNix(pctx->task[tidx]);
  }
  airFree(pctx->task);
  pctx->task = NULL;
  return;
}
