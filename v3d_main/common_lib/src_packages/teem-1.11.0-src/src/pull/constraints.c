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

#define PRAYING 0

/*
typedef struct {
  double val, absval, grad[3];
} stateIso;

static int
probeIso(pullTask *task, pullPoint *point, unsigned int iter, int cond,
         double pos[3],
         stateIso *state) {
  static const char me[]="probeIso";

  ELL_3V_COPY(point->pos, pos);  / * NB: not touching point->pos[3] * /
  _pullPointHistAdd(point, cond);
  if (pullProbe(task, point)) {
    biffAddf(PULL, "%s: on iter %u", me, iter);
    return 1;
  }
  state->val = pullPointScalar(task->pctx, point,
                               pullInfoIsovalue,
                               state->grad, NULL);
  state->absval = AIR_ABS(state->val);
  return 0;
}
*/

/* NOTE: this assumes variables "iter" (uint) and "me" (char*) */
#define NORMALIZE_ERR(dir, grad, len)                                    \
  ELL_3V_NORM((dir), (grad), (len));                                     \
  if (!(len)) {                                                          \
    biffAddf(PULL, "%s: got zero grad at (%g,%g,%g,%g) on iter %u\n", me,\
             point->pos[0], point->pos[1], point->pos[2],                \
             point->pos[3], iter);                                       \
    return 1;                                                            \
  }

#define NORMALIZE(dir, grad, len)                                        \
  ELL_3V_NORM((dir), (grad), (len));                                     \
  if (!(len)) {                                                          \
    ELL_3V_SET((dir), 0, 0, 0) ;                                         \
  }


/* ------------------------------- isosurface */



#define PROBE(v, av, g)  if (pullProbe(task, point)) {         \
      biffAddf(PULL, "%s: on iter %u", me, iter);              \
      return 1;                                                \
    }                                                          \
    (v) = pullPointScalar(task->pctx, point,                   \
                          pullInfoIsovalue, (g), NULL);        \
    (av) = AIR_ABS(v)
#define SAVE(state, aval, val, grad, pos)      \
  state[0] = aval;                             \
  state[1] = val;                              \
  ELL_3V_COPY(state + 1 + 1, grad);            \
  ELL_3V_COPY(state + 1 + 1 + 3, pos)
#define RESTORE(aval, val, grad, pos, state)   \
  aval = state[0];                             \
  val = state[1];                              \
  ELL_3V_COPY(grad, state + 1 + 1);            \
  ELL_3V_COPY(pos, state + 1 + 1 + 3)

static int
constraintSatIso(pullTask *task, pullPoint *point,
                 double stepMax, unsigned int iterMax,
                 /* output */
                 int *constrFailP) {
  static const char me[]="constraintSatIso";
  double
    step,         /* current step size */
    val, aval,    /* last and current function values */
    hack,         /* how to control re-tries in the context of a single
                     for-loop, instead of a nested do-while loop */
    grad[4], dir[3], len, state[1 + 1 + 3 + 3];
  unsigned int iter = 0;  /* 0: initial probe, 1..iterMax: probes in loop */

  PROBE(val, aval, grad);
  SAVE(state, aval, val, grad, point->pos);
  hack = 1;
  for (iter=1; iter<=iterMax; iter++) {
    /* consider? http://en.wikipedia.org/wiki/Halley%27s_method */
    NORMALIZE(dir, grad, len);
    if (!len) {
      /* no gradient; back off */
      hack *= task->pctx->sysParm.backStepScale;
      RESTORE(aval, val, grad, point->pos, state);
      continue;
    }
    step = -val/len; /* the newton-raphson step */
    step = step > 0 ? AIR_MIN(stepMax, step) : AIR_MAX(-stepMax, step);
    ELL_3V_SCALE_INCR(point->pos, hack*step, dir);
    _pullPointHistAdd(point, pullCondConstraintSatA);
    PROBE(val, aval, grad);
    if (aval <= state[0]) {  /* we're no further from the root */
      if (AIR_ABS(step) < stepMax*task->pctx->sysParm.constraintStepMin) {
        /* we have converged! */
        break;
      }
      SAVE(state, aval, val, grad, point->pos);
      hack = 1;
    } else { /* oops, try again, don't update dir or len, reset val */
      hack *= task->pctx->sysParm.backStepScale;
      RESTORE(aval, val, grad, point->pos, state);
    }
  }
  if (iter > iterMax) {
    *constrFailP = pullConstraintFailIterMaxed;
  } else {
    *constrFailP = AIR_FALSE;
  }
  return 0;
}

