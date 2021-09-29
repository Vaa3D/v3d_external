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

#include <teem/ten.h>

#define TEND "tend"

int
main(int argc, const char **argv) {
  int i, ret;
  const char *me;
  char *argv0 = NULL;
  hestParm *hparm;
  airArray *mop;

  me = argv[0];

  /* parse environment variables first, in case they break nrrdDefault*
     or nrrdState* variables in a way that nrrdSanity() should see */
  nrrdDefaultGetenv();
  nrrdStateGetenv();

  /* no harm done in making sure we're sane */
  nrrdSanityOrDie(me);

  mop = airMopNew();
  hparm = hestParmNew();
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hparm->elideSingleEnumType = AIR_TRUE;
  hparm->elideSingleOtherType = AIR_TRUE;
  hparm->elideSingleOtherDefault = AIR_FALSE;
  hparm->elideSingleNonExistFloatDefault = AIR_TRUE;
  hparm->elideMultipleNonExistFloatDefault = AIR_TRUE;
  hparm->elideSingleEmptyStringDefault = AIR_TRUE;
  hparm->elideMultipleEmptyStringDefault = AIR_TRUE;
  hparm->cleverPluralizeOtherY = AIR_TRUE;
  hparm->columns = 78;

  /* if there are no arguments, then we give general usage information */
  if (1 >= argc) {
    unrrduUsage(TEND, hparm, tendTitle, tendCmdList);
    airMopError(mop);
    exit(1);
  }
  /* else, we see if its --version */
  if (!strcmp("--version", argv[1])) {
    printf("Teem version %s (%s)\n",
           airTeemVersion, airTeemReleaseDate);
    exit(0);
  }
  /* else, we should see if they're asking for a command we know about */
  for (i=0; tendCmdList[i]; i++) {
    if (!strcmp(argv[1], tendCmdList[i]->name))
      break;
    if (!strcmp("--help", argv[1])
        && !strcmp("about", tendCmdList[i]->name)) {
      break;
    }
  }
  if (tendCmdList[i]) {
    /* yes, we have that command */
    /* initialize variables used by the various commands */
    argv0 = (char *)calloc(strlen(TEND) + strlen(argv[1]) + 2, sizeof(char));
    airMopMem(mop, &argv0, airMopAlways);
    sprintf(argv0, "%s %s", TEND, argv[1]);

    /* run the individual unu program, saving its exit status */
    ret = tendCmdList[i]->main(argc-2, argv+2, argv0, hparm);
  } else {
    fprintf(stderr, "%s: unrecognized command: \"%s\"; type \"%s\" for "
            "complete list\n", me, argv[1], me);
    ret = 1;
  }

  airMopDone(mop, ret);
  return ret;
}
