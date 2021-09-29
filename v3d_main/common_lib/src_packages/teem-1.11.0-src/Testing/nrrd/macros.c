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

#include <teem/nrrd.h>
#include <teem/ell.h> /* nrrd doesn't depend on ell but the tests don't need
                         to be constrained by the same dependency order;
                         included here for the macros */

/*
** Tests:
** NRRD_COORD_UPDATE, NRRD_COORD_INCR, NRRD_INDEX_GEN, NRRD_COORD_GEN
*/

int
main(int argc, char *argv[]) {
  size_t II, *coord0, *ctmp0, *size0,
    coord1[1], ctmp1[1], size1[1],
    coord2[2], ctmp2[2], size2[2],
    coord3[3], ctmp3[3], size3[3],
    coord4[4], ctmp4[4], size4[4];
  char sbuff[NRRD_DIM_MAX*AIR_STRLEN_SMALL],
    scomp[NRRD_DIM_MAX*AIR_STRLEN_SMALL],
    sigab[NRRD_DIM_MAX*AIR_STRLEN_SMALL], /* index gen and back */
    swhat[NRRD_DIM_MAX*AIR_STRLEN_SMALL];
  unsigned int ii, jj, kk, ll;

  /* This macro makes sure the given length-N coord##N vector is what it
     should be ("want"), via string comparison after the output of
     airSprintVecSize_t.  Also uses NRRD_INDEX_GEN and NRRD_COORD_GEN
     to go from coord##N to a linear index II and back to ctmp##N,
     and makes sure that that too matches "want" */
#define CHECK(N, want)                                                  \
  airSprintVecSize_t(swhat, size##N, N);                                \
  airSprintVecSize_t(sbuff, coord##N, N);                               \
  if (strcmp(sbuff, want)) {                                            \
    fprintf(stderr, "for %s: got %s but wanted %s\n",                   \
            swhat, sbuff, want);                                        \
    return 1;                                                           \
  }                                                                     \
  NRRD_INDEX_GEN(II, coord##N, size##N, N);                             \
  NRRD_COORD_GEN(ctmp##N, size##N, N, II);                              \
  airSprintVecSize_t(sigab, ctmp##N, N);                                \
  if (strcmp(sbuff, want)) {                                            \
    fprintf(stderr, "for %s: NRRD_{INDEX,COORD}_GEN gave %s but want %s\n", \
            swhat, sbuff, want);                                        \
    return 1;                                                           \
  }


  /* actually airTime will always be positive; this is just to thwart the
     compiler thinking coord0 and size0 are necessarily NULL */
  if (airTime() > -100) {
    coord0 = NULL;
    ctmp0 = NULL;
    size0 = NULL;
  } else {
    coord0 = coord1;
    ctmp0 = ctmp1;
    size0 = size1;
  }

  /* http://teem.svn.sourceforge.net/viewvc/teem/teem/trunk/src/nrrd/nrrdMacros.h?r1=3931&r2=4587
     BUG: previous version causes array subscript is below array bounds.  */

  /* 00000000000000000000000000000000000000000000000 */
  /* using coord0 == size0 == NULL here is to make sure that its not even
     accessed */
  NRRD_COORD_UPDATE(coord0, size0, 0);
  CHECK(0, "[]");

  /* 11111111111111111111111111111111111111111111111 */

#define RUN1                                                         \
  coord1[0] = 0;                                                     \
  for (ii=0; ii<size1[0]; ii++) {                                    \
    if (ii) {                                                        \
      /* don't increment on the very first time, otherwise do */     \
      NRRD_COORD_INCR(coord1, size1, 1, 0);                          \
    }                                                                \
    airSprintVecSize_t(sbuff, coord1, 1);                            \
    sprintf(scomp, "[%u]", ii);                                      \
    CHECK(1, scomp);                                                 \
  }                                                                  \

  size1[0] = 1;
  RUN1;

  size1[0] = 10;
  RUN1;

  /* 22222222222222222222222222222222222222222222222 */

#define RUN2                                                         \
  coord2[0] = coord2[1] = 0;                                         \
  for (jj=0; jj<size2[1]; jj++) {                                    \
    for (ii=0; ii<size2[0]; ii++) {                                  \
      if (ii || jj) {                                                \
        /* don't increment on the very first time, otherwise do */   \
        NRRD_COORD_INCR(coord2, size2, 2, 0);                        \
      }                                                              \
      airSprintVecSize_t(sbuff, coord2, 2);                          \
      sprintf(scomp, "[%u,%u]", ii, jj);                             \
      CHECK(2, scomp);                                               \
    }                                                                \
  }

  ELL_2V_SET(size2, 1, 1);
  RUN2;

  ELL_2V_SET(size2, 1, 20);
  RUN2;

  ELL_2V_SET(size2, 20, 1);
  RUN2;

  ELL_2V_SET(size2, 20, 20);
  RUN2;

  /* 33333333333333333333333333333333333333333333333 */

#define RUN3                                                            \
  coord3[0] = coord3[1] = coord3[2] = 0;                                \
  for (kk=0; kk<size3[2]; kk++) {                                       \
    for (jj=0; jj<size3[1]; jj++) {                                     \
      for (ii=0; ii<size3[0]; ii++) {                                   \
        if (ii || jj || kk) {                                           \
          /* don't increment on the very first time, otherwise do */    \
          NRRD_COORD_INCR(coord3, size3, 3, 0);                         \
        }                                                               \
        airSprintVecSize_t(sbuff, coord3, 3);                           \
        sprintf(scomp, "[%u,%u,%u]", ii, jj, kk);                       \
        CHECK(3, scomp);                                                \
      }                                                                 \
    }                                                                   \
  }

  ELL_3V_SET(size3, 1, 1, 1);
  RUN3;

  ELL_3V_SET(size3, 20, 1, 1);
  RUN3;

  ELL_3V_SET(size3, 1, 20, 1);
  RUN3;

  ELL_3V_SET(size3, 1, 1, 20);
  RUN3;

  ELL_3V_SET(size3, 20, 20, 20);
  RUN3;

  /* 44444444444444444444444444444444444444444444444 */

#define RUN4                                                            \
  coord4[0] = coord4[1] = coord4[2] = coord4[3] = 0;                    \
  for (ll=0; ll<size4[3]; ll++) {                                       \
    for (kk=0; kk<size4[2]; kk++) {                                     \
      for (jj=0; jj<size4[1]; jj++) {                                   \
        for (ii=0; ii<size4[0]; ii++) {                                 \
          if (ii || jj || kk || ll) {                                   \
            /* don't increment on the very first time, otherwise do */  \
            NRRD_COORD_INCR(coord4, size4, 4, 0);                       \
          }                                                             \
          airSprintVecSize_t(sbuff, coord4, 4);                         \
          sprintf(scomp, "[%u,%u,%u,%u]", ii, jj, kk, ll);              \
          CHECK(4, scomp);                                              \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }

  ELL_4V_SET(size4, 1, 1, 1, 1);
  RUN4;

  ELL_4V_SET(size4, 20, 1, 1, 1);
  RUN4;

  ELL_4V_SET(size4, 1, 20, 1, 1);
  RUN4;

  ELL_4V_SET(size4, 1, 1, 20, 1);
  RUN4;

  ELL_4V_SET(size4, 1, 1, 1, 20);
  RUN4;

  ELL_4V_SET(size4, 20, 20, 20, 20);
  RUN4;

  return 0;
}
