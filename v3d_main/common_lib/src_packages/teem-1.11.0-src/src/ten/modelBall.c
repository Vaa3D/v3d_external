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

#define PARM_NUM 2
static const tenModelParmDesc
parmDesc[] = {
  /* 0 */ {"B0", 0.0, TEN_MODEL_B0_MAX, AIR_FALSE, AIR_FALSE, 0},
  /* 1 */ {"diffusivity", 0.0, TEN_MODEL_DIFF_MAX, AIR_FALSE, AIR_FALSE, 0}
};

static void
simulate(double *dwiSim, const double *parm, const tenExperSpec *espec) {
  unsigned int ii;
  double b0, diff;

  b0 = parm[0];
  diff = parm[1];
  for (ii=0; ii<espec->imgNum; ii++) {
    dwiSim[ii] = b0*exp(-espec->bval[ii]*diff);
  }
  return;
}

static char *
parmSprint(char str[AIR_STRLEN_MED], const double *parm) {
  sprintf(str, "(%g) %g", parm[0], parm[1]);
  return str;
}

_TEN_PARM_ALLOC
_TEN_PARM_RAND
_TEN_PARM_STEP
_TEN_PARM_DIST
_TEN_PARM_COPY
_TEN_PARM_CONVERT_NOOP

_TEN_SQE
_TEN_SQE_GRAD_CENTDIFF
_TEN_SQE_FIT(tenModelBall)

_TEN_NLL
_TEN_NLL_GRAD_STUB
_TEN_NLL_FIT_STUB

tenModel
_tenModelBall = {
  /*
  TEN_MODEL_STR_BALL,
  _TEN_MODEL_FIELDS
  */
  "ball",
  2,
  parmDesc,
  simulate,
  parmSprint,
  parmAlloc,
  parmRand,
  parmStep,
  parmDist,
  parmCopy,
  parmConvert,
  sqe,
  sqeGrad,
  sqeFit,
  nll,
  nllGrad,
  nllFit
};
const tenModel *const tenModelBall = &_tenModelBall;
