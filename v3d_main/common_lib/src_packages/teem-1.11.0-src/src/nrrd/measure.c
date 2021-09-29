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

#include "nrrd.h"
#include "privateNrrd.h"

/* the non-histogram measures assume that there will be no NaNs in data */
void
_nrrdMeasureUnknown(void *ans, int ansType,
                    const void *line, int lineType,
                    size_t len, double axmin, double axmax) {
  static const char me[]="_nrrdMeasureUnknown";

  AIR_UNUSED(line);
  AIR_UNUSED(lineType);
  AIR_UNUSED(len);
  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  fprintf(stderr, "%s: Need To Specify A Measure !!! \n", me);
  nrrdDStore[ansType](ans, AIR_NAN);
}

void
_nrrdMeasureMin(void *ans, int ansType,
                const void *line, int lineType, size_t len,
                double axmin, double axmax) {
  double val, M, (*lup)(const void*, size_t);
  size_t ii;

  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  lup = nrrdDLookup[lineType];
  if (nrrdTypeIsIntegral[lineType]) {
    M = lup(line, 0);
    for (ii=1; ii<len; ii++) {
      val = lup(line, ii);
      M = AIR_MIN(M, val);
    }
  } else {
    M = AIR_NAN;
    for (ii=0; !AIR_EXISTS(M) && ii<len; ii++) {
      M = lup(line, ii);
    }
    for (; ii<len; ii++) {
      val = lup(line, ii);
      if (AIR_EXISTS(val)) {
        M = AIR_MIN(M, val);
      }
    }
  }
  nrrdDStore[ansType](ans, M);
}


void
_nrrdMeasureMax(void *ans, int ansType,
                const void *line, int lineType, size_t len,
                double axmin, double axmax) {
  double val, M, (*lup)(const void*, size_t);
  size_t ii;

  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  lup = nrrdDLookup[lineType];
  if (nrrdTypeIsIntegral[lineType]) {
    M = lup(line, 0);
    for (ii=1; ii<len; ii++) {
      val = lup(line, ii);
      M = AIR_MAX(M, val);
    }
  } else {
    M = AIR_NAN;
    for (ii=0; !AIR_EXISTS(M) && ii<len; ii++) {
      M = lup(line, ii);
    }
    for (; ii<len; ii++) {
      val = lup(line, ii);
      if (AIR_EXISTS(val)) {
        M = AIR_MAX(M, val);
      }
    }
  }
  nrrdDStore[ansType](ans, M);
}

void
_nrrdMeasureProduct(void *ans, int ansType,
                    const void *line, int lineType, size_t len,
                    double axmin, double axmax) {
  double val, P, (*lup)(const void*, size_t);
  size_t ii;

  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  lup = nrrdDLookup[lineType];
  if (nrrdTypeIsIntegral[lineType]) {
    P = 1.0;
    for (ii=0; ii<len; ii++) {
      P *= lup(line, ii);
    }
  } else {
    P = AIR_NAN;
    /* the point of this is to ensure that that if there are NO
       existent values, then the return is NaN */
    for (ii=0; !AIR_EXISTS(P) && ii<len; ii++) {
      P = lup(line, ii);
    }
    for (; ii<len; ii++) {
      val = lup(line, ii);
      if (AIR_EXISTS(val)) {
        P *= val;
      }
    }
  }
  nrrdDStore[ansType](ans, P);
}

void
_nrrdMeasureSum(void *ans, int ansType,
                const void *line, int lineType, size_t len,
                double axmin, double axmax) {
  double val, S, (*lup)(const void*, size_t);
  size_t ii;

  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  lup = nrrdDLookup[lineType];
  if (nrrdTypeIsIntegral[lineType]) {
    S = 0.0;
    for (ii=0; ii<len; ii++) {
      S += lup(line, ii);
    }
  } else {
    S = AIR_NAN;
    for (ii=0; !AIR_EXISTS(S) && ii<len; ii++)
      S = lup(line, ii);
    for (; ii<len; ii++) {
      val = lup(line, ii);
      if (AIR_EXISTS(val)) {
        S += val;
      }
    }
  }
  nrrdDStore[ansType](ans, S);
}

void
_nrrdMeasureMean(void *ans, int ansType,
                 const void *line, int lineType, size_t len,
                 double axmin, double axmax) {
  double val, S, M, (*lup)(const void*, size_t);
  size_t ii, count;

  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  lup = nrrdDLookup[lineType];
  if (nrrdTypeIsIntegral[lineType]) {
    S = 0.0;
    for (ii=0; ii<len; ii++) {
      S += lup(line, ii);
    }
    M = S/len;
  } else {
    S = AIR_NAN;
    for (ii=0; !AIR_EXISTS(S) && ii<len; ii++) {
      S = lup(line, ii);
    }
    if (AIR_EXISTS(S)) {
      /* there was an existent value */
      count = 1;
      for (; ii<len; ii++) {
        val = lup(line, ii);
        if (AIR_EXISTS(val)) {
          count++;
          S += val;
        }
      }
      M = S/count;
    } else {
      /* there were NO existent values */
      M = AIR_NAN;
    }
  }
  nrrdDStore[ansType](ans, M);
}

