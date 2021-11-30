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


#include "bane.h"
#include "privateBane.h"

int
baneInputCheck (Nrrd *nin, baneHVolParm *hvp) {
  static const char me[]="baneInputCheck";
  int i;

  if (nrrdCheck(nin)) {
    biffMovef(BANE, NRRD, "%s: basic nrrd validity check failed", me);
    return 1;
  }
  if (3 != nin->dim) {
    biffAddf(BANE, "%s: need a 3-dimensional nrrd (not %d)", me, nin->dim);
    return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    biffAddf(BANE, "%s: can't operate on block type", me);
    return 1;
  }
  if (!( AIR_EXISTS(nin->axis[0].spacing) && nin->axis[0].spacing != 0 &&
         AIR_EXISTS(nin->axis[1].spacing) && nin->axis[1].spacing != 0 &&
         AIR_EXISTS(nin->axis[2].spacing) && nin->axis[2].spacing != 0 )) {
    biffAddf(BANE, "%s: must have non-zero existent spacing for all 3 axes",
             me);
    return 1;
  }
  for (i=0; i<=2; i++) {
    if (_baneAxisCheck(hvp->axis + i)) {
      biffAddf(BANE, "%s: trouble with axis %d", me, i);
      return 1;
    }
  }
  if (!hvp->clip) {
    biffAddf(BANE, "%s: got NULL baneClip", me);
    return 1;
  }

  /* all okay */
  return 0;
}

int
baneHVolCheck (Nrrd *hvol) {
  static const char me[]="baneHVolCheck";

  if (3 != hvol->dim) {
    biffAddf(BANE, "%s: need dimension to be 3 (not %d)", me, hvol->dim);
    return 1;
  }
  if (nrrdTypeUChar != hvol->type) {
    biffAddf(BANE, "%s: need type to be %s (not %s)",
             me, airEnumStr(nrrdType, nrrdTypeUChar),
             airEnumStr(nrrdType, hvol->type));
    return 1;
  }
  if (!( AIR_EXISTS(hvol->axis[0].min) && AIR_EXISTS(hvol->axis[0].max) &&
         AIR_EXISTS(hvol->axis[1].min) && AIR_EXISTS(hvol->axis[1].max) &&
         AIR_EXISTS(hvol->axis[2].min) && AIR_EXISTS(hvol->axis[2].max) )) {
    biffAddf(BANE, "%s: axisMin and axisMax must be set for all axes", me);
    return 1;
  }
  /*
  ** NOTE: For the time being, I'm giving up on enforcing a
  ** particular kind of histogram volume
  if (strcmp(hvol->axis[0].label, baneMeasrGradMag->name)) {
    biffAddf(BANE, "%s: expected \"%s\" on axis 0 label",
             me, baneMeasrGradMag->name);
    return 1;
  }
  if (strcmp(hvol->axis[1].label, baneMeasrLapl->name) &&
      strcmp(hvol->axis[1].label, baneMeasrHess->name)) {
    biffAddf(BANE, "%s: expected a 2nd deriv. measr on axis 1 (%s or %s)",
             me, baneMeasrHess->name, baneMeasrLapl->name);
    return 1;
  }
  if (strcmp(hvol->axis[2].label, baneMeasrVal->name)) {
    biffAddf(BANE, "%s: expected \"%s\" on axis 2",
             me, baneMeasrVal->name);
    return 1;
  }
  */
  return 0;
}

int
baneInfoCheck (Nrrd *info, int wantDim) {
  static const char me[]="baneInfoCheck";
  int gotDim;

  if (!info) {
    biffAddf(BANE, "%s: got NULL pointer", me);
    return 1;
  }
  gotDim = info->dim;
  if (wantDim) {
    if (!(1 == wantDim || 2 == wantDim)) {
      biffAddf(BANE, "%s: wantDim should be 1 or 2, not %d", me, wantDim);
      return 1;
    }
    if (wantDim+1 != gotDim) {
      biffAddf(BANE, "%s: dim is %d, not %d", me, gotDim, wantDim+1);
      return 1;
    }
  }
  else {
    if (!(2 == gotDim || 3 == gotDim)) {
      biffAddf(BANE, "%s: dim is %d, not 2 or 3", me, gotDim);
      return 1;
    }
  }
  if (nrrdTypeFloat != info->type) {
    biffAddf(BANE, "%s: need data of type float", me);
    return 1;
  }
  if (2 != info->axis[0].size) {
    char stmp[AIR_STRLEN_SMALL];
    biffAddf(BANE, "%s: 1st axis needs size 2 (not %s)", me,
             airSprintSize_t(stmp, info->axis[0].size));
    return 1;
  }
  return 0;
}

int
banePosCheck (Nrrd *pos, int wantDim) {
  static const char me[]="banePosCheck";
  int gotDim;

  if (!pos) {
    biffAddf(BANE, "%s: got NULL pointer", me);
    return 1;
  }
  gotDim = pos->dim;
  if (wantDim) {
    if (!(1 == wantDim || 2 == wantDim)) {
      biffAddf(BANE, "%s: wantDim should be 1 or 2, not %d", me, wantDim);
      return 1;
    }
    if (wantDim != gotDim) {
      biffAddf(BANE, "%s: dim is %d, not %d", me, gotDim, wantDim);
      return 1;
    }
  }
  else {
    if (!(1 == gotDim || 2 == gotDim)) {
      biffAddf(BANE, "%s: dim is %d, not 1 or 2", me, gotDim);
      return 1;
    }
  }
  if (nrrdTypeFloat != pos->type) {
    biffAddf(BANE, "%s: need data of type float", me);
    return 1;
  }
  /* HEY? check for values in axisMin[0] and axisMax[0] ? */
  /* HEY? check for values in axisMin[0] and axisMax[0] ? */
  /* HEY? check for values in axisMin[1] and axisMax[1] ? */
  return 0;
}

int
baneBcptsCheck (Nrrd *Bcpts) {
  static const char me[]="baneBcptsCheck";
  int i, len;
  float *data;

  if (2 != Bcpts->dim) {
    biffAddf(BANE, "%s: need 2-dimensional (not %d)", me, Bcpts->dim);
    return 1;
  }
  if (2 != Bcpts->axis[0].size) {
    char stmp[AIR_STRLEN_SMALL];
    biffAddf(BANE, "%s: axis#0 needs size 2 (not %s)", me,
             airSprintSize_t(stmp, Bcpts->axis[0].size));
    return 1;
  }
  if (nrrdTypeFloat != Bcpts->type) {
    biffAddf(BANE, "%s: need data of type float", me);
    return 1;
  }
  len = Bcpts->axis[1].size;
  data = (float *)Bcpts->data;
  for (i=0; i<=len-2; i++) {
    if (!(data[0 + 2*i] <= data[0 + 2*(i+1)])) {
      biffAddf(BANE, "%s: value coord %d (%g) not <= coord %d (%g)", me,
               i, data[0 + 2*i], i+1, data[0 + 2*(i+1)]);
      return 1;
    }
  }
  return 0;
}

