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

#include "teem/nrrd.h"

#define BKEY "tskip"

#if defined(WIN32) || defined(_WIN32)
#  define COMMIT "c"
#else
#  define COMMIT ""
#endif

/*
** Tests:
** nrrdLoad with positive and negative byte skipping on data read,
** with nrrdEncodingRaw
*/

static const char *tskipInfo = "for testing byte skipping in nrrd files";

int
main(int argc, const char **argv) {
  /* stock variables */
  char me[] = BKEY;
  hestOpt *hopt=NULL;
  hestParm *hparm;
  airArray *mop;
  /* variables specific to this program */
  int negskip, progress;
  Nrrd *nref, *nin;
  size_t *size, ii, nn, tick, pad[2];
  unsigned int axi, refCRC, gotCRC, sizeNum;
  char *berr, *outS[2], stmp[AIR_STRLEN_SMALL], doneStr[AIR_STRLEN_SMALL];
  airRandMTState *rng;
  unsigned int seed, *rdata, printbytes;
  unsigned char *dataUC;
  double time0, time1;
  FILE *fout;

  /* start-up */
  mop = airMopNew();
  hparm = hestParmNew();
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  /* learn things from hest */
  hestOptAdd(&hopt, "seed", "N", airTypeUInt, 1, 1, &seed, "42",
             "seed for RNG");
  hestOptAdd(&hopt, "s", "sz0", airTypeSize_t, 1, -1, &size, NULL,
             "sizes of desired output", &sizeNum);
  hestOptAdd(&hopt, "p", "pb pa", airTypeSize_t, 2, 2, pad, "0 0",
             "bytes of padding before, and after, the data segment "
             "in the written data");
  hestOptAdd(&hopt, "ns", "bool", airTypeInt, 0, 0, &negskip, NULL,
             "skipping should be relative to end of file");
  hestOptAdd(&hopt, "pb", "print", airTypeUInt, 1, 1, &printbytes, "0",
             "bytes to print at beginning and end of data, to help "
             "debug problems");
  hestOptAdd(&hopt, "o", "out.data out.nhdr", airTypeString, 2, 2,
             outS, NULL, "output filenames of data and header");
  hestParseOrDie(hopt, argc-1, argv+1, hparm, me, tskipInfo,
                 AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  /* generate reference nrrd data */
  nref = nrrdNew();
  airMopAdd(mop, nref, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdMaybeAlloc_nva(nref, nrrdTypeUInt, sizeNum, size)) {
    airMopAdd(mop, berr=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error allocating data: %s\n", me, berr);
    airMopError(mop); return 1;
  }
  rng = airRandMTStateNew(seed);
  airMopAdd(mop, rng, (airMopper)airRandMTStateNix, airMopAlways);
  nn = nrrdElementNumber(nref);
  rdata = AIR_CAST(unsigned int *, nref->data);
  fprintf(stderr, "generating data: . . .       "); fflush(stderr);
  time0 = airTime();
  progress = AIR_FALSE;
  tick = nn/100;
  for (ii=0; ii<nn; ii++) {
    rdata[ii] = airUIrandMT_r(rng);
    if (ii && tick && !(ii % tick)) {
      time1 = airTime();
      if (time1 - time0 > 1.0) {
        /* if it took more than a second to do 1% of the thing,
           would be good to generate some progress indication */
        progress = AIR_TRUE;
      }
      if (progress) {
        fprintf(stderr, "%s", airDoneStr(0, ii, nn, doneStr)); fflush(stderr);
      }
    }
  }
  if (progress) {
    fprintf(stderr, "%s\n", airDoneStr(0, ii, nn, doneStr)); fflush(stderr);
  } else {
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "finding reference (big-endian) CRC: "); fflush(stderr);
  refCRC = nrrdCRC32(nref, airEndianBig);
  fprintf(stderr, "%u\n", refCRC);

  /* write data, with padding */
  fprintf(stderr, "saving data . . . "); fflush(stderr);
  if (!(fout = fopen(outS[0], "wb" COMMIT))) {
    fprintf(stderr, "\n%s: couldn't open %s for writing: %s\n", me,
            outS[0], strerror(errno));
    airMopError(mop); return 1;
  }
  airMopAdd(mop, fout, (airMopper)airFclose, airMopAlways);
  for (ii=0; ii<pad[0]; ii++) {
    if (EOF == fputc(1, fout)) {
      fprintf(stderr, "\n%s: error doing pre-padding\n", me);
      airMopError(mop); return 1;
    }
  }
  if (nn != fwrite(nref->data, nrrdElementSize(nref), nn, fout)) {
    fprintf(stderr, "\n%s: error writing data\n", me);
    airMopError(mop); return 1;
  }
  for (ii=0; ii<pad[1]; ii++) {
    if (EOF == fputc(2, fout)) {
      fprintf(stderr, "\n%s: error doing post-padding\n", me);
      airMopError(mop); return 1;
    }
  }
  if (EOF == fflush(fout)) {
    fprintf(stderr, "\n%s: error fflushing data: %s\n", me,
            strerror(errno));
  }
  fprintf(stderr, "\n");
  if (printbytes) {
    size_t bi, rpb, nn;
    char stmp[AIR_STRLEN_SMALL];
    nn = nrrdElementSize(nref)*nrrdElementNumber(nref);
    rpb = AIR_MIN(printbytes, nn);
    dataUC = AIR_CAST(unsigned char *, nref->data);
    fprintf(stderr, "CORRECT %s bytes at beginning:\n",
            airSprintSize_t(stmp, rpb));
    for (bi=0; bi<rpb; bi++) {
      fprintf(stderr, "%x ", dataUC[bi]);
    }
    fprintf(stderr, "...\n");
    fprintf(stderr, "CORRECT %s bytes at end:\n",
            airSprintSize_t(stmp, rpb));
    fprintf(stderr, "...");
    for (bi=nn - rpb; bi<nn; bi++) {
      fprintf(stderr, " %x", dataUC[bi]);
    }
    fprintf(stderr, "\n");
  }
  airMopSingleOkay(mop, fout);
  airMopSingleOkay(mop, nref); nref = NULL;

  /* write header; for now just writing the header directly */
  fprintf(stderr, "writing header . . . \n");
  if (!(fout = fopen(outS[1], "w"))) {
    fprintf(stderr, "%s: couldn't open %s for writing: %s\n", me,
            outS[1], strerror(errno));
    airMopError(mop); return 1;
  }
  airMopAdd(mop, fout, (airMopper)airFclose, airMopAlways);
  fprintf(fout, "NRRD0005\n");
  fprintf(fout, "type: unsigned int\n");
  fprintf(fout, "dimension: %u\n", sizeNum);
  fprintf(fout, "sizes:");
  for (axi=0; axi<sizeNum; axi++) {
    fprintf(fout, " %s", airSprintSize_t(stmp, size[axi]));
  }
  fprintf(fout, "\n");
  fprintf(fout, "endian: %s\n", airEnumStr(airEndian, airMyEndian()));
  fprintf(fout, "encoding: %s\n", airEnumStr(nrrdEncodingType,
                                             nrrdEncodingTypeRaw));
  if (!negskip) {
    if (pad[0]) {
      fprintf(fout, "byte skip: %s\n", airSprintSize_t(stmp, pad[0]));
    }
  } else {
    fprintf(fout, "byte skip: -%s\n", airSprintSize_t(stmp, pad[1]+1));
  }
  fprintf(fout, "data file: %s\n", outS[0]);
  airMopSingleOkay(mop, fout);

  /* read it in, make sure it checks out */
  fprintf(stderr, "reading data . . . \n");
  nin = nrrdNew();
  airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdLoad(nin, outS[1], NULL)) {
    airMopAdd(mop, berr=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error reading back in: %s\n", me, berr);
    airMopError(mop); return 1;
  }
  if (printbytes) {
    size_t bi, rpb, nn;
    char stmp[AIR_STRLEN_SMALL];
    nn = nrrdElementSize(nin)*nrrdElementNumber(nin);
    rpb = AIR_MIN(printbytes, nn);
    dataUC = AIR_CAST(unsigned char *, nin->data);
    fprintf(stderr, "FOUND %s bytes at beginning:\n",
            airSprintSize_t(stmp, rpb));
    for (bi=0; bi<rpb; bi++) {
      fprintf(stderr, "%x ", dataUC[bi]);
    }
    fprintf(stderr, "...\n");
    fprintf(stderr, "FOUND %s bytes at end:\n",
            airSprintSize_t(stmp, rpb));
    fprintf(stderr, "...");
    for (bi=nn - rpb; bi<nn; bi++) {
      fprintf(stderr, " %x", dataUC[bi]);
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "finding new CRC . . . \n");
  gotCRC = nrrdCRC32(nin, airEndianBig);
  if (refCRC != gotCRC) {
    fprintf(stderr, "%s: got CRC %u but wanted %u\n", me, gotCRC, refCRC);
    airMopError(mop); return 1;
  }
  fprintf(stderr, "(all ok)\n");

  /* HEY: to test gzip reading, we really want to do a system call to
     gzip compress the data, and write a new header to point to the
     compressed data, and make sure we can read in that just the same */

  airMopOkay(mop);
  return 0;
}
