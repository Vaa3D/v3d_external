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
** issues:
** does everything work on the first iteration
** how to handle the needed extra probe for d strength / d scale
** how are force/energy along scale handled differently than in space?
*/

static double
_pointDistSqrd(pullContext *pctx, pullPoint *AA, pullPoint *BB) {
  double diff[4];
  ELL_4V_SUB(diff, AA->pos, BB->pos);
  ELL_3V_SCALE(diff, 1/pctx->sysParm.radiusSpace, diff);
  diff[3] /= pctx->sysParm.radiusScale;
  return ELL_4V_DOT(diff, diff);
}

/*
** this sets, in task->neighPoint (*NOT* point->neighPoint), all the
** points in neighboring bins with which we might possibly interact,
** and returns the number of such points.
*/
static unsigned int
_neighBinPoints(pullTask *task, pullBin *bin, pullPoint *point,
                double distTest) {
  static const char me[]="_neighBinPoints";
  unsigned int nn, herPointIdx, herBinIdx;
  pullBin *herBin;
  pullPoint *herPoint;

  nn = 0;
  herBinIdx = 0;
  while ((herBin = bin->neighBin[herBinIdx])) {
    for (herPointIdx=0; herPointIdx<herBin->pointNum; herPointIdx++) {
      herPoint = herBin->point[herPointIdx];
      /*
      printf("!%s(%u): neighbin %u has point %u\n", me,
             point->idtag, herBinIdx, herPoint->idtag);
      */
      /* can't interact with myself, or anything nixed */
      if (point != herPoint
          && !(herPoint->status & PULL_STATUS_NIXME_BIT)) {
        if (distTest
            && _pointDistSqrd(task->pctx, point, herPoint) > distTest) {
          continue;
        }
        if (nn+1 < _PULL_NEIGH_MAXNUM) {
          task->neighPoint[nn++] = herPoint;
          /*
          printf("%s(%u): neighPoint[%u] = %u\n",
                 me, point->idtag, nn-1, herPoint->idtag);
          */
        } else {
          fprintf(stderr, "%s: hit max# (%u) poss. neighbors (from bins)\n",
                  me, _PULL_NEIGH_MAXNUM);
        }
      }
    }
    herBinIdx++;
  }
  /* also have to consider things in the add queue */
  for (herPointIdx=0; herPointIdx<task->addPointNum; herPointIdx++) {
    herPoint = task->addPoint[herPointIdx];
    if (point != herPoint) {
      if (distTest
          && _pointDistSqrd(task->pctx, point, herPoint) > distTest) {
        continue;
      }
      if (nn+1 < _PULL_NEIGH_MAXNUM) {
        task->neighPoint[nn++] = herPoint;
      } else {
        fprintf(stderr, "%s: hit max# (%u) poss neighs (add queue len %u)\n",
                me, _PULL_NEIGH_MAXNUM, task->addPointNum);
      }
    }
  }
  return nn;
}

/*
** compute the energy at "me" due to "she", and
** the gradient vector of her energy (probably pointing towards her)
**
** we're passed spaceDist to save us work of recomputing sqrt()
**
** egrad will be NULL if this is being called only to assess
** the energy at this point, rather than for learning how to move it
*/
double
_pullEnergyInterParticle(pullContext *pctx, pullPoint *me,
                         const pullPoint *she,
                         double spaceDist, double scaleDist,
                         /* output */
                         double egrad[4]) {
  char meme[]="pullEnergyInterParticle";
  double diff[4], spaceRad, scaleRad, rr, ss, uu, beta,
    en, den, enR, denR, enS, denS, enWR, enWS, denWR, denWS,
    *parmR, *parmS, *parmW,
    (*evalR)(double *, double, const double parm[PULL_ENERGY_PARM_NUM]),
    (*evalS)(double *, double, const double parm[PULL_ENERGY_PARM_NUM]),
    (*evalW)(double *, double, const double parm[PULL_ENERGY_PARM_NUM]);
  int scaleSgn;

  /* the vector "diff" goes from her, to me, in both space and scale */
  ELL_4V_SUB(diff, me->pos, she->pos);
  /* computed by caller: spaceDist = ELL_3V_LEN(diff); */
  /* computed by caller: scaleDist = AIR_ABS(diff[3]); */
  spaceRad = pctx->sysParm.radiusSpace;
  scaleRad = pctx->sysParm.radiusScale;
  rr = spaceDist/spaceRad;
  if (pctx->haveScale) {
    ss = scaleDist/scaleRad;
    scaleSgn = airSgn(diff[3]);
  } else {
    ss = 0;
    scaleSgn = 1;
  }
  if (rr > 1 || ss > 1) {
    if (egrad) {
      ELL_4V_SET(egrad, 0, 0, 0, 0);
    }
    return 0;
  }
  if (rr == 0 && ss == 0) {
    fprintf(stderr, "%s: pos(%u) == pos(%u) !! (%g,%g,%g,%g)\n",
            meme, me->idtag, she->idtag,
            me->pos[0], me->pos[1], me->pos[2], me->pos[3]);
    if (egrad) {
      ELL_4V_SET(egrad, 0, 0, 0, 0);
    }
    return 0;
  }
#if PULL_HINTER
  if (pullProcessModeDescent == pctx->task[0]->processMode
      && pctx->nhinter && pctx->nhinter->data) {
    unsigned int ri, si, sz;
    float *hint;
    hint = AIR_CAST(float *, pctx->nhinter->data);
    sz = pctx->nhinter->axis[0].size;
    ri = airIndex(-1.0, rr, 1.0, sz);
    si = airIndex(-1.0, ss*scaleSgn, 1.0, sz);
    hint[ri + sz*si] += 1;
  }
#endif

  parmR = pctx->energySpecR->parm;
  evalR = pctx->energySpecR->energy->eval;
  parmS = pctx->energySpecS->parm;
  evalS = pctx->energySpecS->energy->eval;
  switch (pctx->interType) {
  case pullInterTypeJustR:
    /* _pullVolumeSetup makes sure that
       !pctx->haveScale iff pullInterTypeJustR == pctx->interType */
    en = evalR(&denR, rr, parmR);
    if (egrad) {
      denR *= 1.0/(spaceRad*spaceDist);
      ELL_3V_SCALE(egrad, denR, diff);
      egrad[3] = 0;
    }
    break;
  case pullInterTypeUnivariate:
    uu = sqrt(rr*rr + ss*ss);
    en = evalR(&den, uu, parmR);
    if (egrad) {
      ELL_3V_SCALE(egrad, den/(uu*spaceRad*spaceRad), diff);
      egrad[3] = den*diff[3]/(uu*scaleRad*scaleRad);
    }
    break;
  case pullInterTypeSeparable:
    enR = evalR(&denR, rr, parmR);
    enS = evalS(&denS, ss, parmS);
    en = enR*enS;
    if (egrad) {
      ELL_3V_SCALE(egrad, denR*enS/(spaceRad*spaceDist), diff);
      egrad[3] = enR*airSgn(diff[3])*denS/scaleRad;
    }
    break;
  case pullInterTypeAdditive:
    parmW = pctx->energySpecWin->parm;
    evalW = pctx->energySpecWin->energy->eval;
    enR = evalR(&denR, rr, parmR);
    enS = evalS(&denS, ss, parmS);
    enWR = evalW(&denWR, rr, parmW);
    enWS = evalW(&denWS, ss, parmW);
    beta = pctx->sysParm.beta;
    en = AIR_LERP(beta, enR*enWS, enS*enWR);
    if (egrad) {
      double egradR[4], egradS[4];
      ELL_3V_SCALE(egradR, denR*enWS/(spaceRad*spaceDist), diff);
      ELL_3V_SCALE(egradS, denWR*enS/(spaceRad*spaceDist), diff);
      egradR[3] = enR*scaleSgn*denWS/scaleRad;
      egradS[3] = enWR*scaleSgn*denS/scaleRad;
      ELL_4V_LERP(egrad, beta, egradR, egradS);
    }
    break;
  default:
    fprintf(stderr, "!%s: sorry, intertype %d unimplemented", meme,
            pctx->interType);
    en = AIR_NAN;
    if (egrad) {
      ELL_4V_SET(egrad, AIR_NAN, AIR_NAN, AIR_NAN, AIR_NAN);
    }
    break;
  }
  /*
  printf("%s: %u <-- %u = %g,%g,%g -> egrad = %g,%g,%g, enr = %g\n",
         meme, me->idtag, she->idtag,
         diff[0], diff[1], diff[2],
         egrad[0], egrad[1], egrad[2], enr);
  */
  return en;
}

