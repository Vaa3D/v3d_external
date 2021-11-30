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

#include "gage.h"
#include "privateGage.h"

/*
** in the 1.11 release this file was not actually linked in;
** but it remains in the repo as notes on the first pass
** at implementing this functionality.  More thinking needed!
*/

/* The "/ *Teem:" (without space) comments in here are an experiment */

gageMultiItem * /*Teem: error if (!ret) */
gageMultiItemNew(const gageKind *kind) {
  gageMultiItem *gmi = NULL;

  if (kind && (gmi = AIR_CALLOC(1, gageMultiItem))) {
    gmi->kind = kind;
    gmi->item = NULL;
    gmi->ansDir = NULL;
    gmi->ansLen = NULL;
    gmi->nans = nrrdNew();
  }
  return gmi;
}

gageMultiItem * /*Teem: no error */
gageMultiItemNix(gageMultiItem *gmi) {

  if (gmi) {
    airFree(gmi->item);
    airFree(gmi);
  }
  return NULL;
}

gageMultiItem * /*Teem: no error */
gageMultiItemNuke(gageMultiItem *gmi) {

  if (gmi) {
    nrrdNuke(gmi->nans);
  }
  return gageMultiItemNix(gmi);
}

int /*Teem: biff if (ret) */
gageMultiItemSet(gageMultiItem *gmi, const int *item, unsigned int itemNum) {
  static const char me[]="gageMultiItemSet";
  unsigned int ii;

  if (!(gmi && item)) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (!itemNum) {
    biffAddf(GAGE, "%s: can't set zero items", me);
    return 1;
  }
  gmi->item = AIR_CAST(int *, airFree(gmi->item));
  if (!( gmi->item = AIR_CALLOC(itemNum, int) )) {
    biffAddf(GAGE, "%s: couldn't allocate %u ints for items", me, itemNum);
    return 1;
  }

  for (ii=0; ii<itemNum; ii++) {
    if (airEnumValCheck(gmi->kind->enm, item[ii])) {
      biffAddf(GAGE, "%s: item[%u] %d not a valid %s value", me,
               ii, item[ii], gmi->kind->enm->name);
      return 1;
    }
    gmi->item[ii] = item[ii];
    fprintf(stderr, "!%s: item[%u] = %d %s\n", me, ii, item[ii],
            airEnumStr(gmi->kind->enm, item[ii]));
  }

  return 0;
}

/*
******** gageMultiItemSet_va
**
** How to set (in one call) the multiple items in a gageMultiItem.
** These are the items for which the answers will be collected into
** a single nrrd on output (maximizing memory locality).
*/
int /*Teem: biff if (ret) */
gageMultiItemSet_va(gageMultiItem *gmi, unsigned int itemNum,
                    ... /* itemNum items */) {
  static const char me[]="gageMultiItemSet_va";
  int *item;
  unsigned int ii;
  va_list ap;
  airArray *mop;

  if (!gmi) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (!itemNum) {
    biffAddf(GAGE, "%s: can't set zero items", me);
    return 1;
  }
  if (!( item = AIR_CALLOC(itemNum, int) )) {
    biffAddf(GAGE, "%s: couldn't allocate %u ints for items", me, itemNum);
    return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, item, airFree, airMopAlways);

  /* consume items from var args */
  va_start(ap, itemNum);
  for (ii=0; ii<itemNum; ii++) {
    item[ii] = va_arg(ap, int);
  }
  va_end(ap);

  if (gageMultiItemSet(gmi, item, itemNum)) {
    biffAddf(GAGE, "%s: problem setting items", me);
    airMopError(mop);
    return 1;
  }

  airMopOkay(mop);
  return 0;
}

/* ----------------------------------------------------------- */

gageMultiQuery * /*Teem: error if (!ret) */
gageMultiQueryNew(const gageContext *gctx) {
  gageMultiQuery *gmq = NULL;

  if (gctx && (gmq = AIR_CALLOC(1, gageMultiQuery))) {
    gmq->pvlNum = gctx->pvlNum;
    gmq->mitmNum = AIR_CALLOC(gmq->pvlNum, unsigned int);
    gmq->mitm = AIR_CALLOC(gmq->pvlNum, gageMultiItem **);
    gmq->nidx = nrrdNew();
    if (!( gmq->mitmNum && gmq->mitm && gmq->nidx )) {
      /* bail */
      airFree(gmq->mitmNum);
      airFree(gmq->mitm);
      nrrdNuke(gmq->nidx);
      airFree(gmq); gmq=NULL;
    } else {
      /* allocated everything ok */
      unsigned int qi;
      for (qi=0; qi<gmq->pvlNum; qi++) {
        gmq->mitm[qi] = NULL;
      }
    }
  }
  return gmq;
}

/*
******** gageMultiQueryAdd_va
**
** add multi-items for one particular pvl (pvlIdx)
*/
int /*Teem: biff if (ret) */
gageMultiQueryAdd_va(gageContext *gctx,
                     gageMultiQuery *gmq, unsigned int pvlIdx,
                     unsigned int mitmNum,
                     ... /* mitmNum gageMultiItem* */) {
  static const char me[]="gageMultiQueryAdd_va";
  unsigned int qi;
  va_list ap;

  if (!gmq) {
    biffAddf(GAGE, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( pvlIdx < gmq->pvlNum )) {
    biffAddf(GAGE, "%s: pvlIdx %u not in valid range [0,%u]", me,
             pvlIdx, gmq->pvlNum-1);
    return 1;
  }

  gmq->mitmNum[pvlIdx] = mitmNum;
  gmq->mitm[pvlIdx] = AIR_CALLOC(mitmNum, gageMultiItem*);
  /* consume and add items to context */
  va_start(ap, mitmNum);
  for (qi=0; qi<mitmNum; qi++) {
    gmq->mitm[pvlIdx][qi] = va_arg(ap, gageMultiItem*);
  }
  va_end(ap);

  AIR_UNUSED(gctx);

  return 0;
}

int /*Teem: biff if (ret) */
gageMultiProbe(gageContext *gctx, gageMultiQuery *gmq,
               const gageMultiInput *minput) {

  AIR_UNUSED(gmq);
  AIR_UNUSED(gctx);
  AIR_UNUSED(minput);
  return 0;
}

gageMultiQuery * /*Teem: no error */
gageMultiQueryNix(gageMultiQuery *gmq) {

  AIR_UNUSED(gmq);
  return NULL;
}

/*
** here the difference between nix and nuke is where the
** nans in gageMultiItem is freed?
*/
gageMultiQuery * /*Teem: no error */
gageMultiQueryNuke(gageMultiQuery *gmq) {

  AIR_UNUSED(gmq);
  return NULL;
}

