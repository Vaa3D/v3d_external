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

#ifndef TEN_HAS_BEEN_INCLUDED
#define TEN_HAS_BEEN_INCLUDED

#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/unrrdu.h>
#include <teem/dye.h>
#include <teem/gage.h>
#include <teem/limn.h>
#include <teem/echo.h>

#include "tenMacros.h"

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(ten_EXPORTS) || defined(teem_EXPORTS)
#    define TEN_EXPORT extern __declspec(dllexport)
#  else
#    define TEN_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define TEN_EXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TEN tenBiffKey

/*
****** tenAniso* enum
**
** the different scalar values that can describe a tensor.  Nearly
** all of these are anisotropy metrics, but with time this has become
** a convenient way to present any scalar invariant (such as trace),
** and even the individual eigenvalues
**
** keep in sync:
** aniso.c: _tenAnisoEval_X_f(), _tenAnisoEval_f[]
** aniso.c: _tenAnisoTen_X_d(), _tenAnisoTen_d[]
** aniso.c: tenAnisoCalc_f()
** enumsTen.c: tenAniso
*/
enum {
  tenAnisoUnknown,  /*  0: nobody knows */
  tenAniso_Conf,    /*  1: not an anisotropy, but enables some useful hacks */
  tenAniso_Cl1,     /*  2: Westin's linear (first version) */
  tenAniso_Cp1,     /*  3: Westin's planar (first version) */
  tenAniso_Ca1,     /*  4: Westin's linear + planar (first version) */
  tenAniso_Clpmin1, /*  5: minimum of Cl and Cp (first version) */
  tenAniso_Cs1,     /*  6: Westin's spherical (first version) */
  tenAniso_Ct1,     /*  7: gk's anisotropy type (first version) */
  tenAniso_Cl2,     /*  8: Westin's linear (second version) */
  tenAniso_Cp2,     /*  9: Westin's planar (second version) */
  tenAniso_Ca2,     /* 10: Westin's linear + planar (second version) */
  tenAniso_Clpmin2, /* 11: minimum of Cl and Cp (second version) */
  tenAniso_Cs2,     /* 12: Westin's spherical (second version) */
  tenAniso_Ct2,     /* 13: gk's anisotropy type (second version) */
  tenAniso_RA,      /* 14: Bass+Pier's relative anisotropy */
  tenAniso_FA,      /* 15: (Bass+Pier's fractional anisotropy)/sqrt(2) */
  tenAniso_VF,      /* 16: volume fraction = 1-(Bass+Pier's volume ratio) */
  tenAniso_B,       /* 17: linear term in cubic characteristic polynomial */
  tenAniso_Q,       /* 18: radius of root circle is 2*sqrt(Q) */
  tenAniso_R,       /* 19: half of third moment of eigenvalues */
  tenAniso_S,       /* 20: frobenius norm, squared */
  tenAniso_Skew,    /* 21: R/sqrt(2*Q^3) */
  tenAniso_Mode,    /* 22: 3*sqrt(6)*det(dev)/norm(dev) = sqrt(2)*skew */
  tenAniso_Th,      /* 23: acos(sqrt(2)*skew)/3 */
  tenAniso_Omega,   /* 24: FA*(1+mode)/2 */
  tenAniso_Det,     /* 25: plain old determinant */
  tenAniso_Tr,      /* 26: plain old trace */
  tenAniso_eval0,   /* 27: largest eigenvalue */
  tenAniso_eval1,   /* 28: middle eigenvalue */
  tenAniso_eval2,   /* 29: smallest eigenvalue */
  tenAnisoLast
};
#define TEN_ANISO_MAX  29

/*
******** tenInterpType* enum
**
** different kinds of interpolations paths between tensors
*/
enum {
  tenInterpTypeUnknown,         /*  0: nobody knows */
  tenInterpTypeLinear,          /*  1: simple per-coefficient linear */
  tenInterpTypeLogLinear,       /*  2: linear on logs (Log-Euclidean) */
  tenInterpTypeAffineInvariant, /*  3: Riemannian approach of many authors */
  tenInterpTypeWang,            /*  4: affine-invariant of Z Wang & B Vemuri */
  tenInterpTypeGeoLoxK,         /*  5: geodesic-loxodrome on K_i invariants */
  tenInterpTypeGeoLoxR,         /*  6: geodesic-loxodrome on R_i invariants */
  tenInterpTypeLoxK,            /*  7: total loxodrome on K_i invariants */
  tenInterpTypeLoxR,            /*  8: total loxodrome on R_i invariants */
  tenInterpTypeQuatGeoLoxK,     /*  9: geodesic-loxodrome on K_i invariants */
  tenInterpTypeQuatGeoLoxR,     /* 10: geodesic-loxodrome on R_i invariants */
  tenInterpTypeRThetaPhiLinear, /* 11: linear interpolation of (R,theta,phi),
                                   oriented by eigensystem of linear avg */
  tenInterpTypeLast
};
#define TEN_INTERP_TYPE_MAX        11

/*
******** tenGlyphType* enum
**
** the different types of glyphs that may be used for tensor viz
*/
enum {
  tenGlyphTypeUnknown,    /* 0: nobody knows */
  tenGlyphTypeBox,        /* 1 */
  tenGlyphTypeSphere,     /* 2 */
  tenGlyphTypeCylinder,   /* 3 */
  tenGlyphTypeSuperquad,  /* 4 */
  tenGlyphTypeBetterquad, /* 5: for T Schultz, GL Kindlmann.
                             Superquadric Glyphs for Symmetric
                             Second-Order Tensors. IEEE TVCG
                             Nov/Dec 2010, 16(6):1595-1604 */
  tenGlyphTypePolarPlot,  /* 6 */
  tenGlyphTypeLast
};
#define TEN_GLYPH_TYPE_MAX   6

/*
******** tenGlyphParm struct
**
** all input parameters to tenGlyphGen
*/
typedef struct {
  int verbose;

  /* glyphs will be shown at samples that have confidence >= confThresh,
     and anisotropy anisoType >= anisoThresh, and if nmask is non-NULL,
     then the corresponding mask value must be >= maskThresh.  If
     onlyPositive, then samples with a non-positive eigenvalue will
     be skipped, regardless of their purported anisotropy */
  Nrrd *nmask;
  int anisoType, onlyPositive;
  float confThresh, anisoThresh, maskThresh;

  /* glyphs have shape glyphType and size glyphScale. Superquadrics
     are tuned by sqdSharp, and things that must polygonalize do so
     according to facetRes.  Postscript rendering of glyph edges is
     governed by edgeWidth[] */
  int glyphType, facetRes;
  float glyphScale, sqdSharp;
  float edgeWidth[5];  /* 0: contour, 1: front crease, 2: front non-crease */

  /* glyphs are colored by eigenvector colEvec with the standard XYZ-RGB
     colormapping, with maximal saturation colMaxSat (use 0.0 to turn off
     coloring).  Saturation is modulated by anisotropy colAnisoType, to a
     degree controlled by colAnisoModulate (if 0, saturation is not at all
     modulated by anistropy).  Post-saturation, there is a per-channel
     gamma of colGamma. */
  int colEvec, colAnisoType;
  float colMaxSat, colIsoGray, colGamma, colAnisoModulate, ADSP[4];

  /* if doSlice, a slice of anisotropy sliceAnisoType will be depicted
     in grayscale as a sheet of grayscale squares, one per sample. As
     with glyphs, these are thresholded by confThresh and maskThresh
     (but not anisoThresh).  Things can be lightened up with a
     sliceGamma > 1, after the slice's gray values have been mapped from
     [0,1] to [sliceBias,1]. The squares will be at their corresponding
     sample locations, but offset by sliceOffset */
  unsigned int sliceAxis;
  size_t slicePos;
  int doSlice, sliceAnisoType;
  float sliceOffset, sliceBias, sliceGamma;
} tenGlyphParm;

#define TEN_ANISO_DESC \
  "All the Westin metrics come in two versions.  Currently supported:\n " \
  "\b\bo \"cl1\", \"cl2\": Westin's linear\n " \
  "\b\bo \"cp1\", \"cp2\": Westin's planar\n " \
  "\b\bo \"ca1\", \"ca2\": Westin's linear + planar\n " \
  "\b\bo \"cs1\", \"cs2\": Westin's spherical (1-ca)\n " \
  "\b\bo \"ct1\", \"ct2\": GK's anisotropy type (cp/ca)\n " \
  "\b\bo \"ra\": Basser/Pierpaoli relative anisotropy/sqrt(2)\n " \
  "\b\bo \"fa\": Basser/Pierpaoli fractional anisotropy\n " \
  "\b\bo \"vf\": volume fraction = 1-(Basser/Pierpaoli volume ratio)\n " \
  "\b\bo \"tr\": trace"

/*
******** tenGage* enum
**
** all the possible queries supported in the tenGage gage kind
** various properties of the quantities below (eigenvalues = v1, v2, v3):
** eigenvalue cubic equation: v^3 + A*v^2 + B*v + C = 0
** Trace = v1 + v2 + v3 = -A
** B = v1*v2 + v1*v3 + v2*v3
** Det = v1*v2*v3 = -C
** S = v1*v1 + v2*v2 + v3*v3
** Q = (S-B)/9 = variance({v1,v2,v3})/2 = (RootRadius/2)^2
** FA = 3*sqrt(Q/S)
** R = (9*A*B - 2*A^3 - 27*C)/54
     = (5*A*B - 2*A*S - 27*C)/54 = thirdmoment({v1,v2,v3})/2
** P = arccos(R/sqrt(Q)^3)/3 = phase angle of cubic solution
**
** NOTE: currently tenGage knows *nothing* about nrrd->measurementFrame!
** You probably want to call tenMeasurementFrameReduce() first.
**
** Hey- the problem with adding the RGB eigenvector coloring to the
** capability of tenGage is that because this is visualization, you
** can't easily control whether the measurement frame is applied, if
** known- in that sense the RGB info is uniquely different from the
** other vector and tensor items that can be queried . . . so after a
** brief appearance here the RGB evec coloring was removed.  The
** gagePerVolume->data field that it motivated has rightly remained.
**
** !!! Changes to this list need to be propogated to:
** !!! tenGage.c: _tenGageTable[], _tenGageAnswer(),
** !!! enumsTen.c: tenGage airEnum.
**
*/
enum {
  tenGageUnknown,          /*   0: nobody knows */

  tenGageTensor,           /*   1: "t", the reconstructed tensor: [7] */
  tenGageConfidence,       /*   2: "c", first of seven tensor values: [1] */
  tenGageTrace,            /*   3: "tr", trace of tensor: [1] */
  tenGageNorm,             /*   4: "n", frobenius norm  of tensor: [1] */
  tenGageB,                /*   5: "b": [1] */
  tenGageDet,              /*   6: "det", determinant of tensor: [1] */
  tenGageS,                /*   7: "s", square of frobenius norm: [1] */
  tenGageQ,                /*   8: "q", (S - B)/9: [1] */
  tenGageFA,               /*   9: "fa", fractional anisotropy: [1] */
  tenGageR,                /*  10: "r", 9*A*B - 2*A^3 - 27*C: [1] */
  tenGageMode,             /*  11: "mode", sqrt(2)*R/sqrt(Q^3): [1] */
  tenGageTheta,            /*  12: "th", arccos(-mode)/AIR_PI: [1] */
  tenGageModeWarp,         /*  13: "modew", mode warped for better contrast:
                                   cos((1-m)*pi/2): [1] */
  tenGageOmega,            /*  14: "om", fa*(mode+1)/2: [1] */