/* stupid little forward declaration */
void
_nrrdMeasureHistoMode(void *ans, int ansType,
                      const void *line, int lineType, size_t len,
                      double axmin, double axmax);

void
_nrrdMeasureMode(void *ans, int ansType,
                 const void *_line, int lineType, size_t len,
                 double axmin, double axmax) {
  Nrrd *nline, *nhist;
  void *line;

  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  line = calloc(len, nrrdTypeSize[lineType]);
  if (line) {
    memcpy(line, _line, len*nrrdTypeSize[lineType]);

    nline = nrrdNew();
    if (nrrdWrap_va(nline, line, lineType, 1, len)) {
      free(biffGetDone(NRRD));
      nrrdNix(nline);
      nrrdDStore[ansType](ans, AIR_NAN);
      return;
    }
    nhist = nrrdNew();
    if (nrrdHisto(nhist, nline, NULL, NULL,
                  nrrdStateMeasureModeBins, nrrdTypeInt)) {
      free(biffGetDone(NRRD));
      nrrdNuke(nhist);
      nrrdNix(nline);
      nrrdDStore[ansType](ans, AIR_NAN);
      return;
    }

    /* now we pass this histogram off to histo-mode */
    _nrrdMeasureHistoMode(ans, ansType,
                          nhist->data, nrrdTypeInt, nrrdStateMeasureModeBins,
                          nhist->axis[0].min, nhist->axis[0].max);
    nrrdNuke(nhist);
    nrrdNix(nline);
  } else {
    nrrdDStore[ansType](ans, 0);
  }
  return;
}

void
_nrrdMeasureMedian(void *ans, int ansType,
                   const void *_line, int lineType, size_t len,
                   double axmin, double axmax) {
  double M=0, (*lup)(const void*, size_t);
  size_t ii, mid;
  void *line;

  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  lup = nrrdDLookup[lineType];
  line = calloc(len, nrrdTypeSize[lineType]);
  if (line) {
    memcpy(line, _line, len*nrrdTypeSize[lineType]);

    /* yes, I know, this is not the fastest median.  I'll get to it ... */
    qsort(line, len, nrrdTypeSize[lineType], nrrdValCompare[lineType]);
    M = AIR_NAN;
    for (ii=0; !AIR_EXISTS(M) && ii<len; ii++) {
      M = lup(line, ii);
    }

    if (AIR_EXISTS(M)) {
      /* i is index AFTER first existent value */
      ii--;
      len -= ii;
      mid = len/2;
      if (len % 2) {
        /* len is odd, there is a middle value, its at mid */
        M = lup(line, ii+mid);
      } else {
        /* len is even, two middle values are at mid-1 and mid */
        M = (lup(line, ii+mid-1) + lup(line, ii+mid))/2;
      }
    }
  }
  nrrdDStore[ansType](ans, M);
}

void
_nrrdMeasureL1(void *ans, int ansType,
               const void *line, int lineType, size_t len,
               double axmin, double axmax) {
  double val, S, (*lup)(const void*, size_t);
  size_t ii;

  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  lup = nrrdDLookup[lineType];
  if (nrrdTypeIsIntegral[lineType]) {
    S = 0.0;
    for (ii=0; ii<len; ii++) {
      val = lup(line, ii);
      S += AIR_ABS(val);
    }
  } else {
    S = AIR_NAN;
    for (ii=0; !AIR_EXISTS(S) && ii<len; ii++) {
      S = lup(line, ii);
    }
    S = AIR_ABS(S);
    for (; ii<len; ii++) {
      val = lup(line, ii);
      if (AIR_EXISTS(val)) {
        S += AIR_ABS(val);
      }
    }
  }
  nrrdDStore[ansType](ans, S);
}

#define L2_BODY                                  \
  AIR_UNUSED(axmin);                             \
  AIR_UNUSED(axmax);                             \
  lup = nrrdDLookup[lineType];                   \
  if (nrrdTypeIsIntegral[lineType]) {            \
    S = 0.0;                                     \
    count = len;                                 \
    for (ii=0; ii<len; ii++) {                   \
      val = lup(line, ii);                       \
      S += val*val;                              \
    }                                            \
  } else {                                       \
    S = AIR_NAN;                                 \
    count = 0;                                   \
    for (ii=0; !AIR_EXISTS(S) && ii<len; ii++) { \
      S = lup(line, ii);                         \
    }                                            \
    if (AIR_EXISTS(S)) {                         \
      /* there's at least one existing value */  \
      count = 1;                                 \
      S *= S;                                    \
      for (; ii<len; ii++) {                     \
        val = lup(line, ii);                     \
        if (AIR_EXISTS(val)) {                   \
          count++;                               \
          S += val*val;                          \
        }                                        \
      }                                          \
    }                                            \
  }

