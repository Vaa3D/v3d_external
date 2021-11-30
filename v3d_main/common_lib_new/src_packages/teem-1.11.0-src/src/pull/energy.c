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

const char *
_pullInterTypeStr[PULL_INTER_TYPE_MAX+1] = {
  "(unknown_inter)",
  "justR",
  "univariate",
  "separable",
  "additive"
};

const char *
_pullInterTypeStrEqv[] = {
  "r", "justr",
  "univariate", "univar", "uni",
  "separable", "separ", "sep",
  "additive", "add",
  ""
};

const int
_pullInterTypeValEqv[] = {
  pullInterTypeJustR, pullInterTypeJustR,
  pullInterTypeUnivariate, pullInterTypeUnivariate, pullInterTypeUnivariate,
  pullInterTypeSeparable, pullInterTypeSeparable, pullInterTypeSeparable,
  pullInterTypeAdditive, pullInterTypeAdditive
};

const airEnum
_pullInterType = {
  "interaction type",
  PULL_INTER_TYPE_MAX,
  _pullInterTypeStr,  NULL,
  NULL,
  _pullInterTypeStrEqv, _pullInterTypeValEqv,
  AIR_FALSE
};
const airEnum *const
pullInterType = &_pullInterType;

#define SPRING    "spring"
#define GAUSS     "gauss"
#define BSPLN     "bspln"
#define BUTTER    "butter"
#define COTAN     "cotan"
#define CUBIC     "cubic"
#define QUARTIC   "quartic"
#define CWELL     "cwell"
#define BWELL     "bwell"
#define QWELL     "qwell"
#define HWELL     "hwell"
#define ZERO      "zero"
#define BPARAB    "bparab"

const char *
_pullEnergyTypeStr[PULL_ENERGY_TYPE_MAX+1] = {
  "(unknown_energy)",
  SPRING,
  GAUSS,
  BSPLN,
  BUTTER,
  COTAN,
  CUBIC,
  QUARTIC,
  CWELL,
  BWELL,
  QWELL,
  HWELL,
  ZERO,
  BPARAB
};

const char *
_pullEnergyTypeDesc[PULL_ENERGY_TYPE_MAX+1] = {
  "unknown_energy",
  "Hooke's law-based potential, with a tunable region of attraction",
  "Gaussian potential",
  "uniform cubic B-spline basis function",
  "like a Gaussian, but a lot wider",
  "Cotangent-based potential (from Meyer et al. SMI '05)",
  "Cubic thing",
  "Quartic thing",
  "Piecewice cubic with tunable well location and depth",
  "Better piecewice cubic with tunable well location and depth",
  "Single quartic with tunable well location",
  "Single heptic with tunable well location",
  "no energy",
  "butterworth-windowed spatial repel and scale attract"
};

const airEnum
_pullEnergyType = {
  "energy",
  PULL_ENERGY_TYPE_MAX,
  _pullEnergyTypeStr,  NULL,
  _pullEnergyTypeDesc,
  NULL, NULL,
  AIR_FALSE
};
const airEnum *const
pullEnergyType = &_pullEnergyType;

double
_pullEnergyNoWell(double *wx, const double *parm) {

  AIR_UNUSED(wx);
  AIR_UNUSED(parm);
  return 0.0;
}

/* ----------------------------------------------------------------
** ------------------------------ UNKNOWN -------------------------
** ----------------------------------------------------------------
*/
double
_pullEnergyUnknownEval(double *denr, double dist, const double *parm) {
  static const char me[]="_pullEnergyUnknownEval";

  AIR_UNUSED(dist);
  AIR_UNUSED(parm);
  *denr = AIR_NAN;
  fprintf(stderr, "%s: ERROR- using unknown energy.\n", me);
  return AIR_NAN;
}

pullEnergy
_pullEnergyUnknown = {
  "unknown",
  0,
  _pullEnergyNoWell,
  _pullEnergyUnknownEval
};
const pullEnergy *const
pullEnergyUnknown = &_pullEnergyUnknown;

