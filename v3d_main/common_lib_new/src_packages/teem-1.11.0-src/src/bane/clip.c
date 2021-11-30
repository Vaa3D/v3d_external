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
_baneClipAnswer_Absolute(int *countP, Nrrd *hvol, double *clipParm) {

  AIR_UNUSED(hvol);
  *countP = (int)(clipParm[0]);
  return 0;
}

int
_baneClipAnswer_PeakRatio(int *countP, Nrrd *hvol, double *clipParm) {
  int *hits, maxhits;
  size_t idx, num;

  hits = (int *)hvol->data;
  maxhits = 0;
  num = nrrdElementNumber(hvol);
  for (idx=0; idx<num; idx++) {
    maxhits = AIR_MAX(maxhits, hits[idx]);
  }

  *countP = (int)(maxhits*clipParm[0]);
  return 0;
}

int
_baneClipAnswer_Percentile(int *countP, Nrrd *hvol, double *clipParm) {
  static const char me[]="_baneClipAnswer_Percentile";
  Nrrd *ncopy;
  int *hits, clip;
  size_t num, sum, out, outsofar, hi;

  if (nrrdCopy(ncopy=nrrdNew(), hvol)) {
    biffMovef(BANE, NRRD, "%s: couldn't create copy of histovol", me);
    return 1;
  }
  hits = (int *)ncopy->data;
  num = nrrdElementNumber(ncopy);
  qsort(hits, num, sizeof(int), nrrdValCompare[nrrdTypeInt]);
  sum = 0;
  for (hi=0; hi<num; hi++) {
    sum += hits[hi];
  }
  out = (size_t)(sum*clipParm[0]/100);
  outsofar = 0;
  hi = num-1;
  do {
    outsofar += hits[hi--];
  } while (outsofar < out);
  clip = hits[hi];
  nrrdNuke(ncopy);

  *countP = clip;
  return 0;
}

int
_baneClipAnswer_TopN(int *countP, Nrrd *hvol, double *clipParm) {
  static const char me[]="_baneClipAnwer_TopN";
  Nrrd *copy;
  int *hits, tmp;
  size_t num;

  if (nrrdCopy(copy=nrrdNew(), hvol)) {
    biffMovef(BANE, NRRD, "%s: couldn't create copy of histovol", me);
    return 1;
  }
  hits = (int *)copy->data;
  num = nrrdElementNumber(copy);
  qsort(hits, num, sizeof(int), nrrdValCompare[nrrdTypeInt]);
  tmp = AIR_CLAMP(0, (int)clipParm[0], (int)num-1);
  *countP = hits[num-tmp-1];
  nrrdNuke(copy);

  return 0;
}

baneClip *
baneClipNew(int type, double *parm) {
  static const char me[]="baneClipNew";
  baneClip *clip;

  if (!( AIR_IN_OP(baneClipUnknown, type, baneClipLast) )) {
    biffAddf(BANE, "%s: baneClip %d invalid", me, type);
    return NULL;
  }
  if (!parm) {
    biffAddf(BANE, "%s: got NULL pointer", me);
    return NULL;
  }
  if (!(AIR_EXISTS(parm[0]))) {
    biffAddf(BANE, "%s: parm[0] doesn't exist", me);
    return NULL;
  }
  clip = (baneClip*)calloc(1, sizeof(baneClip));
  if (!clip) {
    biffAddf(BANE, "%s: couldn't allocate baneClip!", me);
    return NULL;
  }
  clip->parm[0] = parm[0];
  clip->type = type;
  switch(type) {
  case baneClipAbsolute:
    sprintf(clip->name, "absolute");
    clip->answer = _baneClipAnswer_Absolute;
    break;
  case baneClipPeakRatio:
    sprintf(clip->name, "peak ratio");
    clip->answer = _baneClipAnswer_PeakRatio;
    break;
  case baneClipPercentile:
    sprintf(clip->name, "percentile");
    clip->answer = _baneClipAnswer_Percentile;
    break;
  case baneClipTopN:
    sprintf(clip->name, "top N");
    clip->answer = _baneClipAnswer_TopN;
    break;
  default:
    biffAddf(BANE, "%s: sorry, baneClip %d not implemented", me, type);
    baneClipNix(clip); return NULL;
    break;
  }
  return clip;
}

int
baneClipAnswer(int *countP, baneClip *clip, Nrrd *hvol) {
  static const char me[]="baneClipAnswer";

  if (!( countP && clip && hvol )) {
    biffAddf(BANE, "%s: got NULL pointer", me);
    return 0;
  }
  if (clip->answer(countP, hvol, clip->parm)) {
    biffAddf(BANE, "%s: trouble", me);
    return 0;
  }
  return 0;
}

baneClip *
baneClipCopy(baneClip *clip) {
  static const char me[]="baneClipCopy";
  baneClip *ret = NULL;

  ret = baneClipNew(clip->type, clip->parm);
  if (!ret) {
    biffAddf(BANE, "%s: couldn't make new clip", me);
    return NULL;
  }
  return ret;
}

baneClip *
baneClipNix(baneClip *clip) {

  if (clip) {
    airFree(clip->name);
    airFree(clip);
  }
  return NULL;
}