#undef PROBE
#undef SAVE
#undef RESTORE



/* ------------------------------- laplacian */



#define PROBE(l)  if (pullProbe(task, point)) {                    \
      biffAddf(PULL, "%s: on iter %u", me, iter);                  \
      return 1;                                                    \
    }                                                              \
    (l) = pullPointScalar(task->pctx, point,                       \
                          pullInfoHeightLaplacian, NULL, NULL);
#define PROBEG(l, g) \
    PROBE(l);                                                      \
    pullPointScalar(task->pctx, point, pullInfoHeight, (g), NULL);

static int
constraintSatLapl(pullTask *task, pullPoint *point,
                  double stepMax, unsigned int iterMax,
                  /* output */
                  int *constrFailP) {
  static const char me[]="constraintSatLapl";
  double
    step,         /* current step size */
    valLast, val, /* last and current function values */
    grad[4], dir[3], len,
    posOld[3], posNew[3], tmpv[3];
  double a=0, b=1, s, fa, fb, fs, tmp, diff;
  int side = 0;
  unsigned int iter = 0;  /* 0: initial probe, 1..iterMax: probes in loop */

  step = stepMax/2;
  PROBEG(val, grad);
  if (0 == val) {
    /* already exactly at the zero, we're done. This actually happens! */
    /* printf("!%s: a lapl == 0!\n", me); */
    return 0;
  }
  valLast = val;
  NORMALIZE(dir, grad, len);
  /* first phase: follow normalized gradient until laplacian sign change */
  for (iter=1; iter<=iterMax; iter++) {
    double sgn;
    ELL_3V_COPY(posOld, point->pos);
    sgn = airSgn(val); /* lapl < 0 => downhill; lapl > 0 => uphill */
    ELL_3V_SCALE_INCR(point->pos, sgn*step, dir);
    _pullPointHistAdd(point, pullCondConstraintSatA);
    PROBEG(val, grad);
    if (val*valLast < 0) {
      /* laplacian has changed sign; stop looking */
      break;
    }
    valLast = val;
    NORMALIZE(dir, grad, len);
  }
  if (iter > iterMax) {
    *constrFailP = pullConstraintFailIterMaxed;
    return 0;
  }
  /* second phase: find the zero-crossing, looking between
     f(posOld)=valLast and f(posNew)=val */
  ELL_3V_COPY(posNew, point->pos);
  ELL_3V_SUB(tmpv, posNew, posOld);
  len = ELL_3V_LEN(tmpv);
  fa = valLast;
  fb = val;
  if (AIR_ABS(fa) < AIR_ABS(fb)) {
    ELL_SWAP2(a, b, tmp); ELL_SWAP2(fa, fb, tmp);
  }
  for (iter=1; iter<=iterMax; iter++) {
    s = AIR_AFFINE(fa, 0, fb, a, b);
    ELL_3V_LERP(point->pos, s, posOld, posNew);
    _pullPointHistAdd(point, pullCondConstraintSatB);
    PROBE(fs);
    if (0 == fs) {
      /* exactly nailed the zero, we're done. This actually happens! */
      printf("!%s: b lapl == 0!\n", me);
      break;
    }
    /* "Illinois" false-position. Dumb, but it works. */
    if (fs*fb > 0) { /* not between s and b */
      b = s;
      fb = fs;
      if (+1 == side) {
        fa /= 2;
      }
      side = +1;
    } else { /* not between a and s */
      a = s;
      fa = fs;
      if (-1 == side) {
        fb /= 2;
      }
      side = -1;
    }
    diff = (b - a)*len;
    if (AIR_ABS(diff) < stepMax*task->pctx->sysParm.constraintStepMin) {
      /* converged! */
      break;
    }
  }
  if (iter > iterMax) {
    *constrFailP = pullConstraintFailIterMaxed;
  } else {
    *constrFailP = AIR_FALSE;
  }
  return 0;
}
#undef PROBE
#undef PROBEG