  tenGageEval,             /*  15: "eval", all eigenvals of tensor : [3] */
  tenGageEval0,            /*  16: "eval0", major eigenval of tensor : [1] */
  tenGageEval1,            /*  17: "eval1", medium eigenval of tensor : [1] */
  tenGageEval2,            /*  18: "eval2", minor eigenval of tensor : [1] */
  tenGageEvec,             /*  19: "evec", major eigenvects of tensor: [9] */
  tenGageEvec0,            /*  20: "evec0", major eigenvect of tensor: [3] */
  tenGageEvec1,            /*  21: "evec1", medium eigenvect of tensor: [3] */
  tenGageEvec2,            /*  22: "evec2", minor eigenvect of tensor: [3] */

  tenGageDelNormK2,        /*  23: "delnk2": normalized gradient tensor of
                                   K2 = eval variance: [7] */
  tenGageDelNormK3,        /*  24: "delnk3": normal gradient tensor of
                                   K3 = R3 = eval skewness: [7] */
  tenGageDelNormR1,        /*  25: "delnr1": normalized gradient tensor of
                                   R1 = tensor norm: [7] */
  tenGageDelNormR2,        /*  26: "delnr2": normalized gradient tensor of
                                   R2 = FA: [7] */
  tenGageDelNormPhi1,      /*  27: "delnphi1": normalized rotation tangent
                                    around principal evector: [7] */
  tenGageDelNormPhi2,      /*  28: "delnphi2": normalized rotation tangent
                                    rotation around medium evector: [7] */
  tenGageDelNormPhi3,      /*  29: "delnphi3": normalized rotation tangent
                                    rotation around minor evector: [7] */

  tenGageTensorGrad,       /*  30: "tg", all tensor component gradients,
                                   starting with confidence gradient: [21] */
  tenGageTensorGradMag,    /*  31: "tgm", actually a 3-vector of tensor
                                   gradient norms, one for each axis: [3] */
  tenGageTensorGradMagMag, /*  32: "tgmm", single scalar magnitude: [1] */

  tenGageTraceGradVec,     /*  33: "trgv": gradient (vector) of trace: [3] */
  tenGageTraceGradMag,     /*  34: "trgm": gradient magnitude of trace: [1] */
  tenGageTraceNormal,      /*  35: "trn": normal of trace: [3] */

  tenGageNormGradVec,      /*  36: "ngv", gradient (vector) of norm: [3] */
  tenGageNormGradMag,      /*  37: "ngm", gradient magnitude of norm: [1] */
  tenGageNormNormal,       /*  38: "nn", normal of norm: [3] */

  tenGageBGradVec,         /*  39: "bgv", gradient (vector) of B: [3] */
  tenGageBGradMag,         /*  40: "bgm", gradient magnitude of B: [1] */
  tenGageBNormal,          /*  41: "bn", normal of B: [3] */

  tenGageDetGradVec,       /*  42: "detgv", gradient (vector) of Det: [3] */
  tenGageDetGradMag,       /*  43: "detgm", gradient magnitude of Det: [1] */
  tenGageDetNormal,        /*  44: "detn", normal of Det: [3] */

  tenGageSGradVec,         /*  45: "sgv", gradient (vector) of S: [3] */
  tenGageSGradMag,         /*  46: "sgm", gradient magnitude of S: [1] */
  tenGageSNormal,          /*  47: "sn", normal of S: [3] */

  tenGageQGradVec,         /*  48: "qgv", gradient vector of Q: [3] */
  tenGageQGradMag,         /*  49: "qgm", gradient magnitude of Q: [1] */
  tenGageQNormal,          /*  50: "qn", normalized gradient of Q: [3] */

  tenGageFAGradVec,        /*  51: "fagv", gradient vector of FA: [3] */
  tenGageFAGradMag,        /*  52: "fagm", gradient magnitude of FA: [1] */
  tenGageFANormal,         /*  53: "fan", normalized gradient of FA: [3] */

  tenGageRGradVec,         /*  54: "rgv", gradient vector of Q: [3] */
  tenGageRGradMag,         /*  55: "rgm", gradient magnitude of Q: [1] */
  tenGageRNormal,          /*  56: "rn", normalized gradient of Q: [3] */

  tenGageModeGradVec,      /*  57: "mgv", gradient vector of Mode: [3] */
  tenGageModeGradMag,      /*  58: "mgm", gradient magnitude of Mode: [1] */
  tenGageModeNormal,       /*  59: "mn", normalized gradient of Mode: [3] */

  tenGageThetaGradVec,     /*  60: "thgv", gradient vector of Th: [3] */
  tenGageThetaGradMag,     /*  61: "thgm", gradient magnitude of Th: [1] */
  tenGageThetaNormal,      /*  62: "thn", normalized gradient of Th: [3] */

  tenGageOmegaGradVec,     /*  63: "omgv", gradient vector of Omega: [3] */
  tenGageOmegaGradMag,     /*  64: "omgm", gradient magnitude of Omega: [1] */
  tenGageOmegaNormal,      /*  65: "omn", normalized gradient of Omega: [3] */

  tenGageInvarKGrads,      /*  66: "ikgs", projections of tensor gradient onto
                                  the normalized shape gradients: eval mean,
                                  variance, skew, in that order: [9]  */
  tenGageInvarKGradMags,   /*  67: "ikgms", vector magnitude of the spatial
                                  invariant gradients (above): [3] */
  tenGageInvarRGrads,      /*  68: "irgs", projections of tensor gradient onto
                                  the normalized shape gradients: eval mean,
                                  variance, skew, in that order: [9]  */
  tenGageInvarRGradMags,   /*  69: "irgms", vector magnitude of the spatial
                                  invariant gradients (above): [3] */
  tenGageRotTans,          /*  70: "rts", projections of the tensor grad onto
                                   *normalized* rotation tangents: [9] */
  tenGageRotTanMags,       /*  71: "rtms", mags of vectors above: [3] */
  tenGageEvalGrads,        /*  72: "evgs", projections of tensor gradient onto
                                   gradients of eigenvalues: [9] */
  tenGageCl1,              /*  73: same as tenAniso_Cl1, but faster */
  tenGageCp1,              /*  74: same as tenAniso_Cp1, but faster */
  tenGageCa1,              /*  75: same as tenAniso_Ca1, but faster */
  tenGageClpmin1,          /*  76: min(cl1,cp1) */
  tenGageCl2,              /*  77: same as tenAniso_Cl2, but faster */
  tenGageCp2,              /*  78: same as tenAniso_Cp2, but faster */
  tenGageCa2,              /*  79: same as tenAniso_Ca2, but faster */
  tenGageClpmin2,          /*  80: min(cl2,cp2) */

  tenGageHessian,          /*  81: "hess", all hessians of tensor
                                   components: [63] */
  tenGageTraceHessian,     /*  82: "trhess", hessian(trace): [9] */
  tenGageTraceHessianEval,  /*  83 */
  tenGageTraceHessianEval0, /*  84 */
  tenGageTraceHessianEval1, /*  85 */
  tenGageTraceHessianEval2, /*  86 */
  tenGageTraceHessianEvec,  /*  87 */
  tenGageTraceHessianEvec0, /*  88 */
  tenGageTraceHessianEvec1, /*  89 */
  tenGageTraceHessianEvec2, /*  90 */
  tenGageTraceHessianFrob,  /*  91 */
  tenGageBHessian,         /*  92: "bhess": [9] */
  tenGageDetHessian,       /*  93: "dethess": [9] */
  tenGageSHessian,         /*  94: "shess": [9] */
  tenGageQHessian,         /*  95: "qhess": [9] */

  tenGageFAHessian,        /*  96: "fahess": [9] */
  tenGageFAHessianEval,    /*  97: "fahesseval": [3] */
  tenGageFAHessianEval0,   /*  98: "fahesseval0": [1] */
  tenGageFAHessianEval1,   /*  99: "fahesseval1": [1] */
  tenGageFAHessianEval2,   /* 100: "fahesseval2": [1] */
  tenGageFAHessianEvec,    /* 101: "fahessevec": [9] */
  tenGageFAHessianEvec0,   /* 102: "fahessevec0": [3] */
  tenGageFAHessianEvec1,   /* 103: "fahessevec1": [3] */
  tenGageFAHessianEvec2,   /* 104: "fahessevec2": [3] */
  tenGageFAHessianFrob,    /* 105 */
  tenGageFARidgeSurfaceStrength,  /* 106: "farsurfstrn": [1] */
  tenGageFAValleySurfaceStrength, /* 107: "favsurfstrn": [1] */
  tenGageFALaplacian,      /* 108: "falapl": [1] */
  tenGageFAHessianEvalMode,/* 109: "fahessevalmode": [1] */
  tenGageFARidgeLineAlignment,    /* 110: "farlinealn": [1] */
  tenGageFARidgeSurfaceAlignment, /* 111: "farsurfaln": [1] */
  tenGageFA2ndDD,          /* 112: "fa2d": [1] */

  tenGageFAGeomTens,       /* 113: "fagten", sym. matx w/ evals {0, K1, K2}
                                   and evecs {grad, cdir0, cdir1}: [9] */
  tenGageFAKappa1,         /* 114: "fak1", 1st princ curv: [1] */
  tenGageFAKappa2,         /* 115: "fak2", 2nd princ curv (k2 <= k1): [1] */
  tenGageFATotalCurv,      /* 116: "fatc", L2 norm(K1,K2): [1] */
  tenGageFAShapeIndex,     /* 117: "fasi", Koen.'s shape index, ("S"): [1] */
  tenGageFAMeanCurv,       /* 118: "famc", mean curvature (K1 + K2)/2: [1] */
  tenGageFAGaussCurv,      /* 119: "fagc", gaussian curvature K1*K2: [1] */
  tenGageFACurvDir1,       /* 120: "facdir1", 1st princ curv direction: [3] */
  tenGageFACurvDir2,       /* 121: "facdir2", 2nd princ curv direction: [3] */
  tenGageFAFlowlineCurv,   /* 122: "fafc", curv of normal streamline: [1] */

  tenGageRHessian,         /* 123: "rhess": [9] */

  tenGageModeHessian,      /* 124: "mhess": [9] */
  tenGageModeHessianEval,  /* 125: "mhesseval": [3] */
  tenGageModeHessianEval0, /* 126: "mhesseval0": [1] */
  tenGageModeHessianEval1, /* 127: "mhesseval1": [1] */
  tenGageModeHessianEval2, /* 128: "mhesseval2": [1] */
  tenGageModeHessianEvec,  /* 129: "mhessevec": [9] */
  tenGageModeHessianEvec0, /* 130: "mhessevec0": [3] */
  tenGageModeHessianEvec1, /* 131: "mhessevec1": [3] */
  tenGageModeHessianEvec2, /* 132: "mhessevec2": [3] */
  tenGageModeHessianFrob,  /* 133 */

  tenGageOmegaHessian,     /* 134: "omhess": [9] */
  tenGageOmegaHessianEval, /* 135: "omhesseval": [3] */
  tenGageOmegaHessianEval0,/* 136: "omhesseval0": [1] */
  tenGageOmegaHessianEval1,/* 137: "omhesseval1": [1] */
  tenGageOmegaHessianEval2,/* 138: "omhesseval2": [1] */
  tenGageOmegaHessianEvec, /* 139: "omhessevec": [9] */
  tenGageOmegaHessianEvec0,/* 140: "omhessevec0": [3] */
  tenGageOmegaHessianEvec1,/* 141: "omhessevec1": [3] */
  tenGageOmegaHessianEvec2,/* 142: "omhessevec2": [3] */
  tenGageOmegaLaplacian,   /* 143: "omlapl": [1] */
  tenGageOmega2ndDD,       /* 144: "om2d": [1] */
  tenGageOmegaHessianContrTenEvec0, /* 145: "omhesscte0": [1] */
  tenGageOmegaHessianContrTenEvec1, /* 146: "omhesscte1": [1] */
  tenGageOmegaHessianContrTenEvec2, /* 147: "omhesscte2": [1] */

