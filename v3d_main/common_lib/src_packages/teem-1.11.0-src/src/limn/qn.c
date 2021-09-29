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


#include "limn.h"

/*
** "16simple": 16 bit representation based on
** <http://www.gamedev.net/reference/articles/article1252.asp>
**
** info: [z sign] [y sign] [x sign] [y component] [x component]
** bits:    1        1        1          7             6
**
** despite appearances, this is isotropic in X and Y
*/

void
_limnQN16simple_QNtoV_f(float *vec, unsigned int qn) {
  int xi, yi;
  float x, y, z, n;

  xi = qn & 0x3F;
  yi = (qn >> 6) & 0x7F;
  if (xi + yi >= 127) {
    xi = 127 - xi;
    yi = 127 - yi;
  }
  x = xi/126.0f;
  y = yi/126.0f;
  z = 1.0f - x - y;
  x = (qn & 0x2000) ? -x : x;
  y = (qn & 0x4000) ? -y : y;
  z = (qn & 0x8000) ? -z : z;
  n = AIR_CAST(float, 1.0/sqrt(x*x + y*y + z*z));
  vec[0] = x*n;
  vec[1] = y*n;
  vec[2] = z*n;
}

unsigned int
_limnQN16simple_VtoQN_f(const float *vec) {
  float x, y, z, L, w;
  unsigned int sgn = 0, xi, yi;

  x = vec[0];
  y = vec[1];
  z = vec[2];
  if (x < 0) {
    sgn |= 0x2000;
    x = -x;
  }
  if (y < 0) {
    sgn |= 0x4000;
    y = -y;
  }
  if (z < 0) {
    sgn |= 0x8000;
    z = -z;
  }
  L = x + y + z;
  if (!L) {
    return 0;
  }
  w = 126.0f/L;
  xi = AIR_CAST(unsigned int, x*w);
  yi = AIR_CAST(unsigned int, y*w);
  if (xi >= 64) {
    xi = 127 - xi;
    yi = 127 - yi;
  }
  return sgn | (yi << 6) | xi;
}

/* ----------------------------------------------------------------  */

void
_limnQN16border1_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int ui, vi;
  float u, v, x, y, z, n;

  ui = qn & 0xFF;
  vi = qn >> 8;
  u = AIR_CAST(float, AIR_AFFINE(0.5, ui, 254.5, -0.5, 0.5));
  v = AIR_CAST(float, AIR_AFFINE(0.5, vi, 254.5, -0.5, 0.5));
  x =  u + v;
  y =  u - v;
  z = 1 - AIR_ABS(x) - AIR_ABS(y);
  z *= AIR_CAST(int, ((ui&1) ^ (vi&1)) << 1) - 1;
  n = AIR_CAST(float, 1.0/sqrt(x*x + y*y + z*z));
  vec[0] = x*n;
  vec[1] = y*n;
  vec[2] = z*n;
}

unsigned int
_limnQN16border1_VtoQN_f(const float *vec) {
  float L, u, v, x, y, z;
  unsigned int ui, vi, zi;
  char me[]="limnQNVto16PB1";

  x = vec[0];
  y = vec[1];
  z = vec[2];
  L = AIR_ABS(x) + AIR_ABS(y) + AIR_ABS(z);
  if (!L) {
    return 0;
  }
  x /= L;
  y /= L;
  u = x + y;
  v = x - y;
  ui = airIndex(-1, u, 1, 254); ui++;
  vi = airIndex(-1, v, 1, 254); vi++;
  zi = (ui ^ vi) & 0x01;
  if (!zi && z > 1.0/128.0) {
    ui -= (((ui >> 7) & 0x01) << 1) - 1;
  }
  else if (zi && z < -1.0/128.0) {
    vi -= (((vi >> 7) & 0x01) << 1) - 1;
  }
  zi = (ui ^ vi) & 0x01;
  if (!zi && z > 1.0/127.0) {
    printf("%s: panic01\n", me);
  }
  else if (zi && z < -1.0/127.0) {
    printf("%s: panic02\n", me);
  }
  return (vi << 8) | ui;
}

/* ----------------------------------------------------------------  */

  /*
    x =  [  1.0   1.0 ] u
    y    [  1.0  -1.0 ] v
  */

  /*
    u =  [  0.5   0.5 ] x
    v    [  0.5  -0.5 ] y
  */

  /* xor of low bits == 0 --> z<0 ; xor == 1 --> z>0 */

