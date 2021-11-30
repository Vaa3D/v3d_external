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


#include "tijk.h"
#include "privateTijk.h"

#include "convertQuietPush.h"

/* Functions for symmetric tensor approximation */

/* a coarse sampling of the unit semicircle */
const unsigned int _tijk_max_candidates_2d=8;

#define _CANDIDATES_2D(TYPE, SUF)                 \
  static TYPE _candidates_2d_##SUF[16] = {        \
    1.0, 0.0,                                     \
    0.92387953251128674, 0.38268343236508978,     \
    0.70710678118654757, 0.70710678118654746,     \
    0.38268343236508984, 0.92387953251128674,     \
    0.0, 1.0,                                     \
    -0.38268343236508973, 0.92387953251128674,    \
    -0.70710678118654746, 0.70710678118654757,    \
    -0.92387953251128674, 0.38268343236508989     \
  };
_CANDIDATES_2D(double, d)
_CANDIDATES_2D(float, f)

/* a coarse sampling of the unit sphere */
const unsigned int _tijk_max_candidates_3d=30;

#define _CANDIDATES_3D(TYPE, SUF)             \
  static TYPE _candidates_3d_##SUF[90] = {    \
    -0.546405, 0.619202, 0.563943,            \
    -0.398931,-0.600006, 0.693432,            \
    0.587973, 0.521686, 0.618168,             \
    0.055894,-0.991971,-0.113444,             \
    -0.666933,-0.677984, 0.309094,            \
    0.163684, 0.533013, 0.830123,             \
    0.542826, 0.133898, 0.829102,             \
    -0.074751,-0.350412, 0.933608,            \
    0.845751, -0.478624,-0.235847,            \
    0.767148,-0.610673, 0.196372,             \
    -0.283810, 0.381633, 0.879663,            \
    0.537228,-0.616249, 0.575868,             \
    -0.711387, 0.197778, 0.674398,            \
    0.886511, 0.219025, 0.407586,             \
    0.296061, 0.842985, 0.449136,             \
    -0.937540,-0.340990, 0.068877,            \
    0.398833, 0.917023, 0.000835,             \
    0.097278,-0.711949, 0.695460,             \
    -0.311534, 0.908623,-0.278121,            \
    -0.432043,-0.089758, 0.897375,            \
    -0.949980, 0.030810, 0.310788,            \
    0.146722,-0.811981,-0.564942,             \
    -0.172201,-0.908573, 0.380580,            \
    0.507209,-0.848578,-0.150513,             \
    -0.730808,-0.654136,-0.194999,            \
    0.077744, 0.094961, 0.992441,             \
    0.383976,-0.293959, 0.875300,             \
    0.788208,-0.213656, 0.577130,             \
    -0.752333,-0.301447, 0.585769,            \
    -0.975732, 0.165497,-0.143382             \
  };

_CANDIDATES_3D(double, d)
_CANDIDATES_3D(float, f)

/* Produces a rough estimate of the position and value of the ABSOLUTE
 * maximum of the homogeneous form.
 * s and v will be set to the corresponding value and (unit-length) direction.
 * ten is the input tensor, type is its type.
 * returns 0 upon success, 1 upon erroneous parameters. */
