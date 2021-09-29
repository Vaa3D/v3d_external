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

#ifndef GAGE_HAS_BEEN_INCLUDED
#define GAGE_HAS_BEEN_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/nrrd.h>
#include <teem/ell.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(gage_EXPORTS) || defined(teem_EXPORTS)
#    define GAGE_EXPORT extern __declspec(dllexport)
#  else
#    define GAGE_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define GAGE_EXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define GAGE gageBiffKey

/*
** GAGE_DERIV_MAX is the maximum derivative that gage knows how to
** work with. The 0th derivative is the reonstructed value (no
** derivative).  Many short arrays are allocated to 1 + this value
**
** MUST KEEP IN SYNC with GAGE_DV_* macros below
*/
#define GAGE_DERIV_MAX 2

/*
** Convenience macros for arrays declared like blah[GAGE_DERIV_MAX+1]
** (so these need to be kept in sync with GAGE_DERIV_MAX above).  Note
** that "DV" is short for "derivative vector".  With consistent use of
** these, changes to GAGE_DERIV_MAX will either obviously break code
** (in an easy to fix way) or there will be no change needed at all.
**
*/
#define GAGE_DV_SET(v, d0, d1, d2) \
  ((v)[0] = (d0),                  \
   (v)[1] = (d1),                  \
   (v)[2] = (d2))

#define GAGE_DV_EQUAL(a, b)        \
  ((a)[0] == (b)[0] &&             \
   (a)[1] == (b)[1] &&             \
   (a)[2] == (b)[2])

#define GAGE_DV_COPY(a, b)         \
  ((a)[0] = (b)[0],                \
   (a)[1] = (b)[1],                \
   (a)[2] = (b)[2])

/*
** the only extent to which gage treats different axes differently is
** the spacing between samples along the axis.  To have different
** filters for the same function, but along different axes, would be
** too messy.  [Sun Mar 9 13:32:22 EDT 2008: Actually, doing per-axis
** kernels is looking more and more sensible all the time. . .] Thus,
** gage is not very useful as the engine for downsampling: it can't
** tell that along one axis samples should be blurred while they
** should be interpolated along another.  Rather, it assumes that the
** main task of probing is *reconstruction*: of values, of
** derivatives, or lots of different quantities
*/

/*
** terminology: gage is intended to measure multiple things at one
** point in a volume.  The SET of ALL the things that you want
** gage to measure is called the "QUERY".  Each of the many quantities
** comprising the query are called "ITEM"s.
*/

/*
******** gageParm.. enum
**
** these are passed to gageSet.  Look for like-wise named field of
** gageParm for documentation on what these mean.
**
** The following things have to agree:
** - gageParm* enum
** - fields of gageParm struct
** - analogous gageDef* defaults (their declaration and setting)
** - action of gageParmSet (ctx.c)
** - action of gageParmReset (miscGage.c)
*/
enum {
  gageParmUnknown,
  gageParmVerbose,                 /* non-boolean int */
  gageParmRenormalize,             /* int */
  gageParmCheckIntegrals,          /* int */
  gageParmK3Pack,                  /* int */
  gageParmGradMagCurvMin,          /* double */
  gageParmCurvNormalSide,          /* int */
  gageParmKernelIntegralNearZero,  /* double */
  gageParmDefaultCenter,           /* int */
  gageParmStackUse,                /* int */
  gageParmStackNormalizeDeriv,     /* int; does NOT imply gageParmStackUse */
  gageParmStackNormalizeDerivBias, /* double; does NOT imply      "        */
  gageParmStackNormalizeRecon,     /* int; does NOT imply         "        */
  gageParmOrientationFromSpacing,  /* int */
  gageParmGenerateErrStr,          /* int */
  gageParmLast
};

/* keep in synch with gageErr airEnum */
enum {
  gageErrUnknown,            /* 0: nobody knows */
  gageErrNone,               /* 1: no error, actually, all's well */
  gageErrBoundsSpace,        /* 2: out of 3-D (index-space) bounds */
  gageErrBoundsStack,        /* 3: out of 1-D bounds of stack */
  gageErrStackIntegral,      /* 4: stack recon coeff's sum to 0, so can't
                                   normalize them */
  gageErrStackSearch,        /* 5: for some reason couldn't find the index
                                   position of the probed stack location */
  gageErrStackUnused,        /* 6: can't probe stack without parm.stackUse */
  gageErrLast
};
#define GAGE_ERR_MAX            6

/*
******** gage{Ctx,Pvl}Flag.. enum
**
** organizes all the dependendies within a context and between a
** context and pervolumes.  This logic is to determine the support
** required for a query: different queries need different kernels,
** different kernels have different supports.  The user should not
** have to be concerned about any of this; it should be useful only
** to gageUpdate().
*/
enum {
  gageCtxFlagUnknown,
  gageCtxFlagNeedD,      /*  1: derivatives required for query */
  gageCtxFlagK3Pack,     /*  2: whether to use 3 or 6 kernels */
  gageCtxFlagNeedK,      /*  3: which of the kernels will actually be used */
  gageCtxFlagKernel,     /*  4: any one of the kernels or its parameters */
  gageCtxFlagRadius,     /*  5: radius of support for kernels with query */
  gageCtxFlagShape,      /*  6: a new pervolume shape was set */
  gageCtxFlagLast
};
#define GAGE_CTX_FLAG_MAX    6

enum {
  gagePvlFlagUnknown,
  gagePvlFlagVolume,     /*  1: got a new volume */
  gagePvlFlagQuery,      /*  2: what do you really care about */
  gagePvlFlagNeedD,      /*  3: derivatives required for query */
  gagePvlFlagLast
};
#define GAGE_PVL_FLAG_MAX    3


/*
******** gageKernel.. enum
**
** these are the different kernels that might be used in gage, regardless
** of what kind of volume is being probed.
** (synch with miscGage.c:gageKernel airEnum)
*/
enum {
  gageKernelUnknown,    /*  0: nobody knows */
  gageKernel00,         /*  1: measuring values */
  gageKernel10,         /*  2: reconstructing 1st derivatives */
  gageKernel11,         /*  3: measuring 1st derivatives */
  gageKernel20,         /*  4: reconstructing 1st partials and 2nd deriv.s */
  gageKernel21,         /*  5: measuring 1st partials for a 2nd derivative */
  gageKernel22,         /*  6: measuring 2nd derivatives */
  gageKernelStack,      /*  7: for reconstructing across a stack */
  gageKernelLast
};
#define GAGE_KERNEL_MAX     7

/*
******** GAGE_ITEM_PREREQ_MAXNUM
**
** Max number of prerequisites for any item in *any* kind.
**
** This value has gotten bumped periodically, which used to mean
** that *all* item tables had to be updated, when "-1" was used
** represent the unknown item.  But now that 0 represents the
** unknown item, and because struct elements are implicitly
** initialized to zero, this is no longer the case.
**
** Wed Nov  8 14:12:44 EST 2006 pre-emptively upping this from 6
*/
#define GAGE_ITEM_PREREQ_MAXNUM 8

