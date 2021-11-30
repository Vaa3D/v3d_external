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

#define INFO "Sees if two nrrds are different in any way"
static const char *_unrrdu_diffInfoL =
(INFO
 ". Looks through all fields to see if two given nrrds contain the "
 "same information. Or, array meta-data can be excluded, and comparison "
 "only on the data values is done with the -od flag.\n "
 "* Uses nrrdCompare");

int
unrrdu_diffMain(int argc, const char **argv, const char *me,
                hestParm *hparm) {
  hestOpt *opt = NULL;
  char *err;
  airArray *mop;
  int pret;

  Nrrd *ninA, *ninB;
  int onlyData, differ;
  double epsilon;
  char explain[AIR_STRLEN_LARGE];

  mop = airMopNew();
  hestOptAdd(&opt, NULL, "ninA", airTypeOther, 1, 1, &ninA, NULL,
             "First input nrrd.",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, NULL, "ninB", airTypeOther, 1, 1, &ninB, NULL,
             "Second input nrrd.",
             NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&opt, "eps,epsilon", "eps", airTypeDouble, 1, 1, &epsilon, "0.0",
             "threshold for allowable difference in values in "
             "data values");
  hestOptAdd(&opt, "od,onlydata", NULL, airTypeInt, 0, 0, &onlyData, NULL,
             "Compare data values only, excluding array meta-data");
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_diffInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  if (nrrdCompare(ninA, ninB, onlyData, epsilon, &differ, explain)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error doing compare:\n%s", me, err);
    airMopError(mop);
    return 1;
  }
  if (differ) {
    printf("%s: %s differ: %s\n", me, onlyData ? "data values" : "nrrds",
           explain);
  } else {
    if (0 == epsilon) {
      printf("%s: %s are the same\n", me, onlyData ? "data values" : "nrrds");
    } else {
      printf("%s: %s are same or within %g of each other\n", me,
             onlyData ? "data values" : "nrrds", epsilon);
    }
  }

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(diff, INFO);
