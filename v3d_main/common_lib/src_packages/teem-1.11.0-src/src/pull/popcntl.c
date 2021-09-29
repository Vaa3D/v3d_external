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

int
_pullPointProcessNeighLearn(pullTask *task, pullBin *bin, pullPoint *point) {

  /* sneaky: we just learn (and discard) the energy, and this function
     will do the work of learning the neighbors */
  _pullEnergyFromPoints(task, bin, point, NULL);
  return 0;
}

static double
_pointEnergyOfNeighbors(pullTask *task, pullBin *bin, pullPoint *point,
                        double *fracNixed) {
  double enr;
  unsigned int ii, xx;
  pullPoint *her;

  enr = 0;
  xx = 0;
  for (ii=0; ii<point->neighPointNum; ii++) {
    her = point->neighPoint[ii];
    if (her->status & PULL_STATUS_NIXME_BIT) {
      xx += 1;
    } else {
      enr += _pullEnergyFromPoints(task, bin, her, NULL);
    }
  }
  *fracNixed = (point->neighPointNum
                ? AIR_CAST(double, xx)/point->neighPointNum
                : 0);
  return enr;
}

int
_pullPointProcessAdding(pullTask *task, pullBin *bin, pullPoint *point) {
  static const char me[]="_pullPointProcessAdding";
  unsigned int npi, iter, api;
  double noffavg[4], npos[4], enrWith, enrWithout,
    fracNixed, newSpcDist, tmp;
  pullPoint *newpnt;
  int E;

  task->pctx->count[pullCountAdding] += 1;

  if (point->neighPointNum && task->pctx->targetDim
      && task->pctx->flag.popCntlEnoughTest) {
    unsigned int plenty;
    plenty = (1 == task->pctx->targetDim
              ? 3
              : (2 == task->pctx->targetDim
                 ? 7
                 : (3 == task->pctx->targetDim
                    ? 13 /* = 1 + 12
                            = 1 + coordination number of 3D sphere packing */
                    : 0 /* shouldn't get here */)));
    /*
    if (0 == (point->idtag % 100)) {
      printf("%s: #num %d >?= plenty %d\n", me, point->neighPointNum, plenty);
    }
    */
    if (point->neighPointNum >= plenty) {
      /* there's little chance that adding points will reduce energy */
      return 0;
    }
  }
  ELL_4V_SET(noffavg, 0, 0, 0, 0);
  for (npi=0; npi<point->neighPointNum; npi++) {
    double off[4];
    ELL_4V_SUB(off, point->pos, point->neighPoint[npi]->pos);
    /* normalize the offset */
    ELL_3V_SCALE(off, 1/task->pctx->sysParm.radiusSpace, off);
    if (task->pctx->haveScale) {
      off[3] /= task->pctx->sysParm.radiusScale;
    }
    ELL_4V_INCR(noffavg, off);
  }
  if (point->neighPointNum) {
    /*
    if (0 == (point->idtag % 100)) {
      printf("%s: len(offavg) %g >?= thresh %g\n", me,
             ELL_4V_LEN(noffavg)/point->neighPointNum,
             _PULL_NEIGH_OFFSET_SUM_THRESH);
    }
    */
    if (ELL_4V_LEN(noffavg)/point->neighPointNum
        < _PULL_NEIGH_OFFSET_SUM_THRESH) {
      /* we have neighbors, and they seem to be balanced well enough;
         don't try to add */
      return 0;
    }
  }
  if (task->pctx->energySpecR->energy->well(&newSpcDist,
                                            task->pctx->energySpecR->parm)) {
    /* HEY: if we don't actually have a well, what is the point of
       guessing a new distance? */
    newSpcDist = _PULL_NEWPNT_DIST;
  }
  /* compute offset (normalized) direction from current point location */
  if (!point->neighPointNum) {
    /* we had no neighbors, have to pretend like we did */
    airNormalRand_r(noffavg + 0, noffavg + 1, task->rng);
    airNormalRand_r(noffavg + 2, noffavg + 3, task->rng);
    if (!task->pctx->haveScale) {
      noffavg[3] = 0;
    }
  }
  if (task->pctx->constraint) {
    double proj[9], tmpvec[3];
    _pullConstraintTangent(task, point, proj);
    ELL_3MV_MUL(tmpvec, proj, noffavg);
    ELL_3V_COPY(noffavg, tmpvec);
  }
  ELL_4V_NORM(noffavg, noffavg, tmp);
  ELL_3V_SCALE(noffavg, task->pctx->sysParm.radiusSpace, noffavg);
  noffavg[3] *= task->pctx->sysParm.radiusScale;
  ELL_4V_SCALE(noffavg, newSpcDist, noffavg);
  /* set new point location */
  ELL_4V_ADD2(npos, noffavg, point->pos);
  if (!_pullInsideBBox(task->pctx, npos)) {
    if (task->pctx->verbose > 2) {
      printf("%s: new pnt would start (%g,%g,%g,%g) outside bbox, nope\n",
             me, npos[0], npos[1], npos[2], npos[3]);
    }
    return 0;
  }
  /* initial pos is good, now we start getting serious */
  newpnt = pullPointNew(task->pctx);
  if (!newpnt) {
    biffAddf(PULL, "%s: couldn't spawn new point from %u", me, point->idtag);
    return 1;
  }
  ELL_4V_COPY(newpnt->pos, npos);
  /* set status to indicate this is an unbinned point, with no
     knowledge of its neighbors */
  newpnt->status |= PULL_STATUS_NEWBIE_BIT;
  /* satisfy constraint if needed */
  if (task->pctx->constraint) {
    int constrFail;
    if (_pullConstraintSatisfy(task, newpnt,
                               _PULL_CONSTRAINT_TRAVEL_MAX,
                               &constrFail)) {
      biffAddf(PULL, "%s: on newbie point %u (spawned from %u)", me,
               newpnt->idtag, point->idtag);
      pullPointNix(newpnt); return 1;
    }
    if (constrFail) {
      /* constraint satisfaction failed, which isn't an error for us,
         we just don't try to add this point.  Can do immediate nix
         because no neighbors know about this point. */
      pullPointNix(newpnt);
      return 0;
    }
    if (!_pullInsideBBox(task->pctx, newpnt->pos)) {
      if (task->pctx->verbose > 2) {
        printf("%s: post constr newpnt %u (%g,%g,%g,%g) outside bbox; nope\n",
               me, newpnt->idtag, newpnt->pos[0], newpnt->pos[1],
               newpnt->pos[2], newpnt->pos[3]);
      }
      newpnt = pullPointNix(newpnt);
      return 0;
    }
  }
  /* do some descent, on this point only, which (HACK!) we do by
     changing the per-task process mode . . . */
  task->processMode = pullProcessModeDescent;
  E = 0;
  for (iter=0; iter<task->pctx->iterParm.addDescent; iter++) {
    double diff[4];
    if (!E) E |= _pullPointProcessDescent(task, bin, newpnt,
                                          /* ignoreImage; actually it is
                                           this use which motivated the
                                           creation of ignoreImage */
                                          AIR_TRUE);
    if (newpnt->status & PULL_STATUS_STUCK_BIT) {
      if (task->pctx->verbose > 2) {
        printf("%s: possible newpnt %u stuck @ iter %u; nope\n", me,
               newpnt->idtag, iter);
      }
      newpnt = pullPointNix(newpnt);
      /* if we don't change the mode back, then pullBinProcess() won't
         know to try adding for the rest of the bins it sees, bad HACK */
      task->processMode = pullProcessModeAdding;
      return 0;
    }
    if (!_pullInsideBBox(task->pctx, newpnt->pos)) {
      if (task->pctx->verbose > 2) {
        printf("%s: newpnt %u went (%g,%g,%g,%g) outside bbox; nope\n",
               me, newpnt->idtag, newpnt->pos[0], newpnt->pos[1],
               newpnt->pos[2], newpnt->pos[3]);
      }
      newpnt = pullPointNix(newpnt);
      task->processMode = pullProcessModeAdding;
      return 0;
    }
    ELL_4V_SUB(diff, newpnt->pos, point->pos);
    ELL_3V_SCALE(diff, 1/task->pctx->sysParm.radiusSpace, diff);
    diff[3] /= task->pctx->sysParm.radiusScale;
    if (ELL_4V_LEN(diff) > _PULL_NEWPNT_STRAY_DIST) {
      if (task->pctx->verbose > 2) {
        printf("%s: newpnt %u went too far %g from old point %u; nope\n",
               me, newpnt->idtag, ELL_4V_LEN(diff), point->idtag);
      }
      newpnt = pullPointNix(newpnt);
      task->processMode = pullProcessModeAdding;
      return 0;
    }
    /* still okay to continue descending */
    newpnt->stepEnergy *= task->pctx->sysParm.opporStepScale;
  }
  /* now that newbie point is final test location, see if it meets
     the live thresh, if there is one */
  if (task->pctx->ispec[pullInfoLiveThresh]
      && 0 > pullPointScalar(task->pctx, newpnt, pullInfoLiveThresh,
                             NULL, NULL)) {
    /* didn't meet threshold */
    newpnt = pullPointNix(newpnt);
    task->processMode = pullProcessModeAdding;
    return 0;
  }
  if (task->pctx->ispec[pullInfoLiveThresh2] /* HEY: copy & paste */
      && 0 > pullPointScalar(task->pctx, newpnt, pullInfoLiveThresh2,
                             NULL, NULL)) {
    /* didn't meet threshold */
    newpnt = pullPointNix(newpnt);
    task->processMode = pullProcessModeAdding;
    return 0;
  }
  if (task->pctx->ispec[pullInfoLiveThresh3] /* HEY: copy & paste */
      && 0 > pullPointScalar(task->pctx, newpnt, pullInfoLiveThresh3,
                             NULL, NULL)) {
    /* didn't meet threshold */
    newpnt = pullPointNix(newpnt);
    task->processMode = pullProcessModeAdding;
    return 0;
  }
  /* see if the new point should be nixed because its at a volume edge */
  if (task->pctx->flag.nixAtVolumeEdgeSpace
      && (newpnt->status & PULL_STATUS_EDGE_BIT)) {
    newpnt = pullPointNix(newpnt);
    task->processMode = pullProcessModeAdding;
    return 0;
  }
  /* no problem with live thresh, test energy by first learn neighbors */
  /* we have to add newpnt to task's add queue, just so that its neighbors
     can see it as a possible neighbor */
  api = airArrayLenIncr(task->addPointArr, 1);
  task->addPoint[api] = newpnt;
  task->processMode = pullProcessModeNeighLearn;
  _pullEnergyFromPoints(task, bin, newpnt, NULL);
  /* and teach the neighbors their neighbors (possibly including newpnt) */
  for (npi=0; npi<newpnt->neighPointNum; npi++) {
    _pullEnergyFromPoints(task, bin, newpnt->neighPoint[npi], NULL);
  }
  /* now back to the actual mode we came here with */
  task->processMode = pullProcessModeAdding;
  enrWith = (_pullEnergyFromPoints(task, bin, newpnt, NULL)
             + _pointEnergyOfNeighbors(task, bin, newpnt, &fracNixed));
  newpnt->status |= PULL_STATUS_NIXME_BIT;  /* turn nixme on */
  enrWithout = _pointEnergyOfNeighbors(task, bin, newpnt, &fracNixed);
  newpnt->status &= ~PULL_STATUS_NIXME_BIT; /* turn nixme off */
  if (enrWith < enrWithout) {
    /* energy is clearly lower *with* newpnt, so we want to add it, which
       means keeping it in the add queue where it already is */
    if (task->pctx->logAdd) {
      double posdiff[4];
      ELL_4V_SUB(posdiff, newpnt->pos, point->pos);
      fprintf(task->pctx->logAdd, "%u %g %g %g %g %g %g\n", newpnt->idtag,
              ELL_3V_LEN(posdiff)/task->pctx->sysParm.radiusSpace,
              AIR_ABS(posdiff[3])/task->pctx->sysParm.radiusScale,
              newpnt->pos[0], newpnt->pos[1], newpnt->pos[2], newpnt->pos[3]);
    }
  } else {
    /* adding point is not an improvement, remove it from the add queue */
    airArrayLenIncr(task->addPointArr, -1);
    /* ugh, have to signal to neighbors that its no longer their neighbor.
       HEY this is the part that is totally screwed with multiple threads */
    task->processMode = pullProcessModeNeighLearn;
    newpnt->status |= PULL_STATUS_NIXME_BIT;
    for (npi=0; npi<newpnt->neighPointNum; npi++) {
      _pullEnergyFromPoints(task, bin, newpnt->neighPoint[npi], NULL);
    }
    task->processMode = pullProcessModeAdding;
    newpnt = pullPointNix(newpnt);
  }
  return 0;
}

