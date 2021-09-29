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


#include "teem/biff.h"

/*
** Tests:
** biffAdd
** biffAddf
** biffMove
** biffMovef
** biffGet
** biffDone
** biffGetDone
**
** Also uses:
** airMopNew, airMopAdd, airMopError, airMopDone
*/

int
main(int argc, const char *argv[]) {
  const char *me;
  char *tmp;
  airArray *mop;

  AIR_UNUSED(argc);
  me = argv[0];
  mop = airMopNew();

  /* HEY: the creation and comparison of these strings is not very
     flexible, especially the hard-coded "good[]" strings */
  biffAdd("axis", "the first error axis");
  biffAdd("chard", "the first error chard");
  biffAdd("chard", "the second error chard");
  biffAdd("axis", "the second error axis");
  biffAdd("chard", "the third error chard");
  biffAdd("bingo", "zero-eth bingo message");
  biffMove("bingo", NULL, "chard");
  biffAdd("bingo", "the first error bingo");
  biffAdd("bingo", "the second bll boo boo boo error bingo");
  biffAdd("bingo", "the third error bingo");
  biffAdd("axis", "the third error axis");
  {
    char good[] = ("[bingo] the third error bingo\n"
                   "[bingo] the second bll boo boo boo error bingo\n"
                   "[bingo] the first error bingo\n"
                   "[bingo] [chard] the third error chard\n"
                   "[bingo] [chard] the second error chard\n"
                   "[bingo] [chard] the first error chard\n"
                   "[bingo] zero-eth bingo message\n");
    tmp = biffGet("bingo");
    airMopAdd(mop, tmp, airFree, airMopAlways);

    /* an ugly macro */
#define COMPARE(N)                                                 \
    airMopAdd(mop, tmp, airFree, airMopAlways);                    \
    if (strcmp(tmp, good)) {                                       \
      fprintf(stderr, "%s: %d: #%s# != #%s#\n", me, N, tmp, good); \
      airMopError(mop);                                            \
      exit(1);                                                     \
    }

    COMPARE(1);
  }

  {
    char good[] = "";
    tmp = biffGet("chard");
    COMPARE(2);
  }

  {
    char good[] = ("[axis] the third error axis\n"
                   "[axis] the second error axis\n"
                   "[axis] the first error axis\n");
    tmp = biffGet("axis");
    COMPARE(3);
  }

  biffAdd("harold", "the first error harold");
  biffAdd("harold", "the second error harold");
  biffAdd("harold", "the third error harold");
  {
    char good[] = ("[harold] the third error harold\n"
                   "[harold] the second error harold\n"
                   "[harold] the first error harold\n");
    tmp = biffGetDone("harold");
    COMPARE(4);
  }

  biffDone("bingo");
  biffDone("axis");
  biffDone("chard");

  biffAdd("axis", "the first error axis");
  biffAdd("axis", "the second error axis");
  biffAdd("axis", "the third error axis");
  biffAdd("axis", "the fourth error axis");
  biffAdd("axis", "the fifth error axis");
  {
    char good[] = ("[axis] the fifth error axis\n"
                   "[axis] the fourth error axis\n"
                   "[axis] the third error axis\n"
                   "[axis] the second error axis\n"
                   "[axis] the first error axis\n");
    tmp = biffGetDone("axis");
    COMPARE(5);
  }

  biffAddf("test", "%s: this is a test of biffAddf %d %g", "me", 1, 4.2);
  {
    char good[] = "[test] me: this is a test of biffAddf 1 4.2\n";
    tmp = biffGetDone("test");
    COMPARE(6);
  }

  biffAddf("test2", "%s: this is a test of biffAddf %d %g", "me", 1, 4.2);
  biffMovef("test3", "test2", "%s: testing biffMove %d.", "me", 1729);
  {
    char good[] = ("[test3] me: testing biffMove 1729.\n"
                   "[test3] [test2] me: this is a test of biffAddf 1 4.2\n");
    tmp = biffGet("test3");
    COMPARE(7);
  }

  airMopOkay(mop);
  exit(0);
}