void
_nrrdMeasureL2(void *ans, int ansType,
               const void *line, int lineType, size_t len,
               double axmin, double axmax) {
  double val, S, aa, (*lup)(const void*, size_t);
  size_t ii, count;

  L2_BODY;
  if (AIR_EXISTS(S)) {
    aa = sqrt(S);
  } else {
    aa = AIR_NAN;
  }
  nrrdDStore[ansType](ans, aa);
}

void
_nrrdMeasureNormalizedL2(void *ans, int ansType,
                         const void *line, int lineType, size_t len,
                         double axmin, double axmax) {
  double val, S, aa, (*lup)(const void*, size_t);
  size_t ii, count;

  L2_BODY;
  if (AIR_EXISTS(S)) {
    aa = sqrt(S)/count;
  } else {
    aa = AIR_NAN;
  }
  nrrdDStore[ansType](ans, aa);
}

void
_nrrdMeasureRootMeanSquare(void *ans, int ansType,
                           const void *line, int lineType, size_t len,
                           double axmin, double axmax) {
  double val, S, aa, (*lup)(const void*, size_t);
  size_t ii, count;

  L2_BODY;
  if (AIR_EXISTS(S)) {
    aa = sqrt(S/count);
  } else {
    aa = AIR_NAN;
  }
  nrrdDStore[ansType](ans, aa);
}

void
_nrrdMeasureLinf(void *ans, int ansType,
                 const void *line, int lineType, size_t len,
                 double axmin, double axmax) {
  double val, M, (*lup)(const void*, size_t);
  size_t ii;

  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  lup = nrrdDLookup[lineType];
  if (nrrdTypeIsIntegral[lineType]) {
    val = lup(line, 0);
    M = AIR_ABS(val);
    for (ii=1; ii<len; ii++) {
      val = lup(line, ii);
      val = AIR_ABS(val);
      M = AIR_MAX(M, val);
    }
  } else {
    M = AIR_NAN;
    for (ii=0; !AIR_EXISTS(M) && ii<len; ii++) {
      M = lup(line, ii);
    }
    M = AIR_ABS(M);
    for (; ii<len; ii++) {
      val = lup(line, ii);
      if (AIR_EXISTS(val)) {
        val = AIR_ABS(val);
        M = AIR_MAX(M, val);
      }
    }
  }
  nrrdDStore[ansType](ans, M);
}

void
_nrrdMeasureVariance(void *ans, int ansType,
                     const void *line, int lineType, size_t len,
                     double axmin, double axmax) {
  double val, S, SS, (*lup)(const void*, size_t);
  size_t ii, count;

  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  SS = S = 0.0;
  lup = nrrdDLookup[lineType];
  if (nrrdTypeIsIntegral[lineType]) {
    for (ii=0; ii<len; ii++) {
      val = lup(line, ii);
      S += val;
      SS += val*val;
    }
    S /= len;
    SS /= len;
  } else {
    count = 0;
    for (ii=0; ii<len; ii++) {
      val = lup(line, ii);
      if (AIR_EXISTS(val)) {
        count++;
        S += val;
        SS += val*val;
      }
    }
    if (count) {
      S /= count;
      SS /= count;
    } else {
      S = SS = AIR_NAN;
    }
  }
  /* HEY: the AIR_MAX is needed because precision errors
     can produce a negative value for SS - S*S;
     this may be a sign of the false economy of doing
     the variance calculation in a single pass */
  nrrdDStore[ansType](ans, AIR_MAX(0.0, SS - S*S));
}

void
_nrrdMeasureSD(void *ans, int ansType,
               const void *line, int lineType, size_t len,
               double axmin, double axmax) {
  double var;

  _nrrdMeasureVariance(ans, ansType, line, lineType, len, axmin, axmax);
  var = nrrdDLoad[ansType](ans);
  nrrdDStore[ansType](ans, sqrt(var));
}

void
_nrrdMeasureCoV(void *ans, int ansType,
                const void *line, int lineType, size_t len,
                double axmin, double axmax) {
  double val, S, M, (*lup)(const void*, size_t), diff, stdv;
  size_t ii, count;

  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  lup = nrrdDLookup[lineType];
  if (nrrdTypeIsIntegral[lineType]) {
    S = 0.0;
    for (ii=0; ii<len; ii++) {
      S += lup(line, ii);
    }
    M = S/len;
    diff = 0.0;
    for (ii=0; ii<len; ii++) {
      val = lup(line, ii);
      diff += (M-val)*(M-val);
    }
    stdv = sqrt(diff/len);
  } else {
    S = AIR_NAN;
    for (ii=0; !AIR_EXISTS(S) && ii<len; ii++) {
      S = lup(line, ii);
    }
    if (AIR_EXISTS(S)) {
      /* there was an existent value */
      count = 1;
      for (; ii<len; ii++) {
        val = lup(line, ii);
        if (AIR_EXISTS(val)) {
          count++;
          S += val;
        }
      }
      M = S/count;
      diff = 0.0;
      for (ii=0; ii<len; ii++) {
        val = lup(line, ii);
        if (AIR_EXISTS(val)) {
          diff += (M-val)*(M-val);
        }
      }
      stdv = sqrt(diff/count);
    } else {
      /* there were NO existent values */
      M = stdv = AIR_NAN;
    }
  }
  nrrdDStore[ansType](ans, stdv/M);
}

