/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2011 Thomas Schultz

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

const char *
_tijk_class_str[TIJK_CLASS_MAX+1] = {
  "(unknown_class)",
  "tensor",
  "esh",
  "efs",
};

const char *
_tijk_class_desc[TIJK_CLASS_MAX+1] = {
  "unknown class",
  "tensor, specified by a tijk_type",
  "even-order spherical harmonics, specified by order",
  "even-order fourier series, specified by order",
};

const char *
_tijk_class_str_eqv[] = {
  "tensor",
  "esh",
  "efs",
  ""
};

int
_tijk_class_val_eqv[] = {
  tijk_class_tensor,
  tijk_class_esh,
  tijk_class_efs,
};

airEnum
_tijk_class_enum = {
  "class",
  TIJK_CLASS_MAX,
  _tijk_class_str, NULL,
  _tijk_class_desc,
  _tijk_class_str_eqv, _tijk_class_val_eqv,
  AIR_FALSE
};
const airEnum *const
tijk_class = &_tijk_class_enum;
