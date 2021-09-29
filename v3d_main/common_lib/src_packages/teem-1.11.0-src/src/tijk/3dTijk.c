/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2011, 2010, 2009, 2008 Thomas Schultz
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

/* Implementation of three-dimensional tensors */

#include "tijk.h"
#include "privateTijk.h"

#include "convertQuietPush.h"

/* 1st order 3D - a simple vector */

/* (un)symmetric doesn't really mean anything in this case */
unsigned int _tijk_1o3d_mult[3]={1,1,1};
int _tijk_1o3d_unsym2uniq[3] = {1,2,3};
int _tijk_1o3d_uniq2unsym[3] = {1,2,3};
unsigned int _tijk_1o3d_uniq_idx[3] = {0,1,2};

double
_tijk_1o3d_tsp_d (const double *A, const double *B) {
  return ELL_3V_DOT(A,B);
}

float
_tijk_1o3d_tsp_f (const float *A, const float *B) {
  return ELL_3V_DOT(A,B);
}

double
_tijk_1o3d_norm_d (const double *A) {
  return sqrt(ELL_3V_DOT(A,A));
}

float
_tijk_1o3d_norm_f (const float *A) {
  return sqrt(ELL_3V_DOT(A,A));
}

void
_tijk_1o3d_trans_d (double *res, const double *A, const double *M) {
  ell_3mv_mul_d(res,M,A);
}

void
_tijk_1o3d_trans_f (float *res, const float *A, const float *M) {
  ell_3mv_mul_f(res,M,A);
}

/* macro-based pseudo-template for type-generic code */
#define _TIJK_1O3D_CONVERT(TYPE, SUF)                             \
  int                                                             \
  _tijk_1o3d_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                            const TYPE *A) {                      \
    if (res_type==tijk_1o3d) { /* copy over */                    \
      ELL_3V_COPY(res, A);                                        \
      return 0;                                                   \
    } else if (res_type==tijk_3o3d_sym) {                         \
      res[0]=A[0]; res[1]=A[1]/3.0; res[2]=A[2]/3.0;              \
      res[3]=A[0]/3.0; res[4]=0; res[5]=A[0]/3.0; res[6]=A[1];    \
      res[7]=A[2]/3.0; res[8]=A[1]/3.0; res[9]=A[2];              \
      return 0;                                                   \
    } else if (NULL!=res_type->_convert_from_##SUF)               \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_1o3d);   \
    else                                                          \
      return 1;                                                   \
  }

_TIJK_1O3D_CONVERT(double, d)
_TIJK_1O3D_CONVERT(float, f)

#define _TIJK_1O3D_APPROX(TYPE, SUF)                             \
  int                                                            \
  _tijk_1o3d_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                           const TYPE *A) {                      \
    if (NULL!=res_type->_approx_from_##SUF)                      \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_1o3d);   \
    else                                                         \
      return 1;                                                  \
  }

_TIJK_1O3D_APPROX(double, d)
_TIJK_1O3D_APPROX(float, f)

double
_tijk_1o3d_s_form_d (const double *A, const double *v) {
  return ELL_3V_DOT(A,v);
}

float
_tijk_1o3d_s_form_f (const float *A, const float *v) {
  return ELL_3V_DOT(A,v);
}

double
_tijk_1o3d_mean_d (const double *A) {
  (void) A; /* odd order; mean does not depend on coeffs */
  return 0.0;
}

float
_tijk_1o3d_mean_f (const float *A) {
  (void) A; /* odd order; mean does not depend on coeffs */
  return 0.0f;
}

double
_tijk_1o3d_var_d (const double *A) {
  /* result from MATHEMATICA */
  return ELL_3V_DOT(A,A)/3.0;
}

float
_tijk_1o3d_var_f (const float *A) {
  /* result from MATHEMATICA */
  return ELL_3V_DOT(A,A)/3.0;
}

void
_tijk_1o3d_v_form_d (double *res, const double *A, const double *v) {
  (void) v; /* not used in this case */
  ELL_3V_COPY(res,A);
}

void
_tijk_1o3d_v_form_f (float *res, const float *A, const float *v) {
  (void) v; /* not used in this case */
  ELL_3V_COPY(res,A);
}

void
_tijk_1o3d_m_form_d (double *res, const double *A, const double *v) {
  (void) A; (void) v; /* not used in this case */
  ELL_3V_SET(res,0,0,0);
  ELL_3V_SET(res+3,0,0,0);
}

void
_tijk_1o3d_m_form_f (float *res, const float *A, const float *v) {
  (void) A; (void) v; /* not used in this case */
  ELL_3V_SET(res,0,0,0);
  ELL_3V_SET(res+3,0,0,0);
}

void
_tijk_1o3d_make_rank1_d (double *res, const double s, const double *v) {
  ELL_3V_SCALE(res,s,v);
}

void
_tijk_1o3d_make_rank1_f (float *res, const float s, const float *v) {
  ELL_3V_SCALE(res,s,v);
}

#define _tijk_1o3d_make_iso_d NULL
#define _tijk_1o3d_make_iso_f NULL

void
_tijk_1o3d_grad_d (double *res, const double *A, const double *v) {
  (void) v; /* not used in this case */
  ELL_3V_COPY(res,A);
}

void
_tijk_1o3d_grad_f (float *res, const float *A, const float *v) {
  (void) v; /* not used in this case */
  ELL_3V_COPY(res,A);
}

void
_tijk_1o3d_hess_d (double *res, const double *A, const double *v) {
  (void) A; (void) v; /* not used in this case */
  ELL_3V_SET(res,0,0,0);
  ELL_3V_SET(res+3,0,0,0);
}

void
_tijk_1o3d_hess_f (float *res, const float *A, const float *v) {
  (void) A; (void) v; /* not used in this case */
  ELL_3V_SET(res,0,0,0);
  ELL_3V_SET(res+3,0,0,0);
}

TIJK_TYPE_SYM(1o3d, 1, 3, 3)

/* 2nd order 3D unsymmetric */

double
_tijk_2o3d_unsym_tsp_d (const double *A, const double *B) {
  return ELL_3V_DOT(A,B)+ELL_3V_DOT(A+3,B+3)+ELL_3V_DOT(A+6,B+6);
}

float
_tijk_2o3d_unsym_tsp_f (const float *A, const float *B) {
  return ELL_3V_DOT(A,B)+ELL_3V_DOT(A+3,B+3)+ELL_3V_DOT(A+6,B+6);
}

double
_tijk_2o3d_unsym_norm_d (const double *A) {
  return sqrt(ELL_3V_DOT(A,A)+ELL_3V_DOT(A+3,A+3)+ELL_3V_DOT(A+6,A+6));
}

float
_tijk_2o3d_unsym_norm_f (const float *A) {
  return sqrt(ELL_3V_DOT(A,A)+ELL_3V_DOT(A+3,A+3)+ELL_3V_DOT(A+6,A+6));
}

void
_tijk_2o3d_unsym_trans_d (double *res, const double *A, const double *M) {
  double _ma[9], _mt[9];
  ELL_3M_MUL(_ma, M, A);
  ELL_3M_TRANSPOSE(_mt, M);
  ELL_3M_MUL(res, _ma, _mt);
}

void
_tijk_2o3d_unsym_trans_f (float *res, const float *A, const float *M) {
  float _ma[9], _mt[9];
  ELL_3M_MUL(_ma, M, A);
  ELL_3M_TRANSPOSE(_mt, M);
  ELL_3M_MUL(res, _ma, _mt);
}

/* macro-based pseudo-template for type-generic code */
#define _TIJK_2O3D_UNSYM_CONVERT(TYPE, SUF)                             \
  int                                                                   \
  _tijk_2o3d_unsym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                  const TYPE *A) {                      \
    if (res_type==tijk_2o3d_unsym) { /* copy over */                    \
      ELL_3M_COPY(res, A);                                              \
      return 0;                                                         \
    } else if (NULL!=res_type->_convert_from_##SUF)                     \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_2o3d_unsym);   \
    else                                                                \
      return 1;                                                         \
  }

_TIJK_2O3D_UNSYM_CONVERT(double, d)
_TIJK_2O3D_UNSYM_CONVERT(float, f)

#define _TIJK_2O3D_UNSYM_APPROX(TYPE, SUF)                             \
  int                                                                  \
  _tijk_2o3d_unsym_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                                 const TYPE *A) {                      \
    if (res_type==tijk_2o3d_sym) {                                     \
      res[0]=A[0]; res[1]=0.5*(A[1]+A[3]); res[2]=0.5*(A[2]+A[6]);     \
      res[3]=A[4]; res[4]=0.5*(A[5]+A[7]); res[5]=A[8];                \
      return 0;                                                        \
    } else if (res_type==tijk_2o3d_asym) {                             \
      res[0]=0.5*(A[1]-A[3]); res[1]=0.5*(A[2]-A[6]);                  \
      res[2]=0.5*(A[5]-A[7]);                                          \
      return 0;                                                        \
    } else if (NULL!=res_type->_approx_from_##SUF)                     \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_2o3d_unsym);   \
    else                                                               \
      return 1;                                                        \
  }

_TIJK_2O3D_UNSYM_APPROX(double, d)
_TIJK_2O3D_UNSYM_APPROX(float, f)

TIJK_TYPE_UNSYM(2o3d_unsym, 2, 3, 9)

/* 2nd order 3D symmetric */

unsigned int _tijk_2o3d_sym_mult[6] = {1, 2, 2, 1, 2, 1};
int _tijk_2o3d_sym_unsym2uniq[9] = {1, 2, 3, 2, 4, 5, 3, 5, 6};
int _tijk_2o3d_sym_uniq2unsym[9] = {1, 2, 4, 3, 7, 5, 6, 7, 9};
unsigned int _tijk_2o3d_sym_uniq_idx[6] = {0, 1, 3, 5, 6, 8};

#define _TIJK_2O3D_SYM_TSP(A, B)                  \
  ((A)[0]*(B)[0]+2*(A)[1]*(B)[1]+2*(A)[2]*(B)[2]+ \
   (A)[3]*(B)[3]+2*(A)[4]*(B)[4]+(A)[5]*(B)[5])

double
_tijk_2o3d_sym_tsp_d (const double *A, const double *B) {
  return _TIJK_2O3D_SYM_TSP(A,B);
}

float
_tijk_2o3d_sym_tsp_f (const float *A, const float *B) {
  return _TIJK_2O3D_SYM_TSP(A,B);
}

double
_tijk_2o3d_sym_norm_d (const double *A) {
  return sqrt(_TIJK_2O3D_SYM_TSP(A,A));
}

float
_tijk_2o3d_sym_norm_f (const float *A) {
  return sqrt(_TIJK_2O3D_SYM_TSP(A,A));
}

#define _TIJK_2O3D_SYM_CONVERT(TYPE, SUF)                             \
  int                                                                 \
  _tijk_2o3d_sym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                const TYPE *A) {                      \
    if (res_type==tijk_2o3d_sym) { /* copy over */                    \
      ELL_3V_COPY(res, A); ELL_3V_COPY(res+3, A+3);                   \
      return 0;                                                       \
    } else if (res_type==tijk_2o3d_unsym) {                           \
      res[0]=A[0]; res[1]=res[3]=A[1]; res[2]=res[6]=A[2];            \
      res[4]=A[3]; res[5]=res[7]=A[4]; res[8]=A[5];                   \
      return 0;                                                       \
    } else if (res_type==tijk_4o3d_sym ||                             \
               res_type==tijk_6o3d_sym) {                             \
      /* do this by going to SH and zero-padding */                   \
      TYPE tmp[28];                                                   \
      memset(tmp,0,sizeof(tmp));                                      \
      tijk_3d_sym_to_esh_##SUF (tmp, A, tijk_2o3d_sym);               \
      tijk_esh_to_3d_sym_##SUF (res, tmp, res_type->order);           \
      return 0;                                                       \
    } else if (NULL!=res_type->_convert_from_##SUF)                   \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_2o3d_sym);   \
    return 1;                                                         \
  }

_TIJK_2O3D_SYM_CONVERT(double, d)
_TIJK_2O3D_SYM_CONVERT(float, f)

#define _TIJK_2O3D_SYM_APPROX(TYPE, SUF)                             \
  int                                                                \
  _tijk_2o3d_sym_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                               const TYPE *A) {                      \
    if (NULL!=res_type->_approx_from_##SUF)                          \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_2o3d_sym);   \
    else                                                             \
      return 1;                                                      \
  }

_TIJK_2O3D_SYM_APPROX(double, d)
_TIJK_2O3D_SYM_APPROX(float, f)

void
_tijk_2o3d_sym_trans_d (double *res, const double *A, const double *M) {
  /* this code could be optimized at some point */
  double tmp[9], tmpout[9];
  _tijk_2o3d_sym_convert_d(tmp, tijk_2o3d_unsym, A);
  _tijk_2o3d_unsym_trans_d(tmpout, tmp, M);
  _tijk_2o3d_unsym_approx_d(res, tijk_2o3d_sym, tmpout);
}

void
_tijk_2o3d_sym_trans_f (float *res, const float *A, const float *M) {
  /* this code could be optimized at some point */
  float tmp[9], tmpout[9];
  _tijk_2o3d_sym_convert_f(tmp, tijk_2o3d_unsym, A);
  _tijk_2o3d_unsym_trans_f(tmpout, tmp, M);
  _tijk_2o3d_unsym_approx_f(res, tijk_2o3d_sym, tmpout);
}

double
_tijk_2o3d_sym_s_form_d (const double *A, const double *v) {
  return A[0]*v[0]*v[0]+2*A[1]*v[0]*v[1]+2*A[2]*v[0]*v[2]+
    A[3]*v[1]*v[1]+2*A[4]*v[1]*v[2]+A[5]*v[2]*v[2];
}

float
_tijk_2o3d_sym_s_form_f (const float *A, const float *v) {
  return A[0]*v[0]*v[0]+2*A[1]*v[0]*v[1]+2*A[2]*v[0]*v[2]+
    A[3]*v[1]*v[1]+2*A[4]*v[1]*v[2]+A[5]*v[2]*v[2];
}

double
_tijk_2o3d_sym_mean_d (const double *A) {
  return (A[0]+A[3]+A[5])/3.0;
}

float
_tijk_2o3d_sym_mean_f (const float *A) {
  return (A[0]+A[3]+A[5])/3.0f;
}

double
_tijk_2o3d_sym_var_d (const double *A) {
  return 4.0/45.0*(A[0]*A[0]+A[3]*A[3]+A[5]*A[5]+
                   3*(A[1]*A[1]+A[2]*A[2]+A[4]*A[4])-
                   A[3]*A[5]-A[0]*(A[3]+A[5]));
}

float
_tijk_2o3d_sym_var_f (const float *A) {
  return 4.0f/45.0f*(A[0]*A[0]+A[3]*A[3]+A[5]*A[5]+
                     3.0f*(A[1]*A[1]+A[2]*A[2]+A[4]*A[4])-
                     A[3]*A[5]-A[0]*(A[3]+A[5]));
}

void
_tijk_2o3d_sym_v_form_d (double *res, const double *A, const double *v) {
  res[0]=A[0]*v[0]+A[1]*v[1]+A[2]*v[2];
  res[1]=A[1]*v[0]+A[3]*v[1]+A[4]*v[2];
  res[2]=A[2]*v[0]+A[4]*v[1]+A[5]*v[2];
}

void
_tijk_2o3d_sym_v_form_f (float *res, const float *A, const float *v) {
  res[0]=A[0]*v[0]+A[1]*v[1]+A[2]*v[2];
  res[1]=A[1]*v[0]+A[3]*v[1]+A[4]*v[2];
  res[2]=A[2]*v[0]+A[4]*v[1]+A[5]*v[2];
}

void
_tijk_2o3d_sym_m_form_d (double *res, const double *A, const double *v) {
  (void) v; /* v is only used in higher-order cases */
  ELL_3V_COPY(res,A); ELL_3V_COPY(res+3,A+3);
}

void
_tijk_2o3d_sym_m_form_f (float *res, const float *A, const float *v) {
  (void) v; /* v is only used in higher-order cases */
  ELL_3V_COPY(res,A); ELL_3V_COPY(res+3,A+3);
}

void
_tijk_2o3d_sym_make_rank1_d (double *res, const double s, const double *v) {
  res[0]=s*v[0]*v[0]; res[1]=s*v[0]*v[1]; res[2]=s*v[0]*v[2];
  res[3]=s*v[1]*v[1]; res[4]=s*v[1]*v[2]; res[5]=s*v[2]*v[2];
}

void
_tijk_2o3d_sym_make_rank1_f (float *res, const float s, const float *v) {
  res[0]=s*v[0]*v[0]; res[1]=s*v[0]*v[1]; res[2]=s*v[0]*v[2];
  res[3]=s*v[1]*v[1]; res[4]=s*v[1]*v[2]; res[5]=s*v[2]*v[2];
}

void
_tijk_2o3d_sym_make_iso_d (double *res, const double s) {
  res[0]=res[3]=res[5]=s;
  res[1]=res[2]=res[4]=0;
}

void
_tijk_2o3d_sym_make_iso_f (float *res, const float s) {
  res[0]=res[3]=res[5]=s;
  res[1]=res[2]=res[4]=0;
}

void
_tijk_2o3d_sym_grad_d (double *res, const double *A, const double *v) {
  double proj, projv[3];
  res[0]=2*(A[0]*v[0]+A[1]*v[1]+A[2]*v[2]);
  res[1]=2*(A[1]*v[0]+A[3]*v[1]+A[4]*v[2]);
  res[2]=2*(A[2]*v[0]+A[4]*v[1]+A[5]*v[2]);
  proj=ELL_3V_DOT(res,v);
  ELL_3V_SCALE(projv,-proj,v);
  ELL_3V_INCR(res,projv);
}

void
_tijk_2o3d_sym_grad_f (float *res, const float *A, const float *v) {
  float proj, projv[3];
  res[0]=2.0f*(A[0]*v[0]+A[1]*v[1]+A[2]*v[2]);
  res[1]=2.0f*(A[1]*v[0]+A[3]*v[1]+A[4]*v[2]);
  res[2]=2.0f*(A[2]*v[0]+A[4]*v[1]+A[5]*v[2]);
  proj=ELL_3V_DOT(res,v);
  ELL_3V_SCALE(projv,-proj,v);
  ELL_3V_INCR(res,projv);
}

#define _TIJK_2O3D_SYM_HESS(TYPE, SUF)                                  \
  void                                                                  \
  _tijk_2o3d_sym_hess_##SUF (TYPE *res, const TYPE *A, const TYPE *v) { \
    /* get two orthonormal tangents */                                  \
    TYPE t[2][3], h[4], der, norm, tmp[6];                              \
    int r,c;                                                            \
    ell_3v_perp_##SUF(t[0], v);                                         \
    ELL_3V_NORM(t[0],t[0],norm);                                        \
    ELL_3V_CROSS(t[1],v,t[0]);                                          \
    ELL_3V_NORM(t[1],t[1],norm);                                        \
    /* compute Hessian w.r.t. t1/t2 */                                  \
    der=2*_tijk_2o3d_sym_s_form_##SUF(A, v); /* first der in direction v*/ \
    h[0]=2*_tijk_2o3d_sym_s_form_##SUF(A,t[0])-der;                     \
    h[1]=2*(A[0]*t[0][0]*t[1][0]+A[1]*t[0][0]*t[1][1]+A[2]*t[0][0]*t[1][2]+ \
            A[1]*t[0][1]*t[1][0]+A[3]*t[0][1]*t[1][1]+A[4]*t[0][1]*t[1][2]+ \
            A[2]*t[0][2]*t[1][0]+A[4]*t[0][2]*t[1][1]+A[5]*t[0][2]*t[1][2]); \
    h[2]=h[1];                                                          \
    h[3]=2*_tijk_2o3d_sym_s_form_##SUF(A,t[1])-der;                     \
    /* now turn this into a symmetric order-2 rank-2 3D tensor */       \
    for (r=0; r<2; r++) {                                               \
      for (c=0; c<3; c++) {                                             \
        tmp[3*r+c]=h[2*r]*t[0][c]+h[2*r+1]*t[1][c];                     \
      }                                                                 \
    }                                                                   \
    res[0]=t[0][0]*tmp[0]+t[1][0]*tmp[3];                               \
    res[1]=t[0][0]*tmp[1]+t[1][0]*tmp[4];                               \
    res[2]=t[0][0]*tmp[2]+t[1][0]*tmp[5];                               \
    res[3]=t[0][1]*tmp[1]+t[1][1]*tmp[4];                               \
    res[4]=t[0][1]*tmp[2]+t[1][1]*tmp[5];                               \
    res[5]=t[0][2]*tmp[2]+t[1][2]*tmp[5];                               \
  }

