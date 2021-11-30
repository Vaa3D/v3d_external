/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2011, 2010, 2009  University of Chicago
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


#include "../pull.h"

#define TOP_MARGIN 3

char *info = ("Sees if \"adhoc\" is inefficient.");

enum {
  stepStyleUnknown,        /* 0 */
  stepStyleSmall,          /* 1 */
  stepStyleDescent,        /* 2 */
  stepStyleRandomUniform,  /* 3 */
  stepStyleRandomCool,     /* 4 */
  stepStyleLast
};
#define STEP_STYLE_MAX        4

const char *
_stepStyleStr[STEP_STYLE_MAX+1] = {
  "(unknown_style)",
  "small",
  "descent",
  "runiform",
  "rcool"
};

const airEnum
_stepStyle = {
  "step style",
  STEP_STYLE_MAX,
  _stepStyleStr,  NULL,
  NULL,
  NULL, NULL,
  AIR_FALSE
};
const airEnum *const
stepStyle = &_stepStyle;


static double
tpimod(double phi) {

  if (phi > 0) {
    phi = fmod(phi, 2*AIR_PI);
  } else {
    phi = 2*AIR_PI + fmod(-phi, 2*AIR_PI);
  }
  return phi;
}

static double
enrPairwise(double *grad, double me, double she,
            pullEnergySpec *ensp, double radius) {
  double ad, td, rr, *parm, enr, denr,
    (*eval)(double *, double, const double parm[PULL_ENERGY_PARM_NUM]);

  ad = AIR_ABS(me - she);
  if (0 == ad) {
    if (grad) {
      *grad = 0;
    }
    return 0;
  }
  td = AIR_ABS(me - (she + 2*AIR_PI));
  if (td < ad) {
    she += 2*AIR_PI;
    ad = td;
  } else {
    td = AIR_ABS(me - (she - 2*AIR_PI));
    if (td < ad) {
      she -= 2*AIR_PI;
      ad = td;
    }
  }
  rr = ad/radius;
  if (rr > 1) {
    if (grad) {
      *grad = 0;
    }
    return 0;
  }
  parm = ensp->parm;
  eval = ensp->energy->eval;
  enr = eval(&denr, rr, parm);
  if (grad) {
    *grad = denr * airSgn(me - she)/radius;
  }
  return enr;
}

static double
enrSingle(double *gradP, double me, unsigned int meIdx,
          double *pos, unsigned int posNum,
          pullEnergySpec *ensp, double radius) {
  unsigned int ii;
  double enr, gg, grad;

  enr = 0;
  grad = 0;
  me = tpimod(me);
  for (ii=0; ii<posNum; ii++) {
    if (ii == meIdx) {
      continue;
    }
    enr += enrPairwise(&gg, me, pos[ii], ensp, radius);
    grad += gg;
  }
  if (gradP) {
    *gradP = grad;
  }
  return enr;
}

static double
enrStdv(double *pos, unsigned int posNum,
        pullEnergySpec *ensp, double radius) {
  unsigned int ii;
  double SS, S, enr;

  SS = S = 0.0;
  for (ii=0; ii<posNum; ii++) {
    enr = enrSingle(NULL, pos[ii], ii, pos, posNum, ensp, radius);
    S += enr;
    SS += enr*enr;
  }
  S /= posNum;
  SS /= posNum;
  return sqrt(SS - S*S);
}

static void
runIter(unsigned int iter,
        double *posOut, int sstyle, const double *posIn, double *step,
        unsigned int pntNum,
        pullEnergySpec *ensp, double radius,
        double cool, double backoff, double creepup) {
  unsigned int ii;
  double me, enr0, enr1, frc, tpos;
  int badstep;

  for (ii=0; ii<pntNum; ii++) {
    posOut[ii] = posIn[ii];
  }
  if (iter < TOP_MARGIN) {
    /* hack for better seeing initial locations */
    return;
  }
  for (ii=0; ii<pntNum; ii++) {
    me = posOut[ii];
    enr0 = enrSingle(&frc, me, ii, posOut, pntNum, ensp, radius);
    switch(sstyle) {
    case stepStyleSmall:
      tpos = me + AIR_AFFINE(0.0, airDrandMT(), 1.0, -radius/10, radius/10);
      enr1 = enrSingle(NULL, tpos, ii, posOut, pntNum, ensp, radius);
      if (enr1 < enr0) {
        me = tpos;
      }
      break;
    case stepStyleDescent:
      do {
        tpos = me - step[ii]*frc;
        enr1 = enrSingle(NULL, tpos, ii, posOut, pntNum, ensp, radius);
        badstep = enr1 > enr0;
        if (badstep) {
          step[ii] *= backoff;
        }
      } while (badstep);
      step[ii] *= creepup;
      me = tpos;
      break;
    case stepStyleRandomUniform:
      tpos = AIR_AFFINE(0.0, airDrandMT(), 1.0, 0.0, 2*AIR_PI);
      enr1 = enrSingle(NULL, tpos, ii, posOut, pntNum, ensp, radius);
      if (enr1 < enr0) {
        me = tpos;
      }
      break;
    case stepStyleRandomCool:
      airNormalRand(&tpos, NULL);
      tpos = me + tpos*cool;
      enr1 = enrSingle(NULL, tpos, ii, posOut, pntNum, ensp, radius);
      if (enr1 < enr0) {
        me = tpos;
      }
      break;
    }
    posOut[ii] = tpimod(me);
  }
  return;
}

