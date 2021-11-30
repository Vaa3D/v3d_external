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

#ifndef TIJK_HAS_BEEN_INCLUDED
#define TIJK_HAS_BEEN_INCLUDED

#include <teem/air.h>
#include <teem/nrrd.h>
#include <teem/ell.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(tijk_EXPORTS) || defined(teem_EXPORTS)
#    define TIJK_EXPORT extern __declspec(dllexport)
#  else
#    define TIJK_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define TIJK_EXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tijk_sym_fun_t {
  /* Functions that are specific to completely symmetric tensors. */
  /* homogeneous scalar form */
  double (*s_form_d) (const double *A, const double *v);
  float  (*s_form_f) (const float  *A, const float  *v);
  /* mean value of scalar form */
  double (*mean_d) (const double *A);
  float  (*mean_f) (const float  *A);
  /* variance of scalar form */
  double (*var_d) (const double *A);
  float  (*var_f) (const float  *A);
  /* vector- and matrix-valued forms, proportional to gradient and
   * Hessian of scalar homogeneous forms
   * res and v should not point to the same data! */
  void (*v_form_d) (double *res, const double *A, const double *v);
  void (*v_form_f) (float  *res, const float  *A, const float  *v);
  /* returns a symmetric matrix (in non-redundant representation) */
  void (*m_form_d) (double *res, const double *A, const double *v);
  void (*m_form_f) (float  *res, const float  *A, const float  *v);
  /* gradient of the homogeneous form when restricted to the unit
   * hypersphere; assumes (but does not verify) that v is unit-length */
  void (*grad_d) (double *res, const double *A, const double *v);
  void (*grad_f) (float *res, const float *A, const float *v);
  /* Hessian of the homogeneous form when restricted to the unit
   * hypersphere; assumes (but does not verify) that v is unit-length */
  void (*hess_d) (double *res, const double *A, const double *v);
  void (*hess_f) (float *res, const float *A, const float *v);
  /* make a symmetric rank-1 tensor from the given scalar and vector */
  void (*make_rank1_d) (double *res, const double s, const double *v);
  void (*make_rank1_f) (float *res, const float s, const float *v);
  /* make a symmetric isotropic tensor from the given scalar;
   * NULL if order is odd (i.e., only isotropic tensor is the zero tensor) */
  void (*make_iso_d) (double *res, const double s);
  void (*make_iso_f) (float *res, const float s);
} tijk_sym_fun;

typedef struct tijk_type_t {
  /* Holds information about (and functions needed for processing) a
   * specific type of tensor */
  const char *name; /* description of the tensor type */
  unsigned int order; /* number of tensor indices */
  unsigned int dim; /* dimension of each axis (only square tensors supported) */
  unsigned int num; /* unique number of components */
#define TIJK_TYPE_MAX_NUM 45
  const unsigned int *mult; /* multiplicity of each unique component;
                             * NULL indicates that tensor is unsymmetric */

  /* The following fields are used to map the elements of an
   * unsymmetric tensor to the unique elements of the symmetric one.
   * A value i in these arrays means:
   * i==0: element is unmapped (due to antisymmetry)
   * i>0 : element maps to index (i-1)
   * i<0 : element maps to index -(i+1), with negated sign
   * They are NULL if the tensor is unsymmetric or the order is so
   * high that the unsymmetric variant is not even implemented. */
  const int *unsym2uniq; /* unsymmetric to unique; length: pow(dim,order) */
  const int *uniq2unsym; /* unique to unsymmetric; length: sum(mult) */
  const unsigned int *uniq_idx; /* index into uniq2unsym for each
                                 * unique component */

  /* tensor scalar product */
  double (*tsp_d) (const double *A, const double *B);
  float  (*tsp_f) (const float  *A, const float  *B);
  /* norm */
  double (*norm_d) (const double *A);
  float  (*norm_f) (const float  *A);
  /* transformation under change of basis;
   * applies the same matrix M to all tensor modes
   * assumes that res!=A */
  void (*trans_d) (double *res, const double *A, const double *M);
  void (*trans_f) (float  *res, const float  *A, const float  *M);
  /* converts to a different tensor type. Supported conversions are:
   * - exact same type (identity conversion)
   * - same order & dim, (partially) symmetric -> unsymmetric
   * - same dim & completely symmetric; lower -> higher order
   *   (preserves the homogeneous form)
   * res needs to have length res_type->num
   * Returns a non-zero value if requested conversion is not available.
   */
  int (*convert_d) (double *res, const struct tijk_type_t *res_type,
                    const double *A);
  int (*convert_f) (float  *res, const struct tijk_type_t *res_type,
                    const float  *A);
  /* approximates a tensor with one of the given target type.
   * Supported approximations are:
   * - same order & dim, unsymmetric -> (partially) symmetric
   *   (minimizes the norm of the residual)
   * - same dim & completely symmetric; higher -> lower order
   *   (preserves the frequencies that can be expressed in the low order)
   * res needs to have length res_type->num
   * Returns a non-zero value if requested approximation is not implemented.
   */
  int (*approx_d) (double *res, const struct tijk_type_t *res_type,
                   const double *A);
  int (*approx_f) (float *res, const struct tijk_type_t *res_type,
                   const float *A);

  /* convert/approximate from a different tensor type.
   * This should not be called in user code (instead, use convert/approx).
   * Needed if libraries other than tijk want to define new tensor types.
   */
  int (*_convert_from_d) (double *res, const double *A,
                          const struct tijk_type_t *from_type);
  int (*_convert_from_f) (float *res, const float *A,
                          const struct tijk_type_t *from_type);
  int (*_approx_from_d) (double *res, const double *A,
                         const struct tijk_type_t *from_type);
  int (*_approx_from_f) (float *res, const float *A,
                         const struct tijk_type_t *from_type);
  /* sym holds additional functions which are only useful for
   * completely symmetric tensors. In other cases, sym==NULL */
  const tijk_sym_fun *sym;
} tijk_type;