/*
******** gageItemEntry struct
**
** necessary information about each item supported by the kind, which
** is defined at compile-time in a per-kind table (at least it is for
** the scalar, vector, and tensor kinds)
**
** NOTE!!! YOU CAN NOT re-arrange these variables, because of all the
** compile-time definitions that are done to define the gageKind->table
** for all current kinds.
*/
typedef struct {
  int enumVal;          /* the item's enum value */
  unsigned int
    answerLength;       /* how many double's are needed to store the answer,
                           or (for non-zero items), 0 to represent
                           "the length will be learned later at runtime" */
  int needDeriv,        /* what kind of derivative info is immediately needed
                           for this item (not recursively expanded). This is
                           NO LONGER a bitvector: values are 0, 1, 2, .. */
    prereq[GAGE_ITEM_PREREQ_MAXNUM],
                        /* what are the other items this item depends on
                           (not recusively expanded), you can list up to
                           GAGE_ITEM_PREREQ_MAXNUM of them, and use 0
                           (the unknown item) to fill out the list.
                           _OR_ -1 if this is a dynamic kind and the prereqs
                           will not be known until later in runtime */
    parentItem,         /* the enum value of an item (answerLength > 1)
                           containing the smaller value for which we are
                           merely an alias
                           _OR_ 0 if there's no parent */
    parentIndex,        /* where our value starts in parents value
                           _OR_ 0 if there's no parent */
    needData;           /* whether non-NULL gagePerVolume->data is needed
                           for answering this item */
} gageItemEntry;

/*
** modifying the enums below (scalar, vector, etc query quantities)
** necesitates modifying:
** - the central item table in scl.c, vecGage.c
** - the associated arrays in scl.c, vecGage.c
** - the "answer" method itself in sclanswer.c, vecGage.c
*/

/*
******** gageScl* enum
**
** all the "items" that gage can measure in a scalar volume.
**
** NOTE: although it is currently listed that way, it is not necessary
** that prerequisite measurements are listed before the other measurements
** which need them (that is represented by _gageSclPrereq)
**
** The description for each enum value starts with the numerical value
** followed by a string which identifies the value in the gageScl airEnum.
** The "[N]" indicates how many doubles are used for storing the quantity.
*/
enum {
  gageSclUnknown,      /*  0: nobody knows */
  gageSclValue,        /*  1: "v", data value: [1] */
  gageSclGradVec,      /*  2: "grad", gradient vector, un-normalized: [3] */
  gageSclGradMag,      /*  3: "gm", gradient magnitude: [1] */
  gageSclNormal,       /*  4: "n", gradient vector, normalized: [3] */
  gageSclNProj,        /*  5: "nproj", projection onto normal: [9] */
  gageSclNPerp,        /*  6: "np", projection onto tangent plane: [9] */
  gageSclHessian,      /*  7: "h", Hessian: [9] (column-order) */
  gageSclHessianTen,   /*  8: "ht", Hessian as 7-component tensor: [7]
                           In principle this is a dependency inversion
                           since gage doesn't know about the ten library,
                           where the 7-element tensor is based. */
  gageSclLaplacian,    /*  9: "l", Laplacian: Dxx + Dyy + Dzz: [1] */
  gageSclHessFrob,     /* 10: "hf", Frobenius norm of Hessian: [1] */
  gageSclHessEval,     /* 11: "heval", Hessian's eigenvalues: [3] */
  gageSclHessEval0,    /* 12: "heval0", Hessian's 1st eigenvalue: [1] */
  gageSclHessEval1,    /* 13: "heval1", Hessian's 2nd eigenvalue: [1] */
  gageSclHessEval2,    /* 14: "heval2", Hessian's 3rd eigenvalue: [1] */
  gageSclHessEvec,     /* 15: "hevec", Hessian's eigenvectors: [9] */
  gageSclHessEvec0,    /* 16: "hevec0", Hessian's 1st eigenvector: [3] */
  gageSclHessEvec1,    /* 17: "hevec1", Hessian's 2nd eigenvector: [3] */
  gageSclHessEvec2,    /* 18: "hevec2", Hessian's 3rd eigenvector: [3] */
  gageScl2ndDD,        /* 19: "2d", 2nd dir.deriv. along gradient: [1] */
  gageSclGeomTens,     /* 20: "gten", sym. matx w/ evals {0, K1, K2} and
                              evecs {grad, cdir0, cdir1}: [9] */
  gageSclGeomTensTen,  /* 21: "gtenten", 7-element geometry tensor [7] */
  gageSclK1,           /* 22: "k1", 1st principle curvature: [1] */
  gageSclK2,           /* 23: "k2", 2nd principle curvature (k2 <= k1): [1] */
  gageSclTotalCurv,    /* 24: "tc", L2 norm(K1,K2) (not Koen.'s "C"): [1] */
  gageSclShapeTrace,   /* 25, "st", (K1+K2)/Curvedness: [1] */
  gageSclShapeIndex,   /* 26: "si", Koen.'s shape index, ("S"): [1] */
  gageSclMeanCurv,     /* 27: "mc", mean curvature (K1 + K2)/2: [1] */
  gageSclGaussCurv,    /* 28: "gc", gaussian curvature K1*K2: [1] */
  gageSclCurvDir1,     /* 29: "cdir1", 1st principle curv direction: [3] */
  gageSclCurvDir2,     /* 30: "cdir2", 2nd principle curv direction: [3] */
  gageSclFlowlineCurv, /* 31: "fc", curvature of normal streamline: [1] */
  gageSclMedian,       /* 32: "med", median filter */
  gageSclHessValleyness,   /* 33: "hvalley", vallyness measure: [1] */
  gageSclHessRidgeness,    /* 34: "hridge", ridgeness measure: [1] */
  gageSclHessMode,     /* 35: "hmode", Hessian's mode: [1] */
  gageSclLast
};
#define GAGE_SCL_ITEM_MAX 35

