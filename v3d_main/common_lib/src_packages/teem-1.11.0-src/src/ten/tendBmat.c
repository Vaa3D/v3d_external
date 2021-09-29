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

#include "ten.h"
#include "privateTen.h"

#define INFO "Calculate B-matrix given gradient directions"
static const char *_tend_bmatInfoL =
  (INFO
   ", assuming no diffusion weighting from the other imaging gradients. "
   "The input is a 3-by-N array of floats or doubles, each row being "
   "one of the gradient directions used for diffusion-weighted imaging. "
   "A plain text file with one gradient per line, no punctuation, is an "
   "easy way to specify this information. "
   "The gradient vector coefficients are used as is, without normalization "
   "(since different gradient strengths are sometimes desired). "
   "The output has one row of the B-matrix per line, with coefficient "
   "ordering Bxx, Bxy, Bxz, Byy, Byz, Bzz, and with the off-diagonal "
   "elements NOT pre-multiplied by 2.");

int
tend_bmatMain(int argc, const char **argv, const char *me,
              hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  Nrrd *ngrad, *nout;
  char *outS;

  hestOptAdd(&hopt, "i", "grads", airTypeOther, 1, 1, &ngrad, NULL,
             "array of gradient directions", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
             "output B matrix");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_bmatInfoL);
  JUSTPARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (tenBMatrixCalc(nout, ngrad)) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble making B matrix:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
unrrduCmd tend_bmatCmd = { "bmat", INFO, tend_bmatMain };