_TIJK_2O3D_SYM_HESS(double,d)
_TIJK_2O3D_SYM_HESS(float,f)

TIJK_TYPE_SYM(2o3d_sym, 2, 3, 6)

/* 2nd order 3D antisymmetric */

unsigned int _tijk_2o3d_asym_mult[3] = {2,2,2};
int _tijk_2o3d_asym_unsym2uniq[9] = {0, 1, 2, -1, 0, 3, -2, -3, 0};
int _tijk_2o3d_asym_uniq2unsym[6] = {2, -4, 3, -7, 6, -8};
unsigned int _tijk_2o3d_asym_uniq_idx[3] = {0,2,4};

double
_tijk_2o3d_asym_tsp_d (const double *A, const double *B) {
  return 2*ELL_3V_DOT(A,B);
}

float
_tijk_2o3d_asym_tsp_f (const float *A, const float *B) {
  return 2*ELL_3V_DOT(A,B);
}

double
_tijk_2o3d_asym_norm_d (const double *A) {
  return sqrt(2*ELL_3V_DOT(A,A));
}

float
_tijk_2o3d_asym_norm_f (const float *A) {
  return sqrt(2*ELL_3V_DOT(A,A));
}

#define _TIJK_2O3D_ASYM_CONVERT(TYPE, SUF)                             \
  int                                                                  \
  _tijk_2o3d_asym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                 const TYPE *A) {                      \
    if (res_type==tijk_2o3d_asym) { /* copy over */                    \
      ELL_3V_COPY(res,A);                                              \
      return 0;                                                        \
    } else if (res_type==tijk_2o3d_unsym) {                            \
      res[0]=0; res[1]=A[0]; res[2]=A[1];                              \
      res[3]=-A[0]; res[4]=0; res[5]=A[2];                             \
      res[6]=-A[1]; res[7]=-A[2]; res[8]=0;                            \
      return 0;                                                        \
    } else if (NULL!=res_type->_convert_from_##SUF)                    \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_2o3d_asym);   \
    else                                                               \
      return 1;                                                        \
  }

_TIJK_2O3D_ASYM_CONVERT(double, d)
_TIJK_2O3D_ASYM_CONVERT(float, f)

#define _TIJK_2O3D_ASYM_APPROX(TYPE, SUF)                             \
  int                                                                 \
  _tijk_2o3d_asym_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                                const TYPE *A) {                      \
    if (NULL!=res_type->_approx_from_##SUF)                           \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_2o3d_asym);   \
    else                                                              \
      return 1;                                                       \
  }

_TIJK_2O3D_ASYM_APPROX(double, d)
_TIJK_2O3D_ASYM_APPROX(float, f)

void
_tijk_2o3d_asym_trans_d (double *res, const double *A, const double *M) {
  /* this code could be optimized at some point */
  double tmp[9], tmpout[9];
  _tijk_2o3d_asym_convert_d(tmp, tijk_2o3d_unsym, A);
  _tijk_2o3d_unsym_trans_d(tmpout, tmp, M);
  _tijk_2o3d_unsym_approx_d(res, tijk_2o3d_asym, tmpout);
}

void
_tijk_2o3d_asym_trans_f (float *res, const float *A, const float *M) {
  /* this code could be optimized at some point */
  float tmp[9], tmpout[9];
  _tijk_2o3d_asym_convert_f(tmp, tijk_2o3d_unsym, A);
  _tijk_2o3d_unsym_trans_f(tmpout, tmp, M);
  _tijk_2o3d_unsym_approx_f(res, tijk_2o3d_asym, tmpout);
}

TIJK_TYPE(2o3d_asym, 2, 3, 3)

/* 3rd order 3D unsymmetric */

double
_tijk_3o3d_unsym_tsp_d (const double *A, const double *B) {
  double retval=0;
  unsigned int i;
  for (i=0; i<27; i++)
    retval+=(*A++)*(*B++);
  return retval;
}

float
_tijk_3o3d_unsym_tsp_f (const float *A, const float *B) {
  float retval=0;
  unsigned int i;
  for (i=0; i<27; i++)
    retval+=(*A++)*(*B++);
  return retval;
}

double
_tijk_3o3d_unsym_norm_d (const double *A) {
  return sqrt(_tijk_3o3d_unsym_tsp_d(A,A));
}

float
_tijk_3o3d_unsym_norm_f (const float *A) {
  return sqrt(_tijk_3o3d_unsym_tsp_f(A,A));
}

void
_tijk_3o3d_unsym_trans_d (double *res, const double *A, const double *M) {
  double _tmp1[27], _tmp2[27];
  unsigned int i,j,k;
  for (i=0; i<3; i++)
    for (j=0; j<3; j++)
      for (k=0; k<3; k++)
        _tmp1[3*(3*i+j)+k]=M[3*k]*A[3*(3*i+j)]+
          M[3*k+1]*A[3*(3*i+j)+1]+
          M[3*k+2]*A[3*(3*i+j)+2];
  for (i=0; i<3; i++)
    for (j=0; j<3; j++)
      for (k=0; k<3; k++)
        _tmp2[3*(3*i+j)+k]=M[3*j]*_tmp1[3*(3*i)+k]+
          M[3*j+1]*_tmp1[3*(3*i+1)+k]+
          M[3*j+2]*_tmp1[3*(3*i+2)+k];
  for (i=0; i<3; i++)
    for (j=0; j<3; j++)
      for (k=0; k<3; k++)
        res[3*(3*i+j)+k]=M[3*i]*_tmp2[3*j+k]+
          M[3*i+1]*_tmp2[3*(3+j)+k]+
          M[3*i+2]*_tmp2[3*(6+j)+k];
}

void
_tijk_3o3d_unsym_trans_f (float *res, const float *A, const float *M) {
  float _tmp1[27], _tmp2[27];
  unsigned int i,j,k;
  for (i=0; i<3; i++)
    for (j=0; j<3; j++)
      for (k=0; k<3; k++)
        _tmp1[3*(3*i+j)+k]=M[3*k]*A[3*(3*i+j)]+
          M[3*k+1]*A[3*(3*i+j)+1]+
          M[3*k+2]*A[3*(3*i+j)+2];
  for (i=0; i<3; i++)
    for (j=0; j<3; j++)
      for (k=0; k<3; k++)
        _tmp2[3*(3*i+j)+k]=M[3*j]*_tmp1[3*(3*i)+k]+
          M[3*j+1]*_tmp1[3*(3*i+1)+k]+
          M[3*j+2]*_tmp1[3*(3*i+2)+k];
  for (i=0; i<3; i++)
    for (j=0; j<3; j++)
      for (k=0; k<3; k++)
        res[3*(3*i+j)+k]=M[3*i]*_tmp2[3*j+k]+
          M[3*i+1]*_tmp2[3*(3+j)+k]+
          M[3*i+2]*_tmp2[3*(6+j)+k];
}

#define _TIJK_3O3D_UNSYM_CONVERT(TYPE, SUF)                             \
  int                                                                   \
  _tijk_3o3d_unsym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                  const TYPE *A) {                      \
    if (res_type==tijk_3o3d_unsym) { /* copy over */                    \
      unsigned int i;                                                   \
      for (i=0; i<27; i++)                                              \
        (*res++)=(*A++);                                                \
      return 0;                                                         \
    } else if (NULL!=res_type->_convert_from_##SUF)                     \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_3o3d_unsym);   \
    else                                                                \
      return 1;                                                         \
  }

_TIJK_3O3D_UNSYM_CONVERT(double, d)
_TIJK_3O3D_UNSYM_CONVERT(float, f)

#define _TIJK_3O3D_UNSYM_APPROX(TYPE, SUF)                             \
  int                                                                  \
  _tijk_3o3d_unsym_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                                 const TYPE *A) {                      \
    if (res_type==tijk_3o3d_sym) {                                     \
      res[0]=A[0]; res[1]=(A[1]+A[3]+A[9])/3.0;                        \
      res[2]=(A[2]+A[6]+A[18])/3.0; res[3]=(A[4]+A[10]+A[12])/3.0;     \
      res[4]=(A[5]+A[7]+A[11]+A[15]+A[19]+A[21])/6.0;                  \
      res[5]=(A[8]+A[20]+A[24])/3.0; res[6]=A[13];                     \
      res[7]=(A[14]+A[16]+A[25])/3.0; res[8]=(A[17]+A[23]+A[25])/3.0;  \
      res[9]=A[26];                                                    \
      return 0;                                                        \
    } else if (NULL!=res_type->_approx_from_##SUF)                     \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_3o3d_unsym);   \
    else                                                               \
      return 1;                                                        \
  }

_TIJK_3O3D_UNSYM_APPROX(double, d)
_TIJK_3O3D_UNSYM_APPROX(float, f)

TIJK_TYPE_UNSYM(3o3d_unsym, 3, 3, 27)

/* 3rd order 3D symmetric */

unsigned int _tijk_3o3d_sym_mult[10]={1,3,3,3,6,3,1,3,3,1};
int _tijk_3o3d_sym_unsym2uniq[27] = {1,2,3,2,4,5,3,5,6,
                                     2,4,5,4,7,8,5,8,9,
                                     3,5,6,5,8,9,6,9,10};
int _tijk_3o3d_sym_uniq2unsym[27] = {1, 2,4,10, 3,7,19, 5,11,13,
                                     6,8,12,16,20,22, 9,21,25, 14,
                                     15,17,23, 18,24,26, 27};
unsigned int _tijk_3o3d_sym_uniq_idx[10] = {0,1,4,7,10,16,19,20,23,26};

#define _TIJK_3O3D_SYM_TSP(A, B)                               \
  ((A)[0]*(B)[0]+(A)[6]*(B)[6]+(A)[9]*(B)[9]+                  \
   3*((A)[1]*(B)[1]+(A)[2]*(B)[2]+(A)[3]*(B)[3]+(A)[5]*(B)[5]+ \
      (A)[7]*(B)[7]+(A)[8]*(B)[8])+                            \
   6*(A)[4]*(B)[4])

double
_tijk_3o3d_sym_tsp_d (const double *A, const double *B) {
  return _TIJK_3O3D_SYM_TSP(A,B);
}

float
_tijk_3o3d_sym_tsp_f (const float *A, const float *B) {
  return _TIJK_3O3D_SYM_TSP(A,B);
}

double
_tijk_3o3d_sym_norm_d (const double *A) {
  return sqrt(_TIJK_3O3D_SYM_TSP(A,A));
}

float
_tijk_3o3d_sym_norm_f (const float *A) {
  return sqrt(_TIJK_3O3D_SYM_TSP(A,A));
}

#define _TIJK_3O3D_SYM_CONVERT(TYPE, SUF)                             \
  int                                                                 \
  _tijk_3o3d_sym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                const TYPE *A) {                      \
    if (res_type==tijk_3o3d_sym) { /* copy over */                    \
      tijk_copy_##SUF(res, A, tijk_3o3d_sym);                         \
      return 0;                                                       \
    } else if (res_type==tijk_3o3d_unsym) {                           \
      unsigned int i;                                                 \
      for (i=0; i<27; i++) {                                          \
        res[i]=A[_tijk_3o3d_sym_unsym2uniq[i]-1];                     \
      }                                                               \
      return 0;                                                       \
    } else if (NULL!=res_type->_convert_from_##SUF)                   \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_3o3d_sym);   \
    return 1;                                                         \
  }

_TIJK_3O3D_SYM_CONVERT(double, d)
_TIJK_3O3D_SYM_CONVERT(float, f)

#define _TIJK_3O3D_SYM_APPROX(TYPE, SUF)                             \
  int                                                                \
  _tijk_3o3d_sym_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                               const TYPE *A){                       \
    if (res_type==tijk_1o3d) {                                       \
      res[0]=0.6*(A[0]+A[3]+A[5]);                                   \
      res[1]=0.6*(A[1]+A[6]+A[8]);                                   \
      res[2]=0.6*(A[2]+A[7]+A[9]);                                   \
      return 0;                                                      \
    } else if (NULL!=res_type->_approx_from_##SUF)                   \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_3o3d_sym);   \
    else                                                             \
      return 1;                                                      \
  }

_TIJK_3O3D_SYM_APPROX(double, d)
_TIJK_3O3D_SYM_APPROX(float, f)

void
_tijk_3o3d_sym_trans_d (double *res, const double *A, const double *M) {
  /* this code could be optimized at some point */
  double tmp[27], tmpout[27];
  _tijk_3o3d_sym_convert_d(tmp, tijk_3o3d_unsym, A);
  _tijk_3o3d_unsym_trans_d(tmpout, tmp, M);
  _tijk_3o3d_unsym_approx_d(res, tijk_3o3d_sym, tmpout);
}

void
_tijk_3o3d_sym_trans_f (float *res, const float *A, const float *M) {
  /* this code could be optimized at some point */
  float tmp[27], tmpout[27];
  _tijk_3o3d_sym_convert_f(tmp, tijk_3o3d_unsym, A);
  _tijk_3o3d_unsym_trans_f(tmpout, tmp, M);
  _tijk_3o3d_unsym_approx_f(res, tijk_3o3d_sym, tmpout);
}

double
_tijk_3o3d_sym_s_form_d (const double *A, const double *v) {
  double v00=v[0]*v[0], v11=v[1]*v[1], v22=v[2]*v[2];
  return A[0]*v00*v[0]+3*A[1]*v00*v[1]+3*A[2]*v00*v[2]+3*A[3]*v11*v[0]+
    6*A[4]*v[0]*v[1]*v[2]+3*A[5]*v22*v[0]+A[6]*v11*v[1]+3*A[7]*v11*v[2]+
    3*A[8]*v[1]*v22+A[9]*v22*v[2];
}

float
_tijk_3o3d_sym_s_form_f (const float *A, const float *v) {
  float v00=v[0]*v[0], v11=v[1]*v[1], v22=v[2]*v[2];
  return A[0]*v00*v[0]+3*A[1]*v00*v[1]+3*A[2]*v00*v[2]+3*A[3]*v11*v[0]+
    6*A[4]*v[0]*v[1]*v[2]+3*A[5]*v22*v[0]+A[6]*v11*v[1]+3*A[7]*v11*v[2]+
    3*A[8]*v[1]*v22+A[9]*v22*v[2];
}

double
_tijk_3o3d_sym_mean_d (const double *A) {
  (void) A; /* odd order; mean does not depend on coeffs */
  return 0.0;
}

float
_tijk_3o3d_sym_mean_f (const float *A) {
  (void) A; /* odd order; mean does not depend on coeffs */
  return 0.0f;
}

double
_tijk_3o3d_sym_var_d (const double *A) {
  /* numerical result taken from MATHEMATICA */
  return 1.0/35.0*(5*(A[0]*A[0]+A[6]*A[6]+A[9]*A[9])+
                   6*(A[3]*A[5]+A[2]*(A[7]+A[9])+A[6]*A[8]+A[0]*(A[3]+A[5])+
                      A[1]*(A[6]+A[8])+A[7]*A[9])+
                   9*(A[1]*A[1]+A[2]*A[2]+A[3]*A[3]+A[5]*A[5]+A[7]*A[7]+
                      A[8]*A[8])+
                   12*A[4]*A[4]);
}

float
_tijk_3o3d_sym_var_f (const float *A) {
  /* numerical result taken from MATHEMATICA */
  return 1.0/35.0*(5*(A[0]*A[0]+A[6]*A[6]+A[9]*A[9])+
                   6*(A[3]*A[5]+A[2]*(A[7]+A[9])+A[6]*A[8]+A[0]*(A[3]+A[5])+
                      A[1]*(A[6]+A[8])+A[7]*A[9])+
                   9*(A[1]*A[1]+A[2]*A[2]+A[3]*A[3]+A[5]*A[5]+A[7]*A[7]+
                      A[8]*A[8])+
                   12*A[4]*A[4]);
}

void
_tijk_3o3d_sym_v_form_d (double *res, const double *A, const double *v) {
  double v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],
    v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];
  res[0] = A[0]*v00+A[3]*v11+A[5]*v22+ 2*(A[1]*v01+A[2]*v02+A[4]*v12);
  res[1] = A[1]*v00+A[6]*v11+A[8]*v22+ 2*(A[3]*v01+A[4]*v02+A[7]*v12);
  res[2] = A[2]*v00+A[7]*v11+A[9]*v22+ 2*(A[4]*v01+A[5]*v02+A[8]*v12);
}

void
_tijk_3o3d_sym_v_form_f (float *res, const float *A, const float *v) {
  float v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],
    v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];
  res[0] = A[0]*v00+A[3]*v11+A[5]*v22+ 2*(A[1]*v01+A[2]*v02+A[4]*v12);
  res[1] = A[1]*v00+A[6]*v11+A[8]*v22+ 2*(A[3]*v01+A[4]*v02+A[7]*v12);
  res[2] = A[2]*v00+A[7]*v11+A[9]*v22+ 2*(A[4]*v01+A[5]*v02+A[8]*v12);
}

void
_tijk_3o3d_sym_m_form_d (double *res, const double *A, const double *v) {
  res[0]=A[0]*v[0]+A[1]*v[1]+A[2]*v[2];
  res[1]=A[1]*v[0]+A[3]*v[1]+A[4]*v[2];
  res[2]=A[2]*v[0]+A[4]*v[1]+A[5]*v[2];
  res[3]=A[3]*v[0]+A[6]*v[1]+A[7]*v[2];
  res[4]=A[4]*v[0]+A[7]*v[1]+A[8]*v[2];
  res[5]=A[5]*v[0]+A[8]*v[1]+A[9]*v[2];
}

void
_tijk_3o3d_sym_m_form_f (float *res, const float *A, const float *v) {
  res[0]=A[0]*v[0]+A[1]*v[1]+A[2]*v[2];
  res[1]=A[1]*v[0]+A[3]*v[1]+A[4]*v[2];
  res[2]=A[2]*v[0]+A[4]*v[1]+A[5]*v[2];
  res[3]=A[3]*v[0]+A[6]*v[1]+A[7]*v[2];
  res[4]=A[4]*v[0]+A[7]*v[1]+A[8]*v[2];
  res[5]=A[5]*v[0]+A[8]*v[1]+A[9]*v[2];
}

