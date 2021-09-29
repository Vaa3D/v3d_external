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

const char *
tenDWMRIModalityKey = "modality";

const char *
tenDWMRIModalityVal = "DWMRI";

const char *
tenDWMRINAVal = "n/a";

const char *
tenDWMRIBValueKey = "DWMRI_b-value";

const char *
tenDWMRIGradKeyFmt = "DWMRI_gradient_%04u";

const char *
tenDWMRIBmatKeyFmt = "DWMRI_B-matrix_%04u";

const char *
tenDWMRINexKeyFmt = "DWMRI_NEX_%04u";

const char *
tenDWMRISkipKeyFmt = "DWMRI_skip_%04u";

/*
******** tenDWMRIKeyValueParse
**
** Parses through key-value pairs in the NRRD header to determine the
** list of diffusion-sensitizing gradient directions, or B-matrices
** (depending to what was found), according the NAMIC conventions.
** This requires, among other things, that ndwi be have exactly one
** axis with kind nrrdKindList (or nrrdKindVector), which is taken to
** be the DWI axis.
**
** Either *ngradP or *nbmatP is set to a newly- allocated nrrd
** containing this information, and the other one is set to NULL
** The (scalar) b-value is stored in *bP. The image values that are
** to be skipped are stored in the *skipP array (allocated here),
** the length of that array is stored in *skipNumP.  Unlike the skip
** array used internally with tenEstimate, this is just a simple 1-D
** array; it is not a list of pairs of (index,skipBool).
*/
int
tenDWMRIKeyValueParse(Nrrd **ngradP, Nrrd **nbmatP, double *bP,
                      unsigned int **skipP, unsigned int *skipNumP,
                      const Nrrd *ndwi) {
  static const char me[]="tenDWMRIKeyValueParse";
  char tmpKey[AIR_STRLEN_MED],
    key[AIR_STRLEN_MED], *val;
  const char *keyFmt;
  int dwiAxis;
  unsigned int axi, dwiIdx, dwiNum, valNum, valIdx, parsedNum,
    nexNum, nexIdx, skipIdx, *skipLut;
  Nrrd *ninfo;
  double *info, normMax, norm;
  airArray *mop, *skipArr;

  if (!( ngradP && nbmatP && skipP && skipNumP && bP && ndwi )) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }

  /* check modality */
  val = nrrdKeyValueGet(ndwi, tenDWMRIModalityKey);
  if (!val) {
    biffAddf(TEN, "%s: didn't have \"%s\" key", me, tenDWMRIModalityKey);
    return 1;
  }
  if (strncmp(tenDWMRIModalityVal, val + strspn(val, AIR_WHITESPACE),
              strlen(tenDWMRIModalityVal))) {
    biffAddf(TEN, "%s: \"%s\" value was \"%s\", not \"%s\"", me,
             tenDWMRIModalityKey, val, tenDWMRIModalityVal);
    return 1;
  }
  val = (char *)airFree(val);

  /* learn b-value */
  val = nrrdKeyValueGet(ndwi, tenDWMRIBValueKey);
  if (!val) {
    biffAddf(TEN, "%s: didn't have \"%s\" key", me, tenDWMRIBValueKey);
    return 1;
  }
  if (1 != sscanf(val, "%lg", bP)) {
    biffAddf(TEN, "%s: couldn't parse float from value \"%s\" "
             "for key \"%s\"", me,
             val, tenDWMRIBValueKey);
    return 1;
  }
  val = (char *)airFree(val);

  /* find single DWI axis, set dwiNum to its size */
  dwiAxis = -1;
  for (axi=0; axi<ndwi->dim; axi++) {
    /* the use of nrrdKindVector here is out of deference to how ITK's
       itkNrrdImageIO.cxx uses nrrdKindVector for VECTOR pixels */
    if (nrrdKindList == ndwi->axis[axi].kind
        || nrrdKindVector == ndwi->axis[axi].kind) {
      if (-1 != dwiAxis) {
        biffAddf(TEN, "%s: already saw %s or %s kind on axis %d", me,
                 airEnumStr(nrrdKind, nrrdKindList),
                 airEnumStr(nrrdKind, nrrdKindVector), dwiAxis);
        return 1;
      }
      dwiAxis = axi;
    }
  }
  if (-1 == dwiAxis) {
    biffAddf(TEN, "%s: did not see \"%s\" kind on any axis", me,
             airEnumStr(nrrdKind, nrrdKindList));
    return 1;
  }
  dwiNum = ndwi->axis[dwiAxis].size;

  /* figure out if we're parsing gradients or b-matrices */
  sprintf(tmpKey, tenDWMRIGradKeyFmt, 0);
  val = nrrdKeyValueGet(ndwi, tmpKey);
  if (val) {
    valNum = 3;
  } else {
    valNum = 6;
    sprintf(key, tenDWMRIBmatKeyFmt, 0);
    val = nrrdKeyValueGet(ndwi, key);
    if (!val) {
      biffAddf(TEN, "%s: saw neither \"%s\" nor \"%s\" key", me,
               tmpKey, key);
      return 1;
    }
  }
  val = (char *)airFree(val);

  /* set up parsing and allocate one of output nrrds */
  if (3 == valNum) {
    keyFmt = tenDWMRIGradKeyFmt;
    ninfo = *ngradP = nrrdNew();
    *nbmatP = NULL;
  } else {
    keyFmt = tenDWMRIBmatKeyFmt;
    *ngradP = NULL;
    ninfo = *nbmatP = nrrdNew();
  }
  if (nrrdMaybeAlloc_va(ninfo, nrrdTypeDouble, 2,
                        AIR_CAST(size_t, valNum),
                        AIR_CAST(size_t, dwiNum))) {
    biffMovef(TEN, NRRD, "%s: couldn't allocate output", me);
    return 1;
  }
  info = (double *)(ninfo->data);

  /* set up skip list recording */
  mop = airMopNew();
  skipArr = airArrayNew((void**)skipP, skipNumP, sizeof(unsigned int), 16);
  airMopAdd(mop, skipArr, (airMopper)airArrayNix, airMopAlways);
  skipLut = AIR_CALLOC(dwiNum, unsigned int);
  airMopAdd(mop, skipLut, airFree, airMopAlways);
  if (!skipLut) {
    biffAddf(TEN, "%s: couldn't allocate skip LUT", me);
    airMopError(mop); return 1;
  }

  /* parse values in ninfo */
  for (dwiIdx=0; dwiIdx<dwiNum; dwiIdx++) {
    sprintf(key, keyFmt, dwiIdx);
    val = nrrdKeyValueGet(ndwi, key);
    if (!val) {
      biffAddf(TEN, "%s: didn't see \"%s\" key", me, key);
      airMopError(mop); return 1;
    }
    airToLower(val);
    if (!strncmp(tenDWMRINAVal, val + strspn(val, AIR_WHITESPACE),
                 strlen(tenDWMRINAVal))) {
      /* have no sensible gradient or B-matrix info here, and must skip */
      for (valIdx=0; valIdx<valNum; valIdx++) {
        info[valIdx] = AIR_NAN;
      }
      skipIdx = airArrayLenIncr(skipArr, 1);
      (*skipP)[skipIdx] = dwiIdx;
      skipLut[dwiIdx] = AIR_TRUE;
      /* can't have NEX on a skipped gradient or B-matrix */
      val = (char *)airFree(val);
      sprintf(key, tenDWMRINexKeyFmt, dwiIdx);
      val = nrrdKeyValueGet(ndwi, key);
      if (val) {
        biffAddf(TEN, "%s: can't have NEX of skipped DWI %u", me, skipIdx);
        airMopError(mop); return 1;
      }
      nexNum = 1; /* for "info +=" below */
    } else {
      /* we probably do have sensible gradient or B-matrix info */
      parsedNum = airParseStrD(info, val, AIR_WHITESPACE, valNum);
      if (valNum != parsedNum) {
        biffAddf(TEN, "%s: couldn't parse %d floats in value \"%s\" "
                 "for key \"%s\" (only got %d)",
                 me, valNum, val, key, parsedNum);
        airMopError(mop); return 1;
      }
      val = (char *)airFree(val);
      sprintf(key, tenDWMRINexKeyFmt, dwiIdx);
      val = nrrdKeyValueGet(ndwi, key);
      if (!val) {
        /* there is no NEX indicated */
        nexNum = 1;
      } else {
        if (1 != sscanf(val, "%u", &nexNum)) {
          biffAddf(TEN, "%s: couldn't parse integer in value \"%s\" "
                   "for key \"%s\"", me, val, key);
          airMopError(mop); return 1;
        }
        val = (char *)airFree(val);
        if (!( nexNum >= 1 )) {
          biffAddf(TEN, "%s: NEX (%d) for DWI %d not >= 1",
                   me, nexNum, dwiIdx);
          airMopError(mop); return 1;
        }
        if (!( dwiIdx + nexNum - 1 < dwiNum )) {
          biffAddf(TEN, "%s: NEX %d for DWI %d implies %d DWI > real # DWI %d",
                   me, nexNum, dwiIdx, dwiIdx + nexNum, dwiNum);
          airMopError(mop); return 1;
        }
        for (nexIdx=1; nexIdx<nexNum; nexIdx++) {
          sprintf(key, keyFmt, dwiIdx+nexIdx);
          val = nrrdKeyValueGet(ndwi, key);
          if (val) {
            val = (char *)airFree(val);
            biffAddf(TEN, "%s: shouldn't have key \"%s\" with "
                     "NEX %d for DWI %d", me, key, nexNum, dwiIdx);
            airMopError(mop); return 1;
          }
          for (valIdx=0; valIdx<valNum; valIdx++) {
            info[valIdx + valNum*nexIdx] = info[valIdx];
          }
        }
        dwiIdx += nexNum-1;
      }
    }
    info += valNum*nexNum;
  }

  /* perhaps too paranoid: see if there are extra keys,
     which probably implies confusion/mismatch between number of
     gradients and number of values */
  sprintf(key, keyFmt, dwiIdx);
  val = nrrdKeyValueGet(ndwi, key);
  if (val) {
    biffAddf(TEN, "%s: saw \"%s\" key, more than required %u keys, "
             "likely mismatch between keys and actual gradients",
             me, key, dwiIdx);
    airMopError(mop); return 1;
  }

  /* second pass: see which ones are skipped, even though gradient/B-matrix
     information has been specified */
  for (dwiIdx=0; dwiIdx<dwiNum; dwiIdx++) {
    sprintf(key, tenDWMRISkipKeyFmt, dwiIdx);
    val = nrrdKeyValueGet(ndwi, key);
    if (val) {
      airToLower(val);
      if (!strncmp("true", val + strspn(val, AIR_WHITESPACE),
                   strlen("true"))) {
        skipIdx = airArrayLenIncr(skipArr, 1);
        (*skipP)[skipIdx] = dwiIdx;
        skipLut[dwiIdx] = AIR_TRUE;
      }
    }
  }

  /* normalize so that maximal norm is 1.0 */
  /* Thu Dec 20 03:25:20 CST 2012 this rescaling IS in fact what is
     causing the small discrepency between ngrad before and after
     saving to KVPs. The problem is not related to how the gradient
     vector coefficients are recovered from the string-based
     representation; that is likely bit-for-bit correct.  The problem
     is when everything is rescaled by 1.0/normMax: a "normalized"
     vector will not have *exactly* length 1.0. So what can be done
     to prevent pointlessly altering the lengths of vectors that were
     close enough to unit-length?  Is there some more 754-savvy
     way of doing this normalization? */
  normMax = 0;
  info = (double *)(ninfo->data);
  for (dwiIdx=0; dwiIdx<dwiNum; dwiIdx++) {
    if (!skipLut[dwiIdx]) {
      if (3 == valNum) {
        norm = ELL_3V_LEN(info);
      } else {
        norm = sqrt(info[0]*info[0] + 2*info[1]*info[1] + 2*info[2]*info[2]
                                    +   info[3]*info[3] + 2*info[4]*info[4]
                                                        +   info[5]*info[5]);
      }
      normMax = AIR_MAX(normMax, norm);
    }
    info += valNum;
  }
  info = (double *)(ninfo->data);
  for (dwiIdx=0; dwiIdx<dwiNum; dwiIdx++) {
    if (!skipLut[dwiIdx]) {
      if (3 == valNum) {
        ELL_3V_SCALE(info, 1.0/normMax, info);
      } else {
        ELL_6V_SCALE(info, 1.0/normMax, info);
      }
    }
    info += valNum;
  }

  airMopOkay(mop);
  return 0;
}

