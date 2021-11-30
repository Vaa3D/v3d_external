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
** this is the core of the worker threads: as long as there are bins
** left to process, get the next one, and process it
*/
int
_pushProcess(pushTask *task) {
  static const char me[]="_pushProcess";
  unsigned int binIdx;

  while (task->pctx->binIdx < task->pctx->binNum) {
    /* get the index of the next bin to process */
    if (task->pctx->threadNum > 1) {
      airThreadMutexLock(task->pctx->binMutex);
    }
    do {
      binIdx = task->pctx->binIdx;
      if (task->pctx->binIdx < task->pctx->binNum) {
        task->pctx->binIdx++;
      }
    } while (binIdx < task->pctx->binNum
             && 0 == task->pctx->bin[binIdx].pointNum);
    if (task->pctx->threadNum > 1) {
      airThreadMutexUnlock(task->pctx->binMutex);
    }
    if (binIdx == task->pctx->binNum) {
      /* no more bins to process! */
      break;
    }

    if (pushBinProcess(task, binIdx)) {
      biffAddf(PUSH, "%s(%u): had trouble on bin %u", me,
               task->threadIdx, binIdx);
      return 1;
    }

  }
  return 0;
}

/* the main loop for each worker thread */
void *
_pushWorker(void *_task) {
  static const char me[]="_pushWorker";
  pushTask *task;

  task = (pushTask *)_task;

  while (1) {
    if (task->pctx->verbose > 1) {
      fprintf(stderr, "%s(%u): waiting on barrier A\n",
              me, task->threadIdx);
    }
    /* pushFinish sets finished prior to the barriers */
    airThreadBarrierWait(task->pctx->iterBarrierA);
    if (task->pctx->finished) {
      if (task->pctx->verbose > 1) {
        fprintf(stderr, "%s(%u): done!\n", me, task->threadIdx);
      }
      break;
    }
    /* else there's work to do ... */
    if (task->pctx->verbose > 1) {
      fprintf(stderr, "%s(%u): starting to process\n", me, task->threadIdx);
    }
    if (_pushProcess(task)) {
      /* HEY clearly not threadsafe ... */
      biffAddf(PUSH, "%s: thread %u trouble", me, task->threadIdx);
      task->pctx->finished = AIR_TRUE;
    }
    if (task->pctx->verbose > 1) {
      fprintf(stderr, "%s(%u): waiting on barrier B\n",
              me, task->threadIdx);
    }
    airThreadBarrierWait(task->pctx->iterBarrierB);
  }

  return _task;
}