void
_tijk_3o3d_sym_make_rank1_d (double *res, const double s, const double *v) {
  double v00=v[0]*v[0], v11=v[1]*v[1], v22=v[2]*v[2];
  res[0]=s*v00*v[0]; res[1]=s*v00*v[1]; res[2]=s*v00*v[2];
  res[3]=s*v[0]*v11; res[4]=s*v[0]*v[1]*v[2]; res[5]=s*v[0]*v22;
  res[6]=s*v[1]*v11; res[7]=s*v11*v[2]; res[8]=s*v[1]*v22;
  res[9]=s*v[2]*v22;
}

void
_tijk_3o3d_sym_make_rank1_f (float *res, const float s, const float *v) {
  float v00=v[0]*v[0], v11=v[1]*v[1], v22=v[2]*v[2];
  res[0]=s*v00*v[0]; res[1]=s*v00*v[1]; res[2]=s*v00*v[2];
  res[3]=s*v[0]*v11; res[4]=s*v[0]*v[1]*v[2]; res[5]=s*v[0]*v22;
  res[6]=s*v[1]*v11; res[7]=s*v11*v[2]; res[8]=s*v[1]*v22;
  res[9]=s*v[2]*v22;
}

#define _tijk_3o3d_sym_make_iso_d NULL
#define _tijk_3o3d_sym_make_iso_f NULL

void
_tijk_3o3d_sym_grad_d (double *res, const double *A, const double *v) {
  double proj, projv[3];
  _tijk_3o3d_sym_v_form_d (res, A, v);
  ELL_3V_SCALE(res,3.0,res);
  proj=ELL_3V_DOT(res,v);
  ELL_3V_SCALE(projv,-proj,v);
  ELL_3V_INCR(res,projv);
}

void
_tijk_3o3d_sym_grad_f (float *res, const float *A, const float *v) {
  float proj, projv[3];
  _tijk_3o3d_sym_v_form_f (res, A, v);
  ELL_3V_SCALE(res,3.0,res);
  proj=ELL_3V_DOT(res,v);
  ELL_3V_SCALE(projv,-proj,v);
  ELL_3V_INCR(res,projv);
}

#define _TIJK_3O3D_SYM_HESS(TYPE, SUF)                                  \
  void                                                                  \
  _tijk_3o3d_sym_hess_##SUF (TYPE *res, const TYPE *A, const TYPE *v) { \
    /* get two orthonormal tangents */                                  \
    TYPE t[2][3], cv[2][3], h[6], der, norm, tmp[6];                    \
    int r,c;                                                            \
    ell_3v_perp_##SUF(t[0], v);                                         \
    ELL_3V_NORM(t[0],t[0],norm);                                        \
    ELL_3V_CROSS(t[1],v,t[0]);                                          \
    ELL_3V_NORM(t[1],t[1],norm);                                        \
    /* compute Hessian w.r.t. t1/t2 */                                  \
    _tijk_3o3d_sym_m_form_##SUF(h, A, v);                               \
    der=3*_tijk_3o3d_sym_s_form_##SUF(A, v); /* first der in direction v*/ \
    _tijk_2o3d_sym_v_form_##SUF(cv[0],h,t[0]);                          \
    _tijk_2o3d_sym_v_form_##SUF(cv[1],h,t[1]);                          \
    h[0]=6*ELL_3V_DOT(cv[0],t[0])-der;                                  \
    h[1]=6*ELL_3V_DOT(cv[0],t[1]);                                      \
    h[2]=h[1];                                                          \
    h[3]=6*ELL_3V_DOT(cv[1],t[1])-der;                                  \
    /* now turn this into a symmetric order-2 rank-2 3D tensor */       \
    for (r=0; r<2; r++) {                                               \
      for (c=0; c<3; c++) {                                             \
        tmp[3*r+c]=h[2*r]*t[0][c]+h[2*r+1]*t[1][c];                     \
      }                                                                 \
    }                                                                   \
    res[0]=t[0][0]*tmp[0]+t[1][0]*tmp[3];                               \
    res[1]=t[0][0]*tmp[1]+t[1][0]*tmp[4];                               \
    res[2]=t[0][0]*tmp[2]+t[1][0]*tmp[5];                               \
    res[3]=t[0][1]*tmp[1]+t[1][1]*tmp[4];                               \
    res[4]=t[0][1]*tmp[2]+t[1][1]*tmp[5];                               \
    res[5]=t[0][2]*tmp[2]+t[1][2]*tmp[5];                               \
  }

_TIJK_3O3D_SYM_HESS(double,d)
_TIJK_3O3D_SYM_HESS(float,f)

TIJK_TYPE_SYM(3o3d_sym, 3, 3, 10)

/* 4th order 3D symmetric */
/* (unsymmetric counterpart currently not implemented) */

unsigned int _tijk_4o3d_sym_mult[15]={1,4,4,6,12,6,4,12,12,4,1,4,6,4,1};
#define _tijk_4o3d_sym_unsym2uniq NULL
#define _tijk_4o3d_sym_uniq2unsym NULL
#define _tijk_4o3d_sym_uniq_idx NULL

#define _TIJK_4O3D_SYM_TSP(A, B)                                \
  ((A)[0]*(B)[0]+(A)[10]*(B)[10]+(A)[14]*(B)[14]+               \
   4*((A)[1]*(B)[1]+(A)[2]*(B)[2]+(A)[6]*(B)[6]+(A)[9]*(B)[9]+  \
      (A)[11]*(B)[11]+(A)[13]*(B)[13])+                         \
   6*((A)[3]*(B)[3]+(A)[5]*(B)[5]+(A)[12]*(B)[12])+             \
   12*((A)[4]*(B)[4]+(A)[7]*(B)[7]+(A)[8]*(B)[8]))

double
_tijk_4o3d_sym_tsp_d (const double *A, const double *B) {
  return _TIJK_4O3D_SYM_TSP(A,B);
}

float
_tijk_4o3d_sym_tsp_f (const float *A, const float *B) {
  return _TIJK_4O3D_SYM_TSP(A,B);
}

double
_tijk_4o3d_sym_norm_d (const double *A) {
  return sqrt(_TIJK_4O3D_SYM_TSP(A,A));
}

float
_tijk_4o3d_sym_norm_f (const float *A) {
  return sqrt(_TIJK_4O3D_SYM_TSP(A,A));
}

#define _TIJK_4O3D_SYM_TRANS(TYPE, SUF)                                 \
  void                                                                  \
  _tijk_4o3d_sym_trans_##SUF (TYPE *res, const TYPE *A, const TYPE *M) { \
    /* Tijkl = Mim Mjn Mko Mlp Tmnop                                    \
     * For efficiency, we transform mode by mode; the intermediate results \
     * have incomplete symmetries! */                                   \
    TYPE tmps[30], tmpl[36];                                            \
    int i;                                                              \
    { /* mode 4 */                                                      \
      int m[30]={0,3,6,0,3,6,0,3,6,0,3,6,0,3,6,0,3,6,0,3,6,0,3,6,0,3,6,0,3,6}; \
      int idx[90]={0,1,2,0,1,2,0,1,2,1,3,4,1,3,4,1,3,4,2,4,5,2,4,5,2,4,5,3,6,7,3,6,7,3,6,7,4,7,8,4,7,8,4,7,8,5,8,9,5,8,9,5,8,9,6,10,11,6,10,11,6,10,11,7,11,12,7,11,12,7,11,12,8,12,13,8,12,13,8,12,13,9,13,14,9,13,14,9,13,14}; \
      for (i=0; i<30; i++)                                              \
        tmps[i]=M[m[i]]*A[idx[3*i]]+                                    \
          M[m[i]+1]*A[idx[3*i+1]]+                                      \
          M[m[i]+2]*A[idx[3*i+2]];                                      \
    }                                                                   \
    { /* mode 3 */                                                      \
      int m[36]={0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6}; \
      int idx[108]={0,3,6,1,4,7,2,5,8,1,4,7,2,5,8,2,5,8,3,9,12,4,10,13,5,11,14,4,10,13,5,11,14,5,11,14,6,12,15,7,13,16,8,14,17,7,13,16,8,14,17,8,14,17,9,18,21,10,19,22,11,20,23,10,19,22,11,20,23,11,20,23,12,21,24,13,22,25,14,23,26,13,22,25,14,23,26,14,23,26,15,24,27,16,25,28,17,26,29,16,25,28,17,26,29,17,26,29}; \
      for (i=0; i<36; i++)                                              \
        tmpl[i]=M[m[i]]*tmps[idx[3*i]]+                                 \
          M[m[i]+1]*tmps[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmps[idx[3*i+2]];                                   \
    }                                                                   \
    { /* mode 2 */                                                      \
      int m[30]={0,0,0,0,0,0,3,3,3,6,0,0,0,0,0,0,3,3,3,6,0,0,0,0,0,0,3,3,3,6}; \
      int idx[90]={0,6,12,1,7,13,2,8,14,3,9,15,4,10,16,5,11,17,3,9,15,4,10,16,5,11,17,5,11,17,6,18,24,7,19,25,8,20,26,9,21,27,10,22,28,11,23,29,9,21,27,10,22,28,11,23,29,11,23,29,12,24,30,13,25,31,14,26,32,15,27,33,16,28,34,17,29,35,15,27,33,16,28,34,17,29,35,17,29,35}; \
      for (i=0; i<30; i++)                                              \
        tmps[i]=M[m[i]]*tmpl[idx[3*i]]+                                 \
          M[m[i]+1]*tmpl[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpl[idx[3*i+2]];                                   \
    }                                                                   \
    { /* mode 1 */                                                      \
      int m[15]={0,0,0,0,0,0,0,0,0,0,3,3,3,3,6};                        \
      int idx[45]={0,10,20,1,11,21,2,12,22,3,13,23,4,14,24,5,15,25,6,16,26,7,17,27,8,18,28,9,19,29,6,16,26,7,17,27,8,18,28,9,19,29,9,19,29}; \
      for (i=0; i<15; i++)                                              \
        res[i]=M[m[i]]*tmps[idx[3*i]]+                                  \
          M[m[i]+1]*tmps[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmps[idx[3*i+2]];                                   \
    }                                                                   \
  }

_TIJK_4O3D_SYM_TRANS(double, d)
_TIJK_4O3D_SYM_TRANS(float, f)

#define _TIJK_4O3D_SYM_CONVERT(TYPE, SUF)                             \
  int                                                                 \
  _tijk_4o3d_sym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                const TYPE *A) {                      \
    if (res_type==tijk_4o3d_sym) { /* copy over */                    \
      tijk_copy_##SUF(res, A, tijk_4o3d_sym);                         \
      return 0;                                                       \
    } else if (res_type==tijk_6o3d_sym) {                             \
      /* do this by going to SH and zero-padding */                   \
      TYPE tmp[28];                                                   \
      memset(tmp,0,sizeof(tmp));                                      \
      tijk_3d_sym_to_esh_##SUF (tmp, A, tijk_4o3d_sym);               \
      tijk_esh_to_3d_sym_##SUF (res, tmp, res_type->order);           \
      return 0;                                                       \
    } else if (NULL!=res_type->_convert_from_##SUF)                   \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_4o3d_sym);   \
    return 1;                                                         \
  }

_TIJK_4O3D_SYM_CONVERT(double, d)
_TIJK_4O3D_SYM_CONVERT(float, f)

#define _TIJK_4O3D_SYM_APPROX(TYPE, SUF)                             \
  int                                                                \
  _tijk_4o3d_sym_approx_##SUF (TYPE *res, const tijk_type *res_type, \
                               const TYPE *A){                       \
    if (res_type==tijk_2o3d_sym) {                                   \
      res[0]=3.0/35.0*(9*A[0]+8*A[3]+8*A[5]-A[10]-A[14]-2*A[12]);    \
      res[1]=6.0/7.0*(A[1]+A[6]+A[8]);                               \
      res[2]=6.0/7.0*(A[2]+A[9]+A[7]);                               \
      res[3]=3.0/35.0*(9*A[10]+8*A[3]+8*A[12]-A[0]-A[14]-2*A[5]);    \
      res[4]=6.0/7.0*(A[11]+A[13]+A[4]);                             \
      res[5]=3.0/35.0*(9*A[14]+8*A[5]+8*A[12]-A[0]-A[10]-2*A[3]);    \
      return 0;                                                      \
    } else if (NULL!=res_type->_approx_from_##SUF)                   \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_4o3d_sym);   \
    else                                                             \
      return 1;                                                      \
  }

_TIJK_4O3D_SYM_APPROX(double, d)
_TIJK_4O3D_SYM_APPROX(float, f)

double
_tijk_4o3d_sym_s_form_d (const double *A, const double *v) {
  double v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],
    v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];
  return A[0]*v00*v00+4*A[1]*v00*v01+4*A[2]*v00*v02+6*A[3]*v00*v11+
    12*A[4]*v00*v12+6*A[5]*v00*v22+4*A[6]*v01*v11+12*A[7]*v01*v12+
    12*A[8]*v01*v22+4*A[9]*v02*v22+A[10]*v11*v11+4*A[11]*v11*v12+
    6*A[12]*v11*v22+4*A[13]*v12*v22+A[14]*v22*v22;
}

float
_tijk_4o3d_sym_s_form_f (const float *A, const float *v) {
  float v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],
    v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];
  return A[0]*v00*v00+4*A[1]*v00*v01+4*A[2]*v00*v02+6*A[3]*v00*v11+
    12*A[4]*v00*v12+6*A[5]*v00*v22+4*A[6]*v01*v11+12*A[7]*v01*v12+
    12*A[8]*v01*v22+4*A[9]*v02*v22+A[10]*v11*v11+4*A[11]*v11*v12+
    6*A[12]*v11*v22+4*A[13]*v12*v22+A[14]*v22*v22;
}

double
_tijk_4o3d_sym_mean_d (const double *A) {
  return 0.2*(A[0]+A[10]+A[14]+2*(A[3]+A[5]+A[12]));
}

float
_tijk_4o3d_sym_mean_f (const float *A) {
  return 0.2f*(A[0]+A[10]+A[14]+2.0f*(A[3]+A[5]+A[12]));
}

double
_tijk_4o3d_sym_var_d (const double *A) {
  /* numerical result taken from MATHEMATICA */
  return 0.0795775*(-1.5319*(A[14]*A[3]+A[10]*A[5]+A[0]*A[12])
                    -1.14893*(A[12]*A[3]+A[12]*A[5]+A[3]*A[5])
                    -0.76595*(A[10]*A[14]+A[10]*A[0]+A[0]*A[14])
                    +0.382975*(A[10]*A[12]+A[12]*A[14]+A[10]*A[3]+A[14]*A[5]+
                               A[0]*A[3]+A[0]*A[5])
                    +0.893609*(A[0]*A[0]+A[10]*A[10]+A[14]*A[14])
                    +2.29785*(A[12]*A[12] + A[3]*A[3] + A[5]*A[5])
                    +3.19146*(A[1]*A[1]+A[2]*A[2]+A[11]*A[11]+A[13]*A[13]+
                              A[6]*A[6]+A[9]*A[9])
                    +3.82975*(A[11]*A[13]+A[11]*A[4]+A[13]*A[4]+A[1]*A[6]+
                              A[2]*A[7]+A[1]*A[8]+A[6]*A[8]+A[2]*A[9]+
                              A[7]*A[9])
                    +5.74463*(A[4]*A[4]+A[7]*A[7]+A[8]*A[8]));
}

float
_tijk_4o3d_sym_var_f (const float *A) {
  /* numerical result taken from MATHEMATICA */
  return 0.0795775*(-1.5319*(A[14]*A[3]+A[10]*A[5]+A[0]*A[12])
                    -1.14893*(A[12]*A[3]+A[12]*A[5]+A[3]*A[5])
                    -0.76595*(A[10]*A[14]+A[10]*A[0]+A[0]*A[14])
                    +0.382975*(A[10]*A[12]+A[12]*A[14]+A[10]*A[3]+A[14]*A[5]+
                               A[0]*A[3]+A[0]*A[5])
                    +0.893609*(A[0]*A[0]+A[10]*A[10]+A[14]*A[14])
                    +2.29785*(A[12]*A[12] + A[3]*A[3] + A[5]*A[5])
                    +3.19146*(A[1]*A[1]+A[2]*A[2]+A[11]*A[11]+A[13]*A[13]+
                              A[6]*A[6]+A[9]*A[9])
                    +3.82975*(A[11]*A[13]+A[11]*A[4]+A[13]*A[4]+A[1]*A[6]+
                              A[2]*A[7]+A[1]*A[8]+A[6]*A[8]+A[2]*A[9]+
                              A[7]*A[9])
                    +5.74463*(A[4]*A[4]+A[7]*A[7]+A[8]*A[8]));
}

void
_tijk_4o3d_sym_v_form_d (double *res, const double *A, const double *v) {
  double v000=v[0]*v[0]*v[0], v001=v[0]*v[0]*v[1], v002=v[0]*v[0]*v[2],
    v011=v[0]*v[1]*v[1], v012=v[0]*v[1]*v[2], v022=v[0]*v[2]*v[2],
    v111=v[1]*v[1]*v[1], v112=v[1]*v[1]*v[2], v122=v[1]*v[2]*v[2],
    v222=v[2]*v[2]*v[2];
  res[0] = A[0]*v000+A[6]*v111+A[9]*v222+6*A[4]*v012+
    3*(A[1]*v001+A[2]*v002+A[3]*v011+A[5]*v022+A[7]*v112+A[8]*v122);
  res[1] = A[1]*v000+A[10]*v111+A[13]*v222+6*A[7]*v012+
    3*(A[3]*v001+A[4]*v002+A[6]*v011+A[8]*v022+A[11]*v112+A[12]*v122);
  res[2] = A[2]*v000+A[11]*v111+A[14]*v222+6*A[8]*v012+
    3*(A[4]*v001+A[5]*v002+A[7]*v011+A[9]*v022+A[12]*v112+A[13]*v122);
}

void
_tijk_4o3d_sym_v_form_f (float *res, const float *A, const float *v) {
  float v000=v[0]*v[0]*v[0], v001=v[0]*v[0]*v[1], v002=v[0]*v[0]*v[2],
    v011=v[0]*v[1]*v[1], v012=v[0]*v[1]*v[2], v022=v[0]*v[2]*v[2],
    v111=v[1]*v[1]*v[1], v112=v[1]*v[1]*v[2], v122=v[1]*v[2]*v[2],
    v222=v[2]*v[2]*v[2];
  res[0] = A[0]*v000+A[6]*v111+A[9]*v222+6*A[4]*v012+
    3*(A[1]*v001+A[2]*v002+A[3]*v011+A[5]*v022+A[7]*v112+A[8]*v122);
  res[1] = A[1]*v000+A[10]*v111+A[13]*v222+6*A[7]*v012+
    3*(A[3]*v001+A[4]*v002+A[6]*v011+A[8]*v022+A[11]*v112+A[12]*v122);
  res[2] = A[2]*v000+A[11]*v111+A[14]*v222+6*A[8]*v012+
    3*(A[4]*v001+A[5]*v002+A[7]*v011+A[9]*v022+A[12]*v112+A[13]*v122);
}

