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

#include "ten.h"
#include "privateTen.h"

#define TEN_FIBER_INCR 512

/*
** _tenFiberProbe
**
** The job here is to probe at (world space) "wPos" and then set:
**   tfx->fiberTen
**   tfx->fiberEval (all 3 evals)
**   tfx->fiberEvec (all 3 eigenvectors)
**   if (tfx->stop & (1 << tenFiberStopAniso): tfx->fiberAnisoStop
**
** In the case of non-single-tensor tractography, we do so based on
** ten2Which (when at the seedpoint) or
**
** Note that for performance reasons, a non-zero return value
** (indicating error) and the associated use of biff, is only possible
** if seedProbe is non-zero, the reason being that problems can be
** detected at the seedpoint, and won't arise after the seedpoint.
**
** Errors from gage are indicated by *gageRet, which includes leaving
** the domain of the volume, which is used to terminate fibers.
**
** Our use of tfx->seedEvec (shared with _tenFiberAlign), as well as that
** of tfx->lastDir and tfx->lastDirSet, could stand to have further
** debugging and documentation ...
*/
int
_tenFiberProbe(tenFiberContext *tfx, int *gageRet,
               double wPos[3], int seedProbe) {
  static const char me[]="_tenFiberProbe";
  double iPos[3];
  int ret = 0;
  double tens2[2][7];

  gageShapeWtoI(tfx->gtx->shape, iPos, wPos);
  *gageRet = gageProbe(tfx->gtx, iPos[0], iPos[1], iPos[2]);

  if (tfx->verbose > 2) {
    fprintf(stderr, "%s(%g,%g,%g, %s): hi ----- %s\n", me,
            iPos[0], iPos[1], iPos[2], seedProbe ? "***TRUE***" : "false",
            tfx->useDwi ? "using DWIs" : "");
  }

  if (!tfx->useDwi) {
    /* normal single-tensor tracking */
    TEN_T_COPY(tfx->fiberTen, tfx->gageTen);
    ELL_3V_COPY(tfx->fiberEval, tfx->gageEval);
    ELL_3M_COPY(tfx->fiberEvec, tfx->gageEvec);
    if (tfx->stop & (1 << tenFiberStopAniso)) {
      tfx->fiberAnisoStop = tfx->gageAnisoStop[0];
    }
    if (seedProbe) {
      ELL_3V_COPY(tfx->seedEvec, tfx->fiberEvec);
    }
  } else { /* tracking in DWIs */
    if (tfx->verbose > 2 && seedProbe) {
      fprintf(stderr, "%s:   fiber type = %s\n", me,
              airEnumStr(tenDwiFiberType, tfx->fiberType));
    }
    switch (tfx->fiberType) {
      double evec[2][9], eval[2][3];
    case tenDwiFiberType1Evec0:
      if (tfx->mframeUse) {
        double matTmpA[9], matTmpB[9];
        TEN_T2M(matTmpA, tfx->gageTen);
        ELL_3M_MUL(matTmpB, tfx->mframe, matTmpA);
        ELL_3M_MUL(matTmpA, matTmpB, tfx->mframeT);
        TEN_M2T(tfx->fiberTen, matTmpA);
        tfx->fiberTen[0] = tfx->gageTen[0];
      } else {
        TEN_T_COPY(tfx->fiberTen, tfx->gageTen);
      }
      tenEigensolve_d(tfx->fiberEval, tfx->fiberEvec, tfx->fiberTen);
      if (tfx->stop & (1 << tenFiberStopAniso)) {
        double tmp;
        tmp = tenAnisoTen_d(tfx->fiberTen, tfx->anisoStopType);
        tfx->fiberAnisoStop = AIR_CLAMP(0, tmp, 1);
      }
      if (seedProbe) {
        ELL_3V_COPY(tfx->seedEvec, tfx->fiberEvec);
      }
      break;
    case tenDwiFiberType2Evec0:
      /* Estimate principal diffusion direction of each tensor */
      if (tfx->mframeUse) {
        /* Transform both the tensors */
        double matTmpA[9], matTmpB[9];

        TEN_T2M(matTmpA, tfx->gageTen2 + 0);
        ELL_3M_MUL(matTmpB, tfx->mframe, matTmpA);
        ELL_3M_MUL(matTmpA, matTmpB, tfx->mframeT);
        TEN_M2T(tens2[0], matTmpA);
        /* new eigen values and vectors */
        tenEigensolve_d(eval[0], evec[0], tens2[0]);

        TEN_T2M(matTmpA, tfx->gageTen2 + 7);
        ELL_3M_MUL(matTmpB, tfx->mframe, matTmpA);
        ELL_3M_MUL(matTmpA, matTmpB, tfx->mframeT);
        TEN_M2T(tens2[1], matTmpA);
        tenEigensolve_d(eval[1], evec[1], tens2[1]);
      } else {
        tenEigensolve_d(eval[0], evec[0], tfx->gageTen2 + 0);
        tenEigensolve_d(eval[1], evec[1], tfx->gageTen2 + 7);
      }

      /* set ten2Use */
      if (seedProbe) {  /* we're on the *very* 1st probe per tract,
                           at the seed pt */
        ELL_3V_COPY(tfx->seedEvec, evec[tfx->ten2Which]);
        tfx->ten2Use = tfx->ten2Which;
        if (tfx->verbose > 2) {
          fprintf(stderr, "%s: ** ten2Use == ten2Which == %d\n", me,
                  tfx->ten2Use);
        }
      } else {
        double *lastVec, dot[2];

        if (!tfx->lastDirSet) {
          /* we're on some probe of the first step */
          lastVec = tfx->seedEvec;
        } else {
          /* we're past the first step */
          /* Arish says: "Bug len has not been initialized and don't think
             its needed". The first part is not a problem; "len" is in the
             *output* argument of ELL_3V_NORM.  The second part seems to be
             true, even though Gordon can't currently see why! */
          /* ELL_3V_NORM(tfx->lastDir, tfx->lastDir, len); */
          lastVec = tfx->lastDir;
        }
        dot[0] = ELL_3V_DOT(lastVec, evec[0]);
        dot[1] = ELL_3V_DOT(lastVec, evec[1]);
        if (dot[0] < 0) {
          dot[0] *= -1;
          ELL_3M_SCALE(evec[0], -1, evec[0]);
        }
        if (dot[1] < 0) {
          dot[1] *= -1;
          ELL_3M_SCALE(evec[1], -1, evec[1]);
        }
        tfx->ten2Use = (dot[0] > dot[1]) ? 0 : 1;
        if (tfx->verbose > 2) {
          fprintf(stderr, "%s(%g,%g,%g): dot[0,1] = %f, %f -> use %u\n",
                  me, wPos[0], wPos[1], wPos[2], dot[0], dot[1],
                  tfx->ten2Use );
        }
      }

      /* based on ten2Use, set the rest of the needed fields */
      if (tfx->mframeUse) {
        TEN_T_COPY(tfx->fiberTen, tens2[tfx->ten2Use]);
      } else {
        TEN_T_COPY(tfx->fiberTen, tfx->gageTen2 + 7*(tfx->ten2Use));
      }
      tfx->fiberTen[0] = tfx->gageTen2[0];   /* copy confidence */
      ELL_3V_COPY(tfx->fiberEval, eval[tfx->ten2Use]);
      ELL_3M_COPY(tfx->fiberEvec, evec[tfx->ten2Use]);
      if (tfx->stop & (1 << tenFiberStopAniso)) {
        double tmp;
        tmp = tenAnisoEval_d(tfx->fiberEval, tfx->anisoStopType);
        tfx->fiberAnisoStop = AIR_CLAMP(0, tmp, 1);
        /* HEY: what about speed? */
      } else {
        tfx->fiberAnisoStop = AIR_NAN;
      }
      break;
    default:
      biffAddf(TEN, "%s: %s %s (%d) unimplemented!", me,
              tenDwiFiberType->name,
              airEnumStr(tenDwiFiberType, tfx->fiberType), tfx->fiberType);
      ret = 1;
    } /* switch (tfx->fiberType) */
  }
  if (tfx->verbose > 2) {
    fprintf(stderr, "%s: fiberEvec = %g %g %g\n", me,
            tfx->fiberEvec[0], tfx->fiberEvec[1], tfx->fiberEvec[2]);
  }

  return ret;
}

