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

#include "teem/ten.h"

/*
** Tests:
*/

int
main(int argc, const char **argv) {
  const char *me;
  airArray *mop;

  hestParm *hparm;
  unsigned int tci;
  FILE *out;
  int ret;

  AIR_UNUSED(argc);
  out = stdout;
  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);

  /* (same purpose as Testing/unrrdu/unulist.c) */

  fprintf(out, "%s: ################### BEGIN tend\n", me);
  ret = unrrduUsage("tend", hparm, tendTitle, tendCmdList);
  fprintf(out, "%s: ################### END tend (ret=%d)\n", me, ret);

  tci = 0;
  do {
    fprintf(out, "%s: ################### BEGIN tend %s\n",
            me, tendCmdList[tci]->name);
    ret = tendCmdList[tci]->main(0, NULL, tendCmdList[tci]->name, hparm);
    fprintf(out, "%s: ################### END tend %s (ret=%d)\n",
            me, tendCmdList[tci]->name, ret);
    tci++;
  } while (tendCmdList[tci]);

  airMopOkay(mop);
  return 0;
}
