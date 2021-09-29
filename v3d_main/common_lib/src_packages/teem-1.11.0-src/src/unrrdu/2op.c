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

#define INFO "Binary operation on two nrrds, or on a nrrd and a constant"
static const char *_unrrdu_2opInfoL =
(INFO
 ". Either the first or second operand can be a float constant, "
 "but not both.  Use \"-\" for an operand to signify "
 "a nrrd to be read from stdin (a pipe).  Note, however, "
 "that \"-\" can probably only be used once (reliably).\n "
 "* Uses nrrdArithIterBinaryOp or (with -w) nrrdArithIterBinaryOpSelect");

int
unrrdu_2opMain(int argc, const char **argv, const char *me,
               hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err, *seedS;
  NrrdIter *in1, *in2;
  Nrrd *nout, *ntmp=NULL;
  int op, type, E, pret, which;
  airArray *mop;
  unsigned int seed;

  hestOptAdd(&opt, NULL, "operator", airTypeEnum, 1, 1, &op, NULL,
             "Binary operator. Possibilities include:\n "
             "\b\bo \"+\", \"-\", \"x\", \"/\": "
             "add, subtract, multiply, divide\n "
             "\b\bo \"^\": exponentiation (pow)\n "
             "\b\bo \"spow\": signed exponentiation: sgn(x)pow(abs(x),p)\n "
             "\b\bo \"fpow\": like spow but with curves flipped\n "
             "\b\bo \"%\": integer modulo\n "
             "\b\bo \"fmod\": same as fmod() in C\n "
             "\b\bo \"atan2\": same as atan2() in C\n "
             "\b\bo \"min\", \"max\": minimum, maximum\n "
             "\b\bo \"lt\", \"lte\", \"gt\", \"gte\": same as C's <, <=, >, <=\n "
             "\b\bo \"eq\", \"neq\": same as C's == and !=\n "
             "\b\bo \"comp\": -1, 0, or 1 if 1st value is less than, "
             "equal to, or greater than 2nd value\n "
             "\b\bo \"if\": if 1st value is non-zero, use it, "
             "else use 2nd value\n "
             "\b\bo \"exists\": if 1st value exists, use it, "
             "else use 2nd value\n "
             "\b\bo \"nrand\": scale unit-stdv Gaussian noise by 2nd value "
             "and add to first value\n "
             "\b\bo \"rrand\": sample Rician distribution with 1st value "
             "for \"true\" mean, and 2nd value for sigma",
             NULL, nrrdBinaryOp);
  hestOptAdd(&opt, NULL, "in1", airTypeOther, 1, 1, &in1, NULL,
             "First input.  Can be a single value or a nrrd.",
             NULL, NULL, nrrdHestIter);
  hestOptAdd(&opt, NULL, "in2", airTypeOther, 1, 1, &in2, NULL,
             "Second input.  Can be a single value or a nrrd.",
             NULL, NULL, nrrdHestIter);
  hestOptAdd(&opt, "s,seed", "seed", airTypeString, 1, 1, &seedS, "",
             "seed value for RNG for nrand, so that you "
             "can get repeatable results between runs, or, "
             "by not using this option, the RNG seeding will be "
             "based on the current time");
  hestOptAdd(&opt, "t,type", "type", airTypeOther, 1, 1, &type, "default",
             "type to convert all INPUT nrrds to, prior to "
             "doing operation, useful for doing, for instance, the difference "
             "between two unsigned char nrrds.  This will also determine "
             "output type. By default (not using this option), the types of "
             "the input nrrds are left unchanged.",
             NULL, NULL, &unrrduHestMaybeTypeCB);
  hestOptAdd(&opt, "w,which", "arg", airTypeInt, 1, 1, &which, "-1",
             "Which argument (0 or 1) should be used to determine the "
             "shape of the output nrrd. By default (not using this option), "
             "the first non-constant argument is used. ");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_2opInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  /*
  fprintf(stderr, "%s: op = %d\n", me, op);
  fprintf(stderr, "%s: in1->left = %d, in2->left = %d\n", me,
          (int)(in1->left), (int)(in2->left));
  */
  if (nrrdTypeDefault != type) {
    /* they wanted to convert nrrds to some other type first */
    E = 0;
    if (in1->ownNrrd) {
      if (!E) E |= nrrdConvert(ntmp=nrrdNew(), in1->ownNrrd, type);
      if (!E) nrrdIterSetOwnNrrd(in1, ntmp);
    }
    if (in2->ownNrrd) {
      if (!E) E |= nrrdConvert(ntmp=nrrdNew(), in2->ownNrrd, type);
      if (!E) nrrdIterSetOwnNrrd(in2, ntmp);
    }
    if (E) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error converting input nrrd(s):\n%s", me, err);
      airMopError(mop);
      return 1;
    }
    /* this will still leave a nrrd in the NrrdIter for nrrdIterNix()
       (called by hestParseFree() called be airMopOkay()) to clear up */
  }
  /*
  ** Used to only deal with RNG seed for particular op:
  **   if (nrrdBinaryOpNormalRandScaleAdd == op) {
  ** but then when nrrdBinaryOpRicianRand was added, then the seed wasn't being
  ** used ==> BUG.  To be more future proof, we try to parse and use seed
  ** whenever a non-empty string is given, and end up *ALWAYS* calling
  ** airSrandMT, even for operations that have nothing to do with random
  ** numbers.  Could also have a new array that indicates if an op involves
  ** the RNG, but this would add rarely-needed complexity
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
  if (-1 == which
      ? nrrdArithIterBinaryOp(nout, op, in1, in2)
      : nrrdArithIterBinaryOpSelect(nout, op, in1, in2,
                                    AIR_CAST(unsigned int, which))) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error doing binary operation:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(2op, INFO);