int
pullEnergyPlot(pullContext *pctx, Nrrd *nplot,
               double xx, double yy, double zz,
               unsigned int res) {
  static const char meme[]="pullEnergyPlot";
  pullPoint *me, *she;
  airArray *mop;
  double dir[3], len, *plot, _rr, _ss, rr, ss, enr, egrad[4];
  size_t size[3];
  unsigned int ri, si;

  if (!( pctx && nplot )) {
    biffAddf(PULL, "%s: got NULL pointer", meme);
    return 1;
  }
  ELL_3V_SET(dir, xx, yy, zz);
  if (!ELL_3V_LEN(dir)) {
    biffAddf(PULL, "%s: need non-zero length dir", meme);
    return 1;
  }
  ELL_3V_NORM(dir, dir, len);
  ELL_3V_SET(size, 3, res, res);
  if (nrrdMaybeAlloc_nva(nplot, nrrdTypeDouble, 3, size)) {
    biffMovef(PULL, NRRD, "%s: trouble allocating output", meme);
    return 1;
  }

  mop = airMopNew();
  me = pullPointNew(pctx);
  she = pullPointNew(pctx);
  airMopAdd(mop, me, (airMopper)pullPointNix, airMopAlways);
  airMopAdd(mop, she, (airMopper)pullPointNix, airMopAlways);
  ELL_4V_SET(me->pos, 0, 0, 0, 0);
  plot = AIR_CAST(double *, nplot->data);
  for (si=0; si<res; si++) {
    _ss = AIR_AFFINE(0, si, res-1, -1.0, 1.0);
    ss = _ss*pctx->sysParm.radiusScale;
    for (ri=0; ri<res; ri++) {
      _rr = AIR_AFFINE(0, ri, res-1, -1.0, 1.0);
      rr = _rr*pctx->sysParm.radiusSpace;
      ELL_3V_SCALE(she->pos, rr, dir);
      she->pos[3] = ss;
      enr = _pullEnergyInterParticle(pctx, me, she,
                                     AIR_ABS(rr), AIR_ABS(ss), egrad);
      plot[0] = enr;
      plot[1] = ELL_3V_DOT(egrad, dir);
      plot[2] = egrad[3];
      plot += 3;
    }
  }

  airMopOkay(mop);
  return 0;
}

