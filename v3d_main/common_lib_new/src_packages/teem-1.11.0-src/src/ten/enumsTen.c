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

#include "ten.h"
#include "privateTen.h"

/* -------------------------------------------------------------- */

const char *
_tenAnisoStr[TEN_ANISO_MAX+1] = {
  "(unknown aniso)",
  "Conf",
  "Cl1",
  "Cp1",
  "Ca1",
  "Clpmin1",
  "Cs1",
  "Ct1",
  "Cl2",
  "Cp2",
  "Ca2",
  "Clpmin2",
  "Cs2",
  "Ct2",
  "RA",
  "FA",
  "VF",
  "B",
  "Q",
  "R",
  "S",
  "Skew",
  "Mode",
  "Th",
  "Omega",
  "Det",
  "Tr",
  "eval0",
  "eval1",
  "eval2"
};

const airEnum
_tenAniso = {
  "anisotropy metric",
  TEN_ANISO_MAX,
  _tenAnisoStr,  NULL,
  NULL,
  NULL, NULL,
  AIR_FALSE
};
const airEnum *const
tenAniso = &_tenAniso;

/* --------------------------------------------------------------------- */

const char *
_tenInterpTypeStr[TEN_INTERP_TYPE_MAX+1] = {
  "(unknown interp type)",
  "lin",
  "loglin",
  "affinv",
  "wang",
  "geoloxk",
  "geoloxr",
  "loxk",
  "loxr",
  "qgeoloxk",
  "qgeoloxr",
  "rtplin"
};

const char *
_tenInterpTypeStrEqv[] = {
  "lin", "linear", "lerp",
  "loglin", "loglinear", "loglerp",
  "affinv",
  "wang",
  "geoloxk", "glk",
  "geoloxr", "glr",
  "loxk",
  "loxr",
  "qgeoloxk", "qglk",
  "qgeoloxr", "qglr",
  "rtplin",
  ""
};

const int
_tenInterpTypeValEqv[] = {
  tenInterpTypeLinear, tenInterpTypeLinear, tenInterpTypeLinear,
  tenInterpTypeLogLinear, tenInterpTypeLogLinear, tenInterpTypeLogLinear,
  tenInterpTypeAffineInvariant,
  tenInterpTypeWang,
  tenInterpTypeGeoLoxK, tenInterpTypeGeoLoxK,
  tenInterpTypeGeoLoxR, tenInterpTypeGeoLoxR,
  tenInterpTypeLoxK,
  tenInterpTypeLoxR,
  tenInterpTypeQuatGeoLoxK, tenInterpTypeQuatGeoLoxK,
  tenInterpTypeQuatGeoLoxR, tenInterpTypeQuatGeoLoxR,
  tenInterpTypeRThetaPhiLinear
};

const airEnum
_tenInterpType = {
  "interp type",
  TEN_INTERP_TYPE_MAX,
  _tenInterpTypeStr, NULL,
  NULL,
  _tenInterpTypeStrEqv, _tenInterpTypeValEqv,
  AIR_FALSE
};
const airEnum *const
tenInterpType = &_tenInterpType;

/* --------------------------------------------------------------------- */

const char *
_tenGageStr[] = {
  "(unknown tenGage)",

  "tensor",
  "confidence",

  "trace",
  "N",
  "B",
  "det",
  "S",
  "Q",
  "FA",
  "R",
  "mode",
  "theta",
  "modew",
  "omega",

  "evals",
  "eval0",
  "eval1",
  "eval2",
  "evecs",
  "evec0",
  "evec1",
  "evec2",

  "delnk2",
  "delnk3",
  "delnr1",
  "delnr2",
  "delnphi1",
  "delnphi2",
  "delnphi3",

  "tensor grad",
  "tensor grad mag",
  "tensor grad mag mag",

  "trace grad vec",
  "trace grad mag",
  "trace normal",

  "norm grad vec",
  "norm grad mag",
  "norm normal",

  "B grad vec",
  "B grad mag",
  "B normal",

  "det grad vec",
  "det grad mag",
  "det normal",

  "S grad vec",
  "S grad mag",
  "S normal",

  "Q grad vec",
  "Q grad mag",
  "Q normal",

  "FA grad vec",
  "FA grad mag",
  "FA normal",

  "R grad vec",
  "R grad mag",
  "R normal",

  "mode grad vec",
  "mode grad mag",
  "mode normal",

  "theta grad vec",
  "theta grad mag",
  "theta normal",

  "omega grad vec",
  "omega grad mag",
  "omega normal",

  "invariant K gradients",
  "invariant K gradient mags",
  "invariant R gradients",
  "invariant R gradient mags",
  "rotation tangents",
  "rotation tangent mags",

  "eigenvalue gradients",

  "Cl1",
  "Cp1",
  "Ca1",
  "Clpmin1",
  "Cl2",
  "Cp2",
  "Ca2",
  "Clpmin2",

  "hessian",
  "trace hessian",
  "trace hessian evals",
  "trace hessian eval 0",
  "trace hessian eval 1",
  "trace hessian eval 2",
  "trace hessian evecs",
  "trace hessian evec 0",
  "trace hessian evec 1",
  "trace hessian evec 2",
  "trace hessian frob",
  "B hessian",
  "det hessian",
  "S hessian",
  "Q hessian",

  "FA hessian",
  "FA hessian evals",
  "FA hessian eval 0",
  "FA hessian eval 1",
  "FA hessian eval 2",
  "FA hessian evecs",
  "FA hessian evec 0",
  "FA hessian evec 1",
  "FA hessian evec 2",
  "FA hessian frob",
  "FA ridge surface strength",
  "FA valley surface strength",
  "FA laplacian",
  "FA hessian eval mode",
  "FA ridge line alignment",
  "FA ridge surface alignment",
  "FA 2nd DD",

  "FA geometry tensor",
  "FA kappa1",
  "FA kappa2",
  "FA total curv",
  "FA shape index",
  "FA mean curv",
  "FA gauss curv",
  "FA curv dir1",
  "FA curv dir2",
  "FA flowline curv",

  "R hessian",

  "mode hessian",
  "mode hessian evals",
  "mode hessian eval 0",
  "mode hessian eval 1",
  "mode hessian eval 2",
  "mode hessian evecs",
  "mode hessian evec 0",
  "mode hessian evec 1",
  "mode hessian evec 2",
  "mode hessian frob",

  "omega hessian",
  "omega hessian evals",
  "omega hessian eval 0",
  "omega hessian eval 1",
  "omega hessian eval 2",
  "omega hessian evecs",
  "omega hessian evec 0",
  "omega hessian evec 1",
  "omega hessian evec 2",
  "omega laplacian",
  "omega 2nd DD",
  "omega hessian contracted with ten evec 0",
  "omega hessian contracted with ten evec 1",
  "omega hessian contracted with ten evec 2",

  "trace gradvec dot evec0",
  "diffusionAlign(trace)",
  "diffusionFraction(trace)",
  "FA gradvec dot evec0",
  "diffusionAlign(FA)",
  "diffusionFraction(FA)",
  "omega gradvec dot evec0",
  "diffusionAlign(Omega)",
  "diffusionFraction(Omega)",
  "conf gradvec dot evec0",
  "diffusionAlign(conf)",
  "diffusionFraction(conf)",

  "cov",
  "covr",
  "covk",

  "logeuclid",
  "qglk",
  "qglr",
  "rtpl",

  "cl1gv",
  "cl1gm",
  "cl1gn",
  "cp1gv",
  "cp1gm",
  "cp1gn",
  "ca1gv",
  "ca1gm",
  "ca1gn",
  "tgrote",
  "eval hessian",
  "cl1 hessian",
  "cl1 hessian evals",
  "cl1 hessian eval 0",
  "cl1 hessian eval 1",
  "cl1 hessian eval 2",
  "cl1 hessian evecs",
  "cl1 hessian evec 0",
  "cl1 hessian evec 1",
  "cl1 hessian evec 2",
  "cp1 hessian",
  "cp1 hessian evals",
  "cp1 hessian eval 0",
  "cp1 hessian eval 1",
  "cp1 hessian eval 2",
  "cp1 hessian evecs",
  "cp1 hessian evec 0",
  "cp1 hessian evec 1",
  "cp1 hessian evec 2",
  "ca1 hessian",
  "ca1 hessian evals",
  "ca1 hessian eval 0",
  "ca1 hessian eval 1",
  "ca1 hessian eval 2",
  "ca1 hessian evecs",
  "ca1 hessian evec 0",
  "ca1 hessian evec 1",
  "ca1 hessian evec 2",

  "fiber curving index",
  "fiber dispersion index",

  "anisotropies"
};

