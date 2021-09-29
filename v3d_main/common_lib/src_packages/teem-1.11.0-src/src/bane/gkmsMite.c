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

#define MITE_INFO "Modify opacity function to work with \"mite\""
static const char *_baneGkms_miteInfoL =
  (MITE_INFO
   ". Useful when using the \"mite\" Teem library, or the \"miter\" "
   "command-line renderer.  This adds a \"stub\" axis 0, and setting the "
   "axis labels to identify the domain and range of the opacity function. "
   "The underlying opacity function is not modified.");
int
baneGkms_miteMain(int argc, const char **argv, const char *me,
                  hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *perr;
  Nrrd *nin, *nout;
  airArray *mop;
  int pret, E;

  hestOptAdd(&opt, "i", "opacIn", airTypeOther, 1, 1, &nin, NULL,
             "input opacity function (1 or 2 dimensional), from "
             "\"gkms opac\"",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "o", "opacOut", airTypeString, 1, 1, &out, NULL,
             "output opacity function filename");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_baneGkms_miteInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  if (1 == nin->axis[0].size && nin->axis[0].label &&
      !strcmp("A", nin->axis[0].label)) {
    fprintf(stderr, "%s: already\n", me);
    nout = nin;
  } else {
    nout = nrrdNew();
    airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
    E = 0;
    if (!E) E |= nrrdAxesInsert(nout, nin, 0);
    if (!E) E |= !(nout->axis[0].label = airStrdup("A"));
    if (!E) E |= !(nout->axis[1].label = airStrdup("gage(v)"));
    if (3 == nout->dim) {
      if (!E) E |= !(nout->axis[2].label = airStrdup("gage(gm)"));
    }
    if (E) {
      biffMovef(BANE, NRRD,
                "%s: trouble modifying opacity function nrrd", me);
      airMopError(mop); return 1;
    }
  }
  if (nrrdSave(out, nout, NULL)) {
    biffMovef(BANE, NRRD, "%s: trouble saving opacity function", me);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
BANE_GKMS_CMD(mite, MITE_INFO);

