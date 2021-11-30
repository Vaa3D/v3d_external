/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2010, 2009, 2008 Thomas Schultz
  Copyright (C) 2010, 2009, 2008 Gordon Kindlmann

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


#include "tijk.h"

const int
tijkPresent = 42;

/* Some basic component-wise operations that don't depend on tensor type */

void
tijk_add_d (double *res, const double *A, const double *B,
            const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)=*(A++)+*(B++);
}

void
tijk_add_f (float *res, const float *A, const float *B,
            const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)=*(A++)+*(B++);
}

void
tijk_sub_d (double *res, const double *A, const double *B,
            const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)=*(A++)-*(B++);
}

void
tijk_sub_f (float *res, const float *A, const float *B,
            const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)=*(A++)-*(B++);
}

void
tijk_incr_d (double *res, const double *A, const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)+=*(A++);
}

void
tijk_incr_f (float *res, const float *A, const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)+=*(A++);
}

void
tijk_negate_d (double *res, const double *A, const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)=-*(A++);
}

void
tijk_negate_f (float *res, const float *A, const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)=-*(A++);
}

void
tijk_scale_d (double *res, const double s,
              const double *A, const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)=s*(*A++);
}

void
tijk_scale_f (float *res, const float s,
              const float *A, const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)=s*(*A++);
}

void
tijk_zero_d (double *res, const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)=0.0;
}

void
tijk_zero_f (float *res, const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)=0.0;
}

void
tijk_copy_d (double *res, const double *A, const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)=*(A++);
}

void
tijk_copy_f (float *res, const float *A, const tijk_type *type) {
  unsigned int i;
  for (i=0; i<type->num; i++)
    *(res++)=*(A++);
}