/* ------------------------------------------- height (line xor surf) */

static int
probeHeight(pullTask *task, pullPoint *point,
            /* output */
            double *heightP, double grad[3], double hess[9]) {
  static const char me[]="probeHeight";

  if (pullProbe(task, point)) {
    biffAddf(PULL, "%s: trouble", me);
    return 1;
  }
  *heightP = pullPointScalar(task->pctx, point, pullInfoHeight, grad, hess);
  return 0;
}

/*
** creaseProj
**
** eigenvectors (with non-zero eigenvalues) of output posproj are
** tangents to the directions along which particle is allowed to move
** *downward* (in height) for constraint satisfaction (according to
** tangent 1 or tangents 1&2)
**
** negproj is the same, but for points moving upwards (according to
** negativetangent1 or negativetangent 1&2)
*/
static void
creaseProj(pullTask *task, pullPoint *point,
           int tang1Use, int tang2Use,
           int negtang1Use, int negtang2Use,
           /* output */
           double posproj[9], double negproj[9]) {
#if PRAYING
  static const char me[]="creaseProj";
#endif
  double pp[9];
  double *tng;

  ELL_3M_ZERO_SET(posproj);
  if (tang1Use) {
    tng = point->info + task->pctx->infoIdx[pullInfoTangent1];
#if PRAYING
    fprintf(stderr, "!%s: tng1 = %g %g %g\n", me, tng[0], tng[1], tng[2]);
#endif
    ELL_3MV_OUTER(pp, tng, tng);
    ELL_3M_ADD2(posproj, posproj, pp);
  }
  if (tang2Use) {
    tng = point->info + task->pctx->infoIdx[pullInfoTangent2];
    ELL_3MV_OUTER(pp, tng, tng);
    ELL_3M_ADD2(posproj, posproj, pp);
  }

  ELL_3M_ZERO_SET(negproj);
  if (negtang1Use) {
    tng = point->info + task->pctx->infoIdx[pullInfoNegativeTangent1];
    ELL_3MV_OUTER(pp, tng, tng);
    ELL_3M_ADD2(negproj, negproj, pp);
  }
  if (negtang2Use) {
    tng = point->info + task->pctx->infoIdx[pullInfoNegativeTangent2];
    ELL_3MV_OUTER(pp, tng, tng);
    ELL_3M_ADD2(negproj, negproj, pp);
  }

  if (!tang1Use && !tang2Use && !negtang1Use && !negtang2Use) {
    /* we must be after points, and so need freedom to go after them */
    /* for now we do this via posproj not negproj; see haveNada below */
    ELL_3M_IDENTITY_SET(posproj);
  }

  return;
}

/* HEY: body of probeHeight could really be expanded in here */
#define PROBE(height, grad, hess, posproj, negproj)             \
  if (probeHeight(task, point,                                  \
                  &(height), (grad), (hess))) {                 \
    biffAddf(PULL, "%s: trouble on iter %u", me, iter);         \
    return 1;                                                   \
  }                                                             \
  creaseProj(task, point, tang1Use, tang2Use,                   \
             negtang1Use, negtang2Use, posproj, negproj)