const char *
_tenGageDesc[] = {
  "(unknown tenGage item)",
  "tensor",
  "confidence",
  "trace",
  "norm",
  "B",
  "determinant",
  "S",
  "Q",
  "FA",
  "R",
  "mode",
  "theta",
  "warped mode",
  "omega",
  "3 eigenvalues",
  "eigenvalue 0",
  "eigenvalue 1",
  "eigenvalue 2",
  "3 eigenvectors",
  "eigenvector 0",
  "eigenvector 1",
  "eigenvector 2",
  "delnk2",   /* sorry */
  "delnk3",   /* sorry */
  "delnr1",   /* sorry */
  "delnr2",   /* sorry */
  "delnphi1", /* sorry */
  "delnphi2", /* sorry */
  "delnphi3", /* sorry */
  "tensor gradients",
  "tensor gradients magnitudes",
  "tensor gradients magnitude magnitudes",
  "trace grad vec",
  "trace grad mag",
  "trace normal",
  "norm grad vec",
  "norm grad mag",
  "norm normal",
  "B grad vec",
  "B grad mag",
  "B normal",
  "determinant grad vec",
  "determinant grad mag",
  "determinant normal",
  "S grad vec",
  "S grad mag",
  "S normal",
  "Q grad vec",
  "Q grad mag",
  "Q normal",
  "FA grad vec",
  "FA grad mag",
  "FA normal",
  "R grad vec",
  "R grad mag",
  "R normal",
  "mode grad vec",
  "mode grad mag",
  "mode normal",
  "theta grad vec",
  "theta grad mag",
  "theta normal",
  "omega grad vec",
  "omega grad mag",
  "omega normal",
  "invariant K gradients",
  "invariant K gradient mags",
  "invariant R gradients",
  "invariant R gradient mags",
  "rotation tangents",
  "rotation tangent mags",
  "eigenvalue gradients",
  "linear anisotropy (1)",
  "planar anisotropy (1)",
  "linear+planar anisotropy (1)",
  "min(linear,planar) anisotropy (1)",
  "linear anisotropy (2)",
  "planar anisotropy (2)",
  "linear+planar anisotropy (2)",
  "min(linear,planar) anisotropy (2)",
  "hessian",
  "trace hessian",
  "trace hessian evals",
  "trace hessian eval 0",
  "trace hessian eval 1",
  "trace hessian eval 2",
  "trace hessian evecs",
  "trace hessian evec 0",
  "trace hessian evec 1",
  "trace hessian evec 2",
  "trace hessian frob",
  "B hessian",
  "det hessian",
  "S hessian",
  "Q hessian",
  "FA hessian",
  "FA hessian evals",
  "FA hessian eval 0",
  "FA hessian eval 1",
  "FA hessian eval 2",
  "FA hessian evecs",
  "FA hessian evec 0",
  "FA hessian evec 1",
  "FA hessian evec 2",
  "FA hessian frob",
  "FA ridge surface strength",
  "FA valley surface strength",
  "FA laplacian",
  "FA hessian eval mode",
  "FA ridge line alignment",
  "FA ridge surface alignment",
  "FA 2nd DD",
  "FA geometry tensor",
  "FA kappa1",
  "FA kappa2",
  "FA total curv",
  "FA shape index",
  "FA mean curv",
  "FA gauss curv",
  "FA curv dir1",
  "FA curv dir2",
  "FA flowline curv",
  "R hessian",
  "mode hessian",
  "mode hessian evals",
  "mode hessian eval 0",
  "mode hessian eval 1",
  "mode hessian eval 2",
  "mode hessian evecs",
  "mode hessian evec 0",
  "mode hessian evec 1",
  "mode hessian evec 2",
  "mode hessian frob",
  "omega hessian",
  "omega hessian evals",
  "omega hessian eval 0",
  "omega hessian eval 1",
  "omega hessian eval 2",
  "omega hessian evecs",
  "omega hessian evec 0",
  "omega hessian evec 1",
  "omega hessian evec 2",
  "omega laplacian",
  "omega 2nd DD",
  "omega hessian contracted with ten evec 0",
  "omega hessian contracted with ten evec 1",
  "omega hessian contracted with ten evec 2",
  "trace gradvec dot evec0",
  "diffusion align of trace",
  "diffusion fraction of trace",
  "FA gradvec dot evec0",
  "diffusion align of FA",
  "diffusion fraction of FA",
  "omega gradvec dot evec0",
  "diffusion align of omega",
  "diffusion fraction of omega",
  "conf gradvec dot evec0",
  "diffusion align of conf",
  "diffusion fraction of conf",
  "covariance",
  "covarianceR",
  "covarianceK",
  "log-euclidean",
  "QuatGeoLoxK",
  "QuatGeoLoxR",
  "RThetaPhiLinear interp",
  "gradient vector of cl1",
  "gradient magnitude of cl1",
  "normal of cl1",
  "gradient vector of cp1",
  "gradient magnitude of cp1",
  "normal of cp1",
  "gradient vector of ca1",
  "gradient magnitude of ca1",
  "normal of ca1",
  "all tensor component gradients, starting with confidence gradient, "
  /* !! CONCAT !! */
  "rotated such that eigenvalue derivatives are on the diagonal",
  "eigenvalue hessians",
  "cl1 hessian",
  "cl1 hessian evals",
  "cl1 hessian eval 0",
  "cl1 hessian eval 1",
  "cl1 hessian eval 2",
  "cl1 hessian evecs",
  "cl1 hessian evec 0",
  "cl1 hessian evec 1",
  "cl1 hessian evec 2",
  "cp1 hessian",
  "cp1 hessian evals",
  "cp1 hessian eval 0",
  "cp1 hessian eval 1",
  "cp1 hessian eval 2",
  "cp1 hessian evecs",
  "cp1 hessian evec 0",
  "cp1 hessian evec 1",
  "cp1 hessian evec 2",
  "ca1 hessian",
  "ca1 hessian evals",
  "ca1 hessian eval 0",
  "ca1 hessian eval 1",
  "ca1 hessian eval 2",
  "ca1 hessian evecs",
  "ca1 hessian evec 0",
  "ca1 hessian evec 1",
  "ca1 hessian evec 2",
  "fiber curving",
  "fiber dispersion",
  "anisotropies"
};