/*
** computes energy from neighboring points. The non-NULLity of
** "egradSum" determines the energy *gradient* is computed (with
** possible constraint modifications) and stored there
**
** always computed:
** point->neighInterNum
** point->neighDistMean
**
** if pullProcessModeNeighLearn == task->processMode:
**   point->neighCovar
**   point->neighTanCovar
**   point->neighNum
**
**  0=0  1=1   2=2   3=3
**  (4)  4=5   5=6   6=7
**  (8)   (9)  7=10  8=11
** (12)  (13)  (14)  9=15
*/
double
_pullEnergyFromPoints(pullTask *task, pullBin *bin, pullPoint *point,
                      /* output */
                      double egradSum[4]) {
  static const char me[]="_pullEnergyFromPoints";
  double energySum, spaDistSqMax;  /* modeWghtSum */
  int nlist,    /* we enable the re-use of neighbor lists between inters, or,
                   at system start, creation of neighbor lists */
    ntrue;      /* we search all possible neighbors availble in the bins
                   (either because !nlist, or, this iter we learn true
                   subset of interacting neighbors).  This could also
                   be called "dontreuse" or something like that */
  unsigned int nidx,
    nnum;       /* how much of task->neigh[] we use */

  /* set nlist and ntrue */
  if (pullProcessModeNeighLearn == task->processMode) {
    /* we're here to both learn and store the true interacting neighbors */
    nlist = AIR_TRUE;
    ntrue = AIR_TRUE;
  } else if (pullProcessModeAdding == task->processMode
             || pullProcessModeNixing == task->processMode) {
    /* we assume that the work of learning neighbors has already been
       done, so we can reuse them now */
    nlist = AIR_TRUE;
    ntrue = AIR_FALSE;
  } else if (pullProcessModeDescent == task->processMode) {
    if (task->pctx->sysParm.neighborTrueProb < 1) {
      nlist = AIR_TRUE;
      if (egradSum) {
        /* We allow the neighbor list optimization only when we're also
           asked to compute the energy gradient, since that's the first
           part of moving the particle. */
        ntrue = (0 == task->pctx->iter
                 || (airDrandMT_r(task->rng)
                     < task->pctx->sysParm.neighborTrueProb));
      } else {
        /* When we're not getting the energy gradient, we're being
           called to test the waters at possible new locations, in which
           case we can't be changing the effective neighborhood */
        ntrue = AIR_TRUE;
      }
    } else {
      /* never trying neighborhood caching */
      nlist = AIR_FALSE;
      ntrue = AIR_TRUE;
    }
  } else {
    fprintf(stderr, "%s: process mode %d unrecognized!\n", me,
            task->processMode);
    return AIR_NAN;
  }

  /* NOTE: can't have both nlist and ntrue false. possibilities are:
  **
  **                    nlist:
  **                true     false
  **         true    X         X
  ** ntrue:
  **        false    X
  */
  /*
  printf("!%s(%u), nlist = %d, ntrue = %d\n", me, point->idtag,
         nlist, ntrue);
  */
  /* set nnum and task->neigh[] */
  if (ntrue) {
    /* this finds the over-inclusive set of all possible interacting
       points, based on bin membership as well the task's add queue */
    nnum = _neighBinPoints(task, bin, point, 1.0);
    if (nlist) {
      airArrayLenSet(point->neighPointArr, 0);
    }
  } else {
    /* (nlist true) this iter we re-use this point's existing neighbor
       list, copying it into the the task's neighbor list to simulate
       the action of _neighBinPoints() */
    nnum = point->neighPointNum;
    for (nidx=0; nidx<nnum; nidx++) {
      task->neighPoint[nidx] = point->neighPoint[nidx];
    }
  }

  /* loop through neighbor points */
  spaDistSqMax = (task->pctx->sysParm.radiusSpace
                  * task->pctx->sysParm.radiusSpace);
  /*
  printf("%s: radiusSpace = %g -> spaDistSqMax = %g\n", me,
         task->pctx->sysParm.radiusSpace, spaDistSqMax);
  */
  /* modeWghtSum = 0; */
  energySum = 0;
  point->neighInterNum = 0;
  point->neighDistMean = 0.0;
  if (pullProcessModeNeighLearn == task->processMode) {
    ELL_10V_ZERO_SET(point->neighCovar);
    point->stability = 0.0;
#if PULL_TANCOVAR
    if (task->pctx->ispec[pullInfoTangent1]) {
      double *tng;
      float outer[9];
      tng = point->info + task->pctx->infoIdx[pullInfoTangent1];
      ELL_3MV_OUTER_TT(outer, float, tng, tng);
      point->neighTanCovar[0] = outer[0];
      point->neighTanCovar[1] = outer[1];
      point->neighTanCovar[2] = outer[2];
      point->neighTanCovar[3] = outer[4];
      point->neighTanCovar[4] = outer[5];
      point->neighTanCovar[5] = outer[8];
    }
#endif
  }
  if (egradSum) {
    ELL_4V_SET(egradSum, 0, 0, 0, 0);
  }
  for (nidx=0; nidx<nnum; nidx++) {
    double diff[4], spaDistSq, spaDist, sclDist, enr, egrad[4];
    pullPoint *herPoint;

    herPoint = task->neighPoint[nidx];
    if (herPoint->status & PULL_STATUS_NIXME_BIT) {
      /* this point is not long for this world, pass over it */
      continue;
    }
    ELL_4V_SUB(diff, point->pos, herPoint->pos); /* me - her */
    spaDistSq = ELL_3V_DOT(diff, diff);
    /*
    printf("!%s: %u:%g,%g,%g <-- %u:%g,%g,%g = sqd %g %s %g\n", me,
           point->idtag, point->pos[0], point->pos[1], point->pos[2],
           herPoint->idtag,
           herPoint->pos[0], herPoint->pos[1], herPoint->pos[2],
           spaDistSq, spaDistSq > spaDistSqMax ? ">" : "<=", spaDistSqMax);
    */
    if (spaDistSq > spaDistSqMax) {
      continue;
    }
    sclDist = AIR_ABS(diff[3]);
    if (sclDist > task->pctx->sysParm.radiusScale) {
      continue;
    }
    spaDist = sqrt(spaDistSq);
    /* we pass spaDist to avoid recomputing sqrt(), and sclDist for
       stupid consistency  */
    enr = _pullEnergyInterParticle(task->pctx, point, herPoint,
                                   spaDist, sclDist,
                                   egradSum ? egrad : NULL);
#if 0
    /* sanity checking on energy derivatives */
    if (enr && egradSum) {
      double _pos[4], tdf[4], ee[2], eps=0.000001, apegrad[4], quot[4];
      unsigned int cord, pan;
      ELL_4V_COPY(_pos, point->pos);

      for (cord=0; cord<=3; cord++) {
        for (pan=0; pan<=1; pan++) {
          point->pos[cord] = _pos[cord] + (!pan ? -1 : +1)*eps;
          ELL_4V_SUB(tdf, point->pos, herPoint->pos);
          ee[pan] = _pullEnergyInterParticle(task->pctx, point, herPoint,
                                             ELL_3V_LEN(tdf), AIR_ABS(tdf[3]), NULL);
        }
        point->pos[cord] = _pos[cord];
        apegrad[cord] = (ee[1] - ee[0])/(2*eps);
        quot[cord] = apegrad[cord]/egrad[cord];
      }
      if ( AIR_ABS(1.0 - quot[0]) > 0.01 ||
           AIR_ABS(1.0 - quot[1]) > 0.01 ||
           AIR_ABS(1.0 - quot[2]) > 0.01 ||
           (task->pctx->haveScale && AIR_ABS(1.0 - quot[3]) > 0.01) ) {
        printf("!%s(%u<-%u): ---------- claim egrad (%g,%g,%g,%g)\n", me,
               point->idtag, herPoint->idtag, egrad[0], egrad[1], egrad[2], egrad[3]);
        printf("!%s(%u<-%u):            measr egrad (%g,%g,%g,%g)\n", me,
               point->idtag, herPoint->idtag, apegrad[0], apegrad[1], apegrad[2], apegrad[3]);
        printf("!%s(%u<-%u):            quot (%g,%g,%g,%g)\n", me,
               point->idtag, herPoint->idtag, quot[0], quot[1], quot[2], quot[3]);
      }

      ELL_4V_COPY(point->pos, _pos);
    }
#endif

    if (enr) {
      /* there is some non-zero energy due to her; and we assume that
         its not just a fluke zero-crossing of the potential profile */
      double ndist;

      point->neighInterNum++;
      if (nlist && ntrue) {
        unsigned int ii;
        /* we have to record that we had an interaction with this point */
        ii = airArrayLenIncr(point->neighPointArr, 1);
        point->neighPoint[ii] = herPoint;
      }
      energySum += enr;
      ELL_3V_SCALE(diff, 1.0/task->pctx->sysParm.radiusSpace, diff);
      if (task->pctx->haveScale) {
        diff[3] /= task->pctx->sysParm.radiusScale;
      }
      ndist = ELL_4V_LEN(diff);
      point->neighDistMean += ndist;
      if (pullProcessModeNeighLearn == task->processMode) {
        float outer[16];
        ELL_4MV_OUTER_TT(outer, float, diff, diff);
        point->neighCovar[0] += outer[0];
        point->neighCovar[1] += outer[1];
        point->neighCovar[2] += outer[2];
        point->neighCovar[3] += outer[3];
        point->neighCovar[4] += outer[5];
        point->neighCovar[5] += outer[6];
        point->neighCovar[6] += outer[7];
        point->neighCovar[7] += outer[10];
        point->neighCovar[8] += outer[11];
        point->neighCovar[9] += outer[15];
#if PULL_TANCOVAR
        if (task->pctx->ispec[pullInfoTangent1]) {
          double *tng;
          tng = herPoint->info + task->pctx->infoIdx[pullInfoTangent1];
          ELL_3MV_OUTER_TT(outer, float, tng, tng);
          point->neighTanCovar[0] += outer[0];
          point->neighTanCovar[1] += outer[1];
          point->neighTanCovar[2] += outer[2];
          point->neighTanCovar[3] += outer[4];
          point->neighTanCovar[4] += outer[5];
          point->neighTanCovar[5] += outer[8];
        }
#endif
      }
      if (egradSum) {
        ELL_4V_INCR(egradSum, egrad);
      }
    }
  }

#define CNORM(M)                                               \
  sqrt(M[0]*M[0] + 2*M[1]*M[1] + 2*M[2]*M[2] + 2*M[3]*M[3]     \
       + M[4]*M[4] + 2*M[5]*M[5] + 2*M[6]*M[6]                 \
       + M[7]*M[7] + 2*M[8]*M[8]                               \
       + M[9]*M[9])
#define CTRACE(M) (M[0] + M[4] + M[7] + M[9])

  /* finish computing things averaged over neighbors */
  if (point->neighInterNum) {
    point->neighDistMean /= point->neighInterNum;
    if (pullProcessModeNeighLearn == task->processMode) {
      double Css, trc;
      ELL_10V_SCALE(point->neighCovar, 1.0f/point->neighInterNum,
                    point->neighCovar);
      Css = point->neighCovar[9];
      trc = CTRACE(point->neighCovar);
      point->stability = AIR_CAST(float, (trc
                                          ? (task->pctx->targetDim * Css)/trc
                                          : 0.0));
#if PULL_TANCOVAR
      /* using 1 + # neigh because this includes tan1 of point itself */
      ELL_6V_SCALE(point->neighTanCovar, 1.0f/(1 + point->neighInterNum),
                   point->neighTanCovar);
#endif
    }
  } else {
    /* we had no neighbors at all */
    point->neighDistMean = 0.0; /* shouldn't happen in any normal case */
    /* point->neighCovar,neighTanCovar stay as initialized above */
  }
  return energySum;
}

