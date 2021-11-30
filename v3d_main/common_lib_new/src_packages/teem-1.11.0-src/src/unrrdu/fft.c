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

#define INFO "Fast Fourier Transform of selected axes"
static const char *_unrrdu_fftInfoL_yes =
  (INFO
   ". Initial attempt at wrapping the FFTW3 library; options are "
   "likely to change in Teem 2.0.\n "
   "* Uses nrrdFFT");

static const char *_unrrdu_fftInfoL_no =
  (INFO
   ". This Teem has NOT been compiled with FFTW3 <http://www.fftw.org/>. "
   "If it had been, "
   "this would be a command-line interface to that functionality. "
   "There is currently no non-FFTW implementation of the FFT available.\n "
   "* Uses nrrdFFT");

/* We create an airEnum to parse the "forward" and "backwards" values
   needed to specify which kind of transform to run */

static const char *
_directionStr[] = {
  "(unknown direction)",
  "forward",
  "backward"
};

static const char *
_directionDesc[] = {
  "unknown direction",
  "forward transform",
  "backward (inverse) transform"
};

/*  from fftw3.h
#define FFTW_FORWARD (-1)
#define FFTW_BACKWARD (+1)
*/

#define FORW (-1)
#define BACK (+1)

static const int
_directionVal[] = {
  0,
  FORW,
  BACK
};

static const char *
_directionStrEqv[] = {
  "f", "forw", "forward",
  "b", "back", "backward", "i", "inv", "inverse",
  ""
};

static const int
_directionValEqv[] = {
  FORW, FORW, FORW,
  BACK, BACK, BACK, BACK, BACK, BACK
};

static const airEnum
_direction_enm = {
  "direction",
  2,
  _directionStr,
  _directionVal,
  _directionDesc,
  _directionStrEqv,
  _directionValEqv,
  AIR_FALSE
};

static const airEnum *const
direction_enm = &_direction_enm;

int
unrrdu_fftMain(int argc, const char **argv, const char *me,
               hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *_nin, *nout;
  int pret;
  airArray *mop;

  int sign, rigor, rescale, realInput;
  char *wispath;
  FILE *fwise;
  unsigned int *axes, axesLen;

  hestOptAdd(&opt, NULL, "dir", airTypeEnum, 1, 1, &sign, NULL,
             "forward (\"forw\", \"f\") or backward/inverse "
             "(\"back\", \"b\") transform ", NULL, direction_enm);
  hestOptAdd(&opt, "a,axes", "ax0", airTypeUInt, 1, -1, &axes, NULL,
             "the one or more axes that should be transformed", &axesLen);
  hestOptAdd(&opt, "pr,planrigor", "pr", airTypeEnum, 1, 1, &rigor, "est",
             "rigor with which fftw plan is constructed. Options include:\n "
             "\b\bo \"e\", \"est\", \"estimate\": only an estimate\n "
             "\b\bo \"m\", \"meas\", \"measure\": standard amount of "
             "measurements of system properties\n "
             "\b\bo \"p\", \"pat\", \"patient\": slower, more measurements\n "
             "\b\bo \"x\", \"ex\", \"exhaustive\": slowest, most measurements",
             NULL, nrrdFFTWPlanRigor);
  hestOptAdd(&opt, "r,rescale", "bool", airTypeBool, 1, 1, &rescale, "true",
             "scale fftw output (by sqrt(1/N)) so that forward and backward "
             "transforms will get back to original values");
  hestOptAdd(&opt, "w,wisdom", "filename", airTypeString, 1, 1, &wispath, "",
             "A filename here is used to read in fftw wisdom (if the file "
             "exists already), and is used to save out updated wisdom "
             "after the transform.  By default (not using this option), "
             "no wisdom is read or saved. Note: no wisdom is gained "
             "(that is, learned by FFTW) with planning rigor \"estimate\".");
  OPT_ADD_NIN(_nin, "input nrrd");
  hestOptAdd(&opt, "ri,realinput", NULL, airTypeInt, 0, 0, &realInput, NULL,
             "input is real-valued, so insert new length-2 axis 0 "
             "and set complex component to 0.0.  Axes to transform "
             "(indicated by \"-a\") will be incremented accordingly.");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  if (nrrdFFTWEnabled) {
    USAGE(_unrrdu_fftInfoL_yes);
  } else {
    USAGE(_unrrdu_fftInfoL_no);
  }
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (realInput) {
    ptrdiff_t minPad[NRRD_DIM_MAX], maxPad[NRRD_DIM_MAX];
    unsigned int axi;
    Nrrd *ntmp;
    ntmp = nrrdNew();
    airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdAxesInsert(ntmp, _nin, 0)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error creating complex axis:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
    nin = nrrdNew();
    airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);
    minPad[0] = 0;
    maxPad[0] = 1;
    for (axi=1; axi<ntmp->dim; axi++) {
      minPad[axi] = 0;
      maxPad[axi] = AIR_CAST(ptrdiff_t, ntmp->axis[axi].size-1);
    }
    if (nrrdPad_nva(nin, ntmp, minPad, maxPad, nrrdBoundaryPad, 0.0)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error padding out complex axis:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
    /* increment specified axes to transform */
    for (axi=0; axi<axesLen; axi++) {
      axes[axi]++;
    }
    /* ntmp is really done with, we can free up the space now; this
       is one of the rare times we want airMopSub */
    airMopSub(mop, ntmp, (airMopper)nrrdNuke);
    nrrdNuke(ntmp);
  } else {
    /* input is apparently already complex */
    nin = _nin;
  }

  if (airStrlen(wispath) && nrrdFFTWEnabled) {
    fwise = fopen(wispath, "r");
    if (fwise) {
      if (nrrdFFTWWisdomRead(fwise)) {
        airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
        fprintf(stderr, "%s: error with fft wisdom:\n%s", me, err);
        airMopError(mop);
        return 1;
      }
      fclose(fwise);
    } else {
      fprintf(stderr, "%s: (\"%s\" couldn't be opened, will try to save "
              "wisdom afterwards)", me, wispath);
    }
  }

  if (nrrdFFT(nout, nin, axes, axesLen, sign, rescale, rigor)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error with fft:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  if (airStrlen(wispath) && nrrdFFTWEnabled) {
    if (!(fwise = fopen(wispath, "w"))) {
      fprintf(stderr, "%s: couldn't open %s for writing: %s\n",
              me, wispath, strerror(errno));
      airMopError(mop);
      return 1;
    }
    if (nrrdFFTWWisdomWrite(fwise)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error with fft wisdom:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
    fclose(fwise);
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(fft, INFO);