void
_tijk_4o3d_sym_m_form_d (double *res, const double *A, const double *v) {
  double v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],
    v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];
  res[0]=A[0]*v00+A[3]*v11+A[5]*v22+2*(A[1]*v01+A[2]*v02+A[4]*v12);
  res[1]=A[1]*v00+A[6]*v11+A[8]*v22+2*(A[3]*v01+A[4]*v02+A[7]*v12);
  res[2]=A[2]*v00+A[7]*v11+A[9]*v22+2*(A[4]*v01+A[5]*v02+A[8]*v12);
  res[3]=A[3]*v00+A[10]*v11+A[12]*v22+2*(A[6]*v01+A[7]*v02+A[11]*v12);
  res[4]=A[4]*v00+A[11]*v11+A[13]*v22+2*(A[7]*v01+A[8]*v02+A[12]*v12);
  res[5]=A[5]*v00+A[12]*v11+A[14]*v22+2*(A[8]*v01+A[9]*v02+A[13]*v12);
}

void
_tijk_4o3d_sym_m_form_f (float *res, const float *A, const float *v) {
  float v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],
    v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];
  res[0]=A[0]*v00+A[3]*v11+A[5]*v22+2*(A[1]*v01+A[2]*v02+A[4]*v12);
  res[1]=A[1]*v00+A[6]*v11+A[8]*v22+2*(A[3]*v01+A[4]*v02+A[7]*v12);
  res[2]=A[2]*v00+A[7]*v11+A[9]*v22+2*(A[4]*v01+A[5]*v02+A[8]*v12);
  res[3]=A[3]*v00+A[10]*v11+A[12]*v22+2*(A[6]*v01+A[7]*v02+A[11]*v12);
  res[4]=A[4]*v00+A[11]*v11+A[13]*v22+2*(A[7]*v01+A[8]*v02+A[12]*v12);
  res[5]=A[5]*v00+A[12]*v11+A[14]*v22+2*(A[8]*v01+A[9]*v02+A[13]*v12);
}

void
_tijk_4o3d_sym_make_rank1_d (double *res, const double s, const double *v) {
  double v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],
    v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];
  res[0]=s*v00*v00; res[1]=s*v00*v01; res[2]=s*v00*v02; res[3]=s*v00*v11;
  res[4]=s*v00*v12; res[5]=s*v00*v22; res[6]=s*v01*v11; res[7]=s*v01*v12;
  res[8]=s*v01*v22; res[9]=s*v02*v22; res[10]=s*v11*v11; res[11]=s*v11*v12;
  res[12]=s*v11*v22; res[13]=s*v12*v22; res[14]=s*v22*v22;
}

void
_tijk_4o3d_sym_make_rank1_f (float *res, const float s, const float *v) {
  float v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],
    v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];
  res[0]=s*v00*v00; res[1]=s*v00*v01; res[2]=s*v00*v02; res[3]=s*v00*v11;
  res[4]=s*v00*v12; res[5]=s*v00*v22; res[6]=s*v01*v11; res[7]=s*v01*v12;
  res[8]=s*v01*v22; res[9]=s*v02*v22; res[10]=s*v11*v11; res[11]=s*v11*v12;
  res[12]=s*v11*v22; res[13]=s*v12*v22; res[14]=s*v22*v22;
}

void
_tijk_4o3d_sym_make_iso_d (double *res, const double s) {
  res[0]=res[10]=res[14]=s;
  res[3]=res[5]=res[12]=s/3.0;
  res[1]=res[2]=res[4]=res[6]=res[7]=res[8]=res[9]=res[11]=res[13]=0.0;
}

void
_tijk_4o3d_sym_make_iso_f (float *res, const float s) {
  res[0]=res[10]=res[14]=s;
  res[3]=res[5]=res[12]=s/3.0f;
  res[1]=res[2]=res[4]=res[6]=res[7]=res[8]=res[9]=res[11]=res[13]=0.0f;
}

void
_tijk_4o3d_sym_grad_d (double *res, const double *A, const double *v) {
  double proj, projv[3];
  _tijk_4o3d_sym_v_form_d (res, A, v);
  ELL_3V_SCALE(res,4.0,res);
  proj=ELL_3V_DOT(res,v);
  ELL_3V_SCALE(projv,-proj,v);
  ELL_3V_INCR(res,projv);
}

void
_tijk_4o3d_sym_grad_f (float *res, const float *A, const float *v) {
  float proj, projv[3];
  _tijk_4o3d_sym_v_form_f (res, A, v);
  ELL_3V_SCALE(res,4.0,res);
  proj=ELL_3V_DOT(res,v);
  ELL_3V_SCALE(projv,-proj,v);
  ELL_3V_INCR(res,projv);
}

#define _TIJK_4O3D_SYM_HESS(TYPE, SUF)                                  \
  void                                                                  \
  _tijk_4o3d_sym_hess_##SUF (TYPE *res, const TYPE *A, const TYPE *v) { \
    /* get two orthonormal tangents */                                  \
    TYPE t[2][3], cv[2][3], h[6], der, norm, tmp[6];                    \
    int r,c;                                                            \
    ell_3v_perp_##SUF(t[0], v);                                         \
    ELL_3V_NORM(t[0],t[0],norm);                                        \
    ELL_3V_CROSS(t[1],v,t[0]);                                          \
    ELL_3V_NORM(t[1],t[1],norm);                                        \
    /* compute Hessian w.r.t. t1/t2 */                                  \
    _tijk_4o3d_sym_m_form_##SUF(h, A, v);                               \
    der=4*_tijk_4o3d_sym_s_form_##SUF(A, v); /* first der in direction v*/ \
    _tijk_2o3d_sym_v_form_##SUF(cv[0],h,t[0]);                          \
    _tijk_2o3d_sym_v_form_##SUF(cv[1],h,t[1]);                          \
    h[0]=12*ELL_3V_DOT(cv[0],t[0])-der;                                 \
    h[1]=12*ELL_3V_DOT(cv[0],t[1]);                                     \
    h[2]=h[1];                                                          \
    h[3]=12*ELL_3V_DOT(cv[1],t[1])-der;                                 \
    /* now turn this into a symmetric order-2 rank-2 3D tensor */       \
    for (r=0; r<2; r++) {                                               \
      for (c=0; c<3; c++) {                                             \
        tmp[3*r+c]=h[2*r]*t[0][c]+h[2*r+1]*t[1][c];                     \
      }                                                                 \
    }                                                                   \
    res[0]=t[0][0]*tmp[0]+t[1][0]*tmp[3];                               \
    res[1]=t[0][0]*tmp[1]+t[1][0]*tmp[4];                               \
    res[2]=t[0][0]*tmp[2]+t[1][0]*tmp[5];                               \
    res[3]=t[0][1]*tmp[1]+t[1][1]*tmp[4];                               \
    res[4]=t[0][1]*tmp[2]+t[1][1]*tmp[5];                               \
    res[5]=t[0][2]*tmp[2]+t[1][2]*tmp[5];                               \
  }

_TIJK_4O3D_SYM_HESS(double,d)
_TIJK_4O3D_SYM_HESS(float,f)

TIJK_TYPE_SYM(4o3d_sym, 4, 3, 15)

/* 6th order 3D symmetric */
/* (unsymmetric counterpart currently not implemented) */

unsigned int _tijk_6o3d_sym_mult[28]={1,6,6,15,30,15,20,60,60,20,15,60,90,
                                      60,15,6,30,60,60,30,6,1,6,15,20,15,6,1};
#define _tijk_6o3d_sym_unsym2uniq NULL
#define _tijk_6o3d_sym_uniq2unsym NULL
#define _tijk_6o3d_sym_uniq_idx NULL

#define _TIJK_6O3D_SYM_TSP(A, B)                                    \
  ((A)[0]*(B)[0]+(A)[21]*(B)[21]+(A)[27]*(B)[27]+                   \
   6*((A)[1]*(B)[1]+(A)[2]*(B)[2]+(A)[15]*(B)[15]+(A)[20]*(B)[20]+  \
      (A)[22]*(B)[22]+(A)[26]*(B)[26])+                             \
   15*((A)[3]*(B)[3]+(A)[5]*(B)[5]+(A)[10]*(B)[10]+(A)[14]*(B)[14]+ \
       (A)[23]*(B)[23]+(A)[25]*(B)[25])+                            \
   30*((A)[4]*(B)[4]+(A)[16]*(B)[16]+(A)[19]*(B)[19])+              \
   20*((A)[6]*(B)[6]+(A)[9]*(B)[9]+(A)[24]*(B)[24])+                \
   60*((A)[7]*(B)[7]+(A)[8]*(B)[8]+(A)[11]*(B)[11]+                 \
       (A)[13]*(B)[13]+(A)[17]*(B)[17]+(A)[18]*(B)[18])+            \
   90*(A)[12]*(B)[12])                                              \

double
_tijk_6o3d_sym_tsp_d (const double *A, const double *B) {
  return _TIJK_6O3D_SYM_TSP(A,B);
}

float
_tijk_6o3d_sym_tsp_f (const float *A, const float *B) {
  return _TIJK_6O3D_SYM_TSP(A,B);
}

double
_tijk_6o3d_sym_norm_d (const double *A) {
  return sqrt(_TIJK_6O3D_SYM_TSP(A,A));
}

float
_tijk_6o3d_sym_norm_f (const float *A) {
  return sqrt(_TIJK_6O3D_SYM_TSP(A,A));
}

#define _TIJK_6O3D_SYM_TRANS(TYPE, SUF)                                 \
  void                                                                  \
  _tijk_6o3d_sym_trans_##SUF (TYPE *res, const TYPE *A, const TYPE *M) { \
    /* Tijklmn = Mio Mjp Mkq Mlr Mms Mnt Topqrst                        \
     * For efficiency, we transform mode by mode; the intermediate results \
     * have incomplete symmetries! */                                   \
    TYPE tmpl[100], tmpr[100];                                          \
    int i;                                                              \
    { /* mode 6 */                                                      \
      int m[3]={0,3,6};                                                 \
      int idx[189]={0,1,2,0,1,2,0,1,2,1,3,4,1,3,4,1,3,4,2,4,5,2,4,5,2,4,5,3,6,7,3,6,7,3,6,7,4,7,8,4,7,8,4,7,8,5,8,9,5,8,9,5,8,9,6,10,11,6,10,11,6,10,11,7,11,12,7,11,12,7,11,12,8,12,13,8,12,13,8,12,13,9,13,14,9,13,14,9,13,14,10,15,16,10,15,16,10,15,16,11,16,17,11,16,17,11,16,17,12,17,18,12,17,18,12,17,18,13,18,19,13,18,19,13,18,19,14,19,20,14,19,20,14,19,20,15,21,22,15,21,22,15,21,22,16,22,23,16,22,23,16,22,23,17,23,24,17,23,24,17,23,24,18,24,25,18,24,25,18,24,25,19,25,26,19,25,26,19,25,26,20,26,27,20,26,27,20,26,27}; \
      for (i=0; i<63; i++)                                              \
        tmpl[i]=M[m[i%3]]*A[idx[3*i]]+                                  \
          M[m[i%3]+1]*A[idx[3*i+1]]+                                    \
          M[m[i%3]+2]*A[idx[3*i+2]];                                    \
    }                                                                   \
    { /* mode 5 */                                                      \
      int m[90]={0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6,0,0,0,3,3,6}; \
      int idx[270]={0,3,6,1,4,7,2,5,8,1,4,7,2,5,8,2,5,8,3,9,12,4,10,13,5,11,14,4,10,13,5,11,14,5,11,14,6,12,15,7,13,16,8,14,17,7,13,16,8,14,17,8,14,17,9,18,21,10,19,22,11,20,23,10,19,22,11,20,23,11,20,23,12,21,24,13,22,25,14,23,26,13,22,25,14,23,26,14,23,26,15,24,27,16,25,28,17,26,29,16,25,28,17,26,29,17,26,29,18,30,33,19,31,34,20,32,35,19,31,34,20,32,35,20,32,35,21,33,36,22,34,37,23,35,38,22,34,37,23,35,38,23,35,38,24,36,39,25,37,40,26,38,41,25,37,40,26,38,41,26,38,41,27,39,42,28,40,43,29,41,44,28,40,43,29,41,44,29,41,44,30,45,48,31,46,49,32,47,50,31,46,49,32,47,50,32,47,50,33,48,51,34,49,52,35,50,53,34,49,52,35,50,53,35,50,53,36,51,54,37,52,55,38,53,56,37,52,55,38,53,56,38,53,56,39,54,57,40,55,58,41,56,59,40,55,58,41,56,59,41,56,59,42,57,60,43,58,61,44,59,62,43,58,61,44,59,62,44,59,62}; \
      for (i=0; i<90; i++)                                              \
        tmpr[i]=M[m[i]]*tmpl[idx[3*i]]+                                 \
          M[m[i]+1]*tmpl[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpl[idx[3*i+2]];                                   \
    }                                                                   \
    { /* mode 4 */                                                      \
      int m[100]={0,0,0,0,0,0,3,3,3,6,0,0,0,0,0,0,3,3,3,6,0,0,0,0,0,0,3,3,3,6,0,0,0,0,0,0,3,3,3,6,0,0,0,0,0,0,3,3,3,6,0,0,0,0,0,0,3,3,3,6,0,0,0,0,0,0,3,3,3,6,0,0,0,0,0,0,3,3,3,6,0,0,0,0,0,0,3,3,3,6,0,0,0,0,0,0,3,3,3,6}; \
      int idx[300]={0,6,12,1,7,13,2,8,14,3,9,15,4,10,16,5,11,17,3,9,15,4,10,16,5,11,17,5,11,17,6,18,24,7,19,25,8,20,26,9,21,27,10,22,28,11,23,29,9,21,27,10,22,28,11,23,29,11,23,29,12,24,30,13,25,31,14,26,32,15,27,33,16,28,34,17,29,35,15,27,33,16,28,34,17,29,35,17,29,35,18,36,42,19,37,43,20,38,44,21,39,45,22,40,46,23,41,47,21,39,45,22,40,46,23,41,47,23,41,47,24,42,48,25,43,49,26,44,50,27,45,51,28,46,52,29,47,53,27,45,51,28,46,52,29,47,53,29,47,53,30,48,54,31,49,55,32,50,56,33,51,57,34,52,58,35,53,59,33,51,57,34,52,58,35,53,59,35,53,59,36,60,66,37,61,67,38,62,68,39,63,69,40,64,70,41,65,71,39,63,69,40,64,70,41,65,71,41,65,71,42,66,72,43,67,73,44,68,74,45,69,75,46,70,76,47,71,77,45,69,75,46,70,76,47,71,77,47,71,77,48,72,78,49,73,79,50,74,80,51,75,81,52,76,82,53,77,83,51,75,81,52,76,82,53,77,83,53,77,83,54,78,84,55,79,85,56,80,86,57,81,87,58,82,88,59,83,89,57,81,87,58,82,88,59,83,89,59,83,89}; \
      for (i=0; i<100; i++)                                             \
        tmpl[i]=M[m[i]]*tmpr[idx[3*i]]+                                 \
          M[m[i]+1]*tmpr[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpr[idx[3*i+2]];                                   \
    }                                                                   \
    { /* mode 3 */                                                      \
      int m[90]={0,0,0,0,0,0,0,0,0,0,3,3,3,3,6,0,0,0,0,0,0,0,0,0,0,3,3,3,3,6,0,0,0,0,0,0,0,0,0,0,3,3,3,3,6,0,0,0,0,0,0,0,0,0,0,3,3,3,3,6,0,0,0,0,0,0,0,0,0,0,3,3,3,3,6,0,0,0,0,0,0,0,0,0,0,3,3,3,3,6}; \
      int idx[270]={0,10,20,1,11,21,2,12,22,3,13,23,4,14,24,5,15,25,6,16,26,7,17,27,8,18,28,9,19,29,6,16,26,7,17,27,8,18,28,9,19,29,9,19,29,10,30,40,11,31,41,12,32,42,13,33,43,14,34,44,15,35,45,16,36,46,17,37,47,18,38,48,19,39,49,16,36,46,17,37,47,18,38,48,19,39,49,19,39,49,20,40,50,21,41,51,22,42,52,23,43,53,24,44,54,25,45,55,26,46,56,27,47,57,28,48,58,29,49,59,26,46,56,27,47,57,28,48,58,29,49,59,29,49,59,30,60,70,31,61,71,32,62,72,33,63,73,34,64,74,35,65,75,36,66,76,37,67,77,38,68,78,39,69,79,36,66,76,37,67,77,38,68,78,39,69,79,39,69,79,40,70,80,41,71,81,42,72,82,43,73,83,44,74,84,45,75,85,46,76,86,47,77,87,48,78,88,49,79,89,46,76,86,47,77,87,48,78,88,49,79,89,49,79,89,50,80,90,51,81,91,52,82,92,53,83,93,54,84,94,55,85,95,56,86,96,57,87,97,58,88,98,59,89,99,56,86,96,57,87,97,58,88,98,59,89,99,59,89,99}; \
      for (i=0; i<90; i++)                                              \
        tmpr[i]=M[m[i]]*tmpl[idx[3*i]]+                                 \
          M[m[i]+1]*tmpl[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpl[idx[3*i+2]];                                   \
    }                                                                   \
    { /* mode 2 */                                                      \
      int m[63]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,6}; \
      int idx[189]={0,15,30,1,16,31,2,17,32,3,18,33,4,19,34,5,20,35,6,21,36,7,22,37,8,23,38,9,24,39,10,25,40,11,26,41,12,27,42,13,28,43,14,29,44,10,25,40,11,26,41,12,27,42,13,28,43,14,29,44,14,29,44,15,45,60,16,46,61,17,47,62,18,48,63,19,49,64,20,50,65,21,51,66,22,52,67,23,53,68,24,54,69,25,55,70,26,56,71,27,57,72,28,58,73,29,59,74,25,55,70,26,56,71,27,57,72,28,58,73,29,59,74,29,59,74,30,60,75,31,61,76,32,62,77,33,63,78,34,64,79,35,65,80,36,66,81,37,67,82,38,68,83,39,69,84,40,70,85,41,71,86,42,72,87,43,73,88,44,74,89,40,70,85,41,71,86,42,72,87,43,73,88,44,74,89,44,74,89}; \
      for (i=0; i<63; i++)                                              \
        tmpl[i]=M[m[i]]*tmpr[idx[3*i]]+                                 \
          M[m[i]+1]*tmpr[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpr[idx[3*i+2]];                                   \
    }                                                                   \
    { /* mode 1 */                                                      \
      int m[28]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,6}; \
      int idx[84]={0,21,42,1,22,43,2,23,44,3,24,45,4,25,46,5,26,47,6,27,48,7,28,49,8,29,50,9,30,51,10,31,52,11,32,53,12,33,54,13,34,55,14,35,56,15,36,57,16,37,58,17,38,59,18,39,60,19,40,61,20,41,62,15,36,57,16,37,58,17,38,59,18,39,60,19,40,61,20,41,62,20,41,62}; \
      for (i=0; i<28; i++)                                              \
        res[i]=M[m[i]]*tmpl[idx[3*i]]+                                  \
          M[m[i]+1]*tmpl[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpl[idx[3*i+2]];                                   \
    }                                                                   \
  }

_TIJK_6O3D_SYM_TRANS(double, d)
_TIJK_6O3D_SYM_TRANS(float, f)

#define _TIJK_6O3D_SYM_CONVERT(TYPE, SUF)                             \
  int                                                                 \
  _tijk_6o3d_sym_convert_##SUF (TYPE *res, const tijk_type *res_type, \
                                const TYPE *A) {                      \
    if (res_type==tijk_6o3d_sym) { /* copy over */                    \
      tijk_copy_##SUF(res, A, tijk_6o3d_sym);                         \
      return 0;                                                       \
    } else if (NULL!=res_type->_convert_from_##SUF)                   \
      return (*res_type->_convert_from_##SUF)(res,A,tijk_6o3d_sym);   \
    return 1;                                                         \
  }

_TIJK_6O3D_SYM_CONVERT(double, d)
_TIJK_6O3D_SYM_CONVERT(float, f)

#define _TIJK_6O3D_SYM_APPROX(TYPE, SUF)                                \
  int                                                                   \
  _tijk_6o3d_sym_approx_##SUF (TYPE *res, const tijk_type *res_type,    \
                               const TYPE *A){                          \
    if (res_type==tijk_2o3d_sym) {                                      \
      /* do this in two steps */                                        \
      TYPE tmp[15];                                                     \
      _tijk_6o3d_sym_approx_##SUF(tmp, tijk_4o3d_sym, A);               \
      _tijk_4o3d_sym_approx_##SUF(res, tijk_2o3d_sym, tmp);             \
      return 0;                                                         \
    } else if (res_type==tijk_4o3d_sym) {                               \
      res[0]=5.0/231.0*(43*A[0]+A[21]+A[27]+24*(A[3]+A[5])+             \
                        3*(A[23]+A[25])-18*(A[10]+A[14])-36*A[12]);     \
      res[1]=5.0/22.0*(5*A[1]+4*(A[6]+A[8])-A[15]-A[19]-2*A[17]);       \
      res[2]=5.0/22.0*(5*A[2]+4*(A[9]+A[7])-A[20]-A[16]-2*A[18]);       \
      res[3]=5.0/1386.0*(321*(A[3]+A[10])+306*A[12]-36*(A[5]+A[23])-    \
                         19*(A[0]+A[21])-15*(A[14]+A[25])+2*A[27]);     \
      res[4]=5.0/66.0*(17*A[4]+16*(A[11]+A[13])-A[22]-A[26]-2*A[24]);   \
      res[5]=5.0/1386.0*(321*(A[5]+A[14])+306*A[12]-36*(A[3]+A[25])-    \
                         19*(A[0]+A[27])-15*(A[10]+A[23])+2*A[21]);     \
      res[6]=5.0/22.0*(5*A[15]+4*(A[6]+A[17])-A[1]-A[19]-2*A[8]);       \
      res[7]=5.0/66.0*(17*A[16]+16*(A[7]+A[18])-A[2]-A[20]-2*A[9]);     \
      res[8]=5.0/66.0*(17*A[19]+16*(A[8]+A[17])-A[1]-A[15]-2*A[6]);     \
      res[9]=5.0/22.0*(5*A[20]+4*(A[9]+A[18])-A[2]-A[16]-2*A[7]);       \
      res[10]=5.0/231.0*(43*A[21]+A[0]+A[27]+24*(A[10]+A[23])+          \
                         3*(A[5]+A[14])-18*(A[3]+A[25])-36*A[12]);      \
      res[11]=5.0/22.0*(5*A[22]+4*(A[23]+A[11])-A[26]-A[4]-2*A[13]);    \
      res[12]=5.0/1386.0*(312*(A[23]+A[25])+306*A[12]-36*(A[10]+A[14])- \
                          19*(A[21]+A[27])-15*(A[5]-A[3])+2*A[0]);      \
      res[13]=5.0/22.0*(5*A[26]+4*(A[24]+A[13])-A[22]-A[4]-2*A[11]);    \
      res[14]=5.0/231.0*(43*A[27]+A[0]+A[21]+24*(A[14]+A[25])+          \
                         3*(A[3]+A[10])-18*(A[5]+A[23])-36*A[12]);      \
      return 0;                                                         \
    } else if (NULL!=res_type->_approx_from_##SUF)                      \
      return (*res_type->_approx_from_##SUF)(res,A,tijk_6o3d_sym);      \
    return 1;                                                           \
  }

_TIJK_6O3D_SYM_APPROX(double, d)
_TIJK_6O3D_SYM_APPROX(float, f)

double
_tijk_6o3d_sym_s_form_d (const double *A, const double *v) {
  double v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],
    v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];
  return A[0]*v00*v00*v00+
    A[21]*v11*v11*v11+
    A[27]*v22*v22*v22+
    6*(A[1]*v00*v00*v01+
       A[2]*v00*v00*v02+
       A[15]*v01*v11*v11+
       A[20]*v02*v22*v22+
       A[22]*v11*v11*v12+
       A[26]*v12*v22*v22)+
    15*(A[3]*v00*v00*v11+
        A[5]*v00*v00*v22+
        A[10]*v00*v11*v11+
        A[14]*v00*v22*v22+
        A[23]*v11*v11*v22+
        A[25]*v11*v22*v22)+
    30*(A[4]*v00*v00*v12+
        A[16]*v01*v11*v12+
        A[19]*v01*v22*v22)+
    20*(A[6]*v00*v01*v11+
        A[9]*v00*v02*v22+
        A[24]*v11*v12*v22)+
    60*(A[7]*v00*v01*v12+
        A[8]*v00*v01*v22+
        A[11]*v00*v11*v12+
        A[13]*v00*v12*v22+
        A[17]*v01*v11*v22+
        A[18]*v01*v12*v22)+
    90*A[12]*v00*v11*v22;
}

float
_tijk_6o3d_sym_s_form_f (const float *A, const float *v) {
  float v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],
    v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];
  return A[0]*v00*v00*v00+
    A[21]*v11*v11*v11+
    A[27]*v22*v22*v22+
    6*(A[1]*v00*v00*v01+
       A[2]*v00*v00*v02+
       A[15]*v01*v11*v11+
       A[20]*v02*v22*v22+
       A[22]*v11*v11*v12+
       A[26]*v12*v22*v22)+
    15*(A[3]*v00*v00*v11+
        A[5]*v00*v00*v22+
        A[10]*v00*v11*v11+
        A[14]*v00*v22*v22+
        A[23]*v11*v11*v22+
        A[25]*v11*v22*v22)+
    30*(A[4]*v00*v00*v12+
        A[16]*v01*v11*v12+
        A[19]*v01*v22*v22)+
    20*(A[6]*v00*v01*v11+
        A[9]*v00*v02*v22+
        A[24]*v11*v12*v22)+
    60*(A[7]*v00*v01*v12+
        A[8]*v00*v01*v22+
        A[11]*v00*v11*v12+
        A[13]*v00*v12*v22+
        A[17]*v01*v11*v22+
        A[18]*v01*v12*v22)+
    90*A[12]*v00*v11*v22;
}