static double
_energyFromImage(pullTask *task, pullPoint *point,
                 /* output */
                 double egradSum[4]) {
  static const char me[]="_energyFromImage";
  double energy, grad3[3], gamma;
  int probed;

  /* not sure I have the logic for this right
  int ptrue;
  if (task->pctx->probeProb < 1 && allowProbeProb) {
    if (egrad) {
      ptrue = (0 == task->pctx->iter
               || airDrandMT_r(task->rng) < task->pctx->probeProb);
    } else {
      ptrue = AIR_FALSE;
    }
  } else {
    ptrue = AIR_TRUE;
  }
  */
  probed = AIR_FALSE;

#define MAYBEPROBE \
  if (!probed) { \
    if (pullProbe(task, point)) { \
      fprintf(stderr, "%s: problem probing!!!\n%s\n", me, biffGetDone(PULL)); \
    } \
    probed = AIR_TRUE; \
  }

  gamma = task->pctx->sysParm.gamma;
  energy = 0;
  if (egradSum) {
    ELL_4V_SET(egradSum, 0, 0, 0, 0);
  }
  if (task->pctx->flag.energyFromStrength
      && task->pctx->ispec[pullInfoStrength]) {
    double deltaScale, str0, str1, scl0, scl1, enr;
    int sign;
    if (!egradSum) {
      /* just need the strength */
      MAYBEPROBE;
      enr = pullPointScalar(task->pctx, point, pullInfoStrength,
                            NULL, NULL);
      energy += -gamma*enr;
    } else {
      /* need strength and its gradient */
      /* randomize choice between forward and backward difference */
      /* NOTE: since you only need one bit of random, you could re-used
         a random int and look through its bits to determine forw vs
         back differences, but this is probably not the bottleneck */
      sign = 2*AIR_CAST(int, airRandInt_r(task->rng, 2)) - 1;
      deltaScale = task->pctx->bboxMax[3] - task->pctx->bboxMin[3];
      deltaScale *= sign*_PULL_STRENGTH_ENERGY_DELTA_SCALE;
      scl1 = (point->pos[3] += deltaScale);
      pullProbe(task, point);
      str1 = pullPointScalar(task->pctx, point, pullInfoStrength,
                             NULL, NULL);
      scl0 = (point->pos[3] -= deltaScale);
      MAYBEPROBE;
      str0 = pullPointScalar(task->pctx, point, pullInfoStrength,
                             NULL, NULL);
      energy += -gamma*str0;
      egradSum[3] += -gamma*(str1 - str0)/(scl1 - scl0);
      /*
      if (1560 < task->pctx->iter && 2350 == point->idtag) {
        printf("%s(%u): egrad[3] = %g*((%g-%g)/(%g-%g) = %g/%g = %g)"
               " = %g -> %g\n",
               me, point->idtag, task->pctx->sysParm.gamma,
               str1, str0, scl1, scl0,
               str1 - str0, scl1 - scl0,
               (str1 - str0)/(scl1 - scl0),
               task->pctx->sysParm.gamma*(str1 - str0)/(scl1 - scl0),
               egradSum[3]);
      }
      */
    }
  }
  /* Note that height doesn't contribute to the energy if there is
     a constraint associated with it */
  if (task->pctx->ispec[pullInfoHeight]
      && !task->pctx->ispec[pullInfoHeight]->constraint
      && !(task->pctx->ispec[pullInfoHeightLaplacian]
           && task->pctx->ispec[pullInfoHeightLaplacian]->constraint)) {
    MAYBEPROBE;
    energy += pullPointScalar(task->pctx, point, pullInfoHeight,
                              grad3, NULL);
    if (egradSum) {
      ELL_3V_INCR(egradSum, grad3);
    }
  }
  if (task->pctx->ispec[pullInfoIsovalue]
      && !task->pctx->ispec[pullInfoIsovalue]->constraint) {
    /* we're only going towards an isosurface, not constrained to it
       ==> set up a parabolic potential well around the isosurface */
    double val;
    MAYBEPROBE;
    val = pullPointScalar(task->pctx, point, pullInfoIsovalue,
                          grad3, NULL);
    energy += val*val;
    if (egradSum) {
      ELL_3V_SCALE_INCR(egradSum, 2*val, grad3);
    }
  }
  return energy;
}
#undef MAYBEPROBE

