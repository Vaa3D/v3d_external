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

#define INFO "Compute 32-bit CRC of nrrd data (same as via \"cksum\")"
static const char *_unrrdu_cksumInfoL =
(INFO ". Unlike other commands, this doesn't produce a nrrd.  It only "
 "prints to standard out the CRC and byte counts for the input nrrd(s), "
 "seeking to emulate the formatting of cksum output.\n "
 "* Uses nrrdCRC32");

int
unrrdu_cksumDoit(const char *me, char *inS, int endian,
                 int printendian, FILE *fout) {
  Nrrd *nrrd;
  airArray *mop;
  unsigned int crc;
  char stmp[AIR_STRLEN_SMALL], ends[AIR_STRLEN_SMALL];
  size_t nn;

  mop = airMopNew();
  airMopAdd(mop, nrrd=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdLoad(nrrd, inS, NULL)) {
    biffMovef(me, NRRD, "%s: trouble loading \"%s\"", me, inS);
    airMopError(mop); return 1;
  }
  crc = nrrdCRC32(nrrd, endian);
  nn = nrrdElementNumber(nrrd)*nrrdElementSize(nrrd);
  sprintf(ends, "(%s)", airEnumStr(airEndian, endian));
  fprintf(fout, "%u%s %s%s%s\n", crc,
          printendian ? ends : "",
          airSprintSize_t(stmp, nn),
          strcmp("-", inS) ? " " : "",
          strcmp("-", inS) ? inS : "");

  airMopOkay(mop);
  return 0;
}

int
unrrdu_cksumMain(int argc, const char **argv, const char *me,
                 hestParm *hparm) {
  hestOpt *opt = NULL;
  char *err, **inS;
  airArray *mop;
  int pret, endian, printend;
  unsigned int ni, ninLen;

  mop = airMopNew();
  hestOptAdd(&opt, "en,endian", "end", airTypeEnum, 1, 1, &endian,
             airEnumStr(airEndian, airMyEndian()),
             "Endianness in which to compute CRC; \"little\" for Intel and "
             "friends; \"big\" for everyone else. "
             "Defaults to endianness of this machine",
             NULL, airEndian);
  hestOptAdd(&opt, "pen,printendian", "bool", airTypeBool, 1, 1, &printend,
             "false",
             "whether or not to indicate after the CRC value the endianness "
             "with which the CRC was computed; doing so clarifies "
             "that the CRC result depends on endianness and may remove "
             "confusion in comparing results on platforms of different "
             "endianness");
  hestOptAdd(&opt, NULL, "nin1", airTypeString, 1, -1, &inS, NULL,
             "input nrrd(s)", &ninLen);
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_cksumInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  for (ni=0; ni<ninLen; ni++) {
    if (unrrdu_cksumDoit(me, inS[ni], endian, printend, stdout)) {
      airMopAdd(mop, err = biffGetDone(me), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble with \"%s\":\n%s",
              me, inS[ni], err);
      /* continue working on the remaining files */
    }
  }

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(cksum, INFO);
