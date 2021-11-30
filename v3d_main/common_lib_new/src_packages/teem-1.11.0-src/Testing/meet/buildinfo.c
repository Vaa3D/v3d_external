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

#include "teem/meet.h"

/*
** describes how this Teem was built
*/

int
main(int argc, const char **argv) {
  unsigned int ii;
#if defined(TEEM_BUILD_EXPERIMENTAL_LIBS)
  char explibs[] = "*ON!*";
#else
  char explibs[] = "_off_";
#endif

  /* apparently TEEM_BUILD_EXPERIMENTAL_APPS is not disclosed to
     the compilation of this file? */
#if defined(TEEM_BUILD_EXPERIMENTAL_APPS)
  char expapps[] = "*ON!*";
#else
  char expapps[] = "_off_";
#endif

  char stmp1[AIR_STRLEN_SMALL], stmp2[AIR_STRLEN_SMALL];
  AIR_UNUSED(argc);
  AIR_UNUSED(argv);

  printf("Teem version %s, %s\n", airTeemVersion, airTeemReleaseDate);

  /* some of the things from airSanity */
  printf("airMyEndian() == %d\n", airMyEndian());
  printf("AIR_QNANHIBIT == %d\n", AIR_QNANHIBIT);
  printf("sizeof(size_t) = %s; sizeof(void*) = %s\n",
         airSprintSize_t(stmp1, sizeof(size_t)),
         airSprintSize_t(stmp2, sizeof(void*)));

  printf("experimental libs %s; apps %s\n", explibs, expapps);
  printf("libs = ");
  ii = 0;
  do {
    printf("%s ", meetTeemLibs[ii]);
    ii++;
  } while (meetTeemLibs[ii]);
  printf("(%u)\n", ii);

  printf("airThreadCapable = %d\n", airThreadCapable);

  printf("nrrdFFTWEnabled = %d\n", nrrdFFTWEnabled);

#if TEEM_LEVMAR
  printf("yes, TEEM_LEVMAR #defined\n");
#else
  printf(" no, TEEM_LEVMAR not #defined\n");
#endif

  return 0;
}