  tenGageTraceGradVecDotEvec0,   /* 148: "trgvdotevec0": [1] */
  tenGageTraceDiffusionAlign,    /* 149: "datr": [1] */
  tenGageTraceDiffusionFraction, /* 150: "dftr": [1] */
  tenGageFAGradVecDotEvec0,      /* 151: "fagvdotevec0": [1] */
  tenGageFADiffusionAlign,       /* 152: "dafa": [1] */
  tenGageFADiffusionFraction,    /* 153: "dffa": [1] */
  tenGageOmegaGradVecDotEvec0,   /* 154: "omgvdotevec0": [1] */
  tenGageOmegaDiffusionAlign,    /* 155: "daom": [1] */
  tenGageOmegaDiffusionFraction, /* 156: "dfom": [1] */
  tenGageConfGradVecDotEvec0,    /* 157: "confgvdotevec0": [1] */
  tenGageConfDiffusionAlign,     /* 158: "daconf": [1] */
  tenGageConfDiffusionFraction,  /* 159: "dfconf": [1] */

  tenGageCovariance, /* 160: "cov" 4rth order covariance tensor: [21]
                        in order of appearance:
                        0:xxxx  1:xxxy  2:xxxz  3:xxyy  4:xxyz  5:xxzz
                                6:xyxy  7:xyxz  8:xyyy  9:xyyz 10:xyzz
                                       11:xzxz 12:xzyy 13:xzyz 14:xzzz
                                               15:yyyy 16:yyyz 17:yyzz
                                                       18:yzyz 19:yzzz
                                                               20:zzzz */
  tenGageCovarianceRGRT, /* 161: "covr" covariance tensor expressed in frame
                            of R invariant gradients and rotation tangents */
  tenGageCovarianceKGRT, /* 162: "covk" covariance tensor expressed in frame
                            of K invariant gradients and rotation tangents */
  tenGageTensorLogEuclidean,     /* 163: "logeuc" log-euclidean interp */
  tenGageTensorQuatGeoLoxK,      /* 164: "qglk" QGL-K interpolation */
  tenGageTensorQuatGeoLoxR,      /* 165: "qglr" QGL-R interpolation */
  tenGageTensorRThetaPhiLinear,  /* 166: "rtpl" RThetaPhiLinear interp */

  tenGageCl1GradVec,       /* 167: "cl1gv" gradient vector of cl1: [3] */
  tenGageCl1GradMag,       /* 168: "cl1gm" gradient magnitude of cl1: [1] */
  tenGageCl1Normal,        /* 169: "cl1gn" normal of cl1: [3] */
  tenGageCp1GradVec,       /* 170: "cp1gv" gradient vector of cp1: [3] */
  tenGageCp1GradMag,       /* 171: "cp1gm" gradient magnitude of cp1: [1] */
  tenGageCp1Normal,        /* 172: "cl1gn" normal of cp1: [3] */
  tenGageCa1GradVec,       /* 173: "ca1gv" gradient vector of ca1: [3] */
  tenGageCa1GradMag,       /* 174: "ca1gm" gradient magnitude of ca1: [1] */
  tenGageCa1Normal,        /* 175: "cl1gn" normal of ca1: [3] */
  tenGageTensorGradRotE,   /* 176: "tgrote" all tensor component gradients,
                              starting with confidence gradient.
                              Rotated such that eigenvalue
                              derivatives are on the diagonal: [21] */
  tenGageEvalHessian,    /* 177: "evalhess" Hessian of the eigenvalues: [27] */
  tenGageCl1Hessian,     /* 178: "cl1hess" Hessian of cl1: [9] */
  tenGageCl1HessianEval, /* 179: "cl1hesseval" Hessian evals of cl1: [3] */
  tenGageCl1HessianEval0,/* 180: "cl1hesseval0" First Hess eval of cl1: [1] */
  tenGageCl1HessianEval1,/* 181: "cl1hesseval1" Second Hess eval of cl1: [1] */
  tenGageCl1HessianEval2,/* 182: "cl1hesseval2" Third Hess eval of cl1: [1] */
  tenGageCl1HessianEvec, /* 183: "cl1hessevec" Hessian evecs of cl1: [9] */
  tenGageCl1HessianEvec0,/* 184: "cl1hessevec0" First Hess evec of cl1: [3] */
  tenGageCl1HessianEvec1,/* 185: "cl1hessevec1" Second Hess evec of cl1: [3] */
  tenGageCl1HessianEvec2,/* 186: "cl1hessevec2" Third Hess evec of cl1: [3] */
  tenGageCp1Hessian,     /* 187: "cp1hess" Hessian of cp1: [9] */
  tenGageCp1HessianEval, /* 188: "cp1hesseval" Hessian evals of cp1: [3] */
  tenGageCp1HessianEval0,/* 189: "cp1hesseval0" First Hess eval of cp1: [1] */
  tenGageCp1HessianEval1,/* 190: "cp1hesseval1" Second Hess eval of cp1: [1] */
  tenGageCp1HessianEval2,/* 191: "cp1hesseval2" Third Hess eval of cp1: [1] */
  tenGageCp1HessianEvec, /* 192: "cp1hessevec" Hessian evecs of cp1: [9] */
  tenGageCp1HessianEvec0,/* 193: "cp1hessevec0" First Hess evec of cp1: [3] */
  tenGageCp1HessianEvec1,/* 194: "cp1hessevec1" Second Hess evec of cp1: [3] */
  tenGageCp1HessianEvec2,/* 195: "cp1hessevec2" Third Hess evec of cp1: [3] */
  tenGageCa1Hessian,     /* 196: "ca1hess" Hessian of ca1: [9] */
  tenGageCa1HessianEval, /* 197: "ca1hesseval" Hessian evals of ca1: [3] */
  tenGageCa1HessianEval0,/* 198: "ca1hesseval0" First Hess eval of ca1: [1] */
  tenGageCa1HessianEval1,/* 199: "ca1hesseval1" Second Hess eval of ca1: [1] */
  tenGageCa1HessianEval2,/* 200: "ca1hesseval2" Third Hess eval of ca1: [1] */
  tenGageCa1HessianEvec, /* 201: "ca1hessevec" Hessian evecs of ca1: [9] */
  tenGageCa1HessianEvec0,/* 202: "ca1hessevec0" First Hess evec of ca1: [3] */
  tenGageCa1HessianEvec1,/* 203: "ca1hessevec1" Second Hess evec of ca1: [3] */
  tenGageCa1HessianEvec2,/* 204: "ca1hessevec2" Third Hess evec of ca1: [3] */
  tenGageFiberCurving,   /* 205: "fibcurv" Savadjiev et al. fiber curving */
  tenGageFiberDispersion,/* 206: "fibdisp" Savadjiev et al. fiber dispersion */
  tenGageAniso,          /* 207: "an", all anisos: [TEN_ANISO_MAX+1] */
  tenGageLast
};
#define TEN_GAGE_ITEM_MAX     207

/*
******** tenDwiGage* enum
**
** all things that can be measured in the diffusion weighted images that
** underly diffusion tensor imaging
*/
enum {
  tenDwiGageUnknown,        /* 0: nobody knows */

  /*  1: "all", all the measured values, both baseline and diffusion
      weighted: [N], where N is the number of DWIs */
  tenDwiGageAll,

  /*  2: "b0", the non-Dwi image value, either by direct measurement
      or by estimation: [1]
      HEY: currently a hack, because it assumes a single known B0 */
  tenDwiGageB0,

  /*  3: "jdwi", just the DWIs, no B0: [N-1] (HEY same hack) */
  tenDwiGageJustDWI,

  /*  4: "adc", ADCs from the DWIs: [N-1] (HEY same hack) */
  tenDwiGageADC,

  /*  5: "mdwi", the average Dwi image value, which is thresholded to
      create the confidence mask: [1] */
  tenDwiGageMeanDWIValue,

  /*  6: "tlls": [7],
      7: "tllserr": [1],
      8: "tllserrlog": [1],
      9: "tllslike": [1],
     linear least squares fit of tensor value to log(Dwi)s */
  tenDwiGageTensorLLS,
  tenDwiGageTensorLLSError,      /* RMS error w/ Dwis */
  tenDwiGageTensorLLSErrorLog,   /* RMS error w/ log(Dwi)s */
  tenDwiGageTensorLLSLikelihood,

  /* 10: "twls": [7],
     11: "twlserr": [1],
     12: "twlserrlog": [1],
     13: "twlslike": [1],
     weighted least squares fit of tensor value to log(Dwi)s */
  tenDwiGageTensorWLS,
  tenDwiGageTensorWLSError,
  tenDwiGageTensorWLSErrorLog,
  tenDwiGageTensorWLSLikelihood,

  /* 14: "tnls": [7],
     15: "tnlserr": [1],
     16: "tnlserrlog": [1],
     17: "tnlslike": [1],
     non-linear least squares fit of tensor value to Dwis (not log) */
  tenDwiGageTensorNLS,
  tenDwiGageTensorNLSError,
  tenDwiGageTensorNLSErrorLog,
  tenDwiGageTensorNLSLikelihood,

  /* 18: "tmle": [7],
     19: "tmleerr": [1],
     20: "tmleerrlog": [1],
     21: "tmlelike": [1],
     maximum-likelihood fit of tensor value to Dwis */
  tenDwiGageTensorMLE,
  tenDwiGageTensorMLEError,
  tenDwiGageTensorMLEErrorLog,
  tenDwiGageTensorMLELikelihood,

  /* 22: "t": [7],
     23: "terr": [1],
     24: "terrlog": [1],
     25: "tlike": [1],
     one of the above tensors and its errors, depending on settings */
  tenDwiGageTensor,
  tenDwiGageTensorError,
  tenDwiGageTensorErrorLog,
  tenDwiGageTensorLikelihood,

  /* 26: "c", first of seven tensor values: [1] */
  tenDwiGageConfidence,

  /* 27: "fa", FA computed from the single tensor: [1] */
  tenDwiGageFA,

  /* 28: "adwie", all errors between measured and predicted DWIs
     [N-1] (HEY same hack) */
  tenDwiGageTensorAllDWIError,

  /* 29: "2qserr": [1]
     30: "2qs", two tensor fitting by q-ball segmentation: [14]
     31: "2qsnerr": [15] */
  tenDwiGage2TensorQSeg,
  tenDwiGage2TensorQSegError,
  tenDwiGage2TensorQSegAndError,

  /* 32: "2pelederr": [1]
     33: "2peled", two tensor fitting by q-ball segmentation: [14]
     34: "2pelednerr": [15] */
  tenDwiGage2TensorPeled,
  tenDwiGage2TensorPeledError,
  tenDwiGage2TensorPeledAndError,

  /* 35: "2peledlminfo", levmar output info vector: [9]
     note: length 9 being correct is checked in _tenDwiGagePvlDataNew() */
  tenDwiGage2TensorPeledLevmarInfo,

  tenDwiGageLast
};
#define TEN_DWI_GAGE_ITEM_MAX 35

/*
******** tenEstimate1Method* enum
**
** the different ways of doing single tensor estimation
*/
enum {
  tenEstimate1MethodUnknown,   /* 0 */
  tenEstimate1MethodLLS,       /* 1 */
  tenEstimate1MethodWLS,       /* 2 */
  tenEstimate1MethodNLS,       /* 3 */
  tenEstimate1MethodMLE,       /* 4 */
  tenEstimate1MethodLast
};
#define TEN_ESTIMATE_1_METHOD_MAX 4

