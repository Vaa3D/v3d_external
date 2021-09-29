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

#include "pull.h"
#include "privatePull.h"

/* --------------------------------------------- */

unsigned int
_pullInfoLen[PULL_INFO_MAX+1] = {
  0, /* pullInfoUnknown */
  7, /* pullInfoTensor */
  7, /* pullInfoTensorInverse */
  9, /* pullInfoHessian */
  1, /* pullInfoInside */
  3, /* pullInfoInsideGradient */
  1, /* pullInfoHeight */
  3, /* pullInfoHeightGradient */
  9, /* pullInfoHeightHessian */
  1, /* pullInfoHeightLaplacian */
  1, /* pullInfoSeedPreThresh */
  1, /* pullInfoSeedThresh */
  1, /* pullInfoLiveThresh */
  1, /* pullInfoLiveThresh2 */
  1, /* pullInfoLiveThresh3 */
  3, /* pullInfoTangent1 */
  3, /* pullInfoTangent2 */
  3, /* pullInfoNegativeTangent1 */
  3, /* pullInfoNegativeTangent2 */
  1, /* pullInfoIsovalue */
  3, /* pullInfoIsovalueGradient */
  9, /* pullInfoIsovalueHessian */
  1, /* pullInfoStrength */
  1, /* pullInfoQuality */
};

unsigned int
pullInfoLen(int info) {
  unsigned int ret;

  if (!airEnumValCheck(pullInfo, info)) {
    ret = _pullInfoLen[info];
  } else {
    ret = 0;
  }
  return ret;
}

unsigned int
pullPropLen(int prop) {
  unsigned int ret;

  switch (prop) {
  case pullPropIdtag:
  case pullPropIdCC:
  case pullPropEnergy:
  case pullPropStepEnergy:
  case pullPropStepConstr:
  case pullPropStuck:
  case pullPropNeighDistMean:
  case pullPropScale:
  case pullPropStability:
    ret = 1;
    break;
  case pullPropPosition:
  case pullPropForce:
    ret = 4;
    break;
  case pullPropNeighCovar:
    ret = 10;
    break;
  case pullPropNeighCovar7Ten:
    ret = 7;
    break;
  case pullPropNeighTanCovar:
    ret = 6;
    break;
  default:
    ret = 0;
    break;
  }
  return ret;
}

pullInfoSpec *
pullInfoSpecNew(void) {
  pullInfoSpec *ispec;

  ispec = AIR_CAST(pullInfoSpec *, calloc(1, sizeof(pullInfoSpec)));
  if (ispec) {
    ispec->info = pullInfoUnknown;
    ispec->source = pullSourceUnknown;
    ispec->volName = NULL;
    ispec->item = 0;  /* should be the unknown item for any kind */
    ispec->prop = pullPropUnknown;
    ispec->scale = AIR_NAN;
    ispec->zero = AIR_NAN;
    ispec->constraint = AIR_FALSE;
    ispec->volIdx = UINT_MAX;
  }
  return ispec;
}

pullInfoSpec *
pullInfoSpecNix(pullInfoSpec *ispec) {

  if (ispec) {
    airFree(ispec->volName);
    airFree(ispec);
  }
  return NULL;
}