/*
******** gageVec* enum
**
** all the "items" that gage knows how to measure in a 3-vector volume
**
** The strings gives one of the gageVec airEnum identifiers, and [x]
** says how many scalars are associated with this value.
*/
enum {
  gageVecUnknown,         /*  0: nobody knows */
  gageVecVector,          /*  1: "v", component-wise-interpolated
                                 (CWI) vec: [3] */
  gageVecVector0,         /*  2: "v0", vector[0]: [1] */
  gageVecVector1,         /*  3: "v1", vector[0]: [1] */
  gageVecVector2,         /*  4: "v2", vector[0]: [1] */
  gageVecLength,          /*  5: "l", length of CWI vector: [1] */
  gageVecNormalized,      /*  6: "n", normalized CWI vector: [3] */
  gageVecJacobian,        /*  7: "j", component-wise Jacobian: [9]
                                0:dv_x/dx  1:dv_x/dy  2:dv_x/dz
                                3:dv_y/dx  4:dv_y/dy  5:dv_y/dz
                                6:dv_z/dx  7:dv_z/dy  8:dv_z/dz */
  gageVecStrain,          /*  8: "S", rate-of-strain tensor: [9] */
  gageVecDivergence,      /*  9: "d", divergence (based on Jacobian): [1] */
  gageVecCurl,            /* 10: "c", curl (based on Jacobian): [3] */
  gageVecCurlNorm,        /* 11: "cm", curl magnitude: [1] */
  gageVecHelicity,        /* 12: "h", helicity: vec . curl: [1] */
  gageVecNormHelicity,    /* 13: "nh", normalized helicity: [1] */
  gageVecSOmega,          /* 14: "somega", S squared + Omega squared: [9] */
  gageVecLambda2,         /* 15: "lambda2", lambda2 criterion: [1] */
  gageVecImaginaryPart,   /* 16: "imag", imag. part of jacobian's
                                  complex-conjugate eigenvalues: [1] */
  gageVecHessian,         /* 17: "vh", second-order derivative: [27]
                                 HEY: indices here need to be double checked
                                 0:d2v_x/dxdx   1:d2v_x/dxdy   2:d2v_x/dxdz
                                 3:d2v_x/dydx   4:d2v_x/dydy   5:d2v_x/dydz
                                 6:d2v_x/dzdx   7:d2v_x/dzdy   8:d2v_x/dzdz
                                 9:d2v_y/dxdx       [..]
                                    [..]
                                24:dv2_z/dzdx  25:d2v_z/dzdy  26:d2v_z/dzdz */
  gageVecDivGradient,     /* 18: "dg", divergence gradient: [3] */
  gageVecCurlGradient,    /* 19: "cg", curl gradient: [9] */
  gageVecCurlNormGrad,    /* 20: "cng", curl norm gradient: [3] */
  gageVecNCurlNormGrad,   /* 21: "ncng", normalized curl norm gradient: [3] */
  gageVecHelGradient,     /* 22: "hg", helicity gradient: [3] */
  gageVecDirHelDeriv,     /* 23: "dhd", directional derivative
                                 of helicity: [1] */
  gageVecProjHelGradient, /* 24: "phg", projected helicity gradient: [3] */
  gageVecGradient0,       /* 25: "g0", gradient of 1st coeff of vector: [3] */
  gageVecGradient1,       /* 26: "g1", gradient of 2nd coeff of vector: [3] */
  gageVecGradient2,       /* 27: "g2", gradient of 3rd coeff of vector: [3] */
  gageVecMultiGrad,       /* 28: "mg", sum of outer products of grads: [9] */
  gageVecMGFrob,          /* 29: "mgfrob", frob norm of multi-gradient: [1] */
  gageVecMGEval,          /* 30: "mgeval", evals of multi-gradient: [3] */
  gageVecMGEvec,          /* 31: "mgevec", evecs of multi-gradient: [9] */
  gageVecLast
};
#define GAGE_VEC_ITEM_MAX     31

struct gageKind_t;       /* dumb forward declaraction, ignore */
struct gagePerVolume_t;  /* dumb forward declaraction, ignore */

/*
******** gageItemPackPart* enum
**
** the different ways that some items can be related for a gagePack
*/
enum {
  gageItemPackPartUnknown,     /*  0 */
  gageItemPackPartScalar,      /*  1 */
  gageItemPackPartGradVec,     /*  2 */
  gageItemPackPartGradMag,     /*  3 */
  gageItemPackPartNormal,      /*  4 */
  gageItemPackPartHessian,     /*  5 */
  gageItemPackPartHessEval0,   /*  6 */
  gageItemPackPartHessEval1,   /*  7 */
  gageItemPackPartHessEval2,   /*  8 */
  gageItemPackPartHessEvec0,   /*  9 */
  gageItemPackPartHessEvec1,   /* 10 */
  gageItemPackPartHessEvec2,   /* 11 */
  gageItemPackPartLast
};
#define GAGE_ITEM_PACK_PART_MAX   11

/*
******** gageShape struct
**
** just a container for all the information related to the "shape"
** of all the volumes associated with a context
**
** Note that the utility of gageShape has extended well beyond doing
** convolution-based measurements in volumes- it has become the
** one-stop place for all of Teem to figure out a reasonable way of
** locating a logically a volume in 3-D space, including using a
** nrrd's full orientation information if it is known.
*/
typedef struct gageShape_t {
  /* ========= INPUT ========= (controls for _gageShapeSet) */
  int defaultCenter,          /* default centering to use when given volume
                                 has no centering set. */
    orientationFromSpacing;   /* only meaningful if nrrd has per-axis spacing,
                                 but not full orientation info. If zero, the
                                 volume is crammed into the bi-unit cube.
                                 If non-zero, gage treats the volume as if it
                                 had axis-aligned spaceDirection vectors, with
                                 the non-zero values determined by the given
                                 per-axis spacing. */
  /* ======== OUTPUT ========= (set by _gageShapeSet) */
  int center,                 /* the sample centering of the volume(s)- this
                                 determines the extent of the locations
                                 that may be probed */
    fromOrientation;          /* non-zero iff the spaceDirections and
                                 spaceOrigin information was used */
  unsigned int size[3];       /* raster dimensions of volume
                                 HEY: shouldn't this be size_t? */
  double spacing[3];          /* spacings for each axis */
  /* fwScale[GAGE_KERNEL_MAX+1][3] has been superceded by the cleaner and
     more general ItoWSubInvTransp and ItoWSubInv matrices below */
  double ItoW[16],            /* homogeneous coord transform from index
                                 to world space (defined either by bi-unit
                                 cube or from full orientation info ) */
    WtoI[16],                 /* inverse of above */
    ItoWSubInvTransp[9],      /* inverse transpose of 3x3 sub-matrix of ItoW,
                                 to transform (covariant) gradients */
    ItoWSubInv[9];            /* tranpose of above, to transform hessians */

} gageShape;