#define SAVE(state, height, grad, hess, posproj, negproj, pos)   \
  state[0] = height;                                             \
  ELL_3V_COPY(state + 1, grad);                                  \
  ELL_3M_COPY(state + 1 + 3, hess);                              \
  ELL_3M_COPY(state + 1 + 3 + 9, posproj);                       \
  ELL_3M_COPY(state + 1 + 3 + 9 + 9, negproj);                   \
  ELL_3V_COPY(state + 1 + 3 + 9 + 9 + 9, pos)
#define RESTORE(height, grad, hess, posproj, negproj, pos, state)   \
  height = state[0];                                                \
  ELL_3V_COPY(grad,    state + 1);                                  \
  ELL_3M_COPY(hess,    state + 1 + 3);                              \
  ELL_3M_COPY(posproj, state + 1 + 3 + 9);                          \
  ELL_3M_COPY(negproj, state + 1 + 3 + 9 + 9);                      \
  ELL_3V_COPY(pos,     state + 1 + 3 + 9 + 9 + 9)
#define POSNORM(d1, d2, pdir, plen, pgrad, grad, hess, posproj)       \
  ELL_3MV_MUL(pgrad, posproj, grad);                                  \
  ELL_3V_NORM(pdir, pgrad, plen);                                     \
  d1 = ELL_3V_DOT(grad, pdir);                                        \
  d2 = ELL_3MV_CONTR(hess, pdir)
#define NEGNORM(d1, d2, pdir, plen, pgrad, grad, hess, negproj)       \
  ELL_3MV_MUL(pgrad, negproj, grad);                                  \
  ELL_3V_NORM(pdir, pgrad, plen);                                     \
  d1 = -ELL_3V_DOT(grad, pdir);                                       \
  d2 = -ELL_3MV_CONTR(hess, pdir)
#define PRINT(prefix)                                                   \
  fprintf(stderr, "-------------- probe results %s:\n-- val = %g\n",    \
          prefix, val);                                                 \
  fprintf(stderr, "-- grad = %g %g %g\n", grad[0], grad[1], grad[2]);   \
  fprintf(stderr,"-- hess = %g %g %g;  %g %g %g;  %g %g %g\n",          \
          hess[0], hess[1], hess[2],                                    \
          hess[3], hess[4], hess[5],                                    \
          hess[6], hess[7], hess[8]);                                   \
  fprintf(stderr, "-- posproj = %g %g %g;  %g %g %g;  %g %g %g\n",      \
          posproj[0], posproj[1], posproj[2],                           \
          posproj[3], posproj[4], posproj[5],                           \
          posproj[6], posproj[7], posproj[8]);                          \
  fprintf(stderr, "-- negproj = %g %g %g;  %g %g %g;  %g %g %g\n",      \
          negproj[0], negproj[1], negproj[2],                           \
          negproj[3], negproj[4], negproj[5],                           \
          negproj[6], negproj[7], negproj[8])

