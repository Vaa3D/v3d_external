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

#ifndef TEN_PRIVATE_HAS_BEEN_INCLUDED
#define TEN_PRIVATE_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define TEND_CMD(name, info) \
unrrduCmd tend_##name##Cmd = { #name, info, tend_##name##Main }

/* USAGE, PARSE: both copied verbatim from unrrdu/privateUnrrdu.h, but
** then some hacking was added . . .
*/
#define USAGE(info) \
  if (!argc) { \
    hestInfo(stdout, me, (info), hparm); \
    hestUsage(stdout, hopt, me, hparm); \
    hestGlossary(stdout, hopt, hparm); \
    airMopError(mop); \
    return 0; \
  }

/* JUSTPARSE is called by the tend functions that do *not* take an
** input 7-component tensor volume
*/
#define JUSTPARSE()                                             \
  if ((pret=hestParse(hopt, argc, argv, &perr, hparm))) {       \
    if (1 == pret) {                                            \
      fprintf(stderr, "%s: %s\n", me, perr); free(perr);        \
      hestUsage(stderr, hopt, me, hparm);                       \
      airMopError(mop);                                         \
      return 2;                                                 \
    } else {                                                    \
      exit(1);                                                  \
    }                                                           \
  }

/*
** PARSE is called by tend functions that do take a 7-component tensor
** volume, so that as a hack, we can process 6-component volumes as well,
** by padding on the confidence channel (fixed at 1.0)
*/
#define PARSE()                                                         \
  JUSTPARSE();                                                          \
  if (4 == nin->dim                                                     \
      && 6 == nin->axis[0].size                                         \
      && nrrdTypeBlock != nin->type) {                                  \
    ptrdiff_t padmin[4], padmax[4];                                     \
    Nrrd *npadtmp;                                                      \
    /* create a confidence channel by padding on 1s */                  \
    padmin[0] = -1; padmin[1] = padmin[2] = padmin[3] = 0;              \
    padmax[0] = nin->axis[0].size-1;                                    \
    padmax[1] = nin->axis[1].size-1;                                    \
    padmax[2] = nin->axis[2].size-1;                                    \
    padmax[3] = nin->axis[3].size-1;                                    \
    npadtmp = nrrdNew();                                                \
    if (nrrdPad_nva(npadtmp, nin, padmin, padmax, nrrdBoundaryPad, 1.0) \
        || nrrdCopy(nin, npadtmp)) {                                    \
      char *paderr;                                                     \
      airMopAdd(mop, paderr = biffGetDone(NRRD), airFree, airMopAlways);\
      fprintf(stderr, "%s: can't pad 6-comp tensor:\n%s", me, paderr);  \
      airMopError(mop);                                                 \
      nrrdNuke(npadtmp);                                                \
      return 2;                                                         \
    }                                                                   \
    nrrdNuke(npadtmp);                                                  \
  }

/* qseg.c: 2-tensor estimation */
extern void _tenQball(const double b, const int gradcount,
                      const double svals[], const double grads[],
                      double qvals[] );
extern void _tenSegsamp2(const int gradcount, const double qvals[],
                         const double grads[], const double qpoints[],
                         unsigned int seg[], double dists[] );
extern void _tenCalcdists(const int centcount, const double centroid[6],
                          const int gradcount, const double qpoints[],
                          double dists[] );
extern void _tenInitcent2(const int gradcount, const double qvals[],
                          const double grads[], double centroids[6] );
extern int _tenCalccent2(const int gradcount, const double qpoints[],
                         const double dists[], double centroid[6],
                         unsigned int seg[] );
extern void _tenSeg2weights(const int gradcount, const int seg[],
                            const int segcount, double weights[] );
extern void _tenQvals2points(const int gradcount, const double qvals[],
                             const double grads[], double qpoints[] );
extern double _tenPldist(const double point[], const double line[] );

/* arishFuncs.c: Arish's implementation of Peled's 2-tensor fit */
#define VEC_SIZE 3

extern void twoTensFunc(double *p, double *x, int m, int n, void *data);
extern void formTensor2D(double ten[7], double lam1, double lam3, double phi);

/* qglox.c: stuff for quaternion geodesic-loxodromes that has dubious
   utility for the general public */
extern void tenQGLInterpTwoEvalK(double oeval[3],
                                 const double evalA[3],
                                 const double evalB[3],
                                 const double tt);
extern void tenQGLInterpTwoEvalR(double oeval[3],
                                 const double evalA[3],
                                 const double evalB[3],
                                 const double tt);