/* ----------------------------------------------------------------
** ------------------------------ SPRING --------------------------
** ----------------------------------------------------------------
** 1 parms:
** parm[0]: width of pull region.  Used to be width beyond 1.0, but
** now things are scrunched to fit both repelling and attractive
** region inside [0,1]
**
** learned: "1/2" is not 0.5 !!!!!
*/
double
_pullEnergySpringEval(double *denr, double dist, const double *parm) {
  /* static const char me[]="_pullEnergySpringEval"; */
  double enr, xx, pull;

  pull = parm[0];
  /* support used to be [0,1 + pull], but now is scrunched to [0,1],
     so hack "dist" to match old parameterization */
  dist = AIR_AFFINE(0, dist, 1, 0, 1+pull);
  xx = dist - 1.0;
  if (xx > pull) {
    enr = 0;
    *denr = 0;
  } else if (xx > 0) {
    enr = xx*xx*(xx*xx/(4*pull*pull) - 2*xx/(3*pull) + 1.0/2.0);
    *denr = xx*(xx*xx/(pull*pull) - 2*xx/pull + 1);
  } else {
    enr = xx*xx/2;
    *denr = xx;
  }
  /*
  if (!AIR_EXISTS(ret)) {
    printf("!%s: dist=%g, pull=%g, blah=%d --> ret=%g\n",
           me, dist, pull, blah, ret);
  }
  */
  return enr;
}

int
_pullEnergySpringWell(const double *parm) {

  return (parm[0] > 0.0);
}

const pullEnergy
_pullEnergySpring = {
  SPRING,
  1,
  _pullEnergyNoWell, /* HEY: is this true? */
  _pullEnergySpringEval
};
const pullEnergy *const
pullEnergySpring = &_pullEnergySpring;

/* ----------------------------------------------------------------
** ------------------------------ GAUSS --------------------------
** ----------------------------------------------------------------
** 0 parms: for simplicity we're now always cutting off at 4 sigmas
*/
/* HEY: copied from teem/src/nrrd/kernel.c */
#define _GAUSS(x, sig, cut) ( \
   x >= sig*cut ? 0           \
   : exp(-x*x/(2.0*sig*sig)))

#define _DGAUSS(x, sig, cut) ( \
   x >= sig*cut ? 0            \
   : -exp(-x*x/(2.0*sig*sig))*(x/(sig*sig)))

double
_pullEnergyGaussEval(double *denr, double dist, const double *parm) {

  AIR_UNUSED(parm);
  *denr = _DGAUSS(dist, 0.25, 4);
  return _GAUSS(dist, 0.25, 4);
}

const pullEnergy
_pullEnergyGauss = {
  GAUSS,
  0,
  _pullEnergyNoWell,
  _pullEnergyGaussEval
};
const pullEnergy *const
pullEnergyGauss = &_pullEnergyGauss;

/* ----------------------------------------------------------------
** ------------------------------ BSPLN --------------------------
** ----------------------------------------------------------------
** 0 parms
*/
/* HEY: copied from teem/src/nrrd/bsplKernel.c */
#define BSPL(ret, t, x)                        \
  if (x < 1) {                                 \
    ret = (4 + 3*(-2 + x)*x*x)/6;              \
  } else if (x < 2) {                          \
    t = (-2 + x);                              \
    ret = -t*t*t/6;                            \
  } else {                                     \
    ret = 0;                                   \
  }

#define DBSPL(ret, t, x)                       \
  if (x < 1) {                                 \
    ret = (-4 + 3*x)*x/2;                      \
  } else if (x < 2) {                          \
    t = (-2 + x);                              \
    ret = -t*t/2;                              \
  } else {                                     \
    ret = 0;                                   \
  }

double
_pullEnergyBsplnEval(double *denr, double dist, const double *parm) {
  double tmp, ret;

  AIR_UNUSED(parm);
  dist *= 2;
  DBSPL(*denr, tmp, dist);
  *denr *= 2;
  BSPL(ret, tmp, dist);
  return ret;
}

const pullEnergy
_pullEnergyBspln = {
  BSPLN,
  0,
  _pullEnergyNoWell,
  _pullEnergyBsplnEval
};
const pullEnergy *const
pullEnergyBspln = &_pullEnergyBspln;


/* ----------------------------------------------------------------
** ------------------------------ BUTTER --------------------------
** ----------------------------------------------------------------
** 2 parms: order (an integer) and "cut-ff" (where height==0.5)
*/

