#
# Teem: Tools to process and visualize scientific data and images              
# Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
# Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# (LGPL) as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# The terms of redistributing and/or modifying this software also
# include exceptions to the LGPL that facilitate static linking.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#


TEEM_LITTER = so_locations
TEEM_SHEXT = so
SHARED_LINK_NAME = -rpath 
TEEM_CPP_ERROR_DIE = -diag_error 1035
SHARED_RPATH = -rpath 

OPT_CFLAG ?= -O2
STATIC_CFLAG = -Bstatic
SHARED_CFLAG = -Bdynamic
SHARED_LDFLAG = -shared
ifeq ($(SUBARCH),n32)
  ARCH_CFLAG = -n32
  ARCH_LDFLAG = -n32
else
  ifeq ($(SUBARCH),64)
    ARCH_CFLAG = -64
    ARCH_LDFLAG = -64
  else 
    $(error irix6 sub-architecture "$(SUBARCH)" not recognized) 
  endif
endif

TEEM_QNANHIBIT = 0
TEEM_DIO = 1

TEEM_ZLIB.IPATH ?=
TEEM_ZLIB.LPATH ?=

TEEM_BZIP2.IPATH ?=
TEEM_BZIP2.LPATH ?=