extern void tenQGLInterpTwoEvec(double oevec[9],
                                const double evecA[9], const double evecB[9],
                                double tt);
extern void tenQGLInterpTwo(double oten[7],
                            const double tenA[7], const double tenB[7],
                            int ptype, double aa, tenInterpParm *tip);
extern int _tenQGLInterpNEval(double evalOut[3],
                              const double *evalIn, /* size 3 -by- NN */
                              const double *wght,   /* size NN */
                              unsigned int NN,
                              int ptype, tenInterpParm *tip);
extern int _tenQGLInterpNEvec(double evecOut[9],
                              const double *evecIn, /* size 9 -by- NN */
                              const double *wght,   /* size NN */
                              unsigned int NN,
                              tenInterpParm *tip);
extern int tenQGLInterpN(double tenOut[7],
                         const double *tenIn,
                         const double *wght,
                         unsigned int NN, int ptype, tenInterpParm *tip);

/* experSpec.c */
TEN_EXPORT double _tenExperSpec_sqe(const double *dwiMeas,
                                    const double *dwiSim,
                                    const tenExperSpec *espec,
                                    int knownB0);
TEN_EXPORT double _tenExperSpec_nll(const double *dwiMeas,
                                    const double *dwiSim,
                                    const tenExperSpec *espec,
                                    int rician, double sigma,
                                    int knownB0);

/* tenModel.c */
TEN_EXPORT double _tenModelSqeFitSingle(const tenModel *model,
                                        double *testParm, double *grad,
                                        double *parm, double *convFrac,
                                        unsigned int *itersTaken,
                                        const tenExperSpec *espec,
                                        double *dwiBuff,
                                        const double *dwiMeas,
                                        const double *parmInit,
                                        int knownB0,
                                        unsigned int minIter,
                                        unsigned int maxIter,
                                        double convEps,
                                        int verbose);

/* model*.c */
/*
** NOTE: these functions rely on per-model information (like PARM_NUM)
** or functions (like "simulate"), which is why these functions don't
** all use some common underlying function: more information would
** have to be passed (annoying for rapid hacking), and the function
** call overhead might be doubled
*/

#define _TEN_PARM_ALLOC                                         \
static double *                                                 \
parmAlloc(void) {                                               \
                                                                \
  return AIR_CAST(double *, calloc(PARM_NUM ? PARM_NUM : 1, sizeof(double))); \
}

#define _TEN_PARM_RAND                                                  \
static void                                                             \
parmRand(double *parm, airRandMTState *rng, int knownB0) {              \
  unsigned int ii;                                                      \
                                                                        \
  for (ii=knownB0 ? 1 : 0; ii<PARM_NUM; ii++) {                         \
    if (parmDesc[ii].vec3) {                                            \
      /* its unit vector */                                             \
      double xx, yy, zz, theta, rr;                                     \
                                                                        \
      zz = AIR_AFFINE(0.0, airDrandMT_r(rng), 1.0, -1.0, 1.0);          \
      theta = AIR_AFFINE(0.0, airDrandMT_r(rng), 1.0, 0.0, 2*AIR_PI);   \
      rr = sqrt(1 - zz*zz);                                             \
      xx = rr*cos(theta);                                               \
      yy = rr*sin(theta);                                               \
      parm[ii + 0] = xx;                                                \
      parm[ii + 1] = yy;                                                \
      parm[ii + 2] = zz;                                                \
      /* bump ii by 2, anticipating end of this for-loop iteration */   \
      ii += 2;                                                          \
    } else {                                                            \
      parm[ii] = AIR_AFFINE(0.0, airDrandMT_r(rng), 1.0,                \
                            parmDesc[ii].min, parmDesc[ii].max);        \
    }                                                                   \
  }                                                                     \
  return;                                                               \
}

#define _TEN_PARM_STEP                                                  \
static void                                                             \
parmStep(double *parm1, const double scl,                               \
         const double *grad, const double *parm0) {                     \
  unsigned int ii;                                                      \
                                                                        \
  for (ii=0; ii<PARM_NUM; ii++) {                                       \
    parm1[ii] = scl*grad[ii] + parm0[ii];                               \
    if (parmDesc[ii].cyclic) {                                          \
      double delta = parmDesc[ii].max - parmDesc[ii].min;               \
      while (parm1[ii] > parmDesc[ii].max) {                            \
        parm1[ii] -= delta;                                             \
      }                                                                 \
      while (parm1[ii] < parmDesc[ii].min) {                            \
        parm1[ii] += delta;                                             \
      }                                                                 \
    } else {                                                            \
      parm1[ii] = AIR_CLAMP(parmDesc[ii].min, parm1[ii], parmDesc[ii].max); \
    }                                                                   \
    if (parmDesc[ii].vec3 && 2 == parmDesc[ii].vecIdx) {                \
      double len;                                                       \
      ELL_3V_NORM(parm1 + ii - 2, parm1 + ii - 2, len);                 \
    }                                                                   \
  }                                                                     \
}

