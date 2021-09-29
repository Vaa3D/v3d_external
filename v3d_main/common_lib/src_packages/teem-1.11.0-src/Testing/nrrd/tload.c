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

#include "teem/nrrd.h"
#include <testDataPath.h>

/*
** Tests:
** nrrdLoad
*/

int
main(int argc, const char **argv) {
  const char *me;
  Nrrd *nin;
  airArray *mop;
  char *fullname;
  int differ;

  AIR_UNUSED(argc);
  me = argv[0];
  mop = airMopNew();

  nin = nrrdNew();
  airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);
  fullname = testDataPathPrefix("fmob-c4h.nrrd");
  airMopAdd(mop, fullname, airFree, airMopAlways);
  if (nrrdLoad(nin, fullname, NULL)) {
    char *err;
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble reading data \"%s\":\n%s",
            me, fullname, err);
    airMopError(mop); return 1;
  }

  {
    Nrrd *ncopy;
    char *blah, *blah1L, explain[AIR_STRLEN_LARGE];
    size_t ii, blen;
    ncopy = nrrdNew();
    airMopAdd(mop, ncopy, (airMopper)nrrdNuke, airMopAlways);
    blen = AIR_STRLEN_HUGE*42;
    blah = AIR_CALLOC(blen, char);
    airMopAdd(mop, blah, airFree, airMopAlways);
    for (ii=0; ii<blen-1; ii++) {
      /* NOTE: a more rigorous test would put things in here that can't
         be directly written to file, like \r \v \f, but fixing that bug
         is enough of a significant change to functionality that it should
         be discussed with users first */
      blah[ii] = ("abcdefghijklmnopqrtsuvzwyz\n\"\\ "
                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n\"\\ ")
        [airRandInt((26 + 4)*2)]; /* 4 other characters */
    }
    blah[ii] = '\0';
    /* NOTE: this blah1L is to overcome a long-stanging bug that \n's
       were permitted in labels and units, but can't be written.  Same
       as NOTE above.  New code in Teem
       (nrrd/keyvalue.c/_nrrdWriteEscaped()) now prevents generating
       broken NRRD files, but it means that the comparison test
       between in-memory vs the saved-and-read nrrd would fail, so we
       do the same transformation here to allow the comparison to
       work */
    blah1L = airOneLinify(airStrdup(blah));
    airMopAdd(mop, blah1L, airFree, airMopAlways);
    nrrdAxisInfoSet_va(nin, nrrdAxisInfoLabel,
                       "first axis label", "2nd axis label",
                       blah1L);
    nin->spaceUnits[0] = airOneLinify(airStrdup("\nsp\"\nu0\n"));
    nin->spaceUnits[1] = airStrdup("bob");
    nin->spaceUnits[2] = airStrdup(blah1L);  /* see NOTE above */
    if (nrrdCommentAdd(nin, "the first comment")
        || nrrdCommentAdd(nin, "very long comment follows")
        || nrrdCommentAdd(nin, blah)
        || nrrdKeyValueAdd(nin, "first key", "first value")
        || nrrdKeyValueAdd(nin, "2nd key", "2nd value")
        || nrrdKeyValueAdd(nin, "big key", blah)) {
      char *err;
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble w/ comments or KVPs:\n%s",
              me, err);
      airMopError(mop); return 1;
    }
    if (nrrdCopy(ncopy, nin)
        || nrrdCompare(nin, ncopy, AIR_FALSE /* onlyData */,
                       0.0 /* epsilon */, &differ, explain)) {
      char *err;
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble w/ copy or compare:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
    if (differ) {
      fprintf(stderr, "%s: difference in in-memory copy: %s\n", me, explain);
      airMopError(mop); return 1;
    }
    if (nrrdSave("tloadTest.nrrd", nin, NULL)
        || nrrdLoad(ncopy, "tloadTest.nrrd", NULL)
        || nrrdCompare(nin, ncopy, AIR_FALSE /* onlyData */,
                       0.0 /* epsilon */, &differ, explain)) {
      char *err;
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble w/ save, load, compare:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
    if (differ) {
      fprintf(stderr, "%s: difference in on-disk copy: %s\n", me, explain);
      airMopError(mop); return 1;
    }
  }

  airMopOkay(mop);
  return 0;
}