int
_tenFiberStopCheck(tenFiberContext *tfx) {
  static const char me[]="_tenFiberStopCheck";

  if (tfx->numSteps[tfx->halfIdx] >= TEN_FIBER_NUM_STEPS_MAX) {
    fprintf(stderr, "%s: numSteps[%d] exceeded sanity check value of %d!!\n",
            me, tfx->halfIdx, TEN_FIBER_NUM_STEPS_MAX);
    fprintf(stderr, "%s: Check fiber termination conditions, or recompile "
            "with a larger value for TEN_FIBER_NUM_STEPS_MAX\n", me);
    return tenFiberStopNumSteps;
  }
  if (tfx->stop & (1 << tenFiberStopConfidence)) {
    if (tfx->fiberTen[0] < tfx->confThresh) {
      return tenFiberStopConfidence;
    }
  }
  if (tfx->stop & (1 << tenFiberStopRadius)) {
    if (tfx->radius < tfx->minRadius) {
      return tenFiberStopRadius;
    }
  }
  if (tfx->stop & (1 << tenFiberStopAniso)) {
    if (tfx->fiberAnisoStop  < tfx->anisoThresh) {
      return tenFiberStopAniso;
    }
  }
  if (tfx->stop & (1 << tenFiberStopNumSteps)) {
    if (tfx->numSteps[tfx->halfIdx] > tfx->maxNumSteps) {
      return tenFiberStopNumSteps;
    }
  }
  if (tfx->stop & (1 << tenFiberStopLength)) {
    if (tfx->halfLen[tfx->halfIdx] >= tfx->maxHalfLen) {
      return tenFiberStopLength;
    }
  }
  if (tfx->useDwi
      && tfx->stop & (1 << tenFiberStopFraction)
      && tfx->gageTen2) { /* not all DWI fiber types use gageTen2 */
    double fracUse;
    fracUse = (0 == tfx->ten2Use
               ? tfx->gageTen2[7]
               : 1 - tfx->gageTen2[7]);
    if (fracUse < tfx->minFraction) {
      return tenFiberStopFraction;
    }
  }
  return 0;
}

void
_tenFiberAlign(tenFiberContext *tfx, double vec[3]) {
  static const char me[]="_tenFiberAlign";
  double scale, dot;

  if (tfx->verbose > 2) {
    fprintf(stderr, "%s: hi %s (lds %d):\t%g %g %g\n", me,
            !tfx->lastDirSet ? "**" : "  ",
            tfx->lastDirSet, vec[0], vec[1], vec[2]);
  }
  if (!(tfx->lastDirSet)) {
    dot = ELL_3V_DOT(tfx->seedEvec, vec);
    /* this is the first step (or one of the intermediate steps
       for RK) in this fiber half; 1st half follows the
       eigenvector determined at seed point, 2nd goes opposite */
    if (tfx->verbose > 2) {
      fprintf(stderr, "!%s: dir=%d, dot=%g\n", me, tfx->halfIdx, dot);
    }
    if (!tfx->halfIdx) {
      /* 1st half */
      scale = dot < 0 ? -1 : 1;
    } else {
      /* 2nd half */
      scale = dot > 0 ? -1 : 1;
    }
  } else {
    dot = ELL_3V_DOT(tfx->lastDir, vec);
    /* we have some history in this fiber half */
    scale = dot < 0 ? -1 : 1;
  }
  ELL_3V_SCALE(vec, scale, vec);
  if (tfx->verbose > 2) {
    fprintf(stderr, "!%s: scl = %g -> \t%g %g %g\n",
            me, scale, vec[0], vec[1], vec[2]);
  }
  return;
}

/*
** parm[0]: lerp between 1 and the stuff below
** parm[1]: "t": (parm[1],0) is control point between (0,0) and (1,1)
** parm[2]: "d": parabolic blend between parm[1]-parm[2] and parm[1]+parm[2]
*/
void
_tenFiberAnisoSpeed(double *step, double xx, double parm[3]) {
  double aa, dd, tt, yy;

  tt = parm[1];
  dd = parm[2];
  aa = 1.0/(DBL_EPSILON + 4*dd*(1.0-tt));
  yy = xx - tt + dd;
  xx = (xx < tt - dd
        ? 0
        : (xx < tt + dd
           ? aa*yy*yy
           : (xx - tt)/(1 - tt)));
  xx = AIR_LERP(parm[0], 1, xx);
  ELL_3V_SCALE(step, xx, step);
}

