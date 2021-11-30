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

#define INFO "Affine (lerp) mapping on 5 nrrds or constants"
static const char *_unrrdu_affineInfoL =
(INFO
 ". All the 5 arguments can be either nrrds or single "
 "floating-point values.  When all args are single values, this "
 "is subsuming the functionality of the previous stand-alone "
 "\"affine\" program. "
 "Use \"-\" for an operand to signify "
 "a nrrd to be read from stdin (a pipe).  Note, however, "
 "that \"-\" can probably only be used once (reliably).\n "
 "* Uses nrrdArithAffine or nrrdArithIterAffine");

int
unrrdu_affineMain(int argc, const char **argv, const char *me,
                  hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  NrrdIter *minIn, *in, *maxIn, *minOut, *maxOut, *args[5];
  Nrrd *nout, *ntmp=NULL;
  int type, E, pret, clamp;
  airArray *mop;
  unsigned int ai, nn;

  hestOptAdd(&opt, NULL, "minIn", airTypeOther, 1, 1, &minIn, NULL,
             "Lower end of input value range.",
             NULL, NULL, nrrdHestIter);
  hestOptAdd(&opt, NULL, "in", airTypeOther, 1, 1, &in, NULL,
             "Input value.",
             NULL, NULL, nrrdHestIter);
  hestOptAdd(&opt, NULL, "maxIn", airTypeOther, 1, 1, &maxIn, NULL,
             "Upper end of input value range.",
             NULL, NULL, nrrdHestIter);
  hestOptAdd(&opt, NULL, "minOut", airTypeOther, 1, 1, &minOut, NULL,
             "Lower end of output value range.",
             NULL, NULL, nrrdHestIter);
  hestOptAdd(&opt, NULL, "maxOut", airTypeOther, 1, 1, &maxOut, NULL,
             "Upper end of output value range.",
             NULL, NULL, nrrdHestIter);
  hestOptAdd(&opt, "t,type", "type", airTypeOther, 1, 1, &type, "default",
             "type to convert all nrrd inputs to, prior to "
             "doing operation.  This also determines output type. "
             "By default (not using this option), the types of the input "
             "nrrds are left unchanged.",
             NULL, NULL, &unrrduHestMaybeTypeCB);
  hestOptAdd(&opt, "clamp", "bool", airTypeBool, 1, 1, &clamp, "false",
             "clamp output values to specified output range");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_affineInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  args[0] = minIn;
  args[1] = in;
  args[2] = maxIn;
  args[3] = minOut;
  args[4] = maxOut;
  nn = 0;
  for (ai=0; ai<5; ai++) {
    nn += !!args[ai]->ownNrrd;
  }
  if (nrrdTypeDefault != type) {
    /* they wanted to convert nrrds to some other type first */
    E = 0;
    for (ai=0; ai<5; ai++) {
      if (args[ai]->ownNrrd) {
        if (!E) E |= nrrdConvert(ntmp=nrrdNew(), args[ai]->ownNrrd, type);
        if (!E) nrrdIterSetOwnNrrd(args[ai], ntmp);
      }
    }
    if (E) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error converting input nrrd(s):\n%s", me, err);
      airMopError(mop);
      return 1;
    }
  }

  if (0 == nn) {
    /* actually, there are no nrrds; we represent the functionality
       of the previous stand-alone binary "affine" */
    double valOut;
    valOut = AIR_AFFINE(minIn->val, in->val, maxIn->val,
                        minOut->val, maxOut->val);
    if (clamp) {
      valOut = AIR_CLAMP(minOut->val, valOut, maxOut->val);
    }
    printf("%g\n", valOut);
  } else {
    /* we have a nrrd output */
    if (1 == nn && in->ownNrrd) {
      E = nrrdArithAffine(nout, minIn->val, in->ownNrrd, maxIn->val,
                          minOut->val, maxOut->val, clamp);
    } else {
      E = nrrdArithIterAffine(nout, minIn, in, maxIn, minOut, maxOut, clamp);
    }
    if (E) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error doing ternary operation:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
    SAVE(out, nout, NULL);
  }

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(affine, INFO);

