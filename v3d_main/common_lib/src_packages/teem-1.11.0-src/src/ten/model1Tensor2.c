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

/* NOTE: this model is a single 2nd-order tensor, not a two-tensor model */

#define PARM_NUM 7
/* 1/sqrt(2) */
#define OST 0.70710678118654752440
static const tenModelParmDesc
parmDesc[] = {
  /* 0 */ {"B0", 0.0, TEN_MODEL_B0_MAX, AIR_FALSE, AIR_FALSE, 0},
  /* 1 */ {"Dxx", -TEN_MODEL_DIFF_MAX,     TEN_MODEL_DIFF_MAX,     AIR_FALSE, AIR_FALSE, 0},
  /* 2 */ {"Dxy", -TEN_MODEL_DIFF_MAX*OST, TEN_MODEL_DIFF_MAX*OST, AIR_FALSE, AIR_FALSE, 0},
  /* 3 */ {"Dxz", -TEN_MODEL_DIFF_MAX*OST, TEN_MODEL_DIFF_MAX*OST, AIR_FALSE, AIR_FALSE, 0},
  /* 4 */ {"Dyy", -TEN_MODEL_DIFF_MAX,     TEN_MODEL_DIFF_MAX,     AIR_FALSE, AIR_FALSE, 0},
  /* 5 */ {"Dyz", -TEN_MODEL_DIFF_MAX*OST, TEN_MODEL_DIFF_MAX*OST, AIR_FALSE, AIR_FALSE, 0},
  /* 6 */ {"Dzz", -TEN_MODEL_DIFF_MAX,     TEN_MODEL_DIFF_MAX,     AIR_FALSE, AIR_FALSE, 0}
};

static void
simulate(double *dwiSim, const double *parm, const tenExperSpec *espec) {
  unsigned int ii;
  double b0;

  b0 = parm[0];
  for (ii=0; ii<espec->imgNum; ii++) {
    double adc, bb;
    bb = espec->bval[ii];
    /* safe because TEN_T3V_CONTR never looks at parm[0] */
    adc = TEN_T3V_CONTR(parm, espec->grad + 3*ii);
    dwiSim[ii] = b0*exp(-bb*adc);
  }
  return;
}

static char *
parmSprint(char str[AIR_STRLEN_MED], const double *parm) {
  sprintf(str, "(%g) [%g %g %g;  %g %g;   %g]", parm[0],
          parm[1], parm[2], parm[3],
          parm[4], parm[5],
          parm[6]);
  return str;
}

_TEN_PARM_ALLOC
_TEN_PARM_RAND
_TEN_PARM_STEP
_TEN_PARM_DIST
_TEN_PARM_COPY

static int
parmConvert(double *parmDst, const double *parmSrc,
            const tenModel *modelSrc) {
  int ret;

  if (modelSrc == tenModelBall) {
    TEN_T_SET(parmDst, parmSrc[0],
              parmSrc[1], 0, 0,
              parmSrc[1], 0,
              parmSrc[1]);
    ret = 0;
  } else if (modelSrc == tenModel1Stick) {
    double ten[7];
    TEN_T3V_OUTER(ten, parmSrc + 2);
    TEN_T_SCALE(parmDst, parmSrc[1], ten);
    parmDst[0] = parmSrc[0];
    ret = 0;
  } else if (modelSrc == tenModelBall1Stick) {
    double stick[7], ball[7], diff, frac;
    diff = parmSrc[1];
    frac = parmSrc[2];
    TEN_T3V_OUTER(stick, parmSrc + 3);
    TEN_T_SCALE(stick, diff, stick);
    TEN_T_SET(ball, 1, diff, 0, 0, diff, 0, diff);
    TEN_T_LERP(parmDst, frac, ball, stick);
    parmDst[0] = parmSrc[0];
    ret = 1;
  } else if (modelSrc == tenModel1Cylinder) {
    double stick[7], ball[7], len, rad;
    len = parmSrc[1];
    rad = parmSrc[2];
    TEN_T3V_OUTER(stick, parmSrc + 3);
    TEN_T_SCALE(stick, len-rad, stick);
    TEN_T_SET(ball, 1, rad, 0, 0, rad, 0, rad);
    TEN_T_ADD(parmDst, ball, stick);
    parmDst[0] = parmSrc[0];
    ret = 0;
  } else if (modelSrc == tenModel1Tensor2) {
    parmCopy(parmDst, parmSrc);
    ret = 0;
  } else {
    unsigned int ii;
    for (ii=0; ii<PARM_NUM; ii++) {
      parmDst[ii] = AIR_NAN;
    }
    ret = 2;
  }
  return ret;
}

_TEN_SQE
_TEN_SQE_GRAD_CENTDIFF
_TEN_SQE_FIT(tenModel1Tensor2)

_TEN_NLL
_TEN_NLL_GRAD_STUB
_TEN_NLL_FIT_STUB

tenModel
_tenModel1Tensor2 = {
  TEN_MODEL_STR_1TENSOR2,
  _TEN_MODEL_FIELDS
};
const tenModel *const tenModel1Tensor2 = &_tenModel1Tensor2;
