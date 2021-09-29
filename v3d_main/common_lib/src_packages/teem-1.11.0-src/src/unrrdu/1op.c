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

#include "unrrdu.h"
#include "privateUnrrdu.h"

#define INFO "Unary operation on a nrrd"
static const char *_unrrdu_1opInfoL =
  (INFO
   ".\n "
   "* Uses nrrdArithUnaryOp");

int
unrrdu_1opMain(int argc, const char **argv, const char *me,
               hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err, *seedS;
  Nrrd *nin, *nout, *ntmp=NULL;
  int op, pret, type;
  airArray *mop;
  unsigned int seed;

  hestOptAdd(&opt, NULL, "operator", airTypeEnum, 1, 1, &op, NULL,
             "Unary operator. Possibilities include:\n "
             "\b\bo \"-\": negative (multiply by -1.0)\n "
             "\b\bo \"r\": reciprocal (1.0/value)\n "
             "\b\bo \"sin\", \"cos\", \"tan\", \"asin\", \"acos\", \"atan\": "
             "same as in C\n "
             "\b\bo \"exp\", \"log\", \"log10\": same as in C\n "
             "\b\bo \"log1p\", \"expm1\": accurate log(x+1) and exp(x)-1\n "
             "\b\bo \"log2\": log base 2\n "
             "\b\bo \"sqrt\", \"cbrt\", \"ceil\", \"floor\": same as in C\n "
             "\b\bo \"erf\": error function (integral of Gaussian)\n "
             "\b\bo \"rup\", \"rdn\": round up or down to integral value\n "
             "\b\bo \"abs\": absolute value\n "
             "\b\bo \"sgn\": -1, 0, 1 if value is <0, ==0, or >0\n "
             "\b\bo \"exists\": 1 iff not NaN or +/-Inf, 0 otherwise\n "
             "\b\bo \"rand\": random value in [0.0,1.0), "
             "no relation to input\n "
             "\b\bo \"nrand\": random sample from normal distribution with "
             "mean 0.0 and stdv 1.0, no relation to input\n "
             "\b\bo \"if\": if input is non-zero, 1, else 0\n "
             "\b\bo \"0\": output always 0\n "
             "\b\bo \"1\": output always 1",
             NULL, nrrdUnaryOp);
  hestOptAdd(&opt, "s,seed", "seed", airTypeString, 1, 1, &seedS, "",
             "seed value for RNG for rand and nrand, so that you "
             "can get repeatable results between runs, or, "
             "by not using this option, the RNG seeding will be "
             "based on the current time");
  hestOptAdd(&opt, "t,type", "type", airTypeOther, 1, 1, &type, "default",
             "convert input nrrd to this type prior to "
             "doing operation.  Useful when desired output is float "
             "(e.g., with log1p), but input is integral. By default "
             "(not using this option), the types of "
             "the input nrrds are left unchanged.",
             NULL, NULL, &unrrduHestMaybeTypeCB);
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_1opInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdTypeDefault != type) {
    /* they requested conversion to another type prior to the 1op */
    airMopAdd(mop, ntmp=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    if (nrrdConvert(ntmp, nin, type)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error converting input nrrd:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
  } else {
    ntmp = nin;
  }
  /* see note in 2op.c about the hazards of trying to be clever
  ** about minimizing the seeding of the RNG
  ** if (nrrdUnaryOpRand == op
  **     || nrrdUnaryOpNormalRand == op) {
  */
  if (airStrlen(seedS)) {
    if (1 != sscanf(seedS, "%u", &seed)) {
      fprintf(stderr, "%s: couldn't parse seed \"%s\" as uint\n", me, seedS);
      airMopError(mop);
      return 1;
    } else {
      airSrandMT(seed);
    }
  } else {
    /* got no request for specific seed */
    airSrandMT(AIR_CAST(unsigned int, airTime()));
  }
  if (nrrdArithUnaryOp(nout, op, ntmp)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error doing unary operation:\n%s", me, err);
    airMopError(mop);
    return 1;
  }
  /* if we had to create ntmp with nrrdConvert, it will be mopped,
     otherwise ntmp is an alias for nin, which will also be mopped */

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(1op, INFO);