int
_pullPointProcessNixing(pullTask *task, pullBin *bin, pullPoint *point) {
  double enrWith, enrNeigh, enrWithout, fracNixed;

  task->pctx->count[pullCountNixing] += 1;

  /* if there's a live thresh, do we meet it? */
  if (task->pctx->ispec[pullInfoLiveThresh]
      && 0 > pullPointScalar(task->pctx, point, pullInfoLiveThresh,
                             NULL, NULL)) {
    point->status |= PULL_STATUS_NIXME_BIT;
    return 0;
  }
  /* HEY copy & paste */
  if (task->pctx->ispec[pullInfoLiveThresh2]
      && 0 > pullPointScalar(task->pctx, point, pullInfoLiveThresh2,
                             NULL, NULL)) {
    point->status |= PULL_STATUS_NIXME_BIT;
    return 0;
  }
  /* HEY copy & paste */
  if (task->pctx->ispec[pullInfoLiveThresh3]
      && 0 > pullPointScalar(task->pctx, point, pullInfoLiveThresh3,
                             NULL, NULL)) {
    point->status |= PULL_STATUS_NIXME_BIT;
    return 0;
  }

  /* if many neighbors have been nixed, then system is far from convergence,
     so energy is not a very meaningful guide to whether to nix this point
     NOTE that we use this function to *learn* fracNixed */
  enrNeigh = _pointEnergyOfNeighbors(task, bin, point, &fracNixed);
  if (fracNixed < task->pctx->sysParm.fracNeighNixedMax) {
    /* is energy lower without us around? */
    enrWith = enrNeigh + _pullEnergyFromPoints(task, bin, point, NULL);
    point->status |= PULL_STATUS_NIXME_BIT;    /* turn nixme on */
    enrWithout = _pointEnergyOfNeighbors(task, bin, point, &fracNixed);
    if (enrWith <= enrWithout) {
      /* Energy isn't distinctly lowered without the point, so keep it;
         turn off nixing.  If enrWith == enrWithout == 0, as happens to
         isolated points, then the difference between "<=" and "<"
         keeps the isolated points from getting nixed */
      point->status &= ~PULL_STATUS_NIXME_BIT; /* turn nixme off */
    }
    /* else energy is certainly higher with the point, do nix it */
  }

  return 0;
}