/* May 11 2003 GK (visually) verified that except at equator seam,
   the error due to quantization is unbiased */

#define _EVEN_CHECK_QtoV(HNB, TT, vec, qn) \
  ui = (qn) & ((1<<HNB)-1);                \
  vi = ((qn) >> HNB) & ((1<<HNB)-1);              \
  u = AIR_AFFINE(0, ui, ((1<<HNB)-1), -0.5, 0.5); \
  v = AIR_AFFINE(0, vi, ((1<<HNB)-1), -0.5, 0.5); \
  x =  u + v;                                     \
  y =  u - v;                                     \
  z = 1 - AIR_ABS(x) - AIR_ABS(y);                \
  z *= AIR_CAST(int, ((ui & 1) ^ (vi & 1)) << 1) - 1; \
  n = 1.0/sqrt(x*x + y*y + z*z); \
  (vec)[0] = AIR_CAST(TT, x*n); \
  (vec)[1] = AIR_CAST(TT, y*n); \
  (vec)[2] = AIR_CAST(TT, z*n)

#define _EVEN_CHECK_VtoQ(HNB, vec) \
  x = (vec)[0]; \
  y = (vec)[1]; \
  z = (vec)[2]; \
  L = AIR_ABS(x) + AIR_ABS(y) + AIR_ABS(z); \
  if (!L) { \
    return 0; \
  } \
  x /= L; \
  y /= L; \
  if (z > 0) { \
    xi = airIndex(-1.0, x, 1.0, ((1<<HNB)-1)); \
    yi = airIndex(-1.0 - 1.0/((1<<HNB)-1), y, 1.0 + 1.0/((1<<HNB)-1), \
                  (1<<HNB)); \
    ui = xi + yi - ((1<<(HNB-1))-1); \
    vi = xi - yi + (1<<(HNB-1)); \
  } \
  else { \
    xi = airIndex(-1.0 - 1.0/((1<<HNB)-1), x, 1.0 + 1.0/((1<<HNB)-1), \
                  (1<<HNB)); \
    yi = airIndex(-1, y, 1, ((1<<HNB)-1)); \
    ui = xi + yi - ((1<<(HNB-1))-1); \
    vi = xi - yi + ((1<<(HNB-1))-1); \
  } \
  return (vi << HNB) | ui

#define _EVEN_OCTA_QtoV(HNB, TT, vec, qn) \
  xi = (qn) & ((1<<HNB)-1); \
  yi = ((qn) >> HNB) & ((1<<HNB)-1); \
  x = AIR_AFFINE(-0.5, xi, ((1<<HNB)-1)+0.5, -1, 1); \
  y = AIR_AFFINE(-0.5, yi, ((1<<HNB)-1)+0.5, -1, 1); \
  z = 1 - AIR_ABS(x) - AIR_ABS(y); \
  if (z < 0) { \
    if (x > 0) { \
      x += z; \
    } else { \
      x -= z; \
    } \
    if (y > 0) { \
      y += z; \
    } else { \
      y -= z; \
    } \
  } \
  n = 1.0/sqrt(x*x + y*y + z*z); \
  (vec)[0] = AIR_CAST(TT, x*n); \
  (vec)[1] = AIR_CAST(TT, y*n); \
  (vec)[2] = AIR_CAST(TT, z*n)

#define _EVEN_OCTA_VtoQ(HNB, vec) \
  x = (vec)[0]; \
  y = (vec)[1]; \
  z = (vec)[2]; \
  L = AIR_ABS(x) + AIR_ABS(y) + AIR_ABS(z); \
  if (!L) { \
    return 0; \
  } \
  x /= L; \
  y /= L; \
  z /= L; \
  if (z < 0) { \
    if (x > 0) { \
      x -= z; \
    } else { \
      x += z; \
    } \
    if (y > 0) { \
      y -= z; \
    } else { \
      y += z; \
    } \
  } \
  xi = airIndex(-1, x, 1, (1<<HNB)); \
  yi = airIndex(-1, y, 1, (1<<HNB)); \
  return (yi << HNB) | xi

