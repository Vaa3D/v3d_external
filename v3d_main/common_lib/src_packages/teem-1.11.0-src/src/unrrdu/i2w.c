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

#define INFO "converts from 1-D index to world position"
static const char *_unrrdu_i2wInfoL =
(INFO  ", given the centering of the data (cell vs. node), "
 "the range of positions, and the number of intervals into "
 "which position has been quantized. "
 "This is a demo/utility, which does not actually operate on any nrrds. "
 "Previously available as the stand-alone idx2pos binary.\n "
 "* Uses NRRD_POS macro");

int
unrrdu_i2wMain(int argc, const char **argv, const char *me,
               hestParm *hparm) {
  hestOpt *opt = NULL;
  airArray *mop;
  int pret;
  char *err;

  int center;
  double minPos, maxPos, pos, indx, size;

  mop = airMopNew();
  hestOptAdd(&opt, NULL, "center", airTypeEnum, 1, 1, &center, NULL,
             "which centering applies to the quantized position.\n "
             "Possibilities are:\n "
             "\b\bo \"cell\": for histogram bins, quantized values, and "
             "pixels-as-squares\n "
             "\b\bo \"node\": for non-trivially interpolated "
             "sample points", NULL, nrrdCenter);
  hestOptAdd(&opt, NULL, "minPos", airTypeDouble, 1, 1, &minPos, NULL,
             "smallest position associated with index 0");
  hestOptAdd(&opt, NULL, "maxPos", airTypeDouble, 1, 1, &maxPos, NULL,
             "highest position associated with highest index");
  hestOptAdd(&opt, NULL, "num", airTypeDouble, 1, 1, &size, NULL,
             "number of intervals into which position has been quantized");
  hestOptAdd(&opt, NULL, "index", airTypeDouble, 1, 1, &indx, NULL,
             "the input index position, to be converted to world");
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_unrrdu_i2wInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  pos = NRRD_POS(center, minPos, maxPos, size, indx);
  printf("%g\n", pos);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(i2w, INFO);