/*
******** tenBMatrixCalc
**
** given a list of gradient directions (arbitrary type), contructs the
** B-matrix that records how each coefficient of the diffusion tensor
** is weighted in the diffusion weighted images.  Matrix will be a
** 6-by-N 2D array of doubles.
**
** NOTE 1: The ordering of the elements in each row is (like the ordering
** of the tensor elements in all of ten):
**
**    Bxx  Bxy  Bxz   Byy  Byz   Bzz
**
** NOTE 2: The off-diagonal elements are NOT pre-multiplied by two.
*/
int
tenBMatrixCalc(Nrrd *nbmat, const Nrrd *_ngrad) {
  static const char me[]="tenBMatrixCalc";
  Nrrd *ngrad;
  double *bmat, *G;
  int DD, dd;
  airArray *mop;

  if (!(nbmat && _ngrad && !tenGradientCheck(_ngrad, nrrdTypeDefault, 1))) {
    biffAddf(TEN, "%s: got NULL pointer or invalid arg", me);
    return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, ngrad=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(ngrad, _ngrad, nrrdTypeDouble)
      || nrrdMaybeAlloc_va(nbmat, nrrdTypeDouble, 2,
                           AIR_CAST(size_t, 6), ngrad->axis[1].size)) {
    biffMovef(TEN, NRRD, "%s: trouble", me);
    airMopError(mop); return 1;
  }

  DD = ngrad->axis[1].size;
  G = (double*)(ngrad->data);
  bmat = (double*)(nbmat->data);
  for (dd=0; dd<DD; dd++) {
    ELL_6V_SET(bmat,
               G[0]*G[0], G[0]*G[1], G[0]*G[2],
                          G[1]*G[1], G[1]*G[2],
                                     G[2]*G[2]);
    G += 3;
    bmat += 6;
  }
  nbmat->axis[0].kind = nrrdKind3DSymMatrix;

  airMopOkay(mop);
  return 0;
}