/* _16_CHECK_QtoV and _16_CHECK_VtoQ are not actually used */
#define _16_CHECK_QtoV(vec, qn) \
  ui = (qn) & 0xFF; \
  vi = ((qn) >> 8) & 0xFF; \
  u = AIR_AFFINE(0, ui, 255, -0.5, 0.5); \
  v = AIR_AFFINE(0, vi, 255, -0.5, 0.5); \
  x =  u + v; \
  y =  u - v; \
  z = 1 - AIR_ABS(x) - AIR_ABS(y); \
  /* would this be better served by a branch? */ \
  z *= (((ui ^ vi) & 0x01) << 1) - 1; \
  n = 1.0/sqrt(x*x + y*y + z*z); \
  (vec)[0] = x*n; \
  (vec)[1] = y*n; \
  (vec)[2] = z*n

#define _16_CHECK_VtoQ(vec) \
  x = (vec)[0]; \
  y = (vec)[1]; \
  z = (vec)[2]; \
  L = AIR_ABS(x) + AIR_ABS(y) + AIR_ABS(z); \
  if (!L) { \
    return 0; \
  } \
  x /= L; \
  y /= L; \
  if (z > 0) { \
    xi = airIndex(-1.0, x, 1.0, 255); \
    yi = airIndex(-1.0 - 1.0/255, y, 1.0 + 1.0/255, 256); \
    ui = xi + yi - 127; \
    vi = xi - yi + 128; \
  } \
  else { \
    xi = airIndex(-1.0 - 1.0/255, x, 1.0 + 1.0/255, 256); \
    yi = airIndex(-1, y, 1, 255); \
    ui = xi + yi - 127; \
    vi = xi - yi + 127; \
  }

void
_limnQN16checker_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int ui, vi;
  double u, v, x, y, z, n;

  _EVEN_CHECK_QtoV(8, float, vec, qn);
}

void
_limnQN16checker_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int ui, vi;
  double u, v, x, y, z, n;

  _EVEN_CHECK_QtoV(8, double, vec, qn);
}

unsigned int
_limnQN16checker_VtoQN_f(const float *vec) {
  double L, x, y, z;
  unsigned int xi, yi, ui, vi;

  _EVEN_CHECK_VtoQ(8, vec);
}

unsigned int
_limnQN16checker_VtoQN_d(const double *vec) {
  double L, x, y, z;
  unsigned int xi, yi, ui, vi;

  _EVEN_CHECK_VtoQ(8, vec);
}

/* ----------------------------------------------------------------  */

void
_limnQN16octa_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int xi, yi;
  double x, y, z, n;

  _EVEN_OCTA_QtoV(8, float, vec, qn);
}

void
_limnQN16octa_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int xi, yi;
  double x, y, z, n;

  _EVEN_OCTA_QtoV(8, double, vec, qn);
}

unsigned int
_limnQN16octa_VtoQN_f(const float *vec) {
  double L, x, y, z;
  unsigned int xi, yi;

  _EVEN_OCTA_VtoQ(8, vec);
}

unsigned int
_limnQN16octa_VtoQN_d(const double *vec) {
  double L, x, y, z;
  unsigned int xi, yi;

  _EVEN_OCTA_VtoQ(8, vec);
}

/* ----------------------------------------------------------------  */

/* 15 bit --> HNB == 7 */

#define _ODD_OCTA_QtoV(HNB, TT, vec, qn) \
  ui = qn & ((1<<HNB)-1); \
  vi = (qn >> HNB) & ((1<<HNB)-1); \
  zi = (qn >> (2*HNB)) & 0x01; \
  u = AIR_CAST(TT, AIR_AFFINE(-0.5, ui, ((1<<HNB)-1)+0.5, -0.5, 0.5)); \
  v = AIR_CAST(TT, AIR_AFFINE(-0.5, vi, ((1<<HNB)-1)+0.5, -0.5, 0.5)); \
  x =  u + v; \
  y =  u - v; \
  z = 1 - AIR_ABS(x) - AIR_ABS(y); \
  z *= AIR_CAST(int, zi << 1) - 1;    \
  n = AIR_CAST(TT, 1.0/sqrt(x*x + y*y + z*z)); \
  vec[0] = AIR_CAST(TT, x*n); \
  vec[1] = AIR_CAST(TT, y*n); \
  vec[2] = AIR_CAST(TT, z*n)

