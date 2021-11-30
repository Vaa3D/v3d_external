/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2011 Thomas Schultz

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

/* Reading and setting axis labels to encode Tijk types */

#include "tijk.h"

/* tijk_set_axis_tensor
 *
 * Marks a given nrrd axis as containing tensors of a given type
 * Based on axis size, determines if values are masked.
 * Returns 0 on success
 *         1 if given NULL pointer
 *         2 if nrrd has wrong type (neither float nor double)
 *         3 if axis doesn't exist
 *         4 if axis has wrong size
 */
int tijk_set_axis_tensor(Nrrd *nrrd, unsigned int axis,
                         const tijk_type *type) {
  NrrdAxisInfo *axinfo = NULL;
  unsigned int masked = 0, lablen;
  if (nrrd==NULL || type==NULL) return 1;
  if (nrrd->type!=nrrdTypeFloat && nrrd->type!=nrrdTypeDouble) return 2;
  if (axis>=nrrd->dim) return 3;
  axinfo = nrrd->axis+axis;
  if (axinfo->size==type->num+1)
    masked = 1;
  else if (axinfo->size!=type->num)
    return 4;
  axinfo->label = (char*) airFree(axinfo->label);
  lablen = strlen("tijk__") + strlen(type->name) +
    (masked?strlen("mask_"):0) + 1;
  axinfo->label = AIR_CALLOC(lablen, char);
  sprintf(axinfo->label, "tijk_%s%s", masked?"mask_":"", type->name);
  return 0;
}

/* tijk_set_axis_esh
 *
 * Marks a given nrrd axis as containing even-order spherical harmonics of
 * a given order. Based on axis size, determines if values are masked.
 * Returns 0 on success
 *         1 if given NULL pointer
 *         2 if nrrd has wrong type (neither float nor double)
 *         3 if axis doesn't exist
 *         4 if order is unsupported
 *         5 if axis has wrong size
 */
int tijk_set_axis_esh(Nrrd *nrrd, unsigned int axis, unsigned int order) {
  NrrdAxisInfo *axinfo = NULL;
  unsigned int masked = 0, lablen;
  if (nrrd==NULL) return 1;
  if (nrrd->type!=nrrdTypeFloat && nrrd->type!=nrrdTypeDouble) return 2;
  if (axis>=nrrd->dim) return 3;
  axinfo = nrrd->axis+axis;
  if (order>tijk_max_esh_order) return 4;
  if (axinfo->size==tijk_esh_len[order/2]+1)
    masked = 1;
  else if (axinfo->size!=tijk_esh_len[order/2])
    return 5;
  axinfo->label = (char*) airFree(axinfo->label);
  lablen = strlen("tijk_esh_") + (masked?strlen("mask_"):0) + 3;
  axinfo->label = AIR_CALLOC(lablen, char);
  sprintf(axinfo->label, "tijk_%sesh_%02u", masked?"mask_":"", order);
  return 0;
}

/* tijk_set_axis_efs
 *
 * Marks a given nrrd axis as containing even-order fourier series of
 * a given order. Based on axis size, determines if values are masked.
 * Returns 0 on success
 *         1 if given NULL pointer
 *         2 if nrrd has wrong type (neither float nor double)
 *         3 if axis doesn't exist
 *         4 if order is unsupported
 *         5 if axis has wrong size
 */
int tijk_set_axis_efs(Nrrd *nrrd, unsigned int axis, unsigned int order) {
  NrrdAxisInfo *axinfo = NULL;
  unsigned int masked = 0, lablen;
  if (nrrd==NULL) return 1;
  if (nrrd->type!=nrrdTypeFloat && nrrd->type!=nrrdTypeDouble) return 2;
  if (axis>=nrrd->dim) return 3;
  axinfo = nrrd->axis+axis;
  if (order>tijk_max_efs_order) return 4;
  if (axinfo->size==order+2)
    masked = 1;
  else if (axinfo->size!=order+1)
    return 5;
  axinfo->label = (char*) airFree(axinfo->label);
  lablen = strlen("tijk_efs_") + (masked?strlen("mask_"):0) + 3;
  axinfo->label = AIR_CALLOC(lablen, char);
  sprintf(axinfo->label, "tijk_%sefs_%02u", masked?"mask_":"", order);
  return 0;
}