double
_pullEnergyButterworthEval(double *denr, double x, const double *parm) {
  int n;
  double cut, denom, enr;

  n = AIR_CAST(int, parm[0]);
  cut = parm[1];
  denom = 1 + airIntPow(x/cut, 2*n);
  enr = 1/denom;
  *denr = -2*n*airIntPow(x/cut, 2*n - 1)*enr*enr/cut;
  return enr;
}

const pullEnergy
_pullEnergyButterworth= {
  BUTTER,
  2,
  _pullEnergyNoWell,
  _pullEnergyButterworthEval
};
const pullEnergy *const
pullEnergyButterworth = &_pullEnergyButterworth;

/* ----------------------------------------------------------------
** ------------------------------ COTAN ---------------------------
** ----------------------------------------------------------------
** 0 parms!
*/
double
_pullEnergyCotanEval(double *denr, double dist, const double *parm) {
  double pot, cc, enr;

  AIR_UNUSED(parm);
  pot = AIR_PI/2.0;
  cc = 1.0/(FLT_MIN + tan(dist*pot));
  enr = dist > 1 ? 0 : cc + dist*pot - pot;
  *denr = dist > 1 ? 0 : -cc*cc*pot;
  return enr;
}

const pullEnergy
_pullEnergyCotan = {
  COTAN,
  0,
  _pullEnergyNoWell,
  _pullEnergyCotanEval
};
const pullEnergy *const
pullEnergyCotan = &_pullEnergyCotan;

/* ----------------------------------------------------------------
** ------------------------------ CUBIC ---------------------------
** ----------------------------------------------------------------
** 0 parms!
*/
double
_pullEnergyCubicEval(double *denr, double dist, const double *parm) {
  double omr, enr;

  AIR_UNUSED(parm);
  if (dist <= 1) {
    omr = 1 - dist;
    enr = omr*omr*omr;
    *denr = -3*omr*omr;
  } else {
    enr = *denr = 0;
  }
  return enr;
}

const pullEnergy
_pullEnergyCubic = {
  CUBIC,
  0,
  _pullEnergyNoWell,
  _pullEnergyCubicEval
};
const pullEnergy *const
pullEnergyCubic = &_pullEnergyCubic;

/* ----------------------------------------------------------------
** ----------------------------- QUARTIC --------------------------
** ----------------------------------------------------------------
** 0 parms!
*/
double
_pullEnergyQuarticEval(double *denr, double dist, const double *parm) {
  double omr, enr;

  AIR_UNUSED(parm);
  if (dist <= 1) {
    omr = 1 - dist;
    enr = 2.132*omr*omr*omr*omr;
    *denr = -4*2.132*omr*omr*omr;
  } else {
    enr = *denr = 0;
  }
  return enr;
}

const pullEnergy
_pullEnergyQuartic = {
  QUARTIC,
  0,
  _pullEnergyNoWell,
  _pullEnergyQuarticEval
};
const pullEnergy *const
pullEnergyQuartic = &_pullEnergyQuartic;

/* ----------------------------------------------------------------
** ------------------ tunable piece-wise cubic --------------------
** ----------------------------------------------------------------
** 2 parm: wellX, wellY
*/
double
_pullEnergyCubicWellEval(double *denr, double x, const double *parm) {
  double a, b, c, d, e, wx, wy, enr;

  wx = parm[0];
  wy = parm[1];
  a = (3*(-1 + wy))/wx;
  b = (-3*(-1 + wy))/(wx*wx);
  c = -(1 - wy)/(wx*wx*wx);
  d = (-3*wy)/((wx-1)*(wx-1));
  e = (-2*wy)/((wx-1)*(wx-1)*(wx-1));
  if (x < wx) {
    enr = 1 + x*(a + x*(b + c*x));
    *denr = a + x*(2*b + 3*c*x);
  } else if (x < 1) {
    double _x;
    _x = x - wx;
    enr = wy + _x*_x*(d + e*_x);
    *denr = _x*(2*d + 3*e*_x);
  } else {
    enr = 0;
    *denr = 0;
  }
  return enr;
}

double
_pullEnergyCubicWellWell(double *wx, const double *parm) {

  *wx = parm[0];
  return AIR_MIN(0.0, parm[1]);
}

