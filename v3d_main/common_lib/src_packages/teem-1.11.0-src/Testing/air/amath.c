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
** airLog2, airCbrt
*/

int
main(int argc, const char *argv[]) {
  char stmp[AIR_STRLEN_SMALL];
  airArray *mop;

  AIR_UNUSED(argc);
  AIR_UNUSED(argv);
  mop = airMopNew();

  { /* airLog2 */
    int ret, l2 = 0;
    size_t ee = 1;
    do {
      if (l2 != (ret = airLog2(ee))) {
        fprintf(stderr, "airLog2(%s) = %d != %d\n",
                airSprintSize_t(stmp, ee), ret, l2);
        airMopError(mop); return 1;
      }
      if (ee > 1) {
        if (-1 != (ret = airLog2(ee+1))) {
          fprintf(stderr, "airLog2(%s) = %d != -1\n",
                  airSprintSize_t(stmp, ee+1), ret);
          airMopError(mop); return 1;
        }
      }
      l2 += 1;
      ee *= 2;
    } while (l2 < 31);
  }

  { /* airCbrt */
    unsigned int pi, ti, testnum = 2000;
    airRandMTState *rng;
    double aa[2], uu, eps = 8e-27, error;
    rng = airRandMTStateNew(4242);
    airMopAdd(mop, rng, (airMopper)airRandMTStateNix, airMopAlways);
    error = 0;
    for (ti=0; ti<testnum; ti++) {
      double dif;
      airNormalRand_r(aa+0, aa+1, rng);
      for (pi=0; pi<2; pi++) {
        aa[pi] *= 10000;
        uu = airCbrt(aa[pi]);
        dif = fabs(aa[pi] - uu*uu*uu);
        error += (dif/aa[pi])*dif;
      }
    }
    error /= testnum;
    if (error > eps) {
      fprintf(stderr, "average cbrt error was %.17g > eps %0.17g\n",
              error, eps);
      airMopError(mop); return 1;
    } else {
      fprintf(stderr, "average cbrt error = %.17g\n", error);
    }
  }

  exit(0);
}



