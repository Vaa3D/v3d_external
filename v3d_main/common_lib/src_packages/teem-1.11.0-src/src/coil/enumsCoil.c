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

#include "coil.h"

const char *
_coilMethodTypeStr[COIL_METHOD_TYPE_MAX+1] = {
  "(unknown_method)",
  "testing",
  "homogeneous",
  "perona-malik",
  "modified curvature",
  "modified curvature rings",
  "curvature flow",
  "self",
  "finish"
};

const char *
_coilMethodTypeDesc[COIL_METHOD_TYPE_MAX+1] = {
  "unknown_method",
  "nothing, actually, just here for testing",
  "homogenous isotropic diffusion (Gaussian blurring)",
  "Perona-Malik",
  "modified curvature diffusion",
  "modified curvature diffusion rings",
  "curvature flow",
  "self-diffusion of diffusion tensors",
  "finish a phd already"
};

const char *
_coilMethodTypeStrEqv[] = {
  "test", "testing",
  "iso", "homog", "homogeneous",
  "pm", "perona-malik",
  "mcde", "modified curvature",
  "mcder", "modified curvature rings",
  "flow", "curvature flow",
  "self",
  "finish",
  ""
};

const int
_coilMethodTypeValEqv[] = {
  coilMethodTypeTesting, coilMethodTypeTesting,
  coilMethodTypeHomogeneous, coilMethodTypeHomogeneous, coilMethodTypeHomogeneous,
  coilMethodTypePeronaMalik, coilMethodTypePeronaMalik,
  coilMethodTypeModifiedCurvature, coilMethodTypeModifiedCurvature,
  coilMethodTypeModifiedCurvatureRings, coilMethodTypeModifiedCurvatureRings,
  coilMethodTypeCurvatureFlow, coilMethodTypeCurvatureFlow,
  coilMethodTypeSelf,
  coilMethodTypeFinish,
};

const airEnum
_coilMethodType = {
  "method",
  COIL_METHOD_TYPE_MAX,
  _coilMethodTypeStr,  NULL,
  _coilMethodTypeDesc,
  _coilMethodTypeStrEqv, _coilMethodTypeValEqv,
  AIR_FALSE
};
const airEnum *const
coilMethodType = &_coilMethodType;

/* -------------------------------------------------- */

const char *
_coilKindTypeStr[COIL_KIND_TYPE_MAX+1] = {
  "(unknown_kind)",
  "scalar",
  "3color",
  "7tensor"
};

const char *
_coilKindTypeDesc[COIL_KIND_TYPE_MAX+1] = {
  "unknown_kind",
  "plain old scalar quantities",
  "3-component color",
  "ten-style 7-valued tensor"
};

const char *
_coilKindTypeStrEqv[] = {
  "scalar",
  "3color",
  "7tensor", "tensor",
  ""
};

const int
_coilKindTypeValEqv[] = {
  coilKindTypeScalar,
  coilKindType3Color,
  coilKindType7Tensor, coilKindType7Tensor
};

const airEnum
_coilKindType = {
  "kind",
  COIL_KIND_TYPE_MAX,
  _coilKindTypeStr,  NULL,
  _coilKindTypeDesc,
  _coilKindTypeStrEqv, _coilKindTypeValEqv,
  AIR_FALSE
};
const airEnum *const
coilKindType = &_coilKindType;