#define _ODD_OCTA_VtoQ(HNB, vec) \
  x = vec[0]; \
  y = vec[1]; \
  z = vec[2]; \
  L = AIR_ABS(x) + AIR_ABS(y) + AIR_ABS(z); \
  if (!L) { \
    return 0; \
  } \
  x /= L; \
  y /= L; \
  u = x + y; \
  v = x - y; \
  ui = airIndex(-1, u, 1, (1<<HNB)); \
  vi = airIndex(-1, v, 1, (1<<HNB)); \
  zi = (z > 0); \
  return (zi << (2*HNB)) | (vi << HNB) | ui


void
_limnQN15octa_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int ui, vi, zi;
  float u, v, x, y, z, n;

  _ODD_OCTA_QtoV(7, float, vec, qn);
}

void
_limnQN15octa_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int ui, vi, zi;
  double u, v, x, y, z, n;

  _ODD_OCTA_QtoV(7, double, vec, qn);
}

unsigned int
_limnQN15octa_VtoQN_f(const float *vec) {
  float L, u, v, x, y, z;
  unsigned int ui, vi, zi;

  _ODD_OCTA_VtoQ(7, vec);
}

unsigned int
_limnQN15octa_VtoQN_d(const double *vec) {
  double L, u, v, x, y, z;
  unsigned int ui, vi, zi;

  _ODD_OCTA_VtoQ(7, vec);
}

/* ----------------------------------------------------------- */

void
_limnQN14checker_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int ui, vi;
  double u, v, x, y, z, n;

  _EVEN_CHECK_QtoV(7, float, vec, qn);
}

void
_limnQN14checker_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int ui, vi;
  double u, v, x, y, z, n;

  _EVEN_CHECK_QtoV(7, double, vec, qn);
}

unsigned int
_limnQN14checker_VtoQN_f(const float *vec) {
  double L, x, y, z;
  unsigned int xi, yi, ui, vi;

  _EVEN_CHECK_VtoQ(7, vec);
}

unsigned int
_limnQN14checker_VtoQN_d(const double *vec) {
  double L, x, y, z;
  unsigned int xi, yi, ui, vi;

  _EVEN_CHECK_VtoQ(7, vec);
}

/* ----------------------------------------------------------------  */

void
_limnQN14octa_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int xi, yi;
  double x, y, z, n;

  _EVEN_OCTA_QtoV(7, float, vec, qn);
}

void
_limnQN14octa_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int xi, yi;
  double x, y, z, n;

  _EVEN_OCTA_QtoV(7, double, vec, qn);
}

unsigned int
_limnQN14octa_VtoQN_f(const float *vec) {
  double L, x, y, z;
  unsigned int xi, yi;

  _EVEN_OCTA_VtoQ(7, vec);
}

unsigned int
_limnQN14octa_VtoQN_d(const double *vec) {
  double L, x, y, z;
  unsigned int xi, yi;

  _EVEN_OCTA_VtoQ(7, vec);
}

/* ----------------------------------------------------------- */

void
_limnQN13octa_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int ui, vi, zi;
  float u, v, x, y, z, n;

  _ODD_OCTA_QtoV(6, float, vec, qn);
}

void
_limnQN13octa_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int ui, vi, zi;
  double u, v, x, y, z, n;

  _ODD_OCTA_QtoV(6, double, vec, qn);
}

unsigned int
_limnQN13octa_VtoQN_f(const float *vec) {
  float L, u, v, x, y, z;
  unsigned int ui, vi, zi;

  _ODD_OCTA_VtoQ(6, vec);
}

unsigned int
_limnQN13octa_VtoQN_d(const double *vec) {
  double L, u, v, x, y, z;
  unsigned int ui, vi, zi;

  _ODD_OCTA_VtoQ(6, vec);
}

/* ----------------------------------------------------------- */

void
_limnQN12checker_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int ui, vi;
  double u, v, x, y, z, n;

  _EVEN_CHECK_QtoV(6, float, vec, qn);
}

void
_limnQN12checker_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int ui, vi;
  double u, v, x, y, z, n;

  _EVEN_CHECK_QtoV(6, double, vec, qn);
}

unsigned int
_limnQN12checker_VtoQN_f(const float *vec) {
  double L, x, y, z;
  unsigned int xi, yi, ui, vi;

  _EVEN_CHECK_VtoQ(6, vec);
}