const int
_tenGageVal[] = {
  tenGageUnknown,
  tenGageTensor,        /* "t", the reconstructed tensor: GT[7] */
  tenGageConfidence,    /* "c", first of seven tensor values: GT[1] */
  tenGageTrace,         /* "tr", trace of tensor: GT[1] */
  tenGageNorm,
  tenGageB,             /* "b": GT[1] */
  tenGageDet,           /* "det", determinant of tensor: GT[1] */
  tenGageS,             /* "s", square of frobenius norm: GT[1] */
  tenGageQ,             /* "q", (S - B)/9: GT[1] */
  tenGageFA,            /* "fa", fractional anisotropy: GT[1] */
  tenGageR,             /* "r", 9*A*B - 2*A^3 - 27*C: GT[1] */
  tenGageMode,          /* "mode", sqrt(2)*R/sqrt(Q^3): GT[1] */
  tenGageTheta,         /* "th", arccos(mode/sqrt(2))/AIR_PI: GT[1] */
  tenGageModeWarp,      /* */
  tenGageOmega,         /* */
  tenGageEval,          /* "eval", all eigenvalues of tensor : GT[3] */
  tenGageEval0,         /* "eval0", major eigenvalue of tensor : GT[1] */
  tenGageEval1,         /* "eval1", medium eigenvalue of tensor : GT[1] */
  tenGageEval2,         /* "eval2", minor eigenvalue of tensor : GT[1] */
  tenGageEvec,          /* "evec", major eigenvectors of tensor: GT[9] */
  tenGageEvec0,         /* "evec0", major eigenvectors of tensor: GT[3] */
  tenGageEvec1,         /* "evec1", medium eigenvectors of tensor: GT[3] */
  tenGageEvec2,         /* "evec2", minor eigenvectors of tensor: GT[3] */
  tenGageDelNormK2,
  tenGageDelNormK3,
  tenGageDelNormR1,
  tenGageDelNormR2,
  tenGageDelNormPhi1,
  tenGageDelNormPhi2,
  tenGageDelNormPhi3,
  tenGageTensorGrad,    /* "tg", all tensor component gradients: GT[21] */
  tenGageTensorGradMag,    /* "tgm" */
  tenGageTensorGradMagMag,    /* "tgmm" */
  tenGageTraceGradVec,  /* "trgv": gradient (vector) of trace: GT[3] */
  tenGageTraceGradMag,  /* "trgm": gradient magnitude of trace: GT[1] */
  tenGageTraceNormal,   /* "trn": normal of trace: GT[3] */
  tenGageNormGradVec,
  tenGageNormGradMag,
  tenGageNormNormal,
  tenGageBGradVec,      /* "bgv", gradient (vector) of B: GT[3] */
  tenGageBGradMag,      /* "bgm", gradient magnitude of B: GT[1] */
  tenGageBNormal,       /* "bn", normal of B: GT[3] */
  tenGageDetGradVec,    /* "detgv", gradient (vector) of Det: GT[3] */
  tenGageDetGradMag,    /* "detgm", gradient magnitude of Det: GT[1] */
  tenGageDetNormal,     /* "detn", normal of Det: GT[3] */
  tenGageSGradVec,      /* "sgv", gradient (vector) of S: GT[3] */
  tenGageSGradMag,      /* "sgm", gradient magnitude of S: GT[1] */
  tenGageSNormal,       /* "sn", normal of S: GT[3] */
  tenGageQGradVec,      /* "qgv", gradient vector of Q: GT[3] */
  tenGageQGradMag,      /* "qgm", gradient magnitude of Q: GT[1] */
  tenGageQNormal,       /* "qn", normalized gradient of Q: GT[3] */
  tenGageFAGradVec,     /* "fagv", gradient vector of FA: GT[3] */
  tenGageFAGradMag,     /* "fagm", gradient magnitude of FA: GT[1] */
  tenGageFANormal,      /* "fan", normalized gradient of FA: GT[3] */
  tenGageRGradVec,      /* "rgv", gradient vector of Q: GT[3] */
  tenGageRGradMag,      /* "rgm", gradient magnitude of Q: GT[1] */
  tenGageRNormal,       /* "rn", normalized gradient of Q: GT[3] */
  tenGageModeGradVec,   /* "mgv", gradient vector of mode: GT[3] */
  tenGageModeGradMag,   /* "mgm", gradient magnitude of mode: GT[1] */
  tenGageModeNormal,    /* "mn", normalized gradient of moe: GT[3] */
  tenGageThetaGradVec,  /* "thgv", gradient vector of theta: GT[3] */
  tenGageThetaGradMag,  /* "thgm", gradient magnitude of theta: GT[1] */
  tenGageThetaNormal,   /* "thn", normalized gradient of theta: GT[3] */
  tenGageOmegaGradVec,  /* */
  tenGageOmegaGradMag,  /* */
  tenGageOmegaNormal,   /* */
  tenGageInvarKGrads,
  tenGageInvarKGradMags,
  tenGageInvarRGrads,
  tenGageInvarRGradMags,
  tenGageRotTans,       /* "rts" */
  tenGageRotTanMags,    /* "rtms" */
  tenGageEvalGrads,     /* "evgs" */
  tenGageCl1,
  tenGageCp1,
  tenGageCa1,
  tenGageClpmin1,
  tenGageCl2,
  tenGageCp2,
  tenGageCa2,
  tenGageClpmin2,
  tenGageHessian,
  tenGageTraceHessian,
  tenGageTraceHessianEval,
  tenGageTraceHessianEval0,
  tenGageTraceHessianEval1,
  tenGageTraceHessianEval2,
  tenGageTraceHessianEvec,
  tenGageTraceHessianEvec0,
  tenGageTraceHessianEvec1,
  tenGageTraceHessianEvec2,
  tenGageTraceHessianFrob,
  tenGageBHessian,
  tenGageDetHessian,
  tenGageSHessian,
  tenGageQHessian,
  tenGageFAHessian,
  tenGageFAHessianEval,
  tenGageFAHessianEval0,
  tenGageFAHessianEval1,
  tenGageFAHessianEval2,
  tenGageFAHessianEvec,
  tenGageFAHessianEvec0,
  tenGageFAHessianEvec1,
  tenGageFAHessianEvec2,
  tenGageFAHessianFrob,
  tenGageFARidgeSurfaceStrength,
  tenGageFAValleySurfaceStrength,
  tenGageFALaplacian,
  tenGageFAHessianEvalMode,
  tenGageFARidgeLineAlignment,
  tenGageFARidgeSurfaceAlignment,
  tenGageFA2ndDD,
  tenGageFAGeomTens,
  tenGageFAKappa1,
  tenGageFAKappa2,
  tenGageFATotalCurv,
  tenGageFAShapeIndex,
  tenGageFAMeanCurv,
  tenGageFAGaussCurv,
  tenGageFACurvDir1,
  tenGageFACurvDir2,
  tenGageFAFlowlineCurv,
  tenGageRHessian,
  tenGageModeHessian,
  tenGageModeHessianEval,
  tenGageModeHessianEval0,
  tenGageModeHessianEval1,
  tenGageModeHessianEval2,
  tenGageModeHessianEvec,
  tenGageModeHessianEvec0,
  tenGageModeHessianEvec1,
  tenGageModeHessianEvec2,
  tenGageModeHessianFrob,
  tenGageOmegaHessian,
  tenGageOmegaHessianEval,
  tenGageOmegaHessianEval0,
  tenGageOmegaHessianEval1,
  tenGageOmegaHessianEval2,
  tenGageOmegaHessianEvec,
  tenGageOmegaHessianEvec0,
  tenGageOmegaHessianEvec1,
  tenGageOmegaHessianEvec2,
  tenGageOmegaLaplacian,
  tenGageOmega2ndDD,
  tenGageOmegaHessianContrTenEvec0,
  tenGageOmegaHessianContrTenEvec1,
  tenGageOmegaHessianContrTenEvec2,
  tenGageTraceGradVecDotEvec0,
  tenGageTraceDiffusionAlign,
  tenGageTraceDiffusionFraction,
  tenGageFAGradVecDotEvec0,
  tenGageFADiffusionAlign,
  tenGageFADiffusionFraction,
  tenGageOmegaGradVecDotEvec0,
  tenGageOmegaDiffusionAlign,
  tenGageOmegaDiffusionFraction,
  tenGageConfGradVecDotEvec0,
  tenGageConfDiffusionAlign,
  tenGageConfDiffusionFraction,
  tenGageCovariance,
  tenGageCovarianceRGRT,
  tenGageCovarianceKGRT,
  tenGageTensorLogEuclidean,
  tenGageTensorQuatGeoLoxK,
  tenGageTensorQuatGeoLoxR,
  tenGageTensorRThetaPhiLinear,
  tenGageCl1GradVec,
  tenGageCl1GradMag,
  tenGageCl1Normal,
  tenGageCp1GradVec,
  tenGageCp1GradMag,
  tenGageCp1Normal,
  tenGageCa1GradVec,
  tenGageCa1GradMag,
  tenGageCa1Normal,
  tenGageTensorGradRotE,
  tenGageEvalHessian,      /* Hessian of the eigenvalues: [27] */
  tenGageCl1Hessian,       /* Hessian of cl1: [9] */
  tenGageCl1HessianEval,   /* Hessian eigenvalues of cl1: [3] */
  tenGageCl1HessianEval0,  /* First Hessian eigenvalue of cl1: [1] */
  tenGageCl1HessianEval1,  /* Second Hessian eigenvalue of cl1: [1] */
  tenGageCl1HessianEval2,  /* Third Hessian eigenvalue of cl1: [1] */
  tenGageCl1HessianEvec,   /* Hessian eigenvectors of cl1: [9] */
  tenGageCl1HessianEvec0,  /* First Hessian eigenvector of cl1: [3] */
  tenGageCl1HessianEvec1,  /* Second Hessian eigenvector of cl1: [3] */
  tenGageCl1HessianEvec2,  /* Third Hessian eigenvector of cl1: [3] */
  tenGageCp1Hessian,       /* Hessian of cp1: [9] */
  tenGageCp1HessianEval,   /* Hessian eigenvalues of cp1: [3] */
  tenGageCp1HessianEval0,  /* First Hessian eigenvalue of cp1: [1] */
  tenGageCp1HessianEval1,  /* Second Hessian eigenvalue of cp1: [1] */
  tenGageCp1HessianEval2,  /* Third Hessian eigenvalue of cp1: [1] */
  tenGageCp1HessianEvec,   /* Hessian eigenvectors of cp1: [9] */
  tenGageCp1HessianEvec0,  /* First Hessian eigenvector of cp1: [3] */
  tenGageCp1HessianEvec1,  /* Second Hessian eigenvector of cp1: [3] */
  tenGageCp1HessianEvec2,  /* Third Hessian eigenvector of cp1: [3] */
  tenGageCa1Hessian,       /* Hessian of cp1: [9] */
  tenGageCa1HessianEval,   /* Hessian eigenvalues of cp1: [3] */
  tenGageCa1HessianEval0,  /* First Hessian eigenvalue of cp1: [1] */
  tenGageCa1HessianEval1,  /* Second Hessian eigenvalue of cp1: [1] */
  tenGageCa1HessianEval2,  /* Third Hessian eigenvalue of cp1: [1] */
  tenGageCa1HessianEvec,   /* Hessian eigenvectors of cp1: [9] */
  tenGageCa1HessianEvec0,  /* First Hessian eigenvector of cp1: [3] */
  tenGageCa1HessianEvec1,  /* Second Hessian eigenvector of cp1: [3] */
  tenGageCa1HessianEvec2,   /* Third Hessian eigenvector of cp1: [3] */
  tenGageFiberCurving,
  tenGageFiberDispersion,
  tenGageAniso,
};