/* OBS: If you add a tijk_type, modify nrrdTijk.c:tijk_get_axis_type! */

/* 2dTijk.c */
TIJK_EXPORT const tijk_type *const tijk_2o2d_unsym;
TIJK_EXPORT const tijk_type *const tijk_2o2d_sym;
TIJK_EXPORT const tijk_type *const tijk_2o2d_asym;
TIJK_EXPORT const tijk_type *const tijk_3o2d_sym;
TIJK_EXPORT const tijk_type *const tijk_4o2d_unsym;
TIJK_EXPORT const tijk_type *const tijk_4o2d_sym;

/* 3dTijk.c */
TIJK_EXPORT const tijk_type *const tijk_1o3d;
TIJK_EXPORT const tijk_type *const tijk_2o3d_unsym;
TIJK_EXPORT const tijk_type *const tijk_2o3d_sym;
TIJK_EXPORT const tijk_type *const tijk_2o3d_asym;
TIJK_EXPORT const tijk_type *const tijk_3o3d_unsym;
TIJK_EXPORT const tijk_type *const tijk_3o3d_sym;
TIJK_EXPORT const tijk_type *const tijk_4o3d_sym;
TIJK_EXPORT const tijk_type *const tijk_6o3d_sym;
TIJK_EXPORT const tijk_type *const tijk_8o3d_sym; /* still VERY incomplete! */

/* miscTijk.c */
TIJK_EXPORT const int tijkPresent;
TIJK_EXPORT void tijk_add_d(double *res, const double *A,
                            const double *B, const tijk_type *type);
TIJK_EXPORT void tijk_add_f(float *res, const float *A,
                            const float *B, const tijk_type *type);

TIJK_EXPORT void tijk_sub_d(double *res, const double *A,
                            const double *B, const tijk_type *type);
TIJK_EXPORT void tijk_sub_f(float *res, const float *A,
                            const float *B, const tijk_type *type);

TIJK_EXPORT void tijk_incr_d(double *res, const double *A,
                             const tijk_type *type);
TIJK_EXPORT void tijk_incr_f(float *res, const float *A,
                             const tijk_type *type);

TIJK_EXPORT void tijk_negate_d(double *res, const double *A,
                               const tijk_type *type);
TIJK_EXPORT void tijk_negate_f(float *res, const float *A,
                               const tijk_type *type);

TIJK_EXPORT void tijk_scale_d(double *res, const double s, const double *A,
                              const tijk_type *type);
TIJK_EXPORT void tijk_scale_f(float *res, const float s, const float *A,
                              const tijk_type *type);

TIJK_EXPORT void tijk_zero_d(double *res, const tijk_type *type);
TIJK_EXPORT void tijk_zero_f(float *res, const tijk_type *type);

TIJK_EXPORT void tijk_copy_d(double *res, const double *A,
                             const tijk_type *type);
TIJK_EXPORT void tijk_copy_f(float *res, const float *A,
                             const tijk_type *type);

/* approxTijk.c */