/*
******** gageParm struct
**
** a container for the various switches and knobs which control
** gage, aside from the obvious inputs (kernels, queries, volumes)
*/
typedef struct gageParm_t {
  int renormalize;            /* hack to make sure that sum of
                                 discrete value reconstruction weights
                                 is same as kernel's continuous
                                 integral, and that the 1nd and 2nd
                                 deriv weights really sum to 0.0 */
  int checkIntegrals;         /* call the "integral" method of the
                                 kernel to verify that it is
                                 appropriate for the task for which
                                 the kernel is being set:
                                 reconstruction: 1.0, derivatives: 0.0 */
  int k3pack;                 /* non-zero (true) iff we do not use kernels
                                 gageKernelIJ with I != J. So, we use the
                                 value reconstruction kernel (gageKernel00)
                                 for 1st and 2nd derivative reconstruction,
                                 and so on. This is faster because we can
                                 re-use results from low-order convolutions.
                                 The name "3pack" will likely persist even
                                 with 3rd dervatives, for which "4pack" would
                                 make more sense (for 00, 11, 22, 33) */
  double gradMagCurvMin,      /* punt on computing curvature information if
                                 gradient magnitude is less than this. Yes,
                                 this is scalar-kind-specific, but there's
                                 no other good place for it */
    kernelIntegralNearZero,   /* tolerance with checkIntegrals on derivative
                                 kernels */
    stackNormalizeDerivBias;  /* when using stackNormalizeDeriv, a bias
                                 on the effective scale, used for the
                                 normalization */
  int curvNormalSide,         /* determines direction of gradient that is used
                                 as normal in curvature calculations, exactly
                                 the same as miteUser's normalSide: 1 for
                                 normal pointing to lower values (higher
                                 values are more "inside"); -1 for normal
                                 pointing to higher values (low values more
                                 "inside") */
    defaultCenter,            /* what centering to use when you have to invent
                                 one, because its not set on any axis */
    stackUse,                 /* if non-zero: treat the pvl's (all same kind)
                                 as multiple values of a single logical volume
                                 (e.g. for approximating scale space).
                                 The first pvl is effectively just a buffer;
                                 the N-1 pvls used are at index 1 through N-2.
                                 The query in pvl[0] will determine the
                                 computations done, and answers for the whole
                                 stack will be stored in pvl[0]. */
    stackNormalizeRecon,      /* if non-zero (and if stackUse is non-zero):
                                 the weights used to reconstruct across
                                 stack samples are normalized to unit sum; not
                                 needed if the kernel is accurate enough */
    stackNormalizeDeriv,      /* if non-zero (and if stackUse is non-zero):
                                 derivatives at filter stage (before answer
                                 stage) are renormalized based on the stack
                                 position */
    orientationFromSpacing,   /* only meaningful if nrrd has per-axis spacing,
                                 but not full orientation info. If zero, the
                                 volume is crammed into the bi-unit cube.
                                 If non-zero, gage treats the volume as if it
                                 had axis-aligned spaceDirection vectors, with
                                 the non-zero values determined by the given
                                 per-axis spacing. */
    generateErrStr;           /* when errors happen, biff is never used, but
                                 a descriptive error is sprintf into
                                 gctx->errStr as long as this is non-zero. */
} gageParm;

/*
******** gagePoint struct
**
** stores *Index Space* location of last query location, which is used
** to determine whether the ctx->fsl, ctx->fw values can be re-used
** (based on the "Frac" values), and, whether all the pvl->iv3 have to
** be refilled (based on the "Idx" values).  The last index (frac[3]
** and idx[3])is for the stack, and can safely stay 0 if the stack
** isn't being used.
**
** with stack usage, stackFwNonZeroNum records how many pvls had
** non-zero stack filter weights, which is used to detect when
** iv3s have to be refilled.  Looking at idx[3] alone is not always
** sufficient for this.
*/
typedef struct gagePoint_t {
  double frac[4];         /* last fractional voxel location */
  unsigned int idx[4],    /* last integral voxel location; actually the
                             *upper* corner of the containing voxel */
    stackFwNonZeroNum;    /* last number of non-zero values of stack filter
                             weights (ctx->stackFw) */
} gagePoint;

/*
******** gageQuery typedef
**
** this short, fixed-length array is used as a bit-vector to store
** all the items that comprise a query.  Its length sets an upper
** bound on the maximum item value that a gageKind may use.
**
** The important thing to keep in mind is that a variable of type
** gageKind ends up being passed by reference, even though the
** syntax of its usage doesn't immediately announce that.
**
** gageKindCheck currently has the role of verifying that a gageKind's
** maximum item value is within the bounds set here. Using
** GAGE_QUERY_BYTES_NUM == 8 gives a max item value of 63, which is
** far above anything being used now.
**
** Sat Jan 21 18:12:01 EST 2006: ha! second derivatives of tensors blew
** past old GAGE_QUERY_BYTES_NUM, now GAGE_QUERY_BYTES_NUM == 16
**
** Tue Nov  7 19:51:05 EST 2006; tenGage items now pushing 127,
** guardedly changing GAGE_QUERY_BYTES_NUM to 24 --> max item 191
**
** Wed Nov 18 17:53:23 CST 2009; yikes, tenGage items now at 190,
** changing GAGE_QUERY_BYTES_NUM to 32 --> max item 255
**
** NOTE: increasing GAGE_QUERY_BYTES_NUM means that the macros below
** have to be redefined as well!
*/
#define GAGE_QUERY_BYTES_NUM 32
#define GAGE_ITEM_MAX ((8*GAGE_QUERY_BYTES_NUM)-1)
typedef unsigned char gageQuery[GAGE_QUERY_BYTES_NUM];

/*
******** GAGE_QUERY_RESET, GAGE_QUERY_TEST
******** GAGE_QUERY_ON, GAGE_QUERY_OFF
**
** Macros for manipulating a gageQuery.
**
** airSanity ensures that an unsigned char is in fact 8 bits
*/
#define GAGE_QUERY_RESET(q) \
  q[ 0] = q[ 1] = q[ 2] = q[ 3] = \
  q[ 4] = q[ 5] = q[ 6] = q[ 7] = \
  q[ 8] = q[ 9] = q[10] = q[11] = \
  q[12] = q[13] = q[14] = q[15] = \
  q[16] = q[17] = q[18] = q[19] = \
  q[20] = q[21] = q[22] = q[23] = \
  q[24] = q[25] = q[26] = q[27] = \
  q[28] = q[29] = q[30] = q[31] = 0

#define GAGE_QUERY_COPY(p, q) \
  p[ 0] = q[ 0]; p[ 1] = q[ 1]; p[ 2] = q[ 2]; p[ 3] = q[ 3]; \
  p[ 4] = q[ 4]; p[ 5] = q[ 5]; p[ 6] = q[ 6]; p[ 7] = q[ 7]; \
  p[ 8] = q[ 8]; p[ 9] = q[ 9]; p[10] = q[10]; p[11] = q[11]; \
  p[12] = q[12]; p[13] = q[13]; p[14] = q[14]; p[15] = q[15]; \
  p[16] = q[16]; p[17] = q[17]; p[18] = q[18]; p[19] = q[19]; \
  p[20] = q[20]; p[21] = q[21]; p[22] = q[22]; p[23] = q[23]; \
  p[24] = q[24]; p[25] = q[25]; p[26] = q[26]; p[27] = q[27]; \
  p[28] = q[28]; p[29] = q[29]; p[30] = q[30]; p[31] = q[31]

#define GAGE_QUERY_ADD(p, q) \
  p[ 0] |= q[ 0]; p[ 1] |= q[ 1]; p[ 2] |= q[ 2]; p[ 3] |= q[ 3]; \
  p[ 4] |= q[ 4]; p[ 5] |= q[ 5]; p[ 6] |= q[ 6]; p[ 7] |= q[ 7]; \
  p[ 8] |= q[ 8]; p[ 9] |= q[ 9]; p[10] |= q[10]; p[11] |= q[11]; \
  p[12] |= q[12]; p[13] |= q[13]; p[14] |= q[14]; p[15] |= q[15]; \
  p[16] |= q[16]; p[17] |= q[17]; p[18] |= q[18]; p[19] |= q[19]; \
  p[20] |= q[20]; p[21] |= q[21]; p[22] |= q[22]; p[23] |= q[23]; \
  p[24] |= q[24]; p[25] |= q[25]; p[26] |= q[26]; p[27] |= q[27]; \
  p[28] |= q[28]; p[29] |= q[29]; p[30] |= q[30]; p[31] |= q[31]