static int
constraintSatHght(pullTask *task, pullPoint *point,
                  int tang1Use, int tang2Use,
                  int negtang1Use, int negtang2Use,
                  double stepMax, unsigned int iterMax,
                  int *constrFailP) {
  static const char me[]="constraintSatHght";
  double val, grad[3], hess[9], posproj[9], negproj[9],
    state[1+3+9+9+9+3], hack, step,
    d1, d2, pdir[3], plen, pgrad[3];
#if PRAYING
  double _tmpv[3]={0,0,0};
#endif
  int havePos, haveNeg, haveNada;
  unsigned int iter = 0;  /* 0: initial probe, 1..iterMax: probes in loop */
  /* http://en.wikipedia.org/wiki/Newton%27s_method_in_optimization */

  havePos = tang1Use || tang2Use;
  haveNeg = negtang1Use || negtang2Use;
  haveNada = !havePos && !haveNeg;
#if PRAYING
  {
    double stpmin;
    /* HEY: shouldn't stpmin also be used later in this function? */
    stpmin = task->pctx->voxelSizeSpace*task->pctx->sysParm.constraintStepMin;
    fprintf(stderr, "!%s(%u): starting at %g %g %g %g\n", me, point->idtag,
            point->pos[0], point->pos[1], point->pos[2], point->pos[3]);
    fprintf(stderr, "!%s: pt %d %d nt %d %d (nada %d) "
            "stepMax %g, iterMax %u\n", me,
            tang1Use, tang2Use, negtang1Use, negtang2Use, haveNada,
            stepMax, iterMax);
    fprintf(stderr, "!%s: stpmin = %g = voxsize %g * parm.stepmin %g\n", me,
            stpmin, task->pctx->voxelSizeSpace,
            task->pctx->sysParm.constraintStepMin);
  }
#endif
  _pullPointHistAdd(point, pullCondOld);
  PROBE(val, grad, hess, posproj, negproj);
#if PRAYING
  PRINT("initial probe");
#endif
  SAVE(state, val, grad, hess, posproj, negproj, point->pos);
  hack = 1;
  for (iter=1; iter<=iterMax; iter++) {
#if PRAYING
    fprintf(stderr, "!%s: =============== begin iter %u\n", me, iter);
#endif
    /* HEY: no opportunistic increase of hack? */
    if (havePos || haveNada) {
      POSNORM(d1, d2, pdir, plen, pgrad, grad, hess, posproj);
      if (!plen) {
        /* this use to be a biff error, which got to be annoying */
        *constrFailP = pullConstraintFailProjGradZeroA;
        return 0;
      }
      step = (d2 <= 0 ? -plen : -d1/d2);
#if PRAYING
      fprintf(stderr, "!%s: (+) iter %u step = (%g <= 0 ? %g : %g) --> %g\n",
              me, iter, d2, -plen, -d1/d2, step);
#endif
      step = step > 0 ? AIR_MIN(stepMax, step) : AIR_MAX(-stepMax, step);
      if (AIR_ABS(step) < stepMax*task->pctx->sysParm.constraintStepMin) {
        /* no further iteration needed; we're converged */
#if PRAYING
        fprintf(stderr, "     |step| %g < %g*%g = %g ==> converged!\n",
                AIR_ABS(step),
                stepMax, task->pctx->sysParm.constraintStepMin,
                stepMax*task->pctx->sysParm.constraintStepMin);
#endif
        if (!haveNeg) {
          break;
        } else {
          goto nextstep;
        }
      }
      /* else we have to take a significant step */
#if PRAYING
      fprintf(stderr, "       -> step %g, |pdir| = %g\n",
              step, ELL_3V_LEN(pdir));
      ELL_3V_COPY(_tmpv, point->pos);
      fprintf(stderr, "       ->  pos (%g,%g,%g,%g) += %g * %g * (%g,%g,%g)\n",
              point->pos[0], point->pos[1], point->pos[2], point->pos[3],
              hack, step, pdir[0], pdir[1], pdir[2]);
#endif
      ELL_3V_SCALE_INCR(point->pos, hack*step, pdir);
#if PRAYING
      ELL_3V_SUB(_tmpv, _tmpv, point->pos);
      fprintf(stderr, "       -> moved to %g %g %g %g\n",
              point->pos[0], point->pos[1], point->pos[2], point->pos[3]);
      fprintf(stderr, "       (moved %g)\n", ELL_3V_LEN(_tmpv));
#endif
      _pullPointHistAdd(point, pullCondConstraintSatA);
      PROBE(val, grad, hess, posproj, negproj);
#if PRAYING
      fprintf(stderr, "  (+) probed at (%g,%g,%g,%g)\n",
              point->pos[0], point->pos[1], point->pos[2], point->pos[3]);
      PRINT("after move");
      fprintf(stderr, "  val(%g,%g,%g,%g)=%g %s state[0]=%g\n",
              point->pos[0], point->pos[1], point->pos[2], point->pos[3],
              val, val <= state[0] ? "<=" : ">", state[0]);
#endif
      if (val <= state[0]) {
        /* we made progress */
#if PRAYING
        fprintf(stderr, "  (+) progress!\n");
#endif
        SAVE(state, val, grad, hess, posproj, negproj, point->pos);
        hack = 1;
      } else {
        /* oops, we went uphill instead of down; try again */
#if PRAYING
        fprintf(stderr, "  val *increased*; backing hack from %g to %g\n",
                hack, hack*task->pctx->sysParm.backStepScale);
#endif
        hack *= task->pctx->sysParm.backStepScale;
        RESTORE(val, grad, hess, posproj, negproj, point->pos, state);
#if PRAYING
        fprintf(stderr, "  restored to pos (%g,%g,%g,%g)\n",
                point->pos[0], point->pos[1], point->pos[2], point->pos[3]);
#endif
      }
    }
  nextstep:
    if (haveNeg) {
      /* HEY: copy and paste from above, minus fluff */
      NEGNORM(d1, d2, pdir, plen, pgrad, grad, hess, negproj);
      if (!plen && !haveNeg) {
        /* this use to be a biff error, which got to be annoying */
        *constrFailP = pullConstraintFailProjGradZeroA;
        return 0;
      }
      step = (d2 <= 0 ? -plen : -d1/d2);
#if PRAYING
      fprintf(stderr, "!%s: -+) iter %u step = (%g <= 0 ? %g : %g) --> %g\n",
              me, iter, d2, -plen, -d1/d2, step);
#endif
      step = step > 0 ? AIR_MIN(stepMax, step) : AIR_MAX(-stepMax, step);
      if (AIR_ABS(step) < stepMax*task->pctx->sysParm.constraintStepMin) {
#if PRAYING
        fprintf(stderr, "     |step| %g < %g*%g = %g ==> converged!\n",
                AIR_ABS(step),
                stepMax, task->pctx->sysParm.constraintStepMin,
                stepMax*task->pctx->sysParm.constraintStepMin);
#endif
        /* no further iteration needed; we're converged */
        break;
      }
      /* else we have to take a significant step */
#if PRAYING
      fprintf(stderr, "       -> step %g, |pdir| = %g\n",
              step, ELL_3V_LEN(pdir));
      ELL_3V_COPY(_tmpv, point->pos);
      fprintf(stderr, "       ->  pos (%g,%g,%g,%g) += %g * %g * (%g,%g,%g)\n",
              point->pos[0], point->pos[1], point->pos[2], point->pos[3],
              hack, step, pdir[0], pdir[1], pdir[2]);
#endif
      ELL_3V_SCALE_INCR(point->pos, hack*step, pdir);
#if PRAYING
      ELL_3V_SUB(_tmpv, _tmpv, point->pos);
      fprintf(stderr, "       -> moved to %g %g %g %g\n",
              point->pos[0], point->pos[1], point->pos[2], point->pos[3]);
      fprintf(stderr, "       (moved %g)\n", ELL_3V_LEN(_tmpv));
#endif
      _pullPointHistAdd(point, pullCondConstraintSatA);
      PROBE(val, grad, hess, posproj, negproj);
#if PRAYING
      fprintf(stderr, "  (-) probed at (%g,%g,%g,%g)\n",
              point->pos[0], point->pos[1], point->pos[2], point->pos[3]);
      PRINT("after move");
      fprintf(stderr, "  val(%g,%g,%g,%g)=%g %s state[0]=%g\n",
              point->pos[0], point->pos[1], point->pos[2], point->pos[3],
              val, val >= state[0] ? ">=" : "<", state[0]);
#endif
      if (val >= state[0]) {
        /* we made progress */
#if PRAYING
        fprintf(stderr, "  (-) progress!\n");
#endif
        SAVE(state, val, grad, hess, posproj, negproj, point->pos);
        hack = 1;
      } else {
        /* oops, we went uphill instead of down; try again */
#if PRAYING
        fprintf(stderr, "  val *increased*; backing hack from %g to %g\n",
                hack, hack*task->pctx->sysParm.backStepScale);
#endif
        hack *= task->pctx->sysParm.backStepScale;
        RESTORE(val, grad, hess, posproj, negproj, point->pos, state);
#if PRAYING
        fprintf(stderr, "  restored to pos (%g,%g,%g,%g)\n",
                point->pos[0], point->pos[1], point->pos[2], point->pos[3]);
#endif
      }
    }
  }
  if (iter > iterMax) {
    *constrFailP = pullConstraintFailIterMaxed;
  } else {
    *constrFailP = AIR_FALSE;
  }
  /*
  printf("!%s: %d %s\n", me, *constrFailP,
         *constrFailP ? "FAILED!" : "ok");
          */
  return 0;
}
#undef PROBE
#undef POSNORM
#undef NEGNORM
#undef SAVE
#undef RESTORE