#define _TIJK_INIT_RANK1(TYPE, SUF, DIM)                            \
  int                                                               \
  tijk_init_rank1_##DIM##d_##SUF(TYPE *s, TYPE *v, const TYPE *ten, \
                                 const tijk_type *type) {           \
    TYPE absmax=-1;                                                 \
    unsigned int i;                                                 \
    TYPE *candidate=_candidates_##DIM##d_##SUF;                     \
    if (type->dim!=DIM || type->sym==NULL)                          \
      return 1;                                                     \
    for (i=0; i<_tijk_max_candidates_##DIM##d; i++) {               \
      TYPE val=(*type->sym->s_form_##SUF)(ten, candidate);          \
      TYPE absval=fabs(val);                                        \
      if (absval>absmax) {                                          \
        absmax=absval;                                              \
        *s=val;                                                     \
        ELL_##DIM##V_COPY(v, candidate);                            \
      }                                                             \
      candidate+=DIM;                                               \
    }                                                               \
    return 0;                                                       \
  }

_TIJK_INIT_RANK1(double, d, 2)
_TIJK_INIT_RANK1(float, f, 2)
_TIJK_INIT_RANK1(double, d, 3)
_TIJK_INIT_RANK1(float, f, 3)

/* Produces a rough estimate of the position and value of the SIGNED
 * maximum of the homogeneous form.
 * s and v will be set to the corresponding value and (unit-length) direction.
 * ten is the input tensor, type is its type.
 * returns 0 upon success, 1 upon erroneous parameters.
 */
#define _TIJK_INIT_MAX(TYPE, SUF, DIM)                            \
  int                                                             \
  tijk_init_max_##DIM##d_##SUF(TYPE *s, TYPE *v, const TYPE *ten, \
                               const tijk_type *type) {           \
    TYPE max=0;                                                   \
    unsigned int i;                                               \
    TYPE *candidate=_candidates_##DIM##d_##SUF;                   \
    if (type->dim!=DIM || type->sym==NULL)                        \
      return 1;                                                   \
    *s=max=(*type->sym->s_form_##SUF)(ten, candidate);            \
    ELL_##DIM##V_COPY(v, candidate);                              \
    for (i=1; i<_tijk_max_candidates_##DIM##d; i++) {             \
      TYPE val;                                                   \
      candidate+=DIM;                                             \
      val=(*type->sym->s_form_##SUF)(ten, candidate);             \
      if (val>max) {                                              \
        max=val;                                                  \
        *s=val;                                                   \
        ELL_##DIM##V_COPY(v, candidate);                          \
      }                                                           \
    }                                                             \
    return 0;                                                     \
  }

_TIJK_INIT_MAX(double, d, 2)
_TIJK_INIT_MAX(float, f, 2)
_TIJK_INIT_MAX(double, d, 3)
_TIJK_INIT_MAX(float, f, 3)

static const tijk_refine_rank1_parm refine_rank1_parm_default = {
  1e-10, 1e-4, 0.3, 0.9, 0.5, 50};

tijk_refine_rank1_parm *tijk_refine_rank1_parm_new() {
  tijk_refine_rank1_parm *parm;
  parm = (tijk_refine_rank1_parm *)malloc(sizeof(tijk_refine_rank1_parm));
  if (parm) {
    memcpy(parm,&refine_rank1_parm_default,sizeof(tijk_refine_rank1_parm));
  }
  return parm;
}

tijk_refine_rank1_parm
*tijk_refine_rank1_parm_nix(tijk_refine_rank1_parm *parm) {
  airFree(parm);
  return NULL;
}

/* Refines a given rank-1 tensor to a locally optimal rank-1 approximation,
 * using a gradient descent scheme with Armijo stepsize.
 * s and v are scalar and (unit-len) vector part of the rank-1 tensor and
 *   will be updated by this function.
 * ten is the input tensor, type is its type.
 * parm: if non-NULL, used to change the default optimization parameters
 * We improve the rank-1 approximation by moving s away from zero
 * (unless refinemax is nonzero, in which case we try to make the
 * signed value of s as large as possible)
 * returns 0 upon success
 *         1 when given wrong parameters
 *         2 when the Armijo scheme failed to produce a valid stepsize
 */
#define _TIJK_REFINE_RANK1ORMAX(TYPE, SUF, DIM)                         \
  int                                                                   \
  _tijk_refine_rank1ormax_##DIM##d_##SUF(TYPE *s, TYPE *v, const TYPE *ten, \
                                         const tijk_type *type,         \
                                         const tijk_refine_rank1_parm *parm, \
                                         const int refinemax) {         \
    TYPE isoten[TIJK_TYPE_MAX_NUM], anisoten[TIJK_TYPE_MAX_NUM];        \
    TYPE der[DIM], iso, anisonorm, anisonorminv, oldval;                \
    char sign=(refinemax || *s>0)?1:-1;                                 \
    TYPE alpha, beta;                                                   \
    if (type->dim!=DIM || type->sym==NULL)                              \
      return 1;                                                         \
    if (parm==NULL) parm=&refine_rank1_parm_default;                    \
    /* It's easier to do the optimization on the deviatoric */          \
    iso=(*type->sym->mean_##SUF)(ten);                                  \
    (*type->sym->make_iso_##SUF)(isoten, iso);                          \
    tijk_sub_##SUF(anisoten, ten, isoten, type);                        \
    anisonorm=(*type->norm_##SUF)(anisoten);                            \
    if (anisonorm<parm->eps_start) {                                    \
      return 0; /* nothing to do */                                     \
    } else {                                                            \
      anisonorminv=1.0/anisonorm;                                       \
    }                                                                   \
    alpha=beta=parm->beta*anisonorminv;                                 \
    oldval=*s-iso;                                                      \
    /* set initial derivative */                                        \
    (*type->sym->grad_##SUF)(der, anisoten, v);                         \
    while (1) { /* refine until convergence */                          \
      /* stepsize needs to depend on norm to make the descent */        \
      /* scale invariant */                                             \
      unsigned int armijoct=0;                                          \
      TYPE testv[DIM], val;                                             \
      TYPE dist, derlen=ELL_##DIM##V_LEN(der);                          \
      /* determine stepsize based on Armijo's rule */                   \
      while (1) {                                                       \
        ++armijoct;                                                     \
        if (armijoct>parm->maxtry) {                                    \
          /* failed to find a valid stepsize */                         \
          return 2;                                                     \
        }                                                               \
        ELL_##DIM##V_SCALE_ADD2(testv,1.0,v,sign*alpha,der);            \
        ELL_##DIM##V_NORM(testv,testv,dist);                            \
        dist=1-ELL_##DIM##V_DOT(v,testv);                               \
        val=(*type->sym->s_form_##SUF)(anisoten,testv);                 \
        if (sign*val>=sign*oldval+parm->sigma*derlen*dist) {            \
          /* accept step */                                             \
          ELL_##DIM##V_COPY(v,testv);                                   \
          *s=val+iso;                                                   \
          (*type->sym->grad_##SUF)(der, anisoten, v);                   \
          if (alpha<beta) alpha /= parm->gamma;                         \
          break;                                                        \
        }                                                               \
        alpha *= parm->gamma; /* try again with decreased stepsize */   \
      }                                                                 \
      if (sign*(val-oldval)<=parm->eps_impr*anisonorm) {                \
        break; /* declare convergence */                                \
      }                                                                 \
      oldval=val;                                                       \
    }                                                                   \
    return 0;                                                           \
  }

_TIJK_REFINE_RANK1ORMAX(double, d, 2)
_TIJK_REFINE_RANK1ORMAX(float, f, 2)
_TIJK_REFINE_RANK1ORMAX(double, d, 3)
_TIJK_REFINE_RANK1ORMAX(float, f, 3)

#define _TIJK_REFINE_RANK1(TYPE, SUF, DIM)                              \
  int                                                                   \
  tijk_refine_rank1_##DIM##d_##SUF(TYPE *s, TYPE *v, const TYPE *ten,   \
                                   const tijk_type *type,               \
                                   const tijk_refine_rank1_parm *parm) { \
    return _tijk_refine_rank1ormax_##DIM##d_##SUF(s,v,ten,type,parm,0); \
  }

_TIJK_REFINE_RANK1(double, d, 2)
_TIJK_REFINE_RANK1(float, f, 2)
_TIJK_REFINE_RANK1(double, d, 3)
_TIJK_REFINE_RANK1(float, f, 3)

#define _TIJK_REFINE_MAX(TYPE, SUF, DIM)                                \
  int                                                                   \
  tijk_refine_max_##DIM##d_##SUF(TYPE *s, TYPE *v, const TYPE *ten,     \
                                 const tijk_type *type,                 \
                                 const tijk_refine_rank1_parm *parm) {  \
    return _tijk_refine_rank1ormax_##DIM##d_##SUF(s,v,ten,type,parm,1); \
  }

_TIJK_REFINE_MAX(double, d, 2)
_TIJK_REFINE_MAX(float, f, 2)
_TIJK_REFINE_MAX(double, d, 3)
_TIJK_REFINE_MAX(float, f, 3)

static const tijk_refine_rankk_parm refine_rankk_parm_default = {
  1e-10, 1e-4, 0, NULL};

tijk_refine_rankk_parm *tijk_refine_rankk_parm_new() {
  tijk_refine_rankk_parm *parm;
  parm = (tijk_refine_rankk_parm *)malloc(sizeof(tijk_refine_rankk_parm));
  if (parm) {
    memcpy(parm,&refine_rankk_parm_default,sizeof(tijk_refine_rankk_parm));
  }
  return parm;
}

tijk_refine_rankk_parm
*tijk_refine_rankk_parm_nix(tijk_refine_rankk_parm *parm) {
  if (parm!=NULL) {
    parm->rank1_parm=tijk_refine_rank1_parm_nix(parm->rank1_parm);
    free(parm);
  }
  return NULL;
}

/* Refines an existing rank-k approximation, using the iterative algorithm
 * described in Schultz and Seidel, TVCG/Vis 2008.
 * This is mostly used as a helper routine for tijk_approx_heur and
 * tijk_approx_rankk. It should only appear in user code if you know
 * what you are doing.
 *
 * ls and vs are the scalar and (unit-len) vector parts of the rank-1 tensors.
 * tens has the rank-1 tensors themselves (same information as ls and vs, but
 *   "multiplied out").
 * res is the approximation residual.
 * resnorm is the norm of the approximation residual.
 * orignorm is the original norm of the tensor
 * type is its type of tens and res
 * k is the rank (determines the length of ls, vs, and tens.
 * returns 0 upon success, 1 upon erroneous parameters.
 */
#define _TIJK_REFINE_RANKK(TYPE, SUF, DIM)                              \
  int                                                                   \
  tijk_refine_rankk_##DIM##d_##SUF(TYPE *ls, TYPE *vs, TYPE *tens,      \
                                   TYPE *res, TYPE *resnorm,            \
                                   const TYPE orignorm,                 \
                                   const tijk_type *type,               \
                                   const unsigned int k,                \
                                   const tijk_refine_rankk_parm *parm)  \
  {                                                                     \
    TYPE newnorm=(*resnorm);                                            \
    unsigned int i;                                                     \
    if (type->dim!=DIM || type->sym==NULL)                              \
      return 1;                                                         \
    if (parm==NULL) parm=&refine_rankk_parm_default;                    \
    if (*resnorm<parm->eps_res || k==0) {                               \
      return 0; /* nothing to do */                                     \
    }                                                                   \
    do {                                                                \
      *resnorm=newnorm;                                                 \
      for (i=0; i<k; i++) {                                             \
        if (ls[i]!=0.0) { /* refine an existing term */                 \
          tijk_incr_##SUF(res, tens+i*type->num, type);                 \
          ls[i]=(*type->sym->s_form_##SUF)(res, vs+DIM*i);              \
          if (parm->pos && ls[i]<0.0) { /* try a new one */             \
            tijk_init_max_##DIM##d_##SUF(ls+i, vs+DIM*i, res, type);    \
          }                                                             \
        } else { /* add a new term */                                   \
          if (parm->pos)                                                \
            tijk_init_max_##DIM##d_##SUF(ls+i, vs+DIM*i, res, type);    \
          else                                                          \
            tijk_init_rank1_##DIM##d_##SUF(ls+i, vs+DIM*i, res, type);  \
        }                                                               \
        tijk_refine_rank1_##DIM##d_##SUF(ls+i, vs+DIM*i, res, type,     \
                                         parm->rank1_parm);             \
        if (!parm->pos || ls[i]>0.0) {                                  \
          (*type->sym->make_rank1_##SUF)(tens+i*type->num, ls[i], vs+DIM*i); \
          tijk_sub_##SUF(res, res, tens+i*type->num, type);             \
        } else {                                                        \
          ls[i]=0.0;                                                    \
        }                                                               \
      }                                                                 \
      newnorm=(*type->norm_##SUF)(res);                                 \
    } while (newnorm>parm->eps_res &&                                   \
             (*resnorm)-newnorm>parm->eps_impr*orignorm);               \
    *resnorm=newnorm;                                                   \
    return 0;                                                           \
  }

_TIJK_REFINE_RANKK(double, d, 2)
_TIJK_REFINE_RANKK(float, f, 2)
_TIJK_REFINE_RANKK(double, d, 3)
_TIJK_REFINE_RANKK(float, f, 3)

static const tijk_approx_heur_parm approx_heur_parm_default = {
  1e-10, 0.1, NULL, NULL};

tijk_approx_heur_parm *tijk_approx_heur_parm_new() {
  tijk_approx_heur_parm *parm;
  parm = (tijk_approx_heur_parm *)malloc(sizeof(tijk_approx_heur_parm));
  if (parm) {
    memcpy(parm,&approx_heur_parm_default,sizeof(tijk_approx_heur_parm));
  }
  return parm;
}

tijk_approx_heur_parm
*tijk_approx_heur_parm_nix(tijk_approx_heur_parm *parm) {
  if (parm!=NULL) {
    parm->refine_parm=tijk_refine_rankk_parm_nix(parm->refine_parm);
    free(parm);
  }
  return NULL;
}

/* Approximates a given tensor with a rank-k approximation, guessing the best
 * rank by using a heuristic similar to Schultz and Seidel, TVCG/Vis 2008.
 * ls and vs are the scalar and (unit-len) vector parts of the rank-1 tensors.
 * res is the approximation residual.
 * ten is the input tensor, type is its type.
 * k is the maximum rank that should be used (ls and vs need to have enough
 *   space for k and DIM*k values, respectively).
 * parms: if non-NULL, allows to modify the parameters of the heuristic
 * returns the rank determined by the heuristic.
 */
#define _TIJK_APPROX_HEUR(TYPE, SUF, DIM)                               \
  int                                                                   \
  tijk_approx_heur_##DIM##d_##SUF(TYPE *ls, TYPE *vs, TYPE *res,        \
                                  const TYPE *ten, const tijk_type *type, \
                                  const unsigned int k,                 \
                                  const tijk_approx_heur_parm *parm)    \
  {                                                                     \
    TYPE *lstmp=NULL, *vstmp=NULL, *tens=NULL, *restmp=NULL;            \
    TYPE oldnorm, newnorm, orignorm;                                    \
    unsigned int currank=1;                                             \
    if (type->dim!=DIM || type->sym==NULL)                              \
      return 0;                                                         \
    if (parm==NULL) parm=&approx_heur_parm_default;                     \
    /* initializations */                                               \
    orignorm=newnorm=oldnorm=(*type->norm_##SUF)(ten);                  \
    if (k==0) {                                                         \
      if (res!=NULL) memcpy(res,ten,sizeof(TYPE)*type->num);            \
      return 0; /* nothing to do */                                     \
    }                                                                   \
    lstmp=AIR_CALLOC(k,TYPE);                                           \
    if (lstmp==NULL) goto cleanup_and_exit;                             \
    vstmp=AIR_CALLOC(DIM*k,TYPE);                                       \
    if (vstmp==NULL) goto cleanup_and_exit;                             \
    tens=AIR_CALLOC(k*type->num,TYPE);                                  \
    if (tens==NULL) goto cleanup_and_exit;                              \
    restmp=AIR_CALLOC(type->num,TYPE);                                  \
    if (restmp==NULL) goto cleanup_and_exit;                            \
    memcpy(restmp, ten, sizeof(TYPE)*type->num);                        \
    for (currank=1; currank<=k; currank++) {                            \
      int accept=1;                                                     \
      tijk_refine_rankk_##DIM##d_##SUF(lstmp, vstmp, tens, restmp,      \
                                       &newnorm, orignorm, type,        \
                                       currank, parm->refine_parm);     \
      if (currank>1 && parm->ratios!=NULL) {                            \
        /* make sure the desired ratio is fulfilled */                  \
        TYPE largest=fabs(lstmp[0]), smallest=largest;                  \
        unsigned int i;                                                 \
        for (i=1; i<currank; i++) {                                     \
          if (fabs(lstmp[i])>largest) largest=fabs(lstmp[i]);           \
          if (fabs(lstmp[i])<smallest) smallest=fabs(lstmp[i]);         \
        }                                                               \
        if (largest/smallest>parm->ratios[currank-2])                   \
          accept=0;                                                     \
      }                                                                 \
      if (accept && oldnorm-newnorm>parm->eps_impr*orignorm) {          \
        /* copy over */                                                 \
        memcpy(vs, vstmp, sizeof(TYPE)*DIM*currank);                    \
        memcpy(ls, lstmp, sizeof(TYPE)*currank);                        \
        if (res!=NULL)                                                  \
          memcpy(res, restmp, sizeof(TYPE)*type->num);                  \
        oldnorm=newnorm;                                                \
      } else {                                                          \
        break;                                                          \
      }                                                                 \
      if (newnorm<parm->eps_res*orignorm) {                             \
        currank++;                                                      \
        break; /* mission accomplished */                               \
      }                                                                 \
    }                                                                   \
  cleanup_and_exit:                                                     \
    if (lstmp!=NULL) free(lstmp);                                       \
    if (vstmp!=NULL) free(vstmp);                                       \
    if (tens!=NULL) free(tens);                                         \
    if (restmp!=NULL) free(restmp);                                     \
    return currank-1;                                                   \
  }

_TIJK_APPROX_HEUR(double, d, 2)
_TIJK_APPROX_HEUR(float, f, 2)
_TIJK_APPROX_HEUR(double, d, 3)
_TIJK_APPROX_HEUR(float, f, 3)

/* Approximates a given tensor with a rank-k approximation, using the
 * iterative algorithm described in Schultz and Seidel, TVCG/Vis 2008.
 * ls and vs are the scalar and (unit-len) vector parts of the rank-1 tensors.
 *   They can be NULL if you're just interested in the residual.
 * res is the approximation residual. Can be NULL if you're not interested.
 * ten is the input tensor, type is its type.
 * k is the rank that should be used (if ls and vs are non-NULL, they need
 *   to have enough space for k and DIM*k values, respectively).
 * returns 0 upon success, 1 upon error
 */
#define _TIJK_APPROX_RANKK(TYPE, SUF, DIM)                              \
  int                                                                   \
  tijk_approx_rankk_##DIM##d_##SUF(TYPE *ls, TYPE *vs, TYPE *res,       \
                                   const TYPE *ten, const tijk_type *type, \
                                   const unsigned int k,                \
                                   const tijk_refine_rankk_parm *parm)  \
  {                                                                     \
    char hadls=1, hadvs=1, hadres=1;                                    \
    TYPE *tens=NULL;                                                    \
    TYPE newnorm, orignorm;                                             \
    int retval=0;                                                       \
    if (type->dim!=DIM || type->sym==NULL)                              \
      return 1;                                                         \
    if (parm==NULL) parm=&refine_rankk_parm_default;                    \
    /* initializations */                                               \
    orignorm=newnorm=(*type->norm_##SUF)(ten);                          \
    if (orignorm<parm->eps_res || k==0) {                               \
      if (ls!=NULL) {                                                   \
        unsigned int i;                                                 \
        for (i=0; i<k; i++)                                             \
          ls[i]=0.0;                                                    \
      }                                                                 \
      if (res!=NULL) memcpy(res,ten,sizeof(TYPE)*type->num);            \
      return 0; /* nothing to do */                                     \
    }                                                                   \
    if (ls==NULL) {                                                     \
      ls=AIR_CALLOC(k,TYPE);                                            \
      if (ls==NULL) {retval=1; goto cleanup_and_exit;}                  \
      hadls=0;                                                          \
    } else {                                                            \
      unsigned int i;                                                   \
      for (i=0; i<k; i++)                                               \
        ls[i]=0.0;                                                      \
    }                                                                   \
    if (vs==NULL) {                                                     \
      vs=AIR_CALLOC(DIM*k, TYPE);                                       \
      if (vs==NULL) {retval=1; goto cleanup_and_exit;}                  \
      hadvs=0;                                                          \
    }                                                                   \
    if (res==NULL) {                                                    \
      res=AIR_CALLOC(type->num,TYPE);                                   \
      if (res==NULL) {retval=1; goto cleanup_and_exit;}                 \
      hadres=0;                                                         \
    }                                                                   \
    tens=AIR_CALLOC(k*type->num,TYPE);                                  \
    if (tens==NULL) {retval=1; goto cleanup_and_exit;}                  \
    memcpy(res, ten, sizeof(TYPE)*type->num);                           \
    tijk_refine_rankk_##DIM##d_##SUF(ls, vs, tens, res, &newnorm,       \
                                     orignorm, type, k, parm);          \
  cleanup_and_exit:                                                     \
    if (!hadls) free(ls);                                               \
    if (!hadvs) free(vs);                                               \
    if (!hadres) free(res);                                             \
    if (tens!=NULL) free(tens);                                         \
    return retval;                                                      \
  }

_TIJK_APPROX_RANKK(double, d, 2)
_TIJK_APPROX_RANKK(float, f, 2)
_TIJK_APPROX_RANKK(double, d, 3)
_TIJK_APPROX_RANKK(float, f, 3)


#include "convertQuietPop.h"