#define GAGE_QUERY_EQUAL(p, q) ( \
  p[ 0] == q[ 0] && p[ 1] == q[ 1] && p[ 2] == q[ 2] && p[ 3] == q[ 3] && \
  p[ 4] == q[ 4] && p[ 5] == q[ 5] && p[ 6] == q[ 6] && p[ 7] == q[ 7] && \
  p[ 8] == q[ 8] && p[ 9] == q[ 9] && p[10] == q[10] && p[11] == q[11] && \
  p[12] == q[12] && p[13] == q[13] && p[14] == q[14] && p[15] == q[15] && \
  p[16] == q[16] && p[17] == q[17] && p[18] == q[18] && p[19] == q[19] && \
  p[20] == q[20] && p[21] == q[21] && p[22] == q[22] && p[23] == q[23] && \
  p[24] == q[24] && p[25] == q[25] && p[26] == q[26] && p[27] == q[27] && \
  p[28] == q[28] && p[29] == q[29] && p[30] == q[30] && p[31] == q[31] )

#define GAGE_QUERY_NONZERO(q) ( \
  q[ 0] | q[ 1] | q[ 2] | q[ 3] | \
  q[ 4] | q[ 5] | q[ 6] | q[ 7] | \
  q[ 8] | q[ 9] | q[10] | q[11] | \
  q[12] | q[13] | q[14] | q[15] | \
  q[16] | q[17] | q[18] | q[19] | \
  q[20] | q[21] | q[22] | q[23] | \
  q[24] | q[25] | q[26] | q[27] | \
  q[28] | q[29] | q[30] | q[31] )

#define GAGE_QUERY_ITEM_TEST(q, i) (q[i/8] & (1 << (i % 8)))
#define GAGE_QUERY_ITEM_ON(q, i) (q[i/8] |= (1 << (i % 8)))
#define GAGE_QUERY_ITEM_OFF(q, i) (q[i/8] &= ~(1 << (i % 8)))

/* increment for ctx->pvlArr airArray */
#define GAGE_PERVOLUME_ARR_INCR 32

/* extents of known information about optimal sigma samples for
   Hermite-spline-based scale-space reconstruction */
#define GAGE_OPTIMSIG_SIGMA_MAX 11
#define GAGE_OPTIMSIG_SAMPLES_MAXNUM 11

/*
******** gageContext struct
**
** The information here is specific to the dimensions, scalings, and
** radius of kernel support, but not to kind of volume (all kind-specific
** information is in the gagePerVolume).  One context can be used in
** conjuction with probing multiple volumes.
*/
typedef struct gageContext_t {
  /* INPUT ------------------------- */
  int verbose;                /* verbosity */
  gageParm parm;              /* all parameters */

  /* all the kernels we'll ever need, including the stack kernel */
  NrrdKernelSpec *ksp[GAGE_KERNEL_MAX+1];

  /* all the pervolumes attached to this context.  If using stack,
     the base pvl is the LAST, pvl[pvlNum-1], and the stack samples
     are pvl[0] through pvl[pvlNum-2] */
  struct gagePerVolume_t **pvl;

  /* number of pervolumes currently attached. If using stack,
     this is one more than number of stack samples (because of the
     base volume at the end) */
  unsigned int pvlNum;

  /* airArray for managing pvl and pvlNum */
  airArray *pvlArr;

  /* sizes, spacings, centering, and other geometric aspects of the
     volume */
  gageShape *shape;

  /* if stack is being used, allocated for length pvlNum-1, and
     stackPos[0] through stackPos[pvlNum-2] MUST exist and be
     monotonically increasing stack positions for each volume.
     Otherwise NULL */
  double *stackPos;

  /* INTERNAL ------------------------- */
  /* if using stack: allocated for length pvlNum-1, and filter sample
     locations and weights for reconstruction along the stack.
     Otherwise NULL. */
  double *stackFsl;
  double *stackFw;

  /* all the flags used by gageUpdate() used to describe what changed
     in this context */
  int flag[GAGE_CTX_FLAG_MAX+1];

  /* which value/derivatives need to be calculated for all pervolumes
     (doV, doD1, doD2) */
  int needD[GAGE_DERIV_MAX+1];

  /* which kernels are needed for all pvls.  needK[gageKernelStack]
     is currently not set by the update function that sets needK[] */
  int needK[GAGE_KERNEL_MAX+1];

  /* radius of support of samples needed to satisfy query, given the
     set of kernels.  The "filter diameter" fd == 2*radius.  This is
     incremented by one when filtering across the stack with
     nrrdKernelHermiteScaleSpaceFlag */
  unsigned int radius;

  /* filter sample locations (all axes): logically a fd x 3 array
     (and its 3 because gage works on 3D fields, not because of
     the number of derivatives supported) */
  double *fsl;

  /* filter weights (all axes, all kernels): logically a
     fd x 3 x GAGE_KERNEL_MAX+1 (fast-to-slow) array */
  double *fw;

  /* offsets to other fd^3 samples needed to fill 3D intermediate
     value cache. Allocated size is dependent on kernels, values
     inside are dependent on the dimensions of the volume. It may be
     more correct to be using size_t instead of uint, but the X and Y
     dimensions of the volume would have to be super-outrageous for
     that to be a problem */
  unsigned int *off;

  /* last probe location */
  gagePoint point;

  /* errStr and errNum are for describing errors that happen in gageProbe():
     using biff is too heavy-weight for this, and the idea is that no ill
     should occur if the error is repeatedly ignored. Whether or not
     something is actually sprintf'ed into errStr is controlled by
     parm.generateErrStr.
     NOTE: these variables used to be globals "gageErrStr" and "gageErrNum" */
  char errStr[AIR_STRLEN_LARGE];
  int errNum;                 /* takes values from the gageErr enum */

  /* what fraction of the values in the kernel support had to be invented
     (by bleeding out the margin) in order to satisfy a probe that was near
     the edge (any axis, either high or low) of the volume */
  double edgeFrac;
} gageContext;

