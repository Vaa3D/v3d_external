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

#include "coil.h"

/*
**  x ----> X
**   \   [0][0]  [1][0]  [2][0]
**  |  \   [0][1]  [1][1]  [2][1]
**  |    Y   [0][2]  [1][2]  [2][2]
**  |
**  |    [0][3]  [1][3]  [2][3]
**  Z      [0][4]  [1][4]  [2][4]
**           [0][5]  [1][5]  [2][5]
**
**       [0][6]  [1][6]  [2][6]
**         [0][7]  [1][7]  [2][7]
**           [0][8]  [1][8]  [2][8]
*/

coil_t
_coilLaplacian3(coil_t **iv3, double spacing[3]) {
  double ret;

  ret = (  (iv3[0][4] - 2*iv3[1][4] + iv3[2][4])/(spacing[0]*spacing[0])
         + (iv3[1][3] - 2*iv3[1][4] + iv3[1][5])/(spacing[1]*spacing[1])
         + (iv3[1][1] - 2*iv3[1][4] + iv3[1][7])/(spacing[2]*spacing[2]));
  return AIR_CAST(coil_t, ret);
}

void
_coilKindScalarFilterTesting(coil_t *delta,
                             int xi, int yi, int zi,
                             coil_t **iv3, double spacing[3],
                             double parm[COIL_PARMS_NUM]) {
  AIR_UNUSED(xi);
  AIR_UNUSED(yi);
  AIR_UNUSED(zi);
  AIR_UNUSED(iv3);
  AIR_UNUSED(spacing);
  AIR_UNUSED(parm);
  delta[0] = 0;
}

void
_coilKindScalarFilterHomogeneous(coil_t *delta,
                                 int xi, int yi, int zi,
                                 coil_t **iv3, double spacing[3],
                                 double parm[COIL_PARMS_NUM]) {

  AIR_UNUSED(xi);
  AIR_UNUSED(yi);
  AIR_UNUSED(zi);
  delta[0] = AIR_CAST(coil_t, parm[0])*_coilLaplacian3(iv3, spacing);
}

void
_coilKindScalar3x3x3Gradients(coil_t *forwX, coil_t *backX,
                              coil_t *forwY, coil_t *backY,
                              coil_t *forwZ, coil_t *backZ,
                              coil_t **i,
                              coil_t rspX, coil_t rspY, coil_t rspZ) {

  /* gradients at forward and backward X */
  forwX[0] = rspX*(i[2][4] - i[1][4]);
  forwX[1] = rspY*(i[1][5] + i[2][5] - i[1][3] - i[2][3])/2;
  forwX[2] = rspZ*(i[1][7] + i[2][7] - i[1][1] - i[2][1])/2;
  backX[0] = rspX*(i[1][4] - i[0][4]);
  backX[1] = rspY*(i[0][5] + i[1][5] - i[0][3] - i[1][3])/2;
  backX[2] = rspZ*(i[0][7] + i[1][7] - i[0][1] - i[1][1])/2;

  /* gradients at forward and backward Y */
  forwY[0] = rspX*(i[2][4] + i[2][5] - i[0][4] - i[0][5])/2;
  forwY[1] = rspY*(i[1][5] - i[1][4]);
  forwY[2] = rspZ*(i[1][7] + i[1][8] - i[1][1] - i[1][2])/2;
  backY[0] = rspX*(i[2][3] + i[2][4] - i[0][3] - i[0][4])/2;
  backY[1] = rspY*(i[1][4] - i[1][3]);
  backY[2] = rspZ*(i[1][6] + i[1][7] - i[1][0] - i[1][1])/2;

  /* gradients at forward and backward Z */
  forwZ[0] = rspX*(i[2][4] + i[2][7] - i[0][4] - i[0][7])/2;
  forwZ[1] = rspY*(i[1][5] + i[1][8] - i[1][3] - i[1][6])/2;
  forwZ[2] = rspZ*(i[1][7] - i[1][4]);
  backZ[0] = rspX*(i[2][1] + i[2][4] - i[0][1] - i[0][4])/2;
  backZ[1] = rspY*(i[1][2] + i[1][5] - i[1][0] - i[1][3])/2;
  backZ[2] = rspZ*(i[1][4] - i[1][1]);

  return;
}

