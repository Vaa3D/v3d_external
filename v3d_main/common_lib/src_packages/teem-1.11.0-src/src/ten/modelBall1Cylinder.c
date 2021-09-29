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

#define PARM_NUM 8
static const tenModelParmDesc
parmDesc[] = {
  /* 0 */ {"B0", 0.0, TEN_MODEL_B0_MAX, AIR_FALSE, AIR_FALSE, 0},
  /* 1 */ {"diffusivity", 0.0, TEN_MODEL_DIFF_MAX, AIR_FALSE, AIR_FALSE, 0},
  /* 2 */ {"fraction", 0.0, 1.0, AIR_FALSE, AIR_FALSE, 0},
  /* 3 */ {"length", 0.0, TEN_MODEL_DIFF_MAX, AIR_FALSE, AIR_FALSE, 0},
  /* 4 */ {"radius", 0.0, TEN_MODEL_DIFF_MAX, AIR_FALSE, AIR_FALSE, 0},
  /* 5 */ {"x", -1.0, 1.0, AIR_FALSE, AIR_TRUE, 0},
  /* 6 */ {"y", -1.0, 1.0, AIR_FALSE, AIR_TRUE, 1},
  /* 7 */ {"z", -1.0, 1.0, AIR_FALSE, AIR_TRUE, 2}
};

static void
simulate(double *dwiSim, const double *parm, const tenExperSpec *espec) {
  unsigned int ii;
  double b0, diffBall, frac, length, radius, vec[3], ten[7],
    ident[7] = {1, 1, 0, 0, 1, 0, 1};

  b0 = parm[0];
  diffBall = parm[1];
  frac = parm[2];
  length = parm[3];
  radius = parm[4];
  vec[0] = parm[5];
  vec[1] = parm[6];
  vec[2] = parm[7];
  TEN_T3V_OUTER(ten, vec);
  TEN_T_SCALE_ADD2(ten, length - radius, ten, radius, ident);
  for (ii=0; ii<espec->imgNum; ii++) {
    double diffCyl, dwiBall, dwiCyl, bb;
    bb = espec->bval[ii];
    diffCyl = TEN_T3V_CONTR(ten, espec->grad + 3*ii);
    dwiCyl = exp(-bb*diffCyl);
    dwiBall = exp(-bb*diffBall);
    dwiSim[ii] = b0*AIR_LERP(frac, dwiBall, dwiCyl);
  }
  return;
}

static char *
parmSprint(char str[AIR_STRLEN_MED], const double *parm) {
  sprintf(str, "(%g) [(1-f) %g + (f=%g) %gX%g (%g,%g,%g)]", parm[0],
          parm[1], parm[2], parm[3], parm[4],
          parm[5], parm[6], parm[7]);
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
_TEN_SQE_FIT(tenModelBall1Cylinder)

_TEN_NLL
_TEN_NLL_GRAD_STUB
_TEN_NLL_FIT_STUB

tenModel
_tenModelBall1Cylinder = {
  TEN_MODEL_STR_BALL1CYLINDER,
  _TEN_MODEL_FIELDS
};
const tenModel *const tenModelBall1Cylinder = &_tenModelBall1Cylinder;