/* ------------------------------------------- */

/* HEY: have to make sure that scale position point->pos[3]
** is not modified anywhere in here: constraints are ONLY spatial
**
** This uses biff, but only for showstopper problems
*/
int
_pullConstraintSatisfy(pullTask *task, pullPoint *point,
                       double travelMax,
                       /* output */
                       int *constrFailP) {
  static const char me[]="_pullConstraintSatisfy";
  double stepMax;
  unsigned int iterMax;
  double pos3Orig[3], pos3Diff[3], travel;

  ELL_3V_COPY(pos3Orig, point->pos);
  stepMax = task->pctx->voxelSizeSpace;
  iterMax = task->pctx->iterParm.constraintMax;
  /*
  dlim = _pullDistLimit(task, point);
  if (iterMax*stepMax > dlim) {
    stepMax = dlim/iterMax;
  }
  */
  /*
  fprintf(stderr, "!%s(%d): hi ==== %g %g %g, stepMax = %g, iterMax = %u\n",
          me, point->idtag, point->pos[0], point->pos[1], point->pos[2],
          stepMax, iterMax);
  */
  task->pctx->count[pullCountConstraintSatisfy] += 1;
  switch (task->pctx->constraint) {
  case pullInfoHeightLaplacian: /* zero-crossing edges */
    if (constraintSatLapl(task, point, stepMax/4, 4*iterMax, constrFailP)) {
      biffAddf(PULL, "%s: trouble", me);
      return 1;
    }
    break;
  case pullInfoIsovalue:
    if (constraintSatIso(task, point, stepMax, iterMax, constrFailP)) {
      biffAddf(PULL, "%s: trouble", me);
      return 1;
    }
    break;
  case pullInfoHeight:
    if (constraintSatHght(task, point,
                          !!task->pctx->ispec[pullInfoTangent1],
                          !!task->pctx->ispec[pullInfoTangent2],
                          !!task->pctx->ispec[pullInfoNegativeTangent1],
                          !!task->pctx->ispec[pullInfoNegativeTangent2],
                          stepMax, iterMax, constrFailP)) {
      biffAddf(PULL, "%s: trouble", me);
      return 1;
    }
    break;
  default:
    fprintf(stderr, "%s: constraint on %s (%d) unimplemented!!\n", me,
            airEnumStr(pullInfo, task->pctx->constraint),
            task->pctx->constraint);
  }
  ELL_3V_SUB(pos3Diff, pos3Orig, point->pos);
  travel = ELL_3V_LEN(pos3Diff)/task->pctx->voxelSizeSpace;
  if (travel > travelMax) {
    *constrFailP = pullConstraintFailTravel;
  }
  /*
  fprintf(stderr, "!%s(%u) %s @ (%g,%g,%g) = (%g,%g,%g) + (%g,%g,%g)\n", me,
          point->idtag,
          (*constrFailP
           ? airEnumStr(pullConstraintFail, *constrFailP)
           : "#GOOD#"),
          point->pos[0], point->pos[1], point->pos[2],
          pos3Diff[0], pos3Diff[1], pos3Diff[2],
          pos3Orig[0], pos3Orig[1], pos3Orig[2]);
  */
  return 0;
}

