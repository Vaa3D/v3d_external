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

int
limnEnvMapFill(Nrrd *map, limnEnvMapCB cb, int qnMethod, void *data) {
  static const char me[]="limnEnvMapFill";
  unsigned int sx, sy, qn;
  float vec[3], *mapData;

  if (!(map && cb)) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if (!AIR_IN_OP(limnQNUnknown, qnMethod, limnQNLast)) {
    biffAddf(LIMN, "%s: QN method %d invalid", me, qnMethod);
    return 1;
  }
  switch(qnMethod) {
  case limnQN16checker:
  case limnQN16octa:
    sx = sy = 256;
    break;
  case limnQN14checker:
  case limnQN14octa:
    sx = sy = 128;
    break;
  case limnQN12checker:
  case limnQN12octa:
    sx = sy = 64;
    break;
  case limnQN10checker:
  case limnQN10octa:
    sx = sy = 32;
    break;
  case limnQN8checker:
  case limnQN8octa:
    sx = sy = 16;
    break;
  case limnQN15octa:
    sx = 128;
    sy = 256;
    break;
  case limnQN13octa:
    sx = 64;
    sy = 128;
    break;
  case limnQN11octa:
    sx = 32;
    sy = 64;
    break;
  case limnQN9octa:
    sx = 16;
    sy = 32;
    break;
  default:
    biffAddf(LIMN, "%s: sorry, QN method %d not implemented", me, qnMethod);
    return 1;
  }
  if (nrrdMaybeAlloc_va(map, nrrdTypeFloat, 3,
                        AIR_CAST(size_t, 3),
                        AIR_CAST(size_t, sx),
                        AIR_CAST(size_t, sy))) {
    biffMovef(LIMN, NRRD, "%s: couldn't alloc output", me);
    return 1;
  }
  mapData = (float *)map->data;
  for (qn=0; qn<sx*sy; qn++) {
    limnQNtoV_f[qnMethod](vec, qn);
    cb(mapData + 3*qn, vec, data);
  }

  return 0;
}

void
limnLightDiffuseCB(float rgb[3], float vec[3], void *_lit) {
  float dot, r, g, b, norm;
  limnLight *lit;
  int i;

  lit = (limnLight *)_lit;
  ELL_3V_NORM_TT(vec, float, vec, norm);
  r = lit->amb[0];
  g = lit->amb[1];
  b = lit->amb[2];
  for (i=0; i<LIMN_LIGHT_NUM; i++) {
    if (!lit->on[i])
      continue;
    dot = ELL_3V_DOT(vec, lit->dir[i]);
    dot = AIR_MAX(0, dot);
    r += dot*lit->col[i][0];
    g += dot*lit->col[i][1];
    b += dot*lit->col[i][2];
  }
  /* not really our job to be doing clamping here ... */
  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
}

int
limnEnvMapCheck(Nrrd *envMap) {
  static const char me[]="limnEnvMapCheck";

  if (nrrdCheck(envMap)) {
    biffMovef(LIMN, NRRD, "%s: basic nrrd validity check failed", me);
    return 1;
  }
  if (!(nrrdTypeFloat == envMap->type)) {
    biffAddf(LIMN, "%s: type should be %s, not %s", me,
             airEnumStr(nrrdType, nrrdTypeFloat),
             airEnumStr(nrrdType, envMap->type));
    return 1;
  }
  if (!(3 == envMap->dim)) {
    biffAddf(LIMN, "%s: dimension should be 3, not %d", me, envMap->dim);
    return 1;
  }
  if (!(3 == envMap->axis[0].size
        && 256 == envMap->axis[1].size
        && 256 == envMap->axis[2].size)) {
    char stmp[3][AIR_STRLEN_SMALL];
    biffAddf(LIMN, "%s: dimension should be 3x256x256, not "
             "%s x %s x %s", me,
             airSprintSize_t(stmp[0], envMap->axis[0].size),
             airSprintSize_t(stmp[1], envMap->axis[1].size),
             airSprintSize_t(stmp[2], envMap->axis[2].size));
    return 1;
  }
  return 0;
}