/*
** -------------------------------------------------------------------
** -------------------------------------------------------------------
** The _tenFiberStep_* routines are responsible for putting a step into
** the given step[] vector.  Without anisoStepSize, this should be
** UNIT LENGTH, with anisoStepSize, its scaled by that anisotropy measure
*/
void
_tenFiberStep_Evec(tenFiberContext *tfx, double step[3]) {

  /* fiberEvec points to the correct gage answer based on fiberType */
  ELL_3V_COPY(step, tfx->fiberEvec + 3*0);
  _tenFiberAlign(tfx, step);
  if (tfx->anisoSpeedType) {
    _tenFiberAnisoSpeed(step, tfx->fiberAnisoSpeed,
                        tfx->anisoSpeedFunc);
  }
}

void
_tenFiberStep_TensorLine(tenFiberContext *tfx, double step[3]) {
  double cl, evec0[3], vout[3], vin[3], len;

  ELL_3V_COPY(evec0, tfx->fiberEvec + 3*0);
  _tenFiberAlign(tfx, evec0);

  if (tfx->lastDirSet) {
    ELL_3V_COPY(vin, tfx->lastDir);
    TEN_T3V_MUL(vout, tfx->fiberTen, tfx->lastDir);
    ELL_3V_NORM(vout, vout, len);
    _tenFiberAlign(tfx, vout);  /* HEY: is this needed? */
  } else {
    ELL_3V_COPY(vin, evec0);
    ELL_3V_COPY(vout, evec0);
  }

  /* HEY: should be using one of the tenAnisoEval[] functions */
  cl = (tfx->fiberEval[0] - tfx->fiberEval[1])/(tfx->fiberEval[0] + 0.00001);

  ELL_3V_SCALE_ADD3(step,
                    cl, evec0,
                    (1-cl)*(1-tfx->wPunct), vin,
                    (1-cl)*tfx->wPunct, vout);
  /* _tenFiberAlign(tfx, step); */
  ELL_3V_NORM(step, step, len);
  if (tfx->anisoSpeedType) {
    _tenFiberAnisoSpeed(step, tfx->fiberAnisoSpeed,
                        tfx->anisoSpeedFunc);
  }
}

void
_tenFiberStep_PureLine(tenFiberContext *tfx, double step[3]) {
  static const char me[]="_tenFiberStep_PureLine";

  AIR_UNUSED(tfx);
  AIR_UNUSED(step);
  fprintf(stderr, "%s: sorry, unimplemented!\n", me);
}

void
_tenFiberStep_Zhukov(tenFiberContext *tfx, double step[3]) {
  static const char me[]="_tenFiberStep_Zhukov";

  AIR_UNUSED(tfx);
  AIR_UNUSED(step);
  fprintf(stderr, "%s: sorry, unimplemented!\n", me);
}

void (*
_tenFiberStep[TEN_FIBER_TYPE_MAX+1])(tenFiberContext *, double *) = {
  NULL,
  _tenFiberStep_Evec,
  _tenFiberStep_Evec,
  _tenFiberStep_Evec,
  _tenFiberStep_TensorLine,
  _tenFiberStep_PureLine,
  _tenFiberStep_Zhukov
};

/*
** -------------------------------------------------------------------
** -------------------------------------------------------------------
** The _tenFiberIntegrate_* routines must assume that
** _tenFiberProbe(tfx, tfx->wPos, AIR_FALSE) has just been called
*/

int
_tenFiberIntegrate_Euler(tenFiberContext *tfx, double forwDir[3]) {

  _tenFiberStep[tfx->fiberType](tfx, forwDir);
  ELL_3V_SCALE(forwDir, tfx->stepSize, forwDir);
  return 0;
}

int
_tenFiberIntegrate_Midpoint(tenFiberContext *tfx, double forwDir[3]) {
  double loc[3], half[3];
  int gret;

  _tenFiberStep[tfx->fiberType](tfx, half);
  ELL_3V_SCALE_ADD2(loc, 1, tfx->wPos, 0.5*tfx->stepSize, half);
  _tenFiberProbe(tfx, &gret, loc, AIR_FALSE); if (gret) return 1;
  _tenFiberStep[tfx->fiberType](tfx, forwDir);
  ELL_3V_SCALE(forwDir, tfx->stepSize, forwDir);
  return 0;
}

int
_tenFiberIntegrate_RK4(tenFiberContext *tfx, double forwDir[3]) {
  double loc[3], k1[3], k2[3], k3[3], k4[3], c1, c2, c3, c4, h;
  int gret;

  h = tfx->stepSize;
  c1 = h/6.0; c2 = h/3.0; c3 = h/3.0; c4 = h/6.0;

  _tenFiberStep[tfx->fiberType](tfx, k1);
  ELL_3V_SCALE_ADD2(loc, 1, tfx->wPos, 0.5*h, k1);
  _tenFiberProbe(tfx, &gret, loc, AIR_FALSE); if (gret) return 1;
  _tenFiberStep[tfx->fiberType](tfx, k2);
  ELL_3V_SCALE_ADD2(loc, 1, tfx->wPos, 0.5*h, k2);
  _tenFiberProbe(tfx, &gret, loc, AIR_FALSE); if (gret) return 1;
  _tenFiberStep[tfx->fiberType](tfx, k3);
  ELL_3V_SCALE_ADD2(loc, 1, tfx->wPos, h, k3);
  _tenFiberProbe(tfx, &gret, loc, AIR_FALSE); if (gret) return 1;
  _tenFiberStep[tfx->fiberType](tfx, k4);

  ELL_3V_SET(forwDir,
             c1*k1[0] + c2*k2[0] + c3*k3[0] + c4*k4[0],
             c1*k1[1] + c2*k2[1] + c3*k3[1] + c4*k4[1],
             c1*k1[2] + c2*k2[2] + c3*k3[2] + c4*k4[2]);

  return 0;
}

int (*
_tenFiberIntegrate[TEN_FIBER_INTG_MAX+1])(tenFiberContext *tfx, double *) = {
  NULL,
  _tenFiberIntegrate_Euler,
  _tenFiberIntegrate_Midpoint,
  _tenFiberIntegrate_RK4
};