unsigned int
_limnQN12checker_VtoQN_d(const double *vec) {
  double L, x, y, z;
  unsigned int xi, yi, ui, vi;

  _EVEN_CHECK_VtoQ(6, vec);
}

/* ----------------------------------------------------------------  */

void
_limnQN12octa_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int xi, yi;
  double x, y, z, n;

  _EVEN_OCTA_QtoV(6, float, vec, qn);
}

void
_limnQN12octa_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int xi, yi;
  double x, y, z, n;

  _EVEN_OCTA_QtoV(6, double, vec, qn);
}

unsigned int
_limnQN12octa_VtoQN_f(const float *vec) {
  double L, x, y, z;
  unsigned int xi, yi;

  _EVEN_OCTA_VtoQ(6, vec);
}

unsigned int
_limnQN12octa_VtoQN_d(const double *vec) {
  double L, x, y, z;
  unsigned int xi, yi;

  _EVEN_OCTA_VtoQ(6, vec);
}

/* ----------------------------------------------------------- */

void
_limnQN11octa_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int ui, vi, zi;
  float u, v, x, y, z, n;

  _ODD_OCTA_QtoV(5, float, vec, qn);
}

void
_limnQN11octa_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int ui, vi, zi;
  double u, v, x, y, z, n;

  _ODD_OCTA_QtoV(5, double, vec, qn);
}

unsigned int
_limnQN11octa_VtoQN_f(const float *vec) {
  float L, u, v, x, y, z;
  unsigned int ui, vi, zi;

  _ODD_OCTA_VtoQ(5, vec);
}

unsigned int
_limnQN11octa_VtoQN_d(const double *vec) {
  double L, u, v, x, y, z;
  unsigned int ui, vi, zi;

  _ODD_OCTA_VtoQ(5, vec);
}

/* ----------------------------------------------------------- */

void
_limnQN10checker_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int ui, vi;
  double u, v, x, y, z, n;

  _EVEN_CHECK_QtoV(5, float, vec, qn);
}

void
_limnQN10checker_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int ui, vi;
  double u, v, x, y, z, n;

  _EVEN_CHECK_QtoV(5, double, vec, qn);
}

unsigned int
_limnQN10checker_VtoQN_f(const float *vec) {
  double L, x, y, z;
  unsigned int xi, yi, ui, vi;

  _EVEN_CHECK_VtoQ(5, vec);
}

unsigned int
_limnQN10checker_VtoQN_d(const double *vec) {
  double L, x, y, z;
  unsigned int xi, yi, ui, vi;

  _EVEN_CHECK_VtoQ(5, vec);
}

/* ----------------------------------------------------------------  */

void
_limnQN10octa_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int xi, yi;
  double x, y, z, n;

  _EVEN_OCTA_QtoV(5, float, vec, qn);
}

void
_limnQN10octa_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int xi, yi;
  double x, y, z, n;

  _EVEN_OCTA_QtoV(5, double, vec, qn);
}

unsigned int
_limnQN10octa_VtoQN_f(const float *vec) {
  double L, x, y, z;
  unsigned int xi, yi;

  _EVEN_OCTA_VtoQ(5, vec);
}

unsigned int
_limnQN10octa_VtoQN_d(const double *vec) {
  double L, x, y, z;
  unsigned int xi, yi;

  _EVEN_OCTA_VtoQ(5, vec);
}

/* ----------------------------------------------------------- */
void
_limnQN9octa_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int ui, vi, zi;
  float u, v, x, y, z, n;

  _ODD_OCTA_QtoV(4, float, vec, qn);
}

void
_limnQN9octa_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int ui, vi, zi;
  double u, v, x, y, z, n;

  _ODD_OCTA_QtoV(4, double, vec, qn);
}

unsigned int
_limnQN9octa_VtoQN_f(const float *vec) {
  float L, u, v, x, y, z;
  unsigned int ui, vi, zi;

  _ODD_OCTA_VtoQ(4, vec);
}

unsigned int
_limnQN9octa_VtoQN_d(const double *vec) {
  double L, u, v, x, y, z;
  unsigned int ui, vi, zi;

  _ODD_OCTA_VtoQ(4, vec);
}

/* ----------------------------------------------------------- */

void
_limnQN8checker_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int ui, vi;
  double u, v, x, y, z, n;

  _EVEN_CHECK_QtoV(4, float, vec, qn);
}

