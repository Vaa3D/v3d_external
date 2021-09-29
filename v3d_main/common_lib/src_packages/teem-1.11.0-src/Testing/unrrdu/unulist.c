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

#include "teem/unrrdu.h"

/*
** Tests:
*/

int
main(int argc, const char **argv) {
  const char *me;
  airArray *mop;

  hestParm *hparm;
  unsigned int uci;
  FILE *out;
  int ret;

  AIR_UNUSED(argc);
  out = stdout;
  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);

  /* This just generates all the usage information for unu itself, and then
     for all the unu sub-commands.  The purpose is to exercise hest's
     generation of usage info for all the options, and to make sure (by human
     inspection now, later by something automated) that the use of stderr vs
     stdout, and return values, is consistent across all commands */

  fprintf(out, "%s: ################### BEGIN unu\n", me);
  unrrduUsageUnu("unu", hparm);
  fprintf(out, "%s: ################### END unu\n", me);

  uci = 0;
  do {
    fprintf(out, "%s: ################### BEGIN unu %s\n",
            me, unrrduCmdList[uci]->name);
    ret = unrrduCmdList[uci]->main(0, NULL, unrrduCmdList[uci]->name, hparm);
    fprintf(out, "%s: ################### END unu %s (ret=%d)\n",
            me, unrrduCmdList[uci]->name, ret);
    uci++;
  } while (unrrduCmdList[uci]);

  airMopOkay(mop);
  return 0;
}