int
pullInfoSpecAdd(pullContext *pctx, pullInfoSpec *ispec) {
  static const char me[]="pullInfoSpecAdd";
  unsigned int ii, vi, haveLen, needLen;
  const gageKind *kind;

  if (!( pctx && ispec )) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(pullInfo, ispec->info)) {
    biffAddf(PULL, "%s: %d not a valid %s value", me,
             ispec->info, pullInfo->name);
    return 1;
  }
  if (airEnumValCheck(pullSource, ispec->source)) {
    biffAddf(PULL, "%s: %d not a valid %s value", me,
             ispec->source, pullSource->name);
    return 1;
  }
  if (pctx->ispec[ispec->info]) {
    biffAddf(PULL, "%s: already set info %s (%d)", me,
             airEnumStr(pullInfo, ispec->info), ispec->info);
    return 1;
  }
  for (ii=0; ii<=PULL_INFO_MAX; ii++) {
    if (pctx->ispec[ii] == ispec) {
      biffAddf(PULL, "%s(%s): already got ispec %p as ispec[%u]", me,
               airEnumStr(pullInfo, ispec->info), AIR_VOIDP(ispec), ii);
      return 1;
    }
  }
  if (pctx->verbose) {
    printf("%s: ispec %s from vol %s\n", me,
           airEnumStr(pullInfo, ispec->info), ispec->volName);
  }
  needLen = pullInfoLen(ispec->info);
  if (pullSourceGage == ispec->source) {
    vi = _pullVolumeIndex(pctx, ispec->volName);
    if (UINT_MAX == vi) {
      biffAddf(PULL, "%s(%s): no volume has name \"%s\"", me,
               airEnumStr(pullInfo, ispec->info), ispec->volName);
      return 1;
    }
    kind = pctx->vol[vi]->kind;
    if (airEnumValCheck(kind->enm, ispec->item)) {
      biffAddf(PULL, "%s(%s): %d not a valid \"%s\" item", me,
               airEnumStr(pullInfo, ispec->info), ispec->item, kind->name);
      return 1;
    }
    haveLen = kind->table[ispec->item].answerLength;
    if (needLen != haveLen) {
      biffAddf(PULL, "%s(%s): need len %u, but \"%s\" item \"%s\" has len %u",
               me, airEnumStr(pullInfo, ispec->info), needLen,
               kind->name, airEnumStr(kind->enm, ispec->item), haveLen);
      return 1;
    }
    /* very tricky: seedOnly is initialized to true for everything */
    if (pullInfoSeedThresh != ispec->info
        && pullInfoSeedPreThresh != ispec->info) {
      /* if the info is neither seedthresh nor seedprethresh, then the
         volume will have to be probed after the first iter, so turn
         *off* seedOnly */
      pctx->vol[vi]->seedOnly = AIR_FALSE;
    }
    /* less tricky: turn on forSeedPreThresh as needed;
       its initialized to false */
    if (pullInfoSeedPreThresh == ispec->info) {
      pctx->vol[vi]->forSeedPreThresh = AIR_TRUE;
      if (pctx->verbose) {
        printf("%s: volume %u %s used for %s\n", me, vi, pctx->vol[vi]->name,
               airEnumStr(pullInfo, pullInfoSeedPreThresh));
      }
    }
    /* now set item in gage query */
    if (gageQueryItemOn(pctx->vol[vi]->gctx, pctx->vol[vi]->gpvl,
                        ispec->item)) {
      biffMovef(PULL, GAGE, "%s: trouble adding item %u to vol %u", me,
                ispec->item, vi);
      return 1;
    }
    ispec->volIdx = vi;
  } else if (pullSourceProp == ispec->source) {
    haveLen = pullPropLen(ispec->prop);
    if (needLen != haveLen) {
      biffAddf(PULL, "%s: need len %u, but \"%s\" \"%s\" has len %u",
               me, needLen, pullProp->name,
               airEnumStr(pullProp, ispec->prop), haveLen);
      return 1;
    }

  } else {
    biffAddf(PULL, "%s: sorry, source %s unsupported", me,
             airEnumStr(pullSource, ispec->source));
    return 1;
  }
  if (haveLen > 9) {
    biffAddf(PULL, "%s: sorry, answer length (%u) > 9 unsupported", me,
             haveLen);
    return 1;
  }

  pctx->ispec[ispec->info] = ispec;

  return 0;
}