const pullEnergy
_pullEnergyCubicWell = {
  CWELL,
  2,
  _pullEnergyCubicWellWell,
  _pullEnergyCubicWellEval
};
const pullEnergy *const
pullEnergyCubicWell = &_pullEnergyCubicWell;

/* ----------------------------------------------------------------
** --------------- better tunable piece-wise cubic ----------------
** ----------------------------------------------------------------
** 2 parm: wellX, wellY
*/
double
_pullEnergyBetterCubicWellEval(double *denr, double x, const double *parm) {
  double a, b, c, d, e, f, g, wx, wy, xmo, xmoo, xmooo, enr;

  /* HEY: it would be good if there was a place to store these
     intermediate values, so that they don't need to be computed
     for *every*single* inter-particle interaction */
  wx = parm[0];
  wy = parm[1];
  xmo = wx-1;
  xmoo = xmo*xmo;
  xmooo = xmoo*xmo;
  a = -3*(xmoo + (-1 + 2*wx)*wy)/(xmoo*wx);
  b = 3*(xmoo + (-1 + wx*(2 + wx))*wy)/(xmoo*wx*wx);
  c = (-1 + wy - wx*(-2 + wx + 2*(1 + wx)*wy))/(xmoo*wx*wx*wx);
  d = ((-1 + 3*wx)*wy)/xmooo;
  e = -(6*wx*wy)/xmooo;
  f = (3*(1 + wx)*wy)/xmooo;
  g = -(2*wy)/xmooo;
  if (x < wx) {
    enr = 1 + x*(a + x*(b + x*c));
    *denr = a + x*(2*b + x*3*c);
  } else if (x < 1) {
    enr = d + x*(e + x*(f + x*g));
    *denr = e + x*(2*f + x*3*g);
  } else {
    enr = 0;
    *denr = 0;
  }
  return enr;
}

double
_pullEnergyBetterCubicWellWell(double *wx, const double *parm) {

  *wx = parm[0];
  return AIR_MIN(0.0, parm[1]);
}

const pullEnergy
_pullEnergyBetterCubicWell = {
  BWELL,
  2,
  _pullEnergyBetterCubicWellWell,
  _pullEnergyBetterCubicWellEval
};
const pullEnergy *const
pullEnergyBetterCubicWell = &_pullEnergyBetterCubicWell;

/* ----------------------------------------------------------------
** ----------------- tunable single quartic well ------------------
** ----------------------------------------------------------------
** 1 parm: well radius
*/
double
_pullEnergyQuarticWellEval(double *denr, double x, const double *parm) {
  double a, b, c, d, w, enr;

  w = parm[0];
  a = (12*w)/(1 - 4*w);
  b = 3 + 9/(-1 + 4*w);
  c = (8 + 4*w)/(1 - 4*w);
  d = 3/(-1 + 4*w);
  enr = 1 + x*(a + x*(b + x*(c + x*d)));
  *denr = a + x*(2*b + x*(3*c + x*4*d));
  return enr;
}

double
_pullEnergyQuarticWellWell(double *wx, const double *parm) {
  double t;

  *wx = parm[0];
  t = *wx - 1;
  return t*t*t*t/(4*(*wx) - 1);
}

const pullEnergy
_pullEnergyQuarticWell = {
  QWELL,
  1,
  _pullEnergyQuarticWellWell,
  _pullEnergyQuarticWellEval
};
const pullEnergy *const
pullEnergyQuarticWell = &_pullEnergyQuarticWell;

/* ----------------------------------------------------------------
** ------------------ tunable single heptic well ------------------
** ----------------------------------------------------------------
** 1 parm: well radius
*/
double
_pullEnergyHepticWellEval(double *denr, double x, const double *parm) {
  double a, b, c, d, e, f, g, w, enr;

  w = parm[0];
  a = (42*w)/(1 - 7*w);
  b = 15 + 36/(-1 + 7*w);
  c = -20 + 90/(1 - 7*w);
  d = (105*(1 + w))/(-1 + 7*w);
  e = -((42*(2 + w))/(-1 + 7*w));
  f = (7*(5 + w))/(-1 + 7*w);
  g = 6/(1 - 7*w);
  enr = 1 + x*(a + x*(b + x*(c + x*(d + x*(e + x*(f + g*x))))));
  *denr = a + x*(2*b + x*(3*c + x*(4*d + x*(5*e + x*(6*f + x*7*g)))));
  return enr;
}