/* tijk_get_axis_type
 *
 * Extracts Tijk information from a given nrrd axis and writes it to info.
 * Returns 0 on success
 *           no "tijk_*" label counts as success -> tijk_class_unknown
 *         1 if given NULL pointer
 *         2 if nrrd has wrong type (neither float nor double)
 *         3 if axis doesn't exist
 *         4 if "tijk_*" label couldn't be parsed
 *         5 if label didn't fit axis size
 */
int tijk_get_axis_type(tijk_axis_info *info,
                       const Nrrd *nrrd, unsigned int axis) {
  const tijk_type *
    tijk_types[] = {
    NULL, /*  0: tijk_2o2d_unsym */
    NULL, /*  1: tijk_2o2d_sym */
    NULL, /*  2: tijk_2o2d_asym */
    NULL, /*  3: tijk_3o2d_sym */
    NULL, /*  4: tijk_4o2d_unsym */
    NULL, /*  5: tijk_4o2d_sym */
    NULL, /*  6: tijk_1o3d */
    NULL, /*  7: tijk_2o3d_unsym */
    NULL, /*  8: tijk_2o3d_sym */
    NULL, /*  9: tijk_2o3d_asym */
    NULL, /* 10: tijk_3o3d_unsym */
    NULL, /* 11: tijk_3o3d_sym */
    NULL, /* 12: tijk_4o3d_sym */
    NULL, /* 13: tijk_6o3d_sym */
    NULL, /* 14: tijk_8o3d_sym */
    NULL
  };
  const NrrdAxisInfo *axinfo = NULL;
  const char *labelp = NULL;
  unsigned int i=0;
  if (info==NULL || nrrd==NULL) return 1;
  if (nrrd->type!=nrrdTypeFloat && nrrd->type!=nrrdTypeDouble) return 2;
  if (axis>=nrrd->dim) return 3;
  axinfo = nrrd->axis+axis;
  if (axinfo->label==NULL || strncmp(axinfo->label, "tijk_", 5)) {
    info->tclass = tijk_class_unknown;
    return 0;
  }
  labelp = axinfo->label+5;
  if (!strncmp(labelp, "mask_", 5)) {
    info->masked = 1;
    labelp += 5;
  } else
    info->masked = 0;
  if (!strncmp(labelp, "esh_", 4)) {
    info->tclass = tijk_class_esh;
    if (1!=sscanf(labelp+4, "%02u", &(info->order)) ||
        info->order>tijk_max_esh_order || info->order%2!=0)
      return 4;
    if (axinfo->size!=tijk_esh_len[info->order/2]+info->masked)
      return 5;
    return 0;
  }
  if (!strncmp(labelp, "efs_", 4)) {
    info->tclass = tijk_class_efs;
    if (1!=sscanf(labelp+4, "%02u", &(info->order)) ||
        info->order>tijk_max_efs_order || info->order%2!=0)
      return 4;
    if (axinfo->size!=info->order+1+info->masked)
      return 5;
    return 0;
  }
  tijk_types[ 0] = tijk_2o2d_unsym;
  tijk_types[ 1] = tijk_2o2d_sym;
  tijk_types[ 2] = tijk_2o2d_asym;
  tijk_types[ 3] = tijk_3o2d_sym;
  tijk_types[ 4] = tijk_4o2d_unsym;
  tijk_types[ 5] = tijk_4o2d_sym;
  tijk_types[ 6] = tijk_1o3d;
  tijk_types[ 7] = tijk_2o3d_unsym;
  tijk_types[ 8] = tijk_2o3d_sym;
  tijk_types[ 9] = tijk_2o3d_asym;
  tijk_types[10] = tijk_3o3d_unsym;
  tijk_types[11] = tijk_3o3d_sym;
  tijk_types[12] = tijk_4o3d_sym;
  tijk_types[13] = tijk_6o3d_sym;
  tijk_types[14] = tijk_8o3d_sym;
  while (tijk_types[i]!=NULL) {
    if (!strcmp(labelp, tijk_types[i]->name)) {
      info->tclass = tijk_class_tensor;
      info->type = tijk_types[i];
      if (axinfo->size!=tijk_types[i]->num+info->masked)
        return 5;
      return 0;
    }
    i++;
  }
  return 4;
}