/*
** sets:
** pctx->infoIdx[]
** pctx->infoTotalLen
** pctx->constraint
** pctx->constraintDim
** pctx->targetDim (non-trivial logic for scale-space!)
*/
int
_pullInfoSetup(pullContext *pctx) {
  static const char me[]="_pullInfoSetup";
  unsigned int ii;

  pctx->infoTotalLen = 0;
  pctx->constraint = 0;
  pctx->constraintDim = 0;
  for (ii=0; ii<=PULL_INFO_MAX; ii++) {
    if (pctx->ispec[ii]) {
      pctx->infoIdx[ii] = pctx->infoTotalLen;
      if (pctx->verbose) {
        printf("!%s: infoIdx[%u] (%s) = %u\n", me,
               ii, airEnumStr(pullInfo, ii), pctx->infoIdx[ii]);
      }
      pctx->infoTotalLen += pullInfoLen(ii);
      if (!pullInfoLen(ii)) {
        biffAddf(PULL, "%s: got zero-length answer for ispec[%u]", me, ii);
        return 1;
      }
      if (pctx->ispec[ii]->constraint) {
        /* pullVolume *cvol; */
        pctx->constraint = ii;
        /* cvol = pctx->vol[pctx->ispec[ii]->volIdx]; */
      }
    }
  }
  if (pctx->constraint) {
    pctx->constraintDim = _pullConstraintDim(pctx);
    if (-1 == pctx->constraintDim) {
      biffAddf(PULL, "%s: problem learning constraint dimension", me);
      return 1;
    }
    if (!pctx->flag.allowCodimension3Constraints && !pctx->constraintDim) {
      biffAddf(PULL, "%s: got constr dim 0 but co-dim 3 not allowed", me);
      return 1;
    }
    if (pctx->haveScale) {
      double *parmS, denS,
        (*evalS)(double *, double, const double parm[PULL_ENERGY_PARM_NUM]);
      switch (pctx->interType) {
      case pullInterTypeUnivariate:
        pctx->targetDim = 1 + pctx->constraintDim;
        break;
      case pullInterTypeSeparable:
        /* HEY! need to check if this is true given enr and ens! */
        pctx->targetDim = pctx->constraintDim;
        break;
      case pullInterTypeAdditive:
        parmS = pctx->energySpecS->parm;
        evalS = pctx->energySpecS->energy->eval;
        evalS(&denS, _PULL_TARGET_DIM_S_PROBE, parmS);
        if (denS > 0) {
          /* at small positive s, derivative was positive ==> attractive */
          pctx->targetDim = pctx->constraintDim;
        } else {
          /* derivative was negative ==> repulsive */
          pctx->targetDim = 1 + pctx->constraintDim;
        }
        break;
      default:
        biffAddf(PULL, "%s: sorry, intertype %s not handled here", me,
                 airEnumStr(pullInterType, pctx->interType));
        break;
      }
    } else {
      pctx->targetDim = pctx->constraintDim;
    }
  } else {
    pctx->constraintDim = 0;
    pctx->targetDim = 0;
  }
  if (pctx->verbose) {
    printf("!%s: infoTotalLen=%u, constr=%d, constr,targetDim = %d,%d\n",
           me, pctx->infoTotalLen, pctx->constraint,
           pctx->constraintDim, pctx->targetDim);
  }
  return 0;
}

static void
_infoCopy1(double *dst, const double *src) {
  dst[0] = src[0];
}

static void
_infoCopy2(double *dst, const double *src) {
  dst[0] = src[0]; dst[1] = src[1];
}

static void
_infoCopy3(double *dst, const double *src) {
  dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
}

static void
_infoCopy4(double *dst, const double *src) {
  dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
}

static void
_infoCopy5(double *dst, const double *src) {
  dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
  dst[4] = src[4];
}

static void
_infoCopy6(double *dst, const double *src) {
  dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
  dst[4] = src[4]; dst[5] = src[5];
}

static void
_infoCopy7(double *dst, const double *src) {
  dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
  dst[4] = src[4]; dst[5] = src[5]; dst[6] = src[6];
}

