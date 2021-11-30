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

/* Implementation of two-dimensional tensors */

#include "tijk.h"
#include "privateTijk.h"

#include "convertQuietPush.h"

/* 2nd order 2D unsymmetric */

double
_tijk_2o2d_unsym_tsp_d (const double *A, const double *B) {
  return ELL_4V_DOT(A,B);
}

float
_tijk_2o2d_unsym_tsp_f (const float *A, const float *B) {
  return ELL_4V_DOT(A,B);
}

double
_tijk_2o2d_unsym_norm_d (const double *A) {
  return sqrt(ELL_4V_DOT(A,A));
}

float
_tijk_2o2d_unsym_norm_f (const float *A) {
  return sqrt(ELL_4V_DOT(A,A));
}

void
_tijk_2o2d_unsym_trans_d (double *res, const double *A, const double *M) {
  double _ma[4], _mt[4];
  ELL_2M_MUL(_ma, M, A);
  ELL_2M_TRANSPOSE(_mt, M);
  ELL_2M_MUL(res, _ma, _mt);
}

void
_tijk_2o2d_unsym_trans_f (float *res, const float *A, const float *M) {
  float _ma[4], _mt[4];
  ELL_2M_MUL(_ma, M, A);
  ELL_2M_TRANSPOSE(_mt, M);
  ELL_2M_MUL(res, _ma, _mt);
}