#undef NORMALIZE

/*
** _pullConstraintTangent
**
** eigenvectors (with non-zero eigenvalues) of output proj are
** (hopefully) approximate tangents to the manifold to which particles
** are constrained.  It is *not* the local tangent of the directions
** along which particles are allowed to move during constraint
** satisfaction (that is given by creaseProj for creases)
**
** this can assume that probe() has just been called
*/
void
_pullConstraintTangent(pullTask *task, pullPoint *point,
                       /* output */
                       double proj[9]) {
  double vec[4], nvec[3], outer[9], len, posproj[9], negproj[9];

  ELL_3M_IDENTITY_SET(proj); /* NOTE: we are starting with identity . . . */
  switch (task->pctx->constraint) {
  case pullInfoHeight:
    creaseProj(task, point,
               !!task->pctx->ispec[pullInfoTangent1],
               !!task->pctx->ispec[pullInfoTangent2],
               !!task->pctx->ispec[pullInfoNegativeTangent1],
               !!task->pctx->ispec[pullInfoNegativeTangent2],
               posproj, negproj);
    /* .. and subracting out output from creaseProj */
    ELL_3M_SUB(proj, proj, posproj);
    ELL_3M_SUB(proj, proj, negproj);
    break;
  case pullInfoHeightLaplacian:
  case pullInfoIsovalue:
    if (pullInfoHeightLaplacian == task->pctx->constraint) {
      /* using gradient of height as approx normal to laplacian 0-crossing */
      pullPointScalar(task->pctx, point, pullInfoHeight, vec, NULL);
    } else {
      pullPointScalar(task->pctx, point, pullInfoIsovalue, vec, NULL);
    }
    ELL_3V_NORM(nvec, vec, len);
    if (len) {
      /* .. or and subracting out tensor product of normal with itself */
      ELL_3MV_OUTER(outer, nvec, nvec);
      ELL_3M_SUB(proj, proj, outer);
    }
    break;
  }
  return;
}

