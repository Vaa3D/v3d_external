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

#include "mite.h"
#include "privateMite.h"

miteUser *
miteUserNew() {
  miteUser *muu;
  int i;

  muu = (miteUser *)calloc(1, sizeof(miteUser));
  if (!muu)
    return NULL;

  muu->umop = airMopNew();
  muu->nsin = NULL;
  muu->nvin = NULL;
  muu->ntin = NULL;
  muu->ntxf = NULL;              /* managed by user (with miter: hest) */
  muu->nout = NULL;              /* managed by user (with miter: hest) */
  muu->debug = NULL;
  muu->debugArr = NULL;
  muu->ndebug = NULL;            /* not allocated until the debug pixel
                                    is rendered, see miteRayBegin */
  muu->ntxfNum = 0;
  muu->shadeStr[0] = 0;
  muu->normalStr[0] = 0;
  for (i=0; i<MITE_RANGE_NUM; i++) {
    muu->rangeInit[i] = 1.0;
  }
  muu->normalSide = miteDefNormalSide;
  muu->refStep = miteDefRefStep;
  muu->rayStep = AIR_NAN;
  muu->opacMatters = miteDefOpacMatters;
  muu->opacNear1 = miteDefOpacNear1;
  muu->hctx = hooverContextNew();
  ELL_3V_SET(muu->fakeFrom, AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_3V_SET(muu->vectorD, 0, 0, 0);
  airMopAdd(muu->umop, muu->hctx, (airMopper)hooverContextNix, airMopAlways);
  for (i=gageKernelUnknown+1; i<gageKernelLast; i++) {
    muu->ksp[i] = NULL;
  }
  muu->shape = gageShapeNew();
  muu->gctx0 = gageContextNew();
  airMopAdd(muu->umop, muu->shape, (airMopper)gageShapeNix, airMopAlways);
  airMopAdd(muu->umop, muu->gctx0, (airMopper)gageContextNix, airMopAlways);
  /* gageParmSet(muu->gctx0, gageParmRequireAllSpacings, AIR_FALSE); */
  muu->lit = limnLightNew();
  airMopAdd(muu->umop, muu->lit, (airMopper)limnLightNix, airMopAlways);
  muu->normalSide = miteDefNormalSide;
  muu->verbUi = muu->verbVi = -1;
  muu->rendTime = 0;
  muu->sampRate = 0;
  return muu;
}

miteUser *
miteUserNix(miteUser *muu) {

  if (muu) {
    airMopOkay(muu->umop);
    airFree(muu);
  }
  return NULL;
}

int
_miteUserCheck(miteUser *muu) {
  static const char me[]="_miteUserCheck";
  int T, gotOpac;
  gageItemSpec isp;
  gageQuery queryScl, queryVec, queryTen, queryMite;
  miteShadeSpec *shpec;
  airArray *mop;
  unsigned int axi;

  if (!muu) {
    biffAddf(MITE, "%s: got NULL pointer", me);
    return 1;
  }
  mop = airMopNew();
  if (!( muu->ntxfNum >= 1 )) {
    biffAddf(MITE, "%s: need at least one transfer function", me);
    airMopError(mop); return 1;
  }
  gotOpac = AIR_FALSE;
  GAGE_QUERY_RESET(queryScl);
  GAGE_QUERY_RESET(queryVec);
  GAGE_QUERY_RESET(queryTen);
  GAGE_QUERY_RESET(queryMite);  /* not actually used here */

  /* add on all queries associated with transfer functions */
  for (T=0; T<muu->ntxfNum; T++) {
    if (miteNtxfCheck(muu->ntxf[T])) {
      biffAddf(MITE, "%s: ntxf[%d] (%d of %d) can't be used as a txf",
               me, T, T+1, muu->ntxfNum);
      airMopError(mop); return 1;
    }
    /* NOTE: no error checking because miteNtxfCheck succeeded */
    for (axi=1; axi<muu->ntxf[T]->dim; axi++) {
      miteVariableParse(&isp, muu->ntxf[T]->axis[axi].label);
      miteQueryAdd(queryScl, queryVec, queryTen, queryMite, &isp);
    }
    gotOpac |= !!strchr(muu->ntxf[T]->axis[0].label, 'A');
  }
  if (!gotOpac) {
    fprintf(stderr, "\n\n%s: ****************************************"
            "************************\n", me);
    fprintf(stderr, "%s: !!! WARNING !!! opacity (\"A\") not set "
            "by any transfer function\n", me);
    fprintf(stderr, "%s: ****************************************"
            "************************\n\n\n", me);
  }

  /* add on "normal"-based queries */
  if (airStrlen(muu->normalStr)) {
    miteVariableParse(&isp, muu->normalStr);
    if (miteValGageKind == isp.kind) {
      biffAddf(MITE, "%s: normalStr \"%s\" refers to a miteVal "
               "(normal must be data-intrinsic)", me, muu->normalStr);
      airMopError(mop); return 1;
    }
    if (3 != isp.kind->table[isp.item].answerLength) {
      biffAddf(MITE, "%s: %s not a vector: can't be used as normal",
               me, muu->normalStr);
      return 1;
    }
    miteQueryAdd(queryScl, queryVec, queryTen, queryMite, &isp);
  }

  /* add on shading-based queries */
  shpec = miteShadeSpecNew();
  airMopAdd(mop, shpec, (airMopper)miteShadeSpecNix, airMopAlways);
  if (miteShadeSpecParse(shpec, muu->shadeStr)) {
    biffAddf(MITE, "%s: couldn't parse shading spec \"%s\"",
             me, muu->shadeStr);
    airMopError(mop); return 1;
  }
  miteShadeSpecQueryAdd(queryScl, queryVec, queryTen, queryMite, shpec);

  /* see if anyone asked for an unspecified normal */
  if ((GAGE_QUERY_ITEM_TEST(queryMite, miteValNdotV)
       || GAGE_QUERY_ITEM_TEST(queryMite, miteValNdotL)
       || GAGE_QUERY_ITEM_TEST(queryMite, miteValVrefN))
      && !airStrlen(muu->normalStr)) {
    biffAddf(MITE, "%s: txf or shading requested a miteVal's use of the "
             "\"normal\", but one has not been specified in muu->normalStr",
             me);
    airMopError(mop); return 1;
  }

  /* see if we have volumes for requested queries */
  if (GAGE_QUERY_NONZERO(queryScl) && !(muu->nsin)) {
    biffAddf(MITE, "%s: txf or shading require %s volume, but don't have one",
             me, gageKindScl->name);
    airMopError(mop); return 1;
  }
  if (GAGE_QUERY_NONZERO(queryVec) && !(muu->nvin)) {
    biffAddf(MITE, "%s: txf or shading require %s volume, but don't have one",
             me, gageKindVec->name);
    airMopError(mop); return 1;
  }
  if (GAGE_QUERY_NONZERO(queryTen) && !(muu->ntin)) {
    biffAddf(MITE, "%s: txf or shading require %s volume, but don't have one",
             me, tenGageKind->name);
    airMopError(mop); return 1;
  }

  /* check appropriateness of given volumes */
  if (muu->nsin) {
    if (gageVolumeCheck(muu->gctx0, muu->nsin, gageKindScl)) {
      biffMovef(MITE, GAGE, "%s: trouble with input %s volume",
                me, gageKindScl->name);
      airMopError(mop); return 1;
    }
  }
  if (muu->nvin) {
    if (gageVolumeCheck(muu->gctx0, muu->nvin, gageKindVec)) {
      biffMovef(MITE, GAGE, "%s: trouble with input %s volume",
                me, gageKindVec->name);
      airMopError(mop); return 1;
    }
  }
  if (muu->ntin) {
    if (gageVolumeCheck(muu->gctx0, muu->ntin, tenGageKind)) {
      biffMovef(MITE, GAGE, "%s: trouble with input %s volume",
                me, tenGageKind->name);
      airMopError(mop); return 1;
    }
  }

  if (!muu->nout) {
    biffAddf(MITE, "%s: rendered image nrrd is NULL", me);
    airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}
