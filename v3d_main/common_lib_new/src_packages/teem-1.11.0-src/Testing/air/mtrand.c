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
** airRandMTStateNew
** airRandMTStateNix
** airSrandMT_r
** airRandMT
**
** Also uses:
** airMopNew, airMopAdd, airMopError, airMopDone
*/

#define NUM 10
int
main(int argc, const char *argv[]) {
  airArray *mop;
  airRandMTState *rng;
  const char *me;
  unsigned int rval[NUM][NUM] = {
    {2357136044u, 2546248239u, 3071714933u, 3626093760u, 2588848963u,
     3684848379u, 2340255427u, 3638918503u, 1819583497u, 2678185683u},
    {1608637542u, 3421126067u, 4083286876u, 787846414u, 3143890026u,
     3348747335u, 2571218620u, 2563451924u, 670094950u, 1914837113u},
    {197744554u, 2405527281u, 1590178649u, 2055114040u, 1040749045u,
     1355459964u, 2699301070u, 1591340141u, 4252490304u, 3121394926u},
    {451710822u, 4140706415u, 550374602u, 880776961u, 375407235u,
     576831824u, 495976644u, 1350392909u, 3211465673u, 1227650870u},
    {2567526101u, 397661439u, 2237017401u, 316000557u, 1060138423u,
     2802111455u, 1449535759u, 751581949u, 3635455645u, 658021748u},
    {429171210u, 2009581671u, 1300722668u, 3858470021u, 3363216262u,
     1963629412u, 2166299591u, 229689286u, 484002369u, 2062223911u},
    {23250075u, 3670330222u, 1860540774u, 4216169317u, 1062279565u,
     2886996639u, 2197431119u, 3112004045u, 3229777453u, 1632140913u},
    {2869147098u, 1558248213u, 585501645u, 3600180646u, 2654279825u,
     3658135664u, 287832047u, 912891514u, 2926707351u, 937957965u},
    {1891499427u, 1885608988u, 3850740167u, 3832766153u, 2073041664u,
     3289176644u, 989474400u, 2841420218u, 4096852366u, 1816963771u},
    {2552602868u, 2086504389u, 219288614u, 3347214808u, 215326247u,
     3609464630u, 3506494207u, 997691580u, 1726903302u, 3302470737u}
  };
  unsigned int ii, jj, rr;

  AIR_UNUSED(argc);
  me = argv[0];

  mop = airMopNew();
  rng = airRandMTStateNew(0);
  airMopAdd(mop, rng, (airMopper)airRandMTStateNix, airMopAlways);
  for (jj=0; jj<NUM; jj++) {
    airSrandMT_r(rng, 42*jj);
    for (ii=0; ii<NUM; ii++) {
      rr = airUIrandMT_r(rng);
      if (rval[jj][ii] != rr) {
        fprintf(stderr, "%s: rval[%u][%u] %u != %u\n",
                me, jj, ii, rval[jj][ii], rr);
        airMopError(mop);
        exit(1);
      }
    }
  }

  airMopOkay(mop);
  exit(0);
}



