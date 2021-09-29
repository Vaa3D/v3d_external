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

#define INFO "Connect slices and/or slabs into a bigger nrrd"
static const char *_unrrdu_joinInfoL =
(INFO
 ". Can stich images into volumes, or tile images side "
 "by side, or attach images onto volumes.  If there are many many "
 "files to name in the \"-i\" option, and using wildcards won't work, "
 "consider putting the list of "
 "filenames into a separate text file (e.g. \"slices.txt\"), and then "
 "name this file as a response file (e.g. \"-i @slices.txt\"). "
 "This command now allows you to set the same pieces of information that "
 "previously had to be set with \"unu axinfo\": label, spacing, and min/max. "
 "These can be use whether the join axis is new (because of \"-incr\") or "
 "not.\n "
 "* Uses nrrdJoin");

int
unrrdu_joinMain(int argc, const char **argv, const char *me,
                hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err, *label;
  Nrrd **nin;
  Nrrd *nout;
  int incrDim, pret;
  unsigned int ninLen, axis;
  double mm[2], spc;
  airArray *mop;

  hparm->respFileEnable = AIR_TRUE;

  hestOptAdd(&opt, "i,input", "nin0", airTypeOther, 1, -1, &nin, NULL,
             "everything to be joined together",
             &ninLen, NULL, nrrdHestNrrd);
  OPT_ADD_AXIS(axis, "axis to join along");
  hestOptAdd(&opt, "incr", NULL, airTypeInt, 0, 0, &incrDim, NULL,
             "in situations where the join axis is *not* among the existing "
             "axes of the input nrrds, then this flag signifies that the join "
             "axis should be *inserted*, and the output dimension should "
             "be one greater than input dimension.  Without this flag, the "
             "nrrds are joined side-by-side, along an existing axis.");
  hestOptAdd(&opt, "l,label", "label", airTypeString, 1, 1, &label, "",
             "label to associate with join axis");
  hestOptAdd(&opt, "mm,minmax", "min max", airTypeDouble, 2, 2, mm, "nan nan",
             "min and max values along join axis");
  hestOptAdd(&opt, "sp,spacing", "spc", airTypeDouble, 1, 1, &spc, "nan",
             "spacing between samples along join axis");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_joinInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdJoin(nout, AIR_CAST(const Nrrd*const*, nin), ninLen,
               axis, incrDim)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error joining nrrds:\n%s", me, err);
    airMopError(mop);
    return 1;
  }
  if (strlen(label)) {
    nout->axis[axis].label = (char *)airFree(nout->axis[axis].label);
    nout->axis[axis].label = airStrdup(label);
  }
  if (AIR_EXISTS(mm[0])) {
    nout->axis[axis].min = mm[0];
  }
  if (AIR_EXISTS(mm[1])) {
    nout->axis[axis].max = mm[1];
  }
  if (AIR_EXISTS(spc)) {
    nout->axis[axis].spacing = spc;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(join, INFO);