/*
******** gagePerVolume
**
** information that is specific to one volume, and to one kind of
** volume.
*/
typedef struct gagePerVolume_t {
  int verbose;                /* verbosity */
  const struct gageKind_t *kind;  /* what kind of volume is this for */
  gageQuery query;            /* the query, recursively expanded */
  int needD[GAGE_DERIV_MAX+1];/* which derivatives need to be calculated for
                                 the query (above) in this volume */
  const Nrrd *nin;            /* the volume to sample within */
  int flag[GAGE_PVL_FLAG_MAX+1];/* for the kind-specific flags .. */
  double *iv3, *iv2, *iv1;    /* 3D, 2D, 1D, value caches.  These are cubical,
                                 square, and linear arrays, all length fd on
                                 each edge.  Currently gageIv3Fill() fills
                                 the iv3 (at a latter point this will be
                                 delegated back to the gageKind to facilitate
                                 bricking), and currently the tuple axis (with
                                 length valLen) always slowest.  However, use
                                 of iv2 and iv1 is entirely up the kind's
                                 filter method. */
  double (*lup)(const void *ptr, size_t I);
                              /* nrrd{F,D}Lookup[] element, according to
                                 nin->type and double */
  double *answer;             /* main buffer to hold all the answers */
  double **directAnswer;      /* array of pointers into answer */
  void *data;                 /* extra data, parameters, buffers, etc.
                                 required for answering some items (as per
                                 the gageItemEntry->needData) managed with
                                 kind->pvlDataNew, kind->pvlDataCopy,
                                 kind->pvlDataUpdate, and kind->pvlDataNix,
                                 so there is no channel for extra info to be
                                 passed into the pvl->data, other that what
                                 was put into kind->data */
} gagePerVolume;

/*
******** gageKind struct
**
** all the information and functions that are needed to handle one
** kind of volume (such as scalar, vector, etc.)
**
** these are either statically allocated (e.g. gageKindScl, gageKindVec,
** tenGageKind), or dynamically allocated, which case the kind itself
** needs a constructor (e.g. tenDwiGageKindNew()).  The "dynamicAlloc"
** variable indicates this distinction.
**
** Having dynamically allocated kinds raises the possibility of having
** to set and update (or modify and update) their internal state,
** which is currently completely outside the update framework of gage.
** So as far as the core gage functions are concerned, all kinds are
** static, because there is nothing to modify.  It also means that
** those who dynamically create kinds should try to minimize the
** state info that can/must be dynamically modified (i.e. maybe
** the kind constructor should take all the various parameters,
** and set everything in a single shot).
*/
typedef struct gageKind_t {
  int dynamicAlloc;                 /* non-zero if this kind struct was
                                       dynamically allocated */
  char name[AIR_STRLEN_SMALL];      /* short identifying string for kind */
  const airEnum *enm;               /* such as gageScl.  NB: the "unknown"
                                       value in the enum *must* be 0. */
  unsigned int baseDim,             /* dimension that x,y,z axes start on
                                       (e.g. 0 for scalars, 1 for vectors) */
    valLen;                         /* number of scalars per data point,
                                       -or- 0 to represent "this value will
                                       be learned later at runtime" */
  int itemMax;                      /* such as GAGE_SCL_ITEM_MAX */
  gageItemEntry *table;             /* array of gageItemEntry's, indexed
                                       by the item value,
                                       -or- NULL if the table cannot be
                                       statically allocated (not because it
                                       can come in different sizes, but
                                       because it needs to be a modified
                                       version of the compile-time table */
  void (*iv3Print)(FILE *,          /* such as _gageSclIv3Print() */
                   gageContext *,
                   gagePerVolume *),
    (*filter)(gageContext *,        /* such as _gageSclFilter() */
              gagePerVolume *),
    (*answer)(gageContext *,        /* such as _gageSclAnswer() */
              gagePerVolume *),
    /* for allocating, copying, updating, and nixing the pervolume->data */
    /* pvlDataNew and pvlDataCopy can use biff, but:
       --> they must use GAGE key (and not callback's library's key), and
       --> pvlDataNix can not use biff */
    *(*pvlDataNew)(const struct gageKind_t *kind),
    *(*pvlDataCopy)(const struct gageKind_t *kind, const void *pvlDataOld),
    *(*pvlDataNix)(const struct gageKind_t *kind, void *pvlData);
  int (*pvlDataUpdate)(const struct gageKind_t *kind,
                       const gageContext *ctx,
                       const gagePerVolume *pvl,
                       const void *data);
  void *data;                       /* extra information about the kind of
                                       volume that's being probed.  Likely
                                       used by filter, answer, and the
                                       pvlData functions */
} gageKind;

/*
******** gageItemSpec struct
**
** dumb little way to store a kind/item pair.  Formerly known
** as a gageQuerySpec.
*/
typedef struct {
  const gageKind *kind;       /* the kind of the volume to measure */
  int item;                   /* the quantity to measure */
} gageItemSpec;

/*
******** gageItemPack struct
**
** A way of coherently representing a related set of gageItems. This
** kind of inter-item relationship may eventually be part of the
** definition of a gageKind.
**
** These are intended to be set up at compile-time, just like nearly all
** the gageKinds.
**
** This is not to be confused with the gageMultiItem, which is (or
** will be) a list of items, set at run-time, to learn from a given
** volume.
*/
typedef struct {
  const gageKind *kind;
  int item[GAGE_ITEM_PACK_PART_MAX+1];
} gageItemPack;

/*
******** gageStackBlurParm struct
**
** all parameters associated with blurring one volume to form a "stack"
*/
typedef struct {
  unsigned int num;      /* # of blurring scales == # volumes */
  double *scale;         /* scale parameter for each blurred volume */
  double sigmaMax,       /* nrrdKernelDiscreteGaussian is implemented with
                            airBesselInExpScaled, which has numerical issues
                            for very large kernel sizes.  Instead of doing
                            the blurring in one step, the diffusion is done
                            iteratively, with steps in diffusion time
                            of sigmaMax^2 */
    padValue;            /* padding value for nrrdBoundaryPad */
  NrrdKernelSpec *kspec; /* NOTE: parm[0] will get over-written as part
                            of running the resampler at each scale */
  int dataCheck,         /* when checking given stack to see if its the
                            blurring of a volume, actually go in and look at
                            values of first (probably the least blurred)
                            volume */
    boundary,            /* passed to nrrdResampleBoundarySet */
    renormalize,         /* passed to nrrdResampleRenormalizeSet */
    verbose;
} gageStackBlurParm;

