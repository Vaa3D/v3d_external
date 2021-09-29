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

#include "seek.h"
#include "privateSeek.h"

/*
******** seekVerboseSet
**
*/
void
seekVerboseSet(seekContext *sctx, int verbose) {

  if (sctx) {
    sctx->verbose = verbose;
  }
  return;
}

/*
******** seekDataSet
**
** user sets EITHER: ninscl, or, gctx and pvlIdx
**
** if ninscl: this is a vanilla scalar volume, and we can do seekTypeIsocontour
** if gctx: this is a scalar or non-scalar volume, and we can do any seekType
**
** sets from input:
** ninscl, gctx, pvl
**
** So the rest of seek can use "if (sctx->ninscl)" to see if we're working
** with a vanilla scalar volume or not
**
** invalidates:
** valItem, normItem, gradItem, evalItem, evecItem
*/
int
seekDataSet(seekContext *sctx, const Nrrd *ninscl,
            gageContext *gctx, unsigned int pvlIdx) {
  static const char me[]="seekDataSet";

  if (!( sctx && (ninscl || gctx) )) {
    biffAddf(SEEK, "%s: got NULL pointer", me);
    return 1;
  }
  if (ninscl && gctx) {
    biffAddf(SEEK, "%s: must give ninscl or gctx, but not both", me);
    return 1;
  }

  if (ninscl) {
    if (nrrdCheck(ninscl)) {
      biffMovef(SEEK, NRRD, "%s: problem with volume", me);
      return 1;
    }
    if (3 != ninscl->dim) {
      biffAddf(SEEK, "%s: vanilla scalar volume must be 3-D (not %d-D)",
               me, ninscl->dim);
      return 1;
    }
    if (nrrdTypeBlock == ninscl->type) {
      biffAddf(SEEK, "%s: can't work with %s type values", me,
               airEnumStr(nrrdType, nrrdTypeBlock));
      return 1;
    }
    sctx->ninscl = ninscl;
    sctx->gctx = NULL;
    sctx->pvl = NULL;
  } else {
    if (!( pvlIdx < gctx->pvlNum )) {
      biffAddf(SEEK, "%s: pvlIdx %u not < pvlNum %u",
               me, pvlIdx, gctx->pvlNum);
      return 1;
    }
    /* we assume that caller has done a gageUpdate(), so no other error
       checking is required (or really possible) here */
    sctx->ninscl = NULL;
    sctx->gctx = gctx;
    sctx->pvl = gctx->pvl[pvlIdx];
  }
  sctx->flag[flagData] = AIR_TRUE;

  sctx->sclvItem = -1;
  sctx->normItem = -1;
  sctx->gradItem = -1;
  sctx->evalItem = -1;
  sctx->evecItem = -1;
  sctx->hessItem = -1;

  return 0;
}

/*
******** seekSamplesSet
**
** sets: samples[3]
*/
int
seekSamplesSet(seekContext *sctx, size_t samples[3]) {
  static const char me[]="seekSamplesSet";
  unsigned int numZero;

  if (!(sctx && samples)) {
    biffAddf(SEEK, "%s: got NULL pointer", me);
    return 1;
  }
  numZero = 0;
  numZero += 0 == samples[0];
  numZero += 0 == samples[1];
  numZero += 0 == samples[2];
  if (!( 0 == numZero || 3 == numZero )) {
    biffAddf(SEEK, "%s: samples (%u,%u,%u) must all be 0 or !=0 together", me,
             AIR_CAST(unsigned int, samples[0]),
             AIR_CAST(unsigned int, samples[1]),
             AIR_CAST(unsigned int, samples[2]));
    return 1;
  }
  if (sctx->samples[0] != samples[0]
      || sctx->samples[1] != samples[1]
      || sctx->samples[2] != samples[2]) {
    sctx->samples[0] = samples[0];
    sctx->samples[1] = samples[1];
    sctx->samples[2] = samples[2];
    sctx->flag[flagSamples] = AIR_TRUE;
  }
  return 0;
}

