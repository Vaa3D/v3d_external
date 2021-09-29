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

#include "limn.h"

const char *
_limnSpaceStr[LIMN_SPACE_MAX+1] = {
  "(unknown space)",
  "world",
  "view",
  "screen",
  "device"
};

const airEnum
_limnSpace = {
  "limn space",
  LIMN_SPACE_MAX,
  _limnSpaceStr, NULL,
  NULL,
  NULL, NULL,
  AIR_FALSE
};
const airEnum *const
limnSpace = &_limnSpace;

/* ------------------------------------------------------------ */

const char *
_limnPolyDataInfoStr[LIMN_POLY_DATA_INFO_MAX+1] = {
  "(unknown info)",
  "rgba",
  "norm",
  "tex2",
  "tang"
};

const airEnum
_limnPolyDataInfo = {
  "limn polydata info",
  LIMN_POLY_DATA_INFO_MAX,
  _limnPolyDataInfoStr, NULL,
  NULL,
  NULL, NULL,
  AIR_FALSE
};
const airEnum *const
limnPolyDataInfo = &_limnPolyDataInfo;

/* ------------------------------------------------------------ */

const char *
_limnCameraPathTrackStr[] = {
  "(unknown limnCameraPathTrack)",
  "from",
  "at",
  "both"
};

const char *
_limnCameraPathTrackDesc[] = {
  "unknown limnCameraPathTrack",
  "track through eye points, quaternions for camera orientation",
  "track through look-at points, quaternions for camera orientation",
  "track eye point, look-at point, and up vector with separate splines"
};

const char *
_limnCameraPathTrackStrEqv[] = {
  "from", "fr",
  "at", "look-at", "lookat",
  "both",
  ""
};

const int
_limnCameraPathTrackValEqv[] = {
  limnCameraPathTrackFrom, limnCameraPathTrackFrom,
  limnCameraPathTrackAt, limnCameraPathTrackAt, limnCameraPathTrackAt,
  limnCameraPathTrackBoth
};

const airEnum
_limnCameraPathTrack = {
  "limnCameraPathTrack",
  LIMN_CAMERA_PATH_TRACK_MAX,
  _limnCameraPathTrackStr, NULL,
  _limnCameraPathTrackDesc,
  _limnCameraPathTrackStrEqv, _limnCameraPathTrackValEqv,
  AIR_FALSE
};
const airEnum *const
limnCameraPathTrack = &_limnCameraPathTrack;

/* ------------------------------------------------------------ */

const char *
_limnPrimitiveStr[] = {
  "(unknown limnPrimitive)",
  "noop",
  "triangles",
  "tristrip",
  "trifan",
  "quads",
  "linestrip",
  "lines"
};

const char *
_limnPrimitiveDesc[] = {
  "unknown limnPrimitive",
  "no-op",
  "triangle soup",
  "triangle strip",
  "triangle fan",
  "quad soup",
  "line strip",
  "lines"
};

const char *
_limnPrimitiveStrEqv[] = {
  "noop",
  "triangles",
  "tristrip",
  "trifan",
  "quads",
  "linestrip",
  "lines",
  ""
};

const int
_limnPrimitiveValEqv[] = {
  limnPrimitiveNoop,
  limnPrimitiveTriangles,
  limnPrimitiveTriangleStrip,
  limnPrimitiveTriangleFan,
  limnPrimitiveQuads,
  limnPrimitiveLineStrip,
  limnPrimitiveLines
};

const airEnum
_limnPrimitive = {
  "limnPrimitive",
  LIMN_PRIMITIVE_MAX,
  _limnPrimitiveStr, NULL,
  _limnPrimitiveDesc,
  _limnPrimitiveStrEqv, _limnPrimitiveValEqv,
  AIR_FALSE
};
const airEnum *const
limnPrimitive = &_limnPrimitive;