void
_limnQN8checker_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int ui, vi;
  double u, v, x, y, z, n;

  _EVEN_CHECK_QtoV(4, double, vec, qn);
}

unsigned int
_limnQN8checker_VtoQN_f(const float *vec) {
  double L, x, y, z;
  unsigned int xi, yi, ui, vi;

  _EVEN_CHECK_VtoQ(4, vec);
}

unsigned int
_limnQN8checker_VtoQN_d(const double *vec) {
  double L, x, y, z;
  unsigned int xi, yi, ui, vi;

  _EVEN_CHECK_VtoQ(4, vec);
}

/* ----------------------------------------------------------------  */

void
_limnQN8octa_QNtoV_f(float *vec, unsigned int qn) {
  unsigned int xi, yi;
  double x, y, z, n;

  _EVEN_OCTA_QtoV(4, float, vec, qn);
}

void
_limnQN8octa_QNtoV_d(double *vec, unsigned int qn) {
  unsigned int xi, yi;
  double x, y, z, n;

  _EVEN_OCTA_QtoV(4, double, vec, qn);
}

unsigned int
_limnQN8octa_VtoQN_f(const float *vec) {
  double L, x, y, z;
  unsigned int xi, yi;

  _EVEN_OCTA_VtoQ(4, vec);
}

unsigned int
_limnQN8octa_VtoQN_d(const double *vec) {
  double L, x, y, z;
  unsigned int xi, yi;

  _EVEN_OCTA_VtoQ(4, vec);
}

/* ----------------------------------------------------------- */

void (*
limnQNtoV_f[LIMN_QN_MAX+1])(float *, unsigned int) = {
  NULL,
  _limnQN16simple_QNtoV_f,
  _limnQN16border1_QNtoV_f,
  _limnQN16checker_QNtoV_f,
  _limnQN16octa_QNtoV_f,
  _limnQN15octa_QNtoV_f,
  _limnQN14checker_QNtoV_f,
  _limnQN14octa_QNtoV_f,
  _limnQN13octa_QNtoV_f,
  _limnQN12checker_QNtoV_f,
  _limnQN12octa_QNtoV_f,
  _limnQN11octa_QNtoV_f,
  _limnQN10checker_QNtoV_f,
  _limnQN10octa_QNtoV_f,
  _limnQN9octa_QNtoV_f,
  _limnQN8checker_QNtoV_f,
  _limnQN8octa_QNtoV_f
};

void (*
limnQNtoV_d[LIMN_QN_MAX+1])(double *, unsigned int) = {
  NULL,
  NULL,
  NULL,
  _limnQN16checker_QNtoV_d,
  _limnQN16octa_QNtoV_d,
  _limnQN15octa_QNtoV_d,
  _limnQN14checker_QNtoV_d,
  _limnQN14octa_QNtoV_d,
  _limnQN13octa_QNtoV_d,
  _limnQN12checker_QNtoV_d,
  _limnQN12octa_QNtoV_d,
  _limnQN11octa_QNtoV_d,
  _limnQN10checker_QNtoV_d,
  _limnQN10octa_QNtoV_d,
  _limnQN9octa_QNtoV_d,
  _limnQN8checker_QNtoV_d,
  _limnQN8octa_QNtoV_d
};

unsigned int (*
limnVtoQN_f[LIMN_QN_MAX+1])(const float *vec) = {
  NULL,
  _limnQN16simple_VtoQN_f,
  _limnQN16border1_VtoQN_f,
  _limnQN16checker_VtoQN_f,
  _limnQN16octa_VtoQN_f,
  _limnQN15octa_VtoQN_f,
  _limnQN14checker_VtoQN_f,
  _limnQN14octa_VtoQN_f,
  _limnQN13octa_VtoQN_f,
  _limnQN12checker_VtoQN_f,
  _limnQN12octa_VtoQN_f,
  _limnQN11octa_VtoQN_f,
  _limnQN10checker_VtoQN_f,
  _limnQN10octa_VtoQN_f,
  _limnQN9octa_VtoQN_f,
  _limnQN8checker_VtoQN_f,
  _limnQN8octa_VtoQN_f
};

