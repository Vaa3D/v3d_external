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
** to test:

AIR_EXPORT FILE *airFopen(const char *name, FILE *std, const char *mode);
AIR_EXPORT FILE *airFclose(FILE *file);
AIR_EXPORT int airSinglePrintf(FILE *file, char *str, const char *fmt, ...);
AIR_EXPORT unsigned int airIndex(double min, double val, double max,
                                 unsigned int N);
AIR_EXPORT unsigned int airIndexClamp(double min, double val, double max,
                                      unsigned int N);
AIR_EXPORT airULLong airIndexULL(double min, double val, double max,
                                 airULLong N);
AIR_EXPORT airULLong airIndexClampULL(double min, double val, double max,
                                      airULLong N);
AIR_EXPORT char *airDoneStr(double start, double here, double end, char *str);
AIR_EXPORT void airBinaryPrintUInt(FILE *file, int digits, unsigned int N);
AIR_EXPORT int airILoad(void *v, int t);
AIR_EXPORT float airFLoad(void *v, int t);
AIR_EXPORT double airDLoad(void *v, int t);
AIR_EXPORT int airIStore(void *v, int t, int i);
AIR_EXPORT float airFStore(void *v, int t, float f);
AIR_EXPORT double airDStore(void *v, int t, double d);
AIR_EXPORT void airEqvAdd(airArray *eqvArr, unsigned int j, unsigned int k);
AIR_EXPORT unsigned int airEqvMap(airArray *eqvArr,
                                  unsigned int *map, unsigned int len);
AIR_EXPORT unsigned int airEqvSettle(unsigned int *map, unsigned int len);

*/

static size_t
multiply(size_t aa, size_t bb) {
  return aa*bb;
}

