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
** airStrtok
** airStrntok
**
** Also uses:
** airArrayNew, airArrayNuke, airArrayLenSet, airArrayLenIncr
** airMopNew, airMopAdd, airMopError, airMopDone
*/

#define INCR 10
int
main(int argc, const char *argv[]) {
  airArray *mop;
  const char *me;
  const char *word[] = {
    "There's", "a", "certain", "slant", "of", "light",
    "On", "winter", "afternoons",
    "That", "oppresses,", "like", "the", "weight",
    "Of", "cathedral", "tunes.",
    "Heavenly", "hurt", "it", "gives", "us;",
    "We", "can", "find", "no", "scar,",
    "But", "internal", "difference",
    "Where", "the", "meanings", "are.",
    "None", "may", "teach", "it", "anything,",
    "'T", "is", "the", "seal,", "despair,",
    "An", "imperial", "affliction",
    "Sent", "us", "of", "the", "air.",
    "When", "it", "comes,", "the", "landscape", "listens,",
    "Shadows", "hold", "their", "breath;",
    "When", "it", "goes,", "'t", "is", "like", "the", "distance",
    "On", "the", "look", "of", "death.", ""};
  const char *sep = " \t\n_", *ww;
  unsigned int wi, sepLen, lineLen, wordNum;
  airArray *lineArr;
  char wordsp[AIR_STRLEN_MED], *line, *last=NULL;

  AIR_UNUSED(argc);
  me = argv[0];
  sepLen = AIR_CAST(unsigned int, airStrlen(sep));

  mop = airMopNew();
  lineArr = airArrayNew((void**)(&line), &lineLen, sizeof(char), INCR);
  airMopAdd(mop, lineArr, (airMopper)airArrayNuke, airMopAlways);

  /* initialize line with "" */
  airArrayLenSet(lineArr, 1);
  strcpy(line, "");

  /* add words and separators onto line */
  for (wi=0; airStrlen(word[wi]); wi++) {
    sprintf(wordsp, "%s%c", word[wi], sep[AIR_MOD(wi, sepLen)]);
    airArrayLenIncr(lineArr, AIR_CAST(int, airStrlen(wordsp)));
    strcat(line, wordsp);
  }

  /* lose last sep char */
  line[strlen(line)-1] = '\0';

  /* start tokenizing and comparing */
  wordNum = airStrntok(line, sep);
  if (75 != wordNum) {
    fprintf(stderr, "%s: wordNum %u != 75\n", me, wordNum);
    airMopError(mop);
    return 1;
  }
  wi = 0;
  for (ww = airStrtok(line, sep, &last);
       ww;
       ww = airStrtok(NULL, sep, &last)) {
    if (strcmp(word[wi], ww)) {
      fprintf(stderr, "%s: word[%u] |%s| != parsed |%s|\n",
              me, wi, word[wi], ww);
      airMopError(mop);
      exit(1);
    }
    wi++;
  }

  /* we're done */
  airMopOkay(mop);
  exit(0);
}