const char *
_tenGageStrEqv[] = {
  "t", "ten", "tensor",
  "c", "conf", "confidence",
  "tr", "trace",
  "n", "norm", "r1",
  "b",
  "det",
  "s",
  "q",
  "fa",
  "r",
  "mode", "m",
  "th", "theta",
  "modew", "mw",
  "omega", "om",
  "eval", "evals",
  "eval0",
  "eval1",
  "eval2",
  "evec", "evecs",
  "evec0",
  "evec1",
  "evec2",
  "delnk2",
  "delnk3", "delnr3",
  "delnr1",
  "delnr2",
  "delnphi1",
  "delnphi2",
  "delnphi3",
  "tg", "tensor grad",
  "tgm", "tensor grad mag",
  "tgmm", "tensor grad mag mag",

  "trgv", "tracegv", "trace grad vec",
  "trgm", "tracegm", "trace grad mag",
  "trn", "tracen", "trace normal",

  "ngv", "r1gv", "normgv", "norm grad vec",
  "ngm", "r1gm", "normgm", "norm grad mag",
  "nn", "r1n", "normn", "norm normal",

  "bgv", "b grad vec",
  "bgm", "b grad mag",
  "bn", "b normal",

  "detgv", "det grad vec",
  "detgm", "det grad mag",
  "detn", "det normal",
  "sgv", "s grad vec",
  "sgm", "s grad mag",
  "sn", "s normal",
  "qgv", "q grad vec",
  "qgm", "q grad mag",
  "qn", "q normal",
  "fagv", "fa grad vec",
  "fagm", "fa grad mag",
  "fan", "fa normal",
  "rgv", "r grad vec",
  "rgm", "r grad mag",
  "rn", "r normal",
  "mgv", "mode grad vec",
  "mgm", "mode grad mag",
  "mn", "mode normal",
  "thgv", "th grad vec", "theta grad vec",
  "thgm", "th grad mag", "theta grad mag",
  "thn", "th normal", "theta normal",
  "omgv", "omega grad vec",
  "omgm", "omega grad mag",
  "omn", "omega normal",
  "ikgs", "invariant K gradients",
  "ikgms", "invariant K gradient mags",
  "irgs", "invariant R gradients",
  "irgms", "invariant R gradient mags",
  "rts", "rotation tangents",
  "rtms", "rotation tangent mags",
  "evgs", "eigenvalue gradients",
  "cl1",
  "cp1",
  "ca1",
  "clpmin1",
  "cl2",
  "cp2",
  "ca2",
  "clpmin2",
  "hess", "hessian",
  "trhess", "trace hessian",
  "trhesseval", "trace hessian evals",
  "trhesseval0", "trace hessian eval 0",
  "trhesseval1", "trace hessian eval 1",
  "trhesseval2", "trace hessian eval 2",
  "trhessevec", "trace hessian evecs",
  "trhessevec0", "trace hessian evec 0",
  "trhessevec1", "trace hessian evec 1",
  "trhessevec2", "trace hessian evec 2",
  "trhessfrob", "trace hessian frob",
  "bhess", "B hessian",
  "dethess", "det hessian",
  "shess", "S hessian",
  "qhess", "Q hessian",
  "fahess", "FA hessian",
  "fahesseval", "FA hessian evals",
  "fahesseval0", "FA hessian eval 0",
  "fahesseval1", "FA hessian eval 1",
  "fahesseval2", "FA hessian eval 2",
  "fahessevec", "FA hessian evecs",
  "fahessevec0", "FA hessian evec 0",
  "fahessevec1", "FA hessian evec 1",
  "fahessevec2", "FA hessian evec 2",
  "fahessfrob", "FA hessian frob",
  "farsurfstrn", "FA ridge surface strength",
  "favsurfstrn", "FA valley surface strength",
  "falapl", "FA laplacian",
  "fahessevalmode", "FA hessian eval mode",
  "farlinealn", "FA ridge line alignment",
  "farsurfaln", "FA ridge surface alignment",
  "fa2d", "fa2dd", "FA 2nd DD",
  "fagten", "FA geometry tensor",
  "fak1", "FA kappa1",
  "fak2", "FA kappa2",
  "fatc", "FA total curv",
  "fasi", "FA shape index",
  "famc", "FA mean curv",
  "fagc", "FA gauss curv",
  "facdir1", "FA curv dir1",
  "facdir2", "FA curv dir2",
  "fafc", "FA flowline curv",
  "rhess", "R hessian",
  "mhess", "mode hessian",
  "mhesseval", "mode hessian evals",
  "mhesseval0", "mode hessian eval 0",
  "mhesseval1", "mode hessian eval 1",
  "mhesseval2", "mode hessian eval 2",
  "mhessevec", "mode hessian evecs",
  "mhessevec0", "mode hessian evec 0",
  "mhessevec1", "mode hessian evec 1",
  "mhessevec2", "mode hessian evec 2",
  "mhessfrob", "mode hessian frob",
  "omhess", "omega hessian",
  "omhesseval", "omega hessian evals",
  "omhesseval0", "omega hessian eval 0",
  "omhesseval1", "omega hessian eval 1",
  "omhesseval2", "omega hessian eval 2",
  "omhessevec", "omega hessian evecs",
  "omhessevec0", "omega hessian evec 0",
  "omhessevec1", "omega hessian evec 1",
  "omhessevec2", "omega hessian evec 2",
  "omlapl", "omega laplacian",
  "om2d", "om2dd", "omega 2nd DD",
  "omhesscte0", "omega hessian contracted with ten evec 0",
  "omhesscte1", "omega hessian contracted with ten evec 1",
  "omhesscte2", "omega hessian contracted with ten evec 2",
  "trgvdotevec0", "trace gradvec dot evec0",
  "datr", "diffusionAlign(trace)",
  "dftr", "diffusionFraction(trace)",
  "fagvdotevec0", "FA gradvec dot evec0",
  "dafa", "diffusionAlign(FA)",
  "dffa", "diffusionFraction(FA)",
  "omgvdotevec0", "omega gradvec dot evec0",
  "daom", "diffusionAlign(Omega)",
  "dfom", "diffusionFraction(Omega)",
  "confgvdotevec0", "conf gradvec dot evec0",
  "daconf", "diffusionAlign(Conf)",
  "dfconf", "diffusionFraction(Conf)",
  "cov",
  "covr",
  "covk",
  "logeuclidean", "logeuc", "logeuclid",
  "quatgeoloxk", "qglk",
  "quatgeoloxr", "qglr",
  "rtpl",

  "cl1gv",
  "cl1gm",
  "cl1gn",
  "cp1gv",
  "cp1gm",
  "cp1gn",
  "ca1gv",
  "ca1gm",
  "ca1gn",
  "tgrote",
  "evalhess", "eval hessian",
  "cl1hess",      "cl1 hessian",
  "cl1hesseval",  "cl1 hessian evals",
  "cl1hesseval0", "cl1 hessian eval 0",
  "cl1hesseval1", "cl1 hessian eval 1",
  "cl1hesseval2", "cl1 hessian eval 2",
  "cl1hessevec",  "cl1 hessian evecs",
  "cl1hessevec0", "cl1 hessian evec 0",
  "cl1hessevec1", "cl1 hessian evec 1",
  "cl1hessevec2", "cl1 hessian evec 2",
  "cp1hess",      "cp1 hessian",
  "cp1hesseval",  "cp1 hessian evals",
  "cp1hesseval0", "cp1 hessian eval 0",
  "cp1hesseval1", "cp1 hessian eval 1",
  "cp1hesseval2", "cp1 hessian eval 2",
  "cp1hessevec",  "cp1 hessian evecs",
  "cp1hessevec0", "cp1 hessian evec 0",
  "cp1hessevec1", "cp1 hessian evec 1",
  "cp1hessevec2", "cp1 hessian evec 2",
  "ca1hess",      "ca1 hessian",
  "ca1hesseval",  "ca1 hessian evals",
  "ca1hesseval0", "ca1 hessian eval 0",
  "ca1hesseval1", "ca1 hessian eval 1",
  "ca1hesseval2", "ca1 hessian eval 2",
  "ca1hessevec",  "ca1 hessian evecs",
  "ca1hessevec0", "ca1 hessian evec 0",
  "ca1hessevec1", "ca1 hessian evec 1",
  "ca1hessevec2", "ca1 hessian evec 2",

  "fcurv", "fibcurv", "fiber curving", "fiber curving index",
  "fdisp", "fibdisp", "fiber dispersion", "fiber dispersion index",

  "an", "aniso", "anisotropies",
  ""
};