unsigned int (*
limnVtoQN_d[LIMN_QN_MAX+1])(const double *vec) = {
  NULL,
  NULL,
  NULL,
  _limnQN16checker_VtoQN_d,
  _limnQN16octa_VtoQN_d,
  _limnQN15octa_VtoQN_d,
  _limnQN14checker_VtoQN_d,
  _limnQN14octa_VtoQN_d,
  _limnQN13octa_VtoQN_d,
  _limnQN12checker_VtoQN_d,
  _limnQN12octa_VtoQN_d,
  _limnQN11octa_VtoQN_d,
  _limnQN10checker_VtoQN_d,
  _limnQN10octa_VtoQN_d,
  _limnQN9octa_VtoQN_d,
  _limnQN8checker_VtoQN_d,
  _limnQN8octa_VtoQN_d
};

unsigned int
limnQNBins[LIMN_QN_MAX+1] = {
  0,
  (1 << 16),
  (1 << 16),
  (1 << 16),
  (1 << 16),
  (1 << 15),
  (1 << 14),
  (1 << 14),
  (1 << 13),
  (1 << 12),
  (1 << 12),
  (1 << 11),
  (1 << 10),
  (1 << 10),
  (1 << 9),
  (1 << 8),
  (1 << 8)
};

/*
** can use via test/tqn:
  foreach Q ( 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 )
    echo $Q
    test/tqn -s 400 -q $Q \
     | unu tile -a 2 0 1 -s 2 3 \
     | unu tile -a 2 0 1 -s 2 1 | unu quantize -b 8 -o q${Q}.png
  end
*/
int
limnQNDemo(Nrrd *nqn, unsigned int reso, int qni) {
  static const char me[]="limnQNDemo";
  unsigned int ui, vi, oi;
  double *qdata, ll, uu, vv, ww, vecd[3], unqd[3];
  float vecf[3], unqf[3];

  if (!nqn) {
    biffAddf(LIMN, "%s: got NULL pointer", me);
    return 1;
  }
  if (nrrdMaybeAlloc_va(nqn, nrrdTypeDouble, 4,
                        AIR_CAST(size_t, reso),
                        AIR_CAST(size_t, reso),
                        AIR_CAST(size_t, 6),
                        AIR_CAST(size_t, 2))){
    biffMovef(LIMN, NRRD, "%s: couldn't alloc output", me);
    return 1;
  }
  if (!(limnQNUnknown < qni && qni < limnQNLast)) {
    biffAddf(LIMN, "%s: qni %d not in valid range [%d,%d]", me,
             qni, limnQNUnknown+1, limnQNLast-1);
    return 1;
  }
  qdata = AIR_CAST(double *, nqn->data);
  for (oi=0; oi<6; oi++) {
    for (vi=0; vi<reso; vi++) {
      vv = AIR_AFFINE(0, vi, reso-1, 1, -1);
      for (ui=0; ui<reso; ui++) {
        uu = AIR_AFFINE(0, ui, reso-1, -1, 1);
        ll = uu*uu + vv*vv;
        if (ll <= 1) {
          ww = sqrt(1 - ll);
          if (oi % 2) {
            ww *= -1;
          }
        } else {
          continue;
        }
        switch (oi) {
        case 0:
        case 1:
          ELL_3V_SET(vecd, uu, vv, ww);
          ELL_3V_SET_TT(vecf, float, uu, vv, ww);
          break;
        case 2:
        case 3:
          ELL_3V_SET(vecd, uu, ww, vv);
          ELL_3V_SET_TT(vecf, float, uu, ww, vv);
          break;
        case 4:
        case 5:
          ELL_3V_SET(vecd, ww, uu, vv);
          ELL_3V_SET_TT(vecf, float, ww, uu, vv);
          break;
        }
        if (limnVtoQN_d[qni] && limnQNtoV_d[qni]) {
          (limnQNtoV_d[qni])(unqd, (limnVtoQN_d[qni])(vecd));
          qdata[ui + reso*(vi + reso*(oi + 6))] = ell_3v_angle_d(unqd, vecd);
        }
        if (limnVtoQN_f[qni] && limnQNtoV_f[qni]) {
          (limnQNtoV_f[qni])(unqf, (limnVtoQN_f[qni])(vecf));
          qdata[ui + reso*(vi + reso*(oi + 0))] = ell_3v_angle_f(unqf, vecf);
        }
      }
    }
  }
  return 0;
}