int
_pushContextCheck(pushContext *pctx) {
  static const char me[]="_pushContextCheck";
  unsigned int numSingle;

  if (!pctx) {
    biffAddf(PUSH, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( pctx->pointNum >= 1 )) {
    biffAddf(PUSH, "%s: pctx->pointNum (%d) not >= 1\n", me, pctx->pointNum);
    return 1;
  }
  if (!( AIR_IN_CL(1, pctx->threadNum, PUSH_THREAD_MAXNUM) )) {
    biffAddf(PUSH, "%s: pctx->threadNum (%d) outside valid range [1,%d]", me,
             pctx->threadNum, PUSH_THREAD_MAXNUM);
    return 1;
  }

  if (nrrdCheck(pctx->nin)) {
    biffMovef(PUSH, NRRD, "%s: got a broken input nrrd", me);
    return 1;
  }
  if (!( (4 == pctx->nin->dim && 7 == pctx->nin->axis[0].size) )) {
    biffAddf(PUSH, "%s: input doesn't look like 3D masked tensor volume", me);
    return 1;
  }
  numSingle = 0;
  numSingle += (1 == pctx->nin->axis[1].size);
  numSingle += (1 == pctx->nin->axis[2].size);
  numSingle += (1 == pctx->nin->axis[3].size);
  if (numSingle > 1) {
    biffAddf(PUSH, "%s: can have a single sample along at most one axis", me);
    return 1;
  }

  if (pctx->npos) {
    if (nrrdCheck(pctx->npos)) {
      biffMovef(PUSH, NRRD, "%s: got a broken position nrrd", me);
      return 1;
    }
    if (!( 2 == pctx->npos->dim
           && 3 == pctx->npos->axis[0].size )) {
      biffAddf(PUSH, "%s: position nrrd not 2-D 3-by-N", me);
      return 1;
    }
  }
  if (tenGageUnknown != pctx->gravItem) {
    if (airEnumValCheck(tenGage, pctx->gravItem)) {
      biffAddf(PUSH, "%s: gravity item %u invalid", me, pctx->gravItem);
      return 1;
    }
    if (1 != tenGageKind->table[pctx->gravItem].answerLength) {
      biffAddf(PUSH, "%s: answer length of gravity item %s is %u, not 1", me,
               airEnumStr(tenGage, pctx->gravItem),
               tenGageKind->table[pctx->gravItem].answerLength);
      return 1;
    }
    if (airEnumValCheck(tenGage, pctx->gravGradItem)) {
      biffAddf(PUSH, "%s: gravity gradient item %u invalid",
               me, pctx->gravGradItem);
      return 1;
    }
    if (3 != tenGageKind->table[pctx->gravGradItem].answerLength) {
      biffAddf(PUSH, "%s: answer length of gravity grad item %s is %u, not 3",
               me, airEnumStr(tenGage, pctx->gravGradItem),
               tenGageKind->table[pctx->gravGradItem].answerLength);
      return 1;
    }
    if (!AIR_EXISTS(pctx->gravScl)) {
      biffAddf(PUSH, "%s: gravity scaling doesn't exist", me);
      return 1;
    }
    if (!AIR_EXISTS(pctx->gravZero)) {
      biffAddf(PUSH, "%s: gravity zero doesn't exist", me);
      return 1;
    }
  }
  return 0;
}

int
pushStart(pushContext *pctx) {
  static const char me[]="pushStart";
  unsigned int tidx;

  if (_pushContextCheck(pctx)) {
    biffAddf(PUSH, "%s: trouble", me);
    return 1;
  }

  airSrandMT(pctx->seedRNG);

  /* the ordering of steps below is important: gage context
     has to be set up before its copied by task setup */
  pctx->step = pctx->stepInitial;
  if (_pushTensorFieldSetup(pctx)
      || _pushGageSetup(pctx)
      || _pushTaskSetup(pctx)
      || _pushBinSetup(pctx)
      || _pushPointSetup(pctx)) {
    biffAddf(PUSH, "%s: trouble setting up context", me);
    return 1;
  }
  fprintf(stderr, "!%s: setup done-ish\n", me);

  if (pctx->threadNum > 1) {
    pctx->binMutex = airThreadMutexNew();
    pctx->iterBarrierA = airThreadBarrierNew(pctx->threadNum);
    pctx->iterBarrierB = airThreadBarrierNew(pctx->threadNum);
    /* start threads 1 and up running; they'll all hit iterBarrierA  */
    for (tidx=1; tidx<pctx->threadNum; tidx++) {
      if (pctx->verbose > 1) {
        fprintf(stderr, "%s: spawning thread %d\n", me, tidx);
      }
      airThreadStart(pctx->task[tidx]->thread, _pushWorker,
                     (void *)(pctx->task[tidx]));
    }
  } else {
    pctx->binMutex = NULL;
    pctx->iterBarrierA = NULL;
    pctx->iterBarrierB = NULL;
  }
  pctx->iter = 0;

  return 0;
}

/*
******** pushIterate
**
** (documentation)
**
** NB: this implements the body of thread 0, the master thread
*/
int
pushIterate(pushContext *pctx) {
  static const char me[]="pushIterate";
  unsigned int ti, pointNum;
  double time0, time1;
  int myError;

  if (!pctx) {
    biffAddf(PUSH, "%s: got NULL pointer", me);
    return 1;
  }

  if (pctx->verbose) {
    fprintf(stderr, "%s: starting iterations\n", me);
  }

  time0 = airTime();

  /* the _pushWorker checks finished after iterBarrierA */
  pctx->finished = AIR_FALSE;
  pctx->binIdx=0;
  for (ti=0; ti<pctx->threadNum; ti++) {
    pctx->task[ti]->pointNum = 0;
    pctx->task[ti]->energySum = 0;
    pctx->task[ti]->deltaFracSum = 0;
  }

  if (pctx->verbose) {
    fprintf(stderr, "%s: starting iter %d w/ %u threads\n",
            me, pctx->iter, pctx->threadNum);
  }
  if (pctx->threadNum > 1) {
    airThreadBarrierWait(pctx->iterBarrierA);
  }
  myError = AIR_FALSE;
  if (_pushProcess(pctx->task[0])) {
    biffAddf(PUSH, "%s: master thread trouble w/ iter %u", me, pctx->iter);
    pctx->finished = AIR_TRUE;
    myError = AIR_TRUE;
  }
  if (pctx->threadNum > 1) {
    airThreadBarrierWait(pctx->iterBarrierB);
  }
  if (pctx->finished) {
    if (!myError) {
      /* we didn't set finished- one of the workers must have */
      biffAddf(PUSH, "%s: worker error on iter %u", me, pctx->iter);
    }
    return 1;
  }

  pctx->energySum = 0;
  pctx->deltaFrac = 0;
  pointNum = 0;
  for (ti=0; ti<pctx->threadNum; ti++) {
    pctx->energySum += pctx->task[ti]->energySum;
    pctx->deltaFrac += pctx->task[ti]->deltaFracSum;
    pointNum += pctx->task[ti]->pointNum;
  }
  pctx->deltaFrac /= pointNum;
  if (pushRebin(pctx)) {
    biffAddf(PUSH, "%s: problem with new point locations", me);
    return 1;
  }

  time1 = airTime();
  pctx->timeIteration = time1 - time0;
  pctx->timeRun += time1 - time0;
  pctx->iter += 1;

  return 0;
}

int
pushRun(pushContext *pctx) {
  static const char me[]="pushRun";
  char poutS[AIR_STRLEN_MED], toutS[AIR_STRLEN_MED];
  Nrrd *npos, *nten;
  double time0, time1, enrLast,
    enrNew=AIR_NAN, enrImprov=AIR_NAN, enrImprovAvg=AIR_NAN;

  if (pushIterate(pctx)) {
    biffAddf(PUSH, "%s: trouble on starting iteration", me);
    return 1;
  }
  fprintf(stderr, "!%s: starting pctx->energySum = %g\n", me, pctx->energySum);

  time0 = airTime();
  pctx->iter = 0;
  do {
    enrLast = pctx->energySum;
    if (pushIterate(pctx)) {
      biffAddf(PUSH, "%s: trouble on iter %d", me, pctx->iter);
      return 1;
    }
    if (pctx->snap && !(pctx->iter % pctx->snap)) {
      nten = nrrdNew();
      npos = nrrdNew();
      sprintf(poutS, "snap.%06d.pos.nrrd", pctx->iter);
      sprintf(toutS, "snap.%06d.ten.nrrd", pctx->iter);
      if (pushOutputGet(npos, nten, NULL, pctx)) {
        biffAddf(PUSH, "%s: couldn't get snapshot for iter %d",
                 me, pctx->iter);
        return 1;
      }
      if (nrrdSave(poutS, npos, NULL)
          || nrrdSave(toutS, nten, NULL)) {
        biffMovef(PUSH, NRRD, "%s: couldn't save snapshot for iter %d",
                  me, pctx->iter);
        return 1;
      }
      nten = nrrdNuke(nten);
      npos = nrrdNuke(npos);
    }
    enrNew = pctx->energySum;
    enrImprov = 2*(enrLast - enrNew)/(enrLast + enrNew);
    fprintf(stderr, "!%s: %u, e=%g, de=%g,%g, df=%g\n",
            me, pctx->iter, enrNew, enrImprov, enrImprovAvg, pctx->deltaFrac);
    if (enrImprov < 0 || pctx->deltaFrac < pctx->deltaFracMin) {
      /* either energy went up instead of down,
         or particles were hitting their speed limit too much */
      double tmp;
      tmp = pctx->step;
      if (enrImprov < 0) {
        pctx->step *= pctx->energyStepFrac;
        fprintf(stderr, "%s: ***** iter %u e improv = %g; step = %g --> %g\n",
                me, pctx->iter, enrImprov, tmp, pctx->step);
      } else {
        pctx->step *= pctx->deltaFracStepFrac;
        fprintf(stderr, "%s: ##### iter %u deltaf = %g; step = %g --> %g\n",
                me, pctx->iter, pctx->deltaFrac, tmp, pctx->step);
      }
      /* this forces another iteration */
      enrImprovAvg = AIR_NAN;
    } else {
      /* there was some improvement; energy went down */
      if (!AIR_EXISTS(enrImprovAvg)) {
        /* either enrImprovAvg has initial NaN setting, or was set to NaN
           because we had to decrease step size; either way we now
           re-initialize it to a large-ish value, to delay convergence */
        enrImprovAvg = 3*enrImprov;
      } else {
        /* we had improvement this iteration and last, do weighted average
           of the two, so that we are measuring the trend, rather than being
           sensitive to two iterations that just happen to have the same
           energy.  Thus, when enrImprovAvg gets near user-defined threshold,
           we really must have converged */
        enrImprovAvg = (enrImprovAvg + enrImprov)/2;
      }
    }
  } while ( ((!AIR_EXISTS(enrImprovAvg)
              || enrImprovAvg > pctx->energyImprovMin)
             && (0 == pctx->maxIter
                 || pctx->iter < pctx->maxIter)) );
  fprintf(stderr, "%s: done after %u iters; enr = %g, enrImprov = %g,%g\n",
          me, pctx->iter, enrNew, enrImprov, enrImprovAvg);
  time1 = airTime();

  pctx->timeRun = time1 - time0;

  return 0;
}

/*
** this is called *after* pushOutputGet
**
** should nix everything created by the many _push*Setup() functions
*/
int
pushFinish(pushContext *pctx) {
  static const char me[]="pushFinish";
  unsigned int ii, tidx;

  if (!pctx) {
    biffAddf(PUSH, "%s: got NULL pointer", me);
    return 1;
  }

  pctx->finished = AIR_TRUE;
  if (pctx->threadNum > 1) {
    if (pctx->verbose > 1) {
      fprintf(stderr, "%s: finishing workers\n", me);
    }
    airThreadBarrierWait(pctx->iterBarrierA);
  }
  /* worker threads now pass barrierA and see that finished is AIR_TRUE,
     and then bail, so now we collect them */
  for (tidx=pctx->threadNum; tidx>0; tidx--) {
    if (tidx-1) {
      airThreadJoin(pctx->task[tidx-1]->thread,
                    &(pctx->task[tidx-1]->returnPtr));
    }
    pctx->task[tidx-1]->thread = airThreadNix(pctx->task[tidx-1]->thread);
    pctx->task[tidx-1] = _pushTaskNix(pctx->task[tidx-1]);
  }
  pctx->task = (pushTask **)airFree(pctx->task);

  pctx->nten = nrrdNuke(pctx->nten);
  pctx->ninv = nrrdNuke(pctx->ninv);
  pctx->nmask = nrrdNuke(pctx->nmask);
  pctx->gctx = gageContextNix(pctx->gctx);
  for (ii=0; ii<pctx->binNum; ii++) {
    pushBinDone(pctx->bin + ii);
  }
  pctx->bin = (pushBin *)airFree(pctx->bin);
  ELL_3V_SET(pctx->binsEdge, 0, 0, 0);
  pctx->binNum = 0;

  if (pctx->threadNum > 1) {
    pctx->binMutex = airThreadMutexNix(pctx->binMutex);
    pctx->iterBarrierA = airThreadBarrierNix(pctx->iterBarrierA);
    pctx->iterBarrierB = airThreadBarrierNix(pctx->iterBarrierB);
  }

  return 0;
}
