/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2011, 2010, 2009, 2008  University of Chicago
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

#include "../gage.h"

char *Info = ("for doing silly conversions");

int
main(int argc, const char *argv[]) {
  const char *me;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  double val;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, NULL, "val", airTypeDouble, 1, 1, &val, NULL,
             "input value");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, Info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  printf("%s: tee % 10.7g -> sig % 10.7g    tau % 10.7g\n", me, val,
         gageSigOfTau(gageTauOfTee(val)),
         gageTauOfTee(val));
  printf("%s: tau % 10.7g -> sig % 10.7g    tee % 10.7g\n", me, val,
         gageSigOfTau(val),
         gageTeeOfTau(val));
  printf("%s: sig % 10.7g -> tee % 10.7g    tau % 10.7g\n", me, val,
         gageTeeOfTau(gageTauOfSig(val)),
         gageTauOfSig(val));

  airMopOkay(mop);
  exit(0);
}