static void
_infoCopy8(double *dst, const double *src) {
  dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
  dst[4] = src[4]; dst[5] = src[5]; dst[6] = src[6]; dst[7] = src[7];
}

static void
_infoCopy9(double *dst, const double *src) {
  dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
  dst[4] = src[4]; dst[5] = src[5]; dst[6] = src[6]; dst[7] = src[7];
  dst[8] = src[8];
}

void (*_pullInfoCopy[10])(double *, const double *) = {
  NULL,
  _infoCopy1,
  _infoCopy2,
  _infoCopy3,
  _infoCopy4,
  _infoCopy5,
  _infoCopy6,
  _infoCopy7,
  _infoCopy8,
  _infoCopy9
};

int
pullInfoGet(Nrrd *ninfo, int info, pullContext *pctx) {
  static const char me[]="pullInfoGet";
  size_t size[2];
  unsigned int dim, pointNum, pointIdx, binIdx, outIdx, alen, aidx;
  double *out_d;
  pullBin *bin;
  pullPoint *point;

  if (airEnumValCheck(pullInfo, info)) {
    biffAddf(PULL, "%s: info %d not valid", me, info);
    return 1;
  }
  pointNum = pullPointNumber(pctx);
  alen = pullInfoLen(info);
  aidx = pctx->infoIdx[info];
  if (1 == alen) {
    dim = 1;
    size[0] = pointNum;
  } else {
    dim = 2;
    size[0] = alen;
    size[1] = pointNum;
  }
  if (nrrdMaybeAlloc_nva(ninfo, nrrdTypeDouble, dim, size)) {
    biffMovef(PULL, NRRD, "%s: trouble allocating output", me);
    return 1;
  }
  out_d = AIR_CAST(double *, ninfo->data);

  outIdx = 0;
  for (binIdx=0; binIdx<pctx->binNum; binIdx++) {
    bin = pctx->bin + binIdx;
    for (pointIdx=0; pointIdx<bin->pointNum; pointIdx++) {
      point = bin->point[pointIdx];
      _pullInfoCopy[alen](out_d + outIdx, point->info + aidx);
      outIdx += alen;
    }
  }

  return 0;
}

/* HEY this was written in a hurry;
** needs to be checked against parsing code */
int
pullInfoSpecSprint(char str[AIR_STRLEN_LARGE],
                   const pullContext *pctx, const pullInfoSpec *ispec) {
  static const char me[]="pullInfoSpecSprint";
  const pullVolume *pvol;
  char stmp[AIR_STRLEN_LARGE];

  if (!( str && pctx && ispec )) {
    biffAddf(PULL, "%s: got NULL pointer", me);
    return 1;
  }
  strcpy(str, "");
  /* HEY: no bounds checking! */
  strcat(str, airEnumStr(pullInfo, ispec->info));
  if (ispec->constraint) {
    strcat(str, "-c");
  }
  strcat(str, ":");
  if (pullSourceGage == ispec->source) {
    if (UINT_MAX == ispec->volIdx) {
      biffAddf(PULL, "%s: never learned volIdx for \"%s\"", me,
               ispec->volName);
      return 1;
    }
    strcat(str, ispec->volName);
    strcat(str, ":");
    pvol = pctx->vol[ispec->volIdx];
    strcat(str, airEnumStr(pvol->kind->enm, ispec->item));
  } else if (pullSourceProp == ispec->source) {
    strcat(str, airEnumStr(pullProp, ispec->prop));
  } else {
    biffAddf(PULL, "%s: unexplained source %d", me, ispec->source);
    return 1;
  }
  if ( (pullSourceGage == ispec->source
        && 1 == pullInfoLen(ispec->info))
       ||
       (pullSourceProp == ispec->source
        && 1 == pullPropLen(ispec->prop)) ) {
    sprintf(stmp, "%g", ispec->zero);
    strcat(str, stmp);
    strcat(str, ":");
    sprintf(stmp, "%g", ispec->scale);
    strcat(str, stmp);
  }
  return 0;
}

