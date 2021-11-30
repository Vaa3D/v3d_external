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

#include "ten.h"
#include "privateTen.h"

static int
_experAlloc(tenExperSpec* espec, unsigned int num) {
  static char me[]="_experAlloc";

  airFree(espec->bval); espec->bval = NULL;
  airFree(espec->grad); espec->grad = NULL;
  /* espec->wght = airFree(espec->wght); */
  if (!num) {
    biffAddf(TEN, "%s: need a non-zero number of images", me);
    return 1;
  }
  espec->imgNum = num;
  espec->bval = AIR_CALLOC(num, double);
  espec->grad = AIR_CALLOC(3*num, double);
  /* espec->wght = AIR_CALLOC(num, double); */
  if (!( espec->bval && espec->grad /* && espec->wght */ )) {
    biffAddf(TEN, "%s: couldn't allocate for %u images", me, num);
    return 1;
  }
  return 0;
}

tenExperSpec*
tenExperSpecNew(void) {
  tenExperSpec* espec;

  espec = AIR_CALLOC(1, tenExperSpec);
  espec->set = AIR_FALSE;
  espec->imgNum = 0;
  espec->bval = NULL;
  espec->grad = NULL;
  /* espec->wght = NULL; */
  return espec;
}

int
tenExperSpecGradSingleBValSet(tenExperSpec *espec,
                              int insertB0,
                              double bval,
                              const double *grad,
                              unsigned int gradNum) {
  static const char me[]="tenExperSpecGradSingleBValSet";
  unsigned int ii, imgNum, ei;

  if (!espec) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (insertB0 && !ELL_3V_LEN(grad + 3*0)) {
    biffAddf(TEN, "%s: wanted insertB0 but gradients "
             "already start with (0,0,0)", me);
    return 1;
  }
  imgNum = gradNum + !!insertB0;
  if (_experAlloc(espec, imgNum)) {
    biffAddf(TEN, "%s: couldn't allocate", me);
    return 1;
  }
  if (insertB0) {
    espec->bval[0] = 0;
    ELL_3V_SET(espec->grad + 3*0, 1, 0, 0);
    ei = 1;
  } else {
    ei = 0;
  }
  for (ii=0; ii<gradNum; ei++, ii++) {
    espec->bval[ei] = bval;
    ELL_3V_COPY(espec->grad + 3*ei, grad + 3*ii);
    /* espec->wght[ii] = 1.0; */
  }

  return 0;
}

int
tenExperSpecGradBValSet(tenExperSpec *espec,
                        int insertB0,
                        const double *bval,
                        const double *grad,
                        unsigned int bgNum) {
  static const char me[]="tenExperSpecGradBValSet";
  unsigned int ii, imgNum, ei;

  if (!espec) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (insertB0 && (!ELL_3V_LEN(grad + 3*0) || !bval[0])) {
    biffAddf(TEN, "%s: wanted insertB0 but gradients "
             "already start with (0,0,0) or bvals start with 0", me);
    return 1;
  }
  imgNum = bgNum + !!insertB0;
  if (_experAlloc(espec, imgNum)) {
    biffAddf(TEN, "%s: couldn't allocate", me);
    return 1;
  }
  if (insertB0) {
    espec->bval[0] = 0;
    ELL_3V_SET(espec->grad + 3*0, 0, 0, 0);
    ei = 1;
  } else {
    ei = 0;
  }
  for (ii=0; ii<bgNum; ei++, ii++) {
    espec->bval[ei] = bval[ii];
    ELL_3V_COPY(espec->grad + 3*ei, grad + 3*ii);
    /* espec->wght[ii] = 1.0; */
  }

  return 0;
}

/*
int
tenExperSpecGradBValWghtSet(tenExperSpec *espec,
                            unsigned int imgNum,
                            const double *bval,
                            const double *grad,
                            const double *wght) {
  static const char me[]="tenExperSpecGradBValWghtSet";
  unsigned int ii;

  if (!espec) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (_experAlloc(espec, imgNum)) {
    biffAddf(TEN, "%s: couldn't allocate", me);
    return 1;
  }
  for (ii=0; ii<imgNum; ii++) {
    espec->bval[ii] = bval[ii];
    ELL_3V_COPY(espec->grad + 3*ii, grad + 3*ii);
    espec->wght[ii] = wght[ii];
  }

  return 0;
}
*/

