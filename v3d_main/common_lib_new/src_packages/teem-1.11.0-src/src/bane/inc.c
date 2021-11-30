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

void
_baneIncProcess_LearnMinMax(baneInc *inc, double val) {

  if (AIR_EXISTS(inc->nhist->axis[0].min)) {
    /* then both min and max have seen at least one valid value */
    inc->nhist->axis[0].min = AIR_MIN(inc->nhist->axis[0].min, val);
    inc->nhist->axis[0].max = AIR_MAX(inc->nhist->axis[0].max, val);
  } else {
    inc->nhist->axis[0].min = inc->nhist->axis[0].max = val;
  }
  /*
  fprintf(stderr, "## _baneInc_LearnMinMax: (%g,%g)\n",
          inc->nhist->axis[0].min, inc->nhist->axis[0].max);
  */
  return;
}

void
_baneIncProcess_Stdv(baneInc *inc, double val) {

  inc->S += val;
  inc->SS += val*val;
  inc->num += 1;
  return;
}

void
_baneIncProcess_HistFill(baneInc *inc, double val) {
  int *hist;
  unsigned int idx;

  idx = airIndex(inc->nhist->axis[0].min, val, inc->nhist->axis[0].max,
                 inc->nhist->axis[0].size);
  /*
  fprintf(stderr, "## _baneInc_HistFill: (%g,%g,%g) %d ---> %d\n",
          inc->nhist->axis[0].min, val, inc->nhist->axis[0].max,
          inc->nhist->axis[0].size, idx);
  */
  if (idx < inc->nhist->axis[0].size) {
    hist = (int*)inc->nhist->data;
    hist[idx]++;
  }
  return;
}

/*
** _baneIncAnswer_Absolute
**
** incParm[0]: new min
** incParm[1]: new max
*/
int
_baneIncAnswer_Absolute(double *minP, double *maxP,
                        Nrrd *hist, double *incParm,
                        baneRange *range) {
  AIR_UNUSED(hist);
  AIR_UNUSED(range);
  *minP = incParm[0];
  *maxP = incParm[1];
  return 0;
}

/*
** _baneIncAnswer_RangeRatio
**
** incParm[0]: scales the size of the range after it has been
** sent through the associated range function.
*/
int
_baneIncAnswer_RangeRatio(double *minP, double *maxP,
                          Nrrd *hist, double *incParm,
                          baneRange *range) {
  static const char me[]="_baneIncAnwer_RangeRatio";
  double mid;

  if (range->answer(minP, maxP, hist->axis[0].min, hist->axis[0].max)) {
    biffAddf(BANE, "%s: trouble", me);
    return 1;
  }

  if (baneRangeAnywhere == range->type) {
    mid = AIR_EXISTS(range->center) ? range->center : (*minP + *maxP)/2;
    *minP = AIR_AFFINE(-1, -incParm[0], 0, *minP, mid);
    *maxP = AIR_AFFINE(0, incParm[0], 1, mid, *maxP);
  } else {
    *minP *= incParm[0];
    *maxP *= incParm[0];
  }
  return 0;
}