/*
******** tenEstimate2Method* enum
**
** the different ways of doing two-tensor estimation
*/
enum {
  tenEstimate2MethodUnknown,   /* 0 */
  tenEstimate2MethodQSegLLS,   /* 1 */
  tenEstimate2MethodPeled,     /* 2 */
  tenEstimate2MethodLast
};
#define TEN_ESTIMATE_2_METHOD_MAX 2

/*
******** tenEvecRGBParm struct
**
** dumb little bag for the parameters relating to how to do the
** eigenvector -> RGB mapping, since its needed by various things in
** various contexts.  Note that you may need two of these, one for
** doing rgb(evec0) (the linear part) and one for doing rgb(evec2)
** (the planar part).  This used to have "aniso0" and "aniso2", but
** the associated methods were clumsy and redundant.
*/
typedef struct {
  unsigned int which; /* when the eigenvector hasn't already been computed,
                         which eigenvector to map:
                         0 for linear, 2 or planar, 1 for orthotropic */
  int aniso;          /* which anisotropy metric modulates saturation */
  double confThresh,  /* confidence threshold */
    anisoGamma,       /* gamma on aniso, pre-mapping */
    gamma,            /* per RGB component gamma, post-mapping */
    bgGray,           /* gray-value for low confidence samples */
    isoGray,          /* gray-value for isotropic samples */
    maxSat;           /* maximum saturation */
  int typeOut,        /* when output type is flexible, and if this is
                         nrrdTypeUChar or nrrdTypeUShort, then output will
                         be quantized to those types (range [0,255] and
                         [0,65535] respectively); otherwise values are
                         copied directly to output */
    genAlpha;         /* when output value set is flexible, create RGBA
                         values instead of just RGB */
} tenEvecRGBParm;

/*
******** tenFiberType* enum
**
** the different kinds of fiber tractography that we do
*/
enum {
  tenFiberTypeUnknown,    /* 0: nobody knows */
  tenFiberTypeEvec0,      /* 1: standard following of principal eigenvector */
  tenFiberTypeEvec1,      /* 2: following medium eigenvector */
  tenFiberTypeEvec2,      /* 3: following minor eigenvector */
  tenFiberTypeTensorLine, /* 4: Weinstein-Kindlmann tensorlines */
  tenFiberTypePureLine,   /* 5: "pure" tensorlines- multiplication only */
  tenFiberTypeZhukov,     /* 6: Zhukov's oriented tensor reconstruction */
  tenFiberTypeLast
};
#define TEN_FIBER_TYPE_MAX   6

/*
******** tenDwiFiberType* enum
**
** how tractography is done in DWI volumes.  This is orthogonal to
** how single- or two-tensor estimation is done; it describes what we
** do with the model(s) once estimated
*/
enum {
  tenDwiFiberTypeUnknown,      /* 0: nobody knows */
  tenDwiFiberType1Evec0,       /* 1: like old-fashioned tractography */
  tenDwiFiberType2Evec0,       /* 2: only using 2-tensor fits */
  tenDwiFiberType12BlendEvec0, /* 3: blend between 1- and 2-ten evec0 methods,
                                  based on something else */
  tenDwiFiberTypeLast
};
#define TEN_DWI_FIBER_TYPE_MAX    3

/*
******** tenFiberIntg* enum
**
** the different integration styles supported.  Obviously, this is more
** general purpose than fiber tracking, so this will be moved (elsewhere
** in Teem) as needed
*/
enum {
  tenFiberIntgUnknown,   /* 0: nobody knows */
  tenFiberIntgEuler,     /* 1: dumb but fast */
  tenFiberIntgMidpoint,  /* 2: 2nd order Runge-Kutta */
  tenFiberIntgRK4,       /* 3: 4rth order Runge-Kutta */
  tenFiberIntgLast
};
#define TEN_FIBER_INTG_MAX  3

/*
******** tenFiberStop* enum
**
** the different reasons why fibers stop going (as stored in
** tenFiberSingle->whyStop[]), or never got started
** (tenFiberSingle->whyNowhere), or never went far enough (also
** tenFiberSingle->whyNowhere).
**
** The addition of tenFiberStopMinLength and tenFiberStopMinNumSteps
** really stretch the meaningfulness of "tenFiberStop", but its the
** only logical place for such constraints to go.
** NOTE: tenFiberStopMinLength and tenFiberStopMinNumSteps only make
** sense as a value for whyNowhere, not whyStop, despite the name.
*/
enum {
  tenFiberStopUnknown,     /*  0: nobody knows,
                                  or, for tfx->whyNowhere: no, actually,
                                  we *did* get somewhere with this fiber */
  tenFiberStopAniso,       /*  1: specified aniso got below specified level */
  tenFiberStopLength,      /*  2: fiber length in world space got too long */
  tenFiberStopNumSteps,    /*  3: took too many steps along fiber */
  tenFiberStopConfidence,  /*  4: tensor "confidence" value went too low */
  tenFiberStopRadius,      /*  5: radius of curvature got too small */
  tenFiberStopBounds,      /*  6: fiber position stepped outside volume */
  tenFiberStopFraction,    /*  7: during multi-tensor tracking, fractional
                                  constituency of the tracked tensor got
                                  too small */
  tenFiberStopStub,        /*  8: treat single vertex fibers as non-starters */
  tenFiberStopMinLength,   /*  9: fibers with total (both halves) small length
                                  are discarded */
  tenFiberStopMinNumSteps, /* 10: fibers with total (both halves) small # steps
                                  are discarded (more general-purpose than
                                  tenFiberStopStub) */
  tenFiberStopLast
};
#define TEN_FIBER_STOP_MAX    10

/*
******** #define TEN_FIBER_NUM_STEPS_MAX
**
** whatever the stop criteria are for fiber tracing, no fiber half can
** have more points than this- a useful sanity check against fibers
** done amok.
*/
#define TEN_FIBER_NUM_STEPS_MAX 10240

enum {
  tenFiberParmUnknown,         /* 0: nobody knows */
  tenFiberParmStepSize,        /* 1: base step size */
  tenFiberParmUseIndexSpace,   /* 2: non-zero iff output of fiber should be
                                  seeded in and output in index space,
                                  instead of default world */
  tenFiberParmWPunct,          /* 3: tensor-line parameter */
  tenFiberParmVerbose,         /* 4: verbosity */
  tenFiberParmLast
};
#define TEN_FIBER_PARM_MAX        4

enum {
  tenTripleTypeUnknown,    /* 0: nobody knows */
  tenTripleTypeEigenvalue, /* 1: eigenvalues sorted in descending order */
  tenTripleTypeMoment,     /* 2: (mu1,mu2,mu3) */
  tenTripleTypeXYZ,        /* 3: eval rotation, after Bahn'99 JMR:141(68-77) */
  tenTripleTypeRThetaZ,    /* 4: cylindrical coords of rotated evals */
  tenTripleTypeRThetaPhi,  /* 5: spherical coords of rotated evals */
  tenTripleTypeJ,          /* 6: (J1,J2,J3) principal invariants */
  tenTripleTypeK,          /* 7: (K1,K2,K3) cylindrical invariants */
  tenTripleTypeR,          /* 8: (R1,R2,R3) spherical invariants */
  tenTripleTypeWheelParm,  /* 9: eigenvalue wheel (center,radius,angle) */
  tenTripleTypeLast
};
#define TEN_TRIPLE_TYPE_MAX   9

/*
******** tenFiberContext
**
** catch-all for input, state, and output of fiber tracing.  Luckily, like
** in a gageContext, NOTHING in here is set directly by the user; everything
** should be through the tenFiber* calls
*/
typedef struct {
  /* ---- input -------- */
  const Nrrd *nin;      /* the tensor OR DWI volume being analyzed */
  NrrdKernelSpec *ksp;  /* reconstruction kernel for tensors or DWIs */
  int useDwi,           /* we're working in a DWI, not a tensor, volume */
    fiberType,          /* from tenFiberType* OR tenDwiFiberType* enum */
    fiberProbeItem,     /* item to probe along fiber and possibly save in
                           tenFiberSingle->nval */
    intg,               /* from tenFiberIntg* enum */
    anisoStopType,      /* which aniso we do a threshold on */
    anisoSpeedType,     /* base step size is function of this anisotropy */
    stop,               /* BITFLAG for different reasons to stop a fiber */
    useIndexSpace,      /* output in index space, not world space */
    verbose;            /* blah blah blah */
  double anisoThresh,   /* anisotropy threshold */
    anisoSpeedFunc[3];  /* parameters of mapping aniso to speed */
  unsigned int maxNumSteps, /* max # steps allowed on one fiber *half* */
    minNumSteps;        /* min signficiant # steps on *whole* fiber */
  double stepSize,      /* step size in world space */
    maxHalfLen,         /* longest propagation (forward or backward) allowed
                           from midpoint */
    minWholeLen,        /* minimum significant length of whole fiber */
    confThresh,         /* confidence threshold */
    minRadius,          /* minimum radius of curvature of path */
    minFraction;        /* minimum fractional constituency in multi-tensor */
  double wPunct;        /* knob for tensor lines */
  unsigned int ten2Which;  /* which path to follow in 2-tensor tracking */
  /* ---- internal ----- */
  gageQuery query;      /* query we'll send to gageQuerySet */
  int halfIdx,          /* current fiber half being computed (0 or 1) */
    mframeUse;          /* need to use mframe[] and mframeT[] */
  double mframe[9],     /* measurement frame in normal matrix lay-out */
    mframeT[9],         /* transpose of mframe[] */
    wPos[3],            /* current world space location */
    wDir[3],            /* difference between this and last world space pos */
    lastDir[3],         /* previous value of wDir */
    seedEvec[3];        /* principal eigenvector first found at seed point */
  int lastDirSet,       /* lastDir[] is usefully set */
    lastTenSet;         /* lastTen[] is usefully set */
  unsigned int ten2Use; /* which of the 2-tensors was last used */
  gageContext *gtx;     /* wrapped around pvl */
  gagePerVolume *pvl;   /* wrapped around dtvol */

  const double *gageTen,    /* gageAnswerPointer(pvl, tenGageTensor) */
    *gageEval,              /* gageAnswerPointer(pvl, tenGageEval) */
    *gageEvec,              /* gageAnswerPointer(pvl, tenGageEvec) */
    *gageAnisoStop,         /* gageAnswerPointer(pvl, tenGage<anisoStop>) */
    *gageAnisoSpeed,        /* gageAnswerPointer(pvl, tenGage<anisoSpeed>) */
    *gageTen2;              /* gageAnswerPointer(pvl, tenDwiGage..2Tensor..) */
  double ten2AnisoStop;
  double fiberTen[7], fiberEval[3], fiberEvec[9],
    fiberAnisoStop, fiberAnisoSpeed;
  double radius;        /* current radius of curvature */
  /* ---- output ------- */
  double halfLen[2];    /* length of each fiber half in world space */
  unsigned int numSteps[2]; /* how many samples are used for each fiber half */
  int whyStop[2],       /* why backward/forward (0/1) tracing stopped
                           (from tenFiberStop* enum) */
    whyNowhere;         /* why fiber never got started (from tenFiberStop*) */

} tenFiberContext;

/*
******** tenFiberSingle
**
** experimental struct for holding results from a single tracing
*/
typedef struct {
  /* ------- available for recording for reference, not used by ten */
  double seedPos[3];    /* where was the seed point */
  unsigned int dirIdx;  /* which direction at seedpoint to follow */
  unsigned int dirNum;  /* how many directions at seedpnt could be followed */
  /* ------- output ------- */
  Nrrd *nvert;          /* locations of tract vertices */
  double halfLen[2];    /* (same as in tenFiberContext) */
  unsigned int seedIdx, /* which index in nvert is for seedpoint */
    stepNum[2];         /* (same as in tenFiberContext) */
  int whyStop[2],       /* (same as in tenFiberContext) */
    whyNowhere;         /* (same as in tenFiberContext) */
  Nrrd *nval;           /* results of probing at vertices */
  double measr[NRRD_MEASURE_MAX+1];  /* a controlled mess */
} tenFiberSingle;