double
_tijk_6o3d_sym_mean_d (const double *A) {
  return (A[0]+A[21]+A[27]+
          3*(A[3]+A[5]+A[10]+A[14]+A[23]+A[25])+
          6*A[12])/7.0;
}

float
_tijk_6o3d_sym_mean_f (const float *A) {
  return (A[0]+A[21]+A[27]+
          3*(A[3]+A[5]+A[10]+A[14]+A[23]+A[25])+
          6*A[12])/7.0;
}

double
_tijk_6o3d_sym_var_d (const double *A) {
  double mean=(A[0]+A[21]+A[27]+
               3*(A[3]+A[5]+A[10]+A[14]+A[23]+A[25])+
               6*A[12])/7.0;
  return -mean*mean + (1.0/3003.0)*
    (+ 10*A[21]*A[27]
     + 30*(A[14]*A[21]+A[27]*A[3])
     + 210*(A[21]*(A[3]+A[25])+A[23]*A[27])
     + 231*(A[0]*A[0]+A[21]*A[21]+A[27]*A[27])
     + 270*A[25]*A[3]
     + 420*A[12]*(A[21]+A[27])
     + 450*(A[14]*(A[3]+A[23])+A[23]*A[3])
     + 630*(A[21]*A[23]+A[14]*A[27]+A[25]*A[27])
     + 1050*A[14]*A[25]
     + 1575*(A[3]*A[3]+A[5]*A[5]+A[10]*A[10]+A[14]*A[14]+
             A[23]*A[23]+A[25]*A[25])
     + 2250*A[23]*A[25]
     + 2700*A[12]*(A[3]+A[14]+A[23]+A[25])
     + 4860*A[12]*A[12]
     + 30*A[5]*(90*A[12]+ 75*A[14]+ A[21]+ 9*A[23]+ 15*A[25]+ 7*A[27]+ 35*A[3])
     + 30*A[10]*(90*A[12]+9*A[14]+21*A[21]+35*A[23]+15*A[25]+A[27]+75*A[3]+
                 15*A[5])
     + 10*A[0]*(21*A[10]+42*A[12]+21*A[14]+A[21]+3*A[23]+3*A[25]+A[27]+
                63*(A[3]+A[5]))
     + 4*(+30*A[1]*(3*A[15] + 6*A[17] + 3*A[19] + 14*(A[6] + A[8]))
          +60*(A[8]*(3*(A[15] + 6*A[17] + 5*A[19]) + 10*A[6])+
               A[9]*(3*A[16] + 10*A[18] + 7*(A[2] + A[20]) + 10*A[7])+
               A[11]*(18*A[13] + 7*A[22] + 10*A[24] + 3*A[26] + 15*A[4])+
               A[13]*(3*A[22] + 10*A[24] + 7*A[26] + 15*A[4])+
               A[18]*(3*A[2]+7*A[20]+18*A[7]))
          +90*(A[2]*A[20]+A[15]*A[19]+A[22]*A[26]+
               A[4]*(A[22]+2*A[24]+A[26])+
               A[16]*(10*A[18]+A[2]+A[20]+10*A[7]))
          +180*(A[20]*A[7]+A[19]*A[6])
          +189*(A[1]*A[1]+A[2]*A[2]+A[15]*A[15]+A[20]*A[20]+
                A[22]*A[22]+A[26]*A[26])
          +420*(A[2]*A[7]+A[15]*A[17]+A[22]*A[24]+A[24]*A[26]+A[15]*A[6])
          +500*(A[9]*A[9]+A[24]*A[24]+A[6]*A[6])
          +525*(A[4]*A[4]+A[16]*A[16]+A[19]*A[19])
          +600*A[17]*A[6]
          +900*(A[7]*A[7]+A[8]*A[8]+A[11]*A[11]+A[13]*A[13]+A[17]*A[17]+
                A[18]*A[18]+A[17]*A[19])));
}

float
_tijk_6o3d_sym_var_f (const float *A) {
  float mean=(A[0]+A[21]+A[27]+
              3*(A[3]+A[5]+A[10]+A[14]+A[23]+A[25])+
              6*A[12])/7.0;
  return -mean*mean + (1.0/3003.0)*
    (+ 10*A[21]*A[27]
     + 30*(A[14]*A[21]+A[27]*A[3])
     + 210*(A[21]*(A[3]+A[25])+A[23]*A[27])
     + 231*(A[0]*A[0]+A[21]*A[21]+A[27]*A[27])
     + 270*A[25]*A[3]
     + 420*A[12]*(A[21]+A[27])
     + 450*(A[14]*(A[3]+A[23])+A[23]*A[3])
     + 630*(A[21]*A[23]+A[14]*A[27]+A[25]*A[27])
     + 1050*A[14]*A[25]
     + 1575*(A[3]*A[3]+A[5]*A[5]+A[10]*A[10]+A[14]*A[14]+
             A[23]*A[23]+A[25]*A[25])
     + 2250*A[23]*A[25]
     + 2700*A[12]*(A[3]+A[14]+A[23]+A[25])
     + 4860*A[12]*A[12]
     + 30*A[5]*(90*A[12]+ 75*A[14]+ A[21]+ 9*A[23]+ 15*A[25]+ 7*A[27]+ 35*A[3])
     + 30*A[10]*(90*A[12]+9*A[14]+21*A[21]+35*A[23]+15*A[25]+A[27]+75*A[3]+
                 15*A[5])
     + 10*A[0]*(21*A[10]+42*A[12]+21*A[14]+A[21]+3*A[23]+3*A[25]+A[27]+
                63*(A[3]+A[5]))
     + 4*(+30*A[1]*(3*A[15] + 6*A[17] + 3*A[19] + 14*(A[6] + A[8]))
          +60*(A[8]*(3*(A[15] + 6*A[17] + 5*A[19]) + 10*A[6])+
               A[9]*(3*A[16] + 10*A[18] + 7*(A[2] + A[20]) + 10*A[7])+
               A[11]*(18*A[13] + 7*A[22] + 10*A[24] + 3*A[26] + 15*A[4])+
               A[13]*(3*A[22] + 10*A[24] + 7*A[26] + 15*A[4])+
               A[18]*(3*A[2]+7*A[20]+18*A[7]))
          +90*(A[2]*A[20]+A[15]*A[19]+A[22]*A[26]+
               A[4]*(A[22]+2*A[24]+A[26])+
               A[16]*(10*A[18]+A[2]+A[20]+10*A[7]))
          +180*(A[20]*A[7]+A[19]*A[6])
          +189*(A[1]*A[1]+A[2]*A[2]+A[15]*A[15]+A[20]*A[20]+
                A[22]*A[22]+A[26]*A[26])
          +420*(A[2]*A[7]+A[15]*A[17]+A[22]*A[24]+A[24]*A[26]+A[15]*A[6])
          +500*(A[9]*A[9]+A[24]*A[24]+A[6]*A[6])
          +525*(A[4]*A[4]+A[16]*A[16]+A[19]*A[19])
          +600*A[17]*A[6]
          +900*(A[7]*A[7]+A[8]*A[8]+A[11]*A[11]+A[13]*A[13]+A[17]*A[17]+
                A[18]*A[18]+A[17]*A[19])));
}

#define _TIJK_6O3D_SYM_V_FORM(TYPE, SUF)                                \
  void                                                                  \
  _tijk_6o3d_sym_v_form_##SUF (TYPE *res, const TYPE *A, const TYPE *v) { \
    TYPE v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],                   \
      v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];                      \
    TYPE v00000=v00*v00*v[0], v00001=v00*v00*v[1], v00002=v00*v00*v[2], \
      v00011=v00*v01*v[1],  v00012=v00*v01*v[2],  v00022=v00*v02*v[2],  \
      v00111=v00*v11*v[1],  v00112=v00*v11*v[2],  v00122=v00*v12*v[2],  \
      v00222=v00*v22*v[2],  v01111=v01*v11*v[1],  v01112=v01*v11*v[2],  \
      v01122=v01*v12*v[2],  v01222=v01*v22*v[2],  v02222=v02*v22*v[2],  \
      v11111=v11*v11*v[1],  v11112=v11*v11*v[2],  v11122=v11*v12*v[2],  \
      v11222=v11*v22*v[2],  v12222=v12*v22*v[2],  v22222=v22*v22*v[2];  \
    res[0] = A[0]*v00000+                                               \
      5*A[1]*v00001+                                                    \
      5*A[2]*v00002+                                                    \
      10*A[3]*v00011+                                                   \
      20*A[4]*v00012+                                                   \
      10*A[5]*v00022+                                                   \
      10*A[6]*v00111+                                                   \
      30*A[7]*v00112+                                                   \
      30*A[8]*v00122+                                                   \
      10*A[9]*v00222+                                                   \
      5*A[10]*v01111+                                                   \
      20*A[11]*v01112+                                                  \
      30*A[12]*v01122+                                                  \
      20*A[13]*v01222+                                                  \
      5*A[14]*v02222+                                                   \
      A[15]*v11111+                                                     \
      5*A[16]*v11112+                                                   \
      10*A[17]*v11122+                                                  \
      10*A[18]*v11222+                                                  \
      5*A[19]*v12222+                                                   \
      A[20]*v22222;                                                     \
    res[1] = A[1]*v00000+                                               \
      5*A[3]*v00001+                                                    \
      5*A[4]*v00002+                                                    \
      10*A[6]*v00011+                                                   \
      20*A[7]*v00012+                                                   \
      10*A[8]*v00022+                                                   \
      10*A[10]*v00111+                                                  \
      30*A[11]*v00112+                                                  \
      30*A[12]*v00122+                                                  \
      10*A[13]*v00222+                                                  \
      5*A[15]*v01111+                                                   \
      20*A[16]*v01112+                                                  \
      30*A[17]*v01122+                                                  \
      20*A[18]*v01222+                                                  \
      5*A[19]*v02222+                                                   \
      A[21]*v11111+                                                     \
      5*A[22]*v11112+                                                   \
      10*A[23]*v11122+                                                  \
      10*A[24]*v11222+                                                  \
      5*A[25]*v12222+                                                   \
      A[26]*v22222;                                                     \
    res[2] = A[2]*v00000+                                               \
      5*A[4]*v00001+                                                    \
      5*A[5]*v00002+                                                    \
      10*A[7]*v00011+                                                   \
      20*A[8]*v00012+                                                   \
      10*A[9]*v00022+                                                   \
      10*A[11]*v00111+                                                  \
      30*A[12]*v00112+                                                  \
      30*A[13]*v00122+                                                  \
      10*A[14]*v00222+                                                  \
      5*A[16]*v01111+                                                   \
      20*A[17]*v01112+                                                  \
      30*A[18]*v01122+                                                  \
      20*A[19]*v01222+                                                  \
      5*A[20]*v02222+                                                   \
      A[22]*v11111+                                                     \
      5*A[23]*v11112+                                                   \
      10*A[24]*v11122+                                                  \
      10*A[25]*v11222+                                                  \
      5*A[26]*v12222+                                                   \
      A[27]*v22222;                                                     \
  }

_TIJK_6O3D_SYM_V_FORM(double, d)
_TIJK_6O3D_SYM_V_FORM(float, f)