/* These parameters control optimization of rank-1 terms */
typedef struct tijk_refine_rank1_parm_t {
  /* only do optimization if norm of deviatoric is larger than eps_start */
  double eps_start;
  /* declare convergence if improvement is less than eps_impr times the
   * norm of deviatoric (not residual) */
  double eps_impr;
  /* Parameters associated with Armijo stepsize control */
  double beta; /* initial stepsize (divided by norm of deviatoric) */
  double gamma; /* stepsize reduction factor (0,1) */
  double sigma; /* corridor of values that lead to acceptance (0,1) */
  unsigned int maxtry; /* number of stepsize reductions before giving up */
} tijk_refine_rank1_parm;

TIJK_EXPORT tijk_refine_rank1_parm *tijk_refine_rank1_parm_new(void);
TIJK_EXPORT tijk_refine_rank1_parm
  *tijk_refine_rank1_parm_nix(tijk_refine_rank1_parm *parm);

/* These parameters control optimization of rank-k approximations */
typedef struct tijk_refine_rankk_parm_t {
  double eps_res; /* stop optimization if the residual is smaller than this */
  /* declare convergence if tensor norm improved less than eps_impr times
   * the original norm */
  double eps_impr;
  char pos; /* if non-zero, allow positive terms only */
  tijk_refine_rank1_parm *rank1_parm; /* used for rank1-optimization */
} tijk_refine_rankk_parm;

TIJK_EXPORT tijk_refine_rankk_parm *tijk_refine_rankk_parm_new(void);
TIJK_EXPORT tijk_refine_rankk_parm
  *tijk_refine_rankk_parm_nix(tijk_refine_rankk_parm *parm);

typedef struct tijk_approx_heur_parm_t {
  double eps_res; /* stop adding terms if the residual is smaller than eps_res
     * times the original norm */
  double eps_impr; /* stop adding terms if it would reduce the residual
      * less than eps_impr times the original norm */
  /* If ratios is non-NULL, it should have k-1 entries for a rank-k approx.
   * Do not add the ith rank-1 term when the ratio of largest/smallest
   * coefficient would be greater than ratios[i-2] */
  double *ratios;
  tijk_refine_rankk_parm *refine_parm; /* used for rank-k refinement */
} tijk_approx_heur_parm;

TIJK_EXPORT tijk_approx_heur_parm *tijk_approx_heur_parm_new(void);
TIJK_EXPORT tijk_approx_heur_parm
  *tijk_approx_heur_parm_nix(tijk_approx_heur_parm *parm);

TIJK_EXPORT int tijk_init_rank1_2d_d(double *s, double *v, const double *ten,
                                     const tijk_type *type);
TIJK_EXPORT int tijk_init_rank1_2d_f(float *s, float *v, const float *ten,
                                     const tijk_type *type);

TIJK_EXPORT int tijk_init_rank1_3d_d(double *s, double *v, const double *ten,
                                     const tijk_type *type);
TIJK_EXPORT int tijk_init_rank1_3d_f(float *s, float *v, const float *ten,
                                     const tijk_type *type);

TIJK_EXPORT int tijk_init_max_2d_d(double *s, double *v, const double *ten,
                                   const tijk_type *type);
TIJK_EXPORT int tijk_init_max_2d_f(float *s, float *v, const float *ten,
                                   const tijk_type *type);

TIJK_EXPORT int tijk_init_max_3d_d(double *s, double *v, const double *ten,
                                   const tijk_type *type);
TIJK_EXPORT int tijk_init_max_3d_f(float *s, float *v, const float *ten,
                                   const tijk_type *type);

/* For ANSI C compatibility, these routines rely on
 * type->num<=TIJK_TYPE_MAX_NUM !*/
TIJK_EXPORT int tijk_refine_rank1_2d_d(double *s, double *v, const double *ten,
                                       const tijk_type *type,
                                       const tijk_refine_rank1_parm *parm);
TIJK_EXPORT int tijk_refine_rank1_2d_f(float *s, float *v, const float *ten,
                                       const tijk_type *type,
                                       const tijk_refine_rank1_parm *parm);
TIJK_EXPORT int tijk_refine_rank1_3d_d(double *s, double *v, const double *ten,
                                       const tijk_type *type,
                                       const tijk_refine_rank1_parm *parm);
TIJK_EXPORT int tijk_refine_rank1_3d_f(float *s, float *v, const float *ten,
                                       const tijk_type *type,
                                       const tijk_refine_rank1_parm *parm);

TIJK_EXPORT int tijk_refine_max_2d_d(double *s, double *v, const double *ten,
                                     const tijk_type *type,
                                     const tijk_refine_rank1_parm *parm);