/*
******** tenFiberMulti
**
** container for multiple fibers
*/
typedef struct {
  tenFiberSingle *fiber;
  unsigned int fiberNum;
  airArray *fiberArr;
} tenFiberMulti;

/*
******** struct tenEmBimodalParm
**
** input and output parameters for tenEMBimodal (for fitting two
** gaussians to a histogram).  Currently, all fields are directly
** set/read; no API help here.
**
** "fraction" means prior probability
**
** In the output, material #1 is the one with the lower mean
*/
typedef struct {
  /* ----- input -------- */
  double minProb,        /* threshold for negligible posterior prob. values */
    minProb2,            /* minProb for 2nd stage fitting */
    minDelta,            /* convergence test for maximization */
    minFraction,         /* smallest fraction (in 0.0 to 1.0) that material
                            1 or 2 can legitimately have */
    minConfidence,       /* smallest confidence value that the model fitting
                            is allowed to have */
    twoStage,            /* wacky two-stage fitting */
    verbose;             /* output messages and/or progress images */
  unsigned int maxIteration; /* cap on # of non-convergent iters allowed */
  /* ----- internal ----- */
  double *histo,         /* double version of histogram */
    *pp1, *pp2,          /* pre-computed posterior probabilities for the
                            current iteration */
    vmin, vmax,          /* value range represented by histogram. This is
                            saved from given histogram, and used to inform
                            final output values, but it is not used for
                            any intermediate histogram calculations, all of
                            which are done entirely in index space */
    delta;               /* some measure of model change between iters */
  int N,                 /* number of bins in histogram */
    stage;               /* current stage (1 or 2) */
  unsigned int iteration;  /* current iteration */
  /* ----- output ------- */
  double mean1, stdv1,   /* material 1 mean and  standard dev */
    mean2, stdv2,        /* same for material 2 */
    fraction1,           /* fraction of material 1 (== 1 - fraction2) */
    confidence,          /* (mean2 - mean1)/(stdv1 + stdv2) */
    threshold;           /* minimum-error threshold */
} tenEMBimodalParm;

/*
******** struct tenGradientParm
**
** all parameters for repulsion-based generation of gradient directions
**
** the old physics-based point-repulsion code (RK2 integration of
** equations of motion, with drag force) is gone; this is only the
** fast gradient descent with some strategies for adaptive step size
*/
typedef struct {
  /* ----------------------- INPUT */
  double initStep,        /* initial step size for gradient descent */
    jitter,               /* amount by which distribution is jittered
                             when starting with a given input set, as
                             a fraction of the ideal edge length */
    minVelocity,          /* minimum mean gradient velocity that signifies
                             end of first distribution phase */
    minPotentialChange,   /* minimum change in potential that signifies
                             end of first distribution phase */
    minMean,              /* mean gradient length that signifies end of
                             secondary balancing phase */
    minMeanImprovement;   /* magnitude of improvement (reduction) of mean
                             gradient length that signifies end of
                             secondary balancing phase */
  int single,             /* distribute single points, instead of
                             anti-podal pairs of points */
    insertZeroVec,        /* when computing output in
                             tenGradientDistribute (and only there, though
                             this is called by tenGradientGenerate), insert
                             the zero vector at the beginning of the list,
                             corresponding to a non-DWI B0 image */
    verbose;              /* verbosity level; 0 turns off everything */
  unsigned int snap,      /* interval of interations at which to save
                             snapshats of distribution */
    report,               /* interval of interations at which to report
                             on progress */
    expo,                 /* the exponent N that defines the potential
                             energy profile 1/r^N (coulomb: N=1) */
    seed,                 /* seed value for random number generator */
    maxEdgeShrink,        /* max number of times we try to compute
                             an update with smaller edge normalization */
    minIteration,         /* run for at least this many iterations,
                             which can be useful for high exponents,
                             for which potential measurements can
                             easily go to infinity */
    maxIteration;         /* bail if we haven't converged by this number
                             of iterations */
  double expo_d;          /* floating point exponent.  If expo is zero,
                             this is the value that matters */
  /* ----------------------- INTERNAL */
  double step,            /* actual current step size (adjusted during
                             the algorithm depending on progress) */
    nudge;                /* how to increase realDT with each iteration */
  /* ----------------------- OUTPUT */
  unsigned int itersUsed; /* total number of iterations */
  double potential,       /* potential, without edge normalization */
    potentialNorm,        /* potential, with edge normalization */
    angle,                /* minimum angle */
    edge;                 /* minimum edge length */
} tenGradientParm;

/*
******** struct tenEstimateContext
**
** for handling estimation of diffusion models
*/
typedef struct {
  /* input ----------- */
  double bValue,           /* scalar b value */
    valueMin,              /* smallest sensible for input Dwi value,
                              must be > 0.0 (for taking log) */
    sigma,                 /* noise parameter */
    dwiConfThresh,         /* mean Dwi threshold for confidence mask */
    dwiConfSoft;           /* softness in confidence mask */
                           /* NOTE: for both _ngrad and _nbmat:
                              1) only one can be non-NULL, and axis[1].size
                              is the total # values, both Dwi and non-Dwi
                              2) NO additional re-normalization is done on
                              the grads/bmats, UNLIKE the normalization
                              performed by tenDWMRIKeyValueParse(). */
  const Nrrd *_ngrad,      /* caller's 3-by-allNum gradient list */
    *_nbmat;               /* caller's 6-by-allNum B-matrix list,
                              off-diagonals are *NOT* pre-multiplied by 2 */
  unsigned int *skipList;  /* list of value indices that we are to skip */
  airArray *skipListArr;   /* airArray around skipList */
  const float *all_f;      /* caller's list of all values (length allNum) */
  const double *all_d;     /* caller's list of all values (length allNum) */
  int simulate,            /* if non-zero, we're being used for simulation,
                              not estimation: be tolerant of unset parms */
    estimate1Method,       /* what kind of single-tensor estimation to do */
    estimateB0,            /* if non-zero, B0 should be estimated along with
                              rest of model. Otherwise, B0 is found by simply
                              taking average of non-Dwi images */
    recordTime,            /* if non-zero, record estimation time */
    recordErrorDwi,
    recordErrorLogDwi,
    recordLikelihoodDwi,
    verbose,               /* blah blah blah */
    negEvalShift,          /* if non-zero, shift eigenvalues upwards so that
                              smallest one is non-negative */
    progress;              /* progress indication for volume processing */
  unsigned int WLSIterNum; /* number of iterations for WLS */
  /* internal -------- */
  /* a "dwi" in here is basically any value (diffusion-weighted or not)
     that varies as a function of the model parameters being estimated */
  int flag[128];           /* flags for state management */
  unsigned int allNum,     /* total number of images (Dwi and non-Dwi) */
    dwiNum;                /* number of Dwis */
  Nrrd *nbmat,             /* B-matrices (dwiNum of them) for the Dwis, with
                              off-diagonals (*YES*) pre-multiplied by 2,
                              and with a 7th column of -1.0 if estimateB0 */
    *nwght,                /* dwiNum x dwiNum matrix of weights */
    *nemat;                /* pseudo-inverse of nbmat */
  double knownB0,          /* B0 known from DWI, only if !estimateB0 */
    *all,                  /* (copy of) vector of input values,
                              allocated for allNum */
    *bnorm,                /* frob norm of B-matrix, allocated for allNum */
    *allTmp, *dwiTmp,      /* for storing intermediate values,
                              allocated for allNum and dwiNum respectively */
    *dwi;                  /* the Dwi values, allocated for dwiNum */
  unsigned char *skipLut;  /* skipLut[i] non-zero if we should ignore all[i],
                              allocated for allNum */
  /* output ---------- */
  double estimatedB0,      /* estimated non-Dwi value, only if estimateB0 */
    ten[7],                /* the estimated single tensor */
    conf,                  /* the "confidence" mask value (i.e. ten[0]) */
    mdwi,                  /* mean Dwi value (used for conf mask calc) */
    time,                  /* time required for estimation */
    errorDwi,              /* error in Dwi of estimate */
    errorLogDwi,           /* error in log(Dwi) of estimate */
    likelihoodDwi;         /* the maximized likelihood */
} tenEstimateContext;

/*
******** struct tenDwiGageKindData
**
** the kind data has static info that is set by the user
** and then not altered-- or else it wouldn't really be per-kind,
** and it wouldn't be thread-safe
*/
typedef struct {
  Nrrd *ngrad, *nbmat;       /* owned by us */
  double thresh, soft, bval, valueMin;
  int est1Method, est2Method;
  unsigned int randSeed;
} tenDwiGageKindData;

/*
******** struct tenDwiGagePvlData
**
** the pvl data is the dynamic stuff (buffers, mostly)
** that is potentially modified per query.
** being part of the pvl, such modifications are thread-safe
**
** the reason for having two tenEstimateContexts is that within
** one volume you will sometimes want to measure both one and
** two tensors, and it would be crazy to incur the overhead of
** switching between the two *per-query*.
*/
typedef struct {
  tenEstimateContext *tec1, /* for estimating single tensors */
    *tec2;                  /* for estimating two-tensors */
  double *vbuf;
  unsigned int *wght;
  double *qvals;
  double *qpoints;
  double *dists;
  double *weights;
  Nrrd *nten1EigenGrads;
  airRandMTState *randState;
  unsigned int randSeed;
  double ten1[7], ten1Evec[9], ten1Eval[3];
  int levmarUseFastExp;
  unsigned int levmarMaxIter;
  double levmarTau, levmarEps1, levmarEps2, levmarEps3,
    levmarDelta, levmarMinCp;
  double levmarInfo[9]; /* output */
} tenDwiGagePvlData;

typedef struct {
  /* input ------------- */
  int verbose;
  double convStep, minNorm, convEps, wghtSumEps;
  int enableRecurse;
  unsigned int maxIter, numSteps;
  int lengthFancy;
  /* internal ------------ */
  unsigned int allocLen;
  double *eval, *evec, *rtIn, *rtLog, *qIn, *qBuff, *qInter;
  /* output ------------ */
  unsigned int numIter;
  double convFinal;
  double lengthShape, lengthOrient;
} tenInterpParm;

/* the idea is that there should be a uniform way of describing
   DWI experiments, and as a result of calling one of the Set()
   functions below, the information is set in tenExperSpec so that
   combined with a diffusion model spec, DWIs can be simulated */
typedef struct {
  int set;               /* has been set */
  unsigned imgNum;       /* total number of images, dwi or not */
  double *bval,          /* all b values: imgNum doubles.
                            NOTE: if dwi[i] is to be considered a "b0" value
                            (for the various functions that have a knownB0 or
                            similarly named flag), bval[i] must be 0.0;
                            regardless of the gradient length */
  /* *wght,                 all weights, but not yet used */
    *grad;               /* all gradients: 3 x imgNum doubles */
} tenExperSpec;

/* description of *one* parameter in the parameter vector that
   defines one instance of a given model */
typedef struct {
  char name[AIR_STRLEN_SMALL]; /* name */
  double min, max;             /* bounds */
  int cyclic;                  /* non-zero if effectively min == max */
  int vec3;                    /* non-zero if this is a coefficient
                                  of a UNIT-LENGTH 3-vector */
  unsigned int vecIdx;         /* *if* this param is part of vector,
                                  the index into it, i.e. 0, 1, or 2 */
} tenModelParmDesc;