int
main(int argc, const char *argv[]) {
  const char *me;
  void *ptr, *ptr2;

  AIR_UNUSED(argc);
  me = argv[0];

  /* airNull */
  {
    ptr = airNull();
    if (NULL != ptr) {
      fprintf(stderr, "%s: airNull() returned %p not NULL\n", me, ptr);
      exit(1);
    }
  }

  /* airSetNull */
  {
    ptr = AIR_CAST(void *, airNull);
    if (NULL == ptr) {
      fprintf(stderr, "%s: couldn't set a non-NULL pointer", me);
      exit(1);
    }
    ptr2 = airSetNull(&ptr);
    if (!(NULL == ptr && NULL == ptr2)) {
      fprintf(stderr, "%s: airSetNull() didn't set (%p) or return (%p) NULL\n",
              me, ptr, ptr2);
      exit(1);
    }
  }

  /* AIR_CALLOC, airFree, airTime */
  {
    size_t big = 1024, times = 1, ii, jj;
    double time0, dtime;
    unsigned int *data, sum;
    big = big*big*128; /* 128 megs */
    time0 = airTime();
    sum = 0;
    for (ii=0; ii<times; ii++) {
      /* will have a memory leak if airFree() didn't free() */
      data = AIR_CALLOC(big, unsigned int);
      for (jj=0; jj<big; jj++) {
        sum += data[jj];
      }
      data = airFree(data);
    }
    if (sum) {
      fprintf(stderr, "%s: AIR_CALLOC produced non-zero values\n", me);
      exit(1);
    }
    if (!(NULL == data)) {
      fprintf(stderr, "%s: airFree() returned %p not NULL\n",
              me, AIR_VOIDP(data));
      exit(1);
    }
    dtime = airTime() - time0;
    if (!( dtime > 0 )) {
      fprintf(stderr, "%s: airTime() => nonsense delta time %g\n", me, dtime);
      exit(1);
    }
  }

  /* airStderr, airStdout, airStdin */
  {
    FILE *fret;
    fret = airStderr();
    if (stderr != fret) {
      fprintf(stderr, "%s: airStderr() returned %p not stderr %p\n", me,
              AIR_CAST(void *, fret), AIR_CAST(void *, stderr));
      exit(1);
    }
    fret = airStdout();
    if (stdout != fret) {
      fprintf(stdout, "%s: airStdout() returned %p not stdout %p\n", me,
              AIR_CAST(void *, fret), AIR_CAST(void *, stdout));
      exit(1);
    }
    fret = airStdin();
    if (stdin != fret) {
      fprintf(stdin, "%s: airStdin() returned %p not stdin %p\n", me,
              AIR_CAST(void *, fret), AIR_CAST(void *, stdin));
      exit(1);
    }
  }

  /* airSprintSize_t, airSprintPtrdiff_t in pptest.c */

  /* airPrettySprintSize_t */
  {
    char prstmp[AIR_STRLEN_SMALL];
    size_t vals[] = {0,                      /* 0 */
                     800,                    /* 1 */
                     1024,                   /* 2 = 2^10 */
                     1024 + 1,               /* 3 */
                     500*1024,               /* 4 */
                     1024*1024,              /* 5 = 2^20 */
                     1024*(1024 + 1),        /* 6 */
                     500*1024*1024,          /* 7 */
                     1024*1024*1024ul,       /* 8 = 2^30 */
                     1024*1024*(1024ul + 1), /* 9 */
                     500*1024ul,             /* 10 (will be multiplied below) */
                     1024*1024ul,            /* 11 = 2^40 */
                     1024*(1024ul + 1),      /* 12 */
                     500*1024*1024,          /* 13 */
                     1024*1024*1024ul,       /* 14 = 2^50 */
                     1024*1024*(1024ul + 1), /* 15 */
                     500*1024*1024,          /* 16 */
                     1024*1024*1024ul,       /* 17 = 2^60 */
                     1024*1024*(1024ul + 1), /* 18 */
                     2*1024*1024ul,          /* 19 = 2^61 */
                     16*1023*1024ul,         /* 20 */
                     16*1024*1024ul,         /* 21 = 2^64 */
                     0};
    char *string[] = {
      "0 bytes",    /* 0 */
      "800 bytes",  /* 1 */
      "1024 bytes", /* 2 */
      "1.00098 KB", /* 3 */
      "500 KB",     /* 4 */
      "1024 KB",    /* 5 */
      "1.00098 MB", /* 6 */
      "500 MB",     /* 7 */
      "1024 MB",    /* 8 */
      "1.00098 GB", /* 9 */
      "500 GB",     /* 10 */
      "1024 GB",    /* 11 */
      "1.00098 TB", /* 12 */
      "500 TB",     /* 13 */
      "1024 TB",    /* 14 */
      "1.00098 PB", /* 15 */
      "500 PB",     /* 16 */
      "1024 PB",    /* 17 */
      "1.00098 EB", /* 18 */
      "2 EB",       /* 19 */
      "15.9844 EB", /* 20 */
      "blah",       /* 21 */
    };
    unsigned int ii;
    /* hiding some multiplications in function calls,
       to quiet compiler warnings.  Its ok if there is
       wrap-around here because then we'll stop testing.
       However, it would be better if we did NOT rely on
       what is strictly speaking undefined behavior: the
       overflow of *unsigned* integral quantities. */
    vals[10] = multiply(multiply(vals[10], 1024), 1024);
    vals[11] = multiply(multiply(vals[11], 1024), 1024);
    vals[12] = multiply(multiply(vals[12], 1024), 1024);
    vals[13] = multiply(multiply(vals[13], 1024), 1024);
    vals[14] = multiply(multiply(vals[14], 1024), 1024);
    vals[15] = multiply(multiply(vals[15], 1024), 1024);
    vals[16] = multiply(multiply(multiply(vals[16], 1024), 1024), 1024);
    vals[17] = multiply(multiply(multiply(vals[17], 1024), 1024), 1024);
    vals[18] = multiply(multiply(multiply(vals[18], 1024), 1024), 1024);
    vals[19] = multiply(multiply(multiply(multiply(vals[19], 1024), 1024), 1024), 1024);
    vals[20] = multiply(multiply(multiply(multiply(vals[20], 1024), 1024), 1024), 1024);
    vals[21] = multiply(multiply(multiply(multiply(vals[21], 1024), 1024), 1024), 1024);
    for (ii=0; !ii || vals[ii] > vals[ii-1]; ii++) {
      airPrettySprintSize_t(prstmp, vals[ii]);
      if (strcmp(string[ii], prstmp)) {
        fprintf(stderr, "%s: airPrettySprintSize_t made |%s| not |%s|\n",
                me, prstmp, string[ii]);
        exit(1);
      }
      fprintf(stderr, "%u: %s\n", ii, prstmp);
    }
  }


  exit(0);
}