TIJK_EXPORT int tijk_refine_max_2d_f(float *s, float *v, const float *ten,
                                     const tijk_type *type,
                                     const tijk_refine_rank1_parm *parm);
TIJK_EXPORT int tijk_refine_max_3d_d(double *s, double *v, const double *ten,
                                     const tijk_type *type,
                                     const tijk_refine_rank1_parm *parm);
TIJK_EXPORT int tijk_refine_max_3d_f(float *s, float *v, const float *ten,
                                     const tijk_type *type,
                                     const tijk_refine_rank1_parm *parm);

TIJK_EXPORT int tijk_refine_rankk_2d_d(double *ls, double *vs,
                                       double *tens, double *res,
                                       double *resnorm, const double orignorm,
                                       const tijk_type *type,
                                       const unsigned int k,
                                       const tijk_refine_rankk_parm *parm);
TIJK_EXPORT int tijk_refine_rankk_2d_f(float *ls, float *vs,
                                       float *tens, float *res,
                                       float *resnorm, const float orignorm,
                                       const tijk_type *type,
                                       const unsigned int k,
                                       const tijk_refine_rankk_parm *parm);
TIJK_EXPORT int tijk_refine_rankk_3d_d(double *ls, double *vs,
                                       double *tens, double *res,
                                       double *resnorm, const double orignorm,
                                       const tijk_type *type,
                                       const unsigned int k,
                                       const tijk_refine_rankk_parm *parm);
TIJK_EXPORT int tijk_refine_rankk_3d_f(float *ls, float *vs,
                                       float *tens, float *res,
                                       float *resnorm, const float orignorm,
                                       const tijk_type *type,
                                       const unsigned int k,
                                       const tijk_refine_rankk_parm *parm);

TIJK_EXPORT int tijk_approx_rankk_2d_d(double *ls, double *vs, double *res,
                                       const double *ten, const tijk_type *type,
                                       const unsigned int k,
                                       const tijk_refine_rankk_parm *parm);
TIJK_EXPORT int tijk_approx_rankk_2d_f(float *ls, float *vs, float *res,
                                       const float *ten, const tijk_type *type,
                                       const unsigned int k,
                                       const tijk_refine_rankk_parm *parm);
TIJK_EXPORT int tijk_approx_rankk_3d_d(double *ls, double *vs, double *res,
                                       const double *ten, const tijk_type *type,
                                       const unsigned int k,
                                       const tijk_refine_rankk_parm *parm);
TIJK_EXPORT int tijk_approx_rankk_3d_f(float *ls, float *vs, float *res,
                                       const float *ten, const tijk_type *type,
                                       const unsigned int k,
                                       const tijk_refine_rankk_parm *parm);

TIJK_EXPORT int tijk_approx_heur_2d_d(double *ls, double *vs, double *res,
                                      const double *ten, const tijk_type *type,
                                      const unsigned int k,
                                      const tijk_approx_heur_parm *parm);
TIJK_EXPORT int tijk_approx_heur_2d_f(float *ls, float *vs, float *res,
                                      const float *ten, const tijk_type *type,
                                      const unsigned int k,
                                      const tijk_approx_heur_parm *parm);
TIJK_EXPORT int tijk_approx_heur_3d_d(double *ls, double *vs, double *res,
                                      const double *ten, const tijk_type *type,
                                      const unsigned int k,
                                      const tijk_approx_heur_parm *parm);
TIJK_EXPORT int tijk_approx_heur_3d_f(float *ls, float *vs, float *res,
                                      const float *ten, const tijk_type *type,
                                      const unsigned int k,
                                      const tijk_approx_heur_parm *parm);

/* shTijk.c */
/* at position i, number of coefficients for order 2*i */
TIJK_EXPORT const unsigned int tijk_esh_len[];
TIJK_EXPORT const unsigned int tijk_max_esh_order;

TIJK_EXPORT unsigned int tijk_eval_esh_basis_d(double *res, unsigned int order,
                                               double theta, double phi);
TIJK_EXPORT unsigned int tijk_eval_esh_basis_f(float *res, unsigned int order,
                                               float theta, float phi);

TIJK_EXPORT double tijk_eval_esh_d(double *coeffs, unsigned int order,
                                   double theta, double phi);
TIJK_EXPORT float tijk_eval_esh_f(float *coeffs, unsigned int order,
                                  float theta, float phi);