void
_nrrdMeasureLineFit(double *intc, double *slope,
                    const void *line, int lineType, size_t len,
                    double axmin, double axmax) {
  double x, y, xi=0, yi=0, xiyi=0, xisq=0, det, (*lup)(const void*, size_t);
  size_t ii;

  lup = nrrdDLookup[lineType];
  if (!( AIR_EXISTS(axmin) && AIR_EXISTS(axmax) )) {
    axmin = 0;
    axmax = AIR_CAST(double, len-1);
  }
  if (1 == len) {
    *slope = 0;
    *intc = lup(line, 0);
  } else {
    for (ii=0; ii<len; ii++) {
      x = NRRD_NODE_POS(axmin, axmax, len, ii);
      y = lup(line, ii);
      xi += x;
      yi += y;
      xiyi += x*y;
      xisq += x*x;
    }
    det = len*xisq - xi*xi;
    *slope = (len*xiyi - xi*yi)/det;
    *intc = (-xi*xiyi + xisq*yi)/det;
  }
}

void
_nrrdMeasureLineSlope(void *ans, int ansType,
                      const void *line, int lineType, size_t len,
                      double axmin, double axmax) {
  double slope, intc;

  _nrrdMeasureLineFit(&intc, &slope, line, lineType, len, axmin, axmax);
  nrrdDStore[ansType](ans, slope);
}

void
_nrrdMeasureLineIntercept(void *ans, int ansType,
                          const void *line, int lineType, size_t len,
                          double axmin, double axmax) {
  double slope, intc;

  _nrrdMeasureLineFit(&intc, &slope, line, lineType, len, axmin, axmax);
  nrrdDStore[ansType](ans, intc);
}

void
_nrrdMeasureLineError(void *ans, int ansType,
                      const void *line, int lineType, size_t len,
                      double axmin, double axmax) {
  double x, y, slope, intc, tmp, err=0, (*lup)(const void*, size_t);
  size_t ii;

  _nrrdMeasureLineFit(&intc, &slope, line, lineType, len, axmin, axmax);

  if (!( AIR_EXISTS(axmin) && AIR_EXISTS(axmax) )) {
    axmin = 0;
    axmax = AIR_CAST(double, len-1);
  }
  lup = nrrdDLookup[lineType];
  for (ii=0; ii<len; ii++) {
    x = NRRD_NODE_POS(axmin, axmax, len, ii);
    y = lup(line, ii);
    tmp = slope*x + intc - y;
    err += tmp*tmp;
  }
  nrrdDStore[ansType](ans, err);
}

void
_nrrdMeasureSkew(void *ans, int ansType,
                 const void *line, int lineType, size_t len,
                 double axmin, double axmax) {
  double val, diff, mean, vari, third, (*lup)(const void*, size_t);
  size_t ii, count;

  AIR_UNUSED(axmin);
  AIR_UNUSED(axmax);
  /* we don't try to do any one-pass short-cuts */

  /* find the mean */
  mean = 0;
  lup = nrrdDLookup[lineType];
  if (nrrdTypeIsIntegral[lineType]) {
    count = len;
    for (ii=0; ii<len; ii++) {
      val = lup(line, ii);
      mean += val;
    }
  } else {
    count = 0;
    for (ii=0; ii<len; ii++) {
      val = lup(line, ii);
      if (AIR_EXISTS(val)) {
        count++;
        mean += val;
      }
    }
  }
  if (0 == count) {
    nrrdDStore[ansType](ans, AIR_NAN);
    return;
  }
  mean /= count;

  /* find the variance and third moment */
  vari = third = 0;
  if (nrrdTypeIsIntegral[lineType]) {
    for (ii=0; ii<len; ii++) {
      diff = lup(line, ii) - mean;
      vari += diff*diff;
      third += diff*diff*diff;
    }
  } else {
    for (ii=0; ii<len; ii++) {
      val = lup(line, ii);
      if (AIR_EXISTS(val)) {
        diff = val - mean;
        vari += diff*diff;
        third += diff*diff*diff;
      }
    }
  }
  if (0 == vari) {
    /* why not have an existent value ... */
    nrrdDStore[ansType](ans, 0);
    return;
  }
  vari /= count;
  third /= count;

  nrrdDStore[ansType](ans, third/(vari*sqrt(vari)));
}

