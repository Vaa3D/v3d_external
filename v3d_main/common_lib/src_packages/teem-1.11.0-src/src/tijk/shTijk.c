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

/* Implementation of spherical harmonics and their relation to tensors */

#include "tijk.h"

#include "convertQuietPush.h"

#define TIJK_TABLE_TYPE 0 /* create double version */
#include "shtables.h"
#undef TIJK_TABLE_TYPE /* explicitly undef to avoid compiler warnings */
#define TIJK_TABLE_TYPE 1 /* create float version */
#include "shtables.h"
#undef TIJK_TABLE_TYPE

const unsigned int tijk_max_esh_order=8;
/* for order 8, the maximum number of coefficients is 45 */
#define _TIJK_MAX_ESH_LEN 45
/* number of coefficients for order i/2 */
const unsigned int tijk_esh_len[5]={1,6,15,28,45};

#define TIJK_EVAL_ESH_BASIS(TYPE, SUF)                                  \
  unsigned int                                                          \
  tijk_eval_esh_basis_##SUF(TYPE *res, unsigned int order,              \
                            TYPE theta, TYPE phi) {                     \
    TYPE stheta=sin(theta);                                             \
    TYPE ctheta=cos(theta);                                             \
    TYPE stheta2=stheta*stheta, stheta4=stheta2*stheta2;                \
    TYPE ctheta2=ctheta*ctheta, ctheta4=ctheta2*ctheta2;                \
    res[0]=1.0/sqrt(4.0*AIR_PI);                                        \
    if (order<2)                                                        \
      return 0;                                                         \
    res[5]=res[1]=sqrt(15.0/(16.0*AIR_PI))*stheta2;                     \
    res[5]*=sin(2*phi); res[1]*=cos(2*phi);                             \
    res[4]=res[2]=-sqrt(15.0/(4.0*AIR_PI))*ctheta*stheta;               \
    res[4]*=sin(phi); res[2]*=cos(phi);                                 \
    res[3]=sqrt(5.0/(16.0*AIR_PI))*(3.0*ctheta2-1.0);                   \
    if (order<4)                                                        \
      return 2;                                                         \
    res[14]=res[6]=sqrt(315.0/(256.0*AIR_PI))*stheta4;                  \
    res[14]*=sin(4*phi); res[6]*=cos(4*phi);                            \
    res[13]=res[7]=-sqrt(315.0/(32.0*AIR_PI))*ctheta*stheta*stheta2;    \
    res[13]*=sin(3*phi); res[7]*=cos(3*phi);                            \
    res[12]=res[8]=sqrt(45.0/(64.0*AIR_PI))*(7*ctheta2-1)*stheta2;      \
    res[12]*=sin(2*phi); res[8]*=cos(2*phi);                            \
    res[11]=res[9]=-sqrt(45.0/(32.0*AIR_PI))*(7*ctheta2*ctheta-3*ctheta)*stheta; \
    res[11]*=sin(phi); res[9]*=cos(phi);                                \
    res[10]=sqrt(9.0/(256.0*AIR_PI))*(35.0*ctheta4-30.0*ctheta2+3.0);   \
    if (order<6)                                                        \
      return 4;                                                         \
    res[27]=res[15]=1.0/64.0*sqrt(6006.0/AIR_PI)*stheta4*stheta2;       \
    res[27]*=sin(6*phi); res[15]*=cos(6*phi);                           \
    res[26]=res[16]=-3.0/32.0*sqrt(2002.0/AIR_PI)*stheta4*stheta*ctheta; \
    res[26]*=sin(5*phi); res[16]*=cos(5*phi);                           \
    res[25]=res[17]=3.0/32.0*sqrt(91.0/AIR_PI)*stheta4*(11*ctheta2-1.0); \
    res[25]*=sin(4*phi); res[17]*=cos(4*phi);                           \
    res[24]=res[18]=-1.0/32.0*sqrt(2730.0/AIR_PI)*stheta2*stheta*(11*ctheta2*ctheta-3*ctheta); \
    res[24]*=sin(3*phi); res[18]*=cos(3*phi);                           \
    res[23]=res[19]=1.0/64.0*sqrt(2730.0/AIR_PI)*stheta2*(33*ctheta4-18*ctheta2+1.0); \
    res[23]*=sin(2*phi); res[19]*=cos(2*phi);                           \
    res[22]=res[20]=-1.0/16.0*sqrt(273.0/AIR_PI)*stheta*(33*ctheta4*ctheta-30.0*ctheta2*ctheta+5*ctheta); \
    res[22]*=sin(phi); res[20]*=cos(phi);                               \
    res[21]=1.0/32.0*sqrt(13.0/AIR_PI)*(231*ctheta4*ctheta2-315*ctheta4+105*ctheta2-5.0); \
    if (order<8)                                                        \
      return 6;                                                         \
    res[44]=res[28]=3.0/256.0*sqrt(12155.0/AIR_PI)*stheta4*stheta4;     \
    res[44]*=sin(8*phi); res[28]*=cos(8*phi);                           \
    res[43]=res[29]=-3.0/64.0*sqrt(12155.0/AIR_PI)*stheta4*stheta2*stheta*ctheta; \
    res[43]*=sin(7*phi); res[29]*=cos(7*phi);                           \
    res[42]=res[30]=1.0/128.0*sqrt(2*7293.0/AIR_PI)*stheta4*stheta2*(15*ctheta2-1); \
    res[42]*=sin(6*phi); res[30]*=cos(6*phi);                           \
    res[41]=res[31]=-3.0/64.0*sqrt(17017.0/AIR_PI)*stheta4*stheta*ctheta*(5*ctheta2-1); \
    res[41]*=sin(5*phi); res[31]*=cos(5*phi);                           \
    res[40]=res[32]=3.0/128.0*sqrt(1309.0/AIR_PI)*stheta4*(65*ctheta4-26*ctheta2+1); \
    res[40]*=sin(4*phi); res[32]*=cos(4*phi);                           \
    res[39]=res[33]=-1.0/64.0*sqrt(19635.0/AIR_PI)*stheta2*stheta*ctheta*(39*ctheta4-26*ctheta2+3); \
    res[39]*=sin(3*phi); res[33]*=cos(3*phi);                           \
    res[38]=res[34]=3.0/128.0*sqrt(2*595.0/AIR_PI)*stheta2*(143*ctheta4*ctheta2-143*ctheta4+33*ctheta2-1); \
    res[38]*=sin(2*phi); res[34]*=cos(2*phi);                           \
    res[37]=res[35]=-3.0/64.0*sqrt(17.0/AIR_PI)*stheta*ctheta*(715*ctheta4*ctheta2-1001*ctheta4+385*ctheta2-35); \
    res[37]*=sin(phi); res[35]*=cos(phi);                               \
    res[36]=1.0/256.0*sqrt(17.0/AIR_PI)*(6435*ctheta4*ctheta4-12012*ctheta2*ctheta4+6930*ctheta4-1260*ctheta2+35); \
    return 8; /* higher orders currently not implemented */             \
  }