/*
** NOTE that the "egrad" being non-NULL has consequences for what gets
** computed in _energyFromImage and _pullEnergyFromPoints:
**
** NULL "egrad": we're simply learning the energy (and want to know it
** as truthfully as possible) for the sake of inspecting system state
**
** non-NULL "egrad": we're learning the current energy, but the real point
** is to determine how to move the point to lower energy
**
** the ignoreImage flag is a hack, to allow _pullPointProcessAdding to
** do descent on a new point according to other points, but not the
** image.
*/
double
_pullPointEnergyTotal(pullTask *task, pullBin *bin, pullPoint *point,
                      int ignoreImage,
                      /* output */
                      double egrad[4]) {
  static const char me[]="_pullPointEnergyTotal";
  double enrIm, enrPt, egradIm[4], egradPt[4], energy;

  ELL_4V_SET(egradIm, 0, 0, 0, 0);
  ELL_4V_SET(egradPt, 0, 0, 0, 0);
  if (!( ignoreImage || 1.0 == task->pctx->sysParm.alpha )) {
    enrIm = _energyFromImage(task, point, egrad ? egradIm : NULL);
    task->pctx->count[pullCountEnergyFromImage] += 1;
    if (egrad) {
      task->pctx->count[pullCountForceFromImage] += 1;
    }
  } else {
    enrIm = 0;
  }
  if (task->pctx->sysParm.alpha > 0.0) {
    enrPt = _pullEnergyFromPoints(task, bin, point, egrad ? egradPt : NULL);
    task->pctx->count[pullCountEnergyFromPoints] += 1;
    if (egrad) {
      task->pctx->count[pullCountForceFromPoints] += 1;
    }
  } else {
    enrPt = 0;
  }
  energy = AIR_LERP(task->pctx->sysParm.alpha, enrIm, enrPt);
  /*
  printf("!%s(%u): energy = lerp(%g, im %g, pt %g) = %g\n", me,
         point->idtag, task->pctx->sysParm.alpha, enrIm, enrPt, energy);
  */
  if (egrad) {
    ELL_4V_LERP(egrad, task->pctx->sysParm.alpha, egradIm, egradPt);
    /*
    printf("!%s(%u): egradIm = %g %g %g %g\n", me, point->idtag,
           egradIm[0], egradIm[1], egradIm[2], egradIm[3]);
    printf("!%s(%u): egradPt = %g %g %g %g\n", me, point->idtag,
           egradPt[0], egradPt[1], egradPt[2], egradPt[3]);
    printf("!%s(%u): ---> force = %g %g %g %g\n", me,
           point->idtag, force[0], force[1], force[2], force[3]);
    */
  }
  if (task->pctx->sysParm.wall) {
    unsigned int axi;
    double dwe; /* derivative of wall energy */
    for (axi=0; axi<4; axi++) {
      dwe = point->pos[axi] - task->pctx->bboxMin[axi];
      if (dwe > 0) {
        /* pos not below min */
        dwe = point->pos[axi] - task->pctx->bboxMax[axi];
        if (dwe < 0) {
          /* pos not above max */
          dwe = 0;
        }
      }
      energy += task->pctx->sysParm.wall*dwe*dwe/2;
      if (egrad) {
        egrad[axi] += task->pctx->sysParm.wall*dwe;
      }
    }
  }
  if (!AIR_EXISTS(energy)) {
    fprintf(stderr, "!%s(%u): oops- non-exist energy %g\n", me, point->idtag,
            energy);
  }
  return energy;
}

/*
** distance limit on a particles motion in both r and s,
** in rs-normalized space (sqrt((r/radiusSpace)^2 + (s/radiusScale)^2))
**
** This means that if particles are jammed together in space,
** they aren't allowed to move very far in scale, either, which
** is a little weird, but probably okay.
*/
double
_pullDistLimit(pullTask *task, pullPoint *point) {
  double ret;

  if (point->neighDistMean == 0 /* no known neighbors from last iter */
      || pullEnergyZero == task->pctx->energySpecR->energy) {
    ret = 1;
  } else {
    ret = _PULL_DIST_CAP_RSNORM*point->neighDistMean;
  }
  /* HEY: maybe task->pctx->voxelSizeSpace or voxelSizeScale should
     be considered here? */
  return ret;
}