int
tenExperSpecFromKeyValueSet(tenExperSpec *espec, const Nrrd *ndwi) {
  static const char me[]="tenExperSpecFromKeyValueSet";
  unsigned int *skip, skipNum, ii, imgNum, dwiax;
  Nrrd *ngrad, *nbmat;
  airArray *mop;
  double len, singleBval, *bval, *grad;

  for (dwiax=0; dwiax<ndwi->dim; dwiax++) {
    if (nrrdKindList == ndwi->axis[dwiax].kind
        || nrrdKindVector == ndwi->axis[dwiax].kind) {
      break;
    }
  }
  if (ndwi->dim == dwiax) {
    biffAddf(TEN, "%s: need dwis to have a kind %s or %s axis", me,
             airEnumStr(nrrdKind, nrrdKindList),
             airEnumStr(nrrdKind, nrrdKindVector));
    return 1;
  } else {
    if (0 != dwiax) {
      biffAddf(TEN, "%s: need dwis (kind %s or %s) along axis 0, not %u", me,
               airEnumStr(nrrdKind, nrrdKindList),
               airEnumStr(nrrdKind, nrrdKindVector), dwiax);
      return 1;
    }
  }
  for (ii=dwiax+1; ii<ndwi->dim; ii++) {
    if (nrrdKindList == ndwi->axis[ii].kind
        || nrrdKindVector == ndwi->axis[ii].kind) {
      break;
    }
  }
  if (ii < ndwi->dim) {
    biffAddf(TEN, "%s: saw on %u another %s or %s kind axis, after 0", me,
             ii, airEnumStr(nrrdKind, nrrdKindList),
             airEnumStr(nrrdKind, nrrdKindVector));
    return 1;
  }
  if (tenDWMRIKeyValueParse(&ngrad, &nbmat, &singleBval,
                            &skip, &skipNum, ndwi)) {
    biffAddf(TEN, "%s: trouble parsing DWI info from key/value pairs", me);
    return 1;
  }
  mop = airMopNew();
  if (ngrad) {
    airMopAdd(mop, ngrad, (airMopper)nrrdNuke, airMopAlways);
  }
  if (nbmat) {
    airMopAdd(mop, nbmat, (airMopper)nrrdNuke, airMopAlways);
  }
  if (skip) {
    airMopAdd(mop, skip, airFree, airMopAlways);
  }

  if (nbmat) {
    biffAddf(TEN, "%s: sorry, currently can't handle B-matrices here", me);
    airMopError(mop); return 1;
  }
  if (skipNum) {
    biffAddf(TEN, "%s: sorry, currently can't handle skipping (%u) here", me,
             skipNum);
    airMopError(mop); return 1;
  }

  imgNum = ngrad->axis[1].size;
  bval = AIR_CALLOC(imgNum, double);
  airMopAdd(mop, bval, airFree, airMopAlways);
  grad = AIR_CAST(double *, ngrad->data);
  for (ii=0; ii<imgNum; ii++) {
    len = ELL_3V_LEN(grad + 3*ii);
    bval[ii] = singleBval*len*len;
    if (len) {
      ELL_3V_SCALE(grad + 3*ii, 1/len, grad + 3*ii);
    } else {
      ELL_3V_SET(grad + 3*ii, 0, 0, -1);
    }
  }
  if (tenExperSpecGradBValSet(espec, AIR_FALSE, bval, grad, imgNum)) {
    biffAddf(TEN, "%s: trouble", me);
    airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

tenExperSpec*
tenExperSpecNix(tenExperSpec *espec) {

  if (espec) {
    airFree(espec->bval);
    airFree(espec->grad);
    /* espec->wght = airFree(espec->wght); */
    airFree(espec);
  }
  return NULL;
}

double
_tenExperSpec_sqe(const double *dwiMeas, const double *dwiSim,
                  const tenExperSpec *espec, int knownB0) {
  unsigned int ii;
  double sqe;

  sqe = 0;
  if (knownB0) {
    for (ii=0; ii<espec->imgNum; ii++) {
      double dd;
      if (!espec->bval[ii]) {
        continue;
      }
      dd = dwiMeas[ii] - dwiSim[ii];
      sqe += dd*dd;
      /*
      fprintf(stderr, "!%s: dwi[%u]: %g - %g -> %g\n", "_tenExperSpec_sqe",
              ii, dwiMeas[ii], dwiSim[ii], sqe);
      */
    }
  } else {
    for (ii=0; ii<espec->imgNum; ii++) {
      double dd;
      dd = dwiMeas[ii] - dwiSim[ii];
      sqe += dd*dd;
    }
  }
  return sqe;
}

double
_tenExperSpec_nll(const double *dwiMeas, const double *dwiSim,
                  const tenExperSpec *espec,
                  int rician, double sigma, int knownB0) {
  double nll;
  unsigned int ii;

  nll = 0;
  if (rician) {
    for (ii=0; ii<espec->imgNum; ii++) {
      if (knownB0 && !espec->bval[ii]) {
        continue;
      }
      nll += -airLogRician(dwiMeas[ii], dwiSim[ii], sigma);
    }
  } else {
    double dd, ladd, denom;
    ladd = log(sigma*sqrt(2*AIR_PI));
    denom = 1.0/(2*sigma*sigma);
    for (ii=0; ii<espec->imgNum; ii++) {
      if (knownB0 && !espec->bval[ii]) {
        continue;
      }
      dd = dwiMeas[ii] - dwiSim[ii];
      nll += dd*dd*denom + ladd;
    }
  }
  return nll;
}

int
tenDWMRIKeyValueFromExperSpecSet(Nrrd *ndwi, const tenExperSpec *espec) {
  static char me[]="tenDWMRIKeyValueFromExperSpecSet";
  char keystr[AIR_STRLEN_MED], valstr[AIR_STRLEN_MED];
  double maxb, bb;
  unsigned int ii;

  if (!(ndwi && espec)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }

  nrrdKeyValueAdd(ndwi, tenDWMRIModalityKey, tenDWMRIModalityVal);
  maxb = tenExperSpecMaxBGet(espec);
  sprintf(valstr, "%.17g", maxb);
  nrrdKeyValueAdd(ndwi, tenDWMRIBValueKey, valstr);
  for (ii=0; ii<espec->imgNum; ii++) {
    double vec[3];
    sprintf(keystr, tenDWMRIGradKeyFmt, ii);
    ELL_3V_COPY(vec, espec->grad + 3*ii);
    bb = espec->bval[ii];
    /* Thu Dec 20 03:25:20 CST 2012 this rescaling is not, btw,
       what is causing the small discrepency between ngrad before
       and after saving to KVPs */
    ELL_3V_SCALE(vec, sqrt(bb/maxb), vec);
    sprintf(valstr, "%.17g %.17g %.17g", vec[0], vec[1], vec[2]);
    nrrdKeyValueAdd(ndwi, keystr, valstr);
  }
  /* HEY what if its a full B-matrix? */

  return 0;
}

/*
** learns B0 from DWIs by simple averaging of all the dwi[ii]
** without any diffusion weighting, as indicated by espec->bval[ii],
** or, returns AIR_NAN when there are no such dwi[ii]
*/
double
tenExperSpecKnownB0Get(const tenExperSpec *espec, const double *dwi) {
  unsigned int ii, nb;
  double ret, b0;

  if (!( dwi && espec )) {
    return AIR_NAN;
  }

  nb = 0;
  b0 = 0.0;
  for (ii=0; ii<espec->imgNum; ii++) {
    if (0 == espec->bval[ii]) {
      b0 += dwi[ii];
      ++nb;
    }
  }
  if (nb) {
    ret = b0/nb;
  } else {
    ret = AIR_NAN;
  }
  return ret;
}

double
tenExperSpecMaxBGet(const tenExperSpec *espec) {
  unsigned int ii;
  double bval;

  if (!( espec )) {
    return AIR_NAN;
  }

  bval = -1;
  for (ii=0; ii<espec->imgNum; ii++) {
    bval = AIR_MAX(bval, espec->bval[ii]);
  }
  return bval;
}