#define _COIL_CONDUCT(LL, KK) \
  AIR_CAST(coil_t, exp(-0.5*(LL)/(KK)))

/*
#define _COIL_CONDUCT(vec, KK) \
  AIR_CAST(coil_t, 1.0/(1.0 + (LL)/(KK)))
*/

void
_coilKindScalarFilterPeronaMalik(coil_t *delta,
                                 int xi, int yi, int zi,
                                 coil_t **iv3, double spacing[3],
                                 double parm[COIL_PARMS_NUM]) {
  coil_t forwX[3], backX[3], forwY[3], backY[3], forwZ[3], backZ[3],
    KK, rspX, rspY, rspZ;

  AIR_UNUSED(xi);
  AIR_UNUSED(yi);
  AIR_UNUSED(zi);

  /* reciprocals of spacings in X, Y, and Z */
  rspX = AIR_CAST(coil_t, 1.0/spacing[0]);
  rspY = AIR_CAST(coil_t, 1.0/spacing[1]);
  rspZ = AIR_CAST(coil_t, 1.0/spacing[2]);

  _coilKindScalar3x3x3Gradients(forwX, backX,
                                forwY, backY,
                                forwZ, backZ,
                                iv3,
                                rspX, rspY, rspZ);

  /* compute fluxes */
  KK = AIR_CAST(coil_t, parm[1]*parm[1]);
  forwX[0] *= _COIL_CONDUCT(ELL_3V_DOT(forwX, forwX), KK);
  forwY[1] *= _COIL_CONDUCT(ELL_3V_DOT(forwY, forwY), KK);
  forwZ[2] *= _COIL_CONDUCT(ELL_3V_DOT(forwZ, forwZ), KK);
  backX[0] *= _COIL_CONDUCT(ELL_3V_DOT(backX, backX), KK);
  backY[1] *= _COIL_CONDUCT(ELL_3V_DOT(backY, backY), KK);
  backZ[2] *= _COIL_CONDUCT(ELL_3V_DOT(backZ, backZ), KK);

  delta[0] = AIR_CAST(coil_t, parm[0])*(rspX*(forwX[0] - backX[0])
                                        + rspY*(forwY[1] - backY[1])
                                        + rspZ*(forwZ[2] - backZ[2]));
}

