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


#include "teem/air.h"

/*
** Tests:
** airStrlen
** airStrdup
** airStrcpy
**
** Also uses:
** airMopNew, airMopAdd, airMopError, airMopDone
*/

/* random test strings */
#define STR_A1 "The facts of life: To make an alteration in the evolvement of an organic life system is fatal. A coding sequence cannot be revised once it's been established."
#define STR_A2 "Because by the second day of incubation, any cells that have undergone reversion mutations give rise to revertant colonies like rats leaving a sinking ship; then the ship sinks."
#define STR_A (STR_A1 " " STR_A2)


#define STR_B1 "We've already tried it. Ethyl methane sulfonate is an alkylating agent and a potent mutagen. It created a virus so lethal the subject was dead before he left the table."
#define STR_B2 "Wouldn't obstruct replication, but it does give rise to an error in replication so that the newly formed DNA strand carries a mutation and you've got a virus again. But this - all of this is academic. You were made as well as we could make you."
#define STR_B (STR_B1 " " STR_B2)

#define STR_AB (STR_A " " STR_B)

int
main(int argc, const char *argv[]) {
  airArray *mop;
  const char *me;
  char *aa, *aaCopy, *ab;
  size_t aaSize, abSize;

  AIR_UNUSED(argc);
  me = argv[0];
  mop = airMopNew();

  if (0 != airStrlen(NULL)) {
    fprintf(stderr, "%s: 0 != airStrlen(NULL)\n", me);
    airMopError(mop);
    exit(1);
  }
  if (0 != airStrlen("")) {
    fprintf(stderr, "%s: 0 != airStrlen(\"\")\n", me);
    airMopError(mop);
    exit(1);
  }
  if (1 != airStrlen("A")) {
    fprintf(stderr, "%s: 1 != airStrlen(\"A\")\n", me);
    airMopError(mop);
    exit(1);
  }
  if (NULL != airStrdup(NULL)) {
    fprintf(stderr, "%s: NULL != airStrdup(NULL)\n", me);
    airMopError(mop);
    exit(1);
  }
  if (strlen(STR_A) != airStrlen(STR_A)) {
    fprintf(stderr, "%s: strlen %u != airStrlen %u of |%s|\n", me,
            AIR_CAST(unsigned int, strlen(STR_A)),
            AIR_CAST(unsigned int, airStrlen(STR_A)), STR_A);
    airMopError(mop);
    exit(1);
  }

  aa = airStrdup(STR_A);
  airMopAdd(mop, aa, airFree, airMopAlways);
  if (strcmp(aa, STR_A)) {
    fprintf(stderr, "%s: airStrdup failure: |%s| != |%s|\n", me,
            aa, STR_A);
    airMopError(mop);
    exit(1);
  }
  aaSize = strlen(aa)+1;
  aaCopy = AIR_CALLOC(aaSize, char);
  airMopAdd(mop, aaCopy, airFree, airMopAlways);

  abSize = strlen(STR_A) + strlen(" ") + strlen(STR_B) + 1;
  ab = AIR_CALLOC(abSize, char);
  airMopAdd(mop, ab, airFree, airMopAlways);
  sprintf(ab, "%s %s", STR_A, STR_B);

#define TEST(COPY, COMP)                                     \
  airStrcpy(aaCopy, aaSize, COPY);                           \
  if (strcmp(aaCopy, COMP)) {                                \
    fprintf(stderr, "%s: airStrcpy failure: |%s| != |%s|\n", \
            me, aaCopy, COMP);                               \
    airMopError(mop);                                        \
    exit(1);                                                 \
  }

  /* different args (to copy) to airStrcpy */
  /* arg to copy is smaller than dest */
  TEST("", "");
  TEST(NULL, ""); /* depends on previous test result */
  TEST(STR_A1, STR_A1);
  TEST(STR_A2, STR_A2);
  /* arg to copy is same size as dest */
  TEST(STR_A, STR_A);
  TEST(aa, aa);
  /* arg to copy is bigger than dest */
  TEST(ab, aa);

  airMopOkay(mop);
  exit(0);
}
