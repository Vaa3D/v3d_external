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

#ifndef TIJK_PRIVATE_HAS_BEEN_INCLUDED
#define TIJK_PRIVATE_HAS_BEEN_INCLUDED

/* macros to facilitate definition of new tensor types */

/* for unsymmetric tensors */
#define TIJK_TYPE_UNSYM(name, order, dim, num)           \
  tijk_type                                              \
  _tijk_##name = {                                       \
    #name, order, dim, num,                              \
    NULL, NULL, NULL, NULL,                              \
    _tijk_##name##_tsp_d, _tijk_##name##_tsp_f,          \
    _tijk_##name##_norm_d, _tijk_##name##_norm_f,        \
    _tijk_##name##_trans_d, _tijk_##name##_trans_f,      \
    _tijk_##name##_convert_d, _tijk_##name##_convert_f,  \
    _tijk_##name##_approx_d, _tijk_##name##_approx_f,    \
    NULL, NULL, NULL, NULL,                              \
    NULL                                                 \
  };                                                     \
  const tijk_type *const tijk_##name = &_tijk_##name;

/* for partially symmetric and antisymmetric tensors */
#define TIJK_TYPE(name, order, dim, num)                 \
  tijk_type                                              \
  _tijk_##name = {                                       \
    #name, order, dim, num,                              \
    _tijk_##name##_mult, _tijk_##name##_unsym2uniq,      \
    _tijk_##name##_uniq2unsym, _tijk_##name##_uniq_idx,  \
    _tijk_##name##_tsp_d, _tijk_##name##_tsp_f,          \
    _tijk_##name##_norm_d, _tijk_##name##_norm_f,        \
    _tijk_##name##_trans_d, _tijk_##name##_trans_f,      \
    _tijk_##name##_convert_d, _tijk_##name##_convert_f,  \
    _tijk_##name##_approx_d, _tijk_##name##_approx_f,    \
    NULL, NULL, NULL, NULL,                              \
    NULL                                                 \
  };                                                     \
  const tijk_type *const tijk_##name = &_tijk_##name;

/* for totally symmetric tensors */
#define TIJK_TYPE_SYM(name, order, dim, num)                  \
  tijk_sym_fun                                                \
  _tijk_sym_fun_##name = {                                    \
    _tijk_##name##_s_form_d, _tijk_##name##_s_form_f,         \
    _tijk_##name##_mean_d, _tijk_##name##_mean_f,             \
    _tijk_##name##_var_d, _tijk_##name##_var_f,               \
    _tijk_##name##_v_form_d, _tijk_##name##_v_form_f,         \
    _tijk_##name##_m_form_d, _tijk_##name##_m_form_f,         \
    _tijk_##name##_grad_d, _tijk_##name##_grad_f,             \
    _tijk_##name##_hess_d, _tijk_##name##_hess_f,             \
    _tijk_##name##_make_rank1_d, _tijk_##name##_make_rank1_f, \
    _tijk_##name##_make_iso_d, _tijk_##name##_make_iso_f      \
  };                                                          \
  tijk_type                                                   \
  _tijk_##name = {                                            \
    #name, order, dim, num,                                   \
    _tijk_##name##_mult, _tijk_##name##_unsym2uniq,           \
    _tijk_##name##_uniq2unsym, _tijk_##name##_uniq_idx,       \
    _tijk_##name##_tsp_d, _tijk_##name##_tsp_f,               \
    _tijk_##name##_norm_d, _tijk_##name##_norm_f,             \
    _tijk_##name##_trans_d, _tijk_##name##_trans_f,           \
    _tijk_##name##_convert_d, _tijk_##name##_convert_f,       \
    _tijk_##name##_approx_d, _tijk_##name##_approx_f,         \
    NULL, NULL, NULL, NULL,                                   \
    &_tijk_sym_fun_##name                                     \
  };                                                          \
  const tijk_type *const tijk_##name = &_tijk_##name;


#endif /* TIJK_PRIVATE_HAS_BEEN_INCLUDED */