int
_pullIterFinishNeighLearn(pullContext *pctx) {
  static const char me[]="_pullIterFinishNeighLearn";

  /* a no-op for now */
  AIR_UNUSED(pctx);
  AIR_UNUSED(me);

  return 0;
}

int
_pullIterFinishAdding(pullContext *pctx) {
  static const char me[]="_pullIterFinishAdding";
  unsigned int taskIdx;

  pctx->addNum = 0;
  for (taskIdx=0; taskIdx<pctx->threadNum; taskIdx++) {
    pullTask *task;
    task = pctx->task[taskIdx];
    if (task->addPointNum) {
      unsigned int pointIdx;
      int added;
      for (pointIdx=0; pointIdx<task->addPointNum; pointIdx++) {
        pullPoint *point;
        pullBin *bin;
        point = task->addPoint[pointIdx];
        point->status &= ~PULL_STATUS_NEWBIE_BIT;
        if (pullBinsPointMaybeAdd(pctx, point, &bin, &added)) {
          biffAddf(PULL, "%s: trouble binning new point %u", me, point->idtag);
          return 1;
        }
        if (added) {
          pctx->addNum++;
        } else {
          unsigned int npi, xpi;
          if (pctx->verbose) {
            printf("%s: decided NOT to add new point %u\n", me, point->idtag);
          }
          /* HEY: copied from above */
          /* ugh, have to signal to neigs that its no longer their neighbor */
          task->processMode = pullProcessModeNeighLearn;
          point->status |= PULL_STATUS_NIXME_BIT;
          for (npi=0; npi<point->neighPointNum; npi++) {
            _pullEnergyFromPoints(task, bin, point->neighPoint[npi], NULL);
          }
          task->processMode = pullProcessModeAdding;
          /* can't do immediate nix for reasons GLK doesn't quite understand */
          xpi = airArrayLenIncr(task->nixPointArr, 1);
          task->nixPoint[xpi] = point;
        }
      }
      airArrayLenSet(task->addPointArr, 0);
    }
  }
  if (pctx->verbose && pctx->addNum) {
    printf("%s: ADDED %u\n", me, pctx->addNum);
  }
  return 0;
}