const int
_tenGageValEqv[] = {
  tenGageTensor, tenGageTensor, tenGageTensor,
  tenGageConfidence, tenGageConfidence, tenGageConfidence,
  tenGageTrace, tenGageTrace,
  tenGageNorm, tenGageNorm, tenGageNorm,
  tenGageB,
  tenGageDet,
  tenGageS,
  tenGageQ,
  tenGageFA,
  tenGageR,
  tenGageMode, tenGageMode,
  tenGageTheta, tenGageTheta,
  tenGageModeWarp, tenGageModeWarp,
  tenGageOmega, tenGageOmega,
  tenGageEval, tenGageEval,
  tenGageEval0,
  tenGageEval1,
  tenGageEval2,
  tenGageEvec, tenGageEvec,
  tenGageEvec0,
  tenGageEvec1,
  tenGageEvec2,
  tenGageDelNormK2,
  tenGageDelNormK3, tenGageDelNormK3,
  tenGageDelNormR1,
  tenGageDelNormR2,
  tenGageDelNormPhi1,
  tenGageDelNormPhi2,
  tenGageDelNormPhi3,
  tenGageTensorGrad, tenGageTensorGrad,
  tenGageTensorGradMag, tenGageTensorGradMag,
  tenGageTensorGradMagMag, tenGageTensorGradMagMag,

  tenGageTraceGradVec, tenGageTraceGradVec, tenGageTraceGradVec,
  tenGageTraceGradMag, tenGageTraceGradMag, tenGageTraceGradMag,
  tenGageTraceNormal, tenGageTraceNormal, tenGageTraceNormal,

  tenGageNormGradVec, tenGageNormGradVec, tenGageNormGradVec, tenGageNormGradVec,
  tenGageNormGradMag, tenGageNormGradMag, tenGageNormGradMag, tenGageNormGradMag,
  tenGageNormNormal, tenGageNormNormal, tenGageNormNormal, tenGageNormNormal,

  tenGageBGradVec, tenGageBGradVec,
  tenGageBGradMag, tenGageBGradMag,
  tenGageBNormal, tenGageBNormal,
  tenGageDetGradVec, tenGageDetGradVec,
  tenGageDetGradMag, tenGageDetGradMag,
  tenGageDetNormal, tenGageDetNormal,
  tenGageSGradVec, tenGageSGradVec,
  tenGageSGradMag, tenGageSGradMag,
  tenGageSNormal, tenGageSNormal,
  tenGageQGradVec, tenGageQGradVec,
  tenGageQGradMag, tenGageQGradMag,
  tenGageQNormal, tenGageQNormal,
  tenGageFAGradVec, tenGageFAGradVec,
  tenGageFAGradMag, tenGageFAGradMag,
  tenGageFANormal, tenGageFANormal,
  tenGageRGradVec, tenGageRGradVec,
  tenGageRGradMag, tenGageRGradMag,
  tenGageRNormal, tenGageRNormal,
  tenGageModeGradVec, tenGageModeGradVec,
  tenGageModeGradMag, tenGageModeGradMag,
  tenGageModeNormal, tenGageModeNormal,
  tenGageThetaGradVec, tenGageThetaGradVec, tenGageThetaGradVec,
  tenGageThetaGradMag, tenGageThetaGradMag, tenGageThetaGradMag,
  tenGageThetaNormal, tenGageThetaNormal, tenGageThetaNormal,
  tenGageOmegaGradVec, tenGageOmegaGradVec,
  tenGageOmegaGradMag, tenGageOmegaGradMag,
  tenGageOmegaNormal, tenGageOmegaNormal,
  tenGageInvarKGrads, tenGageInvarKGrads,
  tenGageInvarKGradMags, tenGageInvarKGradMags,
  tenGageInvarRGrads, tenGageInvarRGrads,
  tenGageInvarRGradMags, tenGageInvarRGradMags,
  tenGageRotTans, tenGageRotTans,
  tenGageRotTanMags, tenGageRotTanMags,
  tenGageEvalGrads, tenGageEvalGrads,
  tenGageCl1,
  tenGageCp1,
  tenGageCa1,
  tenGageClpmin1,
  tenGageCl2,
  tenGageCp2,
  tenGageCa2,
  tenGageClpmin2,
  tenGageHessian, tenGageHessian,
  tenGageTraceHessian, tenGageTraceHessian,
  tenGageTraceHessianEval, tenGageTraceHessianEval,
  tenGageTraceHessianEval0, tenGageTraceHessianEval0,
  tenGageTraceHessianEval1, tenGageTraceHessianEval1,
  tenGageTraceHessianEval2, tenGageTraceHessianEval2,
  tenGageTraceHessianEvec, tenGageTraceHessianEvec,
  tenGageTraceHessianEvec0, tenGageTraceHessianEvec0,
  tenGageTraceHessianEvec1, tenGageTraceHessianEvec1,
  tenGageTraceHessianEvec2, tenGageTraceHessianEvec2,
  tenGageTraceHessianFrob, tenGageTraceHessianFrob,
  tenGageBHessian, tenGageBHessian,
  tenGageDetHessian, tenGageDetHessian,
  tenGageSHessian, tenGageSHessian,
  tenGageQHessian, tenGageQHessian,
  tenGageFAHessian, tenGageFAHessian,
  tenGageFAHessianEval, tenGageFAHessianEval,
  tenGageFAHessianEval0, tenGageFAHessianEval0,
  tenGageFAHessianEval1, tenGageFAHessianEval1,
  tenGageFAHessianEval2, tenGageFAHessianEval2,
  tenGageFAHessianEvec, tenGageFAHessianEvec,
  tenGageFAHessianEvec0, tenGageFAHessianEvec0,
  tenGageFAHessianEvec1, tenGageFAHessianEvec1,
  tenGageFAHessianEvec2, tenGageFAHessianEvec2,
  tenGageFAHessianFrob, tenGageFAHessianFrob,
  tenGageFARidgeSurfaceStrength, tenGageFARidgeSurfaceStrength,
  tenGageFAValleySurfaceStrength, tenGageFAValleySurfaceStrength,
  tenGageFALaplacian, tenGageFALaplacian,
  tenGageFAHessianEvalMode, tenGageFAHessianEvalMode,
  tenGageFARidgeLineAlignment, tenGageFARidgeLineAlignment,
  tenGageFARidgeSurfaceAlignment, tenGageFARidgeSurfaceAlignment,
  tenGageFA2ndDD, tenGageFA2ndDD, tenGageFA2ndDD,
  tenGageFAGeomTens, tenGageFAGeomTens,
  tenGageFAKappa1, tenGageFAKappa1,
  tenGageFAKappa2, tenGageFAKappa2,
  tenGageFATotalCurv, tenGageFATotalCurv,
  tenGageFAShapeIndex, tenGageFAShapeIndex,
  tenGageFAMeanCurv, tenGageFAMeanCurv,
  tenGageFAGaussCurv, tenGageFAGaussCurv,
  tenGageFACurvDir1, tenGageFACurvDir1,
  tenGageFACurvDir2, tenGageFACurvDir2,
  tenGageFAFlowlineCurv, tenGageFAFlowlineCurv,
  tenGageRHessian, tenGageRHessian,
  tenGageModeHessian, tenGageModeHessian,
  tenGageModeHessianEval, tenGageModeHessianEval,
  tenGageModeHessianEval0, tenGageModeHessianEval0,
  tenGageModeHessianEval1, tenGageModeHessianEval1,
  tenGageModeHessianEval2, tenGageModeHessianEval2,
  tenGageModeHessianEvec, tenGageModeHessianEvec,
  tenGageModeHessianEvec0, tenGageModeHessianEvec0,
  tenGageModeHessianEvec1, tenGageModeHessianEvec1,
  tenGageModeHessianEvec2, tenGageModeHessianEvec2,
  tenGageModeHessianFrob, tenGageModeHessianFrob,
  tenGageOmegaHessian, tenGageOmegaHessian,
  tenGageOmegaHessianEval, tenGageOmegaHessianEval,
  tenGageOmegaHessianEval0, tenGageOmegaHessianEval0,
  tenGageOmegaHessianEval1, tenGageOmegaHessianEval1,
  tenGageOmegaHessianEval2, tenGageOmegaHessianEval2,
  tenGageOmegaHessianEvec, tenGageOmegaHessianEvec,
  tenGageOmegaHessianEvec0, tenGageOmegaHessianEvec0,
  tenGageOmegaHessianEvec1, tenGageOmegaHessianEvec1,
  tenGageOmegaHessianEvec2, tenGageOmegaHessianEvec2,
  tenGageOmegaLaplacian, tenGageOmegaLaplacian,
  tenGageOmega2ndDD, tenGageOmega2ndDD, tenGageOmega2ndDD,
  tenGageOmegaHessianContrTenEvec0, tenGageOmegaHessianContrTenEvec0,
  tenGageOmegaHessianContrTenEvec1, tenGageOmegaHessianContrTenEvec1,
  tenGageOmegaHessianContrTenEvec2, tenGageOmegaHessianContrTenEvec2,

  tenGageTraceGradVecDotEvec0, tenGageTraceGradVecDotEvec0,
  tenGageTraceDiffusionAlign, tenGageTraceDiffusionAlign,
  tenGageTraceDiffusionFraction, tenGageTraceDiffusionFraction,

  tenGageFAGradVecDotEvec0, tenGageFAGradVecDotEvec0,
  tenGageFADiffusionAlign, tenGageFADiffusionAlign,
  tenGageFADiffusionFraction, tenGageFADiffusionFraction,

  tenGageOmegaGradVecDotEvec0, tenGageOmegaGradVecDotEvec0,
  tenGageOmegaDiffusionAlign, tenGageOmegaDiffusionAlign,
  tenGageOmegaDiffusionFraction, tenGageOmegaDiffusionFraction,

  tenGageConfGradVecDotEvec0, tenGageConfGradVecDotEvec0,
  tenGageConfDiffusionAlign, tenGageConfDiffusionAlign,
  tenGageConfDiffusionFraction, tenGageConfDiffusionFraction,

  tenGageCovariance,
  tenGageCovarianceRGRT,
  tenGageCovarianceKGRT,

  tenGageTensorLogEuclidean, tenGageTensorLogEuclidean, tenGageTensorLogEuclidean,
  tenGageTensorQuatGeoLoxK, tenGageTensorQuatGeoLoxK,
  tenGageTensorQuatGeoLoxR, tenGageTensorQuatGeoLoxR,
  tenGageTensorRThetaPhiLinear,

  tenGageCl1GradVec,
  tenGageCl1GradMag,
  tenGageCl1Normal,
  tenGageCp1GradVec,
  tenGageCp1GradMag,
  tenGageCp1Normal,
  tenGageCa1GradVec,
  tenGageCa1GradMag,
  tenGageCa1Normal,
  tenGageTensorGradRotE,
  tenGageEvalHessian, tenGageEvalHessian,
  tenGageCl1Hessian, tenGageCl1Hessian,
  tenGageCl1HessianEval, tenGageCl1HessianEval,
  tenGageCl1HessianEval0, tenGageCl1HessianEval0,
  tenGageCl1HessianEval1, tenGageCl1HessianEval1,
  tenGageCl1HessianEval2, tenGageCl1HessianEval2,
  tenGageCl1HessianEvec, tenGageCl1HessianEvec,
  tenGageCl1HessianEvec0, tenGageCl1HessianEvec0,
  tenGageCl1HessianEvec1, tenGageCl1HessianEvec1,
  tenGageCl1HessianEvec2, tenGageCl1HessianEvec2,
  tenGageCp1Hessian, tenGageCp1Hessian,
  tenGageCp1HessianEval, tenGageCp1HessianEval,
  tenGageCp1HessianEval0, tenGageCp1HessianEval0,
  tenGageCp1HessianEval1, tenGageCp1HessianEval1,
  tenGageCp1HessianEval2, tenGageCp1HessianEval2,
  tenGageCp1HessianEvec, tenGageCp1HessianEvec,
  tenGageCp1HessianEvec0, tenGageCp1HessianEvec0,
  tenGageCp1HessianEvec1, tenGageCp1HessianEvec1,
  tenGageCp1HessianEvec2, tenGageCp1HessianEvec2,
  tenGageCa1Hessian, tenGageCa1Hessian,
  tenGageCa1HessianEval, tenGageCa1HessianEval,
  tenGageCa1HessianEval0, tenGageCa1HessianEval0,
  tenGageCa1HessianEval1, tenGageCa1HessianEval1,
  tenGageCa1HessianEval2, tenGageCa1HessianEval2,
  tenGageCa1HessianEvec, tenGageCa1HessianEvec,
  tenGageCa1HessianEvec0, tenGageCa1HessianEvec0,
  tenGageCa1HessianEvec1, tenGageCa1HessianEvec1,
  tenGageCa1HessianEvec2, tenGageCa1HessianEvec2,

  tenGageFiberCurving, tenGageFiberCurving, tenGageFiberCurving, tenGageFiberCurving,
  tenGageFiberDispersion, tenGageFiberDispersion, tenGageFiberDispersion, tenGageFiberDispersion,

  tenGageAniso, tenGageAniso, tenGageAniso
};

