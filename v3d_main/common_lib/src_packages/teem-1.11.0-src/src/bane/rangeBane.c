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
_baneRangePositive_Answer(double *ominP, double *omaxP,
                          double imin, double imax) {
  static const char me[]="_baneRangePositive_Answer";

  if (!( AIR_EXISTS(imin) && AIR_EXISTS(imax) )) {
    biffAddf(BANE, "%s: imin and imax don't both exist", me);
    return 1;
  }
  *ominP = 0;
  *omaxP = imax;
  return 0;
}

int
_baneRangeNegative_Answer(double *ominP, double *omaxP,
                          double imin, double imax) {
  static const char me[]="_baneRangeNegative_Answer";

  if (!( AIR_EXISTS(imin) && AIR_EXISTS(imax) )) {
    biffAddf(BANE, "%s: imin and imax don't both exist", me);
    return 1;
  }
  *ominP = imin;
  *omaxP = 0;
  return 0;
}

/*
** _baneRangeZeroCentered_Answer
**
** Unlike the last version of this function, this is conservative: we
** choose the smallest zero-centered range that includes the original
** min and max.  Previously the average of the min and max magnitude
** were used.
*/
int
_baneRangeZeroCentered_Answer(double *ominP, double *omaxP,
                              double imin, double imax) {
  static const char me[]="_baneRangeZeroCentered_Answer";

  if (!( AIR_EXISTS(imin) && AIR_EXISTS(imax) )) {
    biffAddf(BANE, "%s: imin and imax don't both exist", me);
    return 1;
  }
  imin = AIR_MIN(imin, 0);
  imax = AIR_MAX(imax, 0);
  /* now the signs of imin and imax aren't wrong */
  *ominP = AIR_MIN(-imax, imin);
  *omaxP = AIR_MAX(imax, -imin);
  return 0;
}

int
_baneRangeAnywhere_Answer(double *ominP, double *omaxP,
                          double imin, double imax) {
  static const char me[]="_baneRangeAnywhere_Answer";

  if (!( AIR_EXISTS(imin) && AIR_EXISTS(imax) )) {
    biffAddf(BANE, "%s: imin and imax don't both exist", me);
    return 1;
  }
  *ominP = imin;
  *omaxP = imax;
  return 0;
}

baneRange *
baneRangeNew(int type) {
  static const char me[]="baneRangeNew";
  baneRange *range = NULL;

  if (!AIR_IN_OP(baneRangeUnknown, type, baneRangeLast)) {
    biffAddf(BANE, "%s: baneRange %d not valid", me, type);
    return NULL;
  }
  range = (baneRange *)calloc(1, sizeof(baneRange));
  if (!range) {
    biffAddf(BANE, "%s: couldn't allocate baneRange!", me);
    return NULL;
  }
  range->type = type;
  range->center = AIR_NAN;
  switch(type) {
  case baneRangePositive:
    sprintf(range->name, "positive");
    range->answer = _baneRangePositive_Answer;
    break;
  case baneRangeNegative:
    sprintf(range->name, "negative");
    range->answer = _baneRangeNegative_Answer;
    break;
  case baneRangeZeroCentered:
    sprintf(range->name, "zero-centered");
    range->answer = _baneRangeZeroCentered_Answer;
    break;
  case baneRangeAnywhere:
    sprintf(range->name, "anywhere");
    range->answer = _baneRangeAnywhere_Answer;
    break;
  default:
    biffAddf(BANE, "%s: Sorry, baneRange %d not implemented", me, type);
    baneRangeNix(range); return NULL;
  }
  return range;
}

baneRange *
baneRangeCopy(baneRange *range) {
  static const char me[]="baneRangeCopy";
  baneRange *ret = NULL;

  ret = baneRangeNew(range->type);
  if (!ret) {
    biffAddf(BANE, "%s: couldn't make new range", me);
    return NULL;
  }
  ret->center = range->center;
  return ret;
}

int
baneRangeAnswer(baneRange *range,
                double *ominP, double *omaxP,
                double imin, double imax) {
  static const char me[]="baneRangeAnswer";

  if (!(range && ominP && omaxP)) {
    biffAddf(BANE, "%s: got NULL pointer", me);
    return 1;
  }
  if (range->answer(ominP, omaxP, imin, imax)) {
    biffAddf(BANE, "%s: trouble", me);
    return 1;
  }
  return 0;
}

baneRange *
baneRangeNix(baneRange *range) {

  if (range) {
    airFree(range);
  }
  return NULL;
}