/*
** (mcde)
** parm vector:
**   0    1    2   (3)
** step   K  lerp (lerp=1: all laplacian)
*/
void
_coilKindScalarFilterModifiedCurvature(coil_t *delta,
                                       int xi, int yi, int zi,
                                       coil_t **iv3, double spacing[3],
                                       double parm[COIL_PARMS_NUM]) {
  /* char me[]="_coilKindScalarFilterModifiedCurvature"; */
  coil_t forwX[3], backX[3], forwY[3], backY[3], forwZ[3], backZ[3],
    grad[3], gm, eps, KK, LL, denom, rspX, rspY, rspZ, lerp;

  AIR_UNUSED(xi);
  AIR_UNUSED(yi);
  AIR_UNUSED(zi);

  /*
  if (coilVerbose) {
    fprintf(stderr, "!%s: --------- hello --------\n", me);
  }
  */
  /* reciprocals of spacings in X, Y, and Z */
  rspX = AIR_CAST(coil_t, 1.0/spacing[0]);
  rspY = AIR_CAST(coil_t, 1.0/spacing[1]);
  rspZ = AIR_CAST(coil_t, 1.0/spacing[2]);

  _coilKindScalar3x3x3Gradients(forwX, backX,
                                forwY, backY,
                                forwZ, backZ,
                                iv3,
                                rspX, rspY, rspZ);
  grad[0] = rspX*(iv3[2][4] - iv3[0][4]);
  grad[1] = rspY*(iv3[1][5] - iv3[1][3]);
  grad[2] = rspZ*(iv3[1][7] - iv3[1][1]);
  gm = AIR_CAST(coil_t, ELL_3V_LEN(grad));
  /*
  if (coilVerbose) {
    fprintf(stderr, "forwX = %g %g %g    backX = %g %g %g\n",
            forwX[0], forwX[1], forwX[2],
            backX[0], backX[1], backX[2]);
    fprintf(stderr, "forwY = %g %g %g    backY = %g %g %g\n",
            forwY[0], forwY[1], forwY[2],
            backY[0], backY[1], backY[2]);
    fprintf(stderr, "forwZ = %g %g %g    backZ = %g %g %g\n",
            forwZ[0], forwZ[1], forwZ[2],
            backZ[0], backZ[1], backZ[2]);
    fprintf(stderr, "grad = %g %g %g --> gm = %g\n",
            grad[0], grad[1], grad[2], gm);
  }
  */
  /* compute fluxes */
  eps = 0.0000000001f;
  KK = AIR_CAST(coil_t, parm[1]*parm[1]);
  LL = ELL_3V_DOT(forwX, forwX);
  denom = AIR_CAST(coil_t, 1.0/(eps + sqrt(LL)));
  forwX[0] *= _COIL_CONDUCT(LL, KK)*denom;
  LL = ELL_3V_DOT(forwY, forwY);
  denom = AIR_CAST(coil_t, 1.0/(eps + sqrt(LL)));
  forwY[1] *= _COIL_CONDUCT(LL, KK)*denom;
  LL = ELL_3V_DOT(forwZ, forwZ);
  denom = AIR_CAST(coil_t, 1.0/(eps + sqrt(LL)));
  forwZ[2] *= _COIL_CONDUCT(LL, KK)*denom;
  LL = ELL_3V_DOT(backX, backX);
  denom = AIR_CAST(coil_t, 1.0/(eps + sqrt(LL)));
  backX[0] *= _COIL_CONDUCT(LL, KK)*denom;
  LL = ELL_3V_DOT(backY, backY);
  denom = AIR_CAST(coil_t, 1.0/(eps + sqrt(LL)));
  backY[1] *= _COIL_CONDUCT(LL, KK)*denom;
  LL = ELL_3V_DOT(backZ, backZ);
  denom = AIR_CAST(coil_t, 1.0/(eps + sqrt(LL)));
  backZ[2] *= _COIL_CONDUCT(LL, KK)*denom;

  lerp = AIR_CAST(coil_t, parm[2]);
  delta[0] = (lerp*_coilLaplacian3(iv3, spacing)
              + (1-lerp)*gm*(rspX*(forwX[0] - backX[0])
                             + rspY*(forwY[1] - backY[1])
                             + rspZ*(forwZ[2] - backZ[2])));
  delta[0] *= AIR_CAST(coil_t, parm[0]);
  /*
  if (coilVerbose) {
    fprintf(stderr, "!%s: delta = %g\n", me, delta[0]);
  }
  */
}