const airEnum
_tenGage = {
  "tenGage",
  TEN_GAGE_ITEM_MAX,
  _tenGageStr, _tenGageVal,
  _tenGageDesc,
  _tenGageStrEqv, _tenGageValEqv,
  AIR_FALSE
};
const airEnum *const
tenGage = &_tenGage;

/* --------------------------------------------------------------------- */

const char *
_tenFiberTypeStr[] = {
  "(unknown tenFiberType)",
  "evec0",
  "evec1",
  "evec2",
  "tensorline",
  "pureline",
  "zhukov"
};

const char *
_tenFiberTypeDesc[] = {
  "unknown tenFiber type",
  "simply follow principal eigenvector",
  "follow medium eigenvector",
  "follow minor eigenvector",
  "Weinstein-Kindlmann tensorlines",
  "based on tensor multiplication only",
  "Zhukov\'s oriented tensors"
};

const char *
_tenFiberTypeStrEqv[] = {
  "ev0", "evec0",
  "ev1", "evec1",
  "ev2", "evec2",
  "tline", "tensorline",
  "pline", "pureline",
  "z", "zhukov",
  ""
};

const int
_tenFiberTypeValEqv[] = {
  tenFiberTypeEvec0, tenFiberTypeEvec0,
  tenFiberTypeEvec1, tenFiberTypeEvec1,
  tenFiberTypeEvec2, tenFiberTypeEvec2,
  tenFiberTypeTensorLine, tenFiberTypeTensorLine,
  tenFiberTypePureLine, tenFiberTypePureLine,
  tenFiberTypeZhukov, tenFiberTypeZhukov
};