/*
** _baneIncAnswer_Percentile
**
** incParm[0]: resolution of histogram generated
** incParm[1]: PERCENT of hits to throw away, by nibbling away at
** lower and upper ends of range, in a manner dependant on the
** range type
*/
int
_baneIncAnswer_Percentile(double *minP, double *maxP,
                          Nrrd *nhist, double *incParm,
                          baneRange *range) {
  static const char me[]="_baneIncAnswer_Percentile";
  int *hist, i, histSize, sum;
  float minIncr, maxIncr, out, outsofar, mid, minIdx, maxIdx;
  double min, max;

  /* integrate histogram and determine how many hits to exclude */
  sum = 0;
  hist = (int *)nhist->data;
  histSize = nhist->axis[0].size;
  for (i=0; i<histSize; i++) {
    sum += hist[i];
  }
  if (!sum) {
    biffAddf(BANE, "%s: integral of histogram is zero", me);
    return 1;
  }
  /*
  sprintf(err, "%03d-histo.nrrd", baneHack); nrrdSave(err, nhist, NULL);
  baneHack++;
  */
  out = AIR_CAST(float, sum*incParm[1]/100.0);
  fprintf(stderr, "##%s: hist's size=%d, sum=%d --> out = %g\n", me,
          histSize, sum, out);
  if (range->answer(&min, &max, nhist->axis[0].min, nhist->axis[0].max)) {
    biffAddf(BANE, "%s:", me); return 1;
  }
  fprintf(stderr, "##%s: hist's min,max (%g,%g) ---%s---> %g, %g\n",
          me, nhist->axis[0].min, nhist->axis[0].max,
          range->name, min, max);
  if (baneRangeAnywhere == range->type) {
    mid = AIR_CAST(float, (AIR_EXISTS(range->center)
                           ? range->center
                           : (min + max)/2));
  } else {
    mid = 0;
    /* yes, this is okay.  The "mid" is the value we march towards
       from both ends, but we control the rate of marching according
       to the distance to the ends.  So if min == mid == 0, then
       there is no marching up from below
       HOWEVER: the mode of histogram would probably be better. */
  }
  fprintf(stderr, "##%s: hist (%g,%g) --> min,max = (%g,%g) --> mid = %g\n",
          me, nhist->axis[0].min, nhist->axis[0].max, min, max, mid);
  if (max-mid > mid-min) {
    /* the max is further from the mid than the min */
    maxIncr = 1;
    minIncr = AIR_CAST(float, (mid-min)/(max-mid));
  } else {
    /* the min is further */
    minIncr = 1;
    maxIncr = AIR_CAST(float, (max-mid)/(mid-min));
  }
  if (!( AIR_EXISTS(minIncr) && AIR_EXISTS(maxIncr) )) {
    biffAddf(BANE, "%s: minIncr, maxIncr don't both exist", me);
    return 1;
  }
  fprintf(stderr, "##%s: --> {min,max}Incr = %g,%g\n", me, minIncr, maxIncr);
  minIdx = AIR_CAST(float,
                    AIR_AFFINE(nhist->axis[0].min, min, nhist->axis[0].max,
                               0, histSize-1));
  maxIdx = AIR_CAST(float,
                    AIR_AFFINE(nhist->axis[0].min, max, nhist->axis[0].max,
                               0, histSize-1));
  outsofar = 0;
  while (outsofar < out) {
    if (AIR_IN_CL(0, minIdx, histSize-1)) {
      outsofar += minIncr*hist[AIR_ROUNDUP(minIdx)];
    }
    if (AIR_IN_CL(0, maxIdx, histSize-1)) {
      outsofar += maxIncr*hist[AIR_ROUNDUP(maxIdx)];
    }
    minIdx += minIncr;
    maxIdx -= maxIncr;
    if (minIdx > maxIdx) {
      biffAddf(BANE, "%s: minIdx (%g) passed maxIdx (%g) during "
               "histogram traversal", me, minIdx, maxIdx);
      return 1;
    }
  }
  *minP = AIR_AFFINE(0, minIdx, histSize-1,
                     nhist->axis[0].min, nhist->axis[0].max);
  *maxP = AIR_AFFINE(0, maxIdx, histSize-1,
                     nhist->axis[0].min, nhist->axis[0].max);
  fprintf(stderr, "##%s: --> output min, max = %g, %g\n", me, *minP, *maxP);
  return 0;
}

/*
** _baneIncAnswer_Stdv()
**
** incParm[0]: range is standard deviation times this
*/
int
_baneIncAnswer_Stdv(double *minP, double *maxP,
                    Nrrd *hist, double *incParm,
                    baneRange *range) {
  float SS, stdv, mid, mean, width;
  int count;

  count = hist->axis[1].size;
  mean = AIR_CAST(float, hist->axis[1].min/count);
  SS = AIR_CAST(float, hist->axis[1].max/count);
  stdv = AIR_CAST(float, sqrt(SS - mean*mean));
  width = AIR_CAST(float, incParm[0]*stdv);
  fprintf(stderr, "##%s: mean=%g, stdv=%g --> width=%g\n",
          "_baneIncAnswer_Stdv", mean, stdv, width);
  switch (range->type) {
  case baneRangePositive:
    *minP = 0;
    *maxP = width;
    break;
  case baneRangeNegative:
    *minP = -width;
    *maxP = 0;
    break;
  case baneRangeZeroCentered:
    *minP = -width/2;
    *maxP = width/2;
    break;
  case baneRangeAnywhere:
    mid = AIR_CAST(float, AIR_EXISTS(range->center) ? range->center : mean);
    *minP = mid - width/2;
    *maxP = mid + width/2;
    break;
  default:
    *minP = *maxP = AIR_NAN;
    break;
  }
  return 0;
}