/*
** returns the *dimension* (not codimension) of the constraint manifold:
** 0 for points
** 1 for lines
** 2 for surfaces
**
** a -1 return value represents a biff-able error
*/
int
_pullConstraintDim(const pullContext *pctx) {
  static const char me[]="_pullConstraintDim";
  int ret, t1, t2, nt1, nt2;

  switch (pctx->constraint) {
  case pullInfoHeightLaplacian: /* zero-crossing edges */
    ret = 2;
    break;
  case pullInfoIsovalue:
    ret = 2;
    break;
  case pullInfoHeight:
    t1 = !!pctx->ispec[pullInfoTangent1];
    t2 = !!pctx->ispec[pullInfoTangent2];
    nt1 = !!pctx->ispec[pullInfoNegativeTangent1];
    nt2 = !!pctx->ispec[pullInfoNegativeTangent2];
    switch (t1 + t2 + nt1 + nt2) {
    case 0:
    case 3:
      ret = 0;
      break;
    case 1:
      ret = 2;
      break;
    case 2:
      ret = 1;
      break;
    default:
      biffAddf(PULL, "%s: can't simultaneously use all tangents "
               "(%s,%s,%s,%s) as this implies co-dimension of -1", me,
               airEnumStr(pullInfo, pullInfoTangent1),
               airEnumStr(pullInfo, pullInfoTangent2),
               airEnumStr(pullInfo, pullInfoNegativeTangent1),
               airEnumStr(pullInfo, pullInfoNegativeTangent2));
      return -1;
    }
    break;
  default:
    biffAddf(PULL, "%s: constraint on %s (%d) unimplemented", me,
             airEnumStr(pullInfo, pctx->constraint), pctx->constraint);
    return -1;
  }
  return ret;
}