/*
** one thing which ALL the _nrrdMeasureHisto measures assume is that,
** being a histogram, the input array will not have any non-existent
** values.  It can be floating point, because it is plausible to have
** some histogram composed of fractionally weighted hits, but there is
** no way that it is reasonable to have NaN in a bin, and it is extremely
** unlikely that Inf could actually be created in a floating point
** histogram.
**
** Values in the histogram can be positive or negative, but negative
** values are always ignored.
**
** All the the  _nrrdMeasureHisto measures assume that if not both
** axmin and axmax are existent, then (axmin,axmax) = (-0.5,len-0.5).
** Exercise for the reader:  Show that
**
**    i == NRRD_POS(nrrdCenterCell, 0, len-1, len, i)
**
** This justifies that fact that when axmin and axmax are not both
** existent, then we can simply calculate the answer in index space,
** and not have to do any shifting or scaling at the end to account
** for the fact that we assume (axmin,axmax) = (-0.5,len-0.5).
*/

void
_nrrdMeasureHistoMedian(void *ans, int ansType,
                        const void *line, int lineType, size_t len,
                        double axmin, double axmax) {
  double sum, tmp, half, ansD, (*lup)(const void*, size_t);
  size_t ii;

  lup = nrrdDLookup[lineType];
  sum = 0;
  for (ii=0; ii<len; ii++) {
    tmp = lup(line, ii);
    sum += (tmp > 0 ? tmp : 0);
  }
  if (!sum) {
    nrrdDStore[ansType](ans, AIR_NAN);
    return;
  }
  /* else there was something in the histogram */
  half = sum/2;
  sum = 0;
  for (ii=0; ii<len; ii++) {
    tmp = lup(line, ii);
    sum += (tmp > 0 ? tmp : 0);
    if (sum >= half) {
      break;
    }
  }
  ansD = AIR_CAST(double, ii);
  if (AIR_EXISTS(axmin) && AIR_EXISTS(axmax)) {
    ansD = NRRD_CELL_POS(axmin, axmax, len, ansD);
  }
  nrrdDStore[ansType](ans, ansD);
}

void
_nrrdMeasureHistoMode(void *ans, int ansType,
                      const void *line, int lineType, size_t len,
                      double axmin, double axmax) {
  double val, max, idxsum, ansD, (*lup)(const void*, size_t);
  size_t ii, idxcount;

  lup = nrrdDLookup[lineType];
  max = -DBL_MAX;
  for (ii=0; ii<len; ii++) {
    val = lup(line, ii);
    max = AIR_MAX(max, val);
  }
  if (-DBL_MAX == max) {
    nrrdDStore[ansType](ans, AIR_NAN);
    return;
  }
  /* else there was something in the histogram */
  /* we assume that there may be multiple bins which reach the maximum
     height, and we average all those indices.  This may well be
     bone-headed, and is subject to change.  19 July 03: with the
     addition of the final "type" argument to nrrdProject, the
     bone-headedness has been alleviated somewhat, since you can pass
     nrrdTypeFloat or nrrdTypeDouble to get an accurate answer */
  idxsum = 0;
  idxcount = 0;
  for (ii=0; ii<len; ii++) {
    val = lup(line, ii);
    if (val == max) {
      idxcount++;
      idxsum += ii;
    }
  }
  ansD = idxsum/idxcount;
  /*
  printf("idxsum = %g; idxcount = %d --> ansD = %g --> ",
         (float)idxsum, idxcount, ansD);
  */
  if (AIR_EXISTS(axmin) && AIR_EXISTS(axmax)) {
    ansD = NRRD_CELL_POS(axmin, axmax, len, ansD);
  }
  /*
  printf("%g\n", ansD);
  */
  nrrdDStore[ansType](ans, ansD);
}

void
_nrrdMeasureHistoMean(void *ans, int ansType,
                      const void *line, int lineType, size_t len,
                      double axmin, double axmax) {
  double count, hits, ansD, (*lup)(const void*, size_t);
  size_t ii;

  lup = nrrdDLookup[lineType];
  ansD = count = 0;
  for (ii=0; ii<len; ii++) {
    hits = lup(line, ii);
    hits = AIR_MAX(hits, 0);
    count += hits;
    ansD += hits*ii;
  }
  if (!count) {
    nrrdDStore[ansType](ans, AIR_NAN);
    return;
  }
  ansD /= count;
  if (AIR_EXISTS(axmin) && AIR_EXISTS(axmax)) {
    ansD = NRRD_CELL_POS(axmin, axmax, len, ansD);
  }
  nrrdDStore[ansType](ans, ansD);
}