/* macro-based pseudo-template for type-generic code */
#define _TIJK_2O2D_UNSYM_CONVERT(TYPE, SUF)                             \
  int                                                                   \
  _tijk_2o2d_unsym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                  const TYPE *A) {                      \
    if (res_type==tijk_2o2d_unsym) { /* copy over */                    \
      ELL_4V_COPY(res, A);                                              \
      return 0;                                                         \
    } else if (NULL!=res_type->_convert_from_##SUF)                     \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_2o2d_unsym);   \
    else                                                                \
      return 1;                                                         \
  }

_TIJK_2O2D_UNSYM_CONVERT(double, d)
_TIJK_2O2D_UNSYM_CONVERT(float, f)

#define _TIJK_2O2D_UNSYM_APPROX(TYPE, SUF)                             \
  int                                                                  \
  _tijk_2o2d_unsym_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                                 const TYPE *A) {                      \
    if (res_type==tijk_2o2d_sym) {                                     \
      res[0]=A[0]; res[1]=0.5*(A[1]+A[2]); res[2]=A[3];                \
      return 0;                                                        \
    } else if (res_type==tijk_2o2d_asym) {                             \
      res[0]=0.5*(A[1]-A[2]);                                          \
      return 0;                                                        \
    } else if (NULL!=res_type->_approx_from_##SUF)                     \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_2o2d_unsym);   \
    else                                                               \
      return 1;                                                        \
  }

_TIJK_2O2D_UNSYM_APPROX(double, d)
_TIJK_2O2D_UNSYM_APPROX(float, f)

TIJK_TYPE_UNSYM(2o2d_unsym, 2, 2, 4)

/* 2nd order 2D symmetric */

unsigned int _tijk_2o2d_sym_mult[3] = {1, 2, 1};
int _tijk_2o2d_sym_unsym2uniq[4] = {1, 2, 2, 3};
int _tijk_2o2d_sym_uniq2unsym[4] = {1, 2, 3, 4};
unsigned int _tijk_2o2d_sym_uniq_idx[3] = {0, 1, 3};

#define _TIJK_2O2D_SYM_TSP(A, B)                \
  ((A)[0]*(B)[0]+2*(A)[1]*(B)[1]+(A)[2]*(B)[2])

double
_tijk_2o2d_sym_tsp_d (const double *A, const double *B) {
  return _TIJK_2O2D_SYM_TSP(A,B);
}

float
_tijk_2o2d_sym_tsp_f (const float *A, const float *B) {
  return _TIJK_2O2D_SYM_TSP(A,B);
}

double
_tijk_2o2d_sym_norm_d (const double *A) {
  return sqrt(_TIJK_2O2D_SYM_TSP(A,A));
}

float
_tijk_2o2d_sym_norm_f (const float *A) {
  return sqrt(_TIJK_2O2D_SYM_TSP(A,A));
}

void
_tijk_2o2d_sym_trans_d (double *res, const double *A, const double *M) {
  /* sym(M*unsym(A)*M^T) written out: */
  res[0]=M[0]*M[0]*A[0]+2*M[0]*M[1]*A[1]+M[1]*M[1]*A[2];
  res[1]=M[0]*M[2]*A[0]+(M[0]*M[3]+M[1]*M[2])*A[1]+M[1]*M[3]*A[2];
  res[2]=M[2]*M[2]*A[0]+2*M[2]*M[3]*A[1]+M[3]*M[3]*A[2];
}

void
_tijk_2o2d_sym_trans_f (float *res, const float *A, const float *M) {
  /* sym(M*unsym(A)*M^T) written out: */
  res[0]=M[0]*M[0]*A[0]+2*M[0]*M[1]*A[1]+M[1]*M[1]*A[2];
  res[1]=M[0]*M[2]*A[0]+(M[0]*M[3]+M[1]*M[2])*A[1]+M[1]*M[3]*A[2];
  res[2]=M[2]*M[2]*A[0]+2*M[2]*M[3]*A[1]+M[3]*M[3]*A[2];
}

#define _TIJK_2O2D_SYM_CONVERT(TYPE, SUF)                             \
  int                                                                 \
  _tijk_2o2d_sym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                const TYPE *A) {                      \
    if (res_type==tijk_2o2d_sym) { /* copy over */                    \
      ELL_3V_COPY(res, A);                                            \
      return 0;                                                       \
    } else if (res_type==tijk_2o2d_unsym) {                           \
      res[0]=A[0]; res[1]=res[2]=A[1]; res[3]=A[2];                   \
      return 0;                                                       \
    } else if (res_type==tijk_4o2d_sym) {                             \
      res[0]=A[0]; res[1]=res[3]=0.5*A[1];                            \
      res[2]=(A[0]+A[2])/6.0; res[4]=A[2];                            \
      return 0;                                                       \
    } else if (NULL!=res_type->_convert_from_##SUF)                   \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_2o2d_sym);   \
    else                                                              \
      return 1;                                                       \
  }

_TIJK_2O2D_SYM_CONVERT(double, d)
_TIJK_2O2D_SYM_CONVERT(float, f)

#define _TIJK_2O2D_SYM_APPROX(TYPE, SUF)                             \
  int                                                                \
  _tijk_2o2d_sym_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                               const TYPE *A) {                      \
    if (NULL!=res_type->_approx_from_##SUF)                          \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_2o2d_sym);   \
    else                                                             \
      return 1;                                                      \
  }

_TIJK_2O2D_SYM_APPROX(double, d)
_TIJK_2O2D_SYM_APPROX(float, f)

double
_tijk_2o2d_sym_s_form_d (const double *A, const double *v) {
  return A[0]*v[0]*v[0]+2*A[1]*v[0]*v[1]+A[2]*v[1]*v[1];
}

float
_tijk_2o2d_sym_s_form_f (const float *A, const float *v) {
  return A[0]*v[0]*v[0]+2*A[1]*v[0]*v[1]+A[2]*v[1]*v[1];
}

double
_tijk_2o2d_sym_mean_d (const double *A) {
  return 0.5*(A[0]+A[2]);
}

float
_tijk_2o2d_sym_mean_f (const float *A) {
  return 0.5*(A[0]+A[2]);
}

double
_tijk_2o2d_sym_var_d (const double *A) {
  return 0.125*(A[0]*A[0]+A[2]*A[2])-0.25*A[0]*A[2]+0.5*A[1]*A[1];
}

float
_tijk_2o2d_sym_var_f (const float *A) {
  return 0.125*(A[0]*A[0]+A[2]*A[2])-0.25*A[0]*A[2]+0.5*A[1]*A[1];
}

void
_tijk_2o2d_sym_v_form_d (double *res, const double *A, const double *v) {
  res[0]=A[0]*v[0]+A[1]*v[1];
  res[1]=A[1]*v[0]+A[2]*v[1];
}

void
_tijk_2o2d_sym_v_form_f (float *res, const float *A, const float *v) {
  res[0]=A[0]*v[0]+A[1]*v[1];
  res[1]=A[1]*v[0]+A[2]*v[1];
}

void
_tijk_2o2d_sym_m_form_d (double *res, const double *A, const double *v) {
  (void) v; /* v is only used in higher-order cases */
  res[0]=A[0]; res[1]=A[1]; res[2]=A[2];
}

void
_tijk_2o2d_sym_m_form_f (float *res, const float *A, const float *v) {
  (void) v; /* v is only used in higher-order cases */
  res[0]=A[0]; res[1]=A[1]; res[2]=A[2];
}

void
_tijk_2o2d_sym_make_rank1_d (double *res, const double s, const double *v) {
  res[0]=s*v[0]*v[0]; res[1]=s*v[0]*v[1]; res[2]=s*v[1]*v[1];
}

void
_tijk_2o2d_sym_make_rank1_f (float *res, const float s, const float *v) {
  res[0]=s*v[0]*v[0]; res[1]=s*v[0]*v[1]; res[2]=s*v[1]*v[1];
}

void
_tijk_2o2d_sym_make_iso_d (double *res, const double s) {
  res[0]=s; res[1]=0; res[2]=s;
}

void
_tijk_2o2d_sym_make_iso_f (float *res, const float s) {
  res[0]=s; res[1]=0; res[2]=s;
}

void
_tijk_2o2d_sym_grad_d (double *res, const double *A, const double *v) {
  double proj, projv[2];
  res[0]=2*(A[0]*v[0]+A[1]*v[1]);
  res[1]=2*(A[1]*v[0]+A[2]*v[1]);
  proj=ELL_2V_DOT(res,v);
  ELL_2V_SCALE(projv,-proj,v);
  ELL_2V_INCR(res,projv);
}

void
_tijk_2o2d_sym_grad_f (float *res, const float *A, const float *v) {
  float proj, projv[2];
  res[0]=2*(A[0]*v[0]+A[1]*v[1]);
  res[1]=2*(A[1]*v[0]+A[2]*v[1]);
  proj=ELL_2V_DOT(res,v);
  ELL_2V_SCALE(projv,-proj,v);
  ELL_2V_INCR(res,projv);
}

void
_tijk_2o2d_sym_hess_d (double *res, const double *A, const double *v) {
  double tang[2], s;
  ELL_2V_SET(tang,v[1],-v[0]);
  s=2*_tijk_2o2d_sym_s_form_d(A,tang)-2*_tijk_2o2d_sym_s_form_d(A, v);
  _tijk_2o2d_sym_make_rank1_d(res, s, tang);
}

void
_tijk_2o2d_sym_hess_f (float *res, const float *A, const float *v) {
  float tang[2], s;
  ELL_2V_SET(tang,v[1],-v[0]);
  s=2*_tijk_2o2d_sym_s_form_f(A,tang)-2*_tijk_2o2d_sym_s_form_f(A, v);
  _tijk_2o2d_sym_make_rank1_f(res, s, tang);
}

TIJK_TYPE_SYM(2o2d_sym, 2, 2, 3)

/* 2nd order 2D antisymmetric */

unsigned int _tijk_2o2d_asym_mult[1] = {2};
int _tijk_2o2d_asym_unsym2uniq[4] = {0, 1, -1, 0};
int _tijk_2o2d_asym_uniq2unsym[2] = {2, -3};
unsigned int _tijk_2o2d_asym_uniq_idx[1] = {0};

double
_tijk_2o2d_asym_tsp_d (const double *A, const double *B) {
  return 2*A[0]*B[0];
}

float
_tijk_2o2d_asym_tsp_f (const float *A, const float *B) {
  return 2*A[0]*B[0];
}

double
_tijk_2o2d_asym_norm_d (const double *A) {
  return sqrt(2*A[0]*A[0]);
}

float
_tijk_2o2d_asym_norm_f (const float *A) {
  return sqrt(2*A[0]*A[0]);
}

void
_tijk_2o2d_asym_trans_d (double *res, const double *A, const double *M) {
  /* if M is a rotation, this amounts to the identity */
  res[0]=A[0]*(M[0]*M[3]-M[1]*M[2]);
}

void
_tijk_2o2d_asym_trans_f (float *res, const float *A, const float *M) {
  res[0]=A[0]*(M[0]*M[3]-M[1]*M[2]);
}

#define _TIJK_2O2D_ASYM_CONVERT(TYPE, SUF)                             \
  int                                                                  \
  _tijk_2o2d_asym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                 const TYPE *A) {                      \
    if (res_type==tijk_2o2d_asym) { /* copy over */                    \
      res[0]=A[0];                                                     \
      return 0;                                                        \
    } else if (res_type==tijk_2o2d_unsym) {                            \
      res[0]=0; res[1]=A[0]; res[2]=-A[0]; res[3]=0;                   \
      return 0;                                                        \
    } else if (NULL!=res_type->_convert_from_##SUF)                    \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_2o2d_asym);   \
    else                                                               \
      return 1;                                                        \
  }

_TIJK_2O2D_ASYM_CONVERT(double, d)
_TIJK_2O2D_ASYM_CONVERT(float, f)

#define _TIJK_2O2D_ASYM_APPROX(TYPE, SUF)                             \
  int                                                                 \
  _tijk_2o2d_asym_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                                const TYPE *A) {                      \
    if (NULL!=res_type->_approx_from_##SUF)                           \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_2o2d_asym);   \
    else                                                              \
      return 1;                                                       \
  }

_TIJK_2O2D_ASYM_APPROX(double, d)
_TIJK_2O2D_ASYM_APPROX(float, f)

TIJK_TYPE(2o2d_asym, 2, 2, 1)

/* 3rd order 2D symmetric */
/* unsymmetric counterpart currently not implemented */

unsigned int _tijk_3o2d_sym_mult[4] = {1, 3, 3, 1};
#define _tijk_3o2d_sym_unsym2uniq NULL
#define _tijk_3o2d_sym_uniq2unsym NULL
#define _tijk_3o2d_sym_uniq_idx NULL

#define _TIJK_3O2D_SYM_TSP(A, B)                                \
  ((A)[0]*(B)[0]+3*(A)[1]*(B)[1]+3*(A)[2]*(B)[2]+(A)[3]*(B)[3])

double
_tijk_3o2d_sym_tsp_d (const double *A, const double *B) {
  return _TIJK_3O2D_SYM_TSP(A,B);
}

float
_tijk_3o2d_sym_tsp_f (const float *A, const float *B) {
  return _TIJK_3O2D_SYM_TSP(A,B);
}

double
_tijk_3o2d_sym_norm_d (const double *A) {
  return sqrt(_TIJK_3O2D_SYM_TSP(A,A));
}

float
_tijk_3o2d_sym_norm_f (const float *A) {
  return sqrt(_TIJK_3O2D_SYM_TSP(A,A));
}

#define _TIJK_3O2D_SYM_CONVERT(TYPE, SUF)                             \
  int                                                                 \
  _tijk_3o2d_sym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                const TYPE *A) {                      \
    if (res_type==tijk_3o2d_sym) { /* copy over */                    \
      ELL_4V_COPY(res, A);                                            \
      return 0;                                                       \
    } else if (NULL!=res_type->_convert_from_##SUF)                   \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_3o2d_sym);   \
    else                                                              \
      return 1;                                                       \
  }

_TIJK_3O2D_SYM_CONVERT(double, d)
_TIJK_3O2D_SYM_CONVERT(float, f)

#define _TIJK_3O2D_SYM_APPROX(TYPE, SUF)                             \
  int                                                                \
  _tijk_3o2d_sym_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                               const TYPE *A) {                      \
    if (NULL!=res_type->_approx_from_##SUF)                          \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_3o2d_sym);   \
    else                                                             \
      return 1;                                                      \
  }

_TIJK_3O2D_SYM_APPROX(double, d)
_TIJK_3O2D_SYM_APPROX(float, f)

void
_tijk_3o2d_sym_trans_d (double *res, const double *A, const double *M) {
  res[0]=M[0]*M[0]*M[0]*A[0]+3*M[0]*M[0]*M[1]*A[1]+
    3*M[0]*M[1]*M[1]*A[2]+M[1]*M[1]*M[1]*A[3];
  res[1]=M[0]*M[0]*M[2]*A[0]+(M[0]*M[0]*M[3]+2*M[0]*M[1]*M[2])*A[1]+
    (2*M[0]*M[1]*M[3]+M[1]*M[1]*M[2])*A[2]+M[1]*M[1]*M[3]*A[3];
  res[2]=M[0]*M[2]*M[2]*A[0]+(M[1]*M[2]*M[2]+2*M[0]*M[2]*M[3])*A[1]+
    (2*M[1]*M[2]*M[3]+M[0]*M[3]*M[3])*A[2]+M[1]*M[3]*M[3]*A[3];
  res[3]=M[2]*M[2]*M[2]*A[0]+3*M[2]*M[2]*M[3]*A[1]+
    3*M[2]*M[3]*M[3]*A[2]+M[3]*M[3]*M[3]*A[3];
}

void
_tijk_3o2d_sym_trans_f (float *res, const float *A, const float *M) {
  res[0]=M[0]*M[0]*M[0]*A[0]+3*M[0]*M[0]*M[1]*A[1]+
    3*M[0]*M[1]*M[1]*A[2]+M[1]*M[1]*M[1]*A[3];
  res[1]=M[0]*M[0]*M[2]*A[0]+(M[0]*M[0]*M[3]+2*M[0]*M[1]*M[2])*A[1]+
    (2*M[0]*M[1]*M[3]+M[1]*M[1]*M[2])*A[2]+M[1]*M[1]*M[3]*A[3];
  res[2]=M[0]*M[2]*M[2]*A[0]+(M[1]*M[2]*M[2]+2*M[0]*M[2]*M[3])*A[1]+
    (2*M[1]*M[2]*M[3]+M[0]*M[3]*M[3])*A[2]+M[1]*M[3]*M[3]*A[3];
  res[3]=M[2]*M[2]*M[2]*A[0]+3*M[2]*M[2]*M[3]*A[1]+
    3*M[2]*M[3]*M[3]*A[2]+M[3]*M[3]*M[3]*A[3];
}

double
_tijk_3o2d_sym_s_form_d (const double *A, const double *v) {
  return A[0]*v[0]*v[0]*v[0]+3*A[1]*v[0]*v[0]*v[1]+
    3*A[2]*v[0]*v[1]*v[1]+A[3]*v[1]*v[1]*v[1];
}

float
_tijk_3o2d_sym_s_form_f (const float *A, const float *v) {
  return A[0]*v[0]*v[0]*v[0]+3*A[1]*v[0]*v[0]*v[1]+
    3*A[2]*v[0]*v[1]*v[1]+A[3]*v[1]*v[1]*v[1];
}

double
_tijk_3o2d_sym_mean_d (const double *A) {
  (void) A; /* odd order; mean is zero irrespective of coefficients */
  return 0;
}

float
_tijk_3o2d_sym_mean_f (const float *A) {
  (void) A; /* odd order; mean is zero irrespective of coefficients */
  return 0;
}

double
_tijk_3o2d_sym_var_d (const double *A) {
  return (5*(A[0]*A[0]+A[3]*A[3])+9*(A[1]*A[1]+A[2]*A[2])+
          6*(A[0]*A[2]+A[1]*A[3]))/16.0;
}

float
_tijk_3o2d_sym_var_f (const float *A) {
  return (5*(A[0]*A[0]+A[3]*A[3])+9*(A[1]*A[1]+A[2]*A[2])+
          6*(A[0]*A[2]+A[1]*A[3]))/16.0;
}

void
_tijk_3o2d_sym_v_form_d (double *res, const double *A, const double *v) {
  double v00=v[0]*v[0], v01=v[0]*v[1], v11=v[1]*v[1];
  res[0]=A[0]*v00+2*A[1]*v01+A[2]*v11;
  res[1]=A[1]*v00+2*A[2]*v01+A[3]*v11;
}

void
_tijk_3o2d_sym_v_form_f (float *res, const float *A, const float *v) {
  float v00=v[0]*v[0], v01=v[0]*v[1], v11=v[1]*v[1];
  res[0]=A[0]*v00+2*A[1]*v01+A[2]*v11;
  res[1]=A[1]*v00+2*A[2]*v01+A[3]*v11;
}

void
_tijk_3o2d_sym_m_form_d (double *res, const double *A, const double *v) {
  res[0]=A[0]*v[0]+A[1]*v[1];
  res[1]=A[1]*v[0]+A[2]*v[1];
  res[2]=A[2]*v[0]+A[3]*v[1];
}

void
_tijk_3o2d_sym_m_form_f (float *res, const float *A, const float *v) {
  res[0]=A[0]*v[0]+A[1]*v[1];
  res[1]=A[1]*v[0]+A[2]*v[1];
  res[2]=A[2]*v[0]+A[3]*v[1];
}

void
_tijk_3o2d_sym_make_rank1_d (double *res, const double s, const double *v) {
  res[0]=s*v[0]*v[0]*v[0];
  res[1]=s*v[0]*v[0]*v[1];
  res[2]=s*v[0]*v[1]*v[1];
  res[3]=s*v[1]*v[1]*v[1];
}

void
_tijk_3o2d_sym_make_rank1_f (float *res, const float s, const float *v) {
  res[0]=s*v[0]*v[0]*v[0];
  res[1]=s*v[0]*v[0]*v[1];
  res[2]=s*v[0]*v[1]*v[1];
  res[3]=s*v[1]*v[1]*v[1];
}

#define _tijk_3o2d_sym_make_iso_d NULL
#define _tijk_3o2d_sym_make_iso_f NULL

void
_tijk_3o2d_sym_grad_d (double *res, const double *A, const double *v) {
  double proj, projv[2];
  _tijk_3o2d_sym_v_form_d (res, A, v);
  ELL_2V_SCALE(res,3.0,res);
  proj=ELL_2V_DOT(res,v);
  ELL_2V_SCALE(projv,-proj,v);
  ELL_2V_INCR(res,projv);
}

void
_tijk_3o2d_sym_grad_f (float *res, const float *A, const float *v) {
  float proj, projv[2];
  _tijk_3o2d_sym_v_form_f (res, A, v);
  ELL_2V_SCALE(res,3.0,res);
  proj=ELL_2V_DOT(res,v);
  ELL_2V_SCALE(projv,-proj,v);
  ELL_2V_INCR(res,projv);
}

void
_tijk_3o2d_sym_hess_d (double *res, const double *A, const double *v) {
  double tang[2];
  double hess[3], s;
  ELL_2V_SET(tang,v[1],-v[0]);
  _tijk_3o2d_sym_m_form_d(hess,A,v);
  s=6*_tijk_2o2d_sym_s_form_d(hess,tang)-3*_tijk_2o2d_sym_s_form_d(hess,v);
  _tijk_2o2d_sym_make_rank1_d(res, s, tang);
}

void
_tijk_3o2d_sym_hess_f (float *res, const float *A, const float *v) {
  float tang[2];
  float hess[3], s;
  ELL_2V_SET(tang,v[1],-v[0]);
  _tijk_3o2d_sym_m_form_f(hess,A,v);
  s=6*_tijk_2o2d_sym_s_form_f(hess,tang)-3*_tijk_2o2d_sym_s_form_f(hess,v);
  _tijk_2o2d_sym_make_rank1_f(res, s, tang);
}

TIJK_TYPE_SYM(3o2d_sym, 3, 2, 4)

/* 4th order 2D unsymmetric */

double
_tijk_4o2d_unsym_tsp_d (const double *A, const double *B) {
  return ELL_4V_DOT(A,B)+ELL_4V_DOT(A+4,B+4)+
    ELL_4V_DOT(A+8,B+8)+ELL_4V_DOT(A+12,B+12);
}

float
_tijk_4o2d_unsym_tsp_f (const float *A, const float *B) {
  return ELL_4V_DOT(A,B)+ELL_4V_DOT(A+4,B+4)+
    ELL_4V_DOT(A+8,B+8)+ELL_4V_DOT(A+12,B+12);
}

double
_tijk_4o2d_unsym_norm_d (const double *A) {
  return sqrt(ELL_4V_DOT(A,A)+ELL_4V_DOT(A+4,A+4)+
              ELL_4V_DOT(A+8,A+8)+ELL_4V_DOT(A+12,A+12));
}

float
_tijk_4o2d_unsym_norm_f (const float *A) {
  return sqrt(ELL_4V_DOT(A,A)+ELL_4V_DOT(A+4,A+4)+
              ELL_4V_DOT(A+8,A+8)+ELL_4V_DOT(A+12,A+12));
}

#define _TIJK_4O2D_UNSYM_TRANS(TYPE, SUF)                               \
  void                                                                  \
  _tijk_4o2d_unsym_trans_##SUF (TYPE *res, const TYPE *A, const TYPE *M) \
  { /* Tijkl = Mim Mjn Mko Mlp Tmnop                                    \
     * For efficiency, we transform mode by mode, right to left */      \
    TYPE tmp[16]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};                  \
    int i, init;                                                        \
    for (i=0; i<16; i+=2) { /* 4th mode */                              \
      tmp[i]  = M[0]*A[i] + M[1]*A[i+1];                                \
      tmp[i+1]= M[2]*A[i] + M[3]*A[i+1];                                \
    }                                                                   \
    /* using res as additional tmp space */                             \
    for (init=0; init<2; init++) { /* 3rd mode */                       \
      for (i=init; i<16; i+=4) {                                        \
        res[i]  = M[0]*tmp[i] + M[1]*tmp[i+2];                          \
        res[i+2]= M[2]*tmp[i] + M[3]*tmp[i+2];                          \
      }                                                                 \
    }                                                                   \
    for (init=0; init<4; init++) { /* 2nd mode */                       \
      for (i=init; i<16; i+=8) {                                        \
        tmp[i]  = M[0]*res[i] + M[1]*res[i+4];                          \
        tmp[i+4]= M[2]*res[i] + M[3]*res[i+4];                          \
      }                                                                 \
    }                                                                   \
    for (i=0; i<8; i++) { /* 1st mode */                                \
      res[i]  = M[0]*tmp[i] + M[1]*tmp[i+8];                            \
      res[i+8]= M[2]*tmp[i] + M[3]*tmp[i+8];                            \
    }                                                                   \
  }