const airEnum
_tenFiberType = {
  "tenFiberType",
  TEN_FIBER_TYPE_MAX,
  _tenFiberTypeStr, NULL,
  _tenFiberTypeDesc,
  _tenFiberTypeStrEqv, _tenFiberTypeValEqv,
  AIR_FALSE
};
const airEnum *const
tenFiberType = &_tenFiberType;

/* --------------------------------------------------------------------- */

const char *
_tenDwiFiberTypeStr[] = {
  "(unknown tenDwiFiberType)",
  "1evec0",
  "2evec0",
  "12BlendEvec0"
};

const char *
_tenDwiFiberTypeDesc[] = {
  "unknown tenDwiFiber type",
  "single tensor evec0-based",
  "two-tensor evec0-based",
  "parameterized blend between 1- and 2-tensor fits"
};

const char *
_tenDwiFiberTypeStrEqv[] = {
  "1evec0", "1e0",
  "2evec0", "2e0",
  "12BlendEvec0", "12be0",
  ""
};

const int
_tenDwiFiberTypeValEqv[] = {
  tenDwiFiberType1Evec0, tenDwiFiberType1Evec0,
  tenDwiFiberType2Evec0, tenDwiFiberType2Evec0,
  tenDwiFiberType12BlendEvec0, tenDwiFiberType12BlendEvec0
};

const airEnum
_tenDwiFiberType = {
  "tenDwiFiberType",
  TEN_DWI_FIBER_TYPE_MAX,
  _tenDwiFiberTypeStr, NULL,
  _tenDwiFiberTypeDesc,
  _tenDwiFiberTypeStrEqv, _tenDwiFiberTypeValEqv,
  AIR_FALSE
};
const airEnum *const
tenDwiFiberType = &_tenDwiFiberType;

/* ----------------------------------------------------------------------- */

const char *
_tenFiberStopStr[] = {
  "(unknown tenFiberStop)",
  "aniso",
  "length",
  "steps",
  "confidence",
  "radius",
  "bounds",
  "fraction",
  "stub",
  "minlen",
  "minsteps",
};

const char *
_tenFiberStopStrEqv[] = {
  "aniso",
  "length", "len",
  "steps",
  "confidence", "conf", "c",
  "radius",
  "bounds",
  "fraction", "frac", "f",
  "stub",
  "minlen", "minlength",
  "minsteps", "minnumsteps",
  ""
};

const int
_tenFiberStopValEqv[] = {
  tenFiberStopAniso,
  tenFiberStopLength, tenFiberStopLength,
  tenFiberStopNumSteps,
  tenFiberStopConfidence, tenFiberStopConfidence, tenFiberStopConfidence,
  tenFiberStopRadius,
  tenFiberStopBounds,
  tenFiberStopFraction, tenFiberStopFraction, tenFiberStopFraction,
  tenFiberStopStub,
  tenFiberStopMinLength, tenFiberStopMinLength,
  tenFiberStopMinNumSteps, tenFiberStopMinNumSteps,
};