/*
******** gageOptimSigParm struct
**
** a fairly disorganized mess of parameters.  under construction
*/
typedef struct {
  /* INPUT ------------------------- */
  /* these determine the allocation and (slow) computation of ntruth */
  unsigned int dim;            /* either 1, 2, or 3 */
  double sigmaMax,             /* highest sigma in current computation */
    cutoff;                    /* parm[1] for discrete gaussian kernel */
  unsigned int measrSampleNum; /* how many samples along sigma to use
                                  for measurements of error */

  /* these can be changed more often */
  unsigned int sampleNum;      /* how many scale samples to optimize */
  int volMeasr,                /* how to measure error at each reconstructed
                                  scale (interpolated volume) */
    lineMeasr,                 /* how to summarize errors across all scales */
    plotting,                  /* we're plotting, not optimizing */
    tentRecon;                 /* for plotting: use tent instead of hermite */
  unsigned int maxIter;        /* allowed iterations in optimization */
  double convEps;              /* convergence threshold */

  /* INTERNAL ------------------------- */
  /* these related to the allocation and (slow) computation of ntruth */
  unsigned int sx, sy, sz;     /* volume size for testing */
  double *sigmatru,            /* sigmas for all lines of ntruth, allocated
                                  for measrSampleNum */
    *truth;                    /* data pointer of ntruth */
  Nrrd *ntruth,                /* big array of all truth volumes, logically
                                  a sx x sy x sz x measrSampleNum array */
    *nerr,                     /* line of all errors, across scale */
    *ntruline,                 /* *wrapper* around some scanline of ntruth */
    *ninterp,                  /* last recon result */
    *ndiff;                    /* diff between recon and truth, single vol */
  /* most of these allocated according sampleNumMax */
  unsigned int sampleNumMax;   /* largest number of SS samples to look at;
                                  this is set at parm creation time and can't
                                  be changed safely during parm lifetime */
  double *scalePos,            /* current SS sample locations, allocated
                                  for sampleNumMax */
    *step;                     /* per-point stepsize for descent */
  Nrrd **nsampvol;             /* current set of SS samples, allocated for
                                  sampleNumMax */
  gagePerVolume *pvl, **pvlSS; /* for gage; pvlSS allocation different than
                                  scalePos or nsampvol */
  gageContext *gctx;           /* context around nsamplevol */

  /* OUTPUT ------------------------- */
  double finalErr;             /* error of converged points */
} gageOptimSigParm;

/* defaultsGage.c */
GAGE_EXPORT const char *gageBiffKey;
GAGE_EXPORT int gageDefVerbose;
GAGE_EXPORT double gageDefGradMagCurvMin;
GAGE_EXPORT int gageDefRenormalize;
GAGE_EXPORT int gageDefCheckIntegrals;
GAGE_EXPORT int gageDefK3Pack;
GAGE_EXPORT int gageDefCurvNormalSide;
GAGE_EXPORT double gageDefKernelIntegralNearZero;
GAGE_EXPORT int gageDefDefaultCenter;
GAGE_EXPORT int gageDefStackUse;
GAGE_EXPORT int gageDefStackNormalizeRecon;
GAGE_EXPORT int gageDefStackNormalizeDeriv;
GAGE_EXPORT double gageDefStackNormalizeDerivBias;
GAGE_EXPORT double gageDefStackBlurSigmaMax;
GAGE_EXPORT int gageDefOrientationFromSpacing;
GAGE_EXPORT int gageDefGenerateErrStr;

/* miscGage.c */
GAGE_EXPORT const int gagePresent;
GAGE_EXPORT double gageZeroNormal[3];
GAGE_EXPORT const airEnum *const gageErr;
GAGE_EXPORT const airEnum *const gageKernel;
GAGE_EXPORT const airEnum *const gageItemPackPart;
GAGE_EXPORT void gageParmReset(gageParm *parm);
GAGE_EXPORT void gagePointReset(gagePoint *point);
GAGE_EXPORT gageItemSpec *gageItemSpecNew(void);
GAGE_EXPORT void gageItemSpecInit(gageItemSpec *isp);
GAGE_EXPORT gageItemSpec *gageItemSpecNix(gageItemSpec *isp);

/* kind.c */
GAGE_EXPORT int gageKindCheck(const gageKind *kind);
GAGE_EXPORT unsigned int gageKindTotalAnswerLength(const gageKind *kind);
GAGE_EXPORT unsigned int gageKindAnswerLength(const gageKind *kind, int item);
GAGE_EXPORT int gageKindAnswerOffset(const gageKind *kind, int item);
GAGE_EXPORT int gageKindVolumeCheck(const gageKind *kind, const Nrrd *nrrd);

/* print.c */
GAGE_EXPORT void gageQueryPrint(FILE *file, const gageKind *kind,
                                gageQuery query);

/* sclfilter.c */
typedef void (gageScl3PFilter_t)(gageShape *shape,
                                 double *iv3, double *iv2, double *iv1,
                                 double *fw00, double *fw11, double *fw22,
                                 double *val, double *gvec, double *hess,
                                 const int *needD);
GAGE_EXPORT gageScl3PFilter_t gageScl3PFilter2;
GAGE_EXPORT gageScl3PFilter_t gageScl3PFilter4;
GAGE_EXPORT gageScl3PFilter_t gageScl3PFilter6;
GAGE_EXPORT gageScl3PFilter_t gageScl3PFilter8;
GAGE_EXPORT void gageScl3PFilterN(gageShape *shape, int fd,
                                  double *iv3, double *iv2, double *iv1,
                                  double *fw00, double *fw11, double *fw22,
                                  double *val, double *gvec, double *hess,
                                  const int *needD);

/* scl.c */
GAGE_EXPORT const airEnum *const gageScl;
/* HEY: this should perhaps be a "const gageKind", but pointers for
   general kinds may want to point to this, or to the return of
   a dynamic kind generator like tenDwiGageKindNew(), which is
   most certainly not "const gageKind". */
GAGE_EXPORT gageKind *const gageKindScl;
GAGE_EXPORT const gageItemPack *const gageItemPackSclValue;

/* vecGage.c (together with vecprint.c, these contain everything to
   implement the "vec" kind, and could be used as examples of what it
   takes to create a new gageKind) */
GAGE_EXPORT const airEnum *const gageVec;
GAGE_EXPORT gageKind *const gageKindVec;

/* shape.c */
GAGE_EXPORT void gageShapeReset(gageShape *shp);
GAGE_EXPORT gageShape *gageShapeNew(void);
GAGE_EXPORT gageShape *gageShapeCopy(const gageShape *shp);
GAGE_EXPORT gageShape *gageShapeNix(gageShape *shape);
GAGE_EXPORT int gageShapeSet(gageShape *shp, const Nrrd *nin, int baseDim);
GAGE_EXPORT void gageShapeWtoI(const gageShape *shape,
                               double index[3], const double world[3]);
GAGE_EXPORT void gageShapeItoW(const gageShape *shape,
                               double world[3], const double index[3]);
GAGE_EXPORT int gageShapeEqual(const gageShape *shp1, const char *name1,
                               const gageShape *shp2, const char *name2);
GAGE_EXPORT void gageShapeBoundingBox(double min[3], double max[3],
                                      const gageShape *shape);

/* the organization of the next two files used to be according to
   what the first argument is, not what appears in the function name,
   but that's just a complete mess now */
/* pvl.c */
GAGE_EXPORT int gageVolumeCheck(const gageContext *ctx, const Nrrd *nin,
                                const gageKind *kind);
GAGE_EXPORT gagePerVolume *gagePerVolumeNew(gageContext *ctx,
                                            const Nrrd *nin,
                                            const gageKind *kind);
GAGE_EXPORT gagePerVolume *gagePerVolumeNix(gagePerVolume *pvl);
GAGE_EXPORT const double *gageAnswerPointer(const gageContext *ctx,
                                            const gagePerVolume *pvl,
                                            int item);