_TIJK_4O2D_UNSYM_TRANS(double, d)
_TIJK_4O2D_UNSYM_TRANS(float, f)

#define _TIJK_4O2D_UNSYM_CONVERT(TYPE, SUF)                             \
  int                                                                   \
  _tijk_4o2d_unsym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                  const TYPE *A) {                      \
    if (res_type==tijk_4o2d_unsym) { /* copy over */                    \
      ELL_4V_COPY(res, A); ELL_4V_COPY(res+4, A+4);                     \
      ELL_4V_COPY(res+8, A+8); ELL_4V_COPY(res+12, A+12);               \
      return 0;                                                         \
    } else if (NULL!=res_type->_convert_from_##SUF)                     \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_4o2d_unsym);   \
    else                                                                \
      return 1;                                                         \
  }

_TIJK_4O2D_UNSYM_CONVERT(double, d)
_TIJK_4O2D_UNSYM_CONVERT(float, f)

#define _TIJK_4O2D_UNSYM_APPROX(TYPE, SUF)                             \
  int                                                                  \
  _tijk_4o2d_unsym_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                                 const TYPE *A) {                      \
    if (res_type==tijk_4o2d_sym) {                                     \
      res[0]=A[0]; res[1]=0.25*(A[1]+A[2]+A[4]+A[8]);                  \
      res[2]=(A[3]+A[5]+A[6]+A[9]+A[10]+A[12])/6.0;                    \
      res[3]=0.25*(A[7]+A[11]+A[13]+A[14]); res[4]=A[15];              \
      return 0;                                                        \
    } else if (NULL!=res_type->_approx_from_##SUF)                     \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_4o2d_unsym);   \
    else                                                               \
      return 1;                                                        \
  }

_TIJK_4O2D_UNSYM_APPROX(double, d)
_TIJK_4O2D_UNSYM_APPROX(float, f)

TIJK_TYPE_UNSYM(4o2d_unsym, 4, 2, 16)

/* 4th order 2D symmetric */

unsigned int _tijk_4o2d_sym_mult[5] = {1, 4, 6, 4, 1};
int _tijk_4o2d_sym_unsym2uniq[16] = {1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4,
                                     3, 4, 4, 5};
int _tijk_4o2d_sym_uniq2unsym[16] = {1, 2, 3, 5, 9, 4, 6, 7, 10, 11, 13,
                                     8, 12, 14, 15, 16};
unsigned int _tijk_4o2d_sym_uniq_idx[5] = {0, 1, 5, 11, 15};

#define _TIJK_4O2D_SYM_TSP(A, B)                                        \
  ((A)[0]*(B)[0]+4*(A)[1]*(B)[1]+6*(A)[2]*(B)[2]+4*(A)[3]*(B)[3]+(A)[4]*(B)[4])

double
_tijk_4o2d_sym_tsp_d (const double *A, const double *B) {
  return _TIJK_4O2D_SYM_TSP(A,B);
}

float
_tijk_4o2d_sym_tsp_f (const float *A, const float *B) {
  return _TIJK_4O2D_SYM_TSP(A,B);
}

double
_tijk_4o2d_sym_norm_d (const double *A) {
  return sqrt(_TIJK_4O2D_SYM_TSP(A,A));
}

float
_tijk_4o2d_sym_norm_f (const float *A) {
  return sqrt(_TIJK_4O2D_SYM_TSP(A,A));
}

#define _TIJK_4O2D_SYM_CONVERT(TYPE, SUF)                             \
  int                                                                 \
  _tijk_4o2d_sym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                const TYPE *A) {                      \
    if (res_type==tijk_4o2d_sym) { /* copy over */                    \
      ELL_4V_COPY(res, A); res[4]=A[4];                               \
      return 0;                                                       \
    } else if (res_type==tijk_4o2d_unsym) {                           \
      res[0]=A[0]; res[1]=res[2]=res[4]=res[8]=A[1];                  \
      res[3]=res[5]=res[6]=res[9]=res[10]=res[12]=A[2];               \
      res[7]=res[11]=res[13]=res[14]=A[3]; res[15]=A[4];              \
      return 0;                                                       \
    } else if (NULL!=res_type->_convert_from_##SUF)                   \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_4o2d_sym);   \
    else                                                              \
      return 1;                                                       \
  }

_TIJK_4O2D_SYM_CONVERT(double, d)
_TIJK_4O2D_SYM_CONVERT(float, f)

#define _TIJK_4O2D_SYM_APPROX(TYPE, SUF)                             \
  int                                                                \
  _tijk_4o2d_sym_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                               const TYPE *A) {                      \
    if (res_type==tijk_2o2d_sym) {                                   \
      res[0]=0.875*A[0]+0.75*A[2]-0.125*A[4];                        \
      res[1]=A[1]+A[3];                                              \
      res[2]=-0.125*A[0]+0.75*A[2]+0.875*A[4];                       \
      return 0;                                                      \
    } else if (NULL!=res_type->_approx_from_##SUF)                   \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_4o2d_sym);   \
    else                                                             \
      return 1;                                                      \
  }

_TIJK_4O2D_SYM_APPROX(double, d)
_TIJK_4O2D_SYM_APPROX(float, f)

void
_tijk_4o2d_sym_trans_d (double *res, const double *A, const double *M) {
  /* this code should be optimized at some point */
  double tmp[16], tmpout[16];
  _tijk_4o2d_sym_convert_d(tmp, tijk_4o2d_unsym, A);
  _tijk_4o2d_unsym_trans_d(tmpout, tmp, M);
  _tijk_4o2d_unsym_approx_d(res, tijk_4o2d_sym, tmpout);
}

void
_tijk_4o2d_sym_trans_f (float *res, const float *A, const float *M) {
  float tmp[16], tmpout[16];
  _tijk_4o2d_sym_convert_f(tmp, tijk_4o2d_unsym, A);
  _tijk_4o2d_unsym_trans_f(tmpout, tmp, M);
  _tijk_4o2d_unsym_approx_f(res, tijk_4o2d_sym, tmpout);
}

double
_tijk_4o2d_sym_s_form_d (const double *A, const double *v) {
  double v00=v[0]*v[0], v01=v[0]*v[1], v11=v[1]*v[1];
  return A[0]*v00*v00+4*A[1]*v00*v01+6*A[2]*v00*v11+
    4*A[3]*v01*v11+A[4]*v11*v11;
}

float
_tijk_4o2d_sym_s_form_f (const float *A, const float *v) {
  float v00=v[0]*v[0], v01=v[0]*v[1], v11=v[1]*v[1];
  return A[0]*v00*v00+4*A[1]*v00*v01+6*A[2]*v00*v11+
    4*A[3]*v01*v11+A[4]*v11*v11;
}

double
_tijk_4o2d_sym_mean_d (const double *A) {
  return 0.375*(A[0]+A[4])+0.75*A[2];
}

float
_tijk_4o2d_sym_mean_f (const float *A) {
  return 0.375*(A[0]+A[4])+0.75*A[2];
}

double
_tijk_4o2d_sym_var_d (const double *A) {
  return A[0]*(0.1328125*A[0]-0.09375*A[2]-0.234375*A[4]) +
    A[1]*(0.625*A[1]+0.75*A[3]) + 0.28125*A[2]*A[2] +
    0.625*A[3]*A[3] + A[4]*(0.1328125*A[4]-0.09375*A[2]);
}

float
_tijk_4o2d_sym_var_f (const float *A) {
  return A[0]*(0.1328125*A[0]-0.09375*A[2]-0.234375*A[4]) +
    A[1]*(0.625*A[1]+0.75*A[3]) + 0.28125*A[2]*A[2] +
    0.625*A[3]*A[3] + A[4]*(0.1328125*A[4]-0.09375*A[2]);
}

void
_tijk_4o2d_sym_v_form_d (double *res, const double *A, const double *v) {
  double v000=v[0]*v[0]*v[0], v001=v[0]*v[0]*v[1],
    v011=v[0]*v[1]*v[1], v111=v[1]*v[1]*v[1];
  res[0]=A[0]*v000+3*A[1]*v001+3*A[2]*v011+A[3]*v111;
  res[1]=A[1]*v000+3*A[2]*v001+3*A[3]*v011+A[4]*v111;
}

void
_tijk_4o2d_sym_v_form_f (float *res, const float *A, const float *v) {
  float v000=v[0]*v[0]*v[0], v001=v[0]*v[0]*v[1],
    v011=v[0]*v[1]*v[1], v111=v[1]*v[1]*v[1];
  res[0]=A[0]*v000+3*A[1]*v001+3*A[2]*v011+A[3]*v111;
  res[1]=A[1]*v000+3*A[2]*v001+3*A[3]*v011+A[4]*v111;
}

void
_tijk_4o2d_sym_m_form_d (double *res, const double *A, const double *v) {
  double v00=v[0]*v[0], v01=v[0]*v[1], v11=v[1]*v[1];
  res[0]=A[0]*v00+2*A[1]*v01+A[2]*v11;
  res[1]=A[1]*v00+2*A[2]*v01+A[3]*v11;
  res[2]=A[2]*v00+2*A[3]*v01+A[4]*v11;
}

void
_tijk_4o2d_sym_m_form_f (float *res, const float *A, const float *v) {
  float v00=v[0]*v[0], v01=v[0]*v[1], v11=v[1]*v[1];
  res[0]=A[0]*v00+2*A[1]*v01+A[2]*v11;
  res[1]=A[1]*v00+2*A[2]*v01+A[3]*v11;
  res[2]=A[2]*v00+2*A[3]*v01+A[4]*v11;
}

void
_tijk_4o2d_sym_make_rank1_d (double *res, const double s, const double *v) {
  double v00=v[0]*v[0], v01=v[0]*v[1], v11=v[1]*v[1];
  res[0]=s*v00*v00; res[1]=s*v00*v01; res[2]=s*v00*v11;
  res[3]=s*v01*v11; res[4]=s*v11*v11;
}

void
_tijk_4o2d_sym_make_rank1_f (float *res, const float s, const float *v) {
  float v00=v[0]*v[0], v01=v[0]*v[1], v11=v[1]*v[1];
  res[0]=s*v00*v00; res[1]=s*v00*v01; res[2]=s*v00*v11;
  res[3]=s*v01*v11; res[4]=s*v11*v11;
}

void
_tijk_4o2d_sym_make_iso_d (double *res, const double s) {
  res[0]=res[4]=s; res[2]=s/3.0; res[1]=res[3]=0;
}

void
_tijk_4o2d_sym_make_iso_f (float *res, const float s) {
  res[0]=res[4]=s; res[2]=s/3.0; res[1]=res[3]=0;
}

void
_tijk_4o2d_sym_grad_d (double *res, const double *A, const double *v) {
  double proj, projv[2];
  _tijk_4o2d_sym_v_form_d (res, A, v);
  ELL_2V_SCALE(res,4.0,res);
  proj=ELL_2V_DOT(res,v);
  ELL_2V_SCALE(projv,-proj,v);
  ELL_2V_INCR(res,projv);
}

void
_tijk_4o2d_sym_grad_f (float *res, const float *A, const float *v) {
  float proj, projv[2];
  _tijk_4o2d_sym_v_form_f (res, A, v);
  ELL_2V_SCALE(res,4.0,res);
  proj=ELL_2V_DOT(res,v);
  ELL_2V_SCALE(projv,-proj,v);
  ELL_2V_INCR(res,projv);
}

void
_tijk_4o2d_sym_hess_d (double *res, const double *A, const double *v) {
  double tang[2];
  double hess[3], s;
  ELL_2V_SET(tang,v[1],-v[0]);
  _tijk_4o2d_sym_m_form_d(hess,A,v);
  s=12*_tijk_2o2d_sym_s_form_d(hess,tang)-4*_tijk_2o2d_sym_s_form_d(hess,v);
  _tijk_2o2d_sym_make_rank1_d(res, s, tang);
}

void
_tijk_4o2d_sym_hess_f (float *res, const float *A, const float *v) {
  float tang[2];
  float hess[3], s;
  ELL_2V_SET(tang,v[1],-v[0]);
  _tijk_4o2d_sym_m_form_f(hess,A,v);
  s=12*_tijk_2o2d_sym_s_form_f(hess,tang)-4*_tijk_2o2d_sym_s_form_f(hess,v);
  _tijk_2o2d_sym_make_rank1_f(res, s, tang);
}

TIJK_TYPE_SYM(4o2d_sym, 4, 2, 5)

#include "convertQuietPop.h"
