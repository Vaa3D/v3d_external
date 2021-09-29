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

#define PARM_NUM 0
static const tenModelParmDesc
parmDesc[] = {
  /* dummy to avoid compiler error */
  {"dummy", 0.0, 0.0, AIR_FALSE, AIR_FALSE, 0},
};

static void
simulate(double *dwiSim, const double *parm, const tenExperSpec *espec) {
  unsigned int ii;

  AIR_UNUSED(parm);
  AIR_UNUSED(espec);
  for (ii=0; ii<espec->imgNum; ii++) {
    dwiSim[ii] = 0;
  }
  return;
}

static char *
parmSprint(char str[AIR_STRLEN_MED], const double *parm) {

  AIR_UNUSED(parm);
  sprintf(str, "constant 0");
  return str;
}

static double *
parmAlloc(void) {

  return NULL;
}

static void
parmRand(double *parm, airRandMTState *rng, int knownB0) {
  AIR_UNUSED(parm);
  AIR_UNUSED(rng);
  AIR_UNUSED(knownB0);
}

static void
parmStep(double *parm1, const double scl,
         const double *grad, const double *parm0) {
  AIR_UNUSED(parm1);
  AIR_UNUSED(scl);
  AIR_UNUSED(grad);
  AIR_UNUSED(parm0);
}

static double
parmDist(const double *parmA, const double *parmB) {
  AIR_UNUSED(parmA);
  AIR_UNUSED(parmB);
  return 0.0;
}

static void
parmCopy(double *parmA, const double *parmB) {
  AIR_UNUSED(parmA);
  AIR_UNUSED(parmB);
}

static int
parmConvert(double *parmDst, const double *parmSrc,
            const tenModel *modelSrc) {
  AIR_UNUSED(parmDst);
  AIR_UNUSED(parmSrc);
  AIR_UNUSED(modelSrc);
  return 0;
}

_TEN_SQE

static void
sqeGrad(double *grad, const double *parm0,
        const tenExperSpec *espec,
        double *dwiBuff, const double *dwiMeas,
        int knownB0) {
  AIR_UNUSED(grad);
  AIR_UNUSED(parm0);
  AIR_UNUSED(espec);
  AIR_UNUSED(dwiBuff);
  AIR_UNUSED(dwiMeas);
  AIR_UNUSED(knownB0);
}

_TEN_SQE_FIT(tenModelZero)

_TEN_NLL
_TEN_NLL_GRAD_STUB

static double
nllFit(double *parm, const tenExperSpec *espec,
       const double *dwiMeas, const double *parmInit,
       int rician, double sigma, int knownB0) {
  AIR_UNUSED(parm);
  AIR_UNUSED(espec);
  AIR_UNUSED(dwiMeas);
  AIR_UNUSED(parmInit);
  AIR_UNUSED(rician);
  AIR_UNUSED(sigma);
  AIR_UNUSED(knownB0);
  return AIR_NAN;
}

tenModel
_tenModelZero = {
  TEN_MODEL_STR_ZERO,
  _TEN_MODEL_FIELDS
};
const tenModel *const tenModelZero = &_tenModelZero;