baneInc *
baneIncNew(int type, baneRange *range, double *parm) {
  static const char me[]="baneIncNew";
  baneInc *inc;

  if (!(AIR_IN_OP(baneIncUnknown, type, baneIncLast))) {
    biffAddf(BANE, "%s: baneInc %d invalid", me, type);
    return NULL;
  }
  if (!(range && parm)) {
    biffAddf(BANE, "%s: got NULL baneRange or parm", me);
    return NULL;
  }
  inc = (baneInc*)calloc(1, sizeof(baneInc));
  if (!inc) {
    biffAddf(BANE, "%s: couldn't allocated baneInc!", me);
    return NULL;
  }
  inc->S = inc->SS = 0;
  inc->num = 0;
  inc->range = baneRangeCopy(range);
  if (!inc->range) {
    biffAddf(BANE, "%s: couldn't copy baneRange!", me);
    baneIncNix(inc); return NULL;
  }
  inc->type = type;
  switch (type) {
    /* --------------------------------------------------------- */
  case baneIncAbsolute:
    sprintf(inc->name, "absolute");
    inc->nhist = NULL;
    if (!( AIR_EXISTS(parm[0]) && AIR_EXISTS(parm[1]) )) {
      biffAddf(BANE, "%s: parm[0] and parm[1] don't both exist", me);
      baneIncNix(inc); return NULL;
    }
    inc->parm[0] = parm[0];  /* enforced min */
    inc->parm[1] = parm[1];  /* enforced max */
    inc->process[0] = NULL;
    inc->process[1] = NULL;
    inc->answer = _baneIncAnswer_Absolute;
    break;
    /* --------------------------------------------------------- */
  case baneIncRangeRatio:
    sprintf(inc->name, "range ratio");
    inc->nhist = nrrdNew();
    if (!AIR_EXISTS(parm[0])) {
      biffAddf(BANE, "%s: parm[0] doesn't exist", me);
      baneIncNix(inc); return NULL;
    }
    inc->parm[0] = parm[0];  /* scaling on range */
    inc->process[0] = NULL;
    inc->process[1] = _baneIncProcess_LearnMinMax;
    inc->answer = _baneIncAnswer_RangeRatio;
    break;
    /* --------------------------------------------------------- */
  case baneIncPercentile:
    sprintf(inc->name, "percentile");
    inc->nhist = nrrdNew();
    if (!( AIR_EXISTS(parm[0]) && AIR_EXISTS(parm[1]) )) {
      biffAddf(BANE, "%s: parm[0] and parm[1] don't both exist", me);
      baneIncNix(inc); return NULL;
    }
    inc->parm[0] = parm[0];  /* size of histogram */
    if (nrrdMaybeAlloc_va(inc->nhist, nrrdTypeInt, 1,
                          AIR_CAST(size_t, parm[0]))) {
      biffMovef(BANE, NRRD, "%s: couldn't allocate histogram", me);
      baneIncNix(inc); return NULL;
    }
    inc->parm[1] = parm[1];  /* percentile to exclude */
    inc->process[0] = _baneIncProcess_LearnMinMax;
    inc->process[1] = _baneIncProcess_HistFill;
    inc->answer = _baneIncAnswer_Percentile;
    break;
    /* --------------------------------------------------------- */
  case baneIncStdv:
    sprintf(inc->name, "stdv");
    inc->nhist = NULL;
    if (!AIR_EXISTS(parm[0])) {
      biffAddf(BANE, "%s: parm[0] doesn't exist", me);
      baneIncNix(inc); return NULL;
    }
    inc->parm[0] = parm[0];  /* multiple of standard dev to use */
    inc->process[0] = NULL;
    inc->process[1] = _baneIncProcess_Stdv;
    inc->answer = _baneIncAnswer_Stdv;
    break;
    /* --------------------------------------------------------- */
  default:
    biffAddf(BANE, "%s: Sorry, baneInc %d not implemented", me, type);
    baneIncNix(inc); return NULL;
  }
  return inc;
}

void
baneIncProcess(baneInc *inc, int passIdx, double val) {

  if (inc && (0 == passIdx || 1 == passIdx) && inc->process[passIdx]) {
    inc->process[passIdx](inc, val);
  }
  return;
}

int
baneIncAnswer(baneInc *inc, double *minP, double *maxP) {
  static const char me[]="baneIncAnswer";

  if (!( inc && minP && maxP )) {
    biffAddf(BANE, "%s: got NULL pointer", me);
    return 1;
  }
  if (inc->answer(minP, maxP, inc->nhist, inc->parm, inc->range)) {
    biffAddf(BANE, "%s: trouble", me);
    return 1;
  }
  return 0;
}

baneInc *
baneIncCopy(baneInc *inc) {
  static const char me[]="baneIncCopy";
  baneInc *ret = NULL;

  ret = baneIncNew(inc->type, inc->range, inc->parm);
  if (!ret) {
    biffAddf(BANE, "%s: couldn't make new inc", me);
    return NULL;
  }
  return ret;
}

baneInc *
baneIncNix(baneInc *inc) {

  if (inc) {
    baneRangeNix(inc->range);
    nrrdNuke(inc->nhist);
    airFree(inc);
  }
  return NULL;
}