const char *
_tenFiberStopDesc[] = {
  "unknown tenFiber stop",
  "anisotropy went below threshold",
  "fiber length exceeded normalcy bounds",
  "number of steps along fiber too many",
  "tensor \"confidence\" value too low",
  "radius of curvature of path got too small",
  "fiber went outside bounding box",
  "fractional constituency of tracked tensor got too small",
  "neither fiber half got anywhere; fiber has single vert",
  "whole fiber has insufficient length",
  "whole fiber has too few numbers of steps"
};

const airEnum
_tenFiberStop = {
  "fiber stopping criteria",
  TEN_FIBER_STOP_MAX,
  _tenFiberStopStr, NULL,
  _tenFiberStopDesc,
  _tenFiberStopStrEqv, _tenFiberStopValEqv,
  AIR_FALSE
};
const airEnum *const
tenFiberStop = &_tenFiberStop;

/* ----------------------------------------------------------------------- */

const char *
_tenFiberIntgStr[] = {
  "(unknown tenFiberIntg)",
  "euler",
  "midpoint",
  "rk4"
};

const char *
_tenFiberIntgStrEqv[] = {
  "euler",
  "midpoint", "rk2",
  "rk4",
  ""
};

const int
_tenFiberIntgValEqv[] = {
  tenFiberIntgEuler,
  tenFiberIntgMidpoint, tenFiberIntgMidpoint,
  tenFiberIntgRK4
};

const char *
_tenFiberIntgDesc[] = {
  "unknown tenFiber intg",
  "plain Euler",
  "midpoint method, 2nd order Runge-Kutta",
  "4rth order Runge-Kutta"
};

const airEnum
_tenFiberIntg = {
  "fiber integration method",
  TEN_FIBER_INTG_MAX,
  _tenFiberIntgStr, NULL,
  _tenFiberIntgDesc,
  _tenFiberIntgStrEqv, _tenFiberIntgValEqv,
  AIR_FALSE
};
const airEnum *const
tenFiberIntg = &_tenFiberIntg;

/* ----------------------------------------------------------------------- */

const char *
_tenGlyphTypeStr[] = {
  "(unknown tenGlyphType)",
  "box",
  "sphere",
  "cylinder",
  "superquad",
  "betterquad",
  "polarplot"
};

#define BOX tenGlyphTypeBox
#define SPH tenGlyphTypeSphere
#define CYL tenGlyphTypeCylinder
#define SQD tenGlyphTypeSuperquad

const char *
_tenGlyphTypeStrEqv[] = {
  "b", "box",
  "s", "sph", "sphere",
  "c", "cyl", "cylind", "cylinder",
  "q", "superq", "sqd", "superquad", "superquadric",
  "bqd", "betterquad",
  "pplot", "polarplot",
  ""
};

const int
_tenGlyphTypeValEqv[] = {
  BOX, BOX,
  SPH, SPH, SPH,
  CYL, CYL, CYL, CYL,
  SQD, SQD, SQD, SQD, SQD,
  tenGlyphTypeBetterquad, tenGlyphTypeBetterquad,
  tenGlyphTypePolarPlot, tenGlyphTypePolarPlot
};

const char *
_tenGlyphTypeDesc[] = {
  "unknown tenGlyph type",
  "box/cube (rectangular prisms)",
  "sphere (ellipsoids)",
  "cylinders aligned along major eigenvector",
  "superquadric (superellipsoids)",
  "better superquadric",
  "polar plot",
};

const airEnum
_tenGlyphType = {
  "tenGlyphType",
  TEN_GLYPH_TYPE_MAX,
  _tenGlyphTypeStr, NULL,
  _tenGlyphTypeDesc,
  _tenGlyphTypeStrEqv, _tenGlyphTypeValEqv,
  AIR_FALSE
};
const airEnum *const
tenGlyphType = &_tenGlyphType;

/* ---------------------------------------------- */

const char *
_tenEstimate1MethodStr[] = {
  "(unknown tenEstimate1Method)",
  "LLS",
  "WLS",
  "NLS",
  "MLE"
};

const char *
_tenEstimate1MethodDesc[] = {
  "unknown tenEstimate1Method",
  "linear least-squares fit of log(DWI)",
  "weighted least-squares fit of log(DWI)",
  "non-linear least-squares fit of DWI",
  "maximum likelihood estimate from DWI"
};

const airEnum
_tenEstimate1Method = {
  "single-tensor-estimation",
  TEN_ESTIMATE_1_METHOD_MAX,
  _tenEstimate1MethodStr, NULL,
  _tenEstimate1MethodDesc,
  NULL, NULL,
  AIR_FALSE
};
const airEnum *const
tenEstimate1Method= &_tenEstimate1Method;

/* ---------------------------------------------- */

const char *
_tenEstimate2MethodStr[] = {
  "(unknown tenEstimate2Method)",
  "QSegLLS",
  "Peled"
};

const char *
_tenEstimate2MethodDesc[] = {
  "unknown tenEstimate2Method",
  "Q-ball segmentation",
  "Peled"
};

const airEnum
_tenEstimate2Method = {
  "two-tensor-estimation",
  TEN_ESTIMATE_2_METHOD_MAX,
  _tenEstimate2MethodStr, NULL,
  _tenEstimate2MethodDesc,
  NULL, NULL,
  AIR_FALSE
};
const airEnum *const
tenEstimate2Method= &_tenEstimate2Method;

/* ---------------------------------------------- */

const char *
_tenTripleTypeStr[] = {
  "(unknown tenTriple)",
  "eigenvalue",
  "moment",
  "XYZ",
  "RThetaZ",
  "RThetaPhi",
  "J",
  "K",
  "R",
  "wheelParms"
};

const char *
_tenTripleTypeDesc[] = {
  "unknown tenTriple",
  "eigenvalues sorted in descending order",
  "central moments (mu1,mu2,mu3)",
  "rotation of evals, like Bahn 1999 JMR:141(68-77)",
  "cylindrical coords of rotated evals",
  "spherical coords of rotated evals",
  "principal invariants (J1,J2,J3)",
  "cylindrical invariants (K1,K2,K3)",
  "spherical invariants (R1,R2,R3)",
  "eigenvalue wheel (center,radius,angle)"
};

const char *
_tenTripleTypeStrEqv[] = {
  "eigenvalue", "eval", "ev",
  "moment", "mu",
  "XYZ",
  "RThetaZ", "RThZ", "rtz",
  "RThetaPhi", "RThPh", "rtp",
  "J",
  "K",
  "R",
  "wheelParm", "wheelParms", "WP",
  ""
};

const int
_tenTripleTypeValEqv[] = {
  tenTripleTypeEigenvalue, tenTripleTypeEigenvalue, tenTripleTypeEigenvalue,
  tenTripleTypeMoment, tenTripleTypeMoment,
  tenTripleTypeXYZ,
  tenTripleTypeRThetaZ, tenTripleTypeRThetaZ, tenTripleTypeRThetaZ,
  tenTripleTypeRThetaPhi, tenTripleTypeRThetaPhi, tenTripleTypeRThetaPhi,
  tenTripleTypeJ,
  tenTripleTypeK,
  tenTripleTypeR,
  tenTripleTypeWheelParm, tenTripleTypeWheelParm, tenTripleTypeWheelParm
};

const airEnum
_tenTripleType = {
  "tenTripleType",
  TEN_TRIPLE_TYPE_MAX,
  _tenTripleTypeStr, NULL,
  _tenTripleTypeDesc,
  _tenTripleTypeStrEqv, _tenTripleTypeValEqv,
  AIR_FALSE
};
const airEnum *const
tenTripleType = &_tenTripleType;
