/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2010, 2009, 2008 Thomas Schultz
  Copyright (C) 2010, 2009, 2008 Gordon Kindlmann

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

/* Implementation of fourier series and their relation to tensors */

#include "tijk.h"

#include "convertQuietPush.h"

const unsigned int tijk_max_efs_order=4;
/* for order 4, the maximum number of coefficients is 5 */
#define _TIJK_MAX_EFS_LEN 5

#define TIJK_EVAL_EFS_BASIS(TYPE, SUF)                        \
  unsigned int                                                         \
  tijk_eval_efs_basis_##SUF(TYPE *res, unsigned int order, TYPE phi) { \
    res[0]=0.5;                                               \
    if (order<2)                                              \
      return 0;                                               \
    res[1]=cos(phi);                                          \
    res[2]=sin(phi);                                          \
    if (order<4)                                              \
      return 2;                                               \
    res[3]=cos(2*phi);                                        \
    res[4]=sin(2*phi);                                        \
    return 4; /* higher orders not implemented. */            \
  }

TIJK_EVAL_EFS_BASIS(double, d)
TIJK_EVAL_EFS_BASIS(float, f)

#define TIJK_EVAL_EFS(TYPE, SUF)                             \
  TYPE                                                       \
  tijk_eval_efs_##SUF(TYPE *coeffs, unsigned int order, TYPE phi) {   \
    TYPE basis[_TIJK_MAX_EFS_LEN];                                    \
    TYPE res=0.0;                                            \
    unsigned int i;                                          \
    if (order!=tijk_eval_efs_basis_##SUF(basis, order, phi)) \
      return 0; /* there has been an error. */               \
    for (i=0; i<order+1; i++)                                \
      res+=basis[i]*coeffs[i];                               \
    return res;                                              \
  }

TIJK_EVAL_EFS(double, d)
TIJK_EVAL_EFS(float, f)

/* conversion matrices computed in pylab, stored in double precision */
static const double _tijk_sym2efs_o2[3*3] = {
  1.0, 0.0, 1.0,
  0.5, 0.0, -0.5,
  0.0, 1.0, 0.0
};

static const double _tijk_efs2sym_o2[3*3] = {
  0.5, 1.0, 0.0,
  0.0, 0.0, 1.0,
  0.5, -1.0, 0.0
};

static const double _tijk_sym2efs_o4[5*5] = {
  0.75, 0.0, 1.5, 0.0, 0.75,
  0.5, 0.0, 0.0, 0.0, -0.5,
  0.0, 1.0, 0.0, 1.0, 0.0,
  0.125, 0.0, -0.75, 0.0, 0.125,
  0.0, 0.5, 0.0, -0.5, 0.0
};

static const double _tijk_efs2sym_o4[5*5] = {
  0.5, 1.0, 0.0, 1.0, 0.0,
  0.0, 0.0, 0.5, 0.0, 1.0,
  0.16666666666666666667, 0.0, 0.0, -1.0, 0.0,
  0.0, 0.0, 0.5, 0.0, -1.0,
  0.5, -1.0, 0.0, 1.0, 0.0
};

#define TIJK_2D_SYM_TO_EFS(TYPE, SUF)                    \
  int                                                    \
  tijk_2d_sym_to_efs_##SUF(TYPE *res, const TYPE *ten,   \
                           const tijk_type *type) {      \
    const double *m; /* conversion matrix */             \
    unsigned int i, j, n;                                \
    n=type->num;                                         \
    if (type==tijk_2o2d_sym) {                           \
      m=_tijk_sym2efs_o2;                                \
    } else if (type==tijk_4o2d_sym) {                    \
      m=_tijk_sym2efs_o4;                                \
    } else {                                             \
      return -1; /* cannot do conversion */              \
    }                                                    \
    for (i=0; i<n; i++) {                                \
      res[i]=0;                                          \
      for (j=0; j<n; j++) {                              \
        res[i]+=m[n*i+j]*ten[j];                         \
      }                                                  \
    }                                                    \
    return type->order;                                  \
  }

TIJK_2D_SYM_TO_EFS(double, d)
TIJK_2D_SYM_TO_EFS(float, f)

#define TIJK_EFS_TO_2D_SYM(TYPE, SUF)                              \
  const tijk_type*                                                 \
  tijk_efs_to_2d_sym_##SUF(TYPE *res, const TYPE *fs, unsigned int order) { \
    const double *m; /* conversion matrix */                       \
    const tijk_type *type;                                         \
    unsigned int i, j, n;                                          \
    switch (order) {                                               \
    case 2:                                                        \
      m=_tijk_efs2sym_o2;                                          \
      type=tijk_2o2d_sym;                                          \
      break;                                                       \
    case 4:                                                        \
      m=_tijk_efs2sym_o4;                                          \
      type=tijk_4o2d_sym;                                          \
      break;                                                       \
    default:                                                       \
      return NULL; /* cannot do the conversion */                  \
    }                                                              \
    n=order+1;                                                     \
    for (i=0; i<n; i++) {                                          \
      res[i]=0;                                                    \
      for (j=0; j<n; j++) {                                        \
        res[i]+=m[n*i+j]*fs[j];                                    \
      }                                                            \
    }                                                              \
    return type;                                                   \
  }

TIJK_EFS_TO_2D_SYM(double, d)
TIJK_EFS_TO_2D_SYM(float, f)

#include "convertQuietPop.h"