/*
******** seekTypeSet
**
** sets: featureType
*/
int
seekTypeSet(seekContext *sctx, int type) {
  static const char me[]="seekTypeSet";

  if (!sctx) {
    biffAddf(SEEK, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(seekType, type)) {
    biffAddf(SEEK, "%s: %d not a valid %s", me, type, seekType->name);
    return 1;
  }
  if (sctx->type != type) {
    sctx->type = type;
    sctx->flag[flagType] = AIR_TRUE;
  }
  return 0;
}

/*
********* seekLowerInsideSet
**
** sets: lowerInside
*/
int
seekLowerInsideSet(seekContext *sctx, int lowerInside) {
  static const char me[]="seekLowerInsideSet";

  if (!sctx) {
    biffAddf(SEEK, "%s: got NULL pointer", me);
    return 1;
  }
  if (sctx->lowerInside != lowerInside) {
    sctx->lowerInside = lowerInside;
    sctx->flag[flagLowerInside] = AIR_TRUE;
  }
  return 0;
}

/*
********* seekNormalsFindSet
**
** sets: normalsFind
*/
int
seekNormalsFindSet(seekContext *sctx, int normalsFind) {
  static const char me[]="seekNormalsFindSet";

  if (!sctx) {
    biffAddf(SEEK, "%s: got NULL pointer", me);
    return 1;
  }
  if (sctx->normalsFind != normalsFind) {
    sctx->normalsFind = normalsFind;
    sctx->flag[flagNormalsFind] = AIR_TRUE;
  }
  return 0;
}

int
seekStrengthUseSet(seekContext *sctx, int doit) {
  static const char me[]="seekStrengthUseSet";

  if (!sctx) {
    biffAddf(SEEK, "%s: got NULL pointer", me);
    return 1;
  }
  if (sctx->strengthUse != doit) {
    sctx->strengthUse = doit;
    sctx->flag[flagStrengthUse] = AIR_TRUE;
  }
  return 0;
}

int
seekStrengthSet(seekContext *sctx, int strengthSign,
                double strength) {
  static const char me[]="seekStrengthSet";

  if (!sctx) {
    biffAddf(SEEK, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(1 == strengthSign || -1 == strengthSign)) {
    biffAddf(SEEK, "%s: strengthSign (%d) not +1 or -1", me, strengthSign);
    return 1;
  }
  if (!AIR_EXISTS(strength)) {
    biffAddf(SEEK, "%s: strength %g doesn't exist", me, strength);
    return 1;
  }
  if (sctx->strengthSign != strengthSign) {
    sctx->strengthSign = strengthSign;
    sctx->flag[flagStrength] = AIR_TRUE;
  }
  if (sctx->strength != strength) {
    sctx->strength = strength;
    sctx->flag[flagStrength] = AIR_TRUE;
  }
  return 0;
}

static int
itemCheck(seekContext *sctx, int item, unsigned int wantLen) {
  static const char me[]="itemCheck";

  if (!sctx) {
    biffAddf(SEEK, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(sctx->gctx && sctx->pvl)) {
    biffAddf(SEEK, "%s: don't have a gage context", me);
    return 1;
  }
  if (airEnumValCheck(sctx->pvl->kind->enm, item)) {
    biffAddf(SEEK, "%s: %d not valid %s item", me, item,
             sctx->pvl->kind->enm->name);
    return 1;
  }
  if (!GAGE_QUERY_ITEM_TEST(sctx->pvl->query, item)) {
    biffAddf(SEEK, "%s: item \"%s\" (%d) not set in query", me,
             airEnumStr(sctx->pvl->kind->enm, item), item);
    return 1;
  }
  if (sctx->pvl->kind->table[item].answerLength != wantLen) {
    biffAddf(SEEK, "%s: item %s has length %u, not wanted %u", me,
             airEnumStr(sctx->pvl->kind->enm, item),
             sctx->pvl->kind->table[item].answerLength, wantLen);
    return 1;
  }
  return 0;
}

/*
******** seekItemScalarSet
**
** sets: sclvItem
*/
int
seekItemScalarSet(seekContext *sctx, int item) {
  static const char me[]="seekItemScalarSet";

  if (itemCheck(sctx, item, 1)) {
    biffAddf(SEEK, "%s: trouble", me);
    return 1;
  }
  if (sctx->sclvItem != item) {
    sctx->sclvItem = item;
    sctx->flag[flagItemValue] = AIR_TRUE;
  }
  return 0;
}

/*
******** seekItemStrengthSet
**
*/
int
seekItemStrengthSet(seekContext *sctx, int item) {
  static const char me[]="seekItemStrengthSet";

  if (itemCheck(sctx, item, 1)) {
    biffAddf(SEEK, "%s: trouble", me);
    return 1;
  }
  if (sctx->stngItem != item) {
    sctx->stngItem = item;
    sctx->flag[flagItemStrength] = AIR_TRUE;
  }
  return 0;
}

/*
******** seekItemHessSet
**
*/
int
seekItemHessSet(seekContext *sctx, int item) {
  char me[]="seekItemHessSet";

  if (itemCheck(sctx, item, 9)) {
    biffAddf(SEEK, "%s: trouble", me); return 1;
  }
  if (sctx->hessItem != item) {
    sctx->hessItem = item;
    sctx->flag[flagItemHess] = AIR_TRUE;
  }
  return 0;
}

/*
******** seekItemGradientSet
**
** sets: gradItem
*/
int
seekItemGradientSet(seekContext *sctx, int item) {
  static const char me[]="seekItemGradientSet";

  if (itemCheck(sctx, item, 3)) {
    biffAddf(SEEK, "%s: trouble", me);
    return 1;
  }
  if (sctx->gradItem != item) {
    sctx->gradItem = item;
    sctx->flag[flagItemGradient] = AIR_TRUE;
  }
  /* sctx->gradAns = gageAnswerPointer(sctx->gctx, sctx->pvl, item); */
  return 0;
}

/*
******** seekItemNormalSet
**
** sets: normItem
*/
int
seekItemNormalSet(seekContext *sctx, int item) {
  static const char me[]="seekItemNormalSet";

  if (itemCheck(sctx, item, 3)) {
    biffAddf(SEEK, "%s: trouble", me);
    return 1;
  }
  if (sctx->normItem != item) {
    sctx->normItem = item;
    sctx->flag[flagItemNormal] = AIR_TRUE;
  }
  /* sctx->normAns = gageAnswerPointer(sctx->gctx, sctx->pvl, item); */
  return 0;
}

/*
******** seekItemEigensystemSet
**
** sets: evalItem, evecItem
*/
int
seekItemEigensystemSet(seekContext *sctx, int evalItem, int evecItem) {
  static const char me[]="seekItemEigenvectorSet";

  if (itemCheck(sctx, evalItem, 3)) {
    biffAddf(SEEK, "%s: trouble", me);
    return 1;
  }
  if (itemCheck(sctx, evecItem, 9)) {
    biffAddf(SEEK, "%s: trouble", me);
    return 1;
  }
  if (sctx->evalItem != evalItem
      || sctx->evecItem != evecItem) {
    sctx->evalItem = evalItem;
    sctx->evecItem = evecItem;
    sctx->flag[flagItemEigensystem] = AIR_TRUE;
  }
  /*
  sctx->evalAns = gageAnswerPointer(sctx->gctx, sctx->pvl, sctx->evalItem);
  sctx->evecAns = gageAnswerPointer(sctx->gctx, sctx->pvl, sctx->evecItem);
  */
  return 0;
}

/*
******** seekIsovalueSet
**
** sets: isovalue
*/
int
seekIsovalueSet(seekContext *sctx, double isovalue) {
  static const char me[]="seekIsovalueSet";

  if (!sctx) {
    biffAddf(SEEK, "%s: got NULL pointer", me);
    return 1;
  }
  if (!AIR_EXISTS(isovalue)) {
    biffAddf(SEEK, "%s: given isovalue %g doesn't exit", me, isovalue);
    return 1;
  }
  if (sctx->isovalue != isovalue) {
    sctx->isovalue = isovalue;
    sctx->flag[flagIsovalue] = AIR_TRUE;
  }
  return 0;
}

/*
******** seekEvalDiffThreshSet
**
** sets: difference threshold from which two eigenvalues are
** considered "similar" (cf. Eq. (4) in TVCG paper by
** Schultz/Theisel/Seidel)
*/
int
seekEvalDiffThreshSet(seekContext *sctx, double evalDiffThresh) {
  char me[]="seekEvalDiffThreshSet";

  if (!sctx) {
    biffAddf(SEEK, "%s: got NULL pointer", me); return 1;
  }
  if (!AIR_EXISTS(evalDiffThresh)) {
    biffAddf(SEEK, "%s: given eigenvalue difference threshold %g doesn't exit",
             me, evalDiffThresh); return 1;
  }
  if (sctx->evalDiffThresh != evalDiffThresh) {
    sctx->evalDiffThresh = evalDiffThresh;
    sctx->flag[flagEvalDiffThresh] = AIR_TRUE;
  }
  return 0;
}