void
_nrrdMeasureHistoVariance(void *ans, int ansType,
                          const void *line, int lineType, size_t len,
                          double axmin, double axmax) {
  double S, SS, count, hits, val, (*lup)(const void*, size_t);
  size_t ii;

  lup = nrrdDLookup[lineType];
  count = 0;
  SS = S = 0.0;
  /* we fix axmin, axmax now because GK is better safe than sorry */
  if (!(AIR_EXISTS(axmin) && AIR_EXISTS(axmax))) {
    axmin = -0.5;
    axmax = len-0.5;
  }
  for (ii=0; ii<len; ii++) {
    val = NRRD_CELL_POS(axmin, axmax, len, ii);
    hits = lup(line, ii);
    hits = AIR_MAX(hits, 0);
    count += hits;
    S += hits*val;
    SS += hits*val*val;
  }
  if (!count) {
    nrrdDStore[ansType](ans, AIR_NAN);
    return;
  }
  S /= count;
  SS /= count;
  /* HEY: the AIR_MAX is needed because precision errors
     can produce a negative value for SS - S*S;
     this may be a sign of the false economy of doing
     the variance calculation in a single pass */
  nrrdDStore[ansType](ans, AIR_MAX(0.0, SS - S*S));
}

void
_nrrdMeasureHistoSD(void *ans, int ansType,
                    const void *line, int lineType, size_t len,
                    double axmin, double axmax) {
  double var;

  _nrrdMeasureHistoVariance(ans, ansType, line, lineType, len, axmin, axmax);
  var = nrrdDLoad[ansType](ans);
  nrrdDStore[ansType](ans, sqrt(var));
}

void
_nrrdMeasureHistoProduct(void *ans, int ansType,
                         const void *line, int lineType, size_t len,
                         double axmin, double axmax) {
  double val, product, count, hits, (*lup)(const void*, size_t);
  size_t ii;

  if (!(AIR_EXISTS(axmin) && AIR_EXISTS(axmax))) {
    axmin = -0.5;
    axmax = len-0.5;
  }
  lup = nrrdDLookup[lineType];
  product = 1.0;
  count = 0;
  for (ii=0; ii<len; ii++) {
    val = NRRD_CELL_POS(axmin, axmax, len, ii);
    hits = lup(line, ii);
    hits = AIR_MAX(hits, 0);
    count += hits;
    product *= pow(val, hits);
  }
  if (!count) {
    nrrdDStore[ansType](ans, AIR_NAN);
    return;
  }
  nrrdDStore[ansType](ans, product);
}

void
_nrrdMeasureHistoSum(void *ans, int ansType,
                     const void *line, int lineType, size_t len,
                     double axmin, double axmax) {
  double sum, hits, val, (*lup)(const void*, size_t);
  size_t ii;

  if (!(AIR_EXISTS(axmin) && AIR_EXISTS(axmax))) {
    axmin = -0.5;
    axmax = len-0.5;
  }
  lup = nrrdDLookup[lineType];
  sum = 0;
  for (ii=0; ii<len; ii++) {
    val = NRRD_CELL_POS(axmin, axmax, len, ii);
    hits = lup(line, ii);
    hits = AIR_MAX(hits, 0);
    sum += hits*val;
  }
  nrrdDStore[ansType](ans, sum);
}

void
_nrrdMeasureHistoL2(void *ans, int ansType,
                    const void *line, int lineType, size_t len,
                    double axmin, double axmax) {
  double l2, count, hits, val, (*lup)(const void*, size_t);
  size_t ii;

  if (!(AIR_EXISTS(axmin) && AIR_EXISTS(axmax))) {
    axmin = -0.5;
    axmax = len-0.5;
  }
  lup = nrrdDLookup[lineType];
  l2 = count = 0;
  for (ii=0; ii<len; ii++) {
    val = NRRD_CELL_POS(axmin, axmax, len, ii);
    hits = lup(line, ii);
    hits = AIR_MAX(hits, 0);
    count += hits;
    l2 += hits*val*val;
  }
  if (!count) {
    nrrdDStore[ansType](ans, AIR_NAN);
    return;
  }
  nrrdDStore[ansType](ans, l2);
}

void
_nrrdMeasureHistoMax(void *ans, int ansType,
                     const void *line, int lineType, size_t len,
                     double axmin, double axmax) {
  double val, (*lup)(const void*, size_t);
  size_t ii;

  if (!(AIR_EXISTS(axmin) && AIR_EXISTS(axmax))) {
    axmin = -0.5;
    axmax = len-0.5;
  }
  lup = nrrdDLookup[lineType];
  /* we're using ii-1 as index to avoid wrap-around with size_t index */
  for (ii=len; ii>0; ii--) {
    if (lup(line, ii-1) > 0) {
      break;
    }
  }
  if (ii==0) {
    nrrdDStore[ansType](ans, AIR_NAN);
    return;
  }
  val = NRRD_CELL_POS(axmin, axmax, len, ii-1);
  nrrdDStore[ansType](ans, val);
}

void
_nrrdMeasureHistoMin(void *ans, int ansType,
                     const void *line, int lineType, size_t len,
                     double axmin, double axmax) {
  double val, (*lup)(const void*, size_t);
  size_t ii;

  if (!(AIR_EXISTS(axmin) && AIR_EXISTS(axmax))) {
    axmin = -0.5;
    axmax = len-0.5;
  }
  lup = nrrdDLookup[lineType];
  for (ii=0; ii<len; ii++) {
    if (lup(line, ii) > 0) {
      break;
    }
  }
  if (ii==len) {
    nrrdDStore[ansType](ans, AIR_NAN);
    return;
  }
  val = NRRD_CELL_POS(axmin, axmax, len, ii);
  nrrdDStore[ansType](ans, val);
}

