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

#include "seek.h"

const char *
seekBiffKey = "seek";

const char *
_seekTypeStr[SEEK_TYPE_MAX+1] = {
  "(unknown_feature)",
  "isocontour",
  "ridge surface",
  "valley surface",
  "ridge line",
  "valley line",
  "minimal surface",
  "maximal surface",
  "OP ridge surface",
  "T ridge surface",
  "OP valley surface",
  "T valley surface"
};

const char *
_seekTypeDesc[SEEK_TYPE_MAX+1] = {
  "unknown_feature",
  "standard marching cubes surface",
  "ridge surface",
  "valley surface",
  "ridge line",
  "valley line",
  "minimal surface",
  "maximal surface",
  "ridge surface using outer product rule",
  "ridge surface using tensor T",
  "valley surface using outer product rule",
  "valley surface using tensor T"
};

const char *
_seekTypeStrEqv[] = {
  "isocontour",
  "ridge surface", "ridgesurface", "rs",
  "valley surface", "valleysurface", "vs",
  "ridge line", "ridgeline", "rl",
  "valley line", "valleyline", "vl",
  "minimal surface", "mins",
  "maximal surface", "maxs",
  "OP ridge surface", "ridgesurfaceop", "rsop",
  "T ridge surface", "ridgesurfacet", "rst",
  "OP valley surface", "valleysurfaceop", "vsop",
  "T valley surface", "valleysurfacet", "vst",
  ""
};

const int
_seekTypeValEqv[] = {
  seekTypeIsocontour,
  seekTypeRidgeSurface, seekTypeRidgeSurface, seekTypeRidgeSurface,
  seekTypeValleySurface, seekTypeValleySurface, seekTypeValleySurface,
  seekTypeRidgeLine, seekTypeRidgeLine, seekTypeRidgeLine,
  seekTypeValleyLine, seekTypeValleyLine, seekTypeValleyLine,
  seekTypeMinimalSurface, seekTypeMinimalSurface,
  seekTypeMaximalSurface, seekTypeMaximalSurface,
  seekTypeRidgeSurfaceOP, seekTypeRidgeSurfaceOP, seekTypeRidgeSurfaceOP,
  seekTypeRidgeSurfaceT, seekTypeRidgeSurfaceT, seekTypeRidgeSurfaceT,
  seekTypeValleySurfaceOP, seekTypeValleySurfaceOP, seekTypeValleySurfaceOP,
  seekTypeValleySurfaceT, seekTypeValleySurfaceT, seekTypeValleySurfaceT
};

const airEnum
_seekType = {
  "format",
  SEEK_TYPE_MAX,
  _seekTypeStr,  NULL,
  _seekTypeDesc,
  _seekTypeStrEqv, _seekTypeValEqv,
  AIR_FALSE
};
const airEnum *const
seekType = &_seekType;