#define _TEN_PARM_DIST                                                  \
static double                                                           \
parmDist(const double *parmA, const double *parmB) {                    \
  unsigned int ii;                                                      \
  double dist;                                                          \
                                                                        \
  dist = 0;                                                             \
  for (ii=0; ii<PARM_NUM; ii++) {                                       \
  double dp1, delta;                                                    \
    delta = parmDesc[ii].max - parmDesc[ii].min;                        \
    dp1 = parmA[ii] - parmB[ii];                                        \
    if (parmDesc[ii].cyclic) {                                          \
      double dp2, dp3;                                                  \
      dp2 = dp1 + delta;                                                \
      dp3 = dp1 - delta;                                                \
      dp1 *= dp1;                                                       \
      dp2 *= dp2;                                                       \
      dp3 *= dp3;                                                       \
      dp1 = AIR_MIN(dp1, dp2);                                          \
      dp1 = AIR_MIN(dp1, dp3);                                          \
      dist += dp1/(delta*delta);                                        \
    } else {                                                            \
      dist += dp1*dp1/(delta*delta);                                    \
    }                                                                   \
  }                                                                     \
  return sqrt(dist);                                                    \
}

#define _TEN_PARM_COPY                          \
static void                                     \
parmCopy(double *parmA, const double *parmB) {  \
  unsigned int ii;                              \
                                                \
  for (ii=0; ii<PARM_NUM; ii++) {               \
    parmA[ii] = parmB[ii];                      \
  }                                             \
  return;                                       \
}

#define _TEN_PARM_CONVERT_NOOP                         \
static int                                             \
parmConvert(double *parmDst, const double *parmSrc,    \
            const tenModel *modelSrc) {                \
  unsigned int ii;                                     \
                                                       \
  AIR_UNUSED(modelSrc);                                \
  parmDst[0] = parmSrc[0];                             \
  for (ii=1; ii<PARM_NUM; ii++) {                      \
    parmDst[ii] = AIR_NAN;                             \
  }                                                    \
  return 1;                                            \
}

#define _TEN_SQE                                                \
static double                                                   \
sqe(const double *parm, const tenExperSpec *espec,              \
    double *dwiBuff, const double *dwiMeas, int knownB0) {      \
                                                                \
  simulate(dwiBuff, parm, espec);                               \
  return _tenExperSpec_sqe(dwiMeas, dwiBuff, espec, knownB0);   \
}

#define _TEN_SQE_GRAD_CENTDIFF                                          \
static void                                                             \
sqeGrad(double *grad, const double *parm0,                              \
        const tenExperSpec *espec,                                      \
        double *dwiBuff, const double *dwiMeas,                         \
        int knownB0) {                                                  \
  double parm1[PARM_NUM ? PARM_NUM : 1], sqeForw, sqeBack, dp;          \
  unsigned int ii, i0;                                                  \
                                                                        \
  i0 = knownB0 ? 1 : 0;                                                 \
  for (ii=0; ii<PARM_NUM; ii++) {                                       \
    /* have to copy all parms, even B0 with knownB0, and even if we     \
       aren't going to diddle these values, because the                 \
       simulation depends on knowing the values */                      \
    parm1[ii] = parm0[ii];                                              \
  }                                                                     \
  for (ii=i0; ii<PARM_NUM; ii++) {                                      \
    dp = (parmDesc[ii].max - parmDesc[ii].min)*TEN_MODEL_PARM_GRAD_EPS; \
    parm1[ii] = parm0[ii] + dp;                                         \
    sqeForw = sqe(parm1, espec, dwiBuff, dwiMeas, knownB0);             \
    parm1[ii] = parm0[ii] - dp;                                         \
    sqeBack = sqe(parm1, espec, dwiBuff, dwiMeas, knownB0);             \
    grad[ii] = (sqeForw - sqeBack)/(2*dp);                              \
    parm1[ii] = parm0[ii];                                              \
    if (parmDesc[ii].vec3 && 2 == parmDesc[ii].vecIdx) {                \
      double dot, *gg;                                                  \
      const double *vv;                                                 \
      gg = grad + ii - 2;                                               \
      vv = parm0 + ii - 2;                                              \
      dot = ELL_3V_DOT(gg, vv);                                         \
      ELL_3V_SCALE_INCR(gg, -dot, vv);                                  \
    }                                                                   \
  }                                                                     \
  if (knownB0) {                                                        \
    grad[0] = 0;                                                        \
  }                                                                     \
  return;                                                               \
}

