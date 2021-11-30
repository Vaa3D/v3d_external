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

#include "unrrdu.h"
#include "privateUnrrdu.h"

#define INFO "Normalizes array orientation and meta-data"
static const char *_unrrdu_dnormInfoL =
  (INFO
   ". Forces information about kind and orientation into "
   "a consistent form, and nixes various other fields. This was "
   "created as a utility for the Diderot project "
   "(http://diderot-language.cs.uchicago.edu), hence the name.\n "
   "* (as yet there's no single nrrd function which does all this)");

int
unrrdu_dnormMain(int argc, const char **argv, const char *me,
                 hestParm *hparm) {
  char *outS;
  int pret;

  Nrrd *nin, *nout;
  NrrdIoState *nio;
  int kindIn, kindOut, headerOnly, haveMM, trivialOrient, recenter, gotmf;
  unsigned int kindAxis, axi, si, sj;
  double sscl;

  hestOpt *opt = NULL;
  char *err;
  airArray *mop;

  hestOptAdd(&opt, "h,header", NULL, airTypeInt, 0, 0, &headerOnly, NULL,
             "output header of nrrd file only, not the data itself");
  hestOptAdd(&opt, "to", NULL, airTypeInt, 0, 0, &trivialOrient, NULL,
             "(*t*rivial *o*rientation) "
             "even if the input nrrd comes with full orientation or "
             "per-axis min-max info, ignore it and instead assert the "
             "most trivial mapping between index and world space");
  hestOptAdd(&opt, "c,center", NULL, airTypeInt, 0, 0, &recenter, NULL,
             "re-locate output spaceOrigin so that field is centered "
             "around origin of space coordinates");
  hestOptAdd(&opt, "s,scaling", "scl", airTypeDouble, 1, 1, &sscl, "1.0",
             "when contriving orientation information, distance between "
             "samples to use");
  hestOptAdd(&opt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
             "input image", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output filename", NULL);

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_unrrdu_dnormInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  /* can't deal with block type */
  if (nrrdTypeBlock == nin->type) {
    fprintf(stderr, "%s: can only have scalar kinds (not %s)\n", me,
            airEnumStr(nrrdType, nrrdTypeBlock));
    airMopError(mop); exit(1);
  }

  /* make sure all kinds are set to something */
  /* see if there's a range kind, verify that there's only one */
  /* set haveMM */
  haveMM = AIR_TRUE;
  kindIn = nrrdKindUnknown;
  kindAxis = 0;
  for (axi=0; axi<nin->dim; axi++) {
    if (nrrdKindUnknown == nin->axis[axi].kind
        || nrrdKindIsDomain(nin->axis[axi].kind)) {
      haveMM &= AIR_EXISTS(nin->axis[axi].min);
      haveMM &= AIR_EXISTS(nin->axis[axi].max);
    } else {
      if (nrrdKindUnknown != kindIn) {
        fprintf(stderr, "%s: got non-domain kind %s on axis %u, but already "
                "have %s from axis %u\n", me,
                airEnumStr(nrrdKind, nin->axis[axi].kind), axi,
                airEnumStr(nrrdKind, kindIn), kindAxis);
        airMopError(mop); exit(1);
      }
      kindIn = nin->axis[axi].kind;
      kindAxis = axi;
    }
  }
  /* see if the non-domain kind is something we can interpret as a tensor */
  if (nrrdKindUnknown != kindIn) {
    switch (kindIn) {
      /* ======= THESE are the kinds that we can possibly output ======= */
    case nrrdKind2Vector:
    case nrrdKind3Vector:
    case nrrdKind4Vector:
    case nrrdKind2DSymMatrix:
    case nrrdKind2DMatrix:
    case nrrdKind3DSymMatrix:
    case nrrdKind3DMatrix:
      /* =============================================================== */
      kindOut = kindIn;
      break;
      /* Some other kinds are mapped to those above */
    case nrrdKind3Color:
    case nrrdKindRGBColor:
      kindOut = nrrdKind3Vector;
      break;
    case nrrdKind4Color:
    case nrrdKindRGBAColor:
      kindOut = nrrdKind4Vector;
      break;
    default:
      fprintf(stderr, "%s: got non-conforming kind %s on axis %u\n", me,
              airEnumStr(nrrdKind, kindIn), kindAxis);
      airMopError(mop); exit(1);
      break;
    }
  } else {
    kindOut = nrrdKindUnknown;
  }

  /* initialize output by copying */
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdCopy(nout, nin)) {
    airMopAdd(mop, err = biffGet(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble copying:\n%s", me, err);
    airMopError(mop); exit(1);
  }

  /* no comments, either advertising the format URL or anything else */
  nio = nrrdIoStateNew();
  airMopAdd(mop, nio, (airMopper)nrrdIoStateNix, airMopAlways);
  nio->skipFormatURL = AIR_TRUE;
  if (headerOnly) {
    nio->skipData = AIR_TRUE;
  }
  nrrdCommentClear(nout);

  /* no measurement frame */
  gotmf = AIR_FALSE;
  for (si=0; si<NRRD_SPACE_DIM_MAX; si++) {
    for (sj=0; sj<NRRD_SPACE_DIM_MAX; sj++) {
      gotmf |= AIR_EXISTS(nout->measurementFrame[si][sj]);
    }
  }
  if (gotmf) {
    fprintf(stderr, "%s: WARNING: incoming array measurement frame; "
            "it will be erased on output.\n", me);
    airMopError(mop); exit(1);
  }
  for (si=0; si<NRRD_SPACE_DIM_MAX; si++) {
    for (sj=0; sj<NRRD_SPACE_DIM_MAX; sj++) {
      nout->measurementFrame[si][sj] = AIR_NAN;
    }
  }

  /* no key/value pairs */
  nrrdKeyValueClear(nout);

  /* no content field */
  nout->content = airFree(nout->content);

  /* normalize domain kinds to "space" */
  /* turn off centers (perhaps Diderot should assume cell-centered) */
  /* turn off thickness */
  /* turn off labels and units */
  for (axi=0; axi<nout->dim; axi++) {
    if (nrrdKindUnknown == kindOut) {
      nout->axis[axi].kind = nrrdKindSpace;
    } else {
      nout->axis[axi].kind = (kindAxis == axi
                              ? kindOut
                              : nrrdKindSpace);
    }
    nout->axis[axi].center = nrrdCenterUnknown;
    nout->axis[axi].thickness = AIR_NAN;
    nout->axis[axi].label = airFree(nout->axis[axi].label);
    nout->axis[axi].units = airFree(nout->axis[axi].units);
    nout->axis[axi].min = AIR_NAN;
    nout->axis[axi].max = AIR_NAN;
    nout->axis[axi].spacing = AIR_NAN;
  }

  /* logic of orientation definition:
     if space dimension is known:
        set origin to zero if not already set
        set space direction to unit vector if not already set
     else:
        set origin to zero and all space directions to units
     might be nice to use gage's logic for mapping from world to index,
     but we have to accept a greater variety of kinds and dimensions
     than gage ever has to process.
  */
  if (nout->spaceDim && !trivialOrient) {
    int saxi = 0;
    /* we use only the space dimension, not any named space */
    nout->space = nrrdSpaceUnknown;
    if (!nrrdSpaceVecExists(nout->spaceDim, nout->spaceOrigin)) {
      nrrdSpaceVecSetZero(nout->spaceOrigin);
    }
    for (axi=0; axi<nout->dim; axi++) {
      if (nrrdKindUnknown == kindOut || kindAxis != axi) {
        if (!nrrdSpaceVecExists(nout->spaceDim,
                                nout->axis[axi].spaceDirection)) {
          nrrdSpaceVecSetZero(nout->axis[axi].spaceDirection);
          nout->axis[axi].spaceDirection[saxi] = sscl;
        }
        saxi++;
      } else {
        nrrdSpaceVecSetNaN(nout->axis[axi].spaceDirection);
      }
    }
  } else if (haveMM && !trivialOrient) {
    int saxi = 0;
    for (axi=0; axi<nout->dim; axi++) {
      if (nrrdKindUnknown == kindOut || kindAxis != axi) {
        nrrdSpaceVecSetZero(nout->axis[axi].spaceDirection);
        nout->axis[axi].spaceDirection[saxi]
          = (nin->axis[axi].max - nin->axis[axi].min)/(nin->axis[axi].size-1);
        nout->spaceOrigin[saxi] = nin->axis[axi].min;
        saxi++;
      } else {
        nrrdSpaceVecSetNaN(nout->axis[axi].spaceDirection);
      }
    }
    nout->spaceDim = saxi;
  } else {
    /* either trivialOrient, or not spaceDim, or not not haveMM */
    int saxi = 0;
    nout->space = nrrdSpaceUnknown;
    nrrdSpaceVecSetZero(nout->spaceOrigin);
    for (axi=0; axi<nout->dim; axi++) {
      if (nrrdKindUnknown == kindOut || kindAxis != axi) {
        nrrdSpaceVecSetZero(nout->axis[axi].spaceDirection);
        nout->axis[axi].spaceDirection[saxi]
          = (AIR_EXISTS(nin->axis[axi].spacing)
             ? nin->axis[axi].spacing
             : sscl);
        saxi++;
      } else {
        nrrdSpaceVecSetNaN(nout->axis[axi].spaceDirection);
      }
    }
    nout->spaceDim = saxi;
  }

  /* space dimension has to match the number of domain axes */
  if (nout->dim != nout->spaceDim + !!kindOut) {
    fprintf(stderr, "%s: output dim %d != spaceDim %d + %d %s%s%s\n",
            me, nout->dim, nout->spaceDim, !!kindOut,
            kindOut ? "for non-scalar (" : "(scalar data)",
            kindOut ? airEnumStr(nrrdKind, kindOut) : "",
            kindOut ? ") data" : "");
    airMopError(mop); exit(1);
  }

  if (recenter) {
    /* sets field's origin so field is centered on the origin. capiche? */
    /* this code was tacked on later than the stuff above, so its
       logic could probably be moved up there, but it seems cleaner to
       have it as a separate post-process */
    double mean[NRRD_SPACE_DIM_MAX];
    nrrdSpaceVecSetZero(mean);
    for (axi=0; axi<nout->dim; axi++) {
      if (nrrdKindUnknown == kindOut || kindAxis != axi) {
        nrrdSpaceVecScaleAdd2(mean, 1.0, mean,
                              0.5*(nout->axis[axi].size - 1),
                              nout->axis[axi].spaceDirection);
      }
    }
    nrrdSpaceVecScaleAdd2(mean, 1.0, mean,
                          1.0, nout->spaceOrigin);
    /* now mean is the center of the field */
    nrrdSpaceVecScaleAdd2(nout->spaceOrigin,
                          1.0, nout->spaceOrigin,
                          -1.0, mean);
  }

  if (nrrdSave(outS, nout, nio)) {
    airMopAdd(mop, err = biffGet(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving \"%s\":\n%s",
            me, outS, err);
    airMopError(mop); exit(1);
  }

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD_HIDE(dnorm, INFO);