TIJK_EVAL_ESH_BASIS(double, d)
TIJK_EVAL_ESH_BASIS(float, f)

#define TIJK_EVAL_ESH(TYPE, SUF)                                       \
  TYPE                                                                 \
  tijk_eval_esh_##SUF(TYPE *coeffs, unsigned int order,                \
                      TYPE theta, TYPE phi) {                          \
    TYPE basis[_TIJK_MAX_ESH_LEN];                                     \
    TYPE res=0.0;                                                      \
    unsigned int i;                                                    \
    if (order!=tijk_eval_esh_basis_##SUF(basis, order, theta, phi))    \
      return 0; /* there has been an error. */                         \
    for (i=0; i<tijk_esh_len[order/2]; i++)                            \
      res+=basis[i]*coeffs[i];                                         \
    return res;                                                        \
  }

TIJK_EVAL_ESH(double, d)
TIJK_EVAL_ESH(float, f)

#define TIJK_ESH_SP(TYPE, SUF)                     \
  TYPE                                             \
  tijk_esh_sp_##SUF(TYPE *A, TYPE *B, unsigned int order) { \
    TYPE res=0.0;                                  \
    if (order<=tijk_max_esh_order) {               \
      unsigned int i;                              \
      for (i=0; i<tijk_esh_len[order/2]; i++) {    \
        res+=A[i]*B[i];                            \
      }                                            \
    }                                              \
    return res;                                    \
  }