#define TEN_MODEL_B0_MAX 65500    /* HEY: fairly arbitrary, but is set to be
                                     a little below the max storable value
                                     of the unsigned short in which DWI
                                     values might be saved */
#define TEN_MODEL_DIFF_MAX 0.006  /* in units of mm^2/sec; diffusivity of
                                     water is about 0.003 mm^2/sec */
#define TEN_MODEL_PARM_GRAD_EPS 0.000005 /* for gradient calculations */

/*
******** struct tenModel
**
** Container for *information* about how DWI values arise (i.e. model
** parameters), with functions that help in fitting and evaluating
** models.  A tenModel does not contain any vector of model parameter
** values, it is only a description of those values (especially its
** tenModelParmDesc vector), and utilities for computing with those
** parameter values.
**
** NOTE: the current convention is to *always* have the non-DW T2
** image value ("B0") be the first value in the parameter vector that
** a tenModel is used to do describe.  The B0 value will be found either
** by trivial means (i.e copied from the image data), or by a different
** method than the rest of the model parameters, but it is stored along with
** the parameters because
** (1) its very handy to have in one place all the information that, when
** combined with the tenExperSpec, gives you a DWI value, and
** (2) sometimes B0 does have to be estimated at the same time as the rest
** of the model, so we thereby avoid doubling the number of models
** to be able to support this.
** On the other hand, the B0 value may not be stored along with the rest of
** the parm vec in the case of saving out whole nrrd of parm vecs.
** The "parmNum" field below therefore always includes the B0 value
*/
typedef struct tenModel_t {
  char name[AIR_STRLEN_SMALL];
  unsigned int parmNum;
  const tenModelParmDesc *parmDesc;
  /* noise free simulation */
  void (*simulate)(double *dwiSim, const double *parm,
                   const tenExperSpec *espec);
  /* parameter vector operations */
  char *(*sprint)(char str[AIR_STRLEN_MED], const double *parm);
  double *(*alloc)(void);
  void (*rand)(double *parm, airRandMTState *rng, int knownB0);
  void (*step)(double *parm1, const double scl,
               const double *grad, const double *parm0);
  double (*dist)(const double *parmA, const double *parmB);
  void (*copy)(double *parmA, const double *parmB);
  int (*convert)(double *parmDst, const double *parmSrc,
                 const struct tenModel_t *modelSrc);

  /* "sqe" == squared error in DWI values */
  double (*sqe)(const double *parm, const tenExperSpec *espec,
                double *dwiBuff, const double *dwiMeas, int knownB0);
  void (*sqeGrad)(double *grad, const double *parm,
                  const tenExperSpec *espec,
                  double *dwiBuff, const double *dwiMeas,
                  int knownB0);
  double (*sqeFit)(double *parm, double *convFrac, unsigned int *itersTaken,
                   const tenExperSpec *espec,
                   double *dwiBuff, const double *dwiMeas,
                   const double *parmInit, int knownB0,
                   unsigned int minIter, unsigned int maxIter,
                   double convEps, int verbose);
  /* "nll" == negative log likelihood */
  double (*nll)(const double *parm, const tenExperSpec *espec,
                double *dwiBuff, const double *dwiMeas,
                int rician, double sigma, int knownB0);
  void (*nllGrad)(double *grad, const double *parm,
                  const tenExperSpec *espec,
                  double *dwiBuff, const double *dwiMeas,
                  int rician, double sigma);
  double (*nllFit)(double *parm, const tenExperSpec *espec,
                   const double *dwiMeas, const double *parmInit,
                   int rician, double sigma, int knownB0);
} tenModel;

/* defaultsTen.c */
TEN_EXPORT const int tenPresent;
TEN_EXPORT const char *tenBiffKey;
TEN_EXPORT const char tenDefFiberKernel[];
TEN_EXPORT double tenDefFiberStepSize;
TEN_EXPORT int tenDefFiberUseIndexSpace;
TEN_EXPORT int tenDefFiberMaxNumSteps;
TEN_EXPORT double tenDefFiberMaxHalfLen;
TEN_EXPORT int tenDefFiberAnisoStopType;
TEN_EXPORT double tenDefFiberAnisoThresh;
TEN_EXPORT int tenDefFiberIntg;
TEN_EXPORT double tenDefFiberWPunct;

/* triple.c */
TEN_EXPORT void tenTripleConvertSingle_d(double dst[3],
                                         int dstType,
                                         const double src[3],
                                         const int srcType);
TEN_EXPORT void tenTripleConvertSingle_f(float dst[3],
                                         int dstType,
                                         const float src[3],
                                         const int srcType);
TEN_EXPORT void tenTripleCalcSingle_d(double dst[3],
                                      int ttype, double ten[7]);
TEN_EXPORT void tenTripleCalcSingle_f(float dst[3],
                                      int ttype, float ten[7]);
TEN_EXPORT int tenTripleCalc(Nrrd *nout, int ttype, const Nrrd *nten);
TEN_EXPORT int tenTripleConvert(Nrrd *nout, int dstType,
                                const Nrrd *nin, int srcType);

/* grads.c */
TEN_EXPORT tenGradientParm *tenGradientParmNew(void);
TEN_EXPORT tenGradientParm *tenGradientParmNix(tenGradientParm *tgparm);
TEN_EXPORT int tenGradientCheck(const Nrrd *ngrad, int type,
                                unsigned int minnum);
TEN_EXPORT int tenGradientRandom(Nrrd *ngrad, unsigned int num,
                                 unsigned int seed);
TEN_EXPORT double tenGradientIdealEdge(unsigned int N, int single);
TEN_EXPORT int tenGradientJitter(Nrrd *nout, const Nrrd *nin, double dist);
TEN_EXPORT int tenGradientBalance(Nrrd *nout, const Nrrd *nin,
                                  tenGradientParm *tgparm);
TEN_EXPORT void tenGradientMeasure(double *pot, double *minAngle,
                                   double *minEdge,
                                   const Nrrd *npos, tenGradientParm *tgparm,
                                   int edgeNormalize);
TEN_EXPORT int tenGradientDistribute(Nrrd *nout, const Nrrd *nin,
                                     tenGradientParm *tgparm);
TEN_EXPORT int tenGradientGenerate(Nrrd *nout, unsigned int num,
                                   tenGradientParm *tgparm);

/* enumsTen.c */
TEN_EXPORT const airEnum *const tenAniso;
TEN_EXPORT const airEnum *const tenInterpType;
TEN_EXPORT const airEnum _tenGage;
TEN_EXPORT const airEnum *const tenGage;
TEN_EXPORT const airEnum *const tenFiberType;
TEN_EXPORT const airEnum *const tenDwiFiberType;
TEN_EXPORT const airEnum *const tenFiberStop;
TEN_EXPORT const airEnum *const tenFiberIntg;
TEN_EXPORT const airEnum *const tenGlyphType;
TEN_EXPORT const airEnum *const tenEstimate1Method;
TEN_EXPORT const airEnum *const tenEstimate2Method;
TEN_EXPORT const airEnum *const tenTripleType;

/* path.c */
TEN_EXPORT tenInterpParm *tenInterpParmNew(void);
TEN_EXPORT tenInterpParm *tenInterpParmCopy(tenInterpParm *tip);
TEN_EXPORT int tenInterpParmBufferAlloc(tenInterpParm *tip,
                                        unsigned int num);
TEN_EXPORT tenInterpParm *tenInterpParmNix(tenInterpParm *tip);
TEN_EXPORT void tenInterpTwo_d(double oten[7],
                               const double tenA[7],
                               const double tenB[7],
                               int ptype, double aa,
                               tenInterpParm *tip);
TEN_EXPORT int tenInterpN_d(double tenOut[7],
                            const double *tenIn,
                            const double *wght,
                            unsigned int num, int ptype, tenInterpParm *tip);
TEN_EXPORT double tenInterpPathLength(Nrrd *npath, int doubleVerts,
                                      int fancy, int shape);
TEN_EXPORT int tenInterpTwoDiscrete_d(Nrrd *nout,
                                      const double tenA[7],
                                      const double tenB[7],
                                      int ptype, unsigned int num,
                                      tenInterpParm *tip);
TEN_EXPORT double tenInterpDistanceTwo_d(const double tenA[7],
                                         const double tenB[7],
                                         int ptype, tenInterpParm *tip);
TEN_EXPORT int tenInterpMulti3D(Nrrd *nout, const Nrrd *const *nin,
                                const double *wght,
                                unsigned int ninNum,
                                int ptype, tenInterpParm *tip);

/* glyph.c */
TEN_EXPORT tenGlyphParm *tenGlyphParmNew(void);
TEN_EXPORT tenGlyphParm *tenGlyphParmNix(tenGlyphParm *parm);
TEN_EXPORT int tenGlyphParmCheck(tenGlyphParm *parm,
                                 const Nrrd *nten, const Nrrd *npos,
                                 const Nrrd *nslc);
TEN_EXPORT int tenGlyphGen(limnObject *glyphs, echoScene *scene,
                           tenGlyphParm *parm,
                           const Nrrd *nten, const Nrrd *npos,
                           const Nrrd *nslc);
TEN_EXPORT unsigned int tenGlyphBqdZoneEval(const double eval[3]);
TEN_EXPORT void tenGlyphBqdUvEval(double uv[2], const double eval[3]);
TEN_EXPORT void tenGlyphBqdEvalUv(double eval[3], const double uv[2]);
TEN_EXPORT unsigned int tenGlyphBqdZoneUv(const double uv[2]);
TEN_EXPORT void tenGlyphBqdAbcUv(double abc[3], const double uv[2],
                                 double betaMax);

/* tensor.c */
TEN_EXPORT int tenVerbose;
TEN_EXPORT void tenRotateSingle_f(float tenOut[7],
                                  const float rot[9], const float tenIn[7]);
TEN_EXPORT int tenTensorCheck(const Nrrd *nin,
                              int wantType, int want4D, int useBiff);
TEN_EXPORT int tenMeasurementFrameReduce(Nrrd *nout, const Nrrd *nin);
TEN_EXPORT int tenExpand2D(Nrrd *nout, const Nrrd *nin,
                           double scale, double thresh);
TEN_EXPORT int tenExpand(Nrrd *tnine, const Nrrd *tseven,
                         double scale, double thresh);
TEN_EXPORT int tenShrink(Nrrd *tseven, const Nrrd *nconf, const Nrrd *tnine);
TEN_EXPORT int tenEigensolve_f(float eval[3], float evec[9],
                               const float ten[7]);
TEN_EXPORT int tenEigensolve_d(double eval[3], double evec[9],
                               const double ten[7]);
TEN_EXPORT void tenMakeSingle_f(float ten[7],
                                float conf, const float eval[3],
                                const float evec[9]);
TEN_EXPORT void tenMakeSingle_d(double ten[7],
                                double conf, const double eval[3],
                                const double evec[9]);
TEN_EXPORT int tenMake(Nrrd *nout, const Nrrd *nconf,
                       const Nrrd *neval, const Nrrd *nevec);
TEN_EXPORT int tenSlice(Nrrd *nout, const Nrrd *nten,
                        unsigned int axis, size_t pos, unsigned int dim);
TEN_EXPORT void tenInvariantGradientsK_d(double K1[7],
                                         double K2[7],
                                         double K3[7],
                                         const double ten[7],
                                         const double minnorm);
TEN_EXPORT void tenInvariantGradientsR_d(double R1[7],
                                         double R2[7],
                                         double R3[7],
                                         const double ten[7],
                                         const double minnorm);
TEN_EXPORT void tenRotationTangents_d(double phi1[7],
                                      double phi2[7],
                                      double phi3[7],
                                      const double evec[9]);