#define _TIJK_6O3D_SYM_M_FORM(TYPE, SUF)                                \
  void                                                                  \
  _tijk_6o3d_sym_m_form_##SUF (TYPE *res, const TYPE *A, const TYPE *v) { \
    TYPE v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],                   \
      v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];                      \
    TYPE v0000=v00*v00, v0001=v00*v01, v0002=v00*v02, v0011=v00*v11,    \
      v0012=v00*v12, v0022=v00*v22, v0111=v01*v11, v0112=v01*v12,       \
      v0122=v01*v22, v0222=v02*v22, v1111=v11*v11, v1112=v11*v12,       \
      v1122=v11*v22, v1222=v12*v22, v2222=v22*v22;                      \
    res[0] = A[0]*v0000+                                                \
      4*A[1]*v0001+                                                     \
      4*A[2]*v0002+                                                     \
      6*A[3]*v0011+                                                     \
      12*A[4]*v0012+                                                    \
      6*A[5]*v0022+                                                     \
      4*A[6]*v0111+                                                     \
      12*A[7]*v0112+                                                    \
      12*A[8]*v0122+                                                    \
      4*A[9]*v0222+                                                     \
      A[10]*v1111+                                                      \
      4*A[11]*v1112+                                                    \
      6*A[12]*v1122+                                                    \
      4*A[13]*v1222+                                                    \
      A[14]*v2222;                                                      \
    res[1] = A[1]*v0000+                                                \
      4*A[3]*v0001+                                                     \
      4*A[4]*v0002+                                                     \
      6*A[6]*v0011+                                                     \
      12*A[7]*v0012+                                                    \
      6*A[8]*v0022+                                                     \
      4*A[10]*v0111+                                                    \
      12*A[11]*v0112+                                                   \
      12*A[12]*v0122+                                                   \
      4*A[13]*v0222+                                                    \
      A[15]*v1111+                                                      \
      4*A[16]*v1112+                                                    \
      6*A[17]*v1122+                                                    \
      4*A[18]*v1222+                                                    \
      A[19]*v2222;                                                      \
    res[2] = A[2]*v0000+                                                \
      4*A[4]*v0001+                                                     \
      4*A[5]*v0002+                                                     \
      6*A[7]*v0011+                                                     \
      12*A[8]*v0012+                                                    \
      6*A[9]*v0022+                                                     \
      4*A[11]*v0111+                                                    \
      12*A[12]*v0112+                                                   \
      12*A[13]*v0122+                                                   \
      4*A[14]*v0222+                                                    \
      A[16]*v1111+                                                      \
      4*A[17]*v1112+                                                    \
      6*A[18]*v1122+                                                    \
      4*A[19]*v1222+                                                    \
      A[20]*v2222;                                                      \
    res[3] = A[3]*v0000+                                                \
      4*A[6]*v0001+                                                     \
      4*A[7]*v0002+                                                     \
      6*A[10]*v0011+                                                    \
      12*A[11]*v0012+                                                   \
      6*A[12]*v0022+                                                    \
      4*A[15]*v0111+                                                    \
      12*A[16]*v0112+                                                   \
      12*A[17]*v0122+                                                   \
      4*A[18]*v0222+                                                    \
      A[21]*v1111+                                                      \
      4*A[22]*v1112+                                                    \
      6*A[23]*v1122+                                                    \
      4*A[24]*v1222+                                                    \
      A[25]*v2222;                                                      \
    res[4] = A[4]*v0000+                                                \
      4*A[7]*v0001+                                                     \
      4*A[8]*v0002+                                                     \
      6*A[11]*v0011+                                                    \
      12*A[12]*v0012+                                                   \
      6*A[13]*v0022+                                                    \
      4*A[16]*v0111+                                                    \
      12*A[17]*v0112+                                                   \
      12*A[18]*v0122+                                                   \
      4*A[19]*v0222+                                                    \
      A[22]*v1111+                                                      \
      4*A[23]*v1112+                                                    \
      6*A[24]*v1122+                                                    \
      4*A[25]*v1222+                                                    \
      A[26]*v2222;                                                      \
    res[5] = A[5]*v0000+                                                \
      4*A[8]*v0001+                                                     \
      4*A[9]*v0002+                                                     \
      6*A[12]*v0011+                                                    \
      12*A[13]*v0012+                                                   \
      6*A[14]*v0022+                                                    \
      4*A[17]*v0111+                                                    \
      12*A[18]*v0112+                                                   \
      12*A[19]*v0122+                                                   \
      4*A[20]*v0222+                                                    \
      A[23]*v1111+                                                      \
      4*A[24]*v1112+                                                    \
      6*A[25]*v1122+                                                    \
      4*A[26]*v1222+                                                    \
      A[27]*v2222;                                                      \
  }

_TIJK_6O3D_SYM_M_FORM(double, d)
_TIJK_6O3D_SYM_M_FORM(float, f)

#define _TIJK_6O3D_SYM_MAKE_RANK1(TYPE, SUF)                             \
  void                                                                   \
  _tijk_6o3d_sym_make_rank1_##SUF (TYPE *res, const TYPE s, const TYPE *v) { \
    TYPE v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],                    \
      v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];                       \
    res[0]=s*v00*v00*v00; res[1]=s*v00*v00*v01; res[2]=s*v00*v00*v02;    \
    res[3]=s*v00*v00*v11; res[4]=s*v00*v00*v12; res[5]=s*v00*v00*v22;    \
    res[6]=s*v00*v01*v11; res[7]=s*v00*v01*v12; res[8]=s*v00*v01*v22;    \
    res[9]=s*v00*v02*v22; res[10]=s*v00*v11*v11; res[11]=s*v00*v11*v12;  \
    res[12]=s*v00*v11*v22; res[13]=s*v00*v12*v22; res[14]=s*v00*v22*v22; \
    res[15]=s*v01*v11*v11; res[16]=s*v01*v11*v12; res[17]=s*v01*v11*v22; \
    res[18]=s*v01*v12*v22; res[19]=s*v01*v22*v22; res[20]=s*v02*v22*v22; \
    res[21]=s*v11*v11*v11; res[22]=s*v11*v11*v12; res[23]=s*v11*v11*v22; \
    res[24]=s*v11*v12*v22; res[25]=s*v11*v22*v22; res[26]=s*v12*v22*v22; \
    res[27]=s*v22*v22*v22;                                               \
  }

_TIJK_6O3D_SYM_MAKE_RANK1(double, d)
_TIJK_6O3D_SYM_MAKE_RANK1(float, f)

void
_tijk_6o3d_sym_make_iso_d (double *res, const double s) {
  /* initialize to zero, then set non-zero elements */
  unsigned int i;
  for (i=0; i<28; i++)
    res[i]=0;
  res[0]=res[21]=res[27]=s;
  res[3]=res[5]=res[10]=res[14]=res[23]=res[25]=0.2*s;
  res[12]=s/15.0;
}

void
_tijk_6o3d_sym_make_iso_f (float *res, const float s) {
  /* initialize to zero, then set non-zero elements */
  unsigned int i;
  for (i=0; i<28; i++)
    res[i]=0;
  res[0]=res[21]=res[27]=s;
  res[3]=res[5]=res[10]=res[14]=res[23]=res[25]=0.2*s;
  res[12]=s/15.0;
}

void
_tijk_6o3d_sym_grad_d (double *res, const double *A, const double *v) {
  double proj, projv[3];
  _tijk_6o3d_sym_v_form_d (res, A, v);
  ELL_3V_SCALE(res,6.0,res);
  proj=ELL_3V_DOT(res,v);
  ELL_3V_SCALE(projv,-proj,v);
  ELL_3V_INCR(res,projv);
}

void
_tijk_6o3d_sym_grad_f (float *res, const float *A, const float *v) {
  float proj, projv[3];
  _tijk_6o3d_sym_v_form_f (res, A, v);
  ELL_3V_SCALE(res,6.0,res);
  proj=ELL_3V_DOT(res,v);
  ELL_3V_SCALE(projv,-proj,v);
  ELL_3V_INCR(res,projv);
}

#define _TIJK_6O3D_SYM_HESS(TYPE, SUF)                                  \
  void                                                                  \
  _tijk_6o3d_sym_hess_##SUF (TYPE *res, const TYPE *A, const TYPE *v) { \
    /* get two orthonormal tangents */                                  \
    TYPE t[2][3], cv[2][3], h[6], der, norm, tmp[6];                    \
    int r,c;                                                            \
    ell_3v_perp_##SUF(t[0], v);                                         \
    ELL_3V_NORM(t[0],t[0],norm);                                        \
    ELL_3V_CROSS(t[1],v,t[0]);                                          \
    ELL_3V_NORM(t[1],t[1],norm);                                        \
    /* compute Hessian w.r.t. t1/t2 */                                  \
    _tijk_6o3d_sym_m_form_##SUF(h, A, v);                               \
    der=6*_tijk_6o3d_sym_s_form_##SUF(A, v); /* first der in direction v*/ \
    _tijk_2o3d_sym_v_form_##SUF(cv[0],h,t[0]);                          \
    _tijk_2o3d_sym_v_form_##SUF(cv[1],h,t[1]);                          \
    h[0]=30*ELL_3V_DOT(cv[0],t[0])-der;                                 \
    h[1]=30*ELL_3V_DOT(cv[0],t[1]);                                     \
    h[2]=h[1];                                                          \
    h[3]=30*ELL_3V_DOT(cv[1],t[1])-der;                                 \
    /* now turn this into a symmetric order-2 rank-2 3D tensor */       \
    for (r=0; r<2; r++) {                                               \
      for (c=0; c<3; c++) {                                             \
        tmp[3*r+c]=h[2*r]*t[0][c]+h[2*r+1]*t[1][c];                     \
      }                                                                 \
    }                                                                   \
    res[0]=t[0][0]*tmp[0]+t[1][0]*tmp[3];                               \
    res[1]=t[0][0]*tmp[1]+t[1][0]*tmp[4];                               \
    res[2]=t[0][0]*tmp[2]+t[1][0]*tmp[5];                               \
    res[3]=t[0][1]*tmp[1]+t[1][1]*tmp[4];                               \
    res[4]=t[0][1]*tmp[2]+t[1][1]*tmp[5];                               \
    res[5]=t[0][2]*tmp[2]+t[1][2]*tmp[5];                               \
  }

_TIJK_6O3D_SYM_HESS(double,d)
_TIJK_6O3D_SYM_HESS(float,f)

TIJK_TYPE_SYM(6o3d_sym, 6, 3, 28)

/* 8th order 3D symmetric */
/* (unsymmetric counterpart currently not implemented) */

unsigned int _tijk_8o3d_sym_mult[45]={1, 8, 8, 28, 56, 28, 56, 168, 168, 56, 70, 280, 420, 280, 70, 56, 280, 560, 560, 280, 56, 28, 168, 420, 560, 420, 168, 28, 8, 56, 168, 280, 280, 168, 56, 8, 1, 8, 28, 56, 70, 56, 28, 8, 1};
#define _tijk_8o3d_sym_unsym2uniq NULL
#define _tijk_8o3d_sym_uniq2unsym NULL
#define _tijk_8o3d_sym_uniq_idx NULL

double
_tijk_8o3d_sym_tsp_d (const double *A, const double *B) {
  double retval=0.0;
  int i;
  for (i=0; i<45; i++) {
    retval+=_tijk_8o3d_sym_mult[i]*A[i]*B[i];
  }
  return retval;
}

float
_tijk_8o3d_sym_tsp_f (const float *A, const float *B) {
  float retval=0.0;
  int i;
  for (i=0; i<45; i++) {
    retval+=_tijk_8o3d_sym_mult[i]*A[i]*B[i];
  }
  return retval;
}

double
_tijk_8o3d_sym_norm_d (const double *A) {
  return sqrt(_tijk_8o3d_sym_tsp_d(A,A));
}

float
_tijk_8o3d_sym_norm_f (const float *A) {
  return sqrt(_tijk_8o3d_sym_tsp_f(A,A));
}