void (*
nrrdMeasureLine[NRRD_MEASURE_MAX+1])(void *, int,
                                     const void *, int, size_t,
                                     double, double) = {
  _nrrdMeasureUnknown,
  _nrrdMeasureMin,
  _nrrdMeasureMax,
  _nrrdMeasureMean,
  _nrrdMeasureMedian,
  _nrrdMeasureMode,
  _nrrdMeasureProduct,
  _nrrdMeasureSum,
  _nrrdMeasureL1,
  _nrrdMeasureL2,
  _nrrdMeasureNormalizedL2,
  _nrrdMeasureRootMeanSquare,
  _nrrdMeasureLinf,
  _nrrdMeasureVariance,
  _nrrdMeasureSD,
  _nrrdMeasureCoV,
  _nrrdMeasureSkew,
  _nrrdMeasureLineSlope,
  _nrrdMeasureLineIntercept,
  _nrrdMeasureLineError,
  _nrrdMeasureHistoMin,
  _nrrdMeasureHistoMax,
  _nrrdMeasureHistoMean,
  _nrrdMeasureHistoMedian,
  _nrrdMeasureHistoMode,
  _nrrdMeasureHistoProduct,
  _nrrdMeasureHistoSum,
  _nrrdMeasureHistoL2,
  _nrrdMeasureHistoVariance,
  _nrrdMeasureHistoSD
};

int
_nrrdMeasureType(const Nrrd *nin, int measr) {
  static const char me[]="_nrrdMeasureType";
  int type=nrrdTypeUnknown;

  switch(measr) {
  case nrrdMeasureMin:
  case nrrdMeasureMax:
  case nrrdMeasureMedian:
  case nrrdMeasureMode:
    type = nin->type;
    break;
  case nrrdMeasureMean:
    /* the rational for this is that if you're after the average value
       along a scanline, you probably want it in the same format as
       what you started with, and if you really want an exact answer
       than you can always use nrrdMeasureSum and then divide.  This may
       well be bone-headed, so is subject to change */
    type = nin->type;
    break;
  case nrrdMeasureProduct:
  case nrrdMeasureSum:
  case nrrdMeasureL1:
  case nrrdMeasureL2:
  case nrrdMeasureNormalizedL2:
  case nrrdMeasureRootMeanSquare:
  case nrrdMeasureLinf:
  case nrrdMeasureVariance:
  case nrrdMeasureSD:
  case nrrdMeasureCoV:
  case nrrdMeasureSkew:
  case nrrdMeasureLineSlope:
  case nrrdMeasureLineIntercept:
  case nrrdMeasureLineError:
    type = nrrdStateMeasureType;
    break;
  case nrrdMeasureHistoMin:
  case nrrdMeasureHistoMax:
  case nrrdMeasureHistoProduct:
  case nrrdMeasureHistoSum:
  case nrrdMeasureHistoL2:
  case nrrdMeasureHistoMean:
  case nrrdMeasureHistoMedian:
  case nrrdMeasureHistoMode:
  case nrrdMeasureHistoVariance:
  case nrrdMeasureHistoSD:
    /* We (currently) don't keep track of the type of the original
       values which generated the histogram, and we may not even
       have access to that information.  So we end up choosing one
       type for all these histogram-based measures */
    type = nrrdStateMeasureHistoType;
    break;
  default:
    fprintf(stderr, "%s: PANIC: type %d not handled\n", me, type);
    exit(1);
  }

  return type;
}

