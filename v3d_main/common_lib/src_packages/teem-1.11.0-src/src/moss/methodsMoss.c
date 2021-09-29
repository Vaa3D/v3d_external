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

#include "moss.h"
#include "privateMoss.h"

const int
mossPresent = 42;

/*
******** mossSamplerNew()
**
*/
mossSampler *
mossSamplerNew (void) {
  mossSampler *smplr;
  int i;

  smplr = (mossSampler *)calloc(1, sizeof(mossSampler));
  if (smplr) {
    smplr->image = NULL;
    smplr->kernel = NULL;
    for (i=0; i<NRRD_KERNEL_PARMS_NUM; i++)
      smplr->kparm[i] = AIR_NAN;
    smplr->ivc = NULL;
    smplr->xFslw = smplr->yFslw = NULL;
    smplr->xIdx = smplr->yIdx = NULL;
    smplr->bg = NULL;
    smplr->fdiam = smplr->ncol = 0;
    smplr->boundary = mossDefBoundary;
    for (i=0; i<MOSS_FLAG_NUM; i++)
      smplr->flag[i] = AIR_FALSE;
  }
  return smplr;
}

int
mossSamplerFill (mossSampler *smplr, int fdiam, int ncol) {
  static const char me[]="_mossSamplerFill";

  if (!(smplr)) {
    biffAddf(MOSS, "%s: got NULL pointer", me);
    return 1;
  }
  smplr->ivc = (float*)calloc(fdiam*fdiam*ncol, sizeof(float));
  smplr->xFslw = (double*)calloc(fdiam, sizeof(double));
  smplr->yFslw = (double*)calloc(fdiam, sizeof(double));
  smplr->xIdx = (int*)calloc(fdiam, sizeof(int));
  smplr->yIdx = (int*)calloc(fdiam, sizeof(int));
  if (!( smplr->ivc && smplr->xFslw && smplr->yFslw
         && smplr->xIdx && smplr->yIdx )) {
    biffAddf(MOSS, "%s: couldn't allocate buffers", me);
    return 1;
  }
  smplr->fdiam = fdiam;
  smplr->ncol = ncol;
  return 0;
}

void
mossSamplerEmpty (mossSampler *smplr) {

  if (smplr) {
    smplr->ivc = (float *)airFree(smplr->ivc);
    smplr->xFslw = (double *)airFree(smplr->xFslw);
    smplr->yFslw = (double *)airFree(smplr->yFslw);
    smplr->xIdx = (int *)airFree(smplr->xIdx);
    smplr->yIdx = (int *)airFree(smplr->yIdx);
    smplr->fdiam = 0;
    smplr->ncol = 0;
  }
  return;
}

mossSampler *
mossSamplerNix (mossSampler *smplr) {

  if (smplr) {
    mossSamplerEmpty(smplr);
    smplr->bg = (float *)airFree(smplr->bg);
    free(smplr);
  }
  return NULL;
}

int
mossImageCheck (Nrrd *image) {
  static const char me[]="mossImageCheck";

  if (nrrdCheck(image)) {
    biffMovef(MOSS, NRRD, "%s: given nrrd invalid", me);
    return 1;
  }
  if (!( (2 == image->dim || 3 == image->dim)
         && nrrdTypeBlock != image->type )) {
    biffAddf(MOSS, "%s: image has invalid dimension (%d) or type (%s)", me,
             image->dim, airEnumStr(nrrdType, image->type));
    return 1;
  }

  return 0;
}

int
mossImageAlloc (Nrrd *image, int type, int sx, int sy, int ncol) {
  static const char me[]="mossImageAlloc";
  int ret;

  if (!(image && AIR_IN_OP(nrrdTypeUnknown, type, nrrdTypeBlock)
        && sx > 0 && sy > 0 && ncol > 0)) {
    biffAddf(MOSS, "%s: got NULL pointer or bad args", me);
    return 1;
  }
  if (1 == ncol) {
    ret = nrrdMaybeAlloc_va(image, type, 2,
                            AIR_CAST(size_t, sx),
                            AIR_CAST(size_t, sy));
  } else {
    ret = nrrdMaybeAlloc_va(image, type, 3,
                            AIR_CAST(size_t, ncol),
                            AIR_CAST(size_t, sx),
                            AIR_CAST(size_t, sy));
  }
  if (ret) {
    biffMovef(MOSS, NRRD, "%s: couldn't allocate image", me);
    return 1;
  }


  return 0;
}

int
_mossCenter(int center) {

  center =  (nrrdCenterUnknown == center
             ? mossDefCenter
             : center);
  center = AIR_CLAMP(nrrdCenterUnknown+1, center, nrrdCenterLast-1);
  return center;
}