TIJK_ESH_SP(double, d)
TIJK_ESH_SP(float, f)

/* DOES NOT work in-place (with res==ten) */
#define TIJK_3D_SYM_TO_ESH(TYPE, SUF)                    \
  int                                                    \
  tijk_3d_sym_to_esh_##SUF(TYPE *res, const TYPE *ten,   \
                           const tijk_type *type) {      \
    const TYPE *m; /* conversion matrix */               \
    unsigned int i, j, n;                                \
    if (res==ten) return -1;                             \
    n=type->num;                                         \
    if (type==tijk_2o3d_sym) {                           \
      m=_tijk_sym2esh_o2_##SUF;                          \
    } else if (type==tijk_4o3d_sym) {                    \
      m=_tijk_sym2esh_o4_##SUF;                          \
    } else if (type==tijk_6o3d_sym) {                    \
      m=_tijk_sym2esh_o6_##SUF;                          \
    } else if (type==tijk_8o3d_sym) {                    \
      m=_tijk_sym2esh_o8_##SUF;                          \
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

TIJK_3D_SYM_TO_ESH(double, d)
TIJK_3D_SYM_TO_ESH(float, f)

/* DOES NOT work in-place (with res==sh) */
#define TIJK_ESH_TO_3D_SYM(TYPE, SUF)                              \
  const tijk_type*                                                 \
  tijk_esh_to_3d_sym_##SUF(TYPE *res, const TYPE *sh, unsigned int order) { \
    const TYPE *m; /* conversion matrix */                         \
    const tijk_type *type;                                         \
    unsigned int i, j, n;                                          \
    if (res==sh) return NULL;                                      \
    switch (order) {                                               \
    case 2:                                                        \
      m=_tijk_esh2sym_o2_##SUF;                                    \
      type=tijk_2o3d_sym;                                          \
      break;                                                       \
    case 4:                                                        \
      m=_tijk_esh2sym_o4_##SUF;                                    \
      type=tijk_4o3d_sym;                                          \
      break;                                                       \
    case 6:                                                        \
      m=_tijk_esh2sym_o6_##SUF;                                    \
      type=tijk_6o3d_sym;                                          \
      break;                                                       \
    case 8:                                                        \
      m=_tijk_esh2sym_o8_##SUF;                                    \
      type=tijk_8o3d_sym;                                          \
      break;                                                       \
    default:                                                       \
      return NULL; /* cannot do the conversion */                  \
    }                                                              \
    n=tijk_esh_len[order/2];                                       \
    for (i=0; i<n; i++) {                                          \
      res[i]=0;                                                    \
      for (j=0; j<n; j++) {                                        \
        res[i]+=m[n*i+j]*sh[j];                                    \
      }                                                            \
    }                                                              \
    return type;                                                   \
  }

TIJK_ESH_TO_3D_SYM(double, d)
TIJK_ESH_TO_3D_SYM(float, f)

/* Convolve in with kernel and write to out (in==out is permitted) */
#define TIJK_ESH_CONVOLVE(TYPE, SUF)                              \
  void tijk_esh_convolve_##SUF(TYPE *out, const TYPE *in,         \
                               const TYPE *kernel, unsigned int order) { \
    unsigned int o, idx=0;                                              \
    for (o=0; o<=order/2; o++) {                                        \
      while (idx<tijk_esh_len[o]) {                                     \
        *(out++)=*(in++)*kernel[o];                                     \
        idx++;                                                          \
      }                                                                 \
    }                                                                   \
  }

TIJK_ESH_CONVOLVE(double, d)
TIJK_ESH_CONVOLVE(float, f)

/* Deconvolve in with kernel and write to out (in==out is permitted) */
#define TIJK_ESH_DECONVOLVE(TYPE, SUF)                            \
  void tijk_esh_deconvolve_##SUF(TYPE *out, const TYPE *in,             \
                                 const TYPE *kernel, unsigned int order) { \
    unsigned int o, idx=0;                                              \
    for (o=0; o<=order/2; o++) {                                        \
      while (idx<tijk_esh_len[o]) {                                     \
        *(out++)=*(in++)/kernel[o];                                     \
        idx++;                                                          \
      }                                                                 \
    }                                                                   \
  }