int
nrrdProject(Nrrd *nout, const Nrrd *cnin, unsigned int axis,
            int measr, int type) {
  static const char me[]="nrrdProject", func[]="project";
  int iType, oType, axmap[NRRD_DIM_MAX];
  unsigned int ai, ei;
  size_t iElSz, oElSz, iSize[NRRD_DIM_MAX], oSize[NRRD_DIM_MAX], linLen,
    rowIdx, rowNum, colIdx, colNum, colStep;
  const char *ptr, *iData;
  char *oData, *line;
  double axmin, axmax;
  Nrrd *nin;
  airArray *mop;

  if (!(cnin && nout)) {
    biffAddf(NRRD, "%s: got NULL pointer", me);
    return 1;
  }
  if (nout == cnin) {
    biffAddf(NRRD, "%s: nout==nin disallowed", me);
    return 1;
  }
  if (nrrdTypeBlock == cnin->type) {
    biffAddf(NRRD, "%s: can't project nrrd type %s", me,
             airEnumStr(nrrdType, nrrdTypeBlock));
    return 1;
  }
  if (!AIR_IN_OP(nrrdMeasureUnknown, measr, nrrdMeasureLast)) {
    biffAddf(NRRD, "%s: measure %d not recognized", me, measr);
    return 1;
  }
  if (1 == cnin->dim) {
    if (0 != axis) {
      biffAddf(NRRD, "%s: axis must be 0, not %u, for 1-D array", me, axis);
      return 1;
    }
  } else {
    if (!( axis <= cnin->dim-1 )) {
      biffAddf(NRRD, "%s: axis %u not in range [0,%d]", me, axis, cnin->dim-1);
      return 1;
    }
  }
  if (nrrdTypeDefault != type) {
    if (!( AIR_IN_OP(nrrdTypeUnknown, type, nrrdTypeLast) )) {
      biffAddf(NRRD, "%s: got invalid target type %d", me, type);
      return 1;
    }
  }

  mop = airMopNew();
  if (1 == cnin->dim) {
    /* There are more efficient ways of dealing with this case; this way is
       easy to implement because it leaves most of the established code below
       only superficially changed; uniformly replacing nin with (nin ? nin :
       cnin), even if pointlessly so; this expression that can't be assigned
       to a new variable because of the difference in const. */
    nin = nrrdNew();
    airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdAxesInsert(nin, cnin, 1)) {
      biffAddf(NRRD, "%s: trouble inserting axis on 1-D array", me);
      airMopError(mop); return 1;
    }
  } else {
    nin = NULL;
  }

  iType = (nin ? nin : cnin)->type;
  oType = (nrrdTypeDefault != type
           ? type
           : _nrrdMeasureType((nin ? nin : cnin), measr));
  iElSz = nrrdTypeSize[iType];
  oElSz = nrrdTypeSize[oType];
  nrrdAxisInfoGet_nva((nin ? nin : cnin), nrrdAxisInfoSize, iSize);
  colNum = rowNum = 1;
  for (ai=0; ai<(nin ? nin : cnin)->dim; ai++) {
    if (ai < axis) {
      colNum *= iSize[ai];
    } else if (ai > axis) {
      rowNum *= iSize[ai];
    }
  }
  linLen = iSize[axis];
  colStep = linLen*colNum;
  for (ai=0; ai<=(nin ? nin : cnin)->dim-2; ai++) {
    axmap[ai] = ai + (ai >= axis);
  }
  for (ai=0; ai<=(nin ? nin : cnin)->dim-2; ai++) {
    oSize[ai] = iSize[axmap[ai]];
  }
  if (nrrdMaybeAlloc_nva(nout, oType, (nin ? nin : cnin)->dim-1, oSize)) {
    biffAddf(NRRD, "%s: failed to create output", me);
    airMopError(mop); return 1;
  }

  /* allocate a scanline buffer */
  if (!(line = AIR_CALLOC(linLen*iElSz, char))) {
    char stmp1[AIR_STRLEN_SMALL], stmp2[AIR_STRLEN_SMALL];
    biffAddf(NRRD, "%s: couldn't calloc(%s,%s) scanline buffer", me,
             airSprintSize_t(stmp1, linLen),
             airSprintSize_t(stmp2, iElSz));
    airMopError(mop); return 1;
  }
  airMopAdd(mop, line, airFree, airMopAlways);

  /* the skinny */
  axmin = (nin ? nin : cnin)->axis[axis].min;
  axmax = (nin ? nin : cnin)->axis[axis].max;
  iData = AIR_CAST(char *, (nin ? nin : cnin)->data);
  oData = AIR_CAST(char *, nout->data);
  for (rowIdx=0; rowIdx<rowNum; rowIdx++) {
    for (colIdx=0; colIdx<colNum; colIdx++) {
      ptr = iData + iElSz*(colIdx + rowIdx*colStep);
      for (ei=0; ei<linLen; ei++) {
        memcpy(line + ei*iElSz, ptr + ei*iElSz*colNum, iElSz);
      }
      nrrdMeasureLine[measr](oData, oType, line, iType, linLen,
                             axmin, axmax);
      oData += oElSz;
    }
  }

  /* copy the peripheral information */
  if (nrrdAxisInfoCopy(nout, (nin ? nin : cnin), axmap, NRRD_AXIS_INFO_NONE)) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  if (nrrdContentSet_va(nout, func, cnin /* hide possible axinsert */,
                        "%d,%s", axis, airEnumStr(nrrdMeasure, measr))) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }
  /* this will copy the space origin over directly, which is reasonable */
  if (nrrdBasicInfoCopy(nout, (nin ? nin : cnin),
                        NRRD_BASIC_INFO_DATA_BIT
                        | NRRD_BASIC_INFO_TYPE_BIT
                        | NRRD_BASIC_INFO_BLOCKSIZE_BIT
                        | NRRD_BASIC_INFO_DIMENSION_BIT
                        | NRRD_BASIC_INFO_CONTENT_BIT
                        | NRRD_BASIC_INFO_COMMENTS_BIT
                        | (nrrdStateKeyValuePairsPropagate
                           ? 0
                           : NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT))) {
    biffAddf(NRRD, "%s:", me);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