/*
******** tenEMatrixCalc
**
*/
int
tenEMatrixCalc(Nrrd *nemat, const Nrrd *_nbmat, int knownB0) {
  static const char me[]="tenEMatrixCalc";
  Nrrd *nbmat, *ntmp;
  airArray *mop;
  ptrdiff_t padmin[2], padmax[2];
  unsigned int ri;
  double *bmat;

  if (!(nemat && _nbmat)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (tenBMatrixCheck(_nbmat, nrrdTypeDefault, 6)) {
    biffAddf(TEN, "%s: problem with B matrix", me);
    return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, nbmat=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (knownB0) {
    if (nrrdConvert(nbmat, _nbmat, nrrdTypeDouble)) {
      biffMovef(TEN, NRRD, "%s: couldn't convert given bmat to doubles", me);
      airMopError(mop); return 1;
    }
  } else {
    airMopAdd(mop, ntmp=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    if (nrrdConvert(ntmp, _nbmat, nrrdTypeDouble)) {
      biffMovef(TEN, NRRD, "%s: couldn't convert given bmat to doubles", me);
      airMopError(mop); return 1;
    }
    ELL_2V_SET(padmin, 0, 0);
    ELL_2V_SET(padmax, 6, _nbmat->axis[1].size-1);
    if (nrrdPad_nva(nbmat, ntmp, padmin, padmax, nrrdBoundaryPad, -1)) {
      biffMovef(TEN, NRRD, "%s: couldn't pad given bmat", me);
      airMopError(mop); return 1;
    }
  }
  bmat = (double*)(nbmat->data);
  /* HERE is where the off-diagonal elements get multiplied by 2 */
  for (ri=0; ri<nbmat->axis[1].size; ri++) {
    bmat[1] *= 2;
    bmat[2] *= 2;
    bmat[4] *= 2;
    bmat += nbmat->axis[0].size;
  }
  if (ell_Nm_pseudo_inv(nemat, nbmat)) {
    biffMovef(TEN, ELL, "%s: trouble pseudo-inverting B-matrix", me);
    airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

/*
******** tenEstimateLinearSingle_d
**
** estimate one single tensor
**
** !! requires being passed a pre-allocated double array "vbuf" which is
** !! used for intermediate calculations (details below)
**
** DD is always the length of the dwi[] array
**
** -------------- IF knownB0 -------------------------
** input:
** dwi[0] is the B0 image, dwi[1]..dwi[DD-1] are the (DD-1) DWI values,
** emat is the (DD-1)-by-6 estimation matrix, which is the pseudo-inverse
** of the B-matrix (after the off-diagonals have been multiplied by 2).
** vbuf[] is allocated for (at least) DD-1 doubles (DD is fine)
**
** output:
** ten[0]..ten[6] will be the confidence value followed by the tensor
** if B0P, then *B0P is set to the B0 value used in calcs: max(b0,1)
** -------------- IF !knownB0 -------------------------
** input:
** dwi[0]..dwi[DD-1] are the DD DWI values, emat is the DD-by-7 estimation
** matrix.  The 7th column is for estimating the B0 image.
** vbuf[] is allocated for DD doubles
**
** output:
** ten[0]..ten[6] will be the confidence value followed by the tensor
** if B0P, then *B0P is set to estimated B0 value.
** ----------------------------------------------------
*/
void
tenEstimateLinearSingle_d(double *ten, double *B0P,              /* output */
                          const double *dwi, const double *emat, /* input .. */
                          double *vbuf, unsigned int DD, int knownB0,
                          double thresh, double soft, double b) {
  double logB0, tmp, mean;
  unsigned int ii, jj;
  /* static const char me[]="tenEstimateLinearSingle_d"; */

  if (knownB0) {
    if (B0P) {
      /* we save this as a courtesy */
      *B0P = AIR_MAX(dwi[0], 1);
    }
    logB0 = log(AIR_MAX(dwi[0], 1));
    mean = 0;
    for (ii=1; ii<DD; ii++) {
      tmp = AIR_MAX(dwi[ii], 1);
      mean += tmp;
      vbuf[ii-1] = (logB0 - log(tmp))/b;
      /* if (tenVerbose) {
        fprintf(stderr, "vbuf[%d] = %f\n", ii-1, vbuf[ii-1]);
      } */
    }
    mean /= DD-1;
    if (soft) {
      ten[0] = AIR_AFFINE(-1, airErf((mean - thresh)/(soft + 0.000001)), 1,
                          0, 1);
    } else {
      ten[0] = mean > thresh;
    }
    for (jj=0; jj<6; jj++) {
      tmp = 0;
      for (ii=0; ii<DD-1; ii++) {
        tmp += emat[ii + (DD-1)*jj]*vbuf[ii];
      }
      ten[jj+1] = tmp;
    }
  } else {
    /* !knownB0 */
    mean = 0;
    for (ii=0; ii<DD; ii++) {
      tmp = AIR_MAX(dwi[ii], 1);
      mean += tmp;
      vbuf[ii] = -log(tmp)/b;
    }
    mean /= DD;
    if (soft) {
      ten[0] = AIR_AFFINE(-1, airErf((mean - thresh)/(soft + 0.000001)), 1,
                          0, 1);
    } else {
      ten[0] = mean > thresh;
    }
    for (jj=0; jj<7; jj++) {
      tmp = 0;
      for (ii=0; ii<DD; ii++) {
        tmp += emat[ii + DD*jj]*vbuf[ii];
      }
      if (jj < 6) {
        ten[jj+1] = tmp;
      } else {
        /* we're on seventh row, for finding B0 */
        if (B0P) {
          *B0P = exp(b*tmp);
        }
      }
    }
  }
  return;
}

void
tenEstimateLinearSingle_f(float *_ten, float *_B0P,              /* output */
                          const float *_dwi, const double *emat, /* input .. */
                          double *vbuf, unsigned int DD, int knownB0,
                          float thresh, float soft, float b) {
  static const char me[]="tenEstimateLinearSingle_f";
#define DWI_NUM_MAX 256
  double dwi[DWI_NUM_MAX], ten[7], B0;
  unsigned int dwiIdx;

  /* HEY: this is somewhat inelegant .. */
  if (DD > DWI_NUM_MAX) {
    fprintf(stderr, "%s: PANIC: sorry, DD=%u > compile-time DWI_NUM_MAX=%u\n",
            me, DD, DWI_NUM_MAX);
    exit(1);
  }
  for (dwiIdx=0; dwiIdx<DD; dwiIdx++) {
    dwi[dwiIdx] = _dwi[dwiIdx];
  }
  tenEstimateLinearSingle_d(ten, _B0P ? &B0 : NULL,
                            dwi, emat,
                            vbuf, DD, knownB0,
                            thresh, soft, b);
  TEN_T_COPY_TT(_ten, float, ten);
  if (_B0P) {
    *_B0P = AIR_CAST(float, B0);
  }
  return;
}

/*
******** tenEstimateLinear3D
**
** takes an array of DWIs (starting with the B=0 image), joins them up,
** and passes it all off to tenEstimateLinear4D
**
** Note: this will copy per-axis peripheral information from _ndwi[0]
*/
int
tenEstimateLinear3D(Nrrd *nten, Nrrd **nterrP, Nrrd **nB0P,
                    const Nrrd *const *_ndwi, unsigned int dwiLen,
                    const Nrrd *_nbmat, int knownB0,
                    double thresh, double soft, double b) {
  static const char me[]="tenEstimateLinear3D";
  Nrrd *ndwi;
  airArray *mop;
  int amap[4] = {-1, 0, 1, 2};

  if (!(_ndwi)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  mop = airMopNew();
  ndwi = nrrdNew();
  airMopAdd(mop, ndwi, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdJoin(ndwi, (const Nrrd*const*)_ndwi, dwiLen, 0, AIR_TRUE)) {
    biffMovef(TEN, NRRD, "%s: trouble joining inputs", me);
    airMopError(mop); return 1;
  }

  nrrdAxisInfoCopy(ndwi, _ndwi[0], amap, NRRD_AXIS_INFO_NONE);
  if (tenEstimateLinear4D(nten, nterrP, nB0P,
                          ndwi, _nbmat, knownB0, thresh, soft, b)) {
    biffAddf(TEN, "%s: trouble", me);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}

/*
******** tenEstimateLinear4D
**
** given a stack of DWI volumes (ndwi) and the imaging B-matrix used
** for acquisiton (_nbmat), computes and stores diffusion tensors in
** nten.
**
** The mean of the diffusion-weighted images is thresholded at "thresh" with
** softness parameter "soft".
**
** This takes the B-matrix (weighting matrix), such as formed by tenBMatrix,
** or from a more complete account of the gradients present in an imaging
** sequence, and then does the pseudo inverse to get the estimation matrix
*/
int
tenEstimateLinear4D(Nrrd *nten, Nrrd **nterrP, Nrrd **nB0P,
                    const Nrrd *ndwi, const Nrrd *_nbmat, int knownB0,
                    double thresh, double soft, double b) {
  static const char me[]="tenEstimateLinear4D";
  Nrrd *nemat, *nbmat, *ncrop, *nhist;
  airArray *mop;
  size_t cmin[4], cmax[4], sx, sy, sz, II, d, DD;
  int E, amap[4];
  float *ten, *dwi1, *dwi2, *terr,
    _B0, *B0, (*lup)(const void *, size_t);
  double *emat, *bmat, *vbuf;
  NrrdRange *range;
  float te, d1, d2;
  char stmp[2][AIR_STRLEN_SMALL];

  if (!(nten && ndwi && _nbmat)) {
    /* nerrP and _NB0P can be NULL */
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( 4 == ndwi->dim && 7 <= ndwi->axis[0].size )) {
    biffAddf(TEN, "%s: dwi should be 4-D array with axis 0 size >= 7", me);
    return 1;
  }
  if (tenBMatrixCheck(_nbmat, nrrdTypeDefault, 6)) {
    biffAddf(TEN, "%s: problem with B matrix", me);
    return 1;
  }
  if (knownB0) {
    if (!( ndwi->axis[0].size == 1 + _nbmat->axis[1].size )) {
      biffAddf(TEN, "%s: (knownB0 == true) # input images (%s) "
               "!= 1 + # B matrix rows (1+%s)", me,
               airSprintSize_t(stmp[0], ndwi->axis[0].size),
               airSprintSize_t(stmp[1], _nbmat->axis[1].size));
      return 1;
    }
  } else {
    if (!( ndwi->axis[0].size == _nbmat->axis[1].size )) {
      biffAddf(TEN, "%s: (knownB0 == false) # dwi (%s) "
               "!= # B matrix rows (%s)", me,
               airSprintSize_t(stmp[0], ndwi->axis[0].size),
               airSprintSize_t(stmp[1], _nbmat->axis[1].size));
      return 1;
    }
  }

  DD = ndwi->axis[0].size;
  sx = ndwi->axis[1].size;
  sy = ndwi->axis[2].size;
  sz = ndwi->axis[3].size;
  mop = airMopNew();
  airMopAdd(mop, nbmat=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(nbmat, _nbmat, nrrdTypeDouble)) {
    biffMovef(TEN, NRRD, "%s: trouble converting to doubles", me);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, nemat=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (tenEMatrixCalc(nemat, nbmat, knownB0)) {
    biffAddf(TEN, "%s: trouble computing estimation matrix", me);
    airMopError(mop); return 1;
  }
  vbuf = AIR_CALLOC(knownB0 ? DD-1 : DD, double);
  dwi1 = AIR_CALLOC(DD, float);
  dwi2 = AIR_CALLOC(knownB0 ? DD : DD+1, float);
  airMopAdd(mop, vbuf, airFree, airMopAlways);
  airMopAdd(mop, dwi1, airFree, airMopAlways);
  airMopAdd(mop, dwi2, airFree, airMopAlways);
  if (!(vbuf && dwi1 && dwi2)) {
    biffAddf(TEN, "%s: couldn't allocate temp buffers", me);
    airMopError(mop); return 1;
  }
  if (!AIR_EXISTS(thresh)) {
    airMopAdd(mop, ncrop=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, nhist=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    ELL_4V_SET(cmin, knownB0 ? 1 : 0, 0, 0, 0);
    ELL_4V_SET(cmax, DD-1, sx-1, sy-1, sz-1);
    E = 0;
    if (!E) E |= nrrdCrop(ncrop, ndwi, cmin, cmax);
    if (!E) range = nrrdRangeNewSet(ncrop, nrrdBlind8BitRangeState);
    if (!E) airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
    if (!E) E |= nrrdHisto(nhist, ncrop, range, NULL,
                           (int)AIR_MIN(1024, range->max - range->min + 1),
                           nrrdTypeFloat);
    if (E) {
      biffMovef(TEN, NRRD,
                "%s: trouble histograming to find DW threshold", me);
      airMopError(mop); return 1;
    }
    if (_tenFindValley(&thresh, nhist, 0.75, AIR_FALSE)) {
      biffAddf(TEN, "%s: problem finding DW histogram valley", me);
      airMopError(mop); return 1;
    }
    fprintf(stderr, "%s: using %g for DW confidence threshold\n", me, thresh);
  }
  if (nrrdMaybeAlloc_va(nten, nrrdTypeFloat, 4,
                        AIR_CAST(size_t, 7), sx, sy, sz)) {
    biffMovef(TEN, NRRD, "%s: couldn't allocate output", me);
    airMopError(mop); return 1;
  }
  if (nterrP) {
    if (!(*nterrP)) {
      *nterrP = nrrdNew();
    }
    if (nrrdMaybeAlloc_va(*nterrP, nrrdTypeFloat, 3,
                          sx, sy, sz)) {
      biffAddf(NRRD, "%s: couldn't allocate error output", me);
      airMopError(mop); return 1;
    }
    airMopAdd(mop, nterrP, (airMopper)airSetNull, airMopOnError);
    airMopAdd(mop, *nterrP, (airMopper)nrrdNuke, airMopOnError);
    terr = (float*)((*nterrP)->data);
  } else {
    terr = NULL;
  }
  if (nB0P) {
    if (!(*nB0P)) {
      *nB0P = nrrdNew();
    }
    if (nrrdMaybeAlloc_va(*nB0P, nrrdTypeFloat, 3,
                          sx, sy, sz)) {
      biffAddf(NRRD, "%s: couldn't allocate error output", me);
      airMopError(mop); return 1;
    }
    airMopAdd(mop, nB0P, (airMopper)airSetNull, airMopOnError);
    airMopAdd(mop, *nB0P, (airMopper)nrrdNuke, airMopOnError);
    B0 = (float*)((*nB0P)->data);
  } else {
    B0 = NULL;
  }
  bmat = (double*)(nbmat->data);
  emat = (double*)(nemat->data);
  ten = (float*)(nten->data);
  lup = nrrdFLookup[ndwi->type];
  for (II=0; II<sx*sy*sz; II++) {
    /* tenVerbose = (II == 42 + 190*(96 + 196*0)); */
    for (d=0; d<DD; d++) {
      dwi1[d] = lup(ndwi->data, d + DD*II);
      /* if (tenVerbose) {
        fprintf(stderr, "%s: input dwi1[%d] = %g\n", me, d, dwi1[d]);
      } */
    }
    tenEstimateLinearSingle_f(ten, &_B0, dwi1, emat,
                              vbuf, DD, knownB0,
                              AIR_CAST(float, thresh),
                              AIR_CAST(float, soft),
                              AIR_CAST(float, b));
    if (nB0P) {
      *B0 = _B0;
    }
    /* if (tenVerbose) {
      fprintf(stderr, "%s: output ten = (%g) %g,%g,%g  %g,%g  %g\n", me,
              ten[0], ten[1], ten[2], ten[3], ten[4], ten[5], ten[6]);
    } */
    if (nterrP) {
      te = 0;
      if (knownB0) {
        tenSimulateSingle_f(dwi2, _B0, ten, bmat, DD, AIR_CAST(float, b));
        for (d=1; d<DD; d++) {
          d1 = AIR_MAX(dwi1[d], 1);
          d2 = AIR_MAX(dwi2[d], 1);
          te += (d1 - d2)*(d1 - d2);
        }
        te /= (DD-1);
      } else {
        tenSimulateSingle_f(dwi2, _B0, ten, bmat, DD+1, AIR_CAST(float, b));
        for (d=0; d<DD; d++) {
          d1 = AIR_MAX(dwi1[d], 1);
          /* tenSimulateSingle_f always puts the B0 in the beginning of
             the dwi vector, but in this case we didn't have it in
             the input dwi vector */
          d2 = AIR_MAX(dwi2[d+1], 1);
          te += (d1 - d2)*(d1 - d2);
        }
        te /= DD;
      }
      *terr = AIR_CAST(float, sqrt(te));
      terr += 1;
    }
    ten += 7;
    if (B0) {
      B0 += 1;
    }
  }
  /* not our job: tenEigenvalueClamp(nten, nten, 0, AIR_NAN); */

  ELL_4V_SET(amap, -1, 1, 2, 3);
  nrrdAxisInfoCopy(nten, ndwi, amap, NRRD_AXIS_INFO_NONE);
  nten->axis[0].kind = nrrdKind3DMaskedSymMatrix;
  if (nrrdBasicInfoCopy(nten, ndwi,
                        NRRD_BASIC_INFO_ALL ^ NRRD_BASIC_INFO_SPACE)) {
    biffAddf(NRRD, "%s:", me);
    return 1;
  }

  airMopOkay(mop);
  return 0;
}

/*
******** tenSimulateSingle_f
**
** given a tensor, simulate the set of diffusion weighted measurements
** represented by the given B matrix
**
** NOTE: the mindset of this function is very much "knownB0==true":
** B0 is required as an argument (and its always copied to dwi[0]),
** and the given bmat is assumed to have DD-1 rows (similar to how
** tenEstimateLinearSingle_f() is set up), and dwi[1] through dwi[DD-1]
** are set to the calculated DWIs.
**
** So: dwi must be allocated for DD values total
*/
void
tenSimulateSingle_f(float *dwi,
                    float B0, const float *ten, const double *bmat,
                    unsigned int DD, float b) {
  double vv;
  /* this is how we multiply the off-diagonal entries by 2 */
  double matwght[6] = {1, 2, 2, 1, 2, 1};
  unsigned int ii, jj;

  dwi[0] = B0;
  /* if (tenVerbose) {
    fprintf(stderr, "ten = %g,%g,%g  %g,%g  %g\n",
            ten[1], ten[2], ten[3], ten[4], ten[5], ten[6]);
  } */
  for (ii=0; ii<DD-1; ii++) {
    vv = 0;
    for (jj=0; jj<6; jj++) {
      vv += matwght[jj]*bmat[jj + 6*ii]*ten[jj+1];
    }
    dwi[ii+1] = AIR_CAST(float, AIR_MAX(B0, 1)*exp(-b*vv));
    /* if (tenVerbose) {
      fprintf(stderr, "v[%d] = %g --> dwi = %g\n", ii, vv, dwi[ii+1]);
    } */
  }

  return;
}

int
tenSimulate(Nrrd *ndwi, const Nrrd *nT2, const Nrrd *nten,
            const Nrrd *_nbmat, double b) {
  static const char me[]="tenSimulate";
  size_t II;
  Nrrd *nbmat;
  size_t DD, sx, sy, sz;
  airArray *mop;
  double *bmat;
  float *dwi, *ten, (*lup)(const void *, size_t I);
  char stmp[6][AIR_STRLEN_SMALL];

  if (!ndwi || !nT2 || !nten || !_nbmat
      || tenTensorCheck(nten, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)
      || tenBMatrixCheck(_nbmat, nrrdTypeDefault, 6)) {
    biffAddf(TEN, "%s: got NULL pointer or invalid args", me);
    return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, nbmat=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(nbmat, _nbmat, nrrdTypeDouble)) {
    biffMovef(TEN, NRRD, "%s: couldn't convert B matrix", me);
    return 1;
  }

  DD = nbmat->axis[1].size+1;
  sx = nT2->axis[0].size;
  sy = nT2->axis[1].size;
  sz = nT2->axis[2].size;
  if (!(3 == nT2->dim
        && sx == nten->axis[1].size
        && sy == nten->axis[2].size
        && sz == nten->axis[3].size)) {
    biffAddf(TEN, "%s: dimensions of %u-D T2 volume (%s,%s,%s) "
             "don't match tensor volume (%s,%s,%s)", me, nT2->dim,
             airSprintSize_t(stmp[0], sx),
             airSprintSize_t(stmp[1], sy),
             airSprintSize_t(stmp[2], sz),
             airSprintSize_t(stmp[3], nten->axis[1].size),
             airSprintSize_t(stmp[4], nten->axis[2].size),
             airSprintSize_t(stmp[5], nten->axis[3].size));
    return 1;
  }
  if (nrrdMaybeAlloc_va(ndwi, nrrdTypeFloat, 4,
                        AIR_CAST(size_t, DD), sx, sy, sz)) {
    biffMovef(TEN, NRRD, "%s: couldn't allocate output", me);
    return 1;
  }
  dwi = (float*)(ndwi->data);
  ten = (float*)(nten->data);
  bmat = (double*)(nbmat->data);
  lup = nrrdFLookup[nT2->type];
  for (II=0; II<(size_t)(sx*sy*sz); II++) {
    /* tenVerbose = (II == 42 + 190*(96 + 196*0)); */
    tenSimulateSingle_f(dwi, lup(nT2->data, II), ten, bmat, DD,
                        AIR_CAST(float, b));
    dwi += DD;
    ten += 7;
  }

  airMopOkay(mop);
  return 0;
}


















/* old stuff, prior to tenEstimationMatrix .. */


/*
******** tenCalcOneTensor1
**
** make one diffusion tensor from the measurements at one voxel, based
** on the gradient directions used by Andy Alexander
*/
void
tenCalcOneTensor1(float tens[7], float chan[7],
                  float thresh, float slope, float b) {
  double c[7], sum, d1, d2, d3, d4, d5, d6;

  c[0] = AIR_MAX(chan[0], 1);
  c[1] = AIR_MAX(chan[1], 1);
  c[2] = AIR_MAX(chan[2], 1);
  c[3] = AIR_MAX(chan[3], 1);
  c[4] = AIR_MAX(chan[4], 1);
  c[5] = AIR_MAX(chan[5], 1);
  c[6] = AIR_MAX(chan[6], 1);
  sum = c[1] + c[2] + c[3] + c[4] + c[5] + c[6];
  tens[0] = AIR_CAST(float, (1 + airErf(slope*(sum - thresh)))/2.0);
  d1 = (log(c[0]) - log(c[1]))/b;
  d2 = (log(c[0]) - log(c[2]))/b;
  d3 = (log(c[0]) - log(c[3]))/b;
  d4 = (log(c[0]) - log(c[4]))/b;
  d5 = (log(c[0]) - log(c[5]))/b;
  d6 = (log(c[0]) - log(c[6]))/b;
  tens[1] = AIR_CAST(float,  d1 + d2 - d3 - d4 + d5 + d6);    /* Dxx */
  tens[2] = AIR_CAST(float,  d5 - d6);                        /* Dxy */
  tens[3] = AIR_CAST(float,  d1 - d2);                        /* Dxz */
  tens[4] = AIR_CAST(float, -d1 - d2 + d3 + d4 + d5 + d6);    /* Dyy */
  tens[5] = AIR_CAST(float,  d3 - d4);                        /* Dyz */
  tens[6] = AIR_CAST(float,  d1 + d2 + d3 + d4 - d5 - d6);    /* Dzz */
  return;
}

/*
******** tenCalcOneTensor2
**
** using gradient directions used by EK
*/
void
tenCalcOneTensor2(float tens[7], float chan[7],
                  float thresh, float slope, float b) {
  double c[7], sum, d1, d2, d3, d4, d5, d6;

  c[0] = AIR_MAX(chan[0], 1);
  c[1] = AIR_MAX(chan[1], 1);
  c[2] = AIR_MAX(chan[2], 1);
  c[3] = AIR_MAX(chan[3], 1);
  c[4] = AIR_MAX(chan[4], 1);
  c[5] = AIR_MAX(chan[5], 1);
  c[6] = AIR_MAX(chan[6], 1);
  sum = c[1] + c[2] + c[3] + c[4] + c[5] + c[6];
  tens[0] = AIR_CAST(float, (1 + airErf(slope*(sum - thresh)))/2.0);
  d1 = (log(c[0]) - log(c[1]))/b;
  d2 = (log(c[0]) - log(c[2]))/b;
  d3 = (log(c[0]) - log(c[3]))/b;
  d4 = (log(c[0]) - log(c[4]))/b;
  d5 = (log(c[0]) - log(c[5]))/b;
  d6 = (log(c[0]) - log(c[6]))/b;
  tens[1] =  AIR_CAST(float, d1);                 /* Dxx */
  tens[2] =  AIR_CAST(float, d6 - (d1 + d2)/2);   /* Dxy */
  tens[3] =  AIR_CAST(float, d5 - (d1 + d3)/2);   /* Dxz */
  tens[4] =  AIR_CAST(float, d2);                 /* Dyy */
  tens[5] =  AIR_CAST(float, d4 - (d2 + d3)/2);   /* Dyz */
  tens[6] =  AIR_CAST(float, d3);                 /* Dzz */
  return;
}

/*
******** tenCalcTensor
**
** Calculate a volume of tensors from measured data
*/
int
tenCalcTensor(Nrrd *nout, Nrrd *nin, int version,
              float thresh, float slope, float b) {
  static const char me[] = "tenCalcTensor";
  char cmt[128];
  float *out, tens[7], chan[7];
  size_t I, sx, sy, sz;
  void (*calcten)(float tens[7], float chan[7],
                  float thresh, float slope, float b);

  if (!(nout && nin)) {
    biffAddf(TEN, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( 1 == version || 2 == version )) {
    biffAddf(TEN, "%s: version should be 1 or 2, not %d", me, version);
    return 1;
  }
  switch (version) {
  case 1:
    calcten = tenCalcOneTensor1;
    break;
  case 2:
    calcten = tenCalcOneTensor2;
    break;
  default:
    biffAddf(TEN, "%s: PANIC, version = %d not handled", me, version);
    return 1;
    break;
  }
  if (tenTensorCheck(nin, nrrdTypeDefault, AIR_TRUE, AIR_TRUE)) {
    biffAddf(TEN, "%s: wasn't given valid tensor nrrd", me);
    return 1;
  }
  sx = nin->axis[1].size;
  sy = nin->axis[2].size;
  sz = nin->axis[3].size;
  if (nrrdMaybeAlloc_va(nout, nrrdTypeFloat, 4,
                        AIR_CAST(size_t, 7), sx, sy, sz)) {
    biffMovef(TEN, NRRD, "%s: couldn't alloc output", me);
    return 1;
  }
  nout->axis[0].label = airStrdup("c,Dxx,Dxy,Dxz,Dyy,Dyz,Dzz");
  nout->axis[1].label = airStrdup("x");
  nout->axis[2].label = airStrdup("y");
  nout->axis[3].label = airStrdup("z");
  nout->axis[0].spacing = AIR_NAN;
  if (AIR_EXISTS(nin->axis[1].spacing) &&
      AIR_EXISTS(nin->axis[2].spacing) &&
      AIR_EXISTS(nin->axis[3].spacing)) {
    nout->axis[1].spacing = nin->axis[1].spacing;
    nout->axis[2].spacing = nin->axis[2].spacing;
    nout->axis[3].spacing = nin->axis[3].spacing;
  }
  else {
    nout->axis[1].spacing = 1.0;
    nout->axis[2].spacing = 1.0;
    nout->axis[3].spacing = 1.0;
  }
  sprintf(cmt, "%s: using thresh = %g, slope = %g, b = %g\n",
          me, thresh, slope, b);
  nrrdCommentAdd(nout, cmt);
  out = (float *)nout->data;
  for (I=0; I<(size_t)(sx*sy*sz); I++) {
    if (tenVerbose && !(I % (sx*sy))) {
      fprintf(stderr, "%s: z = %d of %d\n", me, (int)(I/(sx*sy)), (int)sz-1);
    }
    chan[0] = nrrdFLookup[nin->type](nin->data, 0 + 7*I);
    chan[1] = nrrdFLookup[nin->type](nin->data, 1 + 7*I);
    chan[2] = nrrdFLookup[nin->type](nin->data, 2 + 7*I);
    chan[3] = nrrdFLookup[nin->type](nin->data, 3 + 7*I);
    chan[4] = nrrdFLookup[nin->type](nin->data, 4 + 7*I);
    chan[5] = nrrdFLookup[nin->type](nin->data, 5 + 7*I);
    chan[6] = nrrdFLookup[nin->type](nin->data, 6 + 7*I);
    calcten(tens, chan, thresh, slope, b);
    out[0 + 7*I] = tens[0];
    out[1 + 7*I] = tens[1];
    out[2 + 7*I] = tens[2];
    out[3 + 7*I] = tens[3];
    out[4 + 7*I] = tens[4];
    out[5 + 7*I] = tens[5];
    out[6 + 7*I] = tens[6];
  }
  return 0;
}