/*
** parm vector:
**   0      1      2     3       4      5      (6)
** step  K_perp  K_tan  lerp  X_ring  Y_ring
*/
void
_coilKindScalarFilterModifiedCurvatureRings(coil_t *delta,
                                            int xi, int yi, int zi,
                                            coil_t **iv3, double spacing[3],
                                            double parm[COIL_PARMS_NUM]) {
  coil_t forwX[3], backX[3], forwY[3], backY[3], forwZ[3], backZ[3],
    grad[3], gm, eps, KK, LL, denom, rspX, rspY, rspZ, lerp;
  double bas0[3], bas1[3], bas2[3], len, norm[3], sk;

  AIR_UNUSED(zi);

  ELL_3V_SET(bas0, 0, 0, 1);
  ELL_3V_SET(bas1, xi - parm[4], yi - parm[5], 0);
  ELL_3V_NORM(bas1, bas1, len);
  ELL_3V_CROSS(bas2, bas0, bas1);

  rspX = AIR_CAST(coil_t, 1.0/spacing[0]);
  rspY = AIR_CAST(coil_t, 1.0/spacing[1]);
  rspZ = AIR_CAST(coil_t, 1.0/spacing[2]);

  _coilKindScalar3x3x3Gradients(forwX, backX,
                                forwY, backY,
                                forwZ, backZ,
                                iv3,
                                rspX, rspY, rspZ);
  grad[0] = rspX*(iv3[2][4] - iv3[0][4]);
  grad[1] = rspY*(iv3[1][5] - iv3[1][3]);
  grad[2] = rspZ*(iv3[1][7] - iv3[1][1]);
  gm = AIR_CAST(coil_t, ELL_3V_LEN(grad));

  if (gm) {
    double tc, rcsq;
    ELL_3V_SCALE(norm, 1.0/gm, grad);
    tc = ELL_3V_DOT(norm, bas2);
    rcsq = 1 - tc*tc;
    sk = AIR_LERP(rcsq, parm[1], parm[2]);
  } else {
    sk = parm[1];
  }

  /* compute fluxes */
  eps = 0.0000000001f;
  KK = AIR_CAST(coil_t, sk*sk);
  LL = ELL_3V_DOT(forwX, forwX);
  denom = AIR_CAST(coil_t, 1.0/(eps + sqrt(LL)));
  forwX[0] *= _COIL_CONDUCT(LL, KK)*denom;
  LL = ELL_3V_DOT(forwY, forwY);
  denom = AIR_CAST(coil_t, 1.0/(eps + sqrt(LL)));
  forwY[1] *= _COIL_CONDUCT(LL, KK)*denom;
  LL = ELL_3V_DOT(forwZ, forwZ);
  denom = AIR_CAST(coil_t, 1.0/(eps + sqrt(LL)));
  forwZ[2] *= _COIL_CONDUCT(LL, KK)*denom;
  LL = ELL_3V_DOT(backX, backX);
  denom = AIR_CAST(coil_t, 1.0/(eps + sqrt(LL)));
  backX[0] *= _COIL_CONDUCT(LL, KK)*denom;
  LL = ELL_3V_DOT(backY, backY);
  denom = AIR_CAST(coil_t, 1.0/(eps + sqrt(LL)));
  backY[1] *= _COIL_CONDUCT(LL, KK)*denom;
  LL = ELL_3V_DOT(backZ, backZ);
  denom = AIR_CAST(coil_t, 1.0/(eps + sqrt(LL)));
  backZ[2] *= _COIL_CONDUCT(LL, KK)*denom;

  lerp = AIR_CAST(coil_t, parm[2]);
  delta[0] = (lerp*_coilLaplacian3(iv3, spacing)
              + (1-lerp)*gm*(rspX*(forwX[0] - backX[0])
                             + rspY*(forwY[1] - backY[1])
                             + rspZ*(forwZ[2] - backZ[2])));
  delta[0] *= AIR_CAST(coil_t, parm[0]);
}

void
_coilKindScalarUpdate(coil_t *val, coil_t *delta) {

  val[0] += delta[0];
}

const coilKind
_coilKindScalar = {
  "scalar",
  1,
  {NULL,
   _coilKindScalarFilterTesting,
   _coilKindScalarFilterHomogeneous,
   _coilKindScalarFilterPeronaMalik,
   _coilKindScalarFilterModifiedCurvature,
   _coilKindScalarFilterModifiedCurvatureRings,
   NULL,
   NULL,
   NULL},
  _coilKindScalarUpdate
};

const coilKind *
coilKindScalar = &_coilKindScalar;

/* ------------------------------------------ */

#ifdef __cplusplus
extern "C" {
#endif
extern const coilKind _coilKind7Tensor;
#ifdef __cplusplus
}
#endif

const coilKind*
coilKindArray[COIL_KIND_TYPE_MAX+1] = {
  NULL,
  &_coilKindScalar,
  NULL,
  &_coilKind7Tensor
};