#define _TIJK_8O3D_SYM_TRANS(TYPE, SUF)                                 \
  void                                                                  \
  _tijk_8o3d_sym_trans_##SUF (TYPE *res, const TYPE *A, const TYPE *M) { \
    /* Tijklmnop = Miq Mjr Mks Mlt Mmu Mnv Mow Mpx Tqrstuvwx            \
     * For efficiency, we transform mode by mode; the intermediate results \
     * have incomplete symmetries! */                                   \
    TYPE tmpl[225], tmpr[225];                                          \
    int i;                                                              \
    { /* mode 8: */                                                     \
      int m[108]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6}; \
      int idx[324]={0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,10,15,16,11,16,17,12,17,18,13,18,19,14,19,20,15,21,22,16,22,23,17,23,24,18,24,25,19,25,26,20,26,27,21,28,29,22,29,30,23,30,31,24,31,32,25,32,33,26,33,34,27,34,35,28,36,37,29,37,38,30,38,39,31,39,40,32,40,41,33,41,42,34,42,43,35,43,44,0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,10,15,16,11,16,17,12,17,18,13,18,19,14,19,20,15,21,22,16,22,23,17,23,24,18,24,25,19,25,26,20,26,27,21,28,29,22,29,30,23,30,31,24,31,32,25,32,33,26,33,34,27,34,35,28,36,37,29,37,38,30,38,39,31,39,40,32,40,41,33,41,42,34,42,43,35,43,44,0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,10,15,16,11,16,17,12,17,18,13,18,19,14,19,20,15,21,22,16,22,23,17,23,24,18,24,25,19,25,26,20,26,27,21,28,29,22,29,30,23,30,31,24,31,32,25,32,33,26,33,34,27,34,35,28,36,37,29,37,38,30,38,39,31,39,40,32,40,41,33,41,42,34,42,43,35,43,44}; \
      for (i=0; i<108; i++)                                             \
        tmpl[i]=M[m[i]]*A[idx[3*i]]+                                    \
          M[m[i]+1]*A[idx[3*i+1]]+                                      \
          M[m[i]+2]*A[idx[3*i+2]];                                      \
    }                                                                   \
    { /* mode 7: */                                                     \
      int m[168]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6}; \
      int idx[504]={0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,10,15,16,11,16,17,12,17,18,13,18,19,14,19,20,15,21,22,16,22,23,17,23,24,18,24,25,19,25,26,20,26,27,21,28,29,22,29,30,23,30,31,24,31,32,25,32,33,26,33,34,27,34,35,0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,10,15,16,11,16,17,12,17,18,13,18,19,14,19,20,15,21,22,16,22,23,17,23,24,18,24,25,19,25,26,20,26,27,21,28,29,22,29,30,23,30,31,24,31,32,25,32,33,26,33,34,27,34,35,0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,10,15,16,11,16,17,12,17,18,13,18,19,14,19,20,15,21,22,16,22,23,17,23,24,18,24,25,19,25,26,20,26,27,21,28,29,22,29,30,23,30,31,24,31,32,25,32,33,26,33,34,27,34,35,36,37,38,37,39,40,38,40,41,39,42,43,40,43,44,41,44,45,42,46,47,43,47,48,44,48,49,45,49,50,46,51,52,47,52,53,48,53,54,49,54,55,50,55,56,51,57,58,52,58,59,53,59,60,54,60,61,55,61,62,56,62,63,57,64,65,58,65,66,59,66,67,60,67,68,61,68,69,62,69,70,63,70,71,36,37,38,37,39,40,38,40,41,39,42,43,40,43,44,41,44,45,42,46,47,43,47,48,44,48,49,45,49,50,46,51,52,47,52,53,48,53,54,49,54,55,50,55,56,51,57,58,52,58,59,53,59,60,54,60,61,55,61,62,56,62,63,57,64,65,58,65,66,59,66,67,60,67,68,61,68,69,62,69,70,63,70,71,72,73,74,73,75,76,74,76,77,75,78,79,76,79,80,77,80,81,78,82,83,79,83,84,80,84,85,81,85,86,82,87,88,83,88,89,84,89,90,85,90,91,86,91,92,87,93,94,88,94,95,89,95,96,90,96,97,91,97,98,92,98,99,93,100,101,94,101,102,95,102,103,96,103,104,97,104,105,98,105,106,99,106,107}; \
      for (i=0; i<168; i++)                                             \
        tmpr[i]=M[m[i]]*tmpl[idx[3*i]]+                                 \
          M[m[i]+1]*tmpl[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpl[idx[3*i+2]];                                   \
    }                                                                   \
    { /* mode 6: */                                                     \
      int m[210]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6}; \
      int idx[630]={0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,10,15,16,11,16,17,12,17,18,13,18,19,14,19,20,15,21,22,16,22,23,17,23,24,18,24,25,19,25,26,20,26,27,0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,10,15,16,11,16,17,12,17,18,13,18,19,14,19,20,15,21,22,16,22,23,17,23,24,18,24,25,19,25,26,20,26,27,0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,10,15,16,11,16,17,12,17,18,13,18,19,14,19,20,15,21,22,16,22,23,17,23,24,18,24,25,19,25,26,20,26,27,28,29,30,29,31,32,30,32,33,31,34,35,32,35,36,33,36,37,34,38,39,35,39,40,36,40,41,37,41,42,38,43,44,39,44,45,40,45,46,41,46,47,42,47,48,43,49,50,44,50,51,45,51,52,46,52,53,47,53,54,48,54,55,28,29,30,29,31,32,30,32,33,31,34,35,32,35,36,33,36,37,34,38,39,35,39,40,36,40,41,37,41,42,38,43,44,39,44,45,40,45,46,41,46,47,42,47,48,43,49,50,44,50,51,45,51,52,46,52,53,47,53,54,48,54,55,56,57,58,57,59,60,58,60,61,59,62,63,60,63,64,61,64,65,62,66,67,63,67,68,64,68,69,65,69,70,66,71,72,67,72,73,68,73,74,69,74,75,70,75,76,71,77,78,72,78,79,73,79,80,74,80,81,75,81,82,76,82,83,84,85,86,85,87,88,86,88,89,87,90,91,88,91,92,89,92,93,90,94,95,91,95,96,92,96,97,93,97,98,94,99,100,95,100,101,96,101,102,97,102,103,98,103,104,99,105,106,100,106,107,101,107,108,102,108,109,103,109,110,104,110,111,84,85,86,85,87,88,86,88,89,87,90,91,88,91,92,89,92,93,90,94,95,91,95,96,92,96,97,93,97,98,94,99,100,95,100,101,96,101,102,97,102,103,98,103,104,99,105,106,100,106,107,101,107,108,102,108,109,103,109,110,104,110,111,112,113,114,113,115,116,114,116,117,115,118,119,116,119,120,117,120,121,118,122,123,119,123,124,120,124,125,121,125,126,122,127,128,123,128,129,124,129,130,125,130,131,126,131,132,127,133,134,128,134,135,129,135,136,130,136,137,131,137,138,132,138,139,140,141,142,141,143,144,142,144,145,143,146,147,144,147,148,145,148,149,146,150,151,147,151,152,148,152,153,149,153,154,150,155,156,151,156,157,152,157,158,153,158,159,154,159,160,155,161,162,156,162,163,157,163,164,158,164,165,159,165,166,160,166,167}; \
      for (i=0; i<210; i++)                                             \
        tmpl[i]=M[m[i]]*tmpr[idx[3*i]]+                                 \
          M[m[i]+1]*tmpr[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpr[idx[3*i+2]];                                   \
    }                                                                   \
    { /* mode 5: */                                                     \
      int m[225]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6}; \
      int idx[675]={0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,10,15,16,11,16,17,12,17,18,13,18,19,14,19,20,0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,10,15,16,11,16,17,12,17,18,13,18,19,14,19,20,0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,10,15,16,11,16,17,12,17,18,13,18,19,14,19,20,21,22,23,22,24,25,23,25,26,24,27,28,25,28,29,26,29,30,27,31,32,28,32,33,29,33,34,30,34,35,31,36,37,32,37,38,33,38,39,34,39,40,35,40,41,21,22,23,22,24,25,23,25,26,24,27,28,25,28,29,26,29,30,27,31,32,28,32,33,29,33,34,30,34,35,31,36,37,32,37,38,33,38,39,34,39,40,35,40,41,42,43,44,43,45,46,44,46,47,45,48,49,46,49,50,47,50,51,48,52,53,49,53,54,50,54,55,51,55,56,52,57,58,53,58,59,54,59,60,55,60,61,56,61,62,63,64,65,64,66,67,65,67,68,66,69,70,67,70,71,68,71,72,69,73,74,70,74,75,71,75,76,72,76,77,73,78,79,74,79,80,75,80,81,76,81,82,77,82,83,63,64,65,64,66,67,65,67,68,66,69,70,67,70,71,68,71,72,69,73,74,70,74,75,71,75,76,72,76,77,73,78,79,74,79,80,75,80,81,76,81,82,77,82,83,84,85,86,85,87,88,86,88,89,87,90,91,88,91,92,89,92,93,90,94,95,91,95,96,92,96,97,93,97,98,94,99,100,95,100,101,96,101,102,97,102,103,98,103,104,105,106,107,106,108,109,107,109,110,108,111,112,109,112,113,110,113,114,111,115,116,112,116,117,113,117,118,114,118,119,115,120,121,116,121,122,117,122,123,118,123,124,119,124,125,126,127,128,127,129,130,128,130,131,129,132,133,130,133,134,131,134,135,132,136,137,133,137,138,134,138,139,135,139,140,136,141,142,137,142,143,138,143,144,139,144,145,140,145,146,126,127,128,127,129,130,128,130,131,129,132,133,130,133,134,131,134,135,132,136,137,133,137,138,134,138,139,135,139,140,136,141,142,137,142,143,138,143,144,139,144,145,140,145,146,147,148,149,148,150,151,149,151,152,150,153,154,151,154,155,152,155,156,153,157,158,154,158,159,155,159,160,156,160,161,157,162,163,158,163,164,159,164,165,160,165,166,161,166,167,168,169,170,169,171,172,170,172,173,171,174,175,172,175,176,173,176,177,174,178,179,175,179,180,176,180,181,177,181,182,178,183,184,179,184,185,180,185,186,181,186,187,182,187,188,189,190,191,190,192,193,191,193,194,192,195,196,193,196,197,194,197,198,195,199,200,196,200,201,197,201,202,198,202,203,199,204,205,200,205,206,201,206,207,202,207,208,203,208,209}; \
      for (i=0; i<225; i++)                                             \
        tmpr[i]=M[m[i]]*tmpl[idx[3*i]]+                                 \
          M[m[i]+1]*tmpl[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpl[idx[3*i+2]];                                   \
    }                                                                   \
    { /* mode 4: */                                                     \
      int m[210]={0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6}; \
      int idx[630]={0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,6,10,11,7,11,12,8,12,13,9,13,14,15,16,17,16,18,19,17,19,20,18,21,22,19,22,23,20,23,24,21,25,26,22,26,27,23,27,28,24,28,29,15,16,17,16,18,19,17,19,20,18,21,22,19,22,23,20,23,24,21,25,26,22,26,27,23,27,28,24,28,29,30,31,32,31,33,34,32,34,35,33,36,37,34,37,38,35,38,39,36,40,41,37,41,42,38,42,43,39,43,44,45,46,47,46,48,49,47,49,50,48,51,52,49,52,53,50,53,54,51,55,56,52,56,57,53,57,58,54,58,59,45,46,47,46,48,49,47,49,50,48,51,52,49,52,53,50,53,54,51,55,56,52,56,57,53,57,58,54,58,59,60,61,62,61,63,64,62,64,65,63,66,67,64,67,68,65,68,69,66,70,71,67,71,72,68,72,73,69,73,74,75,76,77,76,78,79,77,79,80,78,81,82,79,82,83,80,83,84,81,85,86,82,86,87,83,87,88,84,88,89,90,91,92,91,93,94,92,94,95,93,96,97,94,97,98,95,98,99,96,100,101,97,101,102,98,102,103,99,103,104,90,91,92,91,93,94,92,94,95,93,96,97,94,97,98,95,98,99,96,100,101,97,101,102,98,102,103,99,103,104,105,106,107,106,108,109,107,109,110,108,111,112,109,112,113,110,113,114,111,115,116,112,116,117,113,117,118,114,118,119,120,121,122,121,123,124,122,124,125,123,126,127,124,127,128,125,128,129,126,130,131,127,131,132,128,132,133,129,133,134,135,136,137,136,138,139,137,139,140,138,141,142,139,142,143,140,143,144,141,145,146,142,146,147,143,147,148,144,148,149,150,151,152,151,153,154,152,154,155,153,156,157,154,157,158,155,158,159,156,160,161,157,161,162,158,162,163,159,163,164,150,151,152,151,153,154,152,154,155,153,156,157,154,157,158,155,158,159,156,160,161,157,161,162,158,162,163,159,163,164,165,166,167,166,168,169,167,169,170,168,171,172,169,172,173,170,173,174,171,175,176,172,176,177,173,177,178,174,178,179,180,181,182,181,183,184,182,184,185,183,186,187,184,187,188,185,188,189,186,190,191,187,191,192,188,192,193,189,193,194,195,196,197,196,198,199,197,199,200,198,201,202,199,202,203,200,203,204,201,205,206,202,206,207,203,207,208,204,208,209,210,211,212,211,213,214,212,214,215,213,216,217,214,217,218,215,218,219,216,220,221,217,221,222,218,222,223,219,223,224}; \
      for (i=0; i<210; i++)                                             \
        tmpl[i]=M[m[i]]*tmpr[idx[3*i]]+                                 \
          M[m[i]+1]*tmpr[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpr[idx[3*i+2]];                                   \
    }                                                                   \
    { /* mode 3: */                                                     \
      int m[168]={0,0,0,0,0,0,3,3,3,3,3,3,6,6,6,6,6,6,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6}; \
      int idx[504]={0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,0,1,2,1,3,4,2,4,5,3,6,7,4,7,8,5,8,9,10,11,12,11,13,14,12,14,15,13,16,17,14,17,18,15,18,19,10,11,12,11,13,14,12,14,15,13,16,17,14,17,18,15,18,19,20,21,22,21,23,24,22,24,25,23,26,27,24,27,28,25,28,29,30,31,32,31,33,34,32,34,35,33,36,37,34,37,38,35,38,39,30,31,32,31,33,34,32,34,35,33,36,37,34,37,38,35,38,39,40,41,42,41,43,44,42,44,45,43,46,47,44,47,48,45,48,49,50,51,52,51,53,54,52,54,55,53,56,57,54,57,58,55,58,59,60,61,62,61,63,64,62,64,65,63,66,67,64,67,68,65,68,69,60,61,62,61,63,64,62,64,65,63,66,67,64,67,68,65,68,69,70,71,72,71,73,74,72,74,75,73,76,77,74,77,78,75,78,79,80,81,82,81,83,84,82,84,85,83,86,87,84,87,88,85,88,89,90,91,92,91,93,94,92,94,95,93,96,97,94,97,98,95,98,99,100,101,102,101,103,104,102,104,105,103,106,107,104,107,108,105,108,109,100,101,102,101,103,104,102,104,105,103,106,107,104,107,108,105,108,109,110,111,112,111,113,114,112,114,115,113,116,117,114,117,118,115,118,119,120,121,122,121,123,124,122,124,125,123,126,127,124,127,128,125,128,129,130,131,132,131,133,134,132,134,135,133,136,137,134,137,138,135,138,139,140,141,142,141,143,144,142,144,145,143,146,147,144,147,148,145,148,149,150,151,152,151,153,154,152,154,155,153,156,157,154,157,158,155,158,159,150,151,152,151,153,154,152,154,155,153,156,157,154,157,158,155,158,159,160,161,162,161,163,164,162,164,165,163,166,167,164,167,168,165,168,169,170,171,172,171,173,174,172,174,175,173,176,177,174,177,178,175,178,179,180,181,182,181,183,184,182,184,185,183,186,187,184,187,188,185,188,189,190,191,192,191,193,194,192,194,195,193,196,197,194,197,198,195,198,199,200,201,202,201,203,204,202,204,205,203,206,207,204,207,208,205,208,209}; \
      for (i=0; i<168; i++)                                             \
        tmpr[i]=M[m[i]]*tmpl[idx[3*i]]+                                 \
          M[m[i]+1]*tmpl[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpl[idx[3*i+2]];                                   \
    }                                                                   \
    { /* mode 2: */                                                     \
      int m[108]={0,0,0,3,3,3,6,6,6,3,3,3,6,6,6,6,6,6,3,3,3,6,6,6,6,6,6,6,6,6,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,3,3,3,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6}; \
      int idx[324]={0,1,2,1,3,4,2,4,5,0,1,2,1,3,4,2,4,5,0,1,2,1,3,4,2,4,5,6,7,8,7,9,10,8,10,11,6,7,8,7,9,10,8,10,11,12,13,14,13,15,16,14,16,17,18,19,20,19,21,22,20,22,23,18,19,20,19,21,22,20,22,23,24,25,26,25,27,28,26,28,29,30,31,32,31,33,34,32,34,35,36,37,38,37,39,40,38,40,41,36,37,38,37,39,40,38,40,41,42,43,44,43,45,46,44,46,47,48,49,50,49,51,52,50,52,53,54,55,56,55,57,58,56,58,59,60,61,62,61,63,64,62,64,65,60,61,62,61,63,64,62,64,65,66,67,68,67,69,70,68,70,71,72,73,74,73,75,76,74,76,77,78,79,80,79,81,82,80,82,83,84,85,86,85,87,88,86,88,89,90,91,92,91,93,94,92,94,95,90,91,92,91,93,94,92,94,95,96,97,98,97,99,100,98,100,101,102,103,104,103,105,106,104,106,107,108,109,110,109,111,112,110,112,113,114,115,116,115,117,118,116,118,119,120,121,122,121,123,124,122,124,125,126,127,128,127,129,130,128,130,131,126,127,128,127,129,130,128,130,131,132,133,134,133,135,136,134,136,137,138,139,140,139,141,142,140,142,143,144,145,146,145,147,148,146,148,149,150,151,152,151,153,154,152,154,155,156,157,158,157,159,160,158,160,161,162,163,164,163,165,166,164,166,167}; \
      for (i=0; i<108; i++)                                             \
        tmpl[i]=M[m[i]]*tmpr[idx[3*i]]+                                 \
          M[m[i]+1]*tmpr[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpr[idx[3*i+2]];                                   \
    }                                                                   \
    { /* mode 1: */                                                     \
      int m[45]={0,3,6,3,6,6,3,6,6,6,3,6,6,6,6,3,6,6,6,6,6,3,6,6,6,6,6,6,3,6,6,6,6,6,6,6,3,6,6,6,6,6,6,6,6}; \
      int idx[135]={0,1,2,0,1,2,0,1,2,3,4,5,3,4,5,6,7,8,9,10,11,9,10,11,12,13,14,15,16,17,18,19,20,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107}; \
      for (i=0; i<45; i++)                                              \
        res[i]=M[m[i]]*tmpl[idx[3*i]]+                                  \
          M[m[i]+1]*tmpl[idx[3*i+1]]+                                   \
          M[m[i]+2]*tmpl[idx[3*i+2]];                                   \
    }                                                                   \
  }

_TIJK_8O3D_SYM_TRANS(double, d)
_TIJK_8O3D_SYM_TRANS(float, f)

/* many routines left to implement - trying to call them will cause a segfault! */

#define _tijk_8o3d_sym_convert_f NULL
#define _tijk_8o3d_sym_convert_d NULL

#define _tijk_8o3d_sym_approx_f NULL
#define _tijk_8o3d_sym_approx_d NULL

double
_tijk_8o3d_sym_s_form_d (const double *A, const double *v) {
  double v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],
    v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];
  return A[0]*v00*v00*v00*v00+
    A[36]*v11*v11*v11*v11+
    A[44]*v22*v22*v22*v22+
    8*(A[1]*v00*v00*v00*v01+
       A[2]*v00*v00*v00*v02+
       A[28]*v01*v11*v11*v11+
       A[35]*v02*v22*v22*v22+
       A[37]*v11*v11*v11*v12+
       A[43]*v12*v22*v22*v22)+
    28*(A[3]*v00*v00*v00*v11+
        A[5]*v00*v00*v00*v22+
        A[21]*v00*v11*v11*v11+
        A[27]*v00*v22*v22*v22+
        A[38]*v11*v11*v11*v22+
        A[42]*v11*v22*v22*v22)+
    56*(A[4]*v00*v00*v00*v12+
        A[6]*v00*v00*v01*v11+
        A[9]*v00*v00*v02*v22+
        A[15]*v00*v01*v11*v11+
        A[20]*v00*v02*v22*v22+
        A[29]*v01*v11*v11*v12+
        A[34]*v01*v22*v22*v22+
        A[39]*v11*v11*v12*v22+
        A[41]*v11*v12*v22*v22)+
    70*(A[10]*v00*v00*v11*v11+
        A[14]*v00*v00*v22*v22+
        A[40]*v11*v11*v22*v22)+
    168*(A[7]*v00*v00*v01*v12+
         A[8]*v00*v00*v01*v22+
         A[22]*v00*v11*v11*v12+
         A[26]*v00*v12*v22*v22+
         A[30]*v01*v11*v11*v22+
         A[33]*v01*v12*v22*v22)+
    280*(A[11]*v00*v00*v11*v12+
         A[13]*v00*v00*v12*v22+
         A[16]*v00*v01*v11*v12+
         A[19]*v00*v01*v22*v22+
         A[31]*v01*v11*v12*v22+
         A[32]*v01*v11*v22*v22)+
    420*(A[12]*v00*v00*v11*v22+
         A[23]*v00*v11*v11*v22+
         A[25]*v00*v11*v22*v22)+
    560*(A[17]*v00*v01*v11*v22+
         A[18]*v00*v01*v12*v22+
         A[24]*v00*v11*v12*v22);
}

float
_tijk_8o3d_sym_s_form_f (const float *A, const float *v) {
  float v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],
    v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];
  return A[0]*v00*v00*v00*v00+
    A[36]*v11*v11*v11*v11+
    A[44]*v22*v22*v22*v22+
    8*(A[1]*v00*v00*v00*v01+
       A[2]*v00*v00*v00*v02+
       A[28]*v01*v11*v11*v11+
       A[35]*v02*v22*v22*v22+
       A[37]*v11*v11*v11*v12+
       A[43]*v12*v22*v22*v22)+
    28*(A[3]*v00*v00*v00*v11+
        A[5]*v00*v00*v00*v22+
        A[21]*v00*v11*v11*v11+
        A[27]*v00*v22*v22*v22+
        A[38]*v11*v11*v11*v22+
        A[42]*v11*v22*v22*v22)+
    56*(A[4]*v00*v00*v00*v12+
        A[6]*v00*v00*v01*v11+
        A[9]*v00*v00*v02*v22+
        A[15]*v00*v01*v11*v11+
        A[20]*v00*v02*v22*v22+
        A[29]*v01*v11*v11*v12+
        A[34]*v01*v22*v22*v22+
        A[39]*v11*v11*v12*v22+
        A[41]*v11*v12*v22*v22)+
    70*(A[10]*v00*v00*v11*v11+
        A[14]*v00*v00*v22*v22+
        A[40]*v11*v11*v22*v22)+
    168*(A[7]*v00*v00*v01*v12+
         A[8]*v00*v00*v01*v22+
         A[22]*v00*v11*v11*v12+
         A[26]*v00*v12*v22*v22+
         A[30]*v01*v11*v11*v22+
         A[33]*v01*v12*v22*v22)+
    280*(A[11]*v00*v00*v11*v12+
         A[13]*v00*v00*v12*v22+
         A[16]*v00*v01*v11*v12+
         A[19]*v00*v01*v22*v22+
         A[31]*v01*v11*v12*v22+
         A[32]*v01*v11*v22*v22)+
    420*(A[12]*v00*v00*v11*v22+
         A[23]*v00*v11*v11*v22+
         A[25]*v00*v11*v22*v22)+
    560*(A[17]*v00*v01*v11*v22+
         A[18]*v00*v01*v12*v22+
         A[24]*v00*v11*v12*v22);
}

double
_tijk_8o3d_sym_mean_d (const double *A) {
  return (A[0]+A[36]+A[44]+
          4*(A[3]+A[5]+A[21]+A[27]+A[38]+A[42])+
          6*(A[10]+A[14]+A[40])+
          12*(A[12]+A[23]+A[25]))/9.0;
}

float
_tijk_8o3d_sym_mean_f (const float *A) {
  return (A[0]+A[36]+A[44]+
          4*(A[3]+A[5]+A[21]+A[27]+A[38]+A[42])+
          6*(A[10]+A[14]+A[40])+
          12*(A[12]+A[23]+A[25]))/9.0;
}

#define _tijk_8o3d_sym_var_f NULL
#define _tijk_8o3d_sym_var_d NULL

#define _TIJK_8O3D_SYM_V_FORM(TYPE, SUF)                                \
  void                                                                  \
  _tijk_8o3d_sym_v_form_##SUF (TYPE *res, const TYPE *A, const TYPE *v) { \
    TYPE v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],                   \
      v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];                      \
    TYPE v00000=v00*v00*v[0], v00001=v00*v00*v[1], v00002=v00*v00*v[2], \
      v00011=v00*v01*v[1],  v00012=v00*v01*v[2],  v00022=v00*v02*v[2],  \
      v00111=v00*v11*v[1],  v00112=v00*v11*v[2],  v00122=v00*v12*v[2],  \
      v00222=v00*v22*v[2],  v01111=v01*v11*v[1],  v01112=v01*v11*v[2],  \
      v01122=v01*v12*v[2],  v01222=v01*v22*v[2],  v02222=v02*v22*v[2],  \
      v11111=v11*v11*v[1],  v11112=v11*v11*v[2],  v11122=v11*v12*v[2],  \
      v11222=v11*v22*v[2],  v12222=v12*v22*v[2],  v22222=v22*v22*v[2];  \
    res[0] = A[0]*v00*v00000+                                           \
      7*A[1]*v00*v00001+                                                \
      7*A[2]*v00*v00002+                                                \
      21*A[3]*v00*v00011+                                               \
      42*A[4]*v00*v00012+                                               \
      21*A[5]*v00*v00022+                                               \
      35*A[6]*v00*v00111+                                               \
      105*A[7]*v00*v00112+                                              \
      105*A[8]*v00*v00122+                                              \
      35*A[9]*v00*v00222+                                               \
      35*A[10]*v00*v01111+                                              \
      140*A[11]*v00*v01112+                                             \
      210*A[12]*v00*v01122+                                             \
      140*A[13]*v00*v01222+                                             \
      35*A[14]*v00*v02222+                                              \
      21*A[15]*v00*v11111+                                              \
      105*A[16]*v00*v11112+                                             \
      210*A[17]*v00*v11122+                                             \
      210*A[18]*v00*v11222+                                             \
      105*A[19]*v00*v12222+                                             \
      21*A[20]*v00*v22222+                                              \
      7*A[21]*v01*v11111+                                               \
      42*A[22]*v01*v11112+                                              \
      105*A[23]*v01*v11122+                                             \
      140*A[24]*v01*v11222+                                             \
      105*A[25]*v01*v12222+                                             \
      42*A[26]*v01*v22222+                                              \
      7*A[27]*v02*v22222+                                               \
      A[28]*v11*v11111+                                                 \
      7*A[29]*v11*v11112+                                               \
      21*A[30]*v11*v11122+                                              \
      35*A[31]*v11*v11222+                                              \
      35*A[32]*v11*v12222+                                              \
      21*A[33]*v11*v22222+                                              \
      7*A[34]*v12*v22222+                                               \
      A[35]*v22*v22222;                                                 \
    res[1] = A[1]*v00*v00000+                                           \
      7*A[3]*v00*v00001+                                                \
      7*A[4]*v00*v00002+                                                \
      21*A[6]*v00*v00011+                                               \
      42*A[7]*v00*v00012+                                               \
      21*A[8]*v00*v00022+                                               \
      35*A[10]*v00*v00111+                                              \
      105*A[11]*v00*v00112+                                             \
      105*A[12]*v00*v00122+                                             \
      35*A[13]*v00*v00222+                                              \
      35*A[15]*v00*v01111+                                              \
      140*A[16]*v00*v01112+                                             \
      210*A[17]*v00*v01122+                                             \
      140*A[18]*v00*v01222+                                             \
      35*A[19]*v00*v02222+                                              \
      21*A[21]*v00*v11111+                                              \
      105*A[22]*v00*v11112+                                             \
      210*A[23]*v00*v11122+                                             \
      210*A[24]*v00*v11222+                                             \
      105*A[25]*v00*v12222+                                             \
      21*A[26]*v00*v22222+                                              \
      7*A[28]*v01*v11111+                                               \
      42*A[29]*v01*v11112+                                              \
      105*A[30]*v01*v11122+                                             \
      140*A[31]*v01*v11222+                                             \
      105*A[32]*v01*v12222+                                             \
      42*A[33]*v01*v22222+                                              \
      7*A[34]*v02*v22222+                                               \
      A[36]*v11*v11111+                                                 \
      7*A[37]*v11*v11112+                                               \
      21*A[38]*v11*v11122+                                              \
      35*A[39]*v11*v11222+                                              \
      35*A[40]*v11*v12222+                                              \
      21*A[41]*v11*v22222+                                              \
      7*A[42]*v12*v22222+                                               \
      A[43]*v22*v22222;                                                 \
    res[2] = A[2]*v00*v00000+                                           \
      7*A[4]*v00*v00001+                                                \
      7*A[5]*v00*v00002+                                                \
      21*A[7]*v00*v00011+                                               \
      42*A[8]*v00*v00012+                                               \
      21*A[9]*v00*v00022+                                               \
      35*A[11]*v00*v00111+                                              \
      105*A[12]*v00*v00112+                                             \
      105*A[13]*v00*v00122+                                             \
      35*A[14]*v00*v00222+                                              \
      35*A[16]*v00*v01111+                                              \
      140*A[17]*v00*v01112+                                             \
      210*A[18]*v00*v01122+                                             \
      140*A[19]*v00*v01222+                                             \
      35*A[20]*v00*v02222+                                              \
      21*A[22]*v00*v11111+                                              \
      105*A[23]*v00*v11112+                                             \
      210*A[24]*v00*v11122+                                             \
      210*A[25]*v00*v11222+                                             \
      105*A[26]*v00*v12222+                                             \
      21*A[27]*v00*v22222+                                              \
      7*A[29]*v01*v11111+                                               \
      42*A[30]*v01*v11112+                                              \
      105*A[31]*v01*v11122+                                             \
      140*A[32]*v01*v11222+                                             \
      105*A[33]*v01*v12222+                                             \
      42*A[34]*v01*v22222+                                              \
      7*A[35]*v02*v22222+                                               \
      A[37]*v11*v11111+                                                 \
      7*A[38]*v11*v11112+                                               \
      21*A[39]*v11*v11122+                                              \
      35*A[40]*v11*v11222+                                              \
      35*A[41]*v11*v12222+                                              \
      21*A[42]*v11*v22222+                                              \
      7*A[43]*v12*v22222+                                               \
      A[44]*v22*v22222;                                                 \
  }

