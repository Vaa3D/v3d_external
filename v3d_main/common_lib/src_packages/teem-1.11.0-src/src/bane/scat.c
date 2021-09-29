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
baneRawScatterplots(Nrrd *nvg, Nrrd *nvh, Nrrd *hvol, int histEq) {
  Nrrd *gA, *hA, *gB, *hB;
  static const char me[]="baneRawScatterplots";
  int E;

  if (!( nvg && nvh && hvol )) {
    biffAddf(BANE, "%s: got NULL pointer", me);
    return 1;
  }
  if (baneHVolCheck(hvol)) {
    biffAddf(BANE, "%s: didn't get a valid histogram volume", me);
    return 1;
  }

  gA = nrrdNew(); gB = nrrdNew();
  hA = nrrdNew(); hB = nrrdNew();
  /* create initial projections */
  E = 0;
  if (!E) E |= nrrdProject(gA, hvol, 1, nrrdMeasureSum, nrrdTypeDefault);
  if (!E) E |= nrrdProject(hA, hvol, 0, nrrdMeasureSum, nrrdTypeDefault);
  if (E) {
    biffMovef(BANE, NRRD, "%s: trouble creating raw scatterplots", me);
    return 1;
  }

  /* do histogram equalization on them */
  if (histEq) {
    if (!E) E |= nrrdHistoEq(gB, gA, NULL, baneStateHistEqBins,
                             baneStateHistEqSmart, 1.0);
    if (!E) E |= nrrdHistoEq(hB, hA, NULL, baneStateHistEqBins,
                             baneStateHistEqSmart, 1.0);
  } else {
    if (!E) E |= nrrdCopy(gB, gA);
    if (!E) E |= nrrdCopy(hB, hA);
  }
  if (E) {
    biffMovef(BANE, NRRD, "%s: couldn't histogram equalize or copy", me);
    return 1;
  }

  /* re-orient them so they look correct on the screen */
  if (!E) E |= nrrdAxesSwap(gA, gB, 0, 1);
  if (!E) E |= nrrdAxesSwap(hA, hB, 0, 1);
  if (!E) E |= nrrdFlip(gB, gA, 1);
  if (!E) E |= nrrdFlip(hB, hA, 1);
  if (E) {
    biffMovef(BANE, NRRD, "%s: couldn't re-orient scatterplots", me);
    return 1;
  }

  if (!E) E |= nrrdCopy(nvg, gB);
  if (!E) E |= nrrdCopy(nvh, hB);
  if (E) {
    biffMovef(BANE, NRRD, "%s: trouble saving results to given nrrds", me);
    return 1;
  }

  nrrdNuke(gA); nrrdNuke(gB);
  nrrdNuke(hA); nrrdNuke(hB);
  return 0;
}