double
_pullEnergyHepticWellWell(double *wx, const double *parm) {
  double t;

  *wx = parm[0];
  t = *wx - 1;
  return t*t*t*t*t*t*t/(7*(*wx) - 1);
}

const pullEnergy
_pullEnergyHepticWell = {
  HWELL,
  1,
  _pullEnergyHepticWellWell,
  _pullEnergyHepticWellEval
};
const pullEnergy *const
pullEnergyHepticWell = &_pullEnergyHepticWell;

/* ----------------------------------------------------------------
** ------------------------------- ZERO ---------------------------
** ----------------------------------------------------------------
** 0 parms:
*/
double
_pullEnergyZeroEval(double *denr, double dist, const double *parm) {

  AIR_UNUSED(dist);
  AIR_UNUSED(parm);
  *denr = 0;
  return 0;
}

const pullEnergy
_pullEnergyZero = {
  ZERO,
  0,
  _pullEnergyNoWell,
  _pullEnergyZeroEval
};
const pullEnergy *const
pullEnergyZero = &_pullEnergyZero;

/* ----------------------------------------------------------------
** ------------------------------- BPARAB -------------------------
** ----------------------------------------------------------------
** 3 parms, the first two are for butterworth,
** parm[2] is a shift (probably negative) on the parabola
*/
double
_pullEnergyBParabEval(double *denr, double x, const double *parm) {
  double ben, dben;

  ben = _pullEnergyButterworthEval(&dben, x, parm);
  *denr = 2*x*ben + x*x*dben;
  return (x*x + parm[2])*ben;
}

const pullEnergy
_pullEnergyButterworthParabola = {
  BPARAB,
  3,
  _pullEnergyNoWell,
  _pullEnergyBParabEval
};
const pullEnergy *const
pullEnergyButterworthParabola = &_pullEnergyButterworthParabola;

/* ----------------------------------------------------------------
** ----------------------------------------------------------------
** ----------------------------------------------------------------
*/

const pullEnergy *const pullEnergyAll[PULL_ENERGY_TYPE_MAX+1] = {
  &_pullEnergyUnknown,            /* 0 */
  &_pullEnergySpring,             /* 1 */
  &_pullEnergyGauss,              /* 2 */
  &_pullEnergyBspln,              /* 3 */
  &_pullEnergyButterworth,        /* 4 */
  &_pullEnergyCotan,              /* 5 */
  &_pullEnergyCubic,              /* 6 */
  &_pullEnergyQuartic,            /* 7 */
  &_pullEnergyCubicWell,          /* 8 */
  &_pullEnergyBetterCubicWell,    /* 9 */
  &_pullEnergyQuarticWell,        /* 10 */
  &_pullEnergyHepticWell,         /* 11 */
  &_pullEnergyZero,               /* 12 */
  &_pullEnergyButterworthParabola /* 13 */
};

pullEnergySpec *
pullEnergySpecNew() {
  pullEnergySpec *ensp;
  int pi;

  ensp = (pullEnergySpec *)calloc(1, sizeof(pullEnergySpec));
  if (ensp) {
    ensp->energy = pullEnergyUnknown;
    for (pi=0; pi<PULL_ENERGY_PARM_NUM; pi++) {
      ensp->parm[pi] = AIR_NAN;
    }
  }
  return ensp;
}

void
pullEnergySpecSet(pullEnergySpec *ensp, const pullEnergy *energy,
                  const double parm[PULL_ENERGY_PARM_NUM]) {
  unsigned int pi;

  if (ensp && energy && parm) {
    ensp->energy = energy;
    for (pi=0; pi<PULL_ENERGY_PARM_NUM; pi++) {
      ensp->parm[pi] = parm[pi];
    }
  }
  return;
}

void
pullEnergySpecCopy(pullEnergySpec *esDst, const pullEnergySpec *esSrc) {

  if (esDst && esSrc) {
    pullEnergySpecSet(esDst, esSrc->energy, esSrc->parm);
  }
  return;
}

pullEnergySpec *
pullEnergySpecNix(pullEnergySpec *ensp) {

  airFree(ensp);
  return NULL;
}