void
_pullNixTheNixed(pullContext *pctx) {
  unsigned int binIdx;

  pctx->nixNum = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    pullBin *bin;
    unsigned int pointIdx;
    bin = pctx->bin + binIdx;
    pointIdx = 0;
    while (pointIdx < bin->pointNum) {
      pullPoint *point;
      point = bin->point[pointIdx];
      if (pctx->flag.nixAtVolumeEdgeSpace
          && (point->status & PULL_STATUS_EDGE_BIT)) {
        point->status |= PULL_STATUS_NIXME_BIT;
      }
      if (point->status & PULL_STATUS_NIXME_BIT) {
        pullPointNix(point);
        /* copy last point pointer to this slot */
        bin->point[pointIdx] = bin->point[bin->pointNum-1];
        airArrayLenIncr(bin->pointArr, -1); /* will decrement bin->pointNum */
        pctx->nixNum++;
      } else {
        pointIdx++;
      }
    }
  }
  return;
}

int
_pullIterFinishNixing(pullContext *pctx) {
  static const char me[]="_pullIterFinishNixing";
  unsigned int taskIdx;

  _pullNixTheNixed(pctx);
  /* finish nixing the things that we decided not to add */
  for (taskIdx=0; taskIdx<pctx->threadNum; taskIdx++) {
    pullTask *task;
    task = pctx->task[taskIdx];
    if (task->nixPointNum) {
      unsigned int xpi;
      for (xpi=0; xpi<task->nixPointNum; xpi++) {
        pullPointNix(task->nixPoint[xpi]);
      }
      airArrayLenSet(task->nixPointArr, 0);
    }
  }
  if (pctx->verbose && pctx->nixNum) {
    printf("%s: NIXED %u\n", me, pctx->nixNum);
  }
  return 0;
}