/*
** modified body of previous tenFiberTraceSet, in order to
** permit passing the nval for storing desired probed values
*/
static int
_fiberTraceSet(tenFiberContext *tfx, Nrrd *nval, Nrrd *nfiber,
               double *buff, unsigned int halfBuffLen,
               unsigned int *startIdxP, unsigned int *endIdxP,
               double seed[3]) {
  static const char me[]="_fiberTraceSet";
  airArray *fptsArr[2],      /* airArrays of backward (0) and forward (1)
                                fiber points */
    *pansArr[2];             /* airArrays of backward (0) and forward (1)
                                probed values */
  double *fpts[2],           /* arrays storing forward and backward
                                fiber points */
    *pans[2],                /* arrays storing forward and backward
                                probed values */
    tmp[3],
    iPos[3],
    currPoint[3],
    forwDir[3],
    *fiber,                  /* array of both forward and backward points,
                                when finished */
    *valOut;                 /* same for probed values */
  const double *pansP;       /* pointer to gage's probed values */

  int gret, whyStop, buffIdx, fptsIdx, pansIdx, outIdx, oldStop, keepfiber;
  unsigned int i, pansLen;
  airArray *mop;
  airPtrPtrUnion appu;

  if (!(tfx)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (nval) {
    if (!tfx->fiberProbeItem) {
      biffAddf(TEN, "%s: want to record probed values but no item set", me);
      return 1;
    }
    pansLen = gageAnswerLength(tfx->gtx, tfx->pvl, tfx->fiberProbeItem);
    pansP = gageAnswerPointer(tfx->gtx, tfx->pvl, tfx->fiberProbeItem);
  } else {
    pansLen = 0;
    pansP = NULL;
  }
  /*
  fprintf(stderr, "!%s: =========================== \n", me);
  fprintf(stderr, "!%s: \n", me);
  fprintf(stderr, "!%s: item %d -> pansLen = %u\n", me,
          tfx->fiberProbeItem, pansLen);
  fprintf(stderr, "!%s: \n", me);
  fprintf(stderr, "!%s:  =========================== \n", me);
  */

  /* HEY: a hack to preserve the state inside tenFiberContext so that
     we have fewer side effects (tfx->maxNumSteps may still be set) */
  oldStop = tfx->stop;
  if (!nfiber) {
    if (!( buff && halfBuffLen > 0 && startIdxP && startIdxP )) {
      biffAddf(TEN, "%s: need either non-NULL nfiber or fpts buffer info", me);
      return 1;
    }
    if (tenFiberStopSet(tfx, tenFiberStopNumSteps, halfBuffLen)) {
      biffAddf(TEN, "%s: error setting new fiber stop", me);
      return 1;
    }
  }

  /* initialize the quantities which describe the fiber halves */
  tfx->halfLen[0] = tfx->halfLen[1] = 0.0;
  tfx->numSteps[0] = tfx->numSteps[1] = 0;
  tfx->whyStop[0] = tfx->whyStop[1] = tenFiberStopUnknown;
  /*
  fprintf(stderr, "!%s: try probing once, at seed %g %g %g\n", me,
          seed[0], seed[1], seed[2]);
  */
  /* try probing once, at seed point */
  if (tfx->useIndexSpace) {
    gageShapeItoW(tfx->gtx->shape, tmp, seed);
  } else {
    ELL_3V_COPY(tmp, seed);
  }
  if (_tenFiberProbe(tfx, &gret, tmp, AIR_TRUE)) {
    biffAddf(TEN, "%s: first _tenFiberProbe failed", me);
    return 1;
  }
  if (gret) {
    if (gageErrBoundsSpace != tfx->gtx->errNum) {
      biffAddf(TEN, "%s: gage problem on first _tenFiberProbe: %s (%d)",
              me, tfx->gtx->errStr, tfx->gtx->errNum);
      return 1;
    } else {
      /* the problem on the first probe was that it was out of bounds,
         which is not a catastrophe; its handled the same as below */
      tfx->whyNowhere = tenFiberStopBounds;
      if (nval) {
        nrrdEmpty(nval);
      }
      if (nfiber) {
        nrrdEmpty(nfiber);
      } else {
        *startIdxP = *endIdxP = 0;
      }
      return 0;
    }
  }

  /* see if we're doomed (tract dies before it gets anywhere)  */
  /* have to fake out the possible radius check, since at this point
     there is no radius of curvature; this will always pass */
  tfx->radius = DBL_MAX;
  if ((whyStop = _tenFiberStopCheck(tfx))) {
    /* stopped immediately at seed point, but that's not an error */
    tfx->whyNowhere = whyStop;
    if (nval) {
      nrrdEmpty(nval);
    }
    if (nfiber) {
      nrrdEmpty(nfiber);
    } else {
      *startIdxP = *endIdxP = 0;
    }
    return 0;
  } else {
    /* did not immediately halt */
    tfx->whyNowhere = tenFiberStopUnknown;
  }

  /* airMop{Error,Okay}() can safely be called on NULL */
  mop = (nfiber || nval) ? airMopNew() : NULL;

  for (tfx->halfIdx=0; tfx->halfIdx<=1; tfx->halfIdx++) {
    if (nval) {
      appu.d = &(pans[tfx->halfIdx]);
      pansArr[tfx->halfIdx] = airArrayNew(appu.v, NULL,
                                          pansLen*sizeof(double),
                                          TEN_FIBER_INCR);
      airMopAdd(mop, pansArr[tfx->halfIdx],
                (airMopper)airArrayNuke, airMopAlways);
    } else {
      pansArr[tfx->halfIdx] = NULL;
    }
    pansIdx = -1;
    if (nfiber) {
      appu.d = &(fpts[tfx->halfIdx]);
      fptsArr[tfx->halfIdx] = airArrayNew(appu.v, NULL,
                                          3*sizeof(double), TEN_FIBER_INCR);
      airMopAdd(mop, fptsArr[tfx->halfIdx],
                (airMopper)airArrayNuke, airMopAlways);
      buffIdx = -1;
    } else {
      fptsArr[tfx->halfIdx] = NULL;
      fpts[tfx->halfIdx] = NULL;
      buffIdx = halfBuffLen;
    }
    fptsIdx = -1;
    tfx->halfLen[tfx->halfIdx] = 0;
    if (tfx->useIndexSpace) {
      ELL_3V_COPY(iPos, seed);
      gageShapeItoW(tfx->gtx->shape, tfx->wPos, iPos);
    } else {
      /*
      fprintf(stderr, "!%s(A): %p %p %p\n", me,
              tfx->gtx->shape, iPos, seed);
      */
      gageShapeWtoI(tfx->gtx->shape, iPos, seed);
      ELL_3V_COPY(tfx->wPos, seed);
    }
    /* have to initially pass the possible radius check in
       _tenFiberStopCheck(); this will always pass */
    tfx->radius = DBL_MAX;
    ELL_3V_SET(tfx->lastDir, 0, 0, 0);
    tfx->lastDirSet = AIR_FALSE;
    for (tfx->numSteps[tfx->halfIdx] = 0;
         AIR_TRUE;
         tfx->numSteps[tfx->halfIdx]++) {
      _tenFiberProbe(tfx, &gret, tfx->wPos, AIR_FALSE);
      if (gret) {
        /* even if gageProbe had an error OTHER than going out of bounds,
           we're not going to report it any differently here, alas */
        tfx->whyStop[tfx->halfIdx] = tenFiberStopBounds;
        /*
        fprintf(stderr, "!%s: A tfx->whyStop[%d] = %s\n", me, tfx->halfIdx,
                airEnumStr(tenFiberStop, tfx->whyStop[tfx->halfIdx]));
        */
        break;
      }
      if ((whyStop = _tenFiberStopCheck(tfx))) {
        if (tenFiberStopNumSteps == whyStop) {
          /* we stopped along this direction because
             tfx->numSteps[tfx->halfIdx] exceeded tfx->maxNumSteps.
             Okay.  But tfx->numSteps[tfx->halfIdx] is supposed to be
             a record of how steps were (successfully) taken.  So we
             need to decrementing before moving on ... */
          tfx->numSteps[tfx->halfIdx]--;
        }
        tfx->whyStop[tfx->halfIdx] = whyStop;
        /*
        fprintf(stderr, "!%s: B tfx->whyStop[%d] = %s\n", me, tfx->halfIdx,
                airEnumStr(tenFiberStop, tfx->whyStop[tfx->halfIdx]));
        */
        break;
      }
      if (tfx->useIndexSpace) {
        /*
        fprintf(stderr, "!%s(B): %p %p %p\n", me,
                tfx->gtx->shape, iPos, tfx->wPos);
        */
        gageShapeWtoI(tfx->gtx->shape, iPos, tfx->wPos);
        ELL_3V_COPY(currPoint, iPos);
      } else {
        ELL_3V_COPY(currPoint, tfx->wPos);
      }
      if (nval) {
        pansIdx = airArrayLenIncr(pansArr[tfx->halfIdx], 1);
        /* HEY: speed this up */
        memcpy(pans[tfx->halfIdx] + pansLen*pansIdx, pansP,
               pansLen*sizeof(double));
        /*
        fprintf(stderr, "!%s: (dir %d) saving to %d: %g @ (%g,%g,%g)\n", me,
                tfx->halfIdx, pansIdx, pansP[0],
                currPoint[0], currPoint[1], currPoint[2]);
        */
      }
      if (nfiber) {
        fptsIdx = airArrayLenIncr(fptsArr[tfx->halfIdx], 1);
        ELL_3V_COPY(fpts[tfx->halfIdx] + 3*fptsIdx, currPoint);
      } else {
        ELL_3V_COPY(buff + 3*buffIdx, currPoint);
        /*
        fprintf(stderr, "!%s: (dir %d) saving to %d pnt %g %g %g\n", me,
                tfx->halfIdx, buffIdx,
                currPoint[0], currPoint[1], currPoint[2]);
        */
        buffIdx += !tfx->halfIdx ? -1 : 1;
      }
      /* forwDir is set by this to point to the next fiber point */
      if (_tenFiberIntegrate[tfx->intg](tfx, forwDir)) {
        tfx->whyStop[tfx->halfIdx] = tenFiberStopBounds;
        /*
        fprintf(stderr, "!%s: C tfx->whyStop[%d] = %s\n", me, tfx->halfIdx,
                airEnumStr(tenFiberStop, tfx->whyStop[tfx->halfIdx]));
        */
        break;
      }
      /*
      fprintf(stderr, "!%s: forwDir = %g %g %g\n", me,
              forwDir[0], forwDir[1], forwDir[2]);
      */
      if (tfx->stop & (1 << tenFiberStopRadius)) {
        /* some more work required to compute radius of curvature */
        double svec[3], dvec[3], SS, DD, dlen; /* sum,diff length squared */
        /* tfx->lastDir and forwDir are not normalized to unit-length */
        if (tfx->lastDirSet) {
          ELL_3V_ADD2(svec, tfx->lastDir, forwDir);
          ELL_3V_SUB(dvec, tfx->lastDir, forwDir);
          SS = ELL_3V_DOT(svec, svec);
          DD = ELL_3V_DOT(dvec, dvec);
          /* Sun Nov 2 00:04:05 EDT 2008: GLK can't recover how he
             derived this, and can't see why it would be corrrect,
             even though it seems to work correctly...
             tfx->radius = sqrt(SS*(SS+DD)/DD)/4;
          */
          dlen = sqrt(DD);
          tfx->radius = dlen ? (SS + DD)/(4*dlen) : DBL_MAX;
        } else {
          tfx->radius = DBL_MAX;
        }
      }
      /*
      if (!tfx->lastDirSet) {
        fprintf(stderr, "!%s: now setting lastDirSet to (%g,%g,%g)\n", me,
                forwDir[0], forwDir[1], forwDir[2]);
      }
      */
      ELL_3V_COPY(tfx->lastDir, forwDir);
      tfx->lastDirSet = AIR_TRUE;
      ELL_3V_ADD2(tfx->wPos, tfx->wPos, forwDir);
      tfx->halfLen[tfx->halfIdx] += ELL_3V_LEN(forwDir);
    }
  }

  keepfiber = AIR_TRUE;
  if ((tfx->stop & (1 << tenFiberStopStub))
      && (2 == fptsArr[0]->len + fptsArr[1]->len)) {
    /* seed point was actually valid, but neither half got anywhere,
       and the user has set tenFiberStopStub, so we report this as
       a non-starter, via tfx->whyNowhere. */
    tfx->whyNowhere = tenFiberStopStub;
    keepfiber = AIR_FALSE;
  }
  if ((tfx->stop & (1 << tenFiberStopMinNumSteps))
      && (fptsArr[0]->len + fptsArr[1]->len < tfx->minNumSteps)) {
    /* whole fiber didn't have enough steps */
    tfx->whyNowhere = tenFiberStopMinNumSteps;
    keepfiber = AIR_FALSE;
  }
  if ((tfx->stop & (1 << tenFiberStopMinLength))
      && (tfx->halfLen[0] + tfx->halfLen[1] < tfx->minWholeLen)) {
    /* whole fiber wasn't long enough */
    tfx->whyNowhere = tenFiberStopMinLength;
    keepfiber = AIR_FALSE;
  }
  if (!keepfiber) {
    /* for the curious, tfx->whyStop[0,1], tfx->numSteps[0,1], and
       tfx->halfLen[1,2] remain set, from above */
    if (nval) {
      nrrdEmpty(nval);
    }
    if (nfiber) {
      nrrdEmpty(nfiber);
    } else {
      *startIdxP = *endIdxP = 0;
    }
  } else {
    if (nval) {
      if (nrrdMaybeAlloc_va(nval, nrrdTypeDouble, 2,
                            AIR_CAST(size_t, pansLen),
                            AIR_CAST(size_t, (pansArr[0]->len
                                              + pansArr[1]->len - 1)))) {
        biffMovef(TEN, NRRD, "%s: couldn't allocate probed value nrrd", me);
        airMopError(mop); return 1;
      }
      valOut = AIR_CAST(double*, nval->data);
      outIdx = 0;
      /* HEY: speed up memcpy */
      for (i=pansArr[0]->len-1; i>=1; i--) {
        memcpy(valOut + pansLen*outIdx, pans[0] + pansLen*i,
               pansLen*sizeof(double));
        outIdx++;
      }
      for (i=0; i<=pansArr[1]->len-1; i++) {
        memcpy(valOut + pansLen*outIdx, pans[1] + pansLen*i,
               pansLen*sizeof(double));
        outIdx++;
      }
    }
    if (nfiber) {
      if (nrrdMaybeAlloc_va(nfiber, nrrdTypeDouble, 2,
                            AIR_CAST(size_t, 3),
                            AIR_CAST(size_t, (fptsArr[0]->len
                                              + fptsArr[1]->len - 1)))) {
        biffMovef(TEN, NRRD, "%s: couldn't allocate fiber nrrd", me);
        airMopError(mop); return 1;
      }
      fiber = AIR_CAST(double*, nfiber->data);
      outIdx = 0;
      for (i=fptsArr[0]->len-1; i>=1; i--) {
        ELL_3V_COPY(fiber + 3*outIdx, fpts[0] + 3*i);
        outIdx++;
      }
      for (i=0; i<=fptsArr[1]->len-1; i++) {
        ELL_3V_COPY(fiber + 3*outIdx, fpts[1] + 3*i);
        outIdx++;
      }
    } else {
      *startIdxP = halfBuffLen - tfx->numSteps[0];
      *endIdxP = halfBuffLen + tfx->numSteps[1];
    }
  }

  tfx->stop = oldStop;
  airMopOkay(mop);
  return 0;
}

/*
******** tenFiberTraceSet
**
** slightly more flexible API for fiber tracking than tenFiberTrace
**
** EITHER: pass a non-NULL nfiber, and NULL, 0, NULL, NULL for
** the following arguments, and things are the same as with tenFiberTrace:
** data inside the nfiber is allocated, and the tract vertices are copied
** into it, having been stored in dynamically allocated airArrays
**
** OR: pass a NULL nfiber, and a buff allocated for 3*(2*halfBuffLen + 1)
** (note the "+ 1" !!!) doubles.  The fiber tracking on each half will stop
** at halfBuffLen points. The given seedpoint will be stored in
** buff[0,1,2 + 3*halfBuffLen].  The linear (1-D) indices for the end of
** the first tract half, and the end of the second tract half, will be set in
** *startIdxP and *endIdxP respectively (this does not include a multiply
** by 3)
**
** it is worth pointing out here that internally, all tractography is done
** in gage's world space, regardless of tfx->useIndexSpace.  The conversion
** from/to index is space (if tfx->useIndexSpace is non-zero) is only done
** for seedpoints and when fiber vertices are saved out, respectively.
**
** As of Sun Aug  1 20:40:55 CDT 2010 this is just a wrapper around
** _fiberTraceSet; this will probably change in Teem 2.0
*/
int
tenFiberTraceSet(tenFiberContext *tfx, Nrrd *nfiber,
                 double *buff, unsigned int halfBuffLen,
                 unsigned int *startIdxP, unsigned int *endIdxP,
                 double seed[3]) {
  static const char me[]="tenFiberTraceSet";

  if (_fiberTraceSet(tfx, NULL, nfiber, buff, halfBuffLen,
                     startIdxP, endIdxP, seed)) {
    biffAddf(TEN, "%s: problem", me);
    return 1;
  }

  return 0;
}

/*
******** tenFiberTrace
**
** takes a starting position in index or world space, depending on the
** value of tfx->useIndexSpace
*/
int
tenFiberTrace(tenFiberContext *tfx, Nrrd *nfiber, double seed[3]) {
  static const char me[]="tenFiberTrace";

  if (_fiberTraceSet(tfx, NULL, nfiber, NULL, 0, NULL, NULL, seed)) {
    biffAddf(TEN, "%s: problem computing tract", me);
    return 1;
  }

  return 0;
}

/*
******** tenFiberDirectionNumber
**
** NOTE: for the time being, a return of zero indicates an error, not
** that we're being clever and detect that the seedpoint is in such
** isotropy that no directions are possible (though such cleverness
** will hopefully be implemented soon)
*/
unsigned int
tenFiberDirectionNumber(tenFiberContext *tfx, double seed[3]) {
  static const char me[]="tenFiberDirectionNumber";
  unsigned int ret;

  if (!(tfx && seed)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 0;
  }

  /* HEY: eventually this stuff will be specific to the seedpoint ... */

  if (tfx->useDwi) {
    switch (tfx->fiberType) {
    case tenDwiFiberType1Evec0:
      ret = 1;
      break;
    case tenDwiFiberType2Evec0:
      ret = 2;
      break;
    case tenDwiFiberType12BlendEvec0:
      biffAddf(TEN, "%s: sorry, type %s not yet implemented", me,
              airEnumStr(tenDwiFiberType, tenDwiFiberType12BlendEvec0));
      ret = 0;
      break;
    default:
      biffAddf(TEN, "%s: type %d unknown!", me, tfx->fiberType);
      ret = 0;
      break;
    }
  } else {
    /* not using DWIs */
    ret = 1;
  }

  return ret;
}

/*
******** tenFiberSingleTrace
**
** fiber tracing API that uses new tenFiberSingle, as well as being
** aware of multi-direction tractography
**
** NOTE: this will not try any cleverness in setting "num"
** according to whether the seedpoint is a non-starter
*/
int
tenFiberSingleTrace(tenFiberContext *tfx, tenFiberSingle *tfbs,
                    double seed[3], unsigned int which) {
  static const char me[]="tenFiberSingleTrace";

  if (!(tfx && tfbs && seed)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }

  /* set input fields in tfbs */
  ELL_3V_COPY(tfbs->seedPos, seed);
  tfbs->dirIdx = which;
  /* not our job to set tfbx->dirNum ... */

  /* set tfbs->nvert */
  /* no harm in setting this even when there are no multiple fibers */
  tfx->ten2Which = which;
  if (_fiberTraceSet(tfx, (tfx->fiberProbeItem ? tfbs->nval : NULL),
                     tfbs->nvert, NULL, 0, NULL, NULL, seed)) {
    biffAddf(TEN, "%s: problem computing tract", me);
    return 1;
  }

  /* set other fields based on tfx output */
  tfbs->halfLen[0] = tfx->halfLen[0];
  tfbs->halfLen[1] = tfx->halfLen[1];
  tfbs->seedIdx = tfx->numSteps[0];
  tfbs->stepNum[0] = tfx->numSteps[0];
  tfbs->stepNum[1] = tfx->numSteps[1];
  tfbs->whyStop[0] = tfx->whyStop[0];
  tfbs->whyStop[1] = tfx->whyStop[1];
  tfbs->whyNowhere = tfx->whyNowhere;

  return 0;
}

typedef union {
  tenFiberSingle **f;
  void **v;
} fiberunion;

/* uses biff */
tenFiberMulti *
tenFiberMultiNew() {
  static const char me[]="tenFiberMultiNew";
  tenFiberMulti *ret;
  fiberunion tfu;

  ret = AIR_CAST(tenFiberMulti *, calloc(1, sizeof(tenFiberMulti)));
  if (ret) {
    ret->fiber = NULL;
    ret->fiberNum = 0;
    tfu.f = &(ret->fiber);
    ret->fiberArr = airArrayNew(tfu.v, &(ret->fiberNum),
                                sizeof(tenFiberSingle), 512 /* incr */);
    if (ret->fiberArr) {
      airArrayStructCB(ret->fiberArr,
                       AIR_CAST(void (*)(void *), tenFiberSingleInit),
                       AIR_CAST(void (*)(void *), tenFiberSingleDone));
    } else {
      biffAddf(TEN, "%s: couldn't create airArray", me);
      return NULL;
    }
  } else {
    biffAddf(TEN, "%s: couldn't create tenFiberMulti", me);
    return NULL;
  }
  return ret;
}

int
tenFiberMultiCheck(airArray *arr) {
  static const char me[]="tenFiberMultiCheck";

  if (!arr) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (sizeof(tenFiberSingle) != arr->unit) {
    biffAddf(TEN, "%s: given airArray cannot be for fibers", me);
    return 1;
  }
  if (!(AIR_CAST(void (*)(void *), tenFiberSingleInit) == arr->initCB
        && AIR_CAST(void (*)(void *), tenFiberSingleDone) == arr->doneCB)) {
    biffAddf(TEN, "%s: given airArray not set up with fiber callbacks", me);
    return 1;
  }
  return 0;
}

tenFiberMulti *
tenFiberMultiNix(tenFiberMulti *tfm) {

  if (tfm) {
    airArrayNuke(tfm->fiberArr);
    airFree(tfm);
  }
  return NULL;
}

/*
******** tenFiberMultiTrace
**
** does tractography for a list of seedpoints
**
** tfml has been returned from tenFiberMultiNew()
*/
int
tenFiberMultiTrace(tenFiberContext *tfx, tenFiberMulti *tfml,
                   const Nrrd *_nseed) {
  static const char me[]="tenFiberMultiTrace";
  airArray *mop;
  const double *seedData;
  double seed[3];
  unsigned int seedNum, seedIdx, fibrNum, dirNum, dirIdx;
  Nrrd *nseed;

  if (!(tfx && tfml && _nseed)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (tenFiberMultiCheck(tfml->fiberArr)) {
    biffAddf(TEN, "%s: problem with fiber array", me);
    return 1;
  }
  if (!(2 == _nseed->dim && 3 == _nseed->axis[0].size)) {
    biffAddf(TEN, "%s: seed list should be a 2-D (not %u-D) "
            "3-by-X (not %u-by-X) array", me, _nseed->dim,
            AIR_CAST(unsigned int, _nseed->axis[0].size));
    return 1;
  }

  mop = airMopNew();

  seedNum = _nseed->axis[1].size;
  if (nrrdTypeDouble == _nseed->type) {
    seedData = AIR_CAST(const double *, _nseed->data);
  } else {
    nseed = nrrdNew();
    airMopAdd(mop, nseed, AIR_CAST(airMopper, nrrdNuke), airMopAlways);
    if (nrrdConvert(nseed, _nseed, nrrdTypeDouble)) {
      biffMovef(TEN, NRRD, "%s: couldn't convert seed list", me);
      return 1;
    }
    seedData = AIR_CAST(const double *, nseed->data);
  }

  /* HEY: the correctness of the use of the airArray here is quite subtle */
  fibrNum = 0;
  for (seedIdx=0; seedIdx<seedNum; seedIdx++) {
    dirNum = tenFiberDirectionNumber(tfx, seed);
    if (!dirNum) {
      biffAddf(TEN, "%s: couldn't learn dirNum at seed (%g,%g,%g)", me,
              seed[0], seed[1], seed[2]);
      return 1;
    }
    for (dirIdx=0; dirIdx<dirNum; dirIdx++) {
      if (tfx->verbose > 1) {
        fprintf(stderr, "%s: dir %u/%u on seed %u/%u; len %u; # %u\n",
                me, dirIdx, dirNum, seedIdx, seedNum,
                tfml->fiberArr->len, fibrNum);
      }
      /* tfml->fiberArr->len can never be < fibrNum */
      if (tfml->fiberArr->len == fibrNum) {
        airArrayLenIncr(tfml->fiberArr, 1);
      }
      ELL_3V_COPY(tfml->fiber[fibrNum].seedPos, seedData + 3*seedIdx);
      tfml->fiber[fibrNum].dirIdx = dirIdx;
      tfml->fiber[fibrNum].dirNum = dirNum;
      ELL_3V_COPY(seed, seedData + 3*seedIdx);
      if (tenFiberSingleTrace(tfx, &(tfml->fiber[fibrNum]), seed, dirIdx)) {
        biffAddf(TEN, "%s: trouble on seed (%g,%g,%g) %u/%u, dir %u/%u", me,
                seed[0], seed[1], seed[2], seedIdx, seedNum, dirIdx, dirNum);
        return 1;
      }
      if (tfx->verbose) {
        if (tenFiberStopUnknown == tfml->fiber[fibrNum].whyNowhere) {
          fprintf(stderr, "%s: (%g,%g,%g) ->\n"
                  "   steps = %u,%u; len = %g,%g; whyStop = %s,%s\n",
                  me, seed[0], seed[1], seed[2],
                  tfml->fiber[fibrNum].stepNum[0],
                  tfml->fiber[fibrNum].stepNum[1],
                  tfml->fiber[fibrNum].halfLen[0],
                  tfml->fiber[fibrNum].halfLen[1],
                  airEnumStr(tenFiberStop, tfml->fiber[fibrNum].whyStop[0]),
                  airEnumStr(tenFiberStop, tfml->fiber[fibrNum].whyStop[1]));
        } else {
          fprintf(stderr, "%s: (%g,%g,%g) -> whyNowhere: %s\n",
                  me, seed[0], seed[1], seed[2],
                  airEnumStr(tenFiberStop, tfml->fiber[fibrNum].whyNowhere));
        }
      }
      fibrNum++;
    }
  }
  /* if the airArray got to be its length only because of the work above,
     then the following will be a no-op.  Otherwise, via the callbacks,
     it will clear out the tenFiberSingle's that we didn't create here */
  airArrayLenSet(tfml->fiberArr, fibrNum);

  airMopOkay(mop);
  return 0;
}

static int
_fiberMultiExtract(tenFiberContext *tfx, Nrrd *nval,
                   limnPolyData *lpld, tenFiberMulti *tfml) {
  static const char me[]="_fiberMultiExtract";
  unsigned int seedIdx, vertTotalNum, fiberNum, fiberIdx, vertTotalIdx,
    pansLen, pvNum;
  double *valOut;

  if (!(tfx && (lpld || nval) && tfml)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (tenFiberMultiCheck(tfml->fiberArr)) {
    biffAddf(TEN, "%s: problem with fiber array", me);
    return 1;
  }
  if (nval) {
    if (!tfx->fiberProbeItem) {
      biffAddf(TEN, "%s: want probed values but no item set", me);
      return 1;
    }
    pansLen = gageAnswerLength(tfx->gtx, tfx->pvl, tfx->fiberProbeItem);
  } else {
    pansLen = 0;
  }
  /*
  fprintf(stderr, "!%s: =========================== \n", me);
  fprintf(stderr, "!%s: \n", me);
  fprintf(stderr, "!%s: item %d -> pansLen = %u\n", me,
          tfx->fiberProbeItem, pansLen);
  fprintf(stderr, "!%s: \n", me);
  fprintf(stderr, "!%s:  =========================== \n", me);
  */

  /* we have to count the real fibers that went somewhere, excluding
     fibers that went nowhere (counted in tfml->fiberNum) */
  vertTotalNum = 0;
  fiberNum = 0;
  pvNum = 0;
  for (seedIdx=0; seedIdx<tfml->fiberArr->len; seedIdx++) {
    tenFiberSingle *tfs;
    tfs = tfml->fiber + seedIdx;
    if (!(tenFiberStopUnknown == tfs->whyNowhere)) {
      continue;
    }
    if (nval) {
      if (tfs->nval) {
        if (!(2 == tfs->nval->dim
              && pansLen == tfs->nval->axis[0].size
              && tfs->nvert->axis[1].size == tfs->nval->axis[1].size)) {
          biffAddf(TEN, "%s: fiber[%u]->nval seems wrong", me, seedIdx);
          return 1;
        }
        pvNum++;
      }
    }
    vertTotalNum += tfs->nvert->axis[1].size;
    fiberNum++;
  }
  if (nval && pvNum != fiberNum) {
    biffAddf(TEN, "%s: pvNum %u != fiberNum %u", me, pvNum, fiberNum);
    return 1;
  }

  if (nval) {
    if (nrrdMaybeAlloc_va(nval, nrrdTypeDouble, 2,
                          AIR_CAST(size_t, pansLen),
                          AIR_CAST(size_t, vertTotalNum))) {
      biffMovef(TEN, NRRD, "%s: couldn't allocate output", me);
      return 1;
    }
    valOut = AIR_CAST(double *, nval->data);
  } else {
    valOut = NULL;
  }
  if (lpld) {
    if (limnPolyDataAlloc(lpld, 0, /* no extra per-vertex info */
                          vertTotalNum, vertTotalNum, fiberNum)) {
      biffMovef(TEN, LIMN, "%s: couldn't allocate output", me);
      return 1;
    }
  }

  fiberIdx = 0;
  vertTotalIdx = 0;
  for (seedIdx=0; seedIdx<tfml->fiberArr->len; seedIdx++) {
    double *vert, *pans;
    unsigned int vertIdx, vertNum;
    tenFiberSingle *tfs;
    tfs = tfml->fiber + seedIdx;
    if (!(tenFiberStopUnknown == tfs->whyNowhere)) {
      continue;
    }
    vertNum = tfs->nvert->axis[1].size;
    pans = (nval
            ? AIR_CAST(double*, tfs->nval->data)
            : NULL);
    vert = (lpld
            ? AIR_CAST(double*, tfs->nvert->data)
            : NULL);
    for (vertIdx=0; vertIdx<vertNum; vertIdx++) {
      if (lpld) {
        ELL_3V_COPY_TT(lpld->xyzw + 4*vertTotalIdx, float, vert + 3*vertIdx);
        (lpld->xyzw + 4*vertTotalIdx)[3] = 1.0;
        lpld->indx[vertTotalIdx] = vertTotalIdx;
      }
      if (nval) {
        /* HEY speed up memcpy */
        memcpy(valOut + pansLen*vertTotalIdx,
               pans + pansLen*vertIdx,
               pansLen*sizeof(double));
      }
      vertTotalIdx++;
    }
    if (lpld) {
      lpld->type[fiberIdx] = limnPrimitiveLineStrip;
      lpld->icnt[fiberIdx] = vertNum;
    }
    fiberIdx++;
  }

  return 0;
}

/*
******** tenFiberMultiPolyData
**
** converts tenFiberMulti to polydata.
**
** currently the tenFiberContext *tfx arg is not used, but it will
** probably be needed in the future as the way that parameters to the
** polydata creation process are passed.
*/
int
tenFiberMultiPolyData(tenFiberContext *tfx,
                      limnPolyData *lpld, tenFiberMulti *tfml) {
  static const char me[]="tenFiberMultiPolyData";

  if (_fiberMultiExtract(tfx, NULL, lpld, tfml)) {
    biffAddf(TEN, "%s: problem", me);
    return 1;
  }
  return 0;
}


int
tenFiberMultiProbeVals(tenFiberContext *tfx,
                       Nrrd *nval, tenFiberMulti *tfml) {
  static const char me[]="tenFiberMultiProbeVals";

  if (_fiberMultiExtract(tfx, nval, NULL, tfml)) {
    biffAddf(TEN, "%s: problem", me);
    return 1;
  }
  return 0;
}