GAGE_EXPORT unsigned int gageAnswerLength(const gageContext *ctx,
                                          const gagePerVolume *pvl,
                                          int item);
GAGE_EXPORT int gageQueryReset(gageContext *ctx, gagePerVolume *pvl);
GAGE_EXPORT int gageQuerySet(gageContext *ctx, gagePerVolume *pvl,
                             gageQuery query);
GAGE_EXPORT int gageQueryAdd(gageContext *ctx, gagePerVolume *pvl,
                             gageQuery query);
GAGE_EXPORT int gageQueryItemOn(gageContext *ctx, gagePerVolume *pvl,
                                int item);

/* optimsig.c */
GAGE_EXPORT int gageOptimSigSet(double *scale, unsigned int num,
                                unsigned int sigmaMax);
GAGE_EXPORT gageOptimSigParm *gageOptimSigParmNew(unsigned int sampleMaxNum);
GAGE_EXPORT gageOptimSigParm *gageOptimSigParmNix(gageOptimSigParm *parm);
GAGE_EXPORT int gageOptimSigTruthSet(gageOptimSigParm *parm,
                                     unsigned int dim,
                                     double sigmaMax, double cutoff,
                                     unsigned int measrSampleNum);
GAGE_EXPORT int gageOptimSigCalculate(gageOptimSigParm *parm,
                                      double *scalePos, unsigned int num,
                                      int volMeasr, int lineMeasr,
                                      double convEps, unsigned int maxIter);
GAGE_EXPORT int gageOptimSigPlot(gageOptimSigParm *parm, Nrrd *nout,
                                 const double *plotpos,
                                 unsigned int plotPosNum,
                                 int volMeasr, int tentRecon);

/* stack.c */
GAGE_EXPORT double gageTauOfTee(double tee);
GAGE_EXPORT double gageTeeOfTau(double tau);
GAGE_EXPORT double gageSigOfTau(double tau);
GAGE_EXPORT double gageTauOfSig(double sig);
GAGE_EXPORT double gageStackWtoI(gageContext *ctx, double swrl,
                                 int *outside);
GAGE_EXPORT double gageStackItoW(gageContext *ctx, double si,
                                 int *outside);
GAGE_EXPORT int gageStackPerVolumeNew(gageContext *ctx,
                                      gagePerVolume **pvlStack,
                                      const Nrrd *const *nblur,
                                      unsigned int blnum,
                                      const gageKind *kind);
GAGE_EXPORT int gageStackPerVolumeAttach(gageContext *ctx,
                                         gagePerVolume *pvlBase,
                                         gagePerVolume **pvlStack,
                                         const double *stackPos,
                                         unsigned int blnum);
GAGE_EXPORT int gageStackProbe(gageContext *ctx,
                               double xi, double yi, double zi, double si);
GAGE_EXPORT int gageStackProbeSpace(gageContext *ctx,
                                    double x, double y, double z, double s,
                                    int indexSpace, int clamp);

/* stackBlur.c */
GAGE_EXPORT gageStackBlurParm *gageStackBlurParmNew(void);
GAGE_EXPORT gageStackBlurParm *gageStackBlurParmNix(gageStackBlurParm *sbp);
GAGE_EXPORT int gageStackBlurParmScaleSet(gageStackBlurParm *sbp,
                                          unsigned int num,
                                          double scaleMin, double scaleMax,
                                          int uniform, int optim);
GAGE_EXPORT int gageStackBlurParmKernelSet(gageStackBlurParm *sbp,
                                           const NrrdKernelSpec *kspec,
                                           int renormalize);
GAGE_EXPORT int gageStackBlurParmBoundarySet(gageStackBlurParm *sbp,
                                             int boundary, double padValue);
GAGE_EXPORT int gageStackBlurParmVerboseSet(gageStackBlurParm *sbp,
                                            int verbose);
GAGE_EXPORT int gageStackBlurParmCheck(gageStackBlurParm *sbp);
GAGE_EXPORT int gageStackBlur(Nrrd *const nblur[], gageStackBlurParm *sbp,
                              const Nrrd *nin, const gageKind *kind);
GAGE_EXPORT int gageStackBlurCheck(const Nrrd *const nblur[],
                                   gageStackBlurParm *sbp,
                                   const Nrrd *nin, const gageKind *kind);
GAGE_EXPORT int gageStackBlurGet(Nrrd *const nblur[], int *recomputedP,
                                 gageStackBlurParm *sbp,
                                 const char *format,
                                 const Nrrd *nin, const gageKind *kind);
GAGE_EXPORT int gageStackBlurManage(Nrrd ***nblurP, int *recomputedP,
                                    gageStackBlurParm *sbp,
                                    const char *format,
                                    int saveIfComputed, NrrdEncoding *enc,
                                    const Nrrd *nin, const gageKind *kind);

/* ctx.c */
GAGE_EXPORT gageContext *gageContextNew(void);
GAGE_EXPORT gageContext *gageContextCopy(gageContext *ctx);
GAGE_EXPORT gageContext *gageContextNix(gageContext *ctx);
GAGE_EXPORT void gageParmSet(gageContext *ctx, int which, double val);
GAGE_EXPORT int gagePerVolumeIsAttached(const gageContext *ctx,
                                        const gagePerVolume *pvl);
GAGE_EXPORT int gagePerVolumeAttach(gageContext *ctx, gagePerVolume *pvl);
GAGE_EXPORT int gagePerVolumeDetach(gageContext *ctx, gagePerVolume *pvl);
GAGE_EXPORT int gageKernelSet(gageContext *ctx, int which,
                              const NrrdKernel *k, const double *kparm);
GAGE_EXPORT void gageKernelReset(gageContext *ctx);
GAGE_EXPORT int gageProbe(gageContext *ctx, double xi, double yi, double zi);
GAGE_EXPORT int gageProbeSpace(gageContext *ctx, double x, double y, double z,
                               int indexSpace, int clamp);

/* update.c */
GAGE_EXPORT int gageUpdate(gageContext *ctx);

/* st.c */
GAGE_EXPORT int gageStructureTensor(Nrrd *nout, const Nrrd *nin,
                                    int dScale, int iScale, int dsmp);

/* deconvolve.c */
GAGE_EXPORT int gageDeconvolve(Nrrd *nout, double *lastDiffP,
                               const Nrrd *nin, const gageKind *kind,
                               const NrrdKernelSpec *ksp, int typeOut,
                               unsigned int maxIter, int saveAnyway,
                               double step, double epsilon, int verbose);
GAGE_EXPORT int gageDeconvolveSeparableKnown(const NrrdKernelSpec *ksp);
GAGE_EXPORT int gageDeconvolveSeparable(Nrrd *nout, const Nrrd *nin,
                                        const gageKind *kind,
                                        const NrrdKernelSpec *ksp,
                                        int typeOut);

#ifdef __cplusplus
}
#endif

#endif /* GAGE_HAS_BEEN_INCLUDED */