TIJK_ESH_DECONVOLVE(double, d)
TIJK_ESH_DECONVOLVE(float, f)

/* Make a deconvolution kernel that turns a given signal (rotationally
 * symmetric in "compressed" form, i.e., one coefficient per SH order)
 * into a rank-1 term of given order.
 * Return 0 upon success
 *        1 if order is unsupported
 *        2 if any of the given signal values is zero
 */
#define TIJK_ESH_MAKE_KERNEL_RANK1(TYPE, SUF)                           \
  int tijk_esh_make_kernel_rank1_##SUF(TYPE *kernel, const TYPE *signl, \
                                       unsigned int order) {            \
    unsigned int i;                                                     \
    TYPE rank1[TIJK_TYPE_MAX_NUM], rank1sh[TIJK_TYPE_MAX_NUM];          \
    TYPE v[3]={0.0,0.0,1.0};                                            \
    /* Need to determine SH coefficients of a z-aligned rank-1 tensor */ \
    const tijk_type *type = NULL;                                       \
    switch (order) {                                                    \
    case 2: type=tijk_2o3d_sym; break;                                  \
    case 4: type=tijk_4o3d_sym; break;                                  \
    case 6: type=tijk_6o3d_sym; break;                                  \
    case 8: type=tijk_8o3d_sym; break;                                  \
    default: return 1;                                                  \
    }                                                                   \
    (*type->sym->make_rank1_##SUF)(rank1, 1.0, v);                      \
    tijk_3d_sym_to_esh_##SUF(rank1sh, rank1, type);                     \
    for (i=0; i<1+order/2; i++)                                         \
      if (signl[i]==0) {                                                \
        return 2;                                                       \
      }                                                                 \
    kernel[0]=signl[0]/rank1sh[0];                                      \
    if (order>=2) {                                                     \
      kernel[1]=signl[1]/rank1sh[3];                                    \
      if (order>=4) {                                                   \
        kernel[2]=signl[2]/rank1sh[10];                                 \
        if (order>=6) {                                                 \
          kernel[3]=signl[3]/rank1sh[21];                               \
          if (order>=8) {                                               \
            kernel[4]=signl[4]/rank1sh[36];                             \
          }                                                             \
        }                                                               \
      }                                                                 \
    }                                                                   \
    return 0;                                                           \
  }

TIJK_ESH_MAKE_KERNEL_RANK1(double, d)
TIJK_ESH_MAKE_KERNEL_RANK1(float, f)

/* Make a deconvolution kernel that turns a given signal (rotationally
 * symmetric in "compressed" form, i.e., one coefficient per SH order)
 * into a truncated delta peak of given order.
 * Return 0 upon success
 *        1 if order is unsupported
 *        2 if any of the given signal values is zero
 */
#define TIJK_ESH_MAKE_KERNEL_DELTA(TYPE, SUF)                           \
  int tijk_esh_make_kernel_delta_##SUF(TYPE *kernel, const TYPE *signl, \
                                       unsigned int order) {            \
    /* we need a truncated delta peak of given order */                 \
    TYPE deltash[TIJK_TYPE_MAX_NUM];                                    \
    unsigned int i;                                                     \
    if (order>tijk_max_esh_order || order%2!=0)                         \
      return 1;                                                         \
    for (i=0; i<1+order/2; i++)                                         \
      if (signl[i]==0) {                                                \
        return 2;                                                       \
      }                                                                 \
    tijk_eval_esh_basis_##SUF(deltash, order, 0, 0);                    \
    kernel[0]=signl[0]/deltash[0];                                      \
    if (order>=2) {                                                     \
      kernel[1]=signl[1]/deltash[3];                                    \
      if (order>=4) {                                                   \
        kernel[2]=signl[2]/deltash[10];                                 \
        if (order>=6) {                                                 \
          kernel[3]=signl[3]/deltash[21];                               \
          if (order>=8) {                                               \
            kernel[4]=signl[4]/deltash[36];                             \
          }                                                             \
        }                                                               \
      }                                                                 \
    }                                                                   \
    return 0;                                                           \
  }

TIJK_ESH_MAKE_KERNEL_DELTA(double, d)
TIJK_ESH_MAKE_KERNEL_DELTA(float, f)


#include "convertQuietPop.h"