int
main(int argc, const char *argv[]) {
  const char *me;
  hestOpt *hopt=NULL;
  airArray *mop;

  char *outS;
  pullEnergySpec *ensp, *enspCharge;
  unsigned int pntNum, pntIdx, iter, iterMax, rngSeed, posNum, coolHalfLife;
  double stdvMin, radius, *pos, *step, backoff, creepup, stepInitial;
  airArray *posArr;
  Nrrd *npos;
  int sstyle, verbose;

  mop = airMopNew();
  me = argv[0];
  hestOptAdd(&hopt, "v", "verbose", airTypeInt, 1, 1, &verbose, "0",
             "verbosity");
  hestOptAdd(&hopt, "energy", "spec", airTypeOther, 1, 1, &ensp, NULL,
             "specification of force function to use",
             NULL, NULL, pullHestEnergySpec);
  hestOptAdd(&hopt, "ss", "step style", airTypeEnum, 1, 1, &sstyle, NULL,
             "minimization step style", NULL, stepStyle);
  hestOptAdd(&hopt, "rad", "radius", airTypeDouble, 1, 1, &radius, NULL,
             "radius of particle");
  hestOptAdd(&hopt, "esm", "eng stdv min", airTypeDouble, 1, 1, &stdvMin,
             "0.05", "minimum stdv of particle energies");
  hestOptAdd(&hopt, "bo", "backoff", airTypeDouble, 1, 1, &backoff, "0.1",
             "backoff in gradient descent");
  hestOptAdd(&hopt, "step", "step", airTypeDouble, 1, 1, &stepInitial, "1.0",
             "initial step size in gradient descent");
  hestOptAdd(&hopt, "cu", "creepup", airTypeDouble, 1, 1, &creepup, "1.1",
             "creepup in gradient descent");
  hestOptAdd(&hopt, "chl", "cool half life", airTypeUInt, 1, 1, &coolHalfLife,
             "20", "cool half life");
  hestOptAdd(&hopt, "np", "# part", airTypeUInt, 1, 1, &pntNum, "42",
             "# of particles in simulation");
  hestOptAdd(&hopt, "maxi", "max iter", airTypeUInt, 1, 1, &iterMax, "1000",
             "max number of iterations");
  hestOptAdd(&hopt, "seed", "seed", airTypeUInt, 1, 1, &rngSeed, "42",
             "random number generator seed");
  hestOptAdd(&hopt, "o", "out", airTypeString, 1, 1, &outS, "-",
             "output filename");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
                 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  airSrandMT(rngSeed);
  posNum = 0;
  posArr = airArrayNew(AIR_CAST(void **, &pos),
                       &posNum, pntNum*sizeof(double), iterMax/10);
  airMopAdd(mop, posArr, (airMopper)airArrayNuke, airMopAlways);
  step = AIR_CALLOC(pntNum, double);
  airMopAdd(mop, step, airFree, airMopAlways);
  for (pntIdx=0; pntIdx<pntNum; pntIdx++) {
    step[pntIdx] = stepInitial;
  }

  enspCharge = pullEnergySpecNew();
  airMopAdd(mop, enspCharge, (airMopper)pullEnergySpecNix, airMopAlways);
  if (pullEnergySpecParse(enspCharge, "qwell:1")) {
    char *err;
    airMopAdd(mop, err = biffGetDone(PULL), airFree, airMopAlways);
    fprintf(stderr, "%s: problem saving output:\n%s", me, err);
    airMopError(mop); return 1;
  }

  for (iter=0; iter<iterMax; iter++) {
    double stdv, cool, fiter;
    if (!iter) {
      airArrayLenIncr(posArr, 1);
      for (pntIdx=0; pntIdx<pntNum; pntIdx++) {
        pos[pntIdx] = AIR_AFFINE(0.0, airDrandMT(), 1.0,
                                 0.0, 2*AIR_PI);
      }
    }
    fiter = AIR_MAX(0, ((double)iter)-TOP_MARGIN);
    cool = (AIR_PI/2)*pow(0.5, fiter/coolHalfLife);
    airArrayLenIncr(posArr, 1);
    runIter(iter, pos + pntNum*(iter+1), sstyle,
            pos + pntNum*iter, step, pntNum,
            ensp, radius, cool, backoff, creepup);
    stdv = enrStdv(pos + pntNum*(iter+1), pntNum, enspCharge, AIR_PI/2);
    if (verbose > 1) {
      fprintf(stderr, "%s: iter %u stdv %g cool %g\n", me, iter, stdv, cool);
    }
    if (stdv < stdvMin) {
      fprintf(stderr, "%s: converged in %u iters (stdv %g < %g)\n", me,
              iter, stdv, stdvMin);
      break;
    }
  }

  npos = nrrdNew();
  airMopAdd(mop, npos, (airMopper)nrrdNix, airMopAlways);
  if (nrrdWrap_va(npos, pos, nrrdTypeDouble, 2,
                  AIR_CAST(size_t, pntNum),
                  AIR_CAST(size_t, posNum))
      || nrrdSave(outS, npos, NULL)) {
    char *err;
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: problem saving output:\n%s", me, err);
    airMopError(mop); return 1;
  }


  airMopOkay(mop);
  return 0;
}
