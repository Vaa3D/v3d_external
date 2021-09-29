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


#include <teem/ell.h>

char *me;

void
usage(void) {
  /*                      0   1   2   3   (4) */
  fprintf(stderr, "usage: %s <A> <B> <C>\n", me);
  fprintf(stderr, "for cubic x^3 + Ax^2 + Bx + C == 0\n");
  exit(1);
}

int
main(int argc, char **argv) {
  char buf[512];
  double ans0, ans1, ans2, A, B, C;
  int ret;
  double r[3];

  me = argv[0];
  if (argc != 4) {
    usage();
  }

  sprintf(buf, "%s %s %s", argv[1], argv[2], argv[3]);
  if (3 != sscanf(buf, "%lf %lf %lf", &A, &B, &C)) {
    fprintf(stderr, "%s: couldn't parse 3 floats from command line\n", me);
    exit(1);
  }

  ell_debug = AIR_TRUE;
  ret = ell_cubic(r, A, B, C, AIR_TRUE);
  ans0 = C + r[0]*(B + r[0]*(A + r[0]));
  switch(ret) {
  case ell_cubic_root_single:
    printf("1 single root: %f -> %f\n", r[0], ans0);
    break;
  case ell_cubic_root_triple:
    printf("1 triple root: %f -> %f\n", r[0], ans0);
    break;
  case ell_cubic_root_single_double:
    ans1 = C + r[1]*(B + r[1]*(A + r[1]));
    printf("1 single root %f -> %f, 1 double root %f -> %f\n",
           r[0], ans0, r[1], ans1);
    break;
  case ell_cubic_root_three:
    ans1 = C + r[1]*(B + r[1]*(A + r[1]));
    ans2 = C + r[2]*(B + r[2]*(A + r[2]));
    printf("3 distinct roots:\n %f -> %f\n %f -> %f\n %f -> %f\n",
           r[0], ans0, r[1], ans1, r[2], ans2);
    break;
  default:
    printf("%s: something fatally wacky happened\n", me);
    exit(1);
  }
  exit(0);
}