/*
** here is where the energy gradient is converted into force
*/
int
_pullPointProcessDescent(pullTask *task, pullBin *bin, pullPoint *point,
                         int ignoreImage) {
  static const char me[]="_pullPointProcessDescent";
  double energyOld, energyNew, egrad[4], force[4], posOld[4];
  int stepBad, giveUp, hailMary;

  task->pctx->count[pullCountDescent] += 1;
  if (!point->stepEnergy) {
    fprintf(stderr, "\n\n\n%s: whoa, point %u step is zero!!\n\n\n\n",
            me, point->idtag);
    /* HEY: need to track down how this can originate! */
    /*
    biffAddf(PULL, "%s: point %u step is zero!", me, point->idtag);
    return 1;
    */
    point->stepEnergy = task->pctx->sysParm.stepInitial/100;
  }

  /* learn the energy at old location, and the energy gradient */
  energyOld = _pullPointEnergyTotal(task, bin, point, ignoreImage, egrad);
  ELL_4V_SCALE(force, -1, egrad);
  if (!( AIR_EXISTS(energyOld) && ELL_4V_EXISTS(force) )) {
    biffAddf(PULL, "%s: point %u non-exist energy or force", me, point->idtag);
    return 1;
  }
  /*
  if (1560 < task->pctx->iter && 2350 == point->idtag) {
    printf("!%s(%u): old pos = %g %g %g %g\n", me, point->idtag,
           point->pos[0], point->pos[1],
           point->pos[2], point->pos[3]);
    printf("!%s(%u): energyOld = %g; force = %g %g %g %g\n", me,
           point->idtag, energyOld, force[0], force[1], force[2], force[3]);
  }
  */
  if (!ELL_4V_DOT(force, force)) {
    /* this particle has no reason to go anywhere; BUT we still have to
       enforce constraint if we have one */
    int constrFail;
    if (task->pctx->constraint) {
      if (_pullConstraintSatisfy(task, point,
                                 100.0*_PULL_CONSTRAINT_TRAVEL_MAX,
                                 &constrFail)) {
        biffAddf(PULL, "%s: trouble", me);
        return 1;
      }
    }
    if (constrFail) {
      /*
      biffAddf(PULL, "%s: couldn't satisfy constraint on unforced %u: %s",
               me, point->idtag, airEnumStr(pullConstraintFail, constrFail));
      return 1;
      */
      fprintf(stderr, "%s: *** no constr sat on unfrced %u: %s (si# %u;%u)\n",
              me, point->idtag, airEnumStr(pullConstraintFail, constrFail),
              point->stuckIterNum, task->pctx->iterParm.stuckMax);
      point->status |= PULL_STATUS_STUCK_BIT;
      point->stuckIterNum += 1;
      if (task->pctx->iterParm.stuckMax
          && point->stuckIterNum > task->pctx->iterParm.stuckMax) {
        point->status |= PULL_STATUS_NIXME_BIT;
      }
    }
    point->energy = energyOld;
    return 0;
  }

  if (task->pctx->constraint
      && (task->pctx->ispec[pullInfoTangent1]
          || task->pctx->ispec[pullInfoNegativeTangent1])) {
    /* we have a constraint, so do something to get the force more
       tangential to the constraint manifold (only in the spatial axes) */
    double proj[9], pfrc[3];
    _pullConstraintTangent(task, point, proj);
    ELL_3MV_MUL(pfrc, proj, force);
    ELL_3V_COPY(force, pfrc);
    /* force[3] untouched */
  }
  /*
  if (1560 < task->pctx->iter && 2350 == point->idtag) {
    printf("!%s(%u): post-constraint tan: force = %g %g %g %g\n", me,
           point->idtag, force[0], force[1], force[2], force[3]);
    printf("   precap stepEnergy = %g\n", point->stepEnergy);
  }
  */
  /* Cap particle motion. The point is only allowed to move at most unit
     distance in rs-normalized space, which may mean that motion in r
     or s is effectively cramped by crowding in the other axis, oh well.
     Also, particle motion is limited in terms of spatial voxel size,
     and (if haveScale) the average distance between scale samples */
  if (1) {
    double capvec[4], spcLen, sclLen, max, distLimit;

    /* limits based on distLimit in rs-normalized space */
    distLimit = _pullDistLimit(task, point);
    ELL_4V_SCALE(capvec, point->stepEnergy, force);
    spcLen = ELL_3V_LEN(capvec)/task->pctx->sysParm.radiusSpace;
    sclLen = AIR_ABS(capvec[3])/task->pctx->sysParm.radiusScale;
    max = AIR_MAX(spcLen, sclLen);
    if (max > distLimit) {
      point->stepEnergy *= distLimit/max;
    }
    /* limits based on voxelSizeSpace and voxelSizeScale */
    ELL_4V_SCALE(capvec, point->stepEnergy, force);
    spcLen = ELL_3V_LEN(capvec)/task->pctx->voxelSizeSpace;
    if (task->pctx->haveScale) {
      sclLen = AIR_ABS(capvec[3])/task->pctx->voxelSizeScale;
      max = AIR_MAX(spcLen, sclLen);
    } else {
      max = spcLen;
    }
    if (max > _PULL_DIST_CAP_VOXEL) {
      point->stepEnergy *= _PULL_DIST_CAP_VOXEL/max;
    }
  }
  /*
  if (1560 < task->pctx->iter && 2350 == point->idtag) {
    printf("  postcap stepEnergy = %g\n", point->stepEnergy);
  }
  */
  /* turn off stuck bit, will turn it on again if needed */
  point->status &= ~PULL_STATUS_STUCK_BIT;
  ELL_4V_COPY(posOld, point->pos);
  _pullPointHistInit(point);
  _pullPointHistAdd(point, pullCondOld);
  /* try steps along force until we succcessfully lower energy */
  hailMary = AIR_FALSE;
  do {
    int constrFail, energyIncr;
    giveUp = AIR_FALSE;
    ELL_4V_SCALE_ADD2(point->pos, 1.0, posOld,
                      point->stepEnergy, force);
    /*
    if (1560 < task->pctx->iter && 2350 == point->idtag) {
      printf("!%s(%u): (iter %u) try pos = %g %g %g %g%s\n",
             me, point->idtag, task->pctx->iter,
             point->pos[0], point->pos[1],
             point->pos[2], point->pos[3],
             hailMary ? " (Hail Mary)" : "");
    }
    */
    if (task->pctx->haveScale) {
      point->pos[3] = AIR_CLAMP(task->pctx->bboxMin[3],
                                point->pos[3],
                                task->pctx->bboxMax[3]);
    }
    task->pctx->count[pullCountTestStep] += 1;
    _pullPointHistAdd(point, pullCondEnergyTry);
    if (task->pctx->constraint) {
      if (_pullConstraintSatisfy(task, point,
                                 _PULL_CONSTRAINT_TRAVEL_MAX,
                                 &constrFail)) {
        biffAddf(PULL, "%s: trouble", me);
        return 1;
      }
    } else {
      constrFail = AIR_FALSE;
    }
    /*
    if (1560 < task->pctx->iter && 2350 == point->idtag) {
      printf("!%s(%u): post constr = %g %g %g %g (%d)\n", me,
             point->idtag,
             point->pos[0], point->pos[1],
             point->pos[2], point->pos[3], constrFail);
    }
    */
    if (constrFail) {
      energyNew = AIR_NAN;
    } else {
      energyNew = _pullPointEnergyTotal(task, bin, point, ignoreImage, NULL);
    }
    energyIncr = energyNew > (energyOld
                              + task->pctx->sysParm.energyIncreasePermit);
    /*
    if (1560 < task->pctx->iter && 2350 == point->idtag) {
      printf("!%s(%u): constrFail %d; enr Old New = %g %g -> enrIncr %d\n",
             me, point->idtag, constrFail, energyOld, energyNew, energyIncr);
    }
    */
    stepBad = (constrFail || energyIncr);
    if (stepBad) {
      point->stepEnergy *= task->pctx->sysParm.backStepScale;
      if (constrFail) {
        _pullPointHistAdd(point, pullCondConstraintFail);
      } else {
        _pullPointHistAdd(point, pullCondEnergyBad);
      }
      /* you have a problem if you had a non-trivial force, but you can't
         ever seem to take a small enough step to reduce energy */
      if (point->stepEnergy < 0.00000001) {
        /* this can happen if the force is due to a derivative of
           feature strength with respect to scale, which is measured
           WITHOUT enforcing the constraint, while particle updates
           are done WITH the constraint, in which case the computed
           force can be completely misleading. Thus, as a last-ditch
           effort, we try moving in the opposite direction (against
           the force) to see if that helps */
        if (task->pctx->verbose > 1) {
          printf("%s: %u %s (%u); (%g,%g,%g,%g) stepEnr %g\n", me,
                 point->idtag, hailMary ? "STUCK!" : "stuck?",
                 point->stuckIterNum,
                 point->pos[0], point->pos[1], point->pos[2], point->pos[3],
                 point->stepEnergy);
        }
        if (!hailMary) {
          ELL_4V_SCALE(force, -1, force);
          /*
          if (4819 == point->idtag || 4828 == point->idtag) {
            printf("!%s(%u): force now %g %g %g %g\n", me, point->idtag,
                   force[0], force[1], force[2], force[3]);
          }
          */
          hailMary = AIR_TRUE;
        } else {
          /* The hail Mary pass missed too; something is really odd.
             This can happen when the previous iteration did a sloppy job
             enforcing the constraint, so before we move on, we enforce
             it, twice for good measure, so that things may be better next
             time around */
          if (task->pctx->constraint) {
            if (_pullConstraintSatisfy(task, point,
                                       _PULL_CONSTRAINT_TRAVEL_MAX,
                                       &constrFail)
                || _pullConstraintSatisfy(task, point,
                                          _PULL_CONSTRAINT_TRAVEL_MAX,
                                          &constrFail)) {
              biffAddf(PULL, "%s: trouble", me);
              return 1;
            }
          }
          energyNew = _pullPointEnergyTotal(task, bin, point,
                                            ignoreImage, NULL);
          point->stepEnergy = task->pctx->sysParm.stepInitial;
          point->status |= PULL_STATUS_STUCK_BIT;
          point->stuckIterNum += 1;
          giveUp = AIR_TRUE;
        }
      }
    }
  } while (stepBad && !giveUp);
  /* Hail Mary worked if (hailMary && !stepBad). It does sometimes work. */

  /* now: unless we gave up, energy decreased, and,
     if we have one, constraint has been met */
  /*
  if (1560 < task->pctx->iter && 2350 == point->idtag) {
    printf("!%s(%u):iter %u changed (%g,%g,%g,%g)->(%g,%g,%g,%g)\n",
           me, point->idtag, task->pctx->iter,
           posOld[0], posOld[1], posOld[2], posOld[3],
           point->pos[0], point->pos[1], point->pos[2], point->pos[3]);
  }
  */
  _pullPointHistAdd(point, pullCondNew);
  ELL_4V_COPY(point->force, force);

  /* not recorded for the sake of this function, but for system accounting */
  point->energy = energyNew;
  if (!AIR_EXISTS(energyNew)) {
    biffAddf(PULL, "%s: point %u has non-exist final energy %g\n",
             me, point->idtag, energyNew);
    return 1;
  }

  /* if its not stuck, reset stuckIterNum */
  if (!(point->status & PULL_STATUS_STUCK_BIT)) {
    point->stuckIterNum = 0;
  } else if (task->pctx->iterParm.stuckMax
             && point->stuckIterNum > task->pctx->iterParm.stuckMax) {
    /* else if it is stuck then its up to us to set NIXME
       based on point->stuckIterNum */
    point->status |= PULL_STATUS_NIXME_BIT;
  }

  return 0;
}