#define _TEN_SQE_FIT(model)                                             \
static double                                                           \
sqeFit(double *parm, double *convFrac, unsigned int *itersTaken,        \
       const tenExperSpec *espec,                                       \
       double *dwiBuff, const double *dwiMeas,                          \
       const double *parmInit, int knownB0,                             \
       unsigned int minIter, unsigned int maxIter,                      \
       double convEps, int verbose) {                                   \
  double testparm[PARM_NUM ? PARM_NUM : 1],                             \
    grad[PARM_NUM ? PARM_NUM : 1];                                      \
                                                                        \
  return _tenModelSqeFitSingle((model),                                 \
                               testparm, grad,                          \
                               parm, convFrac, itersTaken,              \
                               espec, dwiBuff, dwiMeas,                 \
                               parmInit, knownB0,                       \
                               minIter, maxIter,                        \
                               convEps, verbose);                       \
}

#define _TEN_SQE_FIT_STUB                                              \
static double                                                          \
sqeFit(double *parm, double *convFrac, unsigned int *itersTaken,       \
       const tenExperSpec *espec,                                      \
       double *dwiBuff, const double *dwiMeas,                         \
       const double *parmInit, int knownB0,                            \
       unsigned int minIter, unsigned int maxIter, double convEps) {   \
  unsigned int pp;                                                     \
                                                                       \
  AIR_UNUSED(convFrac);                                                \
  AIR_UNUSED(espec);                                                   \
  AIR_UNUSED(dwiBuff);                                                 \
  AIR_UNUSED(dwiMeas);                                                 \
  AIR_UNUSED(knownB0);                                                 \
  AIR_UNUSED(minIter);                                                 \
  AIR_UNUSED(maxIter);                                                 \
  AIR_UNUSED(convEps);                                                 \
  for (pp=0; pp<PARM_NUM; pp++) {                                      \
    parm[pp] = parmInit[pp];                                           \
  }                                                                    \
  return 0;                                                            \
}

#define _TEN_NLL                                            \
  static double                                             \
nll(const double *parm, const tenExperSpec *espec,          \
    double *dwiBuff, const double *dwiMeas,                 \
    int rician, double sigma, int knownB0) {                \
                                                            \
  simulate(dwiBuff, parm, espec);                           \
  return _tenExperSpec_nll(dwiMeas, dwiBuff, espec,         \
                           rician, sigma, knownB0);         \
}

#define _TEN_NLL_GRAD_STUB                      \
static void                                     \
nllGrad(double *grad, const double *parm,       \
        const tenExperSpec *espec,              \
        double *dwiBuff, const double *dwiMeas, \
        int rician, double sigma) {             \
                                                \
  AIR_UNUSED(grad);                             \
  AIR_UNUSED(parm);                             \
  AIR_UNUSED(espec);                            \
  AIR_UNUSED(dwiBuff);                          \
  AIR_UNUSED(dwiMeas);                          \
  AIR_UNUSED(rician);                           \
  AIR_UNUSED(sigma);                            \
  return;                                       \
}

#define _TEN_NLL_FIT_STUB                               \
static double                                           \
nllFit(double *parm, const tenExperSpec *espec,         \
       const double *dwiMeas, const double *parmInit,   \
       int rician, double sigma, int knownB0) {         \
  unsigned int pp;                                      \
                                                        \
  AIR_UNUSED(espec);                                    \
  AIR_UNUSED(dwiMeas);                                  \
  AIR_UNUSED(rician);                                   \
  AIR_UNUSED(sigma);                                    \
  AIR_UNUSED(knownB0);                                  \
  for (pp=0; pp<PARM_NUM; pp++) {                       \
    parm[pp] = parmInit[pp];                            \
  }                                                     \
  return 0;                                             \
}

#define _TEN_MODEL_FIELDS                                           \
  PARM_NUM, parmDesc,                                               \
  simulate,                                                         \
  parmSprint, parmAlloc, parmRand,                                  \
  parmStep, parmDist, parmCopy, parmConvert,                        \
  sqe, sqeGrad, sqeFit,                                             \
  nll, nllGrad, nllFit

#ifdef __cplusplus
}
#endif

#endif /* TEN_PRIVATE_HAS_BEEN_INCLUDED */