_TIJK_8O3D_SYM_V_FORM(double, d)
_TIJK_8O3D_SYM_V_FORM(float, f)

#define _TIJK_8O3D_SYM_M_FORM(TYPE, SUF)                                \
  void                                                                  \
  _tijk_8o3d_sym_m_form_##SUF (TYPE *res, const TYPE *A, const TYPE *v) { \
    TYPE v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],                   \
      v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];                      \
    TYPE v0000=v00*v00, v0001=v00*v01, v0002=v00*v02, v0011=v00*v11,    \
      v0012=v00*v12, v0022=v00*v22, v0111=v01*v11, v0112=v01*v12,       \
      v0122=v01*v22, v0222=v02*v22, v1111=v11*v11, v1112=v11*v12,       \
      v1122=v11*v22, v1222=v12*v22, v2222=v22*v22;                      \
    res[0] = A[0]*v00*v0000+                                            \
      6*A[1]*v00*v0001+                                                 \
      6*A[2]*v00*v0002+                                                 \
      15*A[3]*v00*v0011+                                                \
      30*A[4]*v00*v0012+                                                \
      15*A[5]*v00*v0022+                                                \
      20*A[6]*v00*v0111+                                                \
      60*A[7]*v00*v0112+                                                \
      60*A[8]*v00*v0122+                                                \
      20*A[9]*v00*v0222+                                                \
      15*A[10]*v00*v1111+                                               \
      60*A[11]*v00*v1112+                                               \
      90*A[12]*v00*v1122+                                               \
      60*A[13]*v00*v1222+                                               \
      15*A[14]*v00*v2222+                                               \
      6*A[15]*v01*v1111+                                                \
      30*A[16]*v01*v1112+                                               \
      60*A[17]*v01*v1122+                                               \
      60*A[18]*v01*v1222+                                               \
      30*A[19]*v01*v2222+                                               \
      6*A[20]*v02*v2222+                                                \
      A[21]*v11*v1111+                                                  \
      6*A[22]*v11*v1112+                                                \
      15*A[23]*v11*v1122+                                               \
      20*A[24]*v11*v1222+                                               \
      15*A[25]*v11*v2222+                                               \
      6*A[26]*v12*v2222+                                                \
      A[27]*v22*v2222;                                                  \
    res[1] = A[1]*v00*v0000+                                            \
      6*A[3]*v00*v0001+                                                 \
      6*A[4]*v00*v0002+                                                 \
      15*A[6]*v00*v0011+                                                \
      30*A[7]*v00*v0012+                                                \
      15*A[8]*v00*v0022+                                                \
      20*A[10]*v00*v0111+                                               \
      60*A[11]*v00*v0112+                                               \
      60*A[12]*v00*v0122+                                               \
      20*A[13]*v00*v0222+                                               \
      15*A[15]*v00*v1111+                                               \
      60*A[16]*v00*v1112+                                               \
      90*A[17]*v00*v1122+                                               \
      60*A[18]*v00*v1222+                                               \
      15*A[19]*v00*v2222+                                               \
      6*A[21]*v01*v1111+                                                \
      30*A[22]*v01*v1112+                                               \
      60*A[23]*v01*v1122+                                               \
      60*A[24]*v01*v1222+                                               \
      30*A[25]*v01*v2222+                                               \
      6*A[26]*v02*v2222+                                                \
      A[28]*v11*v1111+                                                  \
      6*A[29]*v11*v1112+                                                \
      15*A[30]*v11*v1122+                                               \
      20*A[31]*v11*v1222+                                               \
      15*A[32]*v11*v2222+                                               \
      6*A[33]*v12*v2222+                                                \
      A[34]*v22*v2222;                                                  \
    res[2] = A[2]*v00*v0000+                                            \
      6*A[4]*v00*v0001+                                                 \
      6*A[5]*v00*v0002+                                                 \
      15*A[7]*v00*v0011+                                                \
      30*A[8]*v00*v0012+                                                \
      15*A[9]*v00*v0022+                                                \
      20*A[11]*v00*v0111+                                               \
      60*A[12]*v00*v0112+                                               \
      60*A[13]*v00*v0122+                                               \
      20*A[14]*v00*v0222+                                               \
      15*A[16]*v00*v1111+                                               \
      60*A[17]*v00*v1112+                                               \
      90*A[18]*v00*v1122+                                               \
      60*A[19]*v00*v1222+                                               \
      15*A[20]*v00*v2222+                                               \
      6*A[22]*v01*v1111+                                                \
      30*A[23]*v01*v1112+                                               \
      60*A[24]*v01*v1122+                                               \
      60*A[25]*v01*v1222+                                               \
      30*A[26]*v01*v2222+                                               \
      6*A[27]*v02*v2222+                                                \
      A[29]*v11*v1111+                                                  \
      6*A[30]*v11*v1112+                                                \
      15*A[31]*v11*v1122+                                               \
      20*A[32]*v11*v1222+                                               \
      15*A[33]*v11*v2222+                                               \
      6*A[34]*v12*v2222+                                                \
      A[35]*v22*v2222;                                                  \
    res[3] = A[3]*v00*v0000+                                            \
      6*A[6]*v00*v0001+                                                 \
      6*A[7]*v00*v0002+                                                 \
      15*A[10]*v00*v0011+                                               \
      30*A[11]*v00*v0012+                                               \
      15*A[12]*v00*v0022+                                               \
      20*A[15]*v00*v0111+                                               \
      60*A[16]*v00*v0112+                                               \
      60*A[17]*v00*v0122+                                               \
      20*A[18]*v00*v0222+                                               \
      15*A[21]*v00*v1111+                                               \
      60*A[22]*v00*v1112+                                               \
      90*A[23]*v00*v1122+                                               \
      60*A[24]*v00*v1222+                                               \
      15*A[25]*v00*v2222+                                               \
      6*A[28]*v01*v1111+                                                \
      30*A[29]*v01*v1112+                                               \
      60*A[30]*v01*v1122+                                               \
      60*A[31]*v01*v1222+                                               \
      30*A[32]*v01*v2222+                                               \
      6*A[33]*v02*v2222+                                                \
      A[36]*v11*v1111+                                                  \
      6*A[37]*v11*v1112+                                                \
      15*A[38]*v11*v1122+                                               \
      20*A[39]*v11*v1222+                                               \
      15*A[40]*v11*v2222+                                               \
      6*A[41]*v12*v2222+                                                \
      A[42]*v22*v2222;                                                  \
    res[4] = A[4]*v00*v0000+                                            \
      6*A[7]*v00*v0001+                                                 \
      6*A[8]*v00*v0002+                                                 \
      15*A[11]*v00*v0011+                                               \
      30*A[12]*v00*v0012+                                               \
      15*A[13]*v00*v0022+                                               \
      20*A[16]*v00*v0111+                                               \
      60*A[17]*v00*v0112+                                               \
      60*A[18]*v00*v0122+                                               \
      20*A[19]*v00*v0222+                                               \
      15*A[22]*v00*v1111+                                               \
      60*A[23]*v00*v1112+                                               \
      90*A[24]*v00*v1122+                                               \
      60*A[25]*v00*v1222+                                               \
      15*A[26]*v00*v2222+                                               \
      6*A[29]*v01*v1111+                                                \
      30*A[30]*v01*v1112+                                               \
      60*A[31]*v01*v1122+                                               \
      60*A[32]*v01*v1222+                                               \
      30*A[33]*v01*v2222+                                               \
      6*A[34]*v02*v2222+                                                \
      A[37]*v11*v1111+                                                  \
      6*A[38]*v11*v1112+                                                \
      15*A[39]*v11*v1122+                                               \
      20*A[40]*v11*v1222+                                               \
      15*A[41]*v11*v2222+                                               \
      6*A[42]*v12*v2222+                                                \
      A[43]*v22*v2222;                                                  \
    res[5] = A[5]*v00*v0000+                                            \
      6*A[8]*v00*v0001+                                                 \
      6*A[9]*v00*v0002+                                                 \
      15*A[12]*v00*v0011+                                               \
      30*A[13]*v00*v0012+                                               \
      15*A[14]*v00*v0022+                                               \
      20*A[17]*v00*v0111+                                               \
      60*A[18]*v00*v0112+                                               \
      60*A[19]*v00*v0122+                                               \
      20*A[20]*v00*v0222+                                               \
      15*A[23]*v00*v1111+                                               \
      60*A[24]*v00*v1112+                                               \
      90*A[25]*v00*v1122+                                               \
      60*A[26]*v00*v1222+                                               \
      15*A[27]*v00*v2222+                                               \
      6*A[30]*v01*v1111+                                                \
      30*A[31]*v01*v1112+                                               \
      60*A[32]*v01*v1122+                                               \
      60*A[33]*v01*v1222+                                               \
      30*A[34]*v01*v2222+                                               \
      6*A[35]*v02*v2222+                                                \
      A[38]*v11*v1111+                                                  \
      6*A[39]*v11*v1112+                                                \
      15*A[40]*v11*v1122+                                               \
      20*A[41]*v11*v1222+                                               \
      15*A[42]*v11*v2222+                                               \
      6*A[43]*v12*v2222+                                                \
      A[44]*v22*v2222;                                                  \
  }

_TIJK_8O3D_SYM_M_FORM(double, d)
_TIJK_8O3D_SYM_M_FORM(float, f)

#define _TIJK_8O3D_SYM_MAKE_RANK1(TYPE, SUF)                                         \
  void                                                                               \
  _tijk_8o3d_sym_make_rank1_##SUF (TYPE *res, const TYPE s, const TYPE *v) {         \
    TYPE v00=v[0]*v[0], v01=v[0]*v[1], v02=v[0]*v[2],                                \
      v11=v[1]*v[1], v12=v[1]*v[2], v22=v[2]*v[2];                                   \
    res[0]=s*v00*v00*v00*v00; res[1]=s*v00*v00*v00*v01; res[2]=s*v00*v00*v00*v02;    \
    res[3]=s*v00*v00*v00*v11; res[4]=s*v00*v00*v00*v12; res[5]=s*v00*v00*v00*v22;    \
    res[6]=s*v00*v00*v01*v11; res[7]=s*v00*v00*v01*v12; res[8]=s*v00*v00*v01*v22;    \
    res[9]=s*v00*v00*v02*v22; res[10]=s*v00*v00*v11*v11; res[11]=s*v00*v00*v11*v12;  \
    res[12]=s*v00*v00*v11*v22; res[13]=s*v00*v00*v12*v22; res[14]=s*v00*v00*v22*v22; \
    res[15]=s*v00*v01*v11*v11; res[16]=s*v00*v01*v11*v12; res[17]=s*v00*v01*v11*v22; \
    res[18]=s*v00*v01*v12*v22; res[19]=s*v00*v01*v22*v22; res[20]=s*v00*v02*v22*v22; \
    res[21]=s*v00*v11*v11*v11; res[22]=s*v00*v11*v11*v12; res[23]=s*v00*v11*v11*v22; \
    res[24]=s*v00*v11*v12*v22; res[25]=s*v00*v11*v22*v22; res[26]=s*v00*v12*v22*v22; \
    res[27]=s*v00*v22*v22*v22; res[28]=s*v01*v11*v11*v11; res[29]=s*v01*v11*v11*v12; \
    res[30]=s*v01*v11*v11*v22; res[31]=s*v01*v11*v12*v22; res[32]=s*v01*v11*v22*v22; \
    res[33]=s*v01*v12*v22*v22; res[34]=s*v01*v22*v22*v22; res[35]=s*v02*v22*v22*v22; \
    res[36]=s*v11*v11*v11*v11; res[37]=s*v11*v11*v11*v12; res[38]=s*v11*v11*v11*v22; \
    res[39]=s*v11*v11*v12*v22; res[40]=s*v11*v11*v22*v22; res[41]=s*v11*v12*v22*v22; \
    res[42]=s*v11*v22*v22*v22; res[43]=s*v12*v22*v22*v22; res[44]=s*v22*v22*v22*v22; \
  }

_TIJK_8O3D_SYM_MAKE_RANK1(double, d)
_TIJK_8O3D_SYM_MAKE_RANK1(float, f)

void
_tijk_8o3d_sym_make_iso_d (double *res, const double s) {
  /* initialize to zero, then set non-zero elements */
  unsigned int i;
  for (i=0; i<45; i++)
    res[i]=0;
  res[0]=res[36]=res[44]=s;
  res[3]=res[5]=res[21]=res[27]=res[38]=res[42]=s/7.0;
  res[10]=res[14]=res[40]=0.085714285714285396*s;
  res[12]=res[23]=res[25]=0.028571428571428508*s;
}

void
_tijk_8o3d_sym_make_iso_f (float *res, const float s) {
  /* initialize to zero, then set non-zero elements */
  unsigned int i;
  for (i=0; i<45; i++)
    res[i]=0;
  res[0]=res[36]=res[44]=s;
  res[3]=res[5]=res[21]=res[27]=res[38]=res[42]=s/7.0;
  res[10]=res[14]=res[40]=0.085714285714285396*s;
  res[12]=res[23]=res[25]=0.028571428571428508*s;
}

void
_tijk_8o3d_sym_grad_d (double *res, const double *A, const double *v) {
  double proj, projv[3];
  _tijk_8o3d_sym_v_form_d (res, A, v);
  ELL_3V_SCALE(res,8.0,res);
  proj=ELL_3V_DOT(res,v);
  ELL_3V_SCALE(projv,-proj,v);
  ELL_3V_INCR(res,projv);
}

void
_tijk_8o3d_sym_grad_f (float *res, const float *A, const float *v) {
  float proj, projv[3];
  _tijk_8o3d_sym_v_form_f (res, A, v);
  ELL_3V_SCALE(res,8.0,res);
  proj=ELL_3V_DOT(res,v);
  ELL_3V_SCALE(projv,-proj,v);
  ELL_3V_INCR(res,projv);
}

#define _TIJK_8O3D_SYM_HESS(TYPE, SUF)                                  \
  void                                                                  \
  _tijk_8o3d_sym_hess_##SUF (TYPE *res, const TYPE *A, const TYPE *v) { \
    /* get two orthonormal tangents */                                  \
    TYPE t[2][3], cv[2][3], h[6], der, norm, tmp[6];                    \
    int r,c;                                                            \
    ell_3v_perp_##SUF(t[0], v);                                         \
    ELL_3V_NORM(t[0],t[0],norm);                                        \
    ELL_3V_CROSS(t[1],v,t[0]);                                          \
    ELL_3V_NORM(t[1],t[1],norm);                                        \
    /* compute Hessian w.r.t. t1/t2 */                                  \
    _tijk_8o3d_sym_m_form_##SUF(h, A, v);                               \
    der=8*_tijk_8o3d_sym_s_form_##SUF(A, v); /* first der in direction v*/ \
    _tijk_2o3d_sym_v_form_##SUF(cv[0],h,t[0]);                          \
    _tijk_2o3d_sym_v_form_##SUF(cv[1],h,t[1]);                          \
    h[0]=56*ELL_3V_DOT(cv[0],t[0])-der;                                 \
    h[1]=56*ELL_3V_DOT(cv[0],t[1]);                                     \
    h[2]=h[1];                                                          \
    h[3]=56*ELL_3V_DOT(cv[1],t[1])-der;                                 \
    /* now turn this into a symmetric order-2 rank-2 3D tensor */       \
    for (r=0; r<2; r++) {                                               \
      for (c=0; c<3; c++) {                                             \
        tmp[3*r+c]=h[2*r]*t[0][c]+h[2*r+1]*t[1][c];                     \
      }                                                                 \
    }                                                                   \
    res[0]=t[0][0]*tmp[0]+t[1][0]*tmp[3];                               \
    res[1]=t[0][0]*tmp[1]+t[1][0]*tmp[4];                               \
    res[2]=t[0][0]*tmp[2]+t[1][0]*tmp[5];                               \
    res[3]=t[0][1]*tmp[1]+t[1][1]*tmp[4];                               \
    res[4]=t[0][1]*tmp[2]+t[1][1]*tmp[5];                               \
    res[5]=t[0][2]*tmp[2]+t[1][2]*tmp[5];                               \
  }

_TIJK_8O3D_SYM_HESS(double,d)
_TIJK_8O3D_SYM_HESS(float,f)

TIJK_TYPE_SYM(8o3d_sym, 8, 3, 45)

#include "convertQuietPop.h"