int
_pullPointProcess(pullTask *task, pullBin *bin, pullPoint *point) {
  static const char me[]="_pullPointProcess";
  int E;

  E = 0;
  switch (task->processMode) {
  case pullProcessModeDescent:
    E = _pullPointProcessDescent(task, bin, point,
                                 !task->pctx->haveScale /* ignoreImage */);
    break;
  case pullProcessModeNeighLearn:
    E = _pullPointProcessNeighLearn(task, bin, point);
    break;
  case pullProcessModeAdding:
    if (!task->pctx->flag.noAdd) {
      E = _pullPointProcessAdding(task, bin, point);
    }
    break;
  case pullProcessModeNixing:
    E = _pullPointProcessNixing(task, bin, point);
    break;
  default:
    biffAddf(PULL, "%s: process mode %d unrecognized", me, task->processMode);
    return 1;
    break;
  }
  if (E) {
    biffAddf(PULL, "%s: trouble", me);
    return 1;
  }
  return 0;
}

int
pullBinProcess(pullTask *task, unsigned int myBinIdx) {
  static const char me[]="pullBinProcess";
  pullBin *myBin;
  unsigned int myPointIdx;

  if (task->pctx->verbose > 2) {
    printf("%s(%s): doing bin %u\n", me,
           airEnumStr(pullProcessMode, task->processMode), myBinIdx);
  }
  myBin = task->pctx->bin + myBinIdx;
  for (myPointIdx=0; myPointIdx<myBin->pointNum; myPointIdx++) {
    pullPoint *point;
    if (task->pctx->pointNum > _PULL_PROGRESS_POINT_NUM_MIN
        && !task->pctx->flag.binSingle
        && task->pctx->progressBinMod
        && 0 == myBinIdx % task->pctx->progressBinMod) {
      printf("."); fflush(stdout);
    }
    point = myBin->point[myPointIdx];
    if (task->pctx->verbose > 2) {
      printf("%s(%s) processing (bin %u)->point[%u] %u\n", me,
             airEnumStr(pullProcessMode, task->processMode),
             myBinIdx,  myPointIdx, point->idtag);
    }
    if (_pullPointProcess(task, myBin, point)) {
      biffAddf(PULL, "%s: on point %u of bin %u\n", me,
               myPointIdx, myBinIdx);
      return 1;
    }
    task->stuckNum += (point->status & PULL_STATUS_STUCK_BIT);
  } /* for myPointIdx */

  return 0;
}

