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


#include "ell.h"

void
ell_4v_norm_f(float bv[4], const float av[4]) {
  float len;

  len = AIR_CAST(float, ELL_4V_LEN(av));
  ELL_4V_SCALE(bv, 1.0f/len, av);
  return;
}

#define PERP \
  idx = 0; \
  if (b[0]*b[0] < b[1]*b[1]) \
    idx = 1; \
  if (b[idx]*b[idx] < b[2]*b[2]) \
    idx = 2; \
  switch (idx) { \
  case 0: \
    ELL_3V_SET(a, b[1] - b[2], -b[0], b[0]); \
    break; \
  case 1: \
    ELL_3V_SET(a, -b[1], b[0] - b[2], b[1]); \
    break; \
  case 2: \
    ELL_3V_SET(a, -b[2], b[2], b[0] - b[1]); \
    break; \
  }

/*
******** ell_3v_perp_f()
**
** Given a 3-vector, produce one which is perpendicular.
** Output length won't be same as input length, but it will always
** be non-zero, if input length is non-zero.   This does NOT try to
** produce a unit-length vector, regardless of the length of the input.
*/
void
ell_3v_perp_f(float a[3], const float b[3]) {
  int idx;
  PERP;
}

/*
******** ell_3v_perp_d()
**
** same as above, but for doubles
*/
void
ell_3v_perp_d(double a[3], const double b[3]) {
  int idx;
  PERP;
}

void
ell_3mv_mul_f(float v2[3], const float m[9], const float v1[3]) {
  float tmp[3];

  ELL_3MV_MUL(tmp, m, v1);
  ELL_3V_COPY(v2, tmp);
}

void
ell_3mv_mul_d(double v2[3], const double m[9], const double v1[3]) {
  double tmp[3];

  ELL_3MV_MUL(tmp, m, v1);
  ELL_3V_COPY(v2, tmp);
}

void
ell_4mv_mul_f(float v2[4], const float m[16], const float v1[4]) {
  float tmp[4];

  ELL_4MV_MUL(tmp, m, v1);
  ELL_4V_COPY(v2, tmp);
}

void
ell_4mv_mul_d(double v2[4], const double m[16], const double v1[4]) {
  double tmp[4];

  ELL_4MV_MUL(tmp, m, v1);
  ELL_4V_COPY(v2, tmp);
}

/*
** hat tip to http://www.plunk.org/~hatch/rightway.php
*/
float
ell_3v_angle_f(const float _uu[3], const float _vv[3]) {
  float tmp[3], len, uu[3], vv[3], ret;

  ELL_3V_NORM_TT(uu, float, _uu, len);
  ELL_3V_NORM_TT(vv, float, _vv, len);
  if (ELL_3V_DOT(uu, vv) < 0.0) {
    ELL_3V_ADD2(tmp, uu, vv);
    ret = AIR_CAST(float, AIR_PI - 2*asin(ELL_3V_LEN(tmp)/2.0));
  } else {
    ELL_3V_SUB(tmp, uu, vv);
    ret = AIR_CAST(float, 2*asin(ELL_3V_LEN(tmp)/2.0));
  }
  return ret;
}

/* HEY: copy and paste */
double
ell_3v_angle_d(const double _uu[3], const double _vv[3]) {
  double tmp[3], len, uu[3], vv[3], ret;

  ELL_3V_NORM(uu, _uu, len);
  ELL_3V_NORM(vv, _vv, len);
  if (ELL_3V_DOT(uu, vv) < 0.0) {
    ELL_3V_ADD2(tmp, uu, vv);
    ret = AIR_PI - 2*asin(ELL_3V_LEN(tmp)/2.0);
  } else {
    ELL_3V_SUB(tmp, uu, vv);
    ret = 2*asin(ELL_3V_LEN(tmp)/2.0);
  }
  return ret;
}

/*
** input vectors have to be normalized!
*/
double
ell_3v_area_spherical_d(const double avec[3],
                        const double bvec[3],
                        const double cvec[3]) {
  double axb[3], bxc[3], cxa[3], A, B, C, tmp;

  ELL_3V_CROSS(axb, avec, bvec);
  ELL_3V_CROSS(bxc, bvec, cvec);
  ELL_3V_CROSS(cxa, cvec, avec);
  ELL_3V_NORM(axb, axb, tmp);
  ELL_3V_NORM(bxc, bxc, tmp);
  ELL_3V_NORM(cxa, cxa, tmp);
  A = AIR_PI - ell_3v_angle_d(axb, cxa);
  B = AIR_PI - ell_3v_angle_d(bxc, axb);
  C = AIR_PI - ell_3v_angle_d(cxa, bxc);
  return A + B + C - AIR_PI;
}

/*
** all input vectors {a,b,c}vec, dir must be normalized
*/
void
ell_3v_barycentric_spherical_d(double bary[3],
                               const double av[3],
                               const double bv[3],
                               const double cv[3],
                               const double vv[3]) {
  double sum;

  bary[0] = ell_3v_area_spherical_d(vv, bv, cv);
  bary[1] = ell_3v_area_spherical_d(vv, cv, av);
  bary[2] = ell_3v_area_spherical_d(vv, av, bv);
  sum = bary[0] + bary[1] + bary[2];
  if (sum) {
    ELL_3V_SCALE(bary, 1.0/sum, bary);
  }
  return;
}