int
pullEnergySpecParse(pullEnergySpec *ensp, const char *_str) {
  static const char me[]="pullEnergySpecParse";
  char *str, *col, *_pstr, *pstr;
  int etype;
  unsigned int pi, haveParm;
  airArray *mop;
  double pval;

  if (!( ensp && _str )) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }

  /* see if its the name of something that needs no parameters */
  etype = airEnumVal(pullEnergyType, _str);
  if (pullEnergyTypeUnknown != etype) {
    /* the string is the name of some energy */
    ensp->energy = pullEnergyAll[etype];
    if (0 != ensp->energy->parmNum) {
      biffAddf(PULL, "%s: need %u parm%s for %s energy, but got none", me,
               ensp->energy->parmNum,
               (1 == ensp->energy->parmNum ? "" : "s"),
               ensp->energy->name);
      return 1;
    }
    /* the energy needs 0 parameters */
    for (pi=0; pi<PULL_ENERGY_PARM_NUM; pi++) {
      ensp->parm[pi] = AIR_NAN;
    }
    return 0;
  }

  /* start parsing parms after ':' */
  mop = airMopNew();
  str = airStrdup(_str);
  airMopAdd(mop, str, (airMopper)airFree, airMopAlways);
  col = strchr(str, ':');
  if (!col) {
    biffAddf(PULL, "%s: either \"%s\" is not a recognized energy, "
             "or it is an energy with parameters, and there's no "
             "\":\" separator to indicate parameters", me, str);
    airMopError(mop); return 1;
  }
  *col = '\0';
  etype = airEnumVal(pullEnergyType, str);
  if (pullEnergyTypeUnknown == etype) {
    biffAddf(PULL, "%s: didn't recognize \"%s\" as a %s", me,
             str, pullEnergyType->name);
    airMopError(mop); return 1;
  }

  ensp->energy = pullEnergyAll[etype];
  if (0 == ensp->energy->parmNum) {
    biffAddf(PULL, "%s: \"%s\" energy has no parms, but got something", me,
             ensp->energy->name);
    return 1;
  }

  _pstr = pstr = col+1;
  /* code lifted from teem/src/nrrd/kernel.c, should probably refactor. . . */
  for (haveParm=0; haveParm<ensp->energy->parmNum; haveParm++) {
    if (!pstr) {
      break;
    }
    if (1 != sscanf(pstr, "%lg", &pval)) {
      biffAddf(PULL, "%s: trouble parsing \"%s\" as double (in \"%s\")",
               me, _pstr, _str);
      airMopError(mop); return 1;
    }
    ensp->parm[haveParm] = pval;
    if ((pstr = strchr(pstr, ','))) {
      pstr++;
      if (!*pstr) {
        biffAddf(PULL, "%s: nothing after last comma in \"%s\" (in \"%s\")",
                 me, _pstr, _str);
        airMopError(mop); return 1;
      }
    }
  }
  /* haveParm is now the number of parameters that were parsed. */
  if (haveParm < ensp->energy->parmNum) {
    biffAddf(PULL, "%s: parsed only %u of %u required parms (for %s energy)"
             "from \"%s\" (in \"%s\")",
             me, haveParm, ensp->energy->parmNum,
             ensp->energy->name, _pstr, _str);
    airMopError(mop); return 1;
  } else {
    if (pstr) {
      biffAddf(PULL, "%s: \"%s\" (in \"%s\") has more than %u doubles",
               me, _pstr, _str, ensp->energy->parmNum);
      airMopError(mop); return 1;
    }
  }

  airMopOkay(mop);
  return 0;
}

int
_pullHestEnergyParse(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  static const char me[]="_pullHestForceParse";
  pullEnergySpec **enspP;
  char *perr;

  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  enspP = (pullEnergySpec **)ptr;
  *enspP = pullEnergySpecNew();
  if (pullEnergySpecParse(*enspP, str)) {
    perr = biffGetDone(PULL);
    airStrcpy(err, AIR_STRLEN_HUGE, perr);
    free(perr);
    return 1;
  }
  return 0;
}

hestCB
_pullHestEnergySpec = {
  sizeof(pullEnergySpec*),
  "energy specification",
  _pullHestEnergyParse,
  (airMopper)pullEnergySpecNix
};

hestCB *
pullHestEnergySpec = &_pullHestEnergySpec;