int
pullGammaLearn(pullContext *pctx) {
  static const char me[]="pullGammaLearn";
  unsigned int binIdx, pointIdx, pointNum;
  pullBin *bin;
  pullPoint *point;
  pullTask *task;
  double deltaScale, scl, beta, wellX=0, wellY=0,
    *strdd, *gmag, meanGmag, meanStrdd, wght, wghtSum;
  airArray *mop;

  if (!pctx) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (!pctx->haveScale) {
    biffAddf(PULL, "%s: not using scale-space", me);
    return 1;
  }
  if (pullInterTypeAdditive == pctx->interType) {
    if (pullEnergyButterworthParabola != pctx->energySpecS->energy) {
      biffAddf(PULL, "%s: want %s energy along scale, not %s", me,
               pullEnergyButterworthParabola->name,
               pctx->energySpecS->energy->name);
      return 1;
    }
  } else if (pullInterTypeSeparable == pctx->interType) {
    wellY = pctx->energySpecR->energy->well(&wellX,
                                            pctx->energySpecR->parm);
    if (!( wellY < 0 )) {
      biffAddf(PULL, "%s: spatial energy %s didn't have well",
               me, pctx->energySpecR->energy->name);
      return 1;
    }
    if (pullEnergyBspln != pctx->energySpecS->energy) {
      biffAddf(PULL, "%s: want %s energy along scale, not %s", me,
               pullEnergyBspln->name,
               pctx->energySpecS->energy->name);
      return 1;
    }
  } else {
    biffAddf(PULL, "%s: need %s or %s inter type, not %s", me,
             airEnumStr(pullInterType, pullInterTypeAdditive),
             airEnumStr(pullInterType, pullInterTypeSeparable),
             airEnumStr(pullInterType, pctx->interType));
    return 1;
  }
  pointNum = pullPointNumber(pctx);
  if (!pointNum) {
    biffAddf(PULL, "%s: had no points!", me);
    return 1;
  }

  mop = airMopNew();
  strdd = AIR_CALLOC(pointNum, double);
  airMopAdd(mop, strdd, airFree, airMopAlways);
  gmag = AIR_CALLOC(pointNum, double);
  airMopAdd(mop, gmag, airFree, airMopAlways);
  if (!(strdd && gmag)) {
    biffAddf(PULL, "%s: couldn't alloc two buffers of %u doubles",
             me, pointNum);
    airMopError(mop);
    return 1;
  }

  task = pctx->task[0];
  pointIdx = 0;
  deltaScale = pctx->bboxMax[3] - pctx->bboxMin[3];
  deltaScale *= _PULL_STRENGTH_ENERGY_DELTA_SCALE;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    unsigned int pidx;
    bin = pctx->bin + binIdx;
    for (pidx=0; pidx<bin->pointNum; pidx++) {
      double str[3], _ss, _gr;
      point = bin->point[pidx];
      point->pos[3] += deltaScale;
      pullProbe(task, point);
      str[2] = pullPointScalar(pctx, point, pullInfoStrength,
                               NULL, NULL);
      point->pos[3] -= 2*deltaScale;
      pullProbe(task, point);
      str[0] = pullPointScalar(pctx, point, pullInfoStrength,
                               NULL, NULL);
      point->pos[3] += deltaScale;
      pullProbe(task, point);
      str[1] = pullPointScalar(pctx, point, pullInfoStrength,
                               NULL, NULL);
      _ss = (str[0] - 2*str[1] + str[2])/(deltaScale*deltaScale);
      if (_ss < 0.0) {
        _gr = (str[2] - str[0])/(2*deltaScale);
        _gr = AIR_ABS(_gr);
        strdd[pointIdx] = _ss;
        gmag[pointIdx] = _gr;
        pointIdx++;
      }
    }
  }
  if (!pointIdx) {
    biffAddf(PULL, "%s: no points w/ 2nd deriv of strn wrt scale < 0", me);
    airMopError(mop);
    return 1;
  }

  /* resetting pointNum to actual number of points used */
  pointNum = pointIdx;
  /* learn meanGmag, with sqrt() sneakiness to discount high gmags */
  meanGmag = 0.0;
  for (pointIdx=0; pointIdx<pointNum; pointIdx++) {
    meanGmag += sqrt(gmag[pointIdx]);
  }
  meanGmag /= pointNum;
  meanGmag *= meanGmag;
  /* learn meanStrdd with a Gaussian weight on gmag; we want
     to give more weight to the strdds that are near maximal strength
     (hence 1st derivative near zero) */
  meanStrdd = wghtSum = 0.0;
  for (pointIdx=0; pointIdx<pointNum; pointIdx++) {
    /* the "meanGmag/8" allowed the gamma learned from a
       cone dataset immediately post-initialization to
       nearly match the gamma learned post-phase-2 */
    wght = airGaussian(gmag[pointIdx], 0.0, meanGmag/8);
    wghtSum += wght;
    meanStrdd += wght*strdd[pointIdx];
  }
  meanStrdd /= wghtSum;

  scl = pctx->sysParm.radiusScale;
  if (pullInterTypeAdditive == pctx->interType) {
    /* want to satisfy str''(s) = enr''(s)
    **        ==> -gamma*strdd = 2*beta/(radiusScale)^2
    **               ==> gamma = -2*beta/(strdd*(radiusScale)^2)
    **    (beta = 1) ==> gamma = -2/(strdd*(radiusScale)^2)
    ** NOTE: The difference from what is in the paper is a factor of 2,
    ** and the ability to include the influence of beta
    */
    beta = (pctx->flag.useBetaForGammaLearn
            ? pctx->sysParm.beta
            : 1.0);
    pctx->sysParm.gamma = -2*beta/(meanStrdd*scl*scl);
  } else if (pullInterTypeSeparable == pctx->interType) {
    /* want to satisfy str''(s) = enr''(s); wellY < 0
    **          ==> gamma*strdd = wellY*8/(radiusScale)^2
    **                    gamma = wellY*8/(strdd*(radiusScale)^2)
    */
    pctx->sysParm.gamma = wellY*8/(meanStrdd*scl*scl);
    pctx->sysParm.gamma *= pctx->sysParm.separableGammaLearnRescale;
  } else {
    biffAddf(PULL, "%s: sorry %s inter type unimplemented", me,
             airEnumStr(pullInterType, pctx->interType));
    airMopError(mop);
    return 1;
  }
  if (pctx->verbose) {
    printf("%s: learned gamma %g\n", me, pctx->sysParm.gamma);
  }

  airMopOkay(mop);
  return 0;
}