TIJK_EXPORT double tijk_esh_sp_d(double *A, double *B, unsigned int order);
TIJK_EXPORT float  tijk_esh_sp_f(float  *A, float  *B, unsigned int order);

TIJK_EXPORT int tijk_3d_sym_to_esh_d(double *res, const double *ten,
                                     const tijk_type *type);
TIJK_EXPORT int tijk_3d_sym_to_esh_f(float *res, const float *ten,
                                     const tijk_type *type);

TIJK_EXPORT const tijk_type *tijk_esh_to_3d_sym_d(double *res,
                                                  const double *sh,
                                                  unsigned int order);
TIJK_EXPORT const tijk_type *tijk_esh_to_3d_sym_f(float *res,
                                                  const float *sh,
                                                  unsigned int order);

TIJK_EXPORT void tijk_esh_convolve_d(double *out, const double *in,
                                     const double *kernel,
                                     unsigned int order);
TIJK_EXPORT void tijk_esh_convolve_f(float *out, const float *in,
                                     const float *kernel,
                                     unsigned int order);

TIJK_EXPORT void tijk_esh_deconvolve_d(double *out, const double *in,
                                       const double *kernel,
                                       unsigned int order);
TIJK_EXPORT void tijk_esh_deconvolve_f(float *out, const float *in,
                                       const float *kernel,
                                       unsigned int order);

TIJK_EXPORT int tijk_esh_make_kernel_rank1_f(float *kernel, const float *signal,
                                             unsigned int order);
TIJK_EXPORT int tijk_esh_make_kernel_rank1_d(double *kernel,
                                             const double *signal,
                                             unsigned int order);
TIJK_EXPORT int tijk_esh_make_kernel_delta_f(float *kernel, const float *signal,
                                             unsigned int order);
TIJK_EXPORT int tijk_esh_make_kernel_delta_d(double *kernel,
                                             const double *signal,
                                             unsigned int order);

/* fsTijk.c */
/* for any given order, length is simply order+1; no need for a table */
TIJK_EXPORT const unsigned int tijk_max_efs_order;

TIJK_EXPORT unsigned int tijk_eval_efs_basis_d(double *res, unsigned int order,
                                               double phi);
TIJK_EXPORT unsigned int tijk_eval_efs_basis_f(float *res, unsigned int order,
                                               float phi);

TIJK_EXPORT double tijk_eval_efs_d(double *coeffs, unsigned int order,
                                   double phi);
TIJK_EXPORT float tijk_eval_efs_f(float *coeffs, unsigned int order,
                                  float phi);

TIJK_EXPORT int tijk_2d_sym_to_efs_d(double *res, const double *ten,
                                     const tijk_type *type);
TIJK_EXPORT int tijk_2d_sym_to_efs_f(float *res, const float *ten,
                                     const tijk_type *type);

TIJK_EXPORT const tijk_type *tijk_efs_to_2d_sym_d(double *res,
                                                  const double *fs,
                                                  unsigned int order);
TIJK_EXPORT const tijk_type *tijk_efs_to_2d_sym_f(float *res,
                                                  const float *fs,
                                                  unsigned int order);

/* enumsTijk.c */

/* tijk_class_* enum
 *
 * classes of objects Tijk deals with (e.g., tensors, SH series)
 */
enum {
  tijk_class_unknown,
  tijk_class_tensor,    /* 1: a tijk_type */
  tijk_class_esh,       /* 2: even-order spherical harmonic */
  tijk_class_efs,       /* 3: even-order fourier series */
  tijk_class_last
};
#define TIJK_CLASS_MAX 3

TIJK_EXPORT const airEnum *const tijk_class;

/* nrrdTijk.c */

TIJK_EXPORT int tijk_set_axis_tensor(Nrrd *nrrd, unsigned int axis,
                                     const tijk_type *type);

TIJK_EXPORT int tijk_set_axis_esh(Nrrd *nrrd, unsigned int axis,
                                  unsigned int order);

TIJK_EXPORT int tijk_set_axis_efs(Nrrd *nrrd, unsigned int axis,
                                  unsigned int order);

typedef struct tijk_axis_info_t {
  int tclass; /* class of Tijk object, from the tijk_class enum */
  unsigned int masked; /* whether or not values are masked */
  const tijk_type *type;
  unsigned int order;
} tijk_axis_info;

TIJK_EXPORT int tijk_get_axis_type(tijk_axis_info *info,
                                   const Nrrd *nrrd, unsigned int axis);

#ifdef __cplusplus
}
#endif

#endif /* TIJK_HAS_BEEN_INCLUDED */