TEN_EXPORT void tenLogSingle_d(double logten[7], const double ten[7]);
TEN_EXPORT void tenLogSingle_f(float logten[7], const float ten[7]);
TEN_EXPORT void tenExpSingle_d(double expten[7], const double ten[7]);
TEN_EXPORT void tenExpSingle_f(float expten[7], const float ten[7]);
TEN_EXPORT void tenSqrtSingle_d(double sqrtten[7], const double ten[7]);
TEN_EXPORT void tenSqrtSingle_f(float sqrtten[7], const float ten[7]);
TEN_EXPORT void tenPowSingle_d(double powten[7], const double ten[7],
                               double power);
TEN_EXPORT void tenPowSingle_f(float powten[7], const float ten[7],
                               float power);
/* should rename to tenInvSingle_x */
TEN_EXPORT void tenInv_f(float inv[7], const float ten[7]);
TEN_EXPORT void tenInv_d(double inv[7], const double ten[7]);
TEN_EXPORT double tenDoubleContract_d(double a[7], double T[21], double b[7]);

/* chan.c */
/* old tenCalc* functions superceded/deprecated by new tenEstimate* code */
TEN_EXPORT const char *tenDWMRIModalityKey;
TEN_EXPORT const char *tenDWMRIModalityVal;
TEN_EXPORT const char *tenDWMRINAVal;
TEN_EXPORT const char *tenDWMRIBValueKey;
TEN_EXPORT const char *tenDWMRIGradKeyFmt;
TEN_EXPORT const char *tenDWMRIBmatKeyFmt;
TEN_EXPORT const char *tenDWMRINexKeyFmt;
TEN_EXPORT const char *tenDWMRISkipKeyFmt;
TEN_EXPORT int tenDWMRIKeyValueParse(Nrrd **ngradP, Nrrd **nbmatP, double *bP,
                                     unsigned int **skip,
                                     unsigned int *skipNum,
                                     const Nrrd *ndwi);
TEN_EXPORT int tenBMatrixCalc(Nrrd *nbmat, const Nrrd *ngrad);
TEN_EXPORT int tenEMatrixCalc(Nrrd *nemat, const Nrrd *nbmat, int knownB0);
TEN_EXPORT void tenEstimateLinearSingle_f(float *ten, float *B0P,
                                          const float *dwi, const double *emat,
                                          double *vbuf, unsigned int DD,
                                          int knownB0, float thresh,
                                          float soft, float b);
TEN_EXPORT void tenEstimateLinearSingle_d(double *ten, double *B0P,
                                          const double *dwi, const double*emat,
                                          double *vbuf, unsigned int DD,
                                          int knownB0, double thresh,
                                          double soft, double b);
TEN_EXPORT int tenEstimateLinear3D(Nrrd *nten, Nrrd **nterrP, Nrrd **nB0P,
                                   const Nrrd *const *ndwi,
                                   unsigned int dwiLen,
                                   const Nrrd *nbmat, int knownB0,
                                   double thresh, double soft, double b);
TEN_EXPORT int tenEstimateLinear4D(Nrrd *nten, Nrrd **nterrP, Nrrd **nB0P,
                                   const Nrrd *ndwi, const Nrrd *_nbmat,
                                   int knownB0,
                                   double thresh, double soft, double b);
TEN_EXPORT void tenSimulateSingle_f(float *dwi, float B0, const float *ten,
                                    const double *bmat, unsigned int DD,
                                    float b);
TEN_EXPORT int tenSimulate(Nrrd *ndwi, const Nrrd *nT2, const Nrrd *nten,
                           const Nrrd *nbmat, double b);

/* estimate.c */
TEN_EXPORT tenEstimateContext *tenEstimateContextNew(void);
TEN_EXPORT void tenEstimateVerboseSet(tenEstimateContext *tec,
                                      int verbose);
TEN_EXPORT void tenEstimateNegEvalShiftSet(tenEstimateContext *tec,
                                           int doit);
TEN_EXPORT int tenEstimateMethodSet(tenEstimateContext *tec,
                                    int estMethod);
TEN_EXPORT int tenEstimateSigmaSet(tenEstimateContext *tec,
                                   double sigma);
TEN_EXPORT int tenEstimateValueMinSet(tenEstimateContext *tec,
                                      double valueMin);
TEN_EXPORT int tenEstimateGradientsSet(tenEstimateContext *tec,
                                       const Nrrd *ngrad,
                                       double bValue, int estimateB0);
TEN_EXPORT int tenEstimateBMatricesSet(tenEstimateContext *tec,
                                       const Nrrd *nbmat,
                                       double bValue, int estimateB0);
TEN_EXPORT int tenEstimateSkipSet(tenEstimateContext *tec,
                                  unsigned int valIdx,
                                  int doSkip);
TEN_EXPORT int tenEstimateSkipReset(tenEstimateContext *tec);
TEN_EXPORT int tenEstimateThresholdSet(tenEstimateContext *tec,
                                       double thresh, double soft);
TEN_EXPORT int tenEstimateUpdate(tenEstimateContext *tec);
TEN_EXPORT int tenEstimate1TensorSimulateSingle_f(tenEstimateContext *tec,
                                                  float *simval,
                                                  float sigma,
                                                  float bValue, float B0,
                                                  const float _ten[7]);
TEN_EXPORT int tenEstimate1TensorSimulateSingle_d(tenEstimateContext *tec,
                                                  double *simval,
                                                  double sigma,
                                                  double bValue, double B0,
                                                  const double ten[7]);
TEN_EXPORT int tenEstimate1TensorSimulateVolume(tenEstimateContext *tec,
                                                Nrrd *ndwi,
                                                double sigma, double bValue,
                                                const Nrrd *nB0,
                                                const Nrrd *nten,
                                                int outType,
                                                int keyValueSet);
TEN_EXPORT int tenEstimate1TensorSingle_f(tenEstimateContext *tec,
                                          float ten[7], const float *all);
TEN_EXPORT int tenEstimate1TensorSingle_d(tenEstimateContext *tec,
                                          double ten[7], const double *all);
TEN_EXPORT int tenEstimate1TensorVolume4D(tenEstimateContext *tec,
                                          Nrrd *nten,
                                          Nrrd **nB0P, Nrrd **nterrP,
                                          const Nrrd *ndwi, int outType);
TEN_EXPORT tenEstimateContext *tenEstimateContextNix(tenEstimateContext *tec);

/* aniso.c */
TEN_EXPORT float (*_tenAnisoEval_f[TEN_ANISO_MAX+1])(const float eval[3]);
TEN_EXPORT float tenAnisoEval_f(const float eval[3], int aniso);
TEN_EXPORT double (*_tenAnisoEval_d[TEN_ANISO_MAX+1])(const double eval[3]);
TEN_EXPORT double tenAnisoEval_d(const double eval[3], int aniso);

TEN_EXPORT float (*_tenAnisoTen_f[TEN_ANISO_MAX+1])(const float ten[7]);
TEN_EXPORT float tenAnisoTen_f(const float ten[7], int aniso);
TEN_EXPORT double (*_tenAnisoTen_d[TEN_ANISO_MAX+1])(const double ten[7]);
TEN_EXPORT double tenAnisoTen_d(const double ten[7], int aniso);

TEN_EXPORT int tenAnisoPlot(Nrrd *nout, int aniso, unsigned int res,
                            int hflip, int whole, int nanout);
TEN_EXPORT int tenAnisoVolume(Nrrd *nout, const Nrrd *nin,
                              int aniso, double confThresh);
TEN_EXPORT int tenAnisoHistogram(Nrrd *nout, const Nrrd *nin,
                                 const Nrrd *nwght, int right,
                                 int version, unsigned int resolution);
TEN_EXPORT tenEvecRGBParm *tenEvecRGBParmNew(void);
TEN_EXPORT tenEvecRGBParm *tenEvecRGBParmNix(tenEvecRGBParm *rgbp);
TEN_EXPORT int tenEvecRGBParmCheck(const tenEvecRGBParm *rgbp);
TEN_EXPORT void tenEvecRGBSingle_f(float RGB[3], float conf,
                                   const float eval[3], const float evec[3],
                                   const tenEvecRGBParm *rgbp);
TEN_EXPORT void tenEvecRGBSingle_d(double RGB[3], double conf,
                                   const double eval[3], const double evec[3],
                                   const tenEvecRGBParm *rgbp);

/* miscTen.c */
TEN_EXPORT int tenEvecRGB(Nrrd *nout, const Nrrd *nin,
                          const tenEvecRGBParm *rgbp);
TEN_EXPORT short tenEvqSingle_f(float vec[3], float scl);
TEN_EXPORT int tenEvqVolume(Nrrd *nout, const Nrrd *nin, int which,
                            int aniso, int scaleByAniso);
TEN_EXPORT int tenBMatrixCheck(const Nrrd *nbmat,
                               int type, unsigned int minnum);
TEN_EXPORT int _tenFindValley(double *valP, const Nrrd *nhist,
                              double tweak, int save);

/* fiberMethods.c */
TEN_EXPORT void tenFiberSingleInit(tenFiberSingle *tfbs);
TEN_EXPORT void tenFiberSingleDone(tenFiberSingle *tfbs);
TEN_EXPORT tenFiberSingle *tenFiberSingleNew(void);
TEN_EXPORT tenFiberSingle *tenFiberSingleNix(tenFiberSingle *tfbs);
TEN_EXPORT tenFiberContext *tenFiberContextNew(const Nrrd *dtvol);
TEN_EXPORT tenFiberContext *tenFiberContextDwiNew(const Nrrd *dwivol,
                                                  double thresh,
                                                  double soft,
                                                  double valueMin,
                                                  int ten1method,
                                                  int ten2method);
TEN_EXPORT void tenFiberVerboseSet(tenFiberContext *tfx, int verbose);
TEN_EXPORT int tenFiberTypeSet(tenFiberContext *tfx, int type);
TEN_EXPORT int tenFiberKernelSet(tenFiberContext *tfx,
                                 const NrrdKernel *kern,
                                 const double parm[NRRD_KERNEL_PARMS_NUM]);
TEN_EXPORT int tenFiberProbeItemSet(tenFiberContext *tfx, int item);
TEN_EXPORT int tenFiberIntgSet(tenFiberContext *tfx, int intg);
TEN_EXPORT int tenFiberStopSet(tenFiberContext *tfx, int stop, ...);
TEN_EXPORT int tenFiberStopAnisoSet(tenFiberContext *tfx,
                                    int anisoType, double anisoThresh);
TEN_EXPORT int tenFiberStopDoubleSet(tenFiberContext *tfx,
                                     int stop, double val);
TEN_EXPORT int tenFiberStopUIntSet(tenFiberContext *tfx,
                                   int stop, unsigned int val);
TEN_EXPORT void tenFiberStopOn(tenFiberContext *tfx, int stop);
TEN_EXPORT void tenFiberStopOff(tenFiberContext *tfx, int stop);
TEN_EXPORT void tenFiberStopReset(tenFiberContext *tfx);
TEN_EXPORT int tenFiberAnisoSpeedSet(tenFiberContext *tfx, int aniso,
                                     double lerp, double thresh, double soft);
TEN_EXPORT int tenFiberAnisoSpeedReset(tenFiberContext *tfx);
TEN_EXPORT int tenFiberParmSet(tenFiberContext *tfx, int parm, double val);
TEN_EXPORT int tenFiberUpdate(tenFiberContext *tfx);
TEN_EXPORT tenFiberContext *tenFiberContextCopy(tenFiberContext *tfx);
TEN_EXPORT tenFiberContext *tenFiberContextNix(tenFiberContext *tfx);

