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

#define INFO "Modify attributes of one or more axes"
static const char *_unrrdu_axinfoInfoL =
(INFO
 ". The only attributes which are set are those for which command-line "
 "options are given.\n "
 "* Uses no particular function; just sets fields in NrrdAxisInfo");

int
unrrdu_axinfoMain(int argc, const char **argv, const char *me,
                  hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err, *label, *units, *centerStr, *kindStr;
  Nrrd *nin, *nout;
  int pret, center, kind;
  unsigned int *axes, axesLen, axi;
  double mm[2], spc;
  airArray *mop;

  hestOptAdd(&opt, "a,axes", "ax0", airTypeUInt, 1, -1, &axes, NULL,
             "the one or more axes that should be modified", &axesLen);
  hestOptAdd(&opt, "l,label", "label", airTypeString, 1, 1, &label, "",
             "label to associate with axis");
  hestOptAdd(&opt, "u,units", "units", airTypeString, 1, 1, &units, "",
             "units of measurement");
  hestOptAdd(&opt, "mm,minmax", "min max", airTypeDouble, 2, 2, mm, "nan nan",
             "min and max values along axis");
  hestOptAdd(&opt, "sp,spacing", "spacing", airTypeDouble, 1, 1, &spc, "nan",
             "spacing between samples along axis");
  /* HEY: this is currently a fundamental (but only rarely annoying)
     problem in hest.  Because there is functionally no difference
     between whether an option's information comes from the default
     string or from the command-line, there is no real way to tell
     hest, "hey, its just fine for this option to not be used, and
     if its not used, DON'T DO ANYTHING".  The games of setting strings
     to "" and floats/doubles to NaN are ways of compensating for
     this.  However, there is no analogous trick for airEnums.
  hestOptAdd(&opt, "c,center", "center", airTypeEnum, 1, 1, &cent, "unknown",
             "centering of axis: \"cell\" or \"node\"",
             NULL, nrrdCenter);
  */
  /* but this hack will do for now */
  hestOptAdd(&opt, "c,center", "center", airTypeString, 1, 1, &centerStr, "",
             "axis centering: \"cell\" or \"node\".  Not using this option "
             "leaves the centering as it is on input");
  hestOptAdd(&opt, "k,kind", "kind", airTypeString, 1, 1, &kindStr, "",
             "axis kind. Not using this option "
             "leaves the kind as it is on input");

  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_axinfoInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  for (axi=0; axi<axesLen; axi++) {
    if (!( axes[axi] < nin->dim )) {
      fprintf(stderr, "%s: axis %u not in valid range [0,%u]\n",
              me, axes[axi], nin->dim-1);
      airMopError(mop);
      return 1;
    }
  }
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdCopy(nout, nin)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error copying input:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  for (axi=0; axi<axesLen; axi++) {
    unsigned int axis;
    axis = axes[axi];
    if (strlen(label)) {
      nout->axis[axis].label = (char *)airFree(nout->axis[axis].label);
      nout->axis[axis].label = airStrdup(label);
    }
    if (strlen(units)) {
      nout->axis[axis].units = (char *)airFree(nout->axis[axis].units);
      nout->axis[axis].units = airStrdup(units);
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
    /* see above
    if (nrrdCenterUnknown != cent) {
      nout->axis[axis].center = cent;
    }
    */
    if (airStrlen(centerStr)) {
      if (!strcmp("none", centerStr)
          || !strcmp("???", centerStr)) {
        center = nrrdCenterUnknown;
      } else {
        if (!(center = airEnumVal(nrrdCenter, centerStr))) {
          fprintf(stderr, "%s: couldn't parse \"%s\" as %s\n", me,
                  centerStr, nrrdCenter->name);
          airMopError(mop);
          return 1;
        }
      }
      nout->axis[axis].center = center;
    }
    if (airStrlen(kindStr)) {
      if (!strcmp("none", kindStr)
          || !strcmp("???", kindStr)) {
        kind = nrrdKindUnknown;
      } else {
        if (!(kind = airEnumVal(nrrdKind, kindStr))) {
          fprintf(stderr, "%s: couldn't parse \"%s\" as %s\n", me,
                  kindStr, nrrdKind->name);
          airMopError(mop);
          return 1;
        }
      }
      nout->axis[axis].kind = kind;
    }
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(axinfo, INFO);