/* fiber.c */
TEN_EXPORT int tenFiberTraceSet(tenFiberContext *tfx, Nrrd *nfiber,
                                double *buff, unsigned int halfBuffLen,
                                unsigned int *startIdxP, unsigned int *endIdxP,
                                double seed[3]);
TEN_EXPORT int tenFiberTrace(tenFiberContext *tfx,
                             Nrrd *nfiber, double seed[3]);
TEN_EXPORT unsigned int tenFiberDirectionNumber(tenFiberContext *tfx,
                                                double seed[3]);
TEN_EXPORT int tenFiberSingleTrace(tenFiberContext *tfx, tenFiberSingle *tfbs,
                                   double seed[3], unsigned int which);
TEN_EXPORT tenFiberMulti *tenFiberMultiNew(void);
TEN_EXPORT tenFiberMulti *tenFiberMultiNix(tenFiberMulti *tfm);
TEN_EXPORT int tenFiberMultiTrace(tenFiberContext *tfx, tenFiberMulti *tfml,
                                  const Nrrd *nseed);
TEN_EXPORT int tenFiberMultiPolyData(tenFiberContext *tfx,
                                     limnPolyData *lpld, tenFiberMulti *tfml);
TEN_EXPORT int tenFiberMultiProbeVals(tenFiberContext *tfx,
                                      Nrrd *nval, tenFiberMulti *tfml);

/* epireg.c */
TEN_EXPORT int _tenEpiRegThresholdFind(double *DWthrP, Nrrd **nin,
                                       int ninLen, int save, double expo);
TEN_EXPORT int tenEpiRegister3D(Nrrd **nout, Nrrd **ndwi,
                                unsigned int dwiLen, Nrrd *ngrad,
                                int reference,
                                double bwX, double bwY,
                                double fitFrac, double DWthr,
                                int doCC,
                                const NrrdKernel *kern, double *kparm,
                                int progress, int verbose);
TEN_EXPORT int tenEpiRegister4D(Nrrd *nout, Nrrd *nin, Nrrd *ngrad,
                                int reference,
                                double bwX, double bwY,
                                double fitFrac, double DWthr,
                                int doCC,
                                const NrrdKernel *kern, double *kparm,
                                int progress, int verbose);

/* experSpec.c */
TEN_EXPORT tenExperSpec* tenExperSpecNew(void);
TEN_EXPORT int tenExperSpecGradSingleBValSet(tenExperSpec *espec,
                                             int insertB0,
                                             double bval,
                                             const double *grad,
                                             unsigned int gradNum);
TEN_EXPORT int tenExperSpecGradBValSet(tenExperSpec *espec,
                                       int insertB0,
                                       const double *bval,
                                       const double *grad,
                                       unsigned int bgNum);
/*
TEN_EXPORT int tenExperSpecGradBValWghtSet(tenExperSpec *espec,
                                           int insertB0,
                                           const double *bval,
                                           const double *grad,
                                           const double *wght,
                                           unsigned int bgwNum);
*/
TEN_EXPORT int tenExperSpecFromKeyValueSet(tenExperSpec *espec,
                                           const Nrrd *ndwi);
TEN_EXPORT tenExperSpec* tenExperSpecNix(tenExperSpec *espec);
TEN_EXPORT double tenExperSpecKnownB0Get(const tenExperSpec *espec,
                                         const double *dwi);
TEN_EXPORT double tenExperSpecMaxBGet(const tenExperSpec *espec);
TEN_EXPORT int tenDWMRIKeyValueFromExperSpecSet(Nrrd *ndwi,
                                                const tenExperSpec *espec);

/* tenModel.c */
TEN_EXPORT const char *tenModelPrefixStr;
TEN_EXPORT int tenModelParse(const tenModel **model, int *plusB0,
                             int requirePrefix, const char *str);
TEN_EXPORT int tenModelFromAxisLearnPossible(const NrrdAxisInfo *axinfo);
TEN_EXPORT int tenModelFromAxisLearn(const tenModel **model, int *plusB0,
                                     const NrrdAxisInfo *axinfo);
TEN_EXPORT int tenModelSimulate(Nrrd *ndwi, int typeOut,
                                tenExperSpec *espec,
                                const tenModel *model,
                                const Nrrd *nB0, /* maybe NULL */
                                const Nrrd *nparm,
                                int keyValueSet);
TEN_EXPORT int tenModelSqeFit(Nrrd *nparm,
                              Nrrd **nsqeP, Nrrd **nconvP, Nrrd **niterP,
                              const tenModel *model,
                              const tenExperSpec *espec, const Nrrd *ndwi,
                              int knownB0, int saveB0, int typeOut,
                              unsigned int minIter, unsigned int maxIter,
                              unsigned int starts, double convEps,
                              airRandMTState *rng,
                              int verbose);
TEN_EXPORT int tenModelNllFit(Nrrd *nparm, Nrrd **nnllP,
                              const tenModel *model,
                              const tenExperSpec *espec, const Nrrd *ndwi,
                              int rician, double sigma, int knownB0);
TEN_EXPORT int tenModelConvert(Nrrd *nparmDst, int *convRet,
                               const tenModel *modelDst,
                               const Nrrd *nparmSrc,
                               const tenModel *modelSrc);

/*
** Have to keep this list of model declarations in sync with:
** * tenModel.c/str2model()
** * all model*.c/parmConvert()  (fallen behind here)
** * sources.cmake, GNUmakefile
**
** Note that the TEN_MODEL_STR_* strings should be all lowercase,
** as long as we want the model parsing to be case insensitive, and
** as long as the logic in tenModel.c/str2model() is so simplistic
*/
/* modelZero.c */
#define TEN_MODEL_STR_ZERO "zero"
TEN_EXPORT const tenModel *const tenModelZero;
/* modelB0.c */
#define TEN_MODEL_STR_B0 "b0"
TEN_EXPORT const tenModel *const tenModelB0;
/* modelBall.c */
#define TEN_MODEL_STR_BALL "ball"
TEN_EXPORT const tenModel *const tenModelBall;
/* model1Vector2D.c */
#define TEN_MODEL_STR_1VECTOR2D "1vector2d"
TEN_EXPORT const tenModel *const tenModel1Vector2D;
/* model1Unit2D.c */
#define TEN_MODEL_STR_1UNIT2D "1unit2d"
TEN_EXPORT const tenModel *const tenModel1Unit2D;
/* model2Unit2D.c */
#define TEN_MODEL_STR_2UNIT2D "2unit2d"
TEN_EXPORT const tenModel *const tenModel2Unit2D;
/* model1Stick.c */
#define TEN_MODEL_STR_1STICK "1stick"
TEN_EXPORT const tenModel *const tenModel1Stick;
/* modelBall1StickEMD.c */
#define TEN_MODEL_STR_BALL1STICKEMD "ball1stickemd"
TEN_EXPORT const tenModel *const tenModelBall1StickEMD;
/* modelBall1Stick.c */
#define TEN_MODEL_STR_BALL1STICK "ball1stick"
TEN_EXPORT const tenModel *const tenModelBall1Stick;
/* modelBall1Cylinder.c */
#define TEN_MODEL_STR_BALL1CYLINDER "ball1cylinder"
TEN_EXPORT const tenModel *const tenModelBall1Cylinder;
/* model1Cylinder.c */
#define TEN_MODEL_STR_1CYLINDER "1cylinder"
TEN_EXPORT const tenModel *const tenModel1Cylinder;
/* model1Tensor2.c: 2nd-order tensor (one of them), not two-tensor */
#define TEN_MODEL_STR_1TENSOR2 "1tensor2"
TEN_EXPORT const tenModel *const tenModel1Tensor2;

/* mod.c */
TEN_EXPORT int tenSizeNormalize(Nrrd *nout, const Nrrd *nin, double weight[3],
                                double amount, double target);
TEN_EXPORT int tenSizeScale(Nrrd *nout, const Nrrd *nin, double amount);
TEN_EXPORT int tenAnisoScale(Nrrd *nout, const Nrrd *nin, double scale,
                             int fixDet, int makePositive);
TEN_EXPORT int tenEigenvaluePower(Nrrd *nout, const Nrrd *nin, double expo);
TEN_EXPORT int tenEigenvalueClamp(Nrrd *nout, const Nrrd *nin,
                                  double min, double max);
TEN_EXPORT int tenEigenvalueAdd(Nrrd *nout, const Nrrd *nin, double val);
TEN_EXPORT int tenEigenvalueMultiply(Nrrd *nout, const Nrrd *nin, double val);
TEN_EXPORT int tenLog(Nrrd *nout, const Nrrd *nin);
TEN_EXPORT int tenExp(Nrrd *nout, const Nrrd *nin);

/* bvec.c */
TEN_EXPORT int tenBVecNonLinearFit(Nrrd *nout, const Nrrd *nin,
                                   double *bb, double *ww,
                                   int iterMax, double eps);

/* tenGage.c */
TEN_EXPORT gageKind *tenGageKind;

/* tenDwiGage.c */
/* we can't declare or define a tenDwiGageKind->name (analogous to
   tenGageKind->name or gageSclKind->name) because the DWI kind is
   dynamically allocated, but at least we can declare a cannonical
   name (HEY: ugly) */
#define TEN_DWI_GAGE_KIND_NAME "dwi"
TEN_EXPORT const airEnum _tenDwiGage;
TEN_EXPORT const airEnum *const tenDwiGage;
TEN_EXPORT gageKind *tenDwiGageKindNew(void);
TEN_EXPORT gageKind *tenDwiGageKindNix(gageKind *dwiKind);
/* warning: this function will likely change its arguments in the future */
TEN_EXPORT int tenDwiGageKindSet(gageKind *dwiKind,
                                 double thresh, double soft,
                                 double bval, double valueMin,
                                 const Nrrd *ngrad,
                                 const Nrrd *nbmat,
                                 int emethod1, int emethod2,
                                 unsigned int randSeed);
TEN_EXPORT int tenDwiGageKindCheck(const gageKind *kind);

/* bimod.c */
TEN_EXPORT tenEMBimodalParm* tenEMBimodalParmNew(void);
TEN_EXPORT tenEMBimodalParm* tenEMBimodalParmNix(tenEMBimodalParm *biparm);
TEN_EXPORT int tenEMBimodal(tenEMBimodalParm *biparm, const Nrrd *nhisto);

/* tend{Flotsam,Anplot,Anvol,Evec,Eval,. . .}.c */
#define TEND_DECLARE(C) TEN_EXPORT unrrduCmd tend_##C##Cmd;
#define TEND_LIST(C) &tend_##C##Cmd,
/* removed from below (superseded by estim): F(calc) \ */
#define TEND_MAP(F) \
F(about) \
F(grads) \
F(epireg) \
F(bmat) \
F(estim) \
F(sim) \
F(mfit) \
F(mconv) \
F(msim) \
F(make) \
F(avg) \
F(helix) \
F(sten) \
F(glyph) \
F(ellipse) \
F(anplot) \
F(anvol) \
F(anscale) \
F(anhist) \
F(triple) \
F(tconv) \
F(point) \
F(slice) \
F(norm) \
F(fiber) \
F(eval) \
F(evalpow) \
F(evalclamp) \
F(evaladd) \
F(evalmult) \
F(log) \
F(exp) \
F(evec) \
F(evecrgb) \
F(evq) \
F(unmf) \
F(expand) \
F(shrink) \
F(bfit) \
F(satin)
TEND_MAP(TEND_DECLARE)
TEN_EXPORT unrrduCmd *tendCmdList[];
TEN_EXPORT hestCB *tendFiberStopCB;
TEN_EXPORT const char *tendTitle;

#ifdef __cplusplus
}
#endif

#endif /* TEN_HAS_BEEN_INCLUDED */
